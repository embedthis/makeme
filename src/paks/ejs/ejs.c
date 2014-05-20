/*
    ejs.c -- Embedthis Ejscript Shell Command

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: orion
 */

#include "ejs.h"

/************************************************************************/
/*
    Start of file "src/cmd/ejs.c"
 */
/************************************************************************/

/**
    ejs.c - Ejscript shell

    Interactive shell that interprets interactive sessions and command files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/




#if ME_COMPILER_HAS_LIB_EDIT
  #include  <histedit.h>
#endif

/*********************************** Locals ***********************************/

typedef struct App {
    MprList     *files;
    MprList     *modules;
    Ejs         *ejs;
    EcCompiler  *compiler;
    char        *cygroot;
    int         iterations;
} App;

static App *app;

#if ME_COMPILER_HAS_LIB_EDIT
static History  *cmdHistory;
static EditLine *eh; 
static cchar    *prompt;
#endif

static int  consoleGets(EcStream *stream);
static int  commandGets(EcStream *stream);
static int  interpretCommands(EcCompiler*cp, cchar *cmd);
static int  interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method);
static void manageApp(App *app, int flags);
static void require(cchar *name);

/************************************ Code ************************************/

MAIN(ejsMain, int argc, char **argv, char **envp)
{
    Mpr             *mpr;
    EcCompiler      *cp;
    Ejs             *ejs;
    cchar           *cmd, *className, *method, *homeDir;
    char            *argp, *searchPath, *modules, *name, *tok, *extraFiles;
    int             nextArg, err, ecFlags, stats, merge, bind, noout, debug, optimizeLevel, warnLevel, strict, i, next;

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
    className = 0;
    cmd = 0;
    method = 0;
    searchPath = 0;
    stats = 0;
    merge = 0;
    bind = 1;
    noout = 1;
    debug = 1;
    warnLevel = 1;
    optimizeLevel = 9;
    strict = 0;
    app->files = mprCreateList(-1, 0);
    app->iterations = 1;
    argc = mpr->argc;
    argv = (char**) mpr->argv;

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--bind")) {
            bind = 1;

        } else if (smatch(argp, "--class")) {
            if (nextArg >= argc) {
                err++;
            } else {
                className = argv[++nextArg];
            }

        } else if (smatch(argp, "--chdir") || smatch(argp, "--home") || smatch(argp, "-C")) {
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = argv[++nextArg];
                if (chdir((char*) homeDir) < 0) {
                    mprError("Cannot change directory to %s", homeDir);
                }
            }

#if ME_UNIX_LIKE
        } else if (smatch(argp, "--chroot")) {
            /* Not documented or supported */
            if (nextArg >= argc) {
                err++;
            } else {
                homeDir = mprGetAbsPath(argv[++nextArg]);
                if (chroot(homeDir) < 0) {
                    if (errno == EPERM) {
                        mprEprintf("%s: Must be super user to use the --chroot option", mprGetAppName(mpr));
                    } else {
                        mprEprintf("%s: Cannot change change root directory to %s, errno %d",
                            mprGetAppName(), homeDir, errno);
                    }
                    return 4;
                }
            }
#endif

        } else if (smatch(argp, "--cmd") || smatch(argp, "-c")) {
            if (nextArg >= argc) {
                err++;
            } else {
                cmd = argv[++nextArg];
            }

#if ME_WIN_LIKE
        } else if (smatch(argp, "--cygroot")) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->cygroot = sclone(argv[++nextArg]);
            }
#endif
        } else if (smatch(argp, "--debug")) {
            debug = 1;

        } else if (smatch(argp, "--debugger") || smatch(argp, "-D")) {
            mprSetDebugMode(1);

        } else if (smatch(argp, "--files") || smatch(argp, "-f")) {
            /* Compatibility with mozilla shell */
            if (nextArg >= argc) {
                err++;
            } else {
                extraFiles = sclone(argv[++nextArg]);
                name = stok(extraFiles, " \t", &tok);
                while (name != NULL) {
                    mprAddItem(app->files, sclone(name));
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (smatch(argp, "--iterations") || smatch(argp, "-i")) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->iterations = atoi(argv[++nextArg]);
            }

        } else if (smatch(argp, "--log")) {
            if (nextArg >= argc) {
                err++;
            } else {
                mprStartLogging(argv[++nextArg], 0);
                mprSetCmdlineLogging(1);
            }

        } else if (smatch(argp, "--method")) {
            if (nextArg >= argc) {
                err++;
            } else {
                method = argv[++nextArg];
            }

        } else if (smatch(argp, "--name")) {
            /* Just ignore. Used to tag commands with a unique command line */ 
            nextArg++;

        } else if (smatch(argp, "--nobind")) {
            bind = 0;

        } else if (smatch(argp, "--nodebug")) {
            debug = 0;

        } else if (smatch(argp, "--optimize")) {
            if (nextArg >= argc) {
                err++;
            } else {
                optimizeLevel = atoi(argv[++nextArg]);
            }

        } else if (smatch(argp, "-s")) {
            /* Compatibility with mozilla shell. Just ignore */

        } else if (smatch(argp, "--search") || smatch(argp, "--searchpath")) {
            if (nextArg >= argc) {
                err++;
            } else {
                searchPath = argv[++nextArg];
            }

        } else if (smatch(argp, "--standard")) {
            strict = 0;

        } else if (smatch(argp, "--stats")) {
            stats = 1;

        } else if (smatch(argp, "--strict")) {
            strict = 1;

        } else if (smatch(argp, "--require")) {
            if (nextArg >= argc) {
                err++;
            } else {
                if (app->modules == 0) {
                    app->modules = mprCreateList(-1, 0);
                }
                modules = sclone(argv[++nextArg]);
                name = stok(modules, " \t", &tok);
                while (name != NULL) {
                    require(name);
                    name = stok(NULL, " \t", &tok);
                }
            }

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            mprStartLogging("stderr:2", 0);
            mprSetCmdlineLogging(1);

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            mprPrintf("%s\n", EJS_VERSION);
            return 0;

        } else if (smatch(argp, "--warn")) {
            if (nextArg >= argc) {
                err++;
            } else {
                warnLevel = atoi(argv[++nextArg]);
            }

        } else if (*argp == '-' && isdigit((uchar) argp[1])) {
            mprStartLogging(sfmt("stderr:%s", &argp[1]), 0);
            mprSetCmdlineLogging(1);

        } else {
            err++;
            break;
        }
    }

    if (err) {
        /*  
            If --method or --class is specified, then the named class.method will be run (method defaults to "main", class
            defaults to first class with a "main").

            Examples:
                ejs
                ejs script.es arg1 arg2 arg3
                ejs --class "Customer" --method "start" --files "script1.es script2.es" main.es
         */
        mprEprintf("Usage: %s [options] script.es [arguments] ...\n"
            "  Ejscript shell program options:\n"
            "  --class className        # Name of class containing method to run\n"
            "  --cmd ejscriptCode       # Literal ejscript statements to execute\n"
            "  --cygroot path           # Set cygwin root for resolving script paths\n"
            "  --debug                  # Use symbolic debugging information (default)\n"
            "  --debugger               # Disable timeouts to make using a debugger easier\n"
            "  --files \"files..\"        # Extra source to compile\n"
            "  --log logSpec            # Internal compiler diagnostics logging\n"
            "  --method methodName      # Name of method to run. Defaults to main\n"
            "  --nodebug                # Omit symbolic debugging information\n"
            "  --optimize level         # Set the optimization level (0-9 default is 9)\n"
            "  --require 'module,...'   # Required list of modules to pre-load\n"
            "  --search ejsPath         # Module search path\n"
            "  --standard               # Default compilation mode to standard (default)\n"
            "  --stats                  # Print memory stats on exit\n"
            "  --strict                 # Default compilation mode to strict\n"
            "  --verbose | -v           # Same as --log stderr:2 \n"
            "  --version                # Emit the compiler version information\n"
            "  --warn level             # Set the warning message level (0-9 default is 0)\n\n",
            mpr->name);
        return -1;
    }
    if ((ejs = ejsCreateVM(argc - nextArg, (cchar **) &argv[nextArg], 0)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprStartDispatcher(ejs->dispatcher);
    app->ejs = ejs;

    if (ejsLoadModules(ejs, searchPath, app->modules) < 0) {
        return MPR_ERR_CANT_READ;
    }
    mprGC(MPR_GC_FORCE);
    ecFlags = 0;
    ecFlags |= (merge) ? EC_FLAGS_MERGE: 0;
    ecFlags |= (bind) ? EC_FLAGS_BIND: 0;
    ecFlags |= (noout) ? EC_FLAGS_NO_OUT: 0;
    ecFlags |= (debug) ? EC_FLAGS_DEBUG: 0;

    cp = app->compiler = ecCreateCompiler(ejs, ecFlags);
    if (cp == 0) {
        return MPR_ERR_MEMORY;
    }
    ecSetRequire(cp, app->modules);

    ecSetOptimizeLevel(cp, optimizeLevel);
    ecSetWarnLevel(cp, warnLevel);
    ecSetStrictMode(cp, strict);
    if (nextArg < argc) {
        mprAddItem(app->files, sclone(argv[nextArg]));
    }
    if (app->cygroot) {
        /*  
            When cygwin invokes a script with shebang, it passes a cygwin path to the script
            The ejs --cygroot option permits ejscript to conver this to a native path
         */
        for (next = 0; (name = mprGetNextItem(app->files, &next)) != 0; ) {
            if (*name == '/' || *name == '\\') {
                mprSetItem(app->files, next - 1, sjoin(app->cygroot, name, NULL));
            }
        }
    }
    for (i = 0; !err && i < app->iterations; i++) {
        if (cmd) {
            if (interpretCommands(cp, cmd) < 0) {
                err++;
            }
        } else if (mprGetListLength(app->files) > 0) {
            if (interpretFiles(cp, app->files, argc - nextArg, &argv[nextArg], className, method) < 0) {
                err++;
            }
        } else {
            /*  
                No args - run as an interactive shell
             */
            if (interpretCommands(cp, NULL) < 0) {
                err++;
            }
        }
    }
    if (stats) {
#if ME_DEBUG
        mprSetLogLevel(1);
        mprPrintMem("Memory Usage", 1);
#endif
    }
    if (err) {
        mprSetExitStatus(err);
    }
    app->ejs = 0;
    app->compiler = 0;
    ejsDestroy(ejs);
    mprDestroy();
    return mprGetExitStatus();
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->files);
        mprMark(app->ejs);
        mprMark(app->compiler);
        mprMark(app->cygroot);
        mprMark(app->modules);
    }
}


/*  
    Compile the source files supplied on the command line. This will compile in-memory and optionally also save to 
    module files.
 */
static int interpretFiles(EcCompiler *cp, MprList *files, int argc, char **argv, cchar *className, cchar *method)
{
    Ejs     *ejs;

    assert(files);

    ejs = cp->ejs;
    if (ecCompile(cp, files->length, (char**) files->items) < 0) {
        mprRawLog(0, "%s\n", cp->errorMsg);
        return EJS_ERR;
    }
    if (cp->errorCount == 0) {
        if (ejsRunProgram(ejs, className, method) < 0) {
            ejsReportError(ejs, "Error in program");
            return EJS_ERR;
        }
    }
    return 0;
}


/*  
    Interpret from the console or from a literal command
 */
static int interpretCommands(EcCompiler *cp, cchar *cmd)
{
    Ejs         *ejs;
    EjsString   *result;
    char        *tmpArgv[1];
    int         err;

    ejs = cp->ejs;
    cp->interactive = 1;

    if (ecOpenConsoleStream(cp, (cmd) ? commandGets: consoleGets, cmd) < 0) {
        mprError("Cannot open input");
        return EJS_ERR;
    }
    tmpArgv[0] = EC_INPUT_STREAM;

    while (!cp->stream->eof && !mprIsStopping()) {
        err = 0;
        cp->uid = 0;
        ejs->result = ESV(undefined);
        if (ecCompile(cp, 1, tmpArgv) < 0) {
            mprRawLog(0, "%s", cp->errorMsg);
            ejs->result = ESV(undefined);
            err++;
        }
        if (!err && cp->errorCount == 0) {
            if (ejsRunProgram(ejs, NULL, NULL) < 0) {
                ejsReportError(ejs, "Error in script");
            }
        }
        if (!ejs->exception && ejs->result != ESV(undefined)) {
            if (ejsIs(ejs, ejs->result, Date) /* TODO || ejsIsType(ejs, ejs->result) */) {
                if ((result = (EjsString*) ejsToString(ejs, ejs->result)) != 0) {
                    mprPrintf("%@\n", result);
                }
            } else if (ejs->result != ESV(null)) {
                if ((result = (EjsString*) ejsSerialize(ejs, ejs->result, EJS_JSON_SHOW_PRETTY)) != 0) {
                    mprPrintf("%@\n", result);
                }
            }
        }
        ecResetInput(cp);
        cp->errorCount = 0;
        cp->fatalError = 0;
    }
    ecCloseStream(cp);
    return 0;
}


/************************************************* Command line History **************************************/
#if ME_COMPILER_HAS_LIB_EDIT

static cchar *issuePrompt(EditLine *e) 
{
    return prompt;
}


static EditLine *initEditLine()
{
    EditLine    *e;
    HistEvent   ev; 

    cmdHistory = history_init(); 
    history(cmdHistory, &ev, H_SETSIZE, 100); 
    e = el_init("ejs", stdin, stdout, stderr); 
    el_set(e, EL_EDITOR, "vi");
    el_set(e, EL_HIST, history, cmdHistory);
    el_source(e, NULL);
    return e;
}


/*  
    Prompt for input with the level of current nest (block nest depth)
 */
static char *readline(cchar *msg) 
{ 
    HistEvent   ev; 
    cchar       *str; 
    char        *result;
    ssize       len;
    int         count; 
 
    if (eh == NULL) { 
        eh = initEditLine();
    }
    prompt = msg;
    el_set(eh, EL_PROMPT, issuePrompt);
    el_set(eh, EL_SIGNAL, 1);
    str = el_gets(eh, &count); 
    if (str && count > 0) { 
        result = strdup(str); 
        len = slen(result);
        if (result[len - 1] == '\n') {
            result[len - 1] = '\0'; 
        }
        count = history(cmdHistory, &ev, H_ENTER, result); 
        return result; 
    }  
    return NULL; 
} 

#else /* ME_COMPILER_HAS_LIB_EDIT */

static char *readline(cchar *msg)
{
    char    buf[ME_MAX_PATH];

    printf("%s", msg);
    fflush(stdout);
    if (fgets(buf, sizeof(buf) - 1, stdin) == 0) {
        return NULL;
    }
    return strdup(buf);
}
#endif /* ME_COMPILER_HAS_LIB_EDIT */


/*  
    Read input from the console (stdin)
 */
static int consoleGets(EcStream *stream)
{
    char    prompt[ME_MAX_PATH], *line, *cp;
    int     level;

    if (stream->flags & EC_STREAM_EOL) {
        return 0;
    }
    level = stream->compiler->state ? stream->compiler->state->blockNestCount : 0;
    fmt(prompt, sizeof(prompt), "%s-%d> ", EJS_NAME, level);

    mprYield(MPR_YIELD_STICKY);
    line = readline(prompt);
    mprResetYield();
    if (line == NULL) {
        stream->eof = 1;
        mprPrintf("\n");
        return -1;
    }
    cp = strim(line, "\r\n", MPR_TRIM_BOTH);
    ecSetStreamBuf(stream, cp, slen(cp));
    stream->flags |= EC_STREAM_EOL;
    return (int) slen(cp);
}


/*  
    Read input from a literal command
 */
static int commandGets(EcStream *stream)
{
    /*  
        Execute one string of commands. So we only come here once. Second time round, nextChar will be set.
     */
    if (stream->nextChar) {
        stream->eof = 1;
        return -1;
    }
    assert(0);
    return (int) (stream->end - stream->nextChar);
}


static void require(cchar *name) 
{
    if (name && *name) {
        mprAddItem(app->modules, sclone(name));
    }
}

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
