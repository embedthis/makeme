/*
    testme.h -- Header for the MakeMe unit test library

    This file provides a simple API for writing unit tests.

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
#define tfail(E)               ttest(TM_LOC, "assertion failed " #E, 0)
#define ttrue(E)               ttest(TM_LOC, #E, (E) != 0)
#define teq(a, b)              ttestEquals(TM_LOC, #a " == " #b, a == b, (int) a, (int) b)
#define tcontains(s, p)        ttestContains(TM_LOC, #s " == " #p, (s && p && strstr(s, p) != 0), s, p)
#define tmatch(s, p)           ttestMatch(TM_LOC, #s " == " #p, ((s == NULL && p == NULL) || (smatch(s, p))), s, p)
#define tfalse(E)              ttest(TM_LOC, #E, (E) == 0)

#ifdef assert
    #undef assert
    #define assert(E)          ttest(TM_LOC, #E, (E) != 0)
#endif

/**
    Print a debug message.
    @param fmt The format string.
    @param ... The arguments to the format string.
 */
void tdebug(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("debug %s\n", buf);
}

/**
    Get the depth of the test.
    @return The depth of the test.
 */
int tdepth(void)
{
    const char   *value;

    if ((value = getenv("TM_DEPTH")) != 0) {
        return atoi(value);
    }
    return 0;
}


/**
    Get an environment variable.
    @param key The key to get.
    @param def The default value.
    @return The value of the environment variable.
 */
const char *tget(const char *key, const char *def)
{
    const char   *value;

    if ((value = getenv(key)) != 0) {
        return value;
    } else {
        return def;
    }
}


/**
    Get an environment variable as an integer.
    @param key The key to get.
    @param def The default value.
    @return The value of the environment variable.
 */
int tgeti(const char *key, int def)
{
    const char   *value;

    if ((value = getenv(key)) != 0) {
        return atoi(value);
    } else {
        return def;
    }
}

/**
    Check if an environment variable exists.
    @param key The key to check.
    @return 1 if the environment variable exists, 0 otherwise.
 */
int thas(const char *key)
{
    return tgeti(key, 0);
}

/**
    Print an info message.
    @param fmt The format string.
    @param ... The arguments to the format string.
 */
void tinfo(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("info %s\n", buf);
}

/**
    Set an environment variable.
    @param key The key to set.
    @param value The value to set.
 */
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

/**
    Skip a test.
    @param fmt The format string.
    @param ... The arguments to the format string.
 */
void tskip(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    printf("skip %s\n", buf);
}

/**
    Emit a pass/fail message based on the success of the test.
    @param loc The location of the test.
    @param expression The expression to test.
    @param success The success of the test.
    @return 1 if the test passed, 0 otherwise.
 */
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

/**
    Test if two integers are equal.
    @param loc The location of the test.
    @param expression The expression to test.
    @param success The success of the test.
    @param a The first integer.
    @param b The second integer.
    @return 1 if the test passed, 0 otherwise.
 */
int ttestEquals(const char *loc, const char *expression, int success, int a, int b)
{
    ttest(loc, expression, success);
    if (!success) {
        printf("Expected: %d\n", a);
        printf("Received: %d\n", b);
    }
    return success;
}

/**
    Test if a string contains a pattern.
    @param loc The location of the test.
    @param expression The expression to test.
    @param success The success of the test.
    @param str The string to test.
    @param pattern The pattern to test for.
    @return 1 if the test passed, 0 otherwise.
 */
int ttestContains(const char *loc, const char *expression, int success, const char *str, const char *pattern)
{
    ttest(loc, expression, success);
    if (!success) {
        printf("Expected: %s\n", pattern);
        printf("Received: %s\n", str);
    }
    return success;
}

/**
    Test if a string matches a pattern.
    @param loc The location of the test.
    @param expression The expression to test.
    @param success The success of the test.
    @param str The string to test.
    @param pattern The pattern to test for.
    @return 1 if the test passed, 0 otherwise.
 */
int ttestMatch(const char *loc, const char *expression, int success, const char *str, const char *pattern)
{
    ttest(loc, expression, success);
    if (!success) {
        printf("Expected: %s\n", pattern);
        printf("Received: %s\n", str);
    }
    return success;
}

/**
    Print a verbose message.
    @param fmt The format string.
    @param ... The arguments to the format string.
 */
void tverbose(const char *fmt, ...)
{
    va_list     ap;
    char        buf[TM_MAX_BUFFER];

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    printf("verbose %s\n", buf);
}

/**
    Write a message.
    @param fmt The format string.
    @param ... The arguments to the format string.
 */
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
    Copyright (c) Embedthis Software. All Rights Reserved.
    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.
 */

