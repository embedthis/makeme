/*
    http.c -- Http client program

    The http program is a client to issue HTTP requests. It is also a test platform for loading and testing web servers. 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
 
/******************************** Includes ***********************************/

#include    "http.h"

/*********************************** Locals ***********************************/

typedef struct ThreadData {
    HttpConn        *conn;
    MprDispatcher   *dispatcher;
    char            *url;
    MprList         *files;
} ThreadData;

typedef struct App {
    int      activeLoadThreads;  /* Still running test threads */
    char     *authType;          /* Authentication: basic|digest */
    int      benchmark;          /* Output benchmarks */
    cchar    *ca;                /* Certificate bundle to use when validating the server certificate */
    cchar    *cert;              /* Certificate to identify the client */
    int      chunkSize;          /* Ask for response data to be chunked in this quanta */
    char     *ciphers;           /* Set of acceptable ciphers to use for SSL */
    int      continueOnErrors;   /* Continue testing even if an error occurs. Default is to stop */
    int      success;            /* Total success flag */
    int      fetchCount;         /* Total count of fetches */
    MprFile  *inFile;            /* Input file for post/put data */
    MprList  *files;             /* Upload files */
    MprList  *formData;          /* Form body data */
    MprBuf   *bodyData;          /* Block body data */
    Mpr      *mpr;               /* Portable runtime */
    MprList  *headers;           /* Request headers */
    Http     *http;              /* Http service object */
    int      iterations;         /* URLs to fetch (per thread) */
    cchar    *key;               /* Private key file */
    char     *host;              /* Host to connect to */
    int      loadThreads;        /* Number of threads to use for URL requests */
    char     *method;            /* HTTP method when URL on cmd line */
    int      nextArg;            /* Next arg to parse */
    int      noout;              /* Don't output files */
    int      nofollow;           /* Don't automatically follow redirects */
    char     *outFilename;       /* Output filename */
    MprFile  *outFile;           /* Output file */
    char     *password;          /* Password for authentication */
    int      printable;          /* Make binary output printable */
    char     *protocol;          /* HTTP/1.0, HTTP/1.1 */
    char     *provider;          /* SSL provider to use */
    char     *ranges;            /* Request ranges */
    MprList  *requestFiles;      /* Request files */
    int      retries;            /* Times to retry a failed request */
    int      sequence;           /* Sequence requests with a custom header */
    int      status;             /* Status for single requests */
    int      showStatus;         /* Output the Http response status */
    int      showHeaders;        /* Output the response headers */
    int      singleStep;         /* Pause between requests */
    MprSsl   *ssl;               /* SSL configuration */
    char     *target;            /* Destination url */
    int      text;               /* Emit errors in plain text */
    int      timeout;            /* Timeout in msecs for a non-responsive server */
    int      upload;             /* Upload using multipart mime */
    char     *username;          /* User name for authentication of requests */
    int      verifyPeer;         /* Validate server certs */
    int      verifyIssuer;       /* Validate the issuer. Permits self-signed certs if false. */
    int      verbose;            /* Trace progress */
    int      workers;            /* Worker threads. >0 if multi-threaded */
    int      zeroOnErrors;       /* Exit zero status for any valid HTTP response code  */
    MprList  *threadData;        /* Per thread data */
    MprMutex *mutex;
} App;

static App *app;

/***************************** Forward Declarations ***************************/

static void     addFormVars(cchar *buf);
static void     processing();
static int      doRequest(HttpConn *conn, cchar *url, MprList *files);
static void     finishThread(MprThread *tp);
static char     *getPassword();
static void     initSettings();
static bool     isPort(cchar *name);
static cchar    *formatOutput(HttpConn *conn, cchar *buf, ssize *count);
static void     manageApp(App *app, int flags);
static void     manageThreadData(ThreadData *data, int flags);
static int      parseArgs(int argc, char **argv);
static int      processThread(HttpConn *conn, MprEvent *event);
static void     threadMain(void *data, MprThread *tp);
static char     *resolveUrl(HttpConn *conn, cchar *url);
static int      setContentLength(HttpConn *conn, MprList *files);
static int      showUsage();
static void     trace(HttpConn *conn, cchar *url, int fetchCount, cchar *method, int status, MprOff contentLen);
static void     waitForUser();
static ssize    writeBody(HttpConn *conn, MprList *files);

/*********************************** Code *************************************/

MAIN(httpMain, int argc, char **argv, char **envp)
{
    MprTime     start;
    double      elapsed;

    if (mprCreate(argc, argv, MPR_USER_EVENTS_THREAD) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((app = mprAllocObj(App, manageApp)) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprAddRoot(app);
    mprAddStandardSignals();

    initSettings();
    if (parseArgs(argc, argv) < 0) {
        return MPR_ERR_BAD_ARGS;
    }
    mprSetMaxWorkers(app->workers);
    if (mprStart() < 0) {
        mprError("Cannot start MPR for %s", mprGetAppTitle());
        exit(2);
    }
    start = mprGetTime();
    app->http = httpCreate(HTTP_CLIENT_SIDE);
    httpEaseLimits(app->http->clientLimits);

    processing();
    mprServiceEvents(-1, 0);

    if (app->benchmark) {
        elapsed = (double) (mprGetTime() - start);
        if (app->fetchCount == 0) {
            elapsed = 0;
            app->fetchCount = 1;
        }
        mprPrintf("\nRequest Count:       %13d\n", app->fetchCount);
        mprPrintf("Time elapsed:        %13.4f sec\n", elapsed / 1000.0);
        mprPrintf("Time per request:    %13.4f sec\n", elapsed / 1000.0 / app->fetchCount);
        mprPrintf("Requests per second: %13.4f\n", app->fetchCount * 1.0 / (elapsed / 1000.0));
        mprPrintf("Load threads:        %13d\n", app->loadThreads);
        mprPrintf("Worker threads:      %13d\n", app->workers);
    }
    if (!app->success && app->verbose) {
        mprError("Request failed");
    }
    return (app->success) ? 0 : 255;
}


static void manageApp(App *app, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(app->ca);
        mprMark(app->cert);
        mprMark(app->ciphers);
        mprMark(app->files);
        mprMark(app->formData);
        mprMark(app->headers);
        mprMark(app->host);
        mprMark(app->bodyData);
        mprMark(app->http);
        mprMark(app->inFile);
        mprMark(app->key);
        mprMark(app->outFile);
        mprMark(app->outFilename);
        mprMark(app->mutex);
        mprMark(app->password);
        mprMark(app->ranges);
        mprMark(app->requestFiles);
        mprMark(app->ssl);
        mprMark(app->threadData);
    }
}


static void initSettings()
{
    app->method = 0;
    app->verbose = 0;
    app->continueOnErrors = 0;
    app->showHeaders = 0;
    app->verifyIssuer = -1;
    app->verifyPeer = 0;
    app->zeroOnErrors = 0;

    app->authType = sclone("basic");
    app->host = sclone("localhost");
    app->iterations = 1;
    app->loadThreads = 1;
    app->protocol = "HTTP/1.1";
    app->retries = HTTP_RETRIES;
    app->success = 1;

    /* zero means no timeout */
    app->timeout = 0;
    app->workers = 1;            
    app->headers = mprCreateList(0, 0);
    app->mutex = mprCreateLock();
#if WINDOWS
    _setmode(fileno(stdout), O_BINARY);
#endif
}


static int parseArgs(int argc, char **argv)
{
    char        *argp, *key, *value;
    int         i, setWorkers, nextArg, ssl;

    setWorkers = 0;
    ssl = 0;

    for (nextArg = 1; nextArg < argc; nextArg++) {
        argp = argv[nextArg];
        if (*argp != '-') {
            break;
        }
        if (smatch(argp, "--auth")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->authType = slower(argv[++nextArg]);
            }

        } else if (smatch(argp, "--benchmark") || smatch(argp, "-b")) {
            app->benchmark++;

        } else if (smatch(argp, "--ca")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->ca = sclone(argv[++nextArg]);
                if (!mprPathExists(app->ca, R_OK)) {
                    mprError("Cannot find ca file %s", app->ca);
                    return MPR_ERR_BAD_ARGS;
                }
            }
            ssl = 1;

        } else if (smatch(argp, "--cert")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->cert = sclone(argv[++nextArg]);
                if (!mprPathExists(app->cert, R_OK)) {
                    mprError("Cannot find cert file %s", app->cert);
                    return MPR_ERR_BAD_ARGS;
                }
            }
            ssl = 1;

        } else if (smatch(argp, "--chunk")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                value = argv[++nextArg];
                app->chunkSize = atoi(value);
                if (app->chunkSize < 0) {
                    mprError("Bad chunksize %d", app->chunkSize);
                    return MPR_ERR_BAD_ARGS;
                }
            }

        } else if (smatch(argp, "--ciphers")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->ciphers = sclone(argv[++nextArg]);
            }
            ssl = 1;

        } else if (smatch(argp, "--continue")) {
            app->continueOnErrors++;

        } else if (smatch(argp, "--cookie")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                mprAddItem(app->headers, mprCreateKeyPair("Cookie", argv[++nextArg]));
            }

        } else if (smatch(argp, "--data")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                if (app->bodyData == 0) {
                    app->bodyData = mprCreateBuf(-1, -1);
                }
                mprPutStringToBuf(app->bodyData, argv[++nextArg]);
            }

        } else if (smatch(argp, "--debugger") || smatch(argp, "-D")) {
            mprSetDebugMode(1);
            app->retries = 0;
            app->timeout = MAXINT;

        } else if (smatch(argp, "--delete")) {
            app->method = "DELETE";

        } else if (smatch(argp, "--form") || smatch(argp, "-f")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                if (app->formData == 0) {
                    app->formData = mprCreateList(-1, 0);
                }
                addFormVars(argv[++nextArg]);
            }

        } else if (smatch(argp, "--header")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                key = argv[++nextArg];
                if ((value = strchr(key, ':')) == 0) {
                    mprError("Bad header format. Must be \"key: value\"");
                    return MPR_ERR_BAD_ARGS;
                }
                *value++ = '\0';
                while (isspace((uchar) *value)) {
                    value++;
                }
                mprAddItem(app->headers, mprCreateKeyPair(key, value));
            }

        } else if (smatch(argp, "--host")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->host = argv[++nextArg];
                if (*app->host == ':') {
                    app->host = &app->host[1];
                } 
                if (isPort(app->host)) {
                    app->host = sfmt("http://127.0.0.1:%s", app->host);
                } else {
                    app->host = sclone(app->host);
                }
            }

        } else if (smatch(argp, "--iterations") || smatch(argp, "-i")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->iterations = atoi(argv[++nextArg]);
            }

        } else if (smatch(argp, "--key")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->key = sclone(argv[++nextArg]);
                if (!mprPathExists(app->key, R_OK)) {
                    mprError("Cannot find key file %s", app->key);
                    return MPR_ERR_BAD_ARGS;
                }
            }
            ssl = 1;

        } else if (smatch(argp, "--log") || smatch(argp, "-l")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                mprStartLogging(argv[++nextArg], 0);
            }

        } else if (smatch(argp, "--method") || smatch(argp, "-m")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->method = argv[++nextArg];
            }

        } else if (smatch(argp, "--out") || smatch(argp, "-o")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->outFilename = sclone(argv[++nextArg]);
            }

        } else if (smatch(argp, "--noout") || smatch(argp, "-n")  ||
                   smatch(argp, "--quiet") || smatch(argp, "-q")) {
            app->noout++;

        } else if (smatch(argp, "--nofollow")) {
            app->nofollow++;

        } else if (smatch(argp, "--password") || smatch(argp, "-p")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->password = sclone(argv[++nextArg]);
            }

        } else if (smatch(argp, "--post")) {
            app->method = "POST";

        } else if (smatch(argp, "--printable")) {
            app->printable++;

        } else if (smatch(argp, "--protocol")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->protocol = supper(argv[++nextArg]);
            }

        } else if (smatch(argp, "--provider")) {
            /* Undocumented SSL provider selection */
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->provider = sclone(argv[++nextArg]);
            }
            ssl = 1;

        } else if (smatch(argp, "--put")) {
            app->method = "PUT";

        } else if (smatch(argp, "--range")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                //  TODO - should allow multiple ranges
                if (app->ranges == 0) {
                    app->ranges = sfmt("bytes=%s", argv[++nextArg]);
                } else {
                    app->ranges = srejoin(app->ranges, ",", argv[++nextArg], NULL);
                }
            }
            
        } else if (smatch(argp, "--retries") || smatch(argp, "-r")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->retries = atoi(argv[++nextArg]);
            }

        } else if (smatch(argp, "--self")) {
            /* Undocumented. Allow self-signed certs. Users should just not set --verify */
            app->verifyIssuer = 0;
            ssl = 1;
            
        } else if (smatch(argp, "--sequence")) {
            app->sequence++;

        } else if (smatch(argp, "--showHeaders") || smatch(argp, "--show") || smatch(argp, "-s")) {
            app->showHeaders++;

        } else if (smatch(argp, "--showStatus") || smatch(argp, "--showCode")) {
            app->showStatus++;

        } else if (smatch(argp, "--single") || smatch(argp, "-s")) {
            app->singleStep++;

        } else if (smatch(argp, "--text")) {
            app->text++;

        } else if (smatch(argp, "--threads") || smatch(argp, "-t")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->loadThreads = atoi(argv[++nextArg]);
            }

        } else if (smatch(argp, "--timeout")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->timeout = atoi(argv[++nextArg]) * MPR_TICKS_PER_SEC;
            }

        } else if (smatch(argp, "--upload") || smatch(argp, "-u")) {
            app->upload++;

        } else if (smatch(argp, "--user") || smatch(argp, "--username")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->username = argv[++nextArg];
            }

        //  DEPRECATE validate. Preserve verify.
        } else if (smatch(argp, "--validate") || smatch(argp, "--verify")) {
            app->verifyPeer = 1;
            ssl = 1;

        } else if (smatch(argp, "--verbose") || smatch(argp, "-v")) {
            app->verbose++;

        } else if (smatch(argp, "--version") || smatch(argp, "-V")) {
            mprEprintf("%s %s\n"
                "Copyright (C) Embedthis Software 2003-2013\n"
                "Copyright (C) Michael O'Brien 2003-2013\n",
               BIT_TITLE, BIT_VERSION);
            exit(0);

        } else if (smatch(argp, "--workerTheads") || smatch(argp, "-w")) {
            if (nextArg >= argc) {
                return showUsage();
            } else {
                app->workers = atoi(argv[++nextArg]);
            }
            setWorkers++;

        } else if (smatch(argp, "--zero")) {
            app->zeroOnErrors++;

        } else if (smatch(argp, "--")) {
            nextArg++;
            break;

        } else if (smatch(argp, "-")) {
            break;

        } else {
            return showUsage();
        }
    }
    if (argc == nextArg) {
        return showUsage();
    }
    app->nextArg = nextArg;
    argc = argc - nextArg;
    argv = &argv[nextArg];
    app->target = argv[argc - 1];
    if (--argc > 0) {
        /*
            Files present on command line
         */
        app->files = mprCreateList(argc, MPR_LIST_STATIC_VALUES);
        for (i = 0; i < argc; i++) {
            mprAddItem(app->files, argv[i]);
        }
    }
    if (!setWorkers) {
        app->workers = app->loadThreads + 2;
    }
    if (app->method == 0) {
        if (app->bodyData || app->formData || app->upload) {
            app->method = "POST";
        } else if (app->files) {
            app->method = "PUT";
        } else {
            app->method = "GET";
        }
    }
#if BIT_PACK_SSL
{
    HttpUri *uri = httpCreateUri(app->target, 0);
    if (uri->secure || ssl) {
        app->ssl = mprCreateSsl(0);
        if (app->provider) {
            mprSetSslProvider(app->ssl, app->provider);
        }
        if (app->cert) {
            if (!app->key) {
                mprError("Must specify key file");
                return 0;
            }
            mprSetSslCertFile(app->ssl, app->cert);
            mprSetSslKeyFile(app->ssl, app->key);
        }
        if (app->ca) {
            mprLog(4, "Using CA: \"%s\"", app->ca);
            mprSetSslCaFile(app->ssl, app->ca);
        }
        if (app->verifyIssuer == -1) {
            app->verifyIssuer = app->verifyPeer ? 1 : 0;
        }
        mprVerifySslPeer(app->ssl, app->verifyPeer);
        mprVerifySslIssuer(app->ssl, app->verifyIssuer);
        if (app->ciphers) {
            mprSetSslCiphers(app->ssl, app->ciphers);
        }
    } else {
        mprVerifySslPeer(NULL, 0);
    }
}
#else
    /* Suppress comp warning */
    mprNop(&ssl);
#endif
    return 0;
}


static int showUsage()
{
    mprEprintf("usage: %s [options] [files] url\n"
        "  Options:\n"
        "  --auth basic|digest   # Set authentication type.\n"
        "  --benchmark           # Compute benchmark results.\n"
        "  --ca file             # Certificate bundle to use when validating the server certificate.\n"
        "  --cert file           # Certificate to send to the server to identify the client.\n"
        "  --chunk size          # Request response data to use this chunk size.\n"
        "  --ciphers cipher,...  # List of suitable ciphers.\n"
        "  --continue            # Continue on errors.\n"
        "  --cookie CookieString # Define a cookie header. Multiple uses okay.\n"
        "  --data bodyData       # Body data to send with PUT or POST.\n"
        "  --debugger            # Disable timeouts to make running in a debugger easier.\n"
        "  --delete              # Use the DELETE method. Shortcut for --method DELETE..\n"
        "  --form string         # Form data. Must already be form-www-urlencoded.\n"
        "  --header 'key: value' # Add a custom request header.\n"
        "  --host hostName       # Host name or IP address for unqualified URLs.\n"
        "  --iterations count    # Number of times to fetch the URLs per thread (default 1).\n"
        "  --key file            # Private key file.\n"
        "  --log logFile:level   # Log to the file at the verbosity level.\n"
        "  --method KIND         # HTTP request method GET|OPTIONS|POST|PUT|TRACE (default GET).\n"
        "  --nofollow            # Don't automatically follow redirects.\n"
        "  --noout               # Don't output files to stdout.\n"
        "  --out file            # Send output to file.\n"
        "  --password pass       # Password for authentication.\n"
        "  --post                # Use POST method. Shortcut for --method POST.\n"
        "  --printable           # Make binary output printable.\n"
        "  --protocol PROTO      # Set HTTP protocol to HTTP/1.0 or HTTP/1.1 .\n"
        "  --put                 # Use PUT method. Shortcut for --method PUT.\n"
        "  --range byteRanges    # Request a subset range of the document.\n"
        "  --retries count       # Number of times to retry failing requests.\n"
        "  --sequence            # Sequence requests with a custom header.\n"
        "  --showHeaders         # Output response headers.\n"
        "  --showStatus          # Output the Http response status code.\n"
        "  --single              # Single step. Pause for input between requests.\n"
        "  --threads count       # Number of thread instances to spawn.\n"
        "  --timeout secs        # Request timeout period in seconds.\n"
        "  --upload              # Use multipart mime upload.\n"
        "  --user name           # User name for authentication.\n"
        "  --verify              # Validate server certificates when using SSL.\n"
        "  --verbose             # Verbose operation. Trace progress.\n"
        "  --workers count       # Set maximum worker threads.\n"
        "  --zero                # Exit with zero status for any valid HTTP response.\n"
        , mprGetAppName());
    return MPR_ERR_BAD_ARGS;
}


static void processing()
{
    MprThread   *tp;
    ThreadData  *data;
    int         j;

    if (app->chunkSize > 0) {
        mprAddItem(app->headers, mprCreateKeyPair("X-Appweb-Chunk-Size", sfmt("%d", app->chunkSize)));
    }
    app->activeLoadThreads = app->loadThreads;
    app->threadData = mprCreateList(app->loadThreads, 0);

    for (j = 0; j < app->loadThreads; j++) {
        char name[64];
        if ((data = mprAllocObj(ThreadData, manageThreadData)) == 0) {
            return;
        }
        mprAddItem(app->threadData, data);

        fmt(name, sizeof(name), "http.%d", j);
        tp = mprCreateThread(name, threadMain, NULL, 0); 
        tp->data = data;
        mprStartThread(tp);
    }
}


static void manageThreadData(ThreadData *data, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(data->url);
        mprMark(data->files);
        mprMark(data->conn);
    }
}


/*  
    Per-thread execution. Called for main thread and helper threads.
 */ 
static void threadMain(void *data, MprThread *tp)
{
    ThreadData      *td;
    HttpConn        *conn;
    MprEvent        e;

    td = tp->data;
    td->dispatcher = mprCreateDispatcher(tp->name, 1);
    td->conn = conn = httpCreateConn(app->http, NULL, td->dispatcher);

    /*  
        Relay to processThread via the dispatcher. This serializes all activity on the conn->dispatcher
     */
    e.mask = MPR_READABLE;
    e.data = tp;
    mprRelayEvent(conn->dispatcher, (MprEventProc) processThread, conn, &e);
}


static int processThread(HttpConn *conn, MprEvent *event)
{
    ThreadData  *td;
    cchar       *path;
    char        *url;
    int         next, count;

    td = mprGetCurrentThread()->data;
    httpFollowRedirects(conn, !app->nofollow);
    httpSetTimeout(conn, app->timeout, app->timeout);

    if (strcmp(app->protocol, "HTTP/1.0") == 0) {
        httpSetKeepAliveCount(conn, 0);
        httpSetProtocol(conn, "HTTP/1.0");
    }
    if (app->username) {
        if (app->password == 0 && !strchr(app->username, ':')) {
            app->password = getPassword();
        }
        httpSetCredentials(conn, app->username, app->password, app->authType);
    }
    for (count = 0; count < app->iterations; count++) {
        if (mprShouldDenyNewRequests(conn)) {
            break;
        }
        if (!app->success && !app->continueOnErrors) {
            break;
        }
        if (app->singleStep) waitForUser();
        if (app->files && !app->upload) {
            for (next = 0; (path = mprGetNextItem(app->files, &next)) != 0; ) {
                /*
                    If URL ends with "/", assume it is a directory on the target and append each file name 
                 */
                if (app->target[strlen(app->target) - 1] == '/') {
                    url = mprJoinPath(app->target, mprGetPathBase(path));
                } else {
                    url = app->target;
                }
                app->requestFiles = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
                mprAddItem(app->requestFiles, path);
                td->url = url = resolveUrl(conn, url);
                if (app->verbose) {
                    mprPrintf("putting: %s to %s\n", path, url);
                }
                if (doRequest(conn, url, app->requestFiles) < 0) {
                    app->success = 0;
                    break;
                }
            }
        } else {
            td->url = url = resolveUrl(conn, app->target);
            if (doRequest(conn, url, app->files) < 0) {
                app->success = 0;
                break;
            }
        }
        if (app->verbose > 1) {
            mprPrintf(".");
        }
    }
    httpDestroyConn(conn);
    finishThread((MprThread*) event->data);
    return -1;
}


static int prepRequest(HttpConn *conn, MprList *files, int retry)
{
    MprKeyValue     *header;
    char            *seq;
    int             next;

    httpPrepClientConn(conn, retry);

    for (next = 0; (header = mprGetNextItem(app->headers, &next)) != 0; ) {
        if (scaselessmatch(header->key, "User-Agent")) {
            httpSetHeader(conn, header->key, header->value);
        } else {
            httpAppendHeader(conn, header->key, header->value);
        }
    }
    if (app->text) {
        httpSetHeader(conn, "Accept", "text/plain");
    }
    if (app->sequence) {
        static int next = 0;
        seq = itos(next++);
        httpSetHeader(conn, "X-Http-Seq", seq);
    }
    if (app->ranges) {
        httpSetHeader(conn, "Range", app->ranges);
    }
    if (app->formData) {
        httpSetHeader(conn, "Content-Type", "application/x-www-form-urlencoded");
    }
    if (app->chunkSize > 0) {
        httpSetChunkSize(conn, app->chunkSize);
    }
    if (setContentLength(conn, files) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    return 0;
}


static int sendRequest(HttpConn *conn, cchar *method, cchar *url, MprList *files)
{
    if (httpConnect(conn, method, url, app->ssl) < 0) {
        mprError("Cannot process request for \"%s\"\n%s", url, httpGetError(conn));
        return MPR_ERR_CANT_OPEN;
    }
    /*  
        This program does not do full-duplex writes with reads. ie. if you have a request that sends and receives
        data in parallel -- http will do the writes first then read the response.
     */
    if (app->bodyData || app->formData || files) {
        if (writeBody(conn, files) < 0) {
            mprError("Cannot write body data to \"%s\". %s.", url, httpGetError(conn));
            return MPR_ERR_CANT_WRITE;
        }
    }
    assert(!mprGetCurrentThread()->yielded);
    httpFinalizeOutput(conn);
    return 0;
}


static int issueRequest(HttpConn *conn, cchar *url, MprList *files)
{
    HttpRx      *rx;
    HttpUri     *target, *location;
    char        *redirect;
    cchar       *msg, *sep, *authType;
    int         count, redirectCount, rc;

    httpSetRetries(conn, app->retries);
    httpSetTimeout(conn, app->timeout, app->timeout);
    authType = conn->authType;

    for (redirectCount = count = 0; count <= conn->retries && redirectCount < 10 && !mprShouldAbortRequests(conn); count++) {
        if (prepRequest(conn, files, count) < 0) {
            return MPR_ERR_CANT_OPEN;
        }
        if (sendRequest(conn, app->method, url, files) < 0) {
            return MPR_ERR_CANT_WRITE;
        }
        if ((rc = httpWait(conn, HTTP_STATE_PARSED, conn->limits->requestTimeout)) == 0) {
            if (httpNeedRetry(conn, &redirect)) {
                if (redirect) {
                    location = httpCreateUri(redirect, 0);
                    target = httpJoinUri(conn->tx->parsedUri, 1, &location);
                    url = httpUriToString(target, HTTP_COMPLETE_URI);
                    count = 0;
                }
                if (conn->rx && conn->rx->status == HTTP_CODE_UNAUTHORIZED && authType) {
                    /* Supplied authentication details and failed */
                    break;
                }
                redirectCount++;
                count--; 
            } else {
                break;
            }
        } else if (!conn->error) {
            if (rc == MPR_ERR_TIMEOUT) {
                httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TIMEOUT,
                    "Inactive request timed out, exceeded request timeout %d", app->timeout);
            } else {
                httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Connection I/O error");
            }
        }
        if ((rx = conn->rx) != 0) {
            if (rx->status == HTTP_CODE_REQUEST_TOO_LARGE || rx->status == HTTP_CODE_REQUEST_URL_TOO_LARGE ||
                (rx->status == HTTP_CODE_UNAUTHORIZED && conn->username == 0)) {
                /* No point retrying */
                break;
            }
        }
        mprTrace(4, "retry %d of %d for: %s %s", count, conn->retries, app->method, url);
    }
    if (conn->error) {
        msg = (conn->errorMsg) ? conn->errorMsg : "";
        sep = (msg && *msg) ? "\n" : "";
        mprError("http: failed \"%s\" request for %s after %d attempt(s).%s%s", app->method, url, count, sep, msg);
        return MPR_ERR_CANT_CONNECT;
    }
    return 0;
}


static int reportResponse(HttpConn *conn, cchar *url, MprTicks elapsed)
{
    HttpRx      *rx;
    MprOff      bytesRead;
    char        *responseHeaders;
    int         status;

    if (mprShouldAbortRequests(conn)) {
        return 0;
    }
    app->status = status = httpGetStatus(conn);
    bytesRead = httpGetContentLength(conn);
    if (bytesRead < 0 && conn->rx) {
        bytesRead = conn->rx->bytesRead;
    }
    mprTrace(6, "Response status %d, elapsed %Ld", status, elapsed);
    if (conn->error) {
        app->success = 0;
    }
    if (conn->rx) {
        if (app->showHeaders) {
            responseHeaders = httpGetHeaders(conn);
            rx = conn->rx;
            mprPrintf("%s %d %s\n", conn->protocol, status, rx->statusMessage);
            if (responseHeaders) {
                mprPrintf("%s\n", responseHeaders);
            }
        } else if (app->showStatus) {
            mprPrintf("%d\n", status);
        }
    }
    if (status < 0) {
        mprError("Cannot process request for \"%s\" %s", url, httpGetError(conn));
        return MPR_ERR_CANT_READ;

    } else if (status == 0 && conn->protocol == 0) {
        /* Ignore */;

    } else if (!(200 <= status && status <= 206) && !(301 <= status && status <= 304)) {
        if (!app->zeroOnErrors) {
            app->success = 0;
        }
        if (!app->showStatus) {
            mprError("Cannot process request for \"%s\" (%d) %s", url, status, httpGetError(conn));
            return MPR_ERR_CANT_READ;
        }
    }
    mprLock(app->mutex);
    app->fetchCount++;
    if (app->verbose && app->noout) {
        trace(conn, url, app->fetchCount, app->method, status, bytesRead);
    }
    mprUnlock(app->mutex);
    return 0;
}


static void readBody(HttpConn *conn, MprFile *outFile)
{
    char        buf[BIT_MAX_BUFFER];
    cchar       *result;
    ssize       bytes;

    while (!conn->error && conn->sock && (bytes = httpRead(conn, buf, sizeof(buf))) > 0) {
        if (!app->noout) {
            result = formatOutput(conn, buf, &bytes);
            if (result) {
                mprWriteFile(outFile, result, bytes);
            }
        }
#if FUTURE
        //  This should be pushed into a range filter.
        //  Buffer all output and then parsing can work  
        type = httpGetHeader(conn, "Content-Type");
        if (scontains(type, "multipart/byteranges")) {
            if ((boundary = scontains(type, "boundary=")) != 0) {
                boundary += 9;
                if (*boundary) {
                    boundary = sfmt("--%s\r\n", boundary);
                }
            }
        }
#endif
    }
}


static int doRequest(HttpConn *conn, cchar *url, MprList *files)
{
    MprTicks        mark, remaining;
    HttpLimits      *limits;
    MprFile         *outFile;
    cchar           *path;

    assert(url && *url);
    limits = conn->limits;

    mprTrace(4, "fetch: %s %s", app->method, url);
    mark = mprGetTicks();

    if (issueRequest(conn, url, files) < 0) {
        return MPR_ERR_CANT_CONNECT;
    }
    remaining = limits->requestTimeout;

    if (app->outFilename) {
        path = app->loadThreads > 1 ? sfmt("%s-%s.tmp", app->outFilename, mprGetCurrentThreadName()): app->outFilename;
        if ((outFile = mprOpenFile(path, O_CREAT | O_WRONLY | O_TRUNC | O_TEXT, 0664)) == 0) {
            mprError("Cannot open %s", path);
            return MPR_ERR_CANT_OPEN;
        }
    } else {
        outFile = mprGetStdout();
    }
    mprAddRoot(outFile);
    while (!conn->tx->finalized && conn->state < HTTP_STATE_COMPLETE && remaining > 0) {
        remaining = mprGetRemainingTicks(mark, limits->requestTimeout);
        httpWait(conn, 0, remaining);
        readBody(conn, outFile);
    }
    if (conn->state < HTTP_STATE_COMPLETE && !conn->error) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TIMEOUT,
            "Inactive request timed out, exceeded request timeout %d", app->timeout);
    } else {
        readBody(conn, outFile);
    }
    if (app->outFilename) {
        mprCloseFile(outFile);
    }
    mprRemoveRoot(outFile);
    reportResponse(conn, url, mprGetTicks() - mark);
    httpDestroyRx(conn->rx);
    httpDestroyTx(conn->tx);
    return 0;
}


static int setContentLength(HttpConn *conn, MprList *files)
{
    MprPath     info;
    MprOff      len;
    char        *path, *pair;
    int         next;

    len = 0;
    if (app->upload) {
        httpEnableUpload(conn);
        return 0;
    }
    for (next = 0; (path = mprGetNextItem(files, &next)) != 0; ) {
        if (strcmp(path, "-") != 0) {
            if (mprGetPathInfo(path, &info) < 0) {
                mprError("Cannot access file %s", path);
                return MPR_ERR_CANT_ACCESS;
            }
            len += info.size;
        }
    }
    if (app->formData) {
        for (next = 0; (pair = mprGetNextItem(app->formData, &next)) != 0; ) {
            len += slen(pair);
        }
        len += mprGetListLength(app->formData) - 1;
    }
    if (app->bodyData) {
        len += mprGetBufLength(app->bodyData);
    }
    if (len > 0) {
        httpSetContentLength(conn, len);
    }
    return 0;
}


static ssize writeBody(HttpConn *conn, MprList *files)
{
    MprFile     *file;
    char        buf[BIT_MAX_BUFFER], *path, *pair;
    ssize       bytes, len, count, nbytes, sofar;
    int         next;

    if (app->upload) {
        if (httpWriteUploadData(conn, app->files, app->formData) < 0) {
            return MPR_ERR_CANT_WRITE;
        }
    } else {
        if (app->formData) {
            count = mprGetListLength(app->formData);
            for (next = 0; (pair = mprGetNextItem(app->formData, &next)) != 0; ) {
                len = strlen(pair);
                if (next < count) {
                    len = slen(pair);
                    if (httpWrite(conn->writeq, pair, len) != len || httpWrite(conn->writeq, "&", 1) != 1) {
                        return MPR_ERR_CANT_WRITE;
                    }
                } else {
                    if (httpWrite(conn->writeq, pair, len) != len) {
                        return MPR_ERR_CANT_WRITE;
                    }
                }
            }
        }
        if (files) {
            assert(mprGetListLength(files) == 1);
            for (next = 0; (path = mprGetNextItem(files, &next)) != 0; ) {
                if (strcmp(path, "-") == 0) {
                    file = mprAttachFileFd(0, "stdin", O_RDONLY | O_BINARY);
                } else {
                    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
                }
                if (file == 0) {
                    mprError("Cannot open \"%s\"", path);
                    return MPR_ERR_CANT_OPEN;
                }
                app->inFile = file;
                if (app->verbose) {
                    mprPrintf("uploading: %s\n", path);
                }
                while ((bytes = mprReadFile(file, buf, sizeof(buf))) > 0) {
                    sofar = 0;
                    while (bytes > 0) {
                        if ((nbytes = httpWriteBlock(conn->writeq, &buf[sofar], bytes, HTTP_BLOCK)) < 0) {
                            mprCloseFile(file);
                            return MPR_ERR_CANT_WRITE;
                        }
                        bytes -= nbytes;
                        sofar += nbytes;
                        assert(bytes >= 0);
                    }
                    mprYield(0);
                }
                httpFlushQueue(conn->writeq, 1);
                mprCloseFile(file);
                app->inFile = 0;
            }
        }
        if (app->bodyData) {
            mprAddNullToBuf(app->bodyData);
            len = mprGetBufLength(app->bodyData);
            if (httpWriteBlock(conn->writeq, mprGetBufStart(app->bodyData), len, HTTP_BLOCK) != len) {
                return MPR_ERR_CANT_WRITE;
            }
        }
    }
    return 0;
}


static void finishThread(MprThread *tp)
{
    if (tp) {
        mprLock(app->mutex);
        if (--app->activeLoadThreads <= 0) {
            mprTerminate(MPR_EXIT_DEFAULT, -1);
        }
        mprUnlock(app->mutex);
    }
}


static void waitForUser()
{
    int     c;

    mprLock(app->mutex);
    mprPrintf("Pause: ");
    if (read(0, (char*) &c, 1) < 0) {}
    mprUnlock(app->mutex);
}


static void addFormVars(cchar *buf)
{
    char    *pair, *tok;

    pair = stok(sclone(buf), "&", &tok);
    while (pair != 0) {
        mprAddItem(app->formData, sclone(pair));
        pair = stok(0, "&", &tok);
    }
}


static bool isPort(cchar *name)
{
    cchar   *cp;

    for (cp = name; *cp && *cp != '/'; cp++) {
        if (!isdigit((uchar) *cp) || *cp == '.') {
            return 0;
        }
    }
    return 1;
}


static char *resolveUrl(HttpConn *conn, cchar *url)
{
    if (*url == '/') {
        if (app->host) {
            if (sncaselesscmp(app->host, "http://", 7) != 0 && sncaselesscmp(app->host, "https://", 8) != 0) {
                return sfmt("http://%s%s", app->host, url);
            } else {
                return sfmt("%s%s", app->host, url);
            }
        } else {
            return sfmt("http://127.0.0.1%s", url);
        }
    } 
    if (sncaselesscmp(url, "http://", 7) != 0 && sncaselesscmp(url, "https://", 8) != 0) {
        if (*url == ':' && isPort(&url[1])) {
            return sfmt("http://127.0.0.1%s", url);
        } else if (isPort(url)) {
            return sfmt("http://127.0.0.1:%s", url);
        } else {
            return sfmt("http://%s", url);
        }
    }
    return sclone(url);
}


static cchar *formatOutput(HttpConn *conn, cchar *buf, ssize *count)
{
    cchar       *result;
    int         i, c, isBinary;
    
    if (app->noout) {
        return 0;
    }
#if UNUSED
    HttpRx *rx = conn->rx;
    if (rx->status == 401 || (conn->followRedirects && (301 <= rx->status && rx->status <= 302))) {
        return 0;
    }
#endif
    if (!app->printable) {
        return buf;
    }
    isBinary = 0;
    for (i = 0; i < *count; i++) {
        if (!isprint((uchar) buf[i]) && buf[i] != '\n' && buf[i] != '\r' && buf[i] != '\t') {
            isBinary = 1;
            break;
        }
    }
    if (!isBinary) {
        return buf;
    }
    result = mprAlloc(*count * 3 + 1);
    for (i = 0; i < *count; i++) {
        c = (uchar) buf[i];
        if (app->printable && isBinary) {
            fmt("%02x ", -1, &result[i * 3], c & 0xff);
        } else {
            fmt("%c", -1, &result[i], c & 0xff);
        }
    }
    if (app->printable && isBinary) {
        *count *= 3;
    }
    return result;
}


static void trace(HttpConn *conn, cchar *url, int fetchCount, cchar *method, int status, MprOff contentLen)
{
    if (sncaselesscmp(url, "http://", 7) != 0) {
        url += 7;
    }
    if ((fetchCount % 200) == 1) {
        if (fetchCount == 1 || (fetchCount % 5000) == 1) {
            if (fetchCount > 1) {
                mprPrintf("\n");
            }
            mprPrintf("  Count  Thread   Op  Code   Bytes  Url\n");
        }
        mprPrintf("%7d %7s %4s %5d %7d  %s\n", fetchCount - 1,
            mprGetCurrentThreadName(conn), method, status, (uchar) contentLen, url);
    }
}


#if (BIT_WIN_LIKE && !WINCE) || VXWORKS
static char *getpass(char *prompt)
{
    static char password[80];
    int     c, i;

    fputs(prompt, stderr);
    for (i = 0; i < (int) sizeof(password) - 1; i++) {
#if VXWORKS
        c = getchar();
#else
        c = _getch();
#endif
        if (c == '\r' || c == EOF) {
            break;
        }
        if ((c == '\b' || c == 127) && i > 0) {
            password[--i] = '\0';
            fputs("\b \b", stderr);
            i--;
        } else if (c == 26) {           /* Control Z */
            c = EOF;
            break;
        } else if (c == 3) {            /* Control C */
            fputs("^C\n", stderr);
            exit(255);
        } else if (!iscntrl((uchar) c) && (i < (int) sizeof(password) - 1)) {
            password[i] = c;
            fputc('*', stderr);
        } else {
            fputc('', stderr);
            i--;
        }
    }
    if (c == EOF) {
        return "";
    }
    fputc('\n', stderr);
    password[i] = '\0';
    return password;
}

#endif /* WIN */


static char *getPassword()
{
#if !WINCE
    char    *password;

    password = getpass("Password: ");
#else
    password = "no-user-interaction-support";
#endif
    return sclone(password);
}


#if VXWORKS
/*
    VxWorks link resolution
 */
PUBLIC int _cleanup() {
    return 0;
}

PUBLIC int _exit() {
    return 0;
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
