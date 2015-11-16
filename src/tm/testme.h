/*
    testme.h -- Header for the MakeMe unit test library

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

#ifndef _h_TESTME
#define _h_TESTME 1

/*********************************** Includes *********************************/

#ifdef _WIN32
    #undef   _CRT_SECURE_NO_DEPRECATE
    #define  _CRT_SECURE_NO_DEPRECATE 1
    #undef   _CRT_SECURE_NO_WARNINGS
    #define  _CRT_SECURE_NO_WARNINGS 1
    #define  _WINSOCK_DEPRECATED_NO_WARNINGS 1
    #include <winsock2.h>
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/*********************************** Defines **********************************/

#ifdef __cplusplus
extern "C" {
#endif

#define TM_MAX_BUFFER           4096

#define TM_LINE(s)             #s
#define TM_LINE2(s)            TM_LINE(s)
#define TM_LINE3               TM_LINE2(__LINE__)
#define TM_LOC                 __FILE__ "@" TM_LINE3
#define TM_SHORT_NAP           (5 * 1000)

#define tassert(E)             ttest(TM_LOC, #E, (E) != 0)
#define tfail(E)               ttest(TM_LOC, "assertion failed", 0)
#define ttrue(E)               ttest(TM_LOC, #E, (E) != 0)
#define tfalse(E)              ttest(TM_LOC, #E, (E) == 0)

#ifdef assert
    #undef assert
    #define assert(E)          ttest(TM_LOC, #E, (E) != 0)
#endif

void tdebug(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("debug %s\n", buf);
}


int tdepth()
{
    const char   *value;

    if ((value = getenv("TM_DEPTH")) != 0) {
        return atoi(value);
    }
    return 0;
}


const char *tget(const char *key, const char *def)
{
    const char   *value;

    if ((value = getenv(key)) != 0) {
        return value;
    } else {
        return def;
    }
}


int tgeti(const char *key, int def)
{
    const char   *value;

    if ((value = getenv(key)) != 0) {
        return atoi(value);
    } else {
        return def;
    }
}


int thas(const char *key)
{
    return tgeti(key, 0);
}


void tinfo(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("info %s\n", buf);
}


void tset(const char *key, const char *value)
{
#if _WIN32
    char    buf[TM_MAX_BUFFER];
    sprintf_s(buf, sizeof(buf), "%s=%s", key, value);
    _putenv(buf);
#else
    setenv(key, value, 1);
#endif
    printf("set %s %s\n", key, value);
}


void tskip(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    printf("skip %s\n", buf);
}


int ttest(const char *loc, const char *expression, int success)
{
    if (success) {
        printf("pass in %s for \"%s\"\n", loc, expression);
    } else {
        printf("fail in %s for \"%s\"\n", loc, expression);
        if (getenv("TESTME_SLEEP")) {
#if _WIN32
            DebugBreak();
#else
            sleep(60);
#endif
        } else if (getenv("TESTME_STOP")) {
#if _WIN32
            DebugBreak();
#else
            abort();
#endif
        }
    }
    return success;
}


void tverbose(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("verbose %s\n", buf);
}


void twrite(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("write %s\n", buf);
}

#ifdef __cplusplus
}
#endif

#endif /* _h_TESTME */

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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

