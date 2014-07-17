/*
    testme.h -- Header for the MakeMe unit test library

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_TESTME
#define _h_TESTME 1

/********************************** Includes **********************************/

#include    "osdep.h"

/*********************************** Defines **********************************/

#define TM_LINE(s)              #s
#define TM_LINE2(s)             TM_LINE(s)
#define TM_LINE3                TM_LINE2(__LINE__)
#define TM_LOC                  __FILE__ "@" TM_LINE3

#define ttrue(E)               ttest(TM_LOC, #E, E)
#define tfalse(E)              ttest(TM_LOC, #E, !(E))

PUBLIC bool ttest(cchar *loc, cchar *expression, bool success);
PUBLIC cchar *tget(cchar *key, cchar *def);
PUBLIC int tgetInt(cchar *key, int def);
PUBLIC void tinfo(cchar *fmt, ...);
PUBLIC void tset(cchar *key, cchar *value);
PUBLIC void tskip(bool skip);
PUBLIC void twrite(cchar *fmt, ...);

#ifdef __cplusplus
}
#endif
#endif /* _h_TESTME */

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
