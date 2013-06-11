/**
    manger.c -- Manager program 

    The manager watches over daemon programs.
    Key commands:
        uninstall   - Stop, disable then one time removal of configuration. 
        install     - Do one time installation configuration. Post-state: disabled.
        enable      - Enable service to run on reboot. Post-state: enabled. Does not start.
        disable     - Stop, then disable service to run on reboot. Post-state: disabled.
        stop        - Stop service. Post-state: stopped.
        start       - Start service. Post-state: running.
        run         - Run and watch over. Blocks.

    Idempotent. "appweb start" returns 0 if already started.

    Typical use:
        manager --program /usr/local/bin/ejs --args "/usr/src/farm/farm-client" --home /usr/src/farm/client \
            --pidfile /var/run/farm-client.pid run

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
/********************************* Includes ***********************************/

#include    "mpr.h"

#ifndef SERVICE_PROGRAM
    #define SERVICE_PROGRAM BIT_APP_PREFIX "/bin/" BIT_PRODUCT
#endif
#ifndef SERVICE_NAME
    #define SERVICE_NAME BIT_PRODUCT
#endif
#ifndef SERVICE_HOME
    #define SERVICE_HOME "/"
#endif

#if BIT_UNIX_LIKE
/*********************************** Locals ***********************************/

#define RESTART_DELAY (0 * 1000)        /* Default heart beat period (30 sec) */
#define RESTART_MAX   (100)             /* Max restarts per hour */
#define MANAGE_TIMEOUT (20 * 1000)      /* Timeout for actions */

typedef struct App {
    cchar   *appName;                   /* Manager name */
    int     continueOnErrors;           /* Keep going through errors */
    int     exiting;                    /* Program should exit */
    int     retries;                    /* Number of times to retry staring app */
    int     signal;                     /* Signal to use to terminate service */
    char    *command;                   /* Last command */
    char    *error;                     /* Command error message */
    char    *output;                    /* Command output message */
    char    *logSpec;                   /* Log directive for service */
    char    *pidDir;                    /* Location for pid file */
    char    *pidPath;                   /* Path to the manager pid for this service */
    int     restartCount;               /* Service restart count */
    int     restartWarned;              /* Has user been notified */
    int     runAsDaemon;                /* Run as a daemon */
    int     servicePid;                 /* Process ID for the service */
    char    *serviceArgs;               /* Args to pass to service */
    char    *serviceHome;               /* Service home */
    char    *serviceName;               /* Basename of service program */
    char    *serviceProgram;            /* Program to start */
    MprSignal *sigchld;                 /* Child death signal handler */
} App;

static App *app;

/***************************** Forward Declarations ***************************/

static void cleanup();
static bool killPid();
static int  makeDaemon();
static void manageApp(void *unused, int flags);
static int  readPid();
static bool process(cchar *operation, bool quiet);
static void runService();
static void setAppDefaults();
static void terminating(int how, int status);
static int  writePid(int pid);

/*********************************** Code *************************************/

PUBLIC int main(int argc, char *argv[])
{
    char    *argp, *value;
    int     err, nextArg, status;

    err = 0;
    mprCreate(argc, argv, MPR_USER_EVENTS_THREAD);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddTerminator(terminating);
    mprAddStandardSignals();
    setAppDefaults();

    for (nextArg = 1; nextArg < argc && !err; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (strcmp(argp, "--args") == 0) {
            /*
                Args to pass to service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /* Does nothing. Here for compatibility with windows watcher */

        } else if (strcmp(argp, "--continue") == 0) {
            app->continueOnErrors = 1;

        } else if (strcmp(argp, "--daemon") == 0) {
            app->runAsDaemon++;

#if FUTURE
        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
                Set the frequency to check on the program.
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->heartBeatPeriod = atoi(argv[++nextArg]);
            }
#endif

        } else if (strcmp(argp, "--home") == 0) {
            /*
                Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceHome = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--log") == 0) {
            /*
                Pass the log directive through to the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->logSpec = sclone(argv[++nextArg]);
                mprStartLogging(app->logSpec, 0);
                mprSetCmdlineLogging(1);
            }

        } else if (strcmp(argp, "--name") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceName = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--pidfile") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->pidPath = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceProgram = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--retries") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->retries = atoi(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--signal") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                value = argv[++nextArg];
                if (smatch(value, "SIGABRT")) {
                    app->signal = SIGABRT;
                } else if (smatch(value, "SIGINT")) {
                    app->signal = SIGINT;
                } else if (smatch(value, "SIGHUP")) {
                    app->signal = SIGHUP;
                } else if (smatch(value, "SIGQUIT")) {
                    app->signal = SIGQUIT;
                } else if (smatch(value, "SIGTERM")) {
                    app->signal = SIGTERM;
                } else if (smatch(value, "SIGUSR1")) {
                    app->signal = SIGUSR1;
                } else if (smatch(value, "SIGUSR2")) {
                    app->signal = SIGUSR2;
                } else { 
                    app->signal = atoi(argv[++nextArg]);
                }
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            mprSetLogLevel(1);

        } else {
            err++;
            break;
        }
    }
    if (nextArg >= argc) {
        err++;
    }
    if (err) {
        mprEprintf("Bad command line: \n"
            "  Usage: %s [commands]\n"
            "  Switches:\n"
            "    --args               # Args to pass to service\n"
            "    --continue           # Continue on errors\n"
            "    --daemon             # Run manager as a daemon\n"
            "    --home path          # Home directory for service\n"
            "    --log logFile:level  # Log directive for service\n"
            "    --retries count      # Max count of app restarts\n"
            "    --name name          # Name of the service to manage\n"
            "    --pidfile path       # Location of the pid file\n"
            "    --program path       # Service program to start\n"
            "    --signal signo       # Signal number to terminate service\n"
            "    --verbose            # Show command feedback\n"
#if FUTURE
            "    --heartBeat interval # Heart beat interval period (secs) \n"
#endif
            "  Commands:\n"
            "    disable              # Disable the service\n"
            "    enable               # Enable the service\n"
            "    install              # Install the service\n"
            "    run                  # Run and watch over the service\n"
            "    start                # Start the service\n"
            "    stop                 # Start the service\n"
            "    uninstall            # Uninstall the service\n"
            , app->appName);
        return -1;
    }

    if (!app->pidPath) {
        app->pidPath = sjoin(app->pidDir, "/", app->serviceName, ".pid", NULL);
    }
    if (app->runAsDaemon) {
        makeDaemon();
    }
    status = 0;
    if (getuid() != 0) {
        mprError("Must run with administrator privilege. Use sudo.");
        status = 1;                                                                    

    } else if (mprStart() < 0) {
        mprError("Cannot start MPR for %s", mprGetAppName());                                           
        status = 2;                                                                    

    } else {
        mprStartEventsThread();
        for (; nextArg < argc; nextArg++) {
            if (!process(argv[nextArg], 0) && !app->continueOnErrors) {
                status = 3;                                                                    
                break;
            }
        }
    }
    mprDestroy(MPR_EXIT_DEFAULT);                                                                      
    return status;                                                                    
}


static void manageApp(void *ptr, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->appName);
        mprMark(app->error);
        mprMark(app->output);
        mprMark(app->command);
        mprMark(app->logSpec);
        mprMark(app->pidDir);
        mprMark(app->pidPath);
        mprMark(app->serviceArgs);
        mprMark(app->serviceHome);
        mprMark(app->serviceName);
        mprMark(app->serviceProgram);
    }
}


static void setAppDefaults()
{
    app->appName = mprGetAppName();
    app->serviceProgram = sclone(SERVICE_PROGRAM);
    app->serviceName = sclone(SERVICE_NAME);
    app->serviceHome = mprGetNativePath(SERVICE_HOME);
    app->retries = RESTART_MAX;
    app->signal = SIGTERM;

    if (mprPathExists("/var/run", X_OK) && getuid() == 0) {
        app->pidDir = sclone("/var/run");
    } else if (mprPathExists("/tmp", X_OK)) {
        app->pidDir = sclone("/tmp");
    } else if (mprPathExists("/Temp", X_OK)) {
        app->pidDir = sclone("/Temp");
    } else {
        app->pidDir = sclone(".");
    }
}


static void terminating(int how, int status)
{
    cleanup();
}


static bool exists(cchar *fmt, ...)
{
    va_list     args;
    cchar       *path;
    bool        rc;

    va_start(args, fmt);
    path = sfmtv(fmt, args);
    rc = mprPathExists(path, F_OK);
    va_end(args);
    return rc;
}


static bool run(cchar *fmt, ...)
{
    va_list     args;
    MprCmd      *cmd;
    char        *out, *err;
    int         rc;

    va_start(args, fmt);
    app->command = sfmtv(fmt, args);
    mprLog(1, "Run: %s", app->command);
    cmd = mprCreateCmd(NULL);
    rc = mprRunCmd(cmd, app->command, NULL, &out, &err, MANAGE_TIMEOUT, 0);
    app->error = sclone(err);
    app->output = sclone(out);
    mprDestroyCmd(cmd);
    va_end(args);
    return (rc != 0) ? 0 : 1;
}


static bool process(cchar *operation, bool quiet)
{
    cchar   *name, *off, *path;
    int     rc, launch, update, service, upstart;

    /*
        No systemd support yet
     */
    rc = 1;
    name = app->serviceName;
    launch = upstart = update = service = 0;

    if (exists("/bin/launchctl") && exists("/Library/LaunchDaemons/com.%s.%s.plist", slower(BIT_COMPANY), name)) {
        launch++;

    } else if (exists("/sbin/start") && exists("/etc/init/rc.conf") &&
            (exists("/etc/init/%s.conf", name) || exists("/etc/init/%s.off", name))) {
        upstart++;

    } else if (exists("/usr/sbin/update-rc.d") && exists("/etc/init.d/%s", name)) {
        update++;

    } else if (exists("/sbin/service") && exists("/etc/init.d/%s", name)) {
        service++;

    } else {
        mprError("Cannot locate system tool to manage service");
        return 0;
    }

    /*
        Operations
     */
    if (smatch(operation, "install")) {
        if (launch) {
            ;

        } else if (service) {
            if (!run("/sbin/chkconfig --del %s", name)) {
                rc = 0;
            } else if (!run("/sbin/chkconfig --add %s", name)) {
                rc = 0;
            } else if (!run("/sbin/chkconfig --level 5 %s", name)) {
                rc = 0;
            }

        } else if (update) {
            ;

        } else if (upstart) {
            ;
        }

    } else if (smatch(operation, "uninstall")) {
        process("disable", 1);

        if (launch) {
            ;

        } else if (service) {
            rc = run("/sbin/chkconfig --del %s", name);

        } else if (update) {
            ;

        } else if (upstart) {
            ;
        }

    } else if (smatch(operation, "enable")) {
        /* 
            Enable service (will start on reboot)
         */
        if (launch) {
            path = sfmt("/Library/LaunchDaemons/com.%s.%s.plist", slower(BIT_COMPANY), name);
            /* 
                Unfortunately, there is no launchctl command to do an enable without starting. So must do a stop below.
             */
            if (!run("/bin/launchctl load -w %s", path)) {
                rc = 0;
            } else {
                rc = process("stop", 1);
            }

        } else if (update) {
            rc = run("/usr/sbin/update-rc.d %s defaults 90 10", name);
            /*
                Not supported on older versions
                rc = run("/usr/sbin/update-rc.d %s enable", name);
             */

        } else if (service) {
            rc = run("/sbin/chkconfig %s on", name);

        } else if (upstart) {
            off = sfmt("/etc/init/%s.off", name);
            if (exists(off) && !run("mv %s /etc/init/%s.conf", off, name)) {
                rc = 0;
            }
        }

    } else if (smatch(operation, "disable")) {
        process("stop", 1);
        if (launch) {
            rc = run("/bin/launchctl unload -w /Library/LaunchDaemons/com.%s.%s.plist", slower(BIT_COMPANY), name);

        } else if (update) {
            /*  
                Not supported on older versions
                rc = run("/usr/sbin/update-rc.d %s disable", name);
             */
            rc = run("/usr/sbin/update-rc.d -f %s remove", name);

        } else if (service) {
            rc = run("/sbin/chkconfig %s off", name);

        } else if (upstart) {
            if (exists("/etc/init/%s.conf", name)) {
                rc = run("mv /etc/init/%s.conf /etc/init/%s.off", name, name);
            }
        }

    } else if (smatch(operation, "start")) {
        if (launch) {
            rc = run("/bin/launchctl load /Library/LaunchDaemons/com.%s.%s.plist", slower(BIT_COMPANY), name);

        } else if (service) {
            rc = run("/sbin/service %s start", name);

        } else if (update) {
            rc = run("/usr/sbin/invoke-rc.d --quiet %s start", name);

        } else if (upstart) {
            rc = run("/sbin/start %s", name);
            if (!rc) {
                if (scontains(app->error, "start: Job is already running")) {
                    rc = 0;
                }
            }
        }

    } else if (smatch(operation, "stop")) {
        if (launch) {
            rc = run("/bin/launchctl unload /Library/LaunchDaemons/com.%s.%s.plist", slower(BIT_COMPANY), name);

        } else if (service) {
            if (!run("/sbin/service %s stop", name)) {
                rc = killPid();
            }

        } else if (update) {
            if (!run("/usr/sbin/invoke-rc.d --quiet %s stop", name)) {
                rc = killPid();
            }

        } else if (upstart) {
            if (exists("/etc/init/%s.conf", name)) {
                rc = run("/sbin/stop %s", name);
            }
        }

    } else if (smatch(operation, "reload")) {
        rc = process("restart", 0);

    } else if (smatch(operation, "restart")) {
        process("stop", 1);
        rc = process("start", 0);

    } else if (smatch(operation, "run")) {
        runService();
    }

    if (!quiet) {
        if (!rc && app->error && *app->error) {
            mprError("Cannot run command: %s\nCommand output: %s", app->command, app->error);
        }
        /* Logging at level one will be visible if appman -v is used */
        if (app->error && *app->error) {
            mprLog(1, "Error: %s", app->error); 
        }
        if (app->output && *app->output) {
            mprLog(1, "Output: %s", app->output); 
        }
    }
    return rc;
}


static void runService()
{
    MprTicks    mark;
    cchar       **av, **argv;
    char        *env[3];
    int         err, i, status, ac, next;

    app->servicePid = 0;
    atexit(cleanup);

    mprLog(1, "%s: Watching over %s", app->appName, app->serviceProgram);

    if (access(app->serviceProgram, X_OK) < 0) {
        mprError("start: can't access %s, errno %d", app->serviceProgram, mprGetOsError());
        return;
    }
    if (writePid(getpid()) < 0) {
        return;
    }
    mark = mprGetTicks();

    while (!mprIsStopping()) {
        if (mprGetElapsedTicks(mark) > (3600 * 1000)) {
            mark = mprGetTicks();
            app->restartCount = 0;
            app->restartWarned = 0;
        }
        if (app->servicePid == 0) {
            if (app->restartCount >= app->retries) {
                if (! app->restartWarned) {
                    mprError("Too many restarts for %s, %d in ths last hour", app->serviceProgram, app->restartCount);
                    mprError("Suspending restarts for one hour");
                    app->restartWarned++;
                }
                mprSleep(60 * 1000);
                continue;
            }

            /*
                Create the child
             */
            app->servicePid = vfork();
            if (app->servicePid < 0) {
                mprError("Cannot fork new process to run %s", app->serviceProgram);
                continue;

            } else if (app->servicePid == 0) {
                /*
                    Child
                 */
                umask(022);
                setsid();

                mprLog(1, "%s: Change dir to %s", app->appName, app->serviceHome);
                if (chdir(app->serviceHome) < 0) {}

                for (i = 3; i < 128; i++) {
                    close(i);
                }
                if (app->serviceArgs && *app->serviceArgs) {
                    ac = mprMakeArgv(app->serviceArgs, &av, 0);
                } else {
                    ac = 0;
                }
                argv = mprAlloc(sizeof(char*) * (6 + ac));
                env[0] = sjoin("LD_LIBRARY_PATH=", app->serviceHome, NULL);
                env[1] = sjoin("PATH=", getenv("PATH"), NULL);
                env[2] = 0;

                next = 0;
                argv[next++] = app->serviceProgram;
                if (app->logSpec) {
                    argv[next++] = "--log";
                    argv[next++] = (char*) app->logSpec;
                }
                for (i = 0; i < ac; i++) {
                    argv[next++] = av[i];
                }
                argv[next++] = 0;

                mprLog(1, "%s: Running %s", app->appName, app->serviceProgram);
                for (i = 1; argv[i]; i++) {
                    mprLog(1, "%s: argv[%d] = %s", app->appName, i, argv[i]);
                }
                execve(app->serviceProgram, (char**) argv, (char**) &env);

                /* Should not get here */
                err = errno;
                mprError("%s: Cannot exec %s, err %d, cwd %s", app->appName, app->serviceProgram, err, app->serviceHome);
                exit(MPR_ERR_CANT_INITIALIZE);
            }

            /*
                Parent
             */
            mprLog(1, "%s: create child %s at pid %d", app->appName, app->serviceProgram, app->servicePid);
            app->restartCount++;

            waitpid(app->servicePid, &status, 0);
            mprLog(1, "%s: %s has exited with status %d", app->appName, app->serviceProgram, WEXITSTATUS(status));
            if (!mprIsStopping()) {
                mprLog(1, "%s: Restarting %s (%d/%d)...", 
                    app->appName, app->serviceProgram, app->restartCount, app->retries);
            }
            app->servicePid = 0;
        }
    }
}


static void cleanup()
{
    if (app->servicePid > 0) {
        mprLog(1, "%s: Killing %s at pid %d with signal %d", app->appName, app->serviceProgram, 
            app->servicePid, app->signal);
        kill(app->servicePid, app->signal);
        app->servicePid = 0;
    }
}


/*
    Get the pid for the current running manager service
 */
static int readPid()
{
    char    pbuf[32];
    int     pid, fd;

    if ((fd = open(app->pidPath, O_RDONLY, 0666)) < 0) {
        return -1;
    }
    if (read(fd, pbuf, sizeof(pid)) <= 0) {
        close(fd);
        return -1;
    }
    close(fd);
    return (int) stoi(pbuf);
}


static bool killPid()
{
    int     pid;

    if ((pid = readPid()) > 1) {
        return kill(pid, SIGTERM) == 0;
    }
    return 0;
}


/*
    Write the pid so the manager and service can be killed via --stop
 */ 
static int writePid(int pid)
{
    char    *pbuf;
    ssize   len;
    int     fd;

    if ((fd = open(app->pidPath, O_CREAT | O_RDWR | O_TRUNC, 0666)) < 0) {
        mprError("Could not create pid file %s", app->pidPath);
        return MPR_ERR_CANT_CREATE;
    }
    pbuf = sfmt("%d\n", pid);
    len = slen(pbuf);
    if (write(fd, pbuf, len) != len) {
        mprError("Write to file %s failed", app->pidPath);
        return MPR_ERR_CANT_WRITE;
    }
    close(fd);
    return 0;
}


/*
    Convert this Manager to a Deaemon
 */
static int makeDaemon()
{
    struct sigaction    act, old;
    int                 i, pid, status;

    /*
        Ignore child death signals
     */
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = (void (*)(int, siginfo_t*, void*)) SIG_DFL;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_NOCLDSTOP | SA_RESTART | SA_SIGINFO;

    if (sigaction(SIGCHLD, &act, &old) < 0) {
        mprError("Cannot initialize signals");
        return MPR_ERR_BAD_STATE;
    }
    /*
        Close stdio so shells won't hang
     */
    for (i = 0; i < 3; i++) {
        close(i);
    }
    /*
        Fork twice to get a free child with no parent
     */
    if ((pid = fork()) < 0) {
        mprError("Fork failed for background operation");
        return MPR_ERR;

    } else if (pid == 0) {
        if ((pid = fork()) < 0) {
            mprError("Second fork failed");
            exit(127);

        } else if (pid > 0) {
            /* Parent of second child -- must exit */
            exit(0);
        }

        /*
            This is the real child that will continue as a daemon
         */
        setsid();
        if (sigaction(SIGCHLD, &old, 0) < 0) {
            mprError("Cannot restore signals");
            return MPR_ERR_BAD_STATE;
        }
        mprLog(2, "Switching to background operation");
        return 0;
    }

    /*
        Original process waits for first child here. Must get child death notification with a successful exit status
     */
    while (waitpid(pid, &status, 0) != pid) {
        if (errno == EINTR) {
            mprSleep(100);
            continue;
        }
        mprError("Cannot wait for daemon parent.");
        exit(0);
    }
    if (WEXITSTATUS(status) != 0) {
        mprError("Daemon parent had bad exit status.");
        exit(0);
    }
    if (sigaction(SIGCHLD, &old, 0) < 0) {
        mprError("Cannot restore signals");
        return MPR_ERR_BAD_STATE;
    }
    exit(0);
}


#elif BIT_WIN_LIKE
/*********************************** Locals ***********************************/

#define HEART_BEAT_PERIOD   (10 * 1000) /* Default heart beat period (10 sec) */
#define RESTART_MAX         (15)        /* Max restarts per hour */
#define SERVICE_DESCRIPTION ("Manages " BIT_TITLE)

typedef struct App {
    HWND         hwnd;               /* Application window handle */
    HINSTANCE    appInst;            /* Current application instance */
    cchar        *appName;           /* Manager name */
    cchar        *appTitle;          /* Manager title */
    int          continueOnErrors;   /* Keep going through errors */
    int          createConsole;      /* Display service console */
    int          exiting;            /* Program should exit */
    char         *logSpec;           /* Log directive for service */
    int          heartBeatPeriod;    /* Service heart beat interval */
    HANDLE       heartBeatEvent;     /* Heart beat event event to sleep on */
    HWND         otherHwnd;          /* Existing instance window handle */
    int          restartCount;       /* Service restart count */
    int          restartWarned;      /* Has user been notified */
    char         *serviceArgs;       /* Args to pass to service */
    cchar        *serviceHome;       /* Service home */
    cchar        *serviceName;       /* Service name */
    char         *serviceProgram;    /* Program to run */
    int          servicePid;         /* Process ID for the service */
    cchar        *serviceTitle;      /* Application title */
    HANDLE       serviceThreadEvent; /* Service event to block on */
    int          serviceStopped;     /* Service stopped */
    HANDLE       threadHandle;       /* Handle for the service thread */
} App;

static App *app;
static Mpr *mpr;

static SERVICE_STATUS           svcStatus;
static SERVICE_STATUS_HANDLE    svcHandle;
static SERVICE_TABLE_ENTRY      svcTable[] = {
    { UT("default"), 0   },
    { 0,             0   }
};

static void     WINAPI serviceCallback(ulong code);
static bool     enableService(int enable);
static void     setAppDefaults();
static bool     installService();
static void     logHandler(int flags, int level, cchar *msg);
static int      registerService();
static bool     removeService(int removeFromScmDb);
static void     gracefulShutdown(MprTicks timeout);
static bool     process(cchar *operation);
static void     run();
static int      startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain);
static bool     startService();
static bool     stopService(int cmd);
static int      tellSCM(long state, long exitCode, long wait);
static void     terminating(int how, int status);
static void     updateStatus(int status, int exitCode);
static void     writeToOsLog(cchar *message);

/***************************** Forward Declarations ***************************/

static void     manageApp(void *unused, int flags);
static LRESULT  msgProc(HWND hwnd, uint msg, uint wp, long lp);

static void     serviceThread(void *data);
static void WINAPI serviceMain(ulong argc, wchar **argv);

/*********************************** Code *************************************/

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE junk, char *args, int junk2)
{
    char    **argv, *argp;
    int     argc, err, nextArg;

    mpr = mprCreate(0, NULL, MPR_USER_EVENTS_THREAD);
    app = mprAllocObj(App, manageApp);
    mprAddRoot(app);
    mprAddTerminator(terminating);

    err = 0;
    app->appInst = inst;
    app->heartBeatPeriod = HEART_BEAT_PERIOD;

    setAppDefaults();

    mprSetAppName(BIT_PRODUCT "Manager", BIT_TITLE " Manager", BIT_VERSION);
    app->appName = mprGetAppName();
    app->appTitle = mprGetAppTitle(mpr);
    mprSetLogHandler(logHandler);
    mprSetWinMsgCallback((MprMsgCallback) msgProc);

    if ((argc = mprMakeArgv(args, &argv, MPR_ARGV_ARGS_ONLY)) < 0) {
        return MPR_ERR_BAD_ARGS;
    }
    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-' || strcmp(argp, "--") == 0) {
            break;
        }
        if (strcmp(argp, "--args") == 0) {
            /*
                Args to pass to service when it starts
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceArgs = argv[++nextArg];
            }

        } else if (strcmp(argp, "--console") == 0) {
            /*
                Allow the service to interact with the console
             */
            app->createConsole++;
 
        } else if (strcmp(argp, "--continue") == 0) {
            app->continueOnErrors = 1;

        } else if (strcmp(argp, "--daemon") == 0) {
            /* Ignored on windows */

        } else if (strcmp(argp, "--heartBeat") == 0) {
            /*
                Set the heart beat period
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->heartBeatPeriod = atoi(argv[++nextArg]) * 1000;
            }

        } else if (strcmp(argp, "--home") == 0) {
            /*
                Change to this directory before starting the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceHome = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--log") == 0) {
            /*
                Pass the log directive through to the service
             */
            if (nextArg >= argc) {
                err++;
            } else {
                app->logSpec = sclone(argv[++nextArg]);
                mprStartLogging(app->logSpec, 0);
                mprSetCmdlineLogging(1);
            }

        } else if (strcmp(argp, "--name") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceName = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--program") == 0) {
            if (nextArg >= argc) {
                err++;
            } else {
                app->serviceProgram = sclone(argv[++nextArg]);
            }

        } else if (strcmp(argp, "--verbose") == 0 || strcmp(argp, "-v") == 0) {
            mprSetLogLevel(1);

        } else {
            err++;
        }
        if (err) {
            mprEprintf("Bad command line: %s\n"
                "  Usage: %s [options] [program args]\n"
                "  Switches:\n"
                "    --args               # Args to pass to service\n"
                "    --continue           # Continue on errors\n"
                "    --console            # Display the service console\n"
                "    --heartBeat interval # Heart beat interval period (secs)\n"
                "    --home path          # Home directory for service\n"
                "    --log logFile:level  # Log directive for service\n"
                "    --name name          # Name of the service to manage\n"
                "    --program path       # Service program to start\n"
                "    --verbose            # Show command feedback\n"
                "  Commands:\n"
                "    disable              # Disable the service\n"
                "    enable               # Enable the service\n"
                "    start                # Start the service\n"
                "    stop                 # Start the service\n"
                "    run                  # Run and watch over the service\n",
                args, app->appName);
            return MPR_ERR_BAD_ARGS;
        }
    }
    if (mprStart() < 0) {
        mprError("Cannot start MPR for %s", mprGetAppName());                                           
    } else {
        mprStartEventsThread();
        if (nextArg >= argc) {
            if (!process("run")) {
                return MPR_ERR_CANT_COMPLETE;
            }

        } else for (; nextArg < argc; nextArg++) {
            if (!process(argv[nextArg]) && !app->continueOnErrors) {
                return MPR_ERR_CANT_COMPLETE;
            }
        }
    }
    return 0;
}


static void manageApp(void *ptr, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->appName);
        mprMark(app->appTitle);
        mprMark(app->serviceHome);
        mprMark(app->serviceName);
        mprMark(app->serviceProgram);
        mprMark(app->serviceTitle);
        mprMark(app->serviceArgs);
        mprMark(app->logSpec);
    }
}


static bool process(cchar *operation)
{
    bool    rc;

    rc = 1;

    if (smatch(operation, "install")) {
        rc = installService(app->serviceArgs);

    } else if (smatch(operation, "uninstall")) {
        rc = removeService(1);

    } else if (smatch(operation, "enable")) {
        rc = enableService(1);

    } else if (smatch(operation, "disable")) {
        rc = enableService(0);

    } else if (smatch(operation, "start")) {
        rc = startService();

    } else if (smatch(operation, "stop")) {
        rc = removeService(0);

    } else if (smatch(operation, "reload")) {
        rc = process("restart");

    } else if (smatch(operation, "restart")) {
        process("stop");
        return process("start");

    } else if (smatch(operation, "run")) {
        /*
            This thread will block if being started by SCM. While blocked, the serviceMain 
            will be called which becomes the effective main program. 
         */
        startDispatcher(serviceMain);
    }
    return rc;
}


/*
    Secondary entry point when started by the service control manager. Remember 
    that the main program thread is blocked in the startDispatcher called from
    winMain and will be used on callbacks from WinService.
 */ 
static void WINAPI serviceMain(ulong argc, char **argv)
{
    int     threadId;

    mprLog(1, "%s: Watching over %s", app->appName, app->serviceProgram);

    app->serviceThreadEvent = CreateEvent(0, TRUE, FALSE, 0);
    app->heartBeatEvent = CreateEvent(0, TRUE, FALSE, 0);

    if (app->serviceThreadEvent == 0 || app->heartBeatEvent == 0) {
        mprError("Cannot create wait events");
        return;
    }
    app->threadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE) serviceThread, (void*) 0, 0, (ulong*) &threadId);

    if (app->threadHandle == 0) {
        mprError("Cannot create service thread");
        return;
    }
    WaitForSingleObject(app->serviceThreadEvent, INFINITE);
    CloseHandle(app->serviceThreadEvent);

    /*
        We are now exiting, so wakeup the heart beat thread
     */
    app->exiting = 1;
    SetEvent(app->heartBeatEvent);
    CloseHandle(app->heartBeatEvent);
}


static void serviceThread(void *data)
{
    if (registerService() < 0) {
        mprError("Cannot register service");
        ExitThread(0);
        return;
    }
    updateStatus(SERVICE_RUNNING, 0);

    /*
        Start the service and watch over it.
     */
    run();
    updateStatus(SERVICE_STOPPED, 0);
    ExitThread(0);
}


static void run()
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFO         startInfo;
    MprTicks            mark;
    ulong               status;
    char                *path, *cmd, *key;
    int                 createFlags;

    createFlags = 0;

#if USEFUL_FOR_DEBUG
    /* 
        This is useful to debug manager as a windows service. Enable this and then Watson will prompt to attach
        when the service is run. Must have run prior "manager install enable"
     */
    DebugBreak();
#endif
    /*
        Read the service home directory and args. Default to the current dir if none specified.
     */
    key = sfmt("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", app->serviceName);
    app->serviceHome = mprReadRegistry(key, "HomeDir");
    app->serviceArgs = mprReadRegistry(key, "Args");

    /*
        Expect to find the service executable in the same directory as this manager program.
     */
    if (app->serviceProgram == 0) {
        path = sfmt("\"%s\\%s.exe\"", mprGetAppDir(), BIT_PRODUCT);
    } else {
        path = sfmt("\"%s\"", app->serviceProgram);
    }
    if (app->serviceArgs && *app->serviceArgs) {
        cmd = sfmt("%s %s", path, app->serviceArgs);
    } else {
        cmd = path;
    }
    if (app->createConsole) {
        createFlags |= CREATE_NEW_CONSOLE;
    }
    mark = mprGetTicks();

    while (! app->exiting) {
        if (mprGetElapsedTicks(mark) > (3600 * 1000)) {
            mark = mprGetTicks();
            app->restartCount = 0;
            app->restartWarned = 0;
        }
        if (app->servicePid == 0 && !app->serviceStopped) {
            if (app->restartCount >= RESTART_MAX) {
                if (! app->restartWarned) {
                    mprError("Too many restarts for %s, %d in ths last hour", app->appTitle, app->restartCount);
                    app->restartWarned++;
                }
                /*
                    This is not a real heart-beat. We are only waiting till the service process exits.
                 */
                WaitForSingleObject(app->heartBeatEvent, app->heartBeatPeriod);
                continue;
            }
            memset(&startInfo, 0, sizeof(startInfo));
            startInfo.cb = sizeof(startInfo);

            /*
                Launch the process
             */
            if (! CreateProcess(0, cmd, 0, 0, FALSE, createFlags, 0, app->serviceHome, &startInfo, &procInfo)) {
                mprError("Cannot create process: %s, %d", cmd, mprGetOsError());
            } else {
                app->servicePid = (int) procInfo.hProcess;
            }
            app->restartCount++;
        }
        WaitForSingleObject(app->heartBeatEvent, app->heartBeatPeriod);

        if (app->servicePid) {
            if (GetExitCodeProcess((HANDLE) app->servicePid, (ulong*) &status)) {
                if (status != STILL_ACTIVE) {
                    CloseHandle((HANDLE) app->servicePid);
                    app->servicePid = 0;
                }
            } else {
                CloseHandle((HANDLE) app->servicePid);
                app->servicePid = 0;
            }
        }
        mprLog(1, "%s has exited with status %d", app->serviceProgram, status);
        mprLog(1, "%s will be restarted in 10 seconds", app->serviceProgram);
    }
}


static int startDispatcher(LPSERVICE_MAIN_FUNCTION svcMain)
{
    SC_HANDLE       mgr;
    char            name[80];
    ulong           len;

    if (!(mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
        mprError("Cannot open service manager");
        return MPR_ERR_CANT_OPEN;
    }
    /*
        Is the service installed?
     */
    len = sizeof(name);
    if (GetServiceDisplayName(mgr, app->serviceName, name, &len) == 0) {
        CloseServiceHandle(mgr);
        return MPR_ERR_CANT_READ;
    }
    /*
        Register this service with the SCM. This call will block and consume the main thread if the service is 
        installed and the app was started by the SCM. If started manually, this routine will return 0.
     */
    svcTable[0].lpServiceProc = svcMain;
    if (StartServiceCtrlDispatcher(svcTable) == 0) {
        mprError("Could not start the service control dispatcher: 0x%x", GetLastError());
        return MPR_ERR_CANT_INITIALIZE;
    }
    return 0;
}


/*
    Should be called first thing after the service entry point is called by the service manager
 */

static int registerService()
{
    svcHandle = RegisterServiceCtrlHandler(app->serviceName, serviceCallback);
    if (svcHandle == 0) {
        mprError("Cannot register handler: 0x%x", GetLastError());
        return MPR_ERR_CANT_INITIALIZE;
    }
    /*
        Report the svcStatus to the service control manager.
     */
    svcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    svcStatus.dwServiceSpecificExitCode = 0;

    if (!tellSCM(SERVICE_START_PENDING, NO_ERROR, 1000)) {
        tellSCM(SERVICE_STOPPED, 0, 0);
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


static void updateStatus(int status, int exitCode)
{
    tellSCM(status, exitCode, 10000);
}


/*
    Service callback. Invoked by the SCM.
 */ 
static void WINAPI serviceCallback(ulong cmd)
{
    switch(cmd) {
    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_PAUSE:
        app->serviceStopped = 1;
        SuspendThread(app->threadHandle);
        svcStatus.dwCurrentState = SERVICE_PAUSED;
        break;

    case SERVICE_CONTROL_STOP:
        stopService(SERVICE_CONTROL_STOP);
        break;

    case SERVICE_CONTROL_CONTINUE:
        app->serviceStopped = 0;
        ResumeThread(app->threadHandle);
        svcStatus.dwCurrentState = SERVICE_RUNNING;
        break;

    case SERVICE_CONTROL_SHUTDOWN:
        stopService(SERVICE_CONTROL_SHUTDOWN);
        return;

    default:
        break;
    }
    tellSCM(svcStatus.dwCurrentState, NO_ERROR, 0);
}


static bool installService()
{
    SC_HANDLE   svc, mgr;
    char        cmd[BIT_MAX_FNAME], key[BIT_MAX_FNAME];
    int         serviceType;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Cannot open service manager");
        return 0;
    }
    /*
        Install this app as a service
     */
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (svc == NULL) {
        serviceType = SERVICE_WIN32_OWN_PROCESS;
        if (app->createConsole) {
            serviceType |= SERVICE_INTERACTIVE_PROCESS;
        }
        GetModuleFileName(0, cmd, sizeof(cmd));
        svc = CreateService(mgr, app->serviceName, app->serviceTitle, SERVICE_ALL_ACCESS, serviceType, SERVICE_DISABLED, 
            SERVICE_ERROR_NORMAL, cmd, NULL, NULL, "", NULL, NULL);
        if (! svc) {
            mprError("Cannot create service: 0x%x == %d", GetLastError(), GetLastError());
            CloseServiceHandle(mgr);
            return 0;
        }
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);

    /*
        Write a service description
     */
    fmt(key, sizeof(key), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services");
    if (mprWriteRegistry(key, NULL, app->serviceName) < 0) {
        mprError("Cannot write %s key to registry");
        return 0;
    }
    fmt(key, sizeof(key), "HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", app->serviceName);
    if (mprWriteRegistry(key, "Description", SERVICE_DESCRIPTION) < 0) {
        mprError("Cannot write service Description key to registry");
        return 0;
    }

    /*
        Write the home directory
     */
    if (app->serviceHome == 0) {
        app->serviceHome = mprGetPathParent(mprGetAppDir());
    }
    if (mprWriteRegistry(key, "HomeDir", app->serviceHome) < 0) {
        mprError("Cannot write HomeDir key to registry");
        return 0;
    }

    /*
        Write the service args
     */
    if (app->serviceArgs && *app->serviceArgs) {
        if (mprWriteRegistry(key, "Args", app->serviceArgs) < 0) {
            mprError("Cannot write Args key to registry");
            return 0;
        }
    }
    return 1;
}


/*
    Remove the application service
 */ 
static bool removeService(int removeFromScmDb)
{
    SC_HANDLE   svc, mgr;

    app->exiting = 1;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Cannot open service manager");
        return 0;
    }
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        CloseServiceHandle(mgr);
        mprError("Cannot open service");
        return 0;
    }
    gracefulShutdown(0);

    if (ControlService(svc, SERVICE_CONTROL_STOP, &svcStatus)) {
        mprSleep(500);

        while (QueryServiceStatus(svc, &svcStatus)) {
            if (svcStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                mprSleep(250);
            } else {
                break;
            }
        }
        if (svcStatus.dwCurrentState != SERVICE_STOPPED) {
            mprError("Cannot stop service: 0x%x", GetLastError());
        }
    }
    if (removeFromScmDb && !DeleteService(svc)) {
        if (GetLastError() != ERROR_SERVICE_MARKED_FOR_DELETE) {
            mprError("Cannot delete service: 0x%x", GetLastError());
        }
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return 1;
}


static bool enableService(int enable)
{
    SC_HANDLE   svc, mgr;
    int         flag;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Cannot open service manager");
        return 0;
    }
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (svc == NULL) {
        if (enable) {
            mprError("Cannot access service");
        }
        CloseServiceHandle(mgr);
        return 0;
    }
    flag = (enable) ? SERVICE_AUTO_START : SERVICE_DISABLED;
    if (!ChangeServiceConfig(svc, SERVICE_NO_CHANGE, flag, SERVICE_NO_CHANGE, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
        mprError("Cannot change service: 0x%x == %d", GetLastError(), GetLastError());
        CloseServiceHandle(svc);
        CloseServiceHandle(mgr);
        return 0;
    }
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);
    return 1;
}


static bool startService()
{
    SC_HANDLE   svc, mgr;
    int         rc;

    app->exiting = 0;

    mgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (! mgr) {
        mprError("Cannot open service manager");
        return 0;
    }
    svc = OpenService(mgr, app->serviceName, SERVICE_ALL_ACCESS);
    if (! svc) {
        mprError("Cannot open service");
        CloseServiceHandle(mgr);
        return 0;
    }
    rc = StartService(svc, 0, NULL);
    CloseServiceHandle(svc);
    CloseServiceHandle(mgr);

    if (rc == 0) {
        mprError("Cannot start %s service: 0x%x", app->serviceName, GetLastError());
        return 0;
    }
    return 1;
}


static bool stopService(int cmd)
{
    int     exitCode;

    app->exiting = 1;
    app->serviceStopped = 1;

    gracefulShutdown(10 * 1000);
    if (cmd == SERVICE_CONTROL_SHUTDOWN) {
        return 1;
    }

    /*
        Wakeup service event. This will make it exit.
     */
    SetEvent(app->serviceThreadEvent);

    svcStatus.dwCurrentState = SERVICE_STOP_PENDING;
    tellSCM(svcStatus.dwCurrentState, NO_ERROR, 1000);

    exitCode = 0;
    GetExitCodeThread(app->threadHandle, (ulong*) &exitCode);

    while (exitCode == STILL_ACTIVE) {
        GetExitCodeThread(app->threadHandle, (ulong*) &exitCode);
        mprSleep(100);
        tellSCM(svcStatus.dwCurrentState, NO_ERROR, 125);
    }
    svcStatus.dwCurrentState = SERVICE_STOPPED;
    tellSCM(svcStatus.dwCurrentState, exitCode, 0);
    return 1;
}


/*
    Tell SCM our current status
 */ 
static int tellSCM(long state, long exitCode, long wait)
{
    static ulong generation = 1;

    svcStatus.dwWaitHint = wait;
    svcStatus.dwCurrentState = state;
    svcStatus.dwWin32ExitCode = exitCode;

    if (state == SERVICE_START_PENDING) {
        svcStatus.dwControlsAccepted = 0;
    } else {
        svcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
    }
    if ((state == SERVICE_RUNNING) || (state == SERVICE_STOPPED)) {
        svcStatus.dwCheckPoint = 0;
    } else {
        svcStatus.dwCheckPoint = generation++;
    }

    /*
        Report the svcStatus of the service to the service control manager
     */
    return SetServiceStatus(svcHandle, &svcStatus);
}


static void setAppDefaults()
{
    app->appName = mprGetAppName();
    app->serviceProgram = sjoin(mprGetAppDir(), "\\", BIT_PRODUCT, ".exe", NULL);
    app->serviceName = sclone(BIT_COMPANY "-" BIT_PRODUCT);
    app->serviceHome = NULL;
    app->serviceTitle = sclone(BIT_TITLE);
    app->serviceStopped = 0;
}

/*
    Windows message processing loop
 */
static LRESULT msgProc(HWND hwnd, uint msg, uint wp, long lp)
{
    switch (msg) {
    case WM_DESTROY:
        break;

    case WM_QUIT:
        gracefulShutdown(0);
        break;
    
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


/*
    Default log output is just to the console
 */
static void logHandler(int flags, int level, cchar *msg)
{
#if KEEP
    if (flags & MPR_USER_MSG) {
        MessageBoxEx(NULL, msg, mprGetAppTitle(), MB_OK, 0);
    }
#endif
    mprWriteToOsLog(msg, 0, 0);
}


static void terminating(int how, int status)
{
}


static void gracefulShutdown(MprTicks timeout)
{
    HWND    hwnd;

    hwnd = FindWindow(BIT_PRODUCT, BIT_TITLE);
    if (hwnd) {
        PostMessage(hwnd, WM_QUIT, 0, 0L);

        /*
            Wait for up to ten seconds while the service exits
         */
        while (timeout > 0 && hwnd) {
            mprSleep(100);
            timeout -= 100;
            hwnd = FindWindow(BIT_PRODUCT, BIT_TITLE);
            if (hwnd == 0) {
                return;
            }
        }
    }
    if (app->servicePid) {
        TerminateProcess((HANDLE) app->servicePid, MPR_EXIT_GRACEFUL);
        app->servicePid = 0;
    }
}


#else
PUBLIC void stubManager() {
    fprintf(stderr, "Manager not supported on this architecture");
}
#endif /* BIT_WIN_LIKE */

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
