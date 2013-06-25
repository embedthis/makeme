/**
    bit.c - Run the bit.es|bit.mod script.

    This program runs a script of the same name as this script.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "ejs.h"

/*********************************** Locals ***********************************/

typedef struct App {
    Ejs     *ejs;
    cchar   *script;
} App;

static App *app;

static cchar *findBitScript();
static void manageApp(App *app, int flags);

/************************************ Code ************************************/

MAIN(ejsMain, int argc, char **argv, char **envp)
{
    Mpr         *mpr;
    Ejs         *ejs;
    EcCompiler  *ec;
    char    *argp, *searchPath, *homeDir;
    int     nextArg, err, flags;

    /*  
        Initialize Multithreaded Portable Runtime (MPR)
     */
    mpr = mprCreate(argc, argv, 0);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddStandardSignals();

    if (mprStart(mpr) < 0) {
        mprError("Cannot start mpr services");
        return EJS_ERR;
    }
    err = 0;
    searchPath = 0;
    argc = mpr->argc;
    argv = (char**) mpr->argv;

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--chdir") || smatch(argp, "--home") || smatch(argp, "-C")) {
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
                if (chdir((char*) homeDir) < 0) {
                    mprError("Cannot change directory to %s", homeDir);
                }
            }

        } else if (smatch(argp, "--debugger") || smatch(argp, "-D")) {
            mprSetDebugMode(1);

        } else if (smatch(argp, "--log")) {
            if (nextArg >= argc) {
                err++;
            } else {
                mprStartLogging(argv[++nextArg], 0);
                mprSetCmdlineLogging(1);
            }

        } else if (smatch(argp, "--name")) {
            /* Just ignore. Used to tag commands with a unique command line */ 
            nextArg++;

        } else if (smatch(argp, "--search") || smatch(argp, "--searchpath")) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            mprStartLogging("stderr:1", 0);
            mprSetCmdlineLogging(1);

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            mprPrintf("%s-%s\n", BIT_VERSION, BIT_BUILD_NUMBER);
            return 0;

        } else {
            /* Ignore */
        }
    }
    app->script = findBitScript();
    mprLog(2, "Using bit script %s", app->script);

    argv[0] = (char*) app->script;
    if ((ejs = ejsCreateVM(argc, (cchar**) &argv[0], 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    app->ejs = ejs;
    if (ejsLoadModules(ejs, searchPath, NULL) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mprTrace(2, "Load script \"%s\"", app->script);
    flags = EC_FLAGS_BIND | EC_FLAGS_DEBUG | EC_FLAGS_NO_OUT | EC_FLAGS_THROW;
    if ((ec = ecCreateCompiler(ejs, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(ec);
    ecSetOptimizeLevel(ec, 9);
    ecSetWarnLevel(ec, 1);
    if (ecCompile(ec, 1, (char**) &app->script) < 0) {
        if (flags & EC_FLAGS_THROW) {
            ejsThrowSyntaxError(ejs, "%s", ec->errorMsg ? ec->errorMsg : "Cannot parse script");
            ejsReportError(ejs, "Error in script");
        }
        err = MPR_ERR;
    } else {
        mprRemoveRoot(ec);
        if (ejsRunProgram(ejs, NULL, NULL) < 0) {
            ejsReportError(ejs, "Error in script");
            err = MPR_ERR;
        }
    }
    if (!err) {
        err = mpr->exitStatus;
    }
    app->ejs = 0;
    mprTerminate(MPR_EXIT_DEFAULT, err);
    ejsDestroyVM(ejs);
    mprDestroy(MPR_EXIT_DEFAULT);
    return err;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->ejs);
        mprMark(app->script);
    }
}


static cchar *findBitScript()
{
    cchar    *path;

    path = "bits/bit.mod"; 
    if (mprPathExists(path, R_OK)) {
        return sclone(path);
    }
    path = mprJoinPath(mprGetAppDir(), "bits/bit.mod"); 
    if (mprPathExists(path, R_OK)) {
        return path;
    }
    path = "bits/bit.es"; 
    if (mprPathExists(path, R_OK)) {
        return sclone(path);
    }
    path = mprJoinPath(mprGetAppDir(), "bits/bit.es"); 
    if (mprPathExists(path, R_OK)) {
        return path;
    }
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
