/*
    ejs.h -- Embethis Ejscript Library Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: orion
 */

#include "bit.h"
#
#if BIT_PACK_EJSCRIPT

#include "bitos.h"
#include "mpr.h"
#include "http.h"
#include "ejs.slots.h"

/************************************************************************/
/*
    Start of file "src/paks/zlib/zlib.h"
 */
/************************************************************************/

/*
    zlib.h -- Zlib Library Library Header

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: orion
 */

#define local static
#define NO_DUMMY_DECL
#undef TIME
#include "bit.h"

/************************************************************************/
/*
    Start of file "src/zconf.h"
 */
/************************************************************************/

/* zconf.h -- configuration of the zlib compression library
 * Copyright (C) 1995-2011 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* @(#) $Id$ */

#ifndef ZCONF_H
#define ZCONF_H

#include "bit.h"
#include "bitos.h"

#if EMBEDTHIS || 1
#if _WIN32
    #define ZLIB_DLL
#endif
#endif

#ifndef ZLIB_INTERNAL
#if ((__GNUC__-0) * 10 + __GNUC_MINOR__-0 >= 33) && !defined(NO_VIZ)
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#  define ZLIB_INTERNAL
#endif
#endif

#if VXWORKS
    #ifndef _VSB_CONFIG_FILE
        #define _VSB_CONFIG_FILE "vsbConfig.h"
    #endif
#endif

/*
 * If you *really* need a unique prefix for all types and library functions,
 * compile with -DZ_PREFIX. The "standard" zlib should be compiled without it.
 * Even better than compiling with -DZ_PREFIX would be to use configure to set
 * this permanently in zconf.h using "./configure --zprefix".
 */
#ifdef Z_PREFIX     /* may be set to #if 1 by ./configure */
#  define Z_PREFIX_SET

/* all linked symbols */
#  define _dist_code            z__dist_code
#  define _length_code          z__length_code
#  define _tr_align             z__tr_align
#  define _tr_flush_block       z__tr_flush_block
#  define _tr_init              z__tr_init
#  define _tr_stored_block      z__tr_stored_block
#  define _tr_tally             z__tr_tally
#  define adler32               z_adler32
#  define adler32_combine       z_adler32_combine
#  define adler32_combine64     z_adler32_combine64
#  ifndef Z_SOLO
#    define compress              z_compress
#    define compress2             z_compress2
#    define compressBound         z_compressBound
#  endif
#  define crc32                 z_crc32
#  define crc32_combine         z_crc32_combine
#  define crc32_combine64       z_crc32_combine64
#  define deflate               z_deflate
#  define deflateBound          z_deflateBound
#  define deflateCopy           z_deflateCopy
#  define deflateEnd            z_deflateEnd
#  define deflateInit2_         z_deflateInit2_
#  define deflateInit_          z_deflateInit_
#  define deflateParams         z_deflateParams
#  define deflatePending        z_deflatePending
#  define deflatePrime          z_deflatePrime
#  define deflateReset          z_deflateReset
#  define deflateResetKeep      z_deflateResetKeep
#  define deflateSetDictionary  z_deflateSetDictionary
#  define deflateSetHeader      z_deflateSetHeader
#  define deflateTune           z_deflateTune
#  define deflate_copyright     z_deflate_copyright
#  define get_crc_table         z_get_crc_table
#  ifndef Z_SOLO
#    define gz_error              z_gz_error
#    define gz_intmax             z_gz_intmax
#    define gz_strwinerror        z_gz_strwinerror
#    define gzbuffer              z_gzbuffer
#    define gzclearerr            z_gzclearerr
#    define gzclose               z_gzclose
#    define gzclose_r             z_gzclose_r
#    define gzclose_w             z_gzclose_w
#    define gzdirect              z_gzdirect
#    define gzdopen               z_gzdopen
#    define gzeof                 z_gzeof
#    define gzerror               z_gzerror
#    define gzflags               z_gzflags
#    define gzflush               z_gzflush
#    define gzgetc                z_gzgetc
#    define gzgetc_               z_gzgetc_
#    define gzgets                z_gzgets
#    define gzoffset              z_gzoffset
#    define gzoffset64            z_gzoffset64
#    define gzopen                z_gzopen
#    define gzopen64              z_gzopen64
#    define gzprintf              z_gzprintf
#    define gzputc                z_gzputc
#    define gzputs                z_gzputs
#    define gzread                z_gzread
#    define gzrewind              z_gzrewind
#    define gzseek                z_gzseek
#    define gzseek64              z_gzseek64
#    define gzsetparams           z_gzsetparams
#    define gztell                z_gztell
#    define gztell64              z_gztell64
#    define gzungetc              z_gzungetc
#    define gzwrite               z_gzwrite
#  endif
#  define inflate               z_inflate
#  define inflateBack           z_inflateBack
#  define inflateBackEnd        z_inflateBackEnd
#  define inflateBackInit_      z_inflateBackInit_
#  define inflateCopy           z_inflateCopy
#  define inflateEnd            z_inflateEnd
#  define inflateGetHeader      z_inflateGetHeader
#  define inflateInit2_         z_inflateInit2_
#  define inflateInit_          z_inflateInit_
#  define inflateMark           z_inflateMark
#  define inflatePrime          z_inflatePrime
#  define inflateReset          z_inflateReset
#  define inflateReset2         z_inflateReset2
#  define inflateSetDictionary  z_inflateSetDictionary
#  define inflateSync           z_inflateSync
#  define inflateSyncPoint      z_inflateSyncPoint
#  define inflateUndermine      z_inflateUndermine
#  define inflateResetKeep      z_inflateResetKeep
#  define inflate_copyright     z_inflate_copyright
#  define inflate_fast          z_inflate_fast
#  define inflate_table         z_inflate_table
#  ifndef Z_SOLO
#    define uncompress            z_uncompress
#  endif
#  define zError                z_zError
#  ifndef Z_SOLO
#    define zcalloc               z_zcalloc
#    define zcfree                z_zcfree
#  endif
#  define zlibCompileFlags      z_zlibCompileFlags
#  define zlibVersion           z_zlibVersion

/* all zlib typedefs in zlib.h and zconf.h */
#  define Byte                  z_Byte
#  define Bytef                 z_Bytef
#  define alloc_func            z_alloc_func
#  define charf                 z_charf
#  define free_func             z_free_func
#  ifndef Z_SOLO
#    define gzFile                z_gzFile
#    define gz_header             z_gz_header
#    define gz_headerp            z_gz_headerp
#  endif
#  define in_func               z_in_func
#  define intf                  z_intf
#  define out_func              z_out_func
#  define uInt                  z_uInt
#  define uIntf                 z_uIntf
#  define uLong                 z_uLong
#  define uLongf                z_uLongf
#  define voidp                 z_voidp
#  define voidpc                z_voidpc
#  define voidpf                z_voidpf

/* all zlib structs in zlib.h and zconf.h */
#  ifndef Z_SOLO
#    define gz_header_s           z_gz_header_s
#  endif
#  define internal_state        z_internal_state

#endif

#if defined(__MSDOS__) && !defined(MSDOS)
#  define MSDOS
#endif
#if (defined(OS_2) || defined(__OS2__)) && !defined(OS2)
#  define OS2
#endif
#if defined(_WINDOWS) && !defined(WINDOWS)
#  define WINDOWS
#endif
#if defined(_WIN32) || defined(_WIN32_WCE) || defined(__WIN32__)
#  ifndef WIN32
#    define WIN32
#  endif
#endif
#if (defined(MSDOS) || defined(OS2) || defined(WINDOWS)) && !defined(WIN32)
#  if !defined(__GNUC__) && !defined(__FLAT__) && !defined(__386__)
#    ifndef SYS16BIT
#      define SYS16BIT
#    endif
#  endif
#endif

/*
 * Compile with -DMAXSEG_64K if the alloc function cannot allocate more
 * than 64k bytes at a time (needed on systems with 16-bit int).
 */
#ifdef SYS16BIT
#  define MAXSEG_64K
#endif
#ifdef MSDOS
#  define UNALIGNED_OK
#endif

#ifdef __STDC_VERSION__
#  ifndef STDC
#    define STDC
#  endif
#  if __STDC_VERSION__ >= 199901L
#    ifndef STDC99
#      define STDC99
#    endif
#  endif
#endif
#if !defined(STDC) && (defined(__STDC__) || defined(__cplusplus))
#  define STDC
#endif
#if !defined(STDC) && (defined(__GNUC__) || defined(__BORLANDC__))
#  define STDC
#endif
#if !defined(STDC) && (defined(MSDOS) || defined(WINDOWS) || defined(WIN32))
#  define STDC
#endif
#if !defined(STDC) && (defined(OS2) || defined(__HOS_AIX__))
#  define STDC
#endif

#if defined(__OS400__) && !defined(STDC)    /* iSeries (formerly AS/400). */
#  define STDC
#endif

#ifndef STDC
#  ifndef const /* cannot use !defined(STDC) && !defined(const) on Mac */
#    define const       /* note: need a more gentle solution here */
#  endif
#endif

#if defined(ZLIB_CONST) && !defined(z_const)
#  define z_const const
#else
#  define z_const
#endif

/* Some Mac compilers merge all .h files incorrectly: */
#if defined(__MWERKS__)||defined(applec)||defined(THINK_C)||defined(__SC__)
#  define NO_DUMMY_DECL
#endif

/* Maximum value for memLevel in deflateInit2 */
#ifndef MAX_MEM_LEVEL
#  ifdef MAXSEG_64K
#    define MAX_MEM_LEVEL 8
#  else
#    define MAX_MEM_LEVEL 9
#  endif
#endif

/* Maximum value for windowBits in deflateInit2 and inflateInit2.
 * WARNING: reducing MAX_WBITS makes minigzip unable to extract .gz files
 * created by gzip. (Files created by minigzip can still be extracted by
 * gzip.)
 */
#ifndef MAX_WBITS
#  define MAX_WBITS   15 /* 32K LZ77 window */
#endif

/* The memory requirements for deflate are (in bytes):
            (1 << (windowBits+2)) +  (1 << (memLevel+9))
 that is: 128K for windowBits=15  +  128K for memLevel = 8  (default values)
 plus a few kilobytes for small objects. For example, if you want to reduce
 the default memory requirements from 256K to 128K, compile with
     make CFLAGS="-O -DMAX_WBITS=14 -DMAX_MEM_LEVEL=7"
 Of course this will generally degrade compression (there's no free lunch).

   The memory requirements for inflate are (in bytes) 1 << windowBits
 that is, 32K for windowBits=15 (default value) plus a few kilobytes
 for small objects.
*/

                        /* Type declarations */

#ifndef OF /* function prototypes */
#  ifdef STDC
#    define OF(args)  args
#  else
#    define OF(args)  ()
#  endif
#endif

#ifndef Z_ARG /* function prototypes for stdarg */
#  if defined(STDC) || defined(Z_HAVE_STDARG_H)
#    define Z_ARG(args)  args
#  else
#    define Z_ARG(args)  ()
#  endif
#endif

/* The following definitions for FAR are needed only for MSDOS mixed
 * model programming (small or medium model with some far allocations).
 * This was tested only with MSC; for other MSDOS compilers you may have
 * to define NO_MEMCPY in zutil.h.  If you don't need the mixed model,
 * just define FAR to be empty.
 */
#ifdef SYS16BIT
#  if defined(M_I86SM) || defined(M_I86MM)
     /* MSC small or medium model */
#    define SMALL_MEDIUM
#    ifdef _MSC_VER
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#  if (defined(__SMALL__) || defined(__MEDIUM__))
     /* Turbo C small or medium model */
#    define SMALL_MEDIUM
#    ifdef __BORLANDC__
#      define FAR _far
#    else
#      define FAR far
#    endif
#  endif
#endif

#if defined(WINDOWS) || defined(WIN32)
   /* If building or using zlib as a DLL, define ZLIB_DLL.
    * This is not mandatory, but it offers a little performance increase.
    */
#  ifdef ZLIB_DLL
#    if defined(WIN32) && (!defined(__BORLANDC__) || (__BORLANDC__ >= 0x500))
#      ifdef ZLIB_INTERNAL
#        define ZEXTERN extern __declspec(dllexport)
#      else
#        define ZEXTERN extern __declspec(dllimport)
#      endif
#    endif
#  endif  /* ZLIB_DLL */
   /* If building or using zlib with the WINAPI/WINAPIV calling convention,
    * define ZLIB_WINAPI.
    * Caution: the standard ZLIB1.DLL is NOT compiled using ZLIB_WINAPI.
    */
#  ifdef ZLIB_WINAPI
#    ifdef FAR
#      undef FAR
#    endif
#    include <windows.h>
     /* No need for _export, use ZLIB.DEF instead. */
     /* For complete Windows compatibility, use WINAPI, not __stdcall. */
#    define ZEXPORT WINAPI
#    ifdef WIN32
#      define ZEXPORTVA WINAPIV
#    else
#      define ZEXPORTVA FAR CDECL
#    endif
#  endif
#endif

#if defined (__BEOS__)
#  ifdef ZLIB_DLL
#    ifdef ZLIB_INTERNAL
#      define ZEXPORT   __declspec(dllexport)
#      define ZEXPORTVA __declspec(dllexport)
#    else
#      define ZEXPORT   __declspec(dllimport)
#      define ZEXPORTVA __declspec(dllimport)
#    endif
#  endif
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif
#ifndef ZEXPORT
#  define ZEXPORT
#endif
#ifndef ZEXPORTVA
#  define ZEXPORTVA
#endif

#ifndef FAR
#  define FAR
#endif

#if !defined(__MACTYPES__)
typedef unsigned char  Byte;  /* 8 bits */
#endif
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

#ifdef SMALL_MEDIUM
   /* Borland C/C++ and some old MSC versions ignore FAR inside typedef */
#  define Bytef Byte FAR
#else
   typedef Byte  FAR Bytef;
#endif
typedef char  FAR charf;
typedef int   FAR intf;
typedef uInt  FAR uIntf;
typedef uLong FAR uLongf;

#ifdef STDC
   typedef void const *voidpc;
   typedef void FAR   *voidpf;
   typedef void       *voidp;
#else
   typedef Byte const *voidpc;
   typedef Byte FAR   *voidpf;
   typedef Byte       *voidp;
#endif

#if !_WIN32    /* was set to #if 1 by ./configure */
#  define Z_HAVE_UNISTD_H
#endif

#if !_WIN32    /* was set to #if 1 by ./configure */
#  define Z_HAVE_STDARG_H
#endif

#ifdef STDC
#  ifndef Z_SOLO
#    include <sys/types.h>      /* for off_t */
#  endif
#endif

/* a little trick to accommodate both "#define _LARGEFILE64_SOURCE" and
 * "#define _LARGEFILE64_SOURCE 1" as requesting 64-bit operations, (even
 * though the former does not conform to the LFS document), but considering
 * both "#undef _LARGEFILE64_SOURCE" and "#define _LARGEFILE64_SOURCE 0" as
 * equivalently requesting no 64-bit operations
 */
#if -_LARGEFILE64_SOURCE - -1 == 1
#  undef _LARGEFILE64_SOURCE
#endif

#if defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
#  define Z_LARGE
#endif

#if (defined(Z_HAVE_UNISTD_H) || defined(Z_LARGE)) && !defined(Z_SOLO)
#  include <unistd.h>       /* for SEEK_* and off_t */
#  ifdef VMS
#    include <unixio.h>     /* for off_t */
#  endif
#  ifndef z_off_t
#    define z_off_t off_t
#  endif
#endif

#if !defined(SEEK_SET) && !defined(Z_SOLO)
#  define SEEK_SET        0       /* Seek from beginning of file.  */
#  define SEEK_CUR        1       /* Seek from current position.  */
#  define SEEK_END        2       /* Set file pointer to EOF plus "offset" */
#endif

#ifndef z_off_t
#  define z_off_t long
#endif

#if !defined(_WIN32) && (defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0)
#  define z_off64_t off64_t
#else
#  if defined(_WIN32)
#    define z_off64_t __int64
#  else
#  define z_off64_t z_off_t
#endif
#endif

/* MVS linker does not support external names larger than 8 bytes */
#if defined(__MVS__)
  #pragma map(deflateInit_,"DEIN")
  #pragma map(deflateInit2_,"DEIN2")
  #pragma map(deflateEnd,"DEEND")
  #pragma map(deflateBound,"DEBND")
  #pragma map(inflateInit_,"ININ")
  #pragma map(inflateInit2_,"ININ2")
  #pragma map(inflateEnd,"INEND")
  #pragma map(inflateSync,"INSY")
  #pragma map(inflateSetDictionary,"INSEDI")
  #pragma map(compressBound,"CMBND")
  #pragma map(inflate_table,"INTABL")
  #pragma map(inflate_fast,"INFA")
  #pragma map(inflate_copyright,"INCOPY")
#endif

#endif /* ZCONF_H */

/************************************************************************/
/*
    Start of file "src/zlib.h"
 */
/************************************************************************/

/* zlib.h -- interface of the 'zlib' general purpose compression library
  version 1.2.6, January 29th, 2012

  Copyright (C) 1995-2012 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
*/

#ifndef ZLIB_H
#define ZLIB_H



#ifdef __cplusplus
extern "C" {
#endif

#define ZLIB_VERSION "1.2.6"
#define ZLIB_VERNUM 0x1260
#define ZLIB_VER_MAJOR 1
#define ZLIB_VER_MINOR 2
#define ZLIB_VER_REVISION 6
#define ZLIB_VER_SUBREVISION 0

/*
    The 'zlib' compression library provides in-memory compression and
  decompression functions, including integrity checks of the uncompressed data.
  This version of the library supports only one compression method (deflation)
  but other algorithms will be added later and will have the same stream
  interface.

    Compression can be done in a single step if the buffers are large enough,
  or can be done by repeated calls of the compression function.  In the latter
  case, the application must provide more input and/or consume the output
  (providing more output space) before each call.

    The compressed data format used by default by the in-memory functions is
  the zlib format, which is a zlib wrapper documented in RFC 1950, wrapped
  around a deflate stream, which is itself documented in RFC 1951.

    The library also supports reading and writing files in gzip (.gz) format
  with an interface similar to that of stdio using the functions that start
  with "gz".  The gzip format is different from the zlib format.  gzip is a
  gzip wrapper, documented in RFC 1952, wrapped around a deflate stream.

    This library can optionally read and write gzip streams in memory as well.

    The zlib format was designed to be compact and fast for use in memory
  and on communications channels.  The gzip format was designed for single-
  file compression on file systems, has a larger header than zlib to maintain
  directory information, and uses a different, slower check method than zlib.

    The library does not install any signal handler.  The decoder checks
  the consistency of the compressed data, so the library should never crash
  even in case of corrupted input.
*/

typedef voidpf (*alloc_func) OF((voidpf opaque, uInt items, uInt size));
typedef void   (*free_func)  OF((voidpf opaque, voidpf address));

struct internal_state;

typedef struct z_stream_s {
    z_const Bytef *next_in;     /* next input byte */
    uInt     avail_in;  /* number of bytes available at next_in */
    uLong    total_in;  /* total number of input bytes read so far */

    Bytef    *next_out; /* next output byte should be put there */
    uInt     avail_out; /* remaining free space at next_out */
    uLong    total_out; /* total number of bytes output so far */

    z_const char *msg;  /* last error message, NULL if no error */
    struct internal_state FAR *state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: binary or text */
    uLong   adler;      /* adler32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} z_stream;

typedef z_stream FAR *z_streamp;

/*
     gzip header information passed to and from zlib routines.  See RFC 1952
  for more details on the meanings of these fields.
*/
typedef struct gz_header_s {
    int     text;       /* true if compressed data believed to be text */
    uLong   time;       /* modification time */
    int     xflags;     /* extra flags (not used when writing a gzip file) */
    int     os;         /* operating system */
    Bytef   *extra;     /* pointer to extra field or Z_NULL if none */
    uInt    extra_len;  /* extra field length (valid if extra != Z_NULL) */
    uInt    extra_max;  /* space at extra (only when reading header) */
    Bytef   *name;      /* pointer to zero-terminated file name or Z_NULL */
    uInt    name_max;   /* space at name (only when reading header) */
    Bytef   *comment;   /* pointer to zero-terminated comment or Z_NULL */
    uInt    comm_max;   /* space at comment (only when reading header) */
    int     hcrc;       /* true if there was or will be a header crc */
    int     done;       /* true when done reading gzip header (not used
                           when writing a gzip file) */
} gz_header;

typedef gz_header FAR *gz_headerp;

/*
     The application must update next_in and avail_in when avail_in has dropped
   to zero.  It must update next_out and avail_out when avail_out has dropped
   to zero.  The application must initialize zalloc, zfree and opaque before
   calling the init function.  All other fields are set by the compression
   library and must not be updated by the application.

     The opaque value provided by the application will be passed as the first
   parameter for calls of zalloc and zfree.  This can be useful for custom
   memory management.  The compression library attaches no meaning to the
   opaque value.

     zalloc must return Z_NULL if there is not enough memory for the object.
   If zlib is used in a multi-threaded application, zalloc and zfree must be
   thread safe.

     On 16-bit systems, the functions zalloc and zfree must be able to allocate
   exactly 65536 bytes, but will not be required to allocate more than this if
   the symbol MAXSEG_64K is defined (see zconf.h).  WARNING: On MSDOS, pointers
   returned by zalloc for objects of exactly 65536 bytes *must* have their
   offset normalized to zero.  The default allocation function provided by this
   library ensures this (see zutil.c).  To reduce memory requirements and avoid
   any allocation of 64K objects, at the expense of compression ratio, compile
   the library with -DMAX_WBITS=14 (see zconf.h).

     The fields total_in and total_out can be used for statistics or progress
   reports.  After compression, total_in holds the total size of the
   uncompressed data and may be saved for use in the decompressor (particularly
   if the decompressor wants to decompress everything in a single step).
*/

                        /* constants */

#define Z_NO_FLUSH      0
#define Z_PARTIAL_FLUSH 1
#define Z_SYNC_FLUSH    2
#define Z_FULL_FLUSH    3
#define Z_FINISH        4
#define Z_BLOCK         5
#define Z_TREES         6
/* Allowed flush values; see deflate() and inflate() below for details */

#define Z_OK            0
#define Z_STREAM_END    1
#define Z_NEED_DICT     2
#define Z_ERRNO        (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR   (-3)
#define Z_MEM_ERROR    (-4)
#define Z_BUF_ERROR    (-5)
#define Z_VERSION_ERROR (-6)
/* Return codes for the compression/decompression functions. Negative values
 * are errors, positive values are used for special but normal events.
 */

#define Z_NO_COMPRESSION         0
#define Z_BEST_SPEED             1
#define Z_BEST_COMPRESSION       9
#define Z_DEFAULT_COMPRESSION  (-1)
/* compression levels */

#define Z_FILTERED            1
#define Z_HUFFMAN_ONLY        2
#define Z_RLE                 3
#define Z_FIXED               4
#define Z_DEFAULT_STRATEGY    0
/* compression strategy; see deflateInit2() below for details */

#define Z_BINARY   0
#define Z_TEXT     1
#define Z_ASCII    Z_TEXT   /* for compatibility with 1.2.2 and earlier */
#define Z_UNKNOWN  2
/* Possible values of the data_type field (though see inflate()) */

#define Z_DEFLATED   8
/* The deflate compression method (the only one supported in this version) */

#define Z_NULL  0  /* for initializing zalloc, zfree, opaque */

#define zlib_version zlibVersion()
/* for compatibility with versions < 1.0.2 */


                        /* basic functions */

ZEXTERN const char * ZEXPORT zlibVersion OF((void));
/* The application can compare zlibVersion and ZLIB_VERSION for consistency.
   If the first character differs, the library code actually used is not
   compatible with the zlib.h header file used by the application.  This check
   is automatically made by deflateInit and inflateInit.
 */

/*
ZEXTERN int ZEXPORT deflateInit OF((z_streamp strm, int level));

     Initializes the internal stream state for compression.  The fields
   zalloc, zfree and opaque must be initialized before by the caller.  If
   zalloc and zfree are set to Z_NULL, deflateInit updates them to use default
   allocation functions.

     The compression level must be Z_DEFAULT_COMPRESSION, or between 0 and 9:
   1 gives best speed, 9 gives best compression, 0 gives no compression at all
   (the input data is simply copied a block at a time).  Z_DEFAULT_COMPRESSION
   requests a default compromise between speed and compression (currently
   equivalent to level 6).

     deflateInit returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_STREAM_ERROR if level is not a valid compression level, or
   Z_VERSION_ERROR if the zlib library version (zlib_version) is incompatible
   with the version assumed by the caller (ZLIB_VERSION).  msg is set to null
   if there is no error message.  deflateInit does not perform any compression:
   this will be done by deflate().
*/


ZEXTERN int ZEXPORT deflate OF((z_streamp strm, int flush));
/*
    deflate compresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full.  It may introduce
  some output latency (reading input without producing any output) except when
  forced to flush.

    The detailed semantics are as follows.  deflate performs one or both of the
  following actions:

  - Compress more input starting at next_in and update next_in and avail_in
    accordingly.  If not all input can be processed (because there is not
    enough room in the output buffer), next_in and avail_in are updated and
    processing will resume at this point for the next call of deflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly.  This action is forced if the parameter flush is non zero.
    Forcing flush frequently degrades the compression ratio, so this parameter
    should be set only when necessary (in interactive applications).  Some
    output may be provided even if flush is not set.

    Before the call of deflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming more
  output, and updating avail_in or avail_out accordingly; avail_out should
  never be zero before the call.  The application can consume the compressed
  output when it wants, for example when the output buffer is full (avail_out
  == 0), or after each call of deflate().  If deflate returns Z_OK and with
  zero avail_out, it must be called again after making room in the output
  buffer because there might be more output pending.

    Normally the parameter flush is set to Z_NO_FLUSH, which allows deflate to
  decide how much data to accumulate before producing output, in order to
  maximize compression.

    If the parameter flush is set to Z_SYNC_FLUSH, all pending output is
  flushed to the output buffer and the output is aligned on a byte boundary, so
  that the decompressor can get all input data available so far.  (In
  particular avail_in is zero after the call if enough output space has been
  provided before the call.) Flushing may degrade compression for some
  compression algorithms and so it should be used only when necessary.  This
  completes the current deflate block and follows it with an empty stored block
  that is three bits plus filler bits to the next byte, followed by four bytes
  (00 00 ff ff).

    If flush is set to Z_PARTIAL_FLUSH, all pending output is flushed to the
  output buffer, but the output is not aligned to a byte boundary.  All of the
  input data so far will be available to the decompressor, as for Z_SYNC_FLUSH.
  This completes the current deflate block and follows it with an empty fixed
  codes block that is 10 bits long.  This assures that enough bytes are output
  in order for the decompressor to finish the block before the empty fixed code
  block.

    If flush is set to Z_BLOCK, a deflate block is completed and emitted, as
  for Z_SYNC_FLUSH, but the output is not aligned on a byte boundary, and up to
  seven bits of the current block are held to be written as the next byte after
  the next deflate block is completed.  In this case, the decompressor may not
  be provided enough bits at this point in order to complete decompression of
  the data provided so far to the compressor.  It may need to wait for the next
  block to be emitted.  This is for advanced applications that need to control
  the emission of deflate blocks.

    If flush is set to Z_FULL_FLUSH, all output is flushed as with
  Z_SYNC_FLUSH, and the compression state is reset so that decompression can
  restart from this point if previous compressed data has been damaged or if
  random access is desired.  Using Z_FULL_FLUSH too often can seriously degrade
  compression.

    If deflate returns with avail_out == 0, this function must be called again
  with the same value of the flush parameter and more output space (updated
  avail_out), until the flush is complete (deflate returns with non-zero
  avail_out).  In the case of a Z_FULL_FLUSH or Z_SYNC_FLUSH, make sure that
  avail_out is greater than six to avoid repeated flush markers due to
  avail_out == 0 on return.

    If the parameter flush is set to Z_FINISH, pending input is processed,
  pending output is flushed and deflate returns with Z_STREAM_END if there was
  enough output space; if deflate returns with Z_OK, this function must be
  called again with Z_FINISH and more output space (updated avail_out) but no
  more input data, until it returns with Z_STREAM_END or an error.  After
  deflate has returned Z_STREAM_END, the only possible operations on the stream
  are deflateReset or deflateEnd.

    Z_FINISH can be used immediately after deflateInit if all the compression
  is to be done in a single step.  In this case, avail_out must be at least the
  value returned by deflateBound (see below).  Then deflate is guaranteed to
  return Z_STREAM_END.  If not enough output space is provided, deflate will
  not return Z_STREAM_END, and it must be called again as described above.

    deflate() sets strm->adler to the adler32 checksum of all input read
  so far (that is, total_in bytes).

    deflate() may update strm->data_type if it can make a good guess about
  the input data type (Z_BINARY or Z_TEXT).  In doubt, the data is considered
  binary.  This field is only for information purposes and does not affect the
  compression algorithm in any manner.

    deflate() returns Z_OK if some progress has been made (more input
  processed or more output produced), Z_STREAM_END if all input has been
  consumed and all output has been produced (only when flush is set to
  Z_FINISH), Z_STREAM_ERROR if the stream state was inconsistent (for example
  if next_in or next_out was Z_NULL), Z_BUF_ERROR if no progress is possible
  (for example avail_in or avail_out was zero).  Note that Z_BUF_ERROR is not
  fatal, and deflate() can be called again with more input and more output
  space to continue compressing.
*/


ZEXTERN int ZEXPORT deflateEnd OF((z_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any pending
   output.

     deflateEnd returns Z_OK if success, Z_STREAM_ERROR if the
   stream state was inconsistent, Z_DATA_ERROR if the stream was freed
   prematurely (some input or output was discarded).  In the error case, msg
   may be set but then points to a static string (which must not be
   deallocated).
*/


/*
ZEXTERN int ZEXPORT inflateInit OF((z_streamp strm));

     Initializes the internal stream state for decompression.  The fields
   next_in, avail_in, zalloc, zfree and opaque must be initialized before by
   the caller.  If next_in is not Z_NULL and avail_in is large enough (the
   exact value depends on the compression method), inflateInit determines the
   compression method from the zlib header and allocates all data structures
   accordingly; otherwise the allocation will be deferred to the first call of
   inflate.  If zalloc and zfree are set to Z_NULL, inflateInit updates them to
   use default allocation functions.

     inflateInit returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_VERSION_ERROR if the zlib library version is incompatible with the
   version assumed by the caller, or Z_STREAM_ERROR if the parameters are
   invalid, such as a null pointer to the structure.  msg is set to null if
   there is no error message.  inflateInit does not perform any decompression
   apart from possibly reading the zlib header if present: actual decompression
   will be done by inflate().  (So next_in and avail_in may be modified, but
   next_out and avail_out are unused and unchanged.) The current implementation
   of inflateInit() does not process any header information -- that is deferred
   until inflate() is called.
*/


ZEXTERN int ZEXPORT inflate OF((z_streamp strm, int flush));
/*
    inflate decompresses as much data as possible, and stops when the input
  buffer becomes empty or the output buffer becomes full.  It may introduce
  some output latency (reading input without producing any output) except when
  forced to flush.

  The detailed semantics are as follows.  inflate performs one or both of the
  following actions:

  - Decompress more input starting at next_in and update next_in and avail_in
    accordingly.  If not all input can be processed (because there is not
    enough room in the output buffer), next_in is updated and processing will
    resume at this point for the next call of inflate().

  - Provide more output starting at next_out and update next_out and avail_out
    accordingly.  inflate() provides as much output as possible, until there is
    no more input data or no more space in the output buffer (see below about
    the flush parameter).

    Before the call of inflate(), the application should ensure that at least
  one of the actions is possible, by providing more input and/or consuming more
  output, and updating the next_* and avail_* values accordingly.  The
  application can consume the uncompressed output when it wants, for example
  when the output buffer is full (avail_out == 0), or after each call of
  inflate().  If inflate returns Z_OK and with zero avail_out, it must be
  called again after making room in the output buffer because there might be
  more output pending.

    The flush parameter of inflate() can be Z_NO_FLUSH, Z_SYNC_FLUSH, Z_FINISH,
  Z_BLOCK, or Z_TREES.  Z_SYNC_FLUSH requests that inflate() flush as much
  output as possible to the output buffer.  Z_BLOCK requests that inflate()
  stop if and when it gets to the next deflate block boundary.  When decoding
  the zlib or gzip format, this will cause inflate() to return immediately
  after the header and before the first block.  When doing a raw inflate,
  inflate() will go ahead and process the first block, and will return when it
  gets to the end of that block, or when it runs out of data.

    The Z_BLOCK option assists in appending to or combining deflate streams.
  Also to assist in this, on return inflate() will set strm->data_type to the
  number of unused bits in the last byte taken from strm->next_in, plus 64 if
  inflate() is currently decoding the last block in the deflate stream, plus
  128 if inflate() returned immediately after decoding an end-of-block code or
  decoding the complete header up to just before the first byte of the deflate
  stream.  The end-of-block will not be indicated until all of the uncompressed
  data from that block has been written to strm->next_out.  The number of
  unused bits may in general be greater than seven, except when bit 7 of
  data_type is set, in which case the number of unused bits will be less than
  eight.  data_type is set as noted here every time inflate() returns for all
  flush options, and so can be used to determine the amount of currently
  consumed input in bits.

    The Z_TREES option behaves as Z_BLOCK does, but it also returns when the
  end of each deflate block header is reached, before any actual data in that
  block is decoded.  This allows the caller to determine the length of the
  deflate block header for later use in random access within a deflate block.
  256 is added to the value of strm->data_type when inflate() returns
  immediately after reaching the end of the deflate block header.

    inflate() should normally be called until it returns Z_STREAM_END or an
  error.  However if all decompression is to be performed in a single step (a
  single call of inflate), the parameter flush should be set to Z_FINISH.  In
  this case all pending input is processed and all pending output is flushed;
  avail_out must be large enough to hold all the uncompressed data.  (The size
  of the uncompressed data may have been saved by the compressor for this
  purpose.) The next operation on this stream must be inflateEnd to deallocate
  the decompression state.  The use of Z_FINISH is not required to perform an
  inflation in one step.  However it may be used to inform inflate that a
  faster approach can be used for the single inflate() call.  Z_FINISH also
  informs inflate to not maintain a sliding window if the stream completes,
  which reduces inflate's memory footprint.

     In this implementation, inflate() always flushes as much output as
  possible to the output buffer, and always uses the faster approach on the
  first call.  So the effects of the flush parameter in this implementation are
  on the return value of inflate() as noted below, when inflate() returns early
  when Z_BLOCK or Z_TREES is used, and when inflate() avoids the allocation of
  memory for a sliding window when Z_FINISH is used.

     If a preset dictionary is needed after this call (see inflateSetDictionary
  below), inflate sets strm->adler to the Adler-32 checksum of the dictionary
  chosen by the compressor and returns Z_NEED_DICT; otherwise it sets
  strm->adler to the Adler-32 checksum of all output produced so far (that is,
  total_out bytes) and returns Z_OK, Z_STREAM_END or an error code as described
  below.  At the end of the stream, inflate() checks that its computed adler32
  checksum is equal to that saved by the compressor and returns Z_STREAM_END
  only if the checksum is correct.

    inflate() can decompress and check either zlib-wrapped or gzip-wrapped
  deflate data.  The header type is detected automatically, if requested when
  initializing with inflateInit2().  Any information contained in the gzip
  header is not retained, so applications that need that information should
  instead use raw inflate, see inflateInit2() below, or inflateBack() and
  perform their own processing of the gzip header and trailer.  When processing
  gzip-wrapped deflate data, strm->adler32 is set to the CRC-32 of the output
  producted so far.  The CRC-32 is checked against the gzip trailer.

    inflate() returns Z_OK if some progress has been made (more input processed
  or more output produced), Z_STREAM_END if the end of the compressed data has
  been reached and all uncompressed output has been produced, Z_NEED_DICT if a
  preset dictionary is needed at this point, Z_DATA_ERROR if the input data was
  corrupted (input stream not conforming to the zlib format or incorrect check
  value), Z_STREAM_ERROR if the stream structure was inconsistent (for example
  next_in or next_out was Z_NULL), Z_MEM_ERROR if there was not enough memory,
  Z_BUF_ERROR if no progress is possible or if there was not enough room in the
  output buffer when Z_FINISH is used.  Note that Z_BUF_ERROR is not fatal, and
  inflate() can be called again with more input and more output space to
  continue decompressing.  If Z_DATA_ERROR is returned, the application may
  then call inflateSync() to look for a good compression block if a partial
  recovery of the data is desired.
*/


ZEXTERN int ZEXPORT inflateEnd OF((z_streamp strm));
/*
     All dynamically allocated data structures for this stream are freed.
   This function discards any unprocessed input and does not flush any pending
   output.

     inflateEnd returns Z_OK if success, Z_STREAM_ERROR if the stream state
   was inconsistent.  In the error case, msg may be set but then points to a
   static string (which must not be deallocated).
*/


                        /* Advanced functions */

/*
    The following functions are needed only in some special applications.
*/

/*
ZEXTERN int ZEXPORT deflateInit2 OF((z_streamp strm,
                                     int  level,
                                     int  method,
                                     int  windowBits,
                                     int  memLevel,
                                     int  strategy));

     This is another version of deflateInit with more compression options.  The
   fields next_in, zalloc, zfree and opaque must be initialized before by the
   caller.

     The method parameter is the compression method.  It must be Z_DEFLATED in
   this version of the library.

     The windowBits parameter is the base two logarithm of the window size
   (the size of the history buffer).  It should be in the range 8..15 for this
   version of the library.  Larger values of this parameter result in better
   compression at the expense of memory usage.  The default value is 15 if
   deflateInit is used instead.

     windowBits can also be -8..-15 for raw deflate.  In this case, -windowBits
   determines the window size.  deflate() will then generate raw deflate data
   with no zlib header or trailer, and will not compute an adler32 check value.

     windowBits can also be greater than 15 for optional gzip encoding.  Add
   16 to windowBits to write a simple gzip header and trailer around the
   compressed data instead of a zlib wrapper.  The gzip header will have no
   file name, no extra data, no comment, no modification time (set to zero), no
   header crc, and the operating system will be set to 255 (unknown).  If a
   gzip stream is being written, strm->adler is a crc32 instead of an adler32.

     The memLevel parameter specifies how much memory should be allocated
   for the internal compression state.  memLevel=1 uses minimum memory but is
   slow and reduces compression ratio; memLevel=9 uses maximum memory for
   optimal speed.  The default value is 8.  See zconf.h for total memory usage
   as a function of windowBits and memLevel.

     The strategy parameter is used to tune the compression algorithm.  Use the
   value Z_DEFAULT_STRATEGY for normal data, Z_FILTERED for data produced by a
   filter (or predictor), Z_HUFFMAN_ONLY to force Huffman encoding only (no
   string match), or Z_RLE to limit match distances to one (run-length
   encoding).  Filtered data consists mostly of small values with a somewhat
   random distribution.  In this case, the compression algorithm is tuned to
   compress them better.  The effect of Z_FILTERED is to force more Huffman
   coding and less string matching; it is somewhat intermediate between
   Z_DEFAULT_STRATEGY and Z_HUFFMAN_ONLY.  Z_RLE is designed to be almost as
   fast as Z_HUFFMAN_ONLY, but give better compression for PNG image data.  The
   strategy parameter only affects the compression ratio but not the
   correctness of the compressed output even if it is not set appropriately.
   Z_FIXED prevents the use of dynamic Huffman codes, allowing for a simpler
   decoder for special applications.

     deflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_STREAM_ERROR if any parameter is invalid (such as an invalid
   method), or Z_VERSION_ERROR if the zlib library version (zlib_version) is
   incompatible with the version assumed by the caller (ZLIB_VERSION).  msg is
   set to null if there is no error message.  deflateInit2 does not perform any
   compression: this will be done by deflate().
*/

ZEXTERN int ZEXPORT deflateSetDictionary OF((z_streamp strm,
                                             const Bytef *dictionary,
                                             uInt  dictLength));
/*
     Initializes the compression dictionary from the given byte sequence
   without producing any compressed output.  When using the zlib format, this
   function must be called immediately after deflateInit, deflateInit2 or
   deflateReset, and before any call of deflate.  When doing raw deflate, this
   function must be called either before any call of deflate, or immediately
   after the completion of a deflate block, i.e. after all input has been
   consumed and all output has been delivered when using any of the flush
   options Z_BLOCK, Z_PARTIAL_FLUSH, Z_SYNC_FLUSH, or Z_FULL_FLUSH.  The
   compressor and decompressor must use exactly the same dictionary (see
   inflateSetDictionary).

     The dictionary should consist of strings (byte sequences) that are likely
   to be encountered later in the data to be compressed, with the most commonly
   used strings preferably put towards the end of the dictionary.  Using a
   dictionary is most useful when the data to be compressed is short and can be
   predicted with good accuracy; the data can then be compressed better than
   with the default empty dictionary.

     Depending on the size of the compression data structures selected by
   deflateInit or deflateInit2, a part of the dictionary may in effect be
   discarded, for example if the dictionary is larger than the window size
   provided in deflateInit or deflateInit2.  Thus the strings most likely to be
   useful should be put at the end of the dictionary, not at the front.  In
   addition, the current implementation of deflate will use at most the window
   size minus 262 bytes of the provided dictionary.

     Upon return of this function, strm->adler is set to the adler32 value
   of the dictionary; the decompressor may later use this value to determine
   which dictionary has been used by the compressor.  (The adler32 value
   applies to the whole dictionary even if only a subset of the dictionary is
   actually used by the compressor.) If a raw deflate was requested, then the
   adler32 value is not computed and strm->adler is not set.

     deflateSetDictionary returns Z_OK if success, or Z_STREAM_ERROR if a
   parameter is invalid (e.g.  dictionary being Z_NULL) or the stream state is
   inconsistent (for example if deflate has already been called for this stream
   or if not at a block boundary for raw deflate).  deflateSetDictionary does
   not perform any compression: this will be done by deflate().
*/

ZEXTERN int ZEXPORT deflateCopy OF((z_streamp dest,
                                    z_streamp source));
/*
     Sets the destination stream as a complete copy of the source stream.

     This function can be useful when several compression strategies will be
   tried, for example when there are several ways of pre-processing the input
   data with a filter.  The streams that will be discarded should then be freed
   by calling deflateEnd.  Note that deflateCopy duplicates the internal
   compression state which can be quite large, so this strategy is slow and can
   consume lots of memory.

     deflateCopy returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if the source stream state was inconsistent
   (such as zalloc being Z_NULL).  msg is left unchanged in both source and
   destination.
*/

ZEXTERN int ZEXPORT deflateReset OF((z_streamp strm));
/*
     This function is equivalent to deflateEnd followed by deflateInit,
   but does not free and reallocate all the internal compression state.  The
   stream will keep the same compression level and any other attributes that
   may have been set by deflateInit2.

     deflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being Z_NULL).
*/

ZEXTERN int ZEXPORT deflateParams OF((z_streamp strm,
                                      int level,
                                      int strategy));
/*
     Dynamically update the compression level and compression strategy.  The
   interpretation of level and strategy is as in deflateInit2.  This can be
   used to switch between compression and straight copy of the input data, or
   to switch to a different kind of input data requiring a different strategy.
   If the compression level is changed, the input available so far is
   compressed with the old level (and may be flushed); the new level will take
   effect only at the next call of deflate().

     Before the call of deflateParams, the stream state must be set as for
   a call of deflate(), since the currently available input may have to be
   compressed and flushed.  In particular, strm->avail_out must be non-zero.

     deflateParams returns Z_OK if success, Z_STREAM_ERROR if the source
   stream state was inconsistent or if a parameter was invalid, Z_BUF_ERROR if
   strm->avail_out was zero.
*/

ZEXTERN int ZEXPORT deflateTune OF((z_streamp strm,
                                    int good_length,
                                    int max_lazy,
                                    int nice_length,
                                    int max_chain));
/*
     Fine tune deflate's internal compression parameters.  This should only be
   used by someone who understands the algorithm used by zlib's deflate for
   searching for the best matching string, and even then only by the most
   fanatic optimizer trying to squeeze out the last compressed bit for their
   specific input data.  Read the deflate.c source code for the meaning of the
   max_lazy, good_length, nice_length, and max_chain parameters.

     deflateTune() can be called after deflateInit() or deflateInit2(), and
   returns Z_OK on success, or Z_STREAM_ERROR for an invalid deflate stream.
 */

ZEXTERN uLong ZEXPORT deflateBound OF((z_streamp strm,
                                       uLong sourceLen));
/*
     deflateBound() returns an upper bound on the compressed size after
   deflation of sourceLen bytes.  It must be called after deflateInit() or
   deflateInit2(), and after deflateSetHeader(), if used.  This would be used
   to allocate an output buffer for deflation in a single pass, and so would be
   called before deflate().  If that first deflate() call is provided the
   sourceLen input bytes, an output buffer allocated to the size returned by
   deflateBound(), and the flush value Z_FINISH, then deflate() is guaranteed
   to return Z_STREAM_END.  Note that it is possible for the compressed size to
   be larger than the value returned by deflateBound() if flush options other
   than Z_FINISH or Z_NO_FLUSH are used.
*/

ZEXTERN int ZEXPORT deflatePending OF((z_streamp strm,
                                       unsigned *pending,
                                       int *bits));
/*
     deflatePending() returns the number of bytes and bits of output that have
   been generated, but not yet provided in the available output.  The bytes not
   provided would be due to the available output space having being consumed.
   The number of bits of output not provided are between 0 and 7, where they
   await more bits to join them in order to fill out a full byte.  If pending
   or bits are Z_NULL, then those values are not set.

     deflatePending returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.
 */

ZEXTERN int ZEXPORT deflatePrime OF((z_streamp strm,
                                     int bits,
                                     int value));
/*
     deflatePrime() inserts bits in the deflate output stream.  The intent
   is that this function is used to start off the deflate output with the bits
   leftover from a previous deflate stream when appending to it.  As such, this
   function can only be used for raw deflate, and must be used before the first
   deflate() call after a deflateInit2() or deflateReset().  bits must be less
   than or equal to 16, and that many of the least significant bits of value
   will be inserted in the output.

     deflatePrime returns Z_OK if success, Z_BUF_ERROR if there was not enough
   room in the internal buffer to insert the bits, or Z_STREAM_ERROR if the
   source stream state was inconsistent.
*/

ZEXTERN int ZEXPORT deflateSetHeader OF((z_streamp strm,
                                         gz_headerp head));
/*
     deflateSetHeader() provides gzip header information for when a gzip
   stream is requested by deflateInit2().  deflateSetHeader() may be called
   after deflateInit2() or deflateReset() and before the first call of
   deflate().  The text, time, os, extra field, name, and comment information
   in the provided gz_header structure are written to the gzip header (xflag is
   ignored -- the extra flags are set according to the compression level).  The
   caller must assure that, if not Z_NULL, name and comment are terminated with
   a zero byte, and that if extra is not Z_NULL, that extra_len bytes are
   available there.  If hcrc is true, a gzip header crc is included.  Note that
   the current versions of the command-line version of gzip (up through version
   1.3.x) do not support header crc's, and will report that it is a "multi-part
   gzip file" and give up.

     If deflateSetHeader is not used, the default gzip header has text false,
   the time set to zero, and os set to 255, with no extra, name, or comment
   fields.  The gzip header is returned to the default state by deflateReset().

     deflateSetHeader returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.
*/

/*
ZEXTERN int ZEXPORT inflateInit2 OF((z_streamp strm,
                                     int  windowBits));

     This is another version of inflateInit with an extra parameter.  The
   fields next_in, avail_in, zalloc, zfree and opaque must be initialized
   before by the caller.

     The windowBits parameter is the base two logarithm of the maximum window
   size (the size of the history buffer).  It should be in the range 8..15 for
   this version of the library.  The default value is 15 if inflateInit is used
   instead.  windowBits must be greater than or equal to the windowBits value
   provided to deflateInit2() while compressing, or it must be equal to 15 if
   deflateInit2() was not used.  If a compressed stream with a larger window
   size is given as input, inflate() will return with the error code
   Z_DATA_ERROR instead of trying to allocate a larger window.

     windowBits can also be zero to request that inflate use the window size in
   the zlib header of the compressed stream.

     windowBits can also be -8..-15 for raw inflate.  In this case, -windowBits
   determines the window size.  inflate() will then process raw deflate data,
   not looking for a zlib or gzip header, not generating a check value, and not
   looking for any check values for comparison at the end of the stream.  This
   is for use with other formats that use the deflate compressed data format
   such as zip.  Those formats provide their own check values.  If a custom
   format is developed using the raw deflate format for compressed data, it is
   recommended that a check value such as an adler32 or a crc32 be applied to
   the uncompressed data as is done in the zlib, gzip, and zip formats.  For
   most applications, the zlib format should be used as is.  Note that comments
   above on the use in deflateInit2() applies to the magnitude of windowBits.

     windowBits can also be greater than 15 for optional gzip decoding.  Add
   32 to windowBits to enable zlib and gzip decoding with automatic header
   detection, or add 16 to decode only the gzip format (the zlib format will
   return a Z_DATA_ERROR).  If a gzip stream is being decoded, strm->adler is a
   crc32 instead of an adler32.

     inflateInit2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_VERSION_ERROR if the zlib library version is incompatible with the
   version assumed by the caller, or Z_STREAM_ERROR if the parameters are
   invalid, such as a null pointer to the structure.  msg is set to null if
   there is no error message.  inflateInit2 does not perform any decompression
   apart from possibly reading the zlib header if present: actual decompression
   will be done by inflate().  (So next_in and avail_in may be modified, but
   next_out and avail_out are unused and unchanged.) The current implementation
   of inflateInit2() does not process any header information -- that is
   deferred until inflate() is called.
*/

ZEXTERN int ZEXPORT inflateSetDictionary OF((z_streamp strm,
                                             const Bytef *dictionary,
                                             uInt  dictLength));
/*
     Initializes the decompression dictionary from the given uncompressed byte
   sequence.  This function must be called immediately after a call of inflate,
   if that call returned Z_NEED_DICT.  The dictionary chosen by the compressor
   can be determined from the adler32 value returned by that call of inflate.
   The compressor and decompressor must use exactly the same dictionary (see
   deflateSetDictionary).  For raw inflate, this function can be called at any
   time to set the dictionary.  If the provided dictionary is smaller than the
   window and there is already data in the window, then the provided dictionary
   will amend what's there.  The application must insure that the dictionary
   that was used for compression is provided.

     inflateSetDictionary returns Z_OK if success, Z_STREAM_ERROR if a
   parameter is invalid (e.g.  dictionary being Z_NULL) or the stream state is
   inconsistent, Z_DATA_ERROR if the given dictionary doesn't match the
   expected one (incorrect adler32 value).  inflateSetDictionary does not
   perform any decompression: this will be done by subsequent calls of
   inflate().
*/

ZEXTERN int ZEXPORT inflateSync OF((z_streamp strm));
/*
     Skips invalid compressed data until a possible full flush point (see above
   for the description of deflate with Z_FULL_FLUSH) can be found, or until all
   available input is skipped.  No output is provided.

     inflateSync searches for a 00 00 FF FF pattern in the compressed data.
   All full flush points have this pattern, but not all occurences of this
   pattern are full flush points.

     inflateSync returns Z_OK if a possible full flush point has been found,
   Z_BUF_ERROR if no more input was provided, Z_DATA_ERROR if no flush point
   has been found, or Z_STREAM_ERROR if the stream structure was inconsistent.
   In the success case, the application may save the current current value of
   total_in which indicates where valid compressed data was found.  In the
   error case, the application may repeatedly call inflateSync, providing more
   input each time, until success or end of the input data.
*/

ZEXTERN int ZEXPORT inflateCopy OF((z_streamp dest,
                                    z_streamp source));
/*
     Sets the destination stream as a complete copy of the source stream.

     This function can be useful when randomly accessing a large stream.  The
   first pass through the stream can periodically record the inflate state,
   allowing restarting inflate at those points when randomly accessing the
   stream.

     inflateCopy returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_STREAM_ERROR if the source stream state was inconsistent
   (such as zalloc being Z_NULL).  msg is left unchanged in both source and
   destination.
*/

ZEXTERN int ZEXPORT inflateReset OF((z_streamp strm));
/*
     This function is equivalent to inflateEnd followed by inflateInit,
   but does not free and reallocate all the internal decompression state.  The
   stream will keep attributes that may have been set by inflateInit2.

     inflateReset returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being Z_NULL).
*/

ZEXTERN int ZEXPORT inflateReset2 OF((z_streamp strm,
                                      int windowBits));
/*
     This function is the same as inflateReset, but it also permits changing
   the wrap and window size requests.  The windowBits parameter is interpreted
   the same as it is for inflateInit2.

     inflateReset2 returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent (such as zalloc or state being Z_NULL), or if
   the windowBits parameter is invalid.
*/

ZEXTERN int ZEXPORT inflatePrime OF((z_streamp strm,
                                     int bits,
                                     int value));
/*
     This function inserts bits in the inflate input stream.  The intent is
   that this function is used to start inflating at a bit position in the
   middle of a byte.  The provided bits will be used before any bytes are used
   from next_in.  This function should only be used with raw inflate, and
   should be used before the first inflate() call after inflateInit2() or
   inflateReset().  bits must be less than or equal to 16, and that many of the
   least significant bits of value will be inserted in the input.

     If bits is negative, then the input stream bit buffer is emptied.  Then
   inflatePrime() can be called again to put bits in the buffer.  This is used
   to clear out bits leftover after feeding inflate a block description prior
   to feeding inflate codes.

     inflatePrime returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.
*/

ZEXTERN long ZEXPORT inflateMark OF((z_streamp strm));
/*
     This function returns two values, one in the lower 16 bits of the return
   value, and the other in the remaining upper bits, obtained by shifting the
   return value down 16 bits.  If the upper value is -1 and the lower value is
   zero, then inflate() is currently decoding information outside of a block.
   If the upper value is -1 and the lower value is non-zero, then inflate is in
   the middle of a stored block, with the lower value equaling the number of
   bytes from the input remaining to copy.  If the upper value is not -1, then
   it is the number of bits back from the current bit position in the input of
   the code (literal or length/distance pair) currently being processed.  In
   that case the lower value is the number of bytes already emitted for that
   code.

     A code is being processed if inflate is waiting for more input to complete
   decoding of the code, or if it has completed decoding but is waiting for
   more output space to write the literal or match data.

     inflateMark() is used to mark locations in the input data for random
   access, which may be at bit positions, and to note those cases where the
   output of a code may span boundaries of random access blocks.  The current
   location in the input stream can be determined from avail_in and data_type
   as noted in the description for the Z_BLOCK flush parameter for inflate.

     inflateMark returns the value noted above or -1 << 16 if the provided
   source stream state was inconsistent.
*/

ZEXTERN int ZEXPORT inflateGetHeader OF((z_streamp strm,
                                         gz_headerp head));
/*
     inflateGetHeader() requests that gzip header information be stored in the
   provided gz_header structure.  inflateGetHeader() may be called after
   inflateInit2() or inflateReset(), and before the first call of inflate().
   As inflate() processes the gzip stream, head->done is zero until the header
   is completed, at which time head->done is set to one.  If a zlib stream is
   being decoded, then head->done is set to -1 to indicate that there will be
   no gzip header information forthcoming.  Note that Z_BLOCK or Z_TREES can be
   used to force inflate() to return immediately after header processing is
   complete and before any actual data is decompressed.

     The text, time, xflags, and os fields are filled in with the gzip header
   contents.  hcrc is set to true if there is a header CRC.  (The header CRC
   was valid if done is set to one.) If extra is not Z_NULL, then extra_max
   contains the maximum number of bytes to write to extra.  Once done is true,
   extra_len contains the actual extra field length, and extra contains the
   extra field, or that field truncated if extra_max is less than extra_len.
   If name is not Z_NULL, then up to name_max characters are written there,
   terminated with a zero unless the length is greater than name_max.  If
   comment is not Z_NULL, then up to comm_max characters are written there,
   terminated with a zero unless the length is greater than comm_max.  When any
   of extra, name, or comment are not Z_NULL and the respective field is not
   present in the header, then that field is set to Z_NULL to signal its
   absence.  This allows the use of deflateSetHeader() with the returned
   structure to duplicate the header.  However if those fields are set to
   allocated memory, then the application will need to save those pointers
   elsewhere so that they can be eventually freed.

     If inflateGetHeader is not used, then the header information is simply
   discarded.  The header is always checked for validity, including the header
   CRC if present.  inflateReset() will reset the process to discard the header
   information.  The application would need to call inflateGetHeader() again to
   retrieve the header from the next gzip stream.

     inflateGetHeader returns Z_OK if success, or Z_STREAM_ERROR if the source
   stream state was inconsistent.
*/

/*
ZEXTERN int ZEXPORT inflateBackInit OF((z_streamp strm, int windowBits,
                                        unsigned char FAR *window));

     Initialize the internal stream state for decompression using inflateBack()
   calls.  The fields zalloc, zfree and opaque in strm must be initialized
   before the call.  If zalloc and zfree are Z_NULL, then the default library-
   derived memory allocation routines are used.  windowBits is the base two
   logarithm of the window size, in the range 8..15.  window is a caller
   supplied buffer of that size.  Except for special applications where it is
   assured that deflate was used with small window sizes, windowBits must be 15
   and a 32K byte window must be supplied to be able to decompress general
   deflate streams.

     See inflateBack() for the usage of these routines.

     inflateBackInit will return Z_OK on success, Z_STREAM_ERROR if any of
   the parameters are invalid, Z_MEM_ERROR if the internal state could not be
   allocated, or Z_VERSION_ERROR if the version of the library does not match
   the version of the header file.
*/

typedef unsigned (*in_func) OF((void FAR *, unsigned char FAR * FAR *));
typedef int (*out_func) OF((void FAR *, unsigned char FAR *, unsigned));

ZEXTERN int ZEXPORT inflateBack OF((z_streamp strm,
                                    in_func in, void FAR *in_desc,
                                    out_func out, void FAR *out_desc));
/*
     inflateBack() does a raw inflate with a single call using a call-back
   interface for input and output.  This is more efficient than inflate() for
   file i/o applications in that it avoids copying between the output and the
   sliding window by simply making the window itself the output buffer.  This
   function trusts the application to not change the output buffer passed by
   the output function, at least until inflateBack() returns.

     inflateBackInit() must be called first to allocate the internal state
   and to initialize the state with the user-provided window buffer.
   inflateBack() may then be used multiple times to inflate a complete, raw
   deflate stream with each call.  inflateBackEnd() is then called to free the
   allocated state.

     A raw deflate stream is one with no zlib or gzip header or trailer.
   This routine would normally be used in a utility that reads zip or gzip
   files and writes out uncompressed files.  The utility would decode the
   header and process the trailer on its own, hence this routine expects only
   the raw deflate stream to decompress.  This is different from the normal
   behavior of inflate(), which expects either a zlib or gzip header and
   trailer around the deflate stream.

     inflateBack() uses two subroutines supplied by the caller that are then
   called by inflateBack() for input and output.  inflateBack() calls those
   routines until it reads a complete deflate stream and writes out all of the
   uncompressed data, or until it encounters an error.  The function's
   parameters and return types are defined above in the in_func and out_func
   typedefs.  inflateBack() will call in(in_desc, &buf) which should return the
   number of bytes of provided input, and a pointer to that input in buf.  If
   there is no input available, in() must return zero--buf is ignored in that
   case--and inflateBack() will return a buffer error.  inflateBack() will call
   out(out_desc, buf, len) to write the uncompressed data buf[0..len-1].  out()
   should return zero on success, or non-zero on failure.  If out() returns
   non-zero, inflateBack() will return with an error.  Neither in() nor out()
   are permitted to change the contents of the window provided to
   inflateBackInit(), which is also the buffer that out() uses to write from.
   The length written by out() will be at most the window size.  Any non-zero
   amount of input may be provided by in().

     For convenience, inflateBack() can be provided input on the first call by
   setting strm->next_in and strm->avail_in.  If that input is exhausted, then
   in() will be called.  Therefore strm->next_in must be initialized before
   calling inflateBack().  If strm->next_in is Z_NULL, then in() will be called
   immediately for input.  If strm->next_in is not Z_NULL, then strm->avail_in
   must also be initialized, and then if strm->avail_in is not zero, input will
   initially be taken from strm->next_in[0 ..  strm->avail_in - 1].

     The in_desc and out_desc parameters of inflateBack() is passed as the
   first parameter of in() and out() respectively when they are called.  These
   descriptors can be optionally used to pass any information that the caller-
   supplied in() and out() functions need to do their job.

     On return, inflateBack() will set strm->next_in and strm->avail_in to
   pass back any unused input that was provided by the last in() call.  The
   return values of inflateBack() can be Z_STREAM_END on success, Z_BUF_ERROR
   if in() or out() returned an error, Z_DATA_ERROR if there was a format error
   in the deflate stream (in which case strm->msg is set to indicate the nature
   of the error), or Z_STREAM_ERROR if the stream was not properly initialized.
   In the case of Z_BUF_ERROR, an input or output error can be distinguished
   using strm->next_in which will be Z_NULL only if in() returned an error.  If
   strm->next_in is not Z_NULL, then the Z_BUF_ERROR was due to out() returning
   non-zero.  (in() will always be called before out(), so strm->next_in is
   assured to be defined if out() returns non-zero.) Note that inflateBack()
   cannot return Z_OK.
*/

ZEXTERN int ZEXPORT inflateBackEnd OF((z_streamp strm));
/*
     All memory allocated by inflateBackInit() is freed.

     inflateBackEnd() returns Z_OK on success, or Z_STREAM_ERROR if the stream
   state was inconsistent.
*/

ZEXTERN uLong ZEXPORT zlibCompileFlags OF((void));
/* Return flags indicating compile-time options.

    Type sizes, two bits each, 00 = 16 bits, 01 = 32, 10 = 64, 11 = other:
     1.0: size of uInt
     3.2: size of uLong
     5.4: size of voidpf (pointer)
     7.6: size of z_off_t

    Compiler, assembler, and debug options:
     8: DEBUG
     9: ASMV or ASMINF -- use ASM code
     10: ZLIB_WINAPI -- exported functions use the WINAPI calling convention
     11: 0 (reserved)

    One-time table building (smaller code, but not thread-safe if true):
     12: BUILDFIXED -- build static block decoding tables when needed
     13: DYNAMIC_CRC_TABLE -- build CRC calculation tables when needed
     14,15: 0 (reserved)

    Library content (indicates missing functionality):
     16: NO_GZCOMPRESS -- gz* functions cannot compress (to avoid linking
                          deflate code when not needed)
     17: NO_GZIP -- deflate can't write gzip streams, and inflate can't detect
                    and decode gzip streams (to avoid linking crc code)
     18-19: 0 (reserved)

    Operation variations (changes in library functionality):
     20: PKZIP_BUG_WORKAROUND -- slightly more permissive inflate
     21: FASTEST -- deflate algorithm with only one, lowest compression level
     22,23: 0 (reserved)

    The sprintf variant used by gzprintf (zero is best):
     24: 0 = vs*, 1 = s* -- 1 means limited to 20 arguments after the format
     25: 0 = *nprintf, 1 = *printf -- 1 means gzprintf() not secure!
     26: 0 = returns value, 1 = void -- 1 means inferred string length returned

    Remainder:
     27-31: 0 (reserved)
 */

#ifndef Z_SOLO

                        /* utility functions */

/*
     The following utility functions are implemented on top of the basic
   stream-oriented functions.  To simplify the interface, some default options
   are assumed (compression level and memory usage, standard memory allocation
   functions).  The source code of these utility functions can be modified if
   you need special options.
*/

ZEXTERN int ZEXPORT compress OF((Bytef *dest,   uLongf *destLen,
                                 const Bytef *source, uLong sourceLen));
/*
     Compresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer.  Upon entry, destLen is the total size
   of the destination buffer, which must be at least the value returned by
   compressBound(sourceLen).  Upon exit, destLen is the actual size of the
   compressed buffer.

     compress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer.
*/

ZEXTERN int ZEXPORT compress2 OF((Bytef *dest,   uLongf *destLen,
                                  const Bytef *source, uLong sourceLen,
                                  int level));
/*
     Compresses the source buffer into the destination buffer.  The level
   parameter has the same meaning as in deflateInit.  sourceLen is the byte
   length of the source buffer.  Upon entry, destLen is the total size of the
   destination buffer, which must be at least the value returned by
   compressBound(sourceLen).  Upon exit, destLen is the actual size of the
   compressed buffer.

     compress2 returns Z_OK if success, Z_MEM_ERROR if there was not enough
   memory, Z_BUF_ERROR if there was not enough room in the output buffer,
   Z_STREAM_ERROR if the level parameter is invalid.
*/

ZEXTERN uLong ZEXPORT compressBound OF((uLong sourceLen));
/*
     compressBound() returns an upper bound on the compressed size after
   compress() or compress2() on sourceLen bytes.  It would be used before a
   compress() or compress2() call to allocate the destination buffer.
*/

ZEXTERN int ZEXPORT uncompress OF((Bytef *dest,   uLongf *destLen,
                                   const Bytef *source, uLong sourceLen));
/*
     Decompresses the source buffer into the destination buffer.  sourceLen is
   the byte length of the source buffer.  Upon entry, destLen is the total size
   of the destination buffer, which must be large enough to hold the entire
   uncompressed data.  (The size of the uncompressed data must have been saved
   previously by the compressor and transmitted to the decompressor by some
   mechanism outside the scope of this compression library.) Upon exit, destLen
   is the actual size of the uncompressed buffer.

     uncompress returns Z_OK if success, Z_MEM_ERROR if there was not
   enough memory, Z_BUF_ERROR if there was not enough room in the output
   buffer, or Z_DATA_ERROR if the input data was corrupted or incomplete.  In
   the case where there is not enough room, uncompress() will fill the output
   buffer with the uncompressed data up to that point.
*/

                        /* gzip file access functions */

/*
     This library supports reading and writing files in gzip (.gz) format with
   an interface similar to that of stdio, using the functions that start with
   "gz".  The gzip format is different from the zlib format.  gzip is a gzip
   wrapper, documented in RFC 1952, wrapped around a deflate stream.
*/

typedef struct gzFile_s *gzFile;    /* semi-opaque gzip file descriptor */

/*
ZEXTERN gzFile ZEXPORT gzopen OF((const char *path, const char *mode));

     Opens a gzip (.gz) file for reading or writing.  The mode parameter is as
   in fopen ("rb" or "wb") but can also include a compression level ("wb9") or
   a strategy: 'f' for filtered data as in "wb6f", 'h' for Huffman-only
   compression as in "wb1h", 'R' for run-length encoding as in "wb1R", or 'F'
   for fixed code compression as in "wb9F".  (See the description of
   deflateInit2 for more information about the strategy parameter.)  'T' will
   request transparent writing or appending with no compression and not using
   the gzip format.

     "a" can be used instead of "w" to request that the gzip stream that will
   be written be appended to the file.  "+" will result in an error, since
   reading and writing to the same gzip file is not supported.

     These functions, as well as gzip, will read and decode a sequence of gzip
   streams in a file.  The append function of gzopen() can be used to create
   such a file.  (Also see gzflush() for another way to do this.)  When
   appending, gzopen does not test whether the file begins with a gzip stream,
   nor does it look for the end of the gzip streams to begin appending.  gzopen
   will simply append a gzip stream to the existing file.

     gzopen can be used to read a file which is not in gzip format; in this
   case gzread will directly read from the file without decompression.  When
   reading, this will be detected automatically by looking for the magic two-
   byte gzip header.

     gzopen returns NULL if the file could not be opened, if there was
   insufficient memory to allocate the gzFile state, or if an invalid mode was
   specified (an 'r', 'w', or 'a' was not provided, or '+' was provided).
   errno can be checked to determine if the reason gzopen failed was that the
   file could not be opened.
*/

ZEXTERN gzFile ZEXPORT gzdopen OF((int fd, const char *mode));
/*
     gzdopen associates a gzFile with the file descriptor fd.  File descriptors
   are obtained from calls like open, dup, creat, pipe or fileno (if the file
   has been previously opened with fopen).  The mode parameter is as in gzopen.

     The next call of gzclose on the returned gzFile will also close the file
   descriptor fd, just like fclose(fdopen(fd, mode)) closes the file descriptor
   fd.  If you want to keep fd open, use fd = dup(fd_keep); gz = gzdopen(fd,
   mode);.  The duplicated descriptor should be saved to avoid a leak, since
   gzdopen does not close fd if it fails.  If you are using fileno() to get the
   file descriptor from a FILE *, then you will have to use dup() to avoid
   double-close()ing the file descriptor.  Both gzclose() and fclose() will
   close the associated file descriptor, so they need to have different file
   descriptors.

     gzdopen returns NULL if there was insufficient memory to allocate the
   gzFile state, if an invalid mode was specified (an 'r', 'w', or 'a' was not
   provided, or '+' was provided), or if fd is -1.  The file descriptor is not
   used until the next gz* read, write, seek, or close operation, so gzdopen
   will not detect if fd is invalid (unless fd is -1).
*/

ZEXTERN int ZEXPORT gzbuffer OF((gzFile file, unsigned size));
/*
     Set the internal buffer size used by this library's functions.  The
   default buffer size is 8192 bytes.  This function must be called after
   gzopen() or gzdopen(), and before any other calls that read or write the
   file.  The buffer memory allocation is always deferred to the first read or
   write.  Two buffers are allocated, either both of the specified size when
   writing, or one of the specified size and the other twice that size when
   reading.  A larger buffer size of, for example, 64K or 128K bytes will
   noticeably increase the speed of decompression (reading).

     The new buffer size also affects the maximum length for gzprintf().

     gzbuffer() returns 0 on success, or -1 on failure, such as being called
   too late.
*/

ZEXTERN int ZEXPORT gzsetparams OF((gzFile file, int level, int strategy));
/*
     Dynamically update the compression level or strategy.  See the description
   of deflateInit2 for the meaning of these parameters.

     gzsetparams returns Z_OK if success, or Z_STREAM_ERROR if the file was not
   opened for writing.
*/

ZEXTERN int ZEXPORT gzread OF((gzFile file, voidp buf, unsigned len));
/*
     Reads the given number of uncompressed bytes from the compressed file.  If
   the input file is not in gzip format, gzread copies the given number of
   bytes into the buffer directly from the file.

     After reaching the end of a gzip stream in the input, gzread will continue
   to read, looking for another gzip stream.  Any number of gzip streams may be
   concatenated in the input file, and will all be decompressed by gzread().
   If something other than a gzip stream is encountered after a gzip stream,
   that remaining trailing garbage is ignored (and no error is returned).

     gzread can be used to read a gzip file that is being concurrently written.
   Upon reaching the end of the input, gzread will return with the available
   data.  If the error code returned by gzerror is Z_OK or Z_BUF_ERROR, then
   gzclearerr can be used to clear the end of file indicator in order to permit
   gzread to be tried again.  Z_OK indicates that a gzip stream was completed
   on the last gzread.  Z_BUF_ERROR indicates that the input file ended in the
   middle of a gzip stream.  Note that gzread does not return -1 in the event
   of an incomplete gzip stream.  This error is deferred until gzclose(), which
   will return Z_BUF_ERROR if the last gzread ended in the middle of a gzip
   stream.  Alternatively, gzerror can be used before gzclose to detect this
   case.

     gzread returns the number of uncompressed bytes actually read, less than
   len for end of file, or -1 for error.
*/

ZEXTERN int ZEXPORT gzwrite OF((gzFile file,
                                voidpc buf, unsigned len));
/*
     Writes the given number of uncompressed bytes into the compressed file.
   gzwrite returns the number of uncompressed bytes written or 0 in case of
   error.
*/

ZEXTERN int ZEXPORTVA gzprintf Z_ARG((gzFile file, const char *format, ...));
/*
     Converts, formats, and writes the arguments to the compressed file under
   control of the format string, as in fprintf.  gzprintf returns the number of
   uncompressed bytes actually written, or 0 in case of error.  The number of
   uncompressed bytes written is limited to 8191, or one less than the buffer
   size given to gzbuffer().  The caller should assure that this limit is not
   exceeded.  If it is exceeded, then gzprintf() will return an error (0) with
   nothing written.  In this case, there may also be a buffer overflow with
   unpredictable consequences, which is possible only if zlib was compiled with
   the insecure functions sprintf() or vsprintf() because the secure snprintf()
   or vsnprintf() functions were not available.  This can be determined using
   zlibCompileFlags().
*/

ZEXTERN int ZEXPORT gzputs OF((gzFile file, const char *s));
/*
     Writes the given null-terminated string to the compressed file, excluding
   the terminating null character.

     gzputs returns the number of characters written, or -1 in case of error.
*/

ZEXTERN char * ZEXPORT gzgets OF((gzFile file, char *buf, int len));
/*
     Reads bytes from the compressed file until len-1 characters are read, or a
   newline character is read and transferred to buf, or an end-of-file
   condition is encountered.  If any characters are read or if len == 1, the
   string is terminated with a null character.  If no characters are read due
   to an end-of-file or len < 1, then the buffer is left untouched.

     gzgets returns buf which is a null-terminated string, or it returns NULL
   for end-of-file or in case of error.  If there was an error, the contents at
   buf are indeterminate.
*/

ZEXTERN int ZEXPORT gzputc OF((gzFile file, int c));
/*
     Writes c, converted to an unsigned char, into the compressed file.  gzputc
   returns the value that was written, or -1 in case of error.
*/

ZEXTERN int ZEXPORT gzgetc OF((gzFile file));
/*
     Reads one byte from the compressed file.  gzgetc returns this byte or -1
   in case of end of file or error.  This is implemented as a macro for speed.
   As such, it does not do all of the checking the other functions do.  I.e.
   it does not check to see if file is NULL, nor whether the structure file
   points to has been clobbered or not.
*/

ZEXTERN int ZEXPORT gzungetc OF((int c, gzFile file));
/*
     Push one character back onto the stream to be read as the first character
   on the next read.  At least one character of push-back is allowed.
   gzungetc() returns the character pushed, or -1 on failure.  gzungetc() will
   fail if c is -1, and may fail if a character has been pushed but not read
   yet.  If gzungetc is used immediately after gzopen or gzdopen, at least the
   output buffer size of pushed characters is allowed.  (See gzbuffer above.)
   The pushed character will be discarded if the stream is repositioned with
   gzseek() or gzrewind().
*/

ZEXTERN int ZEXPORT gzflush OF((gzFile file, int flush));
/*
     Flushes all pending output into the compressed file.  The parameter flush
   is as in the deflate() function.  The return value is the zlib error number
   (see function gzerror below).  gzflush is only permitted when writing.

     If the flush parameter is Z_FINISH, the remaining data is written and the
   gzip stream is completed in the output.  If gzwrite() is called again, a new
   gzip stream will be started in the output.  gzread() is able to read such
   concatented gzip streams.

     gzflush should be called only when strictly necessary because it will
   degrade compression if called too often.
*/

/*
ZEXTERN z_off_t ZEXPORT gzseek OF((gzFile file,
                                   z_off_t offset, int whence));

     Sets the starting position for the next gzread or gzwrite on the given
   compressed file.  The offset represents a number of bytes in the
   uncompressed data stream.  The whence parameter is defined as in lseek(2);
   the value SEEK_END is not supported.

     If the file is opened for reading, this function is emulated but can be
   extremely slow.  If the file is opened for writing, only forward seeks are
   supported; gzseek then compresses a sequence of zeroes up to the new
   starting position.

     gzseek returns the resulting offset location as measured in bytes from
   the beginning of the uncompressed stream, or -1 in case of error, in
   particular if the file is opened for writing and the new starting position
   would be before the current position.
*/

ZEXTERN int ZEXPORT    gzrewind OF((gzFile file));
/*
     Rewinds the given file. This function is supported only for reading.

     gzrewind(file) is equivalent to (int)gzseek(file, 0L, SEEK_SET)
*/

/*
ZEXTERN z_off_t ZEXPORT    gztell OF((gzFile file));

     Returns the starting position for the next gzread or gzwrite on the given
   compressed file.  This position represents a number of bytes in the
   uncompressed data stream, and is zero when starting, even if appending or
   reading a gzip stream from the middle of a file using gzdopen().

     gztell(file) is equivalent to gzseek(file, 0L, SEEK_CUR)
*/

/*
ZEXTERN z_off_t ZEXPORT gzoffset OF((gzFile file));

     Returns the current offset in the file being read or written.  This offset
   includes the count of bytes that precede the gzip stream, for example when
   appending or when using gzdopen() for reading.  When reading, the offset
   does not include as yet unused buffered input.  This information can be used
   for a progress indicator.  On error, gzoffset() returns -1.
*/

ZEXTERN int ZEXPORT gzeof OF((gzFile file));
/*
     Returns true (1) if the end-of-file indicator has been set while reading,
   false (0) otherwise.  Note that the end-of-file indicator is set only if the
   read tried to go past the end of the input, but came up short.  Therefore,
   just like feof(), gzeof() may return false even if there is no more data to
   read, in the event that the last read request was for the exact number of
   bytes remaining in the input file.  This will happen if the input file size
   is an exact multiple of the buffer size.

     If gzeof() returns true, then the read functions will return no more data,
   unless the end-of-file indicator is reset by gzclearerr() and the input file
   has grown since the previous end of file was detected.
*/

ZEXTERN int ZEXPORT gzdirect OF((gzFile file));
/*
     Returns true (1) if file is being copied directly while reading, or false
   (0) if file is a gzip stream being decompressed.

     If the input file is empty, gzdirect() will return true, since the input
   does not contain a gzip stream.

     If gzdirect() is used immediately after gzopen() or gzdopen() it will
   cause buffers to be allocated to allow reading the file to determine if it
   is a gzip file.  Therefore if gzbuffer() is used, it should be called before
   gzdirect().

     When writing, gzdirect() returns true (1) if transparent writing was
   requested ("wT" for the gzopen() mode), or false (0) otherwise.  (Note:
   gzdirect() is not needed when writing.  Transparent writing must be
   explicitly requested, so the application already knows the answer.  When
   linking statically, using gzdirect() will include all of the zlib code for
   gzip file reading and decompression, which may not be desired.)
*/

ZEXTERN int ZEXPORT    gzclose OF((gzFile file));
/*
     Flushes all pending output if necessary, closes the compressed file and
   deallocates the (de)compression state.  Note that once file is closed, you
   cannot call gzerror with file, since its structures have been deallocated.
   gzclose must not be called more than once on the same file, just as free
   must not be called more than once on the same allocation.

     gzclose will return Z_STREAM_ERROR if file is not valid, Z_ERRNO on a
   file operation error, Z_MEM_ERROR if out of memory, Z_BUF_ERROR if the
   last read ended in the middle of a gzip stream, or Z_OK on success.
*/

ZEXTERN int ZEXPORT gzclose_r OF((gzFile file));
ZEXTERN int ZEXPORT gzclose_w OF((gzFile file));
/*
     Same as gzclose(), but gzclose_r() is only for use when reading, and
   gzclose_w() is only for use when writing or appending.  The advantage to
   using these instead of gzclose() is that they avoid linking in zlib
   compression or decompression code that is not used when only reading or only
   writing respectively.  If gzclose() is used, then both compression and
   decompression code will be included the application when linking to a static
   zlib library.
*/

ZEXTERN const char * ZEXPORT gzerror OF((gzFile file, int *errnum));
/*
     Returns the error message for the last error which occurred on the given
   compressed file.  errnum is set to zlib error number.  If an error occurred
   in the file system and not in the compression library, errnum is set to
   Z_ERRNO and the application may consult errno to get the exact error code.

     The application must not modify the returned string.  Future calls to
   this function may invalidate the previously returned string.  If file is
   closed, then the string previously returned by gzerror will no longer be
   available.

     gzerror() should be used to distinguish errors from end-of-file for those
   functions above that do not distinguish those cases in their return values.
*/

ZEXTERN void ZEXPORT gzclearerr OF((gzFile file));
/*
     Clears the error and end-of-file flags for file.  This is analogous to the
   clearerr() function in stdio.  This is useful for continuing to read a gzip
   file that is being written concurrently.
*/

#endif /* !Z_SOLO */

                        /* checksum functions */

/*
     These functions are not related to compression but are exported
   anyway because they might be useful in applications using the compression
   library.
*/

ZEXTERN uLong ZEXPORT adler32 OF((uLong adler, const Bytef *buf, uInt len));
/*
     Update a running Adler-32 checksum with the bytes buf[0..len-1] and
   return the updated checksum.  If buf is Z_NULL, this function returns the
   required initial value for the checksum.

     An Adler-32 checksum is almost as reliable as a CRC32 but can be computed
   much faster.

   Usage example:

     uLong adler = adler32(0L, Z_NULL, 0);

     while (read_buffer(buffer, length) != EOF) {
       adler = adler32(adler, buffer, length);
     }
     if (adler != original_adler) error();
*/

/*
ZEXTERN uLong ZEXPORT adler32_combine OF((uLong adler1, uLong adler2,
                                          z_off_t len2));

     Combine two Adler-32 checksums into one.  For two sequences of bytes, seq1
   and seq2 with lengths len1 and len2, Adler-32 checksums were calculated for
   each, adler1 and adler2.  adler32_combine() returns the Adler-32 checksum of
   seq1 and seq2 concatenated, requiring only adler1, adler2, and len2.  Note
   that the z_off_t type (like off_t) is a signed integer.  If len2 is
   negative, the result has no meaning or utility.
*/

ZEXTERN uLong ZEXPORT crc32   OF((uLong crc, const Bytef *buf, uInt len));
/*
     Update a running CRC-32 with the bytes buf[0..len-1] and return the
   updated CRC-32.  If buf is Z_NULL, this function returns the required
   initial value for the for the crc.  Pre- and post-conditioning (one's
   complement) is performed within this function so it shouldn't be done by the
   application.

   Usage example:

     uLong crc = crc32(0L, Z_NULL, 0);

     while (read_buffer(buffer, length) != EOF) {
       crc = crc32(crc, buffer, length);
     }
     if (crc != original_crc) error();
*/

/*
ZEXTERN uLong ZEXPORT crc32_combine OF((uLong crc1, uLong crc2, z_off_t len2));

     Combine two CRC-32 check values into one.  For two sequences of bytes,
   seq1 and seq2 with lengths len1 and len2, CRC-32 check values were
   calculated for each, crc1 and crc2.  crc32_combine() returns the CRC-32
   check value of seq1 and seq2 concatenated, requiring only crc1, crc2, and
   len2.
*/


                        /* various hacks, don't look :) */

/* deflateInit and inflateInit are macros to allow checking the zlib version
 * and the compiler's view of z_stream:
 */
ZEXTERN int ZEXPORT deflateInit_ OF((z_streamp strm, int level,
                                     const char *version, int stream_size));
ZEXTERN int ZEXPORT inflateInit_ OF((z_streamp strm,
                                     const char *version, int stream_size));
ZEXTERN int ZEXPORT deflateInit2_ OF((z_streamp strm, int  level, int  method,
                                      int windowBits, int memLevel,
                                      int strategy, const char *version,
                                      int stream_size));
ZEXTERN int ZEXPORT inflateInit2_ OF((z_streamp strm, int  windowBits,
                                      const char *version, int stream_size));
ZEXTERN int ZEXPORT inflateBackInit_ OF((z_streamp strm, int windowBits,
                                         unsigned char FAR *window,
                                         const char *version,
                                         int stream_size));
#define deflateInit(strm, level) \
        deflateInit_((strm), (level), ZLIB_VERSION, (int)sizeof(z_stream))
#define inflateInit(strm) \
        inflateInit_((strm), ZLIB_VERSION, (int)sizeof(z_stream))
#define deflateInit2(strm, level, method, windowBits, memLevel, strategy) \
        deflateInit2_((strm),(level),(method),(windowBits),(memLevel),\
                      (strategy), ZLIB_VERSION, (int)sizeof(z_stream))
#define inflateInit2(strm, windowBits) \
        inflateInit2_((strm), (windowBits), ZLIB_VERSION, \
                      (int)sizeof(z_stream))
#define inflateBackInit(strm, windowBits, window) \
        inflateBackInit_((strm), (windowBits), (window), \
                      ZLIB_VERSION, (int)sizeof(z_stream))

#ifndef Z_SOLO

/* gzgetc() macro and its supporting function and exposed data structure.  Note
 * that the real internal state is much larger than the exposed structure.
 * This abbreviated structure exposes just enough for the gzgetc() macro.  The
 * user should not mess with these exposed elements, since their names or
 * behavior could change in the future, perhaps even capriciously.  They can
 * only be used by the gzgetc() macro.  You have been warned.
 */
struct gzFile_s {
    unsigned have;
    unsigned char *next;
    z_off64_t pos;
};
ZEXTERN int ZEXPORT gzgetc_ OF((gzFile file));
#define gzgetc(g) \
    ((g)->have ? ((g)->have--, (g)->pos++, *((g)->next)++) : gzgetc_(g))

/* provide 64-bit offset functions if _LARGEFILE64_SOURCE defined, and/or
 * change the regular functions to 64 bits if _FILE_OFFSET_BITS is 64 (if
 * both are true, the application gets the *64 functions, and the regular
 * functions are changed to 64 bits) -- in case these are set on systems
 * without large file support, _LFS64_LARGEFILE must also be true
 */
#if defined(_LARGEFILE64_SOURCE) && _LFS64_LARGEFILE-0
   ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
   ZEXTERN z_off64_t ZEXPORT gzseek64 OF((gzFile, z_off64_t, int));
   ZEXTERN z_off64_t ZEXPORT gztell64 OF((gzFile));
   ZEXTERN z_off64_t ZEXPORT gzoffset64 OF((gzFile));
   ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off64_t));
   ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off64_t));
#endif

#if !defined(ZLIB_INTERNAL) && _FILE_OFFSET_BITS-0 == 64 && _LFS64_LARGEFILE-0
#  ifdef Z_PREFIX_SET
#    define z_gzopen z_gzopen64
#    define z_gzseek z_gzseek64
#    define z_gztell z_gztell64
#    define z_gzoffset z_gzoffset64
#    define z_adler32_combine z_adler32_combine64
#    define z_crc32_combine z_crc32_combine64
#  else
#    define gzopen gzopen64
#    define gzseek gzseek64
#    define gztell gztell64
#    define gzoffset gzoffset64
#    define adler32_combine adler32_combine64
#    define crc32_combine crc32_combine64
#  endif
#  ifndef _LARGEFILE64_SOURCE
     ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
     ZEXTERN z_off_t ZEXPORT gzseek64 OF((gzFile, z_off_t, int));
     ZEXTERN z_off_t ZEXPORT gztell64 OF((gzFile));
     ZEXTERN z_off_t ZEXPORT gzoffset64 OF((gzFile));
     ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off_t));
     ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off_t));
#  endif
#else
   ZEXTERN gzFile ZEXPORT gzopen OF((const char *, const char *));
   ZEXTERN z_off_t ZEXPORT gzseek OF((gzFile, z_off_t, int));
   ZEXTERN z_off_t ZEXPORT gztell OF((gzFile));
   ZEXTERN z_off_t ZEXPORT gzoffset OF((gzFile));
   ZEXTERN uLong ZEXPORT adler32_combine OF((uLong, uLong, z_off_t));
   ZEXTERN uLong ZEXPORT crc32_combine OF((uLong, uLong, z_off_t));
#endif

#else /* Z_SOLO */

   ZEXTERN uLong ZEXPORT adler32_combine OF((uLong, uLong, z_off_t));
   ZEXTERN uLong ZEXPORT crc32_combine OF((uLong, uLong, z_off_t));

#endif /* !Z_SOLO */

/* hack for buggy compilers */
#if !defined(ZUTIL_H) && !defined(NO_DUMMY_DECL)
    struct internal_state {int dummy;};
#endif

/* undocumented functions */
ZEXTERN const char   * ZEXPORT zError           OF((int));
ZEXTERN int            ZEXPORT inflateSyncPoint OF((z_streamp));
ZEXTERN const uLongf * ZEXPORT get_crc_table    OF((void));
ZEXTERN int            ZEXPORT inflateUndermine OF((z_streamp, int));
ZEXTERN int            ZEXPORT inflateResetKeep OF((z_streamp));
ZEXTERN int            ZEXPORT deflateResetKeep OF((z_streamp));
#ifndef Z_SOLO
  ZEXTERN unsigned long  ZEXPORT gzflags          OF((void));
#endif

#ifdef __cplusplus
}
#endif

#endif /* ZLIB_H */

/************************************************************************/
/*
    Start of file "src/gzguts.h"
 */
/************************************************************************/

/* gzguts.h -- zlib internal header definitions for gz* operations
 * Copyright (C) 2004, 2005, 2010, 2011, 2012 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#include "bit.h"
#include "bitos.h"

#ifndef GZGUTS_H 
#define GZGUTS_H 

#if VXWORKS
    #ifndef _VSB_CONFIG_FILE
        #define _VSB_CONFIG_FILE "vsbConfig.h"
    #endif
#endif

#ifdef _LARGEFILE64_SOURCE
#  ifndef _LARGEFILE_SOURCE
#    define _LARGEFILE_SOURCE 1
#  endif
#  ifdef _FILE_OFFSET_BITS
#    undef _FILE_OFFSET_BITS
#  endif
#endif

#ifndef ZLIB_INTERNAL
#if ((__GNUC__-0) * 10 + __GNUC_MINOR__-0 >= 33) && !defined(NO_VIZ)
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#  define ZLIB_INTERNAL
#endif
#endif

#include <stdio.h>

#ifdef STDC
#  include <string.h>
#  include <stdlib.h>
#  include <limits.h>
#endif
#include <fcntl.h>

#ifdef __TURBOC__
#  include <io.h>
#endif

#ifdef NO_DEFLATE       /* for compatibility with old definition */
#  define NO_GZCOMPRESS
#endif

#if defined(STDC99) || (defined(__TURBOC__) && __TURBOC__ >= 0x550)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#if defined(__CYGWIN__)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#if defined(MSDOS) && defined(__BORLANDC__) && (BORLANDC > 0x410)
#  ifndef HAVE_VSNPRINTF
#    define HAVE_VSNPRINTF
#  endif
#endif

#ifndef HAVE_VSNPRINTF
#  ifdef MSDOS
/* vsnprintf may exist on some MS-DOS compilers (DJGPP?),
 but for now we just assume it doesn't. */
#    define NO_vsnprintf
#  endif
#  ifdef __TURBOC__
#    define NO_vsnprintf
#  endif
#  ifdef WIN32
/* In Win32, vsnprintf is available as the "non-ANSI" _vsnprintf. */
#    if !defined(vsnprintf) && !defined(NO_vsnprintf)
#      if !defined(_MSC_VER) || ( defined(_MSC_VER) && _MSC_VER < 1500 )
#         include <io.h>
#         define vsnprintf _vsnprintf
#      endif
#    endif
#  endif
#  ifdef __SASC
#    define NO_vsnprintf
#  endif
#  ifdef VMS
#    define NO_vsnprintf
#  endif
#  ifdef __OS400__
#    define NO_vsnprintf
#  endif
#  ifdef __MVS__
#    define NO_vsnprintf
#  endif
#endif

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

/* gz* functions always use library allocation functions */
#ifndef STDC
  extern voidp  malloc OF((uInt size));
  extern void   free   OF((voidpf ptr));
#endif

/* get errno and strerror definition */
#if defined UNDER_CE
#  include <windows.h>
#  define zstrerror() gz_strwinerror((DWORD)GetLastError())
#else
#  ifdef STDC
#    include <errno.h>
#    define zstrerror() strerror(errno)
#  else
#    define zstrerror() "stdio error (consult errno)"
#  endif
#endif

/* provide prototypes for these when building zlib without LFS */
#if !defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0
    ZEXTERN gzFile ZEXPORT gzopen64 OF((const char *, const char *));
    ZEXTERN z_off64_t ZEXPORT gzseek64 OF((gzFile, z_off64_t, int));
    ZEXTERN z_off64_t ZEXPORT gztell64 OF((gzFile));
    ZEXTERN z_off64_t ZEXPORT gzoffset64 OF((gzFile));
#endif

/* default memLevel */
#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif

/* default i/o buffer size -- double this for output when reading */
#define GZBUFSIZE 8192

/* gzip modes, also provide a little integrity check on the passed structure */
#define GZ_NONE 0
#define GZ_READ 7247
#define GZ_WRITE 31153
#define GZ_APPEND 1     /* mode set to GZ_WRITE after the file is opened */

/* values for gz_state how */
#define GS_LOOK 0      /* look for a gzip header */
#define GS_COPY 1      /* copy input directly */
#define GS_GZIP 2      /* decompress a gzip stream */

/* internal gzip file state data structure */
typedef struct {
        /* exposed contents for gzgetc() macro */
    struct gzFile_s x;      /* "x" for exposed */
                            /* x.have: number of bytes available at x.next */
                            /* x.next: next output data to deliver or write */
                            /* x.pos: current position in uncompressed data */
        /* used for both reading and writing */
    int mode;               /* see gzip modes above */
    int fd;                 /* file descriptor */
    char *path;             /* path or fd for error messages */
    unsigned size;          /* buffer size, zero if not allocated yet */
    unsigned want;          /* requested buffer size, default is GZBUFSIZE */
    unsigned char *in;      /* input buffer */
    unsigned char *out;     /* output buffer (double-sized when reading) */
    int direct;             /* 0 if processing gzip, 1 if transparent */
        /* just for reading */
    int how;                /* 0: get header, 1: copy, 2: decompress */
    z_off64_t start;        /* where the gzip data started, for rewinding */
    int eof;                /* true if end of input file reached */
    int past;               /* true if read requested past end */
        /* just for writing */
    int level;              /* compression level */
    int strategy;           /* compression strategy */
        /* seek request */
    z_off64_t skip;         /* amount to skip (already rewound if backwards) */
    int seek;               /* true if seek request pending */
        /* error information */
    int err;                /* error code */
    char *msg;              /* error message */
        /* zlib inflate or deflate stream */
    z_stream strm;          /* stream structure in-place (not a pointer) */
} gz_state;
typedef gz_state FAR *gz_statep;

/* shared functions */
void ZLIB_INTERNAL gz_error OF((gz_statep, int, const char *));
#if defined UNDER_CE
char ZLIB_INTERNAL *gz_strwinerror OF((DWORD error));
#endif

/* GT_OFF(x), where x is an unsigned value, is true if x > maximum z_off64_t
   value -- needed when comparing unsigned to z_off64_t, which is signed
   (possible z_off64_t types off_t, off64_t, and long are all signed) */
#ifdef INT_MAX
#  define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > INT_MAX)
#else
unsigned ZLIB_INTERNAL gz_intmax OF((void));
#  define GT_OFF(x) (sizeof(int) == sizeof(z_off64_t) && (x) > gz_intmax())
#endif

#if _WIN32

#endif
#endif /* GZGUTS_H */

/************************************************************************/
/*
    Start of file "src/inftrees.h"
 */
/************************************************************************/

/* inftrees.h -- header to use inftrees.c
 * Copyright (C) 1995-2005, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#ifndef INFTREES_H 
#define INFTREES_H 

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* Structure for decoding tables.  Each entry provides either the
   information needed to do the operation requested by the code that
   indexed that table entry, or it provides a pointer to another
   table that indexes more bits of the code.  op indicates whether
   the entry is a pointer to another table, a literal, a length or
   distance, an end-of-block, or an invalid code.  For a table
   pointer, the low four bits of op is the number of index bits of
   that table.  For a length or distance, the low four bits of op
   is the number of extra bits to get after the code.  bits is
   the number of bits in this code or part of the code to drop off
   of the bit buffer.  val is the actual byte to output in the case
   of a literal, the base length or distance, or the offset from
   the current table to the next table.  Each entry is four bytes. */
typedef struct {
    unsigned char op;           /* operation, extra bits, table bits */
    unsigned char bits;         /* bits in this part of the code */
    unsigned short val;         /* offset in table or code value */
} code;

/* op values as set by inflate_table():
    00000000 - literal
    0000tttt - table link, tttt != 0 is the number of table index bits
    0001eeee - length or distance, eeee is the number of extra bits
    01100000 - end of block
    01000000 - invalid code
 */

/* Maximum size of the dynamic table.  The maximum number of code structures is
   1444, which is the sum of 852 for literal/length codes and 592 for distance
   codes.  These values were found by exhaustive searches using the program
   examples/enough.c found in the zlib distribtution.  The arguments to that
   program are the number of symbols, the initial root table size, and the
   maximum bit length of a code.  "enough 286 9 15" for literal/length codes
   returns returns 852, and "enough 30 6 15" for distance codes returns 592.
   The initial root table size (9 or 6) is found in the fifth argument of the
   inflate_table() calls in inflate.c and infback.c.  If the root table size is
   changed, then these maximum sizes would be need to be recalculated and
   updated. */
#define ENOUGH_LENS 852
#define ENOUGH_DISTS 592
#define ENOUGH (ENOUGH_LENS+ENOUGH_DISTS)

/* Type of code to build for inflate_table() */
typedef enum {
    CODES,
    LENS,
    DISTS
} codetype;

int ZLIB_INTERNAL inflate_table OF((codetype type, unsigned short FAR *lens,
                             unsigned codes, code FAR * FAR *table,
                             unsigned FAR *bits, unsigned short FAR *work));
#endif /* INFTREES_H  */

/************************************************************************/
/*
    Start of file "src/inffast.h"
 */
/************************************************************************/

/* inffast.h -- header to use inffast.c
 * Copyright (C) 1995-2003, 2010 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

#ifndef INFFAST_H 
#define INFFAST_H 
void ZLIB_INTERNAL inflate_fast OF((z_streamp strm, unsigned start));
#endif /* INFFAST_H */

/************************************************************************/
/*
    Start of file "src/inffixed.h"
 */
/************************************************************************/

    /* inffixed.h -- table for decoding fixed codes
     * Generated automatically by makefixed().
     */
#ifndef INFFIXED_H 
#define INFFIXED_H 

    /* WARNING: this file should *not* be used by applications.
       It is part of the implementation of this library and is
       subject to change. Applications should only use zlib.h.
     */

    static const code lenfix[512] = {
        {96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
        {0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
        {0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
        {0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
        {0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
        {21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
        {0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
        {0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
        {18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
        {0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
        {0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
        {0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
        {20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
        {0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
        {0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
        {0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
        {16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
        {0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
        {0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
        {0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
        {0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
        {0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
        {0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
        {0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
        {17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
        {0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
        {0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
        {0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
        {19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
        {0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
        {0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
        {0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
        {16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
        {0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
        {0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
        {0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
        {0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
        {20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
        {0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
        {0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
        {17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
        {0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
        {0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
        {0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
        {20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
        {0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
        {0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
        {0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
        {16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
        {0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
        {0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
        {0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
        {0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
        {0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
        {0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
        {0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
        {16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
        {0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
        {0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
        {0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
        {19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
        {0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
        {0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
        {0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
        {16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
        {0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
        {0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
        {0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
        {0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
        {64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
        {0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
        {0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
        {18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
        {0,9,255}
    };

    static const code distfix[32] = {
        {16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
        {21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
        {18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
        {19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
        {16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
        {22,5,193},{64,5,0}
    };
#endif /* INFFIXED_H  */

/************************************************************************/
/*
    Start of file "src/inflate.h"
 */
/************************************************************************/

/* inflate.h -- internal inflate state definition
 * Copyright (C) 1995-2009 Mark Adler
 * For conditions of distribution and use, see copyright notice in zlib.h
 */
#ifndef INFLATE_H 
#define INFLATE_H 

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer decoding by inflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip decoding
   should be left enabled. */
#ifndef NO_GZIP
#  define GUNZIP
#endif

/* Possible inflate modes between inflate() calls */
typedef enum {
    HEAD,       /* i: waiting for magic header */
    FLAGS,      /* i: waiting for method and flags (gzip) */
    TIME,       /* i: waiting for modification time (gzip) */
    OS,         /* i: waiting for extra flags and operating system (gzip) */
    EXLEN,      /* i: waiting for extra length (gzip) */
    EXTRA,      /* i: waiting for extra bytes (gzip) */
    NAME,       /* i: waiting for end of file name (gzip) */
    COMMENT,    /* i: waiting for end of comment (gzip) */
    HCRC,       /* i: waiting for header crc (gzip) */
    DICTID,     /* i: waiting for dictionary check value */
    DICT,       /* waiting for inflateSetDictionary() call */
        TYPE,       /* i: waiting for type bits, including last-flag bit */
        TYPEDO,     /* i: same, but skip check to exit inflate on new block */
        STORED,     /* i: waiting for stored size (length and complement) */
        COPY_,      /* i/o: same as COPY below, but only first time in */
        COPY,       /* i/o: waiting for input or output to copy stored block */
        TABLE,      /* i: waiting for dynamic block table lengths */
        LENLENS,    /* i: waiting for code length code lengths */
        CODELENS,   /* i: waiting for length/lit and distance code lengths */
            LEN_,       /* i: same as LEN below, but only first time in */
            LEN,        /* i: waiting for length/lit/eob code */
            LENEXT,     /* i: waiting for length extra bits */
            DIST,       /* i: waiting for distance code */
            DISTEXT,    /* i: waiting for distance extra bits */
            MATCH,      /* o: waiting for output space to copy string */
            LIT,        /* o: waiting for output space to write literal */
    CHECK,      /* i: waiting for 32-bit check value */
    LENGTH,     /* i: waiting for 32-bit length (gzip) */
    DONE,       /* finished check, done -- remain here until reset */
    BAD,        /* got a data error -- remain here until reset */
    MEM,        /* got an inflate() memory error -- remain here until reset */
    SYNC        /* looking for synchronization bytes to restart inflate() */
} inflate_mode;

/*
    State transitions between above modes -

    (most modes can go to BAD or MEM on error -- not shown for clarity)

    Process header:
        HEAD -> (gzip) or (zlib) or (raw)
        (gzip) -> FLAGS -> TIME -> OS -> EXLEN -> EXTRA -> NAME -> COMMENT ->
                  HCRC -> TYPE
        (zlib) -> DICTID or TYPE
        DICTID -> DICT -> TYPE
        (raw) -> TYPEDO
    Read deflate blocks:
            TYPE -> TYPEDO -> STORED or TABLE or LEN_ or CHECK
            STORED -> COPY_ -> COPY -> TYPE
            TABLE -> LENLENS -> CODELENS -> LEN_
            LEN_ -> LEN
    Read deflate codes in fixed or dynamic block:
                LEN -> LENEXT or LIT or TYPE
                LENEXT -> DIST -> DISTEXT -> MATCH -> LEN
                LIT -> LEN
    Process trailer:
        CHECK -> LENGTH -> DONE
 */

/* state maintained between inflate() calls.  Approximately 10K bytes. */
struct inflate_state {
    inflate_mode mode;          /* current inflate mode */
    int last;                   /* true if processing last block */
    int wrap;                   /* bit 0 true for zlib, bit 1 true for gzip */
    int havedict;               /* true if dictionary provided */
    int flags;                  /* gzip header method and flags (0 if zlib) */
    unsigned dmax;              /* zlib header max distance (INFLATE_STRICT) */
    unsigned long check;        /* protected copy of check value */
    unsigned long total;        /* protected copy of output count */
    gz_headerp head;            /* where to save gzip header information */
        /* sliding window */
    unsigned wbits;             /* log base 2 of requested window size */
    unsigned wsize;             /* window size or zero if not using window */
    unsigned whave;             /* valid bytes in the window */
    unsigned wnext;             /* window write index */
    unsigned char FAR *window;  /* allocated sliding window, if needed */
        /* bit accumulator */
    unsigned long hold;         /* input bit accumulator */
    unsigned bits;              /* number of bits in "in" */
        /* for string and stored block copying */
    unsigned length;            /* literal or length of data to copy */
    unsigned offset;            /* distance back to copy string from */
        /* for table and code decoding */
    unsigned extra;             /* extra bits needed */
        /* fixed and dynamic code tables */
    code const FAR *lencode;    /* starting table for length/literal codes */
    code const FAR *distcode;   /* starting table for distance codes */
    unsigned lenbits;           /* index bits for lencode */
    unsigned distbits;          /* index bits for distcode */
        /* dynamic table building */
    unsigned ncode;             /* number of code length code lengths */
    unsigned nlen;              /* number of length code lengths */
    unsigned ndist;             /* number of distance code lengths */
    unsigned have;              /* number of code lengths in lens[] */
    code FAR *next;             /* next available space in codes[] */
    unsigned short lens[320];   /* temporary storage for code lengths */
    unsigned short work[288];   /* work area for code table building */
    code codes[ENOUGH];         /* space for code tables */
    int sane;                   /* if false, allow invalid distance too far */
    int back;                   /* bits back of last unprocessed length/lit */
    unsigned was;               /* initial length of match */
};
#endif /* INFLATE_H  */

/************************************************************************/
/*
    Start of file "src/zutil.h"
 */
/************************************************************************/

/* zutil.h -- internal interface and configuration of the compression library
 * Copyright (C) 1995-2011 Jean-loup Gailly.
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ifndef ZUTIL_H
#define ZUTIL_H

#ifndef ZLIB_INTERNAL
#if ((__GNUC__-0) * 10 + __GNUC_MINOR__-0 >= 33) && !defined(NO_VIZ)
#  define ZLIB_INTERNAL __attribute__((visibility ("hidden")))
#else
#  define ZLIB_INTERNAL
#endif
#endif



#if defined(STDC) && !defined(Z_SOLO)
#  if !(defined(_WIN32_WCE) && defined(_MSC_VER))
#    include <stddef.h>
#  endif
#  include <string.h>
#  include <stdlib.h>
#endif

#ifdef Z_SOLO
   typedef long ptrdiff_t;  /* guess -- will be caught if guess is wrong */
#endif

#ifndef local
#  define local static
#endif
/* compile with -Dlocal if your debugger can't find static symbols */

typedef unsigned char  uch;
typedef uch FAR uchf;
typedef unsigned short ush;
typedef ush FAR ushf;
typedef unsigned long  ulg;

extern const char * const z_errmsg[10]; /* indexed by 2-zlib_error */
/* (size given to avoid silly warnings with Visual C++) */

#define ERR_MSG(err) z_errmsg[Z_NEED_DICT-(err)]

#define ERR_RETURN(strm,err) \
  return (strm->msg = (char*)ERR_MSG(err), (err))
/* To be used only when the state is known to be valid */

        /* common constants */

#ifndef DEF_WBITS
#  define DEF_WBITS MAX_WBITS
#endif
/* default windowBits for decompression. MAX_WBITS is for compression only */

#if MAX_MEM_LEVEL >= 8
#  define DEF_MEM_LEVEL 8
#else
#  define DEF_MEM_LEVEL  MAX_MEM_LEVEL
#endif
/* default memLevel */

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

#define MIN_MATCH  3
#define MAX_MATCH  258
/* The minimum and maximum match lengths */

#define PRESET_DICT 0x20 /* preset dictionary flag in zlib header */

        /* target dependencies */

#if defined(MSDOS) || (defined(WINDOWS) && !defined(WIN32))
#  define OS_CODE  0x00
#  ifndef Z_SOLO
#    if defined(__TURBOC__) || defined(__BORLANDC__)
#      if (__STDC__ == 1) && (defined(__LARGE__) || defined(__COMPACT__))
         /* Allow compilation with ANSI keywords only enabled */
         void _Cdecl farfree( void *block );
         void *_Cdecl farmalloc( unsigned long nbytes );
#      else
#        include <alloc.h>
#      endif
#    else /* MSC or DJGPP */
#      include <malloc.h>
#    endif
#  endif
#endif

#ifdef AMIGA
#  define OS_CODE  0x01
#endif

#if defined(VAXC) || defined(VMS)
#  define OS_CODE  0x02
#  define F_OPEN(name, mode) \
     fopen((name), (mode), "mbc=60", "ctx=stm", "rfm=fix", "mrs=512")
#endif

#if defined(ATARI) || defined(atarist)
#  define OS_CODE  0x05
#endif

#ifdef OS2
#  define OS_CODE  0x06
#  if defined(M_I86) && !defined(Z_SOLO)
#    include <malloc.h>
#  endif
#endif

#if defined(MACOS) || defined(TARGET_OS_MAC)
#  define OS_CODE  0x07
#  ifndef Z_SOLO
#    if defined(__MWERKS__) && __dest_os != __be_os && __dest_os != __win32_os
#      include <unix.h> /* for fdopen */
#    else
#      ifndef fdopen
#        define fdopen(fd,mode) NULL /* No fdopen() */
#      endif
#    endif
#  endif
#endif

#ifdef TOPS20
#  define OS_CODE  0x0a
#endif

#ifdef WIN32
#  ifndef __CYGWIN__  /* Cygwin is Unix, not Win32 */
#    define OS_CODE  0x0b
#  endif
#endif

#ifdef __50SERIES /* Prime/PRIMOS */
#  define OS_CODE  0x0f
#endif

#if defined(_BEOS_) || defined(RISCOS)
#  define fdopen(fd,mode) NULL /* No fdopen() */
#endif

#if (defined(_MSC_VER) && (_MSC_VER > 600)) && !defined __INTERIX
#  if defined(_WIN32_WCE)
#    define fdopen(fd,mode) NULL /* No fdopen() */
#    ifndef _PTRDIFF_T_DEFINED
       typedef int ptrdiff_t;
#      define _PTRDIFF_T_DEFINED
#    endif
#  else
#    define fdopen(fd,type)  _fdopen(fd,type)
#  endif
#endif

#if defined(__BORLANDC__) && !defined(MSDOS)
  #pragma warn -8004
  #pragma warn -8008
  #pragma warn -8066
#endif

/* provide prototypes for these when building zlib without LFS */
#if !defined(_WIN32) && (!defined(_LARGEFILE64_SOURCE) || _LFS64_LARGEFILE-0 == 0)
    ZEXTERN uLong ZEXPORT adler32_combine64 OF((uLong, uLong, z_off_t));
    ZEXTERN uLong ZEXPORT crc32_combine64 OF((uLong, uLong, z_off_t));
#endif

        /* common defaults */

#ifndef OS_CODE
#  define OS_CODE  0x03  /* assume Unix */
#endif

#ifndef F_OPEN
#  define F_OPEN(name, mode) fopen((name), (mode))
#endif

         /* functions */

#if defined(pyr) || defined(Z_SOLO)
#  define NO_MEMCPY
#endif
#if defined(SMALL_MEDIUM) && !defined(_MSC_VER) && !defined(__SC__)
 /* Use our own functions for small and medium model with MSC <= 5.0.
  * You may have to use the same strategy for Borland C (untested).
  * The __SC__ check is for Symantec.
  */
#  define NO_MEMCPY
#endif
#if defined(STDC) && !defined(HAVE_MEMCPY) && !defined(NO_MEMCPY)
#  define HAVE_MEMCPY
#endif
#ifdef HAVE_MEMCPY
#  ifdef SMALL_MEDIUM /* MSDOS small or medium model */
#    define zmemcpy _fmemcpy
#    define zmemcmp _fmemcmp
#    define zmemzero(dest, len) _fmemset(dest, 0, len)
#  else
#    define zmemcpy memcpy
#    define zmemcmp memcmp
#    define zmemzero(dest, len) memset(dest, 0, len)
#  endif
#else
   void ZLIB_INTERNAL zmemcpy OF((Bytef* dest, const Bytef* source, uInt len));
   int ZLIB_INTERNAL zmemcmp OF((const Bytef* s1, const Bytef* s2, uInt len));
   void ZLIB_INTERNAL zmemzero OF((Bytef* dest, uInt len));
#endif

/* Diagnostic functions */
#ifdef DEBUG
#  include <stdio.h>
   extern int ZLIB_INTERNAL z_verbose;
   extern void ZLIB_INTERNAL z_error OF((char *m));
#  define Assert(cond,msg) {if(!(cond)) z_error(msg);}
#  define Trace(x) {if (z_verbose>=0) fprintf x ;}
#  define Tracev(x) {if (z_verbose>0) fprintf x ;}
#  define Tracevv(x) {if (z_verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (z_verbose>0 && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (z_verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

#ifndef Z_SOLO
   voidpf ZLIB_INTERNAL zcalloc OF((voidpf opaque, unsigned items,
                                    unsigned size));
   void ZLIB_INTERNAL zcfree  OF((voidpf opaque, voidpf ptr));
#endif

#define ZALLOC(strm, items, size) \
           (*((strm)->zalloc))((strm)->opaque, (items), (size))
#define ZFREE(strm, addr)  (*((strm)->zfree))((strm)->opaque, (voidpf)(addr))
#define TRY_FREE(s, p) {if (p) ZFREE(s, p);}

#endif /* ZUTIL_H */

/************************************************************************/
/*
    Start of file "src/deflate.h"
 */
/************************************************************************/

/* deflate.h -- internal compression state
 * Copyright (C) 1995-2012 Jean-loup Gailly
 * For conditions of distribution and use, see copyright notice in zlib.h
 */

/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the compression library and is
   subject to change. Applications should only use zlib.h.
 */

/* @(#) $Id$ */

#ifndef DEFLATE_H
#define DEFLATE_H



/* define NO_GZIP when compiling if you want to disable gzip header and
   trailer creation by deflate().  NO_GZIP would be used to avoid linking in
   the crc code when it is not needed.  For shared libraries, gzip encoding
   should be left enabled. */
#ifndef NO_GZIP
#  define GZIP
#endif

/* ===========================================================================
 * Internal compression state.
 */

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define Buf_size 16
/* size of bit buffer in bi_buf */

#define INIT_STATE    42
#define EXTRA_STATE   69
#define NAME_STATE    73
#define COMMENT_STATE 91
#define HCRC_STATE   103
#define BUSY_STATE   113
#define FINISH_STATE 666
/* Stream status */


/* Data structure describing a single value and its code string. */
typedef struct ct_data_s {
    union {
        ush  freq;       /* frequency count */
        ush  code;       /* bit string */
    } fc;
    union {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} FAR ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

typedef struct static_tree_desc_s  static_tree_desc;

typedef struct tree_desc_s {
    ct_data *dyn_tree;           /* the dynamic tree */
    int     max_code;            /* largest code with non zero frequency */
    static_tree_desc *stat_desc; /* the corresponding static tree */
} FAR tree_desc;

typedef ush Pos;
typedef Pos FAR Posf;
typedef unsigned IPos;

/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

typedef struct internal_state {
    z_streamp strm;      /* pointer back to this zlib stream */
    int   status;        /* as the name implies */
    Bytef *pending_buf;  /* output still pending */
    ulg   pending_buf_size; /* size of pending_buf */
    Bytef *pending_out;  /* next pending byte to output to the stream */
    uInt   pending;      /* nb of bytes in the pending buffer */
    int   wrap;          /* bit 0 true for zlib, bit 1 true for gzip */
    gz_headerp  gzhead;  /* gzip header information to write */
    uInt   gzindex;      /* where in extra, name, or comment */
    Byte  method;        /* STORED (for zip only) or DEFLATED */
    int   last_flush;    /* value of flush param for previous deflate call */

                /* used by deflate.c: */

    uInt  w_size;        /* LZ77 window size (32K by default) */
    uInt  w_bits;        /* log2(w_size)  (8..16) */
    uInt  w_mask;        /* w_size - 1 */

    Bytef *window;
    /* Sliding window. Input bytes are read into the second half of the window,
     * and move to the first half later to keep a dictionary of at least wSize
     * bytes. With this organization, matches are limited to a distance of
     * wSize-MAX_MATCH bytes, but this ensures that IO is always
     * performed with a length multiple of the block size. Also, it limits
     * the window size to 64K, which is quite useful on MSDOS.
     * To do: use the user input buffer as sliding window.
     */

    ulg window_size;
    /* Actual size of window: 2*wSize, except when the user input buffer
     * is directly used as sliding window.
     */

    Posf *prev;
    /* Link to older string with same hash index. To limit the size of this
     * array to 64K, this link is maintained only for the last 32K strings.
     * An index in this array is thus a window index modulo 32K.
     */

    Posf *head; /* Heads of the hash chains or NIL. */

    uInt  ins_h;          /* hash index of string to be inserted */
    uInt  hash_size;      /* number of elements in hash table */
    uInt  hash_bits;      /* log2(hash_size) */
    uInt  hash_mask;      /* hash_size-1 */

    uInt  hash_shift;
    /* Number of bits by which ins_h must be shifted at each input
     * step. It must be such that after MIN_MATCH steps, the oldest
     * byte no longer takes part in the hash key, that is:
     *   hash_shift * MIN_MATCH >= hash_bits
     */

    long block_start;
    /* Window position at the beginning of the current output block. Gets
     * negative when the window is moved backwards.
     */

    uInt match_length;           /* length of best match */
    IPos prev_match;             /* previous match */
    int match_available;         /* set if previous match exists */
    uInt strstart;               /* start of string to insert */
    uInt match_start;            /* start of matching string */
    uInt lookahead;              /* number of valid bytes ahead in window */

    uInt prev_length;
    /* Length of the best match at previous step. Matches not greater than this
     * are discarded. This is used in the lazy match evaluation.
     */

    uInt max_chain_length;
    /* To speed up deflation, hash chains are never searched beyond this
     * length.  A higher limit improves compression ratio but degrades the
     * speed.
     */

    uInt max_lazy_match;
    /* Attempt to find a better match only when the current match is strictly
     * smaller than this value. This mechanism is used only for compression
     * levels >= 4.
     */
#   define max_insert_length  max_lazy_match
    /* Insert new strings in the hash table only if the match length is not
     * greater than this length. This saves time but degrades compression.
     * max_insert_length is used only for compression levels <= 3.
     */

    int level;    /* compression level (1..9) */
    int strategy; /* favor or force Huffman coding*/

    uInt good_match;
    /* Use a faster search when the previous match is longer than this */

    int nice_match; /* Stop searching when current match exceeds this */

                /* used by trees.c: */
    /* Didn't use ct_data typedef below to suppress compiler warning */
    struct ct_data_s dyn_ltree[HEAP_SIZE];   /* literal and length tree */
    struct ct_data_s dyn_dtree[2*D_CODES+1]; /* distance tree */
    struct ct_data_s bl_tree[2*BL_CODES+1];  /* Huffman tree for bit lengths */

    struct tree_desc_s l_desc;               /* desc. for literal tree */
    struct tree_desc_s d_desc;               /* desc. for distance tree */
    struct tree_desc_s bl_desc;              /* desc. for bit length tree */

    ush bl_count[MAX_BITS+1];
    /* number of codes at each bit length for an optimal tree */

    int heap[2*L_CODES+1];      /* heap used to build the Huffman trees */
    int heap_len;               /* number of elements in the heap */
    int heap_max;               /* element of largest frequency */
    /* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
     * The same heap array is used to build all trees.
     */

    uch depth[2*L_CODES+1];
    /* Depth of each subtree used as tie breaker for trees of equal frequency
     */

    uchf *l_buf;          /* buffer for literals or lengths */

    uInt  lit_bufsize;
    /* Size of match buffer for literals/lengths.  There are 4 reasons for
     * limiting lit_bufsize to 64K:
     *   - frequencies can be kept in 16 bit counters
     *   - if compression is not successful for the first block, all input
     *     data is still in the window so we can still emit a stored block even
     *     when input comes from standard input.  (This can also be done for
     *     all blocks if lit_bufsize is not greater than 32K.)
     *   - if compression is not successful for a file smaller than 64K, we can
     *     even emit a stored file instead of a stored block (saving 5 bytes).
     *     This is applicable only for zip (not gzip or zlib).
     *   - creating new Huffman trees less frequently may not provide fast
     *     adaptation to changes in the input data statistics. (Take for
     *     example a binary file with poorly compressible code followed by
     *     a highly compressible string table.) Smaller buffer sizes give
     *     fast adaptation but have of course the overhead of transmitting
     *     trees more frequently.
     *   - I can't count above 4
     */

    uInt last_lit;      /* running index in l_buf */

    ushf *d_buf;
    /* Buffer for distances. To simplify the code, d_buf and l_buf have
     * the same number of elements. To use different lengths, an extra flag
     * array would be necessary.
     */

    ulg opt_len;        /* bit length of current block with optimal trees */
    ulg static_len;     /* bit length of current block with static trees */
    uInt matches;       /* number of string matches in current block */
    uInt insert;        /* bytes at end of window left to insert */

#ifdef DEBUG
    ulg compressed_len; /* total bit length of compressed file mod 2^32 */
    ulg bits_sent;      /* bit length of compressed data sent mod 2^32 */
#endif

    ush bi_buf;
    /* Output buffer. bits are inserted starting at the bottom (least
     * significant bits).
     */
    int bi_valid;
    /* Number of valid bits in bi_buf.  All bits above the last valid bit
     * are always zero.
     */

    ulg high_water;
    /* High water mark offset in window for initialized bytes -- bytes above
     * this are set to zero in order to avoid memory check warnings when
     * longest match routines access bytes past the input.  This is then
     * updated to the new high water mark.
     */

} FAR deflate_state;

/* Output a byte on the stream.
 * IN assertion: there is enough room in pending_buf.
 */
#define put_byte(s, c) {s->pending_buf[s->pending++] = (c);}


#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST(s)  ((s)->w_size-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#define WIN_INIT MAX_MATCH
/* Number of bytes after end of data in window to initialize in order to avoid
   memory checker errors from longest match routines */

        /* in trees.c */
void ZLIB_INTERNAL _tr_init OF((deflate_state *s));
int ZLIB_INTERNAL _tr_tally OF((deflate_state *s, unsigned dist, unsigned lc));
void ZLIB_INTERNAL _tr_flush_block OF((deflate_state *s, charf *buf,
                        ulg stored_len, int last));
void ZLIB_INTERNAL _tr_flush_bits OF((deflate_state *s));
void ZLIB_INTERNAL _tr_align OF((deflate_state *s));
void ZLIB_INTERNAL _tr_stored_block OF((deflate_state *s, charf *buf,
                        ulg stored_len, int last));

#define d_code(dist) \
   ((dist) < 256 ? _dist_code[dist] : _dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. _dist_code[256] and _dist_code[257] are never
 * used.
 */

#ifndef DEBUG
/* Inline versions of _tr_tally for speed: */

#if defined(GEN_TREES_H) || !defined(STDC)
  extern uch ZLIB_INTERNAL _length_code[];
  extern uch ZLIB_INTERNAL _dist_code[];
#else
  extern const uch ZLIB_INTERNAL _length_code[];
  extern const uch ZLIB_INTERNAL _dist_code[];
#endif

# define _tr_tally_lit(s, c, flush) \
  { uch cc = (c); \
    s->d_buf[s->last_lit] = 0; \
    s->l_buf[s->last_lit++] = cc; \
    s->dyn_ltree[cc].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
   }
# define _tr_tally_dist(s, distance, length, flush) \
  { uch len = (length); \
    ush dist = (distance); \
    s->d_buf[s->last_lit] = dist; \
    s->l_buf[s->last_lit++] = len; \
    dist--; \
    s->dyn_ltree[_length_code[len]+LITERALS+1].Freq++; \
    s->dyn_dtree[d_code(dist)].Freq++; \
    flush = (s->last_lit == s->lit_bufsize-1); \
  }
#else
# define _tr_tally_lit(s, c, flush) flush = _tr_tally(s, 0, c)
# define _tr_tally_dist(s, distance, length, flush) \
              flush = _tr_tally(s, distance, length)
#endif

#endif /* DEFLATE_H */

/************************************************************************/
/*
    Start of file "src/ejsByteCode.h"
 */
/************************************************************************/

/*
    ejsByteCode.h - Ejscript VM Byte Code
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_ejsByteCode
#define _h_EJS_ejsByteCode 1


typedef enum EjsOpCode {
    EJS_OP_ADD,
    EJS_OP_ADD_NAMESPACE,
    EJS_OP_ADD_NAMESPACE_REF,
    EJS_OP_AND,
    EJS_OP_ATTENTION,
    EJS_OP_BRANCH_EQ,
    EJS_OP_BRANCH_STRICTLY_EQ,
    EJS_OP_BRANCH_FALSE,
    EJS_OP_BRANCH_GE,
    EJS_OP_BRANCH_GT,
    EJS_OP_BRANCH_LE,
    EJS_OP_BRANCH_LT,
    EJS_OP_BRANCH_NE,
    EJS_OP_BRANCH_STRICTLY_NE,
    EJS_OP_BRANCH_NULL,
    EJS_OP_BRANCH_NOT_ZERO,
    EJS_OP_BRANCH_TRUE,
    EJS_OP_BRANCH_UNDEFINED,
    EJS_OP_BRANCH_ZERO,
    EJS_OP_BRANCH_FALSE_8,
    EJS_OP_BRANCH_TRUE_8,
    EJS_OP_BREAKPOINT,
    EJS_OP_CALL,
    EJS_OP_CALL_GLOBAL_SLOT,
    EJS_OP_CALL_OBJ_SLOT,
    EJS_OP_CALL_THIS_SLOT,
    EJS_OP_CALL_BLOCK_SLOT,
    EJS_OP_CALL_OBJ_INSTANCE_SLOT,
    EJS_OP_CALL_OBJ_STATIC_SLOT,
    EJS_OP_CALL_THIS_STATIC_SLOT,
    EJS_OP_CALL_OBJ_NAME,
    EJS_OP_CALL_SCOPED_NAME,
    EJS_OP_CALL_CONSTRUCTOR,
    EJS_OP_CALL_NEXT_CONSTRUCTOR,
    EJS_OP_CAST,
    EJS_OP_CAST_BOOLEAN,
    EJS_OP_CLOSE_BLOCK,
    EJS_OP_COMPARE_EQ,
    EJS_OP_COMPARE_STRICTLY_EQ,
    EJS_OP_COMPARE_FALSE,
    EJS_OP_COMPARE_GE,
    EJS_OP_COMPARE_GT,
    EJS_OP_COMPARE_LE,
    EJS_OP_COMPARE_LT,
    EJS_OP_COMPARE_NE,
    EJS_OP_COMPARE_STRICTLY_NE,
    EJS_OP_COMPARE_NULL,
    EJS_OP_COMPARE_NOT_ZERO,
    EJS_OP_COMPARE_TRUE,
    EJS_OP_COMPARE_UNDEFINED,
    EJS_OP_COMPARE_ZERO,
    EJS_OP_DEFINE_CLASS,
    EJS_OP_DEFINE_FUNCTION,
    EJS_OP_DELETE_NAME_EXPR,
    EJS_OP_DELETE_SCOPED_NAME_EXPR,
    EJS_OP_DIV,
    EJS_OP_DUP,
    EJS_OP_DUP2,
    EJS_OP_DUP_STACK,
    EJS_OP_END_CODE,
    EJS_OP_END_EXCEPTION,
    EJS_OP_GOTO,
    EJS_OP_GOTO_8,
    EJS_OP_INC,
    EJS_OP_INIT_DEFAULT_ARGS,
    EJS_OP_INIT_DEFAULT_ARGS_8,
    EJS_OP_INST_OF,
    EJS_OP_IS_A,
    EJS_OP_LOAD_0,
    EJS_OP_LOAD_1,
    EJS_OP_LOAD_2,
    EJS_OP_LOAD_3,
    EJS_OP_LOAD_4,
    EJS_OP_LOAD_5,
    EJS_OP_LOAD_6,
    EJS_OP_LOAD_7,
    EJS_OP_LOAD_8,
    EJS_OP_LOAD_9,
    EJS_OP_LOAD_DOUBLE,
    EJS_OP_LOAD_FALSE,
    EJS_OP_LOAD_GLOBAL,
    EJS_OP_LOAD_INT,
    EJS_OP_LOAD_M1,
    EJS_OP_LOAD_NAMESPACE,
    EJS_OP_LOAD_NULL,
    EJS_OP_LOAD_REGEXP,
    EJS_OP_LOAD_STRING,
    EJS_OP_LOAD_THIS,
    EJS_OP_LOAD_THIS_LOOKUP,
    EJS_OP_LOAD_THIS_BASE,
    EJS_OP_LOAD_TRUE,
    EJS_OP_LOAD_UNDEFINED,
    EJS_OP_LOAD_XML,
    EJS_OP_GET_LOCAL_SLOT_0,
    EJS_OP_GET_LOCAL_SLOT_1,
    EJS_OP_GET_LOCAL_SLOT_2,
    EJS_OP_GET_LOCAL_SLOT_3,
    EJS_OP_GET_LOCAL_SLOT_4,
    EJS_OP_GET_LOCAL_SLOT_5,
    EJS_OP_GET_LOCAL_SLOT_6,
    EJS_OP_GET_LOCAL_SLOT_7,
    EJS_OP_GET_LOCAL_SLOT_8,
    EJS_OP_GET_LOCAL_SLOT_9,
    EJS_OP_GET_OBJ_SLOT_0,
    EJS_OP_GET_OBJ_SLOT_1,
    EJS_OP_GET_OBJ_SLOT_2,
    EJS_OP_GET_OBJ_SLOT_3,
    EJS_OP_GET_OBJ_SLOT_4,
    EJS_OP_GET_OBJ_SLOT_5,
    EJS_OP_GET_OBJ_SLOT_6,
    EJS_OP_GET_OBJ_SLOT_7,
    EJS_OP_GET_OBJ_SLOT_8,
    EJS_OP_GET_OBJ_SLOT_9,
    EJS_OP_GET_THIS_SLOT_0,
    EJS_OP_GET_THIS_SLOT_1,
    EJS_OP_GET_THIS_SLOT_2,
    EJS_OP_GET_THIS_SLOT_3,
    EJS_OP_GET_THIS_SLOT_4,
    EJS_OP_GET_THIS_SLOT_5,
    EJS_OP_GET_THIS_SLOT_6,
    EJS_OP_GET_THIS_SLOT_7,
    EJS_OP_GET_THIS_SLOT_8,
    EJS_OP_GET_THIS_SLOT_9,
    EJS_OP_GET_SCOPED_NAME,
    EJS_OP_GET_SCOPED_NAME_EXPR,
    EJS_OP_GET_OBJ_NAME,
    EJS_OP_GET_OBJ_NAME_EXPR,
    EJS_OP_GET_BLOCK_SLOT,
    EJS_OP_GET_GLOBAL_SLOT,
    EJS_OP_GET_LOCAL_SLOT,
    EJS_OP_GET_OBJ_SLOT,
    EJS_OP_GET_THIS_SLOT,
    EJS_OP_GET_TYPE_SLOT,
    EJS_OP_GET_THIS_TYPE_SLOT,
    EJS_OP_IN,
    EJS_OP_LIKE,
    EJS_OP_LOGICAL_NOT,
    EJS_OP_MUL,
    EJS_OP_NEG,
    EJS_OP_NEW,
    EJS_OP_NEW_ARRAY,
    EJS_OP_NEW_OBJECT,
    EJS_OP_NOP,
    EJS_OP_NOT,
    EJS_OP_OPEN_BLOCK,
    EJS_OP_OPEN_WITH,
    EJS_OP_OR,
    EJS_OP_POP,
    EJS_OP_POP_ITEMS,
    EJS_OP_PUSH_CATCH_ARG,
    EJS_OP_PUSH_RESULT,
    EJS_OP_PUT_LOCAL_SLOT_0,
    EJS_OP_PUT_LOCAL_SLOT_1,
    EJS_OP_PUT_LOCAL_SLOT_2,
    EJS_OP_PUT_LOCAL_SLOT_3,
    EJS_OP_PUT_LOCAL_SLOT_4,
    EJS_OP_PUT_LOCAL_SLOT_5,
    EJS_OP_PUT_LOCAL_SLOT_6,
    EJS_OP_PUT_LOCAL_SLOT_7,
    EJS_OP_PUT_LOCAL_SLOT_8,
    EJS_OP_PUT_LOCAL_SLOT_9,
    EJS_OP_PUT_OBJ_SLOT_0,
    EJS_OP_PUT_OBJ_SLOT_1,
    EJS_OP_PUT_OBJ_SLOT_2,
    EJS_OP_PUT_OBJ_SLOT_3,
    EJS_OP_PUT_OBJ_SLOT_4,
    EJS_OP_PUT_OBJ_SLOT_5,
    EJS_OP_PUT_OBJ_SLOT_6,
    EJS_OP_PUT_OBJ_SLOT_7,
    EJS_OP_PUT_OBJ_SLOT_8,
    EJS_OP_PUT_OBJ_SLOT_9,
    EJS_OP_PUT_THIS_SLOT_0,
    EJS_OP_PUT_THIS_SLOT_1,
    EJS_OP_PUT_THIS_SLOT_2,
    EJS_OP_PUT_THIS_SLOT_3,
    EJS_OP_PUT_THIS_SLOT_4,
    EJS_OP_PUT_THIS_SLOT_5,
    EJS_OP_PUT_THIS_SLOT_6,
    EJS_OP_PUT_THIS_SLOT_7,
    EJS_OP_PUT_THIS_SLOT_8,
    EJS_OP_PUT_THIS_SLOT_9,
    EJS_OP_PUT_OBJ_NAME_EXPR,
    EJS_OP_PUT_OBJ_NAME,
    EJS_OP_PUT_SCOPED_NAME,
    EJS_OP_PUT_SCOPED_NAME_EXPR,
    EJS_OP_PUT_BLOCK_SLOT,
    EJS_OP_PUT_GLOBAL_SLOT,
    EJS_OP_PUT_LOCAL_SLOT,
    EJS_OP_PUT_OBJ_SLOT,
    EJS_OP_PUT_THIS_SLOT,
    EJS_OP_PUT_TYPE_SLOT,
    EJS_OP_PUT_THIS_TYPE_SLOT,
    EJS_OP_REM,
    EJS_OP_RETURN,
    EJS_OP_RETURN_VALUE,
    EJS_OP_SAVE_RESULT,
    EJS_OP_SHL,
    EJS_OP_SHR,
    EJS_OP_SPREAD,
    EJS_OP_SUB,
    EJS_OP_SUPER,
    EJS_OP_SWAP,
    EJS_OP_THROW,
    EJS_OP_TYPE_OF,
    EJS_OP_USHR,
    EJS_OP_XOR,
    EJS_OP_CALL_FINALLY,
    EJS_OP_GOTO_FINALLY,
} EjsOpCode;

#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
    Start of file "src/ejsByteCodeTable.h"
 */
/************************************************************************/

/**
    ejsByteCodeTable.h - Master Byte Code Table
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_EJS_BYTECODETABLE_H
#define _h_EJS_BYTECODETABLE_H 1

#ifdef __cplusplus
extern "C" {
#endif

/*
    Stack effect special values
 */
#define EBC_POPN            101             /* Operand 1 specifies the stack change (pop) */

/*
    Operands
 */
#define EBC_NONE            0x0             /* No operands */
#define EBC_BYTE            0x1             /* 8 bit integer */
#define EBC_DOUBLE          0x10            /* 64 bit floating */
#define EBC_NUM             0x40            /* Encoded integer */
#define EBC_STRING          0x80            /* Interned string as an encoded integer*/
#define EBC_GLOBAL          0x100           /* Encode global */
#define EBC_SLOT            0x200           /* Slot number as an encoded integer */
#define EBC_JMP             0x1000          /* 32 bit jump offset */
#define EBC_JMP8            0x2000          /* 8 bit jump offset */
#define EBC_INIT_DEFAULT    0x8000          /* Computed goto table, 32 bit jumps  */
#define EBC_INIT_DEFAULT8   0x10000         /* Computed goto table, 8 bit jumps */
#define EBC_ARGC            0x20000         /* Argument count */
#define EBC_ARGC2           0x40000         /* Argument count * 2 */
#define EBC_ARGC3           0x80000         /* Argument count * 3 */
#define EBC_NEW_ARRAY       0x100000        /* New Array: Argument count * 2, byte code */
#define EBC_NEW_OBJECT      0x200000        /* New Object: Argument count * 3, byte code: attributes * 3 */

typedef struct EjsOptable {
    char    *name;
    int     stackEffect;
    int     args[8];
} EjsOptable;

#if EJS_DEFINE_OPTABLE
/*  
        Opcode string         Stack Effect      Operands, ...                                   
 */      
EjsOptable ejsOptable[] = {
    {   "ADD",                      -1,         { EBC_NONE,                               },},
    {   "ADD_NAMESPACE",             0,         { EBC_STRING,                             },},
    {   "ADD_NAMESPACE_REF",        -1,         { EBC_NONE,                               },},
    {   "AND",                      -1,         { EBC_NONE,                               },},
    {   "ATTENTION",                -1,         { EBC_NONE,                               },},
    {   "BRANCH_EQ",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_EQ",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE",             -1,         { EBC_JMP,                                },},
    {   "BRANCH_GE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_GT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_LT",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_NE",                -1,         { EBC_JMP,                                },},
    {   "BRANCH_STRICTLY_NE",       -1,         { EBC_JMP,                                },},
    {   "BRANCH_NULL",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_NOT_ZERO",          -1,         { EBC_JMP,                                },},
    {   "BRANCH_TRUE",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_UNDEFINED",         -1,         { EBC_JMP,                                },},
    {   "BRANCH_ZERO",              -1,         { EBC_JMP,                                },},
    {   "BRANCH_FALSE_8",           -1,         { EBC_JMP8,                               },},
    {   "BRANCH_TRUE_8",            -1,         { EBC_JMP8,                               },},
    {   "BREAKPOINT",                0,         { EBC_NUM, EBC_STRING,                    },},
    {   "CALL",                     -2,         { EBC_ARGC,                               },},
    {   "CALL_GLOBAL_SLOT",          0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_SLOT",            -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_THIS_SLOT",            0,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_BLOCK_SLOT",           0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_INSTANCE_SLOT",   -1,         { EBC_SLOT, EBC_ARGC,                     },},
    {   "CALL_OBJ_STATIC_SLOT",     -1,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_THIS_STATIC_SLOT",     0,         { EBC_SLOT, EBC_NUM, EBC_ARGC,            },},
    {   "CALL_OBJ_NAME",            -1,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_SCOPED_NAME",          0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CALL_CONSTRUCTOR",          0,         { EBC_ARGC,                               },},
    {   "CALL_NEXT_CONSTRUCTOR",     0,         { EBC_STRING, EBC_STRING, EBC_ARGC,       },},
    {   "CAST",                     -1,         { EBC_NONE,                               },},
    {   "CAST_BOOLEAN",              0,         { EBC_NONE,                               },},
    {   "CLOSE_BLOCK",               0,         { EBC_NONE,                               },},
    {   "COMPARE_EQ",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_EQ",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_FALSE",            -1,         { EBC_NONE,                               },},
    {   "COMPARE_GE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_GT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_LT",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_NE",               -1,         { EBC_NONE,                               },},
    {   "COMPARE_STRICTLY_NE",      -1,         { EBC_NONE,                               },},
    {   "COMPARE_NULL",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_NOT_ZERO",         -1,         { EBC_NONE,                               },},
    {   "COMPARE_TRUE",             -1,         { EBC_NONE,                               },},
    {   "COMPARE_UNDEFINED",        -1,         { EBC_NONE,                               },},
    {   "COMPARE_ZERO",             -1,         { EBC_NONE,                               },},
    {   "DEFINE_CLASS",              0,         { EBC_GLOBAL,                             },},
    {   "DEFINE_FUNCTION",           0,         { EBC_STRING, EBC_STRING,                 },},
    {   "DELETE_NAME_EXPR",         -2,         { EBC_NONE,                               },},
    {   "DELETE_SCOPED_NAME_EXPR",  -1,         { EBC_NONE,                               },},
    {   "DIV",                      -1,         { EBC_NONE,                               },},
    {   "DUP",                       1,         { EBC_NONE,                               },},
    {   "DUP2",                      2,         { EBC_NONE,                               },},
    {   "DUP_STACK",                 1,         { EBC_BYTE,                               },},
    {   "END_CODE",                  0,         { EBC_NONE,                               },},
    {   "END_EXCEPTION",             0,         { EBC_NONE,                               },},
    {   "GOTO",                      0,         { EBC_JMP,                                },},
    {   "GOTO_8",                    0,         { EBC_JMP8,                               },},
    {   "INC",                       0,         { EBC_BYTE,                               },},
    {   "INIT_DEFAULT_ARGS",         0,         { EBC_INIT_DEFAULT,                       },},
    {   "INIT_DEFAULT_ARGS_8",       0,         { EBC_INIT_DEFAULT8,                      },},
    {   "INST_OF",                  -1,         { EBC_NONE,                               },},
    {   "IS_A",                     -1,         { EBC_NONE,                               },},
    {   "LOAD_0",                    1,         { EBC_NONE,                               },},
    {   "LOAD_1",                    1,         { EBC_NONE,                               },},
    {   "LOAD_2",                    1,         { EBC_NONE,                               },},
    {   "LOAD_3",                    1,         { EBC_NONE,                               },},
    {   "LOAD_4",                    1,         { EBC_NONE,                               },},
    {   "LOAD_5",                    1,         { EBC_NONE,                               },},
    {   "LOAD_6",                    1,         { EBC_NONE,                               },},
    {   "LOAD_7",                    1,         { EBC_NONE,                               },},
    {   "LOAD_8",                    1,         { EBC_NONE,                               },},
    {   "LOAD_9",                    1,         { EBC_NONE,                               },},
    {   "LOAD_DOUBLE",               1,         { EBC_DOUBLE,                             },},
    {   "LOAD_FALSE",                1,         { EBC_NONE,                               },},
    {   "LOAD_GLOBAL",               1,         { EBC_NONE,                               },},
    {   "LOAD_INT",                  1,         { EBC_NUM,                                },},
    {   "LOAD_M1",                   1,         { EBC_NONE,                               },},
    {   "LOAD_NAMESPACE",            1,         { EBC_STRING,                             },},
    {   "LOAD_NULL",                 1,         { EBC_NONE,                               },},
    {   "LOAD_REGEXP",               1,         { EBC_STRING,                             },},
    {   "LOAD_STRING",               1,         { EBC_STRING,                             },},
    {   "LOAD_THIS",                 1,         { EBC_NONE,                               },},
    {   "LOAD_THIS_LOOKUP",          1,         { EBC_NONE,                               },},
    {   "LOAD_THIS_BASE",            1,         { EBC_NUM,                                },},
    {   "LOAD_TRUE",                 1,         { EBC_NONE,                               },},
    {   "LOAD_UNDEFINED",            1,         { EBC_NONE,                               },},
    {   "LOAD_XML",                  1,         { EBC_STRING,                             },},
    {   "GET_LOCAL_SLOT_0",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_1",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_2",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_3",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_4",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_5",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_6",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_7",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_8",          1,         { EBC_NONE,                               },},
    {   "GET_LOCAL_SLOT_9",          1,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_0",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_1",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_2",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_3",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_4",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_5",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_6",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_7",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_8",            0,         { EBC_NONE,                               },},
    {   "GET_OBJ_SLOT_9",            0,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_0",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_1",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_2",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_3",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_4",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_5",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_6",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_7",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_8",           1,         { EBC_NONE,                               },},
    {   "GET_THIS_SLOT_9",           1,         { EBC_NONE,                               },},
    {   "GET_SCOPED_NAME",           1,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_SCOPED_NAME_EXPR",      -1,        { EBC_NONE,                               },},
    {   "GET_OBJ_NAME",              0,         { EBC_STRING, EBC_STRING,                 },},
    {   "GET_OBJ_NAME_EXPR",        -2,         { EBC_NONE,                               },},
    {   "GET_BLOCK_SLOT",            1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_GLOBAL_SLOT",           1,         { EBC_SLOT,                               },},
    {   "GET_LOCAL_SLOT",            1,         { EBC_SLOT,                               },},
    {   "GET_OBJ_SLOT",              0,         { EBC_SLOT,                               },},
    {   "GET_THIS_SLOT",             1,         { EBC_SLOT,                               },},
    {   "GET_TYPE_SLOT",             0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "GET_THIS_TYPE_SLOT",        1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "IN",                       -1,         { EBC_NONE,                               },},
    {   "LIKE",                     -1,         { EBC_NONE,                               },},
    {   "LOGICAL_NOT",               0,         { EBC_NONE,                               },},
    {   "MUL",                      -1,         { EBC_NONE,                               },},
    {   "NEG",                       0,         { EBC_NONE,                               },},
    {   "NEW",                       0,         { EBC_NONE,                               },},
    {   "NEW_ARRAY",                 1,         { EBC_GLOBAL, EBC_NEW_ARRAY,              },},
    {   "NEW_OBJECT",                1,         { EBC_GLOBAL, EBC_NEW_OBJECT,             },},
    {   "NOP",                       0,         { EBC_NONE,                               },},
    {   "NOT",                       0,         { EBC_NONE,                               },},
    {   "OPEN_BLOCK",                0,         { EBC_SLOT, EBC_NUM,                      },},
    {   "OPEN_WITH",                 1,         { EBC_NONE,                               },},
    {   "OR",                       -1,         { EBC_NONE,                               },},
    {   "POP",                      -1,         { EBC_NONE,                               },},
    {   "POP_ITEMS",          EBC_POPN,         { EBC_BYTE,                               },},
    {   "PUSH_CATCH_ARG",            1,         { EBC_NONE,                               },},
    {   "PUSH_RESULT",               1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_0",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_1",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_2",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_3",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_4",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_5",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_6",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_7",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_8",         -1,         { EBC_NONE,                               },},
    {   "PUT_LOCAL_SLOT_9",         -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_0",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_1",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_2",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_3",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_4",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_5",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_6",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_7",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_8",           -2,         { EBC_NONE,                               },},
    {   "PUT_OBJ_SLOT_9",           -2,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_0",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_1",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_2",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_3",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_4",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_5",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_6",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_7",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_8",          -1,         { EBC_NONE,                               },},
    {   "PUT_THIS_SLOT_9",          -1,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME_EXPR",        -4,         { EBC_NONE,                               },},
    {   "PUT_OBJ_NAME",             -2,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME",          -1,         { EBC_STRING, EBC_STRING,                 },},
    {   "PUT_SCOPED_NAME_EXPR",     -3,         { EBC_NONE,                               },},
    {   "PUT_BLOCK_SLOT",           -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_GLOBAL_SLOT",          -1,         { EBC_SLOT,                               },},
    {   "PUT_LOCAL_SLOT",           -1,         { EBC_SLOT,                               },},
    {   "PUT_OBJ_SLOT",             -2,         { EBC_SLOT,                               },},
    {   "PUT_THIS_SLOT",            -1,         { EBC_SLOT,                               },},
    {   "PUT_TYPE_SLOT",            -2,         { EBC_SLOT, EBC_NUM,                      },},
    {   "PUT_THIS_TYPE_SLOT",       -1,         { EBC_SLOT, EBC_NUM,                      },},
    {   "REM",                      -1,         { EBC_NONE,                               },},
    {   "RETURN",                    0,         { EBC_NONE,                               },},
    {   "RETURN_VALUE",             -1,         { EBC_NONE,                               },},
    {   "SAVE_RESULT",              -1,         { EBC_NONE,                               },},
    {   "SHL",                      -1,         { EBC_NONE,                               },},
    {   "SHR",                      -1,         { EBC_NONE,                               },},
    {   "SPREAD",                    0,         { EBC_NONE,                               },},
    {   "SUB",                      -1,         { EBC_NONE,                               },},
    {   "SUPER",                     0,         { EBC_NONE,                               },},
    {   "SWAP",                      0,         { EBC_NONE,                               },},
    {   "THROW",                     0,         { EBC_NONE,                               },},
    {   "TYPE_OF",                  -1,         { EBC_NONE,                               },},
    {   "USHR",                     -1,         { EBC_NONE,                               },},
    {   "XOR",                      -1,         { EBC_NONE,                               },},
    {   "CALL_FINALLY",              0,         { EBC_NONE,                               },},
    {   "GOTO_FINALLY",              0,         { EBC_NONE,                               },},
    {   0,                           0,         { EBC_NONE,                               },},
};
#endif /* EJS_DEFINE_OPTABLE */

PUBLIC EjsOptable *ejsGetOptable();

#ifdef __cplusplus
}
#endif

#endif /* _h_EJS_BYTECODETABLE_H */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
    Start of file "src/ejs.h"
 */
/************************************************************************/

/*
    ejs.h - Ejscript header

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_CORE
#define _h_EJS_CORE 1

#include    "mpr.h"
#include    "http.h"




#ifdef __cplusplus
extern "C" {
#endif

/******************************* Tunable Constants ****************************/

#ifndef BIT_XML_MAX_NODE_DEPTH
    #define BIT_XML_MAX_NODE_DEPTH  64
#endif
#ifndef BIT_MAX_EJS_STACK
#if BIT_HAS_MMU
    #define BIT_MAX_EJS_STACK       (1024 * 1024)   /**< Stack size on virtual memory systems */
#else
    #define BIT_MAX_EJS_STACK       (1024 * 32)     /**< Stack size without MMU */
#endif
#endif

/*
    Internal constants
 */
#define EJS_LOTSA_PROP              256             /**< Object with lots of properties. Grow by bigger chunks */
#define EJS_MIN_FRAME_SLOTS         16              /**< Miniumum number of slots for function frames */
#define EJS_NUM_GLOBAL              256             /**< Number of globals slots to pre-create */
#define EJS_ROUND_PROP              16              /**< Rounding for growing properties */

#define EJS_HASH_MIN_PROP           8               /**< Min props to hash */
#define EJS_MAX_COLLISIONS          4               /**< Max intern string collion chain before rehash */
#define EJS_POOL_INACTIVITY_TIMEOUT (60  * 1000)    /**< Prune inactive pooled VMs older than this */
#define EJS_SESSION_TIMER_PERIOD    (60 * 1000)     /**< Timer checks ever minute */
#define EJS_FILE_PERMS              0664            /**< Default file perms */
#define EJS_DIR_PERMS               0775            /**< Default dir perms */

/*
    Sanity constants. Only for sanity checking. Set large enough to never be a
    real limit but low enough to catch some errors in development.
 */
#define EJS_MAX_POOL                (4*1024*1024)   /**< Size of constant pool */
#define EJS_MAX_ARGS                8192            /**< Max number of args */
#define EJS_MAX_LOCALS              (10*1024)       /**< Max number of locals */
#define EJS_MAX_EXCEPTIONS          8192            /**< Max number of exceptions */
#define EJS_MAX_TRAITS              (0x7fff)        /**< Max number of declared properties per block */

/*
    Should not need to change these
 */
#define EJS_INC_ARGS                8               /**< Frame stack increment */
#define EJS_MAX_BASE_CLASSES        256             /**< Max inheritance chain */
#define EJS_DOC_HASH_SIZE           1007            /**< Hash for doc descriptions */

/*
    Compiler constants
 */
#define EC_MAX_INCLUDE              32              /**< Max number of nested includes */
#define EC_LINE_INCR                1024            /**< Growth increment for input lines */
#define EC_TOKEN_INCR               64              /**< Growth increment for tokens */
#define EC_MAX_LOOK_AHEAD           12
#define EC_BUFSIZE                  4096            /**< General buffer size */
#define EC_MAX_ERRORS               25              /**< Max compilation errors before giving up */
#define EC_CODE_BUFSIZE             4096            /**< Initial size of code gen buffer */
#define EC_NUM_PAK_PROP             32              /**< Initial number of properties */

/********************************* Defines ************************************/

#if !DOXYGEN
/*
    Forward declare types
 */
struct Ejs;
struct EjsBlock;
struct EjsDate;
struct EjsFrame;
struct EjsFunction;
struct EjsGC;
struct EjsHelpers;
struct EjsIntern;
struct EjsMem;
struct EjsNames;
struct EjsModule;
struct EjsNamespace;
struct EjsObj;
struct EjsPot;
struct EjsService;
struct EjsString;
struct EjsState;
struct EjsTrait;
struct EjsTraits;
struct EjsType;
struct EjsUri;
struct EjsWorker;
struct EjsXML;
struct EjsVoid;
#endif

/*
    Trait, type, function and property attributes. These are sometimes combined into a single attributes word.
 */
#define EJS_TRAIT_CAST_NULLS            0x1         /**< Property casts nulls */
#define EJS_TRAIT_DELETED               0x2         /**< Property has been deleted */
#define EJS_TRAIT_GETTER                0x4         /**< Property is a getter */
#define EJS_TRAIT_FIXED                 0x8         /**< Property is not configurable */
#define EJS_TRAIT_HIDDEN                0x10        /**< !Enumerable */
#define EJS_TRAIT_INITIALIZED           0x20        /**< Readonly property has been initialized */
#define EJS_TRAIT_READONLY              0x40        /**< !Writable (used for const) */
#define EJS_TRAIT_SETTER                0x80        /**< Property is a settter */
#define EJS_TRAIT_THROW_NULLS           0x100       /**< Property rejects null */
#define EJS_PROP_HAS_VALUE              0x200       /**< Property has a value record */
#define EJS_PROP_NATIVE                 0x400       /**< Property is backed by native code */
#define EJS_PROP_STATIC                 0x800       /**< Class static property */
#define EJS_PROP_ENUMERABLE             0x1000      /**< Property will be enumerable (compiler use only) */
#define EJS_FUN_CONSTRUCTOR             0x2000      /**< Method is a constructor */
#define EJS_FUN_FULL_SCOPE              0x4000      /**< Function needs closure when defined */
#define EJS_FUN_HAS_RETURN              0x8000      /**< Function has a return statement */
#define EJS_FUN_INITIALIZER             0x10000     /**< Type initializer code */
#define EJS_FUN_OVERRIDE                0x20000     /**< Override base type */
#define EJS_FUN_MODULE_INITIALIZER      0x40000     /**< Module initializer */
#define EJS_FUN_REST_ARGS               0x80000     /**< Parameter is a "..." rest */
#define EJS_TRAIT_MASK                  0xFFFFF     /**< Mask of trait attributes */

/*
    These attributes are never stored in EjsTrait but are often passed in "attributes" which is int64
 */
#define EJS_TYPE_CALLS_SUPER            0x100000    /**< Constructor calls super() */
#define EJS_TYPE_HAS_INSTANCE_VARS      0x200000    /**< Type has non-method instance vars (state) */
#define EJS_TYPE_DYNAMIC_INSTANCES      0x400000    /**< Instances are not sealed and can add properties */
#define EJS_TYPE_FINAL                  0x800000    /**< Type can't be subclassed */
#define EJS_TYPE_FIXUP                  0x1000000   /**< Type needs to inherit base types properties */
#define EJS_TYPE_HAS_CONSTRUCTOR        0x2000000   /**< Type has a constructor */
#define EJS_TYPE_HAS_TYPE_INITIALIZER   0x4000000   /**< Type has an initializer */
#define EJS_TYPE_INTERFACE              0x8000000   /**< Type is an interface */

#define EJS_TYPE_OBJ                    0x10000000  /**< Type is using object helpers */
#define EJS_TYPE_POT                    0x20000000  /**< Type is using pot helpers */
#define EJS_TYPE_BLOCK                  0x40000000  /**< Type is using block helpers */
#define EJS_TYPE_MUTABLE                0x80000000  /**< Type is mutable */

#define EJS_TYPE_MUTABLE_INSTANCES      UINT64(0x100000000) /**< Type has mutable instances */
#define EJS_TYPE_IMMUTABLE_INSTANCES    UINT64(0x200000000) /**< Type has immutable instances */
#define EJS_TYPE_VIRTUAL_SLOTS          UINT64(0x400000000) /**< Type is unsing virtual slots */
#define EJS_TYPE_NUMERIC_INDICIES       UINT64(0x800000000) /**< Type is using numeric indicies for properties */

/*
    Interpreter flags
 */
#define EJS_FLAG_EVENT          0x1         /**< Event pending */
#define EJS_FLAG_NO_INIT        0x8         /**< Don't initialize any modules*/
#define EJS_FLAG_DOC            0x40        /**< Load documentation from modules */
#define EJS_FLAG_NOEXIT         0x200       /**< App should service events and not exit */
#define EJS_FLAG_HOSTED         0x400       /**< Interp is hosted in a web server */

#define EJS_STACK_ARG           -1          /* Offset to locate first arg */

/** 
    Configured numeric type
 */
#define BIT_NUM_TYPE double
typedef BIT_NUM_TYPE MprNumber;

/*  
    Sizes (in bytes) of encoded types in a ByteArray
 */
#define EJS_SIZE_BOOLEAN        1
#define EJS_SIZE_SHORT          2
#define EJS_SIZE_INT            4
#define EJS_SIZE_LONG           8
#define EJS_SIZE_DOUBLE         8
#define EJS_SIZE_DATE           8

/*  
    Reserved and system Namespaces
    The empty namespace is special. When seaching for properties, the empty namespace implies to search all namespaces.
    When properties are defined without a namespace, they are defined in the the empty namespace.
 */
#define EJS_EMPTY_NAMESPACE         ""
#define EJS_BLOCK_NAMESPACE         "-block-"
#define EJS_CONSTRUCTOR_NAMESPACE   "-constructor-"
#define EJS_EJS_NAMESPACE           "ejs"
#define EJS_ITERATOR_NAMESPACE      "iterator"
#define EJS_INIT_NAMESPACE          "-initializer-"
#define EJS_INTERNAL_NAMESPACE      "internal"
#define EJS_META_NAMESPACE          "meta"
#define EJS_PRIVATE_NAMESPACE       "private"
#define EJS_PROTECTED_NAMESPACE     "protected"
#define EJS_PROTOTYPE_NAMESPACE     "-prototype-"
#define EJS_PUBLIC_NAMESPACE        "public"
#define EJS_WORKER_NAMESPACE        "ejs.worker"

/*  
    Flags for fast comparison of namespaces
 */
#define EJS_NSP_PRIVATE         0x1
#define EJS_NSP_PROTECTED       0x2

/*  
    When allocating slots, name hashes and traits, we optimize by rounding up allocations
 */
#define EJS_PROP_ROUNDUP(x) (((x) + EJS_ROUND_PROP - 1) / EJS_ROUND_PROP * EJS_ROUND_PROP)

/*  Property enumeration flags
 */
#define EJS_FLAGS_ENUM_INHERITED 0x1            /**< Enumerate inherited base classes */
#define EJS_FLAGS_ENUM_ALL      0x2             /**< Enumerate non-enumerable and fixture properties */

/*  
    Ejscript return codes.
 */
#define EJS_SUCCESS             MPR_ERR_OK
#define EJS_ERR                 MPR_ERR
#define EJS_EXCEPTION           (MPR_ERR_MAX - 1)

/*  
    Convenient slot aliases
 */
#define EJSLOT_CONSTRUCTOR          EJSLOT_Object___constructor__

/*  
    Default names and extensions
 */
#define EJS_GLOBAL                  "global"
#define EJS_DEFAULT_MODULE          "default"
#define EJS_DEFAULT_MODULE_NAME     EJS_DEFAULT_MODULE EJS_MODULE_EXT
#define EJS_DEFAULT_CLASS_NAME      "__defaultClass__"
#define EJS_INITIALIZER_NAME        "__initializer__"

#define EJS_NAME                    "ejs"
#define EJS_MOD                     "ejs.mod"
#define EJS_MODULE_EXT              ".mod"
#define EJS_SOURCE_EXT              ".es"
#define EJS_LISTING_EXT             ".lst"

/************************************************ EjsObj ************************************************/

#define EJS_SHIFT_VISITED       0
#define EJS_SHIFT_DYNAMIC       1
#define EJS_SHIFT_TYPE          0

#define EJS_MASK_VISITED        0x1
#define EJS_MASK_DYNAMIC        0x2
#define EJS_MASK_TYPE           ~(EJS_MASK_VISITED | EJS_MASK_DYNAMIC)

#define DYNAMIC(obj)            ((int) ((((EjsObj*) obj)->xtype) & EJS_MASK_DYNAMIC))
#define VISITED(obj)            ((int) ((((EjsObj*) obj)->xtype) & EJS_MASK_VISITED))
#define TYPE(obj)               ((EjsType*) ((((EjsObj*) obj)->xtype) & EJS_MASK_TYPE))
#define SET_VISITED(obj, value) ((EjsObj*) obj)->xtype = \
                                    ((value) << EJS_SHIFT_VISITED) | (((EjsObj*) obj)->xtype & ~EJS_MASK_VISITED)
#define SET_DYNAMIC(obj, value) ((EjsObj*) obj)->xtype = \
                                    (((size_t) value) << EJS_SHIFT_DYNAMIC) | (((EjsObj*) obj)->xtype & ~EJS_MASK_DYNAMIC)
#if BIT_DEBUG
    #define SET_TYPE_NAME(obj, t) \
    if (1) { \
        if (t && ((EjsType*) t)->qname.name) { \
            ((EjsObj*) obj)->kind = ((EjsType*) t)->qname.name->value; \
        } \
        ((EjsObj*) obj)->type = ((EjsType*) t); \
    } else
#else
    #define SET_TYPE_NAME(obj, type)
#endif

#define SET_TYPE(obj, value) \
    if (1) { \
        ((EjsObj*) obj)->xtype = \
            (((size_t) value) << EJS_SHIFT_TYPE) | (((EjsObj*) obj)->xtype & ~EJS_MASK_TYPE); \
        SET_TYPE_NAME(obj, value); \
    } else

typedef void EjsAny;

#if BIT_DEBUG
    #define ejsSetMemRef(obj) if (1) { ((EjsObj*) obj)->mem = MPR_GET_MEM(obj); } else 
#else
    #define ejsSetMemRef(obj) 
#endif

/************************************************* Names ************************************************/
/**
    Qualified name structure
    @description All names in Ejscript consist of a property name and a name space. Namespaces provide discrete
        spaces to manage and minimize name conflicts. These names will soon be converted to unicode.
    @defgroup EjsName EjsName
    @see EjsName ejsMarkName
    @stability Internal
 */       
typedef struct EjsName {
    struct EjsString *name;                          /**< Property name */
    struct EjsString *space;                         /**< Property namespace */
} EjsName;

/**
    Mark a name for GC
    @param qname Qualified name reference
    @ingroup EjsName
 */
PUBLIC void ejsMarkName(EjsName *qname);

/**
    Initialize a Qualified Name structure
    @description Initialize the statically allocated qualified name structure using a name and namespace.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */
PUBLIC EjsName ejsName(struct Ejs *ejs, cchar *space, cchar *name);

/**
    Initialize a Qualified Name structure using a wide namespace and name
    @description Initialize the statically allocated qualified name structure using a name and namespace.
        @param ejs Interpreter instance returned from #ejsCreateVM
    @param space Namespace string
    @param name Name string
    @return A reference to the qname structure
    @ingroup EjsName
 */
PUBLIC EjsName ejsWideName(struct Ejs *ejs, wchar *space, wchar *name);

/*
    Internal
 */
PUBLIC EjsName ejsEmptyWideName(struct Ejs *ejs, wchar *name);
PUBLIC EjsName ejsEmptyName(struct Ejs *ejs, cchar *name);

#define WEN(name) ejsEmptyWideName(ejs, name)
#define EN(name) ejsEmptyName(ejs, name)
#define N(space, name) ejsName(ejs, space, name)
#define WN(space, name) ejsWideName(ejs, space, name)

/********************************************** Special Values ******************************************/
/*
    Immutable object slot definitions
 */
#define S_Array ES_Array
#define S_Block ES_Block
#define S_Boolean ES_Boolean
#define S_ByteArray ES_ByteArray
#define S_Config ES_Config
#define S_Date ES_Date
#define S_Error ES_Error
#define S_ErrorEvent ES_ErrorEvent
#define S_Event ES_Event
#define S_File ES_File
#define S_FileSystem ES_FileSystem
#define S_Frame ES_Frame
#define S_Function ES_Function
#define S_Http ES_Http
#define S_Namespace ES_Namespace
#define S_Null ES_Null
#define S_Number ES_Number
#define S_Object ES_Object
#define S_Path ES_Path
#define S_RegExp ES_RegExp
#define S_String ES_String
#define S_Type ES_Type
#define S_Uri ES_Uri
#define S_Void ES_Void
#define S_Worker ES_Worker
#define S_XML ES_XML
#define S_XMLList ES_XMLList
#define S_commaProt ES_commaProt
#define S_empty ES_empty
#define S_false ES_false
#define S_global ES_global
#define S_iterator ES_iterator
#define S_length ES_length
#define S_max ES_max
#define S_min ES_min
#define S_minusOne ES_minusOne
#define S_nop ES_nop
#define S_null ES_null
#define S_one ES_one
#define S_public ES_public
#define S_true ES_true
#define S_undefined ES_undefined
#define S_zero ES_zero

#define S_StopIteration ES_iterator_StopIteration
#define S_nan ES_NaN
#define S_Iterator ES_iterator_Iterator
#define S_infinity ES_Infinity
#define S_negativeInfinity ES_NegativeInfinity
#define S_ejsSpace ES_ejs
#define S_iteratorSpace ES_iterator
#define S_internalSpace ES_internal
#define S_publicSpace ES_public
#define S_emptySpace ES_emptySpace

/*
    Special values outside ejs.mod

*/
#define S_LocalCache ES_global_NUM_CLASS_PROP + 1
#define EJS_MAX_SPECIAL ES_global_NUM_CLASS_PROP + 10

#if DOXYGEN
    /**
        Get immutable special value
        @param name Literal name
        @return special value
      */
    extern EjsAny *ESV(void *name);

    /**
        Special type
     */
    extern EjsType *EST(void *name);
#else
    #define ESV(name) ejs->service->immutable->properties->slots[S_ ## name].value.ref
    #define EST(name) ((EjsType*) ESV(name))
#endif

/************************************** Ejs ***********************************/
/**
    Ejsript VM Structure
    @description The Ejs structure contains the state for a single interpreter. The #ejsCreateVM routine may be used
        to create multiple interpreters and returns a reference to be used in subsequent Ejscript API calls.
    @defgroup Ejs Ejs
    @see ejsAddImmutable ejsAppendSearchPath ejsClearException ejsCloneVM ejsCreateService ejsCreateVM 
        ejsDestroyVM ejsEvalFile ejsEvalModule ejsEvalScript ejsExit ejsGetImmutable ejsGetImmutableByName 
        ejsGetVarByName ejsGethandle ejsLog ejsLookupScope ejsLookupVar ejsLookupVarWithNamespaces ejsReportError 
        ejsRun ejsRunProgram ejsSetDispatcher ejsSetSearchPath ejsThrowException 
    @stability Internal.
 */
typedef struct Ejs {
    char                *name;              /**< Unique interpreter name */
    EjsAny              *exception;         /**< Pointer to exception object */
    EjsAny              *result;            /**< Last expression result */
    struct EjsState     *state;             /**< Current evaluation state and stack */
    struct EjsService   *service;           /**< Back pointer to the service */
    EjsAny              *global;            /**< The "global" object */
    cchar               *bootSearch;        /**< Module search when bootstrapping the VM (not alloced) */
    struct EjsArray     *search;            /**< Module load search path */
    cchar               *className;         /**< Name of a specific class to run for a program */
    cchar               *methodName;        /**< Name of a specific method to run for a program */
    char                *errorMsg;          /**< Error message */
    cchar               **argv;             /**< Command line args (not alloced) */
    char                *hostedDocuments;   /**< Documents directory for hosted HttpServer */
    char                *hostedHome;        /**< Home directory for hosted HttpServer */
    int                 argc;               /**< Count of command line args */
    int                 flags;              /**< Execution flags */
    int                 exitStatus;         /**< Status to exit() */
    int                 firstGlobal;        /**< First global to examine for GC */
    int                 joining;            /**< In Worker.join */
    int                 serializeDepth;     /**< Serialization depth */
    int                 spreadArgs;         /**< Count of spread args */
    int                 gc;                 /**< GC required (don't make bit field) */
    uint                abandoned: 1;       /**< Pooled VM is released awaiting GC  */
    uint                hosted: 1;          /**< Interp is hosted (webserver) */
    uint                configSet: 1;       /**< Config properties defined */
    uint                compiling: 1;       /**< Currently executing the compiler */
    uint                destroying: 1;      /**< Interpreter is being destroyed */
    uint                dontExit: 1;        /**< Prevent App.exit() from exiting */
    uint                empty: 1;           /**< Interpreter will be created empty */
    uint                exiting: 1;         /**< VM should exit */
    uint                hasError: 1;        /**< Interpreter has an initialization error */
    uint                initialized: 1;     /**< Interpreter fully initialized and not empty */

    EjsAny              *exceptionArg;      /**< Exception object for catch block */
    MprDispatcher       *dispatcher;        /**< Event dispatcher */
    MprList             *workers;           /**< Worker interpreters */
    MprList             *modules;           /**< Loaded modules */
    MprList             *httpServers;       /**< Configured HttpServers */

    void                (*loaderCallback)(struct Ejs *ejs, int kind, ...);

    void                *loadData;          /**< Arg to load callbacks */
    void                *httpServer;        /**< HttpServer instance when VM is embedded */

    MprHash             *doc;               /**< Documentation */
    void                *sqlite;            /**< Sqlite context information */

    Http                *http;              /**< Http service object (copy of EjsService.http) */
    MprMutex            *mutex;             /**< Multithread locking */
} Ejs;


/**
    Add an immutable reference. 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call adds an
        object to that set. If the object already exists in the immutable set, its slot number if returned.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param slotNum Unique slot number for the object
    @param qname Qualified name for the object
    @param obj Object to store
    @returns Returns the actual slot number allocated for the object
    @ingroup Ejs
 */
PUBLIC int ejsAddImmutable(struct Ejs *ejs, int slotNum, EjsName qname, EjsAny *obj);

/**
    Get an immutable object. 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call retrieves an
        immutable object from that set.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param slotNum Unique slot number for the object
    @return obj Immutable object found at the given slotNum.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetImmutable(struct Ejs *ejs, int slotNum);

/**
    Get an immutable object by name 
    @description Ejscript keeps a set of immutable objects that are shared across virtual machines. This call retrieves an
        immutable object from that set.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name for the object
    @return obj Immutable object found at the given slotNum.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetImmutableByName(struct Ejs *ejs, EjsName qname);

/**
    Block garbage collection
    @description Garbage collection requires cooperation from threads. However, the VM will normally permit garbage
    collection at various points in the execution of byte code. This call block garbage collection while allowing
    program execution to continue. This is useful for short periods when transient objects are not reachable by the 
    GC marker.
    @param ejs Interpeter object returned from #ejsCreateVM
    @return The previous GC blocked state
    @ingroup Ejs
 */
PUBLIC int ejsBlockGC(Ejs *ejs);

/**
    Unblock garbage collection
    @description Unblock garbage collection that was blocked via #ejsBlockGC
    @param ejs Interpeter object returned from #ejsCreateVM
    @param blocked Blocked state to re-establish
    @ingroup Ejs
 */
PUBLIC void ejsUnblockGC(Ejs *ejs, int blocked);

/************************************ EjsPool *********************************/
/**
    Cached pooled of virtual machines.
    @defgroup EjsPool EjsPool
    @see ejsCreatePool ejsAllocPoolVM ejsFreePoolVM 
    @stability Internal
  */
typedef struct EjsPool {
    MprList     *list;                      /**< Free list */
    MprTicks    lastActivity;               /**< When a VM was last used */
    MprEvent    *timer;                     /**< VM prune timer */
    MprMutex    *mutex;                     /**< Multithread lock */
    int         count;                      /**< Count of allocated VMs */
    int         max;                        /**< Maximum number of VMs */
    Ejs         *template;                  /**< VM template to clone */
    char        *templateScript;            /**< Template initialization script filename */
    char        *startScript;               /**< Template initialization literal script */
    char        *startScriptPath;           /**< Template initialization script filename */
    char        *hostedDocuments;           /**< Documents directory for hosted HttpServer */
    char        *hostedHome;                /**< Home directory for hosted HttpServer */
} EjsPool;


/**
    Create a pool for virutal machines
    @description 
    @param poolMax Maximum number of VMs in the pool
    @param templateScript Script to execute to initialize a template VM from which all VMs in the pool will be cloned.
        This is executed only once when the pool is created. This is typically used to pre-load modules.
    @param startScript Startup script literal. This script is executed each time the VM is allocated from the pool
    @param startScriptPath As an alternative to startScript, a path to a script may be provided in startScriptPath.
        If startScriptPath is specified, startScript is ignored.
    @param home Default home directory for virtual machines
    @param documents Default documents directory for virtual machines
    @returns Allocated pool object
    @ingroup EjsPool
 */
PUBLIC EjsPool *ejsCreatePool(int poolMax, cchar *templateScript, cchar *startScript, cchar *startScriptPath, cchar *home,
        cchar *documents);

/**
    Allocate a VM from the pool
    @param pool EjsPool reference
    @param flags Reserved
    @returns Returns an Ejs VM instance
    @ingroup EjsPool
 */
PUBLIC Ejs *ejsAllocPoolVM(EjsPool *pool, int flags);

/**
    Free a VM back to the pool
    @param pool EjsPool reference
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPool
 */
PUBLIC void ejsFreePoolVM(EjsPool *pool, Ejs *ejs);

/************************************ EjsObj **********************************/
/**
    Base object from which all objects inherit.
    @defgroup EjsObj EjsObj
    @see ejsAlloc ejsCastType ejsClone ejsCreateInstance ejsCreateObj ejsDefineProperty ejsDeleteProperty 
        ejsDeletePropertyByName ejsDeserialize ejsGetLength ejsGetProperty ejsGetPropertyByName ejsGetPropertyName 
        ejsGetPropertyTraits ejsInvokeOperator ejsInvokeOperatorDefault ejsLookupProperty ejsParse ejsSetProperty 
        ejsSetPropertyByName ejsSetPropertyName ejsSetPropertyTraits 
    @stability Internal
 */
typedef struct EjsObj {
    //  WARNING: changes to this structure require changes to mpr/src/mprPrintf.c
    ssize           xtype;              /**< xtype: typeBits | dynamic << 1 | visited */
#if BIT_DEBUG
    char            *kind;              /**< If DEBUG, Type name of object (Type->qname.name) */
    struct EjsType  *type;              /**< If DEBUG, Pointer to object type */
    MprMem          *mem;               /**< If DEBUG, Pointer to underlying memory block */
#endif
} EjsObj;

/** 
    Allocate a new variable
    @description This will allocate space for a bare variable. This routine should only be called by type factories
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type object from which to create an object instance
    @param extra Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsAlloc(Ejs *ejs, struct EjsType *type, ssize extra);

/** 
    Cast a variable to a new type
    @description Cast a variable and return a new variable of the required type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to cast
    @param type Type to cast to
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCastType(Ejs *ejs, EjsAny *obj, struct EjsType *type);

/** 
    Clone a variable
    @description Copy a variable and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to clone
    @param deep Set to true to do a deep copy.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsClone(Ejs *ejs, EjsAny *obj, bool deep);

/** 
    Create a new variable instance 
    @description Create a new variable instance and invoke any required constructors with the given arguments.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type from which to create a new instance
    @param argc Count of args in argv
    @param argv Vector of arguments. Each arg is an EjsAny.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateInstance(Ejs *ejs, struct EjsType *type, int argc, void *argv);

/** 
    Create a variable
    @description Create a variable of the required type. This invokes the createVar helper method for the specified type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type to cast to
    @param numSlots Size of extra property slots to reserve. This is used for dynamic objects.
    @return A newly allocated variable of the requested type. Caller must not free as the GC will manage the lifecycle
        of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateObj(Ejs *ejs, struct EjsType *type, int numSlots);

/** 
    Define a property
    @description Define a property in a variable and give it a name, base type, attributes and default value.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object in which to define a property
    @param slotNum Slot number in the variable for the property. Slots are numbered sequentially from zero. Set to
        -1 to request the next available slot number.
    @param qname Qualified name containing a name and a namespace.
    @param type Base type of the property. Set to ejs->voidType to leave as untyped.
    @param attributes Attribute traits. 
    @param value Initial value of the property
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDefineProperty(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *type, int64 attributes, 
    EjsAny *value);

/** 
    Delete a property
    @description Delete a variable's property and set its slot to null. The slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable in which to delete the property
    @param slotNum Slot number in the variable for the property to delete.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDeleteProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Delete a property by name
    @description Delete a variable's property by name and set its slot to null. The property is resolved by using 
        ejsLookupProperty with the specified name. Once deleted, the slot is not reclaimed and subsequent properties
        are not compacted.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable in which to delete the property
    @param qname Qualified name for the property including name and namespace.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsDeletePropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsGetProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a count of properties in a variable
    @description Get a property from a variable at a given slot.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @return A positive integer count of the properties stored by the variable. 
    @ingroup EjsObj
 */
PUBLIC int ejsGetLength(Ejs *ejs, EjsAny *obj);

/** 
    Get a variable property's name
    @description Get a property name for the property at a given slot in the  variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @return The qualified property name including namespace and name. Caller must not free.
    @ingroup EjsObj
 */
PUBLIC EjsName ejsGetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Get a property by name
    @description Get a property from a variable by name.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param qname Qualified name specifying both a namespace and name.
    @return The variable property stored at the nominated slot.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsGetPropertyByName(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Get a property's traits
    @description Get a property's trait description. The property traits define the properties base type,
        and access attributes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number for the requested property.
    @return A trait structure reference for the property.
    @ingroup EjsObj
 */
PUBLIC struct EjsTrait *ejsGetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum);

/** 
    Invoke an opcode on a native type.
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
        Native types would normally implement the invokeOperator helper function to respond to this function call.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsInvokeOperator(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

//  TODO rename
/** 
    Default implementation for operator invoke
    @description Invoke an Ejscript byte code operator on the specified variable given the expression right hand side.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param opCode Byte ope code to execute
    @param rhs Expression right hand side for binary expression op codes. May be null for other op codes.
    @return The result of the op code or NULL if the opcode does not require a result.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsInvokeOperatorDefault(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);

/** 
    Lookup a property by name
    @description Search for a property by name in the given variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param qname Qualified name of the property to search for.
    @return The slot number containing the property. Then use $ejsGetProperty to retrieve the property or alternatively
        use ejsGetPropertyByName to lookup and retrieve in one step.
    @ingroup EjsObj
 */
PUBLIC int ejsLookupProperty(Ejs *ejs, EjsAny *obj, EjsName qname);

/** 
    Set a property's value
    @description Set a value for a property at a given slot in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param slotNum Slot number for the requested property.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetProperty(Ejs *ejs, void *obj, int slotNum, void *value);

/** 
    Set a property's value 
    @description Set a value for a property. The property is located by name in the specified variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @param qname Qualified property name.
    @param value Reference to a value to store.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyByName(Ejs *ejs, void *obj, EjsName qname, void *value);

/** 
    Set a property's name 
    @description Set a qualified name for a property at the specified slot in the variable. The qualified name
        consists of a namespace and name - both of which must be persistent. A typical paradigm is for these name
        strings to be owned by the memory context of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param qname Qualified property name.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyName(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);

/** 
    Set a property's traits
    @description Set the traits describing a property. These include the property's base type and access attributes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Variable to examine
    @param slotNum Slot number of the property in the variable.
    @param type Base type for the property. Set to NULL for an untyped property.
    @param attributes Integer mask of access attributes.
    @return The slot number of the property updated.
    @ingroup EjsObj
 */
PUBLIC int ejsSetPropertyTraits(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

/**
    Deserialize a JSON encoded string
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param str JSON string to deserialize
    @returns Returns an allocated object equivalent to the supplied JSON encoding
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsDeserialize(Ejs *ejs, struct EjsString *str);

/**
    Parse a string 
    @description This parses a string and intelligently interprets the data type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param str String to parse
    @param prefType Preferred type. Set to the reserved type slot number. E.g. S_Number, S_String etc.
    @returns Returns an allocated object. Returns undefined if the input cannot be parsed.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsParse(Ejs *ejs, wchar *str,  int prefType);

/************************************ EjsPot **********************************/
/** 
    Property traits. 
    @description Property traits describe the type and access attributes of a property. The Trait structure
        is used by EjsBlock to describe the attributes of properties defined within a block.
        Note: These traits apply to a property definition and not to the referenced object. ie. two property 
        definitions may have different traits but will refer to the same object.
    @stability Evolving
    @ingroup EjsPot
 */
typedef struct EjsTrait {
    struct EjsType  *type;                  /**< Property type (prototype) */
    int             attributes;             /**< Modifier attributes */
} EjsTrait;


/*
    Ejs Value storage

    Use a nun-boxing scheme where pointers and core types are stored in the unclaimed NaN space. IEEE doubles have a large space of 
    unclaimed NaN values that can be used to store pointers and other values. IEEE defines:

        Sign(1)  Exponent(11) Mantissa(52)
        
        +inf            0x7ff0 0000 0000 0000
        -inf            0xfff0 0000 0000 0000
        NaN             0xfff8 0000 0000 0000
        Nan Space       0xfff1 type 0000 0000 (we use this one)
        Other NaN Space 0x7ff0 0000 0000 0000

    Doubles are encoded by adding a 2^48 bias so that all doubles have a MSB 16 bits in the range 0x0001 or 0xfffe. This means that the 
    the ranges 0x0000 is available for pointers and 0xffff is available for integers. Further, since all pointers are aligned on 8 byte
    boundaries, the bottom 3 bits are available for use as a tag to encode core types.

    References:
        http://wingolog.org/archives/2011/05/18/value-representation-in-javascript-implementations
        http://evilpie.github.com/sayrer-fatval-backup/cache.aspx.htm
        http://www.slideshare.net/greenwop/high-performance-javascript
        webkit/Source/JavaScriptCore/runtime 
 */

#define NUMBER_BIAS     0x1000000000000L            /* Add 2^48 to all double values. Subtract to use. */

#define GROUP_MASK      0xffff000000000000L         /* Mask for MSB 16 bits */
#define GROUP_DOUBLE    0x0001 // to 0xfffe
#define GROUP_CORE      0x0000000000000000L
#define GROUP_INT       0xffff000000000000L

/*
    Core group tags
    Null/Undefined are paried to have VALID_MASK set.
    True/False are paired to have BOOL_MASK set.
 */
#define TAG_MASK        0x7
#define TAG_POINTER     0x0
#define VALID_MASK      0x1
#define TAG_NULL        0x1
#define TAG_UNDEFINED   0x5
#define BOOL_MASK       0x2
#define TAG_FALSE       0x2
#define TAG_TRUE        0x6
#define TAG_STRING      0x4

#define POINTER_MASK    0xffff000000000007L         /* Single mask to test for pointers */
#define POINTER_VALUE   0x0000fffffffffff8L
#define INTEGER_VALUE   0x00000000ffffffffL

#define viscore(v)      (((v)->word & GROUP_MASK) == GROUP_CORE)
#define vispointer(v)   (((v)->word & POINTER_MASK) == 0)
#define viint(v)        ((v)->word == GROUP_INT)
#define visdouble(v)    (!(vispointer(v) || visint(v)))

#define visbool(v)      (viscore(v) && (v)->word & VALID_MASK)
#define visvalid(v)     (!(viscore(v) && (v)->word & VALID_MASK))
#define vistrue(v)      (viscore(v) && ((v)->word & TAG_MASK) == TAG_TRUE))
#define visfalse(v)     (vicore(v) && ((v)->word & TAG_MASK) == TAG_FALSE))
#define visnull(v)      (viscore(v) && (v)->word & TAG_MASK) == TAG_NULL)
#define visundefined(v) (viscore(v) && (v)->word & TAG_MASK) == TAG_UNDEFINED)
#define visstring(v)    (viscore(v) && (v)->word & TAG_MASK) == TAG_STRING)

#if BIT_64
    #define vpointer(v) ((v)->pointer & POINTER_VALUE)
    #define vinteger(v) ((v)->integer & INTEGER_VALUE)
#else
    #define vpointer(v) ((v)->pointer)
    #define vinteger(v) ((v)->integer)
#endif
#define vdouble(v)      ((v)->number - NUMBER_BIAS)
#define vbool(v)        (v->word & BOOL_MASK) >> 2)
#define vstring(v)      (v->pointer & POINTER_VALUE)

typedef union EjsValue {
    uint64  word;
    double  number;
#if BIT_64
    void    *pointer;
    int     integer;
#else
    #if CPU_ENDIAN == BIT_LITTLE_ENDIAN
        struct {
            void    *pointer;
            int32   filler1;
        };
        struct {
            int32   integer;
            int32   filler2;
        };
    #else
        struct {
            int32   filler1;
            void    *pointer;
        };
        struct {
            int32   filler2;
            int32   integer;
        };
    #endif
#endif
} EjsValue;


/**
    Property slot structure
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsSlot {
    EjsName         qname;                  /**< Property name */
    EjsTrait        trait;                  /**< Property descriptor traits */
    int             hashChain;              /**< Next property in hash chain */
    union {
        EjsAny      *ref;                   /**< Property reference */
#if UNUSED
        EjsValue    value;
#endif
    } value;
} EjsSlot;

/**
    Property hash linkage
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsHash {
    int             size;                   /**< Size of hash */
    int             *buckets;               /**< Hash buckets and head of link chains */
} EjsHash;


/**
    Object properties
    @ingroup EjsPot
    @stability Internal
 */
typedef struct EjsProperties {
    EjsHash         *hash;                  /**< Hash buckets and head of link chains */
    int             size;                   /**< Current size of slots[] in elements */
    struct EjsSlot  slots[ARRAY_FLEX];      /**< Vector of slots containing property references */
} EjsProperties;


/** 
    Object with properties Type. Base object for generic objects with properties.
    @description The EjsPot type is the foundation for types, blocks, functions and scripted classes. 
        It provides storage and hashed lookup for properties.
        \n\n
        EjsPots may be either dynamic or sealed. Dynamic objects can grow the number of properties. Sealed 
        objects cannot. Sealed objects will store the properties as part of the EjsPot memory chunk. Dynamic 
        objects will perform a separate allocation for the properties that it can grow.
        \n\n
        EjsPot stores properties in an array of slots. These slots store the property name and a reference to the 
        property value.  Dynamic objects own their own name hash. Sealed object instances of a type, will refer to the 
        hash of names owned by the type.
    @defgroup EjsPot EjsPot
    @see EjsPot ejsAlloc ejsBlendObject ejsCast ejsCheckSlot ejsClone ejsCloneObject ejsClonePot ejsCoerceOperands 
        ejsCompactPot ejsCopySlots ejsCreateEmptyPot ejsCreateInstance ejsCreateObject ejsCreatePot 
        ejsCreatePotHelpers ejsDefineProperty ejsDeleteProperty ejsDeletePropertyByName 
        ejsDeserialize ejsFixTraits ejsGetHashSize ejsGetPotPropertyName ejsGetProperty ejsGrowObject ejsGrowPot 
        ejsIndexProperties ejsInsertPotProperties ejsIsPot ejsLookupPotProperty ejsLookupProperty ejsManageObject 
        ejsManagePot ejsMatchName ejsObjToJSON ejsObjToString ejsParse ejsPropertyHasTrait ejsRemovePotProperty 
        ejsSetProperty ejsSetPropertyByName ejsSetPropertyName ejsSetPropertyTraits ejsZeroSlots 
    @stability Internal.
 */
typedef struct EjsPot {
    EjsObj  obj;                                /**< Base object */
    uint    isBlock         : 1;                /**< Instance is a block */
    uint    isFrame         : 1;                /**< Instance is a frame */
    uint    isFunction      : 1;                /**< Instance is a function */
    uint    isPrototype     : 1;                /**< Object is a type prototype object */
    uint    isType          : 1;                /**< Instance is a type object */
    uint    separateHash    : 1;                /**< Object has separate hash memory */
    uint    separateSlots   : 1;                /**< Object has separate slots[] memory */
    uint    shortScope      : 1;                /**< Don't follow type or base classes */

    EjsProperties   *properties;                /** Object properties */
    //  TODO - OPT - merge numProp with bits above (24 bits)
    int             numProp;                    /** Number of properties */
} EjsPot;

#define POT(ptr)  (TYPE(ptr)->isPot)
#if DOXYGEN
    /** 
        Determine if a variable is a Pot.
        @description This call tests if the variable is a Pot.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @returns True if the variable is based on EjsPot
        @ingroup EjsPot
     */
    extern bool ejsIsPot(Ejs *ejs, EjsAny *obj);
#else
#define ejsIsPot(ejs, obj) (obj && POT(obj))
#endif

PUBLIC void ejsZeroSlots(Ejs *ejs, EjsSlot *slots, int count);
PUBLIC void ejsCopySlots(Ejs *ejs, EjsPot *dest, int destOff, EjsPot *src, int srcOff, int count);

/** 
    Create an empty property object
    @description Create a simple object using Object as its base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @return A new object instance
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreateEmptyPot(Ejs *ejs);

/** 
    Create an object instance of the specified type
    @description Create a new object using the specified type as a base class. 
        Note: the constructor is not called. If you require the constructor to be invoked, use ejsCreateInstance
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Base type to use when creating the object instance
    @param size Number of extra slots to allocate when creating the object
    @return A new object instance
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsCreatePot(Ejs *ejs, struct EjsType *type, int size);

/**
    Compact an object
    @description This removes deleted properties and compacts property slot references
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to compact
    @returns The number of properties in the object.
    @ingroup EjsObj
 */
PUBLIC int ejsCompactPot(Ejs *ejs, EjsPot *obj);

/**
    Insert properties
    @description Insert properties at the given offset
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pot Object to modify
    @param numSlots Number of slots to insert at offset
    @param offset Slot offset in pot
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsInsertPotProperties(Ejs *ejs, EjsPot *pot, int numSlots, int offset);

/**
    Make or remake a property index
    @description Make a hash lookup of properties. This will be skipped if there are insufficient properties to make the
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsIndexProperties(Ejs *ejs, EjsPot *obj);

/**
    Test a property's traits
    @description Make a hash lookup of properties. This will be skipped if there are insufficient properties to make the
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine.
    @param slotNum Property slot number in obj to examine.
    @param attributes Attribute mask to test with the selected property's traits.
    @returns A mask of the selected attributes. Returns zero if none match.
    @ingroup EjsPot
 */
PUBLIC int ejsPropertyHasTrait(Ejs *ejs, EjsAny *obj, int slotNum, int attributes);

/**
    Remove a property
    @description Remove a property and compact previous properties. WARNING: this should only be used by the compiler.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Property slot number to remove.
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsRemovePotProperty(Ejs *ejs, EjsAny *obj, int slotNum);

/**
    Lookup a property in a Pot
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param qname Property name to look for
    @returns If successful, return the slot number of the propert in obj. Otherwise return -1.
    @ingroup EjsPot
    @internal
 */
PUBLIC int ejsLookupPotProperty(Ejs *ejs, EjsPot *obj, EjsName qname);

/**
    Get a property name
    @description Get the name of the property at the given slot.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Slot number of property to examine
    @returns EjsName for the property
    @ingroup EjsPot
    @internal
 */
PUBLIC EjsName ejsGetPotPropertyName(Ejs *ejs, EjsPot *obj, int slotNum);

/** 
    Copy an object
    @description Copy an object create a new instance. This may do a shallow or deep copy depending on the value of 
        deep. A shallow copy will not copy the property instances, rather it will only duplicate the property 
        reference. A deep copy will recursively clone all the properties of the variable.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param src Source object to copy
    @param deep Set to true to do a deep copy.
    @return A newly allocated object. Caller must not free as the GC will manage the lifecycle of the variable.
    @ingroup EjsObj
 */
PUBLIC EjsAny *ejsClonePot(Ejs *ejs, EjsAny *src, bool deep);


/**
    Fix traits
    @description Fix the trait type references to point to mutable types in the current interpreter. This is needed
    after cloning the global object.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to fixup.
    @ingroup EjsPot
    @internal
 */
PUBLIC void ejsFixTraits(Ejs *ejs, EjsPot *obj);

/** 
    Grow a pot object
    @description Grow the property storage for an object. Object properties are stored in slots. To store more 
        properties, you need to grow the slots.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object reference to grow
    @param numSlots New minimum count of properties. If size is less than the current number of properties, the call
        will be ignored, i.e. it will not shrink objects.
    @return Zero if successful
    @ingroup EjsObj
 */
PUBLIC int ejsGrowPot(Ejs *ejs, EjsPot *obj, int numSlots);

/** 
    Mark an object as currently in use.
    @description Mark an object as currently active so the garbage collector will preserve it. This routine should
        be called by native types that extend EjsObj in their markVar helper.
    @param obj Object to mark as currently being used.
    @param flags manager flags
    @ingroup EjsObj
 */
PUBLIC void ejsManagePot(void *obj, int flags);

/**
    Check the slot
    @description Check the slot refers to a valid property
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to index.
    @param slotNum Slot number to check
    @returns The slotNum if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsCheckSlot(Ejs *ejs, EjsPot *obj, int slotNum);

/**
    Cast the operands as required by the operation code.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param lhs Left-hand-side of operation
    @param opcode Operation byte code
    @param rhs Right-hand-side of operation
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC EjsAny *ejsCoerceOperands(Ejs *ejs, EjsObj *lhs, int opcode, EjsObj *rhs);

/**
    Get the preferred hash size
    @param numProp Number of properties to hash
    @returns A positive hash size integer
    @ingroup EjsPot
 */
PUBLIC int ejsGetHashSize(int numProp);

/**
    Create the Pot helpers 
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPot
    @internal
 */
PUBLIC void ejsCreatePotHelpers(Ejs *ejs);

/**
    Method proc for conversion to a string.
    @description This method provides the default conversion to a string implementation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert to a string
    @param argc Ignored
    @param argv Ignored
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC struct EjsString *ejsObjToString(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv);

/**
    Method proc for conversion to a JSON string.
    @description This method provides the default conversion to a JSON string implementation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert to a JSON string
    @param argc Ignored
    @param argv Ignored
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC struct EjsString *ejsObjToJSON(Ejs *ejs, EjsObj *obj, int argc, EjsObj **argv);

/*
    ejsBlendObject flags
 */
#define EJS_BLEND_COMBINE       0x1         /**< Flag for ejsBlendObject to combine key values */
#define EJS_BLEND_DEEP          0x2         /**< Flag for ejsBlendObject to copy nested object recursively */
#define EJS_BLEND_FUNCTIONS     0x4         /**< Flag for ejsBlendObject to copy function properties */
#define EJS_BLEND_OVERWRITE     0x8         /**< Flag for ejsBlendObject to overwrite existing properties */
#define EJS_BLEND_SUBCLASSES    0x10        /**< Flag for ejsBlendObject to copy subclassed properties */
#define EJS_BLEND_PRIVATE       0x20        /**< Flag for ejsBlendObject to copy private properties */
#define EJS_BLEND_TRACE         0x40        /**< Flag for ejsBlendObject to trace blend operations to the log */
#define EJS_BLEND_ADD           0x80        /**< Flag for ejsBlendObject for "+" property blend */
#define EJS_BLEND_SUB           0x100       /**< Flag for ejsBlendObject for "-" property blend */
#define EJS_BLEND_ASSIGN        0x200       /**< Flag for ejsBlendObject for "=" property blend */
#define EJS_BLEND_COND_ASSIGN   0x400       /**< Flag for ejsBlendObject for "?" property blend */

//  TODO - rename ejsBlend
/**
    Blend objects
    @description Merge one object into another. This is useful for inheriting and optionally overwriting option 
        hashes (among other things). The blending is done at the primitive property level. If overwrite is true, 
        the property is replaced. If overwrite is false, the property will be added if it does not already exist
        index worthwhile.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Destination object.
    @param src Source object.
    @param flags Select from:
        <ul>
            <li>EJS_BLEND_DEEP - to copy nested objects recursively</li>
            <li>EJS_BLEND_FUNCTIONS - to copy function properties</li>
            <li>EJS_BLEND_OVERWRITE - to overwrite existing properties in the destination when copying from source</li>
            <li>EJS_BLEND_SUBCLASSES - to copy subclasses in src</li>
            <li>EJS_BLEND_PRIVATE - to copy private properties</li>
        </ul>
    @returns Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPot
 */
PUBLIC int ejsBlendObject(Ejs *ejs, EjsObj *dest, EjsObj *src, int flags);

/**
    Test if two names match
    @description This tests if two names are equivalent.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param a First name to test
    @param b Second name to test
    @returns True if the names are equivalent
    @ingroup EjsPot
 */
PUBLIC bool ejsMatchName(Ejs *ejs, EjsName *a, EjsName *b);

/********************************************** String ********************************************/
/** 
    String Class
    @description The String class provides the base class for all strings. Each String object represents a single 
    immutable linear sequence of characters. Strings have operators for: comparison, concatenation, copying, 
    searching, conversion, matching, replacement, and, subsetting.
    \n\n
    Strings are currently sequences of Unicode characters. Depending on the configuration, they may be 8, 16 or 32 bit
    code point values.
    @defgroup EjsString EjsString
    @see EjsString ejsAtoi ejsCompareAsc ejsCompareString ejsCompareSubstring ejsCompareWide ejsContainsAsc 
        ejsContainsChar ejsContainsString ejsCreateBareString ejsCreateString ejsCreateStringFromAsc 
        ejsCreateStringFromBytes ejsCreateStringFromConst ejsCreateStringFromMulti ejsCreateStringWithLength 
        ejsDestroyIntern ejsInternAsc ejsInternMulti ejsInternString ejsInternWide ejsJoinString 
        ejsJoinStrings ejsSerialize ejsSerializeWithOptions ejsSprintf ejsStartsWithAsc ejsStrcat ejsStrdup 
        ejsSubstring ejsToJSON ejsToLiteralString ejsToMulti ejsToString ejsToUpper ejsTruncateString ejsVarToString 
        ejsToLower 
    @stability Internal.
 */
typedef struct EjsString {
    //  WARNING: changes to EjsString require changes to mpr/src/mprPrintf.c
    struct EjsObj    obj;               /**< Base object */
    struct EjsString *next;             /**< Next string in hash chain link when interning */
    struct EjsString *prev;             /**< Prev string in hash chain */
    ssize            length;            /**< Length of string */
    wchar            value[ARRAY_FLEX]; /**< String value */
} EjsString;

/** 
    Create a string object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value C string value to define for the string object. Note: this will soon be changed to unicode.
    @param len Length of string to examine in value
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateString(Ejs *ejs, wchar *value, ssize len);

/**
    Create a string from a module string constant
    @param ejs Ejs reference returned from #ejsCreateVM
    @param mp Module object
    @param index String constant index
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
    @internal
 */
PUBLIC EjsString *ejsCreateStringFromConst(Ejs *ejs, struct EjsModule *mp, int index);

/**
    Create a string from ascii
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Null terminated ascii string value to intern
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromAsc(Ejs *ejs, cchar *value);

/**
    Create a string from an ascii block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-8 multibyte string value to intern
    @param len Length of the string in bytes
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromBytes(Ejs *ejs, cchar *value, ssize len);

/**
    Create a string from UTF-8 multibyte string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Ascii string value to intern
    @param len Length of value in bytes
    @returns Allocated string. These are references into the interned string pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateStringFromMulti(Ejs *ejs, cchar *value, ssize len);

/** 
    Create an empty string object. This creates an uninitialized string object of the requrired size. Once initialized,
        the string must be "interned" via $ejsInternString.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param len Length of space to reserve for future string data
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsCreateBareString(Ejs *ejs, ssize len);

/** 
    Intern a string object. 
    @description This stores the string in the internal string pool. This is required if the string was
        created via ejsCreateBareString. The ejsCreateString routine will intern the string automatcially.
    @param sp String object to intern
    @return The internalized string object. NOTE: this may be different to the object passed in, if the string value
        was already present in the intern pool.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternString(EjsString *sp);

/** 
    Intern a string object from a UTF-8 string. 
    @description A string is created using the UTF-8 string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-8 string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternMulti(struct Ejs *ejs, cchar *value, ssize len);

/** 
    Intern a string object from an Ascii string.
    @description A string is created using the ascii string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Ascii string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternAsc(struct Ejs *ejs, cchar *value, ssize len);

/** 
    Intern a string object from a UTF-16 string. 
    @description A string is created using the UTF-16 string as input. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value UTF-16 string buffer
    @param len Size of the input value string
    @return The internalized string object.
    @ingroup EjsString
 */
PUBLIC EjsString *ejsInternWide(struct Ejs *ejs, wchar *value, ssize len);

/** 
    Destroy the intern string cache
    @param intern Reference to the intern object
    @ingroup EjsString
 */
PUBLIC void ejsDestroyIntern(struct EjsIntern *intern);

/** 
    Parse a string and convert to an integer
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp String to parse
    @param radix Radix for parsing the string
    @return Integer representation of the string 
    @ingroup EjsString
 */
PUBLIC int ejsAtoi(Ejs *ejs, EjsString *sp, int radix);

/** 
    Join two strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string to join
    @param s2 Second string to join
    @return A new string representing the joined strings
    @ingroup EjsString
 */
PUBLIC EjsString *ejsJoinString(Ejs *ejs, EjsString *s1, EjsString *s2);

/** 
    Join strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src First string to join
    @param ... Other strings to join
    @return A new string representing the joined strings
    @ingroup EjsString
 */
PUBLIC EjsString *ejsJoinStrings(Ejs *ejs, EjsString *src, ...);

/** 
    Get a substring 
    @description Get a substring at a given offset
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src Source string
    @param offset Offset in string to take the substring
    @param len Length of the substring
    @return The substring
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSubstring(Ejs *ejs, EjsString *src, ssize offset, ssize len);

/** 
    Compare two strings
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Second string
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareString(Ejs *ejs, EjsString *s1, EjsString *s2);

/** 
    Compare a substring
    @description This call compares the first string with a substring in the second.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string to compare
    @param s2 Second string
    @param offset Offset in string to take the substring
    @param len Length of the substring
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareSubstring(Ejs *ejs, EjsString *s1, EjsString *s2, ssize offset, ssize len);

/** 
    Convert a string to lower case
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @return A lower case version of the input string
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToLower(Ejs *ejs, EjsString *sp);

/** 
    Convert a string to upper case
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @return A upper case version of the input string
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToUpper(Ejs *ejs, EjsString *sp);

/** 
    Truncate a string
    @description Truncate the string and return a new string. Note: the original is not modified.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param src Source string
    @param len Length of the result string
    @return The substring
    @ingroup EjsString
 */
PUBLIC EjsString *ejsTruncateString(Ejs *ejs, EjsString *src, ssize len);

/** 
    Compare a string with a multibyte string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Null terminated Ascii string
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareAsc(Ejs *ejs, EjsString *s1, cchar *s2);

/** 
    Compare a string with a wide string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param s1 First string
    @param s2 Wide string
    @param len Maximum length in characters to compare
    @return Return zero if the strings are identical. Return -1 if s1 is less than s2. Otherwise return 1.
    @ingroup EjsString
 */
PUBLIC int ejsCompareWide(Ejs *ejs, EjsString *s1, wchar *s2, ssize len);

/** 
    Test if a string contains a character
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param charPat Character to search for
    @return The index in the string where the character was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsChar(Ejs *ejs, EjsString *sp, int charPat);

/** 
    Test if a string contains an ascii substring
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat Ascii string pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsAsc(Ejs *ejs, EjsString *sp, cchar *pat);

/** 
    Test if a string contains another string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat String pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsContainsString(Ejs *ejs, EjsString *sp, EjsString *pat);

/** 
    Test if a string starts with an ascii pattern
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sp Source string
    @param pat Pattern to search for
    @return The index in the string where the pattern was found. Otherwise return -1.
    @ingroup EjsString
 */
PUBLIC int ejsStartsWithAsc(Ejs *ejs, EjsString *sp, cchar *pat);

/** 
    Convert an object to a UTF-8 string representation
    @description The object is converted to a string and then serialized into UTF-8.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A multibyte UTF-8 representation.
    @ingroup EjsString
 */
PUBLIC char *ejsToMulti(Ejs *ejs, void *obj);

//  TODO - rename ejsFormat
/** 
    Format arguments
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Format specifier.
    @param ... Arguments for the format specifiers
    @return A formatted string 
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSprintf(Ejs *ejs, cchar *fmt, ...);

/**
    Convert a variable to a string in JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param options Encoding options. See serialize for details.
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToJSON(Ejs *ejs, EjsAny *obj, EjsObj *options);

/*
    Serialization flags
 */
#define EJS_JSON_SHOW_COMMAS        0x1     /**< ejsSerialize flag to always put commas after properties*/
#define EJS_JSON_SHOW_HIDDEN        0x2     /**< ejsSerialize flag to include hidden properties */
#define EJS_JSON_SHOW_NAMESPACES    0x4     /**< ejsSerialize flag to include namespaces in names */
#define EJS_JSON_SHOW_PRETTY        0x8     /**< ejsSerialize flag to render in human-readible multiline format */
#define EJS_JSON_SHOW_SUBCLASSES    0x10    /**< ejsSerialize flag to include subclass properties */
#define EJS_JSON_SHOW_NOQUOTES      0x20    /**< ejsSerialize flag to omit quotes if property has no spaces */
#define EJS_JSON_SHOW_REGEXP        0x40    /**< ejsSerialize flag to emit native RegExp literals */

/**
    Serialize a variable into JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param flags Serialization options. The supported options are:
        <ul>
        <li> EJS_JSON_SHOW_COMMAS - Always use commas after properties</li> 
        <li> EJS_JSON_SHOW_HIDDEN - Include hidden properties </li>
        <li> EJS_JSON_SHOW_NOQUOTES - Omit quotes on properties if possible</li> 
        <li> EJS_JSON_SHOW_NAMESPACES - Include namespaces in property names </li>
        <li> EJS_JSON_SHOW_REGEXP - Emit native regular expression literals</li>
        <li> EJS_JSON_SHOW_PRETTY - Use human-readable multiline presentation </li> 
        <li> EJS_JSON_SHOW_SUBCLASSES - Include subclass properties </li>
        </ul>
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSerialize(Ejs *ejs, EjsAny *obj, int flags);

/**
    Serialize a variable into JSON format
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Value to cast
    @param options Serialization options. The supported options are:
        <ul>
        <li> baseClasses - Include subclass properties </li>
        <li> hidden - Include hidden properties </li>
        <li> namespaces - Include namespaces in property names </li>
        <li> pretty - Use human-readable multiline presentation </li> 
        <li> depth - Set a maximum depth to recurse in the object</li> 
        <li> replacer - Function that determines how object values are stringified for objects without a toJSON method. 
                The replace has the following signature: function replacer(key: String, value: String): String</li>
        </ul>
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsSerializeWithOptions(Ejs *ejs, EjsAny *obj, EjsObj *options);

/** 
    Cast a variable to a string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A string object
    @ingroup EjsString
 */
PUBLIC EjsString *ejsToString(Ejs *ejs, EjsAny *obj);

/**
    Convert a string to a literal string style representation
    @description The object is converted to a string and then is wrapped with quotes. Embedded quotes and backquotes
        are backquoted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A string representation of the object
 */
PUBLIC EjsString *ejsToLiteralString(Ejs *ejs, EjsObj *obj);

/*
    Internal
 */
PUBLIC void ejsManageString(struct EjsString *sp, int flags);

/******************************************** EjsArray ********************************************/
/** 
    Array class
    @description Arrays provide a resizable, integer indexed, in-memory store for objects. An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered). Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list. The Array class can store objects with numerical indicies and can also store 
        any named properties. The named properties are stored in the obj field, whereas the numeric indexed values are
        stored in the data field. Array extends EjsObj and has all the capabilities of EjsObj.
    @defgroup EjsArray EjsArray
    @see EjsArray ejsAddItem ejsClearArray ejsCloneArray ejsCreateArray ejsGetFirstItem ejsGetItem ejsGetLastItem 
        ejsGetNextItem ejsGetPrevItem ejsInsertItem ejsAppendArray ejsLookupItem ejsRemoveItem ejsRemoveItemAtPos 
        ejsRemoveLastItem 
    @stability Internal
 */
typedef struct EjsArray {
    EjsPot          pot;                /**< Property storage */
    EjsObj          **data;             /**< Array elements */
    int             length;             /**< Array length property */
} EjsArray;


/** 
    Append an array
    @description This will append the contents of the source array to the destination array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Destination array to modify
    @param src Source array from which to copy elements
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsArray
 */
PUBLIC int ejsAppendArray(Ejs *ejs, EjsArray *dest, EjsArray *src);

/** 
    Create an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param size Initial size of the array
    @return A new array object
    @ingroup EjsArray
 */
PUBLIC EjsArray *ejsCreateArray(Ejs *ejs, int size);

/** 
    Clone an array
    @description This will create a new array and copy the contents from the source array. Both array elements and
        object properties are copied. If deep is true, the call creates a distinct clone with no shared elements. If
        deep is false, object references will be copied and shared between the source and cloned array.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Array source
    @param deep Set to true to clone each element of the array. Otherwise object references will have their references
        copied and not the reference targets.
    @return A new array.
    @ingroup EjsArray
 */
PUBLIC EjsArray *ejsCloneArray(Ejs *ejs, EjsArray *ap, bool deep);

/** 
    Add an item to the array
    @description This will add a new item to the end of the array and grow the array if required.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Array to modify
    @param item Object item to add.
    @return The item index in the array.
    @ingroup EjsArray
 */
PUBLIC int ejsAddItem(Ejs *ejs, EjsArray *ap, EjsAny *item);

/** 
    Clear an array and remove all items
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @ingroup EjsArray
 */
PUBLIC void ejsClearArray(Ejs *ejs, EjsArray *ap);

/** 
    Get an item from an array
    @description This will retrieve the item at the index location
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param index Location to retrieve
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetItem(Ejs *ejs, EjsArray *ap, int index);

/** 
    Get the first item from an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetFirstItem(Ejs *ejs, EjsArray *ap);

/** 
    Get the last item from an array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetLastItem(Ejs *ejs, EjsArray *ap);

/** 
    Get the next item from an array
    @description This will retrieve the item at *next and increment *next
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param next Pointer to an integer index. The *next location is updated to prepare to advance to the next element.
        The *next location should be initialized to zero for the first call to an ejsGetNextItem sequence.
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetNextItem(Ejs *ejs, EjsArray *ap, int *next);

/** 
    Get the previous item from an array
    @description This will retrieve the item at *prev and increment *prev
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param prev Pointer to an integer index. The *prev location is updated to prepare to advance to the previous element.
        The *prev location should be initialized to zero for the first call to an ejsGetPrevItem sequence.
    @return The item
    @ingroup EjsArray
 */
PUBLIC EjsAny *ejsGetPrevItem(Ejs *ejs, EjsArray *ap, int *prev);

/** 
    Insert an item
    @description This will insert an item at the given index. Items at the index and above will be moved upward to 
        make room for the inserted item.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param index Index at which to insert the item. The item will be inserted at the "index" position.
    @param item Item to insert
    @return The index.
    @ingroup EjsArray
 */
PUBLIC int ejsInsertItem(Ejs *ejs, EjsArray *ap, int index, EjsAny *item);

/** 
    Join an array
    @description This will join the array elements using the given join string separator.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param join String to use as a delimiter between elements.
    @return The result string
    @ingroup EjsArray
 */
PUBLIC EjsString *ejsJoinArray(Ejs *ejs, EjsArray *ap, EjsString *join);

/** 
    Lookup an item in the array
    @description This search for the given item (reference) in the array. NOTE: currently numbers are implemented as
        object references and so using this routine to search for a number reference will not work.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to examine
    @param item Item to search for
    @return A positive array element index. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsLookupItem(Ejs *ejs, EjsArray *ap, EjsAny *item);

/** 
    Remove an item from the array
    @description This will remove an item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param item Item to remove
    @param compact Set to true to compact following properties 
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveItem(Ejs *ejs, EjsArray *ap, EjsAny *item, int compact);

/** 
    Remove the last item from the array
    @description This will remove the last item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveLastItem(Ejs *ejs, EjsArray *ap);

/** 
    Remove an item at a given index from the array
    @description This will remove an item from the array. The array will not be compacted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ap Source array to modify
    @param index Array index from which to remove the item
    @param compact Set to true to compact following properties 
    @return The index where the item was found. Otherwise return MPR_ERR_CANT_FIND.
    @ingroup EjsArray
 */
PUBLIC int ejsRemoveItemAtPos(Ejs *ejs, EjsArray *ap, int index, int compact);

/*
    Internal
 */
PUBLIC EjsArray *ejsSortArray(Ejs *ejs, EjsArray *ap, int argc, EjsObj **argv);

/************************************************ Block ********************************************************/
/** 
    Block class
    @description The block class is the base class for all program code block scope objects. This is an internal class
        and not exposed to the script programmer.
    Blocks (including types) may describe their properties via traits. The traits store the property 
    type and access attributes and are stored in EjsBlock which is a sub class of EjsObj. See ejsBlock.c for details.
    @defgroup EjsBlock EjsBlock
    @see EjsBlock ejsIsBlock ejsBindFunction
    @stability Internal
 */
typedef struct EjsBlock {
    EjsPot          pot;                            /**< Property storage */
    MprList         namespaces;                     /**< Current list of namespaces open in this block of properties */
    struct EjsBlock *scope;                         /**< Lexical scope chain for this block */
    struct EjsBlock *prev;                          /**< Previous block in activation chain */

    //  TODO -- OPT and compress / eliminate some of these fields. Every function has these.
    EjsObj          *prevException;                 /**< Previous exception if nested exceptions */
    EjsObj          **stackBase;                    /**< Start of stack in this block */
    uchar           *restartAddress;                /**< Restart instruction address */
    uint            nobind: 1;                      /**< Don't bind to properties in this block */
#if BIT_DEBUG
    struct EjsLine  *line;
#endif
} EjsBlock;

#if DOXYGEN
    /** 
        Determine if a variable is a block.
        @description This call tests if the variable is a block.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @returns True if the variable is based on EjsBlock
        @ingroup EjsBlock
     */
    extern bool ejsIsBlock(Ejs *ejs, EjsObj *obj);
#else
    #define ejsIsBlock(ejs, obj) (ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isBlock)
#endif

/*  
    These are an internal APIs. Native types should probably not be using these routines. Speak up if you find
    you need these routines in your code.
 */
PUBLIC int ejsAddNamespaceToBlock(Ejs *ejs, EjsBlock *blockRef, struct EjsNamespace *nsp);
PUBLIC EjsBlock *ejsCloneBlock(Ejs *ejs, EjsBlock *src, bool deep);
PUBLIC EjsBlock *ejsCreateBlock(Ejs *ejs, int numSlots);
PUBLIC void ejsCreateBlockHelpers(Ejs *ejs);
PUBLIC int ejsGetNamespaceCount(EjsBlock *block);
PUBLIC void ejsManageBlock(EjsBlock *block, int flags);
PUBLIC void ejsPopBlockNamespaces(EjsBlock *block, int count);
PUBLIC void ejsResetBlockNamespaces(Ejs *ejs, EjsBlock *block);

#if BIT_DEBUG
    #define ejsSetBlockLocation(block, loc) block->line = loc
#else
    #define ejsSetBlockLocation(block, loc)
#endif

/******************************************** Emitter *********************************************/
/** 
    Add an observer 
    @description Add an observer for events 
        when implementing the createVar helper.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitterPtr Reference to an emitter. If the reference location is NULL, a new emitter will be created and 
        passed back via *emitterPtr.
    @param name Name of events to observe. Can be an array of events.
    @param observer Function to run when selected events are triggered
    @return Zero if successful.
    @ingroup EjsObj
 */
PUBLIC int ejsAddObserver(Ejs *ejs, EjsObj **emitterPtr, EjsObj *name, struct EjsFunction *observer);

/** 
    Remove an observer 
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter created via #ejsAddObserver.
    @param name Name of observed events. Can be an array of events.
    @param observer Observer function provided to #ejsAddObserver
    @return Zero if successful.
    @ingroup EjsObj
 */
PUBLIC int ejsRemoveObserver(Ejs *ejs, EjsObj *emitter, EjsObj *name, struct EjsFunction *observer);

/** 
    Send an event to observers
    @description This call allows multiple arguments to be passed to the observer. If you only need to pass one, 
        use #ejsSendEvent.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter object
    @param name Name of event to fire
    @param thisObj Object to use for "this" when invoking the observer
    @param argc Argument count of argv
    @param argv Arguments to pass to the observer
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsSendEventv(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, int argc, void *argv);

/** 
    Send an event to observers
    @description This call allows one argument to pass to the observer. If you need to pass more, use #ejsSendEventv.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param emitter Emitter object
    @param name Name of event to fire
    @param thisObj Object to use for "this" when invoking the observer
    @param arg Argument to pass to the observer
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsObj
 */
PUBLIC int ejsSendEvent(Ejs *ejs, EjsObj *emitter, cchar *name, EjsAny *thisObj, EjsAny *arg);

/******************************************** Function ********************************************/
/*
    Exception flags and structure
 */
#define EJS_EX_CATCH            0x1             /**< EjsEx flag for a catch block */
#define EJS_EX_FINALLY          0x2             /**< EjsEx flag for a finally block */
#define EJS_EX_ITERATION        0x4             /**< EjsEx flag for an iteration catch block */
#define EJS_EX_INC              4               /**< Growth increment for exception handlers */

/** 
    Exception Handler Record
    @description Each exception handler has an exception handler record.
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsEx {
    // TODO - OPT. Should this be compressed via bit fields for flags Could use short for these offsets.
    struct EjsType  *catchType;             /**< Type of error to catch */
    uint            flags;                  /**< Exception flags */
    uint            tryStart;               /**< Ptr to start of try block */
    uint            tryEnd;                 /**< Ptr to one past the end */
    uint            handlerStart;           /**< Ptr to start of catch/finally block */
    uint            handlerEnd;             /**< Ptr to one past the end */
    uint            numBlocks;              /**< Count of blocks opened before the try block */
    uint            numStack;               /**< Count of stack slots pushed before the try block */
} EjsEx;

#define EJS_INDEX_INCR  256                 /**< Constant pool growth increment */

/**
    Constant pool for module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsConstants {
    char          *pool;                    /**< Constant pool string data */
    ssize         poolSize;                 /**< Size of constant pool storage in bytes */
    ssize         poolLength;               /**< Length of used bytes in constant pool */
    int           indexSize;                /**< Size of index in elements */
    int           indexCount;               /**< Number of constants used in index */
    int           locked;                   /**< No more additions allowed */
    MprHash       *table;                   /**< Hash table for fast lookup when compiling */
    EjsString     **index;                  /**< Interned string index */
} EjsConstants;

/**
    Symbolic debugging storage for source code in module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsLine {
    int         offset;                     /**< Optional PC offsets of each line in function */
    wchar       *source;                    /**< Program source code. Format: path line: code */         
} EjsLine;

#define EJS_DEBUG_INCR      16              /**< Growth increment for EjsDebug */
#define EJS_DEBUG_MAGIC     0x78654423      /**< Debug record integrity check */
#define EJS_CODE_MAGIC      0x91917128      /**< Code record integrity check */

/**
    Debug record for module files
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsDebug {
    int         magic;
    ssize      size;                        /**< Size of lines[] in elements */
    int        numLines;                    /**< Number of entries in lines[] */
    EjsLine    lines[ARRAY_FLEX];           /**< Debug lines */
} EjsDebug;

/*
    Internal
 */
PUBLIC EjsDebug *ejsCreateDebug(Ejs *ejs, int length);
PUBLIC int ejsAddDebugLine(Ejs *ejs, EjsDebug **debug, int offset, wchar *source);
PUBLIC EjsLine *ejsGetDebugLine(Ejs *ejs, struct EjsFunction *fun, uchar *pc);
PUBLIC int ejsGetDebugInfo(Ejs *ejs, struct EjsFunction *fun, uchar *pc, char **path, int *lineNumber, wchar **source);

/** 
    Byte code
    @description This structure describes a sequence of byte code for a function. It also defines a set of
        execption handlers pertaining to this byte code.
    @ingroup EjsFunction
    @stability Internal
 */
typedef struct EjsCode {
    // TODO OPT. Could compress this.
    int              magic;                  /**< Debug magic id */
    struct EjsModule *module;                /**< Module owning this function */
    EjsDebug         *debug;                 /**< Source code debug information */
    EjsEx            **handlers;             /**< Exception handlers */
    int              codeLen;                /**< Byte code length */
    int              debugOffset;            /**< Offset in mod file for debug info */
    int              numHandlers;            /**< Number of exception handlers */
    int              sizeHandlers;           /**< Size of handlers array */
    uchar            byteCode[ARRAY_FLEX];   /**< Byte code */
} EjsCode;

/**
    Native Function signature
    @description This is the calling signature for C Functions.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param thisObj Reference to the "this" object. (The object containing the method).
    @param argc Number of arguments.
    @param argv Array of arguments.
    @returns Returns a result variable or NULL on errors and exceptions.
    @ingroup EjsFunction
    @stability Evolving.
 */
#if DOXYGEN
typedef EjsObj* (*EjsProc)(Ejs *ejs, EjsAny *thisObj, int argc, struct EjsObj **argv);
#else
typedef struct EjsObj *(*EjsProc)(Ejs *ejs, EjsAny *thisObj, int argc, struct EjsObj **argv);
#endif

/** 
    Function class
    @description The Function type is used to represent closures, function expressions and class methods. 
        It contains a reference to the code to execute, the execution scope and possibly a bound "this" reference.
    @defgroup EjsFunction EjsFunction
    @see EjsFunction ejsIsFunction ejsIsNativeFunction ejsIsInitializer ejsCreateFunction ejsCloneFunction
        ejsRunFunctionBySlot ejsRunFunction ejsRunInitializer
        ejsIsFunction ejsBindFunction ejsCloneFunction ejsCreateFunction ejsInitFunction
        ejsCreateBareFunction ejsCreateActivation ejsRemoveConstructor ejsRunInitializer ejsRunFunction
        ejsRunFunctionBySlot ejsrunFunctionByName 
    @stability Internal
 */
typedef struct EjsFunction {
    /*
        A function can store properties like any other object. If it has parameters, it must also must maintain an
        activation object. When compiling, the compiler stores parameters in the normal property "block", it then
        transfers them into the activation block when complete.
     */
    EjsBlock        block;                  /** Function properties */
    EjsPot          *activation;            /** Parameter and local properties */
    EjsString       *name;                  /** Function name */
#if FUTURE
    union {
#endif
        struct EjsFunction *setter;         /**< Setter function for this property */
        struct EjsType  *archetype;         /**< Type to use to create instances */
#if FUTURE
    } extra;
#endif
    union {
        EjsCode     *code;                  /**< Byte code */
        EjsProc     proc;                   /**< Native function pointer */
    } body;

    struct EjsArray *boundArgs;             /**< Bound "args" */
    EjsAny          *boundThis;             /**< Bound "this" object value */
    struct EjsType  *resultType;            /**< Return type of method */
    int             endFunction;            /**< Offset in mod file for end of function */

    uint    numArgs: 8;                     /**< Count of formal parameters */
    uint    numDefault: 8;                  /**< Count of formal parameters with default initializers */
    uint    allowMissingArgs: 1;            /**< Allow unsufficient args for native functions */
    uint    castNulls: 1;                   /**< Cast return values of null */
    uint    fullScope: 1;                   /**< Closures must capture full scope */
    uint    hasReturn: 1;                   /**< Has a return stmt */
    uint    inCatch: 1;                     /**< Executing catch block */
    uint    inException: 1;                 /**< Executing catch/finally exception processing */
    uint    isConstructor: 1;               /**< Is a constructor */
    uint    isInitializer: 1;               /**< Is a type initializer */
    uint    isNativeProc: 1;                /**< Is native procedure */
    uint    moduleInitializer: 1;           /**< Is a module initializer */
    uint    rest: 1;                        /**< Has a "..." rest of args parameter */
    uint    staticMethod: 1;                /**< Is a static method */
    uint    strict: 1;                      /**< Language strict mode (vs standard) */
    uint    throwNulls: 1;                  /**< Return type cannot be null */
} EjsFunction;

#if DOXYGEN
    /** 
        Determine if a variable is a function. This will return true if the variable is a function of any kind, including
            methods, native and script functions or initializers.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Variable to test
        @return True if the variable is a function
        @ingroup EjsFunction
     */
    extern bool ejsIsFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is a native function. Functions can be either native - meaning the implementation is
            via a C function, or can be scripted.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a native function.
        @ingroup EjsFunction
     */
    extern bool ejsIsNativeFunction(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if the function is an initializer. Initializers are special functions created by the compiler to do
            static and instance initialization of classes during construction.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is an initializer
        @ingroup EjsFunction
     */
    extern bool ejsIsInitializer(Ejs *ejs, EjsAny *obj);
#else
    //  OPT
    #define ejsIsFunction(ejs, obj)       (obj && POT(obj) && ((EjsPot*) obj)->isFunction)
    #define ejsIsNativeFunction(ejs, obj) (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isNativeProc))
    #define ejsIsInitializer(ejs, obj)    (ejsIsFunction(ejs, obj) && (((EjsFunction*) (obj))->isInitializer)
#endif

/** 
    Bind a native C function to a function property
    @description Bind a native C function to an existing javascript function. Functions are typically created
        by compiling a script file of native function definitions into a mod file. When loaded, this mod file 
        will create the function properties. This routine will then bind the specified C function to the 
        function property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fun Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindFunction(Ejs *ejs, EjsAny *obj, int slotNum, void *fun);

/** 
    Clone a function
    @description Copy a function and create a new copy. This may do a shallow or deep copy. A shallow copy
        will not copy the property instances, rather it will only duplicate the property reference. A deep copy
        will recursively clone all the properties of the variable.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to clone
    @param deep Set to true to clone each property of the function. Otherwise object references will have their references
        copied and not the reference targets.
    @return The allocated activation object
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCloneFunction(Ejs *ejs, EjsFunction *fun, int deep);

//  TODO - refactor into several functions
/** 
    Create a function object
    @description This creates a function object and optionally associates byte code with the function.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCreateFunction(Ejs *ejs, EjsString *name, cuchar *code, int codeLen, int numArgs, int numDefault,
    int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, EjsBlock *scope, 
    int strict);

/** 
    Initialize a function object
    @description This initializes a pre-existing function object and optionally associates byte code with the function.
        This is useful to create constructors which are stored inside type objects.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function object.
    @param name Function name used in stack backtraces.
    @param code Pointer to the byte code. The byte code is not copied so this must be a persistent pointer.
    @param codeLen Length of the code.
    @param numArgs Number of formal arguments to the function.
    @param numDefault Number of default args to the function.
    @param numExceptions Number of exception handlers
    @param returnType Return type of the function. Set to NULL for no defined type.
    @param attributes Integer mask of access attributes.
    @param module Reference to the module owning the function.
    @param scope Reference to the chain of blocks that that comprises the lexical scope chain for this function.
    @param strict Run code in strict mode (vs standard).
    @return An initialized function object
    @ingroup EjsFunction
 */
PUBLIC int ejsInitFunction(Ejs *ejs, EjsFunction *fun, EjsString *name, cuchar *code, int codeLen, int numArgs, 
    int numDefault, int numExceptions, struct EjsType *returnType, int attributes, struct EjsModule *module, 
    EjsBlock *scope, int strict);

/** 
    Create a bare function 
    @description This creates a function without code, exceptions or module linkage
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Function name
    @param attributes Function attributes
    @return The allocated function
    @ingroup EjsFunction
 */
PUBLIC EjsFunction *ejsCreateBareFunction(Ejs *ejs, EjsString *name, int attributes);

/** 
    Create an activation record for a function
    @description This creates an activation object that stores the local variables for a function
        This is a onetime operation and is not done for each function invocation.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param numSlots Number of local variables to reserve room for
    @return The allocated activation object
    @ingroup EjsFunction
 */
PUBLIC EjsPot *ejsCreateActivation(Ejs *ejs, EjsFunction *fun, int numSlots);

/** 
    Remove a constructor function from a type.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type reference
    @ingroup EjsFunction
 */
PUBLIC void ejsRemoveConstructor(Ejs *ejs, struct EjsType *type);

/** 
    Run the initializer for a module
    @description A module's initializer runs global code defined in the module
    @param ejs Ejs reference returned from #ejsCreateVM
    @param module Module object reference
    @return The last expression result of global code executed
    @ingroup EjsFunction
 */
PUBLIC EjsObj *ejsRunInitializer(Ejs *ejs, struct EjsModule *module);

/** 
    Run a function
    @description Run a function with the given actual parameters
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fn Function object to run
    @param thisObj Object to use as the "this" object when running the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunction(Ejs *ejs, EjsFunction *fn, EjsAny *thisObj, int argc, void *argv);

/** 
    Run a function by slot number
    @description Run a function identified by slot number with the given actual parameters. This will run the function
        stored at slotNum in the obj variable. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object that holds the function at its "slotNum" slot. Also use this object as the "this" object 
        when running the function.
    @param slotNum Slot number in obj that contains the function to run.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunctionBySlot(Ejs *ejs, EjsAny *obj, int slotNum, int argc, void *argv);

/** 
    Run a function by name
    @description Run a function identified by name in the given container with the given actual parameters. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param container Object that holds the function at its "name". 
    @param qname Qualified name for the function in container.
    @param thisObj Object to use as "this" when invoking the function.
    @param argc Count of actual parameters
    @param argv Vector of actual parameters
    @return The return value from the function. If an exception is thrown, NULL will be returned and ejs->exception
        will be set to the exception object.
    @ingroup EjsFunction
 */
PUBLIC EjsAny *ejsRunFunctionByName(Ejs *ejs, EjsAny *container, EjsName qname, EjsAny *thisObj, int argc, void *argv);

/** 
    Add an exception record
    @description This creates an exception record to define a catch or finally block.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to modify 
    @param tryStart Pointer to the bytecode start of try block
    @param tryEnd Pointer to one past the end of the try block
    @param catchType Type of error to catch. Set to null for all.
    @param handlerStart Pointer to the start of the catch / finally block 
    @param handlerEnd Pointer ot one past the end of the catch / finally block
    @param numBlocks Count of blocks opened before the try block
    @param numStack Count of stack slots pushed before the try block
    @param flags Reserved
    @param preferredIndex Preferred index in the function exceptions list. Set to -1 for the next available slot.
    @return The allocated exception object
    @ingroup EjsFunction
    @internal
 */
PUBLIC EjsEx *ejsAddException(Ejs *ejs, EjsFunction *fun, uint tryStart, uint tryEnd, struct EjsType *catchType,
    uint handlerStart, uint handlerEnd, int numBlocks, int numStack, int flags, int preferredIndex);

/** 
    Set the byte code for a function
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param module Module owning the function
    @param byteCode ByteCode buffer
    @param len Size of the byteCode buffer
    @param debug Debug record with symbolic debug information
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsFunction
    @internal
 */
PUBLIC int ejsSetFunctionCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);

/*
    Internal
 */

/** 
    Create a code block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function to examine
    @param module Module owning the function
    @param byteCode ByteCode buffer
    @param len Size of the byteCode buffer
    @param debug Debug record with symbolic debug information
    @return An allocated code block
    @ingroup EjsFunction
    @internal
 */
PUBLIC EjsCode *ejsCreateCode(Ejs *ejs, EjsFunction *fun, struct EjsModule *module, cuchar *byteCode, ssize len, 
    EjsDebug *debug);
PUBLIC void ejsManageFunction(EjsFunction *fun, int flags);
PUBLIC void ejsShowOpFrequency(Ejs *ejs);

/******************************************** Frame ***********************************************/
/**
    Frame record 
    @defgroup EjsFrame EjsFrame
    @see ejsIsFrame
    @stability Internal
 */
typedef struct EjsFrame {
    EjsFunction     function;               /**< Activation frame for function calls. Stores local variables */
    EjsFunction     *orig;                  /**< Original function frame is based on */
    struct EjsFrame *caller;                /**< Previous invoking frame */
    EjsObj          **stackBase;            /**< Start of stack in this function */
    EjsObj          **stackReturn;          /**< Top of stack to return to */
    EjsLine         *line;                  /**< Debug source line */
    uchar           *pc;                    /**< Program counter */
    uchar           *attentionPc;           /**< Restoration PC value after attention */
    uint            argc;                   /**< Actual parameter count */
    int             slotNum;                /**< Slot in owner */
    uint            getter: 1;              /**< Frame is a getter */
} EjsFrame;

#if DOXYGEN
    /** 
        Determine if a variable is a frame. Only used internally in the VM.
        @param ejs Interpreter instance returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a frame. 
        @ingroup EjsFrame
     */
    extern bool ejsIsFrame(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsFrame(ejs, obj) (obj && ejsIsPot(ejs, obj) && ((EjsPot*) (obj))->isFrame)
#endif

/*
    Internal
 */
PUBLIC EjsFrame *ejsCreateFrame(Ejs *ejs, EjsFunction *src, EjsObj *thisObj, int argc, EjsObj **argv);
PUBLIC EjsFrame *ejsCreateCompilerFrame(Ejs *ejs, EjsFunction *src);
PUBLIC EjsBlock *ejsPopBlock(Ejs *ejs);
PUBLIC EjsBlock *ejsPushBlock(Ejs *ejs, EjsBlock *block);

/******************************************** Boolean *********************************************/
/** 
    Boolean class
    @description The Boolean class provides the base class for the boolean values "true" and "false".
        EjsBoolean is a primitive native type and extends EjsObj. It is still logically an Object, but implements
        Object properties and methods itself. Only two instances of the boolean class are ever created created
        these are referenced as ejs->trueValue and ejs->falseValue.
    @defgroup EjsBoolean EjsBoolean
    @see EjsBoolean ejsCreateBoolean ejsGetBoolean ejsToBoolean
    @stability Internal
 */
typedef struct EjsBoolean {
    EjsObj  obj;                /**< Base object */
    bool    value;              /**< Boolean value */
} EjsBoolean;


#if DOXYGEN
/** 
    Create a boolean
    @description Create a boolean value. This will not actually create a new boolean instance as there can only ever
        be two boolean instances (true and false). Boolean properties are immutable in Ejscript and so this routine
        will simply return the appropriate pre-created true or false boolean value.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Desired boolean value. Set to 1 for true and zero for false.
    @ingroup EjsBoolean
 */
PUBLIC EjsBoolean *ejsCreateBoolean(Ejs *ejs, int value);
#else
#define ejsCreateBoolean(ejs, v) ((v) ? ESV(true) : ESV(false))
#endif

/** 
    Get the C boolean value from a boolean object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Boolean variable to access
    @return True or false
    @ingroup EjsBoolean
 */
PUBLIC bool ejsGetBoolean(Ejs *ejs, EjsAny *obj);

/** 
    Cast a variable to a boolean 
    @description
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to cast
    @return A new boolean object
    @ingroup EjsBoolean
 */
PUBLIC EjsBoolean *ejsToBoolean(Ejs *ejs, EjsAny *obj);

/******************************************** ByteArray *******************************************/
/** 
    ByteArray class
    @description ByteArrays provide a resizable, integer indexed, in-memory store for bytes. ByteArrays can be used as a 
    simple array type to store and encode data as bytes or they can be used as buffered Streams implementing the Stream 
    interface.
    \n\n
    When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
    extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
    can be used to get and put blocks of data. In this mode, the read and write position properties are ignored. 
    Access to the byte array is from index zero up to the size defined by the length property. When constructed, 
    the ByteArray can be designated as resizable, in which case the initial size will grow as required to accomodate 
    data and the length property will be updated accordingly.
    \n\n
    When used as a Stream, the byte array additional write methods to store data at the location specified by the 
    $writePosition property and read methods to read from the $readPosition property. The $available method 
    indicates how much data is available between the read and write position pointers. The $reset method can 
    reset the pointers to the start of the array.  When used with for/in, ByteArrays will iterate or 
    enumerate over the available data between the read and write pointers.
    \n\n
    If numeric values are read or written, they will be encoded according to the value of the endian property 
    which can be set to either LittleEndian or BigEndian. 
    \n\n
    In Stream mode ByteArrays can be configured to run in sync or async mode. Adding observers via the $addObserver
    method will put a stream into async mode. Events will then be issued for close, EOF, read and write events.
    @defgroup EjsByteArray EjsByteArray
    @see EjsByteArray ejsCopyToByteArray ejsCreateByteArray ejsGetByteArrayAvailableData ejsGetByteArrayRoom 
        ejsGrowByteArray ejsMakeRoomInByteArray ejsResetByteArray ejsSetByteArrayPositions ejsWriteToByteArray 
    @stability Internal
 */
typedef struct EjsByteArray {
    EjsObj          obj;                /**< Base object */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    uchar           *value;             /**< Data bytes in the array */
    int             async;              /**< Async mode */
    int             endian;             /**< Endian encoding */
    int             growInc;            /**< Current read position */
    ssize           readPosition;       /**< Current read position */
    ssize           writePosition;      /**< Current write position */
    ssize           size;               /**< Size property */
    int             swap;               /**< I/O must swap bytes due to endian byte ordering */
    bool            resizable;          /**< Aray is resizable */
} EjsByteArray;

/** 
    Create a byte array
    @description Create a new byte array instance.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param size Initial size of the byte array
    @return A new byte array instance
    @ingroup EjsByteArray
 */
PUBLIC EjsByteArray *ejsCreateByteArray(Ejs *ejs, ssize size);

/** 
    Set the I/O byte array positions
    @description Set the read and/or write positions into the byte array. ByteArrays implement the Stream interface
        and support sequential and random access reading and writing of data in the array. The byte array maintains
        read and write positions that are automatically updated as data is read or written from or to the array. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array object
    @param readPosition New read position to set
    @param writePosition New write position to set
    @ingroup EjsByteArray
 */
PUBLIC void ejsSetByteArrayPositions(Ejs *ejs, EjsByteArray *ba, ssize readPosition, ssize writePosition);

/** 
    Copy data into a byte array
    @description Copy data into a byte array at a specified offset. 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array object
    @param offset Offset in the byte array to which to copy the data.
    @param data Pointer to the source data
    @param length Length of the data to copy
    @return Count of bytes written or negative MPR error code.
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsCopyToByteArray(Ejs *ejs, EjsByteArray *ba, ssize offset, cchar *data, ssize length);

/** 
    Reset the byte
    @description This will reset the byte array read/write positions if the array is empty
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to modify
    @ingroup EjsByteArray
 */
PUBLIC void ejsResetByteArray(Ejs *ejs, EjsByteArray *ba);

/**
    Get the number of available bytes
    @param ba Byte array to examine
    @return The number of bytes of data available to read
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGetByteArrayAvailableData(EjsByteArray *ba);

/**
    Determine the spare room in the byte array for more data
    @param ba Byte array to examine
    @return The number of bytes the byte array can fit without growing
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGetByteArrayRoom(EjsByteArray *ba);

/**
    Grow the byte array
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to grow
    @param size The requested new size of the byte array
    @return The new size of the byte array. Otherwise EJS_ERROR if the memory cannot be allocated.
    @ingroup EjsByteArray
 */
PUBLIC ssize ejsGrowByteArray(Ejs *ejs, EjsByteArray *ba, ssize size);

/**
    Make room in the byte array for data
    @description This will ensure there is sufficient room in the byte array. If the required number of bytes of spare 
        room is not available, the byte array will grow.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to examine
    @param require Number of bytes needed.
    @return The number of bytes of data available to read
    @ingroup EjsByteArray
 */
PUBLIC bool ejsMakeRoomInByteArray(Ejs *ejs, EjsByteArray *ba, ssize require);

/**
    Write data to the byte array
    This implements the ByteArray.write function. It is most useful for other types to implement a write to byte 
        array capability.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param ba Byte array to examine
    @param argc Count of args in argv
    @param argv Arguments to write
    @return The number of bytes of data written (EjsNumber)
    @ingroup EjsByteArray
 */
PUBLIC struct EjsNumber *ejsWriteToByteArray(Ejs *ejs, EjsByteArray *ba, int argc, EjsObj **argv);

/******************************************* Cache ************************************************/
/**
    EjsCache
    @defgroup EjsCache EjsCache
    @see ejsCacheExpire ejsCacheRead ejsCacheReadObj ejsCacheRemove ejsSetCacheLimits ejsCacheWrite ejsCacheWriteObj
 */

/** 
    Expire a cache item
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param when When to expire the cache item
    @return Returns Null
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheExpire(Ejs *ejs, EjsObj *cache, struct EjsString *key, struct EjsDate *when);

/** 
    Read an item from the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param options Cache read options
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheRead(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsObj *options);

/** 
    Read an object from the cache
    @description This call reads a cache item and then deserializes using JSON encoding into an object.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param options Cache read options
    @return Cache item object.
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheReadObj(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsObj *options);

/** 
    Read an item from the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsBoolean *ejsCacheRemove(Ejs *ejs, EjsObj *cache, struct EjsString *key);

//  TODO - rename ejsSetCacheLimits
/** 
    Set the cache limits
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param limits Limits is an object hash. Depending on the cache backend in-use, the limits object may have
    some of the following properties. Consult the documentation for the actual cache backend for which properties
    are supported by the backend.
    <ul>
    <li> keys Maximum number of keys in the cache. Set to zero for no limit.</li>
    <li> lifespan Default time in seconds to preserve key data. Set to zero for no timeout.</li>
    <li> memory Total memory to allocate for cache keys and data. Set to zero for no limit.</li>
    <li> retries Maximum number of times to retry I/O operations with cache backends.</li>
    <li> timeout Maximum time to transact I/O operations with cache backends. Set to zero for no timeout.</li>
    </ul>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC EjsAny *ejsCacheSetLimits(Ejs *ejs, EjsObj *cache, EjsObj *limits);

/** 
    Write an item to the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param value Value to write
    @param options Cache write options
    <ul>
    <li> lifespan Preservation time for the key in seconds.</li>
    <li> expire When to expire the key. Takes precedence over lifetime.</li>
    <li> mode Mode of writing: "set" is the default and means set a new value and create if required.
    "add" means set the value only if the key does not already exist. "append" means append to any existing
    value and create if required. "prepend" means prepend to any existing value and create if required.</li>
    <li> version Unique version identifier to be used for a conditional write. The write will only be 
    performed if the version id for the key has not changed. This implements an atomic compare and swap. </li>
    <li> throw Throw an exception rather than returning null if the version id has been updated for the key.</li>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC struct EjsNumber *ejsCacheWrite(Ejs *ejs, EjsObj *cache, struct EjsString *key, struct EjsString *value, 
    EjsObj *options);

/** 
    Write an object to the cache
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param cache Cache object
    @param key Cache item key
    @param value Value to write
    @param options Cache write options
    <ul>
    <li> lifespan Preservation time for the key in seconds.</li>
    <li> expire When to expire the key. Takes precedence over lifetime.</li>
    <li> mode Mode of writing: "set" is the default and means set a new value and create if required.
    "add" means set the value only if the key does not already exist. "append" means append to any existing
    value and create if required. "prepend" means prepend to any existing value and create if required.</li>
    <li> version Unique version identifier to be used for a conditional write. The write will only be 
    performed if the version id for the key has not changed. This implements an atomic compare and swap. </li>
    <li> throw Throw an exception rather than returning null if the version id has been updated for the key.</li>
    @return String cache item
    @ingroup EjsCache
 */
PUBLIC struct EjsNumber *ejsCacheWriteObj(Ejs *ejs, EjsObj *cache, struct EjsString *key, EjsAny *value, EjsObj *options);

/******************************************** Cmd *************************************************/
/** 
    Cmd class
    @defgroup EjsCmd EjsCmd
    @stability Internal
 */
typedef struct EjsCmd {
    EjsPot          pot;                /**< Property storage */
    Ejs             *ejs;               /**< Interpreter back link */
    EjsObj          *emitter;           /**< Event emitter for listeners */
    MprCmd          *mc;                /**< MprCmd object */
    MprBuf          *stdoutBuf;         /**< Stdout from the command */
    MprBuf          *stderrBuf;         /**< Stderr from the command */
    EjsAny          *command;           /**< Command to run */
    EjsAny          *env;               /**< Optional environment */
    EjsAny          *options;           /**< Command options object */
    struct EjsByteArray *error;         /**< Error stream */
    cchar           **argv;             /**< Actual argv when invoking the command */
    int             argc;               /**< Length of argv */
    int             async;              /**< Async mode */
    int             throw;              /**< Set to true if the command should throw exceptions for failures */
    MprTicks        timeout;            /**< Command timeout in milliseconds */
} EjsCmd;

/******************************************** Date ************************************************/
/** 
    Date class
    @description The Date class is a general purpose class for working with dates and times. 
        is a a primitive native type and extends EjsObj. It is still logically an Object, but implements Object 
        properties and methods itself. 
    @defgroup EjsDate EjsDate
    @see EjsDate ejsCreateDate ejsGetDate ejsIsDate  
    @stability Internal
 */
typedef struct EjsDate {
    EjsObj          obj;                /**< Object base */
    MprTime         value;              /**< Time in milliseconds since "1970/01/01 GMT" */
} EjsDate;

/** 
    Create a new date instance
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Date/time value to set the new date instance to
    @return An initialized date instance
    @ingroup EjsDate
 */
PUBLIC EjsDate *ejsCreateDate(Ejs *ejs, MprTime value);

#if DOXYGEN
    /** 
        Get the numeric value stored in a EjsDate object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param date Date object to examine
        @return An MprTime value
        @ingroup EjsDate
     */
    extern MprTime ejsGetDate(Ejs *ejs, EjsDate *date);
#else
    #define ejsGetDate(ejs, obj) (ejsIs(ejs, obj, Date) ? ((EjsDate*) obj)->value : 0)
#endif

/******************************************** Error ***********************************************/
//  TODO - missing SecurityException PermissionsException
/** 
    Error classes
    @description Base class for error exception objects. Exception objects are created by programs and by the system 
    as part of changing the normal flow of execution when some error condition occurs. 
    When an exception is created and acted upon ("thrown"), the system transfers the flow of control to a 
    pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
    point at which the exception was thrown or not. It may re-throw the exception or pass control up the call stack.
    @stability Evolving.
    @defgroup EjsError  EjsError
    @see ejeCreateError ejsCaptureStack ejsGetErrorMsg ejsGetException 
        ejsHasException ejsIsError ejsThrowArgError ejsThrowArithmeticError ejsThrowAssertError ejsThrowError 
        ejsThrowIOError ejsThrowInstructionError ejsThrowInternalError ejsThrowMemoryError ejsThrowOutOfBoundsError 
        ejsThrowReferenceError ejsThrowResourceError ejsThrowStateError ejsThrowStopIteration ejsThrowString 
        ejsThrowSyntaxError ejsThrowTypeError 
 */
typedef EjsPot EjsError;

#if DOXYGEN
    /**
        Test if the given object is an error instance
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @return True if the object is an error
        @ingroup EjsError
     */
    extern bool ejsIsError(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsError(ejs, obj) (obj && ejsIsA(ejs, obj, ESV(Error)))
#endif

/**
    Create an error object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Error base type
    @param message Error message to use when constructing the error object
    @return Error object
    @ingroup EjsError
 */
PUBLIC EjsError *ejsCreateError(Ejs *ejs, struct EjsType *type, EjsObj *message);

/**
    Capture the execution stack
    @param ejs Ejs reference returned from #ejsCreateVM
    @param skip How many levels of stack to skip before capturing (counts from the top down)
    @return Array of stack records.
    @ingroup EjsError
 */
PUBLIC EjsArray *ejsCaptureStack(Ejs *ejs, int skip);

/** 
    Get the interpreter error message
    @description Return a string containing the current interpreter error message
    @param ejs Ejs reference returned from #ejsCreateVM
    @param withStack Set to 1 to include a stack backtrace in the error message
    @return A string containing the error message. The caller must not free.
    @ingroup EjsError
 */
PUBLIC cchar *ejsGetErrorMsg(Ejs *ejs, int withStack);

/**
    Get the Ejs exception object for this interpreter
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The exception object if one exists, otherwise NULL.
    @ingroup EjsError
 */
PUBLIC EjsObj *ejsGetException(Ejs *ejs);

/** 
    Determine if an exception has been thrown
    @param ejs Ejs reference returned from #ejsCreateVM
    @return True if an exception has been thrown
    @ingroup EjsError
 */
PUBLIC bool ejsHasException(Ejs *ejs);

/** 
    Throw an argument exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowArgError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an assertion exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowAssertError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an math exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowArithmeticError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an instruction code exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowInstructionError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an general error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an internal error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowInternalError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an IO exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowIOError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an Memory depletion exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowMemoryError(Ejs *ejs);

/** 
    Throw an out of bounds exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowOutOfBoundsError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an reference exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowReferenceError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an resource exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowResourceError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an state exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowStateError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an stop iteration exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsError
 */
PUBLIC EjsObj *ejsThrowStopIteration(Ejs *ejs);

/** 
    Throw a string message. This will not capture the stack as part of the exception message.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsString *ejsThrowString(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an syntax error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowSyntaxError(Ejs *ejs, cchar *fmt, ...);

/** 
    Throw an type error exception
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fmt Printf style format string to use for the error message
    @param ... Message arguments
    @ingroup EjsError
 */
PUBLIC EjsError *ejsThrowTypeError(Ejs *ejs, cchar *fmt, ...);

/******************************************** File ************************************************/
/** 
    File class
    @description The File class provides a foundation of I/O services to interact with physical files and directories.
    Each File object represents a single file or directory and provides methods for creating, opening, reading, writing 
    and deleting files, and for accessing and modifying information about the file.
    @defgroup EjsFile EjsFile 
    @see EjsFile ejsCreateFile ejsCreateFileFromFd
    @stability Internal
 */
typedef struct EjsFile {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference */
    MprFile         *file;              /**< Open file handle */
    MprPath         info;               /**< Cached file info */
    char            *path;              /**< Filename path */
    char            *modeString;        /**< User supplied mode string */
    int             mode;               /**< Current open mode */
    int             perms;              /**< Posix permissions mask */
    int             attached;           /**< Attached to existing descriptor */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsFile;

/** 
    Create a File object
    @description Create a file object associated with the given filename. The filename is not opened, just stored.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param filename Filename to associate with the file object
    @return A new file object
    @ingroup EjsFile
 */
PUBLIC EjsFile *ejsCreateFile(Ejs *ejs, cchar *filename);

/**
    Create a file object from an O/S file descriptor
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fd O/S file descriptor handle
    @param name Filename to associate with the file object
    @param mode O/S file access mode (see man 2 open)
    @return A new file object
    @ingroup EjsFile
 */
PUBLIC EjsFile *ejsCreateFileFromFd(Ejs *ejs, int fd, cchar *name, int mode);

/******************************************** Path ************************************************/
/**
    Path class
    @description The Path class provides file path name services.
    @defgroup EjsPath EjsPath 
    @see EjsFile ejsCreatePath ejsCreatePathFromAsc ejsToPath
    @stability Internal
 */
typedef struct EjsPath {
    EjsObj          obj;                /**< Base object */
    cchar           *value;             /**< Filename path */
    MprPath         info;               /**< Cached file info */
    MprList         *files;             /**< File list for enumeration */
#if FUTURE
    cchar           *cygdrive;          /**< Cygwin drive directory (c:/cygdrive) */
    cchar           *newline;           /**< Newline delimiters */
    int             delimiter;          /**< Path delimiter ('/' or '\\') */
    int             hasDriveSpecs;      /**< Paths on this file system have a drive spec */
#endif
} EjsPath;

/** 
    Create a Path object
    @description Create a path object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path object
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsCreatePath(Ejs *ejs, EjsString *path);

/** 
    Create a Path object
    @description Create a path object from the given ascii path string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Null terminated Ascii pathname.
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsCreatePathFromAsc(Ejs *ejs, cchar *path);

/** 
    Convert the object to a Path
    @description Convert the object to a string and then to a Path.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to convert
    @return A new Path object
    @ingroup EjsPath
 */
PUBLIC EjsPath *ejsToPath(Ejs *ejs, EjsAny *obj);

/** 
    Set the owner, group and permissions of a file.
    @description Convert the object to a string and then to a Path.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path name to modify
    @param options Owner, group and permissions options.
    @arg permissions optional Posix permissions number mask. Defaults to 0664.
    @arg owner String representing the file owner
    @arg group String representing the file group
    @return A new Path object
    @ingroup EjsPath
    @internal
 */
PUBLIC int ejsSetPathAttributes(Ejs *ejs, cchar *path, EjsObj *options);

/**
    Get files below a path. 
    @description Expand wild cards in a path. Function used to implement Path.files().
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path to use as the base 
    @param argc Count of args (set to 1)
    @param argv Args array. (Set to an array with a single element)
    @return Array of matching paths
  */
PUBLIC EjsArray *ejsGetPathFiles(Ejs *ejs, EjsPath *path, int argc, EjsObj **argv);

/******************************************** FileSystem*******************************************/
/** 
    FileSystem class
    @description The FileSystem class provides file system services.
    @defgroup EjsFileSystem EjsFileSystem 
    @see EjsFile ejsCreateFile 
    @stability Internal
 */
typedef struct EjsFileSystem {
    EjsObj          obj;                /**< Base object */
    char            *path;              /**< Filename path */
    MprFileSystem   *fs;                /**< MPR file system object */
} EjsFileSystem;


/** 
    Create a FileSystem object
    @description Create a file system object associated with the given pathname.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param path Path to describe the file system. Can be any path in the file system.
    @return A new file system object
    @ingroup EjsPath
 */
PUBLIC EjsFileSystem *ejsCreateFileSystem(Ejs *ejs, cchar *path);

/*
    Internal
 */
PUBLIC void ejsFreezeGlobal(Ejs *ejs);
PUBLIC void ejsCreateGlobalNamespaces(Ejs *ejs);
PUBLIC void ejsDefineGlobalNamespaces(Ejs *ejs);

/******************************************** Http ************************************************/
/** 
    Http Class
    @description
        Http objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @defgroup EjsHttp EjsHttp
    @see EjsHttp ejsCreateHttp ejsGetHttpLimits ejsSetHttpLimits ejsSetupHttpTrace ejsLoadHttpService
    @stability Internal
 */
typedef struct EjsHttp {
    EjsObj          obj;                        /**< Base object */
    Ejs             *ejs;                       /**< Interp reference */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsByteArray    *data;                      /**< Buffered write data */
    EjsObj          *limits;                    /**< Limits object */
    EjsString       *responseCache;             /**< Cached response (only used if response() is used) */
    HttpConn        *conn;                      /**< Http connection object */
    MprSsl          *ssl;                       /**< SSL configuration */
    MprBuf          *requestContent;            /**< Request body data supplied */
    MprBuf          *responseContent;           /**< Response data */
    char            *uri;                       /**< Target uri */
    char            *method;                    /**< HTTP method */
    char            *keyFile;                   /**< SSL key file */
    char            *caFile;                    /**< SSL CA certificate file */
    char            *certFile;                  /**< SSL certificate file */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    ssize           readCount;                  /**< Count of body bytes read */
    ssize           requestContentCount;        /**< Count of bytes written from requestContent */
    ssize           writeCount;                 /**< Count of bytes written via write() */
} EjsHttp;

/*
    Thse constants match Stream.READ, Stream.WRITE, Stream.BOTH
 */
#define EJS_STREAM_READ     0x1         /**< Http constant Stream.Read */
#define EJS_STREAM_WRITE    0x2         /**< Http constant Stream.Write */
#define EJS_STREAM_BOTH     0x3         /**< Http constant Stream.Both */

/** 
    Create a new Http object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return a new Http object
    @ingroup EjsHttp
 */
PUBLIC EjsHttp *ejsCreateHttp(Ejs *ejs);

/**
    Get a Http limits 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to contain the limits properties
    @param limits The HttpLimits object 
    @param server Set to true if defining server side limits
    @ingroup EjsHttp
    @internal
 */
PUBLIC void ejsGetHttpLimits(Ejs *ejs, EjsObj *obj, HttpLimits *limits, bool server);

/** 
    Load the Http service
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsPath
    @internal
 */
PUBLIC void ejsLoadHttpService(Ejs *ejs);

/**
    Set a Http limits 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param limits The HttpLimits object receiving the limit settings
    @param obj Object containing the limits values
    @param server Set to true if defining server side limits
    @ingroup EjsHttp
    @internal
 */
PUBLIC void ejsSetHttpLimits(Ejs *ejs, HttpLimits *limits, EjsObj *obj, bool server);

/** 
    Setup tracing for Http transactions
    @param ejs Ejs reference returned from #ejsCreateVM
    @param trace HttpTrace object
    @param options Trace options
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsPath
    @internal
 */
PUBLIC int ejsSetupHttpTrace(Ejs *ejs, HttpTrace *trace, EjsObj *options);

/******************************************** WebSocket ************************************************/
/** 
    WebSocket Class
    @description Client side WebSocket support
    @defgroup EjsWebSocket EjsWebSocket
    @see EjsWebSocket ejsCreateWebSocket 
    @stability Internal
 */
typedef struct EjsWebSocket {
    EjsPot          pot;                        /**< Base pot */
    Ejs             *ejs;                       /**< Interp reference */
    EjsObj          *emitter;                   /**< Event emitter */
    HttpConn        *conn;                      /**< Underlying HttpConn object */
    MprSsl          *ssl;                       /**< SSL configuration */
    char            *certFile;                  /**< SSL certificate file */
    char            *uri;                       /**< Target URI */
    char            *protocols;                 /**< Set of supported protocols */
    char            *protocol;                  /**< Protocol selected by the server */
    int             closed;                     /**< Http is closed and "close" event has been issued */
    int             error;                      /**< Http errored and "error" event has been issued */
    int             frames;                     /**< Preserve frames */
} EjsWebSocket;

/** 
    Create a new WebSocket object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return a new WebSocket object
    @ingroup EjsWebSocket
 */
PUBLIC EjsWebSocket *ejsCreateWebSocket(Ejs *ejs);


/******************************************** Iterator ********************************************/
/** 
    Iterator Class
    @description Iterator is a helper class to implement iterators in other native classes
    @defgroup EjsIterator EjsIterator
    @see EjsIterator ejsCreateIterator
    @stability Internal
 */
typedef struct EjsIterator {
    EjsObj          obj;                /**< Base object */
    EjsObj          *target;            /**< Object to be enumerated */
    EjsProc         nativeNext;         /**< Native next function */
    bool            deep;               /**< Iterator deep (recursively over all properties) */
    EjsArray        *namespaces;        /**< Namespaces to consider in iteration */
    int             index;              /**< Current index */
    int             length;             /**< Collection length prior to iteration */
    EjsObj          *indexVar;          /**< Reference to current item */
} EjsIterator;

/** 
    Create an iterator object
    @description The EjsIterator object is a helper class for native types to implement iteration and enumeration.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param target Target variable to iterate or enumerate 
    @param length Length of collection prior to iteration.
    @param next Function to invoke to step to the next element
    @param deep Set to true to do a deep iteration/enumeration
    @param namespaces Reserved and not used. Supply NULL.
    @return A new EjsIterator object
    @ingroup EjsIterator
 */
PUBLIC EjsIterator *ejsCreateIterator(Ejs *ejs, EjsAny *target, int length, void *next, bool deep, EjsArray *namespaces);

/******************************************** Namespace *******************************************/
/** 
    Namespace Class
    @description Namespaces are used to qualify names into discrete spaces.
    @defgroup EjsNamespace EjsNamespace
    @see EjsNamespace ejsCreateNamespace ejsCreateReservedNamespace ejsDefineReservedNamespace ejsFormatReservedNamespace 
    @stability Internal
 */
typedef struct EjsNamespace {
    EjsObj          obj;                /**< Base object */
    EjsString       *value;             /**< Textual name of the namespace */
} EjsNamespace;

/** 
    Create a namespace object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Space name to use for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsCreateNamespace(Ejs *ejs, EjsString *name);

/** 
    Create a reserved namespace
    @param ejs Ejs reference returned from #ejsCreateVM
    @param typeName Type on which to base the formatted namespace name
    @param name Formatted base name for the namespace
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsCreateReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *name);

/** 
    Define a reserved namespace on a block
    @param ejs Ejs reference returned from #ejsCreateVM
    @param block Block to modify
    @param typeName Type on which to base the formatted namespace name
    @param name Formatted base name for the namespace
    @param block Block to modify
    @return A new namespace object
    @ingroup EjsNamespace
 */
PUBLIC EjsNamespace *ejsDefineReservedNamespace(Ejs *ejs, EjsBlock *block, EjsName *typeName, cchar *name);

/** 
    Format a reserved namespace name to create a unique namespace. 
    @description This is used to extend the "internal", "public", "private", and "protected" namespaces to be 
    unique for their owning class.
    \n\n
    Namespaces are formatted as strings using the following format, where type is optional. Types may be qualified.
        [type,space]
    \n\n
    Example: [debug::Shape,public] where Shape was declared as "debug class Shape"
    @param ejs Ejs reference returned from #ejsCreateVM
    @param typeName Type on which to base the formatted namespace name
    @param spaceName Namespace name
    @return A string containing the formatted name
    @ingroup EjsNamespace
 */
PUBLIC EjsString *ejsFormatReservedNamespace(Ejs *ejs, EjsName *typeName, EjsString *spaceName);

/******************************************** Null ************************************************/
/** 
    Null Class
    @description The Null class provides the base class for the singleton null instance. This instance is stored
        in ejs->nullValue.
    @stability Evolving
    @defgroup EjsNull EjsNull
    @see EjsNull ejsCreateNull
 */
typedef EjsObj EjsNull;

/** 
    Create the null object
    @description There is one null object in the system.
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The null object
    @ingroup EjsNull
    @internal
 */
PUBLIC EjsNull *ejsCreateNull(Ejs *ejs);

/******************************************** Number **********************************************/
/** 
    Number class
    @description The Number class provide the base class for all numeric values. 
        The primitive number storage data type may be set via the configure program to be either double, float, int
        or int64. 
    @defgroup EjsNumber EjsNumber
    @see EjsNumber ejsCreateNumber ejsGetDouble ejsGetInt ejsGetInt64 ejsGetNumber ejsIsInfinite ejsIsNan ejsToNumber 
    @stability Internal
 */
typedef struct EjsNumber {
    EjsObj      obj;                /**< Base object */
    MprNumber   value;              /**< Numeric value */
} EjsNumber;

/** 
    Create a number object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param value Numeric value to initialize the number object
    @return A number object
    @ingroup EjsNumber
 */
PUBLIC EjsNumber *ejsCreateNumber(Ejs *ejs, MprNumber value);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A double value
    @ingroup EjsNumber
 */
PUBLIC double ejsGetDouble(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return An integer value
    @ingroup EjsNumber
 */
PUBLIC int ejsGetInt(Ejs *ejs, EjsAny *obj);

/** 
    Get an 64 bit integer value equivalent to that stored in an EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A 64 bit integer value
    @ingroup EjsNumber
 */
PUBLIC int64 ejsGetInt64(Ejs *ejs, EjsAny *obj);

/** 
    Get the numeric value stored in a EjsNumber object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return A numeric value
    @ingroup EjsNumber
 */
PUBLIC MprNumber ejsGetNumber(Ejs *ejs, EjsAny *obj);

/** 
    Test if a number is infinite
    @param n Number to test
    @return True if the number is infinite
    @ingroup EjsNumber
 */
PUBLIC bool ejsIsInfinite(MprNumber n);

#if DOXYGEN
    /** 
        Test if a value is not-a-number
        @param n Number to test
        @return True if the number is not-a-number
        @ingroup EjsNumber
     */
    extern bool ejsIsNan(MprNumber n);
#elif WINDOWS
    #define ejsIsNan(f) (_isnan(f))
#elif MACOSX || LINUX || VXWORKS
    #define ejsIsNan(f) isnan(f)
#else
    #define ejsIsNan(f) (fpclassify(f) == FP_NAN)
#endif

/** 
    Cast a variable to a number
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to cast
    @return A number object
    @ingroup EjsNumber
 */
PUBLIC struct EjsNumber *ejsToNumber(Ejs *ejs, EjsAny *obj);

/******************************************** RegExp **********************************************/
/** 
    RegExp Class
    @description The regular expression class provides string pattern matching and substitution.
    @defgroup EjsRegExp EjsRegExp
    @see EjsRegExp ejsCreateRegExp ejsRegExpToString
    @stability Internal
 */
typedef struct EjsRegExp {
    EjsObj          obj;                /**< Base object */
    wchar           *pattern;           /**< Pattern to match */
    void            *compiled;          /**< Compiled pattern (not alloced) */
    bool            global;             /**< Search for pattern globally (multiple times) */
    bool            ignoreCase;         /**< Do case insensitive matching */
    bool            multiline;          /**< Match patterns over multiple lines */
    bool            sticky;
    int             options;            /**< Pattern matching options */
    int             endLastMatch;       /**< End of the last match (one past end) */
    int             startLastMatch;     /**< Start of the last match */
    EjsString       *matched;           /**< Last matched component */
} EjsRegExp;

/** 
    Create a new regular expression object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pattern Regular expression pattern string. The regular expression string should not contain the leading or
        trailing slash. Embedded slash characters should not be back-quoted.
    @param flags Regular expression flags. Support flags include "g" for global match, "i" to ignore case, "m" match over
        multiple lines, "y" for sticky match.
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
PUBLIC EjsRegExp *ejsCreateRegExp(Ejs *ejs, cchar *pattern, cchar *flags);

/** 
    Parse a string and create a regular expression object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param pattern Regular expression pattern string
    @return a EjsRegExp object
    @ingroup EjsRegExp
 */
PUBLIC EjsRegExp *ejsParseRegExp(Ejs *ejs, EjsString *pattern);

/** 
    Get a string representation of a regular expression
    @param ejs Ejs reference returned from #ejsCreateVM
    @param rp Regular expression 
    @return A string representation of a regular expression. The result will be of the format: "/PATTERN/suffixes"
    @ingroup EjsRegExp
 */
PUBLIC EjsString *ejsRegExpToString(Ejs *ejs, EjsRegExp *rp);

/******************************************** Socket **********************************************/
/**
    Socket Class
    @description
    @defgroup EjsSocket EjsSocket
    @see EjsSocket ejsCreateSocket
    @stability Internal
 */
typedef struct EjsSocket {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference */
    EjsObj          *emitter;           /**< Event emitter */
    EjsByteArray    *data;              /**< Buffered write data */
    MprSocket       *sock;              /**< Underlying MPR socket object */
    char            *address;           /**< Remote address */
    int             port;               /**< Remote port */
    int             async;              /**< In async mode */
    int             mask;               /**< IO event mask */
    MprMutex        *mutex;             /**< Multithread sync */
} EjsSocket;

/** 
    Create a new Socket object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param sock Socket object
    @param async True if running in async non-blocking mode
    @return a new Socket object
    @ingroup EjsSocket
 */
PUBLIC EjsSocket *ejsCreateSocket(Ejs *ejs, MprSocket *sock, bool async);

/******************************************** Timer ***********************************************/
/** 
    Timer Class
    @description Timers manage the scheduling and execution of Ejscript functions. Timers run repeatedly 
        until stopped by calling the stop method and are scheduled with a granularity of 1 millisecond. 
    @defgroup EjsTimer EjsTimer
    @see EjsTimer
    @stability Internal
 */
typedef struct EjsTimer {
    EjsObj          obj;                /**< Base object */
    Ejs             *ejs;               /**< Interp reference - needed for background timers */
    MprEvent        *event;             /**< MPR event for the timer */
    int             drift;              /**< Timer event is allowed to drift if system conditions requrie */
    int             repeat;             /**< Timer repeatedly fires */
    int             period;             /**< Time in msec between invocations */          
    EjsFunction     *callback;          /**< Callback function */
    EjsFunction     *onerror;           /**< onerror function */
    EjsArray        *args;              /**< Callback args */
} EjsTimer;

/******************************************** Uri *************************************************/
/** 
    Uri class
    @description The Uri class provides file path name services.
    @defgroup EjsUri EjsUri 
    @see EjsFile ejsCreateUri ejsCreateUriFromAsc ejsCreateUriFromParts ejsToUri
    @stability Internal
 */
typedef struct EjsUri {
    EjsObj      obj;            /**< Base object */
    HttpUri     *uri;           /**< Decoded URI */
} EjsUri;


/** 
    Create a Uri object
    @description Create a URI object associated with the given URI string.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param uri Uri string to parse
    @return A new Uri object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUri(Ejs *ejs, EjsString *uri);

/** 
    Create a URI from an ascii path
    @param ejs Ejs reference returned from #ejsCreateVM
    @param uri URI Ascii string 
    @return A new URI object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUriFromAsc(Ejs *ejs, cchar *uri);

/** 
    @description This call constructs a URI from the given parts. Various URI parts can be omitted by setting to null.
        The URI path is the only mandatory parameter.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param scheme The URI scheme. This is typically "http" or "https".
    @param host The URI host name portion. This can be a textual host and domain name or it can be an IP address.
    @param port The URI port number. Set to zero to accept the default value for the selected scheme.
    @param path The URI path to the requested document.
    @param reference URI reference with an HTML document. This is the URI component after the "#" in the URI path.
    @param query URI query component. This is the URI component after the "?" in the URI.
    @param flags Set to HTTP_COMPLETE_URI to complete the URI by supplying missing URI parts with default values.
    @return A new URI 
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsCreateUriFromParts(Ejs *ejs, cchar *scheme, cchar *host, int port, cchar *path, cchar *query, 
        cchar *reference, int flags);

/** 
    Convert an object to a URI
    @description the object is first converted to a String and then to a URI.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Any object
    @return A new URI object
    @ingroup EjsUri
 */
PUBLIC EjsUri *ejsToUri(Ejs *ejs, EjsAny *obj);

/******************************************** Worker **********************************************/
/** 
    Worker Class
    @description The Worker class provides the ability to create new interpreters in dedicated threads
    @defgroup EjsWorker EjsWorker
    @see EjsObj ejsCreateWorker ejsRemoveWorkers
    @stability Internal
 */
typedef struct EjsWorker {
    EjsPot          pot;                /**< Property storage */
    char            *name;              /**< Optional worker name */
    Ejs             *ejs;               /**< Interpreter */
    EjsAny          *event;             /**< Current event object */
    struct EjsWorker *pair;             /**< Corresponding worker object in other thread */
    char            *scriptFile;        /**< Script or module to run */
    EjsString       *scriptLiteral;     /**< Literal script string to run */
    int             state;              /**< Worker state */
    int             inside;             /**< Running inside the worker */
    int             complete;           /**< Worker has completed its work */
    int             gotMessage;         /**< Worker has received a message */
} EjsWorker;

#define EJS_WORKER_BEGIN        1                   /**< Worker state before starting */
#define EJS_WORKER_STARTED      2                   /**< Worker state once started a script */
#define EJS_WORKER_CLOSED       3                   /**< Worker state when finished */
#define EJS_WORKER_COMPLETE     4                   /**< Worker state when completed all messages */

/** 
    Create a worker
    @description This creates a bare worker object
    @param ejs Ejs reference returned from #ejsCreateVM
    @return A new worker object
    @ingroup EjsWorker
 */
PUBLIC EjsWorker *ejsCreateWorker(Ejs *ejs);

/** 
    Remove workers before exiting
    @param ejs Ejs reference returned from #ejsCreateVM
    @ingroup EjsWorker
    @internal
 */
PUBLIC void ejsRemoveWorkers(Ejs *ejs);

/******************************************** Void ************************************************/
/** 
    Void class
    @description The Void class provides the base class for the singleton "undefined" instance. This instance is stored
        in ejs->undefinedValue..
    @stability Evolving
    @defgroup EjsVoid EjsVoid
    @see EjsVoid ejsCreateUndefined
 */

typedef EjsObj EjsVoid;

/** 
    Create the undefined object
    @description There is one undefined object in the system.
    @param ejs Ejs reference returned from #ejsCreateVM
    @return The undefined object
    @ingroup EjsVoid
    @internal
 */
PUBLIC EjsVoid *ejsCreateUndefined(Ejs *ejs);

/******************************************** XML *************************************************/
/*  
    Xml defines
 */
#define E4X_TEXT_PROPERTY       "-txt"
#define E4X_TAG_NAME_PROPERTY   "-tag"
#define E4X_COMMENT_PROPERTY    "-com"
#define E4X_ATTRIBUTES_PROPERTY "-att"
#define E4X_PI_PROPERTY         "-pi"
#define E4X_PARENT_PROPERTY     "-parent"

#define EJS_XML_FLAGS_TEXT      0x1             /* Node is a text node */
#define EJS_XML_FLAGS_PI        0x2             /* Node is a processing instruction */
#define EJS_XML_FLAGS_COMMENT   0x4             /* Node is a comment */
#define EJS_XML_FLAGS_ATTRIBUTE 0x8             /* Node is an attribute */
#define EJS_XML_FLAGS_ELEMENT   0x10            /* Node is an element */

/*  
    XML node kinds
 */
#define EJS_XML_LIST        1                   /**< XML node is a list */
#define EJS_XML_ELEMENT     2                   /**< XML node is an element */
#define EJS_XML_ATTRIBUTE   3                   /**< XML node is an attribute */
#define EJS_XML_TEXT        4                   /**< XML node is text */
#define EJS_XML_COMMENT     5                   /**< XML node is a comment */
#define EJS_XML_PROCESSING  6                   /**< XML node is a processing instruction */

/** 
    Xml tag state
    @ingroup EjsXML
    @stability Internal
 */
typedef struct EjsXmlTagState {
    struct EjsXML   *obj;                       /**< Current object */
    EjsObj          *attributes;                /**< List of attributes */
    EjsObj          *comments;                  /**< List of comments */
} EjsXmlTagState;


/*  
    Xml Parser state
    @ingroup EjsXML
    @stability Internal
 */
typedef struct EjsXmlState {
    //  TODO -- should not be fixed but should be growable
    EjsXmlTagState  nodeStack[BIT_XML_MAX_NODE_DEPTH];      /**< nodeStack */
    Ejs             *ejs;                                   /**< Convenient reference to ejs */
    struct EjsType  *xmlType;                               /**< Xml type reference */
    struct EjsType  *xmlListType;                           /**< Xml list type reference */
    int             topOfStack;                             /**< Pointer to the top of state stack */
    ssize           inputSize;                              /**< Size of input to parse */
    ssize           inputPos;                               /**< Current input position */
    cchar           *inputBuf;                              /**< Input buffer */
    cchar           *filename;                              /**< Input filename */
} EjsXmlState;


/** 
    XML and XMLList class
    @description The XML class and API is based on ECMA-357 -- ECMAScript for XML (E4X). The XML class is a 
    core class in the E4X specification; it provides the ability to load, parse and save XML documents.
    @defgroup EjsXML EjsXML
    @see EjsXML ejsAppendAttributeToXML ejsAppendToXML ejsConfigureXML ejsCreateXML ejsCreateXMLList ejsDeepCopyXML 
        ejsGetXMLDescendants ejsIsXML ejsLoadXMLAsc ejsLoadXMLString ejsSetXML ejsXMLToBuf 
    @stability Internal
 */
typedef struct EjsXML {
    EjsObj          obj;                /**< Base object */
    EjsName         qname;              /**< XML node name (e.g. tagName) */
    int             kind;               /**< Kind of XML node */
    MprList         *elements;          /**< List elements or child nodes */
    MprList         *attributes;        /**< Node attributes */
    MprList         *namespaces;        /**< List of namespaces as Namespace objects */
    struct EjsXML   *parent;            /**< Parent node reference (XML or XMLList) */
    struct EjsXML   *targetObject;      /**< XML/XMLList object modified when items inserted into an empty list */
    EjsName         targetProperty;     /**< XML property modified when items inserted into an empty list */
    EjsString       *value;             /**< Value of text|attribute|comment|pi */
    int             flags;
} EjsXML;

#if DOXYGEN
    /** 
        Determine if a variable is an XML object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return true if the variable is an XML or XMLList object
        @ingroup EjsXML
     */
    extern bool ejsIsXML(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsXML(ejs, obj) (ejsIs(ejs, obj, XML) || ejsIs(ejs, obj, XMLList))
#endif

/** 
    Append an attribute
    @param ejs Ejs reference returned from #ejsCreateVM
    @param parent Xml node to receive the attribute
    @param attribute Attribute node to append.
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC int ejsAppendAttributeToXML(Ejs *ejs, EjsXML *parent, EjsXML *attribute);

/** 
    Append a node
    @param ejs Ejs reference returned from #ejsCreateVM
    @param dest Node to receive the appended node
    @param node Node to append to dest
    @return The dest node
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsAppendToXML(Ejs *ejs, EjsXML *dest, EjsXML *node);

/** 
    Create an XML node object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param kind XML node kind. Set to EJS_XML_LIST, EJS_XML_ELEMENT, EJS_XML_ATTRIBUTE, EJS_XML_TEXT, EJS_XML_COMMENT or
        EJS_XML_PROCESSING.
    @param name Node name. Only the EjsName.name field is used.
    @param parent Parent node
    @param value Node value
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsCreateXML(Ejs *ejs, int kind, EjsName name, EjsXML *parent, EjsString *value);

/** 
    Create an XML list object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param targetObject Object to set as the target object for the list
    @param targetProperty Property to set as the target property for the list
    @return A new XML list object
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsCreateXMLList(Ejs *ejs, EjsXML *targetObject, EjsName targetProperty);

/** 
    Get the descendants of an XML node that match the given name
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml Node to examine
    @param qname Name to search for
    @return An XML node with elements for the descendants.
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsGetXMLDescendants(Ejs *ejs, EjsXML *xml, EjsName qname);

/** 
    Load an XML document
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to hold the parsed XML data.
    @param xmlString String containing XML data to parse
    @ingroup EjsXML
 */
PUBLIC void ejsLoadXMLString(Ejs *ejs, EjsXML *xml, EjsString *xmlString);

/** 
    Load an XML document from an Ascii string
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to hold the parsed XML data.
    @param xmlString String containing XML data to parse
    @return A new XML object
    @ingroup EjsXML
 */
PUBLIC void ejsLoadXMLAsc(Ejs *ejs, EjsXML *xml, cchar *xmlString);

/** 
    Set an indexed element to a value
    @param ejs Ejs reference returned from #ejsCreateVM
    @param xml XML node to receive the appended node
    @param index Element index at which to set the node
    @param node Node to insert
    @return The xml node
    @ingroup EjsXML
 */
PUBLIC EjsXML *ejsSetXMLElement(Ejs *ejs, EjsXML *xml, int index, EjsXML *node);

/** 
    Convert an xml node to string representation in a buffer 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param buf Buffer to hold the output string
    @param xml Node to examine
    @param indentLevel Maximum indent level
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsXML
 */
PUBLIC int ejsXMLToBuf(Ejs *ejs, MprBuf *buf, EjsXML *xml, int indentLevel);

/*  
    Internal
 */
PUBLIC EjsXML *ejsConfigureXML(Ejs *ejs, EjsXML *xml, int kind, EjsString *name, EjsXML *parent, EjsString *value);
PUBLIC void ejsManageXML(EjsXML *xml, int flags);
PUBLIC MprXml *ejsCreateXmlParser(Ejs *ejs, EjsXML *xml, cchar *filename);

/******************************************** Type ************************************************/
/** 
    Allocation and Type Helpers
    @description The type helpers interface defines the set of primitive operations a type must support to
        interact with the virtual machine.
    @ingroup EjsType
    @stability Internal
 */
typedef struct EjsHelpers {
    /* Used by objects and values */
    EjsAny  *(*cast)(struct Ejs *ejs, EjsAny *obj, struct EjsType *type);
    EjsAny  *(*clone)(struct Ejs *ejs, EjsAny *obj, bool deep);
    EjsAny  *(*create)(struct Ejs *ejs, struct EjsType *type, int size);
    int     (*defineProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                int64 attributes, EjsAny *value);
    int     (*deleteProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    int     (*deletePropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    EjsAny  *(*getProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*getPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*getPropertyCount)(struct Ejs *ejs, EjsAny *obj);
    EjsName (*getPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    struct EjsTrait *(*getPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum);
    EjsAny  *(*invokeOperator)(struct Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
    int     (*lookupProperty)(struct Ejs *ejs, EjsAny *obj, EjsName qname);
    int     (*setProperty)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
    int     (*setPropertyByName)(struct Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
    int     (*setPropertyName)(struct Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
    int     (*setPropertyTraits)(struct Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);
} EjsHelpers;

typedef EjsAny  *(*EjsCreateHelper)(Ejs *ejs, struct EjsType *type, int size);
typedef EjsAny  *(*EjsCastHelper)(Ejs *ejs, EjsAny *obj, struct EjsType *type);
typedef EjsAny  *(*EjsCloneHelper)(Ejs *ejs, EjsAny *obj, bool deep);
typedef int     (*EjsDefinePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname, struct EjsType *propType, 
                    int64 attributes, EjsAny *value);
typedef int     (*EjsDeletePropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsDeletePropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef EjsAny  *(*EjsGetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsGetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef struct EjsTrait *(*EjsGetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef int     (*EjsGetPropertyCountHelper)(Ejs *ejs, EjsAny *obj);
typedef EjsName (*EjsGetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum);
typedef EjsAny  *(*EjsInvokeOperatorHelper)(Ejs *ejs, EjsAny *obj, int opCode, EjsAny *rhs);
typedef int     (*EjsLookupPropertyHelper)(Ejs *ejs, EjsAny *obj, EjsName qname);
typedef int     (*EjsSetPropertyByNameHelper)(Ejs *ejs, EjsAny *obj, EjsName qname, EjsAny *value);
typedef int     (*EjsSetPropertyHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsAny *value);
typedef int     (*EjsSetPropertyNameHelper)(Ejs *ejs, EjsAny *obj, int slotNum, EjsName qname);
typedef int     (*EjsSetPropertyTraitsHelper)(Ejs *ejs, EjsAny *obj, int slotNum, struct EjsType *type, int attributes);

/** 
    Type class
    @description Classes in Ejscript are represented by instances of an EjsType. 
        Types are templates for creating instances of the given type, but they are also are runtime accessible objects.
        Types contain the static properties and methods for objects and store these in their object slots array. 
        They store the instance properties in the type->instance object. EjsType inherits from EjsBlock, EjsObj 
        and EjsObj. 
    @defgroup EjsType EjsType
    @see EjsType ejsBindAccess ejsBindConstructor ejsBindMethod ejsCast ejsConfigureType ejsCreateArchetype 
        ejsCreateCoreType ejsCreatePrototype ejsCreateType ejsDefineGlobalFunction ejsDefineInstanceProperty 
        ejsFinalizeCoreType ejsFinalizeScriptType ejsGetPrototype ejsGetType ejsGetTypeByName ejsGetTypeof ejsIs 
        ejsIsA ejsIsDefined ejsIsType ejsIsTypeSubType 
    @stability Internal
 */
typedef struct EjsType {
    EjsFunction     constructor;                    /**< Constructor function and type properties */
    EjsName         qname;                          /**< Qualified name of the type. Type name and namespace */
    EjsPot          *prototype;                     /**< Prototype for instances when using prototype inheritance (only) */
    EjsHelpers      helpers;                        /**< Type helper methods */
    struct EjsType  *baseType;                      /**< Base class */
    MprManager      manager;                        /**< Manager callback */
    MprMutex        *mutex;                         /**< Optional locking for types that require it */
    MprList         *implements;                    /**< List of implemented interfaces */
        
    uint            callsSuper           : 1;       /**< Constructor calls super() */
    uint            configured           : 1;       /**< Type has been configured with native methods */
    uint            dynamicInstances     : 1;       /**< Object instances may add properties */
    uint            final                : 1;       /**< Type is final */
    uint            hasBaseConstructors  : 1;       /**< Base types has constructors */
    uint            hasBaseInitializers  : 1;       /**< Base types have initializers */
    uint            hasConstructor       : 1;       /**< Type has a constructor */
    uint            hasInitializer       : 1;       /**< Type has static level initialization code */
    uint            hasInstanceVars      : 1;       /**< Type has non-function instance vars (state) */
    uint            hasMeta              : 1;       /**< Type has meta methods */
    uint            hasScriptFunctions   : 1;       /**< Block has non-native functions requiring namespaces */
    uint            initialized          : 1;       /**< Static initializer has run */
    uint            isInterface          : 1;       /**< Interface vs class */
    uint            isPot                : 1;       /**< Instances are based on EjsPot */
    uint            mutable              : 1;       /**< Type is mutable (has changable state) */
    uint            mutableInstances     : 1;       /**< Instances are mutable */
    uint            needFixup            : 1;       /**< Slots need fixup */
    uint            numericIndicies      : 1;       /**< Instances support direct numeric indicies */
    uint            virtualSlots         : 1;       /**< Properties are not stored in slots[] */
    
    int             endClass;                       /**< Offset in mod file for end of class */
    ushort          numInherited;                   /**< Number of inherited prototype properties */
    ushort          instanceSize;                   /**< Size of instances in bytes */
    short           sid;                            /**< Slot index into service->immutable[] */
    struct EjsModule *module;                       /**< Module owning the type - stores the constant pool */
    void            *typeData;                      /**< Type specific data */
} EjsType;


#if DOXYGEN
    /** 
        Determine if a variable is an type
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a type
        @ingroup EjsType
     */
    extern bool ejsIsType(Ejs *ejs, EjsAny *obj);

    /** 
        Determine if a variable is a prototype object. Types store the template for instance properties in a prototype object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to test
        @return True if the variable is a prototype object.
        @ingroup EjsType
     */
    extern bool ejsIsPrototype(Ejs *ejs, EjsAny *obj);
#else
    #define ejsIsType(ejs, obj)       (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isType))
    #define ejsIsPrototype(ejs, obj)  (obj && ejsIsPot(ejs, obj) && (((EjsPot*) (obj))->isPrototype))
#endif

/** 
    Bind a native C functions to method accessors
    @description Bind a native C function to an existing javascript accessor function. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method accessor.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param getter Native C getter function to bind. Set to NULL if no getter.
    @param setter Native C setter function to bind. Set to NULL if no setter.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindAccess(Ejs *ejs, EjsAny *obj, int slotNum, void *getter, void *setter);

/** 
    Bind a constructor
    @description Bind a native C function to a type as a constructor function.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type to modify
    @param constructor Native C constructor function to bind.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC void ejsBindConstructor(Ejs *ejs, EjsType *type, void *constructor);

/** 
    Bind a native C function to a method property
    @description Bind a native C function to an existing javascript method. Method functions are typically created
        by compiling a script file of native method definitions into a mod file. When loaded, this mod file will create
        the method properties. This routine will then bind the specified C function to the method property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Type containing the function property to bind.
    @param slotNum Slot number of the method property
    @param fn Native C function to bind
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsBindMethod(Ejs *ejs, EjsAny *obj, int slotNum, void *fn);

/** 
    Configure a type
    @description Called by loader to configure a native type based on the mod file information
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type to configure
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param attributes Attribute mask to modify how the type is initialized.
    @return The configured type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsConfigureType(Ejs *ejs, EjsType *type, struct EjsModule *up, EjsType *baseType, 
    int numTypeProp, int numInstanceProp, int64 attributes);

/** 
    Create an Archetype.
    @description Archetypes are used when functions are used as constructors.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param fun Function object to use as a constructor
    @param prototype Prototype object. If set to null, a new prototype is created.
    @return A new type object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsCreateArchetype(Ejs *ejs, struct EjsFunction *fun, EjsPot *prototype);

/** 
    Create a core type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name to give the type. This name is referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param size Size in bytes to reserve for the type
    @param slotNum Global slot number property index
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param manager MPR manager routine for garbage collection
    @param attributes Attribute mask to modify how the type is initialized.
    @return The created type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsCreateCoreType(Ejs *ejs, EjsName qname, int size, int slotNum, int numTypeProp, void *manager, 
        int64 attributes);

/** 
    Create a type prototype
    @description This creates a prototype object from which instances are crafted.
    @param ejs Ejs reference returned from #ejsCreateVM
    @param type Type object
    @param numProp Number of instance properties in the prototype
    @return The prototype object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsObj *ejsCreatePrototype(Ejs *ejs, EjsType *type, int numProp);

/** 
    Create a new type object
    @description Create a new type object 
    @param ejs Ejs reference returned from #ejsCreateVM
    @param name Qualified name to give the type. This name is referenced by the type and must be persistent.
        This name is not used to define the type as a global property.
    @param up Reference to a module that will own the type. Set to null if not owned by any module.
    @param baseType Base type for this type.
    @param prototype Prototype object instance properties of this type.
    @param size Size of instances. This is the size in bytes of an instance object.
    @param slotNum Unique type ID for core types. For non-core types, set to -1.
    @param numTypeProp Number of type (class) properties for the type. These include static properties and methods.
    @param numInstanceProp Number of instance properties.
    @param manager MPR manager routine for garbage collection
    @param attributes Attribute mask to modify how the type is initialized.
    @return The created type
    @ingroup EjsType EjsType
 */
PUBLIC EjsType *ejsCreateType(Ejs *ejs, EjsName name, struct EjsModule *up, EjsType *baseType, EjsPot *prototype,
    int slotNum, int numTypeProp, int numInstanceProp, int size, void *manager, int64 attributes);

/** 
    Define a global function
    @description Define a global public function and bind it to the C native function. This is a simple one liner
        to define a public global function. The more typical paradigm to define functions is to create a script file
        of native method definitions and and compile it. This results in a mod file that can be loaded which will
        create the function/method definitions. Then use #ejsBindMethod to associate a C function with a property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param name Function name
    @param fn C function that implements the function
    @ingroup EjsType
 */
PUBLIC int ejsDefineGlobalFunction(Ejs *ejs, EjsString *name, EjsProc fn);

/** 
    Define an instance property
    @description Define an instance property on a type. This routine should not normally be called manually. Instance
        properties are best created by creating a script file of native property definitions and then loading the resultant
        mod file.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param type Type in which to create the instance property
    @param slotNum Instance slot number in the type that will hold the property. Set to -1 to allocate the next available
        free slot.
    @param name Qualified name for the property including namespace and name.
    @param propType Type of the instance property.
    @param attributes Integer mask of access attributes.
    @param value Initial value of the instance property.
    @return The slot number used for the property.
    @ingroup EjsType
 */
PUBLIC int ejsDefineInstanceProperty(Ejs *ejs, EjsType *type, int slotNum, EjsName name, EjsType *propType, 
    int attributes, EjsAny *value);

/** 
    Get the prototype object for an object
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to examine
    @return Prototype object for the given object
    @ingroup EjsType
 */
PUBLIC EjsPot *ejsGetPrototype(Ejs *ejs, EjsAny *obj);

/** 
    Get a type
    @description Get the type installed at the given slot number. All core-types are installed a specific global slots.
        When Ejscript is built, these slots are converted into C program defines of the form: ES_TYPE where TYPE is the 
        name of the type concerned. For example, you can get the String type object via:
        @pre
        ejsGetType(ejs, ES_String)
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param slotNum Slot number of the type to retrieve. Use ES_TYPE defines. 
    @return A type object if successful or zero if the type could not be found
    @ingroup EjsType
 */
PUBLIC EjsType  *ejsGetType(Ejs *ejs, int slotNum);

/** 
    Get a type given its name
    @description Types are stored in the global object. This routine looks in the global object for the type property.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param qname Qualified name of the type.
    @return The type object
    @ingroup EjsType
 */
PUBLIC EjsType *ejsGetTypeByName(Ejs *ejs, EjsName qname);

/** 
    Finalize a core type
    @description This sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name of the type
    @return The finalized type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsFinalizeCoreType(Ejs *ejs, EjsName qname);

/** 
    Finalize a script type
    @description This finalizes the type and sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param qname Qualified name of the type
    @param size Instance size of the type
    @param manager Manager function for garbage collection
    @param attributes Type attributes
    @return The configured type
    @ingroup EjsType
    @internal
 */
PUBLIC EjsType *ejsFinalizeScriptType(Ejs *ejs, EjsName qname, int size, void *manager, int64 attributes);

/** 
    Get the type name of an object
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return The string type name of the object
    @ingroup EjsType
    @internal
 */
PUBLIC EjsString *ejsGetTypeName(struct Ejs *ejs, EjsAny *obj);

/** 
    TypeOf operator
    @description This finalizes the type and sets the configured state for the type
    @param ejs Ejs reference returned from #ejsCreateVM
    @param obj Object to examine
    @return String type name for the "typeOf" operator
    @ingroup EjsType
    @internal
 */
PUBLIC EjsString *ejsGetTypeOf(struct Ejs *ejs, EjsAny *obj);

/*
    WARNING: this macros assumes an "ejs" variable in scope. This is done because it is such a pervasive idiom, the
    assumption is worth the benefit.
 */
#if DOXYGEN
    /** 
        Test the type of an object
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @param name Textual name of the type (Not void*). For example: ejsIs(ejs, obj, Number)
        @return True if the object is of the tested type.
        @ingroup EjsType
        @internal
     */
    extern bool ejsIs(Ejs *ejs, EjsAny *obj, void *name);

    /** 
        Test the object is not null and not undefined
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @return True if the object is of a defined type
        @ingroup EjsType
        @internal
     */
    extern bool ejsIsDefined(Ejs *ejs, EjsAny *obj);

    /** 
        Cast the object to the given type name
        @param ejs Ejs reference returned from #ejsCreateVM
        @param obj Object to examine
        @param name Textual name of the type (Not void*). For example: ejsCast(ejs, obj, String)
        @return Casted object.
        @ingroup EjsType
        @internal
     */
    extern EjsAny *ejsCast(Ejs *ejs, EjsAny *obj, void *name);
#else
    #define ejsIs(ejs, obj, name) ejsIsA(ejs, obj, EST(name))
    #define ejsIsDefined(ejs, obj) (obj != 0 && !ejsIs(ejs, obj, Null) && !ejsIs(ejs, obj, Void))
    #define ejsCast(ejs, obj, name) ejsCastType(ejs, obj, ESV(name))
#endif

/** 
    Test if an variable is an instance of a given type
    @description Perform an "is a" test. This tests if a variable is a direct instance or subclass of a given base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param target Target object to test.
    @param type Type to compare with the target
    @return True if target is an instance of "type" or an instance of a subclass of "type".
    @ingroup EjsType
 */
PUBLIC bool ejsIsA(Ejs *ejs, EjsAny *target, EjsType *type);

/** 
    Test if a type is a derived type of a given base type.
    @description Test if a type subclasses a base type.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param target Target type to test.
    @param baseType Base class to see if the target subclasses it.
    @return True if target is a "baseType" or a subclass of "baseType".
    @ingroup EjsType
 */
PUBLIC bool ejsIsTypeSubType(Ejs *ejs, EjsType *target, EjsType *baseType);

/*
    Internal
 */
#define VSPACE(space) space "-" BIT_VNUM
#define ejsGetVType(ejs, space, name) ejsGetTypeByName(ejs, space "-" BIT_VNUM, name)
PUBLIC void     ejsDefineTypeNamespaces(Ejs *ejs, EjsType *type);
PUBLIC int      ejsFixupType(Ejs *ejs, EjsType *type, EjsType *baseType, int makeRoom);
PUBLIC int      ejsGetTypeSize(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitializeBlockHelpers(EjsHelpers *helpers);
PUBLIC int64    ejsSetTypeAttributes(EjsType *type, int size, MprManager manager, int64 attributes);
PUBLIC void     ejsSetTypeHelpers(EjsType *type, int64 attributes);
PUBLIC void     ejsSetTypeName(Ejs *ejs, EjsType *type, EjsName qname);
PUBLIC void     ejsTypeNeedsFixup(Ejs *ejs, EjsType *type);

/******************************** Internal Prototypes *********************************/

PUBLIC int      ejsCreateBootstrapTypes(Ejs *ejs);
PUBLIC void     ejsInitBlockType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitNullType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitStringType(Ejs *ejs, EjsType *type);
PUBLIC void     ejsInitTypeType(Ejs *ejs, EjsType *type);

PUBLIC void     ejsCreateArrayType(Ejs *ejs);
PUBLIC void     ejsCreateBooleanType(Ejs *ejs);
PUBLIC void     ejsCreateConfigType(Ejs *ejs);
PUBLIC void     ejsCreateErrorType(Ejs *ejs);
PUBLIC void     ejsCreateFrameType(Ejs *ejs);
PUBLIC void     ejsCreateFunctionType(Ejs *ejs);
PUBLIC void     ejsCreateIteratorType(Ejs *ejs);
PUBLIC void     ejsCreateNamespaceType(Ejs *ejs);
PUBLIC void     ejsCreateNumberType(Ejs *ejs);
PUBLIC void     ejsCreateObjectType(Ejs *ejs);
PUBLIC void     ejsCreatePathType(Ejs *ejs);
PUBLIC void     ejsCreateRegExpType(Ejs *ejs);
PUBLIC void     ejsCreateVoidType(Ejs *ejs);
PUBLIC void     ejsCreateXMLType(Ejs *ejs);
PUBLIC void     ejsCreateXMLListType(Ejs *ejs);

/*  
    Native type configuration
 */
PUBLIC void     ejsConfigureAppType(Ejs *ejs);
PUBLIC void     ejsConfigureArrayType(Ejs *ejs);
PUBLIC void     ejsConfigureBlockType(Ejs *ejs);
PUBLIC void     ejsConfigureBooleanType(Ejs *ejs);
PUBLIC void     ejsConfigureByteArrayType(Ejs *ejs);
PUBLIC void     ejsConfigureCmdType(Ejs *ejs);
PUBLIC void     ejsConfigureDateType(Ejs *ejs);
PUBLIC void     ejsConfigureSqliteTypes(Ejs *ejs);
PUBLIC void     ejsConfigureDebugType(Ejs *ejs);
PUBLIC void     ejsConfigureErrorType(Ejs *ejs);
PUBLIC void     ejsConfigureEventType(Ejs *ejs);
PUBLIC void     ejsConfigureFrameType(Ejs *ejs);
PUBLIC void     ejsConfigureGCType(Ejs *ejs);
PUBLIC void     ejsConfigureGlobalBlock(Ejs *ejs);
PUBLIC void     ejsConfigureFileType(Ejs *ejs);
PUBLIC void     ejsConfigureFileSystemType(Ejs *ejs);
PUBLIC void     ejsConfigureFunctionType(Ejs *ejs);
PUBLIC void     ejsConfigureHttpType(Ejs *ejs);
PUBLIC void     ejsConfigureIteratorType(Ejs *ejs);
PUBLIC void     ejsConfigureJSONType(Ejs *ejs);
PUBLIC void     ejsConfigureLocalCacheType(Ejs *ejs);
PUBLIC void     ejsConfigureMprLogType(Ejs *ejs);
PUBLIC void     ejsConfigureNamespaceType(Ejs *ejs);
PUBLIC void     ejsConfigureMemoryType(Ejs *ejs);
PUBLIC void     ejsConfigureMathType(Ejs *ejs);
PUBLIC void     ejsConfigureNumberType(Ejs *ejs);
PUBLIC void     ejsConfigureNullType(Ejs *ejs);
PUBLIC void     ejsConfigureObjectType(Ejs *ejs);
PUBLIC void     ejsConfigurePathType(Ejs *ejs);
PUBLIC void     ejsConfigureReflectType(Ejs *ejs);
PUBLIC void     ejsConfigureRegExpType(Ejs *ejs);
PUBLIC void     ejsConfigureStringType(Ejs *ejs);
PUBLIC void     ejsConfigureSocketType(Ejs *ejs);
PUBLIC void     ejsConfigureSystemType(Ejs *ejs);
PUBLIC void     ejsConfigureTimerType(Ejs *ejs);
PUBLIC void     ejsConfigureTypes(Ejs *ejs);
PUBLIC void     ejsConfigureUriType(Ejs *ejs);
PUBLIC void     ejsConfigureVoidType(Ejs *ejs);
PUBLIC void     ejsConfigureWorkerType(Ejs *ejs);
PUBLIC void     ejsConfigureXMLType(Ejs *ejs);
PUBLIC void     ejsConfigureXMLListType(Ejs *ejs);
PUBLIC void     ejsConfigureWebSocketType(Ejs *ejs);

PUBLIC void     ejsCreateCoreNamespaces(Ejs *ejs);
PUBLIC int      ejsCopyCoreTypes(Ejs *ejs);
PUBLIC int      ejsDefineCoreTypes(Ejs *ejs);
PUBLIC int      ejsDefineErrorTypes(Ejs *ejs);
PUBLIC void     ejsInheritBaseClassNamespaces(Ejs *ejs, EjsType *type, EjsType *baseType);
PUBLIC void     ejsSetSqliteMemCtx(MprThreadLocal *tls);
PUBLIC void     ejsSetSqliteTls(MprThreadLocal *tls);
PUBLIC void     ejsDefineConfigProperties(Ejs *ejs);

#if BIT_PACK_SQLITE
    PUBLIC int   ejs_db_sqlite_Init(Ejs *ejs, MprModule *mp);
#endif
#if BIT_PACK_ZLIB
    PUBLIC int   ejs_zlib_Init(Ejs *ejs, MprModule *mp);
#endif
PUBLIC int       ejs_web_Init(Ejs *ejs, MprModule *mp);

/* 
    Move some ejsWeb.h declarations here so handlers can just include ejs.h whether they are using the
    all-in-one ejs.h or the pure ejs.h
 */
PUBLIC HttpStage *ejsAddWebHandler(Http *http, MprModule *module);
PUBLIC int ejsHostHttpServer(HttpConn *conn);

/******************************************** VM **************************************************/
/**
    VM Evaluation state. 
    The VM Stacks grow forward in memory. A push is done by incrementing first, then storing. ie. *++top = value
    A pop is done by extraction then decrement. ie. value = *top--
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsState {
    struct EjsFrame     *fp;                /**< Current Frame function pointer */
    struct EjsBlock     *bp;                /**< Current block pointer */
    EjsObj              **stack;            /**< Top of stack (points to the last element pushed) */
    EjsObj              **stackBase;        /**< Pointer to start of stack mem */
    struct EjsState     *prev;              /**< Previous state */
    struct EjsNamespace *internal;          /**< Current internal namespace */
    ssize               stackSize;          /**< Stack size */
    uint                paused: 1;          /**< Garbage collection paused */
    EjsObj              *t1;                /**< Temp one for GC */
} EjsState;


/**
    Lookup State.
    @description Location information returned when looking up properties.
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsLookup {
    int             slotNum;                /**< Final slot in obj containing the variable reference */
    uint            nthBase;                /**< Property on Nth super type -- count from the object */
    uint            nthBlock;               /**< Property on Nth block in the scope chain -- count from the end */
    EjsType         *type;                  /**< Type containing property (if on a prototype obj) */
    uint            instanceProperty;       /**< Property is an instance property */
    uint            ownerIsType;            /**< Original object owning the property is a type */
    uint            useThis;                /**< Property accessible via "this." */
    EjsAny          *obj;                   /**< Final object / Type containing the variable */
    EjsAny          *originalObj;           /**< Original object used for the search */
    EjsAny          *ref;                   /**< Actual property reference */
    struct EjsTrait *trait;                 /**< Property trait describing the property */
    struct EjsName  name;                   /**< Name and namespace used to find the property */
    int             bind;                   /**< Whether to bind to this lookup */
} EjsLookup;


/**
    Interned string hash shared over all interpreters
    @ingroup Ejs
    @stability Internal
 */
typedef struct EjsIntern {
    struct EjsString    *buckets;               /**< Hash buckets and references to link chains of strings (unicode) */
    int                 size;                   /**< Size of hash */
    int                 count;                  /**< Count of entries */
    uint64              reuse;                  /**< Reuse counter */
    uint64              accesses;               /**< NUmber of accesses to string */
    MprMutex            *mutex;
} EjsIntern;

/**
    Ejscript Service structure
    @description The Ejscript service manages the overall language runtime. It 
        is the factory that creates interpreter instances via #ejsCreateVM.
    @ingroup EjsService
    @stability Internal
 */
typedef struct EjsService {
    EjsObj          *(*loadScriptLiteral)(Ejs *ejs, EjsString *script, cchar *cache);
    EjsObj          *(*loadScriptFile)(Ejs *ejs, cchar *path, cchar *cache);
    MprList         *vmlist;                /**< List of all VM interpreters */
    MprHash         *nativeModules;         /**< Set of loaded native modules */
    Http            *http;                  /**< Http service */
    uint            dontExit: 1;            /**< Prevent App.exit() from exiting */
    uint            logging: 1;             /**< Using --log */
    uint            immutableInitialized: 1;/**< Immutable types are initialized */
    uint            seqno;                  /**< Interp sequence numbers */
    EjsIntern       *intern;                /**< Interned Unicode string hash - shared over all interps */
    EjsPot          *immutable;             /**< Immutable types and special values*/
    EjsHelpers      objHelpers;             /**< Default EjsObj helpers */
    EjsHelpers      potHelpers;             /**< Default EjsPot helpers */
    EjsHelpers      blockHelpers;           /**< Default EjsBlock helpers */
    MprMutex        *mutex;                 /**< Multithread locking */
    MprSpin         *dtoaSpin[2];           /**< Dtoa thread synchronization */
} EjsService;

/*
   Internal
 */
PUBLIC EjsIntern *ejsCreateIntern(EjsService *sp);
PUBLIC int ejsInitCompiler(EjsService *sp);
PUBLIC void ejsAttention(Ejs *ejs);
PUBLIC void ejsClearAttention(Ejs *ejs);

/*********************************** Prototypes *******************************/
/**
    Create an ejs virtual machine 
    @description Create a virtual machine interpreter object to evalute Ejscript programs. Ejscript supports multiple 
        interpreters. 
    @param argc Count of command line argumements in argv
    @param argv Command line arguments
    @param flags Optional flags to modify the interpreter behavior. Valid flags are:
        @li    EJS_FLAG_COMPILER       - Interpreter will compile code from source
        @li    EJS_FLAG_NO_EXE         - Don't execute any code. Just compile.
        @li    EJS_FLAG_DOC            - Load documentation from modules
        @li    EJS_FLAG_NOEXIT         - App should service events and not exit unless explicitly instructed
    @return A new interpreter
    @ingroup Ejs
 */
PUBLIC Ejs *ejsCreateVM(int argc, cchar **argv, int flags);

/**
    Clone an ejs virtual machine 
    @description Create a virtual machine interpreter boy cloning an existing interpreter. Cloning is a fast way
        to create a new interpreter. This saves memory and speeds initialization.
    @param ejs Base VM upon which to base the new VM.
    @return A new interpreter
    @ingroup Ejs
 */
PUBLIC Ejs *ejsCloneVM(Ejs *ejs);

/**
    Set the MPR dispatcher to use for an interpreter.
    @description Interpreters serialize event activity within a dispatcher.
    @ingroup Ejs
 */
PUBLIC void ejsSetDispatcher(Ejs *ejs, MprDispatcher *dispatcher);

/**
    Destroy an interpreter
    @param ejs Interpreter to destroy
 */
PUBLIC void ejsDestroyVM(Ejs *ejs);

/**
    Evaluate a file
    @description Evaluate a file containing an Ejscript. This requires linking with the Ejscript compiler library (libec). 
    @param path Filename of the script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalFile(cchar *path);

/*
    Flags for LoadScript and compiling
 */
#define EC_FLAGS_BIND            0x1        /**< ejsLoad flags to bind global references and type/object properties */
#define EC_FLAGS_DEBUG           0x2        /**< ejsLoad flags to generate symbolic debugging information */
#define EC_FLAGS_MERGE           0x8        /**< ejsLoad flags to merge all output onto one output file */
#define EC_FLAGS_NO_OUT          0x10       /**< ejsLoad flags discard all output */
#define EC_FLAGS_PARSE_ONLY      0x20       /**< ejsLoad flags to only parse source. Don't generate code */
#define EC_FLAGS_THROW           0x40       /**< ejsLoad flags to throw errors when compiling. Used for eval() */
#define EC_FLAGS_VISIBLE         0x80       /**< ejsLoad flags to make global vars visible to all */
#define EC_FLAGS_DOC             0x100      /**< ejsLoad flags to parse inline doc */

/** 
    Load a script from a file
    @description This will read a script from a file, compile it and run. If the cache path argument is 
        provided, the compiled module will be saved to this path.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param script Script pathname.
    @param cache Cache pathname to save compiled module.
    @param flags Compilation control flags. Select from:
    <ul>
        <li> EC_FLAGS_BIND - Bind global references and type/object properties</li>
        <li> EC_FLAGS_DEBUG - Generate symbolic debugging information</li>
        <li> EC_FLAGS_MERGE - Merge all output onto one output file</li>
        <li> EC_FLAGS_NO_OUT - Discard all output</li>
        <li> EC_FLAGS_PARSE_ONLY - Only parse source. Don't generate code</li>
        <li> EC_FLAGS_THROW - Throw errors when compiling. Used for eval()</li>
        <li> EC_FLAGS_VISIBLE - Make global vars visible to all</li>
    </ul>
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLoadScriptFile(Ejs *ejs, cchar *script, cchar *cache, int flags);

//  TODO - rename ejsLoadScriptString
/** 
    Load a script from a string
    @description This will compile the script string and then run it. If the cache path argument is provided, 
    the compiled module will be saved to this path.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param script Script string.
    @param cache Cache pathname to save compiled module.
    @param flags Compilation control flags. Select from:
    <ul>
        <li> EC_FLAGS_BIND - Bind global references and type/object properties</li>
        <li> EC_FLAGS_DEBUG - Generate symbolic debugging information</li>
        <li> EC_FLAGS_MERGE - Merge all output onto one output file</li>
        <li> EC_FLAGS_NO_OUT - Discard all output</li>
        <li> EC_FLAGS_PARSE_ONLY - Only parse source. Don't generate code</li>
        <li> EC_FLAGS_THROW - Throw errors when compiling. Used for eval()</li>
        <li> EC_FLAGS_VISIBLE - Make global vars visible to all</li>
    </ul>
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLoadScriptLiteral(Ejs *ejs, EjsString *script, cchar *cache, int flags);

/**
    Evaluate a module
    @description Evaluate a module containing compiled Ejscript.
    @param path Filename of the module to evaluate.
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalModule(cchar *path);

/**
    Evaluate a script
    @description Evaluate a script. This requires linking with the Ejscript compiler library (libec). 
    @param script Script to evaluate
    @return Return zero on success. Otherwise return a negative Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsEvalScript(cchar *script);

/**
    Instruct the interpreter to exit.
    @description This will instruct the interpreter to cease interpreting any further script code.
    @param ejs Interpeter object returned from #ejsCreateVM
    @param status Reserved and ignored
    @ingroup Ejs
 */
PUBLIC void ejsExit(Ejs *ejs, int status);

/**
    Get the hosting handle
    @description The interpreter can store a hosting handle. This is typically a web server object if hosted inside
        a web server
    @param ejs Interpeter object returned from #ejsCreateVM
    @return Hosting handle
    @ingroup Ejs
 */
PUBLIC void *ejsGetHandle(Ejs *ejs);

//  TODO - variable is the wrong name. ejsGetPropByName?
/** 
    Get a variable by name
    @description This looks for a property name in an object, its prototype or base classes.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return The variable 
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsGetVarByName(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable using the current scope
    @description This looks for a property name in the current lexical scope.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupScope(Ejs *ejs, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable
    @description This looks for a property name in an object, its prototype or base classes. If name.space is null, 
        the variable is searched using the set of currently open namespaces. 
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupVar(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/** 
    Lookup a variable in an object (only)
    @description This looks for a property name in an object, its prototype or base classes. If name.space is null,
    the variable is searched using the set of currently open namespaces.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param obj Object to search
    @param name Property name to search for
    @param lookup Lookup residuals.
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup Ejs
 */
PUBLIC int ejsLookupVarWithNamespaces(Ejs *ejs, EjsAny *obj, EjsName name, EjsLookup *lookup);

/**
    Run a script
    @description Run a script that has previously ben compiled by ecCompile
    @param ejs Interpeter object returned from #ejsCreateVM
    @return Zero if successful, otherwise a non-zero Mpr error code.
    @ingroup Ejs
 */
PUBLIC int ejsRun(Ejs *ejs);

/** 
    Run a program.
    @description Lookup the className and run the designated method. If methodName is null, then "main" is run.
        The method should be a static method of the class.
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param className Class name to search for methodName.
    @param methodName Method to run. If set to NULL, then search for "main".
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsType
 */
PUBLIC int ejsRunProgram(Ejs *ejs, cchar *className, cchar *methodName);

/**
    Throw an exception
    @description Throw an exception object 
    @param ejs Interpeter object returned from #ejsCreateVM
    @param error Exception argument object.
    @return The exception argument for chaining.
    @ingroup Ejs
 */
PUBLIC EjsAny *ejsThrowException(Ejs *ejs, EjsAny *error);

/** 
    Clear an exception
    @param ejs Interpreter instance returned from #ejsCreateVM
    @ingroup Ejs
    @internal
 */
PUBLIC void ejsClearException(Ejs *ejs);

/**
    Report an error message using the MprLog error channel
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreateVM
    @param fmt Is an alternate printf style format to emit if the interpreter has no valid error message.
    @param ... Arguments for fmt
    @ingroup Ejs
 */
PUBLIC void ejsReportError(Ejs *ejs, char *fmt, ...);

/** 
    Enter a message into the log file
    @param ejs Interpreter instance returned from #ejsCreateVM
    @param fmt Message format string
    @ingroup Ejs
 */
PUBLIC void ejsLog(Ejs *ejs, cchar *fmt, ...);

/*
    Internal
 */
PUBLIC void ejsApplyObjHelpers(EjsService *sp, EjsType *type);
PUBLIC void ejsApplyPotHelpers(EjsService *sp, EjsType *type);
PUBLIC void ejsApplyBlockHelpers(EjsService *sp, EjsType *type);
PUBLIC EjsAny *ejsCastOperands(Ejs *ejs, EjsAny *lhs, int opcode, EjsAny *rhs);
PUBLIC int ejsCheckModuleLoaded(Ejs *ejs, cchar *name);
PUBLIC EjsAny *ejsCreateException(Ejs *ejs, int slot, cchar *fmt, va_list fmtArgs);
PUBLIC void ejsClearExiting(Ejs *ejs);
PUBLIC void ejsCreateObjHelpers(Ejs *ejs);
PUBLIC void ejsLockService();
PUBLIC void ejsLockVm(Ejs *ejs);
PUBLIC int  ejsParseModuleVersion(cchar *name);
PUBLIC int  ejsRedirectLogging(cchar *logSpec);
PUBLIC void ejsRedirectLoggingToFile(MprFile *file, int level);
PUBLIC void ejsSetHandle(Ejs *ejs, void *handle);
PUBLIC void ejsShowCurrentScope(Ejs *ejs);
PUBLIC void ejsShowStack(Ejs *ejs, EjsFunction *fp);
PUBLIC void ejsShowBlockScope(Ejs *ejs, EjsBlock *block);
PUBLIC void ejsUnlockService();
PUBLIC void ejsUnlockVm(Ejs *ejs);

/******************************************* Module **************************************************/
/*
    A module file may contain multiple logical modules.

    Module File Format and Layout:

    (N) Numbers are 1-3 little-endian encoded bytes using the 0x80 as the continuation character
    (S) Strings are pointers into the constant pool encoded as number offsets. Strings are UTF-8.
    (T) Types are encoded and ored with the type encoding masks below. Types are either: untyped, 
        unresolved or primitive (builtin). The relevant mask is ored into the lower 2 bits. Slot numbers and
        name tokens are shifted up 2 bits. Zero means untyped.

    ModuleHeader
        short       magic
        int         fileVersion
        int         version
        int         flags

    Module
        byte        section
        string      name
        number      version
        number      checksum
        number      constantPoolLength
        block       constantPool

    Dependencies
        byte        section
        string      moduleName
        number      minVersion
        number      maxVersion
        number      checksum
        byte        flags

    Type
        byte        section
        string      typeName
        string      namespace
        number      attributes
        number      slot
        type        baseType
        number      numStaticProperties
        number      numInstanceProperties
        number      numInterfaces
        type        Interfaces ...
        ...

    Property
        byte        section
        string      name
        string      namespace
        number      attributes
        number      slot
        type        property type

    Function
        byte        section
        string      name
        string      namespace
        number      nextSlotForSetter
        number      attributes
        byte        languageMode
        type        returnType
        number      slot
        number      argCount
        number      defaultArgCount
        number      localCount
        number      exceptionCount
        number      codeLength
        block       code        

    Exception
        byte        section
        byte        flags
        number      tryStartOffset
        number      tryEndOffset
        number      handlerStartOffset
        number      handlerEndOffset
        number      numOpenBlocks
        type        catchType

    Debug
        byte        section
        number      countOfLines
        string      fileName
        number      startLine
        number      addressOffset      
        ...

    Block
        byte        section
        string      name
        number      slot
        number      propCount

    Documentation
        byte        section
        string      text
 */

/*
    Type encoding masks
 */
#define EJS_ENCODE_GLOBAL_NOREF         0x0
#define EJS_ENCODE_GLOBAL_NAME          0x1
#define EJS_ENCODE_GLOBAL_SLOT          0x2
#define EJS_ENCODE_GLOBAL_MASK          0x3

/*
    Fixup kinds
 */
#define EJS_FIXUP_BASE_TYPE             1
#define EJS_FIXUP_INTERFACE_TYPE        2
#define EJS_FIXUP_RETURN_TYPE           3
#define EJS_FIXUP_TYPE_PROPERTY         4
#define EJS_FIXUP_INSTANCE_PROPERTY     5
#define EJS_FIXUP_LOCAL                 6
#define EJS_FIXUP_EXCEPTION             7

/*
    Number encoding uses one bit per byte plus a sign bit in the first byte
 */ 
#define EJS_ENCODE_MAX_WORD             0x07FFFFFF

/*
    Type fixup when loading a type
    @stability Internal
 */
typedef struct EjsTypeFixup
{
    int              kind;                  /* Kind of fixup */
    int              slotNum;               /* Slot of target */
    EjsObj           *target;               /* Target to fixup */
    EjsName          typeName;              /* Type name */
    int              typeSlotNum;           /* Type slot number */
} EjsTypeFixup;


/*
    State while loading modules
    @stability Internal
 */
typedef struct EjsLoadState {
    MprList         *typeFixups;            /**< Loaded types to fixup */
    int             firstModule;            /**< First module in ejs->modules for this load */
    int             flags;                  /**< Module load flags */
} EjsLoadState;

/*
    Loader callback
    @param ejs Ejs reference returned from #ejsCreateVM
    @param kind Kind of load record
 */
typedef void (*EjsLoaderCallback)(struct Ejs *ejs, int kind, ...);

/*
    Module file format version
 */
#define EJS_MODULE_VERSION      3
#define EJS_VERSION_FACTOR      1000
#define EJS_MODULE_MAGIC        0xC7DA

#if DOXYGEN
    /**
        Make an integer  version number
        @param maj Major version number
        @param min Minor version number
        @param patch Patch version number
        @return An integer version number combining major, minor and patch version numbers.
     */
    extern int EJS_MAKE_VERSION(int maj, int min, int patch);

#else
    #define EJS_MAKE_VERSION(maj, min, patch) (((((maj) * EJS_VERSION_FACTOR) + (min)) * EJS_VERSION_FACTOR) + (patch))
    #define EJS_COMPAT_VERSION(v1, v2) ((v1 / EJS_VERSION_FACTOR) == (v2 / EJS_VERSION_FACTOR))
    #define EJS_MAKE_COMPAT_VERSION(version) (version / EJS_VERSION_FACTOR * EJS_VERSION_FACTOR)
    #define EJS_MAJOR(version)      (((version / EJS_VERSION_FACTOR) / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
    #define EJS_MINOR(version)      ((version / EJS_VERSION_FACTOR) % EJS_VERSION_FACTOR)
    #define EJS_PATCH(version)      (version % EJS_VERSION_FACTOR)
    #define EJS_MAX_VERSION         EJS_MAKE_VERSION(EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1, EJS_VERSION_FACTOR-1)
#if UNUSED
    #define EJS_VERSION             EJS_MAKE_VERSION(BIT_MAJOR_VERSION, BIT_MINOR_VERSION, BIT_PATCH_VERSION)
#endif
#endif

#ifndef EJS_VERSION
    #define EJS_VERSION BIT_VERSION
#endif

/*
    Section types
 */
#define EJS_SECT_MODULE         1           /* Module section */
#define EJS_SECT_MODULE_END     2           /* End of a module */
#define EJS_SECT_DEBUG          3           /* Module dependency */
#define EJS_SECT_DEPENDENCY     4           /* Module dependency */
#define EJS_SECT_CLASS          5           /* Class definition */
#define EJS_SECT_CLASS_END      6           /* End of class definition */
#define EJS_SECT_FUNCTION       7           /* Function */
#define EJS_SECT_FUNCTION_END   8           /* End of function definition */
#define EJS_SECT_BLOCK          9           /* Nested block */
#define EJS_SECT_BLOCK_END      10          /* End of Nested block */
#define EJS_SECT_PROPERTY       11          /* Property (variable) definition */
#define EJS_SECT_EXCEPTION      12          /* Exception definition */
#define EJS_SECT_DOC            13          /* Documentation for an element */
#define EJS_SECT_MAX            14

/*
    Psudo section types for loader callback
 */
#define EJS_SECT_START          (EJS_SECT_MAX + 1)
#define EJS_SECT_END            (EJS_SECT_MAX + 2)

/*
    Align headers on a 4 byte boundary
 */
#define EJS_HDR_ALIGN           4

/*
    File format is little-endian. All headers are aligned on word boundaries.
    @stability Internal
 */
typedef struct EjsModuleHdr {
    int32       magic;                      /* Magic number for Ejscript modules */
    int32       fileVersion;                /* Module file format version */
    int32       flags;                      /* Module flags */
} EjsModuleHdr;


/**
    Module control structure
    @defgroup EjsModule EjsModule
    @see ejsLoadModule ejsLoadModules ejsSearchForModule ejsCreateSearchPath ejsSetSearchPath
    @stability Internal
 */
typedef struct EjsModule {
    EjsString       *name;                  /**< Name of this module - basename of the filename without .mod extension */
    //  TODO - document the version format
    EjsString       *vname;                 /**< Versioned name - name with optional version suffix */
    MprMutex        *mutex;                 /**< Multithread locking */
    int             version;                /**< Made with EJS_MAKE_VERSION */
    int             minVersion;             /**< Minimum version when used as a dependency */
    int             maxVersion;             /**< Maximum version when used as a dependency */
    int             checksum;               /**< Checksum of slots and names */

    EjsConstants    *constants;             /**< Constant pool */
    EjsFunction     *initializer;           /**< Initializer method */

    //  TODO - should have isDefault bit
    uint            compiling       : 1;    /**< Module currently being compiled from source */
    uint            configured      : 1;    /**< Module types have been configured with native code */
    uint            loaded          : 1;    /**< Module has been loaded from an external file */
    uint            nativeLoaded    : 1;    /**< Backing shared library loaded */
    uint            hasError        : 1;    /**< Module has a loader error */
    uint            hasInitializer  : 1;    /**< Has initializer function */
    uint            hasNative       : 1;    /**< Has native property definitions */
    uint            initialized     : 1;    /**< Initializer has run */
    uint            visited         : 1;    /**< Module has been traversed */
    int             flags;                  /**< Loading flags */

    /*
        Module loading and residuals 
     */
    EjsLoadState    *loadState;             /**< State while loading */
    MprList         *dependencies;          /**< Module file dependencies. List of EjsModules */
    MprFile         *file;                  /**< File handle for loading and code generation */
    MprList         *current;               /**< Current stack of open objects */
    EjsFunction     *currentMethod;         /**< Current method being loaded */
    EjsBlock        *scope;                 /**< Lexical scope chain */
    EjsString       *doc;                   /**< Current doc string */
    char            *path;                  /**< Module file path name */
    int             firstGlobal;            /**< First global property */
    int             lastGlobal;             /**< Last global property + 1*/

    /*
        Used during code generation
     */
    struct EcCodeGen *code;                 /**< Code generation buffer */
    MprList         *globalProperties;      /**< List of global properties */

} EjsModule;


/*
    Internal
 */
PUBLIC int ejsCreateConstants(Ejs *ejs, EjsModule *mp, int count, ssize size, char *pool);
PUBLIC int ejsGrowConstants(Ejs *ejs, EjsModule *mp, ssize size);
PUBLIC int ejsAddConstant(Ejs *ejs, EjsModule *mp, cchar *str);

/**
    Native module initialization callback
    @param ejs Ejs reference returned from #ejsCreateVM
    @return Zero if successful, otherwise a negative MPR error code.
    @ingroup EjsModule
 */
typedef int (*EjsNativeCallback)(Ejs *ejs);

typedef struct EjsNativeModule {
    EjsNativeCallback callback;             /* Callback to configure module native types and properties */
    char            *name;                  /* Module name */
    int             checksum;               /* Checksum expected by native code */
    int             flags;                  /* Configuration flags */
} EjsNativeModule;

/*
    Documentation string information
    Element documentation string. The loader will create if required.
    @ingroup EjsModule
    @stability Internal
 */
typedef struct EjsDoc {
    EjsString   *docString;                         /* Original doc string */
    wchar       *brief;                             /* Element brief */
    wchar       *description;                       /* Element description */
    wchar       *example;                           /* Element example */
    wchar       *requires;                          /* Element requires */
    wchar       *returns;                           /* Element returns */
    wchar       *stability;                         /* prototype, evolving, stable, mature, deprecated */
    wchar       *spec;                              /* Where specified */
    struct EjsDoc *duplicate;                       /* From @duplicate directive */
    MprList     *defaults;                          /* Parameter default values */
    MprList     *params;                            /* Function parameters */
    MprList     *options;                           /* Option parameter values */
    MprList     *events;                            /* Option parameter values */
    MprList     *see;                               /* Element see also */
    MprList     *throws;                            /* Element throws */
    EjsTrait    *trait;                             /* Back pointer to trait */
    int         deprecated;                         /* Hide doc if true */
    int         hide;                               /* Hide doc if true */
    int         cracked;                            /* Doc has been cracked and tokenized */
} EjsDoc;

/*
    Loader flags
 */
#define EJS_LOADER_STRICT     0x1                   /**< Load module code in strict mode */
#define EJS_LOADER_NO_INIT    0x2                   /**< Load module code in strict mode */
#define EJS_LOADER_ETERNAL    0x4                   /**< Make all loaded types eternal */
#define EJS_LOADER_BUILTIN    0x8                   /**< Loading builtins */
#define EJS_LOADER_DEP        0x10                  /**< Loading a dependency */
#define EJS_LOADER_RELOAD     0x20                  /**< Force a reload if already loaded */

/**
    Create a search path array. This can be used in ejsCreate.
    @description Create and array of search paths.
    @param ejs Ejs interpreter
    @param searchPath Search path string. This is a colon (or semicolon on Windows) separated string of directories.
    @return An array of search paths
    @ingroup EjsModule
 */
struct EjsArray *ejsCreateSearchPath(Ejs *ejs, cchar *searchPath);

/**
    Load a module
    @description This will emit an error message of the format:
        @li program:line:errorCode:SEVERITY: message
    @param ejs Interpeter object returned from #ejsCreateVM
    @param name Module path name
    @param maxVer Maximum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param minVer Minimum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param flags Module loading flags. Select from: EJS_LOADER_STRICT, EJS_LOADER_NO_INIT, EJS_LOADER_ETERNAL,
        EJS_LOADER_BUILTIN, EJS_LOADER_DEP, EJS_LOADER_RELOAD
    @return A postitive slot number or a negative MPR error code.
    @ingroup EjsModule
 */
PUBLIC int ejsLoadModule(Ejs *ejs, EjsString *name, int minVer, int maxVer, int flags);

/**
    Load modules into an interpreter
    @description Initialize an interpreter by loading modules. A list of modules to load can be provided via the "require"
        argument. If the "require" argument is set to null, then the default modules will be loaded. If "require" is 
        set to a list of module names, these will be loaded. If set to an empty list, then no modules will be loaded and
        the interpreter will be marked as an "empty" interpreter.
    @param ejs Interpreter to modify
    @param search Module search path to use. Set to NULL for the default search path.
    @param require Optional list of required modules to load. If NULL, the following modules will be loaded:
        ejs, ejs.io, ejs.events, ejs.xml, ejs.sys and ejs.unix.
    @return Zero if successful, otherwise return a negative MPR error code.
    @ingroup EjsModule
 */
PUBLIC int ejsLoadModules(Ejs *ejs, cchar *search, MprList *require);

/**
    Search for a module in the module search path.
    @param ejs Interpeter object returned from #ejsCreateVM
    @param name Module name
    @param minVer Minimum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @param maxVer Maximum acceptable version to load. Use EJS_MAKE_VERSION to create a version number or set to -1 if 
        any version is acceptable
    @return Path name to the module
    @ingroup EjsModule
 */
PUBLIC char *ejsSearchForModule(Ejs *ejs, cchar *name, int minVer, int maxVer);

/**
    Set the module search path
    @description Set the ejs module search path. The search path is by default set to the value of the EJSPATH
        environment directory. Ejsript will search for modules by name. The search strategy is:
        Given a name "a.b.c", scan for:
        @li File named a.b.c
        @li File named a/b/c
        @li File named a.b.c in EJSPATH
        @li File named a/b/c in EJSPATH
        @li File named c in EJSPATH

    Ejs will search for files with no extension and also search for modules with a ".mod" extension. If there is
    a shared library of the same name with a shared library extension (.so, .dll, .dylib) and the module requires 
    native code, then the shared library will also be loaded.
    @param ejs Ejs interpreter
    @param search Array of search paths
    @ingroup EjsModule
 */
PUBLIC void ejsSetSearchPath(Ejs *ejs, struct EjsArray *search);

/*
    Internal
 */
PUBLIC int ejsAddModule(Ejs *ejs, EjsModule *up);
PUBLIC int ejsAddNativeModule(Ejs *ejs, cchar *name, EjsNativeCallback callback, int checksum, int flags);
PUBLIC EjsModule *ejsCloneModule(Ejs *ejs, EjsModule *mp);
PUBLIC EjsDoc *ejsCreateDoc(Ejs *ejs, cchar *tag, void *vp, int slotNum, EjsString *docString);
PUBLIC EjsModule *ejsCreateModule(Ejs *ejs, EjsString *name, int version, EjsConstants *constants);
PUBLIC double ejsDecodeDouble(Ejs *ejs, uchar **pp);
PUBLIC int ejsDecodeInt32(Ejs *ejs, uchar **pp);
PUBLIC int64 ejsDecodeNum(Ejs *ejs, uchar **pp);
PUBLIC int ejsEncodeByteAtPos(Ejs *ejs, uchar *pos, int value);
PUBLIC int ejsEncodeDouble(Ejs *ejs, uchar *pos, double number);
PUBLIC int ejsEncodeInt32(Ejs *ejs, uchar *pos, int number);
PUBLIC int ejsEncodeNum(Ejs *ejs, uchar *pos, int64 number);
PUBLIC int ejsEncodeInt32AtPos(Ejs *ejs, uchar *pos, int value);
PUBLIC char *ejsGetDocKey(Ejs *ejs, EjsBlock *block, int slotNum, char *buf, int bufsize);
PUBLIC EjsModule *ejsLookupModule(Ejs *ejs, EjsString *name, int minVersion, int maxVersion);
PUBLIC EjsNativeModule *ejsLookupNativeModule(Ejs *ejs, cchar *name);
PUBLIC void ejsModuleReadBlock(Ejs *ejs, EjsModule *module, char *buf, int len);
PUBLIC int ejsModuleReadByte(Ejs *ejs, EjsModule *module);
PUBLIC EjsString *ejsModuleReadConst(Ejs *ejs, EjsModule *module);
PUBLIC int ejsModuleReadInt(Ejs *ejs, EjsModule *module);
PUBLIC int ejsModuleReadInt32(Ejs *ejs, EjsModule *module);
PUBLIC EjsName ejsModuleReadName(Ejs *ejs, EjsModule *module);
PUBLIC int64 ejsModuleReadNum(Ejs *ejs, EjsModule *module);
PUBLIC char *ejsModuleReadMulti(Ejs *ejs, EjsModule *mp);
PUBLIC wchar *ejsModuleReadMultiAsWide(Ejs *ejs, EjsModule *mp);
PUBLIC int ejsModuleReadType(Ejs *ejs, EjsModule *module, EjsType **typeRef, EjsTypeFixup **fixup, EjsName *typeName, 
        int *slotNum);
PUBLIC void ejsRemoveModule(Ejs *ejs, EjsModule *up);
PUBLIC void ejsRemoveModuleFromAll(EjsModule *up);
PUBLIC double ejsSwapDouble(Ejs *ejs, double a);
PUBLIC int ejsSwapInt32(Ejs *ejs, int word);
PUBLIC int64 ejsSwapInt64(Ejs *ejs, int64 word);

#ifdef __cplusplus
}
#endif

/*
    Allow user overrides
 */


#endif /* _h_EJS_CORE */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
    Start of file "src/ejs.web/ejsWeb.h"
 */
/************************************************************************/

/**
    ejsWeb.h -- Header for the Ejscript Web Framework
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EJS_WEB_h
#define _h_EJS_WEB_h 1

#include    "http.h"


/*********************************** Defines **********************************/

#define EJS_SESSION "-ejs-session-"             /**< Default session cookie string */

#ifdef  __cplusplus
extern "C" {
#endif

/*********************************** Types ************************************/

#ifndef EJS_HTTPSERVER_NAME
#define EJS_HTTPSERVER_NAME "ejs-http"
#endif

/** 
    HttpServer Class
    @description
        HttpServer objects represents a Hypertext Transfer Protocol version 1.1 client connection and are used 
        HTTP requests and capture responses. This class supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
    @stability Prototype
    @defgroup EjsHttpServer EjsHttpServer
    @see EjsHttpServer ejsCloneHttpServer
 */
typedef struct EjsHttpServer {
    EjsPot          pot;                        /**< Extends Object */
    Ejs             *ejs;                       /**< Ejscript interpreter handle */
    HttpEndpoint    *endpoint;                  /**< Http endpoint object */
    struct MprSsl   *ssl;                       /**< SSL configuration */
    HttpTrace       trace[2];                   /**< Default tracing for requests */
    cchar           *connector;                 /**< Pipeline connector */
    char            *keyFile;                   /**< SSL key file */
    char            *certFile;                  /**< SSL certificate file */
    char            *protocols;                 /**< SSL protocols */
    char            *ciphers;                   /**< SSL ciphers */
    char            *ip;                        /**< Listening address */
    char            *name;                      /**< Server name */
    int             async;                      /**< Async mode */
    int             port;                       /**< Listening port */
    int             hosted;                     /**< Server being hosted inside a web server */
    struct EjsHttpServer *cloned;               /**< Server that was cloned */
    EjsObj          *emitter;                   /**< Event emitter */
    EjsObj          *limits;                    /**< Limits object */
    EjsArray        *incomingStages;            /**< Incoming Http pipeline stages */
    EjsArray        *outgoingStages;            /**< Outgoing Http pipeline stages */
} EjsHttpServer;

/** 
    Clone a http server
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server HttpServer object
    @param deep Ignored
    @returns A new server object.
    @ingroup EjsHttpServer
*/
extern EjsHttpServer *ejsCloneHttpServer(Ejs *ejs, EjsHttpServer *server, bool deep);

/** 
    Request Class
    @description
        Request objects represent a single Http request.
    @stability Prototype
    @defgroup EjsRequest EjsRequest
    @see EjsRequest ejsCloneRequest ejsCreateRequest 
 */
typedef struct EjsRequest {
    EjsPot          pot;                /**< Base object storage */
    EjsObj          *absHome;           /**< Absolute URI to the home of the application from this request */
    struct EjsRequest *cloned;          /**< Request that was cloned */
    EjsObj          *config;            /**< Request config environment */
    HttpConn        *conn;              /**< Underlying Http connection object */
    EjsObj          *cookies;           /**< Cached cookies */
    EjsPath         *dir;               /**< Home directory containing the application */
    EjsObj          *emitter;           /**< Event emitter */
    EjsObj          *env;               /**< Request.env */
    EjsPath         *filename;          /**< Physical resource filename */
    EjsObj          *files;             /**< Files object */
    EjsString       *formData;          /**< Form data as a stable, sorted string */
    EjsObj          *headers;           /**< Headers object */
    EjsUri          *home;              /**< Relative URI to the home of the application from this request */
    EjsString       *host;              /**< Host property */
    EjsObj          *limits;            /**< Limits object */
    EjsObj          *log;               /**< Log object */
    EjsObj          *originalUri;       /**< Saved original URI */
    EjsObj          *params;            /**< Form variables + routing variables */
    EjsString       *pathInfo;          /**< PathInfo property */
    EjsNumber       *port;              /**< Port property */
    EjsString       *query;             /**< Query property */
    EjsString       *reference;         /**< Reference property */
    EjsObj          *responseHeaders;   /**< Headers object */
    EjsObj          *route;             /**< Matching route in route table */
    EjsString       *scheme;            /**< Scheme property */
    EjsString       *scriptName;        /**< ScriptName property */
    EjsHttpServer   *server;            /**< Owning server */
    EjsUri          *uri;               /**< Complete uri */
    EjsByteArray    *writeBuffer;       /**< Write buffer for capturing output */

    Ejs             *ejs;               /**< Ejscript interpreter handle */
    struct EjsSession *session;         /**< Current session */

    //  OPT - make bit fields
    int             dontAutoFinalize;   /**< Suppress auto-finalization */
    int             probedSession;      /**< Determined if a session exists */
    int             closed;             /**< Request closed and "close" event has been issued */
    int             error;              /**< Request errored and "error" event has been issued */
    int             finalized;          /**< Request has written all output data */
    int             running;            /**< Request has started */
    ssize           written;            /**< Count of data bytes written to the client */
} EjsRequest;

/** 
    Clone a request into another interpreter.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param req Original request to copy
    @param deep Ignored
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCloneRequest(Ejs *ejs, EjsRequest *req, bool deep);

/** 
    Create a new request. Create a new request object associated with the given Http connection.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param server EjsHttpServer owning this request
    @param conn Http connection object
    @param dir Default directory containing web documents
    @return A new request object.
    @ingroup EjsRequest
*/
extern EjsRequest *ejsCreateRequest(Ejs *ejs, EjsHttpServer *server, HttpConn *conn, cchar *dir);


/** 
    Session Class. Requests can access to session state storage via the Session class.
    @description
        Session objects represent a shared session state object into which Http Requests and store and retrieve data
        that persists beyond a single request.
    @stability Prototype
    @defgroup EjsSession EjsSession
    @see EjsSession ejsGetSession ejsDestroySession
 */
typedef struct EjsSession {
    EjsPot      pot;                /* Session properties */
    EjsString   *key;               /* Session ID key */
    EjsObj      *cache;             /* Cache store reference */
    EjsObj      *options;           /* Default write options */
    MprTicks    timeout;            /* Session inactivity timeout (msecs) */
    int         ready;              /* Data cached from store into pot */
} EjsSession;

/** 
    Get a session object for a given key. This will create a session if the given key is NULL or has expired.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param key String containing the session ID
    @param timeout Timeout to use for the session if one is created
    @param create Create a new session if an existing session cannot be found or it has expired.
    @returns A new session object.
    @ingroup EjsSession
*/
extern EjsSession *ejsGetSession(Ejs *ejs, EjsString *key, MprTicks timeout, int create);

/** 
    Destroy as session. This destroys the session object so that subsequent requests will need to establish a new session.
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param session Session object created via ejsGetSession()
    @ingroup EjsSession
 */
extern int ejsDestroySession(Ejs *ejs, EjsSession *session);

/** 
    Set a session timeout
    @param ejs Ejs interpreter handle returned from $ejsCreate
    @param sp Session object
    @param lifespan Lifespan in milliseconds
    @ingroup EjsSession
*/
extern void ejsSetSessionTimeout(Ejs *ejs, EjsSession *sp, MprTicks lifespan);

/******************************* Internal APIs ********************************/

extern void ejsConfigureHttpServerType(Ejs *ejs);
extern void ejsConfigureRequestType(Ejs *ejs);
extern void ejsConfigureSessionType(Ejs *ejs);
extern void ejsConfigureWebTypes(Ejs *ejs);
extern void ejsSendRequestCloseEvent(Ejs *ejs, EjsRequest *req);
extern void ejsSendRequestErrorEvent(Ejs *ejs, EjsRequest *req);

#ifdef  __cplusplus
}
#endif
#endif /* _h_EJS_WEB_h */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
    Start of file "src/ejsCompiler.h"
 */
/************************************************************************/

/*
    ejsCompiler.h - Internal compiler header.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#ifndef _h_EC_COMPILER
#define _h_EC_COMPILER 1



#ifdef __cplusplus
extern "C" {
#endif

/*********************************** Defines **********************************/
/*
    Compiler validation modes. From "use standard|strict"
 */
//  TODO DOC
#define PRAGMA_MODE_STANDARD    1               /* Standard unstrict mode */
#define PRAGMA_MODE_STRICT      2               /* Strict mode */

//  TODO DOC
#define STRICT_MODE(cp)         (cp->fileState->strict)

/*
    Variable Kind bits
 */
#define KIND_CONST              0x1
#define KIND_VAR                0x2
#define KIND_LET                0x4

/*
    Phases for AST processing
 */
//  TODO DOC
#define EC_PHASE_DEFINE         0           /* Define types, functions and properties in types */
#define EC_PHASE_CONDITIONAL    1           /* Do conditional processing, hoisting and then type fixups */
#define EC_PHASE_FIXUP          2           /* Fixup type references */
#define EC_PHASE_BIND           3           /* Bind var references to slots and property types */
#define EC_AST_PHASES           4

typedef struct EcLocation {
    wchar       *source;
    char        *filename;
    int         lineNumber;
    int         column;
} EcLocation;

#define N_ARGS                  1
#define N_ASSIGN_OP             2
#define N_ATTRIBUTES            3
#define N_BINARY_OP             4
#define N_BINARY_TYPE_OP        5
#define N_BLOCK                 6
#define N_BREAK                 7
#define N_CALL                  8
#define N_CASE_ELEMENTS         9
#define N_CASE_LABEL            10
#define N_CATCH                 11
#define N_CATCH_ARG             12
#define N_CATCH_CLAUSES         13
#define N_CLASS                 14
#define N_CONTINUE              15
#define N_DASSIGN               16
#define N_DIRECTIVES            17
#define N_DO                    18
#define N_DOT                   19
#define N_END_FUNCTION          20
#define N_EXPRESSIONS           21
#define N_FIELD                 22
#define N_FOR                   23
#define N_FOR_IN                24
#define N_FUNCTION              25
#define N_GOTO                  26
#define N_HASH                  27
#define N_IF                    28
#define N_LITERAL               29
#define N_MODULE                30
#define N_NEW                   31
#define N_NOP                   32
#define N_OBJECT_LITERAL        33
#define N_PARAMETER             34
#define N_POSTFIX_OP            35
#define N_PRAGMA                36
#define N_PRAGMAS               37
#define N_PROGRAM               38
#define N_QNAME                 39
#define N_REF                   40
#define N_RETURN                41
#define N_SPREAD                42
#define N_SUPER                 43
#define N_SWITCH                44
#define N_THIS                  45
#define N_THROW                 46
#define N_TRY                   47
#define N_TYPE_IDENTIFIERS      48
#define N_UNARY_OP              49
#define N_USE_MODULE            50
#define N_USE_NAMESPACE         51
#define N_VALUE                 52
#define N_VAR                   53
#define N_VAR_DEFINITION        54
#define N_VOID                  55
#define N_WITH                  56

/*
    Ast node define
 */
#if !DOXYGEN
typedef struct EcNode   *Node;
#endif

//  TODO DOC
/*
    Structure for code generation buffers
 */
typedef struct EcCodeGen {
//  TODO DOC
    MprBuf      *buf;                           /* Code generation buffer */
    MprList     *jumps;                         /* Break/continues to patch for this code block */
    MprList     *exceptions;                    /* Exception handlers for this code block */
    EjsDebug    *debug;                         /* Source debug info */ 
    int         jumpKinds;                      /* Kinds of jumps allowed */
    int         breakMark;                      /* Stack item counter for the target for break/continue stmts */
    int         blockMark;                      /* Lexical block counter for the target for break/continue stmts */
    int         stackCount;                     /* Current stack item counter */
    int         blockCount;                     /* Current block counter */
    int         lastLineNumber;                 /* Last line for debug */
} EcCodeGen;


//  TODO DOC
typedef struct EcNode {
    char                *kindName;              /* Node kind string */
#if BIT_DEBUG
    char                *tokenName;
#endif
    EjsName             qname;
    EcLocation          loc;                    /* Source code info */
    EjsBlock            *blockRef;              /* Block scope */
    EjsLookup           lookup;                 /* Lookup residuals */
    EjsNamespace        *namespaceRef;          /* Namespace reference */
    Node                left;                   /* children[0] */
    Node                right;                  /* children[1] */
    Node                typeNode;               /* Type of name */
    Node                parent;                 /* Parent node */
    MprList             *namespaces;            /* Namespaces for hoisted variables */
    MprList             *children;

    int                 kind;                   /* Kind of node */
    int                 attributes;             /* Attributes applying to this node */
    int                 tokenId;                /* Lex token */
    int                 groupMask;              /* Token group */
    int                 subId;                  /* Sub token */
    int                 slotNum;                /* Allocated slot for variable */
    int                 jumpLength;             /* Goto length for exceptions */
    int                 seqno;                  /* Unique sequence number */

    uint                blockCreated      : 1;  /* Block object has been created */
    uint                createBlockObject : 1;  /* Create the block object to contain let scope variables */
    uint                enabled           : 1;  /* Node is enabled via conditional definitions */
    int                 literalNamespace  : 1;  /* Namespace is a literal */
    uint                needThis          : 1;  /* Need to push this object */
    uint                needDupObj        : 1;  /* Need to dup the object on stack (before) */
    uint                needDup           : 1;  /* Need to dup the result (after) */
    uint                slotFixed         : 1;  /* Slot fixup has been done */
    uint                specialNamespace  : 1;  /* Using a public|private|protected|internal namespace */

    uchar               *patchAddress;          /* For code patching */
    EcCodeGen           *code;                  /* Code buffer */

    EjsName             *globalProp;            /* Set if this is a global property */
    EjsString           *doc;                   /* Documentation string */

    struct EcCompiler   *cp;                    /* Compiler instance reference */

#if BIT_HAS_UNNAMED_UNIONS
    union {
#endif
        struct {
            Node        expression;
            EcCodeGen   *expressionCode;        /* Code buffer for the case expression */
            int         kind;
            int         nextCaseCode;           /* Goto length to the next case statements */
        } caseLabel;

        struct {
            Node        arg;                    /* Catch block argument */
        } catchBlock;

        /*
            Var definitions have one child per variable. Child nodes can be either N_NAME or N_ASSIGN_OP
         */
        struct {
            int         varKind;                /* Variable definition kind */
        } def;

        struct {
            /* Children are the catch clauses */
            Node        tryBlock;               /* Try code */
            Node        catchClauses;           /* Catch clauses */
            Node        finallyBlock;           /* Finally code */
            int         numBlocks;              /* Count of open blocks in the try block */
        } exception;

        struct {
            Node        expr;                   /* Field expression */
            Node        fieldName;              /* Field element name for objects */
            int         fieldKind;              /* value or function */
            int         index;                  /* Array index, set to -1 for objects */
            int         varKind;                /* Variable definition kind (const) */
        } field;

        struct {
            Node        resultType;             /* Function return type */
            Node        body;                   /* Function body */
            Node        parameters;             /* Function formal parameters */
            Node        constructorSettings;    /* Constructor settings */
            EjsFunction *functionVar;           /* Function variable */
            uint        operatorFn    : 1;      /* operator function */
            uint        getter        : 1;      /* getter function */
            uint        setter        : 1;      /* setter function */
            uint        call          : 1;      /* */
            uint        has           : 1;      /* */
            uint        hasRest       : 1;      /* Has rest parameter */
            uint        hasReturn     : 1;      /* Has a return statement */
            uint        isMethod      : 1;      /* Is a class method */
            uint        isConstructor : 1;      /* Is constructor method */
            uint        isDefault     : 1;      /* Is default constructor */
            uint        isExpression  : 1;      /* Is a function expression */
        } function;

        struct {
            Node        iterVar;
            Node        iterGet;
            Node        iterNext;
            Node        body;
            EcCodeGen   *initCode;
            EcCodeGen   *bodyCode;
            int         each;                   /* For each used */
        } forInLoop;

        struct {
            Node        body;
            Node        cond;
            Node        initializer;
            Node        perLoop;
            EcCodeGen   *condCode;
            EcCodeGen   *bodyCode;
            EcCodeGen   *perLoopCode;
        } forLoop;

        struct {
            Node        body;
            Node        expr;
            bool        disabled;
        } hash;

        struct {
            Node         implements;          /* Implemented interfaces */
            Node         constructor;         /* Class constructor */
            MprList      *staticProperties;   /* List of static properties */
            MprList      *instanceProperties; /* Implemented interfaces */
            MprList      *classMethods;       /* Static methods */
            MprList      *methods;            /* Instance methods */
            EjsType      *ref;                /* Type instance ref */
            EjsFunction  *initializer;        /* Initializer function */
            EjsNamespace *publicSpace;
            EjsNamespace *internalSpace;
            EjsString    *extends;            /* Class base class */
            int          isInterface;         /* This is an interface */
        } klass;

        struct {
            EjsObj      *var;               /* Special value */
            MprBuf      *data;              /* XML data */
        } literal;

        struct {
            EjsModule   *ref;               /* Module object */
            EjsString   *name;              /* Module name */
            char        *filename;          /* Module file name */
            int         version;
        } module;

        /*
            Name nodes hold a fully qualified name.
         */
        struct {
            Node        nameExpr;           /* Name expression */
            Node        qualifierExpr;      /* Qualifier expression */
            EjsObj      *nsvalue;           /* Initialization value (TODO - remove) */
            uint        instanceVar  : 1;   /* Instance or static var (if defined in class) */
            uint        isAttribute  : 1;   /* Attribute identifier "@" */
            uint        isDefault    : 1;   /* use default namespace */
            uint        isInternal   : 1;   /* internal namespace */
            uint        isLiteral    : 1;   /* use namespace "literal" */
            uint        isNamespace  : 1;   /* Name is a namespace */
            uint        isRest       : 1;   /* ... rest style args */
            uint        isType       : 1;   /* Name is a type */
            uint        letScope     : 1;   /* Variable is defined in let block scope */
            int         varKind;            /* Variable definition kind */
        } name;

        struct {
            int         callConstructors;   /* Bound type has a constructor */
        } newExpr;

        struct {
            Node        typeNode;           /* Type of object */
            int         isArray;            /* Array literal */
        } objectLiteral;

        struct {
            uint        strict;             /* Strict mode */
        } pragma;

        struct {
            MprList     *dependencies;      /* Accumulated list of dependent modules */
        } program;

        struct {
            Node        node;               /* Actual node reference */
        } ref;

        struct {
            int         blockless;          /* Function expression */
        } ret;

        struct {
            Node        cond;
            Node        thenBlock;
            Node        elseBlock;
            EcCodeGen   *thenCode;
            EcCodeGen   *elseCode;
        } tenary;

        struct {
            int         thisKind;           /* Kind of this. See EC_THIS_ flags */
        } thisNode;

        struct {
            int         minVersion;
            int         maxVersion;
        } useModule;

        struct {
            Node        object;
            Node        statement;
        } with;
#if BIT_HAS_UNNAMED_UNIONS
    };
#endif
} EcNode;


/*
    Various per-node flags
 */
#define EC_THIS_GENERATOR       1
#define EC_THIS_CALLEE          2
#define EC_THIS_TYPE            3
#define EC_THIS_FUNCTION        4

#define EC_SWITCH_KIND_CASE     1   /* Case block */
#define EC_SWITCH_KIND_DEFAULT  2   /* Default block */

/*
    Object (and Array) literal field
 */
#define FIELD_KIND_VALUE        0x1
#define FIELD_KIND_FUNCTION     0x2

#define EC_NUM_NODES            8
#define EC_TAB_WIDTH            4

/*
    Fix clash with arpa/nameser.h
 */
#undef T_NULL

/*
    Lexical tokens (must start at 1)
    ASSIGN tokens must be +1 compared to their non-assignment counterparts.
    (Use genTokens to recreate)
    WARNING: ensure T_MOD and T_MOD_ASSIGN are adjacent. rewriteCompoundAssignment relies on this
 */
#define T_ASSIGN                    1
#define T_AT                        2
#define T_ATTRIBUTE                 3
#define T_BIT_AND                   4
#define T_BIT_AND_ASSIGN            5
#define T_BIT_OR                    6
#define T_BIT_OR_ASSIGN             7
#define T_BIT_XOR                   8
#define T_BIT_XOR_ASSIGN            9
#define T_BREAK                    10
#define T_CALL                     11
#define T_CALLEE                   12
#define T_CASE                     13
#define T_CAST                     14
#define T_CATCH                    15
#define T_CDATA_END                16
#define T_CDATA_START              17
#define T_CLASS                    18
#define T_COLON                    19
#define T_COLON_COLON              20
#define T_COMMA                    21
#define T_CONST                    22
#define T_CONTEXT_RESERVED_ID      23
#define T_CONTINUE                 24
#define T_DEBUGGER                 25
#define T_DECREMENT                26
#define T_DEFAULT                  27
#define T_DELETE                   28
#define T_DIV                      29
#define T_DIV_ASSIGN               30
#define T_DO                       31
#define T_DOT                      32
#define T_DOT_DOT                  33
#define T_DOT_LESS                 34
#define T_DOUBLE                   35
#define T_DYNAMIC                  36
#define T_EACH                     37
#define T_ELIPSIS                  38
#define T_ELSE                     39
#define T_ENUMERABLE               40
#define T_EOF                      41
#define T_EQ                       42
#define T_ERR                      43
#define T_EXTENDS                  44
#define T_FALSE                    45
#define T_FINAL                    46
#define T_FINALLY                  47
#define T_FLOAT                    48
#define T_FOR                      49
#define T_FUNCTION                 50
#define T_GE                       51
#define T_GENERATOR                52
#define T_GET                      53
#define T_GOTO                     54
#define T_GT                       55
#define T_HAS                      56
#define T_HASH                     57
#define T_ID                       58
#define T_IF                       59
#define T_IMPLEMENTS               60
#define T_IN                       61
#define T_INCLUDE                  62
#define T_INCREMENT                63
#define T_INSTANCEOF               64
#define T_INT                      65
#define T_INTERFACE                66
#define T_INTERNAL                 67
#define T_INTRINSIC                68
#define T_IS                       69
#define T_LBRACE                   70
#define T_LBRACKET                 71
#define T_LE                       72
#define T_LET                      73
#define T_LOGICAL_AND              74
#define T_LOGICAL_AND_ASSIGN       75
#define T_LOGICAL_NOT              76
#define T_LOGICAL_OR               77
#define T_LOGICAL_OR_ASSIGN        78
#define T_LOGICAL_XOR              79
#define T_LOGICAL_XOR_ASSIGN       80
#define T_LPAREN                   81
#define T_LSH                      82
#define T_LSH_ASSIGN               83
#define T_LT                       84
#define T_LT_SLASH                 85
#define T_MINUS                    86
#define T_MINUS_ASSIGN             87
#define T_MINUS_MINUS              88
#define T_MODULE                   89
#define T_MOD                      90       // WARNING sorted order manually fixed!!
#define T_MOD_ASSIGN               91
#define T_MUL                      92
#define T_MUL_ASSIGN               93
#define T_NAMESPACE                94
#define T_NATIVE                   95
#define T_NE                       96
#define T_NEW                      97
#define T_NOP                      98
#define T_NULL                     99
#define T_NUMBER                  100
#define T_NUMBER_WORD             101
#define T_OVERRIDE                102
#define T_PLUS                    103
#define T_PLUS_ASSIGN             104
#define T_PLUS_PLUS               105
#define T_PRIVATE                 106
#define T_PROTECTED               107
#define T_PROTOTYPE               108
#define T_PUBLIC                  109
#define T_QUERY                   110
#define T_RBRACE                  111
#define T_RBRACKET                112
#define T_REGEXP                  113
#define T_REQUIRE                 114
#define T_RESERVED_NAMESPACE      115
#define T_RETURN                  116
#define T_RPAREN                  117
#define T_RSH                     118
#define T_RSH_ASSIGN              119
#define T_RSH_ZERO                120
#define T_RSH_ZERO_ASSIGN         121
#define T_SEMICOLON               122
#define T_SET                     123
#define T_SLASH_GT                124
#define T_STANDARD                125
#define T_STATIC                  126
#define T_STRICT                  127
#define T_STRICT_EQ               128
#define T_STRICT_NE               129
#define T_STRING                  130
#define T_SUPER                   131
#define T_SWITCH                  132
#define T_THIS                    133
#define T_THROW                   134
#define T_TILDE                   135
#define T_TO                      136
#define T_TRUE                    137
#define T_TRY                     138
#define T_TYPE                    139
#define T_TYPEOF                  140
#define T_UINT                    141
#define T_UNDEFINED               142
#define T_USE                     143
#define T_VAR                     144
#define T_VOID                    145
#define T_WHILE                   146
#define T_WITH                    147
#define T_XML_COMMENT_END         148
#define T_XML_COMMENT_START       149
#define T_XML_PI_END              150
#define T_XML_PI_START            151
#define T_YIELD                   152

/*
    Group masks
 */
#define G_RESERVED          0x1
#define G_CONREV            0x2
#define G_COMPOUND_ASSIGN   0x4                 /* Eg. <<= */
#define G_OPERATOR          0x8                 /* Operator overload*/

/*
    Attributes (including reserved namespaces)
 */
#define A_FINAL         0x1
#define A_OVERRIDE      0x2
#define A_EARLY         0x4                     /* Early binding */
#define A_DYNAMIC       0x8
#define A_NATIVE        0x10
#define A_PROTOTYPE     0x20
#define A_STATIC        0x40
#define A_ENUMERABLE    0x40

#define EC_INPUT_STREAM "__stdin__"

struct EcStream;
typedef int (*EcStreamGet)(struct EcStream *stream);

/*
    Stream flags
 */
#define EC_STREAM_EOL       0x1                 /* End of line */

//  TODO DOC

typedef struct EcStream {
    struct EcCompiler *compiler;                /* Compiler back reference */
    EcLocation  loc;                            /* Source code debug info */
    EcLocation  lastLoc;                        /* Location info for a prior line */
    EcStreamGet getInput;                       /* Get more input callback */
    wchar       *buf;                           /* Buffer holding source file */
    wchar       *nextChar;                      /* Ptr to next input char */
    wchar       *end;                           /* Ptr to one past end of buf */
    bool        eof;                            /* At end of file */
    int         flags;                          /* Input flags */
} EcStream;


/*
    Parse source code from a file
 */
//  TODO DOC
typedef struct EcFileStream {
    EcStream    stream;
    MprFile     *file;
} EcFileStream;


/*
    Parse source code from a memory block
 */
//  TODO DOC
typedef struct EcMemStream {
    EcStream    stream;
} EcMemStream;


/*
    Parse input from the console (or file if using ejsh)
 */
//  TODO DOC
typedef struct EcConsoleStream {
    EcStream    stream;
} EcConsoleStream;


/*
    Program source input tokens
 */
//  TODO DOC
typedef struct EcToken {
    wchar       *text;                  /* Token text */
    int         length;                 /* Length of text in characters */
    int         size;                   /* Size of text in characters */
    int         tokenId;
    int         subId;
    int         groupMask;
    int         eol;                    /* At the end of the line */
    EcLocation  loc;                    /* Source code debug info */
    struct EcToken *next;               /* Putback and freelist linkage */
    EcStream    *stream;
#if BIT_DEBUG
    char        *name;                  /* Debug token name */
#endif
} EcToken;


/*
    Jump types
 */
#define EC_JUMP_BREAK       0x1
#define EC_JUMP_CONTINUE    0x2
#define EC_JUMP_GOTO        0x4

//  TODO DOC
typedef struct EcJump {
    int             kind;               /* Break, continue */
    int             offset;             /* Code offset to patch */
    EcNode          *node;              /* Owning node */
} EcJump;


/*
    Current parse state. Each non-terminal production has its own state.
    Some state fields are inherited. We keep a linked list from EcCompiler.
 */
//  TODO DOC
typedef struct EcState {
    struct EcState  *next;                  /* State stack */
    uint            blockIsMethod    : 1;   /* Current function is a method */
    uint            captureBreak     : 1;   /* Capture break/continue inside a catch/finally block */
    uint            captureFinally   : 1;   /* Capture break/continue with a finally block */
    uint            conditional      : 1;   /* In branching conditional */
    uint            disabled         : 1;   /* Disable nodes below this scope */
    uint            dupLeft          : 1;   /* Dup left side */
    uint            inClass          : 1;   /* Inside a class declaration */
    uint            inFunction       : 1;   /* Inside a function declaration */
    uint            inHashExpression : 1;   /* Inside a # expression */
    uint            inInterface      : 1;   /* Inside an interface */
    uint            inMethod         : 1;   /* Inside a method declaration */
    uint            inSettings       : 1;   /* Inside constructor settings */
    uint            instanceCode     : 1;   /* Generating instance class code */
    uint            needsValue       : 1;   /* Expression must yield a value */
    uint            noin             : 1;   /* Don't allow "in" */
    uint            onLeft           : 1;   /* On the left of an assignment */
    uint            saveOnLeft       : 1;   /* Saved left of an assignment */
    uint            strict           : 1;   /* Compiler checking mode: Strict, standard*/

    int             blockNestCount;         /* Count of blocks encountered. Used by ejs shell */
    int             namespaceCount;         /* Count of namespaces originally in block. Used to pop namespaces */

    EjsModule       *currentModule;         /* Current open module definition */
    EjsType         *currentClass;          /* Current open class */
    EjsName         currentClassName;       /* Current open class name */
    EcNode          *currentClassNode;      /* Current open class */
    EjsFunction     *currentFunction;       /* Current open method */
    EcNode          *currentFunctionNode;   /* Current open method */
    EcNode          *currentObjectNode;     /* Left object in "." or "[" */
    EcNode          *topVarBlockNode;       /* Top var block node */

    EjsBlock        *letBlock;              /* Block for local block scope declarations */
    EjsBlock        *varBlock;              /* Block for var declarations */
    EjsBlock        *optimizedLetBlock;     /* Optimized let block declarations - may equal ejs->global */
    EcNode          *letBlockNode;          /* Node for the current let block */

    EjsString       *nspace;                /* Namespace for declarations */
    EjsString       *defaultNamespace;      /* Default namespace for new top level declarations. Does not propagate */

    EcCodeGen       *code;                  /* Global and function code buffer */
    EcCodeGen       *staticCodeBuf;         /* Class static level code generation buffer */
    EcCodeGen       *instanceCodeBuf;       /* Class instance level code generation buffer */

    struct EcState  *prevBlockState;        /* Block state stack */
    struct EcState  *breakState;            /* State for breakable blocks */
    struct EcState  *classState;            /* State for current class */
} EcState;


PUBLIC int      ecEnterState(struct EcCompiler *cp);
PUBLIC void     ecLeaveState(struct EcCompiler *cp);
PUBLIC EcNode   *ecLeaveStateWithResult(struct EcCompiler *cp,  struct EcNode *np);
PUBLIC int      ecResetModule(struct EcCompiler *cp, struct EcNode *np);
PUBLIC void     ecStartBreakableStatement(struct EcCompiler *cp, int kinds);


/*
    Primary compiler control structure
 */
//  TODO DOC
typedef struct EcCompiler {
    /*
        Properties ordered to make debugging easier
     */
    int         phase;                      /* Ast processing phase */
    EcState     *state;                     /* Current state */
    EcToken     *peekToken;                 /* Peek ahead token */
    EcToken     *token;                     /* Current input token */

    /*  Lexer */
    MprHash     *keywords;
    EcStream    *stream;
    EcToken     *putback;                   /* List of active putback tokens */
    char        *docToken;                  /* Last doc token */

    EcState     *fileState;                 /* Top level state for the file */
//  TODO -- these are risky and should be moved into state. A nested block, directive class etc willl modify
    EcState     *directiveState;            /* State for the current directive - used in parse and CodeGen */
    EcState     *blockState;                /* State for the current block */

    EjsLookup   lookup;                     /* Lookup residuals */
    EjsService  *vmService;                 /* VM runtime */
    Ejs         *ejs;                       /* Interpreter instance */
    MprList     *nodes;                     /* Compiled AST nodes */

    /*
        Compiler command line options
     */
    char        *certFile;                  /* Certificate to sign the module file */
    bool        debug;                      /* Run in debug mode */
    bool        doc;                        /* Include documentation strings in output */
    char        *extraFiles;                /* Extra source files to compile */

    MprList     *require;                   /* Required list of modules to pre-load */
    bool        interactive;                /* Interactive use (ejsh) */
    bool        merge;                      /* Merge all dependent modules */
    bool        bind;                       /* Don't bind properties to slots */
    bool        noout;                      /* Don't generate any module output files */
    bool        visibleGlobals;             /* Make globals visible (no namespace) */
    int         optimizeLevel;              /* Optimization factor (0-9) */
    bool        shbang;                     /* Observe #!/path as the first line of a script */
    int         warnLevel;                  /* Warning level factor (0-9) */

    int         strict;                     /* Compiler default strict mode */
    int         lang;                       /* Language compliance level: ecma|plus|fixed */
    char        *outputDir;                 /* Output directory for modules */
    char        *outputFile;                /* Output module file name override */
    MprFile     *file;                      /* Current output file handle */

    int         modver;                     /* Default module version */
    int         parseOnly;                  /* Only parse the code */
    int         strip;                      /* Strip debug symbols */
    int         tabWidth;                   /* For error reporting "^" */

    MprList     *modules;                   /* List of modules to process */
    MprList     *fixups;                    /* Type reference fixups */

    char        *errorMsg;                  /* Aggregated error messages */
    int         error;                      /* Unresolved parse error */
    int         fatalError;                 /* Any a fatal error - Can't continue */
    int         errorCount;                 /* Count of all errors */
    int         warningCount;               /* Count of all warnings */
    int         nextSeqno;                  /* Node sequence numbers */
    int         blockLevel;                 /* Level of nest in blocks */

    /*
        TODO - aggregate these into flags
     */
    int         lastOpcode;                 /* Last opcode encoded */
    int         uid;                        /* Unique identifier generator */
} EcCompiler;

/********************************** Prototypes *******************************/

//  TODO -- reorder
//  TODO DOC
PUBLIC int          ecAddModule(EcCompiler *cp, EjsModule *mp);
PUBLIC EcNode       *ecAppendNode(EcNode *np, EcNode *child);
PUBLIC int          ecAstFixup(EcCompiler *cp, struct EcNode *np);
PUBLIC EcNode       *ecChangeNode(EcCompiler *cp, EcNode *np, EcNode *oldNode, EcNode *newNode);
PUBLIC void         ecGenConditionalCode(EcCompiler *cp, EcNode *np, EjsModule *up);
PUBLIC int          ecCodeGen(EcCompiler *cp);
PUBLIC int          ecCompile(EcCompiler *cp, int argc, char **path);
PUBLIC EcCompiler   *ecCreateCompiler(struct Ejs *ejs, int flags);
PUBLIC void         ecDestroyCompiler(EcCompiler *cp);
PUBLIC void         ecInitLexer(EcCompiler *cp);
PUBLIC EcNode       *ecCreateNode(EcCompiler *cp, int kind);
PUBLIC void         ecFreeToken(EcCompiler *cp, EcToken *token);
PUBLIC char         *ecGetErrorMessage(EcCompiler *cp);
PUBLIC EjsString    *ecGetInputStreamName(EcCompiler *lp);
PUBLIC int          ecGetToken(EcCompiler *cp);
PUBLIC int          ecGetRegExpToken(EcCompiler *cp, wchar *prefix);
PUBLIC EcNode       *ecLinkNode(EcNode *np, EcNode *child);

PUBLIC EjsModule    *ecLookupModule(EcCompiler *cp, EjsString *name, int minVersion, int maxVersion);
PUBLIC int          ecLookupScope(EcCompiler *cp, EjsName name);
PUBLIC int          ecLookupVar(EcCompiler *cp, EjsAny *vp, EjsName name);
PUBLIC EcNode       *ecParseWarning(EcCompiler *cp, char *fmt, ...);
PUBLIC int          ecPeekToken(EcCompiler *cp);
PUBLIC int          ecPutSpecificToken(EcCompiler *cp, EcToken *token);
PUBLIC int          ecPutToken(EcCompiler *cp);
PUBLIC void         ecError(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, ...);
PUBLIC void         ecErrorv(EcCompiler *cp, cchar *severity, EcLocation *loc, cchar *fmt, va_list args);
PUBLIC void         ecResetInput(EcCompiler *cp);
PUBLIC EcNode       *ecResetError(EcCompiler *cp, EcNode *np, bool eatInput);
PUBLIC int          ecRemoveModule(EcCompiler *cp, EjsModule *mp);
PUBLIC void         ecResetParser(EcCompiler *cp);
PUBLIC int          ecResetModuleList(EcCompiler *cp);
PUBLIC int          ecOpenConsoleStream(EcCompiler *cp, EcStreamGet gets, cchar *contents);
PUBLIC int          ecOpenFileStream(EcCompiler *cp, cchar *path);
PUBLIC int          ecOpenMemoryStream(EcCompiler *cp, cchar *contents, ssize len);
PUBLIC void         ecCloseStream(EcCompiler *cp);
PUBLIC void         ecSetOptimizeLevel(EcCompiler *cp, int level);
PUBLIC void         ecSetWarnLevel(EcCompiler *cp, int level);
PUBLIC void         ecSetStrictMode(EcCompiler *cp, int on);
PUBLIC void         ecSetTabWidth(EcCompiler *cp, int width);
PUBLIC void         ecSetOutputDir(EcCompiler *cp, cchar *outputDir);
PUBLIC void         ecSetOutputFile(EcCompiler *cp, cchar *outputFile);
PUBLIC void         ecSetCertFile(EcCompiler *cp, cchar *certFile);
PUBLIC EcToken      *ecTakeToken(EcCompiler *cp);
PUBLIC int          ecAstProcess(struct EcCompiler *cp);
PUBLIC void         *ecCreateStream(EcCompiler *cp, ssize size, cchar *filename, void *manager);
PUBLIC void         ecSetStreamBuf(EcStream *sp, cchar *contents, ssize len);
PUBLIC EcNode       *ecParseFile(EcCompiler *cp, char *path);
PUBLIC void         ecManageStream(EcStream *sp, int flags);
PUBLIC void         ecMarkLocation(EcLocation *loc);
PUBLIC void         ecSetRequire(EcCompiler *cp, MprList *modules);


/*
    Module file creation routines.
 */
PUBLIC void     ecAddFunctionConstants(EcCompiler *cp, EjsPot *obj, int slotNum);
PUBLIC void     ecAddConstants(EcCompiler *cp, EjsAny *obj);
PUBLIC int      ecAddStringConstant(EcCompiler *cp, EjsString *sp);
PUBLIC int      ecAddCStringConstant(EcCompiler *cp, cchar *str);
PUBLIC int      ecAddNameConstant(EcCompiler *cp, EjsName qname);
PUBLIC int      ecAddDocConstant(EcCompiler *cp, cchar *tag, void *vp, int slotNum);
PUBLIC int      ecAddModuleConstant(EcCompiler *cp, EjsModule *up, cchar *str);
PUBLIC int      ecCreateModuleHeader(EcCompiler *cp);
PUBLIC int      ecCreateModuleSection(EcCompiler *cp);


/*
    Encoding emitter routines
 */
PUBLIC void      ecEncodeBlock(EcCompiler *cp, cuchar *buf, int len);
PUBLIC void      ecEncodeByte(EcCompiler *cp, int value);
PUBLIC void      ecEncodeByteAtPos(EcCompiler *cp, int offset, int value);
PUBLIC void      ecEncodeConst(EcCompiler *cp, EjsString *sp);
PUBLIC void      ecEncodeDouble(EcCompiler *cp, double value);
PUBLIC void      ecEncodeGlobal(EcCompiler *cp, EjsAny *obj, EjsName qname);
PUBLIC void      ecEncodeInt32(EcCompiler *cp, int value);
PUBLIC void      ecEncodeInt32AtPos(EcCompiler *cp, int offset, int value);
PUBLIC void      ecEncodeNum(EcCompiler *cp, int64 number);
PUBLIC void      ecEncodeName(EcCompiler *cp, EjsName qname);
PUBLIC void      ecEncodeMulti(EcCompiler *cp, cchar *str);
PUBLIC void      ecEncodeWideAsMulti(EcCompiler *cp, wchar *str);
PUBLIC void      ecEncodeOpcode(EcCompiler *cp, int value);

PUBLIC void     ecCopyCode(EcCompiler *cp, uchar *pos, int size, int dist);
PUBLIC uint     ecGetCodeOffset(EcCompiler *cp);
PUBLIC int      ecGetCodeLen(EcCompiler *cp, uchar *mark);
PUBLIC void     ecAdjustCodeLength(EcCompiler *cp, int adj);

#ifdef __cplusplus
}
#endif
#endif /* _h_EC_COMPILER */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

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
    Start of file "src/paks/pcre/pcre.h"
 */
/************************************************************************/

/*************************************************
*       Perl-Compatible Regular Expressions      *
*************************************************/

/* This is the public header file for the PCRE library, to be #included by
applications that call the PCRE functions.

           Copyright (c) 1997-2008 University of Cambridge

-----------------------------------------------------------------------------
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of the University of Cambridge nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
-----------------------------------------------------------------------------
*/

#ifndef _PCRE_H
#define _PCRE_H

#ifndef BIT_PACK_PCRE
    #define BIT_PACK_PCRE 1
#endif

/* The current PCRE version information. */

#define PCRE_MAJOR          7
#define PCRE_MINOR          7
#define PCRE_PRERELEASE     
#define PCRE_DATE           2008-05-07

/* When an application links to a PCRE DLL in Windows, the symbols that are
imported have to be identified as such. When building PCRE, the appropriate
export setting is defined in pcre_internal.h, which includes this file. So we
don't change existing definitions of PCRE_EXP_DECL and PCRECPP_EXP_DECL. */

/* EMBEDTHIS */
#ifndef _VSB_CONFIG_FILE
    #define _VSB_CONFIG_FILE "vsbConfig.h"

    /*
     *  When building all-in-one, we must use internal definitions
     */
    #ifndef PCRE_EXP_DECL
    #  ifdef _WIN32
    #    ifndef PCRE_STATIC
    #      define PCRE_EXP_DECL       extern __declspec(dllexport)
    #      define PCRE_EXP_DEFN       __declspec(dllexport)
    #      define PCRE_EXP_DATA_DEFN  __declspec(dllexport)
    #    else
    #      define PCRE_EXP_DECL       extern
    #      define PCRE_EXP_DEFN
    #      define PCRE_EXP_DATA_DEFN
    #    endif
    #  else
    #    ifdef __cplusplus
    #      define PCRE_EXP_DECL       extern "C"
    #    else
    #      define PCRE_EXP_DECL       extern
    #    endif
    #    ifndef PCRE_EXP_DEFN
    #      define PCRE_EXP_DEFN       PCRE_EXP_DECL
    #    endif
    #    ifndef PCRE_EXP_DATA_DEFN
    #      define PCRE_EXP_DATA_DEFN
    #    endif
    #  endif
    #endif
#else
    #if defined(_WIN32) && !defined(PCRE_STATIC)
    #  ifndef PCRE_EXP_DECL
    #    define PCRE_EXP_DECL  extern __declspec(dllimport)
    #  endif
    #  ifdef __cplusplus
    #    ifndef PCRECPP_EXP_DECL
    #      define PCRECPP_EXP_DECL  extern __declspec(dllimport)
    #    endif
    #    ifndef PCRECPP_EXP_DEFN
    #      define PCRECPP_EXP_DEFN  __declspec(dllimport)
    #    endif
    #  endif
    #endif
    
    /* By default, we use the standard "extern" declarations. */
    
    #ifndef PCRE_EXP_DECL
    #  ifdef __cplusplus
    #    define PCRE_EXP_DECL  extern "C"
    #  else
    #    define PCRE_EXP_DECL  extern
    #  endif
    #endif
    
    #ifdef __cplusplus
    #  ifndef PCRECPP_EXP_DECL
    #    define PCRECPP_EXP_DECL  extern
    #  endif
    #  ifndef PCRECPP_EXP_DEFN
    #    define PCRECPP_EXP_DEFN
    #  endif
    #endif
#endif

/* Have to include stdlib.h in order to ensure that size_t is defined;
it is needed here for malloc. */

#include <stdlib.h>

/* Allow for C++ users */

#ifdef __cplusplus
extern "C" {
#endif

/* Options */

#define PCRE_CASELESS           0x00000001
#define PCRE_MULTILINE          0x00000002
#define PCRE_DOTALL             0x00000004
#define PCRE_EXTENDED           0x00000008
#define PCRE_ANCHORED           0x00000010
#define PCRE_DOLLAR_ENDONLY     0x00000020
#define PCRE_EXTRA              0x00000040
#define PCRE_NOTBOL             0x00000080
#define PCRE_NOTEOL             0x00000100
#define PCRE_UNGREEDY           0x00000200
#define PCRE_NOTEMPTY           0x00000400
#define PCRE_UTF8               0x00000800
#define PCRE_NO_AUTO_CAPTURE    0x00001000
#define PCRE_NO_UTF8_CHECK      0x00002000
#define PCRE_AUTO_CALLOUT       0x00004000
#define PCRE_PARTIAL            0x00008000
#define PCRE_DFA_SHORTEST       0x00010000
#define PCRE_DFA_RESTART        0x00020000
#define PCRE_FIRSTLINE          0x00040000
#define PCRE_DUPNAMES           0x00080000
#define PCRE_NEWLINE_CR         0x00100000
#define PCRE_NEWLINE_LF         0x00200000
#define PCRE_NEWLINE_CRLF       0x00300000
#define PCRE_NEWLINE_ANY        0x00400000
#define PCRE_NEWLINE_ANYCRLF    0x00500000
#define PCRE_BSR_ANYCRLF        0x00800000
#define PCRE_BSR_UNICODE        0x01000000
#define PCRE_JAVASCRIPT_COMPAT  0x02000000

/* Exec-time and get/set-time error codes */

#define PCRE_ERROR_NOMATCH         (-1)
#define PCRE_ERROR_NULL            (-2)
#define PCRE_ERROR_BADOPTION       (-3)
#define PCRE_ERROR_BADMAGIC        (-4)
#define PCRE_ERROR_UNKNOWN_OPCODE  (-5)
#define PCRE_ERROR_UNKNOWN_NODE    (-5)  /* For backward compatibility */
#define PCRE_ERROR_NOMEMORY        (-6)
#define PCRE_ERROR_NOSUBSTRING     (-7)
#define PCRE_ERROR_MATCHLIMIT      (-8)
#define PCRE_ERROR_CALLOUT         (-9)  /* Never used by PCRE itself */
#define PCRE_ERROR_BADUTF8        (-10)
#define PCRE_ERROR_BADUTF8_OFFSET (-11)
#define PCRE_ERROR_PARTIAL        (-12)
#define PCRE_ERROR_BADPARTIAL     (-13)
#define PCRE_ERROR_INTERNAL       (-14)
#define PCRE_ERROR_BADCOUNT       (-15)
#define PCRE_ERROR_DFA_UITEM      (-16)
#define PCRE_ERROR_DFA_UCOND      (-17)
#define PCRE_ERROR_DFA_UMLIMIT    (-18)
#define PCRE_ERROR_DFA_WSSIZE     (-19)
#define PCRE_ERROR_DFA_RECURSE    (-20)
#define PCRE_ERROR_RECURSIONLIMIT (-21)
#define PCRE_ERROR_NULLWSLIMIT    (-22)  /* No longer actually used */
#define PCRE_ERROR_BADNEWLINE     (-23)

/* Request types for pcre_fullinfo() */

#define PCRE_INFO_OPTIONS            0
#define PCRE_INFO_SIZE               1
#define PCRE_INFO_CAPTURECOUNT       2
#define PCRE_INFO_BACKREFMAX         3
#define PCRE_INFO_FIRSTBYTE          4
#define PCRE_INFO_FIRSTCHAR          4  /* For backwards compatibility */
#define PCRE_INFO_FIRSTTABLE         5
#define PCRE_INFO_LASTLITERAL        6
#define PCRE_INFO_NAMEENTRYSIZE      7
#define PCRE_INFO_NAMECOUNT          8
#define PCRE_INFO_NAMETABLE          9
#define PCRE_INFO_STUDYSIZE         10
#define PCRE_INFO_DEFAULT_TABLES    11
#define PCRE_INFO_OKPARTIAL         12
#define PCRE_INFO_JCHANGED          13
#define PCRE_INFO_HASCRORLF         14

/* Request types for pcre_config(). Do not re-arrange, in order to remain
compatible. */

#define PCRE_CONFIG_UTF8                    0
#define PCRE_CONFIG_NEWLINE                 1
#define PCRE_CONFIG_LINK_SIZE               2
#define PCRE_CONFIG_POSIX_MALLOC_THRESHOLD  3
#define PCRE_CONFIG_MATCH_LIMIT             4
#define PCRE_CONFIG_STACKRECURSE            5
#define PCRE_CONFIG_UNICODE_PROPERTIES      6
#define PCRE_CONFIG_MATCH_LIMIT_RECURSION   7
#define PCRE_CONFIG_BSR                     8

/* Bit flags for the pcre_extra structure. Do not re-arrange or redefine
these bits, just add new ones on the end, in order to remain compatible. */

#define PCRE_EXTRA_STUDY_DATA             0x0001
#define PCRE_EXTRA_MATCH_LIMIT            0x0002
#define PCRE_EXTRA_CALLOUT_DATA           0x0004
#define PCRE_EXTRA_TABLES                 0x0008
#define PCRE_EXTRA_MATCH_LIMIT_RECURSION  0x0010

/* Types */

struct real_pcre;                 /* declaration; the definition is private  */
typedef struct real_pcre pcre;

/* When PCRE is compiled as a C++ library, the subject pointer type can be
replaced with a custom type. For conventional use, the public interface is a
const char *. */

#ifdef BIT_CHAR
#define PCRE_SPTR const BIT_CHAR *
#else
#ifndef PCRE_SPTR
#define PCRE_SPTR const char *
#endif
#endif

/* The structure for passing additional data to pcre_exec(). This is defined in
such as way as to be extensible. Always add new fields at the end, in order to
remain compatible. */

typedef struct pcre_extra {
  unsigned long int flags;        /* Bits for which fields are set */
  void *study_data;               /* Opaque data from pcre_study() */
  unsigned long int match_limit;  /* Maximum number of calls to match() */
  void *callout_data;             /* Data passed back in callouts */
  const unsigned char *tables;    /* Pointer to character tables */
  unsigned long int match_limit_recursion; /* Max recursive calls to match() */
} pcre_extra;

/* The structure for passing out data via the pcre_callout_function. We use a
structure so that new fields can be added on the end in future versions,
without changing the API of the function, thereby allowing old clients to work
without modification. */

typedef struct pcre_callout_block {
  int          version;           /* Identifies version of block */
  /* ------------------------ Version 0 ------------------------------- */
  int          callout_number;    /* Number compiled into pattern */
  int         *offset_vector;     /* The offset vector */
  PCRE_SPTR    subject;           /* The subject being matched */
  int          subject_length;    /* The length of the subject */
  int          start_match;       /* Offset to start of this match attempt */
  int          current_position;  /* Where we currently are in the subject */
  int          capture_top;       /* Max current capture */
  int          capture_last;      /* Most recently closed capture */
  void        *callout_data;      /* Data passed in with the call */
  /* ------------------- Added for Version 1 -------------------------- */
  int          pattern_position;  /* Offset to next item in the pattern */
  int          next_item_length;  /* Length of next item in the pattern */
  /* ------------------------------------------------------------------ */
} pcre_callout_block;

/* Indirection for store get and free functions. These can be set to
alternative malloc/free functions if required. Special ones are used in the
non-recursive case for "frames". There is also an optional callout function
that is triggered by the (?) regex item. For Virtual Pascal, these definitions
have to take another form. */

#ifndef VPCOMPAT
PCRE_EXP_DECL void *(*pcre_malloc)(size_t);
PCRE_EXP_DECL void  (*pcre_free)(void *);
PCRE_EXP_DECL void *(*pcre_stack_malloc)(size_t);
PCRE_EXP_DECL void  (*pcre_stack_free)(void *);
PCRE_EXP_DECL int   (*pcre_callout)(pcre_callout_block *);
#else   /* VPCOMPAT */
PCRE_EXP_DECL void *pcre_malloc(size_t);
PCRE_EXP_DECL void  pcre_free(void *);
PCRE_EXP_DECL void *pcre_stack_malloc(size_t);
PCRE_EXP_DECL void  pcre_stack_free(void *);
PCRE_EXP_DECL int   pcre_callout(pcre_callout_block *);
#endif  /* VPCOMPAT */

/* Exported PCRE functions */

PCRE_EXP_DECL pcre *pcre_compile(const char *, int, const char **, int *,
                  const unsigned char *);
PCRE_EXP_DECL pcre *pcre_compile2(const char *, int, int *, const char **,
                  int *, const unsigned char *);
PCRE_EXP_DECL int  pcre_config(int, void *);
PCRE_EXP_DECL int  pcre_copy_named_substring(const pcre *, const char *,
                  int *, int, const char *, char *, int);
PCRE_EXP_DECL int  pcre_copy_substring(const char *, int *, int, int, char *,
                  int);
PCRE_EXP_DECL int  pcre_dfa_exec(const pcre *, const pcre_extra *,
                  const char *, int, int, int, int *, int , int *, int);
PCRE_EXP_DECL int  pcre_exec(const pcre *, const pcre_extra *, PCRE_SPTR,
                   int, int, int, int *, int);
PCRE_EXP_DECL void pcre_free_substring(const char *);
PCRE_EXP_DECL void pcre_free_substring_list(const char **);
PCRE_EXP_DECL int  pcre_fullinfo(const pcre *, const pcre_extra *, int,
                  void *);
PCRE_EXP_DECL int  pcre_get_named_substring(const pcre *, const char *,
                  int *, int, const char *, const char **);
PCRE_EXP_DECL int  pcre_get_stringnumber(const pcre *, const char *);
PCRE_EXP_DECL int  pcre_get_stringtable_entries(const pcre *, const char *,
                  char **, char **);
PCRE_EXP_DECL int  pcre_get_substring(const char *, int *, int, int,
                  const char **);
PCRE_EXP_DECL int  pcre_get_substring_list(const char *, int *, int,
                  const char ***);
PCRE_EXP_DECL int  pcre_info(const pcre *, int *, int *);
PCRE_EXP_DECL const unsigned char *pcre_maketables(void);
PCRE_EXP_DECL int  pcre_refcount(pcre *, int);
PCRE_EXP_DECL pcre_extra *pcre_study(const pcre *, int, const char **);
PCRE_EXP_DECL const char *pcre_version(void);

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif /* End of pcre.h */
#endif /* BIT_PACK_EJSCRIPT */
