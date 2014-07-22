/**
    testmeLib.c - MakeMe unit test library

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "testme.h"

/************************************ Code ************************************/

PUBLIC int tdepth()
{
    cchar   *value;

    if ((value = getenv("TM_DEPTH")) != 0) {
        return atoi(value);
    }
    return 0;
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


PUBLIC int tgeti(cchar *key, int def)
{
    cchar   *value;

    if ((value = getenv(key)) != 0) {
        return atoi(value);
    } else {
        return def;
    }
}


PUBLIC bool thas(cchar *key)
{
    return tgeti(key, 0);
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
#if ME_WIN_LIKE
    char    buf[ME_MAX_BUFFER];
    sprintf_s(buf, sizeof(buf), "%s=%s", key, value);
    putenv(buf);
#else
    setenv(key, value, 1);
#endif
    printf("set %s %s\n", key, value);
}


PUBLIC void tskip()
{
    printf("skip\n");
}


PUBLIC bool ttest(cchar *loc, cchar *expression, bool success)
{
    if (success) {
        printf("pass in %s for \"%s\"\n", loc, expression);
    } else {
        printf("fail in %s for \"%s\"\n", loc, expression);
    }
    return success;
}


PUBLIC void tverbose(cchar *fmt, ...)
{
    va_list     ap;
    char        buf[ME_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("verbose %s\n", buf);
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
