/**
    testmeLib.c - MakeMe unit test library

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "testme.h"

/************************************ Code ************************************/

PUBLIC bool ttest(cchar *loc, cchar *expression, bool success)
{
    if (success) {
        printf("pass\n");
    } else {
        printf("fail in %s for \"%s\"\n", loc, expression);
    }
    return success;
}


PUBLIC void tinfo(cchar *fmt, ...)
{
    va_list     ap;
    char        buf[ME_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("info %s\n", buf);
}


PUBLIC void tset(cchar *key, cchar *value)
{
    setenv(key, value, 1);
    printf("set %s %s\n", key, value);
}


PUBLIC cchar *tget(cchar *key, cchar *def)
{
    cchar   *value;

    if ((value = getenv(key)) != 0) {
        return value;
    } else {
        return def;
    }
}


PUBLIC int tgetInt(cchar *key, int def)
{
    cchar   *value;

    if ((value = getenv(key)) != 0) {
        return atoi(value);
    } else {
        return def;
    }
}


PUBLIC void tskip(bool skip)
{
    if (skip) {
        printf("skip\n");
    }
}


PUBLIC void twrite(cchar *fmt, ...)
{
    va_list     ap;
    char        buf[ME_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("write %s\n", buf);
}


#ifdef __cplusplus
}
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
