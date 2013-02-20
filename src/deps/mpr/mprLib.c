/*
    mprLib.c -- Multithreaded Portable Runtime Library Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: magnetar.local
 */

#include "mpr.h"

/************************************************************************/
/*
    Start of file "src/mem.c"
 */
/************************************************************************/

/**
    mem.c - Memory Allocator and Garbage Collector. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Defines ***********************************/

#ifndef BIT_MAX_GC_QUOTA
    #define BIT_MAX_GC_QUOTA   4096            /* Number of allocations before a GC is worthwhile */
#endif
#ifndef BIT_MAX_REGION
    #define BIT_MAX_REGION     (128 * 1024)    /* Memory allocation chunk size */
#endif

#if BIT_HAS_MMU 
    #define VALLOC 1                            /* Use virtual memory allocations */
#else
    #define VALLOC 0
#endif

#if BIT_MEMORY_DEBUG
/*
    Set this address to break when this address is allocated or freed
    Only used for debug, but defined regardless so we can have constant exports.
 */
static MprMem *stopAlloc = 0;
static int stopSeqno = -1;
#endif

#define GET_MEM(ptr)                ((MprMem*) (((char*) (ptr)) - sizeof(MprMem)))
#define GET_PTR(mp)                 ((char*) (((char*) mp) + sizeof(MprMem)))
#define GET_USIZE(mp)               ((ssize) (GET_SIZE(mp) - sizeof(MprMem) - (HAS_MANAGER(mp) * sizeof(void*))))
#define UNMARKED                    MPR_GEN_ETERNAL

#define GET_NEXT(mp)                (IS_LAST(mp)) ? NULL : ((MprMem*) ((char*) mp + GET_SIZE(mp)))
#define GET_REGION(mp)              ((MprRegion*) (((char*) mp) - MPR_ALLOC_ALIGN(sizeof(MprRegion))))

/*
    Macros to set and extract "prior" fields. All accesses (read and write) must be done locked.
        prior | last << 1 | hasManager
 */
#define GET_PRIOR(mp)               ((MprMem*) ((mp->field1 & MPR_MASK_PRIOR) >> MPR_SHIFT_PRIOR))
#define SET_PRIOR(mp, value)        mp->field1 = ((((size_t) value) << MPR_SHIFT_PRIOR) | (mp->field1 & ~MPR_MASK_PRIOR))
#define IS_LAST(mp)                 ((mp->field1 & MPR_MASK_LAST) >> MPR_SHIFT_LAST)
#define SET_LAST(mp, value)         mp->field1 = ((value << MPR_SHIFT_LAST) | (mp->field1 & ~MPR_MASK_LAST))
#define SET_FIELD1(mp, prior, last, hasManager) mp->field1 = (((size_t) prior) << MPR_SHIFT_PRIOR) | \
                                    ((last) << MPR_SHIFT_LAST) | ((hasManager) << MPR_SHIFT_HAS_MANAGER)

#define HAS_MANAGER(mp)             ((int) ((mp->field1 & MPR_MASK_HAS_MANAGER) >> MPR_SHIFT_HAS_MANAGER))
#define SET_HAS_MANAGER(mp, value)  mp->field1 = ((mp->field1 & ~MPR_MASK_HAS_MANAGER) | (value << MPR_SHIFT_HAS_MANAGER))

/*
    Macros to set and extract "size" fields. Accesses can be done unlocked. Updates must be done lock-free.
        gen/2 << 30 | free/1 << 29 | size/29 | mark/2
 */
#define GET_SIZE(mp)                ((ssize) ((mp->field2 & MPR_MASK_SIZE) >> MPR_SHIFT_SIZE))
#define SET_SIZE(mp, value)         mp->field2 = ((value) << MPR_SHIFT_SIZE) | (mp->field2 & ~MPR_MASK_SIZE)
#define IS_FREE(mp)                 ((mp->field2 & MPR_MASK_FREE) >> MPR_SHIFT_FREE)
#define SET_FREE(mp, value)         mp->field2 = (((size_t) (value)) << MPR_SHIFT_FREE) | (mp->field2 & ~MPR_MASK_FREE)
#define GET_GEN(mp)                 ((int) ((mp->field2 & MPR_MASK_GEN) >> MPR_SHIFT_GEN))
#define SET_GEN(mp, value)          mp->field2 = (((size_t) value) << MPR_SHIFT_GEN) | (mp->field2 & ~MPR_MASK_GEN)
#define GET_MARK(mp)                (mp->field2 & MPR_MASK_MARK)
#define SET_MARK(mp, value)         mp->field2 = (value) | (mp->field2 & ~MPR_MASK_MARK)
#define SET_FIELD2(mp, size, gen, mark, free) mp->field2 = \
                                        (((size_t) (gen)) << MPR_SHIFT_GEN) | \
                                        (((size_t) (free)) << MPR_SHIFT_FREE) | \
                                        ((size) << MPR_SHIFT_SIZE) | \
                                        ((mark) << MPR_SHIFT_MARK)
/*
    Padding fields (only manager stored in padding region)
 */
#define PAD_PTR(mp, offset)     ((void*) (((char*) mp) + GET_SIZE(mp) - ((offset) * sizeof(void*))))
#define MANAGER_SIZE            1
#define MANAGER_OFFSET          1
#define GET_MANAGER(mp)         ((MprManager) (*(void**) ((PAD_PTR(mp, MANAGER_OFFSET)))))
#define SET_MANAGER(mp, fn)     *((MprManager*) PAD_PTR(mp, MANAGER_OFFSET)) = fn

/*
    Memory checking and breakpoints
 */
#if BIT_MEMORY_DEBUG
#define BREAKPOINT(mp)          breakpoint(mp)
#define CHECK(mp)               mprCheckBlock((MprMem*) mp)
#define CHECK_FREE_MEMORY(mp)   checkFreeMem(mp)
#define CHECK_PTR(ptr)          CHECK(GET_MEM(ptr))
#define SCRIBBLE(mp)            if (heap->scribble && mp != GET_MEM(MPR)) { \
                                    memset((char*) mp + sizeof(MprFreeMem), 0xFE, GET_SIZE(mp) - sizeof(MprFreeMem)); \
                                } else
#define SCRIBBLE_RANGE(ptr, size) if (heap->scribble) { \
                                    memset((char*) ptr, 0xFE, size); \
                                } else
#define SET_MAGIC(mp)           mp->magic = MPR_ALLOC_MAGIC
#define SET_SEQ(mp)             mp->seqno = heap->nextSeqno++
#define VALID_BLK(mp)           validBlk(mp)
#define SET_NAME(mp, value)     mp->name = value

#else /* Release mode */
#define BREAKPOINT(mp)
#define CHECK(mp)           
#define CHECK_PTR(mp)           
#define SCRIBBLE(mp)           
#define SCRIBBLE_RANGE(ptr, size)
#define CHECK_FREE_MEMORY(mp)           
#define SET_NAME(mp, value)
#define SET_MAGIC(mp)
#define SET_SEQ(mp)           
#define VALID_BLK(mp)           1
#endif

#if BIT_MEMORY_STATS
    #define INC(field)          if (1) { heap->stats.field++; } else 
#else
    #define INC(field)
#endif

#define INIT_BLK(mp, size, hasManager, last, prior) if (1) { \
    SET_FIELD1(mp, prior, last, hasManager); \
    SET_FIELD2(mp, size, heap->active, heap->eternal, 0); \
    SET_MAGIC(mp); \
    SET_SEQ(mp); \
    SET_NAME(mp, NULL); \
    } else

#define lockHeap()              mprSpinLock(&heap->heapLock);
#define unlockHeap()            mprSpinUnlock(&heap->heapLock);

#define percent(a,b) ((int) ((a) * 100 / (b)))

/*
    Fast find first/last bit set
 */
#if LINUX
    #define NEED_FLSL 1
    #if BIT_CPU_ARCH == BIT_CPU_X86 || BIT_CPU_ARCH == BIT_CPU_X64
        #define USE_FLSL_ASM_X86 1
    #endif
    static MPR_INLINE int flsl(ulong word);

#elif BIT_WIN_LIKE
    #define NEED_FFSL 1
    #define NEED_FLSL 1
    static MPR_INLINE int ffsl(ulong word);
    static MPR_INLINE int flsl(ulong word);

#elif !BIT_BSD_LIKE
    #define NEED_FFSL 1
    #define NEED_FLSL 1
    static MPR_INLINE int ffsl(ulong word);
    static MPR_INLINE int flsl(ulong word);
#endif

/********************************** Data **************************************/

#undef              MPR
PUBLIC Mpr          *MPR;
static MprHeap      *heap;
static MprMemStats  memStats;
static int          padding[] = { 0, MANAGER_SIZE };

/***************************** Forward Declarations ***************************/

static void allocException(int cause, ssize size);
static void checkYielded();
static void dummyManager(void *ptr, int flags);
static ssize fastMemSize();
static void *getNextRoot();
static void getSystemInfo();
static void initGen();
static void mark();
static void marker(void *unused, MprThread *tp);
static void markRoots();
static void nextGen();
static int pauseThreads();
static void sweep();
static void resumeThreads();
static void triggerGC(int flags);

#if BIT_WIN_LIKE
    static int winPageModes(int flags);
#endif
#if BIT_MEMORY_DEBUG
    static void breakpoint(MprMem *mp);
    static int validBlk(MprMem *mp);
    static void checkFreeMem(MprMem *mp);
#endif
#if BIT_MEMORY_STATS
#if FUTURE
    static void showMem(MprMem *mp);
#endif
    static void freeLocation(cchar *name, ssize size);
    static void printQueueStats();
    static void printGCStats();
#endif
#if BIT_MEMORY_STACK
static void monitorStack();
#endif

static int initFree();
static MprMem *allocMem(ssize size, int flags);
static MprMem *freeBlock(MprMem *mp);
static int getQueueIndex(ssize size, int roundup);
static MprMem *growHeap(ssize size, int flags);
static void linkBlock(MprMem *mp); 
static void unlinkBlock(MprFreeMem *fp);
static void *vmalloc(ssize size, int mode);
static void vmfree(void *ptr, ssize size);
#if BIT_MEMORY_STATS
    static MprFreeMem *getQueue(ssize size);
#endif

/************************************* Code ***********************************/

PUBLIC Mpr *mprCreateMemService(MprManager manager, int flags)
{
    MprMem      *mp;
    MprMem      *spare;
    MprRegion   *region;
    ssize       size, mprSize, spareSize, regionSize;

    getSystemInfo();

    size = MPR_PAGE_ALIGN(sizeof(MprHeap), memStats.pageSize);
    if ((heap = vmalloc(size, MPR_MAP_READ | MPR_MAP_WRITE)) == NULL) {
        return NULL;
    }
    memset(heap, 0, sizeof(MprHeap));
    heap->stats.numCpu = memStats.numCpu;
    heap->stats.pageSize = memStats.pageSize;
    heap->stats.maxMemory = MAXINT;
    heap->stats.redLine = MAXINT / 100 * 95;
    mprInitSpinLock(&heap->heapLock);
    initGen();

    /*
        Hand-craft the Mpr structure
     */
    mprSize = MPR_ALLOC_ALIGN(sizeof(MprMem) + sizeof(Mpr) + (MANAGER_SIZE * sizeof(void*)));
    regionSize = MPR_ALLOC_ALIGN(sizeof(MprRegion));
    size = max(mprSize + regionSize, BIT_MAX_REGION);
    if ((region = mprVirtAlloc(size, MPR_MAP_READ | MPR_MAP_WRITE)) == NULL) {
        return NULL;
    }
    mp = region->start = (MprMem*) (((char*) region) + regionSize);
    region->size = size;

    MPR = (Mpr*) GET_PTR(mp);
    INIT_BLK(mp, mprSize, 1, 0, NULL);
    SET_MANAGER(mp, manager);
    mprSetName(MPR, "Mpr");
    MPR->heap = heap;

    heap->flags = flags | MPR_THREAD_PATTERN;
    heap->nextSeqno = 1;
    heap->chunkSize = BIT_MAX_REGION;
    heap->stats.maxMemory = MAXINT;
    heap->stats.redLine = MAXINT / 100 * 99;
    heap->newQuota = BIT_MAX_GC_QUOTA;
    heap->earlyYieldQuota = heap->newQuota * 5;
    heap->enabled = !(heap->flags & MPR_DISABLE_GC);
    if (scmp(getenv("MPR_DISABLE_GC"), "1") == 0) {
        heap->enabled = 0;
    }
    if (scmp(getenv("MPR_VERIFY_MEM"), "1") == 0) {
        heap->verify = 1;
    }
    if (scmp(getenv("MPR_SCRIBBLE_MEM"), "1") == 0) {
        heap->scribble = 1;
    }
    if (scmp(getenv("MPR_TRACK_MEM"), "1") == 0) {
        heap->track = 1;
    }
    heap->stats.bytesAllocated += size;
    heap->stats.regions++;
    INC(allocs);

    mprInitSpinLock(&heap->heapLock);
    mprInitSpinLock(&heap->rootLock);
    initGen();
    initFree();

    spareSize = size - regionSize - mprSize;
    if (spareSize > 0) {
        spare = (MprMem*) (((char*) mp) + mprSize);
        INIT_BLK(spare, size - regionSize - mprSize, 0, 1, mp);
        SET_GEN(spare, heap->eternal);
        SET_FREE(spare, 1);
        heap->regions = region;
        SCRIBBLE(spare);
        linkBlock(spare);
    }
    heap->markerCond = mprCreateCond();
    heap->mutex = mprCreateLock();
    heap->roots = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    mprAddRoot(MPR);
    return MPR;
}


/*
    Shutdown memory service. Run managers on all allocated blocks.
 */
PUBLIC void mprDestroyMemService()
{
    volatile MprRegion  *region;
    MprMem              *mp, *next;

    if (heap->destroying) {
        return;
    }
    heap->destroying = 1;
    for (region = heap->regions; region; region = region->next) {
        for (mp = region->start; mp; mp = next) {
            next = GET_NEXT(mp);
            if (unlikely(HAS_MANAGER(mp))) {
                (GET_MANAGER(mp))(GET_PTR(mp), MPR_MANAGE_FREE);
                SET_HAS_MANAGER(mp, 0);
            }
        }
    }
}


/*
    Note this does not zero memory and thus will not pin allocated memory if it is newly, virtually allocated
 */
PUBLIC void *mprAllocMem(ssize usize, int flags)
{
    MprMem      *mp;
    void        *ptr;
    ssize       size;
    int         padWords;

    assert(!heap->marking);
    assert(usize >= 0);

    padWords = padding[flags & MPR_ALLOC_PAD_MASK];
    size = usize + sizeof(MprMem) + (padWords * sizeof(void*));
    size = max(size, usize + (ssize) sizeof(MprFreeMem));
    size = MPR_ALLOC_ALIGN(size);
    
    if ((mp = allocMem(size, flags)) == NULL) {
        return NULL;
    }
    ptr = GET_PTR(mp);
    if (flags & MPR_ALLOC_ZERO) {
        /* Note: real usize may be bigger than requested */
        memset(ptr, 0, GET_USIZE(mp));
    }
    BREAKPOINT(mp);
    CHECK(mp);
    assert(GET_GEN(mp) != heap->eternal);
    return ptr;
}


/*
    Realloc will always zero new memory
 */
PUBLIC void *mprReallocMem(void *ptr, ssize usize)
{
    MprMem      *mp, *newb;
    void        *newptr;
    ssize       oldSize, oldUsize;
    int         flags, hasManager;

    assert(usize > 0);

    if (ptr == 0) {
        return mprAllocMem(usize, 0);
    }
    mp = GET_MEM(ptr);
    CHECK(mp);
    assert(!IS_FREE(mp));
    assert(GET_GEN(mp) != heap->dead);
    oldUsize = GET_USIZE(mp);

    if (usize <= oldUsize) {
        return ptr;
    }
    hasManager = HAS_MANAGER(mp);
    flags = hasManager ? MPR_ALLOC_MANAGER : 0;
    if ((newptr = mprAllocMem(usize, flags)) == NULL) {
        return 0;
    }
    newb = GET_MEM(newptr);
    if (hasManager) {
        SET_MANAGER(newb, GET_MANAGER(mp));
    }
    if (GET_GEN(mp) == heap->eternal) {
        /* Lock-free update */
        SET_FIELD2(newb, GET_SIZE(newb), heap->eternal, UNMARKED, 0);
    }
    oldSize = GET_SIZE(mp);
    memcpy(newptr, ptr, oldSize - sizeof(MprMem));
    /* Note: real usize may be bigger than requested */
    memset(&((char*) newptr)[oldUsize], 0, GET_USIZE(newb) - oldUsize);
    return newptr;
}


PUBLIC void *mprMemdupMem(cvoid *ptr, ssize usize)
{
    char    *newp;

    if ((newp = mprAllocMem(usize, 0)) != 0) {
        memcpy(newp, ptr, usize);
    }
    return newp;
}


PUBLIC int mprMemcmp(cvoid *s1, ssize s1Len, cvoid *s2, ssize s2Len)
{
    int         rc;

    assert(s1);
    assert(s2);
    assert(s1Len >= 0);
    assert(s2Len >= 0);

    if ((rc = memcmp(s1, s2, min(s1Len, s2Len))) == 0) {
        if (s1Len < s2Len) {
            return -1;
        } else if (s1Len > s2Len) {
            return 1;
        }
    }
    return rc;
}


/*
    mprMemcpy will support insitu copy where src and destination overlap
 */
PUBLIC ssize mprMemcpy(void *dest, ssize destMax, cvoid *src, ssize nbytes)
{
    assert(dest);
    assert(destMax <= 0 || destMax >= nbytes);
    assert(src);
    assert(nbytes >= 0);

    if (destMax > 0 && nbytes > destMax) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    if (nbytes > 0) {
        memmove(dest, src, nbytes);
        return nbytes;
    } else {
        return 0;
    }
}

/*************************** Virtual Heap Allocations *************************/
/*
    Initialize the free space map and queues.

    The free map is a two dimensional array of free queues. The first dimension is indexed by
    the most significant bit (MSB) set in the requested block size. The second dimension is the next 
    MPR_ALLOC_BUCKET_SHIFT (4) bits below the MSB.

    +-------------------------------+
    |       |MSB|  Bucket   | rest  |
    +-------------------------------+
    | 0 | 0 | 1 | 1 | 1 | 1 | X | X |
    +-------------------------------+
 */
static int initFree() 
{
    MprFreeMem  *freeq;
#if BIT_MEMORY_STATS
    ssize       bit, size, groupBits, bucketBits;
    int         index, group, bucket;
#endif
    
    heap->freeEnd = &heap->freeq[MPR_ALLOC_NUM_GROUPS * MPR_ALLOC_NUM_BUCKETS];
    for (freeq = heap->freeq; freeq != heap->freeEnd; freeq++) {
#if BIT_MEMORY_STATS
        /*
            NOTE: skip the buckets with MSB == 0 (round up)
         */
        index = (int) (freeq - heap->freeq);
        group = index / MPR_ALLOC_NUM_BUCKETS;
        bucket = index % MPR_ALLOC_NUM_BUCKETS;

        bit = (group != 0);
        groupBits = bit << (group + MPR_ALLOC_BUCKET_SHIFT - 1);
        bucketBits = ((ssize) bucket) << (max(0, group - 1));

        size = groupBits | bucketBits;
        freeq->info.stats.minSize = (int) (size << MPR_ALIGN_SHIFT);
#endif
        freeq->next = freeq->prev = freeq;
    }
    return 0;
}


static MprMem *allocMem(ssize required, int flags)
{
    MprFreeMem  *freeq, *fp;
    MprMem      *mp, *after, *spare;
    ssize       size, maxBlock;
    ulong       groupMap, bucketMap;
    int         bucket, baseGroup, group, index, miss;
    
#if BIT_MEMORY_STACK
    monitorStack();
#endif

    index = getQueueIndex(required, 1);
    baseGroup = index / MPR_ALLOC_NUM_BUCKETS;
    bucket = index % MPR_ALLOC_NUM_BUCKETS;
    heap->newCount += index;
    INC(requests);

    /*
        OPT - could break this locked section up.
        - Can update bit maps conservatively and lockfree
        - Put locks around freeq unqueue
        - use unlinkBlock or linkBlock only. Do locks internally in these routines
        - Probably need unlinkFirst
        - Long term use lockfree
     */
    lockHeap();
    
    /* Mask groups lower than the base group */
    miss = 0;
    groupMap = heap->groupMap & ~((((ssize) 1) << baseGroup) - 1);
    while (groupMap) {
        group = (int) (ffsl(groupMap) - 1);
        if (groupMap & ((((ssize) 1) << group))) {
            bucketMap = heap->bucketMap[group];
            if (baseGroup == group) {
                /* Mask buckets lower than the base bucket */
                bucketMap &= ~((((ssize) 1) << bucket) - 1);
            }
            while (bucketMap) {
                bucket = (int) (ffsl(bucketMap) - 1);
                index = (group * MPR_ALLOC_NUM_BUCKETS) + bucket;
                freeq = &heap->freeq[index];

                if (freeq->next != freeq) {
                    fp = freeq->next;
                    mp = (MprMem*) fp;
                    assert(IS_FREE(mp));
                    unlinkBlock(fp);

                    assert(GET_GEN(mp) == heap->eternal);
                    SET_GEN(mp, heap->active);

                    //  OPT
                    mprAtomicBarrier();
                    if (flags & MPR_ALLOC_MANAGER) {
                        SET_MANAGER(mp, dummyManager);
                        SET_HAS_MANAGER(mp, 1);
                    }
                    INC(reuse);
                    CHECK(mp);
                    CHECK_FREE_MEMORY(mp);
                    if (GET_SIZE(mp) >= (ssize) (required + MPR_ALLOC_MIN_SPLIT)) {
                        maxBlock = (((ssize) 1 ) << group | (((ssize) bucket) << (max(0, group - 1)))) << MPR_ALIGN_SHIFT;
                        maxBlock += sizeof(MprMem);

                        size = GET_SIZE(mp);
                        if (size > maxBlock) {
                            spare = (MprMem*) ((char*) mp + required);
                            INIT_BLK(spare, size - required, 0, IS_LAST(mp), mp);
                            if ((after = GET_NEXT(spare)) != NULL) {
                                SET_PRIOR(after, spare);
                            }
                            SET_SIZE(mp, required);
                            mprAtomicBarrier();
                            SET_LAST(mp, 0);
                            mprAtomicBarrier();
                            INC(splits);
                            linkBlock(spare);
                        }
                    }
                    /* Tested empirically to trigger GC when we are searching too much for an allocation */
                    if (miss > 9) {
                        triggerGC(0);
                    }
                    unlockHeap();
                    return mp;
                }
                bucketMap &= ~(((ssize) 1) << bucket);
                heap->bucketMap[group] &= ~(((ssize) 1) << bucket);
            }
            groupMap &= ~(((ssize) 1) << group);
            heap->groupMap &= ~(((ssize) 1) << group);
            miss++;
        }
    }
    unlockHeap();
    triggerGC(0);
    return growHeap(required, flags);
}


/*
    Grow the heap and return a block of the required size (unqueued)
 */
static MprMem *growHeap(ssize required, int flags)
{
    MprRegion           *region;
    MprMem              *mp, *spare;
    ssize               size, rsize, spareLen;
    int                 hasManager;

    assert(required > 0);

    rsize = MPR_ALLOC_ALIGN(sizeof(MprRegion));
    size = max(required + rsize, (ssize) heap->chunkSize);
    size = MPR_PAGE_ALIGN(size, memStats.pageSize);
    if (size < 0 || size >= ((ssize) 1 << MPR_SIZE_BITS)) {
        allocException(MPR_MEM_TOO_BIG, size);
        return 0;
    }
#if KEEP
{
    static ssize hiwat = 0;
    ssize used = mprGetMem();
    if (used > hiwat) {
        // printf("Grow %ld K, new total %ld K\n", size / 1024, (used + size) / 1024);
        hiwat = used;
    }
}
#endif
    if ((region = mprVirtAlloc(size, MPR_MAP_READ | MPR_MAP_WRITE)) == NULL) {
        return 0;
    }
    mprInitSpinLock(&((MprRegion*) region)->lock);
    region->size = size;
    region->start = (MprMem*) (((char*) region) + rsize);
    region->freeable = 0;
    mp = (MprMem*) region->start;
    hasManager = (flags & MPR_ALLOC_MANAGER) ? 1 : 0;
    spareLen = size - required - rsize;
    if (spareLen < sizeof(MprFreeMem)) {
        required = size - rsize; 
        spareLen = 0;
    }
    INIT_BLK(mp, required, hasManager, spareLen > 0 ? 0 : 1, NULL);
    if (hasManager) {
        SET_MANAGER(mp, dummyManager);
    }
    CHECK(mp);

    lockHeap();
    region->next = heap->regions;
    heap->stats.bytesAllocated += size;
    heap->regions = region;
    heap->stats.regions++;
    if (spareLen > 0) {
        assert(spareLen >= sizeof(MprFreeMem));
        spare = (MprMem*) ((char*) mp + required);
        INIT_BLK(spare, spareLen, 0, 1, mp);
        CHECK(spare);
        INC(allocs);
        linkBlock(spare);
    } else {
        INC(allocs);
    }
    unlockHeap();
    return mp;
}


/*
    Free a block. MUST only ever be called by the sweeper. The sweeper takes advantage of the fact that only it 
    coalesces blocks.
 */
static MprMem *freeBlock(MprMem *mp)
{
    MprMem      *prev, *next, *after;
    MprRegion   *region;
    ssize       size;

    BREAKPOINT(mp);
    SCRIBBLE(mp);
    size = GET_SIZE(mp);
    prev = NULL;
    lockHeap();
    
    /*
        Coalesce with next if it is free
     */
    next = GET_NEXT(mp);
    if (next && IS_FREE(next)) {
        BREAKPOINT(next);
        unlinkBlock((MprFreeMem*) next);
        if ((after = GET_NEXT(next)) != NULL) {
            assert(GET_PRIOR(after) == next);
            SET_PRIOR(after, mp);
        } else {
            SET_LAST(mp, 1);
        }
        size += GET_SIZE(next);
        SET_SIZE(mp, size);
        INC(joins);
        SCRIBBLE_RANGE(next, sizeof(MprFreeMem));
    }

    /*
        Coalesce with previous if it is free
     */
    prev = GET_PRIOR(mp);
    if (prev && IS_FREE(prev)) {
        BREAKPOINT(prev);
        unlinkBlock((MprFreeMem*) prev);
        if ((after = GET_NEXT(mp)) != NULL) {
            assert(GET_PRIOR(after) == mp);
            SET_PRIOR(after, prev);
        } else {
            SET_LAST(prev, 1);
        }
        size += GET_SIZE(prev);
        SET_SIZE(prev, size);
        SCRIBBLE_RANGE(mp, sizeof(MprFreeMem));
        mp = prev;
        INC(joins);
        prev = GET_PRIOR(mp);
        if (prev) {
            CHECK(prev);
        }
        assert(prev == 0 || !IS_FREE(prev));
    }
    next = GET_NEXT(mp);

    /*
        Release entire regions back to the O/S. (Blocks equal to Empty regions have no prior and are last)
     */
    if (GET_PRIOR(mp) == NULL && IS_LAST(mp) && heap->stats.bytesFree > (BIT_MAX_REGION * 4)) {
        INC(unpins);
        unlockHeap();
        region = GET_REGION(mp);
        region->freeable = 1;
        assert(next == NULL);
    } else {
        linkBlock(mp);
        unlockHeap();
    }
    /*
        WARN: there is a race here. Another thread may allocate and split the block just freed. So next will be
        pessimistic and there may be newly created intervening blocks.
     */
    return next;
}


static int getQueueIndex(ssize size, int roundup)
{   
    ssize       usize;
    int         asize, aligned, bucket, group, index, msb;
    
    assert(MPR_ALLOC_ALIGN(size) == size);

    /*
        Allocate based on user sizes (sans header). This permits block searches to avoid scanning the next 
        highest queue for common block sizes: eg. 1K.
     */
    usize = (size - sizeof(MprMem));
    asize = (int) (usize >> MPR_ALIGN_SHIFT);

    /* Zero based most significant bit */
    msb = (flsl((int) asize) - 1);

    group = max(0, msb - MPR_ALLOC_BUCKET_SHIFT + 1);
    assert(group < MPR_ALLOC_NUM_GROUPS);

    bucket = (asize >> max(0, group - 1)) & (MPR_ALLOC_NUM_BUCKETS - 1);
    assert(bucket < MPR_ALLOC_NUM_BUCKETS);

    index = (group * MPR_ALLOC_NUM_BUCKETS) + bucket;
    assert(index < (heap->freeEnd - heap->freeq));
    
#if BIT_MEMORY_STATS
    assert(heap->freeq[index].info.stats.minSize <= (int) usize && 
        (int) usize < heap->freeq[index + 1].info.stats.minSize);
#endif
    if (roundup) {
        /*
            Good-fit strategy: check if the requested size is the smallest possible size in a queue. If not the smallest,
            must look at the next queue higher up to guarantee a block of sufficient size.
            Blocks of of size <= 512 bytes (0x20 shifted) are mapped directly to queues. ie. There is only one block size
            per queue. Otherwise, get a mask of the bits below the group and bucket bits. If any are set, then not the 
            lowest size in the queue.
         */
        if (asize > 0x20) {
            ssize mask = (((ssize) 1) << (msb - MPR_ALLOC_BUCKET_SHIFT)) - 1;
            aligned = (asize & mask) == 0;
            if (!aligned) {
                index++;
            }
        }
    }
    return index;
}


/*
    Add a block to a free q. Must be called locked.
    Called by user threads from allocMem and by sweeper from freeBlock.
 */
static void linkBlock(MprMem *mp) 
{
    MprFreeMem  *freeq, *fp;
    ssize       size;
    int         index, group, bucket;

    CHECK(mp);

    /* 
        Mark block as free and eternal so sweeper will skip 
     */
    size = GET_SIZE(mp);
    SET_FIELD2(mp, size, heap->eternal, UNMARKED, 1);
    SET_HAS_MANAGER(mp, 0);
    
    /*
        Set free space bitmap
     */
    index = getQueueIndex(size, 0);
    group = index / MPR_ALLOC_NUM_BUCKETS;
    bucket = index % MPR_ALLOC_NUM_BUCKETS;
    heap->groupMap |= (((ssize) 1) << group);
    heap->bucketMap[group] |= (((ssize) 1) << bucket);

    /*
        Link onto free queue
     */
    fp = (MprFreeMem*) mp;
    freeq = &heap->freeq[index];
    assert(fp != freeq);
    fp->next = freeq->next;
    fp->prev = freeq;
    freeq->next->prev = fp;
    freeq->next = fp;
    assert(fp != fp->next);
    assert(fp != fp->prev);

    heap->stats.bytesFree += size;
#if BIT_MEMORY_STATS
    freeq->info.stats.count++;
#endif
}


/*
    Remove a block from a free q. Must be called locked.
 */
static void unlinkBlock(MprFreeMem *fp) 
{
    MprMem  *mp;
    ssize   size;

    CHECK(fp);
    fp->prev->next = fp->next;
    fp->next->prev = fp->prev;
#if BIT_MEMORY_DEBUG
    fp->next = fp->prev = NULL;
#endif

    mp = (MprMem*) fp;
    size = GET_SIZE(mp);
    heap->stats.bytesFree -= size;
    assert(IS_FREE(mp));
    SET_FREE(mp, 0);
    mprAtomicBarrier();
#if BIT_MEMORY_STATS
{
    MprFreeMem *freeq = getQueue(size);
    freeq->info.stats.count--;
    assert(freeq->info.stats.count >= 0);
}
#endif
}


#if BIT_MEMORY_STATS
static MprFreeMem *getQueue(ssize size)
{   
    MprFreeMem  *freeq;
    int         index;
    
    index = getQueueIndex(size, 0);
    freeq = &heap->freeq[index];
    return freeq;
}
#endif


/*
    Allocate virtual memory and check a memory allocation request against configured maximums and redlines. 
    An application-wide memory allocation failure routine can be invoked from here when a memory redline is exceeded. 
    It is the application's responsibility to set the red-line value suitable for the system.
    Memory is zereod on all platforms.
 */
PUBLIC void *mprVirtAlloc(ssize size, int mode)
{
    ssize       used;
    void        *ptr;

    used = fastMemSize();
    if (memStats.pageSize) {
        size = MPR_PAGE_ALIGN(size, memStats.pageSize);
    }
    if ((size + used) > heap->stats.maxMemory) {
        allocException(MPR_MEM_LIMIT, size);
    } else if ((size + used) > heap->stats.redLine) {
        allocException(MPR_MEM_REDLINE, size);
    }
    if ((ptr = vmalloc(size, mode)) == 0) {
        allocException(MPR_MEM_FAIL, size);
        return 0;
    }
    return ptr;
}


PUBLIC void mprVirtFree(void *ptr, ssize size)
{
    vmfree(ptr, size);
}


static void *vmalloc(ssize size, int mode)
{
    void    *ptr;

#if VALLOC
    #if BIT_UNIX_LIKE
        if ((ptr = mmap(0, size, mode, MAP_PRIVATE | MAP_ANON, -1, 0)) == (void*) -1) {
            return 0;
        }
    #elif BIT_WIN_LIKE
        ptr = VirtualAlloc(0, size, MEM_RESERVE | MEM_COMMIT, winPageModes(mode));
    #else
        if ((ptr = malloc(size)) != 0) {
            memset(ptr, 0, size);
        }
    #endif
#else
    if ((ptr = malloc(size)) != 0) {
        memset(ptr, 0, size);
    }
#endif
    return ptr;
}


static void vmfree(void *ptr, ssize size)
{
#if VALLOC
    #if BIT_UNIX_LIKE
        if (munmap(ptr, size) != 0) {
            assert(0);
        }
    #elif BIT_WIN_LIKE
        VirtualFree(ptr, 0, MEM_RELEASE);
    #else
        if (heap->scribble) {
            memset(ptr, 0x11, size);
        }
        free(ptr);
    #endif
#else
    free(ptr);
#endif
}


/***************************************************** Garbage Colllector *************************************************/

PUBLIC void mprStartGCService()
{
    if (heap->enabled) {
        if (heap->flags & MPR_MARK_THREAD) {
            mprTrace(7, "DEBUG: startMemWorkers: start marker");
            if ((heap->marker = mprCreateThread("marker", marker, NULL, 0)) == 0) {
                mprError("Cannot create marker thread");
                MPR->hasError = 1;
            } else {
                mprStartThread(heap->marker);
            }
        }
#if FUTURE && KEEP
        if (heap->flags & MPR_SWEEP_THREAD) {
            mprTrace(7, "DEBUG: startMemWorkers: start sweeper");
            heap->hasSweeper = 1;
            if ((heap->sweeper = mprCreateThread("sweeper", sweeper, NULL, 0)) == 0) {
                mprError("Cannot create sweeper thread");
                MPR->hasError = 1;
            } else {
                mprStartThread(heap->sweeper);
            }
        }
#endif
    }
}


PUBLIC void mprStopGCService()
{
    mprWakeGCService();
    mprNap(1);
}


PUBLIC void mprWakeGCService()
{
    mprSignalCond(heap->markerCond);
    mprResumeThreads();
}


static void triggerGC(int flags)
{
    if (!heap->gcRequested && ((flags & MPR_GC_FORCE) || (heap->newCount > heap->newQuota))) {
        heap->gcRequested = 1;
#if !PARALLEL_GC
        heap->mustYield = 1;
#endif
        if (heap->flags & MPR_MARK_THREAD) {
            mprSignalCond(heap->markerCond);
        }
    }
}


PUBLIC void mprRequestGC(int flags)
{
    int     i, count;

    mprTrace(7, "DEBUG: mprRequestGC");

    count = (flags & MPR_GC_COMPLETE) ? 3 : 1;
    for (i = 0; i < count; i++) {
        if ((flags & MPR_GC_FORCE) || (heap->newCount > heap->newQuota)) {
#if PARALLEL_GC
            heap->mustYield = 1;
#endif
            triggerGC(MPR_GC_FORCE);
        }
        if (!(flags & MPR_GC_NO_YIELD)) {
            mprYield((flags & MPR_GC_NO_BLOCK) ? MPR_YIELD_NO_BLOCK: 0);
        }
    }
}


/*
    Marker synchronization point. At the end of each GC mark/sweep, all threads must rendezvous at the 
    synchronization point.  This happens infrequently and is essential to safely move to a new generation.
    All threads must yield to the marker (including sweeper)
 */
static void resumeThreads()
{
#if BIT_MEMORY_STATS
    mprTrace(7, "GC: MARKED %,d/%,d, SWEPT %,d/%,d, freed %,d, bytesFree %,d (prior %,d), newCount %,d/%,d, " 
            "blocks %,d bytes %,d",
            heap->stats.marked, heap->stats.markVisited, heap->stats.swept, heap->stats.sweepVisited, 
            (int) heap->stats.freed, (int) heap->stats.bytesFree, (int) heap->priorFree, heap->priorNewCount, heap->newQuota,
            heap->stats.sweepVisited - heap->stats.swept, (int) heap->stats.bytesAllocated);
#endif
#if PARALLEL_GC
    heap->mustYield = 1;
    if (heap->notifier) {
        (heap->notifier)(MPR_MEM_ATTENTION, 0);
    }
    if (pauseThreads()) {
        nextGen();
    } else {
        mprTrace(7, "DEBUG: Pause for GC sync timed out");
    }
#endif
    heap->mustYield = 0;
    mprResumeThreads();
}


static void mark()
{
    mprTrace(7, "GC: mark started");

    /*
        When parallel, we mark blocks using the current heap->active mark. After marking, synchronization will rotate
        the active/stale/dead markers. After this, existing alive blocks may be marked stale. No blocks will be marked
        active.
        When !parallel, we swap the active/dead markers first and mark all blocks. After marking and synchronization, 
        existing alive blocks will always be marked active.
     */
#if PARALLEL_GC
    if (heap->newCount > heap->earlyYieldQuota) {
        heap->mustYield = 1;
    }
#else
    heap->mustYield = 1;
    if (!pauseThreads()) {
        mprTrace(7, "DEBUG: GC synchronization timed out, some threads did not yield.");
        mprTrace(7, "This is most often caused by a thread doing a long running operation and not first calling mprYield.");
        mprTrace(7, "If debugging, run the process with -D to enable debug mode.");
        return;
    }
    nextGen();
#endif
    heap->priorNewCount = heap->newCount;
    heap->priorFree = heap->stats.bytesFree;
    heap->newCount = 0;
    heap->gcRequested = 0;
    checkYielded();
    markRoots();
    heap->marking = 0;
    if (!heap->hasSweeper) {
        heap->sweeping = 1;
        MPR_MEASURE(7, "GC", "sweep", sweep());
        heap->sweeping = 0;
    }
    resumeThreads();
}


/*
    Sweep up the garbage.
    WARNING: This code uses lock-free algorithms. The sweeper traverses the region list and block list without locking. 
    Other code must similarly use lock-free code -- only add regions to the start of the regions list and never 
    otherwise modify the region list. Other code may modify blocks on the list, but must atomically update MprMem.field1.
    The sweeper is the only routine to do coalesing, other code may split blocks, but this can be done in a lock-free 
    manner by creating the spare 2nd half block first and then updating mp->field2 with the size and last bit.
*/
static void sweep()
{
    MprRegion   *region, *nextRegion, *prior;
    MprMem      *mp, *next;
    MprManager  mgr;
    
    if (!heap->enabled) {
        mprTrace(7, "DEBUG: sweep: Abort sweep - GC disabled");
        return;
    }
    mprTrace(7, "GC: sweep started");
    heap->stats.freed = 0;

    if (heap->newCount > heap->earlyYieldQuota) {
        heap->mustYield = 1;
    }

    /*
        Run all destructors first so all destructors can guarantee dependant memory blocks will still exist.
        Actually free the memory in a 2nd pass below.
     */
    for (region = heap->regions; region; region = region->next) {
        /*
            This code assumes that no other code coalesces blocks and that splitting blocks will be done lock-free
         */
        for (mp = region->start; mp; mp = GET_NEXT(mp)) {
            if (unlikely(GET_GEN(mp) == heap->dead && HAS_MANAGER(mp))) {
                mgr = GET_MANAGER(mp);
                assert(!IS_FREE(mp));
                CHECK(mp);
                BREAKPOINT(mp);
                if (mgr && VALID_BLK(mp)) {
                    (mgr)(GET_PTR(mp), MPR_MANAGE_FREE);
                }
            }
        }
    }
#if BIT_MEMORY_STATS
    heap->stats.sweepVisited = 0;
    heap->stats.swept = 0;
#endif

    /*
        growHeap() will append new regions to the front of heap->regions and so will not race with this code. This code
        is the only code that frees regions.
        RACE: Take from the front. Racing with growHeap.
     */
    prior = NULL;
    for (region = heap->regions; region; region = nextRegion) {
        assert(region->freeable == 0 || region->freeable == 1);
        nextRegion = region->next;

        /*
            This code assumes that no other code coalesces blocks and that splitting blocks will be done lock-free
         */
        for (mp = region->start; mp; mp = next) {
            CHECK(mp);
            INC(sweepVisited);
            if (unlikely(GET_GEN(mp) == heap->dead)) {
                assert(!IS_FREE(mp));
                CHECK(mp);
                BREAKPOINT(mp);
                INC(swept);
#if BIT_MEMORY_STATS
                if (heap->track) {
                    freeLocation(mp->name, GET_SIZE(mp));
                }
#endif
                heap->stats.freed += GET_SIZE(mp);
                next = freeBlock(mp);
            } else {
                /*
                    RACE: Block could be allocated here, but will never be coalesced (sweeper is the only one to do that).
                    So mp->field2 may be reduced so we may skip a newly created block -- no problem. Get it next scan.
                 */
                next = GET_NEXT(mp);
            }
        }
        /*
            The sweeper is the only one who removes regions. 
            Currently all threads are suspended so no locks needed. FUTURE - When doing parallel collection, do this 
            lock-free because user code traverses the region list.
         */ 
        if (region->freeable) {
            lockHeap();
            if (prior) {
                prior->next = nextRegion;
            } else {
                heap->regions = nextRegion;
            }
            heap->stats.regions--;
            heap->stats.bytesAllocated -= region->size;
            assert(heap->stats.bytesAllocated >= 0);
            unlockHeap();
            mprTrace(9, "DEBUG: Unpin %p to %p size %d, used %d", region, 
                ((char*) region) + region->size, region->size,fastMemSize());
            mprManageSpinLock(&region->lock, MPR_MANAGE_FREE);
            mprVirtFree(region, region->size);
        } else {
            prior = region;
        }
    }
}


static void markRoots()
{
    void    *root;

#if BIT_MEMORY_STATS
    heap->stats.markVisited = 0;
    heap->stats.marked = 0;
#endif
    mprMark(heap->roots);
    mprMark(heap->mutex);
    mprMark(heap->markerCond);

    heap->rootIndex = 0;
    while ((root = getNextRoot()) != 0) {
        checkYielded();
        mprMark(root);
    }
    heap->rootIndex = -1;
}


PUBLIC void mprMarkBlock(cvoid *ptr)
{
    MprMem      *mp;
    int         gen;
#if BIT_DEBUG
    static int  depth = 0;
#endif

    if (ptr == 0) {
        return;
    }
    mp = MPR_GET_MEM(ptr);
#if BIT_DEBUG
    if (!mprIsValid(ptr)) {
        mprError("Memory block is either not dynamically allocated, or is corrupted");
        return;
    }
    assert(!IS_FREE(mp));
#if PARALLEL_GC
    assert(GET_MARK(mp) != heap->dead);
    assert(GET_GEN(mp) != heap->dead);
    if (GET_MARK(mp) == heap->dead || IS_FREE(mp)) {
        assert(0);
        return;
    }
#endif
#endif
    CHECK(mp);
    INC(markVisited);
    assert((GET_MARK(mp) != heap->active) || GET_GEN(mp) == heap->active);

    if (GET_MARK(mp) != heap->active) {
        BREAKPOINT(mp);
        INC(marked);
        gen = GET_GEN(mp);
        if (gen != heap->eternal) {
            gen = heap->active;
        }
        /* Lock-free update */
        SET_FIELD2(mp, GET_SIZE(mp), gen, heap->active, 0);
        if (HAS_MANAGER(mp)) {
#if BIT_DEBUG
            if (++depth > 400) {
                fprintf(stderr, "WARNING: Possibly too much recursion. Marking depth exceeds 400\n");
                mprBreakpoint();
            }
#endif
            (GET_MANAGER(mp))((void*) ptr, MPR_MANAGE_MARK);
#if BIT_DEBUG
            --depth;
#endif
        }
    }
}


//  WARNING: these do not mark component members
PUBLIC void mprHold(void *ptr)
{
    MprMem  *mp;

    if (ptr) {
        mp = GET_MEM(ptr);
        if (VALID_BLK(mp)) {
            /* Lock-free update of mp->gen */
            SET_FIELD2(mp, GET_SIZE(mp), heap->eternal, UNMARKED, 0);
        }
    }
}


PUBLIC void mprRelease(void *ptr)
{
    MprMem  *mp;

    if (ptr) {
        mp = GET_MEM(ptr);
        if (VALID_BLK(mp)) {
            assert(!IS_FREE(mp));
            /* Lock-free update of mp->gen */
            SET_FIELD2(mp, GET_SIZE(mp), heap->active, UNMARKED, 0);
        }
    }
}


/*
    If dispatcher is 0, will use MPR->nonBlock if MPR_EVENT_QUICK else MPR->dispatcher
 */
PUBLIC int mprCreateEventOutside(MprDispatcher *dispatcher, void *proc, void *data)
{
    MprEvent    *event;

    heap->pauseGC++;
    mprAtomicBarrier();
    while (heap->mustYield) {
        mprNap(0);
    }
    event = mprCreateEvent(dispatcher, "relay", 0, proc, data, MPR_EVENT_STATIC_DATA);
    heap->pauseGC--;
    if (!event) {
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


/*
    Marker thread main program
 */
static void marker(void *unused, MprThread *tp)
{
    mprTrace(5, "DEBUG: marker thread started");
    tp->stickyYield = 1;
    tp->yielded = 1;

    while (!mprIsFinished()) {
        if (!heap->mustYield) {
            mprWaitForCond(heap->markerCond, -1);
            if (mprIsFinished()) {
                break;
            }
        }
        MPR_MEASURE(7, "GC", "mark", mark());
    }
    heap->mustYield = 0;
}


#if KEEP
/*
    Sweeper thread main program. May be called from the marker thread.
 */
static void sweeper(void *unused, MprThread *tp) 
{
    mprTrace(5, "DEBUG: sweeper thread started");

    heap->sweeper = 1;
    while (!mprIsStoppingCore()) {
        MPR_MEASURE(7, "GC", "sweep", sweep());
        mprYield(MPR_YIELD_BLOCK);
    }
    heap->sweeper = 0;
}
#endif


/*
    Called by user code to signify the thread is ready for GC and all object references are saved. 
    If the GC marker is synchronizing, this call will block at the GC sync point (should be brief).
    NOTE: if called by ResetYield, we may be already marking.
 */
PUBLIC void mprYield(int flags)
{
    MprThreadService    *ts;
    MprThread           *tp;

    ts = MPR->threadService;
    if ((tp = mprGetCurrentThread()) == 0) {
        mprError("Yield called from an unknown thread");
        /* Called from a non-mpr thread */
        return;
    }
    /*
        Must not call mprLog or derviatives after setting yielded as they will allocate memory and assert.
     */
    tp->yielded = 1;
    if (flags & MPR_YIELD_STICKY) {
        tp->stickyYield = 1;
    }
    //  MOB - remove heap->marker
    while (tp->yielded && (heap->mustYield || (flags & MPR_YIELD_BLOCK)) && heap->marker) {
        if (heap->flags & MPR_MARK_THREAD) {
            mprSignalCond(ts->cond);
        }
        if (tp->stickyYield && flags & MPR_YIELD_NO_BLOCK) {
            return;
        }
        mprWaitForCond(tp->cond, -1);
        flags &= ~MPR_YIELD_BLOCK;
    }
    if (!tp->stickyYield) {
        tp->yielded = 0;
    }
    assert(!heap->marking);
}


PUBLIC void mprResetYield()
{
    MprThreadService    *ts;
    MprThread           *tp;

    ts = MPR->threadService;
    if ((tp = mprGetCurrentThread()) != 0) {
        tp->stickyYield = 0;
    }
    /*
        May have been sticky yielded and so marking could be active. If so, must yield here regardless.
        If GC being requested, then do a blocking pause here.
     */
    lock(ts->threads);
    if (heap->mustYield || heap->marking || heap->sweeping) {
        unlock(ts->threads);
        mprYield(0);
    } else {
        tp->yielded = 0;
        unlock(ts->threads);
    }
}


/*
    Pause until all threads have yielded. Called by the GC marker only.
    NOTE: this functions differently if parallel. If so, then it will abort waiting. If !parallel, it waits for all
    threads to yield.
 */
static int pauseThreads()
{
    MprThreadService    *ts;
    MprThread           *tp;
    MprTicks            mark;
    int                 i, allYielded, timeout;

#if BIT_MPR_TRACING
    uint64  hticks = mprGetHiResTicks();
#endif
    ts = MPR->threadService;
    timeout = MPR_TIMEOUT_GC_SYNC;

    mprTrace(7, "pauseThreads: wait for threads to yield, timeout %d", timeout);
    mark = mprGetTicks();
    if (mprGetDebugMode()) {
        timeout = timeout * 500;
    }
    do {
        /*
            Use the thread list lock to serialize access to heap->marking. 
            NOTE: mprResetYield has a race where its thread will have been yielded.
         */
        lock(ts->threads);
        if (!heap->pauseGC) {
            allYielded = 1;
            for (i = 0; i < ts->threads->length; i++) {
                tp = (MprThread*) mprGetItem(ts->threads, i);
                if (!tp->yielded) {
                    allYielded = 0;
                    if (mprGetElapsedTicks(mark) > 1000) {
                        mprTrace(7, "Thread %s is not yielding", tp->name);
                    }
                    break;
                }
            }
            if (allYielded) {
                heap->marking = 1;
                unlock(ts->threads);
                break;
            }
        } else {
            allYielded = 0;
        }
        unlock(ts->threads);
        mprTrace(7, "pauseThreads: waiting for threads to yield");
        mprWaitForCond(ts->cond, 20);

    } while (!allYielded && mprGetElapsedTicks(mark) < timeout);

#if BIT_MPR_TRACING
    mprTrace(7, "TIME: pauseThreads elapsed %,Ld msec, %,Ld hticks", mprGetElapsedTicks(mark), mprGetHiResTicks() - hticks);
#endif
    if (allYielded) {
        checkYielded();
    }
    return (allYielded) ? 1 : 0;
}


/*
    Resume all yielded threads. Called by the GC marker only and when destroying the app.
 */
PUBLIC void mprResumeThreads()
{
    MprThreadService    *ts;
    MprThread           *tp;
    int                 i;

    ts = MPR->threadService;
    mprTrace(7, "mprResumeThreadsAfterGC sync");

    lock(ts->threads);
    for (i = 0; i < ts->threads->length; i++) {
        tp = (MprThread*) mprGetItem(ts->threads, i);
        if (tp && tp->yielded) {
            if (!tp->stickyYield) {
                tp->yielded = 0;
            }
            mprSignalCond(tp->cond);
        }
    }
    unlock(ts->threads);
}


PUBLIC void mprVerifyMem()
{
#if BIT_MEMORY_DEBUG
    MprRegion   *region;
    MprMem      *mp;
    MprFreeMem  *freeq, *fp;
    int         i;
    
    if (!heap->verify) {
        return;
    }
    lockHeap();
    for (region = heap->regions; region; region = region->next) {
        for (mp = region->start; mp; mp = GET_NEXT(mp)) {
            CHECK(mp);
        }
    }
    for (i = 0, freeq = heap->freeq; freeq != heap->freeEnd; freeq++, i++) {
        for (fp = freeq->next; fp != freeq; fp = fp->next) {
            mp = (MprMem*) fp;
            CHECK(mp);
            assert(GET_GEN(mp) == heap->eternal);
            assert(IS_FREE(mp));
#if FUTURE
            uchar *ptr;
            int  usize;
            if (heap->verifyFree) {
                ptr = (uchar*) ((char*) mp + sizeof(MprFreeMem));
                usize = GET_SIZE(mp) - sizeof(MprFreeMem);
                if (HAS_MANAGER(mp)) {
                    usize -= sizeof(MprManager);
                }
                for (i = 0; i < usize; i++) {
                    if (ptr[i] != 0xFE) {
                        mprError("Free memory block %x has been modified at offset %d (MprBlk %x, seqno %d)\n"
                                       "Memory was last allocated by %s", GET_PTR(mp), i, mp, mp->seqno, mp->name);
                    }
                }
            }
#endif
        }
    }
    unlockHeap();
#endif
}


/*
    WARNING: Caller must be locked so that the sweeper will not free this block. 
 */
PUBLIC int mprIsDead(cvoid *ptr)
{
    MprMem      *mp;

    mp = GET_MEM(ptr);
    if (VALID_BLK(mp)) {
        return GET_GEN(mp) == heap->dead;
    }
    return 0;
}


/*
    Revive a block that is scheduled for sweeping.
    WARNING: Caller must be locked so that the sweeper will not free this block. 
 */
PUBLIC void mprRevive(cvoid *ptr)
{
    MprMem      *mp;

    mp = GET_MEM(ptr);
    SET_GEN(mp, heap->active);
    SET_MARK(mp, heap->eternal);
}


PUBLIC bool mprEnableGC(bool on)
{
    bool    old;

    old = heap->enabled;
    heap->enabled = on;
    return old;
}


static void initGen()
{
    heap->eternal = MPR_GEN_ETERNAL;
    heap->active = heap->eternal - 1;
#if PARALLEL_GC
    heap->stale = heap->active - 1;
    heap->dead = heap->stale - 1;
#else
    heap->dead = heap->active - 1;
#endif
}


static void nextGen() 
{
    int     active;

#if PARALLEL_GC
    active = (heap->active + 1) % MPR_MAX_GEN;
    heap->active = active;
    heap->stale = (active - 1 + MPR_MAX_GEN) % MPR_MAX_GEN;
    heap->dead = (active - 2 + MPR_MAX_GEN) % MPR_MAX_GEN;
    mprTrace(7, "GC: Iteration %d, active %d, stale %d, dead %d, eternal %d",
        heap->iteration, heap->active, heap->stale, heap->dead, heap->eternal);
#else
    active = heap->active;
    heap->active = heap->dead;
    heap->dead = active;
    mprTrace(7, "GC: Iteration %d, active %d, dead %d, eternal %d",
        heap->iteration, heap->active, heap->dead, heap->eternal);
#endif
    heap->iteration++;
}


PUBLIC void mprAddRoot(void *root)
{
    /*
        Need to use root lock because mprAddItem may allocate
     */
    mprSpinLock(&heap->rootLock);
    mprAddItem(heap->roots, root);
    mprSpinUnlock(&heap->rootLock);
}


PUBLIC void mprRemoveRoot(void *root)
{
    ssize   index;

    mprSpinLock(&heap->rootLock);
    index = mprRemoveItem(heap->roots, root);
    /*
        RemoveItem copies down. If the item was equal or before the current marker root, must adjust the marker rootIndex
        so we don't skip a root. (OPT but only if doing parallel GC)
     */
    if (index <= heap->rootIndex && heap->rootIndex > 0) {
        heap->rootIndex--;
    }
    mprSpinUnlock(&heap->rootLock);
}


static void *getNextRoot()
{
    void    *root;

    mprSpinLock(&heap->rootLock);
    root = mprGetNextItem(heap->roots, &heap->rootIndex);
    mprSpinUnlock(&heap->rootLock);
    return root;
}


/****************************************************** Debug *************************************************************/

#if BIT_MEMORY_STATS
static void printQueueStats() 
{
    MprFreeMem  *freeq;
    int         i;

    printf("\nFree Queue Stats\n Bucket                     Size   Count\n");
    for (i = 0, freeq = heap->freeq; freeq != heap->freeEnd; freeq++, i++) {
        if (freeq->info.stats.count) {
            printf("%7d %24d %7d\n", i, freeq->info.stats.minSize, freeq->info.stats.count);
        }
    }
}


static MprLocationStats sortLocations[MPR_TRACK_HASH];

static int sortLocation(cvoid *l1, cvoid *l2)
{
    MprLocationStats    *lp1, *lp2;

    lp1 = (MprLocationStats*) l1;
    lp2 = (MprLocationStats*) l2;
    if (lp1->count < lp2->count) {
        return -1;
    } else if (lp1->count == lp2->count) {
        return 0;
    }
    return 1;
}


static void printTracking() 
{
    MprLocationStats     *lp;
    cchar                **np;

    printf("\nAllocation Stats\n     Size Location\n");
    memcpy(sortLocations, heap->stats.locations, sizeof(sortLocations));
    qsort(sortLocations, MPR_TRACK_HASH, sizeof(MprLocationStats), sortLocation);

    for (lp = sortLocations; lp < &sortLocations[MPR_TRACK_HASH]; lp++) {
        if (lp->count) {
            for (np = &lp->names[0]; *np && np < &lp->names[MPR_TRACK_NAMES]; np++) {
                if (*np) {
                    if (np == lp->names) {
                        printf("%10d %-24s\n", (int) lp->count, *np);
                    } else {
                        printf("           %-24s\n", *np);
                    }
                }
            }
        }
    }
}


static void printGCStats()
{
    MprRegion   *region;
    MprMem      *mp;
    ssize       size, bytes[MPR_MAX_GEN + 2];
    int         regionCount, i, freeCount, allocatedCount, counts[MPR_MAX_GEN + 2], free, gen;

    for (i = 0; i < (MPR_MAX_GEN + 2); i++) {
        counts[i] = 0;
        bytes[i] = 0;
    }
    printf("\nRegion Stats\n");
    regionCount = 0;
    free = heap->eternal + 1;
    for (region = heap->regions; region; region = region->next) {
        freeCount = allocatedCount = 0;
        for (mp = region->start; mp; mp = GET_NEXT(mp)) {
            size = GET_SIZE(mp);
            gen = GET_GEN(mp);
            if (IS_FREE(mp)) {
                freeCount++;
                counts[free]++;
                bytes[free] += size;
            } else {
                counts[gen]++;
                bytes[gen] += size;
                allocatedCount++;
            }
        }
        regionCount++;
        printf("  Region %3d is %8d bytes, has %4d allocated %3d free\n", regionCount, (int) region->size, 
            allocatedCount, freeCount);
    }
    printf("Regions: %d\n", regionCount);

    printf("\nGC Stats\n");
    printf("  Eternal generation has %9d blocks, %12d bytes\n", counts[heap->eternal], (int) bytes[heap->eternal]);
#if PARALLEL_GC
    printf("  Stale generation has   %9d blocks, %12d bytes\n", counts[heap->stale], (int) bytes[heap->stale]);
#endif
    printf("  Active generation has  %9d blocks, %12d bytes\n", counts[heap->active], (int) bytes[heap->active]);
    printf("  Dead generation has    %9d blocks, %12d bytes\n", counts[heap->dead], (int) bytes[heap->dead]);
    printf("  Free generation has    %9d blocks, %12d bytes\n", counts[free], (int) bytes[free]);
}
#endif /* BIT_MEMORY_STATS */


PUBLIC void mprPrintMem(cchar *msg, int detail)
{
    MprMemStats     *ap;

    ap = mprGetMemStats();

    printf("\nMemory Report %s\n", msg);
    printf("-------------\n");
    printf("  Total memory      %14d K\n",             (int) (mprGetMem() / 1024));

    printf("  Current heap      %14d K\n",             (int) (ap->bytesAllocated / 1024));
    printf("  Free heap memory  %14d K\n",             (int) (ap->bytesFree / 1024));
    printf("  Memory limit      %14d MB (%d %%)\n",    (int) (ap->maxMemory / (1024 * 1024)),
       percent(ap->bytesAllocated / 1024, ap->maxMemory / 1024));
    printf("  Memory redline    %14d MB (%d %%)\n",    (int) (ap->redLine / (1024 * 1024)),
       percent(ap->bytesAllocated / 1024, ap->redLine / 1024));
    printf("  Allocation errors %14d\n",               ap->errors);

#if BIT_MEMORY_STATS
    printf("  Memory requests   %14d\n",               (int) ap->requests);
    printf("  O/S allocations   %14d %%\n",            percent(ap->allocs, ap->requests));
    printf("  Block unpinns     %14d %%\n",            percent(ap->unpins, ap->requests));
    printf("  Block reuse       %14d %%\n",            percent(ap->reuse, ap->requests));
    printf("  Joins             %14d %%\n",            percent(ap->joins, ap->requests));
    printf("  Splits            %14d %%\n",            percent(ap->splits, ap->requests));
    printGCStats();
    if (detail) {
        printQueueStats();
        if (heap->track) {
            printTracking();
        }
    }
#endif /* BIT_MEMORY_STATS */
}


#if BIT_MEMORY_DEBUG
static int validBlk(MprMem *mp)
{
    ssize   size;

    size = GET_SIZE(mp);
    assert(mp->magic == MPR_ALLOC_MAGIC);
    assert(size > 0);
    return (mp->magic == MPR_ALLOC_MAGIC) && (size > 0);
}


PUBLIC void mprCheckBlock(MprMem *mp)
{
    ssize   size;

    size = GET_SIZE(mp);
    if (mp->magic != MPR_ALLOC_MAGIC || size <= 0) {
        mprError("Memory corruption in memory block %x (MprBlk %x, seqno %d)\n"
            "This most likely happend earlier in the program execution", GET_PTR(mp), mp, mp->seqno);
    }
}


static void checkFreeMem(MprMem *mp)
{
#if FUTURE
    uchar   *ptr;
    int     usize, i;

    if (heap->verify) {
        ptr = (uchar*) ((char*) mp + sizeof(MprFreeMem));
        usize = GET_SIZE(mp) - sizeof(MprFreeMem);
        if (HAS_MANAGER(mp)) {
            usize -= sizeof(MprManager);
        }
        for (i = 0; i < usize; i++) {
            if (ptr[i] != 0xFE) {
                mprError("Free memory block %x has been modified at offset %d (MprBlk %x, seqno %d)\n"
                    "Memory was last allocated by %s", GET_PTR(mp), i, mp, mp->seqno, mp->name);
                break;
            }
        }
    }
#endif
}


static void breakpoint(MprMem *mp) 
{
    if (mp == stopAlloc || mp->seqno == stopSeqno) {
        mprBreakpoint();
    }
}


/*
    Called to set the memory block name when doing an allocation
 */
PUBLIC void *mprSetAllocName(void *ptr, cchar *name)
{
    MPR_GET_MEM(ptr)->name = name;

#if BIT_MEMORY_STATS
    if (heap->track) {
        MprLocationStats    *lp;
        cchar               **np;
        int                 index;
        if (name == 0) {
            name = "";
        }
        index = shash(name, strlen(name)) % MPR_TRACK_HASH;
        lp = &heap->stats.locations[index];
        for (np = lp->names; np <= &lp->names[MPR_TRACK_NAMES]; np++) {
            if (*np == 0 || *np == name || strcmp(*np, name) == 0) {
                break;
            }
        }
        if (np < &lp->names[MPR_TRACK_NAMES]) {
            *np = (char*) name;
        }
        lp->count += GET_SIZE(GET_MEM(ptr));
    }
#endif
    return ptr;
}


static void freeLocation(cchar *name, ssize size)
{
#if BIT_MEMORY_STATS
    MprLocationStats    *lp;
    int                 index, i;

    if (name == 0) {
        name = "";
    }
    index = shash(name, strlen(name)) % MPR_TRACK_HASH;
    lp = &heap->stats.locations[index];
    lp->count -= size;
    if (lp->count <= 0) {
        for (i = 0; i < MPR_TRACK_NAMES; i++) {
            lp->names[i] = 0;
        }
    }
#endif
}


PUBLIC void *mprSetName(void *ptr, cchar *name) 
{
#if BIT_MEMORY_STATS
    MprMem  *mp = GET_MEM(ptr);
    if (mp->name) {
        freeLocation(mp->name, GET_SIZE(mp));
        mprSetAllocName(ptr, name);
    }
#else
    MPR_GET_MEM(ptr)->name = name;
#endif
    return ptr;
}


PUBLIC void *mprCopyName(void *dest, void *src) 
{
    return mprSetName(dest, mprGetName(src));
}
#endif


/********************************************* Misc ***************************************************/

static void allocException(int cause, ssize size)
{
    ssize   used;

    lockHeap();
    INC(errors);
    if (heap->stats.inMemException || mprIsStopping()) {
        unlockHeap();
        return;
    }
    heap->stats.inMemException = 1;
    used = fastMemSize();
    unlockHeap();

    if (cause == MPR_MEM_FAIL) {
        heap->hasError = 1;
        mprError("%s: Cannot allocate memory block of size %,d bytes.", MPR->name, size);

    } else if (cause == MPR_MEM_TOO_BIG) {
        heap->hasError = 1;
        mprError("%s: Cannot allocate memory block of size %,d bytes.", MPR->name, size);

    } else if (cause == MPR_MEM_REDLINE) {
        mprError("%s: Memory request for %,d bytes exceeds memory red-line.", MPR->name, size);
        mprPruneCache(NULL);
        //  OPT - could trim workers too

    } else if (cause == MPR_MEM_LIMIT) {
        mprError("%s: Memory request for %,d bytes exceeds memory limit.", MPR->name, size);
    }
    mprError("%s: Memory used %,d, redline %,d, limit %,d.", MPR->name, (int) used, (int) heap->stats.redLine,
        (int) heap->stats.maxMemory);
    mprError("%s: Consider increasing memory limit.", MPR->name);
    
    if (heap->notifier) {
        (heap->notifier)(cause, heap->allocPolicy,  size, used);
    }
    if (cause & (MPR_MEM_TOO_BIG | MPR_MEM_FAIL)) {
        /*
            Allocation failed
         */
        mprError("Application exiting immediately due to memory depletion.");
        mprTerminate(MPR_EXIT_IMMEDIATE, 2);

    } else if (cause & MPR_MEM_LIMIT) {
        if (heap->allocPolicy == MPR_ALLOC_POLICY_RESTART) {
            mprError("Application restarting due to low memory condition.");
            mprTerminate(MPR_EXIT_GRACEFUL | MPR_EXIT_RESTART, 1);

        } else if (heap->allocPolicy == MPR_ALLOC_POLICY_EXIT) {
            mprError("Application exiting immediately due to memory depletion.");
            mprTerminate(MPR_EXIT_IMMEDIATE, 2);
        }
    }
    heap->stats.inMemException = 0;
}


static void getSystemInfo()
{
    memStats.numCpu = 1;

#if MACOSX
    #ifdef _SC_NPROCESSORS_ONLN
        memStats.numCpu = (uint) sysconf(_SC_NPROCESSORS_ONLN);
    #else
        memStats.numCpu = 1;
    #endif
    memStats.pageSize = (uint) sysconf(_SC_PAGESIZE);
#elif SOLARIS
{
    FILE *ptr;
    if  ((ptr = popen("psrinfo -p", "r")) != NULL) {
        fscanf(ptr, "%d", &alloc.numCpu);
        (void) pclose(ptr);
    }
    alloc.pageSize = sysconf(_SC_PAGESIZE);
}
#elif BIT_WIN_LIKE
{
    SYSTEM_INFO     info;

    GetSystemInfo(&info);
    memStats.numCpu = info.dwNumberOfProcessors;
    memStats.pageSize = info.dwPageSize;

}
#elif BIT_BSD_LIKE
    {
        int     cmd[2];
        ssize   len;

        cmd[0] = CTL_HW;
        cmd[1] = HW_NCPU;
        len = sizeof(memStats.numCpu);
        memStats.numCpu = 0;
        if (sysctl(cmd, 2, &memStats.numCpu, &len, 0, 0) < 0) {
            memStats.numCpu = 1;
        }
        memStats.pageSize = sysconf(_SC_PAGESIZE);
    }
#elif LINUX
    {
        static const char processor[] = "processor\t:";
        char    c;
        int     fd, col, match;

        fd = open("/proc/cpuinfo", O_RDONLY);
        if (fd < 0) {
            return;
        }
        match = 1;
        memStats.numCpu = 0;
        for (col = 0; read(fd, &c, 1) == 1; ) {
            if (c == '\n') {
                col = 0;
                match = 1;
            } else {
                if (match && col < (sizeof(processor) - 1)) {
                    if (c != processor[col]) {
                        match = 0;
                    }
                    col++;

                } else if (match) {
                    memStats.numCpu++;
                    match = 0;
                }
            }
        }
        if (memStats.numCpu <= 0) {
            memStats.numCpu = 1;
        }
        close(fd);
        memStats.pageSize = sysconf(_SC_PAGESIZE);
    }
#else
        memStats.pageSize = 4096;
#endif
    if (memStats.pageSize <= 0 || memStats.pageSize >= (16 * 1024)) {
        memStats.pageSize = 4096;
    }
}


#if BIT_WIN_LIKE
static int winPageModes(int flags)
{
    if (flags & MPR_MAP_EXECUTE) {
        return PAGE_EXECUTE_READWRITE;
    } else if (flags & MPR_MAP_WRITE) {
        return PAGE_READWRITE;
    }
    return PAGE_READONLY;
}
#endif


PUBLIC MprMemStats *mprGetMemStats()
{
#if LINUX
    char            buf[1024], *cp;
    size_t          len;
    int             fd;

    heap->stats.ram = MAXSSIZE;
    if ((fd = open("/proc/meminfo", O_RDONLY)) >= 0) {
        if ((len = read(fd, buf, sizeof(buf) - 1)) > 0) {
            buf[len] = '\0';
            if ((cp = strstr(buf, "MemTotal:")) != 0) {
                for (; *cp && !isdigit((uchar) *cp); cp++) {}
                heap->stats.ram = ((ssize) atoi(cp) * 1024);
            }
        }
        close(fd);
    }
#endif
#if BIT_BSD_LIKE
    size_t len;
    int         mib[2];
#if FREEBSD
    ssize ram, usermem;
    mib[1] = HW_MEMSIZE;
#else
    int64 ram, usermem;
    mib[1] = HW_PHYSMEM;
#endif
    mib[0] = CTL_HW;
    len = sizeof(ram);
    ram = 0;
    sysctl(mib, 2, &ram, &len, NULL, 0);
    heap->stats.ram = ram;

    mib[0] = CTL_HW;
    mib[1] = HW_USERMEM;
    len = sizeof(usermem);
    usermem = 0;
    sysctl(mib, 2, &usermem, &len, NULL, 0);
    heap->stats.user = usermem;
#endif
    heap->stats.rss = mprGetMem();
    return &heap->stats;
}


/*
    Return the amount of memory currently in use. This routine may open files and thus is not very quick on some 
    platforms. On FREEBDS it returns the peak resident set size using getrusage. If a suitable O/S API is not available,
    the amount of heap memory allocated by the MPR is returned.
 */
PUBLIC ssize mprGetMem()
{
    ssize size = 0;

#if LINUX
    int fd;
    char path[BIT_MAX_PATH];
    sprintf(path, "/proc/%d/status", getpid());
    if ((fd = open(path, O_RDONLY)) >= 0) {
        char buf[BIT_MAX_BUFFER], *tok;
        int nbytes = read(fd, buf, sizeof(buf) - 1);
        close(fd);
        if (nbytes > 0) {
            buf[nbytes] = '\0';
            if ((tok = strstr(buf, "VmRSS:")) != 0) {
                for (tok += 6; tok && isspace((uchar) *tok); tok++) {}
                size = stoi(tok) * 1024;
            }
        }
    }
    if (size == 0) {
        struct rusage rusage;
        getrusage(RUSAGE_SELF, &rusage);
        size = rusage.ru_maxrss * 1024;
    }
#elif MACOSX
    struct task_basic_info info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t) &info, &count) == KERN_SUCCESS) {
        size = info.resident_size;
    }
#elif BIT_BSD_LIKE
    struct rusage   rusage;
    getrusage(RUSAGE_SELF, &rusage);
    size = rusage.ru_maxrss;
#endif
    if (size == 0) {
        size = heap->stats.bytesAllocated;
    }
    return size;
}


/*
    Fast routine to teturn the approximately the amount of memory currently in use. If a fast method is not available,
    use the amount of heap memory allocated by the MPR.
    WARNING: this routine must be FAST as it is used by the MPR memory allocation mechanism when more memory is allocated
    from the O/S (i.e. not on every block allocation).
 */
static ssize fastMemSize()
{
    ssize   size = 0;

#if LINUX
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    size = rusage.ru_maxrss * 1024;
#elif MACOSX
    struct task_basic_info info;
    mach_msg_type_number_t count = TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t) &info, &count) == KERN_SUCCESS) {
        size = info.resident_size;
    }
#endif
    if (size == 0) {
        size = heap->stats.bytesAllocated;
    }
    return size;
}


#if NEED_FFSL
/* 
    Find first bit set in word 
 */
#if USE_FFSL_ASM_X86
static MPR_INLINE int ffsl(ulong x)
{
    long    r;

    asm("bsf %1,%0\n\t"
        "jnz 1f\n\t"
        "mov $-1,%0\n"
        "1:" : "=r" (r) : "rm" (x));
    return (int) r + 1;
}
#else
static MPR_INLINE int ffsl(ulong word)
{
    int     b;

    for (b = 0; word; word >>= 1, b++) {
        if (word & 0x1) {
            b++;
            break;
        }
    }
    return b;
}
#endif
#endif


#if NEED_FLSL
/* 
    Find last bit set in word 
 */
#if USE_FFSL_ASM_X86
static MPR_INLINE int flsl(ulong x)
{
    long r;

    asm("bsr %1,%0\n\t"
        "jnz 1f\n\t"
        "mov $-1,%0\n"
        "1:" : "=r" (r) : "rm" (x));
    return (int) r + 1;
}
#else /* USE_FFSL_ASM_X86 */ 

static MPR_INLINE int flsl(ulong word)
{
    int     b;

    for (b = 0; word; word >>= 1, b++) ;
    return b;
}
#endif /* !USE_FFSL_ASM_X86 */
#endif /* NEED_FFSL */


#if BIT_WIN_LIKE
PUBLIC Mpr *mprGetMpr()
{
    return MPR;
}
#endif


PUBLIC int mprGetPageSize()
{
    return memStats.pageSize;
}


PUBLIC ssize mprGetBlockSize(cvoid *ptr)
{
    MprMem      *mp;

    mp = GET_MEM(ptr);
    if (ptr == 0 || !VALID_BLK(mp)) {
        return 0;
    }
    CHECK(mp);
    return GET_USIZE(mp);
}


PUBLIC int mprGetHeapFlags()
{
    return heap->flags;
}


PUBLIC void mprSetMemNotifier(MprMemNotifier cback)
{
    heap->notifier = cback;
}


PUBLIC void mprSetMemLimits(ssize redLine, ssize maxMemory)
{
    if (redLine > 0) {
        heap->stats.redLine = redLine;
    }
    if (maxMemory > 0) {
        heap->stats.maxMemory = maxMemory;
    }
}


PUBLIC void mprSetMemPolicy(int policy)
{
    heap->allocPolicy = policy;
}


PUBLIC void mprSetMemError()
{
    heap->hasError = 1;
}


PUBLIC bool mprHasMemError()
{
    return heap->hasError;
}


PUBLIC void mprResetMemError()
{
    heap->hasError = 0;
}


PUBLIC int mprIsValid(cvoid *ptr)
{
    MprMem      *mp;

    mp = GET_MEM(ptr);
#if BIT_WIN
    if (isBadWritePtr(mp, sizeof(MprMem))) {
        return 0;
    }
    if (!VALID_BLK(GET_MEM(ptr)) {
        return 0;
    }
    if (isBadWritePtr(ptr, GET_SIZE(mp))) {
        return 0;
    }
    return 0;
#else
#if BIT_MEMORY_CHECK
    return ptr && mp->magic == MPR_ALLOC_MAGIC && GET_SIZE(mp) > 0;
#else
    return ptr && GET_SIZE(mp) > 0;
#endif
#endif
}


static void dummyManager(void *ptr, int flags) 
{
}


PUBLIC void *mprSetManager(void *ptr, MprManager manager)
{
    MprMem      *mp;

    mp = GET_MEM(ptr);
    assert(HAS_MANAGER(mp));
    if (HAS_MANAGER(mp)) {
        if (!manager) {
            manager = dummyManager;
        }
        SET_MANAGER(mp, manager);
    }
    return ptr;
}


#if BIT_MEMORY_STATS && FUTURE
static void showMem(MprMem *mp)
{
    char    *gen, *mark, buf[BIT_MAX_BUFFER];
    int     g, m;

    g = GET_GEN(mp);
    m = GET_MARK(mp);
    if (g == heap->eternal) {
        gen = "eternal";
    } else if (g == heap->active) {
        gen = "active";
    } else if (g == heap->stale) {
        gen = "stale";
    } else if (g == heap->dead) {
        gen = "dead";
    } else {
        gen = "INVALID";
    }
    if (m == heap->eternal) {
        mark = "eternal";
    } else if (m == heap->active) {
        mark = "active";
    } else if (m == heap->stale) {
        mark = "stale";
    } else if (m == heap->dead) {
        mark = "dead";
    } else {
        mark = "INVALID";
    }
    sprintf(buf, "Mem 0x%p, size %d, free %d, mgr %d, last %d, prior 0x%p, gen \"%s\", mark \"%s\"\n",
        mp, (int) GET_SIZE(mp), (int) IS_FREE(mp), (int) HAS_MANAGER(mp), (int) IS_LAST(mp), GET_PRIOR(mp), gen, mark);
#if BIT_WIN
    OutputDebugString(buf);
#else
    print(buf);
#endif
}
#endif


static void checkYielded()
{
#if BIT_DEBUG
    MprThreadService    *ts;
    MprThread           *tp;
    int                 i;

    ts = MPR->threadService;
    lock(ts->threads);
    for (i = 0; i < ts->threads->length; i++) {
        tp = (MprThread*) mprGetItem(ts->threads, i);
        assert(tp->yielded);
    }
    unlock(ts->threads);
#endif
}


#if BIT_MEMORY_STACK
static void monitorStack()
{
    MprThread   *tp;
    int         diff;

    if (MPR->threadService && (tp = mprGetCurrentThread()) != 0) {
        if (tp->stackBase == 0) {
            tp->stackBase = &tp;
        }
        diff = (int) ((char*) tp->stackBase - (char*) &diff);
        if (diff < 0) {
            tp->peakStack -= diff;
            tp->stackBase = (void*) &diff;
            diff = 0;
        }
        if (diff > tp->peakStack) {
            tp->peakStack = diff;
        }
    }
}
#endif

#if !BIT_MEMORY_DEBUG
#undef mprSetName
#undef mprCopyName
#undef mprSetAllocName

/*
    Re-instate defines for combo releases, where source will be appended below here
 */
#define mprCopyName(dest, src)
#define mprGetName(ptr) ""
#define mprSetAllocName(ptr, name) ptr
#define mprSetName(ptr, name)
#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/mpr.c"
 */
/************************************************************************/

/*
    mpr.c - Multithreaded Portable Runtime (MPR). Initialization, start/stop and control of the MPR.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



/**************************** Forward Declarations ****************************/

static void manageMpr(Mpr *mpr, int flags);
static void serviceEventsThread(void *data, MprThread *tp);

/************************************* Code ***********************************/
/*
    Create and initialize the MPR service.
 */
PUBLIC Mpr *mprCreate(int argc, char **argv, int flags)
{
    MprFileSystem   *fs;
    Mpr             *mpr;

    srand((uint) time(NULL));

    if ((mpr = mprCreateMemService((MprManager) manageMpr, flags)) == 0) {
        assert(mpr);
        return 0;
    }
    mpr->start = mprGetTime(); 
    mpr->exitStrategy = MPR_EXIT_NORMAL;
    mpr->emptyString = sclone("");
    mpr->exitTimeout = MPR_TIMEOUT_STOP;
    mpr->title = sclone(BIT_TITLE);
    mpr->version = sclone(BIT_VERSION);
    mpr->idleCallback = mprServicesAreIdle;
    mpr->mimeTypes = mprCreateMimeTypes(NULL);
    mpr->terminators = mprCreateList(0, MPR_LIST_STATIC_VALUES);

    mprCreateTimeService();
    mprCreateOsService();
    mpr->mutex = mprCreateLock();
    mpr->spin = mprCreateSpinLock();
    mpr->verifySsl = 1;

    fs = mprCreateFileSystem("/");
    mprAddFileSystem(fs);
    mprCreateLogService();
    
    if (argv) {
#if BIT_WIN_LIKE
        if (argc >= 2 && strstr(argv[1], "--cygroot") != 0) {
            /*
                Cygwin shebang is broken. It will catenate args into argv[1]
             */
            char *args, *arg0;
            int  i;
            args = argv[1];
            for (i = 2; i < argc; i++) {
                args = sjoin(args, " ", argv[i], NULL);
            }
            arg0 = argv[0];
            argc = mprMakeArgv(args, &mpr->argBuf, MPR_ARGV_ARGS_ONLY);
            argv = mpr->argBuf;
            argv[0] = arg0;
        }
#endif
        mpr->argc = argc;
        mpr->argv = (cchar**) argv;
        if (!mprIsPathAbs(mpr->argv[0])) {
            mpr->argv[0] = mprGetAppPath();
        }
        mpr->name = mprTrimPathExt(mprGetPathBase(mpr->argv[0]));
    } else {
        mpr->name = sclone(BIT_PRODUCT);
        mpr->argv = mprAllocZeroed(sizeof(void*));
        mpr->argc = 0;
    }
    mpr->signalService = mprCreateSignalService();
    mpr->threadService = mprCreateThreadService();
    mpr->moduleService = mprCreateModuleService();
    mpr->eventService = mprCreateEventService();
    mpr->cmdService = mprCreateCmdService();
    mpr->workerService = mprCreateWorkerService();
    mpr->waitService = mprCreateWaitService();
    mpr->socketService = mprCreateSocketService();

    mpr->dispatcher = mprCreateDispatcher("main", MPR_DISPATCHER_ENABLED);
    mpr->nonBlock = mprCreateDispatcher("nonblock", MPR_DISPATCHER_ENABLED);
    mpr->pathEnv = sclone(getenv("PATH"));

    if (flags & MPR_USER_EVENTS_THREAD) {
        if (!(flags & MPR_NO_WINDOW)) {
            mprInitWindow();
        }
    } else {
        mprStartEventsThread();
    }
    mprStartGCService();

    if (MPR->hasError || mprHasMemError()) {
        return 0;
    }
    return mpr;
}


static void manageMpr(Mpr *mpr, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mpr->logPath);
        mprMark(mpr->logFile);
        mprMark(mpr->mimeTypes);
        mprMark(mpr->timeTokens);
        mprMark(mpr->pathEnv);
        mprMark(mpr->name);
        mprMark(mpr->title);
        mprMark(mpr->version);
        mprMark(mpr->domainName);
        mprMark(mpr->hostName);
        mprMark(mpr->ip);
        mprMark(mpr->stdError);
        mprMark(mpr->stdInput);
        mprMark(mpr->stdOutput);
        mprMark(mpr->serverName);
        mprMark(mpr->appPath);
        mprMark(mpr->appDir);
        mprMark(mpr->cmdService);
        mprMark(mpr->eventService);
        mprMark(mpr->fileSystem);
        mprMark(mpr->moduleService);
        mprMark(mpr->osService);
        mprMark(mpr->signalService);
        mprMark(mpr->socketService);
        mprMark(mpr->threadService);
        mprMark(mpr->workerService);
        mprMark(mpr->waitService);
        mprMark(mpr->dispatcher);
        mprMark(mpr->nonBlock);
        mprMark(mpr->appwebService);
        mprMark(mpr->ediService);
        mprMark(mpr->ejsService);
        mprMark(mpr->espService);
        mprMark(mpr->httpService);
        mprMark(mpr->testService);
        mprMark(mpr->terminators);
        mprMark(mpr->mutex);
        mprMark(mpr->spin);
        mprMark(mpr->cond);
        mprMark(mpr->emptyString);
        mprMark(mpr->argBuf);
    }
}


/*
    Destroy the Mpr and all services
 */
PUBLIC void mprDestroy(int how)
{
    int         gmode;

    if (!(how & MPR_EXIT_DEFAULT)) {
        MPR->exitStrategy = how;
    }
    how = MPR->exitStrategy;
    if (how & MPR_EXIT_IMMEDIATE) {
        if (how & MPR_EXIT_RESTART) {
            mprRestart();
            /* No return */
            return;
        }
        exit(0);
    }
    mprYield(MPR_YIELD_STICKY);
    if (MPR->state < MPR_STOPPING) {
        mprTerminate(how, -1);
    }
    gmode = MPR_GC_FORCE | MPR_GC_COMPLETE;
    mprRequestGC(gmode);

    if (how & MPR_EXIT_GRACEFUL) {
        mprWaitTillIdle(MPR->exitTimeout);
    }
    MPR->state = MPR_STOPPING_CORE;
    MPR->exitStrategy &= MPR_EXIT_GRACEFUL;
    MPR->exitStrategy |= MPR_EXIT_IMMEDIATE;

    mprWakeWorkers();
    mprStopCmdService();
    mprStopModuleService();
    mprStopEventService();
    mprStopSignalService();

    /* Final GC to run all finalizers */
    mprRequestGC(gmode);

    if (how & MPR_EXIT_RESTART) {
        mprLog(2, "Restarting\n\n");
    } else {
        mprLog(2, "Exiting");
    }
    MPR->state = MPR_FINISHED;
    mprStopGCService();
    mprStopThreadService();
    mprStopOsService();
    mprDestroyMemService();

    if (how & MPR_EXIT_RESTART) {
        mprRestart();
    }
}


/*
    Start termination of the Mpr. May be called by mprDestroy or elsewhere.
 */
PUBLIC void mprTerminate(int how, int status)
{
    MprTerminator   terminator;
    int             next;

    /*
        Set the stopping flag. Services should stop accepting new requests. Current requests should be allowed to
        complete if graceful exit strategy.
     */
    if (MPR->state >= MPR_STOPPING) {
        /* Already stopping and done the code below */
        return;
    }
    MPR->state = MPR_STOPPING;

    MPR->exitStatus = status;
    if (!(how & MPR_EXIT_DEFAULT)) {
        MPR->exitStrategy = how;
    }
    how = MPR->exitStrategy;
    if (how & MPR_EXIT_IMMEDIATE) {
        mprLog(3, "Immediate exit. Terminate all requests and services.");
        exit(status);

    } else if (how & MPR_EXIT_NORMAL) {
        mprLog(3, "Normal exit.");

    } else if (how & MPR_EXIT_GRACEFUL) {
        mprLog(3, "Graceful exit. Waiting for existing requests to complete.");

    } else {
        mprTrace(7, "mprTerminate: how %d", how);
    }
    /*
        Invoke terminators, set stopping state and wake up everybody
        Must invoke terminators before setting stopping state. Otherwise, the main app event loop will return from
        mprServiceEvents and starting calling destroy before we have completed this routine.
     */
    for (ITERATE_ITEMS(MPR->terminators, terminator, next)) {
        (terminator)(how, status);
    }
    mprWakeWorkers();
    mprWakeGCService();
    mprWakeDispatchers();
    mprWakeNotifier();
}


PUBLIC int mprGetExitStatus()
{
    return MPR->exitStatus;
}


PUBLIC void mprAddTerminator(MprTerminator terminator)
{
    mprAddItem(MPR->terminators, terminator);
}


PUBLIC void mprRestart()
{
#if BIT_UNIX_LIKE
    int     i;
    for (i = 3; i < MPR_MAX_FILE; i++) {
        close(i);
    }
    execv(MPR->argv[0], (char*const*) MPR->argv);

    /*
        Last-ditch trace. Can only use stdout. Logging may be closed.
     */
    printf("Failed to exec errno %d: ", errno);
    for (i = 0; MPR->argv[i]; i++) {
        printf("%s ", MPR->argv[i]);
    }
    printf("\n");
#else
    mprError("mprRestart not supported on this platform");
#endif
}


PUBLIC int mprStart()
{
    int     rc;

    rc = mprStartOsService();
    rc += mprStartModuleService();
    rc += mprStartWorkerService();
    if (rc != 0) {
        mprError("Cannot start MPR services");
        return MPR_ERR_CANT_INITIALIZE;
    }
    MPR->state = MPR_STARTED;
    mprLog(3, "MPR services are ready");
    return 0;
}


PUBLIC int mprStartEventsThread()
{
    MprThread   *tp;

    if ((tp = mprCreateThread("events", serviceEventsThread, NULL, 0)) == 0) {
        MPR->hasError = 1;
    } else {
        MPR->cond = mprCreateCond();
        mprStartThread(tp);
        mprWaitForCond(MPR->cond, MPR_TIMEOUT_START_TASK);
    }
    return 0;
}


static void serviceEventsThread(void *data, MprThread *tp)
{
    mprLog(MPR_CONFIG, "Service thread started");
    if (!(MPR->flags & MPR_NO_WINDOW)) {
        mprInitWindow();
    }
    mprSignalCond(MPR->cond);
    mprServiceEvents(-1, 0);
}


/*
    Services should call this to determine if they should accept new services
 */
PUBLIC bool mprShouldAbortRequests()
{
    return (mprIsStopping() && !(MPR->exitStrategy & MPR_EXIT_GRACEFUL));
}


PUBLIC bool mprShouldDenyNewRequests()
{
    return mprIsStopping();
}


PUBLIC bool mprIsStopping()
{
    return MPR->state >= MPR_STOPPING;
}


PUBLIC bool mprIsStoppingCore()
{
    return MPR->state >= MPR_STOPPING_CORE;
}


PUBLIC bool mprIsFinished()
{
    return MPR->state >= MPR_FINISHED;
}


PUBLIC int mprWaitTillIdle(MprTicks timeout)
{
    MprTicks    mark, remaining, lastTrace;

    lastTrace = mark = mprGetTicks(); 
    while (!mprIsIdle() && (remaining = mprGetRemainingTicks(mark, timeout)) > 0) {
        mprSleep(1);
        mprServiceEvents(10, MPR_SERVICE_ONE_THING);
        if ((lastTrace - remaining) > MPR_TICKS_PER_SEC) {
            mprLog(1, "Waiting for requests to complete, %d secs remaining ...", remaining / MPR_TICKS_PER_SEC);
            lastTrace = remaining;
        }
    }
    return mprIsIdle();
}


/*
    Test if the Mpr services are idle. Use mprIsIdle to determine if the entire process is idle.
 */
PUBLIC bool mprServicesAreIdle()
{
    bool    idle;

    /*
        Only test top level services. Dispatchers may have timers scheduled, but that is okay.
        If not, users can install their own idleCallback.
     */
    idle = mprGetListLength(MPR->workerService->busyThreads) == 0 && mprGetListLength(MPR->cmdService->cmds) == 0;
    if (!idle) {
        mprTrace(6, "Not idle: cmds %d, busy threads %d, eventing %d",
            mprGetListLength(MPR->cmdService->cmds), mprGetListLength(MPR->workerService->busyThreads), MPR->eventing);
    }
    return idle;
}


PUBLIC bool mprIsIdle()
{
    return (MPR->idleCallback)();
}


/*
    Parse the args and return the count of args. If argv is NULL, the args are parsed read-only. If argv is set,
    then the args will be extracted, back-quotes removed and argv will be set to point to all the args.
    NOTE: this routine does not allocate.
 */
PUBLIC int mprParseArgs(char *args, char **argv, int maxArgc)
{
    char    *dest, *src, *start;
    int     quote, argc;

    /*
        Example     "showColors" red 'light blue' "yellow white" 'Cannot \"render\"'
        Becomes:    ["showColors", "red", "light blue", "yellow white", "Cannot \"render\""]
     */
    for (argc = 0, src = args; src && *src != '\0' && argc < maxArgc; argc++) {
        while (isspace((uchar) *src)) {
            src++;
        }
        if (*src == '\0')  {
            break;
        }
        start = dest = src;
        if (*src == '"' || *src == '\'') {
            quote = *src;
            src++; 
            dest++;
        } else {
            quote = 0;
        }
        if (argv) {
            argv[argc] = src;
        }
        while (*src) {
            if (*src == '\\' && src[1] && (src[1] == '\\' || src[1] == '"' || src[1] == '\'')) { 
                src++;
            } else {
                if (quote) {
                    if (*src == quote && !(src > start && src[-1] == '\\')) {
                        break;
                    }
                } else if (*src == ' ') {
                    break;
                }
            }
            if (argv) {
                *dest++ = *src;
            }
            src++;
        }
        if (*src != '\0') {
            src++;
        }
        if (argv) {
            *dest++ = '\0';
        }
    }
    return argc;
}


/*
    Make an argv array. All args are in a single memory block of which argv points to the start.
    Set MPR_ARGV_ARGS_ONLY if not passing in a program name. 
    Always returns and argv[0] reserved for the program name or empty string.  First arg starts at argv[1].
 */
PUBLIC int mprMakeArgv(cchar *command, cchar ***argvp, int flags)
{
    char    **argv, *vector, *args;
    ssize   len;
    int     argc;

    assert(command);

    /*
        Allocate one vector for argv and the actual args themselves
     */
    len = slen(command) + 1;
    argc = mprParseArgs((char*) command, NULL, INT_MAX);
    if (flags & MPR_ARGV_ARGS_ONLY) {
        argc++;
    }
    if ((vector = (char*) mprAlloc(((argc + 1) * sizeof(char*)) + len)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    args = &vector[(argc + 1) * sizeof(char*)];
    strcpy(args, command);
    argv = (char**) vector;

    if (flags & MPR_ARGV_ARGS_ONLY) {
        mprParseArgs(args, &argv[1], argc);
        argv[0] = MPR->emptyString;
    } else {
        mprParseArgs(args, argv, argc);
    }
    argv[argc] = 0;
    *argvp = (cchar**) argv;
    return argc;
}


PUBLIC MprIdleCallback mprSetIdleCallback(MprIdleCallback idleCallback)
{
    MprIdleCallback old;
    
    old = MPR->idleCallback;
    MPR->idleCallback = idleCallback;
    return old;
}


PUBLIC int mprSetAppName(cchar *name, cchar *title, cchar *version)
{
    char    *cp;

    if (name) {
        if ((MPR->name = (char*) mprGetPathBase(name)) == 0) {
            return MPR_ERR_CANT_ALLOCATE;
        }
        if ((cp = strrchr(MPR->name, '.')) != 0) {
            *cp = '\0';
        }
    }
    if (title) {
        if ((MPR->title = sclone(title)) == 0) {
            return MPR_ERR_CANT_ALLOCATE;
        }
    }
    if (version) {
        if ((MPR->version = sclone(version)) == 0) {
            return MPR_ERR_CANT_ALLOCATE;
        }
    }
    return 0;
}


PUBLIC cchar *mprGetAppName()
{
    return MPR->name;
}


PUBLIC cchar *mprGetAppTitle()
{
    return MPR->title;
}


/*
    Full host name with domain. E.g. "server.domain.com"
 */
PUBLIC void mprSetHostName(cchar *s)
{
    MPR->hostName = sclone(s);
}


/*
    Return the fully qualified host name
 */
PUBLIC cchar *mprGetHostName()
{
    return MPR->hostName;
}


/*
    Server name portion (no domain name)
 */
PUBLIC void mprSetServerName(cchar *s)
{
    MPR->serverName = sclone(s);
}


PUBLIC cchar *mprGetServerName()
{
    return MPR->serverName;
}


PUBLIC void mprSetDomainName(cchar *s)
{
    MPR->domainName = sclone(s);
}


PUBLIC cchar *mprGetDomainName()
{
    return MPR->domainName;
}


/*
    Set the IP address
 */
PUBLIC void mprSetIpAddr(cchar *s)
{
    MPR->ip = sclone(s);
}


/*
    Return the IP address
 */
PUBLIC cchar *mprGetIpAddr()
{
    return MPR->ip;
}


PUBLIC cchar *mprGetAppVersion()
{
    return MPR->version;
}


PUBLIC bool mprGetDebugMode()
{
    return MPR->debugMode;
}


PUBLIC void mprSetDebugMode(bool on)
{
    MPR->debugMode = on;
}


PUBLIC MprDispatcher *mprGetDispatcher()
{
    return MPR->dispatcher;
}


PUBLIC MprDispatcher *mprGetNonBlockDispatcher()
{
    return MPR->nonBlock;
}


PUBLIC cchar *mprCopyright()
{
    return  "Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.\n"
            "Copyright (c) Michael O'Brien, 1993-2013. All Rights Reserved.";
}


PUBLIC int mprGetEndian()
{
    char    *probe;
    int     test;

    test = 1;
    probe = (char*) &test;
    return (*probe == 1) ? BIT_LITTLE_ENDIAN : BIT_BIG_ENDIAN;
}


PUBLIC char *mprEmptyString()
{
    return MPR->emptyString;
}


PUBLIC void mprSetExitStrategy(int strategy)
{
    MPR->exitStrategy = strategy;
}


PUBLIC void mprSetEnv(cchar *key, cchar *value)
{
#if !WINCE
#if BIT_UNIX_LIKE
    setenv(key, value, 1);
#else
    char *cmd = sjoin(key, "=", value, NULL);
    putenv(cmd);
#endif
#endif
    if (scaselessmatch(key, "PATH")) {
        MPR->pathEnv = sclone(value);
    }
}


PUBLIC void mprSetExitTimeout(MprTicks timeout)
{
    MPR->exitTimeout = timeout;
}


PUBLIC void mprNop(void *ptr) {
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/async.c"
 */
/************************************************************************/

/**
    async.c - Wait for I/O on Windows.

    This module provides io management for sockets on Windows like systems. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if MPR_EVENT_ASYNC
/***************************** Forward Declarations ***************************/

static LRESULT msgProc(HWND hwnd, UINT msg, UINT wp, LPARAM lp);

/************************************ Code ************************************/

PUBLIC int mprCreateNotifierService(MprWaitService *ws)
{   
    ws->socketMessage = MPR_SOCKET_MESSAGE;
    return 0;
}


PUBLIC int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask)
{
    int     winMask;

    assert(ws->hwnd);

    lock(ws);
    winMask = 0;
    if (wp->desiredMask != mask) {
        if (mask & MPR_READABLE) {
            winMask |= FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_READ;
        }
        if (mask & MPR_WRITABLE) {
            winMask |= FD_WRITE;
        }
        wp->desiredMask = mask;
        WSAAsyncSelect(wp->fd, ws->hwnd, ws->socketMessage, winMask);
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    unlock(ws);
    return 0;
}


/*
    Wait for I/O on a single descriptor. Return the number of I/O events found. Mask is the events of interest.
    Timeout is in milliseconds.
 */
PUBLIC int mprWaitForSingleIO(int fd, int desiredMask, MprTicks timeout)
{
    HANDLE      h;
    int         winMask;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
    winMask = 0;
    if (desiredMask & MPR_READABLE) {
        winMask |= FD_CLOSE | FD_READ;
    }
    if (desiredMask & MPR_WRITABLE) {
        winMask |= FD_WRITE;
    }
    h = CreateEvent(NULL, FALSE, FALSE, UT("mprWaitForSingleIO"));
    WSAEventSelect(fd, h, winMask);
    if (WaitForSingleObject(h, (DWORD) timeout) == WAIT_OBJECT_0) {
        CloseHandle(h);
        return desiredMask;
    }
    CloseHandle(h);
    return 0;
}


/*
    Wait for I/O on all registered descriptors. Timeout is in milliseconds. Return the number of events serviced.
    Should only be called by the thread that calls mprServiceEvents
 */
PUBLIC void mprWaitForIO(MprWaitService *ws, MprTicks timeout)
{
    MSG     msg;

    assert(ws->hwnd);

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
#if BIT_DEBUG
    if (mprGetDebugMode() && timeout > 30000) {
        timeout = 30000;
    }
#endif
    if (ws->needRecall) {
        mprDoWaitRecall(ws);
        return;
    }
    SetTimer(ws->hwnd, 0, (UINT) timeout, NULL);

    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    if (GetMessage(&msg, NULL, 0, 0) == 0) {
        mprResetYield();
        mprTerminate(MPR_EXIT_DEFAULT, -1);
    } else {
        mprResetYield();
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    ws->wakeRequested = 0;
}


PUBLIC void mprServiceWinIO(MprWaitService *ws, int sockFd, int winMask)
{
    MprWaitHandler      *wp;
    int                 index;

    lock(ws);

    for (index = 0; (wp = (MprWaitHandler*) mprGetNextItem(ws->handlers, &index)) != 0; ) {
        if (wp->fd == sockFd) {
            break;
        }
    }
    if (wp == 0) {
        /* If the server forcibly closed the socket, we may still get a read event. Just ignore it.  */
        unlock(ws);
        return;
    }
    /*
        Mask values: READ==1, WRITE=2, ACCEPT=8, CONNECT=10, CLOSE=20
     */
    wp->presentMask = 0;
    if (winMask & (FD_READ | FD_ACCEPT | FD_CLOSE)) {
        wp->presentMask |= MPR_READABLE;
    }
    if (winMask & (FD_WRITE | FD_CONNECT)) {
        wp->presentMask |= MPR_WRITABLE;
    }
    wp->presentMask &= wp->desiredMask;
    if (wp->presentMask) {
        if (wp->presentMask) {
            mprNotifyOn(ws, wp, 0);
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    Wake the wait service. WARNING: This routine must not require locking. MprEvents in scheduleDispatcher depends on this.
 */
PUBLIC void mprWakeNotifier()
{
    MprWaitService  *ws;
   
    ws = MPR->waitService;
    if (!ws->wakeRequested && ws->hwnd) {
        ws->wakeRequested = 1;
        PostMessage(ws->hwnd, WM_NULL, 0, 0L);
    }
}


/*
    Create a default window if the application has not already created one.
 */ 
PUBLIC int mprInitWindow()
{
    MprWaitService  *ws;
    WNDCLASS        wc;
    HWND            hwnd;
	wchar			*name, *title;
    int             rc;

    ws = MPR->waitService;
    if (ws->hwnd) {
        return 0;
    }
	name = (wchar*) wide(mprGetAppName());
	title = (wchar*) wide(mprGetAppTitle());
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW+1);
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = 0;
    wc.hIcon            = NULL;
    wc.lpfnWndProc      = (WNDPROC) msgProc;
    wc.lpszMenuName     = wc.lpszClassName = name;

    rc = RegisterClass(&wc);
    if (rc == 0) {
        mprError("Cannot register windows class");
        return MPR_ERR_CANT_INITIALIZE;
    }
    hwnd = CreateWindow(name, title, WS_OVERLAPPED, CW_USEDEFAULT, 0, 0, 0, NULL, NULL, 0, NULL);
    if (!hwnd) {
        mprError("Cannot create window");
        return -1;
    }
    ws->hwnd = hwnd;
    ws->socketMessage = MPR_SOCKET_MESSAGE;
    return 0;
}


/*
    Windows message processing loop for wakeup and socket messages
 */
static LRESULT msgProc(HWND hwnd, UINT msg, UINT wp, LPARAM lp)
{
    MprWaitService      *ws;
    int                 sock, winMask;

    ws = MPR->waitService;

    if (msg == WM_DESTROY || msg == WM_QUIT) {
        mprTerminate(MPR_EXIT_DEFAULT, -1);

    } else if (msg && msg == ws->socketMessage) {
        sock = wp;
        winMask = LOWORD(lp);
        mprServiceWinIO(MPR->waitService, sock, winMask);

    } else if (ws->msgCallback) {
        return ws->msgCallback(hwnd, msg, wp, lp);

    } else {
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


PUBLIC void mprSetWinMsgCallback(MprMsgCallback callback)
{
    MprWaitService  *ws;

    ws = MPR->waitService;
    ws->msgCallback = callback;
}


#endif /* MPR_EVENT_ASYNC */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/atomic.c"
 */
/************************************************************************/

/**
    atomic.c - Atomic operations

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*********************************** Includes *********************************/



/************************************ Code ************************************/

PUBLIC void mprAtomicBarrier()
{
    #ifdef VX_MEM_BARRIER_RW
        VX_MEM_BARRIER_RW();
    #elif MACOSX
        OSMemoryBarrier();
    #elif BIT_WIN_LIKE
        MemoryBarrier();
    #elif BIT_HAS_SYNC
        __sync_synchronize();
    #elif __GNUC__ && (BIT_CPU_ARCH == BIT_CPU_X86 || BIT_CPU_ARCH == BIT_CPU_X64)
        asm volatile ("mfence" : : : "memory");
    #elif __GNUC__ && (BIT_CPU_ARCH == BIT_CPU_PPC)
        asm volatile ("sync" : : : "memory");
    #else
        getpid();
    #endif

#if FUTURE && KEEP
    asm volatile ("lock; add %eax,0");
#endif
}


/*
    Atomic Compare and swap a pointer with a full memory barrier
 */
PUBLIC int mprAtomicCas(void * volatile *addr, void *expected, cvoid *value)
{
    #if MACOSX
        return OSAtomicCompareAndSwapPtrBarrier(expected, (void*) value, (void*) addr);
    #elif BIT_WIN_LIKE
        {
            void *prev;
            prev = InterlockedCompareExchangePointer(addr, (void*) value, expected);
            return expected == prev;
        }
    #elif BIT_HAS_SYNC_CAS
        return __sync_bool_compare_and_swap(addr, expected, value);
    #elif VXWORKS && _VX_ATOMIC_INIT && !BIT_64
        /* vxCas operates with integer values */
        return vxCas((atomic_t*) addr, (atomicVal_t) expected, (atomicVal_t) value);
    #elif BIT_CPU_ARCH == BIT_CPU_X86
        {
            void *prev;
            asm volatile ("lock; cmpxchgl %2, %1"
                : "=a" (prev), "=m" (*addr)
                : "r" (value), "m" (*addr), "0" (expected));
            return expected == prev;
        }
    #elif BIT_CPU_ARCH == BIT_CPU_X64
        {
            void *prev;
            asm volatile ("lock; cmpxchgq %q2, %1"
                : "=a" (prev), "=m" (*addr)
                : "r" (value), "m" (*addr),
                  "0" (expected));
            return expected == prev;
        }
    #else
        mprGlobalLock();
        if (*addr == expected) {
            *addr = (void*) value;
            mprGlobalUnlock();
            return 1;
        }
        mprGlobalUnlock();
        return 0;
    #endif
}


/*
    Atomic add of a signed value. Used for add, subtract, inc, dec
 */
PUBLIC void mprAtomicAdd(volatile int *ptr, int value)
{
    #if MACOSX
        OSAtomicAdd32(value, ptr);
    #elif BIT_WIN_LIKE
        InterlockedExchangeAdd(ptr, value);
    #elif VXWORKS && _VX_ATOMIC_INIT
        vxAtomicAdd(ptr, value);
    #elif (BIT_CPU_ARCH == BIT_CPU_X86 || BIT_CPU_ARCH == BIT_CPU_X64) && FUTURE
        asm volatile ("lock; xaddl %0,%1"
            : "=r" (value), "=m" (*ptr)
            : "0" (value), "m" (*ptr)
            : "memory", "cc");
    #else
        mprGlobalLock();
        *ptr += value;
        mprGlobalUnlock();
    #endif
}


/*
    On some platforms, this operation is only atomic with respect to other calls to mprAtomicAdd64
 */
PUBLIC void mprAtomicAdd64(volatile int64 *ptr, int value)
{
#if MACOSX
    OSAtomicAdd64(value, ptr);
#elif BIT_WIN_LIKE && BIT_64
    InterlockedExchangeAdd64(ptr, value);
#elif BIT_UNIX_LIKE && FUTURE
    asm volatile ("lock; xaddl %0,%1"
        : "=r" (value), "=m" (*ptr)
        : "0" (value), "m" (*ptr)
        : "memory", "cc");
#else
    mprGlobalLock();
    *ptr += value;
    mprGlobalUnlock();
#endif
}


PUBLIC void *mprAtomicExchange(void * volatile *addr, cvoid *value)
{
#if MACOSX
    void *old = *(void**) addr;
    OSAtomicCompareAndSwapPtrBarrier(old, (void*) value, addr);
    return old;
#elif BIT_WIN_LIKE
    return (void*) InterlockedExchange((volatile LONG*) addr, (LONG) value);
#elif BIT_HAS_SYNC
    return __sync_lock_test_and_set(addr, (void*) value);
#else
    {
        void    *old;
        mprGlobalLock();
        old = * (void**) addr;
        *addr = (void*) value;
        mprGlobalUnlock();
        return old;
    }
#endif
}


/*
    Atomic list insertion. Inserts "item" at the "head" of the list. The "link" field is the next field in item.
 */
PUBLIC void mprAtomicListInsert(void * volatile *head, volatile void **link, void *item)
{
    do {
        *link = *head;
    } while (mprAtomicCas(head, (void*) *link, item));
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/buf.c"
 */
/************************************************************************/

/**
    buf.c - Dynamic buffer module

    This module is not thread-safe for performance. Callers must do their own locking.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Forwards **********************************/

static void manageBuf(MprBuf *buf, int flags);

/*********************************** Code *************************************/
/*
    Create a new buffer. "maxsize" is the limit to which the buffer can ever grow. -1 means no limit. "initialSize" is 
    used to define the amount to increase the size of the buffer each time if it becomes full. (Note: mprGrowBuf() will 
    exponentially increase this number for performance.)
 */
PUBLIC MprBuf *mprCreateBuf(ssize initialSize, ssize maxSize)
{
    MprBuf      *bp;
    
    if (initialSize <= 0) {
        initialSize = BIT_MAX_BUFFER;
    }
    if ((bp = mprAllocObj(MprBuf, manageBuf)) == 0) {
        return 0;
    }
    bp->growBy = BIT_MAX_BUFFER;
    mprSetBufSize(bp, initialSize, maxSize);
    return bp;
}


static void manageBuf(MprBuf *bp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(bp->data);
        mprMark(bp->refillArg);
    } 
}


PUBLIC MprBuf *mprCloneBuf(MprBuf *orig)
{
    MprBuf      *bp;
    ssize       len;

    if ((bp = mprCreateBuf(orig->growBy, orig->maxsize)) == 0) {
        return 0;
    }
    bp->refillProc = orig->refillProc;
    bp->refillArg = orig->refillArg;
    if ((len = mprGetBufLength(orig)) > 0) {
        memcpy(bp->data, orig->data, len);
        bp->end = &bp->data[len];
    }
    return bp;
}


PUBLIC char *mprGet(MprBuf *bp)
{
    return (char*) bp->start;
}


/*
    Set the current buffer size and maximum size limit.
 */
PUBLIC int mprSetBufSize(MprBuf *bp, ssize initialSize, ssize maxSize)
{
    assert(bp);

    if (initialSize <= 0) {
        if (maxSize > 0) {
            bp->maxsize = maxSize;
        }
        return 0;
    }
    if (maxSize > 0 && initialSize > maxSize) {
        initialSize = maxSize;
    }
    assert(initialSize > 0);

    if (bp->data) {
        /*
            Buffer already exists
         */
        if (bp->buflen < initialSize) {
            if (mprGrowBuf(bp, initialSize - bp->buflen) < 0) {
                return MPR_ERR_MEMORY;
            }
        }
        bp->maxsize = maxSize;
        return 0;
    }
    if ((bp->data = mprAlloc(initialSize)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    bp->growBy = initialSize;
    bp->maxsize = maxSize;
    bp->buflen = initialSize;
    bp->endbuf = &bp->data[bp->buflen];
    bp->start = bp->data;
    bp->end = bp->data;
    *bp->start = '\0';
    return 0;
}


PUBLIC void mprSetBufMax(MprBuf *bp, ssize max)
{
    bp->maxsize = max;
}


/*
    This appends a silent null. It does not count as one of the actual bytes in the buffer
 */
PUBLIC void mprAddNullToBuf(MprBuf *bp)
{
    ssize      space;

    space = bp->endbuf - bp->end;
    if (space < sizeof(char)) {
        if (mprGrowBuf(bp, 1) < 0) {
            return;
        }
    }
    assert(bp->end < bp->endbuf);
    if (bp->end < bp->endbuf) {
        *((char*) bp->end) = (char) '\0';
    }
}


PUBLIC void mprAdjustBufEnd(MprBuf *bp, ssize size)
{
    assert(bp->buflen == (bp->endbuf - bp->data));
    assert(size <= bp->buflen);
    assert((bp->end + size) >= bp->data);
    assert((bp->end + size) <= bp->endbuf);

    bp->end += size;
    if (bp->end > bp->endbuf) {
        assert(bp->end <= bp->endbuf);
        bp->end = bp->endbuf;
    }
    if (bp->end < bp->data) {
        bp->end = bp->data;
    }
}


/*
    Adjust the start pointer after a user copy. Note: size can be negative.
 */
PUBLIC void mprAdjustBufStart(MprBuf *bp, ssize size)
{
    assert(bp->buflen == (bp->endbuf - bp->data));
    assert(size <= bp->buflen);
    assert((bp->start + size) >= bp->data);
    assert((bp->start + size) <= bp->end);

    bp->start += size;
    if (bp->start > bp->end) {
        bp->start = bp->end;
    }
    if (bp->start <= bp->data) {
        bp->start = bp->data;
    }
}


PUBLIC void mprFlushBuf(MprBuf *bp)
{
    bp->start = bp->data;
    bp->end = bp->data;
}


PUBLIC int mprGetCharFromBuf(MprBuf *bp)
{
    if (bp->start == bp->end) {
        return -1;
    }
    return (uchar) *bp->start++;
}


PUBLIC ssize mprGetBlockFromBuf(MprBuf *bp, char *buf, ssize size)
{
    ssize     thisLen, bytesRead;

    assert(buf);
    assert(size >= 0);
    assert(bp->buflen == (bp->endbuf - bp->data));

    /*
        Get the max bytes in a straight copy
     */
    bytesRead = 0;
    while (size > 0) {
        thisLen = mprGetBufLength(bp);
        thisLen = min(thisLen, size);
        if (thisLen <= 0) {
            break;
        }

        memcpy(buf, bp->start, thisLen);
        buf += thisLen;
        bp->start += thisLen;
        size -= thisLen;
        bytesRead += thisLen;
    }
    return bytesRead;
}


#ifndef mprGetBufLength
PUBLIC ssize mprGetBufLength(MprBuf *bp)
{
    return (bp->end - bp->start);
}
#endif


#ifndef mprGetBufSize
PUBLIC ssize mprGetBufSize(MprBuf *bp)
{
    return bp->buflen;
}
#endif


#ifndef mprGetBufSpace
PUBLIC ssize mprGetBufSpace(MprBuf *bp)
{
    return (bp->endbuf - bp->end);
}
#endif


#ifndef mprGetBuf
PUBLIC char *mprGetBuf(MprBuf *bp)
{
    return (char*) bp->data;
}
#endif


#ifndef mprGetBufStart
PUBLIC char *mprGetBufStart(MprBuf *bp)
{
    return (char*) bp->start;
}
#endif


#ifndef mprGetBufEnd
PUBLIC char *mprGetBufEnd(MprBuf *bp)
{
    return (char*) bp->end;
}
#endif


PUBLIC int mprInsertCharToBuf(MprBuf *bp, int c)
{
    if (bp->start == bp->data) {
        return MPR_ERR_BAD_STATE;
    }
    *--bp->start = c;
    return 0;
}


PUBLIC int mprLookAtNextCharInBuf(MprBuf *bp)
{
    if (bp->start == bp->end) {
        return -1;
    }
    return *bp->start;
}


PUBLIC int mprLookAtLastCharInBuf(MprBuf *bp)
{
    if (bp->start == bp->end) {
        return -1;
    }
    return bp->end[-1];
}


PUBLIC int mprPutCharToBuf(MprBuf *bp, int c)
{
    char       *cp;
    ssize      space;

    assert(bp->buflen == (bp->endbuf - bp->data));

    space = bp->buflen - mprGetBufLength(bp);
    if (space < sizeof(char)) {
        if (mprGrowBuf(bp, 1) < 0) {
            return -1;
        }
    }
    cp = (char*) bp->end;
    *cp++ = (char) c;
    bp->end = (char*) cp;

    if (bp->end < bp->endbuf) {
        *((char*) bp->end) = (char) '\0';
    }
    return 1;
}


/*
    Return the number of bytes written to the buffer. If no more bytes will fit, may return less than size.
    Never returns < 0.
 */
PUBLIC ssize mprPutBlockToBuf(MprBuf *bp, cchar *str, ssize size)
{
    ssize      thisLen, bytes, space;

    assert(str);
    assert(size >= 0);
    assert(size < MAXINT);

    bytes = 0;
    while (size > 0) {
        space = mprGetBufSpace(bp);
        thisLen = min(space, size);
        if (thisLen <= 0) {
            if (mprGrowBuf(bp, size) < 0) {
                break;
            }
            space = mprGetBufSpace(bp);
            thisLen = min(space, size);
        }
        memcpy(bp->end, str, thisLen);
        str += thisLen;
        bp->end += thisLen;
        size -= thisLen;
        bytes += thisLen;
    }
    if (bp->end < bp->endbuf) {
        *((char*) bp->end) = (char) '\0';
    }
    return bytes;
}


PUBLIC ssize mprPutStringToBuf(MprBuf *bp, cchar *str)
{
    if (str) {
        return mprPutBlockToBuf(bp, str, slen(str));
    }
    return 0;
}


PUBLIC ssize mprPutSubStringToBuf(MprBuf *bp, cchar *str, ssize count)
{
    ssize     len;

    if (str) {
        len = slen(str);
        len = min(len, count);
        if (len > 0) {
            return mprPutBlockToBuf(bp, str, len);
        }
    }
    return 0;
}


PUBLIC ssize mprPutPadToBuf(MprBuf *bp, int c, ssize count)
{
    assert(count < MAXINT);

    while (count-- > 0) {
        if (mprPutCharToBuf(bp, c) < 0) {
            return -1;
        }
    }
    return count;
}


PUBLIC ssize mprPutToBuf(MprBuf *bp, cchar *fmt, ...)
{
    va_list     ap;
    char        *buf;

    if (fmt == 0) {
        return 0;
    }
    va_start(ap, fmt);
    buf = sfmtv(fmt, ap);
    va_end(ap);
    return mprPutStringToBuf(bp, buf);
}


/*
    Grow the buffer. Return 0 if the buffer grows. Increase by the growBy size specified when creating the buffer. 
 */
PUBLIC int mprGrowBuf(MprBuf *bp, ssize need)
{
    char    *newbuf;
    ssize   growBy;

    if (bp->maxsize > 0 && bp->buflen >= bp->maxsize) {
        return MPR_ERR_TOO_MANY;
    }
    if (bp->start > bp->end) {
        mprCompactBuf(bp);
    }
    if (need > 0) {
        growBy = max(bp->growBy, need);
    } else {
        growBy = bp->growBy;
    }
    if ((newbuf = mprAlloc(bp->buflen + growBy)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    if (bp->data) {
        memcpy(newbuf, bp->data, bp->buflen);
    }
    bp->buflen += growBy;
    bp->end = newbuf + (bp->end - bp->data);
    bp->start = newbuf + (bp->start - bp->data);
    bp->data = newbuf;
    bp->endbuf = &bp->data[bp->buflen];

    /*
        Increase growBy to reduce overhead
     */
    if (bp->maxsize > 0) {
        if ((bp->buflen + (bp->growBy * 2)) > bp->maxsize) {
            bp->growBy = min(bp->maxsize - bp->buflen, bp->growBy * 2);
        }
    } else {
        if ((bp->buflen + (bp->growBy * 2)) > bp->maxsize) {
            bp->growBy = min(bp->buflen, bp->growBy * 2);
        }
    }
    return 0;
}


/*
    Add a number to the buffer (always null terminated).
 */
PUBLIC ssize mprPutIntToBuf(MprBuf *bp, int64 i)
{
    ssize       rc;

    rc = mprPutStringToBuf(bp, itos(i));
    if (bp->end < bp->endbuf) {
        *((char*) bp->end) = (char) '\0';
    }
    return rc;
}


PUBLIC void mprCompactBuf(MprBuf *bp)
{
    if (mprGetBufLength(bp) == 0) {
        mprFlushBuf(bp);
        return;
    }
    if (bp->start > bp->data) {
        memmove(bp->data, bp->start, (bp->end - bp->start));
        bp->end -= (bp->start - bp->data);
        bp->start = bp->data;
    }
}


PUBLIC MprBufProc mprGetBufRefillProc(MprBuf *bp) 
{
    return bp->refillProc;
}


PUBLIC void mprSetBufRefillProc(MprBuf *bp, MprBufProc fn, void *arg)
{ 
    bp->refillProc = fn; 
    bp->refillArg = arg; 
}


PUBLIC int mprRefillBuf(MprBuf *bp) 
{ 
    return (bp->refillProc) ? (bp->refillProc)(bp, bp->refillArg) : 0; 
}


PUBLIC void mprResetBufIfEmpty(MprBuf *bp)
{
    if (mprGetBufLength(bp) == 0) {
        mprFlushBuf(bp);
    }
}


PUBLIC char *mprBufToString(MprBuf *bp)
{
    mprAddNullToBuf(bp);
    return sclone(mprGetBufStart(bp));
}


#if BIT_CHAR_LEN > 1 && UNUSED
PUBLIC void mprAddNullToWideBuf(MprBuf *bp)
{
    ssize      space;

    space = bp->endbuf - bp->end;
    if (space < sizeof(wchar)) {
        if (mprGrowBuf(bp, sizeof(wchar)) < 0) {
            return;
        }
    }
    assert(bp->end < bp->endbuf);
    if (bp->end < bp->endbuf) {
        *((wchar*) bp->end) = (char) '\0';
    }
}


PUBLIC int mprPutCharToWideBuf(MprBuf *bp, int c)
{
    wchar *cp;
    ssize   space;

    assert(bp->buflen == (bp->endbuf - bp->data));

    space = bp->buflen - mprGetBufLength(bp);
    if (space < (sizeof(wchar) * 2)) {
        if (mprGrowBuf(bp, sizeof(wchar) * 2) < 0) {
            return -1;
        }
    }
    cp = (wchar*) bp->end;
    *cp++ = (wchar) c;
    bp->end = (char*) cp;

    if (bp->end < bp->endbuf) {
        *((wchar*) bp->end) = (char) '\0';
    }
    return 1;
}


PUBLIC ssize mprPutFmtToWideBuf(MprBuf *bp, cchar *fmt, ...)
{
    va_list     ap;
    wchar     *wbuf;
    char        *buf;
    ssize       len, space;
    ssize       rc;

    if (fmt == 0) {
        return 0;
    }
    va_start(ap, fmt);
    space = mprGetBufSpace(bp);
    space += (bp->maxsize - bp->buflen);
    buf = sfmtv(fmt, ap);
    wbuf = amtow(buf, &len);
    rc = mprPutBlockToBuf(bp, (char*) wbuf, len * sizeof(wchar));
    va_end(ap);
    return rc;
}


PUBLIC ssize mprPutStringToWideBuf(MprBuf *bp, cchar *str)
{
    wchar     *wstr;
    ssize       len;

    if (str) {
        wstr = amtow(str, &len);
        return mprPutBlockToBuf(bp, (char*) wstr, len);
    }
    return 0;
}

#endif /* BIT_CHAR_LEN > 1 */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/cache.c"
 */
/************************************************************************/

/**
    cache.c - In-process caching

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/************************************ Locals **********************************/

static MprCache *shared;                /* Singleton shared cache */

typedef struct CacheItem
{
    char        *key;                   /* Original key */
    char        *data;                  /* Cache data */
    MprTicks    lifespan;               /* Lifespan after each access to key (msec) */
    MprTicks    lastAccessed;           /* Last accessed time */
    MprTicks    expires;                /* Fixed expiry date. If zero, key is imortal. */
    MprTime     lastModified;           /* Last update time. This is an MprTime and records world-time. */
    int64       version;
} CacheItem;

#define CACHE_TIMER_PERIOD      (60 * MPR_TICKS_PER_SEC)
#define CACHE_HASH_SIZE         257
#define CACHE_LIFESPAN          (86400 * MPR_TICKS_PER_SEC)

/*********************************** Forwards *********************************/

static void manageCache(MprCache *cache, int flags);
static void manageCacheItem(CacheItem *item, int flags);
static void pruneCache(MprCache *cache, MprEvent *event);
static void removeItem(MprCache *cache, CacheItem *item);

/************************************* Code ***********************************/

PUBLIC MprCache *mprCreateCache(int options)
{
    MprCache    *cache;
    int         wantShared;

    if ((cache = mprAllocObj(MprCache, manageCache)) == 0) {
        return 0;
    }
    wantShared = (options & MPR_CACHE_SHARED);
    if (wantShared && shared) {
        cache->shared = shared;
    } else {
        cache->mutex = mprCreateLock();
        cache->store = mprCreateHash(CACHE_HASH_SIZE, 0);
        cache->maxMem = MAXSSIZE;
        cache->maxKeys = MAXSSIZE;
        cache->resolution = CACHE_TIMER_PERIOD;
        cache->lifespan = CACHE_LIFESPAN;
        if (wantShared) {
            shared = cache;
        }
    }
    return cache;
}


PUBLIC void *mprDestroyCache(MprCache *cache)
{
    assert(cache);

    if (cache->timer && cache != shared) {
        mprRemoveEvent(cache->timer);
        cache->timer = 0;
    }
    if (cache == shared) {
        shared = 0;
    }
    return 0;
}


PUBLIC int mprExpireCache(MprCache *cache, cchar *key, MprTicks expires)
{
    CacheItem   *item;

    assert(cache);
    assert(key && *key);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    lock(cache);
    if ((item = mprLookupKey(cache->store, key)) == 0) {
        unlock(cache);
        return MPR_ERR_CANT_FIND;
    }
    if (expires == 0) {
        removeItem(cache, item);
    } else {
        item->expires = expires;
    }
    unlock(cache);
    return 0;
}


PUBLIC int64 mprIncCache(MprCache *cache, cchar *key, int64 amount)
{
    CacheItem   *item;
    int64       value;

    assert(cache);
    assert(key && *key);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    value = amount;

    lock(cache);
    if ((item = mprLookupKey(cache->store, key)) == 0) {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            return 0;
        }
    } else {
        value += stoi(item->data);
    }
    if (item->data) {
        cache->usedMem -= slen(item->data);
    }
    item->data = itos(value);
    cache->usedMem += slen(item->data);
    item->version++;
    item->lastAccessed = mprGetTicks();
    item->expires = item->lastAccessed + item->lifespan;
    unlock(cache);
    return value;
}


PUBLIC char *mprReadCache(MprCache *cache, cchar *key, MprTime *modified, int64 *version)
{
    CacheItem   *item;
    char        *result;

    assert(cache);
    assert(key && *key);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    lock(cache);
    if ((item = mprLookupKey(cache->store, key)) == 0) {
        unlock(cache);
        return 0;
    }
    if (item->expires && item->expires <= mprGetTicks()) {
        unlock(cache);
        return 0;
    }
    if (version) {
        *version = item->version;
    }
    if (modified) {
        *modified = item->lastModified;
    }
    item->lastAccessed = mprGetTicks();
    item->expires = item->lastAccessed + item->lifespan;
    result = item->data;
    unlock(cache);
    return result;
}


PUBLIC bool mprRemoveCache(MprCache *cache, cchar *key)
{
    CacheItem   *item;
    bool        result;

    assert(cache);
    assert(key && *key);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    lock(cache);
    if (key) {
        if ((item = mprLookupKey(cache->store, key)) != 0) {
            cache->usedMem -= (slen(key) + slen(item->data));
            mprRemoveKey(cache->store, key);
            result = 1;
        } else {
            result = 0;
        }

    } else {
        /* Remove all keys */
        result = mprGetHashLength(cache->store) ? 1 : 0;
        cache->store = mprCreateHash(CACHE_HASH_SIZE, 0);
        cache->usedMem = 0;
    }
    unlock(cache);
    return result;
}


PUBLIC void mprSetCacheLimits(MprCache *cache, int64 keys, MprTicks lifespan, int64 memory, int resolution)
{
    assert(cache);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    if (keys > 0) {
        cache->maxKeys = (ssize) keys;
        if (cache->maxKeys <= 0) {
            cache->maxKeys = MAXSSIZE;
        }
    }
    if (lifespan > 0) {
        cache->lifespan = lifespan;
    }
    if (memory > 0) {
        cache->maxMem = (ssize) memory;
        if (cache->maxMem <= 0) {
            cache->maxMem = MAXSSIZE;
        }
    }
    if (resolution > 0) {
        cache->resolution = resolution;
        if (cache->resolution <= 0) {
            cache->resolution = CACHE_TIMER_PERIOD;
        }
    }
}


PUBLIC ssize mprWriteCache(MprCache *cache, cchar *key, cchar *value, MprTime modified, MprTicks lifespan, int64 version, int options)
{
    CacheItem   *item;
    MprKey      *kp;
    ssize       len, oldLen;
    int         exists, add, set, prepend, append, throw;

    assert(cache);
    assert(key && *key);
    assert(value);

    if (cache->shared) {
        cache = cache->shared;
        assert(cache == shared);
    }
    exists = add = prepend = append = throw = 0;
    add = options & MPR_CACHE_ADD;
    append = options & MPR_CACHE_APPEND;
    prepend = options & MPR_CACHE_PREPEND;
    set = options & MPR_CACHE_SET;
    if ((add + append + prepend) == 0) {
        set = 1;
    }
    lock(cache);
    if ((kp = mprLookupKeyEntry(cache->store, key)) != 0) {
        exists++;
        item = (CacheItem*) kp->data;
        if (version) {
            if (item->version != version) {
                unlock(cache);
                return MPR_ERR_BAD_STATE;
            }
        }
    } else {
        if ((item = mprAllocObj(CacheItem, manageCacheItem)) == 0) {
            unlock(cache);
            return 0;
        }
        mprAddKey(cache->store, key, item);
        item->key = sclone(key);
        set = 1;
    }
    oldLen = (item->data) ? (slen(item->key) + slen(item->data)) : 0;
    if (set) {
        item->data = sclone(value);
    } else if (add) {
        if (exists) {
            return 0;
        }
        item->data = sclone(value);
    } else if (append) {
        item->data = sjoin(item->data, value, NULL);
    } else if (prepend) {
        item->data = sjoin(value, item->data, NULL);
    }
    if (lifespan >= 0) {
        item->lifespan = lifespan;
    }
    item->lastModified = modified ? modified : mprGetTime();
    item->lastAccessed = mprGetTicks();
    item->expires = item->lastAccessed + item->lifespan;
    item->version++;
    len = slen(item->key) + slen(item->data);
    cache->usedMem += (len - oldLen);

    if (cache->timer == 0) {
        mprTrace(5, "Start Cache pruner with resolution %d", cache->resolution);
        /* 
            Use the MPR dispatcher incase this VM is destroyed 
         */
        cache->timer = mprCreateTimerEvent(MPR->dispatcher, "localCacheTimer", cache->resolution, pruneCache, cache, 
            MPR_EVENT_STATIC_DATA); 
    }
    unlock(cache);
    return len;
}


static void removeItem(MprCache *cache, CacheItem *item)
{
    assert(cache);
    assert(item);

    lock(cache);
    mprRemoveKey(cache->store, item->key);
    cache->usedMem -= (slen(item->key) + slen(item->data));
    unlock(cache);
}


static void pruneCache(MprCache *cache, MprEvent *event)
{
    MprTicks        when, factor;
    MprKey          *kp;
    CacheItem       *item;
    ssize           excessKeys;

    if (!cache) {
        cache = shared;
        if (!cache) {
            return;
        }
    }
    if (event) {
        when = mprGetTicks();
    } else {
        /* Expire all items by setting event to NULL */
        when = MAXINT64;
    }
    if (mprTryLock(cache->mutex)) {
        /*
            Check for expired items
         */
        for (kp = 0; (kp = mprGetNextKey(cache->store, kp)) != 0; ) {
            item = (CacheItem*) kp->data;
            mprTrace(6, "Cache: \"%s\" lifespan %d, expires in %d secs", item->key, 
                    item->lifespan / 1000, (item->expires - when) / 1000);
            if (item->expires && item->expires <= when) {
                mprTrace(5, "Cache prune expired key %s", kp->key);
                removeItem(cache, item);
            }
        }
        assert(cache->usedMem >= 0);

        /*
            If too many keys or too much memory used, prune keys that expire soonest.
         */
        if (cache->maxKeys < MAXSSIZE || cache->maxMem < MAXSSIZE) {
            /*
                Look for those expiring in the next 5 minutes, then 20 mins, then 80 ...
             */
            excessKeys = mprGetHashLength(cache->store) - cache->maxKeys;
            factor = 5 * 60 * MPR_TICKS_PER_SEC; 
            when += factor;
            while (excessKeys > 0 || cache->usedMem > cache->maxMem) {
                for (kp = 0; (kp = mprGetNextKey(cache->store, kp)) != 0; ) {
                    item = (CacheItem*) kp->data;
                    if (item->expires && item->expires <= when) {
                        mprTrace(5, "Cache too big execess keys %Ld, mem %Ld, prune key %s", 
                                excessKeys, (cache->maxMem - cache->usedMem), kp->key);
                        removeItem(cache, item);
                    }
                }
                factor *= 4;
                when += factor;
            }
        }
        assert(cache->usedMem >= 0);

        if (mprGetHashLength(cache->store) == 0) {
            if (event) {
                mprRemoveEvent(event);
                cache->timer = 0;
            }
        }
        unlock(cache);
    }
}


PUBLIC void mprPruneCache(MprCache *cache)
{
    pruneCache(cache, NULL);
}


static void manageCache(MprCache *cache, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cache->store);
        mprMark(cache->mutex);
        mprMark(cache->timer);
        mprMark(cache->shared);

    } else if (flags & MPR_MANAGE_FREE) {
        if (cache == shared) {
            shared = 0;
        }
    }
}


static void manageCacheItem(CacheItem *item, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(item->key);
        mprMark(item->data);
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/cmd.c"
 */
/************************************************************************/

/* 
    cmd.c - Run external commands

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/******************************* Forward Declarations *************************/

static int blendEnv(MprCmd *cmd, cchar **env, int flags);
static void closeFiles(MprCmd *cmd);
static void defaultCmdCallback(MprCmd *cmd, int channel, void *data);
static int makeChannel(MprCmd *cmd, int index);
static int makeCmdIO(MprCmd *cmd);
static void manageCmdService(MprCmdService *cmd, int flags);
static void manageCmd(MprCmd *cmd, int flags);
static void reapCmd(MprCmd *cmd, MprSignal *sp);
static void resetCmd(MprCmd *cmd);
static int sanitizeArgs(MprCmd *cmd, int argc, cchar **argv, cchar **env, int flags);
static int startProcess(MprCmd *cmd);
static void stdinCallback(MprCmd *cmd, MprEvent *event);
static void stdoutCallback(MprCmd *cmd, MprEvent *event);
static void stderrCallback(MprCmd *cmd, MprEvent *event);
static void vxCmdManager(MprCmd *cmd);
#if BIT_WIN_LIKE
static cchar *makeWinEnvBlock(MprCmd *cmd);
#endif

#if VXWORKS
typedef int (*MprCmdTaskFn)(int argc, char **argv, char **envp);
static void cmdTaskEntry(char *program, MprCmdTaskFn entry, int cmdArg);
#endif

/*
    Cygwin process creation is not thread-safe (1.7)
 */
#if CYGWIN
    #define slock(cmd) mprLock(MPR->cmdService->mutex)
    #define sunlock(cmd) mprUnlock(MPR->cmdService->mutex)
#else
    #define slock(cmd) 
    #define sunlock(cmd) 
#endif

/************************************* Code ***********************************/

PUBLIC MprCmdService *mprCreateCmdService()
{
    MprCmdService   *cs;

    if ((cs = (MprCmdService*) mprAllocObj(MprCmd, manageCmdService)) == 0) {
        return 0;
    }
    cs->cmds = mprCreateList(0, MPR_LIST_STATIC_VALUES);
    cs->mutex = mprCreateLock();
    return cs;
}


PUBLIC void mprStopCmdService()
{
    mprClearList(MPR->cmdService->cmds);
}


static void manageCmdService(MprCmdService *cs, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cs->cmds);
        mprMark(cs->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        mprStopCmdService();
        cs->mutex = 0;
    }
}


PUBLIC MprCmd *mprCreateCmd(MprDispatcher *dispatcher)
{
    MprCmd          *cmd;
    MprCmdFile      *files;
    int             i;
    
    if ((cmd = mprAllocObj(MprCmd, manageCmd)) == 0) {
        return 0;
    }
#if KEEP
    cmd->timeoutPeriod = MPR_TIMEOUT_CMD;
    cmd->timestamp = mprGetTicks();
#endif
    cmd->forkCallback = (MprForkCallback) closeFiles;
    cmd->dispatcher = dispatcher ? dispatcher : MPR->dispatcher;
    cmd->status = -1;

#if VXWORKS
    cmd->startCond = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
    cmd->exitCond = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
#endif
    files = cmd->files;
    for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
        files[i].clientFd = -1;
        files[i].fd = -1;
    }
    cmd->mutex = mprCreateLock();
    mprAddItem(MPR->cmdService->cmds, cmd);
    return cmd;
}


static void manageCmd(MprCmd *cmd, int flags)
{
    int             i;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(cmd->program);
        mprMark(cmd->makeArgv);
        mprMark(cmd->defaultEnv);
        mprMark(cmd->env);
        mprMark(cmd->dir);
        for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
            mprMark(cmd->files[i].name);
        }
        for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
            mprMark(cmd->handlers[i]);
        }
        mprMark(cmd->dispatcher);
        mprMark(cmd->callbackData);
        mprMark(cmd->signal);
        mprMark(cmd->forkData);
        mprMark(cmd->stdoutBuf);
        mprMark(cmd->stderrBuf);
        mprMark(cmd->userData);
        mprMark(cmd->mutex);
        mprMark(cmd->searchPath);
#if BIT_WIN_LIKE
        mprMark(cmd->command);
        mprMark(cmd->arg0);
#endif

    } else if (flags & MPR_MANAGE_FREE) {
        mprDestroyCmd(cmd);
        vxCmdManager(cmd);
        mprRemoveItem(MPR->cmdService->cmds, cmd);
    }
}


static void vxCmdManager(MprCmd *cmd)
{
#if VXWORKS
    MprCmdFile      *files;
    int             i;

    if (cmd->startCond) {
        semDelete(cmd->startCond);
    }
    if (cmd->exitCond) {
        semDelete(cmd->exitCond);
    }
    files = cmd->files;
    for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
        if (files[i].name) {
            DEV_HDR *dev;
#if _WRS_VXWORKS_MAJOR >= 6
            cchar   *tail;
#else
            char    *tail;
#endif
            if ((dev = iosDevFind(files[i].name, &tail)) != NULL) {
                iosDevDelete(dev);
            }
        }
    }
#endif
}


PUBLIC void mprDestroyCmd(MprCmd *cmd)
{
    assert(cmd);
    slock(cmd);
    resetCmd(cmd);
    if (cmd->signal) {
        mprRemoveSignalHandler(cmd->signal);
        cmd->signal = 0;
    }
    sunlock(cmd);
}


static void resetCmd(MprCmd *cmd)
{
    MprCmdFile      *files;
    int             i;

    assert(cmd);
    files = cmd->files;
    for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
        if (cmd->handlers[i]) {
            mprRemoveWaitHandler(cmd->handlers[i]);
            cmd->handlers[i] = 0;
        }
        if (files[i].clientFd >= 0) {
            close(files[i].clientFd);
            files[i].clientFd = -1;
        }
        if (files[i].fd >= 0) {
            close(files[i].fd);
            files[i].fd = -1;
        }
    }
    cmd->eofCount = 0;
    cmd->complete = 0;
    cmd->status = -1;

    if (cmd->pid && !(cmd->flags & MPR_CMD_DETACH)) {
        mprStopCmd(cmd, -1);
        reapCmd(cmd, 0);
        cmd->pid = 0;
    }
}


PUBLIC void mprDisconnectCmd(MprCmd *cmd)
{
    int     i;

    assert(cmd);

    for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
        if (cmd->handlers[i]) {
            mprRemoveWaitHandler(cmd->handlers[i]);
            cmd->handlers[i] = 0;
        }
    }
}


/*
    Close a command channel. Must be able to be called redundantly.
 */
PUBLIC void mprCloseCmdFd(MprCmd *cmd, int channel)
{
    assert(cmd);
    assert(0 <= channel && channel <= MPR_CMD_MAX_PIPE);

    if (cmd->handlers[channel]) {
        mprRemoveWaitHandler(cmd->handlers[channel]);
        cmd->handlers[channel] = 0;
    }
    if (cmd->files[channel].fd != -1) {
        close(cmd->files[channel].fd);
        cmd->files[channel].fd = -1;
#if BIT_WIN_LIKE
        cmd->files[channel].handle = 0;
#endif
        if (channel != MPR_CMD_STDIN) {
            cmd->eofCount++;
            if (cmd->eofCount >= cmd->requiredEof) {
#if VXWORKS
                reapCmd(cmd, 0);
#endif
                if (cmd->pid == 0) {
                    cmd->complete = 1;
                }
            }
        }
        mprTrace(6, "Close channel %d eof %d/%d, pid %d", channel, cmd->eofCount, cmd->requiredEof, cmd->pid);
    }
}


PUBLIC void mprFinalizeCmd(MprCmd *cmd)
{
    mprTrace(6, "mprFinalizeCmd");
    assert(cmd);
    mprCloseCmdFd(cmd, MPR_CMD_STDIN);
}


PUBLIC int mprIsCmdComplete(MprCmd *cmd)
{
    return cmd->complete;
}


/*
    Run a simple blocking command. See arg usage below in mprRunCmdV.
 */
PUBLIC int mprRunCmd(MprCmd *cmd, cchar *command, cchar **envp, char **out, char **err, MprTicks timeout, int flags)
{
    cchar   **argv;
    int     argc;

    assert(cmd);
    if ((argc = mprMakeArgv(command, &argv, 0)) < 0 || argv == 0) {
        return 0;
    }
    cmd->makeArgv = argv;
    return mprRunCmdV(cmd, argc, argv, envp, out, err, timeout, flags);
}


/*
    Env is an array of "KEY=VALUE" strings. Null terminated
    The user must preserve the environment. This module does not clone the environment and uses the supplied reference.
 */
PUBLIC void mprSetCmdDefaultEnv(MprCmd *cmd, cchar **env)
{
    /* WARNING: defaultEnv is not cloned, but is marked */
    cmd->defaultEnv = env;
}


PUBLIC void mprSetCmdSearchPath(MprCmd *cmd, cchar *search)
{
    cmd->searchPath = sclone(search);
}


/*
    This routine runs a command and waits for its completion. Stdoutput and Stderr are returned in *out and *err 
    respectively. The command returns the exit status of the command.
    Valid flags are:
        MPR_CMD_NEW_SESSION     Create a new session on Unix
        MPR_CMD_SHOW            Show the commands window on Windows
        MPR_CMD_IN              Connect to stdin
 */
PUBLIC int mprRunCmdV(MprCmd *cmd, int argc, cchar **argv, cchar **envp, char **out, char **err, MprTicks timeout, int flags)
{
    int     rc, status;

    assert(cmd);
    if (err) {
        *err = 0;
        flags |= MPR_CMD_ERR;
    } else {
        flags &= ~MPR_CMD_ERR;
    }
    if (out) {
        *out = 0;
        flags |= MPR_CMD_OUT;
    } else {
        flags &= ~MPR_CMD_OUT;
    }
    if (flags & MPR_CMD_OUT) {
        cmd->stdoutBuf = mprCreateBuf(BIT_MAX_BUFFER, -1);
    }
    if (flags & MPR_CMD_ERR) {
        cmd->stderrBuf = mprCreateBuf(BIT_MAX_BUFFER, -1);
    }
    mprSetCmdCallback(cmd, defaultCmdCallback, NULL);
    rc = mprStartCmd(cmd, argc, argv, envp, flags);

    /*
        Close the pipe connected to the client's stdin
     */
    if (cmd->files[MPR_CMD_STDIN].fd >= 0) {
        mprFinalizeCmd(cmd);
    }
    if (rc < 0) {
        if (err) {
            if (rc == MPR_ERR_CANT_ACCESS) {
                *err = sfmt("Cannot access command %s", cmd->program);
            } else if (MPR_ERR_CANT_OPEN) {
                *err = sfmt("Cannot open standard I/O for command %s", cmd->program);
            } else if (rc == MPR_ERR_CANT_CREATE) {
                *err = sfmt("Cannot create process for %s", cmd->program);
            }
        }
        return rc;
    }
    if (cmd->flags & MPR_CMD_DETACH) {
        return 0;
    }
    if (mprWaitForCmd(cmd, timeout) < 0) {
        return MPR_ERR_NOT_READY;
    }
    if ((status = mprGetCmdExitStatus(cmd)) < 0) {
        return MPR_ERR;
    }
    if (err && flags & MPR_CMD_ERR) {
        *err = mprGetBufStart(cmd->stderrBuf);
    }
    if (out && flags & MPR_CMD_OUT) {
        *out = mprGetBufStart(cmd->stdoutBuf);
    }
    return status;
}


static void addCmdHandlers(MprCmd *cmd)
{
    int     stdinFd, stdoutFd, stderrFd;
  
    stdinFd = cmd->files[MPR_CMD_STDIN].fd; 
    stdoutFd = cmd->files[MPR_CMD_STDOUT].fd; 
    stderrFd = cmd->files[MPR_CMD_STDERR].fd; 

    if (stdinFd >= 0 && cmd->handlers[MPR_CMD_STDIN] == 0) {
        cmd->handlers[MPR_CMD_STDIN] = mprCreateWaitHandler(stdinFd, MPR_WRITABLE, cmd->dispatcher, stdinCallback, cmd, 0);
    }
    if (stdoutFd >= 0 && cmd->handlers[MPR_CMD_STDOUT] == 0) {
        cmd->handlers[MPR_CMD_STDOUT] = mprCreateWaitHandler(stdoutFd, MPR_READABLE, cmd->dispatcher, stdoutCallback, cmd,0);
    }
    if (stderrFd >= 0 && cmd->handlers[MPR_CMD_STDERR] == 0) {
        cmd->handlers[MPR_CMD_STDERR] = mprCreateWaitHandler(stderrFd, MPR_READABLE, cmd->dispatcher, stderrCallback, cmd,0);
    }
}


/*
    Start the command to run (stdIn and stdOut are named from the client's perspective). This is the lower-level way to 
    run a command. The caller needs to do code like mprRunCmd() themselves to wait for completion and to send/receive data.
    The routine does not wait. Callers must call mprWaitForCmd to wait for the command to complete.
 */
PUBLIC int mprStartCmd(MprCmd *cmd, int argc, cchar **argv, cchar **envp, int flags)
{
    MprPath     info;
    cchar       *program, *search, *pair;
    int         rc, next, i;

    assert(cmd);
    assert(argv);

    if (argc <= 0 || argv == NULL || argv[0] == NULL) {
        return MPR_ERR_BAD_ARGS;
    }
    resetCmd(cmd);
    program = argv[0];
    cmd->program = sclone(program);
    cmd->flags = flags;

    if (sanitizeArgs(cmd, argc, argv, envp, flags) < 0) {
        return MPR_ERR_MEMORY;
    }
    if (envp == 0) {
        envp = cmd->defaultEnv;
    }
    if (blendEnv(cmd, envp, flags) < 0) {
        return MPR_ERR_MEMORY;
    }
    search = cmd->searchPath ? cmd->searchPath : MPR->pathEnv;
    if ((program = mprSearchPath(program, MPR_SEARCH_EXE, search, NULL)) == 0) {
        mprLog(1, "cmd: can't access %s, errno %d", cmd->program, mprGetOsError());
        return MPR_ERR_CANT_ACCESS;
    }
    cmd->program = cmd->argv[0] = program;

    if (mprGetPathInfo(program, &info) == 0 && info.isDir) {
        mprLog(1, "cmd: program \"%s\", is a directory", program);
        return MPR_ERR_CANT_ACCESS;
    }
    mprLog(4, "mprStartCmd %s", cmd->program);
    for (i = 0; i < cmd->argc; i++) {
        mprLog(6, "    arg[%d]: %s", i, cmd->argv[i]);
    }
    for (ITERATE_ITEMS(cmd->env, pair, next)) {
        mprLog(6, "    env[%d]: %s", next, pair);
    }
    slock(cmd);
    if (makeCmdIO(cmd) < 0) {
        sunlock(cmd);
        return MPR_ERR_CANT_OPEN;
    }
    /*
        Determine how many end-of-files will be seen when the child dies
     */
    cmd->requiredEof = 0;
    if (cmd->flags & MPR_CMD_OUT) {
        cmd->requiredEof++;
    }
    if (cmd->flags & MPR_CMD_ERR) {
        cmd->requiredEof++;
    }
    addCmdHandlers(cmd);
    rc = startProcess(cmd);
    cmd->pid2 = cmd->pid;
    sunlock(cmd);
    return rc;
}


static int makeCmdIO(MprCmd *cmd)
{
    int     rc;

    rc = 0;
    if (cmd->flags & MPR_CMD_IN) {
        rc += makeChannel(cmd, MPR_CMD_STDIN);
    }
    if (cmd->flags & MPR_CMD_OUT) {
        rc += makeChannel(cmd, MPR_CMD_STDOUT);
    }
    if (cmd->flags & MPR_CMD_ERR) {
        rc += makeChannel(cmd, MPR_CMD_STDERR);
    }
    return rc;
}


/*
    Stop the command
 */
PUBLIC int mprStopCmd(MprCmd *cmd, int signal)
{
    mprTrace(7, "cmd: stop");

    if (signal < 0) {
        signal = SIGTERM;
    }
    cmd->stopped = 1;
    if (cmd->pid) {
#if BIT_WIN_LIKE
        return TerminateProcess(cmd->process, 2) == 0;
#elif VXWORKS
        return taskDelete(cmd->pid);
#else
        return kill(cmd->pid, signal);
#endif
    }
    return 0;
}


/*
    Do non-blocking I/O - except on windows - will block
 */
PUBLIC ssize mprReadCmd(MprCmd *cmd, int channel, char *buf, ssize bufsize)
{
#if BIT_WIN_LIKE
    int     rc, count;
    /*
        Need to detect EOF in windows. Pipe always in blocking mode, but reads block even with no one on the other end.
     */
    assert(cmd->files[channel].handle);
    rc = PeekNamedPipe(cmd->files[channel].handle, NULL, 0, NULL, &count, NULL);
    if (rc > 0 && count > 0) {
        return read(cmd->files[channel].fd, buf, (uint) bufsize);
    } 
    if (cmd->process == 0 || WaitForSingleObject(cmd->process, 0) == WAIT_OBJECT_0) {
        /* Process has exited - EOF */
        return 0;
    }
    /* This maps to EAGAIN */
    SetLastError(WSAEWOULDBLOCK);
    return -1;

#elif VXWORKS
    /*
        Only needed when using non-blocking I/O
     */
    int     rc;

    rc = read(cmd->files[channel].fd, buf, bufsize);

    /*
        VxWorks can't signal EOF on non-blocking pipes. Need a pattern indicator.
     */
    if (rc == MPR_CMD_VXWORKS_EOF_LEN && strncmp(buf, MPR_CMD_VXWORKS_EOF, MPR_CMD_VXWORKS_EOF_LEN) == 0) {
        /* EOF */
        return 0;

    } else if (rc == 0) {
        rc = -1;
        errno = EAGAIN;
    }
    return rc;

#else
    assert(cmd->files[channel].fd >= 0);
    return read(cmd->files[channel].fd, buf, bufsize);
#endif
}


/*
    Do non-blocking I/O - except on windows - will block
 */
PUBLIC ssize mprWriteCmd(MprCmd *cmd, int channel, char *buf, ssize bufsize)
{
#if BIT_WIN_LIKE
    /*
        No waiting. Use this just to check if the process has exited and thus EOF on the pipe.
     */
    if (cmd->pid == 0 || WaitForSingleObject(cmd->process, 0) == WAIT_OBJECT_0) {
        return -1;
    }
#endif
    return write(cmd->files[channel].fd, buf, (wsize) bufsize);
}


PUBLIC bool mprAreCmdEventsEnabled(MprCmd *cmd, int channel)
{
    MprWaitHandler  *wp;

    int mask = (channel == MPR_CMD_STDIN) ? MPR_WRITABLE : MPR_READABLE;
    return ((wp = cmd->handlers[channel]) != 0) && (wp->desiredMask & mask);
}


PUBLIC void mprEnableCmdOutputEvents(MprCmd *cmd, bool on)
{
    int     mask;

    mask = on ? MPR_READABLE : 0;
    if (cmd->handlers[MPR_CMD_STDOUT]) {
        mprWaitOn(cmd->handlers[MPR_CMD_STDOUT], mask);
    }
    if (cmd->handlers[MPR_CMD_STDERR]) {
        mprWaitOn(cmd->handlers[MPR_CMD_STDERR], mask);
    }
}


PUBLIC void mprEnableCmdEvents(MprCmd *cmd, int channel)
{
    int mask = (channel == MPR_CMD_STDIN) ? MPR_WRITABLE : MPR_READABLE;
    if (cmd->handlers[channel]) {
        mprWaitOn(cmd->handlers[channel], mask);
    }
}


PUBLIC void mprDisableCmdEvents(MprCmd *cmd, int channel)
{
    if (cmd->handlers[channel]) {
        mprWaitOn(cmd->handlers[channel], 0);
    }
}


#if BIT_WIN_LIKE && !WINCE
/*
    Windows only routine to wait for I/O on the channels to the gateway and the child process.
    This will queue events on the dispatcher queue when I/O occurs or the process dies.
    NOTE: NamedPipes can't use WaitForMultipleEvents, so we dedicate a thread to polling.
    WARNING: this should not be called from a dispatcher other than cmd->dispatcher. 
 */
PUBLIC void mprPollWinCmd(MprCmd *cmd, MprTicks timeout)
{
    MprTicks        mark, delay;
    MprWaitHandler  *wp;
    int             i, rc, nbytes;

    mark = mprGetTicks();
    if (cmd->stopped) {
        timeout = 0;
    }
    slock(cmd);
    for (i = MPR_CMD_STDOUT; i < MPR_CMD_MAX_PIPE; i++) {
        if (cmd->files[i].handle) {
            wp = cmd->handlers[i];
            if (wp && wp->desiredMask & MPR_READABLE) {
                rc = PeekNamedPipe(cmd->files[i].handle, NULL, 0, NULL, &nbytes, NULL);
                if (rc && nbytes > 0 || cmd->process == 0) {
                    mprQueueIOEvent(wp);
                }
            }
        }
    }
    if (cmd->files[MPR_CMD_STDIN].handle) {
        wp = cmd->handlers[MPR_CMD_STDIN];
        if (wp && wp->desiredMask & MPR_WRITABLE) {
            mprQueueIOEvent(wp);
        }
    }
    if (cmd->process) {
        delay = (cmd->eofCount == cmd->requiredEof && cmd->files[MPR_CMD_STDIN].handle == 0) ? timeout : 0;
        do {
            mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
            if (WaitForSingleObject(cmd->process, (DWORD) delay) == WAIT_OBJECT_0) {
                mprResetYield();
                reapCmd(cmd, 0);
                break;
            } else {
                mprResetYield();
            }
            delay = mprGetRemainingTicks(mark, timeout);
        } while (cmd->eofCount == cmd->requiredEof);
    }
    sunlock(cmd);
}
#endif


/*
    Wait for a command to complete. Return 0 if the command completed, otherwise it will return MPR_ERR_TIMEOUT. 
 */
PUBLIC int mprWaitForCmd(MprCmd *cmd, MprTicks timeout)
{
    MprTicks    expires, remaining, delay;

    assert(cmd);

    if (timeout < 0) {
        timeout = MAXINT;
    }
    if (mprGetDebugMode()) {
        timeout = MAXINT;
    }
    if (cmd->stopped) {
        timeout = 0;
    }
    expires = mprGetTicks() + timeout;
    remaining = timeout;

    /* Add root to allow callers to use mprRunCmd without first managing the cmd */
    mprAddRoot(cmd);

    while (!cmd->complete && remaining > 0) {
        if (mprShouldAbortRequests()) {
            break;
        }
#if BIT_WIN_LIKE && !WINCE
        mprPollWinCmd(cmd, remaining);
        delay = 10;
#else
        delay = (cmd->eofCount >= cmd->requiredEof) ? 10 : remaining;
#endif
        mprWaitForEvent(cmd->dispatcher, delay);
        remaining = (expires - mprGetTicks());
    }
    mprRemoveRoot(cmd);
    if (cmd->pid) {
        return MPR_ERR_TIMEOUT;
    }
    mprTrace(6, "cmd: waitForChild: status %d", cmd->status);
    return 0;
}


/*
    Gather the child's exit status. 
    WARNING: this may be called with a false-positive, ie. SIGCHLD will get invoked for all process deaths and not just
    when this cmd has completed.
 */
static void reapCmd(MprCmd *cmd, MprSignal *sp)
{
    int     status, rc;

    mprTrace(6, "reapCmd CHECK pid %d, eof %d, required %d", cmd->pid, cmd->eofCount, cmd->requiredEof);
    
    status = 0;
    if (cmd->pid == 0) {
        return;
    }
#if BIT_UNIX_LIKE
    if ((rc = waitpid(cmd->pid, &status, WNOHANG | __WALL)) < 0) {
        mprTrace(6, "waitpid failed for pid %d, errno %d", cmd->pid, errno);

    } else if (rc == cmd->pid) {
        mprTrace(6, "waitpid pid %d, thread %s", cmd->pid, mprGetCurrentThreadName());
        if (!WIFSTOPPED(status)) {
            if (WIFEXITED(status)) {
                cmd->status = WEXITSTATUS(status);
                mprTrace(6, "waitpid exited pid %d, status %d", cmd->pid, cmd->status);
            } else if (WIFSIGNALED(status)) {
                cmd->status = WTERMSIG(status);
            } else {
                mprTrace(7, "waitpid FUNNY pid %d, errno %d", cmd->pid, errno);
            }
            cmd->pid = 0;
            assert(cmd->signal);
            mprRemoveSignalHandler(cmd->signal);
            cmd->signal = 0;
        } else {
            mprTrace(7, "waitpid ELSE pid %d, errno %d", cmd->pid, errno);
        }
    } else {
        mprTrace(6, "waitpid still running pid %d, thread %s", cmd->pid, mprGetCurrentThreadName());
    }
#endif
#if VXWORKS
    /*
        The command exit status (cmd->status) is set in cmdTaskEntry
     */
    if (!cmd->stopped) {
        if (semTake(cmd->exitCond, MPR_TIMEOUT_STOP_TASK) != OK) {
            mprError("cmd: child %s did not exit, errno %d", cmd->program);
            return;
        }
    }
    semDelete(cmd->exitCond);
    cmd->exitCond = 0;
    cmd->pid = 0;
    rc = 0;
#endif
#if BIT_WIN_LIKE
    if (GetExitCodeProcess(cmd->process, (ulong*) &status) == 0) {
        mprTrace(3, "cmd: GetExitProcess error");
        return;
    }
    if (status != STILL_ACTIVE) {
        cmd->status = status;
        rc = CloseHandle(cmd->process);
        assert(rc != 0);
        rc = CloseHandle(cmd->thread);
        assert(rc != 0);
        cmd->process = 0;
        cmd->thread = 0;
        cmd->pid = 0;
    }
#endif
    if (cmd->pid == 0) {
        if (cmd->eofCount >= cmd->requiredEof) {
            cmd->complete = 1;
        }
        mprTrace(6, "Cmd reaped: status %d, pid %d, eof %d / %d", cmd->status, cmd->pid, cmd->eofCount, cmd->requiredEof);
        if (cmd->callback) {
            (cmd->callback)(cmd, -1, cmd->callbackData);
            /* WARNING - this above call may invoke httpPump and complete the request. HttpConn.tx may be null */
        }
    }
}


/*
    Default callback routine for the mprRunCmd routines. Uses may supply their own callback instead of this routine. 
    The callback is run whenever there is I/O to read/write to the CGI gateway.
 */
static void defaultCmdCallback(MprCmd *cmd, int channel, void *data)
{
    MprBuf      *buf;
    ssize       len, space;
    int         errCode;

    /*
        Note: stdin, stdout and stderr are named from the client's perspective
     */
    buf = 0;
    switch (channel) {
    case MPR_CMD_STDIN:
        return;
    case MPR_CMD_STDOUT:
        buf = cmd->stdoutBuf;
        break;
    case MPR_CMD_STDERR:
        buf = cmd->stderrBuf;
        break;
    default:
        /* Child death notification */
        return;
    }
    /*
        Read and aggregate the result into a single string
     */
    space = mprGetBufSpace(buf);
    if (space < (BIT_MAX_BUFFER / 4)) {
        if (mprGrowBuf(buf, BIT_MAX_BUFFER) < 0) {
            mprCloseCmdFd(cmd, channel);
            return;
        }
        space = mprGetBufSpace(buf);
    }
    len = mprReadCmd(cmd, channel, mprGetBufEnd(buf), space);
    errCode = mprGetError();
    mprTrace(6, "defaultCmdCallback channel %d, read len %d, pid %d, eof %d/%d", channel, len, cmd->pid, cmd->eofCount, 
        cmd->requiredEof);
    if (len <= 0) {
        if (len == 0 || (len < 0 && !(errCode == EAGAIN || errCode == EWOULDBLOCK))) {
            mprCloseCmdFd(cmd, channel);
            return;
        }
    } else {
        mprAdjustBufEnd(buf, len);
    }
    mprAddNullToBuf(buf);
    mprEnableCmdEvents(cmd, channel);
}


static void stdinCallback(MprCmd *cmd, MprEvent *event)
{
    if (cmd->callback && cmd->files[MPR_CMD_STDIN].fd >= 0) {
        (cmd->callback)(cmd, MPR_CMD_STDIN, cmd->callbackData);
    }
}


static void stdoutCallback(MprCmd *cmd, MprEvent *event)
{
    /*
        reapCmd can consume data from the client and close the fd
     */
    if (cmd->callback && cmd->files[MPR_CMD_STDOUT].fd >= 0) {
        (cmd->callback)(cmd, MPR_CMD_STDOUT, cmd->callbackData);
    }
}


static void stderrCallback(MprCmd *cmd, MprEvent *event)
{
    /*
        reapCmd can consume data from the client and close the fd
     */
    if (cmd->callback && cmd->files[MPR_CMD_STDERR].fd >= 0) {
        (cmd->callback)(cmd, MPR_CMD_STDERR, cmd->callbackData);
    }
}


PUBLIC void mprSetCmdCallback(MprCmd *cmd, MprCmdProc proc, void *data)
{
    cmd->callback = proc;
    cmd->callbackData = data;
}


PUBLIC int mprGetCmdExitStatus(MprCmd *cmd)
{
    assert(cmd);

    if (cmd->pid == 0) {
        return cmd->status;
    }
    return MPR_ERR_NOT_READY;
}


PUBLIC bool mprIsCmdRunning(MprCmd *cmd)
{
    return cmd->pid > 0;
}


/* FUTURE - not yet supported */

PUBLIC void mprSetCmdTimeout(MprCmd *cmd, MprTicks timeout)
{
    assert(0);
#if KEEP
    cmd->timeoutPeriod = timeout;
#endif
}


PUBLIC int mprGetCmdFd(MprCmd *cmd, int channel) 
{ 
    return cmd->files[channel].fd; 
}


PUBLIC MprBuf *mprGetCmdBuf(MprCmd *cmd, int channel)
{
    return (channel == MPR_CMD_STDOUT) ? cmd->stdoutBuf : cmd->stderrBuf;
}


PUBLIC void mprSetCmdDir(MprCmd *cmd, cchar *dir)
{
#if VXWORKS
    mprError("WARNING: Setting working directory on VxWorks is global: %s", dir);
#else
    assert(dir && *dir);

    cmd->dir = sclone(dir);
#endif
}


#if BIT_WIN_LIKE
static int sortEnv(char **str1, char **str2)
{
    cchar    *s1, *s2;
    int     c1, c2;

    for (s1 = *str1, s2 = *str2; *s1 && *s2; s1++, s2++) {
        c1 = tolower((uchar) *s1);
        c2 = tolower((uchar) *s2);
        if (c1 < c2) {
            return -1;
        } else if (c1 > c2) {
            return 1;
        }
    }
    if (*s2) {
        return -1;
    } else if (*s1) {
        return 1;
    }
    return 0;
}
#endif


/*
    Match two environment keys up to the '='
 */
static bool matchEnvKey(cchar *s1, cchar *s2) 
{
    for (; *s1 && *s2; s1++, s2++) {
        if (*s1 != *s2) {
            break;
        } else if (*s1 == '=') {
            return 1;
        }
    }
    return 0;
}


static int blendEnv(MprCmd *cmd, cchar **env, int flags)
{
    cchar       **ep, *prior;
    int         next;

    cmd->env = 0;

    if ((cmd->env = mprCreateList(128, MPR_LIST_STATIC_VALUES)) == 0) {
        return MPR_ERR_MEMORY;
    }
#if !VXWORKS
    /*
        Add prior environment to the list
     */
    if (!(flags & MPR_CMD_EXACT_ENV)) {
        for (ep = (cchar**) environ; ep && *ep; ep++) {
            mprAddItem(cmd->env, *ep);
        }
    }
#endif
    /*
        Add new env keys. Detect and overwrite duplicates
     */
    for (ep = env; ep && *ep; ep++) {
        prior = 0;
        for (ITERATE_ITEMS(cmd->env, prior, next)) {
            if (matchEnvKey(*ep, prior)) {
                mprSetItem(cmd->env, next - 1, *ep);
                break;
            }
        }
        if (prior == 0) {
            mprAddItem(cmd->env, *ep);
        }
    }
#if BIT_WIN_LIKE
    /*
        Windows requires a caseless sort with two trailing nulls
     */
    mprSortList(cmd->env, (MprSortProc) sortEnv, 0);
#endif
    mprAddItem(cmd->env, NULL);
    return 0;
}


#if BIT_WIN_LIKE
static cchar *makeWinEnvBlock(MprCmd *cmd)
{
    char    *item, *dp, *ep, *env;
    ssize   len;
    int     next;

    for (len = 2, ITERATE_ITEMS(cmd->env, item, next)) {
        len += slen(item) + 1;
    }
    if ((env = mprAlloc(len)) == 0) {
        return 0;
    }
    ep = &env[len];
    dp = env;
    for (ITERATE_ITEMS(cmd->env, item, next)) {
        strcpy(dp, item);
        dp += slen(item) + 1;
    }
    /* Windows requires two nulls */
    *dp++ = '\0';
    *dp++ = '\0';
    assert(dp <= ep);
    return env;
}
#endif


/*
    Sanitize args. Convert "/" to "\" and converting '\r' and '\n' to spaces, quote all args and put the program as argv[0].
 */
static int sanitizeArgs(MprCmd *cmd, int argc, cchar **argv, cchar **env, int flags)
{
#if BIT_UNIX_LIKE || VXWORKS
    cmd->argv = argv;
    cmd->argc = argc;
#endif

#if BIT_WIN_LIKE
    /*
        WARNING: If starting a program compiled with Cygwin, there is a bug in Cygwin's parsing of the command
        string where embedded quotes are parsed incorrectly by the Cygwin CRT runtime. If an arg starts with a 
        drive spec, embedded backquoted quotes will be stripped and the backquote will be passed in. Windows CRT 
        handles this correctly.  For example:  
            ./args "c:/path \"a b\"
            Cygwin will parse as  argv[1] == c:/path \a \b
            Windows will parse as argv[1] == c:/path "a b"
     */
    cchar       *saveArg0, **ap, *start, *cp;
    char        *pp, *program, *dp, *localArgv[2];
    ssize       len;
    int         quote;

    assert(argc > 0 && argv[0] != NULL);

    cmd->argv = argv;
    cmd->argc = argc;

    program = cmd->arg0 = mprAlloc(slen(argv[0]) * 2 + 1);
    strcpy(program, argv[0]);

    for (pp = program; *pp; pp++) {
        if (*pp == '/') {
            *pp = '\\';
        } else if (*pp == '\r' || *pp == '\n') {
            *pp = ' ';
        }
    }
    if (*program == '\"') {
        if ((pp = strrchr(++program, '"')) != 0) {
            *pp = '\0';
        }
    }
    if (argv == 0) {
        argv = localArgv;
        argv[1] = 0;
        saveArg0 = program;
    } else {
        saveArg0 = argv[0];
    }
    /*
        Set argv[0] to the program name while creating the command line. Restore later.
     */
    argv[0] = program;
    argc = 0;
    for (len = 0, ap = argv; *ap; ap++) {
        len += (slen(*ap) * 2) + 1 + 2;         /* Space and possible quotes and worst case backquoting */
        argc++;
    }
    cmd->command = mprAlloc(len + 1);
    cmd->command[len] = '\0';
    
    /*
        Add quotes around all args that have spaces and backquote [", ', \\]
        Example:    ["showColors", "red", "light blue", "Cannot \"render\""]
        Becomes:    "showColors" "red" "light blue" "Cannot \"render\""
     */
    dp = cmd->command;
    for (ap = &argv[0]; *ap; ) {
        start = cp = *ap;
        quote = '"';
        if (strchr(cp, ' ') != 0 && cp[0] != quote) {
            for (*dp++ = quote; *cp; ) {
                if (*cp == quote && !(cp > start && cp[-1] == '\\')) {
                    *dp++ = '\\';
                }
                *dp++ = *cp++;
            }
            *dp++ = quote;
        } else {
            strcpy(dp, cp);
            dp += strlen(cp);
        }
        if (*++ap) {
            *dp++ = ' ';
        }
    }
    *dp = '\0';
    argv[0] = saveArg0;
    mprLog(5, "Windows command line: %s", cmd->command);
#endif /* BIT_WIN_LIKE */
    return 0;
}


#if BIT_WIN_LIKE
static int startProcess(MprCmd *cmd)
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFO         startInfo;
    cchar               *envBlock;
    int                 err;

    memset(&startInfo, 0, sizeof(startInfo));
    startInfo.cb = sizeof(startInfo);

    startInfo.dwFlags = STARTF_USESHOWWINDOW;
    if (cmd->flags & MPR_CMD_SHOW) {
        startInfo.wShowWindow = SW_SHOW;
    } else {
        startInfo.wShowWindow = SW_HIDE;
    }
    startInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (cmd->flags & MPR_CMD_IN) {
        if (cmd->files[MPR_CMD_STDIN].clientFd > 0) {
            startInfo.hStdInput = (HANDLE) _get_osfhandle(cmd->files[MPR_CMD_STDIN].clientFd);
        }
    } else {
        startInfo.hStdInput = (HANDLE) _get_osfhandle((int) fileno(stdin));
    }
    if (cmd->flags & MPR_CMD_OUT) {
        if (cmd->files[MPR_CMD_STDOUT].clientFd > 0) {
            startInfo.hStdOutput = (HANDLE)_get_osfhandle(cmd->files[MPR_CMD_STDOUT].clientFd);
        }
    } else {
        startInfo.hStdOutput = (HANDLE)_get_osfhandle((int) fileno(stdout));
    }
    if (cmd->flags & MPR_CMD_ERR) {
        if (cmd->files[MPR_CMD_STDERR].clientFd > 0) {
            startInfo.hStdError = (HANDLE) _get_osfhandle(cmd->files[MPR_CMD_STDERR].clientFd);
        }
    } else {
        startInfo.hStdError = (HANDLE) _get_osfhandle((int) fileno(stderr));
    }
    envBlock = makeWinEnvBlock(cmd);
    if (! CreateProcess(0, wide(cmd->command), 0, 0, 1, 0, (char*) envBlock, wide(cmd->dir), &startInfo, &procInfo)) {
        err = mprGetOsError();
        if (err == ERROR_DIRECTORY) {
            mprError("Cannot create process: %s, directory %s is invalid", cmd->program, cmd->dir);
        } else {
            mprError("Cannot create process: %s, %d", cmd->program, err);
        }
        return MPR_ERR_CANT_CREATE;
    }
    cmd->thread = procInfo.hThread;
    cmd->process = procInfo.hProcess;
    cmd->pid = procInfo.dwProcessId;
    return 0;
}


#if WINCE
//  FUTURE - merge this with WIN
static int makeChannel(MprCmd *cmd, int index)
{
    SECURITY_ATTRIBUTES clientAtt, serverAtt, *att;
    HANDLE              readHandle, writeHandle;
    MprCmdFile          *file;
    char                *path;
    int                 readFd, writeFd;

    memset(&clientAtt, 0, sizeof(clientAtt));
    clientAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    clientAtt.bInheritHandle = 1;

    /*
        Server fds are not inherited by the child
     */
    memset(&serverAtt, 0, sizeof(serverAtt));
    serverAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    serverAtt.bInheritHandle = 0;

    file = &cmd->files[index];
    path = mprGetTempPath(cmd, NULL);

    att = (index == MPR_CMD_STDIN) ? &clientAtt : &serverAtt;
    readHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, att, OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL,0);
    if (readHandle == INVALID_HANDLE_VALUE) {
        mprError(cmd, "Cannot create stdio pipes %s. Err %d", path, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    readFd = (int) (int64) _open_osfhandle((int*) readHandle, 0);

    att = (index == MPR_CMD_STDIN) ? &serverAtt: &clientAtt;
    writeHandle = CreateFile(path, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, att, OPEN_ALWAYS, 
        FILE_ATTRIBUTE_NORMAL, 0);
    writeFd = (int) _open_osfhandle((int*) writeHandle, 0);

    if (readFd < 0 || writeFd < 0) {
        mprError(cmd, "Cannot create stdio pipes %s. Err %d", path, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    if (index == MPR_CMD_STDIN) {
        file->clientFd = readFd;
        file->fd = writeFd;
        file->handle = writeHandle;
    } else {
        file->clientFd = writeFd;
        file->fd = readFd;
        file->handle = readHandle;
    }
    return 0;
}

#else /* !WINCE */
static int makeChannel(MprCmd *cmd, int index)
{
    SECURITY_ATTRIBUTES clientAtt, serverAtt, *att;
    HANDLE              readHandle, writeHandle;
    MprCmdFile          *file;
    MprTicks            now;
    char                *pipeName;
    int                 openMode, pipeMode, readFd, writeFd;
    static int          tempSeed = 0;

    memset(&clientAtt, 0, sizeof(clientAtt));
    clientAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    clientAtt.bInheritHandle = 1;

    /*
        Server fds are not inherited by the child
     */
    memset(&serverAtt, 0, sizeof(serverAtt));
    serverAtt.nLength = sizeof(SECURITY_ATTRIBUTES);
    serverAtt.bInheritHandle = 0;

    file = &cmd->files[index];
    now = ((int) mprGetTicks() & 0xFFFF) % 64000;

    lock(MPR->cmdService);
    pipeName = sfmt("\\\\.\\pipe\\MPR_%d_%d_%d.tmp", getpid(), (int) now, ++tempSeed);
    unlock(MPR->cmdService);

    /*
        Pipes are always inbound. The file below is outbound. we swap whether the client or server
        inherits the pipe or file. MPR_CMD_STDIN is the clients input pipe.
        Pipes are blocking since both ends share the same blocking mode. Client must be blocking.
     */
    openMode = PIPE_ACCESS_INBOUND;
    pipeMode = 0;

    att = (index == MPR_CMD_STDIN) ? &clientAtt : &serverAtt;
    readHandle = CreateNamedPipe(wide(pipeName), openMode, pipeMode, 1, 0, 256 * 1024, 1, att);
    if (readHandle == INVALID_HANDLE_VALUE) {
        mprError("Cannot create stdio pipes %s. Err %d", pipeName, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    readFd = (int) (int64) _open_osfhandle((long) readHandle, 0);

    att = (index == MPR_CMD_STDIN) ? &serverAtt: &clientAtt;
    writeHandle = CreateFile(wide(pipeName), GENERIC_WRITE, 0, att, OPEN_EXISTING, openMode, 0);
    writeFd = (int) _open_osfhandle((long) writeHandle, 0);

    if (readFd < 0 || writeFd < 0) {
        mprError("Cannot create stdio pipes %s. Err %d", pipeName, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    if (index == MPR_CMD_STDIN) {
        file->clientFd = readFd;
        file->fd = writeFd;
        file->handle = writeHandle;
    } else {
        file->clientFd = writeFd;
        file->fd = readFd;
        file->handle = readHandle;
    }
    return 0;
}
#endif /* WINCE */


#elif BIT_UNIX_LIKE
static int makeChannel(MprCmd *cmd, int index)
{
    MprCmdFile      *file;
    int             fds[2];

    file = &cmd->files[index];

    if (pipe(fds) < 0) {
        mprError("Cannot create stdio pipes. Err %d", mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    if (index == MPR_CMD_STDIN) {
        file->clientFd = fds[0];        /* read fd */
        file->fd = fds[1];              /* write fd */
    } else {
        file->clientFd = fds[1];        /* write fd */
        file->fd = fds[0];              /* read fd */
    }
    fcntl(file->fd, F_SETFL, fcntl(file->fd, F_GETFL) | O_NONBLOCK);
    mprTrace(7, "makeChannel: pipe handles[%d] read %d, write %d", index, fds[0], fds[1]);
    return 0;
}

#elif VXWORKS
static int makeChannel(MprCmd *cmd, int index)
{
    MprCmdFile      *file;
    int             nonBlock;
    static int      tempSeed = 0;

    file = &cmd->files[index];
    file->name = sfmt("/pipe/%s_%d_%d", BIT_PRODUCT, taskIdSelf(), tempSeed++);

    if (pipeDevCreate(file->name, 5, BIT_MAX_BUFFER) < 0) {
        mprError("Cannot create pipes to run %s", cmd->program);
        return MPR_ERR_CANT_OPEN;
    }
    /*
        Open the server end of the pipe. MPR_CMD_STDIN is from the client's perspective.
     */
    if (index == MPR_CMD_STDIN) {
        file->fd = open(file->name, O_WRONLY, 0644);
    } else {
        file->fd = open(file->name, O_RDONLY, 0644);
    }
    if (file->fd < 0) {
        mprError("Cannot create stdio pipes. Err %d", mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    nonBlock = 1;
    ioctl(file->fd, FIONBIO, (int) &nonBlock);
    return 0;
}
#endif


#if BIT_UNIX_LIKE
static int startProcess(MprCmd *cmd)
{
    MprCmdFile      *files;
    int             rc, i, err;

    files = cmd->files;
    if (!cmd->signal) {
        cmd->signal = mprAddSignalHandler(SIGCHLD, reapCmd, cmd, cmd->dispatcher, MPR_SIGNAL_BEFORE);
    }
    /*
        Create the child
     */
    cmd->pid = vfork();

    if (cmd->pid < 0) {
        mprError("start: can't fork a new process to run %s, errno %d", cmd->program, mprGetOsError());
        return MPR_ERR_CANT_INITIALIZE;

    } else if (cmd->pid == 0) {
        /*
            Child
         */
        umask(022);
        if (cmd->flags & MPR_CMD_NEW_SESSION) {
            setsid();
        }
        if (cmd->dir) {
            if (chdir(cmd->dir) < 0) {
                mprError("cmd: Cannot change directory to %s", cmd->dir);
                return MPR_ERR_CANT_INITIALIZE;
            }
        }
        if (cmd->flags & MPR_CMD_IN) {
            if (files[MPR_CMD_STDIN].clientFd >= 0) {
                dup2(files[MPR_CMD_STDIN].clientFd, 0);
                close(files[MPR_CMD_STDIN].fd);
            } else {
                close(0);
            }
        }
        if (cmd->flags & MPR_CMD_OUT) {
            if (files[MPR_CMD_STDOUT].clientFd >= 0) {
                dup2(files[MPR_CMD_STDOUT].clientFd, 1);
                close(files[MPR_CMD_STDOUT].fd);
            } else {
                close(1);
            }
        }
        if (cmd->flags & MPR_CMD_ERR) {
            if (files[MPR_CMD_STDERR].clientFd >= 0) {
                dup2(files[MPR_CMD_STDERR].clientFd, 2);
                close(files[MPR_CMD_STDERR].fd);
            } else {
                close(2);
            }
        }
        cmd->forkCallback(cmd->forkData);
        if (cmd->env) {
            rc = execve(cmd->program, (char**) cmd->argv, (char**) &cmd->env->items[0]);
        } else {
            rc = execv(cmd->program, (char**) cmd->argv);
        }
        err = errno;
        printf("Cannot exec %s, rc %d, err %d\n", cmd->program, rc, err);

        /*
            Use _exit to avoid flushing I/O any other I/O.
         */
        _exit(-(MPR_ERR_CANT_INITIALIZE));

    } else {
        /*
            Close the client handles
         */
        for (i = 0; i < MPR_CMD_MAX_PIPE; i++) {
            if (files[i].clientFd >= 0) {
                close(files[i].clientFd);
                files[i].clientFd = -1;
            }
        }
    }
    return 0;
}


#elif VXWORKS
/*
    Start the command to run (stdIn and stdOut are named from the client's perspective)
 */
PUBLIC int startProcess(MprCmd *cmd)
{
    MprCmdTaskFn    entryFn;
    MprModule       *mp;
    SYM_TYPE        symType;
    char            *entryPoint, *program, *pair;
    int             pri, next;

    mprLog(4, "cmd: start %s", cmd->program);
    entryPoint = 0;
    if (cmd->env) {
        for (ITERATE_ITEMS(cmd->env, pair, next)) {
            if (sncmp(pair, "entryPoint=", 11) == 0) {
                entryPoint = sclone(&pair[11]);
            }
        }
    }
    program = mprGetPathBase(cmd->program);
    if (entryPoint == 0) {
        program = mprTrimPathExt(program);
        entryPoint = program;
    }
    if (symFindByName(sysSymTbl, entryPoint, (char**) (void*) &entryFn, &symType) < 0) {
        if ((mp = mprCreateModule(cmd->program, cmd->program, NULL, NULL)) == 0) {
            mprError("start: can't create module");
            return MPR_ERR_CANT_CREATE;
        }
        if (mprLoadModule(mp) < 0) {
            mprError("start: can't load DLL %s, errno %d", program, mprGetOsError());
            return MPR_ERR_CANT_READ;
        }
        if (symFindByName(sysSymTbl, entryPoint, (char**) (void*) &entryFn, &symType) < 0) {
            mprError("start: can't find symbol %s, errno %d", entryPoint, mprGetOsError());
            return MPR_ERR_CANT_ACCESS;
        }
    }
    taskPriorityGet(taskIdSelf(), &pri);

    cmd->pid = taskSpawn(entryPoint, pri, VX_FP_TASK | VX_PRIVATE_ENV, BIT_MPR_THREAD_STACK, (FUNCPTR) cmdTaskEntry, 
        (int) cmd->program, (int) entryFn, (int) cmd, 0, 0, 0, 0, 0, 0, 0);

    if (cmd->pid < 0) {
        mprError("start: can't create task %s, errno %d", entryPoint, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    mprTrace(7, "cmd, child taskId %d", cmd->pid);

    if (semTake(cmd->startCond, MPR_TIMEOUT_START_TASK) != OK) {
        mprError("start: child %s did not initialize, errno %d", cmd->program, mprGetOsError());
        return MPR_ERR_CANT_CREATE;
    }
    semDelete(cmd->startCond);
    cmd->startCond = 0;
    return 0;
}


/*
    Executed by the child process
 */
static void cmdTaskEntry(char *program, MprCmdTaskFn entry, int cmdArg)
{
    MprCmd          *cmd;
    MprCmdFile      *files;
    WIND_TCB        *tcb;
    char            *item;
    int             inFd, outFd, errFd, id, next;

    cmd = (MprCmd*) cmdArg;

    /*
        Open standard I/O files (in/out are from the server's perspective)
     */
    files = cmd->files;
    inFd = open(files[MPR_CMD_STDIN].name, O_RDONLY, 0666);
    outFd = open(files[MPR_CMD_STDOUT].name, O_WRONLY, 0666);
    errFd = open(files[MPR_CMD_STDERR].name, O_WRONLY, 0666);

    if (inFd < 0 || outFd < 0 || errFd < 0) {
        exit(255);
    }
    id = taskIdSelf();
    ioTaskStdSet(id, 0, inFd);
    ioTaskStdSet(id, 1, outFd);
    ioTaskStdSet(id, 2, errFd);

    /*
        Now that we have opened the stdin and stdout, wakeup our parent.
     */
    semGive(cmd->startCond);

    /*
        Create the environment
     */
    if (envPrivateCreate(id, -1) < 0) {
        exit(254);
    }
    for (ITERATE_ITEMS(cmd->env, item, next)) {
        putenv(item);
    }

#if !VXWORKS
{
    char    *dir;
    int     rc;

    /*
        Set current directory if required
        WARNING: Setting working directory on VxWorks is global
     */
    if (cmd->dir) {
        rc = chdir(cmd->dir);
    } else {
        dir = mprGetPathDir(cmd->program);
        rc = chdir(dir);
    }
    if (rc < 0) {
        mprError("cmd: Cannot change directory to %s", cmd->dir);
        exit(255);
    }
}
#endif

    /*
        Call the user's entry point
     */
    (entry)(cmd->argc, (char**) cmd->argv, (char**) cmd->env);

    tcb = taskTcb(id);
    cmd->status = tcb->exitCode;

    /*
        Cleanup
     */
    envPrivateDestroy(id);
    close(inFd);
    close(outFd);
    close(errFd);
    semGive(cmd->exitCond);
}


#endif /* VXWORKS */


static void closeFiles(MprCmd *cmd)
{
    int     i;
    for (i = 3; i < MPR_MAX_FILE; i++) {
        close(i);
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/cond.c"
 */
/************************************************************************/

/**
    cond.c - Thread Conditional variables

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */



/***************************** Forward Declarations ***************************/

static void manageCond(MprCond *cp, int flags);

/************************************ Code ************************************/
/*
    Create a condition variable for use by single or multiple waiters
 */

PUBLIC MprCond *mprCreateCond()
{
    MprCond     *cp;

    if ((cp = mprAllocObj(MprCond, manageCond)) == 0) {
        return 0;
    }
    cp->triggered = 0;
    cp->mutex = mprCreateLock();

#if BIT_WIN_LIKE
    cp->cv = CreateEvent(NULL, FALSE, FALSE, NULL);
#elif VXWORKS
    cp->cv = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
#else
    pthread_cond_init(&cp->cv, NULL);
#endif
    return cp;
}


static void manageCond(MprCond *cp, int flags)
{
    assert(cp);
    
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cp->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        assert(cp->mutex);
#if BIT_WIN_LIKE
        CloseHandle(cp->cv);
#elif VXWORKS
        semDelete(cp->cv);
#else
        pthread_cond_destroy(&cp->cv);
#endif
    }
}


/*
    Wait for the event to be triggered. Should only be used when there are single waiters. If the event is already
    triggered, then it will return immediately. Timeout of -1 means wait forever. Timeout of 0 means no wait.
    Returns 0 if the event was signalled. Returns < 0 for a timeout.

    WARNING: On unix, the pthread_cond_timedwait uses an absolute time (Ugh!). So time-warps for daylight-savings may
    cause waits to prematurely return.
 */
PUBLIC int mprWaitForCond(MprCond *cp, MprTicks timeout)
{
    MprTicks            now, expire;
    int                 rc;
#if BIT_UNIX_LIKE
    struct timespec     waitTill;
    struct timeval      current;
    int                 usec;
#endif

    /*
        Avoid doing a mprGetTicks() if timeout is < 0
     */
    rc = 0;
    if (timeout >= 0) {
        now = mprGetTicks();
        expire = now + timeout;
#if BIT_UNIX_LIKE
        gettimeofday(&current, NULL);
        usec = current.tv_usec + ((int) (timeout % 1000)) * 1000;
        waitTill.tv_sec = current.tv_sec + ((int) (timeout / 1000)) + (usec / 1000000);
        waitTill.tv_nsec = (usec % 1000000) * 1000;
#endif
    } else {
        expire = -1;
        now = 0;
    }
    mprLock(cp->mutex);
    /*
        NOTE: The WaitForSingleObject and semTake APIs keeps state as to whether the object is signalled.
        WaitForSingleObject and semTake will not block if the object is already signalled. However, pthread_cond_ 
        is different and does not keep such state. If it is signalled before pthread_cond_wait, the thread will 
        still block. Consequently we need to keep our own state in cp->triggered. This also protects against 
        spurious wakeups which can happen (on windows).
     */
    do {
#if BIT_WIN_LIKE
        /*
            Regardless of the state of cp->triggered, we must call WaitForSingleObject to consume the signalled
            internal state of the object.
         */
        mprUnlock(cp->mutex);
        rc = WaitForSingleObject(cp->cv, (int) (expire - now));
        mprLock(cp->mutex);
        if (rc == WAIT_OBJECT_0) {
            rc = 0;
            ResetEvent(cp->cv);
        } else if (rc == WAIT_TIMEOUT) {
            rc = MPR_ERR_TIMEOUT;
        } else {
            rc = MPR_ERR;
        }
#elif VXWORKS
        /*
            Regardless of the state of cp->triggered, we must call semTake to consume the semaphore signalled state
         */
        mprUnlock(cp->mutex);
        rc = semTake(cp->cv, (int) (expire - now));
        mprLock(cp->mutex);
        if (rc != 0) {
            if (errno == S_objLib_OBJ_UNAVAILABLE) {
                rc = MPR_ERR_TIMEOUT;
            } else {
                rc = MPR_ERR;
            }
        }
        
#elif BIT_UNIX_LIKE
        /*
            NOTE: pthread_cond_timedwait can return 0 (MAC OS X and Linux). The pthread_cond_wait routines will 
            atomically unlock the mutex before sleeping and will relock on awakening.  
         */
        if (!cp->triggered) {
            if (now) {
                rc = pthread_cond_timedwait(&cp->cv, &cp->mutex->cs,  &waitTill);
            } else {
                rc = pthread_cond_wait(&cp->cv, &cp->mutex->cs);
            }
            if (rc == ETIMEDOUT) {
                rc = MPR_ERR_TIMEOUT;
            } else if (rc == EAGAIN) {
                rc = 0;
            } else if (rc != 0) {
                assert(rc == 0);
                mprError("pthread_cond_timedwait error rc %d", rc);
                rc = MPR_ERR;
            }
        }
#endif
    } while (!cp->triggered && rc == 0 && (now && (now = mprGetTicks()) < expire));

    if (cp->triggered) {
        cp->triggered = 0;
        rc = 0;
    } else if (rc == 0) {
        rc = MPR_ERR_TIMEOUT;
    }
    mprUnlock(cp->mutex);
    return rc;
}


/*
    Signal a condition and wakeup the waiter. Note: this may be called prior to the waiter waiting.
 */
PUBLIC void mprSignalCond(MprCond *cp)
{
    mprLock(cp->mutex);
    if (!cp->triggered) {
        cp->triggered = 1;
#if BIT_WIN_LIKE
        SetEvent(cp->cv);
#elif VXWORKS
        semGive(cp->cv);
#else
        pthread_cond_signal(&cp->cv);
#endif
    }
    mprUnlock(cp->mutex);
}


PUBLIC void mprResetCond(MprCond *cp)
{
    mprLock(cp->mutex);
    cp->triggered = 0;
#if BIT_WIN_LIKE
    ResetEvent(cp->cv);
#elif VXWORKS
    semDelete(cp->cv);
    cp->cv = semCCreate(SEM_Q_PRIORITY, SEM_EMPTY);
#else
    pthread_cond_destroy(&cp->cv);
    pthread_cond_init(&cp->cv, NULL);
#endif
    mprUnlock(cp->mutex);
}


/*
    Wait for the event to be triggered when there may be multiple waiters. This routine may return early due to
    other signals or events. The caller must verify if the signalled condition truly exists. If the event is already
    triggered, then it will return immediately. This call will not reset cp->triggered and must be reset manually.
    A timeout of -1 means wait forever. Timeout of 0 means no wait.  Returns 0 if the event was signalled. 
    Returns < 0 for a timeout.

    WARNING: On unix, the pthread_cond_timedwait uses an absolute time (Ugh!). So time-warps for daylight-savings may
    cause waits to prematurely return.
 */
PUBLIC int mprWaitForMultiCond(MprCond *cp, MprTicks timeout)
{
    int         rc;
#if BIT_UNIX_LIKE
    struct timespec     waitTill;
    struct timeval      current;
    int                 usec;
#else
    MprTicks            now, expire;
#endif

    if (timeout < 0) {
        timeout = MAXINT;
    }
#if BIT_UNIX_LIKE
    gettimeofday(&current, NULL);
    usec = current.tv_usec + ((int) (timeout % 1000)) * 1000;
    waitTill.tv_sec = current.tv_sec + ((int) (timeout / 1000)) + (usec / 1000000);
    waitTill.tv_nsec = (usec % 1000000) * 1000;
#else
    now = mprGetTicks();
    expire = now + timeout;
#endif

#if BIT_WIN_LIKE
    rc = WaitForSingleObject(cp->cv, (int) (expire - now));
    if (rc == WAIT_OBJECT_0) {
        rc = 0;
    } else if (rc == WAIT_TIMEOUT) {
        rc = MPR_ERR_TIMEOUT;
    } else {
        rc = MPR_ERR;
    }
#elif VXWORKS
    rc = semTake(cp->cv, (int) (expire - now));
    if (rc != 0) {
        if (errno == S_objLib_OBJ_UNAVAILABLE) {
            rc = MPR_ERR_TIMEOUT;
        } else {
            rc = MPR_ERR;
        }
    }
#elif BIT_UNIX_LIKE
    mprLock(cp->mutex);
    rc = pthread_cond_timedwait(&cp->cv, &cp->mutex->cs,  &waitTill);
    if (rc == ETIMEDOUT) {
        rc = MPR_ERR_TIMEOUT;
    } else if (rc != 0) {
        assert(rc == 0);
        rc = MPR_ERR;
    }
    mprUnlock(cp->mutex);
#endif
    return rc;
}


/*
    Signal a condition and wakeup the all the waiters. Note: this may be called before or after to the waiter waiting.
 */
PUBLIC void mprSignalMultiCond(MprCond *cp)
{
    mprLock(cp->mutex);
#if BIT_WIN_LIKE
    /* Pulse event */
    SetEvent(cp->cv);
    ResetEvent(cp->cv);
#elif VXWORKS
    /* Reset sem count and then give once. Prevents accumulation */
    while (semTake(cp->cv, 0) == OK) ;
    semGive(cp->cv);
    semFlush(cp->cv);
#else
    pthread_cond_broadcast(&cp->cv);
#endif
    mprUnlock(cp->mutex);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/crypt.c"
 */
/************************************************************************/

/*
    crypt.c - Base-64 encoding and decoding and MD5 support.

    Algorithms by RSA. See license at the end of the file. 
    This module is not thread safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Locals ***********************************/
/*
    MD5 Constants for transform routine.
 */
#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

static uchar PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
   MD5 F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/*
   MD5 ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/*
     MD5 - FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
     Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
    (a) += F ((b), (c), (d)) + (x) + (uint)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define GG(a, b, c, d, x, s, ac) { \
    (a) += G ((b), (c), (d)) + (x) + (uint)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define HH(a, b, c, d, x, s, ac) { \
    (a) += H ((b), (c), (d)) + (x) + (uint)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}
#define II(a, b, c, d, x, s, ac) { \
    (a) += I ((b), (c), (d)) + (x) + (uint)(ac); \
    (a) = ROTATE_LEFT ((a), (s)); \
    (a) += (b); \
}

typedef struct {
    uint state[4];
    uint count[2];
    uchar buffer[64];
} MD5CONTEXT;

/******************************* Base 64 Data *********************************/

#define CRYPT_HASH_SIZE   16

/*
    Encoding map lookup
 */
static char encodeMap[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/',
};


/*
    Decode map
 */
static signed char decodeMap[] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, 
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};


#define SHA_SIZE   20

typedef struct MprSha {
    uint    hash[SHA_SIZE / 4];     /* Message Digest */
    uint    lowLength;              /* Message length in bits */
    uint    highLength;             /* Message length in bits */
    int     index;                  /* Index into message block array   */
    uchar   block[64];              /* 512-bit message blocks */
} MprSha;

#define shaShift(bits,word) (((word) << (bits)) | ((word) >> (32-(bits))))

/*************************** Forward Declarations *****************************/

static void decode(uint *output, uchar *input, uint len);
static void encode(uchar *output, uint *input, uint len);
static void finalizeMD5(uchar digest[16], MD5CONTEXT *context);
static void initMD5(MD5CONTEXT *context);
static void transform(uint state[4], uchar block[64]);
static void update(MD5CONTEXT *context, uchar *input, uint inputLen);

static void shaInit(MprSha *sha);
static void shaUpdate(MprSha *sha, cuchar *msg, ssize len);
static void shaFinalize(uchar *digest, MprSha *sha);
static void shaPad(MprSha *sha);
static void shaProcess(MprSha *sha);

/*********************************** Code *************************************/

PUBLIC int mprRandom()
{
#if WINDOWS || VXWORKS
    return rand();
#else
    return (int) random();
#endif
}


/*
    Decode a null terminated string and returns a null terminated string.
    Stops decoding at the end of string or '='
 */
PUBLIC char *mprDecode64(cchar *s)
{
    return mprDecode64Block(s, NULL, MPR_DECODE_TOKEQ);
}


/*
    Decode a null terminated string and return a block with length.
    Stops decoding at the end of the block or '=' if MPR_DECODE_TOKEQ is specified.
 */
PUBLIC char *mprDecode64Block(cchar *s, ssize *len, int flags)
{
    uint    bitBuf;
    char    *buffer, *bp;
    cchar   *end;
    ssize   size;
    int     c, i, j, shift;

    size = slen(s);
    if ((buffer = mprAlloc(size + 1)) == 0) {
        return NULL;
    }
    bp = buffer;
    *bp = '\0';
    end = &s[size];
    while (s < end && (*s != '=' || !(flags & MPR_DECODE_TOKEQ))) {
        bitBuf = 0;
        shift = 18;
        for (i = 0; i < 4 && (s < end && (*s != '=' || !(flags & MPR_DECODE_TOKEQ))); i++, s++) {
            c = decodeMap[*s & 0xff];
            if (c == -1) {
                return NULL;
            } 
            bitBuf = bitBuf | (c << shift);
            shift -= 6;
        }
        --i;
        assert((bp + i) < &buffer[size]);
        for (j = 0; j < i; j++) {
            *bp++ = (char) ((bitBuf >> (8 * (2 - j))) & 0xff);
        }
        *bp = '\0';
    }
    if (len) {
        *len = bp - buffer;
    }
    return buffer;
}


/*
    Encode a null terminated string.
    Returns a null terminated block
 */
PUBLIC char *mprEncode64(cchar *s)
{
    return mprEncode64Block(s, slen(s));
}


/*
    Encode a block of a given length
    Returns a null terminated block
 */
PUBLIC char *mprEncode64Block(cchar *s, ssize len)
{
    uint    shiftbuf;
    char    *buffer, *bp;
    cchar   *end;
    ssize   size;
    int     i, j, shift;

    size = len * 2;
    if ((buffer = mprAlloc(size + 1)) == 0) {
        return NULL;
    }
    bp = buffer;
    *bp = '\0';
    end = &s[len];
    while (s < end) {
        shiftbuf = 0;
        for (j = 2; s < end && j >= 0; j--, s++) {
            shiftbuf |= ((*s & 0xff) << (j * 8));
        }
        shift = 18;
        for (i = ++j; i < 4 && bp < &buffer[size] ; i++) {
            *bp++ = encodeMap[(shiftbuf >> shift) & 0x3f];
            shift -= 6;
        }
        while (j-- > 0) {
            *bp++ = '=';
        }
        *bp = '\0';
    }
    return buffer;
}


PUBLIC char *mprGetMD5(cchar *s)
{
    return mprGetMD5WithPrefix(s, slen(s), NULL);
}


/*
    Return the MD5 hash of a block. Returns allocated string. A prefix for the result can be supplied.
 */
PUBLIC char *mprGetMD5WithPrefix(cchar *buf, ssize length, cchar *prefix)
{
    MD5CONTEXT      context;
    uchar           hash[CRYPT_HASH_SIZE];
    cchar           *hex = "0123456789abcdef";
    char            *r, *str;
    char            result[(CRYPT_HASH_SIZE * 2) + 1];
    ssize           len;
    int             i;

    if (length < 0) {
        length = slen(buf);
    }
    initMD5(&context);
    update(&context, (uchar*) buf, (uint) length);
    finalizeMD5(hash, &context);

    for (i = 0, r = result; i < 16; i++) {
        *r++ = hex[hash[i] >> 4];
        *r++ = hex[hash[i] & 0xF];
    }
    *r = '\0';
    len = (prefix) ? slen(prefix) : 0;
    str = mprAlloc(sizeof(result) + len);
    if (str) {
        if (prefix) {
            strcpy(str, prefix);
        }
        strcpy(str + len, result);
    }
    return str;
}


/*
    MD5 initialization. Begins an MD5 operation, writing a new context.
 */ 
static void initMD5(MD5CONTEXT *context)
{
    context->count[0] = context->count[1] = 0;
    context->state[0] = 0x67452301;
    context->state[1] = 0xefcdab89;
    context->state[2] = 0x98badcfe;
    context->state[3] = 0x10325476;
}


/*
    MD5 block update operation. Continues an MD5 message-digest operation, processing another message block, 
    and updating the context.
 */
static void update(MD5CONTEXT *context, uchar *input, uint inputLen)
{
    uint    i, index, partLen;

    index = (uint) ((context->count[0] >> 3) & 0x3F);

    if ((context->count[0] += ((uint)inputLen << 3)) < ((uint)inputLen << 3)){
        context->count[1]++;
    }
    context->count[1] += ((uint)inputLen >> 29);
    partLen = 64 - index;

    if (inputLen >= partLen) {
        memcpy((uchar*) &context->buffer[index], (uchar*) input, partLen);
        transform(context->state, context->buffer);
        for (i = partLen; i + 63 < inputLen; i += 64) {
            transform(context->state, &input[i]);
        }
        index = 0;
    } else {
        i = 0;
    }
    memcpy((uchar*) &context->buffer[index], (uchar*) &input[i], inputLen-i);
}


/*
    MD5 finalization. Ends an MD5 message-digest operation, writing the message digest and zeroizing the context.
 */ 
static void finalizeMD5(uchar digest[16], MD5CONTEXT *context)
{
    uchar   bits[8];
    uint    index, padLen;

    /* Save number of bits */
    encode(bits, context->count, 8);

    /* Pad out to 56 mod 64. */
    index = (uint)((context->count[0] >> 3) & 0x3f);
    padLen = (index < 56) ? (56 - index) : (120 - index);
    update(context, PADDING, padLen);

    /* Append length (before padding) */
    update(context, bits, 8);
    /* Store state in digest */
    encode(digest, context->state, 16);

    /* Zero sensitive information. */
    memset((uchar*)context, 0, sizeof (*context));
}


/*
    MD5 basic transformation. Transforms state based on block.
 */
static void transform(uint state[4], uchar block[64])
{
    uint a = state[0], b = state[1], c = state[2], d = state[3], x[16];

    decode(x, block, 64);

    /* Round 1 */
    FF(a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
    FF(d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
    FF(c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
    FF(b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
    FF(a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
    FF(d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
    FF(c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
    FF(b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
    FF(a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
    FF(d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
    FF(c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
    FF(b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
    FF(a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
    FF(d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
    FF(c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
    FF(b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

    /* Round 2 */
    GG(a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
    GG(d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
    GG(c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
    GG(b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
    GG(a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
    GG(d, a, b, c, x[10], S22,  0x2441453); /* 22 */
    GG(c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
    GG(b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
    GG(a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
    GG(d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
    GG(c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
    GG(b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
    GG(a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
    GG(d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
    GG(c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
    GG(b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

    /* Round 3 */
    HH(a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
    HH(d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
    HH(c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
    HH(b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
    HH(a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
    HH(d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
    HH(c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
    HH(b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
    HH(a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
    HH(d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
    HH(c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
    HH(b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
    HH(a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
    HH(d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
    HH(c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
    HH(b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

    /* Round 4 */
    II(a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
    II(d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
    II(c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
    II(b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
    II(a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
    II(d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
    II(c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
    II(b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
    II(a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
    II(d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
    II(c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
    II(b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
    II(a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
    II(d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
    II(c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
    II(b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;

    /* Zero sensitive information. */
    memset((uchar*) x, 0, sizeof(x));
}


/*
    Encodes input(uint) into output(uchar). Assumes len is a multiple of 4.
 */
static void encode(uchar *output, uint *input, uint len)
{
    uint i, j;

    for (i = 0, j = 0; j < len; i++, j += 4) {
        output[j] = (uchar) (input[i] & 0xff);
        output[j+1] = (uchar) ((input[i] >> 8) & 0xff);
        output[j+2] = (uchar) ((input[i] >> 16) & 0xff);
        output[j+3] = (uchar) ((input[i] >> 24) & 0xff);
    }
}


/*
    Decodes input(uchar) into output(uint). Assumes len is a multiple of 4.
 */
static void decode(uint *output, uchar *input, uint len)
{
    uint    i, j;

    for (i = 0, j = 0; j < len; i++, j += 4)
        output[i] = ((uint) input[j]) | (((uint) input[j+1]) << 8) | (((uint) input[j+2]) << 16) | 
            (((uint) input[j+3]) << 24);
}


/************************************* Sha1 **********************************/

PUBLIC char *mprGetSHA(cchar *s)
{
    return mprGetSHAWithPrefix(s, slen(s), NULL);
}


PUBLIC char *mprGetSHABase64(cchar *s)
{
    MprSha  sha;
    uchar   hash[SHA_SIZE + 1];

    shaInit(&sha);
    shaUpdate(&sha, (cuchar*) s, slen(s));
    shaFinalize(hash, &sha);
    hash[SHA_SIZE] = '\0';
    return mprEncode64Block((char*) hash, SHA_SIZE);
}


PUBLIC char *mprGetSHAWithPrefix(cchar *buf, ssize length, cchar *prefix)
{
    MprSha  sha;
    uchar   hash[SHA_SIZE];
    cchar   *hex = "0123456789abcdef";
    char    *r, *str;
    char    result[(SHA_SIZE * 2) + 1];
    ssize   len;
    int     i;

    if (length < 0) {
        length = slen(buf);
    }
    shaInit(&sha);
    shaUpdate(&sha, (cuchar*) buf, length);
    shaFinalize(hash, &sha);

    for (i = 0, r = result; i < SHA_SIZE; i++) {
        *r++ = hex[hash[i] >> 4];
        *r++ = hex[hash[i] & 0xF];
    }
    *r = '\0';
    len = (prefix) ? slen(prefix) : 0;
    str = mprAlloc(sizeof(result) + len);
    if (str) {
        if (prefix) {
            strcpy(str, prefix);
        }
        strcpy(str + len, result);
    }
    return str;
}


static void shaInit(MprSha *sha)
{
    sha->lowLength = 0;
    sha->highLength = 0;
    sha->index = 0;
    sha->hash[0] = 0x67452301;
    sha->hash[1] = 0xEFCDAB89;
    sha->hash[2] = 0x98BADCFE;
    sha->hash[3] = 0x10325476;
    sha->hash[4] = 0xC3D2E1F0;
}


static void shaUpdate(MprSha *sha, cuchar *msg, ssize len)
{
    while (len--) {
        sha->block[sha->index++] = (*msg & 0xFF);
        sha->lowLength += 8;
        if (sha->lowLength == 0) {
            sha->highLength++;
        }
        if (sha->index == 64) {
            shaProcess(sha);
        }
        msg++;
    }
}


static void shaFinalize(uchar *digest, MprSha *sha)
{
    int i;

    shaPad(sha);
    memset(sha->block, 0, 64);
    sha->lowLength = 0;
    sha->highLength = 0;
    for  (i = 0; i < SHA_SIZE; i++) {
        digest[i] = sha->hash[i >> 2] >> 8 * (3 - (i & 0x03));
    }
}


static void shaProcess(MprSha *sha)
{
    uint    K[] = { 0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    uint    temp, W[80], A, B, C, D, E;
    int     t;

    for  (t = 0; t < 16; t++) {
        W[t] = sha->block[t * 4] << 24;
        W[t] |= sha->block[t * 4 + 1] << 16;
        W[t] |= sha->block[t * 4 + 2] << 8;
        W[t] |= sha->block[t * 4 + 3];
    }
    for (t = 16; t < 80; t++) {
       W[t] = shaShift(1, W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
    }
    A = sha->hash[0];
    B = sha->hash[1];
    C = sha->hash[2];
    D = sha->hash[3];
    E = sha->hash[4];

    for (t = 0; t < 20; t++) {
        temp =  shaShift(5, A) + ((B & C) | ((~B) & D)) + E + W[t] + K[0];
        E = D;
        D = C;
        C = shaShift(30, B);
        B = A;
        A = temp;
    }
    for (t = 20; t < 40; t++) {
        temp = shaShift(5, A) + (B ^ C ^ D) + E + W[t] + K[1];
        E = D;
        D = C;
        C = shaShift(30, B);
        B = A;
        A = temp;
    }
    for (t = 40; t < 60; t++) {
        temp = shaShift(5, A) + ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
        E = D;
        D = C;
        C = shaShift(30, B);
        B = A;
        A = temp;
    }
    for (t = 60; t < 80; t++) {
        temp = shaShift(5, A) + (B ^ C ^ D) + E + W[t] + K[3];
        E = D;
        D = C;
        C = shaShift(30, B);
        B = A;
        A = temp;
    }
    sha->hash[0] += A;
    sha->hash[1] += B;
    sha->hash[2] += C;
    sha->hash[3] += D;
    sha->hash[4] += E;
    sha->index = 0;
}


static void shaPad(MprSha *sha)
{
    if (sha->index > 55) {
        sha->block[sha->index++] = 0x80;
        while(sha->index < 64) {
            sha->block[sha->index++] = 0;
        }
        shaProcess(sha);
        while (sha->index < 56) {
            sha->block[sha->index++] = 0;
        }
    } else {
        sha->block[sha->index++] = 0x80;
        while(sha->index < 56) {
            sha->block[sha->index++] = 0;
        }
    }
    sha->block[56] = sha->highLength >> 24;
    sha->block[57] = sha->highLength >> 16;
    sha->block[58] = sha->highLength >> 8;
    sha->block[59] = sha->highLength;
    sha->block[60] = sha->lowLength >> 24;
    sha->block[61] = sha->lowLength >> 16;
    sha->block[62] = sha->lowLength >> 8;
    sha->block[63] = sha->lowLength;
    shaProcess(sha);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/disk.c"
 */
/************************************************************************/

/**
    disk.c - File services for systems with a (disk) based file system.

    This module is not thread safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



#if !BIT_ROM
/*********************************** Defines **********************************/

#if WINDOWS
/*
    Open/Delete retries to circumvent windows pending delete problems
 */
#define RETRIES 40

/*
    Windows only permits owner bits
 */
#define MASK_PERMS(perms)    perms & 0600
#else
#define MASK_PERMS(perms)    perms
#endif

/********************************** Forwards **********************************/

static int closeFile(MprFile *file);
static void manageDiskFile(MprFile *file, int flags);
static int getPathInfo(MprDiskFileSystem *fs, cchar *path, MprPath *info);

/************************************ Code ************************************/
#if FUTURE
/*
    Open a file with support for cygwin paths. Tries windows path first then under /cygwin.
 */
static int cygOpen(MprFileSystem *fs, cchar *path, int omode, int perms)
{
    int     fd;

    fd = open(path, omode, MASK_PERMS(perms));
#if WINDOWS
    if (fd < 0) {
        if (*path == '/') {
            path = sjoin(fs->cygwin, path, NULL);
        }
        fd = open(path, omode, MASK_PERMS(perms));
    }
#endif
    return fd;
}
#endif

static MprFile *openFile(MprFileSystem *fs, cchar *path, int omode, int perms)
{
    MprFile     *file;
    
    assert(path);

    if ((file = mprAllocObj(MprFile, manageDiskFile)) == 0) {
        return NULL;
    }
    file->path = sclone(path);
    file->fd = open(path, omode, MASK_PERMS(perms));
    if (file->fd < 0) {
#if WINDOWS
        /*
            Windows opens can fail of immediately following a delete. Windows uses pending deletes which prevent opens.
         */
        int i, err = GetLastError();
        if (err == ERROR_ACCESS_DENIED) {
            for (i = 0; i < RETRIES; i++) {
                file->fd = open(path, omode, MASK_PERMS(perms));
                if (file->fd >= 0) {
                    break;
                }
                mprNap(10);
            }
            if (file->fd < 0) {
                file = NULL;
            }
        } else {
            file = NULL;
        }
#else
        file = NULL;
#endif
    }
    return file;
}


static void manageDiskFile(MprFile *file, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(file->path);
        mprMark(file->fileSystem);
        mprMark(file->buf);
#if BIT_ROM
        mprMark(file->inode);
#endif

    } else if (flags & MPR_MANAGE_FREE) {
        closeFile(file);
    }
}


static int closeFile(MprFile *file)
{
    MprBuf  *bp;

    assert(file);

    if (file == 0) {
        return 0;
    }
    bp = file->buf;
    if (bp && (file->mode & (O_WRONLY | O_RDWR))) {
        mprFlushFile(file);
    }
    if (file->fd >= 0) {
        close(file->fd);
        file->fd = -1;
    }
    return 0;
}


static ssize readFile(MprFile *file, void *buf, ssize size)
{
    assert(file);
    assert(buf);

    return read(file->fd, buf, (uint) size);
}


static ssize writeFile(MprFile *file, cvoid *buf, ssize count)
{
    assert(file);
    assert(buf);

#if VXWORKS
    return write(file->fd, (void*) buf, count);
#else
    return write(file->fd, buf, (uint) count);
#endif
}


static MprOff seekFile(MprFile *file, int seekType, MprOff distance)
{
    assert(file);

    if (file == 0) {
        return MPR_ERR_BAD_HANDLE;
    }
#if BIT_WIN_LIKE
    return (MprOff) _lseeki64(file->fd, (int64) distance, seekType);
#elif BIT_HAS_OFF64
    return (MprOff) lseek64(file->fd, (off64_t) distance, seekType);
#else
    return (MprOff) lseek(file->fd, (off_t) distance, seekType);
#endif
}


static bool accessPath(MprDiskFileSystem *fs, cchar *path, int omode)
{
#if BIT_WIN && FUTURE
    if (access(path, omode) < 0) {
        if (*path == '/') {
            path = sjoin(fs->cygwin, path, NULL);
        }
    }
#endif
    return access(path, omode) == 0;
}


static int deletePath(MprDiskFileSystem *fs, cchar *path)
{
    MprPath     info;

    if (getPathInfo(fs, path, &info) == 0 && info.isDir && !info.isLink) {
        return rmdir((char*) path);
    }
#if WINDOWS
{
    /*
        NOTE: Windows delete makes a file pending delete which prevents immediate recreation. Rename and then delete.
     */
    int i, err;
    for (i = 0; i < RETRIES; i++) {
        if (DeleteFile(wide(path)) != 0) {
            return 0;
        }
        err = GetLastError();
        if (err != ERROR_SHARING_VIOLATION) {
            break;
        }
        mprNap(10);
    }
    return MPR_ERR_CANT_DELETE;
}
#else
    return unlink((char*) path);
#endif
}
 

static int makeDir(MprDiskFileSystem *fs, cchar *path, int perms, int owner, int group)
{
    int     rc;

#if VXWORKS
    rc = mkdir((char*) path);
#else
    rc = mkdir(path, perms);
#endif
    if (rc < 0) {
        return MPR_ERR_CANT_CREATE;
    }
#if BIT_UNIX_LIKE
    if ((owner != -1 || group != -1) && chown(path, owner, group) < 0) {
        rmdir(path);
        return MPR_ERR_CANT_COMPLETE;
    }
#endif
    return 0;
}


static int makeLink(MprDiskFileSystem *fs, cchar *path, cchar *target, int hard)
{
#if BIT_UNIX_LIKE
    if (hard) {
        return link(path, target);
    } else {
        return symlink(path, target);
    }
#else
    return MPR_ERR_BAD_STATE;
#endif
}


static int getPathInfo(MprDiskFileSystem *fs, cchar *path, MprPath *info)
{
#if WINCE
    struct stat s;
    cchar       *ext;

    assert(path);
    assert(info);

    info->checked = 1;
    info->valid = 0;
    info->isReg = 0;
    info->isDir = 0;

    if (_stat64(path, &s) < 0) {
        return -1;
    }
    info->valid = 1;
    info->size = s.st_size;
    info->atime = s.st_atime;
    info->ctime = s.st_ctime;
    info->mtime = s.st_mtime;
    info->perms = s.st_mode & 07777;
    info->owner = s.st_uid;
    info->group = s.st_gid;
    info->inode = s.st_ino;
    info->isDir = (s.st_mode & S_IFDIR) != 0;
    info->isReg = (s.st_mode & S_IFREG) != 0;
    info->isLink = 0;
    ext = mprGetPathExt(path);
    if (ext && strcmp(ext, "lnk") == 0) {
        info->isLink = 1;
    }

#elif BIT_WIN_LIKE
    struct __stat64     s;
    cchar               *ext;

    assert(path);
    assert(info);
    info->checked = 1;
    info->valid = 0;
    info->isReg = 0;
    info->isDir = 0;
    if (sends(path, "/")) {
        /* Windows stat fails with a trailing "/" */
        path = strim(path, "/", MPR_TRIM_END);
    }
    if (_stat64(path, &s) < 0) {
#if BIT_WIN && FUTURE
        /*
            Try under /cygwin
         */
        if (*path == '/') {
            path = sjoin(fs->cygwin, path, NULL);
        }
        if (_stat64(path, &s) < 0) {
            return -1;
        }
#else
        return -1;
#endif
    }
    ext = mprGetPathExt(path);
    info->valid = 1;
    info->size = s.st_size;
    info->atime = s.st_atime;
    info->ctime = s.st_ctime;
    info->mtime = s.st_mtime;
    info->perms = s.st_mode & 07777;
    info->owner = s.st_uid;
    info->group = s.st_gid;
    info->inode = s.st_ino;
    info->isDir = (s.st_mode & S_IFDIR) != 0;
    info->isReg = (s.st_mode & S_IFREG) != 0;
    info->isLink = 0;
    if (ext) {
        if (strcmp(ext, "lnk") == 0) {
            info->isLink = 1;
        } else if (strcmp(ext, "dll") == 0) {
            info->perms |= 111;
        }
    }
    /*
        Work hard on windows to determine if the file is a regular file.
     */
    if (info->isReg) {
        long    att;

        if ((att = GetFileAttributes(wide(path))) == -1) {
            return -1;
        }
        if (att & (FILE_ATTRIBUTE_REPARSE_POINT | FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_ENCRYPTED |
                FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_OFFLINE)) {
            /*
                Catch accesses to devices like CON, AUX, NUL, LPT etc att will be set to ENCRYPTED on Win9X and NT.
             */
            info->isReg = 0;
        }
        if (info->isReg) {
            HANDLE handle;
            handle = CreateFile(wide(path), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
            if (handle == INVALID_HANDLE_VALUE) {
                info->isReg = 0;
            } else {
                long    fileType;
                fileType = GetFileType(handle);
                if (fileType == FILE_TYPE_CHAR || fileType == FILE_TYPE_PIPE) {
                    info->isReg = 0;
                }
                CloseHandle(handle);
            }
        }
    }
    if (strcmp(path, "nul") == 0) {
        info->isReg = 0;
    }

#elif VXWORKS
    struct stat s;
    info->valid = 0;
    info->isReg = 0;
    info->isDir = 0;
    info->checked = 1;
    if (stat((char*) path, &s) < 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    info->valid = 1;
    info->size = s.st_size;
    info->atime = s.st_atime;
    info->ctime = s.st_ctime;
    info->mtime = s.st_mtime;
    info->inode = s.st_ino;
    info->isDir = S_ISDIR(s.st_mode);
    info->isReg = S_ISREG(s.st_mode);
    info->perms = s.st_mode & 07777;
    info->owner = s.st_uid;
    info->group = s.st_gid;
#else
    struct stat s;
    info->valid = 0;
    info->isReg = 0;
    info->isDir = 0;
    info->checked = 1;
    if (lstat((char*) path, &s) < 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    #ifdef S_ISLNK
        info->isLink = S_ISLNK(s.st_mode);
        if (info->isLink) {
            if (stat((char*) path, &s) < 0) {
                return MPR_ERR_CANT_ACCESS;
            }
        }
    #endif
    info->valid = 1;
    info->size = s.st_size;
    info->atime = s.st_atime;
    info->ctime = s.st_ctime;
    info->mtime = s.st_mtime;
    info->inode = s.st_ino;
    info->isDir = S_ISDIR(s.st_mode);
    info->isReg = S_ISREG(s.st_mode);
    info->perms = s.st_mode & 07777;
    info->owner = s.st_uid;
    info->group = s.st_gid;
    if (strcmp(path, "/dev/null") == 0) {
        info->isReg = 0;
    }
#endif
    return 0;
}
 
static char *getPathLink(MprDiskFileSystem *fs, cchar *path)
{
#if BIT_UNIX_LIKE
    char    pbuf[BIT_MAX_PATH];
    ssize   len;

    if ((len = readlink(path, pbuf, sizeof(pbuf) - 1)) < 0) {
        return NULL;
    }
    pbuf[len] = '\0';
    return sclone(pbuf);
#else
    return NULL;
#endif
}


static int truncateFile(MprDiskFileSystem *fs, cchar *path, MprOff size)
{
    if (!mprPathExists(path, F_OK)) {
#if BIT_WIN_LIKE && FUTURE
        /*
            Try under /cygwin
         */
        if (*path == '/') {
            path = sjoin(fs->cygwin, path, NULL);
        }
        if (!mprPathExists(path, F_OK))
#endif
        return MPR_ERR_CANT_ACCESS;
    }
#if BIT_WIN_LIKE
{
    HANDLE  h;

    h = CreateFile(wide(path), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    SetFilePointer(h, (LONG) size, 0, FILE_BEGIN);
    if (h == INVALID_HANDLE_VALUE || SetEndOfFile(h) == 0) {
        CloseHandle(h);
        return MPR_ERR_CANT_WRITE;
    }
    CloseHandle(h);
}
#elif VXWORKS
{
#if FUTURE
    int     fd;

    fd = open(path, O_WRONLY, 0664);
    if (fd < 0 || ftruncate(fd, size) < 0) {
        return MPR_ERR_CANT_WRITE;
    }
    close(fd);
#endif
    return MPR_ERR_CANT_WRITE;
}
#else
    if (truncate(path, size) < 0) {
        return MPR_ERR_CANT_WRITE;
    }
#endif
    return 0;
}


static void manageDiskFileSystem(MprDiskFileSystem *dfs, int flags)
{
#if !WINCE
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dfs->separators);
        mprMark(dfs->newline);
        mprMark(dfs->root);
#if BIT_WIN_LIKE || CYGWIN
        mprMark(dfs->cygdrive);
        mprMark(dfs->cygwin);
#endif
    }
#endif
}


PUBLIC MprDiskFileSystem *mprCreateDiskFileSystem(cchar *path)
{
    MprFileSystem       *fs;
    MprDiskFileSystem   *dfs;

    if ((dfs = mprAllocObj(MprDiskFileSystem, manageDiskFileSystem)) == 0) {
        return 0;
    }

    /*
        Temporary
     */
    fs = (MprFileSystem*) dfs;
    dfs->accessPath = accessPath;
    dfs->deletePath = deletePath;
    dfs->getPathInfo = getPathInfo;
    dfs->getPathLink = getPathLink;
    dfs->makeDir = makeDir;
    dfs->makeLink = makeLink;
    dfs->openFile = openFile;
    dfs->closeFile = closeFile;
    dfs->readFile = readFile;
    dfs->seekFile = seekFile;
    dfs->truncateFile = truncateFile;
    dfs->writeFile = writeFile;

#if !WINCE
    if ((MPR->stdError = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdError, "stderr");
    MPR->stdError->fd = 2;
    MPR->stdError->fileSystem = fs;
    MPR->stdError->mode = O_WRONLY;

    if ((MPR->stdInput = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdInput, "stdin");
    MPR->stdInput->fd = 0;
    MPR->stdInput->fileSystem = fs;
    MPR->stdInput->mode = O_RDONLY;

    if ((MPR->stdOutput = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdOutput, "stdout");
    MPR->stdOutput->fd = 1;
    MPR->stdOutput->fileSystem = fs;
    MPR->stdOutput->mode = O_WRONLY;
#endif
    return dfs;
}
#endif /* !BIT_ROM */


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/dispatcher.c"
 */
/************************************************************************/

/*
    dispatcher.c - Event dispatch services

    This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/***************************** Forward Declarations ***************************/

static void dequeueDispatcher(MprDispatcher *dispatcher);
static int dispatchEvents(MprDispatcher *dispatcher);
static MprTicks getDispatcherIdleTicks(MprDispatcher *dispatcher, MprTicks timeout);
static MprTicks getIdleTicks(MprEventService *es, MprTicks timeout);
static MprDispatcher *getNextReadyDispatcher(MprEventService *es);
static void initDispatcher(MprDispatcher *q);
static int makeRunnable(MprDispatcher *dispatcher);
static void manageDispatcher(MprDispatcher *dispatcher, int flags);
static void manageEventService(MprEventService *es, int flags);
static void queueDispatcher(MprDispatcher *prior, MprDispatcher *dispatcher);
static void scheduleDispatcher(MprDispatcher *dispatcher);
static void serviceDispatcherMain(MprDispatcher *dispatcher);
static bool serviceDispatcher(MprDispatcher *dp);

#define isRunning(dispatcher) (dispatcher->parent == dispatcher->service->runQ)
#define isReady(dispatcher) (dispatcher->parent == dispatcher->service->readyQ)
#define isWaiting(dispatcher) (dispatcher->parent == dispatcher->service->waitQ)
#define isEmpty(dispatcher) (dispatcher->eventQ->next == dispatcher->eventQ)
#if KEEP
static int dqlen(MprDispatcher *dq);
#endif

/************************************* Code ***********************************/
/*
    Create the overall dispatch service. There may be many event dispatchers.
 */
PUBLIC MprEventService *mprCreateEventService()
{
    MprEventService     *es;

    if ((es = mprAllocObj(MprEventService, manageEventService)) == 0) {
        return 0;
    }
    MPR->eventService = es;
    es->now = mprGetTicks();
    es->mutex = mprCreateLock();
    es->waitCond = mprCreateCond();
    es->runQ = mprCreateDispatcher("running", 0);
    es->readyQ = mprCreateDispatcher("ready", 0);
    es->idleQ = mprCreateDispatcher("idle", 0);
    es->pendingQ = mprCreateDispatcher("pending", 0);
    es->waitQ = mprCreateDispatcher("waiting", 0);
    return es;
}


static void manageEventService(MprEventService *es, int flags)
{
    MprDispatcher   *dp;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(es->runQ);
        mprMark(es->readyQ);
        mprMark(es->waitQ);
        mprMark(es->idleQ);
        mprMark(es->pendingQ);
        mprMark(es->waitCond);
        mprMark(es->mutex);

        for (dp = es->runQ->next; dp != es->runQ; dp = dp->next) {
            mprMark(dp);
        }
        for (dp = es->readyQ->next; dp != es->readyQ; dp = dp->next) {
            mprMark(dp);
        }
        for (dp = es->waitQ->next; dp != es->waitQ; dp = dp->next) {
            mprMark(dp);
        }
        for (dp = es->idleQ->next; dp != es->idleQ; dp = dp->next) {
            mprMark(dp);
        }
        for (dp = es->pendingQ->next; dp != es->pendingQ; dp = dp->next) {
            mprMark(dp);
        }

    } else if (flags & MPR_MANAGE_FREE) {
        /* Needed for race with manageDispatcher */
        es->mutex = 0;
    }
}


PUBLIC void mprStopEventService()
{
    mprWakeDispatchers();
    mprWakeNotifier();
}


/*
    Create a disabled dispatcher. A dispatcher schedules events on a single dispatch queue.
 */
PUBLIC MprDispatcher *mprCreateDispatcher(cchar *name, int flags)
{
    MprEventService     *es;
    MprDispatcher       *dispatcher;

    if ((dispatcher = mprAllocObj(MprDispatcher, manageDispatcher)) == 0) {
        return 0;
    }
    dispatcher->name = sclone(name);
    dispatcher->cond = mprCreateCond();
    dispatcher->flags = flags;
    dispatcher->magic = MPR_DISPATCHER_MAGIC;
    es = dispatcher->service = MPR->eventService;
    dispatcher->eventQ = mprCreateEventQueue();
    dispatcher->currentQ = mprCreateEventQueue();
    if (flags & MPR_DISPATCHER_ENABLED) {
        queueDispatcher(es->idleQ, dispatcher);
    } else {
        initDispatcher(dispatcher);
    }
    return dispatcher;
}


static void mprDestroyDispatcher(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *q, *event, *next;

    if (dispatcher && !(dispatcher->flags & MPR_DISPATCHER_DESTROYED)) {
        es = dispatcher->service;
        assert(es == MPR->eventService);
        lock(es);
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        assert(dispatcher->service == MPR->eventService);
        assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
        q = dispatcher->eventQ;
        for (event = q->next; event != q; event = next) {
            assert(event->magic == MPR_EVENT_MAGIC);
            next = event->next;
            if (event->dispatcher) {
                mprRemoveEvent(event);
            }
        }
        dequeueDispatcher(dispatcher);
        assert(dispatcher->parent == dispatcher);

        dispatcher->flags = MPR_DISPATCHER_DESTROYED;
        dispatcher->owner = 0;
        dispatcher->magic = MPR_DISPATCHER_FREE;
        unlock(es);
    }
}


static void manageDispatcher(MprDispatcher *dispatcher, int flags)
{
    MprEventService     *es;
    MprEvent            *q, *event, *next;

    es = dispatcher->service;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(dispatcher->name);
        mprMark(dispatcher->eventQ);
        mprMark(dispatcher->currentQ);
        mprMark(dispatcher->cond);
        mprMark(dispatcher->parent);
        mprMark(dispatcher->service);
        mprMark(dispatcher->requiredWorker);

        //  MOB - is this lock needed?  Surely all threads are stopped.
        lock(es);
        q = dispatcher->eventQ;
        for (event = q->next; event != q; event = next) {
            next = event->next;
            assert(event->magic == MPR_EVENT_MAGIC);
            mprMark(event);
        }
        q = dispatcher->currentQ;
        for (event = q->next; event != q; event = next) {
            next = event->next;
            assert(event->magic == MPR_EVENT_MAGIC);
            mprMark(event);
        }
        unlock(es);
        
    } else if (flags & MPR_MANAGE_FREE) {
        mprDestroyDispatcher(dispatcher);
    }
}


PUBLIC void mprDisableDispatcher(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *q, *event, *next;

    if (dispatcher && (dispatcher->flags & MPR_DISPATCHER_ENABLED)) {
        es = dispatcher->service;
        lock(es);
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        assert(dispatcher->service == MPR->eventService);
        assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
        q = dispatcher->eventQ;
        for (event = q->next; event != q; event = next) {
            assert(event->magic == MPR_EVENT_MAGIC);
            next = event->next;
            if (event->dispatcher) {
                mprRemoveEvent(event);
            }
        }
        dequeueDispatcher(dispatcher);
        assert(dispatcher->parent == dispatcher);
        dispatcher->flags &= ~MPR_DISPATCHER_ENABLED;
        unlock(es);
    }
}


PUBLIC void mprEnableDispatcher(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    int                 mustWake;

    if (dispatcher == 0) {
        dispatcher = MPR->dispatcher;
    }
    es = dispatcher->service;
    mustWake = 0;
    lock(es);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    if (!(dispatcher->flags & MPR_DISPATCHER_ENABLED)) {
        dispatcher->flags |= MPR_DISPATCHER_ENABLED;
        mprTrace(7, "mprEnableDispatcher: %s", dispatcher->name);
        if (!isEmpty(dispatcher) && !isReady(dispatcher) && !isRunning(dispatcher)) {
            queueDispatcher(es->readyQ, dispatcher);
            if (es->waiting) {
                mustWake = 1;
            }
        }
    }
    unlock(es);
    if (mustWake) {
        mprWakeNotifier();
    }
}


/*
    Schedule events. This can be called by any thread. Typically an app will dedicate one thread to be an event service 
    thread. This call will service events until the timeout expires or if MPR_SERVICE_ONE_THING is specified in flags, 
    after one event. This will service all enabled dispatcher queues and pending I/O events.
    @param dispatcher Primary dispatcher to service. This dispatcher is set to the running state and events on this
        dispatcher will be serviced without starting a worker thread. This can be set to NULL.
    @param timeout Time in milliseconds to wait. Set to zero for no wait. Set to -1 to wait forever.
    @returns Zero if not events occurred. Otherwise returns non-zero.
 */
PUBLIC int mprServiceEvents(MprTicks timeout, int flags)
{
    MprEventService     *es;
    MprDispatcher       *dp;
    MprTicks            expires, delay;
    int                 beginEventCount, eventCount, justOne;

    if (MPR->eventing) {
        mprError("mprServiceEvents() called reentrantly");
        return 0;
    }
    MPR->eventing = 1;
    mprInitWindow();
    es = MPR->eventService;
    beginEventCount = eventCount = es->eventCount;

    es->now = mprGetTicks();
    expires = timeout < 0 ? MAXINT64 : (es->now + timeout);
    if (expires < 0) {
        expires = MAXINT64;
    }
    justOne = (flags & MPR_SERVICE_ONE_THING) ? 1 : 0;

    /*
        Stop serviceing events when doing final shutdown of the core
        Post-test for mprIsStopping so callers can pump remaining events once stopping has begun
     */
    while (es->now < expires) {
        eventCount = es->eventCount;
        if (MPR->signalService->hasSignals) {
            mprServiceSignals();
        }
        while ((dp = getNextReadyDispatcher(es)) != NULL) {
            assert(!(dp->flags & MPR_DISPATCHER_DESTROYED));
            assert(dp->magic == MPR_DISPATCHER_MAGIC);
            if (!serviceDispatcher(dp)) {
                queueDispatcher(es->pendingQ, dp);
                es->pendingCount++;
                continue;
            }
            if (justOne) {
                MPR->eventing = 0;
                return abs(es->eventCount - beginEventCount);
            }
        } 
        if (es->eventCount == eventCount) {
            lock(es);
            delay = getIdleTicks(es, expires - es->now);
            if (delay > 0) {
                es->waiting = 1;
                es->willAwake = es->now + delay;
                unlock(es);
                if (mprIsStopping()) {
                    delay = 10;
                }
                mprWaitForIO(MPR->waitService, delay);
            } else {
                unlock(es);
            }
        }
        es->now = mprGetTicks();
        if (justOne || mprIsStopping()) {
            break;
        }
    }
    MPR->eventing = 0;
    return abs(es->eventCount - beginEventCount);
}


/*
    Wait for an event to occur and dispatch the event. This is the primary event dispatch routine.
    Return Return 0 if an event was signalled. Return MPR_ERR_TIMEOUT if no event was seen before the timeout.
    WARNING: this will enable GC while sleeping
 */
PUBLIC int mprWaitForEvent(MprDispatcher *dispatcher, MprTicks timeout)
{
    MprEventService     *es;
    MprTicks            expires, delay;
    MprOsThread         thread;
    int                 claimed, signalled, wasRunning, runEvents;

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));

    es = MPR->eventService;
    es->now = mprGetTicks();

    if (dispatcher == NULL) {
        dispatcher = MPR->dispatcher;
    }
    assert(!(dispatcher->flags & MPR_DISPATCHER_WAITING));
    if (dispatcher->flags & MPR_DISPATCHER_WAITING) {
        return MPR_ERR_BUSY;
    }
    thread = mprGetCurrentOsThread();
    expires = timeout < 0 ? (es->now + MPR_MAX_TIMEOUT) : (es->now + timeout);
    claimed = signalled = 0;

    lock(es);
    /*
        Acquire dedicates the dispatcher to this thread. If acquire fails, another thread is servicing this dispatcher.
        makeRunnable() prevents mprServiceEvents from servicing this dispatcher
     */
    wasRunning = isRunning(dispatcher);
    runEvents = (!wasRunning || dispatcher->owner == thread);
    if (runEvents) {
        if (!wasRunning) {
            makeRunnable(dispatcher);
        }
        dispatcher->owner = thread;
    }
    unlock(es);

    while (es->now <= expires && !mprIsStoppingCore()) {
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        if (runEvents) {
            makeRunnable(dispatcher);
            if (dispatchEvents(dispatcher)) {
                signalled++;
                break;
            }
        }
        lock(es);
        delay = getDispatcherIdleTicks(dispatcher, expires - es->now);
        dispatcher->flags |= MPR_DISPATCHER_WAITING;
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        unlock(es);
        
        assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
        mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
        assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);

        if (mprWaitForCond(dispatcher->cond, delay) == 0) {
            assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
            mprResetYield();
            dispatcher->flags &= ~MPR_DISPATCHER_WAITING;
            if (runEvents) {
                makeRunnable(dispatcher);
                dispatchEvents(dispatcher);
            }
            assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
            signalled++;
            break;
        }
        mprResetYield();
        assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
        dispatcher->flags &= ~MPR_DISPATCHER_WAITING;
        es->now = mprGetTicks();
    }
    if (!wasRunning) {
        scheduleDispatcher(dispatcher);
        if (claimed) {
            dispatcher->owner = 0;
        }
    }
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    return signalled ? 0 : MPR_ERR_TIMEOUT;
}


PUBLIC void mprWakeDispatchers()
{
    MprEventService     *es;
    MprDispatcher       *runQ, *dp;

    es = MPR->eventService;
    lock(es);
    runQ = es->runQ;
    for (dp = runQ->next; dp != runQ; dp = dp->next) {
        mprSignalCond(dp->cond);
    }
    unlock(es);
}


PUBLIC int mprDispatchersAreIdle()
{
    MprEventService     *es;
    MprDispatcher       *runQ, *dispatcher;
    int                 idle;

    es = MPR->eventService;
    runQ = es->runQ;
    lock(es);
    dispatcher = runQ->next;
    if (dispatcher == runQ) {
        idle = 1;
    } else {
        idle = (dispatcher->eventQ == dispatcher->eventQ->next);
    }
    unlock(es);
    return idle;
}


/*
    Relay an event to a dispatcher. This invokes the callback proc as though it was invoked from the given dispatcher. 
 */
PUBLIC void mprRelayEvent(MprDispatcher *dispatcher, void *proc, void *data, MprEvent *event)
{
#if BIT_DEBUG
    MprThread   *tp = mprGetCurrentThread();
    mprNop(tp);
#endif
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));

    if (isRunning(dispatcher) && dispatcher->owner != mprGetCurrentOsThread()) {
        mprError("Relay to a running dispatcher owned by another thread");
    }
    if (event) {
        event->timestamp = dispatcher->service->now;
    }
    dispatcher->owner = mprGetCurrentOsThread();
    makeRunnable(dispatcher);
    ((MprEventProc) proc)(data, event);

    /*
        The event may have disabled the dispatcher. Don't reschedule if disabled
     */
    assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);
    if (dispatcher->flags & MPR_DISPATCHER_ENABLED) {
        scheduleDispatcher(dispatcher);
        dispatcher->owner = 0;
    }
}


/*
    Schedule the dispatcher. If the dispatcher is already running then it is not modified. If the event queue is empty, 
    the dispatcher is moved to the idleQ. If there is a past-due event, it is moved to the readyQ. If there is a future 
    event pending, it is put on the waitQ.
 */
PUBLIC void mprScheduleDispatcher(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *event;
    int                 mustWakeWaitService, mustWakeCond;
   
    assert(dispatcher);
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher->name);
    assert(dispatcher->cond);
    es = dispatcher->service;

    lock(es);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));

    if (isRunning(dispatcher) || !(dispatcher->flags & MPR_DISPATCHER_ENABLED)) {
        /* Wake up if waiting in mprWaitForIO */
        mustWakeWaitService = es->waiting;
        mustWakeCond = dispatcher->flags & MPR_DISPATCHER_WAITING;

    } else {
        if (isEmpty(dispatcher)) {
            queueDispatcher(es->idleQ, dispatcher);
            unlock(es);
            return;
        }
        event = dispatcher->eventQ->next;
        assert(event->magic == MPR_EVENT_MAGIC);
        mustWakeWaitService = mustWakeCond = 0;
        if (event->due > es->now) {
            assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
            queueDispatcher(es->waitQ, dispatcher);
            if (event->due < es->willAwake) {
                mustWakeWaitService = 1;
                mustWakeCond = dispatcher->flags & MPR_DISPATCHER_WAITING;
            }
        } else {
            queueDispatcher(es->readyQ, dispatcher);
            mustWakeWaitService = es->waiting;
            mustWakeCond = dispatcher->flags & MPR_DISPATCHER_WAITING;
        }
    }
    unlock(es);
    if (mustWakeCond) {
        mprSignalDispatcher(dispatcher);
    }
    if (mustWakeWaitService) {
        mprWakeNotifier();
    }
}


/*
    Dispatch events for a dispatcher
 */
static int dispatchEvents(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *event;
    int                 count;

    assert(dispatcher->cond);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));

    es = dispatcher->service;
    /*
        OPT - mprGetNextEvent locks anyway, so should be able to get away without a lock here
     */
    mprTrace(7, "dispatchEvents for %s", dispatcher->name);
    lock(es);
    assert(dispatcher->cond);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);
    /*
        Events are removed from the dispatcher queue and put onto the currentQ. This is so they will be marked for GC.
        If the callback calls mprRemoveEvent, it will not remove from the currentQ. If it was a continuous event, 
        mprRemoveEvent will clear the continuous flag.

        OPT - this could all be simpler if dispatchEvents was never called recursively. Then a currentQ would not be needed,
        and neither would a running flag. See mprRemoveEvent().
     */
    for (count = 0; (dispatcher->flags & MPR_DISPATCHER_ENABLED) && (event = mprGetNextEvent(dispatcher)) != 0; count++) {
        assert(event->magic == MPR_EVENT_MAGIC);
        assert(!(event->flags & MPR_EVENT_RUNNING));
        unlock(es);

        mprTrace(7, "Call event %s", event->name);
        assert(event->proc);
        event->flags |= MPR_EVENT_RUNNING;
        (event->proc)(event->data, event);
        event->flags &= ~MPR_EVENT_RUNNING;

        lock(es);
        if (event->flags & MPR_EVENT_CONTINUOUS) {
            /* Reschedule if continuous */
            event->timestamp = dispatcher->service->now;
            event->due = event->timestamp + (event->period ? event->period : 1);
            mprQueueEvent(dispatcher, event);
        } else {
            /* Remove from currentQ - GC can then collect */
            mprDequeueEvent(event);
        }
    }
    unlock(es);
    if (count && es->waiting) {
        es->eventCount += count;
        mprWakeNotifier();
    }
    return count;
}


static bool serviceDispatcher(MprDispatcher *dispatcher)
{
    assert(isRunning(dispatcher));
    assert(dispatcher->owner == 0);
    assert(dispatcher->cond);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    dispatcher->owner = mprGetCurrentOsThread();

    if (dispatcher == MPR->nonBlock) {
        serviceDispatcherMain(dispatcher);

    } else if (dispatcher->requiredWorker) {
        mprActivateWorker(dispatcher->requiredWorker, (MprWorkerProc) serviceDispatcherMain, dispatcher);

    } else if (mprStartWorker((MprWorkerProc) serviceDispatcherMain, dispatcher) < 0) {
        return 0;
    }
    return 1;
}


static void serviceDispatcherMain(MprDispatcher *dispatcher)
{
    MprEventService     *es;

    assert(dispatcher->parent);
    es = dispatcher->service;
    lock(es);
    if (!(dispatcher->flags & MPR_DISPATCHER_ENABLED) || (dispatcher->flags & MPR_DISPATCHER_DESTROYED)) {
        /* Dispatcher may have been disabled after starting the worker */
        unlock(es);
        return;
    }
    unlock(es);
    assert(isRunning(dispatcher));
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(dispatcher->cond);
    assert(dispatcher->name);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher->parent);

    dispatcher->owner = mprGetCurrentOsThread();
    dispatchEvents(dispatcher);
    /*
        The dispatcher may be disabled in an event above
     */
    if (dispatcher->flags & MPR_DISPATCHER_ENABLED) {
        dispatcher->owner = 0;
        scheduleDispatcher(dispatcher);
    }
}


PUBLIC void mprClaimDispatcher(MprDispatcher *dispatcher)
{
    assert(isRunning(dispatcher));
    dispatcher->owner = mprGetCurrentOsThread();
}


PUBLIC void mprWakePendingDispatchers()
{
    mprWakeNotifier();
}


/*
    Get the next (ready) dispatcher off given runQ and move onto the runQ
 */
static MprDispatcher *getNextReadyDispatcher(MprEventService *es)
{
    MprDispatcher   *dp, *next, *pendingQ, *readyQ, *waitQ, *dispatcher;
    MprEvent        *event;

    waitQ = es->waitQ;
    readyQ = es->readyQ;
    pendingQ = es->pendingQ;
    dispatcher = 0;

    lock(es);
    if (pendingQ->next != pendingQ && mprAvailableWorkers() > 0) {
        dispatcher = pendingQ->next;
        dispatcher->service->pendingCount--;
        assert(dispatcher->service->pendingCount >= 0);
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        queueDispatcher(es->runQ, dispatcher);
        assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);
        dispatcher->owner = 0;

    } else if (readyQ->next == readyQ) {
        /*
            ReadyQ is empty, try to transfer a dispatcher with due events onto the readyQ
         */
        for (dp = waitQ->next; dp != waitQ; dp = next) {
            assert(dp->magic == MPR_DISPATCHER_MAGIC);
            assert(!(dp->flags & MPR_DISPATCHER_DESTROYED));
            next = dp->next;
            event = dp->eventQ->next;
            assert(event->magic == MPR_EVENT_MAGIC);
            if (event->due <= es->now && dp->flags & MPR_DISPATCHER_ENABLED) {
                queueDispatcher(es->readyQ, dp);
                break;
            }
        }
    }
    if (!dispatcher && readyQ->next != readyQ) {
        dispatcher = readyQ->next;
        assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
        queueDispatcher(es->runQ, dispatcher);
        assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);
        dispatcher->owner = 0;
    }
    unlock(es);
    assert(dispatcher == NULL || isRunning(dispatcher));
    assert(dispatcher == NULL || dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(dispatcher == NULL || !(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher == NULL || dispatcher->cond);
    return dispatcher;
}


/*
    Get the time to sleep till the next pending event. Must be called locked.
 */
static MprTicks getIdleTicks(MprEventService *es, MprTicks timeout)
{
    MprDispatcher   *readyQ, *waitQ, *dp;
    MprEvent        *event;
    MprTicks        delay;

    waitQ = es->waitQ;
    readyQ = es->readyQ;

    if (readyQ->next != readyQ) {
        delay = 0;
    } else if (mprIsStopping()) {
        delay = 10;
    } else {
        delay = MPR_MAX_TIMEOUT;
        /*
            Examine all the dispatchers on the waitQ
         */
        for (dp = waitQ->next; dp != waitQ; dp = dp->next) {
            assert(dp->magic == MPR_DISPATCHER_MAGIC);
            assert(!(dp->flags & MPR_DISPATCHER_DESTROYED));
            assert(dp->flags & MPR_DISPATCHER_ENABLED);
            event = dp->eventQ->next;
            assert(event->magic == MPR_EVENT_MAGIC);
            if (event != dp->eventQ) {
                delay = min(delay, (event->due - es->now));
                if (delay <= 0) {
                    break;
                }
            }
        }
        delay = min(delay, timeout);
    }
    return delay;
}


static MprTicks getDispatcherIdleTicks(MprDispatcher *dispatcher, MprTicks timeout)
{
    MprEvent    *next;
    MprTicks    delay;

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);

    if (timeout < 0) {
        timeout = 0;
    } else {
        next = dispatcher->eventQ->next;
        delay = MPR_MAX_TIMEOUT;
        if (next != dispatcher->eventQ) {
            delay = (next->due - dispatcher->service->now);
            if (delay < 0) {
                delay = 0;
            }
        }
        timeout = min(delay, timeout);
    }
    return timeout;
}


static void initDispatcher(MprDispatcher *dispatcher)
{
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags == MPR_DISPATCHER_DESTROYED));
           
    dispatcher->next = dispatcher;
    dispatcher->prev = dispatcher;
    dispatcher->parent = dispatcher;
}


static void queueDispatcher(MprDispatcher *prior, MprDispatcher *dispatcher)
{
    assert(dispatcher->service == MPR->eventService);
    lock(dispatcher->service);

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);

    if (dispatcher->parent) {
        dequeueDispatcher(dispatcher);
    }
    dispatcher->parent = prior->parent;
    dispatcher->prev = prior;
    dispatcher->next = prior->next;
    prior->next->prev = dispatcher;
    prior->next = dispatcher;
    assert(dispatcher->cond);
    unlock(dispatcher->service);
}


static void dequeueDispatcher(MprDispatcher *dispatcher)
{
    assert(dispatcher->service == MPR->eventService);
    lock(dispatcher->service);

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
           
    if (dispatcher->next) {
        dispatcher->next->prev = dispatcher->prev;
        dispatcher->prev->next = dispatcher->next;
        dispatcher->next = dispatcher;
        dispatcher->prev = dispatcher;
        dispatcher->parent = dispatcher;
    } else {
        assert(dispatcher->parent == dispatcher);
        assert(dispatcher->next == dispatcher);
        assert(dispatcher->prev == dispatcher);
    }
    assert(dispatcher->cond);
    unlock(dispatcher->service);
}


static void scheduleDispatcher(MprDispatcher *dispatcher)
{
    MprEventService     *es;

    assert(dispatcher->service == MPR->eventService);
    es = dispatcher->service;

    lock(es);
    assert(dispatcher->cond);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    dequeueDispatcher(dispatcher);
    mprScheduleDispatcher(dispatcher);
    unlock(es);
}


static int makeRunnable(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    int                 wasRunning;

    es = dispatcher->service;

    lock(es);
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    wasRunning = isRunning(dispatcher);
    if (!isRunning(dispatcher)) {
        queueDispatcher(es->runQ, dispatcher);
    }
    unlock(es);
    return wasRunning;
}


#if KEEP
static int dqlen(MprDispatcher *dq)
{
    MprDispatcher   *dp;
    int             count;

    count = 0;
    for (dp = dq->next; dp != dq; dp = dp->next) {
        count++;
    }
    return count;
}
#endif


#if KEEP
/*
    Designate the required worker thread to run the event
 */
PUBLIC void mprDedicateWorkerToDispatcher(MprDispatcher *dispatcher, MprWorker *worker)
{
    dispatcher->requiredWorker = worker;
    mprDedicateWorker(worker);
}


PUBLIC void mprReleaseWorkerFromDispatcher(MprDispatcher *dispatcher, MprWorker *worker)
{
    dispatcher->requiredWorker = 0;
    mprReleaseWorker(worker);
}
#endif


PUBLIC void mprSignalDispatcher(MprDispatcher *dispatcher)
{
    if (dispatcher == NULL) {
        dispatcher = MPR->dispatcher;
    }
    mprSignalCond(dispatcher->cond);
}


PUBLIC bool mprDispatcherHasEvents(MprDispatcher *dispatcher)
{
    if (dispatcher == 0) {
        return 0;
    }
    return !isEmpty(dispatcher);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/encode.c"
 */
/************************************************************************/

/*
    encode.c - URI encode and decode routines

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/************************************ Locals **********************************/
/*
    Character escape/descape matching codes. Generated by charGen.
 */
static uchar charMatch[256] = {
    0x00,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3e,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x0c,0x3f,0x28,0x2a,0x3c,0x2b,0x0f,0x0e,0x0e,0x0e,0x28,0x28,0x00,0x00,0x28,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x28,0x2a,0x3f,0x28,0x3f,0x2a,
    0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3a,0x3e,0x3a,0x3e,0x00,
    0x3e,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x3e,0x3e,0x02,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,
    0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c,0x3c 
};

/*  
    Max size of the port specification in a URL
 */
#define MAX_PORT_LEN 8

#define MIME_HASH_SIZE 67

/************************************ Code ************************************/
/*  
    Uri encode by encoding special characters with hex equivalents. Return an allocated string.
 */
PUBLIC char *mprUriEncode(cchar *inbuf, int map)
{
    static cchar    hexTable[] = "0123456789ABCDEF";
    uchar           c;
    cchar           *ip;
    char            *result, *op;
    int             len;

    assert(inbuf);
    assert(inbuf);

    if (!inbuf) {
        return MPR->emptyString;
    }
    for (len = 1, ip = inbuf; *ip; ip++, len++) {
        if (charMatch[(uchar) *ip] & map) {
            len += 2;
        }
    }
    if ((result = mprAlloc(len)) == 0) {
        return 0;
    }
    op = result;

    while ((c = (uchar) (*inbuf++)) != 0) {
        if (c == ' ' && (map & MPR_ENCODE_URI_COMPONENT)) {
            *op++ = '+';
        } else if (charMatch[c] & map) {
            *op++ = '%';
            *op++ = hexTable[c >> 4];
            *op++ = hexTable[c & 0xf];
        } else {
            *op++ = c;
        }
    }
    assert(op < &result[len]);
    *op = '\0';
    return result;
}


/*  
    Decode a string using URL encoding. Return an allocated string.
 */
PUBLIC char *mprUriDecode(cchar *inbuf)
{
    cchar   *ip;
    char    *result, *op;
    int     num, i, c;

    assert(inbuf);

    if ((result = sclone(inbuf)) == 0) {
        return 0;
    }
    for (op = result, ip = inbuf; ip && *ip; ip++, op++) {
        if (*ip == '+') {
            *op = ' ';

        } else if (*ip == '%' && isxdigit((uchar) ip[1]) && isxdigit((uchar) ip[2])) {
            ip++;
            num = 0;
            for (i = 0; i < 2; i++, ip++) {
                c = tolower((uchar) *ip);
                if (c >= 'a' && c <= 'f') {
                    num = (num * 16) + 10 + c - 'a';
                } else if (c >= '0' && c <= '9') {
                    num = (num * 16) + c - '0';
                } else {
                    /* Bad chars in URL */
                    return 0;
                }
            }
            *op = (char) num;
            ip--;

        } else {
            *op = *ip;
        }
    }
    *op = '\0';
    return result;
}


/*  
    Escape a shell command. Not really Http, but useful anyway for CGI
 */
PUBLIC char *mprEscapeCmd(cchar *cmd, int escChar)
{
    uchar   c;
    cchar   *ip;
    char    *result, *op;
    int     len;

    assert(cmd);

    if (!cmd) {
        return MPR->emptyString;
    }
    for (len = 1, ip = cmd; *ip; ip++, len++) {
        if (charMatch[(uchar) *ip] & MPR_ENCODE_SHELL) {
            len++;
        }
    }
    if ((result = mprAlloc(len)) == 0) {
        return 0;
    }

    if (escChar == 0) {
        escChar = '\\';
    }
    op = result;
    while ((c = (uchar) *cmd++) != 0) {
#if BIT_WIN_LIKE
        //  MOB - should use fs->newline
        if ((c == '\r' || c == '\n') && *cmd != '\0') {
            c = ' ';
            continue;
        }
#endif
        if (charMatch[c] & MPR_ENCODE_SHELL) {
            *op++ = escChar;
        }
        *op++ = c;
    }
    assert(op < &result[len]);
    *op = '\0';
    return result;
}


/*  
    Escape HTML to escape defined characters (prevent cross-site scripting)
 */
PUBLIC char *mprEscapeHtml(cchar *html)
{
    cchar   *ip;
    char    *result, *op;
    int     len;

    if (!html) {
        return MPR->emptyString;
    }
    for (len = 1, ip = html; *ip; ip++, len++) {
        if (charMatch[(uchar) *ip] & MPR_ENCODE_HTML) {
            len += 5;
        }
    }
    if ((result = mprAlloc(len)) == 0) {
        return 0;
    }

    /*  
        Leave room for the biggest expansion
     */
    op = result;
    while (*html != '\0') {
        if (charMatch[(uchar) *html] & MPR_ENCODE_HTML) {
            if (*html == '&') {
                strcpy(op, "&amp;");
                op += 5;
            } else if (*html == '<') {
                strcpy(op, "&lt;");
                op += 4;
            } else if (*html == '>') {
                strcpy(op, "&gt;");
                op += 4;
            } else if (*html == '#') {
                strcpy(op, "&#35;");
                op += 5;
            } else if (*html == '(') {
                strcpy(op, "&#40;");
                op += 5;
            } else if (*html == ')') {
                strcpy(op, "&#41;");
                op += 5;
            } else if (*html == '"') {
                strcpy(op, "&quot;");
                op += 6;
            } else if (*html == '\'') {
                strcpy(op, "&#39;");
                op += 5;
            } else {
                assert(0);
            }
            html++;
        } else {
            *op++ = *html++;
        }
    }
    assert(op < &result[len]);
    *op = '\0';
    return result;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/epoll.c"
 */
/************************************************************************/

/**
    epoll.c - Wait for I/O by using epoll on unix like systems.

    This module augments the mprWait wait services module by providing kqueue() based waiting support.
    Also see mprAsyncSelectWait and mprSelectWait. This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if MPR_EVENT_EPOLL
/********************************** Defines ***********************************/

#ifndef BIT_MAX_EPOLL
    #define BIT_MAX_EPOLL  32
#endif

/********************************** Forwards **********************************/

static int growEvents(MprWaitService *ws);
static void serviceIO(MprWaitService *ws, int count);

/************************************ Code ************************************/

PUBLIC int mprCreateNotifierService(MprWaitService *ws)
{
    struct epoll_event  ev;

    ws->eventsMax = BIT_MAX_EPOLL;
    ws->handlerMax = MPR_FD_MIN;
    ws->events = mprAllocZeroed(sizeof(struct epoll_event) * ws->eventsMax);
    ws->handlerMap = mprAllocZeroed(sizeof(MprWaitHandler*) * ws->handlerMax);
    if (ws->events == 0 || ws->handlerMap == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((ws->epoll = epoll_create(BIT_MAX_EPOLL)) < 0) {
        mprError("Call to epoll() failed");
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Initialize the "wakeup" pipe. This is used to wakeup the service thread if other threads need 
     *  to wait for I/O.
     */
    if (pipe(ws->breakPipe) < 0) {
        mprError("Cannot open breakout pipe");
        return MPR_ERR_CANT_INITIALIZE;
    }
    fcntl(ws->breakPipe[0], F_SETFL, fcntl(ws->breakPipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(ws->breakPipe[1], F_SETFL, fcntl(ws->breakPipe[1], F_GETFL) | O_NONBLOCK);

    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN | EPOLLERR | EPOLLHUP;
    ev.data.fd = ws->breakPipe[MPR_READ_PIPE];
    epoll_ctl(ws->epoll, EPOLL_CTL_ADD, ws->breakPipe[MPR_READ_PIPE], &ev);
    return 0;
}


PUBLIC void mprManageEpoll(MprWaitService *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->events);
    
    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->epoll) {
            close(ws->epoll);
            ws->epoll = 0;
        }
        if (ws->breakPipe[0] >= 0) {
            close(ws->breakPipe[0]);
        }
        if (ws->breakPipe[1] >= 0) {
            close(ws->breakPipe[1]);
        }
    }
}


static int growEvents(MprWaitService *ws)
{
    ws->eventsMax *= 2;
    if ((ws->events = mprRealloc(ws->events, sizeof(struct epoll_event) * ws->eventsMax)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


PUBLIC int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask)
{
    struct epoll_event  ev;
    int                 fd, rc;

    assert(wp);
    fd = wp->fd;

    lock(ws);
    if (wp->desiredMask != mask) {
        memset(&ev, 0, sizeof(ev));
        ev.data.fd = fd;
        if (wp->desiredMask & MPR_READABLE) {
            ev.events |= EPOLLIN | EPOLLHUP;
        }
        if (wp->desiredMask & MPR_WRITABLE) {
            ev.events |= EPOLLOUT;
        }
        if (wp->desiredMask == (MPR_READABLE | MPR_WRITABLE)) {
            ev.events |= EPOLLHUP;
        }
        if (ev.events) {
            rc = epoll_ctl(ws->epoll, EPOLL_CTL_DEL, fd, &ev);
#if KEEP
            if (rc != 0) {
                mprError("Epoll del error %d on fd %d\n", errno, fd);
            }
#endif
        }
        ev.events = 0;
        if (mask & MPR_READABLE) {
            ev.events |= (EPOLLIN | EPOLLHUP);
        }
        if (mask & MPR_WRITABLE) {
            ev.events |= EPOLLOUT | EPOLLHUP;
        }
        if (ev.events) {
            rc = epoll_ctl(ws->epoll, EPOLL_CTL_ADD, fd, &ev);
            if (rc != 0) {
                mprError("Epoll add error %d on fd %d\n", errno, fd);
            }
        }
        if (mask && fd >= ws->handlerMax) {
            ws->handlerMax = fd + 32;
            if ((ws->handlerMap = mprRealloc(ws->handlerMap, sizeof(MprWaitHandler*) * ws->handlerMax)) == 0) {
                assert(!MPR_ERR_MEMORY);
                return MPR_ERR_MEMORY;
            }
        }
        assert(ws->handlerMap[fd] == 0 || ws->handlerMap[fd] == wp);
        wp->desiredMask = mask;
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    ws->handlerMap[fd] = (mask) ? wp : 0;
    unlock(ws);
    return 0;
}


/*
    Wait for I/O on a single file descriptor. Return a mask of events found. Mask is the events of interest.
    timeout is in milliseconds.
 */
PUBLIC int mprWaitForSingleIO(int fd, int mask, MprTicks timeout)
{
    struct epoll_event  ev, events[2];
    int                 epfd, rc, result;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
    memset(&ev, 0, sizeof(ev));
    memset(events, 0, sizeof(events));
    ev.data.fd = fd;
    if ((epfd = epoll_create(BIT_MAX_EPOLL)) < 0) {
        mprError("Call to epoll() failed");
        return MPR_ERR_CANT_INITIALIZE;
    }
    ev.events = 0;
    if (mask & MPR_READABLE) {
        ev.events = EPOLLIN | EPOLLHUP;
    }
    if (mask & MPR_WRITABLE) {
        ev.events = EPOLLOUT | EPOLLHUP;
    }
    if (ev.events) {
        epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    }
    result = 0;
    rc = epoll_wait(epfd, events, sizeof(events) / sizeof(struct epoll_event), timeout);
    close(epfd);
    if (rc < 0) {
        mprTrace(2, "Epoll returned %d, errno %d", rc, errno);
    } else if (rc > 0) {
        if (rc > 0) {
            if ((events[0].events & (EPOLLIN | EPOLLERR | EPOLLHUP)) && (mask & MPR_READABLE)) {
                result |= MPR_READABLE;
            }
            if ((events[0].events & (EPOLLOUT | EPOLLHUP)) && (mask & MPR_WRITABLE)) {
                result |= MPR_WRITABLE;
            }
        }
    }
    return result;
}


/*
    Wait for I/O on all registered file descriptors. Timeout is in milliseconds. Return the number of events detected. 
 */
PUBLIC void mprWaitForIO(MprWaitService *ws, MprTicks timeout)
{
    int     rc;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
#if BIT_DEBUG
    if (mprGetDebugMode() && timeout > 30000) {
        timeout = 30000;
    }
#endif
    if (ws->needRecall) {
        mprDoWaitRecall(ws);
        return;
    }
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    rc = epoll_wait(ws->epoll, ws->events, ws->eventsMax, timeout);
    mprResetYield();

    if (rc < 0) {
        if (errno != EINTR) {
            mprTrace(7, "epoll returned %d, errno %d", mprGetOsError());
        }
    } else if (rc > 0) {
        serviceIO(ws, rc);
        if (rc == ws->eventsMax) {
            growEvents(ws);
        }
    }
    ws->wakeRequested = 0;
}


static void serviceIO(MprWaitService *ws, int count)
{
    MprWaitHandler      *wp;
    struct epoll_event  *ev;
    int                 fd, i, mask;

    lock(ws);
    for (i = 0; i < count; i++) {
        ev = &ws->events[i];
        fd = ev->data.fd;
        assert(fd < ws->handlerMax);
        if ((wp = ws->handlerMap[fd]) == 0) {
            char    buf[128];
            if ((ev->events & (EPOLLIN | EPOLLERR | EPOLLHUP)) && (fd == ws->breakPipe[MPR_READ_PIPE])) {
                if (read(fd, buf, sizeof(buf)) < 0) {}
            }
            continue;
        }
        mask = 0;
        if (ev->events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
            mask |= MPR_READABLE;
        }
        if (ev->events & (EPOLLOUT | EPOLLHUP)) {
            mask |= MPR_WRITABLE;
        }
        if (mask == 0) {
            assert(mask);
            continue;
        }
        wp->presentMask = mask & wp->desiredMask;
        if (wp->presentMask) {
            struct epoll_event  ev;
            memset(&ev, 0, sizeof(ev));
            ev.data.fd = fd;
            wp->desiredMask = 0;
            ws->handlerMap[wp->fd] = 0;
            epoll_ctl(ws->epoll, EPOLL_CTL_DEL, wp->fd, &ev);
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    Wake the wait service. WARNING: This routine must not require locking. MprEvents in scheduleDispatcher depends on this.
    Must be async-safe.
 */
PUBLIC void mprWakeNotifier()
{
    MprWaitService  *ws;
    int             c;

    ws = MPR->waitService;
    if (!ws->wakeRequested) {
        ws->wakeRequested = 1;
        c = 0;
        if (write(ws->breakPipe[MPR_WRITE_PIPE], (char*) &c, 1) < 0) {};
    }
}

#endif /* MPR_EVENT_EPOLL */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/event.c"
 */
/************************************************************************/

/*
    event.c - Event and dispatch services

    This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/***************************** Forward Declarations ***************************/

static void initEvent(MprDispatcher *dispatcher, MprEvent *event, cchar *name, MprTicks period, void *proc, 
        void *data, int flgs);
static void initEventQ(MprEvent *q);
static void manageEvent(MprEvent *event, int flags);
static void queueEvent(MprEvent *prior, MprEvent *event);

/************************************* Code ***********************************/
/*
    Create and queue a new event for service. Period is used as the delay before running the event and as the period 
    between events for continuous events.
 */
PUBLIC MprEvent *mprCreateEventQueue()
{
    MprEvent    *queue;

    if ((queue = mprAllocObj(MprEvent, manageEvent)) == 0) {
        return 0;
    }
    initEventQ(queue);
    return queue;
}


/*
    Create and queue a new event for service. Period is used as the delay before running the event and as the period 
    between events for continuous events.
 */
PUBLIC MprEvent *mprCreateEvent(MprDispatcher *dispatcher, cchar *name, MprTicks period, void *proc, void *data, int flags)
{
    MprEvent    *event;

    if ((event = mprAllocObj(MprEvent, manageEvent)) == 0) {
        return 0;
    }
    if (dispatcher == 0) {
        dispatcher = (flags & MPR_EVENT_QUICK) ? MPR->nonBlock : MPR->dispatcher;
    }
    initEvent(dispatcher, event, name, period, proc, data, flags);
    if (!(flags & MPR_EVENT_DONT_QUEUE)) {
        mprQueueEvent(dispatcher, event);
    }
    return event;
}


static void manageEvent(MprEvent *event, int flags)
{
    assert(event->magic == MPR_EVENT_MAGIC);

    if (flags & MPR_MANAGE_MARK) {
        /*
            Events in dispatcher queues are marked by the dispatcher managers, not via event->next,prev
         */
        assert(event->dispatcher == 0 || event->dispatcher->magic == MPR_DISPATCHER_MAGIC);
        mprMark(event->name);
        mprMark(event->dispatcher);
        mprMark(event->handler);
        if (!(event->flags & MPR_EVENT_STATIC_DATA)) {
            mprMark(event->data);
        }

    } else if (flags & MPR_MANAGE_FREE) {
        if (event->next) {
            assert(event->dispatcher == 0 || event->dispatcher->magic == MPR_DISPATCHER_MAGIC);
            mprRemoveEvent(event);
            event->magic = 1;
        }
    }
}


static void initEvent(MprDispatcher *dispatcher, MprEvent *event, cchar *name, MprTicks period, void *proc, void *data, 
    int flags)
{
    assert(dispatcher);
    assert(event);
    assert(proc);
    assert(event->next == 0);
    assert(event->prev == 0);

    dispatcher->service->now = mprGetTicks();
    event->name = sclone(name);
    event->timestamp = dispatcher->service->now;
    event->proc = proc;
    event->period = period;
    event->due = event->timestamp + period;
    event->data = data;
    event->dispatcher = dispatcher;
    event->next = event->prev = 0;
    event->flags = flags;
    event->magic = MPR_EVENT_MAGIC;
}


/*
    Create an interval timer
 */
PUBLIC MprEvent *mprCreateTimerEvent(MprDispatcher *dispatcher, cchar *name, MprTicks period, void *proc, 
    void *data, int flags)
{
    return mprCreateEvent(dispatcher, name, period, proc, data, MPR_EVENT_CONTINUOUS | flags);
}


PUBLIC void mprQueueEvent(MprDispatcher *dispatcher, MprEvent *event)
{
    MprEventService     *es;
    MprEvent            *prior, *q;

    assert(dispatcher);
    assert(event);
    assert(event->timestamp);
#if KEEP
    assert(dispatcher->flags & MPR_DISPATCHER_ENABLED);
#endif
    assert(!(dispatcher->flags & MPR_DISPATCHER_DESTROYED));
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    assert(event->magic == MPR_EVENT_MAGIC);

    es = dispatcher->service;

    lock(es);
    q = dispatcher->eventQ;
    for (prior = q->prev; prior != q; prior = prior->prev) {
        if (event->due > prior->due) {
            break;
        } else if (event->due == prior->due) {
            break;
        }
    }
    assert(prior->next);
    assert(prior->prev);
    
    queueEvent(prior, event);
    event->dispatcher = dispatcher;
    es->eventCount++;
    if (dispatcher->flags & MPR_DISPATCHER_ENABLED) {
        mprScheduleDispatcher(dispatcher);
    }
    unlock(es);
}


PUBLIC void mprRemoveEvent(MprEvent *event)
{
    MprEventService     *es;
    MprDispatcher       *dispatcher;

    dispatcher = event->dispatcher;
    if (dispatcher) {
        es = dispatcher->service;
        lock(es);
        if (event->next && !(event->flags & MPR_EVENT_RUNNING)) {
            mprDequeueEvent(event);
        }
        event->dispatcher = 0;
        event->flags &= ~MPR_EVENT_CONTINUOUS;
        if (dispatcher->flags & MPR_DISPATCHER_ENABLED && 
                event->due == es->willAwake && dispatcher->eventQ->next != dispatcher->eventQ) {
            mprScheduleDispatcher(dispatcher);
        }
        unlock(es);
    }
}


PUBLIC void mprRescheduleEvent(MprEvent *event, MprTicks period)
{
    MprEventService     *es;
    MprDispatcher       *dispatcher;

    assert(event->magic == MPR_EVENT_MAGIC);
    dispatcher = event->dispatcher;
    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);

    es = dispatcher->service;

    lock(es);
    event->period = period;
    event->timestamp = es->now;
    event->due = event->timestamp + period;
    if (event->next) {
        mprRemoveEvent(event);
    }
    unlock(es);
    mprQueueEvent(dispatcher, event);
}


PUBLIC void mprStopContinuousEvent(MprEvent *event)
{
    lock(event->dispatcher->service);
    event->flags &= ~MPR_EVENT_CONTINUOUS;
    unlock(event->dispatcher->service);
}


PUBLIC void mprRestartContinuousEvent(MprEvent *event)
{
    lock(event->dispatcher->service);
    event->flags |= MPR_EVENT_CONTINUOUS;
    unlock(event->dispatcher->service);
    mprRescheduleEvent(event, event->period);
}


PUBLIC void mprEnableContinuousEvent(MprEvent *event, int enable)
{
    lock(event->dispatcher->service);
    event->flags &= ~MPR_EVENT_CONTINUOUS;
    if (enable) {
        event->flags |= MPR_EVENT_CONTINUOUS;
    }
    unlock(event->dispatcher->service);
}


/*
    Get the next due event from the front of the event queue.
 */
PUBLIC MprEvent *mprGetNextEvent(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *event, *next;

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);
    es = dispatcher->service;
    event = 0;
    lock(es);
    next = dispatcher->eventQ->next;
    if (next != dispatcher->eventQ) {
        if (next->due <= es->now) {
            event = next;
            queueEvent(dispatcher->currentQ, event);
            assert(event->magic == MPR_EVENT_MAGIC);
        }
    }
    unlock(es);
    return event;
}


PUBLIC int mprGetEventCount(MprDispatcher *dispatcher)
{
    MprEventService     *es;
    MprEvent            *event;
    int                 count;

    assert(dispatcher->magic == MPR_DISPATCHER_MAGIC);

    es = dispatcher->service;

    lock(es);
	count = 0;
    for (event = dispatcher->eventQ->next; event != dispatcher->eventQ; event = event->next) {
        assert(event->magic == MPR_EVENT_MAGIC);
        count++;
    }
    unlock(es);
    return count;
}


static void initEventQ(MprEvent *q)
{
    assert(q);

    q->next = q;
    q->prev = q;
    q->magic = MPR_EVENT_MAGIC;
}


/*
    Append a new event. Must be locked when called.
 */
static void queueEvent(MprEvent *prior, MprEvent *event)
{
    assert(prior);
    assert(event);
    assert(prior->next);
    assert(event->magic == MPR_EVENT_MAGIC);
    assert(event->dispatcher == 0 || event->dispatcher->magic == MPR_DISPATCHER_MAGIC);

    if (event->next) {
        mprDequeueEvent(event);
    }
    event->prev = prior;
    event->next = prior->next;
    prior->next->prev = event;
    prior->next = event;
}


/*
    Remove an event. Must be locked when called.
 */
PUBLIC void mprDequeueEvent(MprEvent *event)
{
    assert(event);
    assert(event->magic == MPR_EVENT_MAGIC);
    assert(event->dispatcher == 0 || event->dispatcher->magic == MPR_DISPATCHER_MAGIC);

    /* If a continuous event is removed, next may already be null */
    if (event->next) {
        event->next->prev = event->prev;
        event->prev->next = event->next;
        event->next = 0;
        event->prev = 0;
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/file.c"
 */
/************************************************************************/

/**
    file.c - File services.

    This modules provides a simple cross platform file I/O abstraction. It uses the MprFileSystem to provide I/O services.
    This module is not thread safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/****************************** Forward Declarations **************************/

static ssize fillBuf(MprFile *file);
static void manageFile(MprFile *file, int flags);

/************************************ Code ************************************/

PUBLIC MprFile *mprAttachFileFd(int fd, cchar *name, int omode)
{
    MprFileSystem   *fs;
    MprFile         *file;

    fs = mprLookupFileSystem("/");

    if ((file = mprAllocObj(MprFile, manageFile)) != 0) {
        file->fd = fd;
        file->fileSystem = fs;
        file->path = sclone(name);
        file->mode = omode;
        file->attached = 1;
    }
    return file;
}


static void manageFile(MprFile *file, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(file->buf);
        mprMark(file->path);

    } else if (flags & MPR_MANAGE_FREE) {
        if (!file->attached) {
            mprCloseFile(file);
        }
    }
}


PUBLIC int mprFlushFile(MprFile *file)
{
    MprFileSystem   *fs;
    MprBuf          *bp;
    ssize           len, rc;

    assert(file);
    if (file == 0) {
        return MPR_ERR_BAD_HANDLE;
    }
    if (file->buf == 0) {
        return 0;
    }
    if (file->mode & (O_WRONLY | O_RDWR)) {
        fs = file->fileSystem;
        bp = file->buf;
        while (mprGetBufLength(bp) > 0) {
            len = mprGetBufLength(bp);
            rc = fs->writeFile(file, mprGetBufStart(bp), len);
            if (rc < 0) {
                return (int) rc;
            }
            mprAdjustBufStart(bp, rc);
        }
        mprFlushBuf(bp);
    }
    return 0;
}


PUBLIC MprOff mprGetFilePosition(MprFile *file)
{
    return file->pos;
}


PUBLIC MprOff mprGetFileSize(MprFile *file)
{
    return file->size;
}


PUBLIC MprFile *mprGetStderr()
{
    return MPR->stdError;
}


PUBLIC MprFile *mprGetStdin()
{
    return MPR->stdInput;
}


PUBLIC MprFile *mprGetStdout()
{
    return MPR->stdOutput;
}


/*
    Get a character from the file. This will put the file into buffered mode.
 */
PUBLIC int mprGetFileChar(MprFile *file)
{
    MprBuf      *bp;
    ssize     len;

    assert(file);

    if (file == 0) {
        return MPR_ERR;
    }
    if (file->buf == 0) {
        file->buf = mprCreateBuf(BIT_MAX_BUFFER, BIT_MAX_BUFFER);
    }
    bp = file->buf;

    if (mprGetBufLength(bp) == 0) {
        len = fillBuf(file);
        if (len <= 0) {
            return -1;
        }
    }
    if (mprGetBufLength(bp) == 0) {
        return 0;
    }
    file->pos++;
    return mprGetCharFromBuf(bp);
}


static char *findNewline(cchar *str, cchar *newline, ssize len, ssize *nlen)
{
    char    *start, *best;
    ssize   newlines;
    int     i;

    assert(str);
    assert(newline);
    assert(nlen);
    assert(len > 0);

    if (str == NULL || newline == NULL) {
        return NULL;
    }
    newlines = slen(newline);
    assert(newlines == 1 || newlines == 2);

    start = best = NULL;
    *nlen = 0;
    for (i = 0; i < newlines; i++) {
        if ((start = memchr(str, newline[i], len)) != 0) {
            if (best == NULL || start < best) {
                best = start;
                *nlen = 1;
                if (newlines == 2 && best[1] == newline[!i]) {
                    (*nlen)++;
                }
            }
        }
    }
    return best;
}


/*
    Read a line from the file. This will put the file into buffered mode.
    Return NULL on eof.
 */
PUBLIC char *mprReadLine(MprFile *file, ssize maxline, ssize *lenp)
{
    MprBuf          *bp;
    MprFileSystem   *fs;
    ssize           size, len, nlen, consumed;
    cchar           *eol, *newline, *start;
    char            *result;

    assert(file);

    if (file == 0) {
        return NULL;
    }
    if (lenp) {
        *lenp = 0;
    }
    if (maxline <= 0) {
        maxline = BIT_MAX_BUFFER;
    }
    fs = file->fileSystem;
    newline = fs->newline;
    if (file->buf == 0) {
        file->buf = mprCreateBuf(maxline, maxline);
    }
    bp = file->buf;

    result = NULL;
    size = 0;
    do {
        if (mprGetBufLength(bp) == 0) {
            if (fillBuf(file) <= 0) {
                return result;
            }
        }
        start = mprGetBufStart(bp);
        len = mprGetBufLength(bp);
        if ((eol = findNewline(start, newline, len, &nlen)) != 0) {
            len = eol - start;
            consumed = len + nlen;
        } else {
            consumed = len;
        }
        file->pos += (MprOff) consumed;
        if (lenp) {
            *lenp += len;
        }
        if ((result = mprRealloc(result, size + len + 1)) == 0) {
            return NULL;
        }
        memcpy(&result[size], start, len);
        size += len;
        result[size] = '\0';
        mprAdjustBufStart(bp, consumed);
    } while (!eol);

    return result;
}


PUBLIC MprFile *mprOpenFile(cchar *path, int omode, int perms)
{
    MprFileSystem   *fs;
    MprFile         *file;
    MprPath         info;

    fs = mprLookupFileSystem(path);

    file = fs->openFile(fs, path, omode, perms);
    if (file) {
        file->fileSystem = fs;
        file->path = sclone(path);
        if (omode & (O_WRONLY | O_RDWR)) {
            /*
                OPT. Should compute this lazily.
             */
            fs->getPathInfo(fs, path, &info);
            file->size = (MprOff) info.size;
        }
        file->mode = omode;
        file->perms = perms;
    }
    return file;
}


PUBLIC int mprCloseFile(MprFile *file)
{
    MprFileSystem   *fs;

    if (file == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    fs = mprLookupFileSystem(file->path);
    return fs->closeFile(file);
}


/*
    Put a string to the file. This will put the file into buffered mode.
 */
PUBLIC ssize mprPutFileString(MprFile *file, cchar *str)
{
    MprBuf  *bp;
    ssize   total, bytes, count;
    char    *buf;

    assert(file);
    count = slen(str);

    /*
        Buffer output and flush when full.
     */
    if (file->buf == 0) {
        file->buf = mprCreateBuf(BIT_MAX_BUFFER, 0);
        if (file->buf == 0) {
            return MPR_ERR_CANT_ALLOCATE;
        }
    }
    bp = file->buf;

    if (mprGetBufLength(bp) > 0 && mprGetBufSpace(bp) < count) {
        mprFlushFile(file);
    }
    total = 0;
    buf = (char*) str;

    while (count > 0) {
        bytes = mprPutBlockToBuf(bp, buf, count);
        if (bytes < 0) {
            return MPR_ERR_CANT_ALLOCATE;

        } else if (bytes == 0) {
            if (mprFlushFile(file) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            continue;
        }
        count -= bytes;
        buf += bytes;
        total += bytes;
        file->pos += (MprOff) bytes;
    }
    return total;
}


/*
    Peek at a character from the file without disturbing the read position. This will put the file into buffered mode.
 */
PUBLIC int mprPeekFileChar(MprFile *file)
{
    MprBuf      *bp;
    ssize       len;

    assert(file);

    if (file == 0) {
        return MPR_ERR;
    }
    if (file->buf == 0) {
        file->buf = mprCreateBuf(BIT_MAX_BUFFER, BIT_MAX_BUFFER);
    }
    bp = file->buf;

    if (mprGetBufLength(bp) == 0) {
        len = fillBuf(file);
        if (len <= 0) {
            return -1;
        }
    }
    if (mprGetBufLength(bp) == 0) {
        return 0;
    }
    return ((uchar*) mprGetBufStart(bp))[0];
}


/*
    Put a character to the file. This will put the file into buffered mode.
 */
PUBLIC ssize mprPutFileChar(MprFile *file, int c)
{
    assert(file);

    if (file == 0) {
        return -1;
    }
    if (file->buf) {
        if (mprPutCharToBuf(file->buf, c) != 1) {
            return MPR_ERR_CANT_WRITE;
        }
        file->pos++;
        return 1;

    }
    return mprWriteFile(file, &c, 1);
}


PUBLIC ssize mprReadFile(MprFile *file, void *buf, ssize size)
{
    MprFileSystem   *fs;
    MprBuf          *bp;
    ssize           bytes, totalRead;
    void            *bufStart;

    assert(file);
    if (file == 0) {
        return MPR_ERR_BAD_HANDLE;
    }
    fs = file->fileSystem;
    bp = file->buf;
    if (bp == 0) {
        totalRead = fs->readFile(file, buf, size);

    } else {
        bufStart = buf;
        while (size > 0) {
            if (mprGetBufLength(bp) == 0) {
                bytes = fillBuf(file);
                if (bytes <= 0) {
                    return -1;
                }
            }
            bytes = min(size, mprGetBufLength(bp));
            memcpy(buf, mprGetBufStart(bp), bytes);
            mprAdjustBufStart(bp, bytes);
            buf = (void*) (((char*) buf) + bytes);
            size -= bytes;
        }
        totalRead = ((char*) buf - (char*) bufStart);
    }
    file->pos += (MprOff) totalRead;
    return totalRead;
}


PUBLIC MprOff mprSeekFile(MprFile *file, int seekType, MprOff pos)
{
    MprFileSystem   *fs;

    assert(file);
    fs = file->fileSystem;

    if (file->buf) {
        if (! (seekType == SEEK_CUR && pos == 0)) {
            /*
                Discard buffering as we may be seeking outside the buffer.
                OPT. Could be smarter about this and preserve the buffer.
             */
            if (file->mode & (O_WRONLY | O_RDWR)) {
                if (mprFlushFile(file) < 0) {
                    return MPR_ERR_CANT_WRITE;
                }
            }
            if (file->buf) {
                mprFlushBuf(file->buf);
            }
        }
    }
    if (seekType == SEEK_SET) {
        file->pos = pos;
    } else if (seekType == SEEK_CUR) {
        file->pos += pos;
    } else {
        file->pos = fs->seekFile(file, SEEK_END, 0);
    }
    if (fs->seekFile(file, SEEK_SET, (long) file->pos) != (long) file->pos) {
        return MPR_ERR;
    }
    if (file->mode & (O_WRONLY | O_RDWR)) {
        if (file->pos > file->size) {
            file->size = file->pos;
        }
    }
    return file->pos;
}


PUBLIC int mprTruncateFile(cchar *path, MprOff size)
{
    MprFileSystem   *fs;

    assert(path && *path);

    if ((fs = mprLookupFileSystem(path)) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    return fs->truncateFile(fs, path, size);
}


PUBLIC ssize mprWriteFile(MprFile *file, cvoid *buf, ssize count)
{
    MprFileSystem   *fs;
    MprBuf          *bp;
    ssize           bytes, written;

    assert(file);
    if (file == 0) {
        return MPR_ERR_BAD_HANDLE;
    }

    fs = file->fileSystem;
    bp = file->buf;
    if (bp == 0) {
        if ((written = fs->writeFile(file, buf, count)) < 0) {
            return written;
        }
    } else {
        written = 0;
        while (count > 0) {
            bytes = mprPutBlockToBuf(bp, buf, count);
            if (bytes < 0) {
                return bytes;
            } 
            if (bytes != count) {
                mprFlushFile(file);
            }
            count -= bytes;
            written += bytes;
            buf = (char*) buf + bytes;
        }
    }
    file->pos += (MprOff) written;
    if (file->pos > file->size) {
        file->size = file->pos;
    }
    return written;
}


PUBLIC ssize mprWriteFileString(MprFile *file, cchar *str)
{
    return mprWriteFile(file, str, slen(str));
}


PUBLIC ssize mprWriteFileFmt(MprFile *file, cchar *fmt, ...)
{
    va_list     ap;
    char        *buf;
    ssize       rc;

    rc = -1;
    va_start(ap, fmt);
    if ((buf = sfmtv(fmt, ap)) != NULL) {
        rc = mprWriteFileString(file, buf);
    }
    va_end(ap);
    return rc;
}


/*
    Fill the read buffer. Return the new buffer length. Only called when the buffer is empty.
 */
static ssize fillBuf(MprFile *file)
{
    MprFileSystem   *fs;
    MprBuf          *bp;
    ssize           len;

    bp = file->buf;
    fs = file->fileSystem;

    assert(mprGetBufLength(bp) == 0);
    mprFlushBuf(bp);

    len = fs->readFile(file, mprGetBufStart(bp), mprGetBufSpace(bp));
    if (len <= 0) {
        return len;
    }
    mprAdjustBufEnd(bp, len);
    return len;
}


/*
    Enable and control file buffering
 */
PUBLIC int mprEnableFileBuffering(MprFile *file, ssize initialSize, ssize maxSize)
{
    assert(file);

    if (file == 0) {
        return MPR_ERR_BAD_STATE;
    }
    if (initialSize <= 0) {
        initialSize = BIT_MAX_BUFFER;
    }
    if (maxSize <= 0) {
        maxSize = BIT_MAX_BUFFER;
    }
    if (maxSize <= initialSize) {
        maxSize = initialSize;
    }
    if (file->buf == 0) {
        file->buf = mprCreateBuf(initialSize, maxSize);
    }
    return 0;
}


PUBLIC void mprDisableFileBuffering(MprFile *file)
{
    mprFlushFile(file);
    file->buf = 0;
}


PUBLIC int mprGetFileFd(MprFile *file)
{
    return file->fd;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/fs.c"
 */
/************************************************************************/

/**
    fs.c - File system services.

    This module provides a simple cross platform file system abstraction. File systems provide a file system switch and 
    underneath a file system provider that implements actual I/O.
    This module is not thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/************************************ Code ************************************/

PUBLIC MprFileSystem *mprCreateFileSystem(cchar *path)
{
    MprFileSystem   *fs;
    char            *cp;

    /*
        FUTURE: evolve this to support multiple file systems in a single system
     */
#if BIT_ROM
    fs = (MprFileSystem*) mprCreateRomFileSystem(path);
#else
    fs = (MprFileSystem*) mprCreateDiskFileSystem(path);
#endif

#if BIT_WIN_LIKE
    fs->separators = sclone("\\/");
    fs->newline = sclone("\r\n");
#elif CYGWIN
    fs->separators = sclone("/\\");
    fs->newline = sclone("\n");
#else
    fs->separators = sclone("/");
    fs->newline = sclone("\n");
#endif

#if BIT_WIN_LIKE || MACOSX || CYGWIN
    fs->caseSensitive = 0;
#else
    fs->caseSensitive = 1;
#endif

#if BIT_WIN_LIKE || VXWORKS || CYGWIN
    fs->hasDriveSpecs = 1;
#endif

    if (MPR->fileSystem == NULL) {
        MPR->fileSystem = fs;
    }
    fs->root = mprGetAbsPath(path);
    if ((cp = strpbrk(fs->root, fs->separators)) != 0) {
        *++cp = '\0';
    }
#if BIT_WIN_LIKE || CYGWIN
    fs->cygwin = mprReadRegistry("HKEY_LOCAL_MACHINE\\SOFTWARE\\Cygwin\\setup", "rootdir");
    fs->cygdrive = sclone("/cygdrive");
#endif
    return fs;
}


PUBLIC void mprAddFileSystem(MprFileSystem *fs)
{
    assert(fs);
    
    /* NOTE: this does not currently add a file system. It merely replaces the existing file system. */
    MPR->fileSystem = fs;
}


/*
    Note: path can be null
 */
PUBLIC MprFileSystem *mprLookupFileSystem(cchar *path)
{
    return MPR->fileSystem;
}


PUBLIC cchar *mprGetPathNewline(cchar *path)
{
    MprFileSystem   *fs;

    assert(path);

    fs = mprLookupFileSystem(path);
    return fs->newline;
}


PUBLIC cchar *mprGetPathSeparators(cchar *path)
{
    MprFileSystem   *fs;

    assert(path);

    fs = mprLookupFileSystem(path);
    return fs->separators;
}


PUBLIC void mprSetPathSeparators(cchar *path, cchar *separators)
{
    MprFileSystem   *fs;

    assert(path);
    assert(separators);
    
    fs = mprLookupFileSystem(path);
    fs->separators = sclone(separators);
}


PUBLIC void mprSetPathNewline(cchar *path, cchar *newline)
{
    MprFileSystem   *fs;
    
    assert(path);
    assert(newline);
    
    fs = mprLookupFileSystem(path);
    fs->newline = sclone(newline);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/hash.c"
 */
/************************************************************************/

/*
    hash.c - Fast hashing hash lookup module

    This hash hash uses a fast key lookup mechanism. Keys may be C strings or unicode strings. The hash value entries 
    are arbitrary pointers. The keys are hashed into a series of buckets which then have a chain of hash entries.
    The chain in in collating sequence so search time through the chain is on average (N/hashSize)/2.

    This module is not thread-safe. It is the callers responsibility to perform all thread synchronization.
    There is locking solely for the purpose of synchronization with the GC marker()

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Defines ***********************************/

#ifndef BIT_MAX_HASH
    #define BIT_MAX_HASH 23           /* Default initial hash size */
#endif

/********************************** Forwards **********************************/

static void *dupKey(MprHash *hash, cvoid *key);
static MprKey *lookupHash(int *index, MprKey **prevSp, MprHash *hash, cvoid *key);
static void manageHashTable(MprHash *hash, int flags);

/*********************************** Code *************************************/
/*
    Create a new hash hash of a given size. Caller should provide a size that is a prime number for the greatest efficiency.
    Can use hashSize -1, 0 to get a default hash.
 */
PUBLIC MprHash *mprCreateHash(int hashSize, int flags)
{
    MprHash     *hash;

    if ((hash = mprAllocObj(MprHash, manageHashTable)) == 0) {
        return 0;
    }
    if (hashSize < BIT_MAX_HASH) {
        hashSize = BIT_MAX_HASH;
    }
    if ((hash->buckets = mprAllocZeroed(sizeof(MprKey*) * hashSize)) == 0) {
        return NULL;
    }
    hash->size = hashSize;
    hash->flags = flags | MPR_OBJ_HASH;
    hash->length = 0;
    if (!(flags & MPR_HASH_OWN)) {
        hash->mutex = mprCreateLock();
    }
#if BIT_CHAR_LEN > 1 && KEEP
    if (hash->flags & MPR_HASH_UNICODE) {
        if (hash->flags & MPR_HASH_CASELESS) {
            hash->fn = (MprHashProc) whashlower;
        } else {
            hash->fn = (MprHashProc) whash;
        }
    } else 
#endif
    {
        if (hash->flags & MPR_HASH_CASELESS) {
            hash->fn = (MprHashProc) shashlower;
        } else {
            hash->fn = (MprHashProc) shash;
        }
    }
    return hash;
}


static void manageHashTable(MprHash *hash, int flags)
{
    MprKey      *sp;
    int         i;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(hash->mutex);
        mprMark(hash->buckets);
        lock(hash);
        for (i = 0; i < hash->size; i++) {
            for (sp = (MprKey*) hash->buckets[i]; sp; sp = sp->next) {
                assert(mprIsValid(sp));
                mprMark(sp);
                if (!(hash->flags & MPR_HASH_STATIC_VALUES)) {
                    if (sp->data && !mprIsValid(sp->data)) {
                        mprLog(0, "Data in key %s is not valid", sp->key);
                    }
                    assert(sp->data == 0 || mprIsValid(sp->data));
                    mprMark(sp->data);
                }
                if (!(hash->flags & MPR_HASH_STATIC_KEYS)) {
                    assert(mprIsValid(sp->key));
                    mprMark(sp->key);
                }
            }
        }
        unlock(hash);
    }
}


/*
    Insert an entry into the hash hash. If the entry already exists, update its value. 
    Order of insertion is not preserved.
 */
PUBLIC MprKey *mprAddKey(MprHash *hash, cvoid *key, cvoid *ptr)
{
    MprKey      *sp, *prevSp;
    int         index;

    if (hash == 0) {
        assert(hash);
        return 0;
    }
    lock(hash);
    if ((sp = lookupHash(&index, &prevSp, hash, key)) != 0) {
        if (hash->flags & MPR_HASH_UNIQUE) {
            unlock(hash);
            return 0;
        }
        /*
            Already exists. Just update the data.
         */
        sp->data = ptr;
        unlock(hash);
        return sp;
    }
    /*
        Hash entries are managed by manageHashTable
     */
    if ((sp = mprAllocStruct(MprKey)) == 0) {
        unlock(hash);
        return 0;
    }
    sp->data = ptr;
    if (!(hash->flags & MPR_HASH_STATIC_KEYS)) {
        sp->key = dupKey(hash, key);
    } else {
        sp->key = (void*) key;
    }
    sp->bucket = index;
    sp->next = hash->buckets[index];
    hash->buckets[index] = sp;
    hash->length++;
    unlock(hash);
    return sp;
}


PUBLIC MprKey *mprAddKeyFmt(MprHash *hash, cvoid *key, cchar *fmt, ...)
{
    va_list     ap;
    char        *value;

    va_start(ap, fmt);
    value = sfmtv(fmt, ap);
    va_end(ap);
    return mprAddKey(hash, key, value);
}


/*
    Multiple insertion. Insert an entry into the hash hash allowing for multiple entries with the same key.
    Order of insertion is not preserved. Lookup cannot be used to retrieve all duplicate keys, some will be shadowed. 
    Use enumeration to retrieve the keys.
 */
PUBLIC MprKey *mprAddDuplicateKey(MprHash *hash, cvoid *key, cvoid *ptr)
{
    MprKey      *sp;
    int         index;

    assert(hash);
    assert(key);

    if ((sp = mprAllocStruct(MprKey)) == 0) {
        return 0;
    }
    sp->data = ptr;
    if (!(hash->flags & MPR_HASH_STATIC_KEYS)) {
        sp->key = dupKey(hash, key);
    } else {
        sp->key = (void*) key;
    }
    lock(hash);
    index = hash->fn(key, slen(key)) % hash->size;
    sp->bucket = index;
    sp->next = hash->buckets[index];
    hash->buckets[index] = sp;
    hash->length++;
    unlock(hash);
    return sp;
}


PUBLIC int mprRemoveKey(MprHash *hash, cvoid *key)
{
    MprKey      *sp, *prevSp;
    int         index;

    assert(hash);
    assert(key);

    lock(hash);
    if ((sp = lookupHash(&index, &prevSp, hash, key)) == 0) {
        unlock(hash);
        return MPR_ERR_CANT_FIND;
    }
    if (prevSp) {
        prevSp->next = sp->next;
    } else {
        hash->buckets[index] = sp->next;
    }
    hash->length--;
    unlock(hash);
    return 0;
}


PUBLIC MprHash *mprBlendHash(MprHash *hash, MprHash *extra)
{
    MprKey      *kp;

    if (hash == 0 || extra == 0) {
        return hash;
    }
    for (ITERATE_KEYS(extra, kp)) {
        mprAddKey(hash, kp->key, kp->data);
    }
    return hash;
}


PUBLIC MprHash *mprCloneHash(MprHash *master)
{
    MprKey      *kp;
    MprHash     *hash;

    if ((hash = mprCreateHash(master->size, master->flags)) == 0) {
        return 0;
    }
    kp = mprGetFirstKey(master);
    while (kp) {
        mprAddKey(hash, kp->key, kp->data);
        kp = mprGetNextKey(master, kp);
    }
    return hash;
}


/*
    Lookup a key and return the hash entry
 */
PUBLIC MprKey *mprLookupKeyEntry(MprHash *hash, cvoid *key)
{
    return lookupHash(0, 0, hash, key);
}


/*
    Lookup a key and return the hash entry data
 */
PUBLIC void *mprLookupKey(MprHash *hash, cvoid *key)
{
    MprKey      *sp;

    if ((sp = lookupHash(0, 0, hash, key)) == 0) {
        return 0;
    }
    return (void*) sp->data;
}


/*
    Exponential primes
 */
static int hashSizes[] = {
     19, 29, 59, 79, 97, 193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317, 196613, 0
};


static int getHashSize(int numKeys)
{
    int     i;

    for (i = 0; hashSizes[i]; i++) {
        if (numKeys < hashSizes[i]) {
            return hashSizes[i];
        }
    }
    return hashSizes[i - 1];
}


/*
    This is unlocked because it is read-only
 */
static MprKey *lookupHash(int *bucketIndex, MprKey **prevSp, MprHash *hash, cvoid *key)
{
    MprKey      *sp, *prev, *next;
    MprKey      **buckets;
    int         hashSize, i, index, rc;

    if (key == 0 || hash == 0) {
        return 0;
    }
    if (hash->length > hash->size) {
        hashSize = getHashSize(hash->length * 4 / 3);
        if (hash->size < hashSize) {
            if ((buckets = mprAllocZeroed(sizeof(MprKey*) * hashSize)) != 0) {
                hash->length = 0;
                for (i = 0; i < hash->size; i++) {
                    for (sp = hash->buckets[i]; sp; sp = next) {
                        next = sp->next;
                        assert(next != sp);
                        index = hash->fn(sp->key, slen(sp->key)) % hashSize;
                        if (buckets[index]) {
                            sp->next = buckets[index];
                        } else {
                            sp->next = 0;
                        }
                        buckets[index] = sp;
                        sp->bucket = index;
                        hash->length++;
                    }
                }
                hash->size = hashSize;
                hash->buckets = buckets;
            }
        }
    }
    index = hash->fn(key, slen(key)) % hash->size;
    if (bucketIndex) {
        *bucketIndex = index;
    }
    sp = hash->buckets[index];
    prev = 0;

    while (sp) {
#if BIT_CHAR_LEN > 1 && KEEP
        if (hash->flags & MPR_HASH_UNICODE) {
            wchar *u1, *u2;
            u1 = (wchar*) sp->key;
            u2 = (wchar*) key;
            rc = -1;
            if (hash->flags & MPR_HASH_CASELESS) {
                rc = wcasecmp(u1, u2);
            } else {
                rc = wcmp(u1, u2);
            }
        } else 
#endif
        if (hash->flags & MPR_HASH_CASELESS) {
            rc = scaselesscmp(sp->key, key);
        } else {
            rc = strcmp(sp->key, key);
        }
        if (rc == 0) {
            if (prevSp) {
                *prevSp = prev;
            }
            return sp;
        }
        prev = sp;
        assert(sp != sp->next);
        sp = sp->next;
    }
    return 0;
}


PUBLIC int mprGetHashLength(MprHash *hash)
{
    return hash->length;
}


/*
    Return the first entry in the hash.
 */
PUBLIC MprKey *mprGetFirstKey(MprHash *hash)
{
    MprKey      *sp;
    int         i;

    assert(hash);

    for (i = 0; i < hash->size; i++) {
        if ((sp = (MprKey*) hash->buckets[i]) != 0) {
            return sp;
        }
    }
    return 0;
}


/*
    Return the next entry in the hash
 */
PUBLIC MprKey *mprGetNextKey(MprHash *hash, MprKey *last)
{
    MprKey      *sp;
    int         i;

    if (hash == 0) {
        return 0;
    }
    if (last == 0) {
        return mprGetFirstKey(hash);
    }
    if (last->next) {
        return last->next;
    }
    for (i = last->bucket + 1; i < hash->size; i++) {
        if ((sp = (MprKey*) hash->buckets[i]) != 0) {
            return sp;
        }
    }
    return 0;
}


static void *dupKey(MprHash *hash, cvoid *key)
{
#if BIT_CHAR_LEN > 1 && KEEP
    if (hash->flags & MPR_HASH_UNICODE) {
        return wclone((wchar*) key);
    } else
#endif
        return sclone(key);
}


PUBLIC MprHash *mprCreateHashFromWords(cchar *str)
{
    MprHash     *hash;
    char        *word, *next;

    hash = mprCreateHash(0, 0);
    word = stok(sclone(str), ", \t\n\r", &next);
    while (word) {
        mprAddKey(hash, word, word);
        word = stok(NULL, " \t\n\r", &next);
    }
    return hash;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/json.c"
 */
/************************************************************************/

/**
    json.c - A JSON parser and serializer. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************** Includes **********************************/



/****************************** Forward Declarations **************************/

static MprObj *deserialize(MprJson *jp);
static char advanceToken(MprJson *jp);
static cchar *findEndKeyword(MprJson *jp, cchar *str);
static cchar *findQuote(cchar *tok, int quote);
static MprObj *makeObj(MprJson *jp, bool list);
static cchar *parseComment(MprJson *jp);
static void jsonParseError(MprJson *jp, cchar *msg);
static cchar *parseName(MprJson *jp);
static cchar *parseValue(MprJson *jp);
static int setValue(MprJson *jp, MprObj *obj, int index, cchar *name, cchar *value, int type);

/************************************ Code ************************************/

PUBLIC MprObj *mprDeserializeCustom(cchar *str, MprJsonCallback callback, void *data)
{
    MprJson     jp;

    /*
        There is no need for GC management as this routine does not yield
     */
    memset(&jp, 0, sizeof(jp));
    jp.lineNumber = 1;
    jp.tok = str;
    jp.callback = callback;
    jp.data = data;
    return deserialize(&jp);
}


/*
    Deserialize a JSON string into an MprHash object. Objects and lists "[]" are stored in hashes. 
 */
PUBLIC MprObj *mprDeserialize(cchar *str)
{
    MprJsonCallback cb;

    cb.checkState = 0;
    cb.makeObj = makeObj;
    cb.parseError = jsonParseError;
    cb.setValue = setValue;
    return mprDeserializeCustom(str, cb, 0); 
}


static MprObj *deserialize(MprJson *jp)
{
    cvoid   *value;
    MprObj  *obj;
    cchar   *name;
    int     token, rc, index, valueType;

    if ((token = advanceToken(jp)) == '[') {
        obj = jp->callback.makeObj(jp, 1);
        index = 0;
    } else if (token == '{') {
        obj = jp->callback.makeObj(jp, 0);
        index = -1;
    } else {
        return (MprObj*) parseValue(jp);
    }
    jp->tok++;

    while (*jp->tok) {
        switch (advanceToken(jp)) {
        case '\0':
            break;

        case ',':
            if (index >= 0) {
                index++;
            }
            jp->tok++;
            continue;

        case '/':
            if (jp->tok[1] == '/' || jp->tok[1] == '*') {
                jp->tok = parseComment(jp);
            } else {
                mprJsonParseError(jp, "Unexpected character '%c'", *jp->tok);
                return 0;
            }
            continue;

        case '}':
        case ']':
            /* End of object or array */
            if (jp->callback.checkState && jp->callback.checkState(jp, NULL) < 0) {
                return 0;
            }
            jp->tok++;
            return obj;
            
        default:
            /*
                Value: String, "{" or "]"
             */
            value = 0;
            valueType = MPR_JSON_STRING;
            if (index < 0) {
                if ((name = parseName(jp)) == 0) {
                    return 0;
                }
                if ((token = advanceToken(jp)) != ':') {
                    if (token == ',' || token == '}' || token == ']') {
                        valueType = MPR_JSON_STRING;
                        value = name;
                    } else {
                        mprJsonParseError(jp, "Bad separator '%c'", *jp->tok);
                        return 0;
                    }
                }
                jp->tok++;
            } else {
                name = 0;
            }
            if (!value) {
                advanceToken(jp);
                if (jp->callback.checkState && jp->callback.checkState(jp, name) < 0) {
                    return 0;
                }
                if (*jp->tok == '{') {
                    value = deserialize(jp);
                    valueType = MPR_JSON_OBJ;

                } else if (*jp->tok == '[') {
                    value = deserialize(jp);
                    valueType = MPR_JSON_ARRAY;

                } else {
                    value = parseValue(jp);
                    valueType = MPR_JSON_STRING;
                }
                if (!value) {
                    /* Error already reported */
                    return 0;
                }
            }
            if ((rc = jp->callback.setValue(jp, obj, index, name, value, valueType)) < 0) {
                return 0;
            }
        }
    }
    return obj;
}


static cchar *parseComment(MprJson *jp)
{
    cchar   *tok;

    tok = jp->tok;
    if (*tok == '/') {
        for (tok++; *tok && *tok != '\n'; tok++) ;

    } else if (*jp->tok == '*') {
        tok++;
        for (tok++; tok[0] && (tok[0] != '*' || tok[1] != '/'); tok++) {
            if (*tok == '\n') {
                jp->lineNumber++;
            }
        }
    }
    return tok - 1;
}


static cchar *parseQuotedName(MprJson *jp)
{
    cchar    *etok, *name;
    int      quote;

    quote = *jp->tok;
    if ((etok = findQuote(++jp->tok, quote)) == 0) {
        mprJsonParseError(jp, "Missing closing quote");
        return 0;
    }
    name = snclone(jp->tok, etok - jp->tok);
    jp->tok = ++etok;
    return name;
}


static cchar *parseUnquotedName(MprJson *jp)
{
    cchar    *etok, *name;

    etok = findEndKeyword(jp, jp->tok);
    name = snclone(jp->tok, etok - jp->tok);
    jp->tok = etok;
    return name;
}


static cchar *parseName(MprJson *jp)
{
    char    token;

    token = advanceToken(jp);
    if (token == '"' || token == '\'') {
        return parseQuotedName(jp);
    } else {
        return parseUnquotedName(jp);
    }
}


static cchar *parseValue(MprJson *jp)
{
    cchar   *etok, *value;
    int     quote;

    value = 0;
    if (*jp->tok == '"' || *jp->tok == '\'') {
        quote = *jp->tok;
        if ((etok = findQuote(++jp->tok, quote)) == 0) {
            mprJsonParseError(jp, "Missing closing quote");
            return 0;
        }
        value = snclone(jp->tok, etok - jp->tok);
        jp->tok = etok + 1;

    } else {
        etok = findEndKeyword(jp, jp->tok);
        value = snclone(jp->tok, etok - jp->tok);
        jp->tok = etok;
    }
    return value;
}


static int setValue(MprJson *jp, MprObj *obj, int index, cchar *key, cchar *value, int type)
{
    MprKey  *kp;
    char    keybuf[32];

    if (index >= 0) {
        itosbuf(keybuf, sizeof(keybuf), index, 10);
        key = keybuf;
    }
    if ((kp = mprAddKey(obj, key, value)) == 0) {
        return MPR_ERR_MEMORY;
    }
    kp->type = type;
    return 0;
}


static MprObj *makeObj(MprJson *jp, bool list)
{
    MprHash     *hash;

    if ((hash = mprCreateHash(0, 0)) == 0) {
        return 0;
    }
    if (list) {
        hash->flags |= MPR_HASH_LIST;
    }
    return hash;
}


static void quoteValue(MprBuf *buf, cchar *str)
{
    cchar   *cp;

    mprPutCharToBuf(buf, '\'');
    for (cp = str; *cp; cp++) {
        if (*cp == '\'') {
            mprPutCharToBuf(buf, '\\');
        }
        mprPutCharToBuf(buf, *cp);
    }
    mprPutCharToBuf(buf, '\'');
}


/*
    Supports hashes where properties are strings or hashes of strings. N-level nest is supported.
 */
static cchar *objToString(MprBuf *buf, MprObj *obj, int type, int pretty)
{
    MprKey  *kp;
    char    numbuf[32];
    int     i, len;

    if (type == MPR_JSON_ARRAY) {
        mprPutCharToBuf(buf, '[');
        if (pretty) mprPutCharToBuf(buf, '\n');
        len = mprGetHashLength(obj);
        for (i = 0; i < len; i++) {
            itosbuf(numbuf, sizeof(numbuf), i, 10);
            if (pretty) mprPutStringToBuf(buf, "    ");
            if ((kp = mprLookupKeyEntry(obj, numbuf)) == 0) {
                assert(kp);
                continue;
            }
            if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
                objToString(buf, (MprObj*) kp->data, kp->type, pretty);
            } else {
                quoteValue(buf, kp->data);
            }
            mprPutCharToBuf(buf, ',');
            if (pretty) mprPutCharToBuf(buf, '\n');
        }
        mprPutCharToBuf(buf, ']');

    } else if (type == MPR_JSON_OBJ) {
        mprPutCharToBuf(buf, '{');
        if (pretty) mprPutCharToBuf(buf, '\n');
        for (ITERATE_KEYS(obj, kp)) {
            if (kp->key == 0 || kp->data == 0) continue;
            if (pretty) mprPutStringToBuf(buf, "    ");
            mprPutStringToBuf(buf, kp->key);
            mprPutStringToBuf(buf, ": ");
            if (kp->type == MPR_JSON_ARRAY || kp->type == MPR_JSON_OBJ) {
                objToString(buf, (MprObj*) kp->data, kp->type, pretty);
            } else {
                quoteValue(buf, kp->data);
            }
            mprPutCharToBuf(buf, ',');
            if (pretty) mprPutCharToBuf(buf, '\n');
        }
        mprPutCharToBuf(buf, '}');
    }
    if (pretty) mprPutCharToBuf(buf, '\n');
    return sclone(mprGetBufStart(buf));
}


/*
    Serialize into JSON format.
 */
PUBLIC cchar *mprSerialize(MprObj *obj, int flags)
{
    MprBuf  *buf;
    int     pretty;

    pretty = (flags & MPR_JSON_PRETTY);
    if ((buf = mprCreateBuf(0, 0)) == 0) {
        return 0;
    }
    objToString(buf, obj, MPR_JSON_OBJ, pretty);
    return mprGetBuf(buf);
}


static char advanceToken(MprJson *jp)
{
    while (isspace((uchar) *jp->tok)) {
        if (*jp->tok == '\n') {
            jp->lineNumber++;
        }
        jp->tok++;
    }
    return *jp->tok;
}


static cchar *findQuote(cchar *tok, int quote)
{
    cchar   *cp;

    assert(tok);
    for (cp = tok; *cp; cp++) {
        if (*cp == quote && (cp == tok || *cp != '\\')) {
            return cp;
        }
    }
    return 0;
}


static cchar *findEndKeyword(MprJson *jp, cchar *str)
{
    cchar   *cp, *etok;

    assert(str);
    for (cp = jp->tok; *cp; cp++) {
        if ((etok = strpbrk(cp, " \t\n\r:,}]")) != 0) {
            if (etok == jp->tok || *etok != '\\') {
                return etok;
            }
        }
    }
    return &str[strlen(str)];
}


static void jsonParseError(MprJson *jp, cchar *msg)
{
    if (jp->path) {
        mprTrace(4, "%s\nIn file '%s' at line %d", msg, jp->path, jp->lineNumber);
    } else {
        mprTrace(4, "%s\nAt line %d", msg, jp->lineNumber);
    }
}


PUBLIC void mprJsonParseError(MprJson *jp, cchar *fmt, ...)
{
    va_list     args;
    cchar       *msg;

    va_start(args, fmt);
    msg = sfmtv(fmt, args);
    (jp->callback.parseError)(jp, msg);
    va_end(args);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/kqueue.c"
 */
/************************************************************************/

/**
    kevent.c - Wait for I/O by using kevent on BSD based Unix systems.

    This module augments the mprWait wait services module by providing kqueue() based waiting support.
    Also see mprAsyncSelectWait and mprSelectWait. This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if MPR_EVENT_KQUEUE
/********************************** Forwards **********************************/

static int growEvents(MprWaitService *ws);
static void serviceIO(MprWaitService *ws, int count);

/************************************ Code ************************************/

PUBLIC int mprCreateNotifierService(MprWaitService *ws)
{
    ws->interestMax = MPR_FD_MIN;
    ws->eventsMax = MPR_FD_MIN;
    ws->handlerMax = MPR_FD_MIN;
    ws->interest = mprAllocZeroed(sizeof(struct kevent) * ws->interestMax);
    ws->events = mprAllocZeroed(sizeof(struct kevent) * ws->eventsMax);
    ws->handlerMap = mprAllocZeroed(sizeof(MprWaitHandler*) * ws->handlerMax);
    if (ws->interest == 0 || ws->events == 0 || ws->handlerMap == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    if ((ws->kq = kqueue()) < 0) {
        mprError("Call to kqueue() failed");
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Initialize the "wakeup" pipe. This is used to wakeup the service thread if other threads need to wait for I/O.
     */
    if (pipe(ws->breakPipe) < 0) {
        mprError("Cannot open breakout pipe");
        return MPR_ERR_CANT_INITIALIZE;
    }
    fcntl(ws->breakPipe[0], F_SETFL, fcntl(ws->breakPipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(ws->breakPipe[1], F_SETFL, fcntl(ws->breakPipe[1], F_GETFL) | O_NONBLOCK);
    EV_SET(&ws->interest[ws->interestCount], ws->breakPipe[MPR_READ_PIPE], EVFILT_READ, EV_ADD, 0, 0, 0);
    ws->interestCount++;
    return 0;
}


PUBLIC void mprManageKqueue(MprWaitService *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->events);
        mprMark(ws->interest);
        mprMark(ws->stableInterest);

    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->kq) {
            close(ws->kq);
        }
        if (ws->breakPipe[0] >= 0) {
            close(ws->breakPipe[0]);
        }
        if (ws->breakPipe[1] >= 0) {
            close(ws->breakPipe[1]);
        }
    }
}


static int growEvents(MprWaitService *ws)
{
    ws->interestMax *= 2;
    ws->eventsMax = ws->interestMax;
    ws->interest = mprRealloc(ws->interest, sizeof(struct kevent) * ws->interestMax);
    ws->events = mprRealloc(ws->events, sizeof(struct kevent) * ws->eventsMax);
    if (ws->interest == 0 || ws->events == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


PUBLIC int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask)
{
    struct kevent   *kp, *start;
    int             fd;

    assert(wp);
    fd = wp->fd;

    lock(ws);
    mprTrace(7, "mprNotifyOn: fd %d, mask %x, old mask %x", wp->fd, mask, wp->desiredMask);
    if (wp->desiredMask != mask) {
        assert(fd >= 0);
        while ((ws->interestCount + 4) >= ws->interestMax) {
            growEvents(ws);
        }
        start = kp = &ws->interest[ws->interestCount];
        if (wp->desiredMask & MPR_READABLE && !(mask & MPR_READABLE)) {
            EV_SET(kp, fd, EVFILT_READ, EV_DELETE, 0, 0, 0);
            kp++;
        }
        if (wp->desiredMask & MPR_WRITABLE && !(mask & MPR_WRITABLE)) {
            EV_SET(kp, fd, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
            kp++;
        }
        if (mask & MPR_READABLE) {
            EV_SET(kp, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
            kp++;
        }
        if (mask & MPR_WRITABLE) {
            EV_SET(kp, fd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
            kp++;
        }
        ws->interestCount += (int) (kp - start);
        if (fd >= ws->handlerMax) {
            ws->handlerMax = fd + 32;
            if ((ws->handlerMap = mprRealloc(ws->handlerMap, sizeof(MprWaitHandler*) * ws->handlerMax)) == 0) {
                assert(!MPR_ERR_MEMORY);
                return MPR_ERR_MEMORY;
            }
        }
        assert(ws->handlerMap[fd] == 0 || ws->handlerMap[fd] == wp);
        wp->desiredMask = mask;
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    ws->handlerMap[fd] = (mask) ? wp : 0;
    unlock(ws);
    return 0;
}


/*
    Wait for I/O on a single file descriptor. Return a mask of events found. Mask is the events of interest.
    timeout is in milliseconds.
 */
PUBLIC int mprWaitForSingleIO(int fd, int mask, MprTicks timeout)
{
    struct timespec ts;
    struct kevent   interest[2], events[1];
    int             kq, interestCount, rc, result;

    if (timeout < 0) {
        timeout = MAXINT;
    }
    interestCount = 0; 
    if (mask & MPR_READABLE) {
        EV_SET(&interest[interestCount++], fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    }
    if (mask & MPR_WRITABLE) {
        EV_SET(&interest[interestCount++], fd, EVFILT_WRITE, EV_ADD, 0, 0, 0);
    }
    kq = kqueue();
    ts.tv_sec = ((int) (timeout / 1000));
    ts.tv_nsec = ((int) (timeout % 1000)) * 1000 * 1000;

    result = 0;
    rc = kevent(kq, interest, interestCount, events, 1, &ts);
    close(kq);
    if (rc < 0) {
        mprTrace(7, "Kevent returned %d, errno %d", rc, errno);
    } else if (rc > 0) {
        if (rc > 0) {
            if (events[0].filter & EVFILT_READ) {
                result |= MPR_READABLE;
            }
            if (events[0].filter == EVFILT_WRITE) {
                result |= MPR_WRITABLE;
            }
        }
    }
    return result;
}


/*
    Wait for I/O on all registered file descriptors. Timeout is in milliseconds. Return the number of events detected.
 */
PUBLIC void mprWaitForIO(MprWaitService *ws, MprTicks timeout)
{
    struct timespec ts;
    int             rc;

    assert(timeout > 0);

    if (timeout < 0) {
        timeout = MAXINT;
    }
#if BIT_DEBUG
    if (mprGetDebugMode() && timeout > 30000) {
        timeout = 30000;
    }
#endif
    ts.tv_sec = ((int) (timeout / 1000));
    ts.tv_nsec = ((int) ((timeout % 1000) * 1000 * 1000));

    if (ws->needRecall) {
        mprDoWaitRecall(ws);
        return;
    }
    lock(ws);
    ws->stableInterest = mprMemdup(ws->interest, sizeof(struct kevent) * ws->interestCount);
    ws->stableInterestCount = ws->interestCount;
    /* Preserve the wakeup pipe fd */
    ws->interestCount = 1;
    unlock(ws);

    mprTrace(8, "kevent sleep for %d", timeout);
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    rc = kevent(ws->kq, ws->stableInterest, ws->stableInterestCount, ws->events, ws->eventsMax, &ts);
    mprResetYield();
    mprTrace(8, "kevent wakes rc %d", rc);

    if (rc < 0) {
        mprTrace(7, "Kevent returned %d, errno %d", rc, mprGetOsError());
    } else if (rc > 0) {
        serviceIO(ws, rc);
    }
    ws->wakeRequested = 0;
}


static void serviceIO(MprWaitService *ws, int count)
{
    MprWaitHandler      *wp;
    struct kevent       *kev;
    char                buf[128];
    int                 fd, i, mask, err;

    lock(ws);
    for (i = 0; i < count; i++) {
        kev = &ws->events[i];
        fd = (int) kev->ident;
        assert(fd < ws->handlerMax);
        if ((wp = ws->handlerMap[fd]) == 0) {
            if (kev->filter == EVFILT_READ && fd == ws->breakPipe[MPR_READ_PIPE]) {
                (void) read(fd, buf, sizeof(buf));
            }
            continue;
        }
        if (kev->flags & EV_ERROR) {
            err = (int) kev->data;
            if (err == ENOENT) {
                /* File descriptor was closed and re-opened */
                mask = wp->desiredMask;
                mprNotifyOn(ws, wp, 0);
                wp->desiredMask = 0;
                mprNotifyOn(ws, wp, mask);
                mprTrace(7, "kqueue: file descriptor may have been closed and reopened, fd %d", wp->fd);

            } else if (err == EBADF) {
                /* File descriptor was closed */
                mask = wp->desiredMask;
                mprNotifyOn(ws, wp, 0);
                wp->desiredMask = 0;
                mprNotifyOn(ws, wp, mask);
                mprTrace(7, "kqueue: invalid file descriptor %d, fd %d", wp->fd);
            }
            continue;
        }
        mask = 0;
        if (kev->filter == EVFILT_READ) {
            mask |= MPR_READABLE;
        }
        if (kev->filter == EVFILT_WRITE) {
            mask |= MPR_WRITABLE;
        }
        wp->presentMask = mask & wp->desiredMask;
        mprTrace(7, "Got I/O event mask %x", wp->presentMask);
        if (wp->presentMask) {
            mprTrace(7, "ServiceIO for wp %p", wp);
            /* Suppress further events while this event is being serviced. User must re-enable */
            mprNotifyOn(ws, wp, 0);            
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    Wake the wait service. WARNING: This routine must not require locking. MprEvents in scheduleDispatcher depends on this.
    Must be async-safe.
 */
PUBLIC void mprWakeNotifier()
{
    MprWaitService  *ws;
    int             c;

    ws = MPR->waitService;
    if (!ws->wakeRequested) {
        ws->wakeRequested = 1;
        c = 0;
        (void) write(ws->breakPipe[MPR_WRITE_PIPE], (char*) &c, 1);
    }
}

#endif /* MPR_EVENT_KQUEUE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/list.c"
 */
/************************************************************************/

/**
    list.c - Simple list type.

    The list supports two modes of operation. Compact mode where the list is compacted after removing list items, 
    and no-compact mode where removed items are zeroed. No-compact mode implies that all valid list entries must 
    be non-zero.

    This module is not thread-safe. It is the callers responsibility to perform all thread synchronization.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Defines ***********************************/

#ifndef BIT_MAX_LIST
    #define BIT_MAX_LIST   8
#endif

/********************************** Forwards **********************************/

static int growList(MprList *lp, int incr);
static void manageList(MprList *lp, int flags);

/************************************ Code ************************************/
/*
    Create a general growable list structure
 */
PUBLIC MprList *mprCreateList(int size, int flags)
{
    MprList     *lp;

    if ((lp = mprAllocObj(MprList, manageList)) == 0) {
        return 0;
    }
    lp->maxSize = MAXINT;
    lp->flags = flags | MPR_OBJ_LIST;
    if (!(flags & MPR_LIST_STABLE)) {
        lp->mutex = mprCreateLock();
    }
    if (size != 0) {
        mprSetListLimits(lp, size, -1);
    }
    return lp;
}


static void manageList(MprList *lp, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(lp->mutex);
        lock(lp);
        mprMark(lp->items);
        if (!(lp->flags & MPR_LIST_STATIC_VALUES)) {
            for (i = 0; i < lp->length; i++) {
                assert(lp->items[i] == 0 || mprIsValid(lp->items[i]));
                mprMark(lp->items[i]);
            }
        }
        unlock(lp);
    }
}

/*
    Initialize a list which may not be a memory context.
 */
PUBLIC void mprInitList(MprList *lp, int flags)
{
    lp->flags = 0;
    lp->size = 0;
    lp->length = 0;
    lp->maxSize = MAXINT;
    lp->items = 0;
    lp->mutex = (flags & MPR_LIST_STABLE) ? 0 : mprCreateLock();
}


/*
    Define the list maximum size. If the list has not yet been written to, the initialSize will be observed.
 */
PUBLIC int mprSetListLimits(MprList *lp, int initialSize, int maxSize)
{
    ssize   size;

    if (initialSize <= 0) {
        initialSize = BIT_MAX_LIST;
    }
    if (maxSize <= 0) {
        maxSize = MAXINT;
    }
    size = initialSize * sizeof(void*);

    lock(lp);
    if (lp->items == 0) {
        if ((lp->items = mprAlloc(size)) == 0) {
            assert(!MPR_ERR_MEMORY);
            unlock(lp);
            return MPR_ERR_MEMORY;
        }
        memset(lp->items, 0, size);
        lp->size = initialSize;
    }
    lp->maxSize = maxSize;
    unlock(lp);
    return 0;
}


PUBLIC int mprCopyListContents(MprList *dest, MprList *src)
{
    void        *item;
    int         next;

    mprClearList(dest);

    lock(src);
    if (mprSetListLimits(dest, src->size, src->maxSize) < 0) {
        assert(!MPR_ERR_MEMORY);
        unlock(src);
        return MPR_ERR_MEMORY;
    }
    for (next = 0; (item = mprGetNextItem(src, &next)) != 0; ) {
        if (mprAddItem(dest, item) < 0) {
            assert(!MPR_ERR_MEMORY);
            unlock(src);
            return MPR_ERR_MEMORY;
        }
    }
    unlock(src);
    return 0;
}


PUBLIC MprList *mprCloneList(MprList *src)
{
    MprList     *lp;

    if ((lp = mprCreateList(src->size, src->flags)) == 0) {
        return 0;
    }
    if (mprCopyListContents(lp, src) < 0) {
        return 0;
    }
    return lp;
}


PUBLIC MprList *mprAppendList(MprList *lp, MprList *add)
{
    void        *item;
    int         next;

    assert(lp);

    for (next = 0; ((item = mprGetNextItem(add, &next)) != 0); ) {
        if (mprAddItem(lp, item) < 0) {
            return 0;
        }
    }
    return lp;
}


/*
    Change the item in the list at index. Return the old item.
 */
PUBLIC void *mprSetItem(MprList *lp, int index, cvoid *item)
{
    void    *old;
    int     length;

    assert(lp);
    assert(lp->size >= 0);
    assert(lp->length >= 0);
    assert(index >= 0);

    length = lp->length;

    if (index >= length) {
        length = index + 1;
    }
    lock(lp);
    if (length > lp->size) {
        if (growList(lp, length - lp->size) < 0) {
            unlock(lp);
            return 0;
        }
    }
    old = lp->items[index];
    lp->items[index] = (void*) item;
    lp->length = length;
    unlock(lp);
    return old;
}



/*
    Add an item to the list and return the item index.
 */
PUBLIC int mprAddItem(MprList *lp, cvoid *item)
{
    int     index;

    assert(lp);
    assert(lp->size >= 0);
    assert(lp->length >= 0);

    lock(lp);
    if (lp->length >= lp->size) {
        if (growList(lp, 1) < 0) {
            unlock(lp);
            return MPR_ERR_TOO_MANY;
        }
    }
    index = lp->length++;
    lp->items[index] = (void*) item;
    unlock(lp);
    return index;
}


PUBLIC int mprAddNullItem(MprList *lp)
{
    int     index;

    assert(lp);
    assert(lp->size >= 0);
    assert(lp->length >= 0);

    lock(lp);
    if (lp->length != 0 && lp->items[lp->length - 1] == 0) {
        index = lp->length - 1;
    } else {
        if (lp->length >= lp->size) {
            if (growList(lp, 1) < 0) {
                unlock(lp);
                return MPR_ERR_TOO_MANY;
            }
        }
        index = lp->length;
        lp->items[index] = 0;
    }
    unlock(lp);
    return index;
}


/*
    Insert an item to the list at a specified position. We insert before the item at "index".
    ie. The inserted item will go into the "index" location and the other elements will be moved up.
 */
PUBLIC int mprInsertItemAtPos(MprList *lp, int index, cvoid *item)
{
    void    **items;
    int     i;

    assert(lp);
    assert(lp->size >= 0);
    assert(lp->length >= 0);
    assert(index >= 0);

    if (index < 0) {
        index = 0;
    }
    lock(lp);
    if (index >= lp->size) {
        if (growList(lp, index - lp->size + 1) < 0) {
            unlock(lp);
            return MPR_ERR_TOO_MANY;
        }

    } else if (lp->length >= lp->size) {
        if (growList(lp, 1) < 0) {
            unlock(lp);
            return MPR_ERR_TOO_MANY;
        }
    }
    if (index >= lp->length) {
        lp->length = index + 1;
    } else {
        /*
            Copy up items to make room to insert
         */
        items = lp->items;
        for (i = lp->length; i > index; i--) {
            items[i] = items[i - 1];
        }
        lp->length++;
    }
    lp->items[index] = (void*) item;
    unlock(lp);
    return index;
}


/*
    Remove an item from the list. Return the index where the item resided.
 */
PUBLIC int mprRemoveItem(MprList *lp, cvoid *item)
{
    int     index;

    assert(lp);

    lock(lp);
    index = mprLookupItem(lp, item);
    if (index < 0) {
        unlock(lp);
        return index;
    }
    index = mprRemoveItemAtPos(lp, index);
    assert(index >= 0);
    unlock(lp);
    return index;
}


PUBLIC int mprRemoveLastItem(MprList *lp)
{
    assert(lp);
    assert(lp->size > 0);
    assert(lp->length > 0);

    if (lp->length <= 0) {
        return MPR_ERR_CANT_FIND;
    }
    return mprRemoveItemAtPos(lp, lp->length - 1);
}


/*
    Remove an index from the list. Return the index where the item resided.
    The list is compacted.
 */
PUBLIC int mprRemoveItemAtPos(MprList *lp, int index)
{
    void    **items;

    assert(lp);
    assert(lp->size > 0);
    assert(index >= 0 && index < lp->size);
    assert(lp->length > 0);

    if (index < 0 || index >= lp->length) {
        return MPR_ERR_CANT_FIND;
    }
    lock(lp);
    items = lp->items;
#if FUTURE
    void    **ip;
    if (index == (lp->length - 1)) {
        /* Scan backwards to find last non-null item */
        for (ip = &items[index - 1]; ip >= items && *ip == 0; ip--) ;
        lp->length = ++ip - items;
        assert(lp->length >= 0);
    } else {
        /* Copy down following items */
        for (ip = &items[index]; ip < &items[lp->length]; ip++) {
            *ip = ip[1];
        }
        lp->length--;
    }
#else
    memmove(&items[index], &items[index + 1], (lp->length - index - 1) * sizeof(void*));
    lp->length--;
#endif
    lp->items[lp->length] = 0;
    assert(lp->length >= 0);
    unlock(lp);
    return index;
}


/*
    Remove a set of items. Return 0 if successful.
 */
PUBLIC int mprRemoveRangeOfItems(MprList *lp, int start, int end)
{
    void    **items;
    int     i, count;

    assert(lp);
    assert(lp->size > 0);
    assert(lp->length > 0);
    assert(start > end);

    if (start < 0 || start >= lp->length) {
        return MPR_ERR_CANT_FIND;
    }
    if (end < 0 || end >= lp->length) {
        return MPR_ERR_CANT_FIND;
    }
    if (start > end) {
        return MPR_ERR_BAD_ARGS;
    }
    /*
        Copy down to compress
     */
    items = lp->items;
    count = end - start;
    lock(lp);
    for (i = start; i < (lp->length - count); i++) {
        items[i] = items[i + count];
    }
    lp->length -= count;
    for (i = lp->length; i < lp->size; i++) {
        items[i] = 0;
    }
    unlock(lp);
    return 0;
}


/*
    Remove a string item from the list. Return the index where the item resided.
 */
PUBLIC int mprRemoveStringItem(MprList *lp, cchar *str)
{
    int     index;

    assert(lp);

    lock(lp);
    index = mprLookupStringItem(lp, str);
    if (index < 0) {
        unlock(lp);
        return index;
    }
    index = mprRemoveItemAtPos(lp, index);
    assert(index >= 0);
    unlock(lp);
    return index;
}


PUBLIC void *mprGetItem(MprList *lp, int index)
{
    assert(lp);

    if (index < 0 || index >= lp->length) {
        return 0;
    }
    return lp->items[index];
}


PUBLIC void *mprGetFirstItem(MprList *lp)
{
    assert(lp);

    if (lp == 0) {
        return 0;
    }
    if (lp->length == 0) {
        return 0;
    }
    return lp->items[0];
}


PUBLIC void *mprGetLastItem(MprList *lp)
{
    assert(lp);

    if (lp == 0) {
        return 0;
    }
    if (lp->length == 0) {
        return 0;
    }
    return lp->items[lp->length - 1];
}


PUBLIC void *mprGetNextItem(MprList *lp, int *next)
{
    void    *item;
    int     index;

    assert(next);
    assert(*next >= 0);

    if (lp == 0) {
        return 0;
    }
    lock(lp);
    index = *next;
    if (index < lp->length) {
        item = lp->items[index];
        *next = ++index;
        unlock(lp);
        return item;
    }
    unlock(lp);
    return 0;
}


PUBLIC void *mprGetNextStableItem(MprList *lp, int *next)
{
    void    *item;
    int     index;

    assert(next);
    assert(*next >= 0);
    assert(lp->flags & MPR_LIST_STABLE);

    if (lp == 0) {
        return 0;
    }
    index = *next;
    if (index < lp->length) {
        item = lp->items[index];
        *next = ++index;
        return item;
    }
    return 0;
}


PUBLIC void *mprGetPrevItem(MprList *lp, int *next)
{
    void    *item;
    int     index;

    assert(next);

    if (lp == 0) {
        return 0;
    }
    lock(lp);
    if (*next < 0) {
        *next = lp->length;
    }
    index = *next;
    if (--index < lp->length && index >= 0) {
        *next = index;
        item = lp->items[index];
        unlock(lp);
        return item;
    }
    unlock(lp);
    return 0;
}


PUBLIC int mprPushItem(MprList *lp, cvoid *item)
{
    return mprAddItem(lp, item);
}


PUBLIC void *mprPopItem(MprList *lp)
{
    void    *item;
    int     index;

    item = NULL;
    if (lp->length > 0) {
        lock(lp);
        index = lp->length - 1;
        item = mprGetItem(lp, index);
        mprRemoveItemAtPos(lp, index);
        unlock(lp);
    }
    return item;
}


#ifndef mprGetListLength
PUBLIC int mprGetListLength(MprList *lp)
{
    if (lp == 0) {
        return 0;
    }
    return lp->length;
}
#endif


PUBLIC int mprGetListCapacity(MprList *lp)
{
    assert(lp);

    if (lp == 0) {
        return 0;
    }
    return lp->size;
}


PUBLIC void mprClearList(MprList *lp)
{
    int     i;

    assert(lp);

    lock(lp);
    for (i = 0; i < lp->length; i++) {
        lp->items[i] = 0;
    }
    lp->length = 0;
    unlock(lp);
}


PUBLIC int mprLookupItem(MprList *lp, cvoid *item)
{
    int     i;

    assert(lp);
    
    lock(lp);
    for (i = 0; i < lp->length; i++) {
        if (lp->items[i] == item) {
            unlock(lp);
            return i;
        }
    }
    unlock(lp);
    return MPR_ERR_CANT_FIND;
}


PUBLIC int mprLookupStringItem(MprList *lp, cchar *str)
{
    int     i;

    assert(lp);
    
    lock(lp);
    for (i = 0; i < lp->length; i++) {
        if (smatch(lp->items[i], str)) {
            unlock(lp);
            return i;
        }
    }
    unlock(lp);
    return MPR_ERR_CANT_FIND;
}


/*
    Grow the list by the requried increment
 */
static int growList(MprList *lp, int incr)
{
    ssize       memsize;
    int         len;

    if (lp->maxSize <= 0) {
        lp->maxSize = MAXINT;
    }
    /*
        Need to grow the list
     */
    if (lp->size >= lp->maxSize) {
        assert(lp->size < lp->maxSize);
        return MPR_ERR_TOO_MANY;
    }
    /*
        If growing by 1, then use the default increment which exponentially grows. Otherwise, assume the caller knows exactly
        how much the list needs to grow.
     */
    if (incr <= 1) {
        len = BIT_MAX_LIST + (lp->size * 2);
    } else {
        len = lp->size + incr;
    }
    memsize = len * sizeof(void*);

    if ((lp->items = mprRealloc(lp->items, memsize)) == NULL) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    lp->size = len;
    return 0;
}


static int defaultSort(char **q1, char **q2, void *ctx)
{
    return scmp(*q1, *q2);
}


PUBLIC MprList *mprSortList(MprList *lp, MprSortProc compare, void *ctx)
{
    lock(lp);
    if (!compare) {
        compare = (MprSortProc) defaultSort;
    }
    mprSort(lp->items, lp->length, sizeof(void*), compare, ctx);
    unlock(lp);
    return lp;
}


static void manageKeyValue(MprKeyValue *pair, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(pair->key);
        mprMark(pair->value);
    }
}


PUBLIC MprKeyValue *mprCreateKeyPair(cchar *key, cchar *value)
{
    MprKeyValue     *pair;
    
    if ((pair = mprAllocObj(MprKeyValue, manageKeyValue)) == 0) {
        return 0;
    }
    pair->key = sclone(key);
    pair->value = sclone(value);
    return pair;
}


static void swapElt(char *a, char *b, ssize width)
{
    char tmp;

    if (a != b) {
        while (width--) {
            tmp = *a;
            *a++ = *b;
            *b++ = tmp;
        }
    }
}


static void shortsort(char *lo, char *hi, ssize width, MprSortProc comp, void *ctx)
{
    char    *p, *max;

    while (hi > lo) {
        max = lo;
        for (p = lo + width; p <= hi; p += width) {
            if (comp(p, max, ctx) > 0) {
                max = p;
            }
        }
        swapElt(max, hi, width);
        hi -= width;
    }
}

PUBLIC void mprSort(void *base, ssize num, ssize width, MprSortProc comp, void *ctx) 
{
    char    *lo, *hi, *mid, *l, *h, *lostk[30], *histk[30];
    ssize   size;
    int     stkptr;

    if (num < 2 || width == 0) {
        return;
    }
    stkptr = 0;
    lo = base;
    hi = (char *) base + width * (num - 1);

recurse:
    size = (int) (hi - lo) / width + 1;
    if (size <= 8) {
        shortsort(lo, hi, width, comp, ctx);
    } else {
        mid = lo + (size / 2) * width;
        swapElt(mid, lo, width);
        l = lo;
        h = hi + width;

        for (;;) {
            do { l += width; } while (l <= hi && comp(l, lo, ctx) <= 0);
            do { h -= width; } while (h > lo && comp(h, lo, ctx) >= 0);
            if (h < l) break;
            swapElt(l, h, width);
        }
        swapElt(lo, h, width);

        if (h - 1 - lo >= hi - l) {
            if (lo + width < h) {
                lostk[stkptr] = lo;
                histk[stkptr] = h - width;
                ++stkptr;
            }
            if (l < hi) {
                lo = l;
                goto recurse;
            }
        } else {
            if (l < hi) {
                lostk[stkptr] = l;
                histk[stkptr] = hi;
                ++stkptr;
            }
            if (lo + width < h) {
                hi = h - width;
                goto recurse;
            }
        }
    }
    --stkptr;
    if (stkptr >= 0) {
        lo = lostk[stkptr];
        hi = histk[stkptr];
        goto recurse;
    } else {
        return;
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/lock.c"
 */
/************************************************************************/

/**
    lock.c - Thread Locking Support

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*********************************** Includes *********************************/



/***************************** Forward Declarations ***************************/

static void manageLock(MprMutex *lock, int flags);

/************************************ Code ************************************/

PUBLIC MprMutex *mprCreateLock()
{
    MprMutex    *lock;
#if BIT_UNIX_LIKE
    pthread_mutexattr_t attr;
#endif
    if ((lock = mprAllocObj(MprMutex, manageLock)) == 0) {
        return 0;
    }
#if BIT_UNIX_LIKE
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&lock->cs, &attr);
    pthread_mutexattr_destroy(&attr);
#elif WINCE
    InitializeCriticalSection(&lock->cs);
#elif BIT_WIN_LIKE
    InitializeCriticalSectionAndSpinCount(&lock->cs, 5000);
#elif VXWORKS
    /* Removed SEM_INVERSION_SAFE */
    lock->cs = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    if (lock->cs == 0) {
        assert(0);
        return 0;
    }
#endif
    return lock;
}


static void manageLock(MprMutex *lock, int flags)
{
    if (flags & MPR_MANAGE_FREE) {
        assert(lock);
#if BIT_UNIX_LIKE
        pthread_mutex_destroy(&lock->cs);
#elif BIT_WIN_LIKE
        DeleteCriticalSection(&lock->cs);
        lock->cs.SpinCount = 0;
#elif VXWORKS
        semDelete(lock->cs);
#endif
    }
}


PUBLIC MprMutex *mprInitLock(MprMutex *lock)
{
#if BIT_UNIX_LIKE
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&lock->cs, &attr);
    pthread_mutexattr_destroy(&attr);

#elif WINCE
    InitializeCriticalSection(&lock->cs);

#elif BIT_WIN_LIKE
    InitializeCriticalSectionAndSpinCount(&lock->cs, 5000);

#elif VXWORKS
    /* Removed SEM_INVERSION_SAFE */
    lock->cs = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);
    if (lock->cs == 0) {
        assert(0);
        return 0;
    }
#endif
    return lock;
}


/*
    Try to attain a lock. Do not block! Returns true if the lock was attained.
 */
PUBLIC bool mprTryLock(MprMutex *lock)
{
    int     rc;

    if (lock == 0) return 0;

#if BIT_UNIX_LIKE
    rc = pthread_mutex_trylock(&lock->cs) != 0;
#elif BIT_WIN_LIKE
    /* Rely on SpinCount being non-zero */
    if (lock->cs.SpinCount) {
        rc = TryEnterCriticalSection(&lock->cs) == 0;
    } else {
        rc = 0;
    }
#elif VXWORKS
    rc = semTake(lock->cs, NO_WAIT) != OK;
#endif
#if BIT_DEBUG
    lock->owner = mprGetCurrentOsThread();
#endif
    return (rc) ? 0 : 1;
}


PUBLIC MprSpin *mprCreateSpinLock()
{
    MprSpin    *lock;

    if ((lock = mprAllocObj(MprSpin, mprManageSpinLock)) == 0) {
        return 0;
    }
    return mprInitSpinLock(lock);
}


PUBLIC void mprManageSpinLock(MprSpin *lock, int flags)
{
    if (flags & MPR_MANAGE_FREE) {
        assert(lock);
#if USE_MPR_LOCK || MACOSX
        ;
#elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
        pthread_spin_destroy(&lock->cs);
#elif BIT_UNIX_LIKE
        pthread_mutex_destroy(&lock->cs);
#elif BIT_WIN_LIKE
        DeleteCriticalSection(&lock->cs);
        lock->cs.SpinCount = 0;
#elif VXWORKS
        semDelete(lock->cs);
#endif
    }
}


/*
    Static version just for mprAlloc which needs locks that don't allocate memory.
 */
PUBLIC MprSpin *mprInitSpinLock(MprSpin *lock)
{
#if BIT_UNIX_LIKE && !BIT_HAS_SPINLOCK && !MACOSX
    pthread_mutexattr_t attr;
#endif

#if USE_MPR_LOCK
    mprInitLock(&lock->cs);
#elif MACOSX
    lock->cs = OS_SPINLOCK_INIT;
#elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
    pthread_spin_init(&lock->cs, 0);
#elif BIT_UNIX_LIKE
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&lock->cs, &attr);
    pthread_mutexattr_destroy(&attr);
#elif WINCE
    InitializeCriticalSection(&lock->cs);
#elif BIT_WIN_LIKE
    InitializeCriticalSectionAndSpinCount(&lock->cs, 5000);
#elif VXWORKS
    #if FUTURE
        spinLockTaskInit(&lock->cs, 0);
    #else
        /* Removed SEM_INVERSION_SAFE */
        lock->cs = semMCreate(SEM_Q_PRIORITY | SEM_DELETE_SAFE);
        if (lock->cs == 0) {
            assert(0);
            return 0;
        }
    #endif
#endif /* VXWORKS */
#if BIT_DEBUG
    lock->owner = 0;
#endif
    return lock;
}


/*
    Try to attain a lock. Do not block! Returns true if the lock was attained.
 */
PUBLIC bool mprTrySpinLock(MprSpin *lock)
{
    int     rc;

    if (lock == 0) return 0;

#if USE_MPR_LOCK
    mprTryLock(&lock->cs);
#elif MACOSX
    rc = !OSSpinLockTry(&lock->cs);
#elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
    rc = pthread_spin_trylock(&lock->cs) != 0;
#elif BIT_UNIX_LIKE
    rc = pthread_mutex_trylock(&lock->cs) != 0;
#elif BIT_WIN_LIKE
    /* Rely on SpinCount being non-zero */
    if (lock->cs.SpinCount) {
        rc = TryEnterCriticalSection(&lock->cs) == 0;
    } else {
        rc = 0;
    }
#elif VXWORKS
    rc = semTake(lock->cs, NO_WAIT) != OK;
#endif
#if BIT_DEBUG
    if (rc == 0) {
        assert(lock->owner != mprGetCurrentOsThread());
        lock->owner = mprGetCurrentOsThread();
    }
#endif
    return (rc) ? 0 : 1;
}


/*
    Big global lock. Avoid using this.
 */
PUBLIC void mprGlobalLock()
{
    if (MPR && MPR->mutex) {
        mprLock(MPR->mutex);
    }
}


PUBLIC void mprGlobalUnlock()
{
    if (MPR && MPR->mutex) {
        mprUnlock(MPR->mutex);
    }
}


#if BIT_USE_LOCK_MACROS
/*
    Still define these even if using macros to make linking with *.def export files easier
 */
#undef mprLock
#undef mprUnlock
#undef mprSpinLock
#undef mprSpinUnlock
#endif

/*
    Lock a mutex
 */
PUBLIC void mprLock(MprMutex *lock)
{
    if (lock == 0) return;
#if BIT_UNIX_LIKE
    pthread_mutex_lock(&lock->cs);
#elif BIT_WIN_LIKE
    /* Rely on SpinCount being non-zero */
    if (lock->cs.SpinCount) {
        EnterCriticalSection(&lock->cs);
    }
#elif VXWORKS
    semTake(lock->cs, WAIT_FOREVER);
#endif
#if BIT_DEBUG
    /* Store last locker only */ 
    lock->owner = mprGetCurrentOsThread();
#endif
}


PUBLIC void mprUnlock(MprMutex *lock)
{
    if (lock == 0) return;
#if BIT_UNIX_LIKE
    pthread_mutex_unlock(&lock->cs);
#elif BIT_WIN_LIKE
    LeaveCriticalSection(&lock->cs);
#elif VXWORKS
    semGive(lock->cs);
#endif
}


/*
    Use functions for debug mode. Production release uses macros
 */
/*
    Lock a mutex
 */
PUBLIC void mprSpinLock(MprSpin *lock)
{
    if (lock == 0) return;

#if BIT_DEBUG
    /*
        Spin locks don't support recursive locking on all operating systems.
     */
    assert(lock->owner != mprGetCurrentOsThread());
#endif

#if USE_MPR_LOCK
    mprLock(&lock->cs);
#elif MACOSX
    OSSpinLockLock(&lock->cs);
#elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
    pthread_spin_lock(&lock->cs);
#elif BIT_UNIX_LIKE
    pthread_mutex_lock(&lock->cs);
#elif BIT_WIN_LIKE
    if (lock->cs.SpinCount) {
        EnterCriticalSection(&lock->cs);
    }
#elif VXWORKS
    semTake(lock->cs, WAIT_FOREVER);
#endif
#if BIT_DEBUG
    assert(lock->owner != mprGetCurrentOsThread());
    lock->owner = mprGetCurrentOsThread();
#endif
}


PUBLIC void mprSpinUnlock(MprSpin *lock)
{
    if (lock == 0) return;

#if BIT_DEBUG
    lock->owner = 0;
#endif

#if USE_MPR_LOCK
    mprUnlock(&lock->cs);
#elif MACOSX
    OSSpinLockUnlock(&lock->cs);
#elif BIT_UNIX_LIKE && BIT_HAS_SPINLOCK
    pthread_spin_unlock(&lock->cs);
#elif BIT_UNIX_LIKE
    pthread_mutex_unlock(&lock->cs);
#elif BIT_WIN_LIKE
    LeaveCriticalSection(&lock->cs);
#elif VXWORKS
    semGive(lock->cs);
#endif
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/log.c"
 */
/************************************************************************/

/**
    log.c - Multithreaded Portable Runtime (MPR) Logging and error reporting.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Defines ***********************************/

#ifndef BIT_MAX_LOGLINE
    #define BIT_MAX_LOGLINE 8192           /* Max size of a log line */
#endif

/********************************** Forwards **********************************/

static void defaultLogHandler(int flags, int level, cchar *msg);
static void logOutput(int flags, int level, cchar *msg);

/************************************ Code ************************************/
/*
    Put first in file so it is easy to locate in a debugger
 */
PUBLIC void mprBreakpoint()
{
#if DEBUG_PAUSE
    {
        static int  paused = 1;
        int         i;
        printf("Paused to permit debugger to attach - will awake in 2 minutes\n");
        fflush(stdout);
        for (i = 0; i < 120 && paused; i++) {
            mprNap(1000);
        }
    }
#endif
}


PUBLIC void mprCreateLogService() 
{
    MPR->logFile = MPR->stdError;
}


PUBLIC int mprStartLogging(cchar *logSpec, int showConfig)
{
    MprFile     *file;
    char        *levelSpec, *path;
    int         level;

    level = -1;
    file = 0;
    if (logSpec == 0) {
        logSpec = "stderr:0";
    }
    if (*logSpec && strcmp(logSpec, "none") != 0) {
        MPR->logPath = path = sclone(logSpec);
        if ((levelSpec = strrchr(path, ':')) != 0 && isdigit((uchar) levelSpec[1])) {
            *levelSpec++ = '\0';
            level = atoi(levelSpec);
        }
        if (strcmp(path, "stdout") == 0) {
            file = MPR->stdOutput;
        } else if (strcmp(path, "stderr") == 0) {
            file = MPR->stdError;
#if !BIT_ROM
        } else {
            MprPath     info;
            int         mode;
            mode = (MPR->flags & MPR_LOG_APPEND)  ? O_APPEND : O_TRUNC;
            mode |= O_CREAT | O_WRONLY | O_TEXT;
            if (MPR->logBackup > 0) {
                mprGetPathInfo(path, &info);
                if (MPR->logSize <= 0 || (info.valid && info.size > MPR->logSize) || (MPR->flags & MPR_LOG_ANEW)) {
                    mprBackupLog(path, MPR->logBackup);
                }
            }
            if ((file = mprOpenFile(path, mode, 0664)) == 0) {
                mprError("Cannot open log file %s", path);
                return -1;
            }
#endif
        }
        if (level >= 0) {
            mprSetLogLevel(level);
        }
        if (file) {
            mprSetLogFile(file);
        }
        if (showConfig) {
            mprLogHeader();
        }
    }
    return 0;
}


PUBLIC void mprLogHeader()
{
    mprLog(MPR_CONFIG, "Configuration for %s", mprGetAppTitle());
    mprLog(MPR_CONFIG, "---------------------------------------------");
    mprLog(MPR_CONFIG, "Version:            %s-%s", BIT_VERSION, BIT_BUILD_NUMBER);
    mprLog(MPR_CONFIG, "BuildType:          %s", BIT_DEBUG ? "Debug" : "Release");
    mprLog(MPR_CONFIG, "CPU:                %s", BIT_CPU);
    mprLog(MPR_CONFIG, "OS:                 %s", BIT_OS);
    mprLog(MPR_CONFIG, "Host:               %s", mprGetHostName());
    mprLog(MPR_CONFIG, "Directory:          %s", mprGetCurrentPath());
    mprLog(MPR_CONFIG, "Configure:          %s", BIT_CONFIG_CMD);
    mprLog(MPR_CONFIG, "---------------------------------------------");
}


PUBLIC int mprBackupLog(cchar *path, int count)
{
    char    *from, *to;
    int     i;

    for (i = count - 1; i > 0; i--) {
        from = sfmt("%s.%d", path, i - 1);
        to = sfmt("%s.%d", path, i);
        unlink(to);
        rename(from, to);
    }
    from = sfmt("%s", path);
    to = sfmt("%s.0", path);
    unlink(to);
    if (rename(from, to) < 0) {
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


PUBLIC void mprSetLogBackup(ssize size, int backup, int flags)
{
    MPR->logBackup = backup;
    MPR->logSize = size;
    MPR->flags |= (flags & (MPR_LOG_APPEND | MPR_LOG_ANEW));
}


PUBLIC void mprTraceProc(int level, cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    logOutput(MPR_LOG_MSG, level, fmtv(buf, sizeof(buf), fmt, args));
    va_end(args);
}


PUBLIC void mprLogProc(int level, cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    logOutput(MPR_LOG_MSG, level, fmtv(buf, sizeof(buf), fmt, args));
    va_end(args);
}


/*
    Warning: this will allocate
 */
PUBLIC void mprRawLog(int level, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    logOutput(MPR_RAW_MSG, 0, sfmtv(fmt, args));
    va_end(args);
}


PUBLIC void mprError(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    logOutput(MPR_ERROR_MSG, 0, fmtv(buf, sizeof(buf), fmt, args));
    va_end(args);
    mprBreakpoint();
}


PUBLIC void mprInfo(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    logOutput(MPR_INFO_MSG, 0, fmtv(buf, sizeof(buf), fmt, args));
    va_end(args);
    mprBreakpoint();
}


PUBLIC void mprWarn(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    logOutput(MPR_WARN_MSG, 0, fmtv(buf, sizeof(buf), fmt, args));
    va_end(args);
    mprBreakpoint();
}


#if DEPRECATED
PUBLIC void mprMemoryError(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    if (fmt == 0) {
        logOutput(MPR_ERROR_MSG, 0, "Memory allocation error");
    } else {
        va_start(args, fmt);
        fmtv(buf, sizeof(buf), fmt, args);
        va_end(args);
        logOutput(MPR_ERROR_MSG, 0, buf);
    }
}


PUBLIC void mprUserError(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    fmtv(buf, sizeof(buf), fmt, args);
    va_end(args);
    logOutput(MPR_USER_MSG | MPR_ERROR_MSG, 0, buf);
}


PUBLIC void mprFatalError(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    fmtv(buf, sizeof(buf), fmt, args);
    va_end(args);
    logOutput(MPR_USER_MSG | MPR_FATAL_MSG, 0, buf);
    exit(2);
}


PUBLIC void mprStaticError(cchar *fmt, ...)
{
    va_list     args;
    char        buf[BIT_MAX_LOGLINE];

    va_start(args, fmt);
    fmtv(buf, sizeof(buf), fmt, args);
    va_end(args);
#if BIT_UNIX_LIKE || VXWORKS
    if (write(2, (char*) buf, slen(buf)) < 0) {}
    if (write(2, (char*) "\n", 1) < 0) {}
#elif BIT_WIN_LIKE
    if (fprintf(stderr, "%s\n", buf) < 0) {}
    fflush(stderr);
#endif
    mprBreakpoint();
}
#endif /* DEPRECATED */


PUBLIC void mprAssert(cchar *loc, cchar *msg)
{
#if BIT_ASSERT
    char    buf[BIT_MAX_LOGLINE];

    if (loc) {
#if BIT_UNIX_LIKE
        snprintf(buf, sizeof(buf), "Assertion %s, failed at %s", msg, loc);
#else
        sprintf(buf, "Assertion %s, failed at %s", msg, loc);
#endif
        msg = buf;
    }
    mprTrace(0, "%s", buf);
    mprBreakpoint();
#if WATSON_PAUSE
    printf("Stop for WATSON\n");
    mprNap(60 * 1000);
#endif
#endif
}


/*
    Output a log message to the log handler
 */
static void logOutput(int flags, int level, cchar *msg)
{
    MprLogHandler   handler;

    if (level < 0 || level > mprGetLogLevel()) {
        return;
    }
    handler = MPR->logHandler;
    if (handler != 0) {
        (handler)(flags, level, msg);
        return;
    }
    defaultLogHandler(flags, level, msg);
}


static void defaultLogHandler(int flags, int level, cchar *msg)
{
    MprFile     *file;
    char        *prefix, buf[BIT_MAX_LOGLINE], *tag;

    if ((file = MPR->logFile) == 0) {
        return;
    }
    prefix = MPR->name;

#if !BIT_ROM
    {
        static int  check = 0;
        MprPath     info;
        int         mode;
        if (MPR->logBackup > 0 && MPR->logSize && (check++ % 1000) == 0) {
            mprGetPathInfo(MPR->logPath, &info);
            if (info.valid && info.size > MPR->logSize) {
                lock(MPR);
                mprSetLogFile(0);
                mprBackupLog(MPR->logPath, MPR->logBackup);
                mode = O_CREAT | O_WRONLY | O_TEXT;
                if ((file = mprOpenFile(MPR->logPath, mode, 0664)) == 0) {
                    mprError("Cannot open log file %s", MPR->logPath);
                    unlock(MPR);
                    return;
                }
                mprSetLogFile(file);
                unlock(MPR);
            }
        }
    }
#endif
    while (*msg == '\n') {
        mprWriteFile(file, "\n", 1);
        msg++;
    }
    if (flags & MPR_LOG_MSG) {
        fmt(buf, sizeof(buf), "%s: %d: %s\n", prefix, level, msg);
        mprWriteFileString(file, buf);

    } else if (flags & MPR_RAW_MSG) {
        mprWriteFileString(file, msg);

    } else {
        if (flags & MPR_FATAL_MSG) {
            tag = "Fatal";
        } else if (flags & MPR_WARN_MSG) {
           tag = "Warning";
        } else {
           tag = "Error";
        }
        fmt(buf, sizeof(buf), "%s: %s: %s\n", prefix, tag, msg);
        mprWriteToOsLog(buf, flags, level);
        fmt(buf, sizeof(buf), "%s: Error: %s\n", prefix, msg);
        mprWriteFileString(file, buf);
    }
}


/*
    Return the raw O/S error code
 */
PUBLIC int mprGetOsError()
{
#if BIT_WIN_LIKE
    int     rc;
    rc = GetLastError();

    /*
        Client has closed the pipe
     */
    if (rc == ERROR_NO_DATA) {
        return EPIPE;
    }
    return rc;
#elif BIT_UNIX_LIKE || VXWORKS
    return errno;
#else
    return 0;
#endif
}


/*
    Return the mapped (portable, Posix) error code
 */
PUBLIC int mprGetError()
{
#if !BIT_WIN_LIKE
    return mprGetOsError();
#else
    int     err;

    err = mprGetOsError();

    switch (err) {
    case ERROR_SUCCESS:
        return 0;
    case ERROR_FILE_NOT_FOUND:
        return ENOENT;
    case ERROR_ACCESS_DENIED:
        return EPERM;
    case ERROR_INVALID_HANDLE:
        return EBADF;
    case ERROR_NOT_ENOUGH_MEMORY:
        return ENOMEM;
    case ERROR_PATH_BUSY:
    case ERROR_BUSY_DRIVE:
    case ERROR_NETWORK_BUSY:
    case ERROR_PIPE_BUSY:
    case ERROR_BUSY:
        return EBUSY;
    case ERROR_FILE_EXISTS:
        return EEXIST;
    case ERROR_BAD_PATHNAME:
    case ERROR_BAD_ARGUMENTS:
        return EINVAL;
    case WSAENOTSOCK:
        return ENOENT;
    case WSAEINTR:
        return EINTR;
    case WSAEBADF:
        return EBADF;
    case WSAEACCES:
        return EACCES;
    case WSAEINPROGRESS:
        return EINPROGRESS;
    case WSAEALREADY:
        return EALREADY;
    case WSAEADDRINUSE:
        return EADDRINUSE;
    case WSAEADDRNOTAVAIL:
        return EADDRNOTAVAIL;
    case WSAENETDOWN:
        return ENETDOWN;
    case WSAENETUNREACH:
        return ENETUNREACH;
    case WSAECONNABORTED:
        return ECONNABORTED;
    case WSAECONNRESET:
        return ECONNRESET;
    case WSAECONNREFUSED:
        return ECONNREFUSED;
    case WSAEWOULDBLOCK:
        return EAGAIN;
    }
    return MPR_ERR;
#endif
}


PUBLIC int mprGetLogLevel()
{
    Mpr     *mpr;

    /* Leave the code like this so debuggers can patch logLevel before returning */
    mpr = MPR;
    return mpr->logLevel;
}


PUBLIC MprLogHandler mprGetLogHandler()
{
    return MPR->logHandler;
}


PUBLIC int mprUsingDefaultLogHandler()
{
    return MPR->logHandler == defaultLogHandler;
}


PUBLIC MprFile *mprGetLogFile()
{
    return MPR->logFile;
}


PUBLIC void mprSetLogHandler(MprLogHandler handler)
{
    MPR->logHandler = handler;
}


PUBLIC void mprSetLogFile(MprFile *file)
{
    if (file != MPR->logFile && MPR->logFile != MPR->stdOutput && MPR->logFile != MPR->stdError) {
        mprCloseFile(MPR->logFile);
    }
    MPR->logFile = file;
}


PUBLIC void mprSetLogLevel(int level)
{
    MPR->logLevel = level;
}


PUBLIC bool mprSetCmdlineLogging(bool on)
{
    bool    wasLogging;

    wasLogging = MPR->cmdlineLogging;
    MPR->cmdlineLogging = on;
    return wasLogging;
}


PUBLIC bool mprGetCmdlineLogging()
{
    return MPR->cmdlineLogging;
}


#if MACOSX
/*
    Just for conditional breakpoints when debugging in Xcode
 */
PUBLIC int _cmp(char *s1, char *s2)
{
    return !strcmp(s1, s2);
}
#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/mime.c"
 */
/************************************************************************/

/* 
    mime.c - Mime type handling

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Code *************************************/
/*  
    Inbuilt mime type support
 */
static char *standardMimeTypes[] = {
    "ai",    "application/postscript",
    "asc",   "text/plain",
    "au",    "audio/basic",
    "avi",   "video/x-msvideo",
    "bin",   "application/octet-stream",
    "bmp",   "image/bmp",
    "class", "application/octet-stream",
    "css",   "text/css",
    "deb",   "application/octet-stream",
    "dll",   "application/octet-stream",
    "dmg",   "application/octet-stream",
    "doc",   "application/msword",
    "eps",   "application/postscript",
    "es",    "application/x-javascript",
    "exe",   "application/octet-stream",
    "gif",   "image/gif",
    "gz",    "application/x-gzip",
    "htm",   "text/html",
    "html",  "text/html",
    "ico",   "image/x-icon",
    "jar",   "application/octet-stream",
    "jpeg",  "image/jpeg",
    "jpg",   "image/jpeg",
    "js",    "application/javascript",
    "json",  "application/json",
    "mp3",   "audio/mpeg",
    "mpg",   "video/mpeg",
    "mpeg",  "video/mpeg",
    "pdf",   "application/pdf",
    "php",   "application/x-php",
    "pl",    "application/x-perl",
    "png",   "image/png",
    "ppt",   "application/vnd.ms-powerpoint",
    "ps",    "application/postscript",
    "py",    "application/x-python",
    "py",    "application/x-python",
    "ra",    "audio/x-realaudio",
    "ram",   "audio/x-pn-realaudio",
    "rmm",   "audio/x-pn-realaudio",
    "rtf",   "text/rtf",
    "rv",    "video/vnd.rn-realvideo",
    "so",    "application/octet-stream",
    "swf",   "application/x-shockwave-flash",
    "tar",   "application/x-tar",
    "tgz",   "application/x-gzip",
    "tiff",  "image/tiff",
    "txt",   "text/plain",
    "wav",   "audio/x-wav",
    "xls",   "application/vnd.ms-excel",
    "zip",   "application/zip",
    0,       0,
};

/********************************** Forward ***********************************/

static void addStandardMimeTypes(MprHash *table);
static void manageMimeType(MprMime *mt, int flags);

/*********************************** Code *************************************/

PUBLIC MprHash *mprCreateMimeTypes(cchar *path)
{
    MprHash     *table;
#if !BIT_ROM
    MprFile     *file;
    char        *buf, *tok, *ext, *type;
    int         line;

    if (path) {
        if ((file = mprOpenFile(path, O_RDONLY | O_TEXT, 0)) == 0) {
            return 0;
        }
        if ((table = mprCreateHash(47, 0)) == 0) {
            mprCloseFile(file);
            return 0;
        }
        line = 0;
        while ((buf = mprReadLine(file, 0, NULL)) != 0) {
            line++;
            if (buf[0] == '#' || isspace((uchar) buf[0])) {
                continue;
            }
            type = stok(buf, " \t\n\r", &tok);
            ext = stok(0, " \t\n\r", &tok);
            if (type == 0 || ext == 0) {
                mprError("Bad mime type in %s at line %d", path, line);
                continue;
            }
            while (ext) {
                mprAddMime(table, ext, type);
                ext = stok(0, " \t\n\r", &tok);
            }
        }
        mprCloseFile(file);

    } else 
#endif
    {
        if ((table = mprCreateHash(59, 0)) == 0) {
            return 0;
        }
        addStandardMimeTypes(table);
    }
    return table;
}


static void addStandardMimeTypes(MprHash *table)
{
    char    **cp;

    for (cp = standardMimeTypes; cp[0]; cp += 2) {
        mprAddMime(table, cp[0], cp[1]);
    }
}


static void manageMimeType(MprMime *mt, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mt->type);
        mprMark(mt->program);
    }
}


PUBLIC MprMime *mprAddMime(MprHash *table, cchar *ext, cchar *mimeType)
{
    MprMime  *mt;

    if ((mt = mprAllocObj(MprMime, manageMimeType)) == 0) {
        return 0;
    }
    mt->type = sclone(mimeType);
    if (*ext == '.') {
        ext++;
    }
    mprAddKey(table, ext, mt);
    return mt;
}


PUBLIC int mprSetMimeProgram(MprHash *table, cchar *mimeType, cchar *program)
{
    MprKey      *kp;
    MprMime     *mt;
    
    kp = 0;
    mt = 0;
    while ((kp = mprGetNextKey(table, kp)) != 0) {
        mt = (MprMime*) kp->data;
        if (mt->type[0] == mimeType[0] && strcmp(mt->type, mimeType) == 0) {
            break;
        }
    }
    if (mt == 0) {
        mprError("Cannot find mime type %s for action program %s", mimeType, program);
        return MPR_ERR_CANT_FIND;
    }
    mt->program = sclone(program);
    return 0;
}


PUBLIC cchar *mprGetMimeProgram(MprHash *table, cchar *mimeType)
{
    MprMime      *mt;

    if (mimeType == 0 || *mimeType == '\0') {
        return 0;
    }
    if ((mt = mprLookupKey(table, mimeType)) == 0) {
        return 0;
    }
    return mt->program;
}


PUBLIC cchar *mprLookupMime(MprHash *table, cchar *ext)
{
    MprMime     *mt;
    cchar       *ep;

    if (ext == 0 || *ext == '\0') {
        return "";
    }
    if ((ep = strrchr(ext, '.')) != 0) {
        ext = &ep[1];
    }
    if (table == 0) {
        table = MPR->mimeTypes;
    }
    if ((mt = mprLookupKey(table, ext)) == 0) {
        return 0;
    }
    return mt->type;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/mixed.c"
 */
/************************************************************************/

/**
    mixed.c - Mixed mode strings. Unicode results with ascii args.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if BIT_CHAR_LEN > 1 && KEEP
/********************************** Forwards **********************************/

PUBLIC int mcaselesscmp(wchar *str1, cchar *str2)
{
    return mncaselesscmp(str1, str2, -1);
}


PUBLIC int mcmp(wchar *s1, cchar *s2)
{
    return mncmp(s1, s2, -1);
}


PUBLIC wchar *mncontains(wchar *str, cchar *pattern, ssize limit)
{
    wchar   *cp, *s1;
    cchar   *s2;
    ssize   lim;

    assert(0 <= limit && limit < MAXSSIZE);

    if (limit < 0) {
        limit = MAXINT;
    }
    if (str == 0) {
        return 0;
    }
    if (pattern == 0 || *pattern == '\0') {
        return (wchar*) str;
    }
    for (cp = str; *cp && limit > 0; cp++, limit--) {
        s1 = cp;
        s2 = pattern;
        for (lim = limit; *s1 && *s2 && (*s1 == (uchar) *s2) && lim > 0; lim--) {
            s1++;
            s2++;
        }
        if (*s2 == '\0') {
            return cp;
        }
    }
    return 0;
}


PUBLIC wchar *mcontains(wchar *str, cchar *pattern)
{
    return mncontains(str, pattern, -1);
}


/*
    destMax and len are character counts, not sizes in bytes
 */
PUBLIC ssize mcopy(wchar *dest, ssize destMax, cchar *src)
{
    ssize       len;

    assert(src);
    assert(dest);
    assert(0 < destMax && destMax < MAXINT);

    len = slen(src);
    if (destMax <= len) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    return mtow(dest, len + 1, src, len);
}


PUBLIC int mends(wchar *str, cchar *suffix)
{
    wchar   *cp;
    cchar   *sp;

    if (str == NULL || suffix == NULL) {
        return 0;
    }
    cp = &str[wlen(str) - 1];
    sp = &suffix[slen(suffix)];
    for (; cp > str && sp > suffix; ) {
        if (*cp-- != *sp--) {
            return 0;
        }
    }
    if (sp > suffix) {
        return 0;
    }
    return 1;
}


PUBLIC wchar *mfmt(cchar *fmt, ...)
{
    va_list     ap;
    char        *mresult;

    assert(fmt);

    va_start(ap, fmt);
    mresult = sfmtv(fmt, ap);
    va_end(ap);
    return amtow(mresult, NULL);
}


PUBLIC wchar *mfmtv(cchar *fmt, va_list arg)
{
    char    *mresult;

    assert(fmt);
    mresult = sfmtv(fmt, arg);
    return amtow(mresult, NULL);
}


/*
    Sep is ascii, args are wchar
 */
PUBLIC wchar *mjoin(wchar *str, ...)
{
    wchar       *result;
    va_list     ap;

    assert(str);

    va_start(ap, str);
    result = mjoinv(str, ap);
    va_end(ap);
    return result;
}


PUBLIC wchar *mjoinv(wchar *buf, va_list args)
{
    va_list     ap;
    wchar       *dest, *str, *dp;
    int         required, len;

    assert(buf);

    va_copy(ap, args);
    required = 1;
    if (buf) {
        required += wlen(buf);
    }
    str = va_arg(ap, wchar*);
    while (str) {
        required += wlen(str);
        str = va_arg(ap, wchar*);
    }
    if ((dest = mprAlloc(required)) == 0) {
        return 0;
    }
    dp = dest;
    if (buf) {
        wcopy(dp, -1, buf);
        dp += wlen(buf);
    }
    va_copy(ap, args);
    str = va_arg(ap, wchar*);
    while (str) {
        wcopy(dp, required, str);
        len = wlen(str);
        dp += len;
        required -= len;
        str = va_arg(ap, wchar*);
    }
    *dp = '\0';
    return dest;
}


/*
    Case insensitive string comparison. Limited by length
 */
PUBLIC int mncaselesscmp(wchar *s1, cchar *s2, ssize n)
{
    int     rc;

    assert(0 <= n && n < MAXSSIZE);

    if (s1 == 0 || s2 == 0) {
        return -1;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; n > 0 && *s1 && rc == 0; s1++, s2++, n--) {
        rc = tolower((uchar) *s1) - tolower((uchar) *s2);
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}



PUBLIC int mncmp(wchar *s1, cchar *s2, ssize n)
{
    assert(0 <= n && n < MAXSSIZE);

    if (s1 == 0 && s2 == 0) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; n > 0 && *s1 && rc == 0; s1++, s2++, n--) {
        rc = *s1 - (uchar) *s2;
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}


PUBLIC ssize mncopy(wchar *dest, ssize destMax, cchar *src, ssize len)
{
    assert(0 <= len && len < MAXSSIZE);
    assert(0 < destMax && destMax < MAXSSIZE);

    return mtow(dest, destMax, src, len);
}


PUBLIC wchar *mpbrk(wchar *str, cchar *set)
{
    cchar   *sp;
    int     count;

    if (str == NULL || set == NULL) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                return str;
            }
        }
    }
    return 0;
}


/*
    Sep is ascii, args are wchar
 */
PUBLIC wchar *mrejoin(wchar *buf, ...)
{
    va_list     ap;
    wchar       *result;

    va_start(ap, buf);
    result = mrejoinv(buf, ap);
    va_end(ap);
    return result;
}


PUBLIC wchar *mrejoinv(wchar *buf, va_list args)
{
    va_list     ap;
    wchar       *dest, *str, *dp;
    int         required, len;

    va_copy(ap, args);
    required = 1;
    if (buf) {
        required += wlen(buf);
    }
    str = va_arg(ap, wchar*);
    while (str) {
        required += wlen(str);
        str = va_arg(ap, wchar*);
    }
    if ((dest = mprRealloc(buf, required)) == 0) {
        return 0;
    }
    dp = dest;
    va_copy(ap, args);
    str = va_arg(ap, wchar*);
    while (str) {
        wcopy(dp, required, str);
        len = wlen(str);
        dp += len;
        required -= len;
        str = va_arg(ap, wchar*);
    }
    *dp = '\0';
    return dest;
}


PUBLIC ssize mspn(wchar *str, cchar *set)
{
    cchar   *sp;
    int     count;

    if (str == NULL || set == NULL) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                return break;
            }
        }
        if (*str != *sp) {
            break;
        }
    }
    return count;
}
 

PUBLIC int mstarts(wchar *str, cchar *prefix)
{
    if (str == NULL || prefix == NULL) {
        return 0;
    }
    if (mncmp(str, prefix, slen(prefix)) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC wchar *mtok(wchar *str, cchar *delim, wchar **last)
{
    wchar   *start, *end;
    ssize   i;

    start = str ? str : *last;

    if (start == 0) {
        *last = 0;
        return 0;
    }
    i = mspn(start, delim);
    start += i;
    if (*start == '\0') {
        *last = 0;
        return 0;
    }
    end = mpbrk(start, delim);
    if (end) {
        *end++ = '\0';
        i = mspn(end, delim);
        end += i;
    }
    *last = end;
    return start;
}


PUBLIC wchar *mtrim(wchar *str, cchar *set, int where)
{
    wchar   s;
    ssize   len, i;

    if (str == NULL || set == NULL) {
        return str;
    }
    s = wclone(str);
    if (where & MPR_TRIM_START) {
        i = mspn(s, set);
    } else {
        i = 0;
    }
    s += i;
    if (where & MPR_TRIM_END) {
        len = wlen(s);
        while (len > 0 && mspn(&s[len - 1], set) > 0) {
            s[len - 1] = '\0';
            len--;
        }
    }
    return s;
}

#else
PUBLIC void dummyWide() {}
#endif /* BIT_CHAR_LEN > 1 */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/module.c"
 */
/************************************************************************/

/**
    module.c - Dynamic module loading support.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static void manageModule(MprModule *mp, int flags);
static void manageModuleService(MprModuleService *ms, int flags);

/************************************* Code ***********************************/
/*
    Open the module service
 */
PUBLIC MprModuleService *mprCreateModuleService()
{
    MprModuleService    *ms;

    if ((ms = mprAllocObj(MprModuleService, manageModuleService)) == 0) {
        return 0;
    }
    ms->modules = mprCreateList(-1, 0);
    ms->mutex = mprCreateLock();
    MPR->moduleService = ms;
    mprSetModuleSearchPath(NULL);
    return ms;
}


static void manageModuleService(MprModuleService *ms, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ms->modules);
        mprMark(ms->searchPath);
        mprMark(ms->mutex);
    }
}


/*
    Call the start routine for each module
 */
PUBLIC int mprStartModuleService()
{
    MprModuleService    *ms;
    MprModule           *mp;
    int                 next;

    ms = MPR->moduleService;
    assert(ms);

    for (next = 0; (mp = mprGetNextItem(ms->modules, &next)) != 0; ) {
        if (mprStartModule(mp) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
#if VXWORKS && BIT_DEBUG && SYM_SYNC_INCLUDED
    symSyncLibInit();
#endif
    return 0;
}


PUBLIC void mprStopModuleService()
{
    MprModuleService    *ms;
    MprModule           *mp;
    int                 next;

    ms = MPR->moduleService;
    assert(ms);
    mprLock(ms->mutex);
    for (next = 0; (mp = mprGetNextItem(ms->modules, &next)) != 0; ) {
        mprStopModule(mp);
    }
    mprUnlock(ms->mutex);
}


PUBLIC MprModule *mprCreateModule(cchar *name, cchar *path, cchar *entry, void *data)
{
    MprModuleService    *ms;
    MprModule           *mp;
    int                 index;

    ms = MPR->moduleService;
    assert(ms);

    if ((mp = mprAllocObj(MprModule, manageModule)) == 0) {
        return 0;
    }
    mp->name = sclone(name);
    mp->path = sclone(path);
    if (entry && *entry) {
        mp->entry = sclone(entry);
    }
    mp->moduleData = data;
    mp->lastActivity = mprGetTicks();
    index = mprAddItem(ms->modules, mp);
    if (index < 0 || mp->name == 0) {
        return 0;
    }
    return mp;
}


static void manageModule(MprModule *mp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(mp->entry);
        mprMark(mp->name);
        mprMark(mp->path);
        mprMark(mp->moduleData);
    }
}


PUBLIC int mprStartModule(MprModule *mp)
{
    assert(mp);

    if (mp->start && !(mp->flags & MPR_MODULE_STARTED)) {
        if (mp->start(mp) < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    mp->flags |= MPR_MODULE_STARTED;
    return 0;
}


PUBLIC int mprStopModule(MprModule *mp)
{
    assert(mp);

    if (mp->stop && (mp->flags & MPR_MODULE_STARTED) && !(mp->flags & MPR_MODULE_STOPPED)) {
        if (mp->stop(mp) < 0) {
            return MPR_ERR_NOT_READY;
        }
        mp->flags |= MPR_MODULE_STOPPED;
    }
    return 0;
}


/*
    See if a module is already loaded
 */
PUBLIC MprModule *mprLookupModule(cchar *name)
{
    MprModuleService    *ms;
    MprModule           *mp;
    int                 next;

    assert(name && name);

    ms = MPR->moduleService;
    assert(ms);

    for (next = 0; (mp = mprGetNextItem(ms->modules, &next)) != 0; ) {
        assert(mp->name);
        if (mp && strcmp(mp->name, name) == 0) {
            return mp;
        }
    }
    return 0;
}


PUBLIC void *mprLookupModuleData(cchar *name)
{
    MprModule   *module;

    if ((module = mprLookupModule(name)) == NULL) {
        return NULL;
    }
    return module->moduleData;
}


PUBLIC void mprSetModuleTimeout(MprModule *module, MprTicks timeout)
{
    assert(module);
    if (module) {
        module->timeout = timeout;
    }
}


PUBLIC void mprSetModuleFinalizer(MprModule *module, MprModuleProc stop)
{
    assert(module);
    if (module) {
        module->stop = stop;
    }
}


PUBLIC void mprSetModuleSearchPath(char *searchPath)
{
    MprModuleService    *ms;

    ms = MPR->moduleService;
    if (searchPath == 0) {
        ms->searchPath = sjoin(mprGetAppDir(), MPR_SEARCH_SEP, mprGetAppDir(), MPR_SEARCH_SEP, 
            BIT_VAPP_PREFIX "/bin", NULL);
    } else {
        ms->searchPath = sclone(searchPath);
    }
}


PUBLIC cchar *mprGetModuleSearchPath()
{
    return MPR->moduleService->searchPath;
}


/*
    Load a module. The module is located by searching for the filename by optionally using the module search path.
 */
PUBLIC int mprLoadModule(MprModule *mp)
{
#if BIT_HAS_DYN_LOAD
    assert(mp);

    if (mprLoadNativeModule(mp) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mprStartModule(mp);
    return 0;
#else
    mprError("Product built without the ability to load modules dynamically");
    return MPR_ERR_BAD_STATE;
#endif
}


PUBLIC int mprUnloadModule(MprModule *mp)
{
    mprTrace(6, "Unloading native module %s from %s", mp->name, mp->path);
    if (mprStopModule(mp) < 0) {
        return MPR_ERR_NOT_READY;
    }
#if BIT_HAS_DYN_LOAD
    if (mp->handle) {
        if (mprUnloadNativeModule(mp) != 0) {
            mprError("Cannot unload module %s", mp->name);
        }
        mp->handle = 0;
    }
#endif
    mprRemoveItem(MPR->moduleService->modules, mp);
    return 0;
}


#if BIT_HAS_DYN_LOAD
/*
    Return true if the shared library in "file" can be found. Return the actual path in *path. The filename
    may not have a shared library extension which is typical so calling code can be cross platform.
 */
static char *probe(cchar *filename)
{
    char    *path;

    assert(filename && *filename);

    mprTrace(7, "Probe for native module %s", filename);
    if (mprPathExists(filename, R_OK)) {
        return sclone(filename);
    }
    if (strstr(filename, BIT_SHOBJ) == 0) {
        path = sjoin(filename, BIT_SHOBJ, NULL);
        mprTrace(7, "Probe for native module %s", path);
        if (mprPathExists(path, R_OK)) {
            return path;
        }
    }
    return 0;
}
#endif


/*
    Search for a module "filename" in the modulePath. Return the result in "result"
 */
PUBLIC char *mprSearchForModule(cchar *filename)
{
#if BIT_HAS_DYN_LOAD
    char    *path, *f, *searchPath, *dir, *tok;

    filename = mprNormalizePath(filename);

    /*
        Search for the path directly
     */
    if ((path = probe(filename)) != 0) {
        mprTrace(6, "Found native module %s at %s", filename, path);
        return path;
    }

    /*
        Search in the searchPath
     */
    searchPath = sclone(mprGetModuleSearchPath());
    tok = 0;
    dir = stok(searchPath, MPR_SEARCH_SEP, &tok);
    while (dir && *dir) {
        f = mprJoinPath(dir, filename);
        if ((path = probe(f)) != 0) {
            mprTrace(6, "Found native module %s at %s", filename, path);
            return path;
        }
        dir = stok(0, MPR_SEARCH_SEP, &tok);
    }
#endif /* BIT_HAS_DYN_LOAD */
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/path.c"
 */
/************************************************************************/

/**
    path.c - Path (filename) services.

    This modules provides cross platform path name services.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Defines ***********************************/
/*
    Find the first separator in the path
 */
#if BIT_UNIX_LIKE
    #define firstSep(fs, path)  strchr(path, fs->separators[0])
#else
    #define firstSep(fs, path)  strpbrk(path, fs->separators)
#endif

#define defaultSep(fs)          (fs->separators[0])

/************************************* Code ***********************************/

static MPR_INLINE bool isSep(MprFileSystem *fs, int c) 
{
    char    *separators;

    assert(fs);
    for (separators = fs->separators; *separators; separators++) {
        if (*separators == c)
            return 1;
    }
    return 0;
}


static MPR_INLINE bool hasDrive(MprFileSystem *fs, cchar *path) 
{
    char    *cp, *endDrive;

    assert(fs);
    assert(path);

    if (fs->hasDriveSpecs) {
        cp = firstSep(fs, path);
        endDrive = strchr(path, ':');
        if (endDrive && (cp == NULL || endDrive < cp)) {
            return 1;
        }
    }
    return 0;
}


/*
    Return true if the path is absolute.
    This means the path portion after an optional drive specifier must begin with a directory speparator charcter.
    Cygwin returns true for "/abc" and "C:/abc".
 */
static MPR_INLINE bool isAbsPath(MprFileSystem *fs, cchar *path) 
{
    char    *cp, *endDrive;

    assert(fs);
    assert(path);

    if (path == NULL || *path == '\0') {
        return 0;
    }
    if (fs->hasDriveSpecs) {
        if ((cp = firstSep(fs, path)) != 0) {
            if ((endDrive = strchr(path, ':')) != 0) {
                if (&endDrive[1] == cp) {
                    return 1;
                }
            }
            if (cp == path) {
                return 1;
            }
        }
    } else {
        if (isSep(fs, path[0])) {
            return 1;
        }
    }
    return 0;
}


/*
    Return true if the path is a fully qualified absolute path.
    On windows, this means it must have a drive specifier.
    On cygwin, this means it must not have a drive specifier.
 */
static MPR_INLINE bool isFullPath(MprFileSystem *fs, cchar *path) 
{
    assert(fs);
    assert(path);

#if (BIT_WIN_LIKE || VXWORKS) && !WINCE
{
    char    *cp, *endDrive;

    if (fs->hasDriveSpecs) {
        cp = firstSep(fs, path);
        endDrive = strchr(path, ':');
        if (endDrive && cp && &endDrive[1] == cp) {
            return 1;
        }
        return 0;
    }
}
#endif
    if (isSep(fs, path[0])) {
        return 1;
    }
    return 0;
}


/*
    Return true if the directory is the root directory on a file system
 */
static MPR_INLINE bool isRoot(MprFileSystem *fs, cchar *path) 
{
    char    *cp;

    if (isAbsPath(fs, path)) {
        cp = firstSep(fs, path);
        if (cp && cp[1] == '\0') {
            return 1;
        }
    }
    return 0;
}


static MPR_INLINE char *lastSep(MprFileSystem *fs, cchar *path) 
{
    char    *cp;

    for (cp = (char*) &path[slen(path)] - 1; cp >= path; cp--) {
        if (isSep(fs, *cp)) {
            return cp;
        }
    }
    return 0;
}

/************************************ Code ************************************/
/*
    This copies a file.
 */
PUBLIC int mprCopyPath(cchar *fromName, cchar *toName, int mode)
{
    MprFile     *from, *to;
    ssize       count;
    char        buf[BIT_MAX_BUFFER];

    if ((from = mprOpenFile(fromName, O_RDONLY | O_BINARY, 0)) == 0) {
        mprError("Cannot open %s", fromName);
        return MPR_ERR_CANT_OPEN;
    }
    if ((to = mprOpenFile(toName, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, mode)) == 0) {
        mprError("Cannot open %s", toName);
        return MPR_ERR_CANT_OPEN;
    }
    while ((count = mprReadFile(from, buf, sizeof(buf))) > 0) {
        mprWriteFile(to, buf, count);
    }
    mprCloseFile(from);
    mprCloseFile(to);
    return 0;
}


PUBLIC int mprDeletePath(cchar *path)
{
    MprFileSystem   *fs;

    if (path == NULL || *path == '\0') {
        return MPR_ERR_CANT_ACCESS;
    }
    fs = mprLookupFileSystem(path);
    return fs->deletePath(fs, path);
}


/*
    Return an absolute (normalized) path.
    On CYGWIN, this is a cygwin path with forward-slashes and without drive specs. 
    Use mprGetWinPath for a windows style path with a drive specifier and back-slashes.
 */
PUBLIC char *mprGetAbsPath(cchar *path)
{
    MprFileSystem   *fs;
    char            *result;

    if (path == 0 || *path == '\0') {
        path = ".";
    }
#if BIT_ROM
    return mprNormalizePath(path);
#elif CYGWIN
    {
        ssize   len;
        /*
            cygwin_conf_path has a bug for paths that attempt to address a directory above the root. ie. "../../../.."
            So must convert to a windows path first.
         */
        if (strncmp(path, "../", 3) == 0) {
            path = mprGetWinPath(path);
        }
        if ((len = cygwin_conv_path(CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, path, NULL, 0)) >= 0) {
            /* Len includes room for the null */
            if ((result = mprAlloc(len)) == 0) {
                return 0;
            }
            cygwin_conv_path(CCP_WIN_A_TO_POSIX | CCP_ABSOLUTE, path, result, len);
            if (len > 3 && result[len - 2] == '/' && result[len - 3] != ':') {
                /* Trim trailing "/" */
                result[len - 2] = '\0';
            }
            return result;
        }
    }
#endif
    fs = mprLookupFileSystem(path);
    if (isFullPath(fs, path)) {
        /* Already absolute. On windows, must contain a drive specifier */
        return mprNormalizePath(path);
    }

#if BIT_WIN_LIKE && !WINCE
{
    wchar    buf[BIT_MAX_PATH];
    GetFullPathName(wide(path), sizeof(buf) - 1, buf, NULL);
    buf[TSZ(buf) - 1] = '\0';
    result = mprNormalizePath(multi(buf));
}
#elif VXWORKS
{
    char    *dir;
    if (hasDrive(fs, path)) {
        dir = mprGetCurrentPath();
        result = mprJoinPath(dir, &strchr(path, ':')[1]);

    } else {
        if (isAbsPath(fs, path)) {
            /*
                Path is absolute, but without a drive. Use the current drive.
             */
            dir = mprGetCurrentPath();
            result = mprJoinPath(dir, path);
        } else {
            dir = mprGetCurrentPath();
            result = mprJoinPath(dir, path);
        }
    }
}
#else
{
    char   *dir;
    dir = mprGetCurrentPath();
    result = mprJoinPath(dir, path);
}
#endif
    return result;
}


/*
    Get the directory containing the application executable. Tries to return an absolute path.
 */
PUBLIC char *mprGetAppDir()
{ 
    if (MPR->appDir == 0) {
        MPR->appDir = mprGetPathDir(mprGetAppPath());
    }
    return sclone(MPR->appDir); 
} 


/*
    Get the path for the application executable. Tries to return an absolute path.
 */
PUBLIC char *mprGetAppPath()
{ 
    if (MPR->appPath) {
        return sclone(MPR->appPath);
    }

#if MACOSX
{
    char    path[BIT_MAX_PATH], pbuf[BIT_MAX_PATH];
    uint    size;
    ssize   len;

    size = sizeof(path) - 1;
    if (_NSGetExecutablePath(path, &size) < 0) {
        return mprGetAbsPath(".");
    }
    path[size] = '\0';
    len = readlink(path, pbuf, sizeof(pbuf) - 1);
    if (len < 0) {
        return mprGetAbsPath(path);
    }
    pbuf[len] = '\0';
    MPR->appPath = mprGetAbsPath(pbuf);
}
#elif BIT_BSD_LIKE 
{
    char    pbuf[BIT_MAX_PATH];
    int     len;

    len = readlink("/proc/curproc/file", pbuf, sizeof(pbuf) - 1);
    if (len < 0) {
        return mprGetAbsPath(".");
     }
     pbuf[len] = '\0';
     MPR->appPath = mprGetAbsPath(pbuf);
}
#elif BIT_UNIX_LIKE 
{
    char    pbuf[BIT_MAX_PATH], *path;
    int     len;
#if SOLARIS
    path = sfmt("/proc/%i/path/a.out", getpid()); 
#else
    path = sfmt("/proc/%i/exe", getpid()); 
#endif
    len = readlink(path, pbuf, sizeof(pbuf) - 1);
    if (len < 0) {
        return mprGetAbsPath(".");
    }
    pbuf[len] = '\0';
    MPR->appPath = mprGetAbsPath(pbuf);
}
#elif BIT_WIN_LIKE
{
    wchar    pbuf[BIT_MAX_PATH];

    if (GetModuleFileName(0, pbuf, sizeof(pbuf) - 1) <= 0) {
        return 0;
    }
    MPR->appPath = mprGetAbsPath(multi(pbuf));
}
#else
    if (mprIsPathAbs(MPR->argv[0])) {
        MPR->appPath = sclone(MPR->argv[0]);
    } else {
        MPR->appPath = mprGetCurrentPath();
    }
#endif
    return sclone(MPR->appPath);
}

 
/*
    This will return a fully qualified absolute path for the current working directory.
 */
PUBLIC char *mprGetCurrentPath()
{
    char    dir[BIT_MAX_PATH];

    if (getcwd(dir, sizeof(dir)) == 0) {
        return mprGetAbsPath("/");
    }

#if VXWORKS
{
    MprFileSystem   *fs;
    char            sep[2];

    fs = mprLookupFileSystem(dir);

    /*
        Vx will sometimes just return a drive with no path.
     */
    if (firstSep(fs, dir) == NULL) {
        sep[0] = defaultSep(fs);
        sep[1] = '\0';
        return sjoin(dir, sep, NULL);
    }
}
#elif BIT_WIN_LIKE || CYGWIN
{
    MprFileSystem   *fs;
    fs = mprLookupFileSystem(dir);
    mprMapSeparators(dir, fs->separators[0]);
}
#endif
    return sclone(dir);
}


PUBLIC cchar *mprGetFirstPathSeparator(cchar *path) 
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    return firstSep(fs, path);
}


/*
    Return a pointer into the path at the last path separator or null if none found
 */
PUBLIC cchar *mprGetLastPathSeparator(cchar *path) 
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    return lastSep(fs, path);
}


/*
    Return a path with native separators. This means "\\" on windows and cygwin
 */
PUBLIC char *mprGetNativePath(cchar *path)
{
    return mprTransformPath(path, MPR_PATH_NATIVE_SEP);
}


/*
    Return the last portion of a pathname. The separators are not mapped and the path is not cleaned.
 */
PUBLIC char *mprGetPathBase(cchar *path)
{
    MprFileSystem   *fs;
    char            *cp;

    if (path == 0) {
        return sclone("");
    }
    fs = mprLookupFileSystem(path);
    cp = (char*) lastSep(fs, path);
    if (cp == 0) {
        return sclone(path);
    } 
    if (cp == path) {
        if (cp[1] == '\0') {
            return sclone(path);
        }
    } else if (cp[1] == '\0') {
        return sclone("");
    }
    return sclone(&cp[1]);
}


/*
    Return the last portion of a pathname. The separators are not mapped and the path is not cleaned.
    This returns a reference into the original string
 */
PUBLIC cchar *mprGetPathBaseRef(cchar *path)
{
    MprFileSystem   *fs;
    char            *cp;

    if (path == 0) {
        return sclone("");
    }
    fs = mprLookupFileSystem(path);
    if ((cp = (char*) lastSep(fs, path)) == 0) {
        return path;
    } 
    if (cp == path) {
        if (cp[1] == '\0') {
            return path;
        }
    }
    return &cp[1];
}


/*
    Return the directory portion of a pathname.
 */
PUBLIC char *mprGetPathDir(cchar *path)
{
    MprFileSystem   *fs;
    cchar           *cp, *start;
    char            *result;
    ssize          len;

    assert(path);

    if (path == 0 || *path == '\0') {
        return sclone(path);
    }

    fs = mprLookupFileSystem(path);
    len = slen(path);
    cp = &path[len - 1];
    start = hasDrive(fs, path) ? strchr(path, ':') + 1 : path;

    /*
        Step back over trailing slashes
     */
    while (cp > start && isSep(fs, *cp)) {
        cp--;
    }
    for (; cp > start && !isSep(fs, *cp); cp--) { }

    if (cp == start) {
        if (!isSep(fs, *cp)) {
            /* No slashes found, parent is current dir */
            return sclone(".");
        }
        cp++;
    }
    len = (cp - path);
    result = mprAlloc(len + 1);
    mprMemcpy(result, len + 1, path, len);
    result[len] = '\0';
    return result;
}


/*
    Return the extension portion of a pathname.
    Return the extension without the "."
 */
PUBLIC char *mprGetPathExt(cchar *path)
{
    MprFileSystem  *fs;
    char            *cp;

    if ((cp = srchr(path, '.')) != NULL) {
        fs = mprLookupFileSystem(path);
        /*
            If there is no separator ("/") after the extension, then use it.
         */
        if (firstSep(fs, cp) == 0) {
            return sclone(++cp);
        }
    } 
    return 0;
}


static void manageDirEntry(MprDirEntry *dp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dp->name);
    }
}


#if BIT_ROM
static MprList *getDirFiles(cchar *path)
{
    MprRomFileSystem    *rfs;
    MprRomInode         *ri;
    MprPath             fileInfo;
    MprList             *list;
    MprDirEntry         *dp;
    ssize               len;

    rfs = (MprRomFileSystem*) MPR->fileSystem;
    list = mprCreateList(256, 0);
    len = slen(path);

    for (ri = rfs->romInodes; ri->path; ri++) {
        if (!sstarts(ri->path, path) || !schr(&ri->path[len], '/')) {
            continue;
        }
        fileInfo.isDir = (ri->size == 0);
        fileInfo.isLink = 0;
        if ((dp = mprAllocObj(MprDirEntry, manageDirEntry)) == 0) {
            return list;
        }
        dp->name = sclone(ri->path);
        dp->size = ri->size;
        dp->isDir = (ri->data == 0);
        dp->isLink = 0;
        dp->lastModified = 0;
        mprAddItem(list, &ri->path[len]);
    }
    return list;
}

#else /* !BIT_ROM */
/*
    This returns a list of MprDirEntry objects
 */
#if BIT_WIN_LIKE
static MprList *getDirFiles(cchar *dir)
{
    HANDLE          h;
    MprDirEntry     *dp;
    MprPath         fileInfo;
    MprList         *list;
    cchar           *seps;
    char            *path, pbuf[BIT_MAX_PATH];
#if WINCE
    WIN32_FIND_DATAA findData;
#else
    WIN32_FIND_DATA findData;
#endif

    list = mprCreateList(-1, 0);
    dp = 0;

    if ((path = mprJoinPath(dir, "*.*")) == 0) {
        return list;
    }
    seps = mprGetPathSeparators(dir);

    h = FindFirstFile(wide(path), &findData);
    if (h == INVALID_HANDLE_VALUE) {
        return list;
    }

    do {
        if (findData.cFileName[0] == '.' && (findData.cFileName[1] == '\0' || findData.cFileName[1] == '.')) {
            continue;
        }
        if ((dp = mprAlloc(sizeof(MprDirEntry))) == 0) {
            return list;
        }
        dp->name = awtom(findData.cFileName, 0);
        if (dp->name == 0) {
            return list;
        }
        /* dp->lastModified = (uint) findData.ftLastWriteTime.dwLowDateTime; */

        if (fmt(pbuf, sizeof(pbuf), "%s%c%s", dir, seps[0], dp->name) < 0) {
            dp->lastModified = 0;
        } else {
            mprGetPathInfo(pbuf, &fileInfo);
            dp->lastModified = fileInfo.mtime;
        }
        dp->isDir = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        dp->isLink = 0;

#if FUTURE_64_BIT
        if (findData.nFileSizeLow < 0) {
            dp->size = (((uint64) findData.nFileSizeHigh) * INT64(4294967296)) + (4294967296L - 
                (uint64) findData.nFileSizeLow);
        } else {
            dp->size = (((uint64) findData.nFileSizeHigh * INT64(4294967296))) + (uint64) findData.nFileSizeLow;
        }
#else
        dp->size = (uint) findData.nFileSizeLow;
#endif
        mprAddItem(list, dp);
    } while (FindNextFile(h, &findData) != 0);

    FindClose(h);
    return list;
}

#else /* !WIN */
static MprList *getDirFiles(cchar *path)
{
    DIR             *dir;
    MprPath         fileInfo;
    MprList         *list;
    struct dirent   *dirent;
    MprDirEntry     *dp;
    char            *fileName;
    int             rc;

    list = mprCreateList(256, 0);
    if ((dir = opendir((char*) path)) == 0) {
        return list;
    }
    while ((dirent = readdir(dir)) != 0) {
        if (dirent->d_name[0] == '.' && (dirent->d_name[1] == '\0' || dirent->d_name[1] == '.')) {
            continue;
        }
        fileName = mprJoinPath(path, dirent->d_name);
        /* workaround for if target of symlink does not exist */
        fileInfo.isLink = 0;
        fileInfo.isDir = 0;
        rc = mprGetPathInfo(fileName, &fileInfo);
        if ((dp = mprAllocObj(MprDirEntry, manageDirEntry)) == 0) {
            return list;
        }
        dp->name = sclone(dirent->d_name);
        if (dp->name == 0) {
            return list;
        }
        if (rc == 0 || fileInfo.isLink) {
            dp->lastModified = fileInfo.mtime;
            dp->size = fileInfo.size;
            dp->isDir = fileInfo.isDir;
            dp->isLink = fileInfo.isLink;
        } else {
            dp->lastModified = 0;
            dp->size = 0;
            dp->isDir = 0;
            dp->isLink = 0;
        }
        mprAddItem(list, dp);
    }
    closedir(dir);
    return list;
}
#endif /* !WIN */
#endif /* !BIT_ROM */

/*
    Find files in the directory "dir". If base is set, use that as the prefix for returned files.
    Returns a list of MprDirEntry objects.
 */
static MprList *findFiles(MprList *list, cchar *dir, cchar *base, int flags)
{
    MprDirEntry     *dp;
    MprList         *files;
    char            *name;
    int             next;

    if ((files = getDirFiles(dir)) == 0) {
        return 0;
    }
    for (next = 0; (dp = mprGetNextItem(files, &next)) != 0; ) {
        if (dp->name[0] == '.') {
            if (dp->name[1] == '\0' || (dp->name[1] == '.' && dp->name[2] == '\0')) {
                continue;
            }
            if (!(flags & MPR_PATH_INC_HIDDEN)) {
                continue;
            }
        }
        name = dp->name;
        dp->name = mprJoinPath(base, name);

        if (!(flags & MPR_PATH_DEPTH_FIRST) && !(dp->isDir && flags & MPR_PATH_NODIRS)) {
            mprAddItem(list, dp);
        }
        if (dp->isDir) {
            if (flags & MPR_PATH_DESCEND) {
                findFiles(list, mprJoinPath(dir, name), mprJoinPath(base, name), flags);
            } 
        }
        if ((flags & MPR_PATH_DEPTH_FIRST) && (!(dp->isDir && flags & MPR_PATH_NODIRS))) {
            mprAddItem(list, dp);
        }
    }
    return list;
}


#if LINUX
static int sortFiles(MprDirEntry **dp1, MprDirEntry **dp2)
{
    return strcmp((*dp1)->name, (*dp2)->name);
}
#endif


/*
    Get the files in a directory. Returns a list of MprDirEntry objects.

    MPR_PATH_DESCEND        to traverse subdirectories
    MPR_PATH_DEPTH_FIRST    to do a depth-first traversal
    MPR_PATH_INC_HIDDEN     to include hidden files
    MPR_PATH_NODIRS         to exclude subdirectories
    MPR_PATH_RELATIVE       to return paths relative to the initial directory
 */
PUBLIC MprList *mprGetPathFiles(cchar *dir, int flags)
{
    MprList *list;
    cchar   *base;

    if (dir == 0 || *dir == '\0') {
        dir = ".";
    }
    base = (flags & MPR_PATH_RELATIVE) ? 0 : dir;
    if ((list = findFiles(mprCreateList(-1, 0), dir, base, flags)) == 0) {
        return 0;
    }
#if LINUX
    /* Linux returns directories not sorted */
    mprSortList(list, (MprSortProc) sortFiles, 0);
#endif
    return list;
}


/*
    Return the first directory of a pathname
 */
PUBLIC char *mprGetPathFirstDir(cchar *path)
{
    MprFileSystem   *fs;
    cchar           *cp;
    int             len;

    assert(path);

    fs = mprLookupFileSystem(path);
    if (isAbsPath(fs, path)) {
        len = hasDrive(fs, path) ? 2 : 1;
        return snclone(path, len);
    } else {
        if ((cp = firstSep(fs, path)) != 0) {
            return snclone(path, cp - path);
        }
        return sclone(path);
    }
}


PUBLIC int mprGetPathInfo(cchar *path, MprPath *info)
{
    MprFileSystem  *fs;

    fs = mprLookupFileSystem(path);
    return fs->getPathInfo(fs, path, info);
}


PUBLIC char *mprGetPathLink(cchar *path)
{
    MprFileSystem  *fs;

    fs = mprLookupFileSystem(path);
    return fs->getPathLink(fs, path);
}


/*
    GetPathParent is smarter than GetPathDir which operates purely textually on the path. GetPathParent will convert
    relative paths to absolute to determine the parent directory.
 */
PUBLIC char *mprGetPathParent(cchar *path)
{
    MprFileSystem   *fs;
    char            *dir;

    fs = mprLookupFileSystem(path);

    if (path == 0 || path[0] == '\0') {
        return mprGetAbsPath(".");
    }
    if (firstSep(fs, path) == NULL) {
        /*
            No parents in the path, so convert to absolute
         */
        dir = mprGetAbsPath(path);
        return mprGetPathDir(dir);
    }
    return mprGetPathDir(path);
}


PUBLIC char *mprGetPortablePath(cchar *path)
{
    char    *result, *cp;

    result = mprTransformPath(path, 0);
    for (cp = result; *cp; cp++) {
        if (*cp == '\\') {
            *cp = '/';
        }
    }
    return result;
}


/*
    This returns a path relative to the current working directory for the given path
 */
PUBLIC char *mprGetRelPath(cchar *destArg, cchar *originArg)
{
    MprFileSystem   *fs;
    char            originBuf[BIT_MAX_FNAME], *cp, *result, *dest, *lastcp, *origin, *op, *lastop;
    int             originSegments, i, commonSegments, sep;

    fs = mprLookupFileSystem(destArg);
    
    if (destArg == 0 || *destArg == '\0') {
        return sclone(".");
    }
    dest = mprNormalizePath(destArg);

    if (!isAbsPath(fs, dest) && (originArg == 0 || *originArg == '\0')) {
        return dest;
    }
    sep = (cp = firstSep(fs, dest)) ? *cp : defaultSep(fs);
    
    if (originArg == 0 || *originArg == '\0') {
        /*
            Get the working directory. Ensure it is null terminated and leave room to append a trailing separators
            On cygwin, this will be a cygwin style path (starts with "/" and no drive specifier).
         */
        if (getcwd(originBuf, sizeof(originBuf)) == 0) {
            strcpy(originBuf, ".");
        }
        originBuf[sizeof(originBuf) - 2] = '\0';
        origin = originBuf;
    } else {
        origin = mprGetAbsPath(originArg);
    }
    dest = mprGetAbsPath(dest);

    /*
        Count segments in origin working directory. Ignore trailing separators.
     */
    for (originSegments = 0, cp = origin; *cp; cp++) {
        if (isSep(fs, *cp) && cp[1]) {
            originSegments++;
        }
    }

    /*
        Find portion of dest that matches the origin directory, if any. Start at -1 because matching root doesn't count.
     */
    commonSegments = -1;
    for (lastop = op = origin, lastcp = cp = dest; *op && *cp; op++, cp++) {
        if (isSep(fs, *op)) {
            lastop = op + 1;
            if (isSep(fs, *cp)) {
                lastcp = cp + 1;
                commonSegments++;
            }
        } else if (fs->caseSensitive) {
            if (*op != *cp) {
                break;
            }
        } else if (*op != *cp && tolower((uchar) *op) != tolower((uchar) *cp)) {
            break;
        }
    }
    assert(commonSegments >= 0);

    if (*cp && *op) {
        op = lastop;
        cp = lastcp;
    }

    /*
        Add one more segment if the last segment matches. Handle trailing separators
     */
    if ((isSep(fs, *op) || *op == '\0') && (isSep(fs, *cp) || *cp == '\0')) {
        commonSegments++;
    }
    if (isSep(fs, *cp)) {
        cp++;
    }
    
    op = result = mprAlloc(originSegments * 3 + slen(dest) + 2);
    for (i = commonSegments; i < originSegments; i++) {
        *op++ = '.';
        *op++ = '.';
        *op++ = defaultSep(fs);
    }
    if (*cp) {
        strcpy(op, cp);
    } else if (op > result) {
        /*
            Cleanup trailing separators ("../" is the end of the new path)
         */
        op[-1] = '\0';
    } else {
        strcpy(result, ".");
    }
    mprMapSeparators(result, sep);
    return result;
}


/*
    Get a temporary file name. The file is created in the system temp location.
 */
PUBLIC char *mprGetTempPath(cchar *tempDir)
{
    MprFile         *file;
    char            *dir, *path;
    int             i, now;
    static int      tempSeed = 0;

    if (tempDir == 0 || *tempDir == '\0') {
#if WINCE
        dir = sclone("/Temp");
#elif BIT_WIN_LIKE
{
        MprFileSystem   *fs;
        fs = mprLookupFileSystem(tempDir ? tempDir : (cchar*) "/");
        dir = sclone(getenv("TEMP"));
        mprMapSeparators(dir, defaultSep(fs));
}
#elif VXWORKS
        dir = sclone(".");
#else
        dir = sclone("/tmp");
#endif
    } else {
        dir = sclone(tempDir);
    }
    now = ((int) mprGetTime() & 0xFFFF) % 64000;
    file = 0;
    path = 0;
    for (i = 0; i < 128; i++) {
        path = sfmt("%s/MPR_%d_%d_%d.tmp", dir, getpid(), now, ++tempSeed);
        file = mprOpenFile(path, O_CREAT | O_EXCL | O_BINARY, 0664);
        if (file) {
            mprCloseFile(file);
            break;
        }
    }
    if (file == 0) {
        return 0;
    }
    return path;
}


/*
    Return a windows path.
    On CYGWIN, this is a cygwin path without drive specs.
 */
PUBLIC char *mprGetWinPath(cchar *path)
{
    char            *result;

    if (path == 0 || *path == '\0') {
        path = ".";
    }
#if BIT_ROM
    result = mprNormalizePath(path);
#elif CYGWIN
{
    ssize   len;
    if ((len = cygwin_conv_path(CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE, path, NULL, 0)) >= 0) {
        if ((result = mprAlloc(len)) == 0) {
            return 0;
        }
        cygwin_conv_path(CCP_POSIX_TO_WIN_A | CCP_ABSOLUTE, path, result, len);
        return result;
    } else {
        result = mprGetAbsPath(path);
    }
}
#else
    result = mprNormalizePath(path);
    mprMapSeparators(result, '\\');
#endif
    return result;
}


PUBLIC bool mprIsPathAbs(cchar *path)
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    return isAbsPath(fs, path);
}


PUBLIC bool mprIsPathDir(cchar *path)
{
    MprPath     info;

    return (mprGetPathInfo(path, &info) == 0 && info.isDir);
}


PUBLIC bool mprIsPathRel(cchar *path)
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    return !isAbsPath(fs, path);
}


PUBLIC bool mprIsPathSeparator(cchar *path, cchar c)
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    return isSep(fs, c);
}


/*
    Join paths. Returns a joined (normalized) path.
    If other is absolute, then return other. If other is null, empty or "." then return path.
    The separator is chosen to match the first separator found in either path. If none, it uses the default separator.
 */
PUBLIC char *mprJoinPath(cchar *path, cchar *other)
{
    MprFileSystem   *fs;
    char            *join, *drive, *cp;
    int             sep;

    fs = mprLookupFileSystem(path);
    if (other == NULL || *other == '\0' || strcmp(other, ".") == 0) {
        return sclone(path);
    }
    if (isAbsPath(fs, other)) {
        if (fs->hasDriveSpecs && !isFullPath(fs, other) && isFullPath(fs, path)) {
            /*
                Other is absolute, but without a drive. Use the drive from path.
             */
            drive = sclone(path);
            if ((cp = strchr(drive, ':')) != 0) {
                *++cp = '\0';
            }
            return sjoin(drive, other, NULL);
        } else {
            return mprNormalizePath(other);
        }
    }
    if (path == NULL || *path == '\0') {
        return mprNormalizePath(other);
    }
    if ((cp = firstSep(fs, path)) != 0) {
        sep = *cp;
    } else if ((cp = firstSep(fs, other)) != 0) {
        sep = *cp;
    } else {
        sep = defaultSep(fs);
    }
    if ((join = sfmt("%s%c%s", path, sep, other)) == 0) {
        return 0;
    }
    return mprNormalizePath(join);
}


/*
    Join an extension to a path. If path already has an extension, this call does nothing.
    The extension should not have a ".", but this routine is tolerant if it does.
 */
PUBLIC char *mprJoinPathExt(cchar *path, cchar *ext)
{
    MprFileSystem   *fs;
    char            *cp;

    fs = mprLookupFileSystem(path);
    if (ext == NULL || *ext == '\0') {
        return sclone(path);
    }
    cp = srchr(path, '.');
    if (cp && firstSep(fs, cp) == 0) {
        return sclone(path);
    }
    if (ext[0] == '.') {
        return sjoin(path, ext, NULL);
    } else {
        return sjoin(path, ".", ext, NULL);
    }
}


/*
    Make a directory with all necessary intervening directories.
 */
PUBLIC int mprMakeDir(cchar *path, int perms, int owner, int group, bool makeMissing)
{
    MprFileSystem   *fs;
    char            *parent;
    int             rc;

    fs = mprLookupFileSystem(path);

    if (mprPathExists(path, X_OK)) {
        return 0;
    }
    if (fs->makeDir(fs, path, perms, owner, group) == 0) {
        return 0;
    }
    if (makeMissing && !isRoot(fs, path)) {
        parent = mprGetPathParent(path);
        if ((rc = mprMakeDir(parent, perms, owner, group, makeMissing)) < 0) {
            return rc;
        }
        return fs->makeDir(fs, path, perms, owner, group);
    }
    return MPR_ERR_CANT_CREATE;
}


PUBLIC int mprMakeLink(cchar *path, cchar *target, bool hard)
{
    MprFileSystem   *fs;

    fs = mprLookupFileSystem(path);
    if (mprPathExists(path, X_OK)) {
        return 0;
    }
    return fs->makeLink(fs, path, target, hard);
}


/*
    Normalize a path to remove redundant "./" and cleanup "../" and make separator uniform. Does not make an abs path.
    It does not map separators, change case, nor add drive specifiers.
 */
PUBLIC char *mprNormalizePath(cchar *pathArg)
{
    MprFileSystem   *fs;
    char            *path, *sp, *dp, *mark, **segments;
    ssize           len;
    int             addSep, i, segmentCount, hasDot, last, sep;

    if (pathArg == 0 || *pathArg == '\0') {
        return sclone("");
    }
    fs = mprLookupFileSystem(pathArg);

    /*
        Allocate one spare byte incase we need to break into segments. If so, will add a trailing "/" to make 
        parsing easier later.
     */
    len = slen(pathArg);
    if ((path = mprAlloc(len + 2)) == 0) {
        return NULL;
    }
    strcpy(path, pathArg);
    sep = (sp = firstSep(fs, path)) ? *sp : defaultSep(fs);

    /*
        Remove multiple path separators. Check if we have any "." characters and count the number of path segments
        Map separators to the first separator found
     */
    hasDot = segmentCount = 0;
    for (sp = dp = path; *sp; ) {
        if (isSep(fs, *sp)) {
            *sp = sep;
            segmentCount++;
            while (isSep(fs, sp[1])) {
                sp++;
            }
        } 
        if (*sp == '.') {
            hasDot++;
        }
        *dp++ = *sp++;
    }
    *dp = '\0';
    if (!sep) {
        sep = defaultSep(fs);
    }
    if (!hasDot && segmentCount == 0) {
        if (fs->hasDriveSpecs) {
            last = path[slen(path) - 1];
            if (last == ':') {
                path = sjoin(path, ".", NULL);
            }
        }
        return path;
    }

    if (dp > path && !isSep(fs, dp[-1])) {
        *dp++ = sep;
        *dp = '\0';
        segmentCount++;
    }

    /*
        Have dots to process so break into path segments. Add one incase we need have an absolute path with a drive-spec.
     */
    assert(segmentCount > 0);
    if ((segments = mprAlloc(sizeof(char*) * (segmentCount + 1))) == 0) {
        return NULL;
    }

    /*
        NOTE: The root "/" for absolute paths will be stored as empty.
     */
    len = 0;
    for (i = 0, mark = sp = path; *sp; sp++) {
        if (isSep(fs, *sp)) {
            *sp = '\0';
            if (*mark == '.' && mark[1] == '\0' && segmentCount > 1) {
                /* Remove "."  However, preserve lone "." */
                mark = sp + 1;
                segmentCount--;
                continue;
            }
            if (*mark == '.' && mark[1] == '.' && mark[2] == '\0' && i > 0 && strcmp(segments[i-1], "..") != 0) {
                /* Erase ".." and previous segment */
                if (*segments[i - 1] == '\0' ) {
                    assert(i == 1);
                    /* Previous segement is "/". Prevent escape from root */
                    segmentCount--;
                } else {
                    i--;
                    segmentCount -= 2;
                }
                assert(segmentCount >= 0);
                mark = sp + 1;
                continue;
            }
            segments[i++] = mark;
            len += (sp - mark);
#if KEEP
            if (i == 1 && segmentCount == 1 && fs->hasDriveSpecs && strchr(mark, ':') != 0) {
                /*
                    Normally we truncate a trailing "/", but this is an absolute path with a drive spec (c:/). 
                 */
                segments[i++] = "";
                segmentCount++;
            }
#endif
            mark = sp + 1;
        }
    }

    if (--sp > mark) {
        segments[i++] = mark;
        len += (sp - mark);
    }
    assert(i <= segmentCount);
    segmentCount = i;

    if (segmentCount <= 0) {
        return sclone(".");
    }

    addSep = 0;
    sp = segments[0];
    if (fs->hasDriveSpecs && *sp != '\0') {
        last = sp[slen(sp) - 1];
        if (last == ':') {
            /* This matches an original path of: "c:/" but not "c:filename" */
            addSep++;
        }
    }
#if BIT_WIN_LIKE
    if (strcmp(segments[segmentCount - 1], " ") == 0) {
        segmentCount--;
    }
#endif
    if ((path = mprAlloc(len + segmentCount + 1)) == 0) {
        return NULL;
    }
    assert(segmentCount > 0);

    /*
        First segment requires special treatment due to drive specs
     */
    dp = path;
    strcpy(dp, segments[0]);
    dp += slen(segments[0]);

    if (segmentCount == 1 && (addSep || (*segments[0] == '\0'))) {
        *dp++ = sep;
    }

    for (i = 1; i < segmentCount; i++) {
        *dp++ = sep;
        strcpy(dp, segments[i]);
        dp += slen(segments[i]);
    }
    *dp = '\0';
    return path;
}


PUBLIC void mprMapSeparators(char *path, int separator)
{
    MprFileSystem   *fs;
    char            *cp;

    fs = mprLookupFileSystem(path);
    for (cp = path; *cp; cp++) {
        if (isSep(fs, *cp)) {
            *cp = separator;
        }
    }
}


PUBLIC bool mprPathExists(cchar *path, int omode)
{
    MprFileSystem  *fs;

    if (path == 0 || *path == '\0') {
        return 0;
    }
    fs = mprLookupFileSystem(path);
    return fs->accessPath(fs, path, omode);
}


PUBLIC char *mprReadPathContents(cchar *path, ssize *lenp)
{
    MprFile     *file;
    MprPath     info;
    ssize       len;
    char        *buf;

    if ((file = mprOpenFile(path, O_RDONLY | O_BINARY, 0)) == 0) {
        mprError("Cannot open %s", path);
        return 0;
    }
    if (mprGetPathInfo(path, &info) < 0) {
        mprCloseFile(file);
        return 0;
    }
    len = (ssize) info.size;
    if ((buf = mprAlloc(len + 1)) == 0) {
        mprCloseFile(file);
        return 0;
    }
    if (mprReadFile(file, buf, len) != len) {
        mprCloseFile(file);
        return 0;
    }
    buf[len] = '\0';
    if (lenp) {
        *lenp = len;
    }
    mprCloseFile(file);
    return buf;
}


PUBLIC int mprRenamePath(cchar *from, cchar *to)
{
    return rename(from, to);
}


PUBLIC char *mprReplacePathExt(cchar *path, cchar *ext)
{
    return mprJoinPathExt(mprTrimPathExt(path), ext);
}


/*
    Resolve paths in the neighborhood of this path. Resolve operates like join, except that it joins the 
    given paths to the directory portion of the current ("this") path. For example: 
    Path("/usr/bin/ejs/bin").resolve("lib") will return "/usr/lib/ejs/lib". i.e. it will return the
    sibling directory "lib".

    Resolve operates by determining a virtual current directory for this Path object. It then successively 
    joins the given paths to the directory portion of the current result. If the next path is an absolute path, 
    it is used unmodified.  The effect is to find the given paths with a virtual current directory set to the 
    directory containing the prior path.

    Resolve is useful for creating paths in the region of the current path and gracefully handles both 
    absolute and relative path segments.

    Returns a joined (normalized) path.
    If path is absolute, then return path. If path is null, empty or "." then return path.
 */
PUBLIC char *mprResolvePath(cchar *base, cchar *path)
{
    MprFileSystem   *fs;
    char            *join, *drive, *cp, *dir;

    fs = mprLookupFileSystem(base);
    if (path == NULL || *path == '\0' || strcmp(path, ".") == 0) {
        return sclone(base);
    }
    if (isAbsPath(fs, path)) {
        if (fs->hasDriveSpecs && !isFullPath(fs, path) && isFullPath(fs, base)) {
            /*
                Other is absolute, but without a drive. Use the drive from base.
             */
            drive = sclone(base);
            if ((cp = strchr(drive, ':')) != 0) {
                *++cp = '\0';
            }
            return sjoin(drive, path, NULL);
        }
        return mprNormalizePath(path);
    }
    if (base == NULL || *base == '\0') {
        return mprNormalizePath(path);
    }
    dir = mprGetPathDir(base);
    if ((join = sfmt("%s/%s", dir, path)) == 0) {
        return 0;
    }
    return mprNormalizePath(join);
}


/*
    Compare two file path to determine if they point to the same file.
 */
PUBLIC int mprSamePath(cchar *path1, cchar *path2)
{
    MprFileSystem   *fs;
    cchar           *p1, *p2;

    fs = mprLookupFileSystem(path1);

    /*
        Convert to absolute (normalized) paths to compare. 
        MOB - resolve symlinks.
     */
    if (!isFullPath(fs, path1)) {
        path1 = mprGetAbsPath(path1);
    } else {
        path1 = mprNormalizePath(path1);
    }
    if (!isFullPath(fs, path2)) {
        path2 = mprGetAbsPath(path2);
    } else {
        path2 = mprNormalizePath(path2);
    }
    if (fs->caseSensitive) {
        for (p1 = path1, p2 = path2; *p1 && *p2; p1++, p2++) {
            if (*p1 != *p2 && !(isSep(fs, *p1) && isSep(fs, *p2))) {
                break;
            }
        }
    } else {
        for (p1 = path1, p2 = path2; *p1 && *p2; p1++, p2++) {
            if (tolower((uchar) *p1) != tolower((uchar) *p2) && !(isSep(fs, *p1) && isSep(fs, *p2))) {
                break;
            }
        }
    }
    return (*p1 == *p2);
}


/*
    Compare two file path to determine if they point to the same file.
 */
PUBLIC int mprSamePathCount(cchar *path1, cchar *path2, ssize len)
{
    MprFileSystem   *fs;
    cchar           *p1, *p2;

    fs = mprLookupFileSystem(path1);

    /*
        Convert to absolute paths to compare. 
        MOB - resolve symlinks.
     */
    if (!isFullPath(fs, path1)) {
        path1 = mprGetAbsPath(path1);
    }
    if (!isFullPath(fs, path2)) {
        path2 = mprGetAbsPath(path2);
    }
    if (fs->caseSensitive) {
        for (p1 = path1, p2 = path2; *p1 && *p2 && len > 0; p1++, p2++, len--) {
            if (*p1 != *p2 && !(isSep(fs, *p1) && isSep(fs, *p2))) {
                break;
            }
        }
    } else {
        for (p1 = path1, p2 = path2; *p1 && *p2 && len > 0; p1++, p2++, len--) {
            if (tolower((uchar) *p1) != tolower((uchar) *p2) && !(isSep(fs, *p1) && isSep(fs, *p2))) {
                break;
            }
        }
    }
    return len == 0;
}


PUBLIC void mprSetAppPath(cchar *path)
{ 
    MPR->appPath = sclone(path);
    MPR->appDir = mprGetPathDir(MPR->appPath);
}


static char* checkPath(cchar *path, int flags) 
{
    MprPath     info;
    int         access;

    access = (flags & (MPR_SEARCH_EXE | MPR_SEARCH_DIR)) ? X_OK : R_OK;

    if (mprPathExists(path, access)) {
        mprGetPathInfo(path, &info);
        if (flags & MPR_SEARCH_DIR && info.isDir) {
            mprTrace(4, "mprSearchForFile: found %s", path);
            return sclone(path);
        }
        if (info.isReg) {
            mprTrace(4, "mprSearchForFile: found %s", path);
            return sclone(path);
        }
    }
    return 0;
}


PUBLIC char *mprSearchPath(cchar *file, int flags, cchar *search, ...)
{
    va_list     args;
    char        *result, *path, *dir, *nextDir, *tok;

    va_start(args, search);

    mprTrace(6, "mprSearchForFile: %s", file);
    if ((result = checkPath(file, flags)) != 0) {
        return result;
    }
    if ((flags & MPR_SEARCH_EXE) && *BIT_EXE) {
        if ((result = checkPath(mprJoinPathExt(file, BIT_EXE), flags)) != 0) {
            return result;
        }
    }
    for (nextDir = (char*) search; nextDir; nextDir = va_arg(args, char*)) {
        tok = NULL;
        nextDir = sclone(nextDir);
        dir = stok(nextDir, MPR_SEARCH_SEP, &tok);
        while (dir && *dir) {
            mprTrace(6, "mprSearchForFile: %s in search path %s", file, dir);
            path = mprJoinPath(dir, file);
            if ((result = checkPath(path, flags)) != 0) {
                return mprNormalizePath(result);
            }
            if ((flags & MPR_SEARCH_EXE) && *BIT_EXE) {
                if ((result = checkPath(mprJoinPathExt(path, BIT_EXE), flags)) != 0) {
                    return mprNormalizePath(result);
                }
            }
            dir = stok(0, MPR_SEARCH_SEP, &tok);
        }
    }
    va_end(args);
    return 0;
}


/*
    This normalizes a path. Returns a normalized path according to flags. Default is absolute. 
    if MPR_PATH_NATIVE_SEP is specified in the flags, map separators to the native format.
 */
PUBLIC char *mprTransformPath(cchar *path, int flags)
{
    char    *result;

#if CYGWIN
    if (flags & MPR_PATH_ABS) {
        if (flags & MPR_PATH_WIN) {
            result = mprGetWinPath(path);
        } else {
            result = mprGetAbsPath(path);
        }
#else
    if (flags & MPR_PATH_ABS) {
        result = mprGetAbsPath(path);

#endif
    } else if (flags & MPR_PATH_REL) {
        result = mprGetRelPath(path, 0);

    } else {
        result = mprNormalizePath(path);
    }
    if (flags & MPR_PATH_NATIVE_SEP) {
#if BIT_WIN_LIKE
        mprMapSeparators(result, '\\');
#elif CYGWIN
        mprMapSeparators(result, '/');
#endif
    }
    return result;
}


PUBLIC char *mprTrimPathExt(cchar *path)
{
    MprFileSystem   *fs;
    char            *cp, *result;

    fs = mprLookupFileSystem(path);
    result = sclone(path);
    if ((cp = srchr(result, '.')) != NULL) {
        if (firstSep(fs, cp) == 0) {
            *cp = '\0';
        }
    } 
    return result;
}


PUBLIC char *mprTrimPathDrive(cchar *path)
{
    MprFileSystem   *fs;
    char            *cp, *endDrive;

    fs = mprLookupFileSystem(path);
    if (fs->hasDriveSpecs) {
        cp = firstSep(fs, path);
        endDrive = strchr(path, ':');
        if (endDrive && (cp == NULL || endDrive < cp)) {
            return sclone(++endDrive);
        }
    }
    return sclone(path);
}


PUBLIC ssize mprWritePathContents(cchar *path, cchar *buf, ssize len, int mode)
{
    MprFile     *file;

    if (mode == 0) {
        mode = 0644;
    }
    if (len < 0) {
        len = slen(buf);
    }
    if ((file = mprOpenFile(path, O_WRONLY | O_TRUNC | O_CREAT | O_BINARY, mode)) == 0) {
        mprError("Cannot open %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    if (mprWriteFile(file, buf, len) != len) {
        mprError("Cannot write %s", path);
        mprCloseFile(file);
        return MPR_ERR_CANT_WRITE;
    }
    mprCloseFile(file);
    return len;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/poll.c"
 */
/************************************************************************/

/**
    poll.c - Wait for I/O by using poll on unix like systems.

    This module augments the mprWait wait services module by providing poll() based waiting support.
    Also see mprAsyncSelectWait and mprSelectWait. This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if MPR_EVENT_POLL
/********************************** Forwards **********************************/

static void serviceIO(MprWaitService *ws, struct pollfd *fds, int count);

/************************************ Code ************************************/

PUBLIC int mprCreateNotifierService(MprWaitService *ws)
{
    struct pollfd   *pollfd;
    int             fd;

    ws->fdsCount = 0;
    ws->fdMax = MPR_FD_MIN;
    ws->handlerMax = MPR_FD_MIN;

    ws->fds = mprAllocZeroed(sizeof(struct pollfd) * ws->fdMax);
    ws->handlerMap = mprAllocZeroed(sizeof(MprWaitHandler*) * ws->handlerMax);
    if (ws->fds == 0 || ws->handlerMap == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Initialize the "wakeup" pipe. This is used to wakeup the service thread if other threads need to wait for I/O.
     */
    if (pipe(ws->breakPipe) < 0) {
        mprError("Cannot open breakout pipe");
        return MPR_ERR_CANT_INITIALIZE;
    }
    fcntl(ws->breakPipe[0], F_SETFL, fcntl(ws->breakPipe[0], F_GETFL) | O_NONBLOCK);
    fcntl(ws->breakPipe[1], F_SETFL, fcntl(ws->breakPipe[1], F_GETFL) | O_NONBLOCK);

    fd = ws->breakPipe[MPR_READ_PIPE];
    pollfd = &ws->fds[ws->fdsCount];
    pollfd->fd = ws->breakPipe[MPR_READ_PIPE];
    pollfd->events = POLLIN | POLLHUP;
    ws->fdsCount++;
    return 0;
}


PUBLIC void mprManagePoll(MprWaitService *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->fds);
        mprMark(ws->pollFds);

    } else if (flags & MPR_MANAGE_FREE) {
        if (ws->breakPipe[0] >= 0) {
            close(ws->breakPipe[0]);
        }
        if (ws->breakPipe[1] >= 0) {
            close(ws->breakPipe[1]);
        }
    }
}


static int growFds(MprWaitService *ws)
{
    ws->fdMax *= 2;
    if ((ws->fds = mprRealloc(ws->fds, sizeof(struct pollfd) * ws->fdMax)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


static int growHandlers(MprWaitService *ws, int fd)
{
    ws->handlerMax = fd + 1;
    if ((ws->handlerMap = mprRealloc(ws->handlerMap, sizeof(MprWaitHandler*) * ws->handlerMax)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


PUBLIC int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask)
{
    struct pollfd   *pollfd;
    int             fd, index;

    fd = wp->fd;

    lock(ws);
    if (wp->desiredMask != mask) {
        index = wp->notifierIndex;
        pollfd = 0;
        if (mask) {
            if (index < 0) {
                if (ws->fdsCount >= ws->fdMax && growFds(ws) < 0) {
                    unlock(ws);
                    assert(!MPR_ERR_MEMORY);
                    return MPR_ERR_MEMORY;
                }
                if (fd >= ws->handlerMax && growHandlers(ws, fd) < 0) {
                    unlock(ws);
                    return MPR_ERR_MEMORY;
                }
                assert(fd < ws->handlerMax);
                assert(ws->handlerMap[fd] == 0 || ws->handlerMap[fd] == wp);
                ws->handlerMap[fd] = wp;
                index = wp->notifierIndex = ws->fdsCount++;
                pollfd = &ws->fds[index];
                pollfd->fd = fd;
            } else {
                pollfd = &ws->fds[index];
            }
        } else {
            /* Removal */
            if (index >= 0) {
                pollfd = &ws->fds[index];
            }
        }
        if (pollfd) {
            pollfd->events = 0;
            if (mask & MPR_READABLE) {
                pollfd->events |= POLLIN | POLLHUP;
            }
            if (mask & MPR_WRITABLE) {
                pollfd->events |= POLLOUT | POLLHUP;
            }
            wp->desiredMask = mask;
        }
        /*
            Compact on removal. If not the last entry, copy last poll entry to replace the deleted fd.
         */
        if (mask == 0) {
            if (index >= 0 && --ws->fdsCount > index) {
                ws->fds[index] = ws->fds[ws->fdsCount];
                ws->handlerMap[ws->fds[index].fd]->notifierIndex = index;
                ws->fds[ws->fdsCount].fd = -1;
            }
            ws->handlerMap[wp->fd] = 0;
            wp->notifierIndex = -1;
            wp->desiredMask = 0;
        }
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    unlock(ws);
    return 0;
}


/*
    Wait for I/O on a single file descriptor. Return a mask of events found. Mask is the events of interest.
    timeout is in milliseconds.
 */
PUBLIC int mprWaitForSingleIO(int fd, int mask, MprTicks timeout)
{
    struct pollfd   fds[1];
    int             rc, result;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
    fds[0].fd = fd;
    fds[0].events = 0;
    fds[0].revents = 0;

    if (mask & MPR_READABLE) {
        fds[0].events |= POLLIN | POLLHUP;
    }
    if (mask & MPR_WRITABLE) {
        fds[0].events |= POLLOUT | POLLHUP;
    }

    result = 0;
    rc = poll(fds, 1, (int) timeout);
    if (rc < 0) {
        mprTrace(8, "Poll returned %d, errno %d", rc, mprGetOsError());
    } else if (rc > 0) {
        if ((fds[0].revents & (POLLIN | POLLHUP)) && (mask & MPR_READABLE)) {
            result |= MPR_READABLE;
        }
        if ((fds[0].revents & (POLLOUT | POLLHUP)) && (mask & MPR_WRITABLE)) {
            result |= MPR_WRITABLE;
        }
    }
    return result;
}


/*
    Wait for I/O on all registered file descriptors. Timeout is in milliseconds. Return the number of events detected.
 */
PUBLIC void mprWaitForIO(MprWaitService *ws, MprTicks timeout)
{
    int     count, rc;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
#if BIT_DEBUG
    if (mprGetDebugMode() && timeout > 30000) {
        timeout = 30000;
    }
#endif
    if (ws->needRecall) {
        mprDoWaitRecall(ws);
        return;
    }
    lock(ws);
    count = ws->fdsCount;
    if ((ws->pollFds = mprMemdup(ws->fds, sizeof(struct pollfd) * count)) == 0) {
        unlock(ws);
        return;
    }
    unlock(ws);

    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    rc = poll(ws->pollFds, count, (int) timeout);
    mprResetYield();

    if (rc < 0) {
        mprTrace(2, "Poll returned %d, errno %d", rc, mprGetOsError());
    } else if (rc > 0) {
        serviceIO(ws, ws->pollFds, count);
    }
    ws->wakeRequested = 0;
}


/*
    Service I/O events
 */
static void serviceIO(MprWaitService *ws, struct pollfd *fds, int count)
{
    MprWaitHandler      *wp;
    struct pollfd       *fp;
    int                 mask;

    lock(ws);
    for (fp = fds; fp < &fds[count]; fp++) {
        if (fp->revents == 0) {
           continue;
        }
        mask = 0;
        if (fp->revents & (POLLIN | POLLHUP | POLLERR | POLLNVAL)) {
            mask |= MPR_READABLE;
        }
        if (fp->revents & (POLLOUT | POLLHUP)) {
            mask |= MPR_WRITABLE;
        }
        assert(mask);
        assert(fp->fd >= 0);
        if ((wp = ws->handlerMap[fp->fd]) == 0) {
            char    buf[128];
            if (fp->fd == ws->breakPipe[MPR_READ_PIPE]) {
                read(fp->fd, buf, sizeof(buf));
            }
            continue;
        }
        wp->presentMask = mask & wp->desiredMask;
        fp->revents = 0;
        if (wp->presentMask) {
            mprNotifyOn(ws, wp, 0);
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    Wake the wait service. WARNING: This routine must not require locking. MprEvents in scheduleDispatcher depends on this.
    Must be async-safe.
 */
PUBLIC void mprWakeNotifier()
{
    MprWaitService  *ws;
    int             c;

    ws = MPR->waitService;
    if (!ws->wakeRequested) {
        ws->wakeRequested = 1;
        c = 0;
        (void) write(ws->breakPipe[MPR_WRITE_PIPE], (char*) &c, 1);
    }
}

#endif /* MPR_EVENT_POLL */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/posix.c"
 */
/************************************************************************/

/**
    posix.c - Posix specific adaptions

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if BIT_UNIX_LIKE
/*********************************** Code *************************************/

PUBLIC int mprCreateOsService()
{
    umask(022);

    /*
        Cleanup the environment. IFS is often a security hole
     */
    putenv("IFS=\t ");
    return 0;
}


PUBLIC int mprStartOsService()
{
    /* 
        Open a syslog connection
     */
#if SOLARIS
    openlog(mprGetAppName(), LOG_LOCAL0);
#else
    openlog(mprGetAppName(), 0, LOG_LOCAL0);
#endif
    return 0;
}


PUBLIC void mprStopOsService()
{
    closelog();
}


PUBLIC int mprGetRandomBytes(char *buf, ssize length, bool block)
{
    ssize   sofar, rc;
    int     fd;

    if ((fd = open((block) ? "/dev/random" : "/dev/urandom", O_RDONLY, 0666)) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    sofar = 0;
    do {
        rc = read(fd, &buf[sofar], length);
        if (rc < 0) {
            assert(0);
            return MPR_ERR_CANT_READ;
        }
        length -= rc;
        sofar += rc;
    } while (length > 0);
    close(fd);
    return 0;
}


#if BIT_HAS_DYN_LOAD
PUBLIC int mprLoadNativeModule(MprModule *mp)
{
    MprModuleEntry  fn;
    MprPath         info;
    char            *at;
    void            *handle;

    assert(mp);

    /*
        Search the image incase the module has been statically linked
     */
#ifdef RTLD_DEFAULT
    handle = RTLD_DEFAULT;
#else
#ifdef RTLD_MAIN_ONLY
    handle = RTLD_MAIN_ONLY;
#else
    handle = 0;
#endif
#endif
    if (!mp->entry || !dlsym(handle, mp->entry)) {
        if ((at = mprSearchForModule(mp->path)) == 0) {
            mprError("Cannot find module \"%s\", cwd: \"%s\", search path \"%s\"", mp->path, mprGetCurrentPath(),
                mprGetModuleSearchPath());
            return MPR_ERR_CANT_ACCESS;
        }
        mp->path = at;
        mprGetPathInfo(mp->path, &info);
        mp->modified = info.mtime;
        mprLog(2, "Loading native module %s", mprGetPathBase(mp->path));
        if ((handle = dlopen(mp->path, RTLD_LAZY | RTLD_GLOBAL)) == 0) {
            mprError("Cannot load module %s\nReason: \"%s\"", mp->path, dlerror());
            return MPR_ERR_CANT_OPEN;
        } 
        mp->handle = handle;

    } else if (mp->entry) {
        mprLog(2, "Activating native module %s", mp->name);
    }
    if (mp->entry) {
        if ((fn = (MprModuleEntry) dlsym(handle, mp->entry)) != 0) {
            if ((fn)(mp->moduleData, mp) < 0) {
                mprError("Initialization for module %s failed", mp->name);
                dlclose(handle);
                return MPR_ERR_CANT_INITIALIZE;
            }
        } else {
            mprError("Cannot load module %s\nReason: can't find function \"%s\"", mp->path, mp->entry);
            dlclose(handle);
            return MPR_ERR_CANT_READ;
        }
    }
    return 0;
}


PUBLIC int mprUnloadNativeModule(MprModule *mp)
{
    return dlclose(mp->handle);
}
#endif


/*
    This routine does not yield
 */
PUBLIC void mprNap(MprTicks timeout)
{
    MprTicks        remaining, mark;
    struct timespec t;
    int             rc;

    assert(timeout >= 0);
    
    mark = mprGetTicks();
    remaining = timeout;
    do {
        /* MAC OS X corrupts the timeout if using the 2nd paramater, so recalc each time */
        t.tv_sec = ((int) (remaining / 1000));
        t.tv_nsec = ((int) ((remaining % 1000) * 1000000));
        rc = nanosleep(&t, NULL);
        remaining = mprGetRemainingTicks(mark, timeout);
    } while (rc < 0 && errno == EINTR && remaining > 0);
}


PUBLIC void mprSleep(MprTicks timeout)
{
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    mprNap(timeout);
    mprResetYield();
}


/*  
    Write a message in the O/S native log (syslog in the case of linux)
 */
PUBLIC void mprWriteToOsLog(cchar *message, int flags, int level)
{
    int     sflag;

    if (flags & MPR_FATAL_MSG) {
        sflag = LOG_ERR;
    } else if (flags & MPR_INFO_MSG) {
        sflag = LOG_WARNING;
    } else if (flags & MPR_ASSERT_MSG) {
        sflag = LOG_WARNING;
    } else if (flags & MPR_ERROR_MSG) {
        sflag = LOG_ERR;
    } else {
        sflag = LOG_WARNING;
    }
    syslog(sflag, "%s", message);
}


PUBLIC int mprInitWindow()
{
    return 0;
}


PUBLIC void mprSetFilesLimit(int limit)
{
    struct rlimit r;
    int           i;

    if (limit == 0 || limit == MAXINT) {
        /*
            We need to determine a reasonable maximum possible limit value.
            There is no #define we can use for this, so we test to determine it empirically
         */
        for (limit = 0x40000000; limit > 0; limit >>= 1) {
            r.rlim_cur = r.rlim_max = limit;
            if (setrlimit(RLIMIT_NOFILE, &r) == 0) {
                for (i = (limit >> 4) * 15; i > 0; i--) {
                    r.rlim_max = r.rlim_cur = limit + i;
                    if (setrlimit(RLIMIT_NOFILE, &r) == 0) {
                        limit = 0;
                        break;
                    }
                }
                break;
            }
        }
    } else {
        r.rlim_cur = r.rlim_max = limit;
        if (setrlimit(RLIMIT_NOFILE, &r) < 0) {
            mprError("Can't set file limit to %d", limit);
        }
    }
    getrlimit(RLIMIT_NOFILE, &r);
    mprTrace(6, "Set files limit to soft %d, max %d", r.rlim_cur, r.rlim_max);
}

#endif /* BIT_UNIX_LIKE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/printf.c"
 */
/************************************************************************/

/**
    printf.c - Printf routines safe for embedded programming

    This module provides safe replacements for the standard printf formatting routines. Most routines in this file 
    are not thread-safe. It is the callers responsibility to perform all thread synchronization.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/*********************************** Defines **********************************/
/*
    Class definitions
 */
#define CLASS_NORMAL    0               /* [All other]      Normal characters */
#define CLASS_PERCENT   1               /* [%]              Begin format */
#define CLASS_MODIFIER  2               /* [-+ #,]          Modifiers */
#define CLASS_ZERO      3               /* [0]              Special modifier - zero pad */
#define CLASS_STAR      4               /* [*]              Width supplied by arg */
#define CLASS_DIGIT     5               /* [1-9]            Field widths */
#define CLASS_DOT       6               /* [.]              Introduce precision */
#define CLASS_BITS      7               /* [hlL]            Length bits */
#define CLASS_TYPE      8               /* [cdefginopsSuxX] Type specifiers */

#define STATE_NORMAL    0               /* Normal chars in format string */
#define STATE_PERCENT   1               /* "%" */
#define STATE_MODIFIER  2               /* -+ #,*/
#define STATE_WIDTH     3               /* Width spec */
#define STATE_DOT       4               /* "." */
#define STATE_PRECISION 5               /* Precision spec */
#define STATE_BITS      6               /* Size spec */
#define STATE_TYPE      7               /* Data type */
#define STATE_COUNT     8

static char stateMap[] = {
    /*     STATES:  Normal Percent Modifier Width  Dot  Prec Bits Type */
    /* CLASS           0      1       2       3     4     5    6    7  */
    /* Normal   0 */   0,     0,      0,      0,    0,    0,   0,   0,
    /* Percent  1 */   1,     0,      1,      1,    1,    1,   1,   1,
    /* Modifier 2 */   0,     2,      2,      0,    0,    0,   0,   0,
    /* Zero     3 */   0,     2,      2,      3,    5,    5,   0,   0,
    /* Star     4 */   0,     3,      3,      0,    5,    0,   0,   0,
    /* Digit    5 */   0,     3,      3,      3,    5,    5,   0,   0,
    /* Dot      6 */   0,     4,      4,      4,    0,    0,   0,   0,
    /* Bits     7 */   0,     6,      6,      6,    6,    6,   6,   0,
    /* Types    8 */   0,     7,      7,      7,    7,    7,   7,   0,
};

/*
    Format:         %[modifier][width][precision][bits][type]
  
    The Class map will map from a specifier letter to a state.
 */
static char classMap[] = {
    /*   0  ' '    !     "     #     $     %     &     ' */
             2,    0,    0,    2,    0,    1,    0,    0,
    /*  07   (     )     *     +     ,     -     .     / */
             0,    0,    4,    2,    2,    2,    6,    0,
    /*  10   0     1     2     3     4     5     6     7 */
             3,    5,    5,    5,    5,    5,    5,    5,
    /*  17   8     9     :     ;     <     =     >     ? */
             5,    5,    0,    0,    0,    0,    0,    0,
    /*  20   @     A     B     C     D     E     F     G */
             8,    0,    0,    0,    0,    0,    0,    0,
    /*  27   H     I     J     K     L     M     N     O */
             0,    0,    0,    0,    7,    0,    8,    0,
    /*  30   P     Q     R     S     T     U     V     W */
             0,    0,    0,    8,    0,    0,    0,    0,
    /*  37   X     Y     Z     [     \     ]     ^     _ */
             8,    0,    0,    0,    0,    0,    0,    0,
    /*  40   '     a     b     c     d     e     f     g */
             0,    0,    0,    8,    8,    8,    8,    8,
    /*  47   h     i     j     k     l     m     n     o */
             7,    8,    0,    0,    7,    0,    8,    8,
    /*  50   p     q     r     s     t     u     v     w */
             8,    0,    0,    8,    0,    8,    0,    8,
    /*  57   x     y     z  */
             8,    0,    0,
};

/*
    Flags
 */
#define SPRINTF_LEFT        0x1         /* Left align */
#define SPRINTF_SIGN        0x2         /* Always sign the result */
#define SPRINTF_LEAD_SPACE  0x4         /* put leading space for +ve numbers */
#define SPRINTF_ALTERNATE   0x8         /* Alternate format */
#define SPRINTF_LEAD_ZERO   0x10        /* Zero pad */
#define SPRINTF_SHORT       0x20        /* 16-bit */
#define SPRINTF_LONG        0x40        /* 32-bit */
#define SPRINTF_INT64       0x80        /* 64-bit */
#define SPRINTF_COMMA       0x100       /* Thousand comma separators */
#define SPRINTF_UPPER_CASE  0x200       /* As the name says for numbers */

typedef struct Format {
    uchar   *buf;
    uchar   *endbuf;
    uchar   *start;
    uchar   *end;
    ssize   growBy;
    ssize   maxsize;
    int     precision;
    int     radix;
    int     width;
    int     flags;
    int     len;
} Format;

#define BPUT(fmt, c) \
    if (1) { \
        /* Less one to allow room for the null */ \
        if ((fmt)->end >= ((fmt)->endbuf - sizeof(char))) { \
            if (growBuf(fmt) > 0) { \
                *(fmt)->end++ = (c); \
            } \
        } else { \
            *(fmt)->end++ = (c); \
        } \
    } else

#define BPUTNULL(fmt) \
    if (1) { \
        if ((fmt)->end > (fmt)->endbuf) { \
            if (growBuf(fmt) > 0) { \
                *(fmt)->end = '\0'; \
            } \
        } else { \
            *(fmt)->end = '\0'; \
        } \
    } else 

/*
    Just for Ejscript to be able to do %N and %S. THIS MUST MATCH EjsString in ejs.h
 */
typedef struct MprEjsString {
    void            *xtype;
#if BIT_DEBUG
    char            *kind;
    void            *type;
    MprMem          *mem;
#endif
    void            *next;
    void            *prev;
    ssize           length;
    wchar         value[0];
} MprEjsString;

typedef struct MprEjsName {
    MprEjsString    *name;
    MprEjsString    *space;
} MprEjsName;

/********************************** Defines ***********************************/

#ifndef BIT_MAX_FMT
    #define BIT_MAX_FMT 256           /* Initial size of a printf buffer */
#endif

/***************************** Forward Declarations ***************************/

static int  getState(char c, int state);
static int  growBuf(Format *fmt);
PUBLIC char *mprPrintfCore(char *buf, ssize maxsize, cchar *fmt, va_list arg);
static void outNum(Format *fmt, cchar *prefix, uint64 val);
static void outString(Format *fmt, cchar *str, ssize len);
#if BIT_CHAR_LEN > 1 && KEEP
static void outWideString(Format *fmt, wchar *str, ssize len);
#endif
#if BIT_FLOAT
static void outFloat(Format *fmt, char specChar, double value);
#endif

/************************************* Code ***********************************/

PUBLIC ssize mprPrintf(cchar *fmt, ...)
{
    va_list     ap;
    char        *buf;
    ssize       len;

    /* No asserts here as this is used as part of assert reporting */

    va_start(ap, fmt);
    buf = sfmtv(fmt, ap);
    va_end(ap);
    if (buf != 0 && MPR->stdOutput) {
        len = mprWriteFileString(MPR->stdOutput, buf);
    } else {
        len = -1;
    }
    return len;
}


PUBLIC ssize mprEprintf(cchar *fmt, ...)
{
    va_list     ap;
    ssize       len;
    char        *buf;

    /* No asserts here as this is used as part of assert reporting */

    va_start(ap, fmt);
    buf = sfmtv(fmt, ap);
    va_end(ap);
    if (buf && MPR->stdError) {
        len = mprWriteFileString(MPR->stdError, buf);
    } else {
        len = -1;
    }
    return len;
}


PUBLIC ssize mprFprintf(MprFile *file, cchar *fmt, ...)
{
    ssize       len;
    va_list     ap;
    char        *buf;

    if (file == 0) {
        return MPR_ERR_BAD_HANDLE;
    }
    va_start(ap, fmt);
    buf = sfmtv(fmt, ap);
    va_end(ap);
    if (buf) {
        len = mprWriteFileString(file, buf);
    } else {
        len = -1;
    }
    return len;
}


#if KEEP
/*
    Printf with a static buffer. Used internally only. WILL NOT MALLOC.
 */
PUBLIC int mprStaticPrintf(cchar *fmt, ...)
{
    MprFileSystem   *fs;
    va_list         ap;
    char            buf[BIT_MAX_BUFFER];

    fs = mprLookupFileSystem(NULL, "/");

    va_start(ap, fmt);
    mprPrintfCore(buf, BIT_MAX_BUFFER, fmt, ap);
    va_end(ap);
    return mprWriteFile(fs->stdOutput, buf, slen(buf));
}


/*
    Printf with a static buffer. Used internally only. WILL NOT MALLOC.
 */
PUBLIC int mprStaticPrintfError(cchar *fmt, ...)
{
    MprFileSystem   *fs;
    va_list         ap;
    char            buf[BIT_MAX_BUFFER];

    fs = mprLookupFileSystem(NULL, "/");

    va_start(ap, fmt);
    mprPrintfCore(buf, BIT_MAX_BUFFER, fmt, ap);
    va_end(ap);
    return mprWriteFile(fs->stdError, buf, slen(buf));
}
#endif


PUBLIC char *fmt(char *buf, ssize bufsize, cchar *fmt, ...)
{
    va_list     ap;
    char        *result;

    assert(buf);
    assert(fmt);
    assert(bufsize > 0);

    va_start(ap, fmt);
    result = mprPrintfCore(buf, bufsize, fmt, ap);
    va_end(ap);
    return result;
}


PUBLIC char *fmtv(char *buf, ssize bufsize, cchar *fmt, va_list arg)
{
    assert(buf);
    assert(fmt);
    assert(bufsize > 0);

    return mprPrintfCore(buf, bufsize, fmt, arg);
}


static int getState(char c, int state)
{
    int     chrClass;

    if (c < ' ' || c > 'z') {
        chrClass = CLASS_NORMAL;
    } else {
        assert((c - ' ') < (int) sizeof(classMap));
        chrClass = classMap[(c - ' ')];
    }
    assert((chrClass * STATE_COUNT + state) < (int) sizeof(stateMap));
    state = stateMap[chrClass * STATE_COUNT + state];
    return state;
}


PUBLIC char *mprPrintfCore(char *buf, ssize maxsize, cchar *spec, va_list args)
{
    Format        fmt;
    MprEjsString  *es;
    MprEjsName    qname;
    ssize         len;
    int64         iValue;
    uint64        uValue;
    int           state;
    char          c, *safe;

    if (spec == 0) {
        spec = "";
    }
    if (buf != 0) {
        assert(maxsize > 0);
        fmt.buf = (uchar*) buf;
        fmt.endbuf = &fmt.buf[maxsize];
        fmt.growBy = -1;
    } else {
        if (maxsize <= 0) {
            maxsize = MAXINT;
        }
        len = min(BIT_MAX_FMT, maxsize);
        if ((buf = mprAlloc(len)) == 0) {
            return 0;
        }
        fmt.buf = (uchar*) buf;
        fmt.endbuf = &fmt.buf[len];
        fmt.growBy = min(len * 2, maxsize - len);
    }
    fmt.maxsize = maxsize;
    fmt.start = fmt.buf;
    fmt.end = fmt.buf;
    fmt.len = 0;
    *fmt.start = '\0';

    state = STATE_NORMAL;

    while ((c = *spec++) != '\0') {
        state = getState(c, state);

        switch (state) {
        case STATE_NORMAL:
            BPUT(&fmt, c);
            break;

        case STATE_PERCENT:
            fmt.precision = -1;
            fmt.width = 0;
            fmt.flags = 0;
            break;

        case STATE_MODIFIER:
            switch (c) {
            case '+':
                fmt.flags |= SPRINTF_SIGN;
                break;
            case '-':
                fmt.flags |= SPRINTF_LEFT;
                break;
            case '#':
                fmt.flags |= SPRINTF_ALTERNATE;
                break;
            case '0':
                fmt.flags |= SPRINTF_LEAD_ZERO;
                break;
            case ' ':
                fmt.flags |= SPRINTF_LEAD_SPACE;
                break;
            case ',':
                fmt.flags |= SPRINTF_COMMA;
                break;
            }
            break;

        case STATE_WIDTH:
            if (c == '*') {
                fmt.width = va_arg(args, int);
                if (fmt.width < 0) {
                    fmt.width = -fmt.width;
                    fmt.flags |= SPRINTF_LEFT;
                }
            } else {
                while (isdigit((uchar) c)) {
                    fmt.width = fmt.width * 10 + (c - '0');
                    c = *spec++;
                }
                spec--;
            }
            break;

        case STATE_DOT:
            fmt.precision = 0;
            break;

        case STATE_PRECISION:
            if (c == '*') {
                fmt.precision = va_arg(args, int);
            } else {
                while (isdigit((uchar) c)) {
                    fmt.precision = fmt.precision * 10 + (c - '0');
                    c = *spec++;
                }
                spec--;
            }
            break;

        case STATE_BITS:
            switch (c) {
            case 'L':
                fmt.flags |= SPRINTF_INT64;
                break;

            case 'l':
                fmt.flags |= SPRINTF_LONG;
                break;

            case 'h':
                fmt.flags |= SPRINTF_SHORT;
                break;
            }
            break;

        case STATE_TYPE:
            switch (c) {
            case 'e':
#if BIT_FLOAT
            case 'g':
            case 'f':
                fmt.radix = 10;
                outFloat(&fmt, c, (double) va_arg(args, double));
                break;
#endif /* BIT_FLOAT */

            case 'c':
                BPUT(&fmt, (char) va_arg(args, int));
                break;

            case 'N':
                /* Name */
                qname = va_arg(args, MprEjsName);
                if (qname.name) {
#if BIT_CHAR_LEN > 1 && KEEP
                    outWideString(&fmt, (wchar*) qname.space->value, qname.space->length);
                    BPUT(&fmt, ':');
                    BPUT(&fmt, ':');
                    outWideString(&fmt, (wchar*) qname.name->value, qname.name->length);
#else
                    outString(&fmt, (char*) qname.space->value, qname.space->length);
                    BPUT(&fmt, ':');
                    BPUT(&fmt, ':');
                    outString(&fmt, (char*) qname.name->value, qname.name->length);
#endif
                } else {
                    outString(&fmt, NULL, 0);
                }
                break;

            case 'S':
                /* Safe string */
#if BIT_CHAR_LEN > 1 && KEEP
                if (fmt.flags & SPRINTF_LONG) {
                    //  UNICODE - not right wchar
                    safe = mprEscapeHtml(va_arg(args, wchar*));
                    outWideString(&fmt, safe, -1);
                } else
#endif
                {
                    safe = mprEscapeHtml(va_arg(args, char*));
                    outString(&fmt, safe, -1);
                }
                break;

            case '@':
                /* MprEjsString */
                es = va_arg(args, MprEjsString*);
                if (es) {
#if BIT_CHAR_LEN > 1 && KEEP
                    outWideString(&fmt, es->value, es->length);
#else
                    outString(&fmt, (char*) es->value, es->length);
#endif
                } else {
                    outString(&fmt, NULL, 0);
                }
                break;

            case 'w':
                /* Wide string of wchar characters (Same as %ls"). Null terminated. */
#if BIT_CHAR_LEN > 1 && KEEP
                outWideString(&fmt, va_arg(args, wchar*), -1);
                break;
#else
                /* Fall through */
#endif

            case 's':
                /* Standard string */
#if BIT_CHAR_LEN > 1 && KEEP
                if (fmt.flags & SPRINTF_LONG) {
                    outWideString(&fmt, va_arg(args, wchar*), -1);
                } else
#endif
                    outString(&fmt, va_arg(args, char*), -1);
                break;

            case 'i':
                ;

            case 'd':
                fmt.radix = 10;
                if (fmt.flags & SPRINTF_SHORT) {
                    iValue = (short) va_arg(args, int);
                } else if (fmt.flags & SPRINTF_LONG) {
                    iValue = (long) va_arg(args, long);
                } else if (fmt.flags & SPRINTF_INT64) {
                    iValue = (int64) va_arg(args, int64);
                } else {
                    iValue = (int) va_arg(args, int);
                }
                if (iValue >= 0) {
                    if (fmt.flags & SPRINTF_LEAD_SPACE) {
                        outNum(&fmt, " ", iValue);
                    } else if (fmt.flags & SPRINTF_SIGN) {
                        outNum(&fmt, "+", iValue);
                    } else {
                        outNum(&fmt, 0, iValue);
                    }
                } else {
                    outNum(&fmt, "-", -iValue);
                }
                break;

            case 'X':
                fmt.flags |= SPRINTF_UPPER_CASE;
#if BIT_64
                fmt.flags &= ~(SPRINTF_SHORT|SPRINTF_LONG);
                fmt.flags |= SPRINTF_INT64;
#else
                fmt.flags &= ~(SPRINTF_INT64);
#endif
                /*  Fall through  */
            case 'o':
            case 'x':
            case 'u':
                if (fmt.flags & SPRINTF_SHORT) {
                    uValue = (ushort) va_arg(args, uint);
                } else if (fmt.flags & SPRINTF_LONG) {
                    uValue = (ulong) va_arg(args, ulong);
                } else if (fmt.flags & SPRINTF_INT64) {
                    uValue = (uint64) va_arg(args, uint64);
                } else {
                    uValue = va_arg(args, uint);
                }
                if (c == 'u') {
                    fmt.radix = 10;
                    outNum(&fmt, 0, uValue);
                } else if (c == 'o') {
                    fmt.radix = 8;
                    if (fmt.flags & SPRINTF_ALTERNATE && uValue != 0) {
                        outNum(&fmt, "0", uValue);
                    } else {
                        outNum(&fmt, 0, uValue);
                    }
                } else {
                    fmt.radix = 16;
                    if (fmt.flags & SPRINTF_ALTERNATE && uValue != 0) {
                        if (c == 'X') {
                            outNum(&fmt, "0X", uValue);
                        } else {
                            outNum(&fmt, "0x", uValue);
                        }
                    } else {
                        outNum(&fmt, 0, uValue);
                    }
                }
                break;

            case 'n':       /* Count of chars seen thus far */
                if (fmt.flags & SPRINTF_SHORT) {
                    short *count = va_arg(args, short*);
                    *count = (int) (fmt.end - fmt.start);
                } else if (fmt.flags & SPRINTF_LONG) {
                    long *count = va_arg(args, long*);
                    *count = (int) (fmt.end - fmt.start);
                } else {
                    int *count = va_arg(args, int *);
                    *count = (int) (fmt.end - fmt.start);
                }
                break;

            case 'p':       /* Pointer */
#if BIT_64
                uValue = (uint64) va_arg(args, void*);
#else
                uValue = (uint) PTOI(va_arg(args, void*));
#endif
                fmt.radix = 16;
                outNum(&fmt, "0x", uValue);
                break;

            default:
                BPUT(&fmt, c);
            }
        }
    }
    /*
        Return the buffer as the result. Prevents a double alloc.
     */
    BPUTNULL(&fmt);
    return (char*) fmt.buf;
}


static void outString(Format *fmt, cchar *str, ssize len)
{
    cchar   *cp;
    ssize   i;

    if (str == NULL) {
        str = "null";
        len = 4;
    } else if (fmt->flags & SPRINTF_ALTERNATE) {
        str++;
        len = (ssize) *str;
    } else if (fmt->precision >= 0) {
        for (cp = str, len = 0; len < fmt->precision; len++) {
            if (*cp++ == '\0') {
                break;
            }
        }
    } else if (len < 0) {
        len = slen(str);
    }
    if (!(fmt->flags & SPRINTF_LEFT)) {
        for (i = len; i < fmt->width; i++) {
            BPUT(fmt, (char) ' ');
        }
    }
    for (i = 0; i < len && *str; i++) {
        BPUT(fmt, *str++);
    }
    if (fmt->flags & SPRINTF_LEFT) {
        for (i = len; i < fmt->width; i++) {
            BPUT(fmt, (char) ' ');
        }
    }
}


#if BIT_CHAR_LEN > 1 && KEEP
static void outWideString(Format *fmt, wchar *str, ssize len)
{
    wchar     *cp;
    int         i;

    if (str == 0) {
        BPUT(fmt, (char) 'n');
        BPUT(fmt, (char) 'u');
        BPUT(fmt, (char) 'l');
        BPUT(fmt, (char) 'l');
        return;
    } else if (fmt->flags & SPRINTF_ALTERNATE) {
        str++;
        len = (ssize) *str;
    } else if (fmt->precision >= 0) {
        for (cp = str, len = 0; len < fmt->precision; len++) {
            if (*cp++ == 0) {
                break;
            }
        }
    } else if (len < 0) {
        for (cp = str, len = 0; *cp++ == 0; len++) ;
    }
    if (!(fmt->flags & SPRINTF_LEFT)) {
        for (i = len; i < fmt->width; i++) {
            BPUT(fmt, (char) ' ');
        }
    }
    for (i = 0; i < len && *str; i++) {
        BPUT(fmt, *str++);
    }
    if (fmt->flags & SPRINTF_LEFT) {
        for (i = len; i < fmt->width; i++) {
            BPUT(fmt, (char) ' ');
        }
    }
}
#endif


static void outNum(Format *fmt, cchar *prefix, uint64 value)
{
    char    numBuf[64];
    char    *cp;
    char    *endp;
    char    c;
    int     letter, len, leadingZeros, i, fill;

    endp = &numBuf[sizeof(numBuf) - 1];
    *endp = '\0';
    cp = endp;

    /*
     *  Convert to ascii
     */
    if (fmt->radix == 16) {
        do {
            letter = (int) (value % fmt->radix);
            if (letter > 9) {
                if (fmt->flags & SPRINTF_UPPER_CASE) {
                    letter = 'A' + letter - 10;
                } else {
                    letter = 'a' + letter - 10;
                }
            } else {
                letter += '0';
            }
            *--cp = letter;
            value /= fmt->radix;
        } while (value > 0);

    } else if (fmt->flags & SPRINTF_COMMA) {
        i = 1;
        do {
            *--cp = '0' + (int) (value % fmt->radix);
            value /= fmt->radix;
            if ((i++ % 3) == 0 && value > 0) {
                *--cp = ',';
            }
        } while (value > 0);
    } else {
        do {
            *--cp = '0' + (int) (value % fmt->radix);
            value /= fmt->radix;
        } while (value > 0);
    }

    len = (int) (endp - cp);
    fill = fmt->width - len;

    if (prefix != 0) {
        fill -= (int) slen(prefix);
    }
    leadingZeros = (fmt->precision > len) ? fmt->precision - len : 0;
    fill -= leadingZeros;

    if (!(fmt->flags & SPRINTF_LEFT)) {
        c = (fmt->flags & SPRINTF_LEAD_ZERO) ? '0': ' ';
        for (i = 0; i < fill; i++) {
            BPUT(fmt, c);
        }
    }
    if (prefix != 0) {
        while (*prefix) {
            BPUT(fmt, *prefix++);
        }
    }
    for (i = 0; i < leadingZeros; i++) {
        BPUT(fmt, '0');
    }
    while (*cp) {
        BPUT(fmt, *cp);
        cp++;
    }
    if (fmt->flags & SPRINTF_LEFT) {
        for (i = 0; i < fill; i++) {
            BPUT(fmt, ' ');
        }
    }
}


#if BIT_FLOAT
static void outFloat(Format *fmt, char specChar, double value)
{
    char    result[256], *cp;
    int     c, fill, i, len, index;

    result[0] = '\0';
    if (specChar == 'f') {
        sprintf(result, "%.*f", fmt->precision, value);
    } else if (specChar == 'g') {
        sprintf(result, "%*.*g", fmt->width, fmt->precision, value);
    } else if (specChar == 'e') {
        sprintf(result, "%*.*e", fmt->width, fmt->precision, value);
    }
    len = (int) slen(result);
    fill = fmt->width - len;
    if (fmt->flags & SPRINTF_COMMA) {
        if (((len - 1) / 3) > 0) {
            fill -= (len - 1) / 3;
        }
    }

    if (fmt->flags & SPRINTF_SIGN && value > 0) {
        BPUT(fmt, '+');
        fill--;
    }
    if (!(fmt->flags & SPRINTF_LEFT)) {
        c = (fmt->flags & SPRINTF_LEAD_ZERO) ? '0': ' ';
        for (i = 0; i < fill; i++) {
            BPUT(fmt, c);
        }
    }
    index = len;
    for (cp = result; *cp; cp++) {
        BPUT(fmt, *cp);
        if (fmt->flags & SPRINTF_COMMA) {
            if ((--index % 3) == 0 && index > 0) {
                BPUT(fmt, ',');
            }
        }
    }
    if (fmt->flags & SPRINTF_LEFT) {
        for (i = 0; i < fill; i++) {
            BPUT(fmt, ' ');
        }
    }
    BPUTNULL(fmt);
}


PUBLIC int mprIsNan(double value) {
#if WINDOWS
    return _fpclass(value) & (_FPCLASS_SNAN | _FPCLASS_QNAN);
#elif VXWORKS
    return value == (0.0 / 0.0);
#else
    return fpclassify(value) == FP_NAN;
#endif
}


PUBLIC int mprIsInfinite(double value) {
#if WINDOWS
    return _fpclass(value) & (_FPCLASS_PINF | _FPCLASS_NINF);
#elif VXWORKS
    return value == (1.0 / 0.0) || value == (-1.0 / 0.0);
#else
    return fpclassify(value) == FP_INFINITE;
#endif
}

PUBLIC int mprIsZero(double value) {
#if WINDOWS
    return _fpclass(value) & (_FPCLASS_NZ | _FPCLASS_PZ);
#elif VXWORKS
    return value == 0.0 || value == -0.0;
#else
    return fpclassify(value) == FP_ZERO;
#endif
}
#endif /* BIT_FLOAT */


/*
    Grow the buffer to fit new data. Return 1 if the buffer can grow. 
    Grow using the growBy size specified when creating the buffer. 
 */
static int growBuf(Format *fmt)
{
    uchar   *newbuf;
    ssize   buflen;

    buflen = (int) (fmt->endbuf - fmt->buf);
    if (fmt->maxsize >= 0 && buflen >= fmt->maxsize) {
        return 0;
    }
    if (fmt->growBy <= 0) {
        /*
            User supplied buffer
         */
        return 0;
    }
    newbuf = mprAlloc(buflen + fmt->growBy);
    if (newbuf == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    if (fmt->buf) {
        memcpy(newbuf, fmt->buf, buflen);
    }
    buflen += fmt->growBy;
    fmt->end = newbuf + (fmt->end - fmt->buf);
    fmt->start = newbuf + (fmt->start - fmt->buf);
    fmt->buf = newbuf;
    fmt->endbuf = &fmt->buf[buflen];

    /*
        Increase growBy to reduce overhead
     */
    if ((buflen + (fmt->growBy * 2)) < fmt->maxsize) {
        fmt->growBy *= 2;
    }
    return 1;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/rom.c"
 */
/************************************************************************/

/*
    rom.c - ROM File system

    ROM support for systems without disk or flash based file systems. This module provides read-only file retrieval 
    from compiled file images. Use the mprRomComp program to compile files into C code and then link them into your 
    application. This module uses a hashed symbol table for fast file lookup.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if BIT_ROM 
/********************************** Defines ***********************************/

#ifndef BIT_MAX_ROMFS
    #define BIT_MAX_ROMFS 37           /* Size of the ROMFS hash lookup */
#endif

/********************************** Forwards **********************************/

static void manageRomFile(MprFile *file, int flags);
static int getPathInfo(MprRomFileSystem *rfs, cchar *path, MprPath *info);
static MprRomInode *lookup(MprRomFileSystem *rfs, cchar *path);

/*********************************** Code *************************************/

static MprFile *openFile(MprFileSystem *fileSystem, cchar *path, int flags, int omode)
{
    MprRomFileSystem    *rfs;
    MprFile             *file;
    
    assert(path && *path);

    rfs = (MprRomFileSystem*) fileSystem;
    file = mprAllocObj(MprFile, manageRomFile);
    file->fileSystem = fileSystem;
    file->mode = omode;
    file->fd = -1;
    file->path = sclone(path);
    if ((file->inode = lookup(rfs, path)) == 0) {
        return 0;
    }
    return file;
}


static void manageRomFile(MprFile *file, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(file->fileSystem);
        mprMark(file->path);
        mprMark(file->buf);
        mprMark(file->inode);
    }
}


static int closeFile(MprFile *file)
{
    return 0;
}


static ssize readFile(MprFile *file, void *buf, ssize size)
{
    MprRomInode     *inode;
    ssize           len;

    assert(buf);

    if (file->fd == 0) {
        return read(file->fd, buf, size);
    }
    inode = file->inode;
    len = min(inode->size - file->iopos, size);
    assert(len >= 0);
    memcpy(buf, &inode->data[file->iopos], len);
    file->iopos += len;
    return len;
}


static ssize writeFile(MprFile *file, cvoid *buf, ssize size)
{
    if (file->fd == 1 || file->fd == 2) {
        return write(file->fd, buf, size);
    }
    return MPR_ERR_CANT_WRITE;
}


static long seekFile(MprFile *file, int seekType, long distance)
{
    MprRomInode     *inode;

    assert(seekType == SEEK_SET || seekType == SEEK_CUR || seekType == SEEK_END);
    inode = file->inode;

    switch (seekType) {
    case SEEK_CUR:
        file->iopos += distance;
        break;

    case SEEK_END:
        file->iopos = inode->size + distance;
        break;

    default:
        file->iopos = distance;
        break;
    }
    if (file->iopos < 0) {
        errno = EBADF;
        return MPR_ERR_BAD_STATE;
    }
    return file->iopos;
}


static bool accessPath(MprRomFileSystem *fileSystem, cchar *path, int omode)
{
    MprPath     info;
    return getPathInfo(fileSystem, path, &info) == 0 ? 1 : 0;
}


static int deletePath(MprRomFileSystem *fileSystem, cchar *path)
{
    return MPR_ERR_CANT_WRITE;
}
 

static int makeDir(MprRomFileSystem *fileSystem, cchar *path, int perms, int owner, int group)
{
    return MPR_ERR_CANT_WRITE;
}


static int makeLink(MprRomFileSystem *fileSystem, cchar *path, cchar *target, int hard)
{
    return MPR_ERR_CANT_WRITE;
}


static int getPathInfo(MprRomFileSystem *rfs, cchar *path, MprPath *info)
{
    MprRomInode *ri;

    assert(path && *path);
    memset(info, 0, sizeof(MprPath));
    info->checked = 1;

    if ((ri = (MprRomInode*) lookup(rfs, path)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    info->valid = 1;
    info->size = ri->size;
    info->mtime = 0;
    info->inode = ri->num;

    if (ri->data == 0) {
        info->isDir = 1;
        info->isReg = 0;
    } else {
        info->isReg = 1;
        info->isDir = 0;
    }
    return 0;
}


static int getPathLink(MprRomFileSystem *rfs, cchar *path)
{
    /* Links not supported on ROMfs */
    return -1;
}


static MprRomInode *lookup(MprRomFileSystem *rfs, cchar *path)
{
    if (path == 0) {
        return 0;
    }
    /*
        Remove "./" segments
     */
    while (*path == '.') {
        if (path[1] == '\0') {
            path++;
        } else if (path[1] == '/') {
            path += 2;
        } else {
            break;
        }
    }
    /*
        Skip over the leading "/"
     */
    if (*path == '/') {
        path++;
    }
    return (MprRomInode*) mprLookupKey(rfs->fileIndex, path);
}


PUBLIC int mprSetRomFileSystem(MprRomInode *inodeList)
{
    MprRomFileSystem    *rfs;
    MprRomInode         *ri;

    rfs = (MprRomFileSystem*) MPR->fileSystem;
    rfs->romInodes = inodeList;
    rfs->fileIndex = mprCreateHash(BIT_MAX_ROMFS, MPR_HASH_STATIC_KEYS | MPR_HASH_STATIC_VALUES);

    for (ri = inodeList; ri->path; ri++) {
        if (mprAddKey(rfs->fileIndex, ri->path, ri) < 0) {
            assert(!MPR_ERR_MEMORY);
            return MPR_ERR_MEMORY;
        }
    }
    return 0;
}


PUBLIC void manageRomFileSystem(MprRomFileSystem *rfs, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
#if !WINCE
        MprFileSystem *fs = (MprFileSystem*) rfs;
        mprMark(fs->separators);
        mprMark(fs->newline);
        mprMark(fs->root);
#if BIT_WIN_LIKE || CYGWIN
        mprMark(fs->cygdrive);
        mprMark(fs->cygwin);
#endif
        mprMark(rfs->fileIndex);
#endif
    }
}


PUBLIC MprRomFileSystem *mprCreateRomFileSystem(cchar *path)
{
    MprFileSystem      *fs;
    MprRomFileSystem   *rfs;

    if ((rfs = mprAllocObj(MprRomFileSystem, manageRomFileSystem)) == 0) {
        return rfs;
    }
    fs = &rfs->fileSystem;
    fs->accessPath = (MprAccessFileProc) accessPath;
    fs->deletePath = (MprDeleteFileProc) deletePath;
    fs->getPathInfo = (MprGetPathInfoProc) getPathInfo;
    fs->getPathLink = (MprGetPathLinkProc) getPathLink;
    fs->makeDir = (MprMakeDirProc) makeDir;
    fs->makeLink = (MprMakeLinkProc) makeLink;
    fs->openFile = (MprOpenFileProc) openFile;
    fs->closeFile = (MprCloseFileProc) closeFile;
    fs->readFile = (MprReadFileProc) readFile;
    fs->seekFile = (MprSeekFileProc) seekFile;
    fs->writeFile = (MprWriteFileProc) writeFile;

    if ((MPR->stdError = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdError, "stderr");
    MPR->stdError->fd = 2;
    MPR->stdError->fileSystem = fs;
    MPR->stdError->mode = O_WRONLY;

    if ((MPR->stdInput = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdInput, "stdin");
    MPR->stdInput->fd = 0;
    MPR->stdInput->fileSystem = fs;
    MPR->stdInput->mode = O_RDONLY;

    if ((MPR->stdOutput = mprAllocStruct(MprFile)) == 0) {
        return NULL;
    }
    mprSetName(MPR->stdOutput, "stdout");
    MPR->stdOutput->fd = 1;
    MPR->stdOutput->fileSystem = fs;
    MPR->stdOutput->mode = O_WRONLY;
    return rfs;
}


#endif /* BIT_ROM */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/select.c"
 */
/************************************************************************/

/**
    select.c - Wait for I/O by using select.

    This module provides I/O wait management for sockets on VxWorks and systems that use select(). Windows and Unix
    uses different mechanisms. See mprAsyncSelectWait and mprPollWait. This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */
/********************************* Includes ***********************************/



#if MPR_EVENT_SELECT
/********************************** Forwards **********************************/

static void serviceIO(MprWaitService *ws, int maxfd);
static void readPipe(MprWaitService *ws);

/************************************ Code ************************************/

PUBLIC int mprCreateNotifierService(MprWaitService *ws)
{
    int     rc, retries, breakPort, breakSock, maxTries;

    ws->highestFd = 0;
    ws->handlerMax = MPR_FD_MIN;
    if ((ws->handlerMap = mprAllocZeroed(sizeof(MprWaitHandler*) * ws->handlerMax)) == 0) {
        return MPR_ERR_CANT_INITIALIZE;
    }
    FD_ZERO(&ws->readMask);
    FD_ZERO(&ws->writeMask);

    /*
        Try to find a good port to use to break out of the select wait
     */ 
    maxTries = 100;
    breakPort = BIT_WAKEUP_PORT;
    for (rc = retries = 0; retries < maxTries; retries++) {
        breakSock = socket(AF_INET, SOCK_DGRAM, 0);
        if (breakSock < 0) {
            mprWarn("Cannot open port %d to use for select. Retrying.\n");
        }
#if BIT_UNIX_LIKE
        fcntl(breakSock, F_SETFD, FD_CLOEXEC);
#endif
        ws->breakAddress.sin_family = AF_INET;
#if CYGWIN || VXWORKS
        /*
            Cygwin & VxWorks don't work with INADDR_ANY
         */
        ws->breakAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
        ws->breakAddress.sin_addr.s_addr = INADDR_ANY;
#endif
        ws->breakAddress.sin_port = htons((short) breakPort);
        rc = bind(breakSock, (struct sockaddr *) &ws->breakAddress, sizeof(ws->breakAddress));
        if (breakSock >= 0 && rc == 0) {
#if VXWORKS
            /* VxWorks 6.0 bug workaround */
            ws->breakAddress.sin_port = htons((short) breakPort);
#endif
            break;
        }
        if (breakSock >= 0) {
            closesocket(breakSock);
        }
        breakPort++;
    }

    if (breakSock < 0 || rc < 0) {
        mprWarn("Cannot bind any port to use for select. Tried %d-%d\n", breakPort, breakPort - maxTries);
        return MPR_ERR_CANT_OPEN;
    }
    ws->breakSock = breakSock;
    FD_SET(breakSock, &ws->readMask);
    ws->highestFd = breakSock;
    return 0;
}


PUBLIC void mprManageSelect(MprWaitService *ws, int flags)
{
    if (flags & MPR_MANAGE_FREE) {
        if (ws->breakSock >= 0) {
            close(ws->breakSock);
        }
    }
}


static int growFds(MprWaitService *ws, int fd)
{
    ws->handlerMax = max(ws->handlerMax * 2, fd);
    if ((ws->handlerMap = mprRealloc(ws->handlerMap, sizeof(MprWaitHandler*) * ws->handlerMax)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


PUBLIC int mprNotifyOn(MprWaitService *ws, MprWaitHandler *wp, int mask)
{
    int     fd;

    fd = wp->fd;
    if (fd >= FD_SETSIZE) {
        mprError("File descriptor exceeds configured maximum in FD_SETSIZE (%d vs %d)", fd, FD_SETSIZE);
        return MPR_ERR_CANT_INITIALIZE;
    }
    lock(ws);
    if (wp->desiredMask != mask) {
        if (wp->desiredMask & MPR_READABLE && !(mask & MPR_READABLE)) {
            FD_CLR(fd, &ws->readMask);
        }
        if (wp->desiredMask & MPR_WRITABLE && !(mask & MPR_WRITABLE)) {
            FD_CLR(fd, &ws->writeMask);
        }
        if (mask & MPR_READABLE) {
            FD_SET(fd, &ws->readMask);
        }
        if (mask & MPR_WRITABLE) {
            FD_SET(fd, &ws->writeMask);
        }
        if (mask) {
            if (fd >= ws->handlerMax && growFds(ws, fd) < 0) {
                unlock(ws);
                assert(!MPR_ERR_MEMORY);
                return MPR_ERR_MEMORY;
            }
        }
        assert(ws->handlerMap[fd] == 0 || ws->handlerMap[fd] == wp);
        ws->handlerMap[fd] = (mask) ? wp : 0;
        wp->desiredMask = mask;
        ws->highestFd = max(fd, ws->highestFd);
        if (mask == 0 && fd == ws->highestFd) {
            while (--fd > 0) {
                if (FD_ISSET(fd, &ws->readMask) || FD_ISSET(fd, &ws->writeMask)) {
                    break;
                }
            }
            ws->highestFd = fd;
        }
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    unlock(ws);
    return 0;
}


/*
    Wait for I/O on a single file descriptor. Return a mask of events found. Mask is the events of interest.
    timeout is in milliseconds.
 */
PUBLIC int mprWaitForSingleIO(int fd, int mask, MprTicks timeout)
{
    MprWaitService  *ws;
    struct timeval  tval;
    fd_set          readMask, writeMask;
    int             rc;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
    ws = MPR->waitService;
    tval.tv_sec = (int) (timeout / 1000);
    tval.tv_usec = (int) ((timeout % 1000) * 1000);

    FD_ZERO(&readMask);
    if (mask & MPR_READABLE) {
        FD_SET(fd, &readMask);
    }
    FD_ZERO(&writeMask);
    if (mask & MPR_WRITABLE) {
        FD_SET(fd, &writeMask);
    }
    mask = 0;
    rc = select(fd + 1, &readMask, &writeMask, NULL, &tval);
    if (rc < 0) {
        mprTrace(2, "Select returned %d, errno %d", rc, mprGetOsError());
    } else if (rc > 0) {
        if (FD_ISSET(fd, &readMask)) {
            mask |= MPR_READABLE;
        }
        if (FD_ISSET(fd, &writeMask)) {
            mask |= MPR_WRITABLE;
        }
    }
    return mask;
}


/*
    Wait for I/O on all registered file descriptors. Timeout is in milliseconds. Return the number of events detected.
 */
PUBLIC void mprWaitForIO(MprWaitService *ws, MprTicks timeout)
{
    struct timeval  tval;
    int             rc, maxfd;

    if (timeout < 0 || timeout > MAXINT) {
        timeout = MAXINT;
    }
#if BIT_DEBUG
    if (mprGetDebugMode() && timeout > 30000) {
        timeout = 30000;
    }
#endif
#if VXWORKS
    /* Minimize VxWorks task starvation */
    timeout = max(timeout, 50);
#endif
    tval.tv_sec = (int) (timeout / 1000);
    tval.tv_usec = (int) ((timeout % 1000) * 1000);

    if (ws->needRecall) {
        mprDoWaitRecall(ws);
        return;
    }
    lock(ws);
    ws->stableReadMask = ws->readMask;
    ws->stableWriteMask = ws->writeMask;
    maxfd = ws->highestFd + 1;
    unlock(ws);

    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    rc = select(maxfd, &ws->stableReadMask, &ws->stableWriteMask, NULL, &tval);
    mprResetYield();

    if (rc > 0) {
        serviceIO(ws, maxfd);
    }
    ws->wakeRequested = 0;
}


static void serviceIO(MprWaitService *ws, int maxfd)
{
    MprWaitHandler      *wp;
    int                 fd, mask;

    lock(ws);
    for (fd = 0; fd < maxfd; fd++) {
        mask = 0;
        if (FD_ISSET(fd, &ws->stableReadMask)) {
            mask |= MPR_READABLE;
        }
        if (FD_ISSET(fd, &ws->stableWriteMask)) {
            mask |= MPR_WRITABLE;
        }
        if (mask == 0) {
            continue;
        }
        if ((wp = ws->handlerMap[fd]) == 0) {
            if (fd == ws->breakSock) {
                readPipe(ws);
            }
            continue;
        }
        wp->presentMask = mask & wp->desiredMask;
        if (wp->presentMask) {
            mprNotifyOn(ws, wp, 0);
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    Wake the wait service. WARNING: This routine must not require locking. MprEvents in scheduleDispatcher depends on this.
    Must be async-safe.
 */
PUBLIC void mprWakeNotifier()
{
    MprWaitService  *ws;
    ssize           rc;
    int             c;

    ws = MPR->waitService;
    if (!ws->wakeRequested) {
        ws->wakeRequested = 1;
        c = 0;
        rc = sendto(ws->breakSock, (char*) &c, 1, 0, (struct sockaddr*) &ws->breakAddress, (int) sizeof(ws->breakAddress));
        if (rc < 0) {
            static int warnOnce = 0;
            if (warnOnce++ == 0) {
                mprError("Cannot send wakeup to breakout socket: errno %d", errno);
            }
        }
    }
}


static void readPipe(MprWaitService *ws)
{
    char        buf[128];

#if VXWORKS
    int len = sizeof(ws->breakAddress);
    (void) recvfrom(ws->breakSock, buf, (int) sizeof(buf), 0, (struct sockaddr*) &ws->breakAddress, (int*) &len);
#else
    socklen_t   len = sizeof(ws->breakAddress);
    (void) recvfrom(ws->breakSock, buf, (int) sizeof(buf), 0, (struct sockaddr*) &ws->breakAddress, (socklen_t*) &len);
#endif
}

#endif /* MPR_EVENT_SELECT */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/signal.c"
 */
/************************************************************************/

/**
    signal.c - Signal handling for Unix systems

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*********************************** Includes *********************************/



/*********************************** Forwards *********************************/
#if BIT_UNIX_LIKE

static void manageSignal(MprSignal *sp, int flags);
static void manageSignalService(MprSignalService *ssp, int flags);
static void signalEvent(MprSignal *sp, MprEvent *event);
static void signalHandler(int signo, siginfo_t *info, void *arg);
static void standardSignalHandler(void *ignored, MprSignal *sp);
static void unhookSignal(int signo);

/************************************ Code ************************************/

PUBLIC MprSignalService *mprCreateSignalService()
{
    MprSignalService    *ssp;

    if ((ssp = mprAllocObj(MprSignalService, manageSignalService)) == 0) {
        return 0;
    }
    ssp->mutex = mprCreateLock();
    ssp->signals = mprAllocZeroed(sizeof(MprSignal*) * MPR_MAX_SIGNALS);
    ssp->standard = mprCreateList(-1, 0);
    return ssp;
}


static void manageSignalService(MprSignalService *ssp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ssp->mutex);
        mprMark(ssp->standard);
        mprMark(ssp->signals);
        /* Don't mark signals elements as it will prevent signal handlers being reclaimed */
    }
}


PUBLIC void mprStopSignalService()
{
    int     i;

    for (i = 1; i < MPR_MAX_SIGNALS; i++) {
        unhookSignal(i);
    }
}


/*
    Signals are hooked on demand and remain till the Mpr is destroyed
 */
static void hookSignal(int signo, MprSignal *sp)
{
    MprSignalService    *ssp;
    struct sigaction    act, old;
    int                 rc;

    assert(0 < signo && signo < MPR_MAX_SIGNALS);
    ssp = MPR->signalService;
    lock(ssp);
    rc = sigaction(signo, 0, &old);
    if (rc == 0 && old.sa_sigaction != signalHandler) {
        sp->sigaction = old.sa_sigaction;
        ssp->prior[signo] = old;
        memset(&act, 0, sizeof(act));
        act.sa_sigaction = signalHandler;
        act.sa_flags |= SA_SIGINFO | SA_RESTART | SA_NOCLDSTOP;
        act.sa_flags &= ~SA_NODEFER;
        sigemptyset(&act.sa_mask);
        if (sigaction(signo, &act, 0) != 0) {
            mprError("Cannot hook signal %d, errno %d", signo, mprGetOsError());
        }
    }
    unlock(ssp);
}


static void unhookSignal(int signo)
{
    MprSignalService    *ssp;
    struct sigaction    act;
    int                 rc;

    ssp = MPR->signalService;
    lock(ssp);
    rc = sigaction(signo, 0, &act);
    if (rc == 0 && act.sa_sigaction == signalHandler) {
        if (sigaction(signo, &ssp->prior[signo], 0) != 0) {
            mprError("Cannot unhook signal %d, errno %d", signo, mprGetOsError());
        }
    }
    unlock(ssp);
}


/*
    Actual signal handler - must be async-safe. Do very, very little here. Just set a global flag and wakeup the wait
    service (mprWakeNotifier is async-safe). WARNING: Don't put memory allocation, logging or printf here.

    NOTES: The problems here are several fold. The signalHandler may be invoked re-entrantly for different threads for
    the same signal (SIGCHLD). Masked signals are blocked by a single bit and so siginfo will only store one such instance, 
    so you can't use siginfo to get the pid for SIGCHLD. So you really can't save state here, only set an indication that
    a signal has occurred. MprServiceSignals will then process. Signal handlers must then all be invoked and they must
    test if the signal is valid for them. 
 */
static void signalHandler(int signo, siginfo_t *info, void *arg)
{
    MprSignalService    *ssp;
    MprSignalInfo       *ip;
    int                 saveErrno;

    if (signo <= 0 || signo >= MPR_MAX_SIGNALS || MPR == 0) {
        return;
    }
    if (signo == SIGINT) {
        /* Fixes command line recall to complete the line */
        printf("\n");
        exit(1);
        return;
    }
    ssp = MPR->signalService;
    ip = &ssp->info[signo];
    ip->triggered = 1;
    ssp->hasSignals = 1;
    saveErrno = errno;
    mprWakeNotifier();
    errno = saveErrno;
}


/*
    Called by mprServiceEvents after a signal has been received. Create an event and queue on the appropriate dispatcher
 */
PUBLIC void mprServiceSignals()
{
    MprSignalService    *ssp;
    MprSignal           *sp;
    MprSignalInfo       *ip;
    int                 signo;

    ssp = MPR->signalService;
    ssp->hasSignals = 0;
    for (ip = ssp->info; ip < &ssp->info[MPR_MAX_SIGNALS]; ip++) {
        if (ip->triggered) {
            ip->triggered = 0;
            /*
                Create an event for the head of the signal handler chain for this signal
                Copy info from Thread.sigInfo to MprSignal structure.
             */
            signo = (int) (ip - ssp->info);
            if ((sp = ssp->signals[signo]) != 0) {
                mprCreateEvent(sp->dispatcher, "signalEvent", 0, signalEvent, sp, 0);
            }
        }
    }
}


/*
    Invoke the next signal handler. Runs from the dispatcher so signal handlers don't have to be async-safe.
 */
static void signalEvent(MprSignal *sp, MprEvent *event)
{
    MprSignal   *np;
    
    assert(sp);
    assert(event);

    mprTrace(7, "signalEvent signo %d, flags %x", sp->signo, sp->flags);
    np = sp->next;

    if (sp->flags & MPR_SIGNAL_BEFORE) {
        (sp->handler)(sp->data, sp);
    } 
    if (sp->sigaction) {
        /*
            Call the original (foreign) action handler. Cannot pass on siginfo, because there is no reliable and scalable
            way to save siginfo state when the signalHandler is reentrant for a given signal across multiple threads.
         */
        (sp->sigaction)(sp->signo, NULL, NULL);
    }
    if (sp->flags & MPR_SIGNAL_AFTER) {
        (sp->handler)(sp->data, sp);
    }
    if (np) {
        /* 
            Call all chained signal handlers. Create new event for each handler so we get the right dispatcher.
            WARNING: sp may have been removed and so sp->next may be null. That is why we capture np = sp->next above.
         */
        mprCreateEvent(np->dispatcher, "signalEvent", 0, signalEvent, np, MPR_EVENT_QUICK);
    }
}


static void linkSignalHandler(MprSignal *sp)
{
    MprSignalService    *ssp;

    ssp = MPR->signalService;
    lock(ssp);
    sp->next = ssp->signals[sp->signo];
    ssp->signals[sp->signo] = sp;
    unlock(ssp);
}


static void unlinkSignalHandler(MprSignal *sp)
{
    MprSignalService    *ssp;
    MprSignal           *np, *prev;

    ssp = MPR->signalService;
    lock(ssp);
    for (prev = 0, np = ssp->signals[sp->signo]; np; np = np->next) {
        if (sp == np) {
            if (prev) {
                prev->next = sp->next;
            } else {
                ssp->signals[sp->signo] = sp->next;
            }
            break;
        }
        prev = np;
    }
    assert(np);
    sp->next = 0;
    unlock(ssp);
}


/*
    Add a safe-signal handler. This creates a signal handler that will run from a dispatcher without the
    normal async-safe strictures of normal signal handlers. This manages a next of signal handlers and ensures
    that prior handlers will be called appropriately.
 */
PUBLIC MprSignal *mprAddSignalHandler(int signo, void *handler, void *data, MprDispatcher *dispatcher, int flags)
{
    MprSignal           *sp;

    if (signo <= 0 || signo >= MPR_MAX_SIGNALS) {
        mprError("Bad signal: %d", signo);
        return 0;
    }
    if (!(flags & MPR_SIGNAL_BEFORE)) {
        flags |= MPR_SIGNAL_AFTER;
    }
    if ((sp = mprAllocObj(MprSignal, manageSignal)) == 0) {
        return 0;
    }
    sp->signo = signo;
    sp->flags = flags;
    sp->handler = handler;
    sp->dispatcher = dispatcher;
    sp->data = data;
    linkSignalHandler(sp);
    hookSignal(signo, sp);
    return sp;
}


static void manageSignal(MprSignal *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->dispatcher);
        mprMark(sp->data);
        /* Don't mark next as it will prevent other signal handlers being reclaimed */
    }
}


PUBLIC void mprRemoveSignalHandler(MprSignal *sp)
{
    if (sp) {
        unlinkSignalHandler(sp);
    }
}


/*
    Standard signal handler. The following signals are handled:
        SIGINT - immediate exit
        SIGTERM - graceful shutdown
        SIGPIPE - ignore
        SIGXFZ - ignore
        SIGUSR1 - graceful shutdown, then restart
        SIGUSR2 - toggle trace level (Appweb)
        All others - default exit
 */
PUBLIC void mprAddStandardSignals()
{
    MprSignalService    *ssp;

    ssp = MPR->signalService;
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGINT,  standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGQUIT, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGTERM, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGPIPE, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGUSR1, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
#if SIGXFSZ
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGXFSZ, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
#endif
#if MACOSX && BIT_DEBUG
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGBUS, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
    mprAddItem(ssp->standard, mprAddSignalHandler(SIGSEGV, standardSignalHandler, 0, 0, MPR_SIGNAL_AFTER));
#endif
}


static void standardSignalHandler(void *ignored, MprSignal *sp)
{
    mprTrace(6, "standardSignalHandler signo %d, flags %x", sp->signo, sp->flags);
    if (sp->signo == SIGTERM) {
        mprTerminate(MPR_EXIT_GRACEFUL, -1);

    } else if (sp->signo == SIGINT) {
#if BIT_UNIX_LIKE
        /*  Ensure shell input goes to a new line */
        if (isatty(1)) {
            if (write(1, "\n", 1) < 0) {}
        }
#endif
        mprTerminate(MPR_EXIT_IMMEDIATE, -1);

    } else if (sp->signo == SIGUSR1) {
        mprTerminate(MPR_EXIT_GRACEFUL | MPR_EXIT_RESTART, 0);

    } else if (sp->signo == SIGPIPE || sp->signo == SIGXFSZ) {
        /* Ignore */

    } else if (sp->signo == SIGSEGV || sp->signo == SIGBUS) {
#if EMBEDTHIS
        printf("PAUSED for watson to debug\n");
        sleep(120);
#else
        exit(255);
#endif

    } else {
        mprTerminate(MPR_EXIT_DEFAULT, -1);
    }
}


#else /* BIT_UNIX_LIKE */
    void mprAddStandardSignals() {}
    MprSignalService *mprCreateSignalService() { return mprAlloc(0); }
    void mprStopSignalService() {};
    void mprRemoveSignalHandler(MprSignal *sp) { }
    void mprServiceSignals() {}
#endif /* BIT_UNIX_LIKE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/socket.c"
 */
/************************************************************************/

/**
    socket.c - Convenience class for the management of sockets

    This module provides a higher interface to interact with the standard sockets API. It does not perform buffering.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



#if !VXWORKS && !WINCE
/*
    On MAC OS X, getaddrinfo is not thread-safe and crashes when called by a 2nd thread at any time. ie. locking wont help.
 */
#define BIT_HAS_GETADDRINFO 1
#endif

/********************************** Defines ***********************************/

#ifndef BIT_MAX_IP
    #define BIT_MAX_IP 1024
#endif

/********************************** Forwards **********************************/

static void closeSocket(MprSocket *sp, bool gracefully);
static int connectSocket(MprSocket *sp, cchar *ipAddr, int port, int initialFlags);
static MprSocketProvider *createStandardProvider(MprSocketService *ss);
static void disconnectSocket(MprSocket *sp);
static ssize flushSocket(MprSocket *sp);
static int getSocketIpAddr(struct sockaddr *addr, int addrlen, char *ip, int size, int *port);
static int ipv6(cchar *ip);
static Socket listenSocket(MprSocket *sp, cchar *ip, int port, int initialFlags);
static void manageSocket(MprSocket *sp, int flags);
static void manageSocketService(MprSocketService *ss, int flags);
static void manageSsl(MprSsl *ssl, int flags);
static ssize readSocket(MprSocket *sp, void *buf, ssize bufsize);
static char *socketState(MprSocket *sp);
static ssize writeSocket(MprSocket *sp, cvoid *buf, ssize bufsize);

/************************************ Code ************************************/
/*
    Open the socket service
 */

PUBLIC MprSocketService *mprCreateSocketService()
{
    MprSocketService    *ss;
    char                hostName[BIT_MAX_IP], serverName[BIT_MAX_IP], domainName[BIT_MAX_IP], *dp;
    Socket              fd;

    if ((ss = mprAllocObj(MprSocketService, manageSocketService)) == 0) {
        return 0;
    }
    ss->maxAccept = MAXINT;
    ss->numAccept = 0;

    if ((ss->standardProvider = createStandardProvider(ss)) == 0) {
        return 0;
    }
    if ((ss->mutex = mprCreateLock()) == 0) {
        return 0;
    }
    serverName[0] = '\0';
    domainName[0] = '\0';
    hostName[0] = '\0';
    if (gethostname(serverName, sizeof(serverName)) < 0) {
        scopy(serverName, sizeof(serverName), "localhost");
        mprError("Cannot get host name. Using \"localhost\".");
        /* Keep going */
    }
    if ((dp = strchr(serverName, '.')) != 0) {
        scopy(hostName, sizeof(hostName), serverName);
        *dp++ = '\0';
        scopy(domainName, sizeof(domainName), dp);

    } else {
        scopy(hostName, sizeof(hostName), serverName);
    }
    mprSetServerName(serverName);
    mprSetDomainName(domainName);
    mprSetHostName(hostName);
    ss->secureSockets = mprCreateList(0, 0);

    if ((fd = socket(AF_INET6, SOCK_STREAM, 0)) != -1) {
        ss->hasIPv6 = 1;
        closesocket(fd);
    } else {
        mprInfo("System has only IPv4 support");
    }
    return ss;
}


static void manageSocketService(MprSocketService *ss, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ss->standardProvider);
        mprMark(ss->providers);
        mprMark(ss->defaultProvider);
        mprMark(ss->mutex);
        mprMark(ss->secureSockets);
    }
}


static void manageSocketProvider(MprSocketProvider *provider, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(provider->name);
    }
}


static MprSocketProvider *createStandardProvider(MprSocketService *ss)
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, manageSocketProvider)) == 0) {
        return 0;
    }
    provider->name = sclone("standard");;
    provider->closeSocket = closeSocket;
    provider->disconnectSocket = disconnectSocket;
    provider->flushSocket = flushSocket;
    provider->listenSocket = listenSocket;
    provider->readSocket = readSocket;
    provider->writeSocket = writeSocket;
    provider->socketState = socketState;
    return provider;
}


PUBLIC void mprAddSocketProvider(cchar *name, MprSocketProvider *provider)
{
    MprSocketService    *ss;

    ss = MPR->socketService;
    
    if (ss->providers == 0 && (ss->providers = mprCreateHash(0, 0)) == 0) {
        return;
    }
    provider->name = sclone(name);
    mprAddKey(ss->providers, name, provider);
}


PUBLIC bool mprHasSecureSockets()
{
    return (MPR->socketService->providers != 0);
}


PUBLIC int mprSetMaxSocketAccept(int max)
{
    assert(max >= 0);

    MPR->socketService->maxAccept = max;
    return 0;
}


PUBLIC MprSocket *mprCreateSocket()
{
    MprSocketService    *ss;
    MprSocket           *sp;

    ss = MPR->socketService;
    if ((sp = mprAllocObj(MprSocket, manageSocket)) == 0) {
        return 0;
    }
    sp->port = -1;
    sp->fd = -1;

    sp->provider = ss->standardProvider;
    sp->service = ss;
    sp->mutex = mprCreateLock();
    return sp;
}


static void manageSocket(MprSocket *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->service);
        mprMark(sp->errorMsg);
        mprMark(sp->handler);
        mprMark(sp->acceptIp);
        mprMark(sp->ip);
        mprMark(sp->provider);
        mprMark(sp->listenSock);
        mprMark(sp->sslSocket);
        mprMark(sp->ssl);
        mprMark(sp->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        if (sp->fd >= 0) {
            sp->mutex = 0;
            mprCloseSocket(sp, 1);
        }
    }
}


/*  
    Re-initialize all socket variables so the socket can be reused. This closes the socket and removes all wait handlers.
 */
static void resetSocket(MprSocket *sp)
{
    if (sp->fd >= 0) {
        mprCloseSocket(sp, 0);
    }
    if (sp->flags & MPR_SOCKET_CLOSED) {
        sp->flags = 0;
        sp->port = -1;
        sp->fd = -1;
        sp->ip = 0;
    }
    assert(sp->provider);
}


PUBLIC bool mprHasDualNetworkStack() 
{
    bool dual;

#if defined(BIT_HAS_SINGLE_STACK) || VXWORKS
    dual = 0;
#elif WINDOWS
    {
        OSVERSIONINFO info;
        info.dwOSVersionInfoSize = sizeof(info);
        GetVersionEx(&info);
        /* Vista or later */
        dual = info.dwMajorVersion >= 6;
    }
#else
    dual = MPR->socketService->hasIPv6;
#endif
    return dual;
}


PUBLIC bool mprHasIPv6() 
{
    return MPR->socketService->hasIPv6;
}


/*  
    Open a server connection
 */
PUBLIC Socket mprListenOnSocket(MprSocket *sp, cchar *ip, int port, int flags)
{
    if (sp->provider == 0) {
        return MPR_ERR_NOT_INITIALIZED;
    }
    return sp->provider->listenSocket(sp, ip, port, flags);
}


static Socket listenSocket(MprSocket *sp, cchar *ip, int port, int initialFlags)
{
    struct sockaddr     *addr;
    Socklen             addrlen;
    cchar               *sip;
    int                 datagram, family, protocol, rc, only;

    lock(sp);
    if (ip == 0 || *ip == '\0') {
        mprTrace(6, "listenSocket: %d, flags %x", port, initialFlags);
    } else {
        mprTrace(6, "listenSocket: %s:%d, flags %x", ip, port, initialFlags);
    }
    resetSocket(sp);

    sp->ip = sclone(ip);
    sp->port = port;
    sp->flags = (initialFlags & (MPR_SOCKET_BROADCAST | MPR_SOCKET_DATAGRAM | MPR_SOCKET_BLOCK |
         MPR_SOCKET_NOREUSE | MPR_SOCKET_NODELAY | MPR_SOCKET_THREAD));
    datagram = sp->flags & MPR_SOCKET_DATAGRAM;

    /*
        Change null IP address to be an IPv6 endpoint if the system is dual-stack. That way we can listen on 
        both IPv4 and IPv6
     */
    sip = ((ip == 0 || *ip == '\0') && mprHasDualNetworkStack()) ? "::" : ip;

    if (mprGetSocketInfo(sip, port, &family, &protocol, &addr, &addrlen) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_FIND;
    }
    sp->fd = (int) socket(family, datagram ? SOCK_DGRAM: SOCK_STREAM, protocol);
    if (sp->fd == SOCKET_ERROR) {
        unlock(sp);
        return MPR_ERR_CANT_OPEN;
    }

#if !BIT_WIN_LIKE && !VXWORKS
    /*
        Children won't inherit this fd
     */
    fcntl(sp->fd, F_SETFD, FD_CLOEXEC);
#endif

    if (!(sp->flags & MPR_SOCKET_NOREUSE)) {
        rc = 1;
#if BIT_UNIX_LIKE
        setsockopt(sp->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &rc, sizeof(rc));
#elif BIT_WIN_LIKE && defined(SO_EXCLUSIVEADDRUSE)
        setsockopt(sp->fd, SOL_SOCKET, SO_REUSEADDR | SO_EXCLUSIVEADDRUSE, (char*) &rc, sizeof(rc));
#endif
    }
    /*
        By default, most stacks listen on both IPv6 and IPv4 if ip == 0, except windows which inverts this.
        So we explicitly control.
     */
#if defined(IPV6_V6ONLY)
    if (MPR->socketService->hasIPv6) {
        if (ip == 0 || *ip == '\0') {
            only = 0;
            setsockopt(sp->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*) &only, sizeof(only));
        } else if (ipv6(ip)) {
            only = 1;
            setsockopt(sp->fd, IPPROTO_IPV6, IPV6_V6ONLY, (char*) &only, sizeof(only));
        }
    }
#endif
    if (sp->service->prebind) {
        if ((sp->service->prebind)(sp) < 0) {
            closesocket(sp->fd);
            sp->fd = -1;
            unlock(sp);
            return MPR_ERR_CANT_OPEN;
        }
    }
    if ((rc = bind(sp->fd, addr, addrlen)) < 0) {
        rc = errno;
        if (rc == EADDRINUSE) {
            mprLog(3, "Cannot bind, address %s:%d already in use", ip, port);
        } else {
            mprLog(3, "Cannot bind, address %s:%d errno", ip, port, errno);
        }
        closesocket(sp->fd);
        sp->fd = -1;
        unlock(sp);
        return (rc == EADDRINUSE) ? MPR_ERR_ALREADY_EXISTS : MPR_ERR_CANT_OPEN;
    }

    /* NOTE: Datagrams have not been used in a long while. Maybe broken */
    if (!datagram) {
        sp->flags |= MPR_SOCKET_LISTENER;
        if (listen(sp->fd, SOMAXCONN) < 0) {
            mprLog(3, "Listen error %d", mprGetOsError());
            closesocket(sp->fd);
            sp->fd = -1;
            unlock(sp);
            return MPR_ERR_CANT_OPEN;
        }
    }

#if BIT_WIN_LIKE
    /*
        Delay setting reuse until now so that we can be assured that we have exclusive use of the port.
     */
    if (!(sp->flags & MPR_SOCKET_NOREUSE)) {
        int rc = 1;
        setsockopt(sp->fd, SOL_SOCKET, SO_REUSEADDR, (char*) &rc, sizeof(rc));
    }
#endif
    mprSetSocketBlockingMode(sp, (bool) (sp->flags & MPR_SOCKET_BLOCK));

    /*
        TCP/IP stacks have the No delay option (nagle algorithm) on by default.
     */
    if (sp->flags & MPR_SOCKET_NODELAY) {
        mprSetSocketNoDelay(sp, 1);
    }
    unlock(sp);
    return sp->fd;
}


PUBLIC MprWaitHandler *mprAddSocketHandler(MprSocket *sp, int mask, MprDispatcher *dispatcher, void *proc, 
    void *data, int flags)
{
    assert(sp);
    assert(sp->fd >= 0);
    assert(proc);

    if (sp->fd < 0) {
        return 0;
    }
    if (sp->handler) {
        mprRemoveWaitHandler(sp->handler);
    }
    if (sp->flags & MPR_SOCKET_BUFFERED_READ) {
        mask |= MPR_READABLE;
    }
    if (sp->flags & MPR_SOCKET_BUFFERED_WRITE) {
        mask |= MPR_WRITABLE;
    }
    sp->handler = mprCreateWaitHandler((int) sp->fd, mask, dispatcher, proc, data, flags);
    return sp->handler;
}


PUBLIC void mprRemoveSocketHandler(MprSocket *sp)
{
    if (sp->handler) {
        mprRemoveWaitHandler(sp->handler);
        sp->handler = 0;
    }
}


PUBLIC void mprHiddenSocketData(MprSocket *sp, ssize len, int dir)
{
    lock(sp);
    if (len > 0) {
        sp->flags |= (dir == MPR_READABLE) ? MPR_SOCKET_BUFFERED_READ : MPR_SOCKET_BUFFERED_WRITE;
        if (sp->handler) {
            mprRecallWaitHandler(sp->handler);
        }
    } else {
        sp->flags &= ~((dir == MPR_READABLE) ? MPR_SOCKET_BUFFERED_READ : MPR_SOCKET_BUFFERED_WRITE);
    }
    unlock(sp);
}


//  MOB rename to mprWaitOnSocket

PUBLIC void mprEnableSocketEvents(MprSocket *sp, int mask)
{
    assert(sp->handler);
    if (sp->handler) {
        if (sp->flags & MPR_SOCKET_BUFFERED_READ) {
            mask |= MPR_READABLE;
        }
        if (sp->flags & MPR_SOCKET_BUFFERED_WRITE) {
            mask |= MPR_WRITABLE;
        }
        mprWaitOn(sp->handler, mask);
    }
}


/*  
    Open a client socket connection
 */
PUBLIC int mprConnectSocket(MprSocket *sp, cchar *ip, int port, int flags)
{
    if (sp->provider == 0) {
        return MPR_ERR_NOT_INITIALIZED;
    }
    return connectSocket(sp, ip, port, flags);
}


static int connectSocket(MprSocket *sp, cchar *ip, int port, int initialFlags)
{
    struct sockaddr     *addr;
    Socklen             addrlen;
    int                 broadcast, datagram, family, protocol, rc;

    mprTrace(6, "openClient: %s:%d, flags %x", ip, port, initialFlags);

    lock(sp);
    resetSocket(sp);

    sp->port = port;
    sp->flags = (initialFlags &
        (MPR_SOCKET_BROADCAST | MPR_SOCKET_DATAGRAM | MPR_SOCKET_BLOCK |
         MPR_SOCKET_LISTENER | MPR_SOCKET_NOREUSE | MPR_SOCKET_NODELAY | MPR_SOCKET_THREAD));
    sp->ip = sclone(ip);

    broadcast = sp->flags & MPR_SOCKET_BROADCAST;
    if (broadcast) {
        sp->flags |= MPR_SOCKET_DATAGRAM;
    }
    datagram = sp->flags & MPR_SOCKET_DATAGRAM;

    if (mprGetSocketInfo(ip, port, &family, &protocol, &addr, &addrlen) < 0) {
        closesocket(sp->fd);
        sp->fd = -1;
        unlock(sp);
        return MPR_ERR_CANT_ACCESS;
    }
    if ((sp->fd = (int) socket(family, datagram ? SOCK_DGRAM: SOCK_STREAM, protocol)) < 0) {
        unlock(sp);
        return MPR_ERR_CANT_OPEN;
    }
#if !BIT_WIN_LIKE && !VXWORKS
    /*  
        Children should not inherit this fd
     */
    fcntl(sp->fd, F_SETFD, FD_CLOEXEC);
#endif
    if (broadcast) {
        int flag = 1;
        if (setsockopt(sp->fd, SOL_SOCKET, SO_BROADCAST, (char *) &flag, sizeof(flag)) < 0) {
            closesocket(sp->fd);
            sp->fd = -1;
            unlock(sp);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    if (!datagram) {
        sp->flags |= MPR_SOCKET_CONNECTING;
        do {
            rc = connect(sp->fd, addr, addrlen);
        } while (rc == -1 && errno == EINTR);
        if (rc < 0) {
            /* MAC/BSD returns EADDRINUSE */
            if (errno == EINPROGRESS || errno == EALREADY || errno == EADDRINUSE) {
#if BIT_UNIX_LIKE
                do {
                    struct pollfd pfd;
                    pfd.fd = sp->fd;
                    pfd.events = POLLOUT;
                    rc = poll(&pfd, 1, 1000);
                } while (rc < 0 && errno == EINTR);
#endif
                if (rc > 0) {
                    errno = EISCONN;
                }
            } 
            if (errno != EISCONN) {
                closesocket(sp->fd);
                sp->fd = -1;
                unlock(sp);
                return MPR_ERR_CANT_COMPLETE;
            }
        }
    }
    mprSetSocketBlockingMode(sp, (bool) (sp->flags & MPR_SOCKET_BLOCK));

    /*  
        TCP/IP stacks have the no delay option (nagle algorithm) on by default.
     */
    if (sp->flags & MPR_SOCKET_NODELAY) {
        mprSetSocketNoDelay(sp, 1);
    }
    unlock(sp);
    return 0;
}


/*
    Abortive disconnect. Thread-safe. (e.g. from a timeout or callback thread). This closes the underlying socket file
    descriptor but keeps the handler and socket object intact. It also forces a recall on the wait handler.
 */
PUBLIC void mprDisconnectSocket(MprSocket *sp)
{
    if (sp && sp->provider) {
        sp->provider->disconnectSocket(sp);
    }
}


static void disconnectSocket(MprSocket *sp)
{
    char    buf[BIT_MAX_BUFFER];
    int     i;

    /*  
        Defensive lock buster. Use try lock incase an operation is blocked somewhere with a lock asserted. 
        Should never happen.
     */
    if (!mprTryLock(sp->mutex)) {
        return;
    }
    if (sp->fd >= 0 || !(sp->flags & MPR_SOCKET_EOF)) {
        /*
            Read a reasonable amount of outstanding data to minimize resets. Then do a shutdown to send a FIN and read 
            outstanding data.  All non-blocking.
         */
        mprTrace(6, "Disconnect socket %d", sp->fd);
        mprSetSocketBlockingMode(sp, 0);
        for (i = 0; i < 16; i++) {
            if (recv(sp->fd, buf, sizeof(buf), 0) <= 0) {
                break;
            }
        }
        shutdown(sp->fd, SHUT_RDWR);
        for (i = 0; i < 16; i++) {
            if (recv(sp->fd, buf, sizeof(buf), 0) <= 0) {
                break;
            }
        }
        sp->flags |= MPR_SOCKET_EOF | MPR_SOCKET_DISCONNECTED;
        if (sp->handler) {
            mprRecallWaitHandler(sp->handler);
        }
    }
    unlock(sp);
}


PUBLIC void mprCloseSocket(MprSocket *sp, bool gracefully)
{
    if (sp == NULL) {
        return;
    }
    assert(sp->provider);
    if (sp->provider == 0) {
        return;
    }
    mprRemoveSocketHandler(sp);
    sp->provider->closeSocket(sp, gracefully);
}


/*  
    Standard (non-SSL) close. Permit multiple calls.
 */
static void closeSocket(MprSocket *sp, bool gracefully)
{
    MprSocketService    *ss;
    MprTime             timesUp;
    char                buf[16];

    ss = MPR->socketService;

    lock(sp);
    if (sp->flags & MPR_SOCKET_CLOSED) {
        unlock(sp);
        return;
    }
    sp->flags |= MPR_SOCKET_CLOSED | MPR_SOCKET_EOF;

    if (sp->fd >= 0) {
        /*
            Read any outstanding read data to minimize resets. Then do a shutdown to send a FIN and read outstanding 
            data. All non-blocking.
         */
        mprTrace(6, "Close socket %d, graceful %d", sp->fd, gracefully);
        if (gracefully) {
            mprSetSocketBlockingMode(sp, 0);
            while (recv(sp->fd, buf, sizeof(buf), 0) > 0) { }
        }
        if (shutdown(sp->fd, SHUT_RDWR) == 0) {
            if (gracefully) {
                timesUp = mprGetTime() + MPR_TIMEOUT_LINGER;
                do {
                    if (recv(sp->fd, buf, sizeof(buf), 0) <= 0) {
                        break;
                    }
                } while (mprGetTime() < timesUp);
            }
        }
        closesocket(sp->fd);
        sp->fd = -1;
    }

    if (sp->flags & MPR_SOCKET_SERVER) {
        mprLock(ss->mutex);
        if (--ss->numAccept < 0) {
            ss->numAccept = 0;
        }
        mprUnlock(ss->mutex);
    }
    unlock(sp);
}


PUBLIC MprSocket *mprAcceptSocket(MprSocket *listen)
{
    MprSocketService            *ss;
    MprSocket                   *nsp;
    struct sockaddr_storage     addrStorage, saddrStorage;
    struct sockaddr             *addr, *saddr;
    char                        ip[BIT_MAX_IP], acceptIp[BIT_MAX_IP];
    Socklen                     addrlen, saddrlen;
    Socket                      fd;
    int                         port, acceptPort;

    ss = MPR->socketService;
    addr = (struct sockaddr*) &addrStorage;
    addrlen = sizeof(addrStorage);

    if (listen->flags & MPR_SOCKET_BLOCK) {
        mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    }
    fd = accept(listen->fd, addr, &addrlen);
    if (listen->flags & MPR_SOCKET_BLOCK) {
        mprResetYield();
    }
    if (fd == SOCKET_ERROR) {
        if (mprGetError() != EAGAIN) {
            mprTrace(6, "socket: accept failed, errno %d", mprGetOsError());
        }
        return 0;
    }
    if ((nsp = mprCreateSocket()) == 0) {
        closesocket(fd);
        return 0;
    }
    nsp->fd = fd;
    nsp->listenSock = listen;
    nsp->port = listen->port;
    nsp->flags = ((listen->flags & ~MPR_SOCKET_LISTENER) | MPR_SOCKET_SERVER);

    /*  
        Limit the number of simultaneous clients
     */
    mprLock(ss->mutex);
    if (++ss->numAccept >= ss->maxAccept) {
        mprUnlock(ss->mutex);
        mprLog(2, "Rejecting connection, too many client connections (%d)", ss->numAccept);
        mprCloseSocket(nsp, 0);
        return 0;
    }
    mprUnlock(ss->mutex);

#if !BIT_WIN_LIKE && !VXWORKS
    /* Prevent children inheriting this socket */
    fcntl(fd, F_SETFD, FD_CLOEXEC);         
#endif

    mprSetSocketBlockingMode(nsp, (nsp->flags & MPR_SOCKET_BLOCK) ? 1: 0);
    if (nsp->flags & MPR_SOCKET_NODELAY) {
        mprSetSocketNoDelay(nsp, 1);
    }
    /*
        Get the remote client address
     */
    if (getSocketIpAddr(addr, addrlen, ip, sizeof(ip), &port) != 0) {
        assert(0);
        mprCloseSocket(nsp, 0);
        return 0;
    }
    nsp->ip = sclone(ip);
    nsp->port = port;

    /*
        Get the server interface address accepting the connection
     */
    saddr = (struct sockaddr*) &saddrStorage;
    saddrlen = sizeof(saddrStorage);
    getsockname(fd, saddr, &saddrlen);
    acceptPort = 0;
    getSocketIpAddr(saddr, saddrlen, acceptIp, sizeof(acceptIp), &acceptPort);
    nsp->acceptIp = sclone(acceptIp);
    nsp->acceptPort = acceptPort;
    return nsp;
}


/*  
    Read data. Return -1 for EOF and errors. On success, return the number of bytes read.
 */
PUBLIC ssize mprReadSocket(MprSocket *sp, void *buf, ssize bufsize)
{
    assert(sp);
    assert(buf);
    assert(bufsize > 0);
    assert(sp->provider);

    if (sp->provider == 0) {
        return MPR_ERR_NOT_INITIALIZED;
    }
    return sp->provider->readSocket(sp, buf, bufsize);
}


/*  
    Standard read from a socket (Non SSL)
    Return number of bytes read. Return -1 on errors and EOF.
 */
static ssize readSocket(MprSocket *sp, void *buf, ssize bufsize)
{
    struct sockaddr_storage server;
    Socklen                 len;
    ssize                   bytes;
    int                     errCode;

    assert(buf);
    assert(bufsize > 0);
    assert(~(sp->flags & MPR_SOCKET_CLOSED));

    lock(sp);
    if (sp->flags & MPR_SOCKET_EOF) {
        unlock(sp);
        return -1;
    }
again:
    if (sp->flags & MPR_SOCKET_BLOCK) {
        mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    }
    if (sp->flags & MPR_SOCKET_DATAGRAM) {
        len = sizeof(server);
        bytes = recvfrom(sp->fd, buf, (int) bufsize, MSG_NOSIGNAL, (struct sockaddr*) &server, (Socklen*) &len);
    } else {
        bytes = recv(sp->fd, buf, (int) bufsize, MSG_NOSIGNAL);
    }
    if (sp->flags & MPR_SOCKET_BLOCK) {
        mprResetYield();
    }
    if (bytes < 0) {
        errCode = mprGetSocketError(sp);
        if (errCode == EINTR) {
            goto again;

        } else if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
            bytes = 0;                          /* No data available */

        } else if (errCode == ECONNRESET) {
            sp->flags |= MPR_SOCKET_EOF;        /* Disorderly disconnect */
            bytes = -1;

        } else {
            sp->flags |= MPR_SOCKET_EOF;        /* Some other error */
            bytes = -errCode;
        }

    } else if (bytes == 0) {                    /* EOF */
        sp->flags |= MPR_SOCKET_EOF;
        bytes = -1;
    }
    unlock(sp);
    return bytes;
}


/*  
    Write data. Return the number of bytes written or -1 on errors. NOTE: this routine will return with a
    short write if the underlying socket can't accept any more data.
 */
PUBLIC ssize mprWriteSocket(MprSocket *sp, cvoid *buf, ssize bufsize)
{
    assert(sp);
    assert(buf);
    assert(bufsize > 0);
    assert(sp->provider);

    if (sp->provider == 0) {
        return MPR_ERR_NOT_INITIALIZED;
    }
    return sp->provider->writeSocket(sp, buf, bufsize);
}


/*  
    Standard write to a socket (Non SSL)
 */
static ssize writeSocket(MprSocket *sp, cvoid *buf, ssize bufsize)
{
    struct sockaddr     *addr;
    Socklen             addrlen;
    ssize               len, written, sofar;
    int                 family, protocol, errCode;

    assert(buf);
    assert(bufsize >= 0);
    assert((sp->flags & MPR_SOCKET_CLOSED) == 0);

    lock(sp);
    if (sp->flags & (MPR_SOCKET_BROADCAST | MPR_SOCKET_DATAGRAM)) {
        if (mprGetSocketInfo(sp->ip, sp->port, &family, &protocol, &addr, &addrlen) < 0) {
            unlock(sp);
            return MPR_ERR_CANT_FIND;
        }
    }
    if (sp->flags & MPR_SOCKET_EOF) {
        sofar = MPR_ERR_CANT_WRITE;
    } else {
        errCode = 0;
        len = bufsize;
        sofar = 0;
        while (len > 0) {
            unlock(sp);
            if (sp->flags & MPR_SOCKET_BLOCK) {
                mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
            }
            if ((sp->flags & MPR_SOCKET_BROADCAST) || (sp->flags & MPR_SOCKET_DATAGRAM)) {
                written = sendto(sp->fd, &((char*) buf)[sofar], (int) len, MSG_NOSIGNAL, addr, addrlen);
            } else {
                written = send(sp->fd, &((char*) buf)[sofar], (int) len, MSG_NOSIGNAL);
            }
            /* Get the error code before calling mprResetYield to avoid clearing global error numbers */
            errCode = mprGetSocketError(sp);
            if (sp->flags & MPR_SOCKET_BLOCK) {
                mprResetYield();
            }
            lock(sp);
            if (written < 0) {
                assert(errCode != 0);
                if (errCode == EINTR) {
                    continue;
                } else if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
#if BIT_WIN_LIKE
                    /*
                        Windows sockets don't support blocking I/O. So we simulate here
                        OPT - could wait for a writable event
                     */
                    if (sp->flags & MPR_SOCKET_BLOCK) {
                        mprNap(0);
                        continue;
                    }
#endif
                    unlock(sp);
                    if (sofar) {
                        return sofar;
                    }
                    return -errCode;
                }
                unlock(sp);
                return -errCode;
            }
            len -= written;
            sofar += written;
        }
    }
    unlock(sp);
    return sofar;
}


/*  
    Write a string to the socket
 */
PUBLIC ssize mprWriteSocketString(MprSocket *sp, cchar *str)
{
    return mprWriteSocket(sp, str, slen(str));
}


PUBLIC ssize mprWriteSocketVector(MprSocket *sp, MprIOVec *iovec, int count)
{
    char        *start;
    ssize       total, len, written;
    int         i;

#if BIT_UNIX_LIKE
    if (sp->sslSocket == 0) {
        return writev(sp->fd, (const struct iovec*) iovec, (int) count);
    } else
#endif
    {
        //  OPT - better to buffer and have fewer raw writes
        if (count <= 0) {
            return 0;
        }
        start = iovec[0].start;
        len = (int) iovec[0].len;
        assert(len > 0);

        for (total = i = 0; i < count; ) {
            written = mprWriteSocket(sp, start, len);
            if (written < 0) {
                if (total > 0) {
                    break;
                }
                return written;
            } else if (written == 0) {
                break;
            } else {
                len -= written;
                start += written;
                total += written;
                if (len <= 0) {
                    i++;
                    start = iovec[i].start;
                    len = (int) iovec[i].len;
                }
            }
        }
        return total;
    }
}


#if !BIT_ROM
#if !LINUX || __UCLIBC__
static ssize localSendfile(MprSocket *sp, MprFile *file, MprOff offset, ssize len)
{
    char    buf[BIT_MAX_BUFFER];

    mprSeekFile(file, SEEK_SET, (int) offset);
    len = min(len, sizeof(buf));
    if ((len = mprReadFile(file, buf, len)) < 0) {
        assert(0);
        return MPR_ERR_CANT_READ;
    }
    return mprWriteSocket(sp, buf, len);
}
#endif


/*  
    Write data from a file to a socket. Includes the ability to write header before and after the file data.
    Works even with a null "file" to just output the headers.
 */
PUBLIC MprOff mprSendFileToSocket(MprSocket *sock, MprFile *file, MprOff offset, MprOff bytes, MprIOVec *beforeVec, 
    int beforeCount, MprIOVec *afterVec, int afterCount)
{
#if MACOSX && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
    struct sf_hdtr  def;
#endif
    MprOff          written, toWriteFile;
    ssize           i, rc, toWriteBefore, toWriteAfter, nbytes;
    int             done;

    rc = 0;

#if MACOSX && __MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
    def.hdr_cnt = (int) beforeCount;
    def.headers = (beforeCount > 0) ? (struct iovec*) beforeVec: 0;
    def.trl_cnt = (int) afterCount;
    def.trailers = (afterCount > 0) ? (struct iovec*) afterVec: 0;

    if (file && file->fd >= 0) {
        written = bytes;
        if (sock->flags & MPR_SOCKET_BLOCK) {
            mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
        }
        rc = sendfile(file->fd, sock->fd, offset, &written, &def, 0);
        if (sock->flags & MPR_SOCKET_BLOCK) {
            mprResetYield();
        }
    } else
#else
    if (1) 
#endif
    {
        /* Either !MACOSX or no file */
        done = 0;
        written = 0;
        for (i = toWriteBefore = 0; i < beforeCount; i++) {
            toWriteBefore += beforeVec[i].len;
        }
        for (i = toWriteAfter = 0; i < afterCount; i++) {
            toWriteAfter += afterVec[i].len;
        }
        toWriteFile = (bytes - toWriteBefore - toWriteAfter);
        assert(toWriteFile >= 0);

        /*
            Linux sendfile does not have the integrated ability to send headers. Must do it separately here.
            I/O requests may return short (write fewer than requested bytes).
         */
        if (beforeCount > 0) {
            rc = mprWriteSocketVector(sock, beforeVec, beforeCount);
            if (rc > 0) {
                written += rc;
            }
            if (rc != toWriteBefore) {
                done++;
            }
        }

        if (!done && toWriteFile > 0 && file->fd >= 0) {
#if LINUX && !__UCLIBC__ && !BIT_HAS_OFF64
            off_t off = (off_t) offset;
#endif
            while (!done && toWriteFile > 0) {
                nbytes = (ssize) min(MAXSSIZE, toWriteFile);
                if (sock->flags & MPR_SOCKET_BLOCK) {
                    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
                }
#if LINUX && !__UCLIBC__
    #if BIT_HAS_OFF64
                rc = sendfile64(sock->fd, file->fd, &offset, nbytes);
    #else
                rc = sendfile(sock->fd, file->fd, &off, nbytes);
    #endif
#else
                rc = localSendfile(sock, file, offset, nbytes);
#endif
                if (sock->flags & MPR_SOCKET_BLOCK) {
                    mprResetYield();
                }
                if (rc > 0) {
                    written += rc;
                    toWriteFile -= rc;
                }
                if (rc != nbytes) {
                    done++;
                    break;
                }
            }
        }
        if (!done && afterCount > 0) {
            rc = mprWriteSocketVector(sock, afterVec, afterCount);
            if (rc > 0) {
                written += rc;
            }
        }
    }
    if (rc < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return written;
        }
        return -1;
    }
    return written;
}
#endif /* !BIT_ROM */


static ssize flushSocket(MprSocket *sp)
{
    return 0;
}


PUBLIC ssize mprFlushSocket(MprSocket *sp)
{
    if (sp->provider == 0) {
        return MPR_ERR_NOT_INITIALIZED;
    }
    return sp->provider->flushSocket(sp);
}


static char *socketState(MprSocket *sp)
{
    return MPR->emptyString;
}


PUBLIC char *mprGetSocketState(MprSocket *sp)
{
    if (sp->provider == 0) {
        return 0;
    }
    return sp->provider->socketState(sp);
}


PUBLIC bool mprSocketHasBufferedRead(MprSocket *sp)
{
    return (sp->flags & MPR_SOCKET_BUFFERED_READ) ? 1 : 0;
}


PUBLIC bool mprSocketHasBufferedWrite(MprSocket *sp)
{
    return (sp->flags & MPR_SOCKET_BUFFERED_WRITE) ? 1 : 0;
}


PUBLIC bool mprSocketHandshaking(MprSocket *sp)
{
    return (sp->flags & MPR_SOCKET_HANDSHAKING) ? 1 : 0;
}


/*  
    Return true if end of file
 */
PUBLIC bool mprIsSocketEof(MprSocket *sp)
{
    return ((sp->flags & MPR_SOCKET_EOF) != 0);
}


/*  
    Set the EOF condition
 */
PUBLIC void mprSetSocketEof(MprSocket *sp, bool eof)
{
    if (eof) {
        sp->flags |= MPR_SOCKET_EOF;
    } else {
        sp->flags &= ~MPR_SOCKET_EOF;
    }
}


/*
    Return the O/S socket file handle
 */
PUBLIC Socket mprGetSocketFd(MprSocket *sp)
{
    return sp->fd;
}


/*  
    Return the blocking mode of the socket
 */
PUBLIC bool mprGetSocketBlockingMode(MprSocket *sp)
{
    assert(sp);
    return sp && (sp->flags & MPR_SOCKET_BLOCK);
}


/*  
    Get the socket flags
 */
PUBLIC int mprGetSocketFlags(MprSocket *sp)
{
    return sp->flags;
}


/*  
    Set whether the socket blocks or not on read/write
 */
PUBLIC int mprSetSocketBlockingMode(MprSocket *sp, bool on)
{
    int     oldMode;

    assert(sp);

    lock(sp);
    oldMode = sp->flags & MPR_SOCKET_BLOCK;

    sp->flags &= ~(MPR_SOCKET_BLOCK);
    if (on) {
        sp->flags |= MPR_SOCKET_BLOCK;
    }
#if BIT_WIN_LIKE
{
    int flag = (sp->flags & MPR_SOCKET_BLOCK) ? 0 : 1;
    ioctlsocket(sp->fd, FIONBIO, (ulong*) &flag);
}
#elif VXWORKS
{
    int flag = (sp->flags & MPR_SOCKET_BLOCK) ? 0 : 1;
    ioctl(sp->fd, FIONBIO, (int) &flag);
}
#else
    if (on) {
        fcntl(sp->fd, F_SETFL, fcntl(sp->fd, F_GETFL) & ~O_NONBLOCK);
    } else {
        fcntl(sp->fd, F_SETFL, fcntl(sp->fd, F_GETFL) | O_NONBLOCK);
    }
#endif
    unlock(sp);
    return oldMode;
}


/*  
    Set the TCP delay behavior (nagle algorithm)
 */
PUBLIC int mprSetSocketNoDelay(MprSocket *sp, bool on)
{
    int     oldDelay;

    lock(sp);
    oldDelay = sp->flags & MPR_SOCKET_NODELAY;
    if (on) {
        sp->flags |= MPR_SOCKET_NODELAY;
    } else {
        sp->flags &= ~(MPR_SOCKET_NODELAY);
    }
#if BIT_WIN_LIKE
    {
        BOOL    noDelay;
        noDelay = on ? 1 : 0;
        setsockopt(sp->fd, IPPROTO_TCP, TCP_NODELAY, (FAR char*) &noDelay, sizeof(BOOL));
    }
#else
    {
        int     noDelay;
        noDelay = on ? 1 : 0;
        setsockopt(sp->fd, IPPROTO_TCP, TCP_NODELAY, (char*) &noDelay, sizeof(int));
    }
#endif /* BIT_WIN_LIKE */
    unlock(sp);
    return oldDelay;
}


/*  
    Get the port number
 */
PUBLIC int mprGetSocketPort(MprSocket *sp)
{
    return sp->port;
}


/*
    Map the O/S error code to portable error codes.
 */
PUBLIC int mprGetSocketError(MprSocket *sp)
{
#if BIT_WIN_LIKE
    int     rc;
    switch (rc = WSAGetLastError()) {
    case WSAEINTR:
        return EINTR;

    case WSAENETDOWN:
        return ENETDOWN;

    case WSAEWOULDBLOCK:
        return EWOULDBLOCK;

    case WSAEPROCLIM:
        return EAGAIN;

    case WSAECONNRESET:
    case WSAECONNABORTED:
        return ECONNRESET;

    case WSAECONNREFUSED:
        return ECONNREFUSED;

    case WSAEADDRINUSE:
        return EADDRINUSE;
    default:
        return EINVAL;
    }
#else
    return errno;
#endif
}


#if BIT_HAS_GETADDRINFO
/*  
    Get a socket address from a host/port combination. If a host provides both IPv4 and IPv6 addresses, 
    prefer the IPv4 address.
 */
PUBLIC int mprGetSocketInfo(cchar *ip, int port, int *family, int *protocol, struct sockaddr **addr, socklen_t *addrlen)
{
    MprSocketService    *ss;
    struct addrinfo     hints, *res, *r;
    char                *portStr;
    int                 v6;

    assert(addr);
    ss = MPR->socketService;

    mprLock(ss->mutex);
    memset((char*) &hints, '\0', sizeof(hints));

    /*
        Note that IPv6 does not support broadcast, there is no 255.255.255.255 equivalent.
        Multicast can be used over a specific link, but the user must provide that address plus %scope_id.
     */
    if (ip == 0 || ip[0] == '\0') {
        ip = 0;
        hints.ai_flags |= AI_PASSIVE;           /* Bind to 0.0.0.0 and :: if available */
    }
    v6 = ipv6(ip);
    hints.ai_socktype = SOCK_STREAM;
    if (ip) {
        hints.ai_family = v6 ? AF_INET6 : AF_INET;
    } else {
        hints.ai_family = AF_UNSPEC;
    }
    portStr = itos(port);

    /*  
        Try to sleuth the address to avoid duplicate address lookups. Then try IPv4 first then IPv6.
     */
    res = 0;
    if (getaddrinfo(ip, portStr, &hints, &res) != 0) {
        mprUnlock(ss->mutex);
        return MPR_ERR_CANT_OPEN;
    }
    /*
        Prefer IPv4 if IPv6 not requested
     */
    for (r = res; r; r = r->ai_next) {
        if (v6) {
            if (r->ai_family == AF_INET6) {
                break;
            }
        } else {
            if (r->ai_family == AF_INET) {
                break;
            }
        }
    }
    if (r == NULL) {
        r = res;
    }
    *addr = mprAlloc(sizeof(struct sockaddr_storage));
    mprMemcpy((char*) *addr, sizeof(struct sockaddr_storage), (char*) r->ai_addr, (int) r->ai_addrlen);

    *addrlen = (int) r->ai_addrlen;
    *family = r->ai_family;
    *protocol = r->ai_protocol;

    freeaddrinfo(res);
    mprUnlock(ss->mutex);
    return 0;
}
#else

PUBLIC int mprGetSocketInfo(cchar *ip, int port, int *family, int *protocol, struct sockaddr **addr, Socklen *addrlen)
{
    MprSocketService    *ss;
    struct sockaddr_in  *sa;

    ss = MPR->socketService;

    if ((sa = mprAllocStruct(struct sockaddr_in)) == 0) {
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    memset((char*) sa, '\0', sizeof(struct sockaddr_in));
    sa->sin_family = AF_INET;
    sa->sin_port = htons((short) (port & 0xFFFF));

    if (strcmp(ip, "") != 0) {
        sa->sin_addr.s_addr = inet_addr((char*) ip);
    } else {
        sa->sin_addr.s_addr = INADDR_ANY;
    }

    /*
        gethostbyname is not thread safe on some systems
     */
    mprLock(ss->mutex);
    if (sa->sin_addr.s_addr == INADDR_NONE) {
#if VXWORKS
        /*
            VxWorks only supports one interface and this code only supports IPv4
         */
        sa->sin_addr.s_addr = (ulong) hostGetByName((char*) ip);
        if (sa->sin_addr.s_addr < 0) {
            mprUnlock(ss->mutex);
            assert(0);
            return 0;
        }
#else
        struct hostent *hostent;
        hostent = gethostbyname2(ip, AF_INET);
        if (hostent == 0) {
            hostent = gethostbyname2(ip, AF_INET6);
            if (hostent == 0) {
                mprUnlock(ss->mutex);
                return MPR_ERR_CANT_FIND;
            }
        }
        memcpy((char*) &sa->sin_addr, (char*) hostent->h_addr_list[0], (ssize) hostent->h_length);
#endif
    }
    *addr = (struct sockaddr*) sa;
    *addrlen = sizeof(struct sockaddr_in);
    *family = sa->sin_family;
    *protocol = 0;
    mprUnlock(ss->mutex);
    return 0;
}
#endif


/*  
    Return a numerical IP address and port for the given socket info
 */
static int getSocketIpAddr(struct sockaddr *addr, int addrlen, char *ip, int ipLen, int *port)
{
#if (BIT_UNIX_LIKE || WIN)
    char    service[NI_MAXSERV];

#ifdef IN6_IS_ADDR_V4MAPPED
    if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*) addr;
        if (IN6_IS_ADDR_V4MAPPED(&addr6->sin6_addr)) {
            struct sockaddr_in addr4;
            memset(&addr4, 0, sizeof(addr4));
            addr4.sin_family = AF_INET;
            addr4.sin_port = addr6->sin6_port;
            memcpy(&addr4.sin_addr.s_addr, addr6->sin6_addr.s6_addr + 12, sizeof(addr4.sin_addr.s_addr));
            memcpy(addr, &addr4, sizeof(addr4));
            addrlen = sizeof(addr4);
        }
    }
#endif
    if (getnameinfo(addr, addrlen, ip, ipLen, service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV | NI_NOFQDN)) {
        return MPR_ERR_BAD_VALUE;
    }
    *port = atoi(service);

#else
    struct sockaddr_in  *sa;

#if HAVE_NTOA_R
    sa = (struct sockaddr_in*) addr;
    inet_ntoa_r(sa->sin_addr, ip, ipLen);
#else
    uchar   *cp;
    sa = (struct sockaddr_in*) addr;
    cp = (uchar*) &sa->sin_addr;
    fmt(ip, ipLen, "%d.%d.%d.%d", cp[0], cp[1], cp[2], cp[3]);
#endif
    *port = ntohs(sa->sin_port);
#endif
    return 0;
}


/*
    Looks like an IPv6 address if it has 2 or more colons
 */
static int ipv6(cchar *ip)
{
    cchar   *cp;
    int     colons;

    if (ip == 0 || *ip == 0) {
        /*
            Listening on just a bare port means IPv4 only.
         */
        return 0;
    }
    colons = 0;
    for (cp = (char*) ip; ((*cp != '\0') && (colons < 2)) ; cp++) {
        if (*cp == ':') {
            colons++;
        }
    }
    return colons >= 2;
}


/*  
    Parse address and return the IP address and port components. Handles ipv4 and ipv6 addresses. 
    If the IP portion is absent, *pip is set to null. If the port portion is absent, port is set to the defaultPort.
    If a ":*" port specifier is used, *pport is set to -1;
    When an address contains an ipv6 port it should be written as:

        aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh:iiii
    or
        [aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh:iiii]:port

    If supplied an IPv6 address, the backets are stripped in the returned IP address.
    This routine parses any "https://" prefix.
 */
PUBLIC int mprParseSocketAddress(cchar *address, char **pip, int *pport, int *psecure, int defaultPort)
{
    char    *ip, *cp;
    int     port;

    ip = 0;
    if (defaultPort < 0) {
        defaultPort = 80;
    }
    if (psecure) {
        *psecure = sncmp(address, "https", 5);
    }
    ip = sclone(address);
    if ((cp = strchr(ip, ' ')) != 0) {
        *cp++ = '\0';
    }
    if ((cp = strstr(ip, "://")) != 0) {
        ip = sclone(&cp[3]);
    }
    if (ipv6(ip)) {
        /*  
            IPv6. If port is present, it will follow a closing bracket ']'
         */
        if ((cp = strchr(ip, ']')) != 0) {
            cp++;
            if ((*cp) && (*cp == ':')) {
                port = (*++cp == '*') ? -1 : atoi(cp);

                /* Set ipAddr to ipv6 address without brackets */
                ip = sclone(ip + 1);
                cp = strchr(ip, ']');
                *cp = '\0';

            } else {
                /* Handles [a:b:c:d:e:f:g:h:i] case (no port)- should not occur */
                ip = sclone(ip + 1);
                if ((cp = strchr(ip, ']')) != 0) {
                    *cp = '\0';
                }
                if (*ip == '\0') {
                    ip = 0;
                }
                /* No port present, use callers default */
                port = defaultPort;
            }
        } else {
            /* Handles a:b:c:d:e:f:g:h:i case (no port) */
            /* No port present, use callers default */
            port = defaultPort;
        }

    } else {
        /*  
            ipv4 
         */
        if ((cp = strchr(ip, ':')) != 0) {
            *cp++ = '\0';
            if (*cp == '*') {
                port = -1;
            } else {
                port = atoi(cp);
            }
            if (*ip == '*') {
                ip = 0;
            }

        } else if (strchr(ip, '.')) {
            if ((cp = strchr(ip, ' ')) != 0) {
                *cp++ = '\0';
            }
            port = defaultPort;

        } else {
            if (isdigit((uchar) *ip)) {
                port = atoi(ip);
                ip = 0;
            } else {
                /* No port present, use callers default */
                port = defaultPort;
            }
        }
    }
    if (pport) {
        *pport = port;
    }
    if (pip) {
        *pip = ip;
    }
    return 0;
}


PUBLIC bool mprIsSocketSecure(MprSocket *sp)
{
    return sp->sslSocket != 0;
}


PUBLIC bool mprIsSocketV6(MprSocket *sp)
{
    return sp->ip && ipv6(sp->ip);
}


PUBLIC bool mprIsIPv6(cchar *ip)
{
    return ip && ipv6(ip);
}


PUBLIC void mprSetSocketPrebindCallback(MprSocketPrebind callback)
{
    MPR->socketService->prebind = callback;
}


static void manageSsl(MprSsl *ssl, int flags) 
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ssl->key);
        mprMark(ssl->keyFile);
        mprMark(ssl->certFile);
        mprMark(ssl->caFile);
        mprMark(ssl->caPath);
        mprMark(ssl->ciphers);
        mprMark(ssl->mutex);
        mprMark(ssl->config);
        mprMark(ssl->provider);
        mprMark(ssl->providerName);
    }
}


/*
    Create a new SSL context object
 */
PUBLIC MprSsl *mprCreateSsl(int server)
{
    MprSsl      *ssl;

    if ((ssl = mprAllocObj(MprSsl, manageSsl)) == 0) {
        return 0;
    }
    ssl->protocols = MPR_PROTO_TLSV1 | MPR_PROTO_TLSV11 | MPR_PROTO_TLSV12;

    /*
        The default for servers is not to verify client certificates.
        The default for clients is to verify unless MPR->verifySsl has been set to false
     */
    if (server) {
        ssl->verifyDepth = 10;
        ssl->verifyPeer = 0;
        ssl->verifyIssuer = 0;
    } else {
        ssl->verifyDepth = 10;
        ssl->verifyPeer = MPR->verifySsl;
        ssl->verifyIssuer = MPR->verifySsl;
        ssl->caFile = mprJoinPath(mprGetAppDir(), MPR_CA_CERT);
    }
    ssl->mutex = mprCreateLock();
    return ssl;
}


/*
    Clone a SSL context object
 */
PUBLIC MprSsl *mprCloneSsl(MprSsl *src)
{
    MprSsl      *ssl;

    if ((ssl = mprAllocObj(MprSsl, manageSsl)) == 0) {
        return 0;
    }
    if (src) {
        *ssl = *src;
    }
    return ssl;
}


PUBLIC int mprLoadSsl()
{
#if BIT_SSL
    MprSocketService    *ss;
    MprModule           *mp;
    cchar               *path;

    ss = MPR->socketService;
    if (ss->providers) {
        return 0;
    }
    path = mprJoinPath(mprGetAppDir(), "libmprssl");
    if ((mp = mprCreateModule("sslModule", path, "mprSslInit", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    if (mprLoadModule(mp) < 0) {
        mprError("Cannot load %s", path);
        ss->providers = 0;
        return MPR_ERR_CANT_READ;
    }
    return 0;
#else
    mprError("SSL communications support not included in build");
    return MPR_ERR_BAD_STATE;
#endif
}


static int loadProviders()
{
    MprSocketService    *ss;

    ss = MPR->socketService;
    if (!ss->providers && mprLoadSsl() < 0) {
        return MPR_ERR_CANT_READ;
    }
    if (!ss->providers) {
        mprError("Cannot load SSL provider");
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


/*
    Upgrade a socket to use SSL
 */
PUBLIC int mprUpgradeSocket(MprSocket *sp, MprSsl *ssl, cchar *peerName)
{
    MprSocketService    *ss;
    char                *providerName;

    ss  = sp->service;
    assert(sp);

    if (!ssl) {
        return MPR_ERR_BAD_ARGS;
    }
    if (!ssl->provider) {
        if (loadProviders() < 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
        providerName = (ssl->providerName) ? ssl->providerName : ss->defaultProvider;
        if ((ssl->provider = mprLookupKey(ss->providers, providerName)) == 0) {
            mprError("Cannot use SSL, missing SSL provider %s", providerName);
            return MPR_ERR_CANT_INITIALIZE;
        }
        ssl->providerName = providerName;
    }
    mprLog(4, "Using SSL provider: %s", ssl->providerName);
    sp->provider = ssl->provider;
#if FUTURE
    /* session resumption can cause problems with Nagle. However, appweb opens sockets with nodelay by default */
    sp->flags |= MPR_SOCKET_NODELAY;
    mprSetSocketNoDelay(sp, 1);
#endif
    return sp->provider->upgradeSocket(sp, ssl, peerName);
}


PUBLIC void mprAddSslCiphers(MprSsl *ssl, cchar *ciphers)
{
    assert(ssl);
    if (ssl->ciphers) {
        ssl->ciphers = sjoin(ssl->ciphers, ":", ciphers, NULL);
    } else {
        ssl->ciphers = sclone(ciphers);
    }
    ssl->changed = 1;
}


PUBLIC void mprSetSslCiphers(MprSsl *ssl, cchar *ciphers)
{
    assert(ssl);
    ssl->ciphers = sclone(ciphers);
    ssl->changed = 1;
}


PUBLIC void mprSetSslKeyFile(MprSsl *ssl, cchar *keyFile)
{
    assert(ssl);
    ssl->keyFile = (keyFile && *keyFile) ? sclone(keyFile) : 0;
    ssl->changed = 1;
}


PUBLIC void mprSetSslCertFile(MprSsl *ssl, cchar *certFile)
{
    assert(ssl);
    ssl->certFile = (certFile && *certFile) ? sclone(certFile) : 0;
    ssl->changed = 1;
}


PUBLIC void mprSetSslCaFile(MprSsl *ssl, cchar *caFile)
{
    assert(ssl);
    ssl->caFile = (caFile && *caFile) ? sclone(caFile) : 0;
    ssl->changed = 1;
}


/* Only supported in OpenSSL */
PUBLIC void mprSetSslCaPath(MprSsl *ssl, cchar *caPath)
{
    assert(ssl);
    ssl->caPath = (caPath && *caPath) ? sclone(caPath) : 0;
    ssl->changed = 1;
}


/* Only supported in OpenSSL */
PUBLIC void mprSetSslProtocols(MprSsl *ssl, int protocols)
{
    assert(ssl);
    ssl->protocols = protocols;
    ssl->changed = 1;
}


PUBLIC void mprSetSslProvider(MprSsl *ssl, cchar *provider)
{
    assert(ssl);
    ssl->providerName = (provider && *provider) ? sclone(provider) : 0;
    ssl->changed = 1;
}


PUBLIC void mprVerifySslPeer(MprSsl *ssl, bool on)
{
    if (ssl) {
        ssl->verifyPeer = on;
        ssl->verifyIssuer = on;
        ssl->changed = 1;
    } else {
        MPR->verifySsl = on;
    }
}


PUBLIC void mprVerifySslIssuer(MprSsl *ssl, bool on)
{
    assert(ssl);
    ssl->verifyIssuer = on;
    ssl->changed = 1;
}


PUBLIC void mprVerifySslDepth(MprSsl *ssl, int depth)
{
    assert(ssl);
    ssl->verifyDepth = depth;
    ssl->changed = 1;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/string.c"
 */
/************************************************************************/

/**
    string.c - String routines safe for embedded programming

    This module provides safe replacements for the standard string library. 
    Most routines in this file are not thread-safe. It is the callers responsibility to perform all thread synchronization.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/*********************************** Locals ***********************************/

#define HASH_PRIME 0x01000193

/************************************ Code ************************************/

PUBLIC char *itos(int64 value)
{
    return itosradix(value, 10);
}


/*
    Format a number as a string. Support radix 10 and 16.
 */
PUBLIC char *itosradix(int64 value, int radix)
{
    char    numBuf[32];
    char    *cp;
    char    digits[] = "0123456789ABCDEF";
    int     negative;

    if (radix != 10 && radix != 16) {
        return 0;
    }
    cp = &numBuf[sizeof(numBuf)];
    *--cp = '\0';

    if (value < 0) {
        negative = 1;
        value = -value;
    } else {
        negative = 0;
    }
    do {
        *--cp = digits[value % radix];
        value /= radix;
    } while (value > 0);

    if (negative) {
        *--cp = '-';
    }
    return sclone(cp);
}


PUBLIC char *itosbuf(char *buf, ssize size, int64 value, int radix)
{
    char    *cp, *end;
    char    digits[] = "0123456789ABCDEF";
    int     negative;

    if ((radix != 10 && radix != 16) || size < 2) {
        return 0;
    }
    end = cp = &buf[size];
    *--cp = '\0';

    if (value < 0) {
        negative = 1;
        value = -value;
        size--;
    } else {
        negative = 0;
    }
    do {
        *--cp = digits[value % radix];
        value /= radix;
    } while (value > 0 && cp > buf);

    if (negative) {
        if (cp <= buf) {
            return 0;
        }
        *--cp = '-';
    }
    if (buf < cp) {
        /* Move the null too */
        memmove(buf, cp, end - cp + 1);
    }
    return buf;
}


PUBLIC char *scamel(cchar *str)
{
    char    *ptr;
    ssize   size, len;

    if (str == 0) {
        str = "";
    }
    len = slen(str);
    size = len + 1;
    if ((ptr = mprAlloc(size)) != 0) {
        memcpy(ptr, str, len);
        ptr[len] = '\0';
    }
    ptr[0] = (char) tolower((uchar) ptr[0]);
    return ptr;
}


/*
    Case insensitive string comparison. Limited by length
 */
PUBLIC int scaselesscmp(cchar *s1, cchar *s2)
{
    if (s1 == 0 || s2 == 0) {
        return -1;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    return sncaselesscmp(s1, s2, max(slen(s1), slen(s2)));
}


PUBLIC bool scaselessmatch(cchar *s1, cchar *s2)
{
    return scaselesscmp(s1, s2) == 0;
}


PUBLIC char *schr(cchar *s, int c)
{
    if (s == 0) {
        return 0;
    }
    return strchr(s, c);
}


PUBLIC char *sncontains(cchar *str, cchar *pattern, ssize limit)
{
    cchar   *cp, *s1, *s2;
    ssize   lim;

    if (limit < 0) {
        limit = MAXINT;
    }
    if (str == 0) {
        return 0;
    }
    if (pattern == 0 || *pattern == '\0') {
        return 0;
    }
    for (cp = str; *cp && limit > 0; cp++, limit--) {
        s1 = cp;
        s2 = pattern;
        for (lim = limit; *s1 && *s2 && (*s1 == *s2) && lim > 0; lim--) {
            s1++;
            s2++;
        }
        if (*s2 == '\0') {
            return (char*) cp;
        }
    }
    return 0;
}


PUBLIC char *scontains(cchar *str, cchar *pattern)
{
    return sncontains(str, pattern, -1);
}


/*
    Copy a string into a buffer. Always ensure it is null terminated
 */
PUBLIC ssize scopy(char *dest, ssize destMax, cchar *src)
{
    ssize      len;

    assert(src);
    assert(dest);
    assert(0 < dest && destMax < MAXINT);

    len = slen(src);
    /* Must ensure room for null */
    if (destMax <= len) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    strcpy(dest, src);
    return len;
}


PUBLIC char *sclone(cchar *str)
{
    char    *ptr;
    ssize   size, len;

    if (str == 0) {
        str = "";
    }
    len = slen(str);
    size = len + 1;
    if ((ptr = mprAlloc(size)) != 0) {
        memcpy(ptr, str, len);
        ptr[len] = '\0';
    }
    return ptr;
}


PUBLIC int scmp(cchar *s1, cchar *s2)
{
    if (s1 == s2) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    return sncmp(s1, s2, max(slen(s1), slen(s2)));
}


PUBLIC bool sends(cchar *str, cchar *suffix)
{
    if (str == 0 || suffix == 0) {
        return 0;
    }
    if (strcmp(&str[slen(str) - slen(suffix)], suffix) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC char *sfmt(cchar *format, ...)
{
    va_list     ap;
    char        *buf;

    if (format == 0) {
        format = "%s";
    }
    va_start(ap, format);
    buf = mprPrintfCore(NULL, -1, format, ap);
    va_end(ap);
    return buf;
}


PUBLIC char *sfmtv(cchar *format, va_list arg)
{
    assert(format);
    return mprPrintfCore(NULL, -1, format, arg);
}


PUBLIC uint shash(cchar *cname, ssize len)
{
    uint    hash;

    assert(cname);
    assert(0 <= len && len < MAXINT);

    if (cname == 0) {
        return 0;
    }
    hash = (uint) len;
    while (len-- > 0) {
        hash ^= *cname++;
        hash *= HASH_PRIME;
    }
    return hash;
}


/*
    Hash the lower case name
 */
PUBLIC uint shashlower(cchar *cname, ssize len)
{
    uint    hash;

    assert(cname);
    assert(0 <= len && len < MAXINT);

    if (cname == 0) {
        return 0;
    }
    hash = (uint) len;
    while (len-- > 0) {
        hash ^= tolower((uchar) *cname++);
        hash *= HASH_PRIME;
    }
    return hash;
}


PUBLIC char *sjoin(cchar *str, ...)
{
    va_list     ap;
    char        *result;

    va_start(ap, str);
    result = sjoinv(str, ap);
    va_end(ap);
    return result;
}


PUBLIC char *sjoinv(cchar *buf, va_list args)
{
    va_list     ap;
    char        *dest, *str, *dp;
    ssize       required;

    va_copy(ap, args);
    required = 1;
    if (buf) {
        required += slen(buf);
    }
    str = va_arg(ap, char*);
    while (str) {
        required += slen(str);
        str = va_arg(ap, char*);
    }
    if ((dest = mprAlloc(required)) == 0) {
        return 0;
    }
    dp = dest;
    if (buf) {
        strcpy(dp, buf);
        dp += slen(buf);
    }
    va_copy(ap, args);
    str = va_arg(ap, char*);
    while (str) {
        strcpy(dp, str);
        dp += slen(str);
        str = va_arg(ap, char*);
    }
    *dp = '\0';
    return dest;
}


PUBLIC ssize slen(cchar *s)
{
    return s ? strlen(s) : 0;
}


/*  
    Map a string to lower case. Allocates a new string.
 */
PUBLIC char *slower(cchar *str)
{
    char    *cp, *s;

    assert(str);

    if (str) {
        s = sclone(str);
        for (cp = s; *cp; cp++) {
            if (isupper((uchar) *cp)) {
                *cp = (char) tolower((uchar) *cp);
            }
        }
        str = s;
    }
    return (char*) str;
}


PUBLIC bool smatch(cchar *s1, cchar *s2)
{
    return scmp(s1, s2) == 0;
}


PUBLIC int sncaselesscmp(cchar *s1, cchar *s2, ssize n)
{
    int     rc;

    assert(0 <= n && n < MAXINT);

    if (s1 == 0 || s2 == 0) {
        return -1;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; n > 0 && *s1 && rc == 0; s1++, s2++, n--) {
        rc = tolower((uchar) *s1) - tolower((uchar) *s2);
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}


/*
    Clone a sub-string of a specified length. The null is added after the length. The given len can be longer than the
    source string.
 */
PUBLIC char *snclone(cchar *str, ssize len)
{
    char    *ptr;
    ssize   size, l;

    if (str == 0) {
        str = "";
    }
    l = slen(str);
    len = min(l, len);
    size = len + 1;
    if ((ptr = mprAlloc(size)) != 0) {
        memcpy(ptr, str, len);
        ptr[len] = '\0';
    }
    return ptr;
}


/*
    Case sensitive string comparison. Limited by length
 */
PUBLIC int sncmp(cchar *s1, cchar *s2, ssize n)
{
    int     rc;

    assert(0 <= n && n < MAXINT);

    if (s1 == 0 && s2 == 0) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; n > 0 && *s1 && rc == 0; s1++, s2++, n--) {
        rc = *s1 - *s2;
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}


/*
    This routine copies at most "count" characters from a string. It ensures the result is always null terminated and 
    the buffer does not overflow. Returns MPR_ERR_WONT_FIT if the buffer is too small.
 */
PUBLIC ssize sncopy(char *dest, ssize destMax, cchar *src, ssize count)
{
    ssize      len;

    assert(dest);
    assert(src);
    assert(src != dest);
    assert(0 <= count && count < MAXINT);
    assert(0 < destMax && destMax < MAXINT);

    //  OPT use strnlen(src, count);
    len = slen(src);
    len = min(len, count);
    if (destMax <= len) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    if (len > 0) {
        memcpy(dest, src, len);
        dest[len] = '\0';
    } else {
        *dest = '\0';
        len = 0;
    } 
    return len;
}


PUBLIC bool snumber(cchar *s)
{
    return s && *s && strspn(s, "1234567890") == strlen(s);
} 


PUBLIC char *spascal(cchar *str)
{
    char    *ptr;
    ssize   size, len;

    if (str == 0) {
        str = "";
    }
    len = slen(str);
    size = len + 1;
    if ((ptr = mprAlloc(size)) != 0) {
        memcpy(ptr, str, len);
        ptr[len] = '\0';
    }
    ptr[0] = (char) toupper((uchar) ptr[0]);
    return ptr;
}


PUBLIC char *spbrk(cchar *str, cchar *set)
{
    cchar       *sp;
    int         count;

    if (str == 0 || set == 0) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                return (char*) str;
            }
        }
    }
    return 0;
}


PUBLIC char *srchr(cchar *s, int c)
{
    if (s == 0) {
        return 0;
    }
    return strrchr(s, c);
}


PUBLIC char *srejoin(char *buf, ...)
{
    va_list     args;
    char        *result;

    va_start(args, buf);
    result = srejoinv(buf, args);
    va_end(args);
    return result;
}


PUBLIC char *srejoinv(char *buf, va_list args)
{
    va_list     ap;
    char        *dest, *str, *dp;
    ssize       len, required;

    va_copy(ap, args);
    len = slen(buf);
    required = len + 1;
    str = va_arg(ap, char*);
    while (str) {
        required += slen(str);
        str = va_arg(ap, char*);
    }
    if ((dest = mprRealloc(buf, required)) == 0) {
        return 0;
    }
    dp = &dest[len];
    va_copy(ap, args);
    str = va_arg(ap, char*);
    while (str) {
        strcpy(dp, str);
        dp += slen(str);
        str = va_arg(ap, char*);
    }
    *dp = '\0';
    return dest;
}


PUBLIC char *sreplace(cchar *str, cchar *pattern, cchar *replacement)
{
    MprBuf      *buf;
    cchar       *s;
    ssize       plen;

    if (!pattern || pattern[0] == '\0') {
        return sclone(str);
    }
    buf = mprCreateBuf(-1, -1);
    plen = slen(pattern);
    for (s = str; *s; s++) {
        if (sncmp(s, pattern, plen) == 0) {
            if (replacement) {
                mprPutStringToBuf(buf, replacement);
            }
            s += plen - 1;
        } else {
            mprPutCharToBuf(buf, *s);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


PUBLIC ssize sspn(cchar *str, cchar *set)
{
#if KEEP
    cchar       *sp;
    int         count;

    if (str == 0 || set == 0) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                break;
            }
        }
        if (*str != *sp) {
            break;
        }
    }
    return count;
#else
    if (str == 0 || set == 0) {
        return 0;
    }
    return strspn(str, set);
#endif
}
 

PUBLIC bool sstarts(cchar *str, cchar *prefix)
{
    if (str == 0 || prefix == 0) {
        return 0;
    }
    if (strncmp(str, prefix, slen(prefix)) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC int64 stoi(cchar *str)
{
    return stoiradix(str, 10, NULL);
}


/*
    Parse a number and check for parse errors. Supports radix 8, 10 or 16. 
    If radix is <= 0, then the radix is sleuthed from the input.
    Supports formats:
        [(+|-)][0][OCTAL_DIGITS]
        [(+|-)][0][(x|X)][HEX_DIGITS]
        [(+|-)][DIGITS]

 */
PUBLIC int64 stoiradix(cchar *str, int radix, int *err)
{
    cchar   *start;
    int64   val;
    int     n, c, negative;

    if (err) {
        *err = 0;
    }
    if (str == 0) {
        if (err) {
            *err = MPR_ERR_BAD_SYNTAX;
        }
        return 0;
    }
    while (isspace((uchar) *str)) {
        str++;
    }
    val = 0;
    if (*str == '-') {
        negative = 1;
        str++;
    } else {
        negative = 0;
    }
    start = str;
    if (radix <= 0) {
        radix = 10;
        if (*str == '0') {
            if (tolower((uchar) str[1]) == 'x') {
                radix = 16;
                str += 2;
            } else {
                radix = 8;
                str++;
            }
        }

    } else if (radix == 16) {
        if (*str == '0' && tolower((uchar) str[1]) == 'x') {
            str += 2;
        }

    } else if (radix > 10) {
        radix = 10;
    }
    if (radix == 16) {
        while (*str) {
            c = tolower((uchar) *str);
            if (isdigit((uchar) c)) {
                val = (val * radix) + c - '0';
            } else if (c >= 'a' && c <= 'f') {
                val = (val * radix) + c - 'a' + 10;
            } else {
                break;
            }
            str++;
        }
    } else {
        while (*str && isdigit((uchar) *str)) {
            n = *str - '0';
            if (n >= radix) {
                break;
            }
            val = (val * radix) + n;
            str++;
        }
    }
    if (str == start) {
        /* No data */
        if (err) {
            *err = MPR_ERR_BAD_SYNTAX;
        }
        return 0;
    }
    return (negative) ? -val: val;
}


/*
    Note "str" is modifed as per strtok()
    WARNING:  this does not allocate
 */
PUBLIC char *stok(char *str, cchar *delim, char **last)
{
    char    *start, *end;
    ssize   i;

    assert(delim);
    start = (str || *last == 0) ? str : *last;

    if (start == 0) {
        if (last) {
            *last = 0;
        }
        return 0;
    }
    i = strspn(start, delim);
    start += i;
    if (*start == '\0') {
        if (last) {
            *last = 0;
        }
        return 0;
    }
    end = strpbrk(start, delim);
    if (end) {
        *end++ = '\0';
        i = strspn(end, delim);
        end += i;
    }
    if (last) {
        *last = end;
    }
    return start;
}


PUBLIC char *ssub(cchar *str, ssize offset, ssize len)
{
    char    *result;
    ssize   size;

    assert(str);
    assert(offset >= 0);
    assert(0 <= len && len < MAXINT);

    if (str == 0) {
        return 0;
    }
    size = len + 1;
    if ((result = mprAlloc(size)) == 0) {
        return 0;
    }
    sncopy(result, size, &str[offset], len);
    return result;
}


/*
    Trim characters from the given set. Returns a newly allocated string.
 */
PUBLIC char *strim(cchar *str, cchar *set, int where)
{
    char    *s;
    ssize   len, i;

    if (str == 0 || set == 0) {
        return 0;
    }
    if (where & MPR_TRIM_START) {
        i = strspn(str, set);
    } else {
        i = 0;
    }
    s = sclone(&str[i]);
    if (where & MPR_TRIM_END) {
        len = slen(s);
        while (len > 0 && strspn(&s[len - 1], set) > 0) {
            s[len - 1] = '\0';
            len--;
        }
    }
    return s;
}


/*  
    Map a string to upper case
 */
PUBLIC char *supper(cchar *str)
{
    char    *cp, *s;

    assert(str);
    if (str) {
        s = sclone(str);
        for (cp = s; *cp; cp++) {
            if (islower((uchar) *cp)) {
                *cp = (char) toupper((uchar) *cp);
            }
        }
        str = s;
    }
    return (char*) str;
}


/*
    Expand ${token} references in a path or string.
 */
PUBLIC char *stemplate(cchar *str, MprHash *keys)
{
    MprBuf      *buf;
    char        *src, *result, *cp, *tok, *value;

    if (str) {
        if (schr(str, '$') == 0) {
            return sclone(str);
        }
        buf = mprCreateBuf(0, 0);
        for (src = (char*) str; *src; ) {
            if (*src == '$') {
                if (*++src == '{') {
                    for (cp = ++src; *cp && *cp != '}'; cp++) ;
                    tok = snclone(src, cp - src);
                } else {
                    for (cp = src; *cp && (isalnum((uchar) *cp) || *cp == '_'); cp++) ;
                    tok = snclone(src, cp - src);
                }
                if ((value = mprLookupKey(keys, tok)) != 0) {
                    mprPutStringToBuf(buf, value);
                    if (src > str && src[-1] == '{') {
                        src = cp + 1;
                    } else {
                        src = cp;
                    }
                } else {
                    mprPutCharToBuf(buf, '$');
                    if (src > str && src[-1] == '{') {
                        mprPutCharToBuf(buf, '{');
                    }
                    mprPutCharToBuf(buf, *src++);
                }
            } else {
                mprPutCharToBuf(buf, *src++);
            }
        }
        mprAddNullToBuf(buf);
        result = sclone(mprGetBufStart(buf));
    } else {
        result = MPR->emptyString;
    }
    return result;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/test.c"
 */
/************************************************************************/

/*
    test.c - Embedthis Unit Test Framework

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/***************************** Forward Declarations ***************************/

static void     adjustFailedCount(int adj);
static void     adjustThreadCount(int adj);
static void     buildFullNames(MprTestGroup *gp, cchar *runName);
static MprList  *copyGroups(MprTestService *sp, MprList *groups);
static MprTestFailure *createFailure(MprTestGroup *gp, cchar *loc, cchar *message);
static MprTestGroup *createTestGroup(MprTestService *sp, MprTestDef *def, MprTestGroup *parent);
static bool     filterTestGroup(MprTestGroup *gp);
static bool     filterTestCast(MprTestGroup *gp, MprTestCase *tc);
static char     *getErrorMessage(MprTestGroup *gp);
static int      loadTestModule(MprTestService *sp, cchar *fileName);
static void     manageTestService(MprTestService *ts, int flags);
static int      parseFilter(MprTestService *sp, cchar *str);
static void     relayEvent(MprList *groups, MprThread *tp);
static void     runInit(MprTestGroup *parent);
static void     runTerm(MprTestGroup *parent);
static void     runTestGroup(MprTestGroup *gp);
static void     runTestProc(MprTestGroup *gp, MprTestCase *test);
static void     runTestThread(MprList *groups, MprThread *tp);
static int      setLogging(char *logSpec);

/******************************************************************************/

PUBLIC MprTestService *mprCreateTestService()
{
    MprTestService      *sp;

    if ((sp = mprAllocObj(MprTestService, manageTestService)) == 0) {
        return 0;
    }
    MPR->testService = sp;
    sp->iterations = 1;
    sp->numThreads = 1;
    sp->workers = 0;
    sp->testFilter = mprCreateList(-1, 0);
    sp->groups = mprCreateList(-1, 0);
    sp->threadData = mprCreateList(-1, 0);
    sp->start = mprGetTime();
    sp->mutex = mprCreateLock();
    return sp;
}


static void manageTestService(MprTestService *ts, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ts->commandLine);
        mprMark(ts->groups);
        mprMark(ts->threadData);
        mprMark(ts->name);
        mprMark(ts->testFilter);
        mprMark(ts->mutex);
    }
}


PUBLIC int mprParseTestArgs(MprTestService *sp, int argc, char *argv[], MprTestParser extraParser)
{
    cchar       *programName;
    char        *argp;
    int         rc, err, i, depth, nextArg, outputVersion;

    i = 0;
    err = 0;
    outputVersion = 0;

    programName = mprGetPathBase(argv[0]);
    sp->name = sclone(BIT_PRODUCT);

    /*
        Save the command line
     */
    sp->commandLine = sclone(mprGetPathBase(argv[i++]));
    for (; i < argc; i++) {
        sp->commandLine = sjoin(sp->commandLine, " ", argv[i], NULL);
    }

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];

        if (strcmp(argp, "--continue") == 0) {
            sp->continueOnFailures = 1; 

        } else if (strcmp(argp, "--depth") == 0 || strcmp(argp, "-d") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                depth = atoi(argv[++nextArg]);
                if (depth < 0 || depth > 10) {
                    mprError("Bad test depth %d, (range 0-9)", depth);
                    err++;
                } else {
                    sp->testDepth = depth;
                }
            }

        } else if (strcmp(argp, "--debugger") == 0 || strcmp(argp, "-D") == 0) {
            mprSetDebugMode(1);
            sp->debugOnFailures = 1;

        } else if (strcmp(argp, "--echo") == 0) {
            sp->echoCmdLine = 1;

        } else if (strcmp(argp, "--filter") == 0 || strcmp(argp, "-f") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (parseFilter(sp, argv[++nextArg]) < 0) {
                    err++;
                }
            }

        } else if (strcmp(argp, "--iterations") == 0 || (strcmp(argp, "-i") == 0)) {
            if (nextArg >= argc) {
                err++;
            } else {
                sp->iterations = atoi(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--log") == 0 || strcmp(argp, "-l") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                setLogging(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--module") == 0) {
            if (nextArg >= argc) {
                err++;
            } else if (loadTestModule(sp, argv[++nextArg]) < 0) {
                return MPR_ERR_CANT_OPEN;
            }

        } else if (strcmp(argp, "--name") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                sp->name = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--step") == 0 || strcmp(argp, "-s") == 0) {
            sp->singleStep = 1; 

        } else if (strcmp(argp, "--threads") == 0 || strcmp(argp, "-t") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                i = atoi(argv[++nextArg]);
                if (i <= 0 || i > 100) {
                    mprError("%s: Bad number of threads (1-100)", programName);
                    return MPR_ERR_BAD_ARGS;
                }
                sp->numThreads = i;
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            sp->verbose++;

        } else if (strcmp(argp, "--version") == 0 || strcmp(argp, "-V") == 0) {
            outputVersion++;

        } else if (strcmp(argp, "--workers") == 0 || strcmp(argp, "-w") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                i = atoi(argv[++nextArg]);
                if (i < 0 || i > 100) {
                    mprError("%s: Bad number of worker threads (0-100)", programName);
                    return MPR_ERR_BAD_ARGS;
                }
                sp->workers = i;
            }

        } else if (strcmp(argp, "-?") == 0 || (strcmp(argp, "--help") == 0 || strcmp(argp, "--?") == 0)) {
            err++;

        } else if (*argp != '-') {
            break;

        } else if (extraParser) {
            rc = extraParser(argc - nextArg, &argv[nextArg]);
            if (rc < 0) {
                err++;
            } else {
                nextArg += rc;
            }
        } else {
            mprError("Unknown arg %s", argp);
            err++;
        }
    }

    if (sp->workers == 0) {
        sp->workers = 2 + sp->numThreads * 2;
    }
    if (nextArg < argc) {
        if (parseFilter(sp, argv[nextArg++]) < 0) {
            err++;
        }
    }
    if (err) {
        mprEprintf("usage: %s [options] [filter paths]\n"
        "    --continue            # Continue on errors\n"
        "    --depth number        # Zero == basic, 1 == throrough, 2 extensive\n"
        "    --debug               # Run in debug mode\n"
        "    --echo                # Echo the command line\n"
        "    --iterations count    # Number of iterations to run the test\n"
        "    --log logFile:level   # Log to file file at verbosity level\n"
        "    --name testName       # Set test name\n"
        "    --step                # Single step tests\n"
        "    --threads count       # Number of test threads\n"
        "    --verbose             # Verbose mode\n"
        "    --version             # Output version information\n"
        "    --workers count       # Set maximum worker threads\n\n",
        programName);
        return MPR_ERR_BAD_ARGS;
    }
    if (outputVersion) {
        mprEprintf("%s: Version: %s\n", BIT_TITLE, BIT_VERSION);
        return MPR_ERR_BAD_ARGS;
    }
    sp->argc = argc;
    sp->argv = argv;
    sp->firstArg = nextArg;

    mprSetMaxWorkers(sp->workers);
    return 0;
}


static int parseFilter(MprTestService *sp, cchar *filter)
{
    char    *str, *word, *tok;

    assert(filter);
    if (filter == 0 || *filter == '\0') {
        return 0;
    }

    tok = 0;
    str = sclone(filter);
    word = stok(str, " \t\r\n", &tok);
    while (word) {
        if (mprAddItem(sp->testFilter, sclone(word)) < 0) {
            assert(!MPR_ERR_MEMORY);
            return MPR_ERR_MEMORY;
        }
        word = stok(0, " \t\r\n", &tok);
    }
    return 0;
}


static int loadTestModule(MprTestService *sp, cchar *fileName)
{
    MprModule   *mp;
    char        *cp, *base, entry[BIT_MAX_FNAME], path[BIT_MAX_FNAME];

    assert(fileName && *fileName);

    base = mprGetPathBase(fileName);
    assert(base);
    if ((cp = strrchr(base, '.')) != 0) {
        *cp = '\0';
    }
    if (mprLookupModule(base)) {
        return 0;
    }
    fmt(entry, sizeof(entry), "%sInit", base);
    if (fileName[0] == '/' || (*fileName && fileName[1] == ':')) {
        fmt(path, sizeof(path), "%s%s", fileName, BIT_SHOBJ);
    } else {
        fmt(path, sizeof(path), "./%s%s", fileName, BIT_SHOBJ);
    }
    if ((mp = mprCreateModule(base, path, entry, sp)) == 0) {
        mprError("Cannot create module %s", path);
        return -1;
    }
    if (mprLoadModule(mp) < 0) {
        mprError("Cannot load module %s", path);
        return -1;
    }
    return 0;
}


PUBLIC int mprRunTests(MprTestService *sp)
{
    MprTestGroup    *gp;
    MprThread       *tp;
    MprList         *lp;
    char            tName[64];
    int             i, next;

    /*
        Build the full names for all groups
     */
    next = 0; 
    while ((gp = mprGetNextItem(sp->groups, &next)) != 0) {
        buildFullNames(gp, gp->name);
    }
    sp->activeThreadCount = sp->numThreads;

    if (sp->echoCmdLine) {
        mprPrintf("%12s %s ... ", "[Test]", sp->commandLine);
        if (sp->verbose) {
            mprPrintf("\n");
        }
    }
    sp->start = mprGetTime();

    /*
        Create worker threads for each test thread. 
     */
    for (i = 0; i < sp->numThreads; i++) {
        fmt(tName, sizeof(tName), "test.%d", i);
        if ((lp = copyGroups(sp, sp->groups)) == 0) {
            assert(!MPR_ERR_MEMORY);
            return MPR_ERR_MEMORY;
        }
        if (mprAddItem(sp->threadData, lp) < 0) {
            assert(!MPR_ERR_MEMORY);
            return MPR_ERR_MEMORY;
        }
        /*
            Build the full names for all groups
         */
        next = 0; 
        while ((gp = mprGetNextItem(lp, &next)) != 0) {
            buildFullNames(gp, gp->name);
        }
        if ((tp = mprCreateThread(tName, relayEvent, lp, 0)) == 0) {
            assert(!MPR_ERR_MEMORY);
            return MPR_ERR_MEMORY;
        }
        if (mprStartThread(tp) < 0) {
            mprError("Cannot start thread %d", i);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    mprServiceEvents(-1, 0);
    return (sp->totalFailedCount == 0) ? 0 : 1;
}


static MprList *copyGroups(MprTestService *sp, MprList *groups)
{
    MprTestGroup    *gp, *newGp;
    MprList         *lp;
    int             next;

    if ((lp = mprCreateList(0, 0)) == NULL) {
        return 0;
    }
    next = 0; 
    while ((gp = mprGetNextItem(groups, &next)) != 0) {
        newGp = createTestGroup(sp, gp->def, NULL);
        if (newGp == 0) {
            return 0;
        }
        if (mprAddItem(lp, newGp) < 0) {
            return 0;
        }
    }
    return lp;
}


/*
    Relay the event to claim the dispatcher
 */
static void relayEvent(MprList *groups, MprThread *tp)
{
    MprTestGroup    *gp;

    gp = mprGetFirstItem(groups);
    assert(gp);

    mprRelayEvent(gp->dispatcher, runTestThread, groups, NULL);
    if (tp) {
        adjustThreadCount(-1);
    }
}


/*
    Run the test groups. One invocation per thread. Used even if not multithreaded.
 */
static void runTestThread(MprList *groups, MprThread *tp)
{
    MprTestService  *sp;
    MprTestGroup    *gp;
    int             next, i, count;

    sp = MPR->testService;
    assert(sp);

    for (next = 0; (gp = mprGetNextItem(groups, &next)) != 0; ) {
        runInit(gp);
    }
    count = 0;
    for (i = (sp->iterations + sp->numThreads - 1) / sp->numThreads; i > 0; i--) {
        if (sp->totalFailedCount > 0 && !sp->continueOnFailures) {
            break;
        }
        next = 0; 
        while ((gp = mprGetNextItem(groups, &next)) != 0) {
            runTestGroup(gp);
        }
        mprPrintf("%12s Iteration %d complete (%s)\n", "[Notice]", ++count, mprGetCurrentThreadName());
    }
    for (next = 0; (gp = mprGetNextItem(groups, &next)) != 0; ) {
        runTerm(gp);
    }
}


PUBLIC void mprReportTestResults(MprTestService *sp)
{
    if (sp->totalFailedCount == 0 && sp->verbose >= 1) {
        mprPrintf("%12s All tests PASSED for \"%s\"\n", "[REPORT]", sp->name);
    }
    if (sp->totalFailedCount > 0 || sp->verbose >= 2) {
        double  elapsed;
        elapsed = ((mprGetTime() - sp->start) * 1.0 / 1000.0);
        mprPrintf("%12s %d tests completed, %d test(s) failed.\n", 
            "[DETAILS]", sp->totalTestCount, sp->totalFailedCount);
        mprPrintf("%12s Elapsed time: %5.2f seconds.\n", "[BENCHMARK]", elapsed);
    }
    if (MPR->heap->track) {
        mprPrintMem("Memory Results", 1);
    }
}


static void buildFullNames(MprTestGroup *gp, cchar *name)
{
    MprTestGroup    *np;
    char            *nameBuf;
    cchar           *nameStack[MPR_TEST_MAX_STACK];
    int             next, tos;

    tos = 0;

    /*
        Build the full name for this case
     */
    nameStack[tos++] = name;
    for (np = gp->parent; np && np != np->parent && tos < MPR_TEST_MAX_STACK;  np = np->parent) {
        nameStack[tos++] = np->name;
    }
    nameBuf = sclone(gp->service->name);
    while (--tos >= 0) {
        nameBuf = sjoin(nameBuf, ".", nameStack[tos], NULL);
    }
    assert(gp->fullName == 0);
    gp->fullName = sclone(nameBuf);

    /*
        Recurse for all test case groups
     */
    next = 0;
    np = mprGetNextItem(gp->groups, &next);
    while (np) {
        buildFullNames(np, np->name);
        np = mprGetNextItem(gp->groups, &next);
    }
}


/*
    Used by main program to add the top level test group(s)
 */
PUBLIC MprTestGroup *mprAddTestGroup(MprTestService *sp, MprTestDef *def)
{
    MprTestGroup    *gp;

    gp = createTestGroup(sp, def, NULL);
    if (gp == 0) {
        return 0;
    }
    if (mprAddItem(sp->groups, gp) < 0) {
        return 0;
    }
    return gp;
}


static void manageTestGroup(MprTestGroup *gp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(gp->name);
        mprMark(gp->fullName);
        mprMark(gp->failures);
        mprMark(gp->service);
        mprMark(gp->dispatcher);
        mprMark(gp->parent);
        mprMark(gp->root);
        mprMark(gp->groups);
        mprMark(gp->cases);
        mprMark(gp->http);
        mprMark(gp->conn);
        mprMark(gp->content);
        mprMark(gp->data);
        mprMark(gp->mutex);
    }
}


static MprTestGroup *createTestGroup(MprTestService *sp, MprTestDef *def, MprTestGroup *parent)
{
    MprTestGroup    *gp, *child;
    MprTestDef      **dp;
    MprTestCase     *tc;
    char            name[80];
    static int      counter = 0;

    assert(sp);
    assert(def);

    gp = mprAllocObj(MprTestGroup, manageTestGroup);
    if (gp == 0) {
        return 0;
    }
    gp->service = sp;
    if (parent) {
        gp->dispatcher = parent->dispatcher;
    } else {
        fmt(name, sizeof(name), "Test-%d", counter++);
        gp->dispatcher = mprCreateDispatcher(name, 1);
    }

    gp->failures = mprCreateList(0, 0);
    if (gp->failures == 0) {
        return 0;
    }
    gp->cases = mprCreateList(0, MPR_LIST_STATIC_VALUES);
    if (gp->cases == 0) {
        return 0;
    }
    gp->groups = mprCreateList(0, 0);
    if (gp->groups == 0) {
        return 0;
    }
    gp->def = def;
    gp->name = sclone(def->name);
    gp->success = 1;

    for (tc = def->caseDefs; tc->proc; tc++) {
        if (mprAddItem(gp->cases, tc) < 0) {
            return 0;
        }
    }
    if (def->groupDefs) {
        for (dp = &def->groupDefs[0]; *dp && (*dp)->name; dp++) {
            child = createTestGroup(sp, *dp, gp);
            if (child == 0) {
                return 0;
            }
            if (mprAddItem(gp->groups, child) < 0) {
                return 0;
            }
            child->parent = gp;
            child->root = gp->parent;
        }
    }
    return gp;
}


PUBLIC void mprResetTestGroup(MprTestGroup *gp)
{
    gp->success = 1;
    gp->mutex = mprCreateLock();
}


static void runInit(MprTestGroup *parent)
{
    MprTestGroup    *gp;
    int             next;

    next = 0; 
    while ((gp = mprGetNextItem(parent->groups, &next)) != 0) {
        if (! filterTestGroup(gp)) {
            continue;
        }
        if (gp->def->init && (*gp->def->init)(gp) < 0) {
            gp->failedCount++;
            if (!gp->service->continueOnFailures) {
                break;
            }
        }
        runInit(gp);
    }
}


static void runTerm(MprTestGroup *parent)
{
    MprTestGroup    *gp;
    int             next;

    next = 0; 
    while ((gp = mprGetNextItem(parent->groups, &next)) != 0) {
        if (! filterTestGroup(gp)) {
            continue;
        }
        if (gp->def->term && (*gp->def->term)(gp) < 0) {
            gp->failedCount++;
            if (!gp->service->continueOnFailures) {
                break;
            }
        }
        runInit(gp);
    }
}


static void runTestGroup(MprTestGroup *parent)
{
    MprTestService  *sp;
    MprTestGroup    *gp, *nextGroup;
    MprTestCase     *tc;
    int             nextItem, count;

    sp = parent->service;

    /*
        Recurse over sub groups
     */
    nextItem = 0;
    gp = mprGetNextItem(parent->groups, &nextItem);
    while (gp && (parent->success || sp->continueOnFailures)) {
        nextGroup = mprGetNextItem(parent->groups, &nextItem);
        if (gp->testDepth > sp->testDepth) {
            gp = nextGroup;
            continue;
        }

        /*
            See if this group has been filtered for execution
         */
        if (! filterTestGroup(gp)) {
            gp = nextGroup;
            continue;
        }
        count = sp->totalFailedCount;
        if (count > 0 && !sp->continueOnFailures) {
            return;
        }

        /*
            Recurse over all tests in this group
         */
        runTestGroup(gp);
        gp->testCount++;

        if (! gp->success) {
            /*  Propagate the failures up the parent chain */
            parent->failedCount++;
            parent->success = 0;
        }
        gp = nextGroup;
    }

    /*
        Run test cases for this group
     */
    nextItem = 0;
    tc = mprGetNextItem(parent->cases, &nextItem);
    while (tc && (parent->success || sp->continueOnFailures)) {
        if (parent->testDepth <= sp->testDepth) {
            if (filterTestCast(parent, tc)) {
                runTestProc(parent, tc);
            }
        }
        tc = mprGetNextItem(parent->cases, &nextItem);
    }
}


/*
    Return true if we are to run the test group
 */
static bool filterTestGroup(MprTestGroup *gp)
{
    MprTestService  *sp;
    MprList         *testFilter;
    char            *pattern;
    ssize           len;
    int             next;

    sp = gp->service;
    testFilter = sp->testFilter;

    if (testFilter == 0) {
        return 1;
    }

    /*
        See if this test has been filtered
     */
    if (mprGetListLength(testFilter) > 0) {
        next = 0;
        pattern = mprGetNextItem(testFilter, &next);
        while (pattern) {
            len = min(slen(pattern), slen(gp->fullName));
            if (sncaselesscmp(gp->fullName, pattern, len) == 0) {
                break;
            }
            pattern = mprGetNextItem(testFilter, &next);
        }
        if (pattern == 0) {
            return 0;
        }
    }
    return 1;
}


/*
    Return true if we are to run the test case
 */
static bool filterTestCast(MprTestGroup *gp, MprTestCase *tc)
{
    MprTestService  *sp;
    MprList         *testFilter;
    char            *pattern, *fullName;
    ssize           len;
    int             next;

    sp = gp->service;
    testFilter = sp->testFilter;

    if (testFilter == 0) {
        return 1;
    }

    /*
        See if this test has been filtered
     */
    if (mprGetListLength(testFilter) > 0) {
        fullName = sfmt("%s.%s", gp->fullName, tc->name);
        next = 0;
        pattern = mprGetNextItem(testFilter, &next);
        while (pattern) {
            len = min(slen(pattern), slen(fullName));
            if (sncaselesscmp(fullName, pattern, len) == 0) {
                break;
            }
            pattern = mprGetNextItem(testFilter, &next);
        }
        if (pattern == 0) {
            return 0;
        }
    }
    return 1;
}


static void runTestProc(MprTestGroup *gp, MprTestCase *test)
{
    MprTestService      *sp;

    if (test->proc == 0) {
        return;
    }
    sp = gp->service;

    mprResetTestGroup(gp);

    if (sp->singleStep) {
        mprPrintf("%12s Run test \"%s.%s\", press <ENTER>: ", "[Test]", gp->fullName, test->name);
        getchar();

    } else if (sp->verbose) {
        mprPrintf("%12s Run test \"%s.%s\": ", "[Test]", gp->fullName, test->name);
    }
    if (gp->skip) {
        if (sp->verbose) {
            if (gp->skipWarned++ == 0) {
                mprPrintf("%12s Skipping test: \"%s.%s\": \n", "[Skip]", gp->fullName, test->name);
            }
        }
    } else {
        (test->proc)(gp);
        mprYield(0);
    
        mprLock(sp->mutex);
        if (gp->success) {
            ++sp->totalTestCount;
            if (sp->verbose) {
                mprPrintf("PASSED\n");
            }
        } else {
            mprEprintf("FAILED test \"%s.%s\"\nDetails: %s\n", gp->fullName, test->name, getErrorMessage(gp));
        }
    }
    mprUnlock(sp->mutex);
}


static char *getErrorMessage(MprTestGroup *gp)
{
    MprTestFailure  *fp;
    char            msg[BIT_MAX_BUFFER], *errorMsg;
    int             next;

    next = 0;
    errorMsg = sclone("");
    fp = mprGetNextItem(gp->failures, &next);
    while (fp) {
        fmt(msg, sizeof(msg), "Failure in %s\nAssertion: \"%s\"\n", fp->loc, fp->message);
        if ((errorMsg = sjoin(errorMsg, msg, NULL)) == NULL) {
            break;
        }
        fp = mprGetNextItem(gp->failures, &next);
    }
    return errorMsg;
}


static int addFailure(MprTestGroup *gp, cchar *loc, cchar *message)
{
    MprTestFailure  *fp;

    fp = createFailure(gp, loc, message);
    if (fp == 0) {
        assert(fp);
        assert(!MPR_ERR_MEMORY);
        return MPR_ERR_MEMORY;
    }
    mprAddItem(gp->failures, fp);
    return 0;
}


static void manageTestFailure(MprTestFailure *fp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(fp->loc);
        mprMark(fp->message);
    }
}


static MprTestFailure *createFailure(MprTestGroup *gp, cchar *loc, cchar *message)
{
    MprTestFailure  *fp;

    if ((fp = mprAllocObj(MprTestFailure, manageTestFailure)) == 0) {
        return 0;
    }
    fp->loc = sclone(loc);
    fp->message = sclone(message);
    return fp;
}


PUBLIC bool assertTrue(MprTestGroup *gp, cchar *loc, bool isTrue, cchar *msg)
{
    if (! isTrue) {
        gp->success = isTrue;
    }
    if (! isTrue) {
        if (gp->service->debugOnFailures) {
            mprBreakpoint();
        }
        addFailure(gp, loc, msg);
        gp->failedCount++;
        adjustFailedCount(1);
    }
    return isTrue;
}


PUBLIC bool mprWaitForTestToComplete(MprTestGroup *gp, MprTicks timeout)
{
    MprTicks    expires, remaining;
    int         rc;
    
    assert(gp->dispatcher);
    assert(timeout >= 0);

    if (mprGetDebugMode()) {
        timeout *= 100;
    }
    expires = mprGetTicks() + timeout;
    remaining = timeout;
    do {
        mprWaitForEvent(gp->dispatcher, remaining);
        remaining = expires - mprGetTicks();
    } while (!gp->testComplete && remaining > 0);
    rc = gp->testComplete;
    gp->testComplete = 0;
    return rc;
}


PUBLIC void mprSignalTestComplete(MprTestGroup *gp)
{
    gp->testComplete = 1;
    mprSignalDispatcher(gp->dispatcher);
}


static void adjustThreadCount(int adj)
{
    MprTestService  *sp;

    sp = MPR->testService;
    mprLock(sp->mutex);
    sp->activeThreadCount += adj;
    if (sp->activeThreadCount <= 0) {
        mprTerminate(MPR_EXIT_DEFAULT, 0);
    }
    mprUnlock(sp->mutex);
}


static void adjustFailedCount(int adj)
{
    MprTestService  *sp;

    sp = MPR->testService;
    mprLock(sp->mutex);
    sp->totalFailedCount += adj;
    mprUnlock(sp->mutex);
}


static void logHandler(int flags, int level, cchar *msg)
{
    MprFile     *file;
    char        *prefix;

    file = MPR->logFile;
    prefix = MPR->name;

    while (*msg == '\n') {
        mprFprintf(file, "\n");
        msg++;
    }
    if (flags & MPR_LOG_MSG) {
        mprFprintf(file, "%s: %d: %s\n", prefix, level, msg);
    } else if (flags & MPR_ERROR_MSG) {
        mprFprintf(file, "%s: Error: %s\n", prefix, msg);
    } else if (flags & MPR_FATAL_MSG) {
        mprFprintf(file, "%s: Fatal: %s\n", prefix, msg);
    } else if (flags & MPR_RAW_MSG) {
        mprFprintf(file, "%s", msg);
    }
    if (flags & (MPR_ERROR_MSG | MPR_FATAL_MSG)) {
        mprBreakpoint();
    }
}


static int setLogging(char *logSpec)
{
    MprFile     *file;
    char        *levelSpec;
    int         level;

    level = 0;

    if ((levelSpec = strchr(logSpec, ':')) != 0) {
        *levelSpec++ = '\0';
        level = atoi(levelSpec);
    }

    if (strcmp(logSpec, "stdout") == 0) {
        file = MPR->stdOutput;

    } else if (strcmp(logSpec, "stderr") == 0) {
        file = MPR->stdError;

    } else {
        if ((file = mprOpenFile(logSpec, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
            mprEprintf("Cannot open log file %s\n", logSpec);
            return MPR_ERR_CANT_OPEN;
        }
    }
    mprSetLogLevel(level);
    mprSetLogHandler(logHandler);
    mprSetLogFile(file);
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/thread.c"
 */
/************************************************************************/

/**
    thread.c - Primitive multi-threading support for Windows

    This module provides threading, mutex and condition variable APIs.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes **********************************/



/*************************** Forward Declarations ****************************/

static void changeState(MprWorker *worker, int state);
static MprWorker *createWorker(MprWorkerService *ws, ssize stackSize);
static int getNextThreadNum(MprWorkerService *ws);
static void manageThreadService(MprThreadService *ts, int flags);
static void manageThread(MprThread *tp, int flags);
static void manageWorker(MprWorker *worker, int flags);
static void manageWorkerService(MprWorkerService *ws, int flags);
static void pruneWorkers(MprWorkerService *ws, MprEvent *timer);
static void threadProc(MprThread *tp);
static void workerMain(MprWorker *worker, MprThread *tp);

/************************************ Code ***********************************/

PUBLIC MprThreadService *mprCreateThreadService()
{
    MprThreadService    *ts;

    if ((ts = mprAllocObj(MprThreadService, manageThreadService)) == 0) {
        return 0;
    }
    if ((ts->cond = mprCreateCond()) == 0) {
        return 0;
    }
    if ((ts->threads = mprCreateList(-1, 0)) == 0) {
        return 0;
    }
    MPR->mainOsThread = mprGetCurrentOsThread();
    MPR->threadService = ts;
    ts->stackSize = BIT_MPR_THREAD_STACK;
    /*
        Don't actually create the thread. Just create a thread object for this main thread.
     */
    if ((ts->mainThread = mprCreateThread("main", NULL, NULL, 0)) == 0) {
        return 0;
    }
    ts->mainThread->isMain = 1;
    ts->mainThread->osThread = mprGetCurrentOsThread();
    return ts;
}


PUBLIC void mprStopThreadService()
{
}


static void manageThreadService(MprThreadService *ts, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ts->threads);
        mprMark(ts->mainThread);
        mprMark(ts->cond);

    } else if (flags & MPR_MANAGE_FREE) {
        mprStopThreadService();
    }
}


PUBLIC void mprSetThreadStackSize(ssize size)
{
    MPR->threadService->stackSize = size;
}


PUBLIC MprThread *mprGetCurrentThread()
{
    MprThreadService    *ts;
    MprThread           *tp;
    MprOsThread         id;
    int                 i;

    ts = MPR->threadService;
    id = mprGetCurrentOsThread();
    lock(ts->threads);
    for (i = 0; i < ts->threads->length; i++) {
        tp = mprGetItem(ts->threads, i);
        if (tp->osThread == id) {
            unlock(ts->threads);
            return tp;
        }
    }
    unlock(ts->threads);
    return 0;
}


PUBLIC cchar *mprGetCurrentThreadName()
{
    MprThread       *tp;

    if ((tp = mprGetCurrentThread()) == 0) {
        return 0;
    }
    return tp->name;
}


/*
    Return the current thread object
 */
PUBLIC void mprSetCurrentThreadPriority(int pri)
{
    MprThread       *tp;

    if ((tp = mprGetCurrentThread()) == 0) {
        return;
    }
    mprSetThreadPriority(tp, pri);
}


/*
    Create a main thread
 */
PUBLIC MprThread *mprCreateThread(cchar *name, void *entry, void *data, ssize stackSize)
{
    MprThreadService    *ts;
    MprThread           *tp;

    ts = MPR->threadService;
    tp = mprAllocObj(MprThread, manageThread);
    if (tp == 0) {
        return 0;
    }
    tp->data = data;
    tp->entry = entry;
    tp->name = sclone(name);
    tp->mutex = mprCreateLock();
    tp->cond = mprCreateCond();
    tp->pid = getpid();
    tp->priority = MPR_NORMAL_PRIORITY;

    if (stackSize == 0) {
        tp->stackSize = ts->stackSize;
    } else {
        tp->stackSize = stackSize;
    }
#if BIT_WIN_LIKE
    tp->threadHandle = 0;
#endif
    assert(ts);
    assert(ts->threads);
    if (mprAddItem(ts->threads, tp) < 0) {
        return 0;
    }
    return tp;
}


static void manageThread(MprThread *tp, int flags)
{
    MprThreadService    *ts;

    ts = MPR->threadService;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(tp->name);
        mprMark(tp->data);
        mprMark(tp->cond);
        mprMark(tp->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        if (ts->threads) {
            mprRemoveItem(ts->threads, tp);
        }
#if BIT_WIN_LIKE
        if (tp->threadHandle) {
            CloseHandle(tp->threadHandle);
        }
#endif
    }
}


/*
    Entry thread function
 */ 
#if BIT_WIN_LIKE
static uint __stdcall threadProcWrapper(void *data) 
{
    threadProc((MprThread*) data);
    return 0;
}
#elif VXWORKS

static int threadProcWrapper(void *data) 
{
    threadProc((MprThread*) data);
    return 0;
}

#else
PUBLIC void *threadProcWrapper(void *data) 
{
    threadProc((MprThread*) data);
    return 0;
}

#endif


/*
    Thread entry
 */
static void threadProc(MprThread *tp)
{
    assert(tp);

    tp->osThread = mprGetCurrentOsThread();

#if VXWORKS
    tp->pid = tp->osThread;
#else
    tp->pid = getpid();
#endif
    (tp->entry)(tp->data, tp);
    mprRemoveItem(MPR->threadService->threads, tp);
}


/*
    Start a thread
 */
PUBLIC int mprStartThread(MprThread *tp)
{
    lock(tp);

#if BIT_WIN_LIKE
{
    HANDLE          h;
    uint            threadId;

#if WINCE
    h = (HANDLE) CreateThread(NULL, 0, threadProcWrapper, (void*) tp, 0, &threadId);
#else
    h = (HANDLE) _beginthreadex(NULL, 0, threadProcWrapper, (void*) tp, 0, &threadId);
#endif
    if (h == NULL) {
        unlock(tp);
        return MPR_ERR_CANT_INITIALIZE;
    }
    tp->osThread = (int) threadId;
    tp->threadHandle = (HANDLE) h;
}
#elif VXWORKS
{
    int     taskHandle, pri;

    taskPriorityGet(taskIdSelf(), &pri);
    taskHandle = taskSpawn(tp->name, pri, VX_FP_TASK, tp->stackSize, (FUNCPTR) threadProcWrapper, (int) tp, 
        0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (taskHandle < 0) {
        mprError("Cannot create thread %s\n", tp->name);
        unlock(tp);
        return MPR_ERR_CANT_INITIALIZE;
    }
}
#else /* UNIX */
{
    pthread_attr_t  attr;
    pthread_t       h;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_attr_setstacksize(&attr, tp->stackSize);

    if (pthread_create(&h, &attr, threadProcWrapper, (void*) tp) != 0) { 
        assert(0);
        pthread_attr_destroy(&attr);
        unlock(tp);
        return MPR_ERR_CANT_CREATE;
    }
    pthread_attr_destroy(&attr);
}
#endif
    unlock(tp);
    return 0;
}


PUBLIC MprOsThread mprGetCurrentOsThread()
{
#if BIT_UNIX_LIKE
    return (MprOsThread) pthread_self();
#elif BIT_WIN_LIKE
    return (MprOsThread) GetCurrentThreadId();
#elif VXWORKS
    return (MprOsThread) taskIdSelf();
#endif
}


PUBLIC void mprSetThreadPriority(MprThread *tp, int newPriority)
{
    int     osPri;

    lock(tp);
    osPri = mprMapMprPriorityToOs(newPriority);

#if BIT_WIN_LIKE
    SetThreadPriority(tp->threadHandle, osPri);
#elif VXWORKS
    taskPrioritySet(tp->osThread, osPri);
#else
    setpriority(PRIO_PROCESS, (int) tp->pid, osPri);
#endif
    tp->priority = newPriority;
    unlock(tp);
}


static void manageThreadLocal(MprThreadLocal *tls, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
#if !BIT_UNIX_LIKE && !BIT_WIN_LIKE
        mprMark(tls->store);
#endif
    } else if (flags & MPR_MANAGE_FREE) {
#if BIT_UNIX_LIKE
        if (tls->key) {
            pthread_key_delete(tls->key);
        }
#elif BIT_WIN_LIKE
        if (tls->key >= 0) {
            TlsFree(tls->key);
        }
#endif
    }
}


PUBLIC MprThreadLocal *mprCreateThreadLocal()
{
    MprThreadLocal      *tls;

    if ((tls = mprAllocObj(MprThreadLocal, manageThreadLocal)) == 0) {
        return 0;
    }
#if BIT_UNIX_LIKE
    if (pthread_key_create(&tls->key, NULL) != 0) {
        tls->key = 0;
        return 0;
    }
#elif BIT_WIN_LIKE
    if ((tls->key = TlsAlloc()) < 0) {
        return 0;
    }
#else
    if ((tls->store = mprCreateHash(0, MPR_HASH_STATIC_VALUES)) == 0) {
        return 0;
    }
#endif
    return tls;
}


PUBLIC int mprSetThreadData(MprThreadLocal *tls, void *value)
{
    bool    err;

#if BIT_UNIX_LIKE
    err = pthread_setspecific(tls->key, value) != 0;
#elif BIT_WIN_LIKE
    err = TlsSetValue(tls->key, value) != 0;
#else
    {
        char    key[32];
        itosbuf(key, sizeof(key), (int64) mprGetCurrentOsThread(), 10);
        err = mprAddKey(tls->store, key, value) == 0;
    }
#endif
    return (err) ? MPR_ERR_CANT_WRITE: 0;
}


PUBLIC void *mprGetThreadData(MprThreadLocal *tls)
{
#if BIT_UNIX_LIKE
    return pthread_getspecific(tls->key);
#elif BIT_WIN_LIKE
    return TlsGetValue(tls->key);
#else
    {
        char    key[32];
        itosbuf(key, sizeof(key), (int64) mprGetCurrentOsThread(), 10);
        return mprLookupKey(tls->store, key);
    }
#endif
}


#if BIT_WIN_LIKE
/*
    Map Mpr priority to Windows native priority. Windows priorities range from -15 to +15 (zero is normal). 
    Warning: +15 will not yield the CPU, -15 may get starved. We should be very wary going above +11.
 */

PUBLIC int mprMapMprPriorityToOs(int mprPriority)
{
    assert(mprPriority >= 0 && mprPriority <= 100);
 
    if (mprPriority <= MPR_BACKGROUND_PRIORITY) {
        return THREAD_PRIORITY_LOWEST;
    } else if (mprPriority <= MPR_LOW_PRIORITY) {
        return THREAD_PRIORITY_BELOW_NORMAL;
    } else if (mprPriority <= MPR_NORMAL_PRIORITY) {
        return THREAD_PRIORITY_NORMAL;
    } else if (mprPriority <= MPR_HIGH_PRIORITY) {
        return THREAD_PRIORITY_ABOVE_NORMAL;
    } else {
        return THREAD_PRIORITY_HIGHEST;
    }
}


/*
    Map Windows priority to Mpr priority
 */ 
PUBLIC int mprMapOsPriorityToMpr(int nativePriority)
{
    int     priority;

    priority = (45 * nativePriority) + 50;
    if (priority < 0) {
        priority = 0;
    }
    if (priority >= 100) {
        priority = 99;
    }
    return priority;
}


#elif VXWORKS
/*
    Map MPR priority to VxWorks native priority.
 */

PUBLIC int mprMapMprPriorityToOs(int mprPriority)
{
    int     nativePriority;

    assert(mprPriority >= 0 && mprPriority < 100);

    nativePriority = (100 - mprPriority) * 5 / 2;

    if (nativePriority < 10) {
        nativePriority = 10;
    } else if (nativePriority > 255) {
        nativePriority = 255;
    }
    return nativePriority;
}


/*
    Map O/S priority to Mpr priority.
 */ 
PUBLIC int mprMapOsPriorityToMpr(int nativePriority)
{
    int     priority;

    priority = (255 - nativePriority) * 2 / 5;
    if (priority < 0) {
        priority = 0;
    }
    if (priority >= 100) {
        priority = 99;
    }
    return priority;
}


#else /* UNIX */
/*
    Map MR priority to linux native priority. Unix priorities range from -19 to +19. Linux does -20 to +19. 
 */
PUBLIC int mprMapMprPriorityToOs(int mprPriority)
{
    assert(mprPriority >= 0 && mprPriority < 100);

    if (mprPriority <= MPR_BACKGROUND_PRIORITY) {
        return 19;
    } else if (mprPriority <= MPR_LOW_PRIORITY) {
        return 10;
    } else if (mprPriority <= MPR_NORMAL_PRIORITY) {
        return 0;
    } else if (mprPriority <= MPR_HIGH_PRIORITY) {
        return -8;
    } else {
        return -19;
    }
    assert(0);
    return 0;
}


/*
    Map O/S priority to Mpr priority.
 */ 
PUBLIC int mprMapOsPriorityToMpr(int nativePriority)
{
    int     priority;

    priority = (nativePriority + 19) * (100 / 40); 
    if (priority < 0) {
        priority = 0;
    }
    if (priority >= 100) {
        priority = 99;
    }
    return priority;
}

#endif /* UNIX */


PUBLIC MprWorkerService *mprCreateWorkerService()
{
    MprWorkerService      *ws;

    ws = mprAllocObj(MprWorkerService, manageWorkerService);
    if (ws == 0) {
        return 0;
    }
    ws->mutex = mprCreateLock();
    ws->minThreads = MPR_DEFAULT_MIN_THREADS;
    ws->maxThreads = MPR_DEFAULT_MAX_THREADS;

    /*
        Presize the lists so they cannot get memory allocation failures later on.
     */
    ws->idleThreads = mprCreateList(0, 0);
    mprSetListLimits(ws->idleThreads, ws->maxThreads, -1);
    ws->busyThreads = mprCreateList(0, 0);
    mprSetListLimits(ws->busyThreads, ws->maxThreads, -1);
    return ws;
}


static void manageWorkerService(MprWorkerService *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->busyThreads);
        mprMark(ws->idleThreads);
        mprMark(ws->mutex);
        mprMark(ws->pruneTimer);
    }
}


PUBLIC int mprStartWorkerService()
{
    MprWorkerService    *ws;

    /*
        Create a timer to trim excess workers
     */
    ws = MPR->workerService;
    mprSetMinWorkers(ws->minThreads);
    ws->pruneTimer = mprCreateTimerEvent(NULL, "pruneWorkers", MPR_TIMEOUT_PRUNER, pruneWorkers, ws, MPR_EVENT_QUICK);
    return 0;
}


PUBLIC void mprWakeWorkers()
{
    MprWorkerService    *ws;
    MprWorker           *worker;
    int                 next;

    ws = MPR->workerService;
    lock(ws);
    if (ws->pruneTimer) {
        mprRemoveEvent(ws->pruneTimer);
    }
    /*
        Wake up all idle workers. Busy workers take care of themselves. An idle thread will wakeup, exit and be 
        removed from the busy list and then delete the thread. We progressively remove the last thread in the idle
        list. ChangeState will move the workers to the busy queue.
     */
    for (next = -1; (worker = (MprWorker*) mprGetPrevItem(ws->idleThreads, &next)) != 0; ) {
        changeState(worker, MPR_WORKER_BUSY);
    }
    unlock(ws);
}


/*
    Define the new minimum number of workers. Pre-allocate the minimum.
 */
PUBLIC void mprSetMinWorkers(int n)
{ 
    MprWorker           *worker;
    MprWorkerService    *ws;

    ws = MPR->workerService;
    lock(ws);
    ws->minThreads = n; 
    if (n > 0) {
        mprTrace(4, "Pre-start %d workers", ws->minThreads);
    }
    while (ws->numThreads < ws->minThreads) {
        worker = createWorker(ws, ws->stackSize);
        ws->numThreads++;
        ws->maxUsedThreads = max(ws->numThreads, ws->maxUsedThreads);
        changeState(worker, MPR_WORKER_BUSY);
        mprStartThread(worker->thread);
    }
    unlock(ws);
}


/*
    Define a new maximum number of theads. Prune if currently over the max.
 */
PUBLIC void mprSetMaxWorkers(int n)
{
    MprWorkerService  *ws;

    ws = MPR->workerService;

    lock(ws);
    ws->maxThreads = n; 
    if (ws->numThreads > ws->maxThreads) {
        pruneWorkers(ws, 0);
    }
    if (ws->minThreads > ws->maxThreads) {
        ws->minThreads = ws->maxThreads;
    }
    unlock(ws);
}


PUBLIC int mprGetMaxWorkers()
{
    return MPR->workerService->maxThreads;
}


/*
    Return the current worker thread object
 */
PUBLIC MprWorker *mprGetCurrentWorker()
{
    MprWorkerService    *ws;
    MprWorker           *worker;
    MprThread           *thread;
    int                 next;

    ws = MPR->workerService;

    lock(ws);
    thread = mprGetCurrentThread();
    for (next = -1; (worker = (MprWorker*) mprGetPrevItem(ws->busyThreads, &next)) != 0; ) {
        if (worker->thread == thread) {
            unlock(ws);
            return worker;
        }
    }
    unlock(ws);
    return 0;
}


PUBLIC void mprActivateWorker(MprWorker *worker, MprWorkerProc proc, void *data)
{
    MprWorkerService    *ws;

    ws = worker->workerService;

    lock(ws);
    worker->proc = proc;
    worker->data = data;
    changeState(worker, MPR_WORKER_BUSY);
    unlock(ws);
}


PUBLIC void mprSetWorkerStartCallback(MprWorkerProc start)
{
    MPR->workerService->startWorker = start;
}


PUBLIC void mprGetWorkerStats(MprWorkerStats *stats)
{
    MprWorkerService    *ws;
    MprWorker           *wp;
    int                 next;

    ws = MPR->workerService;

    lock(ws);
    stats->max = ws->maxThreads;
    stats->min = ws->minThreads;
    stats->maxUsed = ws->maxUsedThreads;

    stats->idle = (int) ws->idleThreads->length;
    stats->busy = (int) ws->busyThreads->length;

    stats->yielded = 0;
    for (ITERATE_ITEMS(ws->busyThreads, wp, next)) {
        /*
            Only count as yielded, those workers who call yield from their user code
            This excludes the yield in worker main
         */
        stats->yielded += (wp->thread->yielded && wp->running);
    }
    unlock(ws);
}


PUBLIC int mprAvailableWorkers()
{
    MprWorkerStats  wstats;
    int             activeWorkers, spareThreads, spareCores, result;

    mprGetWorkerStats(&wstats);
    spareThreads = wstats.max - wstats.busy - wstats.idle;
    activeWorkers = wstats.busy - wstats.yielded;
    spareCores = MPR->heap->stats.numCpu - activeWorkers;
    if (spareCores <= 0 || spareThreads <= 0) {
        return 0;
    }
    result = wstats.idle + min(spareThreads, spareCores);
#if KEEP
    printf("Avail %d, busy %d, yielded %d, idle %d, spare-threads %d, spare-cores %d, mustYield %d\n", result, wstats.busy,
        wstats.yielded, wstats.idle, spareThreads, spareCores, MPR->heap->mustYield);
#endif
    return result;
}


PUBLIC int mprStartWorker(MprWorkerProc proc, void *data)
{
    MprWorkerService    *ws;
    MprWorker           *worker;

    ws = MPR->workerService;
    lock(ws);

    /*
        Try to find an idle thread and wake it up. It will wakeup in workerMain(). If not any available, then add 
        another thread to the worker. Must account for workers we've already created but have not yet gone to work 
        and inserted themselves in the idle/busy queues. Get most recently used idle worker so we tend to reuse 
        active threads. This lets the pruner trim idle workers.
     */
    worker = mprGetLastItem(ws->idleThreads);
    if (worker) {
        worker->data = data;
        worker->proc = proc;
        changeState(worker, MPR_WORKER_BUSY);

    } else if (ws->numThreads < ws->maxThreads) {
        if (mprAvailableWorkers() == 0) {
            unlock(ws);
            return MPR_ERR_BUSY;
        }
        worker = createWorker(ws, ws->stackSize);
        ws->numThreads++;
        ws->maxUsedThreads = max(ws->numThreads, ws->maxUsedThreads);
        worker->data = data;
        worker->proc = proc;
        changeState(worker, MPR_WORKER_BUSY);
        mprStartThread(worker->thread);

    } else {
        unlock(ws);
        return MPR_ERR_BUSY;
    }
    unlock(ws);
    return 0;
}


/*
    Trim idle workers
 */
static void pruneWorkers(MprWorkerService *ws, MprEvent *timer)
{
    MprWorker     *worker;
    int           index, pruned;

    if (mprGetDebugMode()) {
        return;
    }
    lock(ws);
    pruned = 0;
    for (index = 0; index < ws->idleThreads->length; index++) {
        if (ws->numThreads <= ws->minThreads) {
            break;
        }
        worker = mprGetItem(ws->idleThreads, index);
        if ((worker->lastActivity + MPR_TIMEOUT_WORKER) < MPR->eventService->now) {
            changeState(worker, MPR_WORKER_PRUNED);
            pruned++;
            index--;
        }
    }
    if (pruned) {
        mprLog(2, "Pruned %d workers, pool has %d workers. Limits %d-%d.", 
            pruned, ws->numThreads - pruned, ws->minThreads, ws->maxThreads);
    }
    unlock(ws);
}


static int getNextThreadNum(MprWorkerService *ws)
{
    int     rc;

    lock(ws);
    rc = ws->nextThreadNum++;
    unlock(ws);
    return rc;
}


/*
    Define a new stack size for new workers. Existing workers unaffected.
 */
PUBLIC void mprSetWorkerStackSize(int n)
{
    MPR->workerService->stackSize = n; 
}


/*
    Create a new thread for the task
 */
static MprWorker *createWorker(MprWorkerService *ws, ssize stackSize)
{
    MprWorker   *worker;

    char    name[16];

    if ((worker = mprAllocObj(MprWorker, manageWorker)) == 0) {
        return 0;
    }
    worker->workerService = ws;
    worker->idleCond = mprCreateCond();

    fmt(name, sizeof(name), "worker.%u", getNextThreadNum(ws));
    mprLog(2, "Create %s, pool has %d workers. Limits %d-%d.", name, ws->numThreads + 1, ws->minThreads, ws->maxThreads);
    worker->thread = mprCreateThread(name, (MprThreadProc) workerMain, worker, stackSize);
    return worker;
}


static void manageWorker(MprWorker *worker, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(worker->data);
        mprMark(worker->thread);
        mprMark(worker->workerService);
        mprMark(worker->idleCond);
    }
}


static void workerMain(MprWorker *worker, MprThread *tp)
{
    MprWorkerService    *ws;

    ws = MPR->workerService;
    assert(worker->state == MPR_WORKER_BUSY);
    assert(!worker->idleCond->triggered);

    if (ws->startWorker) {
        (*ws->startWorker)(worker->data, worker);
    }
    /*
        Very important for performance to elimminate to locking the WorkerService
     */
    while (!(worker->state & MPR_WORKER_PRUNED)) {
        if (worker->proc) {
            worker->running = 1;
            (*worker->proc)(worker->data, worker);
            worker->running = 0;
        }
        worker->lastActivity = MPR->eventService->now;
        if (mprIsStopping()) {
            break;
        }
        assert(worker->cleanup == 0);
        if (worker->cleanup) {
            (*worker->cleanup)(worker->data, worker);
            worker->cleanup = NULL;
        }
        worker->proc = 0;
        worker->data = 0;
        changeState(worker, MPR_WORKER_IDLE);

        /*
            Sleep till there is more work to do. Yield for GC first.
         */
        mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
        mprWaitForCond(worker->idleCond, -1);
        mprResetYield();
    }
    lock(ws);
    changeState(worker, 0);
    worker->thread = 0;
    ws->numThreads--;
    unlock(ws);
    mprLog(4, "Worker exiting. There are %d workers remaining in the pool.", ws->numThreads);
}


static void changeState(MprWorker *worker, int state)
{
    MprWorkerService    *ws;
    MprList             *lp;
    int                 wake;

    if (state == worker->state) {
        return;
    }
    wake = 0;
    lp = 0;
    ws = worker->workerService;
    lock(ws);

    switch (worker->state) {
    case MPR_WORKER_BUSY:
        lp = ws->busyThreads;
        break;

    case MPR_WORKER_IDLE:
        lp = ws->idleThreads;
        wake = 1;
        break;
        
    case MPR_WORKER_PRUNED:
        break;
    }

    /*
        Reassign the worker to the appropriate queue
     */
    if (lp) {
        mprRemoveItem(lp, worker);
    }
    lp = 0;
    switch (state) {
    case MPR_WORKER_BUSY:
        lp = ws->busyThreads;
        break;

    case MPR_WORKER_IDLE:
        lp = ws->idleThreads;
        mprWakePendingDispatchers();
        break;

    case MPR_WORKER_PRUNED:
        /* Don't put on a queue and the thread will exit */
        mprWakePendingDispatchers();
        break;
    }
    worker->state = state;

    if (lp) {
        if (mprAddItem(lp, worker) < 0) {
            unlock(ws);
            assert(!MPR_ERR_MEMORY);
            return;
        }
    }
    unlock(ws);
    if (wake) {
        mprSignalCond(worker->idleCond); 
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/time.c"
 */
/************************************************************************/

/**
    time.c - Date and Time handling
 
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Defines ***********************************/

#define MS_PER_SEC  (MPR_TICKS_PER_SEC)
#define MS_PER_HOUR (60 * 60 * MPR_TICKS_PER_SEC)
#define MS_PER_MIN  (60 * MPR_TICKS_PER_SEC)
#define MS_PER_DAY  (86400 * MPR_TICKS_PER_SEC)
#define MS_PER_YEAR (INT64(31556952000))

/*
    On some platforms, time_t is only 32 bits (linux-32) and on some 64 bit systems, time calculations
    outside the range of 32 bits are unreliable. This means there is a minimum and maximum year that 
    can be analysed using the O/S localtime routines. However, we really want to use the O/S 
    calculations for daylight savings time, so when a date is outside a 32 bit time_t range, we use
    some trickery to remap the year to a temporary (current) year so localtime can be used.
    FYI: 32 bit time_t expires at: 03:14:07 UTC on Tuesday, 19 January 2038
 */
#define MIN_YEAR    1901
#define MAX_YEAR    2037

/*
    MacOSX can't handle MIN_TIME == -0x7FFFFFFF
 */
#define MAX_TIME    0x7FFFFFFF
#define MIN_TIME    -0xFFFFFFF

/*
    Token types or'd into the TimeToken value
 */
#define TOKEN_DAY       0x01000000
#define TOKEN_MONTH     0x02000000
#define TOKEN_ZONE      0x04000000
#define TOKEN_OFFSET    0x08000000
#define TOKEN_MASK      0xFF000000

typedef struct TimeToken {
    char    *name;
    int     value;
} TimeToken;

static TimeToken days[] = {
    { "sun",  0 | TOKEN_DAY },
    { "mon",  1 | TOKEN_DAY },
    { "tue",  2 | TOKEN_DAY },
    { "wed",  3 | TOKEN_DAY },
    { "thu",  4 | TOKEN_DAY },
    { "fri",  5 | TOKEN_DAY },
    { "sat",  6 | TOKEN_DAY },
    { 0, 0 },
};

static TimeToken fullDays[] = {
    { "sunday",     0 | TOKEN_DAY },
    { "monday",     1 | TOKEN_DAY },
    { "tuesday",    2 | TOKEN_DAY },
    { "wednesday",  3 | TOKEN_DAY },
    { "thursday",   4 | TOKEN_DAY },
    { "friday",     5 | TOKEN_DAY },
    { "saturday",   6 | TOKEN_DAY },
    { 0, 0 },
};

/*
    Make origin 1 to correspond to user date entries 10/28/2013
 */
static TimeToken months[] = {
    { "jan",  1 | TOKEN_MONTH },
    { "feb",  2 | TOKEN_MONTH },
    { "mar",  3 | TOKEN_MONTH },
    { "apr",  4 | TOKEN_MONTH },
    { "may",  5 | TOKEN_MONTH },
    { "jun",  6 | TOKEN_MONTH },
    { "jul",  7 | TOKEN_MONTH },
    { "aug",  8 | TOKEN_MONTH },
    { "sep",  9 | TOKEN_MONTH },
    { "oct", 10 | TOKEN_MONTH },
    { "nov", 11 | TOKEN_MONTH },
    { "dec", 12 | TOKEN_MONTH },
    { 0, 0 },
};

static TimeToken fullMonths[] = {
    { "january",    1 | TOKEN_MONTH },
    { "february",   2 | TOKEN_MONTH },
    { "march",      3 | TOKEN_MONTH },
    { "april",      4 | TOKEN_MONTH },
    { "may",        5 | TOKEN_MONTH },
    { "june",       6 | TOKEN_MONTH },
    { "july",       7 | TOKEN_MONTH },
    { "august",     8 | TOKEN_MONTH },
    { "september",  9 | TOKEN_MONTH },
    { "october",   10 | TOKEN_MONTH },
    { "november",  11 | TOKEN_MONTH },
    { "december",  12 | TOKEN_MONTH },
    { 0, 0 }
};

static TimeToken ampm[] = {
    { "am", 0 | TOKEN_OFFSET },
    { "pm", (12 * 3600) | TOKEN_OFFSET },
    { 0, 0 },
};


static TimeToken zones[] = {
    { "ut",      0 | TOKEN_ZONE},
    { "utc",     0 | TOKEN_ZONE},
    { "gmt",     0 | TOKEN_ZONE},
    { "edt",  -240 | TOKEN_ZONE},
    { "est",  -300 | TOKEN_ZONE},
    { "cdt",  -300 | TOKEN_ZONE},
    { "cst",  -360 | TOKEN_ZONE},
    { "mdt",  -360 | TOKEN_ZONE},
    { "mst",  -420 | TOKEN_ZONE},
    { "pdt",  -420 | TOKEN_ZONE},
    { "pst",  -480 | TOKEN_ZONE},
    { 0, 0 },
};


static TimeToken offsets[] = {
    { "tomorrow",    86400 | TOKEN_OFFSET},
    { "yesterday",  -86400 | TOKEN_OFFSET},
    { "next week",   (86400 * 7) | TOKEN_OFFSET},
    { "last week",  -(86400 * 7) | TOKEN_OFFSET},
    { 0, 0 },
};

static int timeSep = ':';

/*
    Formats for mprFormatTime
 */
#if WINDOWS
    #define VALID_FMT "AaBbCcDdEeFHhIjklMmnOPpRrSsTtUuvWwXxYyZz+%"
#elif MACOSX
    #define VALID_FMT "AaBbCcDdEeFGgHhIjklMmnOPpRrSsTtUuVvWwXxYyZz+%"
#else
    #define VALID_FMT "AaBbCcDdEeFGgHhIjklMmnOPpRrSsTtUuVvWwXxYyZz+%"
#endif

#if WINCE
    #define HAS_STRFTIME 0
#else
    #define HAS_STRFTIME 1
#endif

#if !HAS_STRFTIME
static char *abbrevDay[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static char *day[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};

static char *abbrevMonth[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *month[] = {
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
};
#endif /* !HAS_STRFTIME */


static int normalMonthStart[] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 0,
};
static int leapMonthStart[] = {
    0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 0
};

static MprTime daysSinceEpoch(int year);
static void decodeTime(struct tm *tp, MprTime when, bool local);
static int getTimeZoneOffsetFromTm(struct tm *tp);
static int leapYear(int year);
static int localTime(struct tm *timep, MprTime time);
static MprTime makeTime(struct tm *tp);
static void validateTime(struct tm *tm, struct tm *defaults);

/************************************ Code ************************************/
/*
    Initialize the time service
 */
PUBLIC int mprCreateTimeService()
{
    Mpr                 *mpr;
    TimeToken           *tt;

    mpr = MPR;
    mpr->timeTokens = mprCreateHash(59, MPR_HASH_STATIC_KEYS | MPR_HASH_STATIC_VALUES);
    for (tt = days; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = fullDays; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = months; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = fullMonths; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = ampm; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = zones; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    for (tt = offsets; tt->name; tt++) {
        mprAddKey(mpr->timeTokens, tt->name, (void*) tt);
    }
    return 0;
}


PUBLIC int mprCompareTime(MprTime t1, MprTime t2)
{
    if (t1 < t2) {
        return -1;
    } else if (t1 == t2) {
        return 0;
    }
    return 1;
}


PUBLIC void mprDecodeLocalTime(struct tm *tp, MprTime when)
{
    decodeTime(tp, when, 1);
}


PUBLIC void mprDecodeUniversalTime(struct tm *tp, MprTime when)
{
    decodeTime(tp, when, 0);
}


PUBLIC char *mprGetDate(char *format)
{
    struct tm   tm;

    mprDecodeLocalTime(&tm, mprGetTime());
    if (format == 0 || *format == '\0') {
        format = MPR_DEFAULT_DATE;
    }
    return mprFormatTm(format, &tm);
}


PUBLIC char *mprFormatLocalTime(cchar *format, MprTime time)
{
    struct tm   tm;
    if (format == 0) {
        format = MPR_DEFAULT_DATE;
    }
    mprDecodeLocalTime(&tm, time);
    return mprFormatTm(format, &tm);
}


PUBLIC char *mprFormatUniversalTime(cchar *format, MprTime time)
{
    struct tm   tm;
    if (format == 0) {
        format = MPR_DEFAULT_DATE;
    }
    mprDecodeUniversalTime(&tm, time);
    return mprFormatTm(format, &tm);
}


/*
    Returns time in milliseconds since the epoch: 0:0:0 UTC Jan 1 1970.
 */
PUBLIC MprTime mprGetTime()
{
#if VXWORKS
    struct timespec  tv;
    clock_gettime(CLOCK_REALTIME, &tv);
    return (MprTime) (((MprTime) tv.tv_sec) * 1000) + (tv.tv_nsec / (1000 * 1000));
#else
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    return (MprTime) (((MprTime) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
#endif
}


/*
    High resolution timer
 */
#if MPR_HIGH_RES_TIMER
    #if (LINUX || MACOSX) && (BIT_CPU_ARCH == BIT_CPU_X86 || BIT_CPU_ARCH == BIT_CPU_X64)
        uint64 mprGetHiResTicks() {
            uint64  now;
            __asm__ __volatile__ ("rdtsc" : "=A" (now));
            return now;
        }
    #elif WINDOWS
        uint64 mprGetHiResTicks() {
            LARGE_INTEGER  now;
            QueryPerformanceCounter(&now);
            return (((uint64) now.HighPart) << 32) + now.LowPart;
        }
    #else
        uint64 mprGetHiResTicks() {
            return (uint64) mprGetTicks();
        }
    #endif
#else 
    uint64 mprGetHiResTicks() {
        return (uint64) mprGetTicks();
    }
#endif


/*
    Return time in milliseconds that never goes backwards. This is used for timers and not for time of day.
    The actual value returned is system dependant and does not represent time since Jan 1 1970.
 */
PUBLIC MprTicks mprGetTicks()
{
#if BIT_WIN_LIKE && _WIN32_WINNT >= 0x0600
    /* Windows Vista and later */
    return GetTickCount64();
#elif MACOSX
    mach_timebase_info_data_t info;
    mach_timebase_info(&info);
    return mach_absolute_time() * info.numer / info.denom / (1000 * 1000);
#elif CLOCK_MONOTONIC_RAW
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tv);
    return (MprTicks) (((MprTicks) tv.tv_sec) * 1000) + (tv.tv_nsec / (1000 * 1000));
#elif CLOCK_MONOTONIC
    struct timespec tv;
    clock_gettime(CLOCK_MONOTONIC, &tv);
    return (MprTicks) (((MprTicks) tv.tv_sec) * 1000) + (tv.tv_nsec / (1000 * 1000));
#else
    static MprTime lastTicks;
    static MprTime adjustTicks = 0;
    MprTime     result, diff;
    if (lastTicks == 0) {
        lastTicks = mprGetTime();
    }
    result = mprGetTime() + adjustTicks;
    /*
        Handle time reversals. Don't handle jumps forward. Sorry.
     */
    if ((diff = (result - lastTicks)) < 0) {
        adjustTicks += diff;
        result -= diff;
    }
    lastTicks = result;
    return result;
#endif
}


/*
    Return the number of milliseconds until the given timeout has expired.
 */
PUBLIC MprTicks mprGetRemainingTicks(MprTicks mark, MprTicks timeout)
{
    MprTicks    now, diff;

    now = mprGetTicks();
    diff = (now - mark);

    if (diff < 0) {
        /*
            Detect time going backwards. Should never happen now.
         */
        assert(diff >= 0);
        diff = 0;
    }
    return (timeout - diff);
}


PUBLIC MprTicks mprGetElapsedTicks(MprTicks mark)
{
    return mprGetTicks() - mark;
}


PUBLIC MprTime mprGetElapsedTime(MprTime mark)
{
    return mprGetTime() - mark;
}


/*
    Get the timezone offset including DST
    Return the timezone offset (including DST) in msec. local == (UTC + offset)
 */
PUBLIC int mprGetTimeZoneOffset(MprTime when)
{
    MprTime     alternate, secs;
    struct tm   t;
    int         offset;

    alternate = when;
    secs = when / MS_PER_SEC;
    if (secs < MIN_TIME || secs > MAX_TIME) {
        /* secs overflows time_t on this platform. Need to map to an alternate valid year */
        decodeTime(&t, when, 0);
        t.tm_year = 111;
        alternate = makeTime(&t);
    }
    t.tm_isdst = -1;
    if (localTime(&t, alternate) < 0) {
        localTime(&t, time(0) * MS_PER_SEC);
    }
    offset = getTimeZoneOffsetFromTm(&t);
    return offset;
}


/*
    Make a time value interpreting "tm" as a local time
 */
PUBLIC MprTime mprMakeTime(struct tm *tp)
{
    MprTime     when, alternate;
    struct tm   t;
    int         offset, year;

    when = makeTime(tp);
    year = tp->tm_year;
    if (MIN_YEAR <= year && year <= MAX_YEAR) {
        localTime(&t, when);
        offset = getTimeZoneOffsetFromTm(&t);
    } else {
        t = *tp;
        t.tm_year = 111;
        alternate = makeTime(&t);
        localTime(&t, alternate);
        offset = getTimeZoneOffsetFromTm(&t);
    }
    return when - offset;
}


PUBLIC MprTime mprMakeUniversalTime(struct tm *tp)
{
    return makeTime(tp);
}


/*************************************** O/S Layer ***********************************/

static int localTime(struct tm *timep, MprTime time)
{
#if BIT_UNIX_LIKE || WINCE
    time_t when = (time_t) (time / MS_PER_SEC);
    if (localtime_r(&when, timep) == 0) {
        return MPR_ERR;
    }
#else
    struct tm   *tp;
    time_t when = (time_t) (time / MS_PER_SEC);
    if ((tp = localtime(&when)) == 0) {
        return MPR_ERR;
    }
    *timep = *tp;
#endif
    return 0;
}


struct tm *universalTime(struct tm *timep, MprTime time)
{
#if BIT_UNIX_LIKE || WINCE
    time_t when = (time_t) (time / MS_PER_SEC);
    return gmtime_r(&when, timep);
#else
    struct tm   *tp;
    time_t      when;
    when = (time_t) (time / MS_PER_SEC);
    if ((tp = gmtime(&when)) == 0) {
        return 0;
    }
    *timep = *tp;
    return timep;
#endif
}


/*
    Return the timezone offset (including DST) in msec. local == (UTC + offset)
    Assumes a valid (local) "tm" with isdst correctly set.
 */
static int getTimeZoneOffsetFromTm(struct tm *tp)
{
#if BIT_WIN_LIKE
    int                     offset;
    TIME_ZONE_INFORMATION   tinfo;
    GetTimeZoneInformation(&tinfo);
    offset = tinfo.Bias;
    if (tp->tm_isdst) {
        offset += tinfo.DaylightBias;
    } else {
        offset += tinfo.StandardBias;
    }
    return -offset * 60 * MS_PER_SEC;
#elif VXWORKS
    char  *tze, *p;
    int   offset = 0;
    if ((tze = getenv("TIMEZONE")) != 0) {
        if ((p = strchr(tze, ':')) != 0) {
            if ((p = strchr(tze, ':')) != 0) {
                offset = - stoi(++p) * MS_PER_MIN;
            }
        }
        if (tp->tm_isdst) {
            offset += MS_PER_HOUR;
        }
    }
    return offset;
#elif BIT_UNIX_LIKE && !CYGWIN
    return (int) tp->tm_gmtoff * MS_PER_SEC;
#else
    struct timezone     tz;
    struct timeval      tv;
    int                 offset;
    gettimeofday(&tv, &tz);
    offset = -tz.tz_minuteswest * MS_PER_MIN;
    if (tp->tm_isdst) {
        offset += MS_PER_HOUR;
    }
    return offset;
#endif
}

/********************************* Calculations *********************************/
/*
    Convert "struct tm" to MprTime. This ignores GMT offset and DST.
 */
static MprTime makeTime(struct tm *tp)
{
    MprTime     days;
    int         year, month;

    year = tp->tm_year + 1900 + tp->tm_mon / 12; 
    month = tp->tm_mon % 12;
    if (month < 0) {
        month += 12;
        --year;
    }
    days = daysSinceEpoch(year);
    days += leapYear(year) ? leapMonthStart[month] : normalMonthStart[month];
    days += tp->tm_mday - 1;
    return (days * MS_PER_DAY) + ((((((tp->tm_hour * 60)) + tp->tm_min) * 60) + tp->tm_sec) * MS_PER_SEC);
}


static MprTime daysSinceEpoch(int year)
{
    MprTime     days;

    days = ((MprTime) 365) * (year - 1970);
    days += ((year-1) / 4) - (1970 / 4);
    days -= ((year-1) / 100) - (1970 / 100);
    days += ((year-1) / 400) - (1970 / 400);
    return days;
}


static int leapYear(int year)
{
    if (year % 4) {
        return 0;
    } else if (year % 400 == 0) {
        return 1;
    } else if (year % 100 == 0) {
        return 0;
    }
    return 1;
}


static int getMonth(int year, int day)
{
    int     *days, i;

    days = leapYear(year) ? leapMonthStart : normalMonthStart;
    for (i = 1; days[i]; i++) {
        if (day < days[i]) {
            return i - 1;
        }
    }
    return 11;
}


static int getYear(MprTime when)
{
    MprTime     ms;
    int         year;

    year = 1970 + (int) (when / MS_PER_YEAR);
    ms = daysSinceEpoch(year) * MS_PER_DAY;
    if (ms > when) {
        return year - 1;
    } else if (ms + (((MprTime) MS_PER_DAY) * (365 + leapYear(year))) <= when) {
        return year + 1;
    }
    return year;
}


PUBLIC MprTime floorDiv(MprTime x, MprTime divisor)
{
    if (x < 0) {
        return (x - divisor + 1) / divisor;
    } else {
        return x / divisor;
    }
}


/*
    Decode an MprTime into components in a "struct tm" 
 */
static void decodeTime(struct tm *tp, MprTime when, bool local)
{
    MprTime     timeForZoneCalc, secs;
    struct tm   t;
    char        *zoneName;
    int         year, offset, dst;

    zoneName = 0;
    offset = dst = 0;

    if (local) {
        //  OPT -- cache the results somehow
        timeForZoneCalc = when;
        secs = when / MS_PER_SEC;
        if (secs < MIN_TIME || secs > MAX_TIME) {
            /*
                On some systems, localTime won't work for very small (negative) or very large times. 
                Cannot be certain localTime will work for all O/Ss with this year.  Map to an a date with a valid year.
             */
            decodeTime(&t, when, 0);
            t.tm_year = 111;
            timeForZoneCalc = makeTime(&t);
        }
        t.tm_isdst = -1;
        if (localTime(&t, timeForZoneCalc) == 0) {
            offset = getTimeZoneOffsetFromTm(&t);
            dst = t.tm_isdst;
        }
#if BIT_UNIX_LIKE && !CYGWIN
        zoneName = (char*) t.tm_zone;
#endif
        when += offset;
    }
    year = getYear(when);

    tp->tm_year     = year - 1900;
    tp->tm_hour     = (int) (floorDiv(when, MS_PER_HOUR) % 24);
    tp->tm_min      = (int) (floorDiv(when, MS_PER_MIN) % 60);
    tp->tm_sec      = (int) (floorDiv(when, MS_PER_SEC) % 60);
    tp->tm_wday     = (int) ((floorDiv(when, MS_PER_DAY) + 4) % 7);
    tp->tm_yday     = (int) (floorDiv(when, MS_PER_DAY) - daysSinceEpoch(year));
    tp->tm_mon      = getMonth(year, tp->tm_yday);
    tp->tm_isdst    = dst != 0;
#if BIT_UNIX_LIKE && !CYGWIN
    tp->tm_gmtoff   = offset / MS_PER_SEC;
    tp->tm_zone     = zoneName;
#endif
    if (tp->tm_hour < 0) {
        tp->tm_hour += 24;
    }
    if (tp->tm_min < 0) {
        tp->tm_min += 60;
    }
    if (tp->tm_sec < 0) {
        tp->tm_sec += 60;
    }
    if (tp->tm_wday < 0) {
        tp->tm_wday += 7;
    }
    if (tp->tm_yday < 0) {
        tp->tm_yday += 365;
    }
    if (leapYear(year)) {
        tp->tm_mday = tp->tm_yday - leapMonthStart[tp->tm_mon] + 1;
    } else {
        tp->tm_mday = tp->tm_yday - normalMonthStart[tp->tm_mon] + 1;
    }
    assert(tp->tm_hour >= 0);
    assert(tp->tm_min >= 0);
    assert(tp->tm_sec >= 0);
    assert(tp->tm_wday >= 0);
    assert(tp->tm_mon >= 0);
    /* This asserts with some calculating some intermediate dates <= year 100 */
    assert(tp->tm_yday >= 0);
    assert(tp->tm_yday < 365 || (tp->tm_yday < 366 && leapYear(year)));
    assert(tp->tm_mday >= 1);
}


/********************************* Formatting **********************************/
/*
    Format a time string. This uses strftime if available and so the supported formats vary from platform to platform.
    Strftime should supports some of these these formats:

     %A      full weekday name (Monday)
     %a      abbreviated weekday name (Mon)
     %B      full month name (January)
     %b      abbreviated month name (Jan)
     %C      century. Year / 100. (0-N)
     %c      standard date and time representation
     %D      date (%m/%d/%y)
     %d      day-of-month (01-31)
     %e      day-of-month with a leading space if only one digit ( 1-31)
     %F      same as %Y-%m-%d
     %H      hour (24 hour clock) (00-23)
     %h      same as %b
     %I      hour (12 hour clock) (01-12)
     %j      day-of-year (001-366)
     %k      hour (24 hour clock) (0-23)
     %l      the hour (12-hour clock) as a decimal number (1-12); single digits are preceded by a blank.
     %M      minute (00-59)
     %m      month (01-12)
     %n      a newline
     %P      lower case am / pm
     %p      AM / PM
     %R      same as %H:%M
     %r      same as %H:%M:%S %p
     %S      second (00-59)
     %s      seconds since epoch
     %T      time (%H:%M:%S)
     %t      a tab.
     %U      week-of-year, first day sunday (00-53)
     %u      the weekday (Monday as the first day of the week) as a decimal number (1-7).
     %v      is equivalent to ``%e-%b-%Y''.
     %W      week-of-year, first day monday (00-53)
     %w      weekday (0-6, sunday is 0)
     %X      standard time representation
     %x      standard date representation
     %Y      year with century
     %y      year without century (00-99)
     %Z      timezone name
     %z      offset from UTC (-hhmm or +hhmm)
     %+      national representation of the date and time (the format is similar to that produced by date(1)).
     %%      percent sign

     Some platforms may also support the following format extensions:
     %E*     POSIX locale extensions. Where "*" is one of the characters: c, C, x, X, y, Y.
             representations. 
     %G      a year as a decimal number with century. This year is the one that contains the greater part of
             the week (Monday as the first day of the week).
     %g      the same year as in ``%G'', but as a decimal number without century (00-99).
     %O*     POSIX locale extensions. Where "*" is one of the characters: d, e, H, I, m, M, S, u, U, V, w, W, y.
             Additionly %OB implemented to represent alternative months names (used standalone, without day mentioned). 
     %V      the week number of the year (Monday as the first day of the week) as a decimal number (01-53). If the week 
             containing January 1 has four or more days in the new year, then it is week 1; otherwise it is the last 
             week of the previous year, and the next week is week 1.

    Useful formats:
        RFC822: "%a, %d %b %Y %H:%M:%S %Z           "Fri, 07 Jan 2003 12:12:21 PDT"
                "%T %F                              "12:12:21 2007-01-03"
                "%v                                 "07-Jul-2003"
 */

#if HAS_STRFTIME
/*
    Preferred implementation as strftime() will be localized
 */
PUBLIC char *mprFormatTm(cchar *format, struct tm *tp)
{
    struct tm       tm;
    cchar           *cp;
    char            localFmt[256], buf[256], *dp, *endp, *sign;
    ssize           size;
    int             value;

    dp = localFmt;
    if (format == 0) {
        format = MPR_DEFAULT_DATE;
    }
    if (tp == 0) {
        mprDecodeLocalTime(&tm, mprGetTime());
        tp = &tm;
    }
    endp = &localFmt[sizeof(localFmt) - 1];
    size = sizeof(localFmt) - 1;
    for (cp = format; *cp && dp < &localFmt[sizeof(localFmt) - 32]; size = (int) (endp - dp - 1)) {
        if (*cp == '%') {
            *dp++ = *cp++;
        again:
            switch (*cp) {
            case '+':
                if (tp->tm_mday < 10) {
                    /* Some platforms don't support 'e' so avoid it here. Put a double space before %d */
                    fmt(dp, size, "%s  %d %s", "a %b", tp->tm_mday, "%H:%M:%S %Z %Y");
                } else {
                    strcpy(dp, "a %b %d %H:%M:%S %Z %Y");
                }
                dp += slen(dp);
                cp++;
                break;

            case 'C':
                dp--;
                itosbuf(dp, size, (1900 + tp->tm_year) / 100, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'D':
                strcpy(dp, "m/%d/%y");
                dp += 7;
                cp++;
                break;

            case 'e':                       /* day of month (1-31). Single digits preceeded by blanks */
                dp--;
                if (tp->tm_mday < 10) {
                    *dp++ = ' ';
                }
                itosbuf(dp, size - 1, (int64) tp->tm_mday, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'E':
                /* Skip the 'E' */
                cp++;
                goto again;
            
            case 'F':
                strcpy(dp, "Y-%m-%d");
                dp += 7;
                cp++;
                break;

            case 'h':
                *dp++ = 'b';
                cp++;
                break;

            case 'k':
                dp--;
                if (tp->tm_hour < 10) {
                    *dp++ = ' ';
                }
                itosbuf(dp, size - 1, (int64) tp->tm_hour, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'l':
                dp--;
                value = tp->tm_hour;
                if (value < 10) {
                    *dp++ = ' ';
                }
                if (value > 12) {
                    value -= 12;
                }
                itosbuf(dp, size - 1, (int64) value, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'n':
                dp[-1] = '\n';
                cp++;
                break;

            case 'O':
                /* Skip the 'O' */
                cp++;
                goto again;
            
            case 'P':
                dp--;
                strcpy(dp, (tp->tm_hour > 11) ? "pm" : "am");
                dp += 2;
                cp++;
                break;

            case 'R':
                strcpy(dp, "H:%M");
                dp += 4;
                cp++;
                break;

            case 'r':
                strcpy(dp, "I:%M:%S %p");
                dp += 10;
                cp++;
                break;

            case 's':
                dp--;
                itosbuf(dp, size, (int64) mprMakeTime(tp) / MS_PER_SEC, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'T':
                strcpy(dp, "H:%M:%S");
                dp += 7;
                cp++;
                break;

            case 't':
                dp[-1] = '\t';
                cp++;
                break;

            case 'u':
                dp--;
                value = tp->tm_wday;
                if (value == 0) {
                    value = 7;
                }
                itosbuf(dp, size, (int64) value, 10);
                dp += slen(dp);
                cp++;
                break;

            case 'v':
                /* Inline '%e' */
                dp--;
                if (tp->tm_mday < 10) {
                    *dp++ = ' ';
                }
                itosbuf(dp, size - 1, (int64) tp->tm_mday, 10);
                dp += slen(dp);
                cp++;
                strcpy(dp, "-%b-%Y");
                dp += 6;
                break;

            case 'z':
                dp--;
                value = mprGetTimeZoneOffset(makeTime(tp)) / (MS_PER_SEC * 60);
                sign = (value < 0) ? "-" : "";
                if (value < 0) {
                    value = -value;
                }
                fmt(dp, size, "%s%02d%02d", sign, value / 60, value % 60);
                dp += slen(dp);
                cp++;
                break;

            default: 
                if (strchr(VALID_FMT, (int) *cp) != 0) {
                    *dp++ = *cp++;
                } else {
                    dp--;
                    cp++;
                }
                break;
            }
        } else {
            *dp++ = *cp++;
        }
    }
    *dp = '\0';
    format = localFmt;
    if (*format == '\0') {
        format = "%a %b %d %H:%M:%S %Z %Y";
    }
    if (strftime(buf, sizeof(buf) - 1, format, tp) > 0) {
        buf[sizeof(buf) - 1] = '\0';
        return sclone(buf);
    }
    return 0;
}


#else /* !HAS_STRFTIME */
/*
    This implementation is used only on platforms that don't support strftime. This version is not localized.
 */
static void digits(MprBuf *buf, int count, int fill, int value)
{
    char    tmp[32]; 
    int     i, j; 

    if (value < 0) {
        mprPutCharToBuf(buf, '-');
        value = -value;
    }
    for (i = 0; value && i < count; i++) { 
        tmp[i] = '0' + value % 10; 
        value /= 10; 
    } 
    if (fill) {
        for (j = i; j < count; j++) {
            mprPutCharToBuf(buf, fill);
        }
    }
    while (i-- > 0) {
        mprPutCharToBuf(buf, tmp[i]); 
    } 
}


static char *getTimeZoneName(struct tm *tp)
{
#if BIT_WIN_LIKE
    TIME_ZONE_INFORMATION   tz;
    WCHAR                   *wzone;
    GetTimeZoneInformation(&tz);
    wzone = tp->tm_isdst ? tz.DaylightName : tz.StandardName;
    return mprToMulti(wzone);
#else
    tzset();
    return sclone(tp->tm_zone);
#endif
}


PUBLIC char *mprFormatTm(cchar *format, struct tm *tp)
{
    struct tm       tm;
    MprBuf          *buf;
    char            *zone;
    int             w, value;

    if (format == 0) {
        format = MPR_DEFAULT_DATE;
    }
    if (tp == 0) {
        mprDecodeLocalTime(&tm, mprGetTime());
        tp = &tm;
    }
    if ((buf = mprCreateBuf(64, -1)) == 0) {
        return 0;
    }
    while ((*format != '\0')) {
        if (*format++ != '%') {
            mprPutCharToBuf(buf, format[-1]);
            continue;
        }
    again:
        switch (*format++) {
        case '%' :                                      /* percent */
            mprPutCharToBuf(buf, '%');
            break;

        case '+' :                                      /* date (Mon May 18 23:29:50 PDT 2013) */
            mprPutStringToBuf(buf, abbrevDay[tp->tm_wday]);
            mprPutCharToBuf(buf, ' ');
            mprPutStringToBuf(buf, abbrevMonth[tp->tm_mon]);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 2, ' ', tp->tm_mday);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 2, '0', tp->tm_hour);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_sec);
            mprPutCharToBuf(buf, ' ');
            zone = getTimeZoneName(tp);
            mprPutStringToBuf(buf, zone);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 4, 0, tp->tm_year + 1900);
            break;

        case 'A' :                                      /* full weekday (Sunday) */
            mprPutStringToBuf(buf, day[tp->tm_wday]);
            break;

        case 'a' :                                      /* abbreviated weekday (Sun) */
            mprPutStringToBuf(buf, abbrevDay[tp->tm_wday]);
            break;

        case 'B' :                                      /* full month (January) */
            mprPutStringToBuf(buf, month[tp->tm_mon]);
            break;

        case 'b' :                                      /* abbreviated month (Jan) */
            mprPutStringToBuf(buf, abbrevMonth[tp->tm_mon]);
            break;

        case 'C' :                                      /* century number (19, 20) */
            digits(buf, 2, '0', (1900 + tp->tm_year) / 100);
            break;

        case 'c' :                                      /* preferred date+time in current locale */
            mprPutStringToBuf(buf, abbrevDay[tp->tm_wday]);
            mprPutCharToBuf(buf, ' ');
            mprPutStringToBuf(buf, abbrevMonth[tp->tm_mon]);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 2, ' ', tp->tm_mday);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 2, '0', tp->tm_hour);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_sec);
            mprPutCharToBuf(buf, ' ');
            digits(buf, 4, 0, tp->tm_year + 1900);
            break;

        case 'D' :                                      /* mm/dd/yy */
            digits(buf, 2, '0', tp->tm_mon + 1);
            mprPutCharToBuf(buf, '/');
            digits(buf, 2, '0', tp->tm_mday);
            mprPutCharToBuf(buf, '/');
            digits(buf, 2, '0', tp->tm_year - 100);
            break;

        case 'd' :                                      /* day of month (01-31) */
            digits(buf, 2, '0', tp->tm_mday);
            break;

        case 'E':
            /* Skip the 'E' */
            goto again;

        case 'e':                                       /* day of month (1-31). Single digits preceeded by a blank */
            digits(buf, 2, ' ', tp->tm_mday);
            break;

        case 'F':                                       /* %m/%d/%y */
            digits(buf, 4, 0, tp->tm_year + 1900);
            mprPutCharToBuf(buf, '-');
            digits(buf, 2, '0', tp->tm_mon + 1);
            mprPutCharToBuf(buf, '-');
            digits(buf, 2, '0', tp->tm_mday);
            break;

        case 'H':                                       /* hour using 24 hour clock (00-23) */
            digits(buf, 2, '0', tp->tm_hour);
            break;

        case 'h':                                       /* Same as %b */
            mprPutStringToBuf(buf, abbrevMonth[tp->tm_mon]);
            break;

        case 'I':                                       /* hour using 12 hour clock (00-01) */
            digits(buf, 2, '0', (tp->tm_hour % 12) ? tp->tm_hour % 12 : 12);
            break;

        case 'j':                                       /* julian day (001-366) */
            digits(buf, 3, '0', tp->tm_yday+1);
            break;

        case 'k':                                       /* hour (0-23). Single digits preceeded by a blank */
            digits(buf, 2, ' ', tp->tm_hour);
            break;

        case 'l':                                       /* hour (1-12). Single digits preceeded by a blank */
            digits(buf, 2, ' ', tp->tm_hour < 12 ? tp->tm_hour : (tp->tm_hour - 12));
            break;

        case 'M':                                       /* minute as a number (00-59) */
            digits(buf, 2, '0', tp->tm_min);
            break;

        case 'm':                                       /* month as a number (01-12) */
            digits(buf, 2, '0', tp->tm_mon + 1);
            break;

        case 'n':                                       /* newline */
            mprPutCharToBuf(buf, '\n');
            break;

        case 'O':
            /* Skip the 'O' */
            goto again;

        case 'p':                                       /* AM/PM */
            mprPutStringToBuf(buf, (tp->tm_hour > 11) ? "PM" : "AM");
            break;

        case 'P':                                       /* am/pm */
            mprPutStringToBuf(buf, (tp->tm_hour > 11) ? "pm" : "am");
            break;

        case 'R':
            digits(buf, 2, '0', tp->tm_hour);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            break;

        case 'r':
            digits(buf, 2, '0', (tp->tm_hour % 12) ? tp->tm_hour % 12 : 12);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_sec);
            mprPutCharToBuf(buf, ' ');
            mprPutStringToBuf(buf, (tp->tm_hour > 11) ? "PM" : "AM");
            break;

        case 'S':                                       /* seconds as a number (00-60) */
            digits(buf, 2, '0', tp->tm_sec);
            break;

        case 's':                                       /* seconds since epoch */
            mprPutToBuf(buf, "%d", mprMakeTime(tp) / MS_PER_SEC);
            break;

        case 'T':
            digits(buf, 2, '0', tp->tm_hour);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_sec);
            break;

        case 't':                                       /* Tab */
            mprPutCharToBuf(buf, '\t');
            break;

        case 'U':                                       /* week number (00-53. Staring with first Sunday */
            w = tp->tm_yday / 7;
            if (tp->tm_yday % 7 > tp->tm_wday) {
                w++;
            }
            digits(buf, 2, '0', w);
            break;

        case 'u':                                       /* Week day (1-7) */
            value = tp->tm_wday;
            if (value == 0) {
                value = 7;
            }
            digits(buf, 1, 0, tp->tm_wday == 0 ? 7 : tp->tm_wday);
            break;

        case 'v':                                       /* %e-%b-%Y */
            digits(buf, 2, ' ', tp->tm_mday);
            mprPutCharToBuf(buf, '-');
            mprPutStringToBuf(buf, abbrevMonth[tp->tm_mon]);
            mprPutCharToBuf(buf, '-');
            digits(buf, 4, '0', tp->tm_year + 1900);
            break;

        case 'W':                                       /* week number (00-53). Staring with first Monday */
            w = (tp->tm_yday + 7 - (tp->tm_wday ?  (tp->tm_wday - 1) : (7 - 1))) / 7;
            digits(buf, 2, '0', w);
            break;

        case 'w':                                       /* day of week (0-6) */
            digits(buf, 1, '0', tp->tm_wday);
            break;

        case 'X':                                       /* preferred time without date */
            digits(buf, 2, '0', tp->tm_hour);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_min);
            mprPutCharToBuf(buf, ':');
            digits(buf, 2, '0', tp->tm_sec);
            break;

        case 'x':                                      /* preferred date without time */
            digits(buf, 2, '0', tp->tm_mon + 1);
            mprPutCharToBuf(buf, '/');
            digits(buf, 2, '0', tp->tm_mday);
            mprPutCharToBuf(buf, '/');
            digits(buf, 2, '0', tp->tm_year + 1900);
            break;

        case 'Y':                                       /* year as a decimal including century (1900) */
            digits(buf, 4, '0', tp->tm_year + 1900);
            break;

        case 'y':                                       /* year without century (00-99) */
            digits(buf, 2, '0', tp->tm_year % 100);
            break;

        case 'Z':                                       /* Timezone */
            zone = getTimeZoneName(tp);
            mprPutStringToBuf(buf, zone);
            break;

        case 'z':
            value = mprGetTimeZoneOffset(makeTime(tp)) / (MS_PER_SEC * 60);
            if (value < 0) {
                mprPutCharToBuf(buf, '-');
                value = -value;
            }
            digits(buf, 2, '0', value / 60);
            digits(buf, 2, '0', value % 60);
            break;

        case 'g':
        case 'G':
        case 'V':
            break;

        default:
            mprPutCharToBuf(buf, '%');
            mprPutCharToBuf(buf, format[-1]);
            break;
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}
#endif /* HAS_STRFTIME */


/*************************************** Parsing ************************************/

static int lookupSym(cchar *token, int kind)
{
    TimeToken   *tt;

    if ((tt = (TimeToken*) mprLookupKey(MPR->timeTokens, token)) == 0) {
        return -1;
    }
    if (kind != (tt->value & TOKEN_MASK)) {
        return -1;
    }
    return tt->value & ~TOKEN_MASK;
}


static int getNum(char **token, int sep)
{
    int     num;

    if (*token == 0) {
        return 0;
    }

    num = atoi(*token);
    *token = strchr(*token, sep);
    if (*token) {
        *token += 1;
    }
    return num;
}


static int getNumOrSym(char **token, int sep, int kind, int *isAlpah)
{
    char    *cp;
    int     num;

    assert(token && *token);

    if (*token == 0) {
        return 0;
    }
    if (isalpha((uchar) **token)) {
        *isAlpah = 1;
        cp = strchr(*token, sep);
        if (cp) {
            *cp++ = '\0';
        }
        num = lookupSym(*token, kind);
        *token = cp;
        return num;
    }
    num = atoi(*token);
    *token = strchr(*token, sep);
    if (*token) {
        *token += 1;
    }
    *isAlpah = 0;
    return num;
}


static void swapDayMonth(struct tm *tp)
{
    int     tmp;

    tmp = tp->tm_mday;
    tp->tm_mday = tp->tm_mon;
    tp->tm_mon = tmp;
}


/*
    Parse the a date/time string according to the given zoneFlags and return the result in *time. Missing date items 
    may be provided via the defaults argument.
 */ 
PUBLIC int mprParseTime(MprTime *time, cchar *dateString, int zoneFlags, struct tm *defaults)
{
    TimeToken       *tt;
    struct tm       tm;
    char            *str, *next, *token, *cp, *sep;
    int64           value;
    int             kind, hour, min, negate, value1, value2, value3, alpha, alpha2, alpha3;
    int             dateSep, offset, zoneOffset, explicitZone, fullYear;

    offset = 0;
    zoneOffset = 0;
    explicitZone = 0;
    sep = ", \t";
    cp = 0;
    next = 0;
    fullYear = 0;

    /*
        Set these mandatory values to -1 so we can tell if they are set to valid values
        WARNING: all the calculations use tm_year with origin 0, not 1900. It is fixed up below.
     */
    tm.tm_year = -MAXINT;
    tm.tm_mon = tm.tm_mday = tm.tm_hour = tm.tm_sec = tm.tm_min = tm.tm_wday = -1;
    tm.tm_min = tm.tm_sec = tm.tm_yday = -1;
#if BIT_UNIX_LIKE && !CYGWIN
    tm.tm_gmtoff = 0;
    tm.tm_zone = 0;
#endif

    /*
        Set to -1 to try to determine if DST is in effect
     */
    tm.tm_isdst = -1;
    str = slower(dateString);

    /*
        Handle ISO dates: "2009-05-21t16:06:05.000z
     */
    if (strchr(str, ' ') == 0 && strchr(str, '-') && str[slen(str) - 1] == 'z') {
        for (cp = str; *cp; cp++) {
            if (*cp == '-') {
                *cp = '/';
            } else if (*cp == 't' && cp > str && isdigit((uchar) cp[-1]) && isdigit((uchar) cp[1]) ) {
                *cp = ' ';
            }
        }
    }
    token = stok(str, sep, &next);

    while (token && *token) {
        if (snumber(token)) {
            /*
                Parse either day of month or year. Priority to day of month. Format: <29> Jan <15> <2013>
             */ 
            value = stoi(token);
            if (value > 3000) {
                *time = value;
                return 0;
            } else if (value > 32 || (tm.tm_mday >= 0 && tm.tm_year == -MAXINT)) {
                if (value >= 1000) {
                    fullYear = 1;
                }
                tm.tm_year = (int) value - 1900;
            } else if (tm.tm_mday < 0) {
                tm.tm_mday = (int) value;
            }

        } else if ((*token == '+') || (*token == '-') ||
                ((strncmp(token, "gmt", 3) == 0 || strncmp(token, "utc", 3) == 0) &&
                ((cp = strchr(&token[3], '+')) != 0 || (cp = strchr(&token[3], '-')) != 0))) {
            /*
                Timezone. Format: [GMT|UTC][+-]NN[:]NN
             */
            if (!isalpha((uchar) *token)) {
                cp = token;
            }
            negate = *cp == '-' ? -1 : 1;
            cp++;
            hour = getNum(&cp, timeSep);
            if (hour >= 100) {
                hour /= 100;
            }
            min = getNum(&cp, timeSep);
            zoneOffset = negate * (hour * 60 + min);
            explicitZone = 1;

        } else if (isalpha((uchar) *token)) {
            if ((tt = (TimeToken*) mprLookupKey(MPR->timeTokens, token)) != 0) {
                kind = tt->value & TOKEN_MASK;
                value = tt->value & ~TOKEN_MASK; 
                switch (kind) {

                case TOKEN_DAY:
                    tm.tm_wday = (int) value;
                    break;

                case TOKEN_MONTH:
                    tm.tm_mon = (int) value;
                    break;

                case TOKEN_OFFSET:
                    /* Named timezones or symbolic names like: tomorrow, yesterday, next week ... */ 
                    /* Units are seconds */
                    offset += (int) value;
                    break;

                case TOKEN_ZONE:
                    zoneOffset = (int) value;
                    explicitZone = 1;
                    break;

                default:
                    /* Just ignore unknown values */
                    break;
                }
            }

        } else if ((cp = strchr(token, timeSep)) != 0 && isdigit((uchar) token[0])) {
            /*
                Time:  10:52[:23]
                Must not parse GMT-07:30
             */
            tm.tm_hour = getNum(&token, timeSep);
            tm.tm_min = getNum(&token, timeSep);
            tm.tm_sec = getNum(&token, timeSep);

        } else {
            dateSep = '/';
            if (strchr(token, dateSep) == 0) {
                dateSep = '-';
                if (strchr(token, dateSep) == 0) {
                    dateSep = '.';
                    if (strchr(token, dateSep) == 0) {
                        dateSep = 0;
                    }
                }
            }
            if (dateSep) {
                /*
                    Date:  07/28/2013, 07/28/08, Jan/28/2013, Jaunuary-28-2013, 28-jan-2013
                    Support order: dd/mm/yy, mm/dd/yy and yyyy/mm/dd
                    Support separators "/", ".", "-"
                 */
                value1 = getNumOrSym(&token, dateSep, TOKEN_MONTH, &alpha);
                value2 = getNumOrSym(&token, dateSep, TOKEN_MONTH, &alpha2);
                value3 = getNumOrSym(&token, dateSep, TOKEN_MONTH, &alpha3);

                if (value1 > 31) {
                    /* yy/mm/dd */
                    tm.tm_year = value1;
                    tm.tm_mon = value2;
                    tm.tm_mday = value3;

                } else if (value1 > 12 || alpha2) {
                    /* 
                        dd/mm/yy 
                        Cannot detect 01/02/03  This will be evaluated as Jan 2 2003 below.
                     */  
                    tm.tm_mday = value1;
                    tm.tm_mon = value2;
                    tm.tm_year = value3;

                } else {
                    /*
                        The default to parse is mm/dd/yy unless the mm value is out of range
                     */
                    tm.tm_mon = value1;
                    tm.tm_mday = value2;
                    tm.tm_year = value3;
                }
            }
        }
        token = stok(NULL, sep, &next);
    }

    /*
        Y2K fix and rebias
     */
    if (0 <= tm.tm_year && tm.tm_year < 100 && !fullYear) {
        if (tm.tm_year < 50) {
            tm.tm_year += 2000;
        } else {
            tm.tm_year += 1900;
        }
    }    
    if (tm.tm_year >= 1900) {
        tm.tm_year -= 1900;
    }

    /*
        Convert back to origin 0 for months
     */
    if (tm.tm_mon > 0) {
        tm.tm_mon--;
    }

    /*
        Validate and fill in missing items with defaults
     */
    validateTime(&tm, defaults);

    if (zoneFlags == MPR_LOCAL_TIMEZONE && !explicitZone) {
        *time = mprMakeTime(&tm);
    } else {
        *time = mprMakeUniversalTime(&tm);
        *time += -(zoneOffset * MS_PER_MIN);
    }
    *time += (offset * MS_PER_SEC);
    return 0;
}


static void validateTime(struct tm *tp, struct tm *defaults)
{
    struct tm   empty;

    /*
        Fix apparent day-mon-year ordering issues. Cannot fix everything!
     */
    if ((12 <= tp->tm_mon && tp->tm_mon <= 31) && 0 <= tp->tm_mday && tp->tm_mday <= 11) {
        /*
            Looks like day month are swapped
         */
        swapDayMonth(tp);
    }

    if (tp->tm_year != -MAXINT && tp->tm_mon >= 0 && tp->tm_mday >= 0 && tp->tm_hour >= 0) {
        /*  Everything defined */
        return;
    }

    /*
        Use empty time if missing
     */
    if (defaults == NULL) {
        memset(&empty, 0, sizeof(empty));
        defaults = &empty;
        empty.tm_mday = 1;
        empty.tm_year = 70;
    }
    if (tp->tm_hour < 0 && tp->tm_min < 0 && tp->tm_sec < 0) {
        tp->tm_hour = defaults->tm_hour;
        tp->tm_min = defaults->tm_min;
        tp->tm_sec = defaults->tm_sec;
    }

    /*
        Get weekday, if before today then make next week
     */
    if (tp->tm_wday >= 0 && tp->tm_year == -MAXINT && tp->tm_mon < 0 && tp->tm_mday < 0) {
        tp->tm_mday = defaults->tm_mday + (tp->tm_wday - defaults->tm_wday + 7) % 7;
        tp->tm_mon = defaults->tm_mon;
        tp->tm_year = defaults->tm_year;
    }

    /*
        Get month, if before this month then make next year
     */
    if (tp->tm_mon >= 0 && tp->tm_mon <= 11 && tp->tm_mday < 0) {
        if (tp->tm_year == -MAXINT) {
            tp->tm_year = defaults->tm_year + (((tp->tm_mon - defaults->tm_mon) < 0) ? 1 : 0);
        }
        tp->tm_mday = defaults->tm_mday;
    }

    /*
        Get date, if before current time then make tomorrow
     */
    if (tp->tm_hour >= 0 && tp->tm_year == -MAXINT && tp->tm_mon < 0 && tp->tm_mday < 0) {
        tp->tm_mday = defaults->tm_mday + ((tp->tm_hour - defaults->tm_hour) < 0 ? 1 : 0);
        tp->tm_mon = defaults->tm_mon;
        tp->tm_year = defaults->tm_year;
    }
    if (tp->tm_year == -MAXINT) {
        tp->tm_year = defaults->tm_year;
    }
    if (tp->tm_mon < 0) {
        tp->tm_mon = defaults->tm_mon;
    }
    if (tp->tm_mday < 0) {
        tp->tm_mday = defaults->tm_mday;
    }
    if (tp->tm_yday < 0) {
        tp->tm_yday = (leapYear(tp->tm_year + 1900) ? 
            leapMonthStart[tp->tm_mon] : normalMonthStart[tp->tm_mon]) + tp->tm_mday - 1;
    }
    if (tp->tm_hour < 0) {
        tp->tm_hour = defaults->tm_hour;
    }
    if (tp->tm_min < 0) {
        tp->tm_min = defaults->tm_min;
    }
    if (tp->tm_sec < 0) {
        tp->tm_sec = defaults->tm_sec;
    }
}


/********************************* Compatibility **********************************/
/*
    Compatibility for windows and VxWorks
 */
#if BIT_WIN_LIKE || VXWORKS
PUBLIC int gettimeofday(struct timeval *tv, struct timezone *tz)
{
    #if BIT_WIN_LIKE
        FILETIME        fileTime;
        MprTime         now;
        static int      tzOnce;

        if (NULL != tv) {
            /* Convert from 100-nanosec units to microsectonds */
            GetSystemTimeAsFileTime(&fileTime);
            now = ((((MprTime) fileTime.dwHighDateTime) << BITS(uint)) + ((MprTime) fileTime.dwLowDateTime));
            now /= 10;

            now -= TIME_GENESIS;
            tv->tv_sec = (long) (now / 1000000);
            tv->tv_usec = (long) (now % 1000000);
        }
        if (NULL != tz) {
            TIME_ZONE_INFORMATION   zone;
            int                     rc, bias;
            rc = GetTimeZoneInformation(&zone);
            bias = (int) zone.Bias;
            if (rc == TIME_ZONE_ID_DAYLIGHT) {
                tz->tz_dsttime = 1;
            } else {
                tz->tz_dsttime = 0;
            }
            tz->tz_minuteswest = bias;
        }
        return 0;

    #elif VXWORKS
        struct tm       tm;
        struct timespec now;
        time_t          t;
        char            *tze, *p;
        int             rc;

        if ((rc = clock_gettime(CLOCK_REALTIME, &now)) == 0) {
            tv->tv_sec  = now.tv_sec;
            tv->tv_usec = (now.tv_nsec + 500) / MS_PER_SEC;
            if ((tze = getenv("TIMEZONE")) != 0) {
                if ((p = strchr(tze, ':')) != 0) {
                    if ((p = strchr(tze, ':')) != 0) {
                        tz->tz_minuteswest = stoi(++p);
                    }
                }
                t = tickGet();
                tz->tz_dsttime = (localtime_r(&t, &tm) == 0) ? tm.tm_isdst : 0;
            }
        }
        return rc;
    #endif
}
#endif /* BIT_WIN_LIKE || VXWORKS */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/vxworks.c"
 */
/************************************************************************/

/**
    vxworks.c - Vxworks specific adaptions

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if VXWORKS
/*********************************** Code *************************************/

PUBLIC int mprCreateOsService()
{
    return 0;
}


PUBLIC int mprStartOsService()
{
    return 0;
}


PUBLIC void mprStopOsService()
{
}


#if _WRS_VXWORKS_MAJOR < 6 || (_WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 9)

PUBLIC int access(const char *path, int mode)
{
    struct stat sbuf;
    return stat((char*) path, &sbuf);
}
#endif


PUBLIC int mprGetRandomBytes(char *buf, int length, bool block)
{
    int     i;

    for (i = 0; i < length; i++) {
        buf[i] = (char) (mprGetTime() >> i);
    }
    return 0;
}


PUBLIC int mprLoadNativeModule(MprModule *mp)
{
    MprModuleEntry  fn;
    SYM_TYPE        symType;
    MprPath         info;
    char            *at;
    void            *handle;
    int             fd;

    assert(mp);
    fn = 0;
    handle = 0;

    if (!mp->entry || symFindByName(sysSymTbl, mp->entry, (char**) (void*) &fn, &symType) == -1) {
        if ((at = mprSearchForModule(mp->path)) == 0) {
            mprError("Cannot find module \"%s\", cwd: \"%s\", search path \"%s\"", mp->path, mprGetCurrentPath(),
                mprGetModuleSearchPath());
            return MPR_ERR_CANT_ACCESS;
        }
        mp->path = at;
        mprGetPathInfo(mp->path, &info);
        mp->modified = info.mtime;

        mprLog(2, "Loading native module %s", mp->path);
        if ((fd = open(mp->path, O_RDONLY, 0664)) < 0) {
            mprError("Cannot open module \"%s\"", mp->path);
            return MPR_ERR_CANT_OPEN;
        }
        handle = loadModule(fd, LOAD_GLOBAL_SYMBOLS);
        if (handle == 0) {
            close(fd);
            if (handle) {
                unldByModuleId(handle, 0);
            }
            mprError("Cannot load module %s", mp->path);
            return MPR_ERR_CANT_READ;
        }
        close(fd);
        mp->handle = handle;

    } else if (mp->entry) {
        mprLog(2, "Activating module %s", mp->name);
    }
    if (mp->entry) {
        if (symFindByName(sysSymTbl, mp->entry, (char**) (void*) &fn, &symType) == -1) {
            mprError("Cannot find symbol %s when loading %s", mp->entry, mp->path);
            return MPR_ERR_CANT_READ;
        }
        if ((fn)(mp->moduleData, mp) < 0) {
            mprError("Initialization for %s failed.", mp->path);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    return 0;
}


PUBLIC int mprUnloadNativeModule(MprModule *mp)
{
    unldByModuleId((MODULE_ID) mp->handle, 0);
    return 0;
}


PUBLIC void mprNap(MprTicks milliseconds)
{
    struct timespec timeout;
    int             rc;

    assert(milliseconds >= 0);
    timeout.tv_sec = milliseconds / 1000;
    timeout.tv_nsec = (milliseconds % 1000) * 1000000;
    do {
        rc = nanosleep(&timeout, &timeout);
    } while (rc < 0 && errno == EINTR);
}


PUBLIC void mprSleep(MprTicks timeout)
{
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    mprNap(timeout);
    mprResetYield();
}


PUBLIC void mprWriteToOsLog(cchar *message, int flags, int level)
{
}


PUBLIC uint mprGetpid(void) {
    return taskIdSelf();
}


#if _WRS_VXWORKS_MAJOR < 6 || (_WRS_VXWORKS_MAJOR == 6 && _WRS_VXWORKS_MINOR < 9)

PUBLIC int fsync(int fd) { 
    return 0; 
}
#endif


PUBLIC int ftruncate(int fd, off_t offset) { 
    return 0; 
}


PUBLIC int usleep(uint msec)
{
    struct timespec     timeout;
    int                 rc;

    timeout.tv_sec = msec / (1000 * 1000);
    timeout.tv_nsec = msec % (1000 * 1000) * 1000;
    do {
        rc = nanosleep(&timeout, &timeout);
    } while (rc < 0 && errno == EINTR);
    return 0;
}


PUBLIC int mprInitWindow()
{
    return 0;
}


/*
    Create a routine to pull in the GCC support routines for double and int64 manipulations for some platforms. Do this
    incase modules reference these routines. Without this, the modules have to reference them. Which leads to multiple 
    defines if two modules include them. (Code to pull in moddi3, udivdi3, umoddi3)
 */
double  __mpr_floating_point_resolution(double a, double b, int64 c, int64 d, uint64 e, uint64 f) {
    a = a / b; a = a * b; c = c / d; c = c % d; e = e / f; e = e % f;
    c = (int64) a; d = (uint64) a; a = (double) c; a = (double) e;
    return (a == b) ? a : b;
}


#endif /* VXWORKS */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/wait.c"
 */
/************************************************************************/

/*
    wait.c - Wait for I/O service.

    This module provides wait management for sockets and other file descriptors and allows users to create wait
    handlers which will be called when I/O events are detected. Multiple backends (one at a time) are supported.

    This module is thread-safe.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/***************************** Forward Declarations ***************************/

static void ioEvent(void *data, MprEvent *event);
static void manageWaitService(MprWaitService *ws, int flags);
static void manageWaitHandler(MprWaitHandler *wp, int flags);

/************************************ Code ************************************/
/*
    Initialize the service
 */
PUBLIC MprWaitService *mprCreateWaitService()
{
    MprWaitService  *ws;

    ws = mprAllocObj(MprWaitService, manageWaitService);
    if (ws == 0) {
        return 0;
    }
    MPR->waitService = ws;
    ws->handlers = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    ws->mutex = mprCreateLock();
    ws->spin = mprCreateSpinLock();
    mprCreateNotifierService(ws);
    return ws;
}


static void manageWaitService(MprWaitService *ws, int flags)
{
    lock(ws);
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->handlers);
        mprMark(ws->handlerMap);
        mprMark(ws->mutex);
        mprMark(ws->spin);
    }
#if MPR_EVENT_ASYNC
    /* Nothing to manage */
#endif
#if MPR_EVENT_KQUEUE
    mprManageKqueue(ws, flags);
#endif
#if MPR_EVENT_EPOLL
    mprManageEpoll(ws, flags);
#endif
#if MPR_EVENT_POLL
    mprManagePoll(ws, flags);
#endif
#if MPR_EVENT_SELECT
    mprManageSelect(ws, flags);
#endif
    unlock(ws);
}


static MprWaitHandler *initWaitHandler(MprWaitHandler *wp, int fd, int mask, MprDispatcher *dispatcher, void *proc, 
    void *data, int flags)
{
    MprWaitService  *ws;

    assert(fd >= 0);

    ws = MPR->waitService;
    wp->fd              = fd;
    wp->notifierIndex   = -1;
    wp->dispatcher      = dispatcher;
    wp->proc            = proc;
    wp->flags           = 0;
    wp->handlerData     = data;
    wp->service         = ws;
    wp->flags           = flags;

    if (mprGetListLength(ws->handlers) == FD_SETSIZE) {
        mprError("io: Too many io handlers: %d\n", FD_SETSIZE);
        return 0;
    }
#if BIT_UNIX_LIKE || VXWORKS
    if (fd >= FD_SETSIZE) {
        mprError("File descriptor %d exceeds max io of %d", fd, FD_SETSIZE);
    }
#endif
    if (mask) {
        lock(ws);
        if (mprAddItem(ws->handlers, wp) < 0) {
            unlock(ws);
            return 0;
        }
        mprNotifyOn(ws, wp, mask);
        unlock(ws);
        mprWakeNotifier();
    }
    return wp;
}


PUBLIC MprWaitHandler *mprCreateWaitHandler(int fd, int mask, MprDispatcher *dispatcher, void *proc, void *data, int flags)
{
    MprWaitHandler  *wp;

    assert(fd >= 0);

    if ((wp = mprAllocObj(MprWaitHandler, manageWaitHandler)) == 0) {
        return 0;
    }
    return initWaitHandler(wp, fd, mask, dispatcher, proc, data, flags);
}


static void manageWaitHandler(MprWaitHandler *wp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(wp->handlerData);
        mprMark(wp->dispatcher);
        mprMark(wp->service);
        mprMark(wp->next);
        mprMark(wp->prev);
        mprMark(wp->requiredWorker);
        mprMark(wp->thread);
        mprMark(wp->callbackComplete);
        mprMark(wp->event);

    } else if (flags & MPR_MANAGE_FREE) {
        mprRemoveWaitHandler(wp);
    }
}


PUBLIC void mprRemoveWaitHandler(MprWaitHandler *wp)
{
    MprWaitService      *ws;

    if (wp == 0) {
        return;
    }
    ws = wp->service;
    if (ws == 0) {
        /* This wait handler was never initialized. */
        return;
    }
    lock(ws);
    if (wp->fd >= 0) {
        if (wp->desiredMask) {
            mprNotifyOn(ws, wp, 0);
        }
        mprRemoveItem(ws->handlers, wp);
        wp->fd = -1;
        if (wp->event) {
            mprRemoveEvent(wp->event);
            wp->event = 0;
        }
    }
    mprWakeNotifier();
    unlock(ws);
}


PUBLIC void mprQueueIOEvent(MprWaitHandler *wp)
{
    MprDispatcher   *dispatcher;
    MprEvent        *event;

    lock(wp->service);
    if (wp->flags & MPR_WAIT_NEW_DISPATCHER) {
        dispatcher = mprCreateDispatcher("IO", MPR_DISPATCHER_ENABLED | MPR_DISPATCHER_AUTO_CREATE);
    } else {
        dispatcher = (wp->dispatcher) ? wp->dispatcher: mprGetDispatcher();
    }
    event = wp->event = mprCreateEvent(dispatcher, "IOEvent", 0, ioEvent, wp->handlerData, MPR_EVENT_DONT_QUEUE);
    event->mask = wp->presentMask;
    event->handler = wp;
    mprQueueEvent(dispatcher, event);
    unlock(wp->service);
}


static void ioEvent(void *data, MprEvent *event)
{
    event->handler->event = 0;
    event->handler->proc(data, event);
}


PUBLIC void mprWaitOn(MprWaitHandler *wp, int mask)
{
    lock(wp->service);
    if (mask != wp->desiredMask) {
        if (wp->flags & MPR_WAIT_RECALL_HANDLER) {
            wp->service->needRecall = 1;
        }
        mprNotifyOn(wp->service, wp, mask);
        mprWakeNotifier();
    }
    unlock(wp->service);
}


/*
    Set a handler to be recalled without further I/O
 */
PUBLIC void mprRecallWaitHandlerByFd(int fd)
{
    MprWaitService  *ws;
    MprWaitHandler  *wp;
    int             index;

    ws = MPR->waitService;
    lock(ws);
    for (index = 0; (wp = (MprWaitHandler*) mprGetNextItem(ws->handlers, &index)) != 0; ) {
        if (wp->fd == fd) {
            wp->flags |= MPR_WAIT_RECALL_HANDLER;
            ws->needRecall = 1;
            mprWakeNotifier();
            break;
        }
    }
    unlock(ws);
}


PUBLIC void mprRecallWaitHandler(MprWaitHandler *wp)
{
    MprWaitService  *ws;

    if (wp) {
        ws = MPR->waitService;
        lock(ws);
        wp->flags |= MPR_WAIT_RECALL_HANDLER;
        ws->needRecall = 1;
        mprWakeNotifier();
        unlock(ws);
    }
}


/*
    Recall a handler which may have buffered data. Only called by notifiers.
 */
PUBLIC void mprDoWaitRecall(MprWaitService *ws)
{
    MprWaitHandler      *wp;
    int                 index;

    lock(ws);
    ws->needRecall = 0;
    for (index = 0; (wp = (MprWaitHandler*) mprGetNextItem(ws->handlers, &index)) != 0; ) {
        if ((wp->flags & MPR_WAIT_RECALL_HANDLER) && (wp->desiredMask & MPR_READABLE)) {
            wp->presentMask |= MPR_READABLE;
            wp->flags &= ~MPR_WAIT_RECALL_HANDLER;
            mprNotifyOn(ws, wp, 0);
            mprQueueIOEvent(wp);
        }
    }
    unlock(ws);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/wide.c"
 */
/************************************************************************/

/**
    unicode.c - Unicode support

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if BIT_CHAR_LEN > 1
#if UNUSED
/************************************ Code ************************************/
/*
    Format a number as a string. Support radix 10 and 16.
    Count is the length of buf in characters.
 */
PUBLIC wchar *itow(wchar *buf, ssize count, int64 value, int radix)
{
    wchar   numBuf[32];
    wchar   *cp, *dp, *endp;
    char    digits[] = "0123456789ABCDEF";
    int     negative;

    if (radix != 10 && radix != 16) {
        return 0;
    }
    cp = &numBuf[sizeof(numBuf)];
    *--cp = '\0';

    if (value < 0) {
        negative = 1;
        value = -value;
        count--;
    } else {
        negative = 0;
    }
    do {
        *--cp = digits[value % radix];
        value /= radix;
    } while (value > 0);

    if (negative) {
        *--cp = '-';
    }
    dp = buf;
    endp = &buf[count];
    while (dp < endp && *cp) {
        *dp++ = *cp++;
    }
    *dp++ = '\0';
    return buf;
}


PUBLIC wchar *wchr(wchar *str, int c)
{
    wchar   *s;

    if (str == NULL) {
        return 0;
    }
    for (s = str; *s; ) {
        if (*s == c) {
            return s;
        }
    }
    return 0;
}


PUBLIC int wcasecmp(wchar *s1, wchar *s2)
{
    if (s1 == 0 || s2 == 0) {
        return -1;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    return wncasecmp(s1, s2, max(slen(s1), slen(s2)));
}


PUBLIC wchar *wclone(wchar *str)
{
    wchar   *result, nullBuf[1];
    ssize   len, size;

    if (str == NULL) {
        nullBuf[0] = 0;
        str = nullBuf;
    }
    len = wlen(str);
    size = (len + 1) * sizeof(wchar);
    if ((result = mprAlloc(size)) != NULL) {
        memcpy(result, str, len * sizeof(wchar));
    }
    result[len] = '\0';
    return result;
}


PUBLIC int wcmp(wchar *s1, wchar *s2)
{
    if (s1 == s2) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    return wncmp(s1, s2, max(slen(s1), slen(s2)));
}


/*
    Count is the maximum number of characters to compare
 */
PUBLIC wchar *wncontains(wchar *str, wchar *pattern, ssize count)
{
    wchar   *cp, *s1, *s2;
    ssize   lim;

    assert(0 <= count && count < MAXINT);

    if (count < 0) {
        count = MAXINT;
    }
    if (str == 0) {
        return 0;
    }
    if (pattern == 0 || *pattern == '\0') {
        return str;
    }
    for (cp = str; *cp && count > 0; cp++, count--) {
        s1 = cp;
        s2 = pattern;
        for (lim = count; *s1 && *s2 && (*s1 == *s2) && lim > 0; lim--) {
            s1++;
            s2++;
        }
        if (*s2 == '\0') {
            return cp;
        }
    }
    return 0;
}


PUBLIC wchar *wcontains(wchar *str, wchar *pattern)
{
    return wncontains(str, pattern, -1);
}


/*
    count is the size of dest in characters
 */
PUBLIC ssize wcopy(wchar *dest, ssize count, wchar *src)
{
    ssize      len;

    assert(src);
    assert(dest);
    assert(0 < count && count < MAXINT);

    len = wlen(src);
    if (count <= len) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    memcpy(dest, src, (len + 1) * sizeof(wchar));
    return len;
}


PUBLIC int wends(wchar *str, wchar *suffix)
{
    if (str == NULL || suffix == NULL) {
        return 0;
    }
    if (wncmp(&str[wlen(str) - wlen(suffix)], suffix, -1) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC wchar *wfmt(wchar *fmt, ...)
{
    va_list     ap;
    char        *mfmt, *mresult;

    assert(fmt);

    va_start(ap, fmt);
    mfmt = awtom(fmt, NULL);
    mresult = sfmtv(mfmt, ap);
    va_end(ap);
    return amtow(mresult, NULL);
}


PUBLIC wchar *wfmtv(wchar *fmt, va_list arg)
{
    char        *mfmt, *mresult;

    assert(fmt);
    mfmt = awtom(fmt, NULL);
    mresult = sfmtv(mfmt, arg);
    return amtow(mresult, NULL);
}


/*
    Compute a hash for a Unicode string 
    (Based on work by Paul Hsieh, see http://www.azillionmonkeys.com/qed/hash.html)
    Count is the length of name in characters
 */
PUBLIC uint whash(wchar *name, ssize count)
{
    uint    tmp, rem, hash;

    assert(name);
    assert(0 <= count && count < MAXINT);

    if (count < 0) {
        count = wlen(name);
    }
    hash = count;
    rem = count & 3;

    for (count >>= 2; count > 0; count--, name += 4) {
        hash  += name[0] | (name[1] << 8);
        tmp   =  ((name[2] | (name[3] << 8)) << 11) ^ hash;
        hash  =  (hash << 16) ^ tmp;
        hash  += hash >> 11;
    }
    switch (rem) {
    case 3: 
        hash += name[0] + (name[1] << 8);
        hash ^= hash << 16;
        hash ^= name[2] << 18;
        hash += hash >> 11;
        break;
    case 2: 
        hash += name[0] + (name[1] << 8);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: 
        hash += name[0];
        hash ^= hash << 10;
        hash += hash >> 1;
    }
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash;
}


/*
    Count is the length of name in characters
 */
PUBLIC uint whashlower(wchar *name, ssize count)
{
    uint    tmp, rem, hash;

    assert(name);
    assert(0 <= count && count < MAXINT);

    if (count < 0) {
        count = wlen(name);
    }
    hash = count;
    rem = count & 3;

    for (count >>= 2; count > 0; count--, name += 4) {
        hash  += tolower((uchar) name[0]) | (tolower((uchar) name[1]) << 8);
        tmp   =  ((tolower((uchar) name[2]) | (tolower((uchar) name[3]) << 8)) << 11) ^ hash;
        hash  =  (hash << 16) ^ tmp;
        hash  += hash >> 11;
    }
    switch (rem) {
    case 3: 
        hash += tolower((uchar) name[0]) + (tolower((uchar) name[1]) << 8);
        hash ^= hash << 16;
        hash ^= tolower((uchar) name[2]) << 18;
        hash += hash >> 11;
        break;
    case 2: 
        hash += tolower((uchar) name[0]) + (tolower((uchar) name[1]) << 8);
        hash ^= hash << 11;
        hash += hash >> 17;
        break;
    case 1: 
        hash += tolower((uchar) name[0]);
        hash ^= hash << 10;
        hash += hash >> 1;
    }
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;
    return hash;
}


PUBLIC wchar *wjoin(wchar *str, ...)
{
    wchar       *result;
    va_list     ap;

    va_start(ap, str);
    result = wrejoinv(NULL, str, ap);
    va_end(ap);
    return result;
}


PUBLIC wchar *wjoinv(wchar *buf, va_list args)
{
    va_list     ap;
    wchar       *dest, *str, *dp, nullBuf[1];
    int         required, len, blen;

    va_copy(ap, args);
    required = 1;
    blen = wlen(buf);
    if (buf) {
        required += blen;
    }
    str = va_arg(ap, wchar*);
    while (str) {
        required += wlen(str);
        str = va_arg(ap, wchar*);
    }
    if ((dest = mprAlloc(required * sizeof(wchar))) == 0) {
        return 0;
    }
    dp = dest;
    if (buf) {
        wcopy(dp, required, buf);
        dp += blen;
        required -= blen;
    }
    va_copy(ap, args);
    str = va_arg(ap, wchar*);
    while (str) {
        wcopy(dp, required, str);
        len = wlen(str);
        dp += len;
        required -= len;
        str = va_arg(ap, wchar*);
    }
    *dp = '\0';
    return dest;
}


/*
    Return the length of "s" in characters
 */
PUBLIC ssize wlen(wchar *s)
{
    ssize  i;

    i = 0;
    if (s) {
        while (*s) s++;
    }
    return i;
}


/*  
    Map a string to lower case 
 */
PUBLIC wchar *wlower(wchar *str)
{
    wchar   *cp, *s;

    assert(str);

    if (str) {
        s = wclone(str);
        for (cp = s; *cp; cp++) {
            if (isupper((uchar) *cp)) {
                *cp = (wchar) tolower((uchar) *cp);
            }
        }
        str = s;
    }
    return str;
}


/*
    Count is the maximum number of characters to compare
 */
PUBLIC int wncasecmp(wchar *s1, wchar *s2, ssize count)
{
    int     rc;

    assert(0 <= count && count < MAXINT);

    if (s1 == 0 || s2 == 0) {
        return -1;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; count > 0 && *s1 && rc == 0; s1++, s2++, count--) {
        rc = tolower((uchar) *s1) - tolower((uchar) *s2);
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}


/*
    Count is the maximum number of characters to compare
 */
PUBLIC int wncmp(wchar *s1, wchar *s2, ssize count)
{
    int     rc;

    assert(0 <= count && count < MAXINT);

    if (s1 == 0 && s2 == 0) {
        return 0;
    } else if (s1 == 0) {
        return -1;
    } else if (s2 == 0) {
        return 1;
    }
    for (rc = 0; count > 0 && *s1 && rc == 0; s1++, s2++, count--) {
        rc = *s1 - *s2;
    }
    if (rc) {
        return (rc > 0) ? 1 : -1;
    } else if (n == 0) {
        return 0;
    } else if (*s1 == '\0' && *s2 == '\0') {
        return 0;
    } else if (*s1 == '\0') {
        return -1;
    } else if (*s2 == '\0') {
        return 1;
    }
    return 0;
}


/*
    This routine copies at most "count" characters from a string. It ensures the result is always null terminated and 
    the buffer does not overflow. DestCount is the maximum size of dest in characters.
    Returns MPR_ERR_WONT_FIT if the buffer is too small.
 */
PUBLIC ssize wncopy(wchar *dest, ssize destCount, wchar *src, ssize count)
{
    ssize      len;

    assert(dest);
    assert(src);
    assert(dest != src);
    assert(0 <= count && count < MAXINT);
    assert(0 < destCount && destCount < MAXINT);

    len = wlen(src);
    len = min(len, count);
    if (destCount <= len) {
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    if (len > 0) {
        memcpy(dest, src, len * sizeof(wchar));
        dest[len] = '\0';
    } else {
        *dest = '\0';
        len = 0;
    } 
    return len;
}


PUBLIC wchar *wpbrk(wchar *str, wchar *set)
{
    wchar   *sp;
    int     count;

    if (str == NULL || set == NULL) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                return str;
            }
        }
    }
    return 0;
}


PUBLIC wchar *wrchr(wchar *str, int c)
{
    wchar   *s;

    if (str == NULL) {
        return 0;
    }
    for (s = &str[wlen(str)]; *s; ) {
        if (*s == c) {
            return s;
        }
    }
    return 0;
}


PUBLIC wchar *wrejoin(wchar *buf, ...)
{
    wchar       *result;
    va_list     ap;

    va_start(ap, buf);
    result = wrejoinv(buf, buf, ap);
    va_end(ap);
    return result;
}


PUBLIC wchar *wrejoinv(wchar *buf, va_list args)
{
    va_list     ap;
    wchar       *dest, *str, *dp, nullBuf[1];
    int         required, len, n;

    va_copy(ap, args);
    len = wlen(buf);
    required = len + 1;
    str = va_arg(ap, wchar*);
    while (str) {
        required += wlen(str);
        str = va_arg(ap, wchar*);
    }
    if ((dest = mprRealloc(buf, required * sizeof(wchar))) == 0) {
        return 0;
    }
    dp = &dest[len];
    required -= len;
    va_copy(ap, args);
    str = va_arg(ap, wchar*);
    while (str) {
        wcopy(dp, required, str);
        n = wlen(str);
        dp += n;
        required -= n;
        str = va_arg(ap, wchar*);
    }
    assert(required >= 0);
    *dp = '\0';
    return dest;
}


PUBLIC ssize wspn(wchar *str, wchar *set)
{
    wchar   *sp;
    int     count;

    if (str == NULL || set == NULL) {
        return 0;
    }
    for (count = 0; *str; count++, str++) {
        for (sp = set; *sp; sp++) {
            if (*str == *sp) {
                return break;
            }
        }
        if (*str != *sp) {
            return break;
        }
    }
    return count;
}
 

PUBLIC int wstarts(wchar *str, wchar *prefix)
{
    if (str == NULL || prefix == NULL) {
        return 0;
    }
    if (wncmp(str, prefix, wlen(prefix)) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC int64 wtoi(wchar *str)
{
    return wtoiradix(str, 10, NULL);
}


PUBLIC int64 wtoiradix(wchar *str, int radix, int *err)
{
    char    *bp, buf[32];

    for (bp = buf; bp < &buf[sizeof(buf)]; ) {
        *bp++ = *str++;
    }
    buf[sizeof(buf) - 1] = 0;
    return stoiradix(buf, radix, err);
}


PUBLIC wchar *wtok(wchar *str, wchar *delim, wchar **last)
{
    wchar   *start, *end;
    ssize   i;

    start = str ? str : *last;

    if (start == 0) {
        *last = 0;
        return 0;
    }
    i = wspn(start, delim);
    start += i;
    if (*start == '\0') {
        *last = 0;
        return 0;
    }
    end = wpbrk(start, delim);
    if (end) {
        *end++ = '\0';
        i = wspn(end, delim);
        end += i;
    }
    *last = end;
    return start;
}


/*
    Count is the length in characters to extract
 */
PUBLIC wchar *wsub(wchar *str, ssize offset, ssize count)
{
    wchar   *result;
    ssize   size;

    assert(str);
    assert(offset >= 0);
    assert(0 <= count && count < MAXINT);

    if (str == 0) {
        return 0;
    }
    size = (count + 1) * sizeof(wchar);
    if ((result = mprAlloc(size)) == NULL) {
        return NULL;
    }
    wncopy(result, count + 1, &str[offset], count);
    return result;
}


PUBLIC wchar *wtrim(wchar *str, wchar *set, int where)
{
    wchar   s;
    ssize   len, i;

    if (str == NULL || set == NULL) {
        return str;
    }
    s = wclone(str);
    if (where & MPR_TRIM_START) {
        i = wspn(s, set);
    } else {
        i = 0;
    }
    s += i;
    if (where & MPR_TRIM_END) {
        len = wlen(s);
        while (len > 0 && wspn(&s[len - 1], set) > 0) {
            s[len - 1] = '\0';
            len--;
        }
    }
    return s;
}


/*  
    Map a string to upper case
 */
PUBLIC char *wupper(wchar *str)
{
    wchar   *cp, *s;

    assert(str);
    if (str) {
        s = wclone(str);
        for (cp = s; *cp; cp++) {
            if (islower((uchar) *cp)) {
                *cp = (wchar) toupper((uchar) *cp);
            }
        }
        str = s;
    }
    return str;
}
#endif /* UNUSED */

/*********************************** Conversions *******************************/
/*
    Convert a wide unicode string into a multibyte string buffer. If count is supplied, it is used as the source length 
    in characters. Otherwise set to -1. DestCount is the max size of the dest buffer in bytes. At most destCount - 1 
    characters will be stored. The dest buffer will always have a trailing null appended.  If dest is NULL, don't copy 
    the string, just return the length of characters. Return a count of bytes copied to the destination or -1 if an 
    invalid unicode sequence was provided in src.
    NOTE: does not allocate.
 */
PUBLIC ssize wtom(char *dest, ssize destCount, wchar *src, ssize count)
{
    ssize   len;

    if (destCount < 0) {
        destCount = MAXSSIZE;
    }
    if (count > 0) {
#if BIT_CHAR_LEN == 1
        if (dest) {
            len = scopy(dest, destCount, src);
        } else {
            len = min(slen(src), destCount - 1);
        }
#elif BIT_WIN_LIKE
        len = WideCharToMultiByte(CP_ACP, 0, src, count, dest, (DWORD) destCount - 1, NULL, NULL);
#else
        len = wcstombs(dest, src, destCount - 1);
#endif
        if (dest) {
            if (len >= 0) {
                dest[len] = 0;
            }
        } else if (len >= destCount) {
            assert(!MPR_ERR_WONT_FIT);
            return MPR_ERR_WONT_FIT;
        }
    }
    return len;
}


/*
    Convert a multibyte string to a unicode string. If count is supplied, it is used as the source length in bytes.
    Otherwise set to -1. DestCount is the max size of the dest buffer in characters. At most destCount - 1 
    characters will be stored. The dest buffer will always have a trailing null characters appended.  If dest is NULL, 
    don't copy the string, just return the length of characters. Return a count of characters copied to the destination 
    or -1 if an invalid multibyte sequence was provided in src.
    NOTE: does not allocate.
 */
PUBLIC ssize mtow(wchar *dest, ssize destCount, cchar *src, ssize count) 
{
    ssize      len;

    if (destCount < 0) {
        destCount = MAXSSIZE;
    }
    if (destCount > 0) {
#if BIT_CHAR_LEN == 1
        if (dest) {
            len = scopy(dest, destCount, src);
        } else {
            len = min(slen(src), destCount - 1);
        }
#elif BIT_WIN_LIKE
        len = MultiByteToWideChar(CP_ACP, 0, src, count, dest, (DWORD) destCount - 1);
#else
        len = mbstowcs(dest, src, destCount - 1);
#endif
        if (dest) {
            if (len >= 0) {
                dest[len] = 0;
            }
        } else if (len >= destCount) {
            assert(!MPR_ERR_WONT_FIT);
            return MPR_ERR_WONT_FIT;
        }
    }
    return len;
}


PUBLIC wchar *amtow(cchar *src, ssize *lenp)
{
    wchar   *dest;
    ssize   len;

    len = mtow(NULL, MAXSSIZE, src, -1);
    if (len < 0) {
        return NULL;
    }
    if ((dest = mprAlloc((len + 1) * sizeof(wchar))) != NULL) {
        mtow(dest, len + 1, src, -1);
    }
    if (lenp) {
        *lenp = len;
    }
    return dest;
}


//  FUTURE UNICODE - need a version that can supply a length

PUBLIC char *awtom(wchar *src, ssize *lenp)
{
    char    *dest;
    ssize   len;

    len = wtom(NULL, MAXSSIZE, src, -1);
    if (len < 0) {
        return NULL;
    }
    if ((dest = mprAlloc(len + 1)) != 0) {
        wtom(dest, len + 1, src, -1);
    }
    if (lenp) {
        *lenp = len;
    }
    return dest;
}


#if FUTURE

#define BOM_MSB_FIRST       0xFEFF
#define BOM_LSB_FIRST       0xFFFE

/*
    Surrogate area  (0xD800 <= x && x <= 0xDFFF) => mapped into 0x10000 ... 0x10FFFF
 */

static int utf8Length(int c)
{
    if (c & 0x80) {
        return 1;
    }
    if ((c & 0xc0) != 0xc0) {
        return 0;
    }
    if ((c & 0xe0) != 0xe0) {
        return 2;
    }
    if ((c & 0xf0) != 0xf0) {
        return 3;
    }
    if ((c & 0xf8) != 0xf8) {
        return 4;
    }
    return 0;
}


static int isValidUtf8(cuchar *src, int len)
{
    if (len == 4 && (src[4] < 0x80 || src[3] > 0xBF)) {
        return 0;
    }
    if (len >= 3 && (src[3] < 0x80 || src[2] > 0xBF)) {
        return 0;
    }
    if (len >= 2 && src[1] > 0xBF) {
        return 0;
    }
    if (src[0]) {
        if (src[0] == 0xE0) {
            if (src[1] < 0xA0) {
                return 0;
            }
        } else if (src[0] == 0xED) {
            if (src[1] < 0xA0) {
                return 0;
            }
        } else if (src[0] == 0xF0) {
            if (src[1] < 0xA0) {
                return 0;
            }
        } else if (src[0] == 0xF4) {
            if (src[1] < 0xA0) {
                return 0;
            }
        } else if (src[1] < 0x80) {
            return 0;
        }
    }
    if (len >= 1) {
        if (src[0] >= 0x80 && src[0] < 0xC2) {
            return 0;
        }
    }
    if (src[0] >= 0xF4) {
        return 0;
    }
    return 1;
}


static int offsets[6] = { 0x00000000UL, 0x00003080UL, 0x000E2080UL, 0x03C82080UL, 0xFA082080UL, 0x82082080UL };

PUBLIC ssize xmtow(wchar *dest, ssize destMax, cchar *src, ssize len) 
{
    wchar   *dp, *dend;
    cchar   *sp, *send;
    int     i, c, count;

    assert(0 <= len && len < MAXINT);

    if (len < 0) {
        len = slen(src);
    }
    if (dest) {
        dend = &dest[destMax];
    }
    count = 0;
    for (sp = src, send = &src[len]; sp < send; ) {
        len = utf8Length(*sp) - 1;
        if (&sp[len] >= send) {
            return MPR_ERR_BAD_FORMAT;
        }
        if (!isValidUtf8((uchar*) sp, len + 1)) {
            return MPR_ERR_BAD_FORMAT;
        }
        for (c = 0, i = len; i >= 0; i--) {
            c = *sp++;
            c <<= 6;
        }
        c -= offsets[len];
        count++;
        if (dp >= dend) {
            assert(!MPR_ERR_WONT_FIT);
            return MPR_ERR_WONT_FIT;
        }
        if (c <= 0xFFFF) {
            if (dest) {
                if (c >= 0xD800 && c <= 0xDFFF) {
                    *dp++ = 0xFFFD;
                } else {
                    *dp++ = c;
                }
            }
        } else if (c > 0x10FFFF) {
            *dp++ = 0xFFFD;
        } else {
            c -= 0x0010000UL;
            *dp++ = (c >> 10) + 0xD800;
            if (dp >= dend) {
                assert(!MPR_ERR_WONT_FIT);
                return MPR_ERR_WONT_FIT;
            }
            *dp++ = (c & 0x3FF) + 0xDC00;
            count++;
        }
    }
    return count;
}

static cuchar marks[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

/*
   if (c < 0x80) 
      b1 = c >> 0  & 0x7F | 0x00
      b2 = null
      b3 = null
      b4 = null
   else if (c < 0x0800)
      b1 = c >> 6  & 0x1F | 0xC0
      b2 = c >> 0  & 0x3F | 0x80
      b3 = null
      b4 = null
   else if (c < 0x010000)
      b1 = c >> 12 & 0x0F | 0xE0
      b2 = c >> 6  & 0x3F | 0x80
      b3 = c >> 0  & 0x3F | 0x80
      b4 = null
   else if (c < 0x110000)
      b1 = c >> 18 & 0x07 | 0xF0
      b2 = c >> 12 & 0x3F | 0x80
      b3 = c >> 6  & 0x3F | 0x80
      b4 = c >> 0  & 0x3F | 0x80
   end if
*/

PUBLIC ssize xwtom(char *dest, ssize destMax, wchar *src, ssize len)
{
    wchar   *sp, *send;
    char    *dp, *dend;
    int     i, c, c2, count, bytes, mark, mask;

    assert(0 <= len && len < MAXINT);

    if (len < 0) {
        len = wlen(src);
    }
    if (dest) {
        dend = &dest[destMax];
    }
    count = 0;
    mark = 0x80;
    mask = 0xBF;
    for (sp = src, send = &src[len]; sp < send; ) {
        c = *sp++;
        if (c >= 0xD800 && c <= 0xD8FF) {
            if (sp < send) {
                c2 = *sp++;
                if (c2 >= 0xDC00 && c2 <= 0xDFFF) {
                    c = ((c - 0xD800) << 10) + (c2 - 0xDC00) + 0x10000;
                }
            } else {
                assert(!MPR_ERR_WONT_FIT);
                return MPR_ERR_WONT_FIT;
            }
        }
        if (c < 0x80) {
            bytes = 1;
        } else if (c < 0x10000) {
            bytes = 2;
        } else if (c < 0x110000) {
            bytes = 4;
        } else {
            bytes = 3;
            c = 0xFFFD;
        }
        if (dest) {
            dp += bytes;
            if (dp >= dend) {
                assert(!MPR_ERR_WONT_FIT);
                return MPR_ERR_WONT_FIT;
            }
            for (i = 1; i < bytes; i++) {
                *--dp = (c | mark) & mask;
                c >>= 6;
            }
            *--dp = (c | marks[bytes]);
            dp += bytes;
        }
        count += bytes;
    }
    return count;
}


#endif /* FUTURE */

#else /* BIT_CHAR_LEN == 1 */

PUBLIC wchar *amtow(cchar *src, ssize *len)
{
    if (len) {
        *len = slen(src);
    }
    return (wchar*) sclone(src);
}


PUBLIC char *awtom(wchar *src, ssize *len)
{
    if (len) {
        *len = slen((char*) src);
    }
    return sclone((char*) src);
}


#endif /* BIT_CHAR_LEN > 1 */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/win.c"
 */
/************************************************************************/

/**
    win.c - Windows specific adaptions

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if CYGWIN
 #include "w32api/windows.h"
#endif

#if BIT_WIN_LIKE && !WINCE
/*********************************** Code *************************************/
/*
    Initialize the O/S platform layer
 */ 

PUBLIC int mprCreateOsService()
{
    WSADATA     wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }
    return 0;
}


PUBLIC int mprStartOsService()
{
    return 0;
}


PUBLIC void mprStopOsService()
{
    WSACleanup();
}


PUBLIC long mprGetInst()
{
    return (long) MPR->appInstance;
}


PUBLIC HWND mprGetHwnd()
{
    return MPR->waitService->hwnd;
}


PUBLIC int mprGetRandomBytes(char *buf, ssize length, bool block)
{
    HCRYPTPROV      prov;
    int             rc;

    rc = 0;
    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | 0x40)) {
        return mprGetError();
    }
    if (!CryptGenRandom(prov, (wsize) length, buf)) {
        rc = mprGetError();
    }
    CryptReleaseContext(prov, 0);
    return rc;
}


PUBLIC int mprLoadNativeModule(MprModule *mp)
{
    MprModuleEntry  fn;
    MprPath         info;
    char            *at, *baseName;
    void            *handle;

    assert(mp);

    if ((handle = (HANDLE) MPR->appInstance) == 0) {
        handle = GetModuleHandle(NULL);
    }
    if (!handle || !mp->entry || !GetProcAddress(handle, mp->entry)) {
        if ((at = mprSearchForModule(mp->path)) == 0) {
            mprError("Cannot find module \"%s\", cwd: \"%s\", search path \"%s\"", mp->path, mprGetCurrentPath(),
                mprGetModuleSearchPath());
            return MPR_ERR_CANT_ACCESS;
        }
        mp->path = at;
        mprGetPathInfo(mp->path, &info);
        mp->modified = info.mtime;
        mprLog(2, "Loading native module %s", mp->path);
        baseName = mprGetPathBase(mp->path);
        if ((handle = GetModuleHandle(wide(baseName))) == 0 && (handle = LoadLibrary(wide(mp->path))) == 0) {
            mprError("Cannot load module %s\nReason: \"%d\"\n", mp->path, mprGetOsError());
            return MPR_ERR_CANT_READ;
        } 
        mp->handle = handle;

    } else if (mp->entry) {
        mprLog(2, "Activating native module %s", mp->name);
    }
    if (mp->entry) {
        if ((fn = (MprModuleEntry) GetProcAddress((HINSTANCE) handle, mp->entry)) == 0) {
            mprError("Cannot load module %s\nReason: can't find function \"%s\"\n", mp->name, mp->entry);
            FreeLibrary((HINSTANCE) handle);
            return MPR_ERR_CANT_ACCESS;
        }
        if ((fn)(mp->moduleData, mp) < 0) {
            mprError("Initialization for module %s failed", mp->name);
            FreeLibrary((HINSTANCE) handle);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    return 0;
}


PUBLIC int mprUnloadNativeModule(MprModule *mp)
{
    assert(mp->handle);

    if (FreeLibrary((HINSTANCE) mp->handle) == 0) {
        return MPR_ERR_ABORTED;
    }
    return 0;
}


PUBLIC void mprSetInst(HINSTANCE inst)
{
    MPR->appInstance = inst;
}


PUBLIC void mprSetHwnd(HWND h)
{
    MPR->waitService->hwnd = h;
}


PUBLIC void mprSetSocketMessage(int socketMessage)
{
    MPR->waitService->socketMessage = socketMessage;
}


PUBLIC void mprNap(MprTicks timeout)
{
    Sleep((int) timeout);
}


PUBLIC void mprSleep(MprTicks timeout)
{
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    mprNap(timeout);
    mprResetYield();
}


PUBLIC void mprWriteToOsLog(cchar *message, int flags, int level)
{
    HKEY        hkey;
    void        *event;
    long        errorType;
    ulong       exists;
    char        buf[BIT_MAX_PATH], logName[BIT_MAX_PATH], *cp, *value;
	wchar		*lines[9];
    int         type;
    static int  once = 0;

    scopy(buf, sizeof(buf), message);
    cp = &buf[slen(buf) - 1];
    while (*cp == '\n' && cp > buf) {
        *cp-- = '\0';
    }
    type = EVENTLOG_ERROR_TYPE;
    lines[0] = wide(buf);
    lines[1] = 0;
    lines[2] = lines[3] = lines[4] = lines[5] = 0;
    lines[6] = lines[7] = lines[8] = 0;

    if (once == 0) {
        /*  Initialize the registry */
        once = 1;
        fmt(logName, sizeof(logName), "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s", mprGetAppName());
        hkey = 0;

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, wide(logName), 0, NULL, 0, KEY_ALL_ACCESS, NULL, 
                &hkey, &exists) == ERROR_SUCCESS) {
            value = "%SystemRoot%\\System32\\netmsg.dll";
            if (RegSetValueEx(hkey, UT("EventMessageFile"), 0, REG_EXPAND_SZ, 
                    (uchar*) value, (int) slen(value) + 1) != ERROR_SUCCESS) {
                RegCloseKey(hkey);
                return;
            }
            errorType = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            if (RegSetValueEx(hkey, UT("TypesSupported"), 0, REG_DWORD, (uchar*) &errorType, 
                    sizeof(DWORD)) != ERROR_SUCCESS) {
                RegCloseKey(hkey);
                return;
            }
            RegCloseKey(hkey);
        }
    }

    event = RegisterEventSource(0, wide(mprGetAppName()));
    if (event) {
        /*
            3299 is the event number for the generic message in netmsg.dll.
            "%1 %2 %3 %4 %5 %6 %7 %8 %9" -- thanks Apache for the tip
         */
        ReportEvent(event, EVENTLOG_ERROR_TYPE, 0, 3299, NULL, sizeof(lines) / sizeof(wchar*), 0, lines, 0);
        DeregisterEventSource(event);
    }
}


#endif /* BIT_WIN_LIKE */


#if (BIT_WIN_LIKE && !WINCE) || CYGWIN
/*
    Determine the registry hive by the first portion of the path. Return 
    a pointer to the rest of key path after the hive portion.
 */ 
static cchar *getHive(cchar *keyPath, HKEY *hive)
{
    char    key[BIT_MAX_PATH], *cp;
    ssize   len;

    assert(keyPath && *keyPath);

    *hive = 0;

    scopy(key, sizeof(key), keyPath);
    key[sizeof(key) - 1] = '\0';

    if ((cp = schr(key, '\\')) != 0) {
        *cp++ = '\0';
    }
    if (cp == 0 || *cp == '\0') {
        return 0;
    }
    if (!scaselesscmp(key, "HKEY_LOCAL_MACHINE") || !scaselesscmp(key, "HKLM")) {
        *hive = HKEY_LOCAL_MACHINE;
    } else if (!scaselesscmp(key, "HKEY_CURRENT_USER") || !scaselesscmp(key, "HKCU")) {
        *hive = HKEY_CURRENT_USER;
    } else if (!scaselesscmp(key, "HKEY_USERS")) {
        *hive = HKEY_USERS;
    } else if (!scaselesscmp(key, "HKEY_CLASSES_ROOT")) {
        *hive = HKEY_CLASSES_ROOT;
    } else {
        return 0;
    }
    if (*hive == 0) {
        return 0;
    }
    len = slen(key) + 1;
    return keyPath + len;
}


PUBLIC char *mprReadRegistry(cchar *key, cchar *name)
{
    HKEY        top, h;
    char        *value;
    ulong       type, size;

    assert(key && *key);

    /*
        Get the registry hive
     */
    if ((key = getHive(key, &top)) == 0) {
        return 0;
    }
    if (RegOpenKeyEx(top, wide(key), 0, KEY_READ, &h) != ERROR_SUCCESS) {
        return 0;
    }

    /*
        Get the type
     */
    if (RegQueryValueEx(h, wide(name), 0, &type, 0, &size) != ERROR_SUCCESS) {
        RegCloseKey(h);
        return 0;
    }
    if (type != REG_SZ && type != REG_EXPAND_SZ) {
        RegCloseKey(h);
        return 0;
    }
    if ((value = mprAlloc(size + 1)) == 0) {
        return 0;
    }
    if (RegQueryValueEx(h, wide(name), 0, &type, (uchar*) value, &size) != ERROR_SUCCESS) {
        RegCloseKey(h);
        return 0;
    }
    RegCloseKey(h);
    value[size] = '\0';
    return value;
}

PUBLIC int mprWriteRegistry(cchar *key, cchar *name, cchar *value)
{
    HKEY    top, h, subHandle;
    ulong   disposition;

    assert(key && *key);
    assert(value && *value);

    /*
        Get the registry hive
     */
    if ((key = getHive(key, &top)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    if (name && *name) {
        /*
            Write a registry string value
         */
        if (RegOpenKeyEx(top, wide(key), 0, KEY_ALL_ACCESS, &h) != ERROR_SUCCESS) {
            return MPR_ERR_CANT_ACCESS;
        }
        if (RegSetValueEx(h, wide(name), 0, REG_SZ, (uchar*) value, (int) slen(value) + 1) != ERROR_SUCCESS) {
            RegCloseKey(h);
            return MPR_ERR_CANT_READ;
        }

    } else {
        /*
            Create a new sub key
         */
        if (RegOpenKeyEx(top, wide(key), 0, KEY_CREATE_SUB_KEY, &h) != ERROR_SUCCESS){
            return MPR_ERR_CANT_ACCESS;
        }
        if (RegCreateKeyEx(h, wide(value), 0, NULL, REG_OPTION_NON_VOLATILE,
                KEY_ALL_ACCESS, NULL, &subHandle, &disposition) != ERROR_SUCCESS) {
            return MPR_ERR_CANT_ACCESS;
        }
        RegCloseKey(subHandle);
    }
    RegCloseKey(h);
    return 0;
}


#endif /* (BIT_WIN_LIKE && !WINCE) || CYGWIN */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/wince.c"
 */
/************************************************************************/

/**
    wince.c - Windows CE platform specific code.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************************** Includes ********************************************/



#if WINCE
/******************************************** Locals and Defines ************************************/
/*
    Windows file time is in 100 ns units starting 1601
    Unix (time_t) time is in sec units starting 1970
    MprTime time is in msec units starting 1970
 */
#define WIN_TICKS         10000000      /* Number of windows units in a second */
#define ORIGIN_GAP        11644473600   /* Gap in seconds between 1601 and 1970 */
#define fileTimeToTime(f) ((((((uint64) ((f).dwHighDateTime)) << 32) | (f).dwLowDateTime) / WIN_TICKS) - ORIGIN_GAP)

static char     *currentDir;            /* Current working directory */
static MprList  *files;                 /* List of open files */
PUBLIC int             errno;                  /* Last error */
static char     timzeone[2][32];        /* Standard and daylight savings zones */

/*
    Adjust by seconds between 1601 and 1970
 */
#define WIN_TICKS_TO_MPR  (WIN_TICKS / MPR_TICKS_PER_SEC)

/********************************************** Forwards ********************************************/

static HANDLE getHandle(int fd);
static long getJulianDays(SYSTEMTIME *when);
static void timeToFileTime(uint64 t, FILETIME *ft);

/************************************************ Code **********************************************/

PUBLIC int mprCreateOsService()
{
    files = mprCreateList();
    currentDir = sclone("/");
    return 0;
}


PUBLIC int mprStartOsService()
{
    WSADATA     wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return -1;
    }
    return 0;
}


PUBLIC void mprStopOsService()
{
    WSACleanup();
}


PUBLIC int mprGetRandomBytes(char *buf, int length, bool block)
{
    HCRYPTPROV      prov;
    int             rc;

    rc = 0;
    if (!CryptAcquireContext(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | 0x40)) {
        return mprGetError();
    }
    if (!CryptGenRandom(prov, length, buf)) {
        rc = mprGetError();
    }
    CryptReleaseContext(prov, 0);
    return rc;
}


PUBLIC int mprLoadModule(MprModule *mp)
    cchar *moduleName, cchar *initFunction)
{
    MprModuleEntry  fn;
    void            *handle;
    char            *baseName;

    assert(moduleName && *moduleName);

    baseName = mprGetPathBase(mp->path);
    if ((handle = GetModuleHandle(baseName)) == 0 && (handle = LoadLibrary(mp->path)) == 0) {
        mprError("Cannot load module %s\nReason: \"%d\"\n", mp->path, mprGetOsError());
        return MPR_ERR_CANT_READ;
    } 
    mp->handle = handle;
    if (mp->entry) {
        if ((fn = (MprModuleEntry) GetProcAddress((HINSTANCE) handle, mp->entry)) != 0) {
            if ((fn)(mp->moduleData, mp)) < 0) {
                mprError("Initialization for module %s failed", mp->name);
                FreeLibrary((HINSTANCE) handle);
                return MPR_ERR_CANT_ACCESS;
            }
        } else {
            mprError("Cannot load module %s\nReason: can't find function \"%s\"\n", mp->name, mp->entry);
            FreeLibrary((HINSTANCE) handle);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    return mp;
}


#if KEEP
/*
    Determine the registry hive by the first portion of the path. Return 
    a pointer to the rest of key path after the hive portion.
 */ 
static cchar *getHive(cchar *keyPath, HKEY *hive)
{
    char    key[BIT_MAX_PATH], *cp;
    int     len;

    assert(keyPath && *keyPath);

    *hive = 0;

    scopy(key, sizeof(key), keyPath);
    key[sizeof(key) - 1] = '\0';

    if (cp = strchr(key, '\\')) {
        *cp++ = '\0';
    }
    if (cp == 0 || *cp == '\0') {
        return 0;
    }
    if (!mprStrcmpAnyCase(key, "HKEY_LOCAL_MACHINE")) {
        *hive = HKEY_LOCAL_MACHINE;
    } else if (!mprStrcmpAnyCase(key, "HKEY_CURRENT_USER")) {
        *hive = HKEY_CURRENT_USER;
    } else if (!mprStrcmpAnyCase(key, "HKEY_USERS")) {
        *hive = HKEY_USERS;
    } else if (!mprStrcmpAnyCase(key, "HKEY_CLASSES_ROOT")) {
        *hive = HKEY_CLASSES_ROOT;
    } else {
        return 0;
    }
    if (*hive == 0) {
        return 0;
    }
    len = slen(key) + 1;
    return keyPath + len;
}


PUBLIC int mprReadRegistry(char **buf, int max, cchar *key, cchar *name)
{
    HKEY        top, h;
    LPWSTR      wkey, wname;
    char        *value;
    ulong       type, size;

    assert(key && *key);
    assert(buf);

    if ((key = getHive(key, &top)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    wkey = mprToUni(key);
    if (RegOpenKeyEx(top, wkey, 0, KEY_READ, &h) != ERROR_SUCCESS) {
        return MPR_ERR_CANT_ACCESS;
    }

    /*
        Get the type
     */
    wname = mprToUni(name);
    if (RegQueryValueEx(h, wname, 0, &type, 0, &size) != ERROR_SUCCESS) {
        RegCloseKey(h);
        return MPR_ERR_CANT_READ;
    }
    if (type != REG_SZ && type != REG_EXPAND_SZ) {
        RegCloseKey(h);
        return MPR_ERR_BAD_TYPE;
    }
    value = mprAlloc(size);
    if ((int) size > max) {
        RegCloseKey(h);
        assert(!MPR_ERR_WONT_FIT);
        return MPR_ERR_WONT_FIT;
    }
    if (RegQueryValueEx(h, wname, 0, &type, (uchar*) value, &size) != ERROR_SUCCESS) {
        RegCloseKey(h);
        return MPR_ERR_CANT_READ;
    }
    RegCloseKey(h);
    *buf = value;
    return 0;
}


PUBLIC void mprSetInst(Mpr *mpr, long inst)
{
    mpr->appInstance = inst;
}


PUBLIC void mprSetHwnd(HWND h)
{
    MPR->service->hwnd = h;
}


PUBLIC void mprSetSocketMessage(int socketMessage)
{
    MPR->service->socketMessage = socketMessage;
}
#endif /* WINCE */


PUBLIC void mprSleep(MprTicks timeout)
{
    Sleep((int) timeout);
}


PUBLIC void mprSleep(MprTicks timeout)
{
    mprYield(MPR_YIELD_STICKY | MPR_YIELD_NO_BLOCK);
    mprNap(timeout);
    mprResetYield();
}


PUBLIC void mprUnloadNativeModule(MprModule *mp)
{
    assert(mp->handle);

    if (FreeLibrary((HINSTANCE) mp->handle) == 0) {
        return MPR_ERR_ABORTED;
    }
    return 0;
}


#if KEEP
PUBLIC void mprWriteToOsLog(cchar *message, int flags, int level)
{
    HKEY        hkey;
    void        *event;
    long        errorType;
    ulong       exists;
    char        buf[BIT_MAX_PATH], logName[BIT_MAX_PATH], *lines[9], *cp, *value;
    int         type;
    static int  once = 0;

    scopy(buf, sizeof(buf), message);
    cp = &buf[slen(buf) - 1];
    while (*cp == '\n' && cp > buf) {
        *cp-- = '\0';
    }

    type = EVENTLOG_ERROR_TYPE;

    lines[0] = buf;
    lines[1] = 0;
    lines[2] = lines[3] = lines[4] = lines[5] = 0;
    lines[6] = lines[7] = lines[8] = 0;

    if (once == 0) {
        /*  Initialize the registry */
        once = 1;
        fmt(logName, sizeof(logName), "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\%s", mprGetAppName());
        hkey = 0;

        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, logName, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hkey, &exists) == ERROR_SUCCESS) {
            value = "%SystemRoot%\\System32\\netmsg.dll";
            if (RegSetValueEx(hkey, "EventMessageFile", 0, REG_EXPAND_SZ, 
                    (uchar*) value, slen(value) + 1) != ERROR_SUCCESS) {
                RegCloseKey(hkey);
                return;
            }
            errorType = EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE;
            if (RegSetValueEx(hkey, "TypesSupported", 0, REG_DWORD, (uchar*) &errorType, sizeof(DWORD)) != ERROR_SUCCESS) {
                RegCloseKey(hkey);
                return;
            }
            RegCloseKey(hkey);
        }
    }
    event = RegisterEventSource(0, mprGetAppName());
    if (event) {
        /*
            3299 is the event number for the generic message in netmsg.dll.
            "%1 %2 %3 %4 %5 %6 %7 %8 %9" -- thanks Apache for the tip
         */
        ReportEvent(event, EVENTLOG_ERROR_TYPE, 0, 3299, NULL, sizeof(lines) / sizeof(char*), 0, (LPCSTR*) lines, 0);
        DeregisterEventSource(event);
    }
}

PUBLIC int mprWriteRegistry(cchar *key, cchar *name, cchar *value)
{
    HKEY    top, h, subHandle;
    ulong   disposition;

    assert(key && *key);
    assert(name && *name);
    assert(value && *value);

    /*
        Get the registry hive
     */
    if ((key = getHive(key, &top)) == 0) {
        return MPR_ERR_CANT_ACCESS;
    }

    if (name) {
        /*
            Write a registry string value
         */
        if (RegOpenKeyEx(top, key, 0, KEY_ALL_ACCESS, &h) != ERROR_SUCCESS) {
            return MPR_ERR_CANT_ACCESS;
        }
        if (RegSetValueEx(h, name, 0, REG_SZ, value, slen(value) + 1) != ERROR_SUCCESS) {
            RegCloseKey(h);
            return MPR_ERR_CANT_READ;
        }

    } else {
        /*
            Create a new sub key
         */
        if (RegOpenKeyEx(top, key, 0, KEY_CREATE_SUB_KEY, &h) != ERROR_SUCCESS){
            return MPR_ERR_CANT_ACCESS;
        }
        if (RegCreateKeyEx(h, name, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &subHandle, &disposition) != ERROR_SUCCESS) {
            return MPR_ERR_CANT_ACCESS;
        }
        RegCloseKey(subHandle);
    }
    RegCloseKey(h);
    return 0;
}
#endif


/******************************************* Posix Layer ********************************/

PUBLIC int access(cchar *path, int flags)
{
    char    *tmpPath;
    int     rc;

    if (!mprIsPathAbs(MPR, path)) {
        path = (cchar*) tmpPath = mprJoinPath(MPR, currentDir, path);
    } else {
        tmpPath = 0;
    }
    rc = GetFileAttributesA(path) != -1 ? 0 : -1;
    return rc;
}


PUBLIC int chdir(cchar *dir)
{
    currentDir = mprGetAbsPath(MPR, dir);
    return 0;
}


PUBLIC int chmod(cchar *path, int mode)
{
    /* CE has no such permissions */
    return 0;
}


PUBLIC int close(int fd)
{
    int     rc;

    //  LOCKING
    rc = CloseHandle(getHandle(fd));
    mprSetItem(files, fd, NULL);
    return (rc != 0) ? 0 : -1;
}


PUBLIC long _get_osfhandle(int handle)
{
    return (long) handle;
}


PUBLIC char *getenv(cchar *key)
{
    return 0;
}


PUBLIC char *getcwd(char *buf, int size)
{
    scopy(buf, size, currentDir);
    return buf;
}


PUBLIC uint getpid() {
    return 0;
}


PUBLIC long lseek(int handle, long offset, int origin)
{
    switch (origin) {
        case SEEK_SET: offset = FILE_BEGIN; break;
        case SEEK_CUR: offset = FILE_CURRENT; break;
        case SEEK_END: offset = FILE_END; break;
    }
    return SetFilePointer((HANDLE) handle, offset, NULL, origin);
}


PUBLIC int mkdir(cchar *dir, int mode)
{
    char    *tmpDir;
    uni     *wdir;
    int     rc;

    if (!mprIsPathAbs(MPR, dir)) {
        dir = (cchar*) tmpDir = mprJoinPath(MPR, currentDir, dir);
    } else {
        tmpDir = 0;
    }

    wdir = mprToUni(MPR, dir);
    rc = CreateDirectoryW(wdir, NULL);
    return (rc != 0) ? 0 : -1;
}


static HANDLE getHandle(int fd)
{
    //  LOCKING
    return (HANDLE) mprGetItem(files, fd);
}


static int addHandle(HANDLE h)
{
    int     i;

    //  LOCKING
    for (i = 0; i < files->length; i++) {
        if (files->items[i] == 0) {
            mprSetItem(files, i, h);
            return i;
        }
    }
    return mprAddItem(files, h);
}


PUBLIC int _open_osfhandle(int *handle, int flags)
{
    return addHandle((HANDLE) handle);
}


PUBLIC uint open(cchar *path, int mode, va_list arg)
{
    uni     *wpath;
    char    *tmpPath;
    DWORD   accessFlags, shareFlags, createFlags;
    HANDLE  h;

    if (!mprIsPathAbs(MPR, path)) {
        path = (cchar*) tmpPath = mprGetAbsPath(MPR, path);
    } else {
        tmpPath = 0;
    }

    shareFlags = FILE_SHARE_READ;
    accessFlags = 0;
    createFlags = 0;

    if ((mode & O_RDWR) != 0) {
        accessFlags = GENERIC_READ | GENERIC_WRITE;
    } else if ((mode & O_WRONLY) != 0) {
        accessFlags = GENERIC_WRITE;
    } else {
        accessFlags = GENERIC_READ;
    }
    if ((mode & O_CREAT) != 0) {
        createFlags = CREATE_ALWAYS;
    } else {
        createFlags = OPEN_EXISTING;
    }
    wpath = mprToUni(MPR, path);
    h = CreateFileW(wpath, accessFlags, shareFlags, NULL, createFlags, FILE_ATTRIBUTE_NORMAL, NULL);

    return h == INVALID_HANDLE_VALUE ? -1 : addHandle(h);
}


PUBLIC int read(int fd, void *buffer, uint length)
{
    DWORD   dw;

    ReadFile(getHandle(fd), buffer, length, &dw, NULL);
    return (int) dw;
}


PUBLIC int rename(cchar *oldname, cchar *newname)
{
    uni     *from, *to;
    char    *tmpOld, *tmpNew;
    int     rc;

    if (!mprIsPathAbs(MPR, oldname)) {
        oldname = (cchar*) tmpOld = mprJoinPath(MPR, currentDir, oldname);
    } else {
        tmpOld = 0;
    }
    if (!mprIsPathAbs(MPR, newname)) {
        newname = (cchar*) tmpNew = mprJoinPath(MPR, currentDir, newname);
    } else {
        tmpNew = 0;
    }
    from = mprToUni(MPR, oldname);
    to = mprToUni(MPR, newname);
    rc = MoveFileW(from, to);
    return rc == 0 ? 0 : -1;
}


PUBLIC int rmdir(cchar *dir)
{
    uni     *wdir;
    char    *tmpDir;
    int     rc;

    if (!mprIsPathAbs(MPR, dir)) {
        dir = (cchar*) tmpDir = mprJoinPath(MPR, currentDir, dir);
    } else {
        tmpDir = 0;
    }
    wdir = mprToUni(MPR, dir);
    rc = RemoveDirectoryW(wdir);
    return rc == 0 ? 0 : -1;
}


PUBLIC int stat(cchar *path, struct stat *sbuf)
{
    WIN32_FIND_DATAW    fd;
    DWORD               attributes;
    HANDLE              h;
    DWORD               dwSizeLow, dwSizeHigh, dwError;
    char                *tmpPath;
    uni                 *wpath;

    dwSizeLow = 0;
    dwSizeHigh = 0;
    dwError = 0;

    memset(sbuf, 0, sizeof(struct stat));

    if (!mprIsPathAbs(MPR, path)) {
        path = (cchar*) tmpPath = mprJoinPath(MPR, currentDir, path);
    } else {
        tmpPath = 0;
    }
    wpath = mprToUni(MPR, path);

    attributes = GetFileAttributesW(wpath);
    if (attributes == -1) {
        return -1;
    }
    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        sbuf->st_mode += S_IFDIR;
    } else {
        sbuf->st_mode += S_IFREG;
    }

    h = FindFirstFileW(wpath, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        if (wpath[wcslen(wpath)-1]  == L'\\') {
            wpath[wcslen(wpath)-1] = L'\0';
            h = FindFirstFileW(wpath, &fd);
            if (h == INVALID_HANDLE_VALUE) {
                return 0;
            }
        } else {
            return 0;
        }
    }
    sbuf->st_atime = (time_t) fileTimeToTime(fd.ftLastAccessTime);
    sbuf->st_mtime = (time_t) fileTimeToTime(fd.ftLastWriteTime);
    sbuf->st_ctime = (time_t) fileTimeToTime(fd.ftCreationTime);
    sbuf->st_size  = fd.nFileSizeLow;

    FindClose(h);
    return 0;
}


/*
    Convert time in seconds to a file time
 */
static void timeToFileTime(uint64 t, FILETIME *ft)
{
    t += ORIGIN_GAP;
    t *= WIN_TICKS;
    ft->dwHighDateTime = (DWORD) ((t >> 32) & 0xFFFFFFFF);
    ft->dwLowDateTime  = (DWORD) (t & 0xFFFFFFFF);
}


/*
    Get the Julian current year day.

    General Julian Day formula:
        a = (14 - month) / 12;
        y = year + 4800 - a;
        m = month + 12 * a - 3;
        jd = day + (y * 365) + (y / 4) - (y / 100) + (y / 400) + (m * 153 + 2) / 5 - 32045;
 */
static long getJulianDays(SYSTEMTIME *when)
{
    int     y, m, d, a, day, startYearDay;

    a = (14 - when->wMonth) / 12;
    y = when->wYear + 4800 - a;
    m = when->wMonth + 12   a - 3;
    d = when->wDay;

    /*
        Compute the difference between Julian days for Jan 1 and "when" of the same year
     */
    day = d + (y   365) + (y / 4) - (y / 100) + (y / 400) + (m * 153 + 2) / 5;
    y = when->wYear + 4799;
    startYearDay = 1 + (y   365) + (y / 4) - (y / 100) + (y / 400) + 1532 / 5;

    return day - startYearDay;
}


struct tm *gmtime_r(const time_t *when, struct tm *tp)
{
    FILETIME    f;
    SYSTEMTIME  s;
    
    timeToFileTime(*when, &f);
    FileTimeToSystemTime(&f, &s);

    tp->tm_year  = s.wYear - 1900;
    tp->tm_mon   = s.wMonth- 1;
    tp->tm_wday  = s.wDayOfWeek;
    tp->tm_mday  = s.wDay;
    tp->tm_yday  = getJulianDays(&s);
    tp->tm_hour  = s.wHour;
    tp->tm_min   = s.wMinute;
    tp->tm_sec   = s.wSecond;
    tp->tm_isdst = 0;

    return tp;
}


struct tm *localtime_r(const time_t *when, struct tm *tp)
{
    FILETIME                f;
    SYSTEMTIME              s;
    TIME_ZONE_INFORMATION   tz;
    int                     bias, rc;

    assert(when);
    assert(tp);

    rc = GetTimeZoneInformation(&tz);
    bias = tz.Bias;
    if (rc == TIME_ZONE_ID_DAYLIGHT) {
        tp->tm_isdst = 1;
        bias += tz.DaylightBias;
    } else {
        tp->tm_isdst = 0;
    }
    bias *= 60;

    timeToFileTime(*when - bias, &f);
    FileTimeToSystemTime(&f, &s);
    
    tp->tm_year   = s.wYear - 1900;
    tp->tm_mon    = s.wMonth- 1;
    tp->tm_wday   = s.wDayOfWeek;
    tp->tm_mday   = s.wDay;
    tp->tm_yday   = getJulianDays(&s);
    tp->tm_hour   = s.wHour;
    tp->tm_min    = s.wMinute;
    tp->tm_sec    = s.wSecond;

    return tp;
}


PUBLIC time_t mktime(struct tm *tp)
{
    TIME_ZONE_INFORMATION   tz;
    SYSTEMTIME              s;
    FILETIME                f;
    time_t                  result;
    int                     rc, bias;

    assert(tp);

    rc = GetTimeZoneInformation(&tz);
    bias = tz.Bias;
    if (rc == TIME_ZONE_ID_DAYLIGHT) {
        tp->tm_isdst = 1;
        bias += tz.DaylightBias;
    }
    bias *= 60;
    
    s.wYear = tp->tm_year + 1900;
    s.wMonth = tp->tm_mon + 1;
    s.wDayOfWeek = tp->tm_wday;
    s.wDay = tp->tm_mday;
    s.wHour = tp->tm_hour;
    s.wMinute = tp->tm_min;
    s.wSecond = tp->tm_sec;

    SystemTimeToFileTime(&s, &f);
    result = (time_t) (fileTimeToTime(f) + tz.Bias   60);
    if (rc == TIME_ZONE_ID_DAYLIGHT) {
        result -= bias;
    }
    return result;
}


PUBLIC int write(int fd, cvoid *buffer, uint count)
{
    DWORD   dw;

    WriteFile(getHandle(fd), buffer, count, &dw, NULL);
    return (int) dw;
}


PUBLIC int unlink(cchar *file)
{
    uni     *wpath;
    int     rc;

    wpath = mprToUni(MPR, file);
    rc = DeleteFileW(wpath);
    return rc == 0 ? 0 : -1;
}


/********************************************** Windows32 Extensions *********************************************/

PUBLIC WINBASEAPI HANDLE WINAPI CreateFileA(LPCSTR path, DWORD access, DWORD sharing,
    LPSECURITY_ATTRIBUTES security, DWORD create, DWORD flags, HANDLE template)
{
    LPWSTR  wpath;
    HANDLE  h;

    wpath = mprToUni(MPR, path);
    h = CreateFileW(wpath, access, sharing, security, create, flags, template);
    return h;
}


PUBLIC BOOL WINAPI CreateProcessA(LPCSTR app, LPCSTR cmd, LPSECURITY_ATTRIBUTES att, LPSECURITY_ATTRIBUTES threadatt,
    BOOL options, DWORD flags, LPVOID env, LPSTR dir, LPSTARTUPINFO lpsi, LPPROCESS_INFORMATION info)
{
    LPWSTR      wapp, wcmd, wdir;

    wapp  = mprToUni(MPR, app);
    wcmd  = mprToUni(MPR, cmd);
    wdir  = mprToUni(MPR, dir);

    return CreateProcessW(wapp, wcmd, att, threadatt, options, flags, env, wdir, lpsi, info);
}


PUBLIC HANDLE FindFirstFileA(LPCSTR path, WIN32_FIND_DATAA *data)
{
    WIN32_FIND_DATAW    wdata;
    LPWSTR              wpath;
    HANDLE              h;
    char                *file;

    wpath = mprToUni(MPR, path);
    h = FindFirstFileW(wpath, &wdata);
    
    file = mprToMulti(MPR, wdata.cFileName);
    strcpy(data->cFileName, file);
    return h;
}


PUBLIC BOOL FindNextFileA(HANDLE handle, WIN32_FIND_DATAA *data)
{
    WIN32_FIND_DATAW    wdata;
    char                *file;
    BOOL                result;

    result = FindNextFileW(handle, &wdata);
    file = mprToMulti(MPR, wdata.cFileName);
    strcpy(data->cFileName, file);
    return result;
}


PUBLIC DWORD GetFileAttributesA(cchar *path)
{
    LPWSTR      wpath;
    DWORD       result;

    wpath = mprToUni(MPR, path);
    result = GetFileAttributesW(wpath);
    return result;
}


PUBLIC DWORD GetModuleFileNameA(HMODULE module, LPSTR buf, DWORD size)
{
    LPWSTR      wpath;
    LPSTR       mb;
    ssize       ret;

    wpath = mprAlloc( size * sizeof(wchar_t));
    ret = GetModuleFileNameW(module, wpath, size);
    mb = mprToMulti(MPR, wpath);
    strcpy(buf, mb);
    return ret;
}


PUBLIC WINBASEAPI HMODULE WINAPI GetModuleHandleA(LPCSTR path)
{
    LPWSTR      wpath;

    wpath = mprToUni(MPR, path);
    return GetModuleHandleW(wpath);
}


PUBLIC void GetSystemTimeAsFileTime(FILETIME *ft)
{
    SYSTEMTIME  s;

    GetSystemTime(&s);
    SystemTimeToFileTime(&s, ft);
}


PUBLIC HINSTANCE WINAPI LoadLibraryA(LPCSTR path)
{
    LPWSTR      wpath;

    wpath = mprToUni(MPR, path);
    return LoadLibraryW(wpath);
}

PUBLIC void mprWriteToOsLog(cchar *message, int flags, int level)
{
}

#endif /* WINCE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/xml.c"
 */
/************************************************************************/

/**
    xml.c - A simple SAX style XML parser

    This is a recursive descent parser for XML text files. It is a one-pass simple parser that invokes a user 
    supplied callback for key tokens in the XML file. The user supplies a read function so that XML files can 
    be parsed from disk or in-memory. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Forwards **********************************/

static MprXmlToken getXmlToken(MprXml *xp, int state);
static int  getNextChar(MprXml *xp);
static void manageXml(MprXml *xml, int flags);
static int  scanFor(MprXml *xp, char *str);
static int  parseNext(MprXml *xp, int state);
static int  putLastChar(MprXml *xp, int c);
static void xmlError(MprXml *xp, char *fmt, ...);
static void trimToken(MprXml *xp);

/************************************ Code ************************************/

PUBLIC MprXml *mprXmlOpen(ssize initialSize, ssize maxSize)
{
    MprXml  *xp;

    xp = mprAllocObj(MprXml, manageXml);
    
    xp->inBuf = mprCreateBuf(BIT_MAX_BUFFER, BIT_MAX_BUFFER);
    xp->tokBuf = mprCreateBuf(initialSize, maxSize);
    return xp;
}


static void manageXml(MprXml *xml, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(xml->inBuf);
        mprMark(xml->tokBuf);
        mprMark(xml->parseArg);
        mprMark(xml->inputArg);
        mprMark(xml->errMsg);
    }
}


PUBLIC void mprXmlSetParserHandler(MprXml *xp, MprXmlHandler h)
{
    assert(xp);
    xp->handler = h;
}


PUBLIC void mprXmlSetInputStream(MprXml *xp, MprXmlInputStream s, void *arg)
{
    assert(xp);

    xp->readFn = s;
    xp->inputArg = arg;
}


/*
    Set the parse arg
 */ 
PUBLIC void mprXmlSetParseArg(MprXml *xp, void *parseArg)
{
    assert(xp);

    xp->parseArg = parseArg;
}


/*
    Set the parse arg
 */ 
PUBLIC void *mprXmlGetParseArg(MprXml *xp)
{
    assert(xp);

    return xp->parseArg;
}


/*
    Parse an XML file. Return 0 for success, -1 for error.
 */ 
PUBLIC int mprXmlParse(MprXml *xp)
{
    assert(xp);

    return parseNext(xp, MPR_XML_BEGIN);
}


/*
    XML recursive descent parser. Return -1 for errors, 0 for EOF and 1 if there is still more data to parse.
 */
static int parseNext(MprXml *xp, int state)
{
    MprXmlHandler   handler;
    MprXmlToken     token;
    MprBuf          *tokBuf;
    char            *tname, *aname;
    int             rc;

    assert(state >= 0);

    tokBuf = xp->tokBuf;
    handler = xp->handler;
    tname = aname = 0;
    rc = 0;
    
    /*
        In this parse loop, the state is never assigned EOF or ERR. In such cases we always return EOF or ERR.
     */
    while (1) {

        token = getXmlToken(xp, state);

        if (token == MPR_XMLTOK_TOO_BIG) {
            xmlError(xp, "XML token is too big");
            return MPR_ERR_WONT_FIT;
        }

        switch (state) {
        case MPR_XML_BEGIN:     /* ------------------------------------------ */
            /*
                Expect to get an element, comment or processing instruction 
             */
            switch (token) {
            case MPR_XMLTOK_EOF:
                return 0;

            case MPR_XMLTOK_LS:
                /*
                    Recurse to handle the new element, comment etc.
                 */
                rc = parseNext(xp, MPR_XML_AFTER_LS);
                if (rc < 0) {
                    return rc;
                }
                break;

            default:
                xmlError(xp, "Syntax error");
                return MPR_ERR_BAD_SYNTAX;
            }
            break;

        case MPR_XML_AFTER_LS: /* ------------------------------------------ */
            switch (token) {
            case MPR_XMLTOK_COMMENT:
                state = MPR_XML_COMMENT;
                rc = (*handler)(xp, state, "!--", 0, mprGetBufStart(tokBuf));
                if (rc < 0) {
                    return rc;
                }
                return 1;

            case MPR_XMLTOK_CDATA:
                state = MPR_XML_CDATA;
                rc = (*handler)(xp, state, "!--", 0, mprGetBufStart(tokBuf));
                if (rc < 0) {
                    return rc;
                }
                return 1;

            case MPR_XMLTOK_INSTRUCTIONS:
                /* Just ignore processing instructions */
                return 1;

            case MPR_XMLTOK_TEXT:
                state = MPR_XML_NEW_ELT;
                tname = sclone(mprGetBufStart(tokBuf));
                if (tname == 0) {
                    assert(!MPR_ERR_MEMORY);
                    return MPR_ERR_MEMORY;
                }
                rc = (*handler)(xp, state, tname, 0, 0);
                if (rc < 0) {
                    return rc;
                }
                break;

            default:
                xmlError(xp, "Syntax error");
                return MPR_ERR_BAD_SYNTAX;
            }
            break;

        case MPR_XML_NEW_ELT:   /* ------------------------------------------ */
            /*
                We have seen the opening "<element" for a new element and have not yet seen the terminating 
                ">" of the opening element.
             */
            switch (token) {
            case MPR_XMLTOK_TEXT:
                /*
                    Must be an attribute name
                 */
                aname = sclone(mprGetBufStart(tokBuf));
                token = getXmlToken(xp, state);
                if (token != MPR_XMLTOK_EQ) {
                    xmlError(xp, "Missing assignment for attribute \"%s\"", aname);
                    return MPR_ERR_BAD_SYNTAX;
                }

                token = getXmlToken(xp, state);
                if (token != MPR_XMLTOK_TEXT) {
                    xmlError(xp, "Missing value for attribute \"%s\"", aname);
                    return MPR_ERR_BAD_SYNTAX;
                }
                state = MPR_XML_NEW_ATT;
                rc = (*handler)(xp, state, tname, aname, mprGetBufStart(tokBuf));
                if (rc < 0) {
                    return rc;
                }
                state = MPR_XML_NEW_ELT;
                break;

            case MPR_XMLTOK_GR:
                /*
                    This is ">" the termination of the opening element
                 */
                if (*tname == '\0') {
                    xmlError(xp, "Missing element name");
                    return MPR_ERR_BAD_SYNTAX;
                }

                /*
                    Tell the user that the opening element is now complete
                 */
                state = MPR_XML_ELT_DEFINED;
                rc = (*handler)(xp, state, tname, 0, 0);
                if (rc < 0) {
                    return rc;
                }
                state = MPR_XML_ELT_DATA;
                break;

            case MPR_XMLTOK_SLASH_GR:
                /*
                    If we see a "/>" then this is a solo element
                 */
                if (*tname == '\0') {
                    xmlError(xp, "Missing element name");
                    return MPR_ERR_BAD_SYNTAX;
                }
                state = MPR_XML_SOLO_ELT_DEFINED;
                rc = (*handler)(xp, state, tname, 0, 0);
                if (rc < 0) {
                    return rc;
                }
                return 1;
    
            default:
                xmlError(xp, "Syntax error");
                return MPR_ERR_BAD_SYNTAX;
            }
            break;

        case MPR_XML_ELT_DATA:      /* -------------------------------------- */
            /*
                We have seen the full opening element "<name ...>" and now await data or another element.
             */
            if (token == MPR_XMLTOK_LS) {
                /*
                    Recurse to handle the new element, comment etc.
                 */
                rc = parseNext(xp, MPR_XML_AFTER_LS);
                if (rc < 0) {
                    return rc;
                }
                break;

            } else if (token == MPR_XMLTOK_LS_SLASH) {
                state = MPR_XML_END_ELT;
                break;

            } else if (token != MPR_XMLTOK_TEXT) {
                return rc;
            }
            if (mprGetBufLength(tokBuf) > 0) {
                /*
                    Pass the data between the element to the user
                 */
                rc = (*handler)(xp, state, tname, 0, mprGetBufStart(tokBuf));
                if (rc < 0) {
                    return rc;
                }
            }
            break;

        case MPR_XML_END_ELT:           /* -------------------------------------- */
            if (token != MPR_XMLTOK_TEXT) {
                xmlError(xp, "Missing closing element name for \"%s\"", tname);
                return MPR_ERR_BAD_SYNTAX;
            }
            /*
                The closing element name must match the opening element name 
             */
            if (strcmp(tname, mprGetBufStart(tokBuf)) != 0) {
                xmlError(xp, "Closing element name \"%s\" does not match on line %d. Opening name \"%s\"",
                    mprGetBufStart(tokBuf), xp->lineNumber, tname);
                return MPR_ERR_BAD_SYNTAX;
            }
            rc = (*handler)(xp, state, tname, 0, 0);
            if (rc < 0) {
                return rc;
            }
            if (getXmlToken(xp, state) != MPR_XMLTOK_GR) {
                xmlError(xp, "Syntax error");
                return MPR_ERR_BAD_SYNTAX;
            }
            return 1;

        case MPR_XML_EOF:       /* ---------------------------------------------- */
            return 0;

        case MPR_XML_ERR:   /* ---------------------------------------------- */
        default:
            return MPR_ERR;
        }
    }
    assert(0);
}


/*
    Lexical analyser for XML. Return the next token reading input as required. It uses a one token look ahead and 
    push back mechanism (LAR1 parser). Text token identifiers are left in the tokBuf parser buffer on exit. This Lex 
    has special cases for the states MPR_XML_ELT_DATA where we have an optimized read of element data, and 
    MPR_XML_AFTER_LS where we distinguish between element names, processing instructions and comments. 
 */
static MprXmlToken getXmlToken(MprXml *xp, int state)
{
    MprBuf      *tokBuf;
    char        *cp;
    int         c, rc;

    assert(state >= 0);
    tokBuf = xp->tokBuf;

    if ((c = getNextChar(xp)) < 0) {
        return MPR_XMLTOK_EOF;
    }
    mprFlushBuf(tokBuf);

    /*
        Special case parsing for names and for element data. We do this for performance so we can return to the caller 
        the largest token possible.
     */
    if (state == MPR_XML_ELT_DATA) {
        /*
            Read all the data up to the start of the closing element "<" or the start of a sub-element.
         */
        if (c == '<') {
            if ((c = getNextChar(xp)) < 0) {
                return MPR_XMLTOK_EOF;
            }
            if (c == '/') {
                return MPR_XMLTOK_LS_SLASH;
            }
            putLastChar(xp, c);
            return MPR_XMLTOK_LS;
        }
        do {
            if (mprPutCharToBuf(tokBuf, c) < 0) {
                return MPR_XMLTOK_TOO_BIG;
            }
            if ((c = getNextChar(xp)) < 0) {
                return MPR_XMLTOK_EOF;
            }
        } while (c != '<');

        /*
            Put back the last look-ahead character
         */
        putLastChar(xp, c);

        /*
            If all white space, then zero the token buffer
         */
        for (cp = tokBuf->start; *cp; cp++) {
            if (!isspace((uchar) *cp & 0x7f)) {
                return MPR_XMLTOK_TEXT;
            }
        }
        mprFlushBuf(tokBuf);
        return MPR_XMLTOK_TEXT;
    }

    while (1) {
        switch (c) {
        case ' ':
        case '\n':
        case '\t':
        case '\r':
            break;

        case '<':
            if ((c = getNextChar(xp)) < 0) {
                return MPR_XMLTOK_EOF;
            }
            if (c == '/') {
                return MPR_XMLTOK_LS_SLASH;
            }
            putLastChar(xp, c);
            return MPR_XMLTOK_LS;
    
        case '=':
            return MPR_XMLTOK_EQ;

        case '>':
            return MPR_XMLTOK_GR;

        case '/':
            if ((c = getNextChar(xp)) < 0) {
                return MPR_XMLTOK_EOF;
            }
            if (c == '>') {
                return MPR_XMLTOK_SLASH_GR;
            }
            return MPR_XMLTOK_ERR;
        
        case '\"':
        case '\'':
            xp->quoteChar = c;
            /* Fall through */

        default:
            /*
                We handle element names, attribute names and attribute values 
                here. We do NOT handle data between elements here. Read the 
                token.  Stop on white space or a closing element ">"
             */
            if (xp->quoteChar) {
                if ((c = getNextChar(xp)) < 0) {
                    return MPR_XMLTOK_EOF;
                }
                while (c != xp->quoteChar) {
                    if (mprPutCharToBuf(tokBuf, c) < 0) {
                        return MPR_XMLTOK_TOO_BIG;
                    }
                    if ((c = getNextChar(xp)) < 0) {
                        return MPR_XMLTOK_EOF;
                    }
                }
                xp->quoteChar = 0;

            } else {
                while (!isspace((uchar) c) && c != '>' && c != '/' && c != '=') {
                    if (mprPutCharToBuf(tokBuf, c) < 0) {
                        return MPR_XMLTOK_TOO_BIG;
                    }
                    if ((c = getNextChar(xp)) < 0) {
                        return MPR_XMLTOK_EOF;
                    }
                }
                putLastChar(xp, c);
            }
            if (mprGetBufLength(tokBuf) < 0) {
                return MPR_XMLTOK_ERR;
            }
            mprAddNullToBuf(tokBuf);

            if (state == MPR_XML_AFTER_LS) {
                /*
                    If we are just inside an element "<", then analyze what we have to see if we have an element name, 
                    instruction or comment. Tokbuf will hold "?" for instructions or "!--" for comments.
                 */
                if (mprLookAtNextCharInBuf(tokBuf) == '?') {
                    /*  Just ignore processing instructions */
                    rc = scanFor(xp, "?>");
                    if (rc < 0) {
                        return MPR_XMLTOK_TOO_BIG;
                    } else if (rc == 0) {
                        return MPR_XMLTOK_ERR;
                    }
                    return MPR_XMLTOK_INSTRUCTIONS;

                } else if (mprLookAtNextCharInBuf(tokBuf) == '!') {
                    if (strncmp((char*) tokBuf->start, "![CDATA[", 8) == 0) {
                        mprAdjustBufStart(tokBuf, 8);
                        rc = scanFor(xp, "]]>");
                        if (rc < 0) {
                            return MPR_XMLTOK_TOO_BIG;
                        } else if (rc == 0) {
                            return MPR_XMLTOK_ERR;
                        }
                        return MPR_XMLTOK_CDATA;

                    } else {
                        mprFlushBuf(tokBuf);
                        rc = scanFor(xp, "-->");
                        if (rc < 0) {
                            return MPR_XMLTOK_TOO_BIG;
                        } else if (rc == 0) {
                            return MPR_XMLTOK_ERR;
                        }
                        return MPR_XMLTOK_COMMENT;
                    }
                }
            }
            trimToken(xp);
            return MPR_XMLTOK_TEXT;
        }
        if ((c = getNextChar(xp)) < 0) {
            return MPR_XMLTOK_EOF;
        }
    }

    /* Should never get here */
    assert(0);
    return MPR_XMLTOK_ERR;
}


/*
    Scan for a pattern. Trim the pattern from the token. Return 1 if the pattern was found, return 0 if not found. 
    Return < 0 on errors.
 */
static int scanFor(MprXml *xp, char *pattern)
{
    MprBuf  *tokBuf;
    char    *start, *p, *cp;
    int     c;

    assert(pattern);

    tokBuf = xp->tokBuf;
    assert(tokBuf);

    start = mprGetBufStart(tokBuf);
    while (1) {
        cp = start;
        for (p = pattern; *p; p++) {
            if (cp >= (char*) tokBuf->end) {
                if ((c = getNextChar(xp)) < 0) {
                    return 0;
                }
                if (mprPutCharToBuf(tokBuf, c) < 0) {
                    return -1;
                }
            }
            if (*cp++ != *p) {
                break;
            }
        }
        if (*p == '\0') {
            /*
                Remove the pattern from the tokBuf
             */
            mprAdjustBufEnd(tokBuf, - (int) slen(pattern));
            trimToken(xp);
            return 1;
        }
        start++;
    }
}


/*
    Get another character. We read and buffer blocks of data if we need more data to parse.
 */
static int getNextChar(MprXml *xp)
{
    MprBuf  *inBuf;
    ssize   l;
    int     c;

    inBuf = xp->inBuf;
    if (mprGetBufLength(inBuf) <= 0) {
        /*
            Flush to reset the servp/endp pointers to the start of the buffer so we can do a maximal read 
         */
        mprFlushBuf(inBuf);
        l = (xp->readFn)(xp, xp->inputArg, mprGetBufStart(inBuf), mprGetBufSpace(inBuf));
        if (l <= 0) {
            return -1;
        }
        mprAdjustBufEnd(inBuf, l);
    }
    c = mprGetCharFromBuf(inBuf);
    if (c == '\n') {
        xp->lineNumber++;
    }
    return c;
}


/*
    Put back a character in the input buffer
 */
static int putLastChar(MprXml *xp, int c)
{
    if (mprInsertCharToBuf(xp->inBuf, (char) c) < 0) {
        assert(0);
        return MPR_ERR_BAD_STATE;
    }
    if (c == '\n') {
        xp->lineNumber--;
    }
    return 0;
}


/*
    Output a parse message
 */ 
static void xmlError(MprXml *xp, char *fmt, ...)
{
    va_list     args;
    char        *buf;

    assert(fmt);

    va_start(args, fmt);
    buf = sfmtv(fmt, args);
    va_end(args);
    xp->errMsg = sfmt("XML error: %s\nAt line %d\n", buf, xp->lineNumber);
}


/*
    Remove trailing whitespace in a token and ensure it is terminated with a NULL for easy parsing
 */
static void trimToken(MprXml *xp)
{
    while (isspace((uchar) mprLookAtLastCharInBuf(xp->tokBuf))) {
        mprAdjustBufEnd(xp->tokBuf, -1);
    }
    mprAddNullToBuf(xp->tokBuf);
}


PUBLIC cchar *mprXmlGetErrorMsg(MprXml *xp)
{
    if (xp->errMsg == 0) {
        return "";
    }
    return xp->errMsg;
}


PUBLIC int mprXmlGetLineNumber(MprXml *xp)
{
    return xp->lineNumber;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
