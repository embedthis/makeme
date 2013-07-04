/*
    ejs.es -- Embedthis Ejscript Script Library

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: magnetar.local
 */


/************************************************************************/
/*
    Start of file "src/core/App.es"
 */
/************************************************************************/

/*
    App.es -- Application configuration and control.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /** 
        Standard error text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stderr: TextStream

    /** 
        Standard input text stream. Use read(), readLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdin: TextStream

    /** 
        Standard output text stream. Use write(), writeLine() and other TextStream methods.
        @spec ejs
        @stability evolving
     */
    public var stdout: TextStream

    /** 
        Application configuration state. The App class is a singleton class object and that accesses and controls 
        the applications execution environment including the current working directory, application command line 
        arguments, environment strings, path to the application's executable and input and output streams.
        @spec ejs
        @stability evolving
     */
    enumerable class App {

        use default namespace public

        /** 
            Configuration environment specified in the ~/.ejsrc or ejsrc configuration files.
         */
        static var config: Object

        /**
            Separator string to use when constructing PATH style search strings
         */
        static var SearchSeparator: String = (Config.OS == 'windows') ? ';' : ':'

        /*  
            Standard I/O streams. These can be any kind of stream.
         */
        private static var _errorStream: Stream
        private static var _inputStream: Stream
        private static var _outputStream: Stream

        /**
            Default ejsrc configuration for the application
            @hide
         */
        static internal var defaultConfig = {
            log: {
                enable: true,
                location: "stderr",
                level: 0,
            },
            app: {
                reload: false,
            },
            cache: {
                app: { enable: false },
            },
            dirs: {
                cache: Path("cache"),
            },
            files: {
                ejsrc: Path("ejsrc"),
            },
            init: { },
            uris: { },
            test: {},
        }

        /**
            Default event Emitter for the application.
         */
        static var emitter: Emitter = new Emitter

        /** 
            Application logger. This singleton object respresents the Application default logger.
            If the ejsrc startup configuration file defines a log.location field, the log logger will send messages to
            the defined location. Otherwise, messages will be sent to the MprLog stream. 
         */
        public static var log: Logger

        /** 
            Application MPR log object. This singleton object represents the Application log file specified via the
            --log command line switch.
         */
        public static var mprLog: MprLog

        /** 
            Application name. Single word, lower-case name for the application. This is initialized to the name of
            the script or "ejs" if running interactively.
         */
        static var name: String

        /**
            Application in-memory cache reference
         */
        static var cache: Cache

        /**
            Application start time
         */
        static var started: Date = new Date

        /**
            Test object for unit tests when run via utest
         */
        static var test 

        /** 
            Application title name. Multi-word, Camel Case name for the application suitable for display. This is 
            initialized to the name of the script or "Embedthis Ejscript" if running interactively.
         */
        static var title: String

        /** 
            Application version string. Set to a version string [format Major.Minor.Patch-Build]. For example: 1.1.2-3.
            Initialized to the ejs version.
         */
        static var version: String

        /** 
            Application command line arguments. Set to an array containing each of the arguments. If the ejs command 
                is invoked as "ejs script arg1 arg2", then args[0] will be "script", args[1] will be "arg1" etc.
         */
        native static function get args(): Array

        /** 
            Change the application's working directory
            @param value The path to the new working directory
         */
        native static function chdir(value: Object): Void

        /** 
            Create a search path array for locating ejs modules. This converts a delimited PATH to an array of 
            paths suitable for use by the $search property. NOTE: this does not modify the application's search path.
            @param searchPath String containing a colon separated (or semi-colon on Windows) set of paths.
                If search path is null, the default system search paths are returned.
            @return An array of search paths.
         */
        native static function createSearch(searchPath: String? = null): Array

        /** 
            The application's current directory
            @return the path to the current directory
         */
        native static function get dir(): Path

        /** 
            Application environment. Object hash of all environment variables.
         */
        native static function get env(): Object

        /** 
            The directory containing the application executable
         */
        native static function get exeDir(): Path

        /** 
            The application executable path
         */
        native static function get exePath(): Path

        /** 
            The application's standard error file stream
            Set the standard error stream. Changing the error stream will close and reopen stderr.
         */
        static function get errorStream(): Stream
            _errorStream

        static function set errorStream(stream: Stream): Void {
            _errorStream = stream
            if (stderr) {
                stderr.close()
            }
            stderr = TextStream(_errorStream)
        }

        /** 
            Stop the program and exit.
            @param status The optional exit code to provide the environment. If running inside the ejs command program,
                the status is used as process exit status.
            @param how How the exit should proceed. Options are: "normal", "immediate" or "graceful". A normal exit
                will flush buffered data and close files and resources and then exit without waiting.  
                An immediate exit will exit without writing buffered data or closing files. A graceful exit will wait
                until the system is idle and then do a normal exit. A system is idle when it has no running commands, 
                sockets, Http requests or worker threads.
         */
        native static function exit(status: Number = 0, how: String = "normal"): Void

        /** 
            Get an environment variable.
            @param name The name of the environment variable to retrieve.
            @return The value of the environment variable or null if not found.
         */
        native static function getenv(name: String): String?

        /**
            The group ID of the user account running the application. Only supported on Unix.
         */
        native static function get gid(): Number
        
        /** 
            Set the standard input stream. Changing the input stream will close and reopen stdin.
         */
        static function get inputStream(): Stream
            _inputStream

       static function set inputStream(stream: Stream): Void {
            _inputStream = stream
            if (stdin) {
                stdin.close()
            }
            stdin = TextStream(_inputStream)
        }
        
        /**
            Load an "ejsrc" configuration file
            This loads an Ejscript configuration file and blends the contents with App.config. 
            @param path Path name of the file to load
            @param overwrite If true, then new configuration values overwrite existing values in App.config.
         */
        static function loadrc(path: Path, overwrite: Boolean = true) {
            if (path.exists) {
                try {
                    blend(App.config, path.readJSON(), {overwrite: overwrite})
                } catch (e) {
                    errorStream.write(App.exePath.basename +  " Cannot parse " + path + ": " + e + "\n")
                }
            }
        }

        //  TODO - move to locale
        /** 
            Get the current language locale for this application
            @hide
         */
        # FUTURE
        native static function get locale(): String
        # FUTURE
        native static function set locale(locale: String): Void

        /** 
            The standard output stream. Changing the output stream will close and reopen stdout.
         */
        static function get outputStream(): Stream
            _outputStream

        static function set outputStream(stream: Stream): Void {
            _outputStream = stream
            if (stdout) {
                stdout.close()
            }
            stdout = TextStream(_outputStream)
        }

        /**
            The process ID of the application
         */
        native static function get pid(): Number
        
        /** 
            Update an environment variable.
            @param name The name of the environment variable to retrieve.
            @param value The new value to define for the variable.
         */
        native static function putenv(name: String, value: String): Void

        //  MOB - should this be renamed App.pump()
        /** 
            Run the application event loop. 
            A script may call run() to service events. Calling run() will cause the ejs shell to wait and service 
            events until instructed to exit via App.exit. If the application is hosted in a web server, this routine will
            return true immediately without blocking. 
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately. Set to -1 for no timeout.
            @param oneEvent If true, return immediately after servicing at least one ejs event.
            @return True if an event happened. Otherwise return false if the timeout expired before any event.
         */
        native static function run(timeout: Number = -1, oneEvent: Boolean = false): Boolean

        /** 
            The current module search path. Set to an array of Paths.
         */
        native static function get search(): Array
        native static function set search(paths: Array): Void

        //  MOB - sleep should not throw
        /** 
            Sleep the application for the given number of milliseconds. Events will be serviced while asleep.
            An alternative to sleep is $App.run which can be configured to sleep and return early if an event is received.
            @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
         */
        native static function sleep(delay: Number = -1): Void

        /** 
            The current module search path . Set to a delimited searchPath string. Warning: This will be changed to an
            array of paths in a future release.
            @stability deprecated.
            @deprecate 1.0.0
            @hide
         */
        # Config.Legacy
        static function get searchPath(): String {
            if (Config.OS == 'windows') {
                return search.join(';')
            } else {
                return search.join(':')
            }
        }

        # Config.Legacy
        static function set searchPath(path: String): Void {
            if (Config.OS == 'windows') {
                search = path.split(';')
            } else {
                search = path.split(':')
            }
        }

        /**
            Service events
            @param count Count of events to service. Defaults to unlimited.
            @param timeout Timeout to block waiting for an event in milliseconds before returning. If an event occurs, the
                call returns immediately.
            @stability deprecated
            @hide
         */
        # Config.Legacy
        static function serviceEvents(count: Number = -1, timeout: Number = -1): Void {
            if (count < 0) {
                run(timeout, false)
            } else {
                for (i in count) {
                    run(timeout, true)
                }
            }
        }

        /**
            The user ID of the user account running the application. Only supported on Unix.
         */
        native static function get uid(): Number
        
        /**
            Redirect the Application's logger based on the App.config.log setting
            Ignored if app is invoked with --log on the command line.
         */
        static function updateLog(): Void {
            let log = config.log
            if (log && log.enable && !App.mprLog.fixed) {
                App.log.redirect(log.location, log.level)
                App.mprLog.redirect(log.location, log.level)
            }
        }

        /** 
            @hide
            Wait for an event
            @param Observable object
            @param events Events to consider
            @param timeout Timeout in milliseconds
         */
        static function waitForEvent(obj: *, events: Object, timeout: Number = Number.MaxInt32): Boolean {
            let done = false
            function callback(event) done = true
            obj.on(events, callback)
            for (let mark = new Date; !done && mark.elapsed < timeout; )
                App.run(timeout - mark.elapsed, true)
            obj.off(events, callback)
            return done
        }
    }

    /**  
        Initialize ejs applications. This is invoked when this module loads
        @hide
     */
    function appInit(): Void {
        App.name = Path(App.args[0] || Config.Product).basename
        App.title = App.name
        App.version = Config.Version
        App.mprLog = new MprLog

        /*  
            Load ~/.ejsrc and ejsrc
         */
        let config = App.config = {}
        let dir = App.getenv("HOME")
        if (dir) {
            App.loadrc(Path(dir).join(".ejsrc"))
        }
        App.loadrc("ejsrc")
        blend(config, App.defaultConfig, {overwrite: false})

        stdout = TextStream(App.outputStream)
        stderr = TextStream(App.errorStream)
        stdin = TextStream(App.inputStream)

        let log = config.log
        let stream
        if (log.enable) {
            let level = log.level
            let location = log.location
            if (App.mprLog.fixed) {
                /* App invoked with a --log switch */
                level = App.mprLog.level
                location = App.mprLog;
            } else {
                App.mprLog.redirect(location, level)
            }
            App.log = new Logger(App.name, location, level)
            if (log.match) {
                App.log.match = log.match
            }
        }
        /*  Append search paths */
        if (config.search) {
            if (config.search is Array) {
                App.search = config.search + App.search
            } else if (config.search is String) {
                App.search = config.search.split(App.SearchSeparator) + App.search
            }
        }
        /*  Pre-load modules and scripts */
        if (config.init.load) {
            for each (m in config.init.load) {
                load(m)
            }
        }
        if (config.cache) {
            //  MOB - should there be a config.cache.enable instead
            App.cache = new Cache(null, blend({shared: true}, config.cache))
        }
    }

    appInit()
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

/************************************************************************/
/*
    Start of file "src/core/Args.es"
 */
/************************************************************************/

/*
    Args.es - Command line argument parsing
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /**
        The Args class parses the Application's command line. A template of permissible command line options
        is passed to the Args constructor. The Args class supports command options that begin with "-" or "--" and parses
        option arguments of the forms: "-flag x" and "-flag=x". An option may have aliased forms (i.e. -v for --verbose).

        The command template contains properties for options, usage message, and error handling policy should 
        invalid options or arguments be submitted. The template can provide a range of valid values for a command option.
        The option range may be an Ejscript type, regular expression or set of values that constrains the permissible 
        option argument values. A default value may be provided for each option argument. 
        
        Once parsed, the arguments are accessible via args.options. The remaining command line arguments are available 
        in args.rest.

        @spec ejs
        @stability prototype
     */
    class Args {
        /* Program name from App.args[0] */
        public var program: Path

        /* Rest of the User supplied arguments that follow the command options */
        public var rest: Array = []

        /** User supplied command line options */
        public var options: Object = {}

        /* Copy of the argument template specification */
        public var template: Object

        /**
            The Args constructor creates a new instance of the Args. It parses the command line 
            and stores the parsed options in the $options and $args properties. 
            Args supports command options that begin with "-" or "--" and parses option arguments of the forms: 
            "-flag x" and "-flag=x".
            @param template Command argument template. The template is an object with option properties for:
                options, usage, onerror ad silent.
            @param argv Array of command arguments to parse. Defaults to App.args.
            @option options This is an object with properties for each command line option. The value of each property
                is an object with properties: 'alias', 'range', 'value' and 'separator'. The 'alias' property defines
                a String alias for the option. This is typically used to define a single character alias for the full
                command line option name. The 'range' property defines what permissible values an option parameter may take.
                It may be set to either an Ejscript type, a Regular Expression or it may be set to an array of values.

                The 'value' property may define a default value for the option if it is not defined.
                This is the value that args.options[OPTION] will be set to when the command line option is absent.  If
                an option without an argument is specified by the user, its value in options[NAME] will be set to true.  

                If the 'separator' property is defined, multiple command line options of this name are permitted.
                If 'separator' is set to a string, then the multiple command line option values are catenated
                using the 'separator'. If set to array, the option values are stored in an array.
            @option usage Function to invoke for argument parse errors 
            @option onerror
                If set to 'throw', an exception will be thrown for argument parse errors. The usage function will not be 
                called and no message will be written to the application log.
                If set to 'exit', a message will be written to the console for argument parse errors and any usage
                function will be invoked before exiting the application with a non-zero exit status.
            @option silent Don't emit any message on argument parse errors
            @option unknown Callback to invoke for unknown arguments
            @example
let args = Args({
    options: {
        depth: { range: Number },
        quiet: { },
        verbose: { alias: 'v', value: true, },
        log: { range: /\w+(:\d)/, value: 'stderr:4' },
        mode: { range: ['low', 'medium', 'high'], value: 'high' },
        with: { range: String, separator: ',' },
    },
    usage: usage,
    onerror: 'exit|throw',
}, [])

let options = args.options
if (options.verbose) { }
for each (file in args.rest) {
    ...
}
        */
        function Args(template: Object, argv: Array = App.args) {
            this.template = template.clone()
            for each (item in template.options) {
                if (item.alias) template.options[item.alias] = item
            }
            try {
                program = Path(argv[0])
                for (i = 1; i < argv.length; i++) {
                    let arg = argv[i]
                    if (arg.startsWith("-")) {
                        let name = arg.slice(arg.startsWith("--") ? 2 : 1)
                        let [key,value] = name.split("=")
                        let item = template.options[key]
                        if (!item) {
                            if (template.unknown) {
                                i = (template.unknown)(argv, i)
                                continue
                            } else {
                                throw "Undefined option '" + key + "'"
                            }
                        } else {
                            if (!value) {
                                if (!item.range) {
                                    value = true
                                } else {
                                    if (++i >= argv.length) {
                                        throw "Missing option for " + key
                                    }
                                    value = argv[i]
                                }
                            }
                            if (item.separator) {
                                item.value ||= []
                                item.value += (item.commas && value.contains(',')) ? value.split(',') : [value]
                            } else {
                                item.value = value
                            }
                        }
                    } else {
                        rest.append(arg)
                    }
                }
                validate()
            } catch (e) {
                if (template.onerror == 'throw') {
                    throw e
                } 
                if (!template.silent) {
                    App.log.error(e)
                }
                if (template.usage) {
                    template.usage()
                }
                if (template.onerror == 'exit') {
                    App.exit(1)
                }
            }
        }

        /*
            Validate options against the range of permissible values
         */
        private function validate(): Void {
            for (let [key,item]  in template.options) {
                if (item.value == undefined) {
                    continue
                }
                if (item.separator == Array) {
                    options[key] = []
                    for each (value in item.value) {
                        let v = validateItem(item, value)
                        options[key].push(v)
                    }
                } else if (item.separator is String) {
                    options[key] = ''
                    for each (value in item.value) {
                        if (options[key]) {
                            options[key] += item.separator + validateItem(item, value)
                        } else {
                            options[key] += validateItem(item, value)
                        }
                    }
                } else {
                    options[key] = validateItem(item, item.value)
                }
            }
        }

        private function validateItem(item, value): Object {
            if (item.range === Number) {
                if (value) {
                    if (!(value is Number)) {
                        if (value.toString().match(/^\d+$/)) {
                            value = value cast Number
                        } else { 
                           throw "Option \"" + key + "\" must be a number"
                        }
                    }
                } else {
                    value = 0
                }
            } else if (item.range === Boolean) {
                if (value is Boolean) {
                    value = value.toString()
                } else {
                    value = value.toString().toLowerCase()
                }
                if (value != "true" && value != "false") {
                   throw 'Option "' + key + '" must be true or false'
                }
                value = value cast Boolean

            } else if (item.range === String) {
                value = value.toString();

            } else if (item.range === Path) {
                value = Path(value)

            } else if (item.range is RegExp) {
                value = value.toString()
                if (!value.match(item.range)) {
                    throw 'Option "' + key + '" has bad value: "' + value + '"'
                }

            } else if (item.range is Array) {
                let ok = false
                for each (v in item.range) {
                    if (value == v) {
                        ok = true
                        break
                    }
                }
                if (! ok) {
                    throw 'Option "' + key + '" has bad value: "' + value + '"'
                }
            } else {
                if (item.range && value != item.range) {
                    throw 'Option "' + key + '" has bad value: "' + value + '"'
                }
            }
            return value
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Array.es"
 */
/************************************************************************/

/**
    Array.es - Array class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Arrays provide a growable, integer indexed, in-memory store for objects.  An array can be treated as a 
        stack (FIFO or LIFO) or a list (ordered).  Insertions can be done at the beginning or end of the stack or at an 
        indexed location within a list.
        @stability evolving
     */
    dynamic class Array {

        use default namespace public

        /**
            Create a new array.
            @param values Initialization values. The constructor allows three forms:
            <ul>
                <li>Array()</li>
                <li>Array(size: Number)</li>
                <li>Array(elt: Object, ...args)</li>
            </ul>
         */
        native function Array(...values)

        /**
            Append an item to the array.
            @param obj Object to add to the array 
            @return The array itself.
            @spec ejs
         */
        native function append(obj: Object): Array

        /**
            Clear an array. Remove all elements of the array.
            @spec ejs
         */
        native function clear() : Void

        /**
            Clone the array and all its elements.
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            @return A new array
            @spec ejs
         */
        override native function clone(deep: Boolean = true) : Array

        /**
            Compact an array. Remove all null elements.
            @return Modified original array
            @spec ejs
         */
        native function compact() : Array

        /**
            Concatenate the supplied elements with the array to create a new array. If any arguments specify an 
            array, their elements are concatenated. This is a one level deep copy.
            @param args A variable length set of values of any data type.
            @return A new array of the combined elements.
         */
        native function concat(...args): Array 

        /**
            See if the array contains an element using strict equality "===". This call searches from the start of the 
            array for the specified element. Note that some types are not interned like strings and so contains may
            return false if a different object with the same value is contained in the array. This can happen with Path
            types.
            @param element The object to search for.
            @return True if the element is found. Otherwise false.
            @spec ejs
         */
        function contains(element: Object): Boolean
            indexOf(element) >= 0

        /**
            Determine if all elements match.
            Iterate over every element in the array and determine if the matching function is true for all elements. 
            This method is lazy and will cease iterating when an unsuccessful match is found. The checker is called 
            with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function always returns true.
         */
        function every(match: Function): Boolean {
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    return false
                }
            }
            return true
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The match is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            This method is identical to the @findAll method.
            @param match Matching function
            @return Returns a new array containing all matching elements.
         */
        function filter(match: Function): Array
            findAll(match)

        /**
            Find the first matching element.
            Iterate over elements in the object and select the first element for which the matching function returns true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return The matched item
            @spec ejs
         */
        function find(match: Function): Object? {
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return this[i]
                }
            }
            return null
        }

        /**
            Find all matching elements.
            Iterate over all elements in the object and find all elements for which the matching function is true.
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return Returns an array containing all matching elements.
            @spec ejs
         */
        function findAll(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Transform all elements.
            Iterate over the elements in the array and transform all elements by applying the transform function. 
            The matching function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param callback Transforming function
            @param thisObj Object to use for the "this" value when invoking the callback
            @return Returns the original (transformed) array.
         */
        function forEach(callback: Function, thisObj: Object? = null): Array {
            if (thisObj) {
                for (let i: Number in this) {
                    callback.call(thisObj, this[i], i, this)
                }
            } else {
                for (let i: Number in this) {
                    callback(this[i], i, this)
                }
            }
            return this
        }

        /**
            Iterator for this array to be used by "for (v in array)"
         */
        override iterator native function get(): Iterator

        /**
            Iterator for this array to be used by "for each (v in array)"
         */
        override iterator native function getValues(): Iterator

        /**
            Search for an item using strict equality "===". This call searches from the start of the array for 
            the specified element.  
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to zero). If the index 
                is negative, it is taken as an offset from the end of the array. If the calculated index is less than 
                zero the entire array is searched. If the index is greater than the length of the array, -1 is returned.
            @return The items index into the array if found, otherwise -1.
         */
        native function indexOf(element: Object, startIndex: Number = 0): Number

        /**
            Insert elements. Insert elements at the specified position. The insertion occurs before the element at the 
                specified position. Negative indicies will measure from the end so that -1 will specify the last element.  
                Indicies greater than the array length will append to the end. Indicies before the first position will
                insert at the start.
            @param pos Where in the array to insert the objects.
            @param ...args Arguments are aggregated and passed to the method in an array.
            @return The original array.
            @spec ejs
         */
        native function insert(pos: Number, ...args): Array

        /**
            Convert the array into a string.
            Joins the elements in the array into a single string by converting each element to a string and then 
            concatenating the strings inserting a separator between each.
            @param sep Element separator.
            @return A string.
         */
        native function join(sep: String = ""): String

        /**
            Find an item searching from the end of the array.
            Search for an item using strict equality "===". This call searches from the end of the array for the given 
            element.
            @param element The object to search for.
            @param startIndex Where in the array to start searching for the object (Defaults to the array's length).
                If the index is negative, it is taken as an offset from the end of the array. If the calculated index 
                is less than zero, -1 is returned. If the index is greater or equal to the length of the array, the
                whole array will be searched.
            @return The items index into the array if found, otherwise -1.
         */
        native function lastIndexOf(element: Object, startIndex: Number = -1): Number

        /**
            Length of an array.
         */
        native function get length(): Number

        /**
            Set the length of an array. The array will be truncated if required. If the new length is greater then 
            the old length, new elements will be created as required and set to undefined. If the new length is less
            than 0 the length is set to 0.
            @param value The new length
         */
        native function set length(value: Number): Void

        /**
            Call the mapper on each array element in increasing index order and return a new array with the values returned 
            from the mapper. The mapper function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
         */
        function map(mapper: Function): Array {
            var result: Array = clone()
            result.transform(mapper)
            return result
        }

        /**
            Remove and return the last value in the array.
            @return The last element in the array. Returns undefined if the array is empty
         */
        native function pop(): Object?

        /**
            Append items to the end of the array.
            @param items Items to add to the array.
            @return The new length of the array.
         */
        native function push(...items): Number 

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            left to right. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduce(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = 0
                if (initial) {
                    previous = callback(initial, this[i++], 0, this)
                }
                for (; i < length; i++) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Reduce array elements.
            Apply a callback function against two values of the array and reduce to a single value. Traversal is from
            right to left. The first time the callback is called, previous will be set to the first value and current
            will be set to the second value. If an $initial parameter is provided, then previous will be set to initial
            and current will be set to the first value.
            The callback is called with the following signature:
                function callback(previous, current, index: Number, array: Array): Object
            @param callback Callback function
            @param initial Initial value to use in the reduction
            @return Returns a new array containing all matching elements.
         */
        function reduceRight(callback: Function, initial = null): Object {
            let previous
            if (length > 0) {
                let i = length - 1
                if (initial) {
                    previous = callback(initial, this[i--], 0, this)
                }
                for (; i >= 0; i--) {
                    previous = callback(previous, this[i], i, this)
                }
            }
            return previous
        }

        /**
            Find non-matching elements. Iterate over all elements in the array and select all elements for which 
            the filter function returns false. The matching function is called with the following signature:
          
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
          
            @param match Matching function
            @return A new array of non-matching elements.
            @spec ejs
         */
        function reject(match: Function): Array {
            var result: Array = new Array
            for (let i: Number in this) {
                if (!match(this[i], i, this)) {
                    result.append(this[i])
                }
            }
            return result
        }

        /**
            Remove elements. Remove the elements from $start to $end inclusive. The elements are removed and not just set 
            to undefined as the delete operator will do. Indicies are renumbered. NOTE: this routine delegates to splice.
            @param start Numeric index of the first element to remove. Negative indices measure from the end of the array.
            -1 is the last element.
            @param end Numeric index of the last element to remove
            @spec ejs
         */
        function remove(start: Number, end: Number = -1): Void {
            if (start < 0) {
                start += length
            }
            if (end < 0) {
                end += length
            }
            splice(start, end - start + 1)
        }

        /**
            Remove specified elements from the array. The elements are removed and not just set 
            to undefined as the delete operator will do. Indicies are renumbered. 
            @param elts List of elements to remove.
            @spec ejs
         */
        native function removeElements(...elts): Void

        /**
            Reverse the order of the objects in the array. The elements are reversed in the original array.
            @return A reference to the array.
         */
        native function reverse(): Array 

        /**
            Remove and return the first element in the array.
            @returns the previous first element in the array.
         */
        native function shift(): Object 

        /**
            Create a new array subset by taking a slice from an array.
            @param start The array index at which to begin. Negative indicies will measure from the end so that -1 will 
                specify the last element. If start is greater than or equal to end, the call returns an empty array.
            @param end The array index at which to end. This is one beyond the index of the last element to extract. If 
                end is negative, it is measured from the end of the array, so use -1 to mean up to but not including the 
                last element of the array.
            @param step Slice every step (nth) element. The step value may be negative.
            @return A new array that is a subset of the original array.
         */
        native function slice(start: Number, end: Number = -1, step: Number = 1): Array 

        /**
            Determine if some elements match.
            Iterate over all element in the array and determine if the matching function is true for at least one element. 
            This method is lazy and will cease iterating when a successful match is found.
            The match function is called with the following signature:
                function match(element: Object, elementIndex: Number, arr: Array): Boolean
            @param match Matching function
            @return True if the match function ever is true.
         */
        function some(match: Function): Boolean {
            var result: Array = new Array
            for (let i: Number in this) {
                if (match(this[i], i, this)) {
                    return true
                }
            }
            return false
        }

        /**
            Sort the array. The array is sorted in lexical order. A compare function may be supplied.
            @param compare Function to use to compare. A null comparator will use a text compare. The compare signature is:
                function comparator (array: Array, index1: Number, index2: Number): Number
                The comparison function should return 0 if the items are equal, -1 if the item at index1 is less and should
                return 1 otherwise.
            @param order If order is >= 0, then an ascending lexical order is used. Otherwise descending.
            @return the sorted array reference
            @spec ejs Added the order argument.
         */
        native function sort(compare: Function? = null, order: Number = 1): Array 

        /**
            Insert, remove or replace array elements. Splice modifies an array in place. 
            @param start The array index at which the insertion or deleteion will begin. Negative indicies will measure 
                from the end so that -1 will specify the last element.  
            @param deleteCount Number of elements to delete. If omitted, splice will delete all elements from the 
                start argument to the end.  
            @param values The array elements to add.
            @return Array containing the removed elements.
         */
        native function splice(start: Number, deleteCount: Number, ...values): Array 

        /**
            Convert an array to an equivalent JSON encoding.
            @return This function returns an array literal string.
            @throws TypeError If the array could not be converted to a string.
            NOTE: currently using Object.toJSON for this capability
         */ 
        #FUTURE
        override native function toJSON(): String

        # FUTURE
        override native function toLocaleString(): String 

        /**
            Convert the array to a single string each member of the array has toString called on it and the resulting 
            strings are concatenated.
            @return A string
         */
        override native function toString(): String 

        /**
            Transform all elements.
            Iterate over all elements in the object and transform the elements by applying the transform function. 
            This modifies the object elements in-situ. The transform function is called with the following signature:
                function mapper(element: Object, elementIndex: Number, arr: Array): Object
            @param mapper Transforming function
            @spec ejs
         */
        function transform(mapper: Function): Array {
            for (let i: Number in this) {
                this[i] = mapper(this[i], i, this);
            }
            return this
        }

        /**
            Remove duplicate elements and make the array unique. Duplicates are detected by using "==" (ie. content 
                equality, not strict equality).
            @return The original array with unique elements
            @spec ejs
         */
        native function unique(): Array

        /**
            Insert the items at the front of the array.
            @param items to insert
            @return Returns the array reference
         */
        native function unshift(...items): Array

        /**
            Array intersection. Return the elements that appear in both arrays. 
            @param arr The array to join.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function & (arr: Array): Array

        /**
            Append. Appends elements to an array.
            @param elements The array to add append.
            @return The modified original array.
            @spec ejs
         */
        # DOC_ONLY
        native function << (elements: Array): Array

        //  MOB - need a function equivalent for this
        /**
            Array subtraction. Remove any items that appear in the supplied array.
            @param arr The array to remove.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function - (arr: Array): Array

        /**
            Array union. Return the union of two arrays. 
            @param arr The array to join.
            @return A new array
            @spec ejs
         */
        # DOC_ONLY
        native function | (arr: Array): Array

        /**
            Concatenate two arrays. 
            @param arr The array to add.
            @return A new array.
            @spec ejs
         */
        # DOC_ONLY
        native function + (arr: Array): Array
    }
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

/************************************************************************/
/*
    Start of file "src/core/BinaryStream.es"
 */
/************************************************************************/

/*
    BinaryStream.es -- BinaryStream class. This class is a filter or endpoint stream to encode and decode binary types.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        BinaryStreams encode and decode various objects onto streams. A BinaryStream may be stacked atop an underlying stream
        provider such as ByteArray, File, Http or Socket. The underlying stream must be in sync mode.
        @spec ejs
        @stability evolving
     */
    class BinaryStream implements Stream {

        use default namespace public

        /** 
            Big endian byte order 
         */
        static const BigEndian: Number = 0

        /** 
            Little endian byte order 
         */
        static const LittleEndian: Number = 1

        /* 
            Data input and output buffers. The buffers are used to marshall the data for encoding and decoding. The inbuf 
            also hold excess input data. The outbuf is only used to encode data -- no buffering occurs.
         */
        private var inbuf: ByteArray
        private var outbuf: ByteArray
        private var nextStream: Stream
        private var emitter: Emitter

        /** 
            Create a new BinaryStream
            @param stream stream to stack upon.
         */
        function BinaryStream(stream: Stream) {
            if (!stream) {
                throw new ArgError("Must supply a Stream to connect with")
            }
            nextStream = stream
            inbuf = new ByteArray
            outbuf = new ByteArray

            inbuf.on("writable", function (event: String, buffer: ByteArray) {
                nextStream.read(buffer)
            })

            outbuf.on("readable", function (event: String, buffer: ByteArray) {
                count = nextStream.write(buffer)
                buffer.readPosition += count
                buffer.reset()
            })
        }

        /** 
            @duplicate Stream.async 
         */
        function get async(): Boolean
            false

        /** 
            @duplicate Stream.async 
         */
        function set async(enable: Boolean): Void {
            throw "async mode not implemented for BinaryStreams"
        }

        /** 
            @duplicate Stream.close 
         */
        function close(): void
            nextStream.close()

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String {
            return inbuf.enc
        }

        /** 
            @duplicate BinaryStream.encoding
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            inbuf.encoding = enc
            outbuf.encoding = enc
        }

        /** 
            Current byte ordering. Set to either LittleEndian or BigEndian.
         */
        function get endian(): Number
            inbuf.endian

        /** 
            Set the system encoding to little or big endian.
            @param value Set to true for little endian encoding or false for big endian.
         */
        function set endian(value: Number): Void {
            if (value != BigEndian && value != LittleEndian) {
                throw new ArgError("Bad endian value")
            }
            inbuf.endian = value
            outbuf.endian = value
        }

        /**
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.READ) 
                inbuf.flush(Stream.READ)
            if (dir & Stream.WRITE) 
                outbuf.flush(Stream.WRITE)
            if (!(nextStream is ByteArray)) {
                /* Don't flush loopback bytearrays */
                nextStream.flush(dir)
            }
        }

        /** 
            The number of bytes available to read without blocking. This is the number of bytes internally buffered
            in the binary stream and does not include any data buffered downstream.
            @return the number of available bytes
         */
        function get length(): Number
            inbuf.length

        /** 
            @duplicate Stream.on 
         */
        function on(name, observer: Function): BinaryStream {
            emitter ||= new Emitter
            emitter.on(name, observer)
            return this
        }

        /** 
            @duplicate Stream.read
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?
            inbuf.read(buffer, offset, count)

        /** 
            Read a boolean from the stream.
            @returns a boolean. Returns null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readBoolean(): Boolean?
            inbuf.readBoolean()

        /** 
            Read a byte from the stream.
            @returns a byte. Returns -1 on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readByte(): Number?
            inbuf.readByte()

        /** 
            Read a date from the stream.
            @returns a date
            @throws IOError if an I/O error occurs or premature EOF
         */
        function readDate(): Date?
            inbuf.readDate()

        /** 
            Read a double from the stream. The data will be decoded according to the encoding property.
            @returns a double
            @throws IOError if an I/O error occurs or premature EOF
         */
        function readDouble(): Double?
            inbuf.readDouble()

        /** 
            Read a 32-bit integer from the stream. The data will be decoded according to the encoding property.
            @returns an 32-bitinteger
            @throws IOError if an I/O error occurs or premature EOF
         */
        function readInteger(): Number?
            inbuf.readInteger()

        /** 
            Read a 64-bit long from the stream.The data will be decoded according to the encoding property.
            @returns a 64-bit long number
            @throws IOError if an I/O error occurs or premature EOF
         */
        function readLong(): Number?
            inbuf.readInteger()

        /** 
            Read a UTF-8 string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string
            @throws IOError if an I/O error occurs or premature EOF.
         */
        function readString(count: Number = -1): String ?
            inbuf.readString(count)

        /** 
            Read an XML document from the stream. This assumes the XML document will be the only data until EOF.
            @returns an XML document
            @throws IOError if an I/O error occurs or premature EOF
         */
        function readXML(): XML? {
            var data: String = ""
            while (1) {
                var s: String? = inbuf.readString()
                if (s == null && data.length == 0) {
                    return null
                }
                if (s.length == 0) {
                    break
                }
                data += s
            }
            return new XML(data)
        }

        /** 
            @duplicate Stream.off 
         */
        function off(name, observer: Function): Void {
            if (emitter) emitter.off(name, observer)
        }

        /** 
            Return the space available for write data. This call can be used to prevent write from blocking or 
            doing partial writes. If it cannot be determined how much room is available, this call will return null.
            @return The number of bytes that can be written without blocking or null if it cannot be determined.
         */
        function room(): Number
            outbuf.room()
       
        /** 
            Write data to the stream. Write intelligently encodes various data types onto the stream and will encode 
            data in a portable cross-platform manner according to the setting of the $endian property. If data is an 
            array, each element of the array will be written. 
            @param items Data items to write. The ByteStream class intelligently encodes various data types according 
            to the current setting of the $endian property. 
            @returns The total number of bytes that were written.
            @throws IOError if there is an I/O error.
            @event 
         */
        function write(...items): Number {
            let count: Number = 0
            for each (i in items) {
                count += outbuf.write(i)
            }
            return count
        }

        /** 
            Write a byte to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeByte(data: Number): Void 
            outbuf.writeByte(outbuf)

        /** 
            Write a short to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeShort(data: Number): Void
            outbuf.writeShort(data)

        /** 
            Write a double to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeDouble(data: Number): Void
            outbuf.writeDouble(data)

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeInteger(data: Number): Void
            outbuf.writeInteger(data)

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer.
            @param data Data to write
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        function writeLong(data: Number): Void
            outbuf.writeLong(data)
    }
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

/************************************************************************/
/*
    Start of file "src/core/Block.es"
 */
/************************************************************************/

/*
    Block.es -- Block scope class used internally by the VM.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Block type is used to represent program block scope. It is used internally and should not be 
        instantiated by user programs.
        @hide
        @stability stable
        @spec ejs
     */
    dynamic final class Block { }
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

/************************************************************************/
/*
    Start of file "src/core/Boolean.es"
 */
/************************************************************************/

/*
    Boolean.es -- Boolean class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Boolean class from which to create true and false values.
        @stability stable
     */
    final class Boolean {
        /**
            Boolean constructor. Construct a Boolean object and initialize its value. Since Boolean values are 
            immutable, this constructor will return a reference to either the "true" or "false" values.
            @param value Optional value to use in creating the Boolean object. If the value is omitted or 0, -1, NaN,
                false, null, undefined or the empty string, then the object will be created and set to false.
         */
        native function Boolean(value: Boolean? = false)
    }
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

/************************************************************************/
/*
    Start of file "src/core/ByteArray.es"
 */
/************************************************************************/

/*
    ByteArray.es - ByteArray class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    // use strict
    //  MOB - add indexOf(byteValue)

    /** 
        ByteArrays provide a resizable, integer indexed, in-memory store for bytes. ByteArrays can be used as a simple 
        array type to store and encode data as bytes or they can be used as buffered loop-back Streams.
        
        When used as a simple byte array, the ByteArray class offers a low level set of methods to insert and 
        extract bytes. The index operator [] can be used to access individual bytes and the copyIn and copyOut methods 
        can be used to get and put blocks of data. In this mode, the $readPosition and $writePosition properties are ignored.

        Access to the byte array is from index zero up to the size defined by the $size property. When constructed, 
        the ByteArray can be designated as resizable, in which case the initial size will grow as required to accomodate 
        data and the $size property will be updated accordingly.
        
        ByteArrays provide additional write methods to store data at the location specified by the $writePosition 
        property and read methods to read from the $readPosition property. The $length property indicates how much 
        data is available between the read and write position pointers. The $reset method can reset the pointers to 
        the start of the array.  When used with for/in, ByteArrays will iterate or enumerate over the available 
        data between the read and write pointers.

        If numeric values are read or written, they will be coded according to the value of the endian property 
        which can be set to either LittleEndian or BigEndian.  If strings values are read or written, they will 
        be encoded according to the value of the character set $encoding property.
        
        When used as loop-back streams, data written to ByteArrays is immediately available for reading. 
        ByteArrays can be run in sync or async mode. ByteArrays will issue events for key state transitions such as 
        close, EOF, readable and writable events. All event observers are called with the following signature:
            function callback(event: String, ba: ByteArray): Void
        @spec ejs
        @stability evolving
     */
    final class ByteArray implements Stream {

        use default namespace public

        /** 
            Little endian byte order used for the $endian property
         */
        static const LittleEndian: Number = 0

        /** 
            Big endian byte order used for the $endian property
         */
        static const BigEndian: Number = 1

        /** 
            Create a new array.
            @param size The initial size of the byte array. If not supplied a default buffer size will be used which is
            typically 4K or larger.
            @param resizable Set to true to automatically grow the array as required to fit written data.
         */
        native function ByteArray(size: Number = -1, resizable: Boolean = true)

        /** @duplicate Stream.async */
        native function get async(): Boolean

        /** @duplicate Stream.async */
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        /** 
            Compact available data down and adjust the read/write positions accordingly. This sets the read pointer 
            to the zero index and adjusts the write pointer by the corresponding amount.
         */
        native function compact(): Void

        /** 
            Compress the array contents.
         */
        # FUTURE
        native function compress(): Void

        /**
            Test if the buffer contains a pattern
         */
        # FUTURE
        native function contains(pattern): Boolean

        /** 
            Copy data into the array. This is a low-level data copy routine that does not update read and write positions.
            Data is written at the destOffset index. This call does not issue events unless required to make room
            for the incoming data ("readable" event).
            @param destOffset Index in the destination byte array to copy the data to
            @param src Source byte array containing the data elements to copy
            @param srcOffset Location in the source buffer from which to copy the data. Defaults to the start.
            @param count Number of bytes to copy. Set to -1 to read all the src buffer.
            @return the number of bytes written into the array. If the array is not resizable and there is insufficient
                room, this may be less than the requested amount.
         */
        native function copyIn(destOffset: Number, src: ByteArray, srcOffset: Number = 0, count: Number = -1): Number

        /** 
            Copy data from the array. Data is copied from the srcOffset pointer. This call does not update the 
                read and write positions. This call does not issue events.
            @param srcOffset Location in the source array from which to copy data.
            @param dest Destination byte array
            @param destOffset Location in the destination array to copy the data. Defaults to the start.
            @param count Number of bytes to read. Set to -1 to read all available data.
            @returns the count of bytes read. Returns null on EOF.
         */
        native function copyOut(srcOffset: Number, dest: ByteArray, destOffset: Number = 0, count: Number = -1): Number

        /** 
            Current encoding scheme for serializing strings. The default encoding is "utf-8". Set to "" for no encoding.
            If no encoding, string character points are stored as a pairs of two byte in little-endian format.
         */
        function get encoding(): String 
            "utf-8"

        /** 
            Current encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
         */
        native function get endian(): Number

        /** 
            Current byte ordering for storing and retrieving numbers. Set to either LittleEndian or BigEndian
            @param value Set to true for little endian ordering or false for big endian.
         */
        native function set endian(value: Number): Void

        /** 
            Flush (discard) the data in the byte array and reset the read and write positions. 
            This call may block if the stream is in sync mode.
            @param dir The dir parameter is Ignored. Flushing a ByteArray in either direction the same effect of 
                discarding all buffered data and resetting the read and write positions -- so this argument is ignored. 
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /**
            Is the ByteArray is resizable.
         */
        native function get resizable(): Boolean

        /** 
            Iterator for this array to be used by "for (v in array)". This will return array indicies.
         */
        override iterator native function get(): Iterator

        /** 
            Iterator for this array to be used by "for each (v in array)". This will return read data in the array.
         */
        override iterator native function getValues(): Iterator

        /** 
            Number of bytes that are currently available for reading. This consists of the bytes available
            from the current $readPosition up to the current $writePosition. To get the total size of the ByteArray
            use $size.
         */
        native function get length(): Number

        /**
            An MD5 checksum for the buffer contents
         */
        function get MD5(): String {
            //  OPT - don't close. Just safe read/write position
            let buf: ByteArray = this.clone()
            return md5(buf.readString())
        }

        /** @duplicate Stream.off */
        native function off(name: Object, observer: Function): Void

        /** @duplicate Stream.on */
        native function on(name: Object, observer: Function): ByteArray

        /** 
            @duplicate Stream.read
            Data is read from the current read $position pointer toward the current $writePosition. 
            This byte array's $readPosition is updated. If offset is < 0, then data is copied to the destination buffer's 
            $writePosition and the destination buffer's $writePosition is also updated. If the offset is >= 0, the 
            read position is set to the specified offset and data is stored at this offset. The write position is set to
            one past the last byte read.
         */
        native function read(buffer: ByteArray!, offset: Number = 0, count: Number = -1): Number?

        /** 
            Read a boolean from the array. Data is read from the current read $position pointer.
            If insufficient data, a "writable" event will be issued indicating that the byte array is writable. This enables 
            observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF.
            @returns a boolean or null on EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readBoolean(): Boolean?

        /** 
            Read a byte from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readByte(): Number?

        /** 
            Read a date from the array. Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readDate(): Date?

        /** 
            Read a double from the array. The data will be decoded according to the endian property. Data is read 
            from the current read $position pointer. If insufficient data, a "write" event will be issued indicating 
            that the byte array is writable. This enables observers to write data into the byte array. If there is 
            no data available, the call will return return null indicating EOF.
            @returns a double or null on EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readDouble(): Number?

        /** 
            Read an 32-bit integer from the array. The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readInteger(): Number?

        /** 
            Read a 64-bit long from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readLong(): Number?

        /** 
            Current read position offset
         */
        native function get readPosition(): Number?

        /** 
            Set the current read position offset
            @param position The new read position
         */
        native function set readPosition(position: Number): Void

        /** 
            Read a 16-bit short integer from the array.The data will be decoded according to the endian property.
            Data is read from the current read $position pointer.
            If insufficient data, a "write" event will be issued indicating that the byte array is 
            writable.  This enables observers to write data into the byte array.  If there is no data available, the call 
            will return return null indicating EOF. If there is insufficient data 
            @returns a short int or null on EOF.
            @throws IOError if an I/O error occurs or premature EOF.
         */
        native function readShort(): Number?

        /** 
            Read a data from the array as a string. Read data from the $readPosition to a string up to the $writePosition,
            but not more than count characters. If insufficient data, a "writable" event will be issued indicating that 
            the byte array is writable. This enables observers to write data into the byte array.  If there is no data 
            available, the call will return return null indicating EOF. If there is insufficient data 
            @param count of bytes to read. If -1, convert the data up to the $writePosition.
            @returns a string or null on EOF.
            @throws IOError if an I/O error occurs or a premature EOF.
         */
        native function readString(count: Number = -1): String?

        /**
            Read an XML document from the array. Data is read from the current read $position pointer.
            @returns an XML document
            @throws IOError if an I/O error occurs or a premature end of file.
         */
        function readXML(): XML?
            XML(readString())

        /** 
            Reset the read and $writePosition pointers if there is no available data.
            This is used to rewind the read/write pointers to maximize available buffer space.
         */
        native function reset(): Void

        /** 
            Number of data bytes that the array can store from the $writePosition till the end of the array.
         */
        native function get room(): Number 

        /** 
            Size of the byte array. This is not the amount of read or write data, but is the size of the total 
            array storage. Use $length to get the amount of data between the read and write positions.
         */
        native function get size(): Number 

        /** 
            Convert the data in the byte array between the $readPosition and writePosition.
            @return A string
         */
        override native function toString(): String 

        /** 
            Uncompress the array
         */
        # FUTURE
        native function uncompress(): Void

        /** 
            Write data to the ByteArray.
            Data is written to the current $writePosition. If the data argument is itself a ByteArray, the available data 
            from the byte array will be copied, ie. the $data byte array will not have its readPosition adjusted. If the 
            byte array is resizable, the underlying data storage will grow to accomodate written data. If the data will not
            fit in the ByteArray, the call may return having only written a portion of the data.
            When strings are written, they are not null terminated.
            @duplicate Stream.write
         */
        native function write(...data): Number

//  MOB -- should these routines return the number of bytes written?
//  MOB -- function write(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number

        /** 
            Write a byte to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeByte(data: Number): Void

        /** 
            Write a short to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeShort(data: Number): Void

        /** 
            Write a double to the array. Data is written to the current write $position pointer which is then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeDouble(data: Number): Void

        /** 
            Write a 32-bit integer to the array. Data is written to the current write $position pointer which is 
                then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeInteger(data: Number): Void

        /** 
            Write a 64 bit long integer to the array. Data is written to the current write $position pointer which is 
            then incremented.
            See $write for details about sync, async modes and event handling.
            @param data Data to write
            @throws IOError if an I/O error occurs or if the stream cannot absorb all the data.
            @event readable Issued when data is written and a consumer can read without blocking.
         */
        native function writeLong(data: Number): Void

        /** 
            Current $writePosition offset.
         */
        native function get writePosition(): Number

        /** 
            Set the current write position offset.
            @param position the new write  position
         */
        native function set writePosition(position: Number): Void

        /** 
            Input callback function when read data is required. The input callback should write to the supplied buffer.
            @hide
            @deprecated 1.0.0B3
        */
        # Config.Legacy
        function get input(): Function? { return null; }

        /**  
            @hide
            @deprecated 1.0.0B3
         */
        # Config.Legacy
        function set input(callback: Function): Void {
            on("writable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
        }

        /**  
            Output function to process (output) data. The output callback should read from the supplied buffer.
            @param callback Function to invoke when the byte array is full or flush() is called.
                function outputCallback(buffer: ByteArray): Number
            @hide
            @deprecated 1.0.0B3
         */
        # Config.Legacy
        function get output(): Function? { return null; } 

        # Config.Legacy
        function set output(callback: Function): Void {
            on("readable", function(event: String, ba: ByteArray): Void {
                callback(ba)
            })
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Cache.es"
 */
/************************************************************************/

/*
    Cache.es -- Cache class providing key/value storage.

    Usage Tutorial:

        cache = new Cache("local", {lifespan: 86400, timeout: 5000, memory: 200000000, keys: 10000000})

        session = cache.read(key)
        cache.write(key, value, {lifespan: 3600})
        cache.remove(key)
        cache.destroy()

        cache = new Cache("memcached", {addresses: ["127.0.0.1:11211"], debug: false})
        cache = new Cache("file", {dir: "/tmp"})

        Ejs internally uses the key naming convention:  ::module::key
 */
module ejs {

    /**
        Cache meta class to manage in-memory storage of key-value data. The Cache class provides an abstraction over
        various in-memory and disk-based caching cache backends.
        @stability prototype
     */
    class Cache {
        use default namespace public

        private var adapter: Object?

        /**
            Cache constructor.
            @param adapter Adapter for the cache cache. E.g. "local". The Local cache is the only currently supported
                cache backend. 
            @param options Adapter options. The common options are described below, other options are passed through
            to the relevant caching backend.
            @option lifespan Default lifespan for key values in seconds.
            @option resolution Time in milliseconds to check for expired expired keys
            @option timeout Timeout on cache I/O operations
            @option trace Trace I/O operations for debug
            @option module Module name containing the cache connector class. This is a bare module name without ".mod"
                or any leading path. If module is not present, a module name is derrived using "ejs.cache" + adapter.
            @option class Class name containing the cache backend. If the class property is not present, the 
                class is derived from the adapter name with "Cache" appended. The first letter of the adapter is converted
                to uppercase. For example, if the adapter was "mem", the class would be inferred to be "MemCache".
         */
        function Cache(adapter: String? = null, options: Object = {}) {
            let adapterClass, modname
            if (adapter == null || adapter == "local") {
                options = blend({shared: true}, options)
                adapter = "local"
                modname = "ejs"
                adapterClass = "LocalCache"
            } else {
                adapterClass ||= options["class"] || (adapter.toPascal() + "Cache")
                modname ||= options.module || ("ejs.cache." + adapter)
                if (!global.modname::[adapterClass]) {
                    load(modname + ".mod", {reload: false})
                    if (!global.modname::[adapterClass]) {
                        throw "Cannot find cache adapter: \"" + modname + "::" + adapter + "\""
                    }
                }
            }
            this.adapter = new global.modname::[adapterClass](options)
        }

        /**
            Destroy the cache
         */
        function destroy(): Void
            adapter.destroy()

        //  MOB - bit inconsistent that expires takes only a date and not a lifespan too
        /**
            Set a new expire date for a key
            @param key Key to modify
            @param when Date at which to expire the data. Set to null to never expire.
            @return True if the key's expiry can be updated. 
         */
        function expire(key: String, when: Date?): Boolean
            adapter.expire(key, when)

        /**
            Increment a key's value by a given amount. This operation is atomic.
            @param key Key value to read.
            @param amount Amount by which to increment the value. This amount can be negative to achieve a decrement.
            @return The new key value. If the key does not exist, it is initialized to the amount value.
         */ 
        function inc(key: String, amount: Number = 1): Number
            adapter.inc(key, amount)

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash. Depending on the cache backend in-use, the limits object may have
                some of the following properties. Consult the documentation for the actual cache backend for which properties
                are supported by the backend.
            @option keys Maximum number of keys in the cache. Set to zero for no limit.
            @option lifespan Default time in seconds to preserve key data. Set to zero for no timeout.
            @option memory Total memory to allocate for cache keys and data. Set to zero for no limit.
            @option retries Maximum number of times to retry I/O operations with cache backends.
            @option timeout Maximum time to transact I/O operations with cache backends. Set to zero for no timeout.
            @see setLimits
          */
        function get limits(): Object
            adapter.limits

        /**
            Read a key. 
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as a string or if the options parameter 
                specified "version == true", return an object with the properties "data" for the key data and 
                "version" for the CAS version identifier.
         */
        function read(key: String, options: Object = null): String?
            adapter.read(key, options)

        /**
            Read a key and return an object 
            This will read the data for a key and then deserialize. This assumes that $writeObj was used to store the
            key value.
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as an object.
         */
        function readObj(key: String, options: Object = null): Object? {
            let data = adapter.read(key, options)
            if (data) {
                return deserialize(data)
            }
            return null
        }

        /**
            Remove the key and associated value from the cache
            @param key Key value to remove. If key is null, then all keys are removed.
            @return true if the key was removed
         */
        function remove(key: String): Boolean
            adapter.remove(key)

        /**
            Update the cache cache resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        function setLimits(limits: Object): Void
            adapter.setLimits(limits)

        /**
            Write the key and associated value to the cache. The value is written according to the optional mode option.  
            @param key Key to modify
            @param value String value to associate with the key
            @param options Options values
            @option lifespan Preservation time for the key in seconds.
            @option expire When to expire the key. Takes precedence over lifetime.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
            @option version Unique version identifier to be used for a conditional write. The write will only be 
                performed if the version id for the key has not changed. This implements an atomic compare and swap.
                See $read.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        function write(key: String, value: String, options: Object = null): Number?
            adapter.write(key, value, options)

        /**
            Write the key and associated object value to the cache. The object value is serialized using JSON notation and
            written according to the optional mode option.  
            @param key Key to modify
            @param value Object to associate with the key
            @param options Options values
            @option lifespan Preservation time for the key in seconds. Set to zero for never expire.
            @option expire When to expire the key. Takes precedence over lifetime.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
            @option version Unique version identifier to be used for a conditional write. The write will only be 
                performed if the version id for the key has not changed. This implements an atomic compare and swap.
                See $read.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        function writeObj(key: String, value: Object, options: Object = null): Number?
            adapter.write(key, serialize(value), options)
    }

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

/************************************************************************/
/*
    Start of file "src/core/Cmd.es"
 */
/************************************************************************/

/*
    Cmd.es - Cmd class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Cmd class supports invoking other programs as separate processes on the same system. 
        @spec ejs
     */
    class Cmd implements Stream {

        use default namespace public

        /**
            Cached command output
            @hide
         */
        private var _response: String
        private var _errorResponse: String

        /**
            Create an Cmd object. If a command line is provided, the command is immediately started.
            @param command The (optional) command line to initialize with. If a command line is provided, the start()
                method is automatically invoked after the command is constructed. The command may be either a string or
                an array of arguments. Using an array of args can simplify quoting if the args have embedded spaces or
                quotes.
            @param options. Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to false.
            @options timeout Number This is the default number of milliseconds for the command to complete.
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
         */
        native function Cmd(command: Object = null, options: Object = null)

        /**
            Close the connection to the command and free up all associated resources. It is not normally required to call 
            $close, but it can be useful to force a command termination. After calling close, the command status and
            unread response data are not accessible.
         */
        native function close(): Void 

        //  MOB - should be a function. Need flags MPR_CMD_EXACT_ENV support.
        /**
            Hash of environment strings to pass to the command.
         */
        native function get env(): Object
        native function set env(values: Object): Void

        /**
            Command error output data as a string. The first time this property is read, the error content will be read 
            and buffered.
         */
        function get error(): String {
            if (!_errorResponse) {
                _errorResponse = errorStream.toString()
            }
            return _errorResponse
        }

        /**
            The error stream object for the command's standard error output 
         */
        native function get errorStream(): Stream

        /** 
            Signal the end of writing data to the command. The finalize() call must be invoked to properly 
            signify the end of write data.
         */
        native function finalize(): Void 

        /**
            This call has no effect
            @duplicate Stream.flush
            @hide
         */
        native function flush(dir: Number = Stream.BOTH): Void

        /**
            Locate a program using the application PATH
            @param program Program to find
            @param search Optional additional search paths to use before using PATH
            @return Located path or null
         */
        static function locate(program: Path, search = []): Path? {
            search += App.getenv("PATH").split(App.SearchSeparator)
            for each (dir in App.getenv("PATH").split(App.SearchSeparator)) {
                let path = Path(dir).join(program)
                if (path.exists && !path.isDir) {
                    return path
                }
            }
            if (Config.OS == 'windows' || Config.OS == 'cygwin') {
                if (program.extension == '') {
                    for each (ext in ['exe', 'bat', 'cmd']) {
                        let path = locate(program.joinExt('.exe'))
                        if (path) {
                            return path;
                        }
                    }
                }
            }
            return null
        }

        /** 
            @duplicate Stream.on
            @event readable Issued when output data to read.
            @event writable Issued when the command can accept more write data.
            @event complete Issued when the command completes
            @event error Issued if the $error stream is readable.

            All events are called with the signature:
            function (event: String, cmd: Cmd): Void
         */
        native function on(name, observer: Function): Cmd

        /** 
            This call is not supported.
            @duplicate Stream.off
            @hide
         */
        native function off(name, observer: Function): Void

        /**
            Process ID of the the command. This is set to the process ID (PID) of the command. If no command has 
            started or the command has called $close, the call with throw an exception.
         */
        native function get pid(): Number

        /**
            @duplicate Stream.read
            If no observer has been defined via $on(), this call will block if there is no data to be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /**
            Read the data from the command output as a string. This reads from the command's standard output. 
            @param count of bytes to read. Returns the entire output data contents if count is -1.
            @returns a string of $count characters beginning at the start of the output data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String?

        /**
            Read the data from the command as an array of lines. This reads from the command's standard output.
            @param count of linese to read. Returns the entire output contents if count is -1.
            @returns a string containing count lines of data starting with the first line of output data
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array? {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /**
            Read the command output as an XML document. This reads from the command's standard output.
            @returns the output content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML?
            XML(readString())

        /**
            Command output data as a string. This is an alias for $readString() but it will cache the 
                output data and may be called multiple times. This reads from the command's standard output.
         */
        function get response(): String? {
            if (!_response) {
                _response = readString()
            }
            return _response
        }

        /**
            Start the command.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @param options Command options. Sames as options in $Cmd
            @throws IOError if the request was cannot be issued to the remote server.
         */
        native function start(cmdline: Object, options: Object = null): Void

        /**
            Get the command exit status. This command will block until the command has completed. Use wait(0) to 
            test if the command has completed.
            @return The command exit status
         */
        native function get status(): Number

        /**
            Stop the current command. After stopping, the command exit status and any response or error data are
            available for interrogation.
            @param signal Signal to send the the active process.
            @return True if the command is successfully stopped or the command has completed.
          */
        native function stop(signal: Number = 2): Boolean

        /**
            Default command timeout for wait(), read(), and blocking start(). If the timeout expires, the command is
            not killed. Rather any blocking calls will simply return. 
            This is the number of milliseconds for the call to complete.
         */
        native function get timeout(): Number
        native function set timeout(msec: Number): Void

        /**
            Wait for a command to complete. 
            @param timeout Time in milliseconds to wait for the command to complete. If unspecified, the $timeout propoperty
                value is used instead.
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /**
            @duplicate Stream.write
            Call finalize() to signify the end of write data. Failure to call finalize() may prevent some commands 
            from exiting.
         */
        native function write(...data): Number


        /* Static Helper Methods */

        /**
            Start a command in the background as a daemon.  The daemon command is detached and the application 
            continues immediately in the foreground. Note: No data can be written to the daemon's stdin.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @return The process ID. This PID can be used with kill().
         */
        static function daemon(cmdline: Object, options: Object = null): Number {
            let cmd = new Cmd
            options ||= {}
            cmd.start(cmdline, blend({detach: true}, options))
            cmd.finalize()
            return cmd.pid
        }

        /**
            Execute a new program by the current process. This Replaces the current program with another without
            creating a new process.
            @param cmdline Command line to use. The cmdline may be either a string or an array of strings.
            @param options Command options. Sames as options in $Cmd
            @throws IOError if the request was cannot be issued to the remote server.
         */
        native static function exec(cmdline: String? = null, options: Object = {}): Void

        /**
            Kill the specified process.
            @param pid Process ID of the process to kill
            @param signal If pid is greater than zero, the signal is sent to the process whoes ID is pid. If pid is
                zero, the process is tested but no signal is sent. 
            @return True if successful
            @throws IOError if the pid is invalid or if the requesting process does not have sufficient privilege to
                send the signal.
         */
        native static function kill(pid: Number, signal: Number = 2): Boolean

        /** 
            Kill all matching processes. This call enables selection of the processes to kill a pattern match over
            the processes command line. Note: this command does not throw exceptions if a matching process cannot be
            killed. Use kill() for reliable process execution.
            @param pattern of processes to kill. This can be a string name or a regular expression to match with.
            @param signal Signal number to send to the processes to kill. If the signal is not supplied, then the system
            default signal is sent (SIGTERM).
            @param preserve Optional set of process IDs to preserve
            @hide 
         */
        static function killall(pattern: Object, signal: Number = 15, ...preserve): Void {
            let cmd = new Cmd
            if (Config.OS == "windows" || Config.OS == "cygwin") {
                cmd.start('cmd /A /C "WMIC PROCESS get Processid,Commandline /format:csv"')
                for each (line in cmd.readLines()) {
                    let fields = line.trim().split(",")
                    let pid = fields.pop().trim()
                    let command = fields.slice(1).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        if (preserve.length == 0 || !preserve.find(function(e, index, arrr) { return e == pid })) {
                            // print("KILL " + pid + " pattern " + pattern + " signal " + signal)
                            try {
                                Cmd.kill(pid, signal)
                            } catch {}
                        }
                    }
                }
            } else {
               cmd.start("/bin/ps -e")
                for each (line in cmd.readLines()) {
                    let fields = line.split(/ +/g)
                    let pid = fields[0]
                    let command = fields.slice(3).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        if (preserve.length == 0 || !preserve.find(function(e, index, arrr) { return e == pid })) {
                            // print("KILL " + pid + " pattern " + pattern + " signal " + signal)
                            try {
                                Cmd.kill(pid, signal)
                            } catch {}
                        }
                    }
                }
            }
            cmd.close()
        }

        /** 
            Return a list of processes running on the system. This enables selection of processes by string pattern or by
            a regular expression against the entire process command line.
            @param pattern Pattern to use when selecting matching processes. This can be a string name or a regular 
            expression to match with. A string name will match if the processes's command line contains the pattern as
            a sub-string.
            @return An array of matching processes. Each array entry is an object with properties "pid" and "command".
            @hide 
         */
        static function ps(pattern: Object = ""): Array {
            let result = []
            let cmd = new Cmd
            if (Config.OS == "windows" || Config.OS == "cygwin") {
                cmd.start('cmd /A /C "WMIC PROCESS get Processid,Commandline /format:csv"')
                for each (line in cmd.readLines()) {
                    let fields = line.split(",")
                    let pid = fields.pop().trim()
                    let command = fields.slice(1).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        result.append({pid: pid, command: command})
                    }
                }
                //  Windows WMIC drops this
                Path("TempWmicBatchFile.bat").remove()
            } else {
                cmd.start("/bin/ps -ef")
                for each (line in cmd.readLines()) {
                    let fields = line.trim().split(/ +/g)
                    let pid = fields[1]
                    let command = fields.slice(7).join(" ")
                    if (!pid.isDigit || command == "") continue
                    if ((pattern is RegExp && pattern.test(command)) || command.search(pattern.toString()) >= 0) {
                        result.append({pid: pid, command: command})
                    }
                }
            }
            cmd.close()
            return result
        }

        /**
            Execute a command/program and return the output as a result. 
            The call blocks while executing the command.
            @param command Command or program to execute
            @param options Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to true.
            @options timeout Number This is the default number of milliseconds for the command to complete.
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
            @param data Optional data to write to the command on it's standard input.
            @returns The command output from the standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function run(command: Object, options: Object = {}, data: Object = null): String {
            let cmd = new Cmd
            cmd.start(command, blend({detach: true}, options))
            if (data) {
                cmd.write(data)
            }
            cmd.finalize()
            cmd.wait()
            if (cmd.status != 0 && !options.exception) {
                throw new IOError(cmd.error)
            }
            return cmd.readString()
        }

        /**
            Run a command using the system command shell and wait for completion. On Windows, this requires that
            sh.exe is installed (See Cygwin). 
            @param command The (optional) command line to initialize with. The command may be either a string or
                an array of arguments. 
            @param options Command options hash. Supported options are:
            @options detach Boolean If true, run the command and return immediately. If detached, finalize() must be
                called to signify the end of data being written to the command's stdin.
            @options dir Path or String. Directory to set as the current working directory for the command.
            @options exception Boolean If true, throw exceptions if the command returns a non-zero status code. 
                Defaults to false.
            @options timeout Number This is the default number of milliseconds for the command to complete.
            @options noio Don't capture stdout from the command. If true, the command's standard output will go to the 
                application's current standard output. Defaults to false.
            @param data Optional data to write to the command on it's standard input.
            @return The command output from the standard output.
            @throws IOError if the command exits with non-zero status. The exception object will contain the command's
                standard error output. 
         */
        static function sh(command: Object, options: Object = null, data: Object = null): String {
            /*
                The form is:  sh -c "command args"
                The args must be wrapped in single quotes if they contain spaces. 
                Example:
                    This:       ["showColors", "red", "light blue", "Cannot \"render\""]
                    Becomes:    sh -c "showColors red 'light blue' 'Cannot \"render\"'
             */
            let shell = Cmd.locate("sh")
            if (command is Array) {
                for (let arg in command) {
                    /*  
                        Backquote backslashes and backquote quotes. Then wrap in single quotes. Single quotes are 
                        required because Cmd on Windows must format the entire command as a single string (not argv[])
                     */
                    let s = command[arg].toString().trimEnd('\n')
                    s = s.replace(/\"/g, '\\\"').replace(/\'/g, '\\\'')
                    command[arg] = "'" + s + "'"
                }
                return run([shell, "-c"] + [command.join(" ")], options, data).trimEnd()
            }
            /*
                Must quote single and double quotes as the comand will be wrapped in quotes on Windows.
                WARNING: If starting a program compiled with Cygwin, Cygwin has a bug where embedded quotes are parsed
                incorrectly by the Cygwin crt runtime startup. If an arg starts with a drive spec, embedded backquoted 
                quotes will be stripped and the backquote will be passed in. Windows crt runtime handles this correctly.
                For example:  ./args "c:/path \"a b\"
                    Cygwin will parse as  argv[1] == c:/path \a \b
                    Windows will parse as argv[1] == c:/path "a b"
             */
            return run([shell, "-c", command.toString().trimEnd('\n')], options, data).trimEnd()
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Compat.es"
 */
/************************************************************************/

/*
    Compat.es -- Compatibility with other JS engines

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  MOB -- much more 

module ejs {

    use default namespace public

    /** @hide */
    function gc(): Void
        GC.run 

    /** @hide */
    function readFile(path: String, encoding: String? = null): String?
        Path(path).readString()

//    //  Rhino
//    /** @hide */
//    var arguments = App.args
//
//    //  Command line history
//    /** @hide */
//    var history
//
//    /** @hide */
//    function help() {}
//
//    /** @hide */
//    function get environment() {
//        //  TODO
//    }
//
//    /** @hide */
//    function runCommand(name: String, ...args): Number {
//        try {
//            Cmd.sh(name + args.join(" "))
//            return 0
//        } catch (e) {
//            return 2
//        }
//    }
//
//    /** @hide */
//    function deserialize(filename): Object
//
//    /** @hide */
//    function load([filenames, ...])
//
//    /** @hide */
//    function defineClass(name) {}
//
//    /** @hide */
//    function loadClass(name) {}
//
//    /** @hide */
//    function seal(obj) {}
//
//    /** @hide */
//    function serialize(obj, filename) {}
//
//    /** @hide */
//    //  Run in another thread
//    function spawn(scriptOrFunction) {}
//
//    /** @hide */
//    function quit() {}
//
//    /** @hide */
//    function version(version) {}
}

/*
    Ease backward compatibility

    module ejs.xml { }
    module ejs.sys { }
    module ejs.io { }
*/

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

/************************************************************************/
/*
    Start of file "src/core/Config.es"
 */
/************************************************************************/

/*
    Config.es - Configuration settings from ./configure
 
    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.
 */

module ejs {

    /**
        Config class providing settings for various "configure" program settings.
        @spec ejs
        @stability evolving
     */
    enumerable class Config {

        use default namespace public

        /**
            True if a debug build
         */
        native static const Debug: Boolean

        /**
            System CPU type (eg. x86, x64, ppc, arm).
            This is the type of the system CPU and not the application instruction set.
         */
        native static const CPU: String

        /**
            Operating system version. One of: WINDOWS, LINUX, MACOSX, FREEBSD, SOLARIS, CYGWIN, VXWORKS
         */
        native static const OS: String

        /**
            Ejscript product name. Single word name.
         */
        native static const Product: String

        /**
            Ejscript product title. Multiword title.
         */
        native static const Title: String

        /**
            Ejscript version. Multiword title. Format is Major.Minor.Patch-Build For example: 1.1.2-1
         */
        native static const Version: String

        /**
            Binaries directory
         */
        native static const Bin: Path

        /**
            Include directory
         */
        native static const Inc: Path

        /** @hide */
        native static const Legacy: Boolean

        /** @hide */
        native static const SSL: Boolean

        /** @hide */
        native static const SQLITE: Boolean
        /** @hide */
        native static const DB: Boolean
        /** @hide */
        native static const MAPPER: Boolean
        /** @hide */
        native static const WEB: Boolean
        /** @hide */
        native static const MAIL: Boolean
        /** @hide */
        native static const TEMPLATE: Boolean
        /** @hide */
        native static const TAR: Boolean
    }
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

/************************************************************************/
/*
    Start of file "src/core/Date.es"
 */
/************************************************************************/

/*
    Date.es -- Date class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        General purpose class for representing and working with dates, times, time spans and time zones.
        @stability evolving
     */
    final class Date {

        use default namespace public

/* TODO 
            Better to convert Date to not have args, but rather default values
            @param milliseconds Integer representing milliseconds since 1 January 1970 00:00:00 UTC.
            @param dateString String date value in a format recognized by parse().
            @param year Integer value for the year. Should be a Four digit year (e.g. 1998).
            @param month Integer month value (0-11)
            @param date Integer date of the month (1-31)
            @param hour Integer hour value (0-23)
            @param minute Integer minute value (0-59)
            @param second Integer second value (0-59)
            @param msec Integer millisecond value (0-999)
*/

        /**
            Construct a new date object. Permissible constructor forms:
            <ul>
                <li>Date() This is the default and it constructs a date instance for the current time.</li>
                <li>Date(milliseconds) where (seconds sincde 1 Jan 1970 00:00:00 UTC))</li>
                <li>Date(dateString) where (In a format recognized by parse())</li>
                <li>Date(year, month, date) where (Four digit year, month: 0-11, date: 1-31)</li>
                <li>Date(year, month, date [, hour, minute, second, msec]) where (hour: 0-23, minute: 0-59, second: 0-59, msec: 0-999)</li>
            </ul>
         */
        native function Date(...args)

        /**
            The day of the week (0 - 6, where 0 is Sunday) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get day(): Number 
        native function set day(day: Number): Void

        /**
            The day of the year (0 - 365) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get dayOfYear(): Number 
        native function set dayOfYear(day: Number): Void

        /**
            The day of the month (1-31).
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get date(): Number 
        native function set date(date: Number): Void

        /**
            Time in milliseconds since the date object was constructed
            @spec ejs
         */
        native function get elapsed(): Number

        /**
            Format a date using a format specifier in local time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            For full details, consult your platform API manual for strftime.

            The format specifiers are:
            <ul>
            <li>%A    national representation of the full weekday name.</li>
            <li>%a    national representation of the abbreviated weekday name.</li>
            <li>%B    national representation of the full month name.</li>
            <li>%b    national representation of the abbreviated month name.</li>
            <li>%C    (year / 100) as decimal number; single digits are preceded by a zero..</li>
            <li>%c    national representation of time and date.</li>
            <li>%D    is equivalent to %m/%d/%y.</li>
            <li>%d    the day of the month as a decimal number (01-31).</li>
            <li>%e    the day of month as a decimal number (1-31); single digits are preceded by a blank.</li>
            <li>%F    is equivalent to %Y-%m-%d.</li>
            <li>%H    the hour (24-hour clock) as a decimal number (00-23).</li>
            <li>%h    the same as %b.</li>
            <li>%I    the hour (12-hour clock) as a decimal number (01-12).</li>
            <li>%j    the day of the year as a decimal number (001-366). Note: this range is different to that of
                      the dayOfYear property which is 0-365.</li>
            <li>%k    the hour (24-hour clock) as a decimal number (0-23); single digits are preceded by a blank.</li>
            <li>%l    the hour (12-hour clock) as a decimal number (1-12); single digits are preceded by a blank.</li>
            <li>%M    the minute as a decimal number (00-59).</li>
            <li>%m    the month as a decimal number (01-12).</li>
            <li>%n    a newline.</li>
            <li>%P    Lower case national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%p    national representation of either "ante meridiem" or "post meridiem" as appropriate.</li>
            <li>%R    is equivalent to %H:%M.</li>
            <li>%r    is equivalent to %I:%M:%S %p.</li>
            <li>%S    the second as a decimal number (00-60).</li>
            <li>%s    the number of seconds since the January 1, 1970 UTC.</li>
            <li>%T    is equivalent to %H:%M:%S.</li>
            <li>%t    a tab.</li>
            <li>%U    the week number of the year (Sunday as the first day of the week) as a decimal number (00-53).</li>
            <li>%u    the weekday (Monday as the first day of the week) as a decimal number (1-7).</li>
            <li>%v    is equivalent to %e-%b-%Y.</li>
            <li>%W    the week number of the year (Monday as the first day of the week) as a decimal number (00-53).</li>
            <li>%w    the weekday (Sunday as the first day of the week) as a decimal number (0-6).</li>
            <li>%X    national representation of the time.</li>
            <li>%x    national representation of the date.</li>
            <li>%Y    the year with century as a decimal number.</li>
            <li>%y    the year without century as a decimal number (00-99).</li>
            <li>%Z    the time zone name.</li>
            <li>%z    the time zone offset from UTC; a leading plus sign stands for east of UTC, a minus
                      sign for west of UTC, hours and minutes follow with two digits each and no delimiter between them
                      (common form for RFC 822 date headers).</li>
            <li>%+    national representation of the date and time (the format is similar to that produced by date(1)).
                      This format is platform dependent.</li>
            <li>%%    Literal percent.</li>
            </ul>
        
         Some platforms may also support the following format extensions:
            <ul>
            <li>%E*   POSIX locale extensions. Where "*" is one of the characters: c, C, x, X, y, Y.
            <li>%G    a year as a decimal number with century. This year is the one that contains the greater part of
                      the week (Monday as the first day of the week).</li>
            <li>%g    the same year as in %G, but as a decimal number without century (00-99).</li>
            <li>%O*   POSIX locale extensions. Where "*" is one of the characters: d e H I m M S u U V w W y.
                      supposed to provide alternate representations. Additionly %OB implemented to represent alternative 
                      months names (used standalone, without day mentioned).
            <li>%V    the week number of the year (Monday as the first day of the week) as a decimal
                      number (01-53).  If the week containing January 1 has four or more days in the new year, then it
                      is week 1; otherwise it is the last week of the previous year, and the next week is week 1.</li>
            </ul>

            @param layout Format layout string using the above format specifiers. See strftime(3) for more information.
            @return string representation of the date.
            @spec ejs
         */
        native function format(layout: String): String 

        /**
            Format a date using a format specifier in UTC time. This routine is implemented by calling 
            the O/S strftime routine and so not all the format specifiers are available on all platforms.
            @param layout Format layout string using the above format specifiers. See format() for the list of format
            specifiers.
            @return string representation of the date.
            @spec ejs
         */
        native function formatUTC(layout: String): String 

//  MOB -- rethink name
        /**
            Calculate a time that is $msec in the future
            @param msec Period into the future in milliseconds. Can be negative for the past.
            @return A new date object
         */
        native function future(msec: Number): Date

        /**
            Return the day of the month in local time
            @return Returns the day of the year (1-31)
         */
        function getDate(): Number 
            date

        /**
            Return the day of the week in local time.
            @return The integer day of the week (0 - 6, where 0 is Sunday)
         */
        function getDay(): Number 
            day

        /**
            Return the year as four digits in local time
            @return The integer year
         */
        function getFullYear(): Number 
            year

        /**
            Return the hour (0 - 23) in local time.
            @return The integer hour of the day
         */
        function getHours(): Number 
            hours

        /**
            Return the millisecond (0 - 999) in local time.
            @return The number of milliseconds as an integer
         */
        function getMilliseconds(): Number 
            milliseconds

        /**
            Return the minute (0 - 59) in local time.
            @return The number of minutes as an integer
         */
        function getMinutes(): Number 
            minutes

        /**
            Return the month (0 - 11) in local time.
            @return The month number as an integer
         */
        function getMonth(): Number 
            month

        /**
            Return the second (0 - 59) in local time.
            @return The number of seconds as an integer
         */
        function getSeconds(): Number 
            seconds

        /**
            Return the number of milliseconds since midnight, January 1st, 1970 UTC.
            @return The number of milliseconds as a long
         */
        function getTime(): Number
            time

        /**
            Return the number of minutes between the local computer time and Coordinated Universal Time.
            @return Integer containing the number of minutes between UTC and local time. The offset is positive if
            local time is behind UTC and negative if it is ahead. E.g. American PST is UTC-8 so 480 will be retured.
            This value will vary if daylight savings time is in effect.
         */
        native function getTimezoneOffset(): Number

        /**
            Return the day (date) of the month (1 - 31) in UTC time.
            @return The day of the month as an integer
         */
        native function getUTCDate(): Number 

        /**
            Return the day of the week (0 - 6) in UTC time.
            @return The day of the week as an integer
         */
        native function getUTCDay(): Number 

        /**
            Return the year in UTC time.
            @return The full year in 4 digits as an integer
         */
        native function getUTCFullYear(): Number 

        /**
            Return the hour (0 - 23) in UTC time.
            @return The integer hour of the day
         */
        native function getUTCHours(): Number 

        /**
            Return the millisecond (0 - 999) in UTC time.
            @return The number of milliseconds as an integer
         */
        native function getUTCMilliseconds(): Number 

        /**
            Return the minute (0 - 59) in UTC time.
            @return The number of minutes as an integer
         */
        native function getUTCMinutes(): Number 

        /**
            Return the month (1 - 12) in UTC time.
            @return The month number as an integer
         */
        native function getUTCMonth(): Number 

        /**
            Return the second (0 - 59) in UTC time.
            @return The number of seconds as an integer
         */
        native function getUTCSeconds(): Number 

        /**
            The current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get hours(): Number 
        native function set hours(hour: Number): void

        /**
            The current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get milliseconds(): Number 
        native function set milliseconds(ms: Number): void

        /**
            The current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get minutes(): Number 
        native function set minutes(min: Number): void

        /**
            The current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get month(): Number 
        native function set month(month: Number): void

        /**
            Time in nanoseconds since the date object was constructed
            @spec ejs
         */
        function nanoAge(): Number
            elapsed * 1000

        /**
            Return a new Date object that is one day greater than this one.
            @param inc Increment in days to add (or subtract if negative)
            @return A Date object
            @spec ejs
         */
        native function nextDay(inc: Number = 1): Date

        /**
            Return the current time as milliseconds since Jan 1 1970.
            This returns the same value as date.time for instances.
            @spec mozilla
         */
        static native function now(): Number

        /**
            Return a high resolution CPU tick counter. If the platform cannot provide such a counter, the call
            will return the $now value in milliseconds.
            @spec ejs
         */
        static native function get ticks(): Number

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a local date/time.  This is similar to parse() but it returns a
                date object.
            @param dateString The date string to parse.
            The date parsing logic uses heuristics and attempts to intelligently parse a range of dates. Some of the
            possible formats are:
            <ul>
                <li>07/28/2013</li>
                <li>07/28/08</li>
                <li>Jan/28/2013</li>
                <li>Jaunuary-28-2013</li>
                <li>28-jan-2013</li>
                <li>[29] Jan [15] [2013]</li>
                <li>dd/mm/yy, dd.mm.yy, dd-mm-yy</li>
                <li>mm/dd/yy, mm.dd.yy, mm-dd-yy</li>
                <li>yyyy/mm/dd, yyyy.mm.dd, yyyy-mm-dd</li>
                <li>10:52[:23]</li>
                <li>2009-05-21t16:06:05.000z (ISO date)</li>
                <li>[GMT|UTC][+-]NN[:]NN (timezone)</li>
            </ul>
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and Return a new Date object. If $dateString does not contain a timezone,
                the date string will be interpreted as a UTC date/time.
            @param dateString UTC date string to parse. See $parseDate for supported formats.
            @param defaultDate Default date to use to fill out missing items in the date string.
            @return Return a new Date.
            @spec ejs
         */
        static native function parseUTCDate(dateString: String, defaultDate: Date? = null): Date

        /**
            Parse a date string and return the number of milliseconds since midnight, January 1st, 1970 UTC. 
            If $dateString does not contain a timezone, the date string will be interpreted as a local date/time.
            @param dateString The string to parse. See $parseDate for supported formats.
            @return Return a new date number.
         */
        static native function parse(dateString: String): Number

        /**
            The current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @spec ejs
         */
        native function get seconds(): Number 
        native function set seconds(sec: Number): void

        /**
            Set the date of the month (1 - 31)
            If a value outside the range is given, the date is adjusted without error.
            @param d Date of the month
         */
        function setDate(d: Number): void
            date = d

        /**
            Set the current year as four digits in local time.
            @param y Current year
         */
        function setFullYear(y: Number): void
            year = y

        /**
            Set the current hour (0 - 23) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        function setHours(h: Number): void
            hours = h

        /**
            Set the current millisecond (0 - 999) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        function setMilliseconds(ms: Number): void
            milliseconds = ms

        /**
            Set the current minute (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        function setMinutes(min: Number): void
            minutes = min

        /**
            Set the current month (0 - 11) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        function setMonth(mon: Number): void
            month = mon

        /**
            Set the current second (0 - 59) in local time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        function setSeconds(sec: Number, msec: Number? = null): void {
            seconds = sec
            if (msec != null) {
                milliseconds = msec
            }
        }

        /**
            Set the number of milliseconds since midnight, January 1st, 1970 UTC.
            @param value The millisecond as a long
         */
        function setTime(value: Number): void
            time = value

        /**
            Set the date of the month (1 - 31) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param d The date to set
         */
        native function setUTCDate(d: Number): void

        /**
            Set the current year as four digits in UTC time.
            @param y The year to set
         */
        native function setUTCFullYear(y: Number): void

        /**
            Set the current hour (0 - 23) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param h The hour as an integer
         */
        native function setUTCHours(h: Number): void

        /**
            Set the current millisecond (0 - 999) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param ms The millisecond as an integer
         */
        native function setUTCMilliseconds(ms: Number): void

        /**
            Set the current minute (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param min The minute as an integer
         */
        native function setUTCMinutes(min: Number): void

        /**
            Set the current month (0 - 11) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param mon The month as an integer
         */
        native function setUTCMonth(mon: Number): void

        /**
            Set the current second (0 - 59) in UTC time.
            If a value outside the range is given, the date is adjusted without error.
            @param sec The second as an integer
            @param msec Optional milliseconds as an integer
         */
        native function setUTCSeconds(sec: Number, msec: Number? = null): void

        /**
            The number of milliseconds since midnight, January 1st, 1970 UTC and the current date object.
            This is the same as Date.now()
            @spec ejs
         */
        native function get time(): Number 
        native function set time(value: Number): Void 

        /**
            Return a string containing the date portion excluding the time portion of the date in local time.
            The format is American English.
            Sample: "Fri Jan 15 2013"
            @return A string representing the date portion.
         */
        function toDateString(): String 
            format("%a %b %d %Y")

        /** 
            Convert a date to an equivalent JSON encoding.
            @return This function returns a date in ISO format as a string.
            @throws TypeError If the object could not be converted to a string.
         */ 
        native override function toJSON(): String

        /**
            Return an ISO formatted date string.
            Sample: "2006-12-15T23:45:09.33-08:00"
            @return An ISO formatted string representing the date.
            @spec ejs
         */
        native function toISOString(): String 

        /**
            Return a localized string containing the date portion excluding the time portion of the date in local time.
            Note: You should not rely on the format of the output as the exact format will depend on the platform
            and locale.
            Sample: "Fri 15 Dec 2006 GMT-0800". (Note: Other platforms render as:
            V8  format: "Fri, 15 Dec 2006 GMT-0800"
            JS  format: "01/15/2013"
            JSC format: "January 15, 2013")
            @return A string representing the date portion.
         */
        function toLocaleDateString(): String 
            format("%a, %d %b %Y GMT%z")

        /**
            Return a localized string containing the date. This formats the date using the operating system's locale
            conventions.
            Sample:  "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)". (Note: Other JavaScript platforms render as:
            V8 format:  "Fri, 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            JS format:  "Fri Jan 15 13:09:02 2013"
            JSC format: "January 15, 2013 1:09:06 PM PST"
            @return A string representing the date.
         */
        override function toLocaleString(): String
            format("%a, %d %b %Y %T GMT%z (%Z)")

        /**
            Return a string containing the time portion of the date in local time.
            Sample:  "13:08:57". Note: Other JavaScript platforms render as:
            V8 format:  "13:08:57"
            JS format:  "13:09:02"
            JSC format: "1:09:06 PM PST"
            @return A string representing the time.
         */
        function toLocaleTimeString(): String 
            format("%X")

        /**
            Return a string representing the date in local time. The format is American English.
            Sample: "Fri 15 Dec 2006 23:45:09 GMT-0800 (PST)"
            @return A string representing the date.
         */
        override native function toString(): String 

        /**
            Return a string containing the time portion in human readable form in American English.
            Sample: "13:08:57 GMT-0800 (PST)"
            @return A string representing the time.
         */
        function toTimeString(): String 
            format("%X GMT%z (%Z)")

        /**
            Return a string containing the date in UTC time.
            Sample: "Sat, 16 Dec 2006 08:06:21 GMT"
            @return A string representing the date.
         */
        function toUTCString(): String 
            formatUTC("%a, %d %b %Y %T GMT")


        /**
            Calculate the number of milliseconds since the epoch for a UTC time.
            Date(year, month, date [, hour, minute, second, msec])</li>
            @param year Year
            @param month Month of year
            @param day Day of month
            @param hours Hour of day
            @param minutes Minute of hour
            @param seconds Secods of minute
            @param milliseconds Milliseconds of second
            @return The number of milliseconds since January 1, 1970 00:00:00 UTC.
         */
        static native function UTC(year: Number, month: Number, day: Number, hours: Number = 0, 
            minutes: Number = 0, seconds: Number = 0, milliseconds: Number = 0): Number

        /**
            Return the primitive value of the object
            @returns A number corresponding to the $time property.
         */ 
        override function valueOf(): Number
            time

        /**
            The year in local time. This is the full year. E.g. A date of the year 2010 will return the number 2010.
            @spec ejs
         */
        native function get year(): Number 
        native function set year(year: Number): void

        /**
            Date difference. Return a new Date that is the difference of the two dates.
            @param The operand date
            @return Return a new Date.
         */
//  MOB -- reconsider
        # TODO
        native function -(date: Date): Date
    }
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

/************************************************************************/
/*
    Start of file "src/core/Debug.es"
 */
/************************************************************************/

/*
    Debug.es -- Debug class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Debug configuration class. Singleton class containing the application's debug configuration.
        @spec ejs
        @stability prototype
     */
    class Debug {

        use default namespace public

        /**
            Break to the debugger. Suspend execution and break to the debugger.
         */ 
        native static function breakpoint(): void

        /**
            The current debug mode.  Setting mode to true will put the application in debug mode. When debug mode 
            is enabled, the runtime will typically suspend timeouts and will take other actions to make debugging easier.
            Invoking the ejs shell with a -D command line switch will also enable debug mode.
            This property is read-write. 
         */
        native static function get mode(): Boolean

        /**
            @duplicate Debug.mode
            @param value True to turn debug mode on or off.
         */
        native static function set mode(value: Boolean): void
    }

    /** 
        Convenient way to trap to the debugger
     */
    native function breakpoint(): Void
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

/************************************************************************/
/*
    Start of file "src/core/Emitter.es"
 */
/************************************************************************/

/*
    Emitter.es -- Event emitter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /** 
        The emitter class provides a publish/subscribe model of communication via events. It supports the 
        registration of observers who want to subscribe to events of interest. 
        @example
            events.on(event, function (event, ...args) {
                //  Do something
            }).fire("topic", 1, 2, 3)
        @stability prototype
     */
    class Emitter {
        use default namespace public

        private var endpoints: Object

        /** 
            Construct a new event Emitter object 
         */
        function Emitter()
            endpoints = new Object

        private function addOneObserver(name: String, callback: Function): Void {
            let observers : Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (e.callback == callback && e.name == name) {
                        return
                    }
                }
            } else {
                observers = endpoints[name] = new Array
            }
            if (callback) {
                observers.append(new Endpoint(callback, name))
                fire("on", name, callback)
            }
        }

        /** 
            Clear observers for a given event name. 
            @param name Event name to clear. The name can be a string or an array of event strings. If null, observers 
            for all event names are cleared.
         */
        function clearObservers(name: Object? = null): Void {
            if (name == null) {
                endpoints = new Object
            } else if (name is Array) {
                for each (n in name) {
                    observers = endpoints[n] = new Array
                }
            } else {
                observers = endpoints[name] = new Array
            }
        }

        /** 
            @hide
            TODO - complete or remove
         */
        function delayedFire(name: String, delay: Number, ...args): Void {
            Timer(delay, function() {
                fire(name, ...args)
            })
        }

        /** 
            Determine if the emitter has any observers.
            @return True if there are currently registered observers
        */
        function hasObservers(): Boolean 
            endpoints.length > 0

        /** 
            Return the observers for this emitter. 
            @param name Event name to fire to the observers.
            @return An array of observer endpoints. These are cloned and not the actual observer objects.
         */
        function getObservers(name: String): Array
            endpoints[name].clone(true)
       
        /** 
            Emit an event to the registered observers.
            @param name Event name to fire to the observers.
            @param args Args to pass to the observer callback
         */
        function fire(name: String, ...args): Void
            fireThis(name, null, ...args)

        /** 
            Emit an event to the registered observers using an explict value for the "this" object.
            @param name Event name to fire to the observers.
            @param thisObj Object to use for "this" when running the callback. This overrides any bound values for "this"
            @param args Args to pass to the observer callback
         */
        function fireThis(name: String, thisObj: Object?, ...args): Void {
            let observers: Array? = endpoints[name]
            if (observers) {
                for each (var e: Endpoint in observers) {
                    if (name == e.name) {
                        if (e.active) {
                            e.pending ||= []
                            e.pending.append([name, args])
                        } else {
                            e.active = true
                            for (;;) {
                                try {
                                    /* This forces to use the bound this value */
                                    e.callback.apply(thisObj, [name] + args)
                                } catch (e) {
                                    App.errorStream.write("Exception in event on observer: " + name  + "\n" + e)
                                }
                                if (e.pending && e.pending.length > 0) {
                                    [name, args] = e.pending.shift()
                                } else {
                                    break
                                }
                            }
                            e.pending = null
                            e.active = false
                        }
                    }
                }
            }
        }

        /** 
            Add an observer for a set of named event.
            The callback will be invoked when the requested event is fired by calling Emitter.fire. When the callback 
            runs, it will be invoked with the value of "this" relevant to the context of the callback. If the callback
            is a class method, the value of "this" will be the object instance. Global functions will have "this" set to
            the global object. Use Function.bind to override the bound "this" value.
            @param name Event name to observe. The observer will receive events of this event class or any of its subclasses.
            The name can be a string or an array of event strings.
            @param callback Function to call when the event is received.
         */
        function on(name: Object!, callback: Function!): Emitter {
            if (name is String) {
                addOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    addOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for observe: " + typeOf(name))
            }
            return this
        }

        private function removeOneObserver(name: String!, callback: Function?): Void {
            var observers: Array? = endpoints[name]
            for (let i in observers) {
                var e: Endpoint = observers[i]
                if (e.name == name && (callback == null || e.callback == callback)) {
                    fire("off", name, callback)
                    observers.splice(i, 1)
                }
            }
        }

        /** 
            Turn off and remove a registered observer.
            @param name Event name used when the observer was added.
            @param callback Callback function used when the observer was added. If null is supplied, all callbacks 
                will be removed.
         */
        function off(name: Object!, callback: Function?): Void {
            if (name is String) {
                removeOneObserver(name, callback)
            } else if (name is Array) {
                for each (n in name) {
                    removeOneObserver(n, callback)
                }
            } else {
                throw new Error("Bad name type for off()")
            }
        }
    }


    /* 
        Observing endpoints 
     */
    internal class Endpoint {
        public var callback: Function
        public var name: String
        public var active: Boolean
        public var pending: Array?
        function Endpoint(callback: Function, name: String) {
            this.callback = callback
            this.name = name
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Error.es"
 */
/************************************************************************/

/*
    Error.es -- Error exception classes

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        Error exception object and base class. Exception objects are created by the system as part of changing 
        the normal flow of execution when some error condition occurs. 

        When an exception is triggered and acted upon ("thrown"), the system transfers the flow of control to a 
        pre-defined instruction stream (the handler or "catch" code). The handler may return processing to the 
        point execution block where the exception was thrown. It may re-throw the exception or pass control up 
        the call stack for an outer handler to process.
        @stability evolving
     */
    dynamic class Error {

        use default namespace public

        /** 
            Source filename of the script that created the error
         */
        function get filename(): Path
            Path(stack[0].filename)

        /** 
            Source line number in the script that created the error
         */
        function get lineno(): Number
            stack[0].lineno

        /**
            Supplemental error data
         */
        enumerable var data: Object

        /**
            Error message
         */
        enumerable var message: String

        /**
            Callback stack
         */
        enumerable var stack: Array

        /** 
            Time the event was created. The Context constructor will automatically set the timestamp to the current time.  
         */
        enumerable var timestamp: Date

        /**
            Optional error code
         */
        enumerable var code: Number

        /**
            Construct a new Error object.
            @params message Message to use when defining the Error.message property. Typically a string but can be an
                object of any type.
         */
        native function Error(message: String? = null)

        /**
            Capture the stack. This call captures the stack and returns an Array describing the current stack frame.
            @param uplevels Skip a given count of stack frames from the stop of the call stack.
         */
        native static function capture(uplevels: Number = 0): Array

        /**
            Format the captured stack
            @return A string containing the formatted stack backtrace. Format is:
                [INDEX FILENAME, line LINENO, FUNCTION, CODE_LINE]
         */
        function formatStack(): String {
            let result = ""
            let i = 0
            for each (frame in stack) {
                if (frame.filename) {
                    result += " [%02d] %s, line %d, %s, %s\n".format(
                        i++, frame.filename, frame.lineno, frame.func, frame.code)
                } else {
                    result += " [%02d] %s\n".format(i++, frame.func)
                }
            }
            return result
        }
    }

    /**
        Arguments error exception class. 
        Thrown when function arguments are not compatible with the required function parameters.
        @spec ejs
        @stability evolving
     */
    dynamic class ArgError extends Error {
        /**
            ArgError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArgError(message: String? = null) 
    }

    /**
        Arithmetic error exception class. Thrown when the system cannot perform an arithmetic or math operation, 
        @spec ejs
        @stability evolving
     */
    dynamic class ArithmeticError extends Error {
        /**
            ArithmeticError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ArithmeticError(message: String? = null) 
    }

    /**
        Assertion error exception class. Thrown when the $global.assert method is invoked with a false value.
        @spec ejs
        @stability evolving
     */
    dynamic class AssertError extends Error {
        /**
            AssertError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function AssertError(message: String? = null) 
    }

//UNUSED Who is using this? Delete
    /**
        Code (instruction) error exception class. Thrown when an illegal or insecure operation code is detected 
        in the instruction stream.
        @spec ejs
        @stability evolving
     */
    dynamic class InstructionError extends Error {
        /**
            InstructionError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InstructionError(message: String? = null) 
    }

    /**
        IO error exception class. Thrown when an I/O/ interruption or failure occurs, e.g. a file is not found 
        or there is an error in a communication stack.
        @spec ejs
        @stability evolving
     */
    dynamic class IOError extends Error {
        /**
            IOError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function IOError(message: String? = null) 
    }

    /**
        Internal error exception class. Thrown when some error occurs in the virtual machine.
        @spec ejs
        @stability evolving
        @hide
     */
    dynamic class InternalError extends Error {
        /**
            InternalError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function InternalError(message: String? = null) 
    }

    /**
        Memory error exception class. Thrown when the system attempts to allocate memory and none is available 
        or the stack overflows.
        @spec ejs
        @stability evolving
     */
    dynamic class MemoryError extends Error {
        /**
            MemoryError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function MemoryError(message: String? = null) 
    }

//UNUSED Who is using this? Delete
    /**
        OutOfBounds error exception class. Thrown to indicate that an attempt has been made to set or access an 
        object's property outside of the permitted set of values for that property. For example, an array has been 
        accessed with an illegal index or, in a date object, attempting to set the day of the week to greater then 7.
        @spec ejs
        @stability evolving
        @hide
     */
    dynamic class OutOfBoundsError extends Error {
        /**
            OutOfBoundsError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function OutOfBoundsError(message: String? = null) 
    }

    /**
        Reference error exception class. Thrown when an invalid reference to an object is made, e.g. a method is 
        invoked on an object whose type does not define that method.
        @stability evolving
     */
    dynamic class ReferenceError extends Error {
        /**
            ReferenceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ReferenceError(message: String? = null)
    }

    /**
        Resource error exception class. Thrown when the system cannot allocate a resource it needs to continue, 
        e.g. a native thread, process, file handle or the like.
        @spec ejs
        @stability evolving
     */
    dynamic class ResourceError extends Error {
        /**
            ResourceError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function ResourceError(message: String? = null) 
    }

    /**
        Security error exception class. Thrown when an access violation occurs. Access violations include attempting 
        to write a file without having write permission or assigning permissions without being the owner of the 
        securable entity.
        @spec ejs
        @stability evolving
        @hide
     */
    dynamic class SecurityError extends Error {
        /**
            SecurityError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SecurityError(message: String? = null) 
    }

    /**
        State error exception class. Thrown when an object cannot be transitioned from its current state to the 
        desired state.
        @spec ejs
        @stability evolving
     */
    dynamic class StateError extends Error {
        /**
            StateError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function StateError(message: String? = null) 
    }

    /**
        Syntax error exception class. Thrown when the system cannot parse a character sequence for the intended 
        purpose, e.g. a regular expression containing invalid characters.
        @stability evolving
     */
    dynamic class SyntaxError extends Error {
        /**
            SyntaxError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function SyntaxError(message: String? = null) 
    }

    /**
        Type error exception class. Thrown when a type casting or creation operation fails, e.g. when an operand 
        cannot be cast to a required parameter type.
        @stability evolving
     */
    dynamic class TypeError extends Error {
        /**
            TypeError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function TypeError(message: String? = null) 
    }

//UNUSED Delete
    /**
        Uri error exception class. Throw a Uri fails to parse.
        @stability prototype
        @hide
     */
    dynamic class URIError extends Error {
        /**
            URIError constructor.
            @params message Message to use when defining the Error.message property
         */
        native function URIError(message: String? = null) 
    }
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

/************************************************************************/
/*
    Start of file "src/core/File.es"
 */
/************************************************************************/

/*
    File.es -- File I/O class. Do file I/O and manage files.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  MOB - rethink various throws

module ejs {

    /** 
        The File class provides a foundation of I/O services to interact with physical files.
        Each File object represents a single file, a named path to data stored in non-volatile memory. A File object 
        provides methods for creating, opening, reading, writing and deleting a file, and for accessing and modifying 
        information about the file. All I/O is unbuffered and synchronous.
        @spec ejs
        @stability evolving
     */
    final class File implements Stream {

        use default namespace public

        /**
            Async mode is not yet supported
            @hide 
          */
        function get async(): Boolean
            false

        /** 
            Async mode is not yet supported
            @hide 
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("File class does not support async I/O")
            }
        }

        /** 
            Create a File object and open the requested path.
            @param path the name of the file to associate with this file object. Can be either a String or a Path.
            @param options If the open options are provided, the file is opened. See $open for the available options.
         */
        native function File(path: Object, options: Object? = null)

        /** 
            Is the file opened for reading
         */
        native function get canRead(): Boolean

        /** 
            Is the file opened for writing
         */
        native function get canWrite(): Boolean

        /** 
            Close the input stream and free up all associated resources.
         */
        native function close(): void 

        /** 
            Current encoding schem for serializing strings. Defaults to "utf-8".
         */
        function get encoding(): String
            "utf-8"

        /** 
            Set the encoding scheme for serializing strings. The default encoding is UTF-8.
            @param enc String representing the encoding scheme
         */
        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            File I/O is currently unbuffered
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void { }

        /** 
            Iterate over the positions in a file. This will get an iterator for this file to be used by 
                "for (v in files)"
            @return An iterator object that will return numeric offset positions in the file.
         */
        override iterator native function get(): Iterator

        /** 
            Get an iterator for this file to be used by "for each (v in obj)". Return each byte of the file in turn.
            @return An iterator object that will return the bytes of the file.
         */
        override iterator native function getValues(): Iterator

        /** 
            Is the file open
         */
        native function get isOpen(): Boolean

        /** 
            TODO
            @duplicate Stream.off 
            @hide
         */
        native function off(name, observer: Function): Void

        /** 
            TODO
            @duplicate Stream.on 
            @hide
         */
        native function on(name, observer: Function): File

        //  MOB - would it be better not to throw?
        /**  
            Open a file. This opens the file designated when the File constructor was called.
            @params options Optional options. If ommitted, the options default to open the file in read mode.
                Options can be either a mode string or can be an options hash. 
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "c" to create the file if it does not exist, "l" to gain an exclusive lock, "s" for a shared lock,
                "t" for text mode, and "+" to never truncate. Defaults to "r". NOTE: not all platforms support "l" and "s".
                If "w" is specified and the file does not exist, it will be created. If "+" is not specified, the file
                    will be truncated when opened, unless "a" is specified to append to existing content.
                If "c" is specified and the file exists, the open will fail.
                Use: "wa+" to append to a file and create if it does not exist.
            @options permissions Number containing the Posix permissions number value. Note: this is a number
                and not a string representation of an octal posix number.
            @options owner String representing the file owner (Not implemented)
            @options group String representing the file group (Not implemented)
            @return the File object. This permits method chaining.
            @throws IOError if the path or file cannot be created.
         */
        native function open(options: Object? = null): File

        /** 
            Current file options set when opening the file.
         */ 
        native function get options(): Object

        /** 
            The name of the file associated with this File object or null if there is no associated file.
         */
        native function get path(): Path?

        /** 
            The current read/write I/O position in the file.
         */
        native function get position(): Number

        /** 
            Seek to a new location in the file and set the File marker to a new read/write position.
            @param loc Location in the file to seek to. Set the position to zero to reset the position to the beginning 
            of the file. Set the position to a negative number to seek relative to the end of the file (-1 positions 
            at the end of file).
            @throws IOError if the seek failed.
         */
        native function set position(loc: Number): void

        /** 
            Read a block of data from a file into a byte array. This will advance the read file's position.
                This will read up to $count bytes that will fit into the provided buffer. 
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of bytes to read. If -1, read much as the buffer will hold up to the entire file if the 
                buffer is of sufficient size or is growable.
            @return A count of the bytes actually read. Returns null on end of file.
            @throws IOError if the file could not be read.
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /** 
            Read data bytes from a file and return a byte array containing the data.
            @param count Number of bytes to read. If -1, read the entire file.
            @return A byte array containing the read data. Returns an empty array on end of file.
            @throws IOError if the file could not be read.
         */
        native function readBytes(count: Number = -1): ByteArray?

        //  MOB UNICODE encoding. Should this do UTF-8 to Unicode-16?
        /** 
            Read data from a file as a string.
            @param count Number of bytes to read. If -1, read the entire file.
            @return A string containing the read data. Returns an empty string on end of file.
            @throws IOError if the file could not be read.
         */
        native function readString(count: Number = -1): String?

        /** 
            Remove a file
            @return true if the file could be removed
         */
        function remove(): Boolean {
            if (isOpen) {
                return false
            }
            return Path(path).remove()
        }

        /** 
            The size of the file in bytes.
         */
        native function get size(): Number 

        //  MOB -- perhaps better to not throw
        /**     
            Truncate the file. 
            @param value the new length of the file
            @throws IOError if the file's size cannot be changed
         */
        native function truncate(value: Number): Void 

        /** 
            Write data to the file. All I/O is unbuffered and synchronous. 
            @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
            first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
            the BinaryStream class to control the byte ordering when writing numbers.
            @returns the number of bytes written.
            @throws IOError if the file could not be written.
         */
        native function write(...items): Number

        //  MOB writeln would be useful
    }
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

/************************************************************************/
/*
    Start of file "src/core/FileSystem.es"
 */
/************************************************************************/

/*
    FileSystem.es -- FileSystem class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The FileSystem objects provide global information about a file systems.
        @spec ejs
        @stability prototype
     */
    class FileSystem {

        use default namespace public

        //  MOB - getter
        /**
            Get a list of file system drives
            @return An array of drives. Returns an empty list on systems without drive specifiers in filenames.
         */
        native static function drives(): Array 

        /** 
            Create a new FileSystem object for the file system that contains the given path.
            @param path String or Path of a file that would reside in the file system.
         */
        native function FileSystem(path: Path? = null)

        /** 
            Are path names on this file system case sensitive. ie. are uppercase and lowercase path names equivalent
         */
        #FUTURE
        native function get caseSensitive(): Boolean

        /** 
            Do path names on this file system preserve case
         */
        #FUTURE
        native function get casePreserved(): Boolean

        /** 
            Free space in the file system in 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get space(): Number

        /** 
            Do path names on this file system support a drive specifications.
         */
        native function get hasDrives(): Boolean 

        /** 
            Is the file system available, mounted and ready for use
         */
        #FUTURE
        native function get isReady(): Boolean

        /** 
            Is the file system is writable
         */
        #FUTURE
        native function get isWritable(): Boolean

        /** 
            The new line characters for this file system. Usually "\n" or "\r\n".
         */
        native function get newline(): String 

        /** */
        native function set newline(terminator: String): Void

        /** 
            Path to the root directory of the file system
         */
        native function get root(): Path

        /** 
            Path directory separators. The first character is the default separator. Usually "/" or "\\".
         */
        native function get separators(): String 

        /** */
        native function set separators(sep: String): Void 

        /** 
            The total size of the file system in of 1M blocks (1024 * 1024 bytes).
         */
        #FUTURE
        native function get size(): Number
    }
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

/************************************************************************/
/*
    Start of file "src/core/Frame.es"
 */
/************************************************************************/

/*
    Frame.es -- Frame class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Function call frame class. The Frame type is used internally by the ejs virtual machine to manage
        function calls and store function local variables.
        @stability evolving
     */
    dynamic class Frame extends Function { }
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

/************************************************************************/
/*
    Start of file "src/core/Function.es"
 */
/************************************************************************/

/*
    Function.es -- Function class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Function type is used to represent functions, function expressions, closures and class methods. It contains a 
        reference to the code to execute, the execution scope and possibly a bound "this" reference.
        @stability evolving
     */
    class Function {

        use default namespace public

        /**
            Create a function from the supplied formal parameter names and function body expression.
            @param args The parameters and body are suplied as discrete parameters to Function(), i.e. 
                not as an array of args.
            Function ([args, ...], body)
         */
        native function Function(...args)
/*
    FUTURE -- todo
            let body = args.pop()
            let code = "function(" + args.join(",") + ") {\n" + body + "\n}"
            print("CODE " + code)
            return eval(code)
        }
*/

        /** 
            Invoke the function on another object.
            @param thisObject The object to set as the "this" object when the function is called.
            @param args Array of actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function apply(thisObject: Object, args: Array): Object 

        /** 
            Invoke the function on another object. This function takes the "this" parameter and then a variable 
                number of actual parameters to pass to the function.
            @param thisObject The object to set as the "this" object when the function is called. 
            @param args Actual parameters to the function.
            @return Any object returned as a result of applying the function
            @throws ReferenceError If the function cannot be applied to this object.
         */
        native function call(thisObject: Object, ...args): Object 

        //  MOB this should return a bound function
        /** 
            Bind the value of "this" for the function. This can set the value of "this" for the function. If
            $overwrite is false, it will only define the value of "this" if it is not already defined.
            Use $bound to examine the bound "this" value.
            @param thisObj Value of "this" to define
            @param args Function arguments to supply to the function. These arguments preceed any caller supplied
                arguments when the function is actually invoked.
         */
        native function bind(thisObj: Object, ...args): Void

        /** 
            The bound object representing the "this" object for the function. Will be set to null if no object is bound.
            Use $bind() to set the bound "this" object.
            @see bind
         */
        native function get bound(): Object?

        /**
            The name of the function. Function expressions do not have a name and set the name property to the empty string.
         */
        native function get name(): String

        //  Number of arguments expected by the function
        native function get length(): Number

        //  DOC
        /** @hide */
        native function setScope(scope: Object): Void
    }
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

/************************************************************************/
/*
    Start of file "src/core/GC.es"
 */
/************************************************************************/

/*
    GC.es -- Garbage collector class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Garbage collector control class. Singleton class to control operation of the Ejscript garbage collector.
        @spec ejs
        @stability evolving
     */
    class GC {

        use default namespace public

        /**
            Is the garbage collector is enabled.  Enabled by default.
         */
        native static function get enabled(): Boolean
        native static function set enabled(on: Boolean): Void

        /**
            The quota of work to perform before the GC will be invoked. Set to the number of work units that will 
            trigger the GC to run. This roughly corresponds to the number of allocated objects.
         */
        native static function get newQuota(): Number
        native static function set newQuota(quota: Number): Void

        /**
            Run the garbage collector and reclaim memory allocated to objects and properties that are no longer reachable. 
            When objects and properties are freed, any registered native destructors will be called. The run function will 
            run the garbage collector even if the $enable property is set to false. 
            It is normally not required to manually call the $run method as the ejs virtual machine runtime will 
            automatically run the garbage collector as required.
            @param deep If set to true, will collect from all generations. The default is to collect only the youngest
                geneartion of objects.
         */
        native static function run(deep: Boolean = false): void

        /**
            Verify memory. In debug builds, this call verifies all memory blocks by checking a per-block signature.
            This is very slow, so call sparingly. In release builds, this call does nothing.
         */
        native static function verify(): Void

    }
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

/************************************************************************/
/*
    Start of file "src/core/Global.es"
 */
/************************************************************************/

/*
    Global.es -- Global variables, namespaces and functions.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB - fix order

    /**
        Constant set to true in all Ejscript interpreters
        @spec ejs
      */
    public var EJSCRIPT: Boolean = true

    /** 
        The "ejs" namespace used for the core library
        @spec ejs
     */
    public namespace ejs

    /** 
        The public namespace used to make entities visible accross modules.
        @hide
     */
    public namespace public

    //  MOB - is this used as a global?
    /** 
        The internal namespace used to make entities visible within a single module only.
        @spec ejs
     */
    public namespace internal

    //  MOB -- remove iterator namespace
    /** 
        The iterator namespace used to define iterators.
        @spec ejs
     */
    public namespace iterator

    /** 
        Alias for the Boolean type
       @spec ejs
     */
    const boolean: Type

    /** 
        Alias for the Number type
     */
    const double: Type

    /** 
       Alias for the Number type
       @spec ejs
     */
    const num: Type

    /** 
        Alias for the String type
     */
    const string: Type

    /**
        Empty string value
     */
    const empty: String

    /** 
        Boolean false value.
     */
    const false: Boolean

    /** 
        Global variable space reference. The global variable references an object which is the global variable space. 
        This is useful when guaranteed access to a global variable is required. e.g. global.someName.
        @spec ejs
     */
    var global: Object

    /** 
        Null value. The null value is returned when testing a nullable variable that has not yet had a 
        value assigned or one that has had null explicitly assigned.
     */
    const null: Null

    /** 
        Infinity value.
     */
    const Infinity: Number

    /** 
        Negative infinity value.
     */
    const NegativeInfinity: Number

    /** 
        Invalid numeric value. If the numeric type is set to an integral type, the value is zero.
     */
    const NaN: Number

    /** 
        True value
     */
    const true: Boolean

    /** 
        Undefined variable value. The undefined value is returned when testing
        for a property that has not been defined. 
     */
    const undefined: Void

    /** 
        Void type value. 
        This is an alias for Void.
        @spec ejs
     */
    const void: Type

    /* Internal slot reservations */
    internal const commaProt: String
    internal const one: Number
    internal const zero: Number
    internal const length: String
    internal const minusOne: Number
    internal const emptySpace: Namespace
    internal const max: Number
    internal const min: Number
    internal const nop: Function

    /** 
        Assert a condition is true. This call tests if a condition is true by testing to see if the supplied 
        expression is true. If the expression is false, the interpreter will throw an exception.
        @param condition JavaScript expression evaluating or castable to a Boolean result.
        @throws AssertError if the condition is false.
        @spec ejs
     */
    native function assert(condition: Boolean): Void

    //  MOB - is this used?
    /** 
        Replace the base type of a type with an exact clone. 
        @param klass Class in which to replace the base class.
        @spec ejs
        @hide
     */
    native function cloneBase(klass: Type): Void

    /** 
        Dump the contents of objects. Used for debugging, this routine serializes the objects and prints to the standard
        output.
        @param args Variable number of arguments of any type
        @hide
     */
    function dump(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true}))
        }
    }

    /** @hide */
    function dumpAll(...args): Void {
        for each (var e: Object in args) {
            print(serialize(e, {pretty: true, hidden: true, baseClasses: true}))
        }
    }

    /** @hide */
    function dumpDef(...args): Void {
        for each (var o: Object in args) {
            for each (var key: Object in Object.getOwnPropertyNames(o)) {
                print(key + ": " + serialize(Object.getOwnPropertyDescriptor(o, key), {pretty: true, depth: 1}))
            }
        }
    }

    //  TODO - move to Crypt
    /** 
        Computed an MD5 sum of a string
        This function is prototype and may be renamed in a future release.
        @param str String on which to compute a checksum
        @returns An MD5 checksum
        @spec ejs
     */
    native function md5(str: String): String

    /** 
        Blend the contents of one object into another. This routine copies the properites of one object into another.
        If a property is an object reference and the "deep" option is set, the object is cloned using a deep clone. 
        Otherwise the object reference is copied.
        If the overwrite option is false, the property will only be copied if it does not already exist in the destination.
        @param dest Destination object
        @param src Source object
        @param options Control options
        @option combine Boolean. If true, then support key prefixes "+", "=", "-", "?" to add, assign and subtract 
            and conditionally assign key values.
            When adding string properties, values will be appended using a space separator. Extra spaces will not 
            be removed on subtraction.
            Arrays with string values may also be combined using the key prefixes. 
        @option overwrite Boolean. If true, then overwrite existing properties in the destination object. Defaults to true.
        @option deep Boolean. If true, then recursively copy the properties of any objects referenced in the source object.
            Otherwise, the copy is only one level deep. Defaults to true.
        @option functions Boolean. If true, then copy functions. Defaults to false.
        @option subclasses Boolean. If true, then copy subclass properties. Defaults to true.
        @option trace Boolean. If true, then trace to the App.log the copied property names.
        @returns An the destination object
        @spec ejs
        @example blend(dest, src, { overwrite: true, deep: true, functions: false, subclasses: true })
     */
    native function blend(dest: Object, src: Object, options = null): Object

    // TODO - should cache be a Path
    /** 
        Evaluate a script. Not present in ejsvm.
        @param script Script string to evaluate
        @returns the the script expression value.
     */
    native function eval(script: String, cache: String? = null): Object

    /** 
        Get the object's Unique hash id. All objects have a unique object hash. 
        @return This property accessor returns a long containing the object's unique hash identifier. 
     */ 
    native function hashcode(o: Object): Number

    /**
        Evaluate an argument to determine if it is a number.
        @param arg to evaluate.
        @return True if the argument is a number
     */
    function isNaN(arg: Number): Boolean
        arg.isNaN

    /**
        Evaluate an argument to determine if it is a finite number.
        @param arg to evaluate.
        @return True if the argument is a finite number
     */
    function isFinite(arg: Number): Boolean
        arg.isFinite

    //  TODO - should file and cache be paths?
    /** 
        Load a script or module
        @param file path name to load. File will be interpreted relative to EJSPATH if it is not found as an absolute or
            relative file name.
        @param options Option parameters
        @options cache String or Path name to save the compiled script as.
        @options reload Boolean If true, reload the module if already loaded. Defaults to true.
        @returns The value of the last expression in the loaded module
     */
    native function load(file: String, options: Object? = null): Object

    /** 
        Print the arguments to the standard output with a new line appended. This call evaluates the arguments, 
        converts the result to strings and prints the result to the standard output. Arguments are converted to 
        strings by calling their toString method. 
        @param args Variables to print
        @spec ejs
     */
    native function print(...args): void

    /** 
        Print the arguments to the standard output using the supplied format template. This call evaluates the arguments, 
        converts the result to strings and invokes String.format to format the args. The result is then printed to the
        standard output. Arguments are converted to strings by calling their toString method. 
        @param fmt Format specifier
        @param args Variables to print
        @spec ejs
     */
    function printf(fmt: String, ...args): Void
        App.outputStream.write(fmt.format(args))

    /** 
        Parse a string and convert to a primitive type
        @param str String to parse
        @param preferredType Preferred type to use if the input can be parsed multiple ways.
        @return Parsed object
     */
    native function parse(str: String, preferredType: Type? = null): Object

    /** 
        Parse a string as a floating point number 
        @param str String to parse
        @return A parsed number
     */
    function parseFloat(str: String): Number
        parse(str, Number)

    /** 
        Parse a string and convert to an integral number using the specified radix.
        @param str String to parse
        @param radix Base radix to use when parsing the number.
        @return A parsed number
     */
    native function parseInt(str: String, radix: Number = 10): Number

    /** @hide */
    function printHash(name: String, o: Object): Void
        print("%20s %X" % [name, hashcode(o)])

    /**  @hide TODO - doc */
    function instanceOf(obj: Object, target: Object): Boolean
        obj is target

    /** @hide */
    native function base64(str: String): String
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

/************************************************************************/
/*
    Start of file "src/core/Http.es"
 */
/************************************************************************/

/**
    Http.es -- HTTP client side communications
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Http object represents a Hypertext Transfer Protocol version 1.1 client connection. It is used to issue 
        HTTP requests and capture responses. It supports the HTTP/1.1 standard including methods for GET, POST, 
        PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections. 
        @spec ejs
        @stability evolving
     */
    class Http implements Stream {

        use default namespace public

        /** 
          HTTP Continue Status (100)
         */     
        static const Continue : Number = 100

        /** 
            HTTP Success Status (200) 
         */     
        static const Ok : Number = 200

        /** 
            HTTP Created Status (201) 
         */     
        static const Created : Number = 201

        /** 
            HTTP Accepted Status (202) 
         */     
        static const Accepted : Number = 202

        /** 
            HTTP Non-Authoritative Information Status (203) 
         */     
        static const NotAuthoritative : Number = 203

        /** 
            HTTP No Content Status (204)  
         */     
        static const NoContent : Number = 204

        /** 
            HTTP Reset Content Status (205) 
         */     
        static const Reset : Number = 205

        /** 
            HTTP Partial Content Status (206) 
         */     
        static const PartialContent : Number = 206

        /** 
            HTTP Multiple Choices Status (300) 
         */     
        static const MultipleChoice : Number = 300

        /** 
            HTTP Moved Permanently Status (301) 
         */     
        static const MovedPermanently : Number = 301

        /** 
            HTTP Found but Moved Temporily Status (302) 
         */     
        static const MovedTemporarily : Number = 302

        /** 
            HTTP See Other Status (303) 
         */     
        static const SeeOther : Number = 303

        /** 
            HTTP Not Modified Status (304)     
         */
        static const NotModified : Number = 304

        /** 
            HTTP Use Proxy Status (305) 
         */     
        static const UseProxy : Number = 305

        /** 
            HTTP Bad Request Status(400) 
         */     
        static const BadRequest : Number = 400

        /** 
            HTTP Unauthorized Status (401) 
         */     
        static const Unauthorized : Number = 401

        /** 
            HTTP Payment Required Status (402) 
         */     
        static const PaymentRequired : Number = 402

        /** 
            HTTP Forbidden Status (403)  
         */     
        static const Forbidden : Number = 403

        /** 
            HTTP Not Found Status (404) 
         */     
        static const NotFound : Number = 404

        /** 
            HTTP Method Not Allowed Status (405) 
         */     
        static const BadMethod : Number = 405

        /** 
            HTTP Not Acceptable Status (406) 
         */     
        static const NotAcceptable : Number = 406

        /** 
            HTTP ProxyAuthentication Required Status (407) 
         */     
        static const ProxyAuthRequired : Number = 407

        /** 
            HTTP Request Timeout Status (408) 
         */     
        static const RequestTimeout : Number = 408

        /** 
            HTTP Conflict Status (409) 
         */     
        static const Conflict : Number = 409

        /** 
            HTTP Gone Status (410) 
         */     
        static const Gone : Number = 410

        /** 
            HTTP Length Required Status (411) 
         */     
        static const LengthRequired : Number = 411
        
        /** 
            HTTP Precondition Failed Status (412) 
         */     
        static const PrecondFailed : Number = 412

        /** 
            HTTP Request Entity Too Large Status (413) 
         */     
        static const EntityTooLarge : Number = 413

        /** 
            HTTP Request URI Too Long Status (414)  
         */     
        static const UriTooLong : Number = 414

        /** 
            HTTP Unsupported Media Type (415) 
         */     
        static const UnsupportedMedia : Number = 415

        /** 
            HTTP Requested Range Not Satisfiable (416) 
         */     
        static const BadRange : Number = 416

        /** 
            HTTP Server Error Status (500) 
         */     
        static const ServerError : Number = 500

        /** 
            HTTP Not Implemented Status (501) 
         */     
        static const NotImplemented : Number = 501

        /** 
            HTTP Bad Gateway Status (502) 
         */     
        static const BadGateway : Number = 502

        /** 
            HTTP Service Unavailable Status (503) 
         */     
        static const ServiceUnavailable : Number = 503

        /** 
            HTTP Gateway Timeout Status (504) 
         */     
        static const GatewayTimeout : Number = 504

        /** 
            HTTP Http Version Not Supported Status (505) 
         */     
        static const VersionNotSupported: Number = 505

        /** 
            Create an Http object. The object is initialized with the Uri
            @param uri The (optional) Uri to initialize with.
            @throws IOError if the Uri is malformed.
         */
        native function Http(uri: Uri? = null)

        /**
            Convenience routine to fetch a URI and return the response. 
            This routine is a simple, one-liner to fetch a remote resource using Http.
            This routine is deliberatly inconsistent with connect(), in that is accepts the URI to fetch as the
            first parameter and assumes a default method of GET. 
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
            @param method Http method. Defaults to GET.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the Uri is malformed
         */
        static function fetch(uri: Uri, method: String = 'GET', ...data): String
            Http().connect(method, uri, ...data).response

        /** 
            @duplicate Stream.async
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            Filename of the certificate bundle used to verify server certificates.
            This file contains catenated CA certificates in PEM format.
         */
        native function get ca(): Path?
        native function set ca(bundle: Path?): Void

        /** 
            @duplicate Stream.close 
            This immediately closes any open network connection and resets the http object to be ready for another 
            connection.  Connections should be explicitly closed rather than relying on the garbage collector to 
            dispose of the Http object and automatically close the connection. If you have more requests that can 
            be issued on the same network connection, use reset() rather than close to prepare for a new request 
            on the same connection.
         */
        native function close(): Void 

        /** 
            Commence a HTTP request for the current method and uri. The HTTP method should be defined via the $method 
            property and Uri via the $uri property. This routine is typically not used. Rather it is invoked via one 
            of the Http methods $get(), $head(), $post() instead. This call, and the Http method calls  may not immediately
            initiate the connection. The Http class will delay connections until $finalize() is called explicitly or 
            implicitly reading $status or response content. This enables the request content length to be determined 
            automatically for smaller requests where the request body data can be buffered and measured before sending 
            the request headers.  
            @param method Http method. This is typically "GET" or "POST"
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function connect(method: String, uri: Uri? = null, ...data): Http

        /** 
            Filename of the certificate used to identify the client.
            This file contains the client certificate in PEM format.
         */
        native function get certificate(): Path?
        native function set certificate(certFile: Path?): Void

        /** 
            Response content body length. Set to the length of the response body in bytes or -1 if no body or not known.
            To set the request body Content-Length, use setHeader("Content-Length", Length)
         */
        native function get contentLength(): Number

        /** 
            Response content type derrived from the response Http Content-Type header.
            To set the request body Content-Type, use setHeader("Content-Type", MimeType)
         */
        native function get contentType(): String

        /** 
            When the response was generated. Response date derrived from the response Http Date header.
         */
        native function get date(): Date

        /** 
            Encoding scheme for serializing strings. The default encoding is UTF-8. Not yet implemented.
            @hide
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** 
            Signals the end of any write data and flushes any buffered write data to the server. 
         */
        native function finalize(): Void 

        /** 
            Has the request output been finalized. 
            @return True if the all the output has been written.
         */
        native function get finalized(): Boolean 

        /** 
            Flush request data. 
            Flush will force buffered request data to be sent immediately to the server. 
            @duplicate Stream.flush
         */
        native function flush(dir: Number = Stream.WRITE): Void

        /** 
            Control whether redirects should be automatically followed by this Http object. When true, a redirected
            response will be followed and the redirected URL will be transparently re-fetched.  Default is false.
         */
        native function get followRedirects(): Boolean
        native function set followRedirects(flag: Boolean): Void

        /** 
            Commence a POST request with www-url encoded key=value data. See $connect() for connection details.
            This will encode each data objects as a string of "key=value" pairs separated by "&" characters.
            After writing data, $form() will call $finalize().
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional object hash of key value pairs to use as the post data. These are www-url-encoded and
                the content mime type is set to "application/x-www-form-urlencoded".
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function form(uri: Uri, data: Object): Http

        /**
FUTURE & KEEP
            Commence a POST request with form data the current uri. See $connect() for connection details.
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object.
                If null, use the previously defined uri.
            @param data Data objects to pass with the POST request. The objects are json encoded and the Content-Type is
            set to "application/json". If you require "application/x-www-form-urlencoded" encoding, use publicForm().
            @throws IOError if the request cannot be issued to the remote server.
            @hide

            function publicForm(uri: Uri, ...data): Http
                connect("POST", uri, Uri.encodeObjects(data))
         */
        native function jsonForm(uri: Uri, ...data): Http

        /** 
            Commence a GET request for the current uri. See $connect() for connection details.
            This call initiates a GET request. It does not wait for the request to complete. 
            The $get() method will call finalize. If you need to send body content with a get request, use $connect(). 
            Once initiated, one of the $read or response routines  may be used to receive the response data.
            @param uri The uri to get. This overrides any previously defined uri for the Http object. If null, use
                a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server. Once the connection has been made, 
                exceptions will not be thrown and $status must be consulted for request status.
         */
        native function get(uri: Uri? = null, ...data): Http

        /** 
            Get the (proposed) request headers that will be sent with the request. Use $headers to get the response
            headers or header(key) to get a single response header.
            @return The set of request headers that will be used when the request is sent.
         */
        native function getRequestHeaders(): Object

        /** 
            Commence a HEAD request for the current uri. See $connect() for connection details.
            @param uri The request uri. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function head(uri: Uri? = null): Http

        /** 
            Get the value of a single response header. This is a higher performance API than using response.headers["key"].
            @param key Header key value to lookup. The lookup is caseless, i.e. the key can be any case of mix of case.
            @return The header field value as a string or null if not known.
         */
        native function header(key: String): String?

        /** 
            Response headers. Use $header() to retrieve a single header value.
            Set to an object filled with all the response headers. If multiple headers of the same key value are
                defined, their contents will be catenated with a ", " separator as per the HTTP/1.1 specification.
         */
        native function get headers(): Object

        /** 
            Connection information. This object has properties for SSL certificates and ciphers.
            @hide
         */
        native function get info(): Object

        /** 
            Is the connection is utilizing SSL.
            @return True if the connection is using SSL.
         */
        native function get isSecure(): Boolean

        /** 
            Filename of the private key to use with the certificate for SSL communications.
            This file contains the client certificate in PEM format.
         */
        native function get key(): Path?
        native function set key(keyFile: Path?): Void

        /** 
            When the response content was last modified. Set to the the value of the response Http Last-Modified header.
            Set to null if not known.
         */
        native function get lastModified(): Date?

        /**
            Resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option connReuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            Chunked encoding will be used if the total body content length is unknown at the time the request headers 
            must be emitted. The Http class will typically buffer output until $flush is called and will often be able 
            to determine the content length even if a Content-Length header has not been explicitly defined. 
            @option headers Maximum number of headers in a response.
            @option header Maximum size of response headers.
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option stageBuffer Maximum stage buffer size for each direction.
            @option transmission Maximum size of outgoing body data.
            @see setLimits
          */
        native function get limits(): Object

        /** 
            Http request method for this Http object. Default is "GET". Typical methods are: GET, POST, HEAD, OPTIONS, 
            PUT, DELETE and TRACE.
         */
        native function get method(): String
        native function set method(name: String)

        /** 
            @duplicate Stream.off
         */
        native function off(name, observer: Function): Void

        //   MOB - rename "headers" => open
        //   MOB - rename "complete" => close
        /** 
            @duplicate Stream.on
            All events are called with the following signature.  The "this" object will be set to the instance object
            if the callback is a method. Otherwise, "this" will be set to the Http instance. If Function.bind may also
            be used to define the "this" object and to inject additional callback arguments. 
                function (event: String, http: Http): Void
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes. Complete is always issued whether the request errors or not.
            @event error Issued if the request does not complete successfully. This is not issued if the request 
                ompletes successfully but with a non 200 Http status code.
         */
        native function on(name, observer: Function): Http

        /** 
            Initiate a POST request. This call initiates a POST request. It does not wait for the request to complete. 
            Posted data is NOT URL encoded. If you want to post data to a form, consider using the $form method instead 
            which automatically URL encodes the data. After writing data, $post() will call $finalize(). Post data may be 
            supplied may alternatively via $write. 
            @param uri Optional request uri. If non-null, this overrides any previously defined uri for the Http object. 
                If null, use a previously defined uri.
            @param data Data objects to send with the post request. Data is written raw and is not encoded or converted. 
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function post(uri: Uri?, ...data): Http

        /** 
            SSL provider to use
            @hide
         */
        native function get provider(): String
        native function set provider(name: String): Void

        /**
            SSL providers
            @hide
          */
        static native function get providers(): Array

        /** 
            Commence a PUT request for the current uri. See $connect() for connection details.
            If a contentLength has not been previously defined for this request, chunked transfer encoding will be enabled.
            @param uri The uri to put. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Optional data objects to write to the request stream. Data is written raw and is not encoded 
                or converted.  However, put intelligently handles arrays such that, each element of the array will be 
                written. If encoding of put data is required, use the BinaryStream filter. If no putData is supplied,
                and the ContentLength header is non-zero you must call $write to supply the body data.
            @param data Optional object hash of key value pairs to use as the post data.
            @throws IOError if the request cannot be issued to the remote server.
         */
        native function put(uri: Uri?, ...data): Http

        /** 
            @duplicate Stream.read
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /** 
            Read the response as a string. This call will block and should not be used in async mode.
            @param count of bytes to read. Returns the entire response contents if count is -1.
            @returns a string of $count characters beginning at the start of the response data.
            @throws IOError if an I/O error occurs.
         */
        native function readString(count: Number = -1): String?

        /** 
            Read the request response as an array of lines. This call will block and should not be used in async mode.
            @param count of linese to read. Returns the entire response contents if count is -1.
            @returns an array of strings
            @throws IOError if an I/O error occurs.
         */
        function readLines(count: Number = -1): Array? {
            let stream: TextStream = TextStream(this)
            result = stream.readLines()
            return result
        }

        /** 
            Read the request response as an XML document. This call will block and should not be used in async mode.
            @returns the response content as an XML object 
            @throws IOError if an I/O error occurs.
         */
        function readXml(): XML?
            XML(response)

        /**
            Reset the Http object to prepare for a new request. This will discard existing headers and security 
            credentials. It will not close the connection to TCP/IP and thus Keep-Alive will be maintained.
         */
        native function reset(): Void

        /** 
            Response body content as a string. The first time this property is read, the response content will be read 
            and buffered.  Don't use this property in async mode as it will block. Set to the response as a string of 
            characters. If the response has no body content, the empty string will be returned.
            @throws IOError if an I/O error occurs.
         */
        native function get response(): String
        native function set response(data: String): Void

        /** 
            The maximum number of retries for a request. Retries are essential as the HTTP protocol permits a 
            server or network to be unreliable. The default retries is 2.
            @hide
         */
        native function get retries(): Number
        native function set retries(count: Number): Void

        /** 
            Get the ejs session cookie. This call extracts the ejs session cookie from the Http response headers.
            Ejscript sessions are identified by a client cookie which when transmitted with subsequent requests will 
            permit the server to locate the relevant session state store for the server-side application. 
            Use: setCookie(cookie) to transmit the cookie on subsquent requests.
            Will be a string or null if there is no session cookie defined.
         */
        function get sessionCookie(): String? {
            let cookie = header("Set-Cookie")
            if (cookie) {
                return cookie.match(/(-ejs-session-=[^;]*);/)[1]
            }
            return null
        }

        /**
            Set a "Cookie" header in the request headers. This is used to send a cookie to the server.
            @param cookie Cookie header value
         */
        function setCookie(cookie: String): Void
            setHeader("Cookie", cookie)

        /** 
            Set the user credentials to use if the request requires authentication.
            @param username String user name to use. If null, then reset the current credentials.
                The password may be included in the username by appending a ":" then the password. In this case
                set the password parameter to null.
            @param password Un-encrypted string password to use. Set to null to reset the current credentials.
            @param type Authentication type. Set to basic or digest. Defaults to basic.
                If not set, then the authentication type is determined by the response from the server and the 
                request will be retried if it fails with an authentication error.
         */
        native function setCredentials(username: String?, password: String?, type: String = null): Void

        /** 
            Set a request header. Use setHeaders() to set all the headers. Use $getRequestHeaders() to retrieve and examine
            the request header set.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /** 
            Set request headers. Use setHeader() to set a single header. Use $getRequestHeaders() to retrieve and examine 
            the request headers set.
            @param headers Object hash of headers to set.
            @param overwrite If true, the new set of headers completely replaces the existing set of request headers.
                If overwrite is false, a new header value will be catenated to the old value after a ", " separator.
         */
        native function setHeaders(headers: Object, overwrite: Boolean = true): Void

        /**
            Update the request resource limits. The supplied limit fields are updated.
            See $limit for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /** 
            Http response status code from the Http response status line, e.g. 200. Set to null if unknown.
            This command will block until the request completes.
         */
        native function get status(): Number?

        /** 
            Descriptive status message for the Http response. This message may come from either the HTTP response status
                line or from a local error message if the response fails to parse.
         */
        native function get statusMessage(): String

        /**
            @hide
         */
        function get success(): Boolean
            200 <= status && status < 300

        /**
            Configure request tracing for the request. The default is to trace the first line of requests and responses at
            level 2 and to trace headers at level 3. The options argument contains optional properties: rx and tx 
            (for receive and transmit tracing). The rx and tx properties may contain an object hash which describes 
            the tracing for that direction and includes any of the following fields:
            @param options. Set of trace options with properties "rx" and "tx" for receive and transmit direction tracing.
                The include property is an array of file extensions to include in tracing.
                The include property is an array of file extensions to exclude from tracing.
                The all property specifies that everything for this direction should be traced.
                The conn property specifies that new connections should be traced.
                The first property specifies that the first line of the request should be traced.
                The headers property specifies that the headers (including first line) of the request should be traced.
                The body property specifies that the body content of the request should be traced.
                The size property specifies a maximum body size in bytes that will be traced. Content beyond this limit 
                    will not be traced.
            @option transmit. Object hash with optional properties: include, exclude, first, headers, body, size.
            @option receive. Object hash with optional properties: include, exclude, conn, first, headers, body, size.
            @example:
                trace({
                    transmit: { exclude: ["gif", "png"], "headers": 3, "body": 4, size: 1000000 }
                    receive:  { "conn": 1, "headers": 2 , "body": 4, size: 1024 }
                })
          */
        native function trace(options: Object): Void

        /** 
            Upload files using multipart/mime. This routine initiates a POST request and sends the specified files
            and form fields using multipart mime encoding. This call is synchronous (blocks) until complete.
            @param uri The uri to upload to. This overrides any previously defined uri for the Http object.
            @param files Object hash of files to upload
            @param fields Object hash of of form fields to send
            @example
                fields = { name: "John Smith", address: "700 Park Avenue" }
                files = { file1: "a.txt, file2: "b.txt" }
                http.upload(URL, files, fields)
         */
        function upload(uri: String, files: Object, fields: Object? = null): Http {
            reset()
            let boundary = "<<Upload Boundary - " + md5(Date.now()) + ">>"
            setHeader("Content-Type", "multipart/form-data; boundary=" + boundary)
            post(uri)
            if (fields) {
                for (let [key,value] in fields) {
                    write('--' + boundary + "\r\n")
                    write('Content-Disposition: form-data; name=' + Uri.encode(key) + "\r\n")
                    write('Content-Type: application/x-www-form-urlencoded\r\n\r\n')
                    write(Uri.encode(value) + "\r\n")
                }
            }
            if (files is String || files is Path) {
                files = {file0: files}
            }
            if (files is Array) {
                let o = {}
                for (let [key,value] in files) {
                    o['file' + key] = value
                }
                files = o
            }
            for (let [key,file] in files) {
                write('--' + boundary + "\r\n")
                write('Content-Disposition: form-data; name=' + key + '; filename=' + Path(file).basename + "\r\n")
                write('Content-Type: ' + Uri(file).mimeType + "\r\n\r\n")

                let f = File(file, "r")
                let data = new ByteArray(System.Bufsize, false)
                while (f.read(data) > 0) {
                    let n = write(data)
                }
                f.close()
                write("\r\n")
            }
            write('--' + boundary + "--\r\n\r\n")
            finalize()
            return this
        }

        /** 
            The current Uri for this Http object. The Uri is used for the request URL when making a $connect call.
         */
        native function get uri(): Uri?
        native function set uri(newUri: Uri): Void

        /**
            Verify peer certificates. Setting verify to true will also set verifyIssuer to the same value.
         */
        native function get verify(): Boolean
        native function set verify(enable: Boolean): Void

        /**
            Verify the certificate issuer. If false, then the the certificate verification process will accept certificates
            for which the issuer cannot be verified. This permits self-signed certificates to be accepted. 
         */
        native function get verifyIssuer(): Boolean
        native function set verifyIssuer(enable: Boolean): Void

        /** 
            Wait for a request to complete. This will call $finalize() if in sync mode and the request is not already 
                finalized.
            @param timeout Timeout in milliseconds to wait for the request to complete. A timeout of zero means don't block.
            A timeout of < 0 (default), means use the default request timeout.
            @return True if the request successfully completes.
         */
        native function wait(timeout: Number = -1): Boolean

        /** 
            Write body data to the server. This will buffer the written data until either $flush() or $finalize() is called. 
            The Http "Content-Length" header should normally be set prior to writing any data for optimial data transfter.
            If the Content-Length header has not been defined, the data will be transferred using chunked transfers. 
            @duplicate Stream.write
         */
        native function write(...data): Number

        /* ***************************************** Legacy *******************************************/

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function addHeader(key: String, value: String, overwrite: Boolean = true): Void
            setHeader(key, value, overwrite)

        /** 
            @hide
            @deprecated 2.3.0
            Fetch a URL. This is a convenience method to asynchronously invoke an Http method without waiting. 
            It can be useful to wait for completion using App.waitForEvent(http, "close"))
            @param method Http method. This is typically "GET" or "POST"
            @param uri URL to fetch
            @param data Body data to send with the request. Set to null for no data. If set to null, the request
                will be finalized. If not set to null, $finalize() must be called after writing all data.
            @param callback Optional function to invoke on completion of the request.
            MOB - reimplement using pure Http and not XMLHttp
          */
        function afetch(method: String, uri: Uri, data: *, callback: Function? = null) {
            let xh = XMLHttp(this)
            xh.open(method, uri)
            xh.send(data)
            xh.onreadystatechange = function () {
                if (xh.readyState == XMLHttp.Loaded) {
                    response = xh.responseText
                    if (callback) {
                        if (callback.bound) {
                            callback()
                        } else {
                            callback.call(this)
                        }
                    }
                }
            }
        }

        /** 
            The number of response data bytes that are currently available for reading.
            @returns The number of available bytes.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        native function get available(): Number 

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get bodyLength(): Void
            contentLength

        # Config.Legacy
        function set bodyLength(value: Number): Void
            setHeader("content-length", value)

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get chunked(): Boolean
            chunksize != 0

        # Config.Legacy
        function set chunked(enable: Boolean): Void
            chunkSize = (enable) ? 8192 : 0

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get code(): Number
            status

        /** 
            @hide 
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get codeString(): String
            statusMessage

        /**
            Get the value of the content encoding of the response.
            @return A string with the content type or null if not known.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get contentEncoding(): String?
            header("content-encoding")

        /** 
            Commence a DELETE request for the current uri. See $connect() for connection details.
            @param uri The uri to delete. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @param data Data objects to send with the request. Data is written raw and is not encoded or converted. 
                However, the routine intelligently handles arrays such that, each element of the array will be written. 
            @throws IOError if the request cannot be issued to the remote server.
            @deprecated 1.0.0
            @hide
         */
        # Config.Legacy
        native function del(uri: Uri? = null, ...data): Void

        /** 
            When the response content expires. This is derrived from the response Http Expires header.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function get expires(): Date
            Date.parseUTCDate(header("expires"))

        /**
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        static function mimeType(path: String): String
            Uri(path)..mimeType

        /**
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        function setCallback(eventMask: Number, cb: Function): Void {
            on("" + eventMask, cb);
        }

        /** 
            Commence a TRACE request for the current uri. See $connect() for connection details.
            @param uri New uri to use. This overrides any previously defined uri for the Http object.
                If null, use a previously defined uri.
            @throws IOError if the request cannot be issued to the remote server.
            @hide
            @deprecated 1.0.0
         */
        # Config.Legacy
        native function trace_old(uri: Uri? = null): Void

        //  MOB - missing timeout legacy getter/setter method
    }
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

/************************************************************************/
/*
    Start of file "src/core/Inflector.es"
 */
/************************************************************************/

/**
    Inflector.es -- Mangage word transformations
 */

module ejs {

    /** 
        Inflector class to convert singular to plural and vice-versa.
        The Inflector class emulates the Rails Inflector. 
        See: http://api.rubyonrails.org/classes/ActiveSupport/Inflector.html
     */
    class Inflector {
        static var irregularMap = [
            ['child',  'children'],
            ['man',    'men'     ],
            ['move',   'moves'   ],
            ['person', 'people'  ],
            ['sex',    'sexes'   ],
        ]

        static var pluralRules = [
            [/(quiz)$/i,               "$1zes"  ],
            [/^(ox)$/i,                "$1en"   ],
            [/([m|l])ouse$/i,          "$1ice"  ],
            [/(matr|vert|ind)ix|ex$/i, "$1ices" ],
            [/(x|ch|ss|sh)$/i,         "$1es"   ],
            [/([^aeiouy]|qu)y$/i,      "$1ies"  ],
            [/(hive)$/i,               "$1s"    ],
            [/(?:([^f])fe|([lr])f)$/i, "$1$2ves"],
            [/sis$/i,                  "ses"    ],
            [/([ti])um$/i,             "$1a"    ],
            [/(buffal|tomat)o$/i,      "$1oes"  ],
            [/(bu)s$/i,                "$1ses"  ],
            [/(alias|status)$/i,       "$1es"   ],
            [/(octop|vir)us$/i,        "$1i"    ],
            [/(ax|test)is$/i,          "$1es"   ],
            [/s$/i,                    "s"      ],
            [/$/,                      "s"      ]
        ]

        static var singularRules = [
            [/(quiz)zes$/i,                                                    "$1"     ],
            [/(matr)ices$/i,                                                   "$1ix"   ],
            [/(vert|ind)ices$/i,                                               "$1ex"   ],
            [/^(ox)en/i,                                                       "$1"     ],
            [/(alias|status)es$/i,                                             "$1"     ],
            [/(octop|vir)i$/i,                                                 "$1us"   ],
            [/(cris|ax|test)es$/i,                                             "$1is"   ],
            [/(shoe)s$/i,                                                      "$1"     ],
            [/(o)es$/i,                                                        "$1"     ],
            [/(bus)es$/i,                                                      "$1"     ],
            [/([m|l])ice$/i,                                                   "$1ouse" ],
            [/(x|ch|ss|sh)es$/i,                                               "$1"     ],
            [/(m)ovies$/i,                                                     "$1ovie" ],
            [/(s)eries$/i,                                                     "$1eries"],
            [/([^aeiouy]|qu)ies$/i,                                            "$1y"    ],
            [/([lr])ves$/i,                                                    "$1f"    ],
            [/(tive)s$/i,                                                      "$1"     ],
            [/(hive)s$/i,                                                      "$1"     ],
            [/([^f])ves$/i,                                                    "$1fe"   ],
            [/(^analy)ses$/i,                                                  "$1sis"  ],
            [/((a)naly|(b)a|(d)iagno|(p)arenthe|(p)rogno|(s)ynop|(t)he)ses$/i, "$1$2sis"],
            [/([ti])a$/i,                                                      "$1um"   ],
            [/(n)ews$/i,                                                       "$1ews"  ],
            [/s$/i,                                                            ""       ]
        ]

        static var uncountable = [
            "equipment",
            "fish",
            "information",
            "money",
            "rice",
            "series",
            "sheep",
            "species",
        ]

        use default namespace public

        /**
            Add a new irregular mapping
            @param singular Singular word form
            @param plural Plural word form
         */
        static function addIrregular(singular: String, plural: String): Void
            irregularMap.append([singular, plural])

        /**
            Add a new plural rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addPlural(expr: RegExp, replacement: String): Void
            pluralRules.append([expr, replacement])

        /**
            Add a new singular rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addSingular(expr: RegExp, replacement: String): Void
            singular.append([expr, replacement])

        /**
            Add a new uncountable rule
            @param expr Regular expression to match
            @param replacement Regular expression replacement string
         */
        static function addUncountable(expr: RegExp, replacement: String): Void
            uncountable.append([expr, replacement])

        /**
            Convert a number to an ordinal string
            @param number to convert
            @return an ordinal string matching the number
         */
        static function toOrdinal(number: Number) {
            if (11 <= number % 100 && number % 100 <= 13) {
                return number + "th"
            } else switch (number % 10) {
                case  1: return number + "st"
                case  2: return number + "nd"
                case  3: return number + "rd"
                default: return number + "th"
            }
        }

        /**
            Convert a singular word to its plural form
            @param word Word to transform
            @return the plural form of a word. Return the word if it can't be converted.
        */
        static function toPlural(word: String): String {
            for each (item in uncountable) {
                if (word.toLowerCase() == item) {
                    return item
                }
            }
            for each (rule in irregularMap) {
                let [singular, plural] = rule
                if ((word.toLowerCase() == singular) || (word == plural)) {
                    return plural
                }
            }
            for each (rule in pluralRules) {
                let [regex, replacement] = rule
                if (regex.test(word)) {
                    return word.replace(regex, replacement)
                }
            }
            return word
        }

        /**
            Convert a singular word to its plural form
            @param word Word to transform
            @return the plural form of a word. Return the word if it can't be converted.
        */
        static function toSingular(word) {
            for each (item in uncountable) {
                if (word.toLowerCase() == item) {
                    return item
                }
            }
            for each (rule in irregularMap) {
                let [singular, plural] = rule
                if ((word.toLowerCase() == singular) || (word == plural)) {
                    return singular
                }
            }
            for each (rule in singularRules) {
                let [regex, replacement] = rule
                if (regex.test(word)) {
                    return word.replace(regex, replacement)
                }
            }
            return word
        }
    }

    use default namespace public

    /**
        Convert a number to an ordinal string
        @param number to convert
        @return an ordinal string matching the number
     */
    function toOrdinal(number: Number): String
        Inflector.toOrdinal(number)

    /**
        Convert a singular word to its plural form
        @param word Word to transform
        @param count Set to a number greater than 1 to force a plural
        @return the plural form of a word
     */
    function toPlural(word: String, count: Number? = null): String {
        if (count == null || count > 1) {
            return Inflector.toPlural(word)
        }
        return word
    }

    /**
        Convert a singular word to its plural form
        @param word Plural word to transform
        @return the plural form of a word
     */
    function toSingular(word: String): String
        Inflector.toSingular(word)
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

/************************************************************************/
/*
    Start of file "src/core/Iterator.es"
 */
/************************************************************************/

/**
    Iterator.es -- Iteration support via the Iterable interface and Iterator class. 

    This provides a high performance native iterator for native classes. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Iterable is an internal interface used by native types to provide iterators for use in for/in statements.
        @hide
        @stability evolving
     */
    iterator interface Iterable {
        use default namespace iterator

        /**
            Get an Iterator for use with "for ... in"
            @return An Iterator
         */
        function get(): Iterator

        /**
            Get an Iterator for use with "for each ... in"
            @return An Iterator
         */
        function getValues(): Iterator
    }

    /**
        Iterator is a helper class to implement iterators.
        @hide
     */
    iterator final class Iterator {

        use default namespace public

        /**
            Return the next element in the object.
            @return An object
            @throws StopIteration
         */
        native function next(): Object
    }

    /** 
        StopIteration class. Iterators throw the StopIteration class instance to signal the end of iteration in for/in loops.
        @spec ejs
     */
    iterator final class StopIteration {}
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

/************************************************************************/
/*
    Start of file "src/core/JSON.es"
 */
/************************************************************************/

/*
    JSON.es -- JSON class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Encoding and decoding to JavaScript Object Notation strings (JSON). 
        This class supports the JSON data exchange format as described by: 
        RFC 4627 at (http://www.ietf.org/rfc/rfc4627.txt).
        @stability evolving
     */
    final class JSON {

        use default namespace public

        /**
            Parse a string JSON representation and return an object equivalent
            @param data JSON string data to parse
            @param filter The optional filter parameter is a function that can filter and transform the results. It 
                receives each of the keys and values, and its return value is used instead of the original value. If 
                it returns what it received, then the structure is not modified. If it returns undefined then the 
                member is deleted.  The filter function is invoked with the following signature:

                function filter(key: String, value: Object): Object

                NOTE: the filter function is not yet implemented. 

            @return An object representing the JSON string.
         */
        static function parse(data: String, filter: Function? = null): Object
            deserialize(data, { filter: filter })

        /**
            Convert an object into a string JSON representation
            @param obj Object to stringify
            @param replacer an optional function that determines how object values are stringified for objects without a 
                toJSON method. The replace has the following signature:

                function replacer(key: String, value: String): String
            @param indent optional parameter for the level of indentation of nested structures. If omitted, 
                the text will be packed without whitespace. If a number, it specifies the number of spaces 
                to indent at each level. If a string, it contains the characters used to indent at each level.
                NOTE: The indent argument is not yet implemented.
            @return A JSON string representing the object
         */
        static function stringify(obj: Object, replacer: Object? = null, indent: Object? = null): String
            serialize(obj, {replacer: replacer, indent: indent})
    }


    /** 
        Convert a string into an object. This will parse a string that has been encoded via serialize. It may contain
        nested objects and arrays. This is a static method.
        @param str The string containing the object encoding.
        @return The fully constructed object or undefined if it could not be reconstructed.
        @throws IOError If the object could not be reconstructed from the string.
        @spec ejs
     */
    native function deserialize(str: String): Object?

    //  TODO -- change pretty to format: "pretty" | "compact"
    //  TODO - change to includeBases (deprecated baseClasses)
    /** 
        Encode an object as a string. This function returns a literal string for the object and all its properties. 
        If $maxDepth is sufficiently large (or zero for infinite depth), each property will be processed recursively 
        until all properties are rendered.  NOTE: the maxDepth, all and base properties are not yet supported.
        @param obj Object to serialize. If options is null, each option takes the defaults described.
        @param options Serialization options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option commas Boolean Always put commas after the last property in an object. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option indent Number|String Indentation of nested structures. If omitted, the result is packed without any
            whitespace. If a number, it represents the number of spaces to indent at each level. If a string, it is
            contains the characters to indent. If the $pretty argument is true, it overrides indent.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
        @option namespaces Boolean determining if properties will also display their namespace attributes.
            Default is false.
        @option pretty Boolean determining if a human readable output is used with new lines after each property. 
            Default is false.
        @option quotes Boolean If false, emit property names without quotes if they do not contain spaces.
        @option replacer an optional parameter that determines how object values are stringified for objects without a 
            toJSON method.  The replace has the following signature:

                function replacer(key: String, value: String): String
        @return This function returns a string containing an object literal that can be used to reinstantiate an object.
        @throws TypeError If the object could not be converted to a string.
        @spec ejs
     */ 
    native function serialize(obj: Object, options: Object? = null): String
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

/************************************************************************/
/*
    Start of file "src/core/Loader.es"
 */
/************************************************************************/

/**
    Loader.es - CommonJS module loader with require() support.
 */

module ejs {

    /**
        Loader for CommonJS modules. The Loader class provides provides a $require() function to load
            script files.
        WARNING: this will not be supported in future releases as the Harmony loader will be supported instead.
        @param id Module identifier. May be top level or may be an identifier relative to the loading script.
        @returns An object hash of exports from the module
        @spec ejs
        @stability prototype
        @hide
     */
    public function require(id: String): Object
        Loader.require(id)

    /** 
        CommonJS loader class. This is public do assist dynamic loading of ejs.cjs so namespace qualification is not needed.
        @spec ejs
        @stability prototype
     */ 
    public class Loader {
        //  TODO - doc
        public  static var mainId
        public static var signatures = {}
        public static var initializers = {}
        private static var timestamps = {}
        private static const defaultExtensions = [".es", ".js"]

        /**
            WARNING: this will not be supported in future releases as the Harmony loader will be supported instead.
            @hide
         */
        public static function init(mainId: String? = null)
            require.main = mainId

        /**
            Register a CommonJS module initializer
            @param id Unique path ID for the module. This is typically a relative path to the module
            @param fn Module initialization function
            @return The function 
         */
        public static function register(id, fn): Function {
            Loader.initializers[id] = fn
            return fn
        }

        /**
            Load a CommonJS module. The module is loaded only once unless it is modified.
            WARNING: this will not be supported in future releases as the Harmony loader will be supported instead.
            @param id Name of the module to load. The id may be an absolute path, relative path or a path fragment that is
                resolved relative to the App search path. Ids may or may not include a ".es" or ".js" extension.
            @return a hash of exported properties
         */
        public static function require(id: String, config: Object = App.config): Object {
            let exports = signatures[id]
            if (!exports || config.cache.app.reload) {
                let path: Path = locate(id, config)
                if (path.modified > timestamps[path]) {
                    //  On-demand loading of CommonJS support
                    //  DEPRECATED
                    if (config.commonJS && !global."ejs.cjs"::CommonSystem) {
                        global.load("ejs.cjs.mod")
                    }
                    return load(id, path, config)
                }
            }
            return exports
        }

        /** 
            Load a CommonJS module and return the exports object. After the first load, the script module will be compiled
            and cached as a byte-code module. It will be recompiled if the script source is modified or missing.
            @param id Unique name of the module to load. The id may be a unique ID, an absolute path, relative path or a 
                path fragment that is resolved relative to the App search path. Ids may or may not include a ".es" or 
                ".js" extension.
            @param path Optional path to the physical file corresponding to the module. If the module source code has
                changed, it will be re-compiled and then cached.
            @param codeReader Optional function to provide script code to use instead of reading from the path. 
            @return a hash of exported properties
         */
        public static function load(id: String, path: Path, config = App.config, codeReader: Function? = null): Object {
            let initializer, code
            let cache: Path? = cached(id, config)
            if (path) {
                if (cache && cache.exists && (!config.cache.app.reload || cache.modified > path.modified)) {
                    /* Cache mod file exists and is current */
                    if (initializers[path]) {
                        App.log.debug(4, "Use memory cache for \"" + path + "\"")
                        initializer = initializers[path]
                        signatures[path] = exports = {}
                        initializer(require, exports, {id: id, path: path}, null)
                        return exports
                    }
                    App.log.debug(4, "Use disk cache for \"" + path + "\" from \"" + cache + "\"")
                    try { initializer = global.load(cache); } catch {}

                } else {
                    /* Missing or out of date cache mod file */
                    //  MOB - can we eliminate preloaded and just use reload?
                    if (initializers[path] && config.app.preloaded) {
                        //  Everything compiled flat - everything in App.mod
                        //  MOB -- warning. This prevents reload working. Should rebuild all and reload.
                        initializer = initializers[path]
                        signatures[path] = exports = {}
                        App.log.debug(4, "Use preloaded \"" + path + "\"")
                        initializer(require, exports, {id: id, path: path}, null)
                        return exports
                    }
                    if (codeReader) {
                        code = codeReader(id, path)
                    } else {
                        if (!path.exists) {
                            throw "Cannot find \"" + path + "\"."
                        }
                        code = wrap(id, path.readString())
                    }
                    if (cache) {
                        App.log.debug(4, "Recompile \"" + path + "\" to \"" + cache + "\"")
                    } else {
                        App.log.debug(4, "Compile \"" + path + "\" no caching")
                    }
                    initializer = eval(code, cache)
                }
                timestamps[path] = path.modified
            }
            if (initializer == null) {
                App.log.debug(4, "Compile \"" + path + "\" to \"" + cache + "\"")
                if (codeReader) {
                    code = codeReader(id, path)
                } else {
                    throw new StateError("Cannot load " + id + ". Must provide a codeReader if path is not specified.")
                }
                initializer = eval(code, cache)
            }
            signatures[path] = exports = {}
            initializer(require, exports, {id: id, path: path}, null)
            return exports
        }

        /** @hide */
        public static function cached(id: Path, config = App.config, cachedir: Path? = null): Path? {
            config ||= App.config
            if (id && config.cache.app.enable) {
                let dir = cachedir || Path(config.dirs.cache) || Path("cache")
                if (dir.exists) {
                    return Path(dir).join(md5(id)).joinExt('.mod')
                } else {
                    App.log.debug(5, "Skip caching, missing cache directory \"" + dir + "\" at " + App.dir)
                }
            }
            return null
        }

        /** @hide */
        public static function wrap(id: String, code: String): String
            "Loader.register(\"" + id + "\", function(require, exports, module, system) {\n" + code + "\n})"

        /*  
            Locate a CommonJS module. The id can be an absolute path, a path with/without a "es" or "js" extension. 
            It will also search for the id relative to the App search path.
            @param id Path fragment to the module
            @return A full path to the module
         */
        private static function locate(id: Path, config = App.config) {
            if (id.exists) {
                return id
            } 
            //  TODO - need logging here
            let extensions = config.extensions || defaultExtensions
            for each (let dir: Path in App.search) {
                for each (ext in extensions) {
                    //  TODO - remove when typed expressions are enabled
                    let path: Path = Path(dir).join(id)
                    path = path.joinExt(ext)
                    if (path.exists) {
                        return path
                    }
                }
            }
            throw "Cannot find \"" + id + "\""
        }

        /** 
            Set the configuration options hash for require to use. Loader uses the config.extensions field to
            determine the eligible file extensions to use when searching for modules.
            @param newConfig Configuration options hash.
         */
        public static function setConfig(newConfig: Object): Void
            config = newConfig
    }
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

/************************************************************************/
/*
    Start of file "src/core/LocalCache.es"
 */
/************************************************************************/

/*
    LocalCache.es -- Local in-memory key/value cache class 
 */

module ejs {

    /**
        Fast, non-scalable, non-durable, in-memory key/value cache class. The cache is in-process and is not 
        available to other processes on the same machine. Key/value data is not persisted. Use this cache if you require
        the fastest, non-scalable key/value cache alternative. There is one cache per Application. All uses of the LocalCache
        class connect to the one cache using one domain of keys.
        @stability prototype
     */
    class LocalCache {
        use default namespace public

        /**
            Create and connect to the local cache. An application has only one cache regardless of how many LocalCache
            instances are created. All LocalCache instances connect to the same underlying cache. Multiple interpreters
            can use the local cache to reliably share information.
            @param options Configuration options.
            @option shared Create or connect to a single, shared cache. If multiple interpreters in a single
                process create shared LocalCache instances they will be able to share key/value data.
            @option lifespan Default lifespan for key values in seconds. Set to zero for a default unlimited timeout.
            @option resolution Time in milliseconds to check for expired expired keys
            @option memory Maximum memory to use for keys and data
            @option trace Trace I/O operations for debug
         */
        native function LocalCache(options: Object? = null)

        /**
            Destroy the cache
         */
        native function destroy(): Void

        /**
            Set a fixed expire date for a key. After defining an expiry, the key's lifespan will not be renewed via
            access to the key.
            @param key Key to modify.
            @param expires Date at which to expire the data. Set expires to null to cause the key to never expire.
            @return True if the key's expiry can be updated. 
         */
        native function expire(key: String~, expires: Date?): Boolean

        /**
            Increment a key's value by a given amount. This operation is atomic.
            @param key Key value to read.
            @param amount Amount by which to increment the value. This amount can be negative to achieve a decrement.
            @return The new key value
         */ 
        native function inc(key: String~, amount: Number~ = 1): Number

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option keys Maximum number of keys in the cache.
            @option lifespan Default time to preserve key data in seconds. Set to zero for an unlimited default timeout.
            @option memory Maximum memory to use for keys and data.
            @see setLimits
          */
        native function get limits(): Object

        /**
            Read a key. Read will return the keys value or null if the key is not present or expired. If options.version
            is set to true, then the call will return an object with "data" and "version" properties where version is
            the key's unique update version ID. Each time the key is updated, the version ID is automatically changed to
            a new value. Versions IDs can be used used for conditional (CAS) writes.
            @param key Key value to read.
            @param options Read options
            @option version If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return Null if the key is not present. Otherwise return key data as a string or if the options parameter 
                specified "version == true", return an object with the properties "data" for the key data and 
                "version" for the CAS version identifier.
         */
        native function read(key: String~, options: Object? = null): String?

        /**
            Remove the key and associated value from the cache
            @param key Key value to remove. If key is null, then all keys are removed.
            @return True if the key was removed.
         */
        native function remove(key: String?): Boolean

        /**
            Update the cache cache resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /**
            Write the key and associated value to the cache. The value is written according to the optional mode option.  
            The key's expiry will be updated based on the defined lifespan from the current time.
            @param key Key to modify
            @param value String value to associate with the key
            @param options Options values
            @option expires When to expire the key. Takes precedence over lifetime. 
            @option lifespan Preservation time for the key in seconds. If zero, the key will never expire.
            @option mode Mode of writing: "set" is the default and means set a new value and create if required.
                "add" means set the value only if the key does not already exist. "append" means append to any existing
                value and create if required. "prepend" means prepend to any existing value and create if required.
                NOTE: only "set" is implemented.
            @option throw Throw an exception rather than returning null if the version id has been updated for the key. 
            @option version Required key version for the write to succeed. A version can be obtained via the $read 
                call. If set to true, the read will return an object hash containing the data and a unique version 
                identifier for the last update to this key. This version identifier can be specified to write to peform
                an atomic CAS (check and swap) operation.
            @return The number of bytes written, returns null if the write failed due to an updated version identifier for
                the key.
         */
        native function write(key: String~, value: String~, options: Object? = null): Number
    }
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

/************************************************************************/
/*
    Start of file "src/core/Locale.es"
 */
/************************************************************************/

/*
    Locale.es - Locale specific defaults and control

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*  TODO
    Calendar Date Time
    Currency
*/

module ejs {

    /**
        Locale information.
        @stability prototype
        @hide
     */
    class Locale {

        use default namespace public

        static const textEncoding = "utf-8"

        /*  
            Configure the locale
            @param language See http://www.ics.uci.edu/pub/ietf/http/related/iso639.txt
            @param country  See http://www.chemie.fu-berlin.de/diverse/doc/ISO_3166.html
            @hide
         */
        function Locale(language: String, country: String, variant: String) {}

        # FUTURE
        var config = {
            formats: {
                currency:   "$%10f",
                Date:       "%a %e %b %H:%M",
            },
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Logger.es"
 */
/************************************************************************/

/*
    Logger.es - Log file control class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Logger objects provide a convenient and consistent method to capture and store logging information.
        Loggers can direct output to Streams and may be aggregated in a hierarchical manner. The verbosity of 
        logging can be managed and messages can be filtered.
   
        Hierarchies of loggers can be constructed by specifying a parent logger when creating a logger instance.
        For example, a logger can be created for each class in a package with all such loggers having a single parent. 
        Loggers can send log messages to their parent and inherit their parent's log level. 
        The top level logger for an application is defined by App.log.
 
        Each logger may define a filter function that returns true or false depending on whether a specific message 
        should be logged or not. A matching pattern can alternatively be used to filter messages.

        An Application will have a default Logger instance created and stored in App.log at startup. This Logger uses
        the LogFile class to communicate with the Application's native log file. This is typically initialized
        by the "--log" command line switch and otherwise defaults to send messages to the standard error console.

        @spec ejs
        @stability prototype
     */
    class Logger implements Stream {

        use default namespace public

        /** 
            Logging level for no logging.
         */
        static const Off: Number = -1 

        /** 
            Logging level for most serious errors.
         */
        static const Error: Number = 0

        /** 
            Logging level for warnings.
         */
        static const Warn: Number = 1

        /** 
            Logging level for informational messages.
         */
        static const Info: Number = 2

        /** 
            Logging level for configuration output.
         */
        static const Config: Number = 3

        /** 
            Logging level to output all messages.
         */
        static const All: Number = 9

        private var _filter: Function?
        private var _level: Number = 0
        private var _location
        private var _pattern: RegExp?
        private var _name: String

        private var _outStream: Stream?

        /** 
            Logger constructor.
            The Logger constructor can create different types of loggers based on the three (optional) arguments. 
            @param name Unique name of the logger. Loggers are typically named after the module, class or subsystem they 
            are associated with.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is provided for
                the output parameter, messages are sent to the parent for rendering.
            @param level Optional integer verbosity level. Messages with a message level less than or equal to the defined
                logger level will be emitted. Range is 0 (least verbose) to 9.
            @example:
                var file = File("progress.log", "w")
                var log = new Logger("name", file, 5)
                log.debug(2, "message")
         */
        function Logger(name: String, location, level: Number? = 0) {
            _level = level
            redirect(location)
            _name = (_outStream is Logger) ? (_outStream.name + "." + name) : name 
        }

        /** 
            Redirect log output.
            @param location Optional output stream or Logger to send messages to. If a parent Logger instance is 
                provided for the output parameter, messages are sent to the parent for rendering.
            @param level Optional integer verbosity level. Messages with a message level less than or equal to the defined
                logger level will be emitted. Range is 0 (least verbose) to 9.
         */
        function redirect(location, level: Number? = null): Void {
            if (location is Stream) {
                _outStream = location
            } else {
                location = location.toString()
                let [path, lev] = location.split(":")
                _level = lev || level || this._level
                let stream
                if (path == "stdout") {
                    stream = App.outputStream
                } else if (path == "stderr") {
                    stream = App.errorStream
                } else {
                    stream = File(path).open("wa+")
                }
                _outStream = stream
            }
            _location = location
        }

        /** 
            Sync/async mode. Not supported for Loggers.
            @hide
         */
        function get async(): Boolean
            false

        function set async(enable: Boolean): Void {
            throw "Async mode not supported"
        }

        /** 
            Close the logger 
         */
        function close(): Void {
            if (_outStream) {
                _outStream.close()
            }
            _outStream = null
        }

        /** 
            Filter function for this logger. The filter function is called with the following signature:
            with "this" set to the Logger instance. The $log parameter is set to the original logger that created the
            message.
            function filter(log: Logger, name: String, level: Number, kind: String, msg: String): Boolean
            @param fn The filter function must return true or false.
         */
        function get filter(): Function?
            _filter

        function set filter(fn: Function?): void
            _filter = fn

        /**
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (_outStream) {
                _outStream.flush(dir)
            }
        }

        /** 
            The numeric verbosity setting (0-9) of this logger. Zero is least verbose, nine is the most verbose.
            Messages with a lower (or equal) verbosity level than the logger's level are emitted.
         */
        function get level(): Number
            _level

        function set level(level: Number): void {
            _level = level
            if (_outStream is Logger) {
                _outStream.level = level
            }
        }

        /**
            The logging location parameter specified when constructing or redirecting the logger. 
         */
        function get location()
            _location

        /** 
            Matching expression to filter log messages. The match regular expression is used to match 
            against the Logger names.
         */
        function get match(): RegExp?
            _pattern

        function set match(pattern: RegExp?): void 
            _pattern = pattern

        /** 
            The name of this logger.
         */
        function get name(): String
            _name

        function set name(name: String): void
            _name = name

        /** 
            The output stream used by the logger.
         */
        function get outStream(): Stream
            _outStream

        function set outStream(stream: Stream): void
            _outStream = stream

        //  MOB - rename from debug as useful for any level zero output
        /** 
            Emit a debug message. The message level will be compared to the logger setting to determine 
            whether it will be output to the devices or not. Also, if the logger has a filter function set that 
            may filter the message out before logging.
            @param level The level of the message.
            @param msgs The string message to log.
         */
        function debug(level: Number, ...msgs): void 
            emit("", level, "", msgs.join(" ") + "\n")

        /** 
            Emit a configuration message.
            @param msgs Data to log.
         */
        function config(...msgs): void
            emit("", Config, "CONFIG", msgs.join(" ") + "\n")

        /** 
            Emit an error message.
            @param msgs Data to log.
         */
        function error(...msgs): void
            emit("", Error, "ERROR", msgs.join(" ") + "\n")

        /** 
            Emit an informational message.
            @param msgs Data to log.
         */
        function info(...msgs): void
            emit("", Info, "INFO", msgs.join(" ") + "\n")

        //  TODO - should activity take a level?
        /** 
            Emit an activity message
            @param tag Activity tag to prefix the message. The tag string is wraped in "[]".
            @param args Output string to log
            @hide
         */
        function activity(tag: String, ...args): Void {
            let msg = args.join(" ")
            let msg = "%12s %s" % (["[" + tag + "]"] + [msg]) + "\n"
            write(msg)
        }

        /** 
            @hide
         */
        function off(name, observer: Function): Void {
            throw "off is not supported"
        }

        /** 
            @hide
         */
        function on(name, observer: Function): Logger {
            throw "on is not supported"
            return this
        }

        /** 
            @hide
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? {
            throw "Read not supported"
            return null
        }

        /** 
            Write messages to the logger stream. NOTE: for the Logger class, I/O errors will not throw exceptions. 
            @duplicate Stream.write
         */
        function write(...data): Number {
            try {
                return (_outStream) ? _outStream.write(data.join(" ")) : 0
            } catch {}
            return 0
        }

        /** 
            Emit a warning message.
            @param msgs The data to log.
         */
        function warn(...msgs): void
            emit("", Warn, "WARN", msgs.join(" ") + "\n")

        /* 
            Emit a message. The message level will be compared to the logger setting to determine whether it will be 
            output to the devices or not. Also, if the logger has a filter function set that may filter the message 
            out before logging.
            @param origin Name of the logger that originated the message
            @param level The level of the message.
            @param kind Message kind (debug, info, warn, error)
            @param msg The string message to emit
         */
        private function emit(origin: String, level: Number, kind: String, msg: String): Void {
            origin ||= _name
            if (level > _level || !_outStream) {
                return
            }
            if (_pattern && !origin.match(_pattern)) {
                return
            }
            if (_filter && !filter(this, origin, level, kind, msg)) {
                return
            }
            if (_outStream is Logger) {
                _outStream.emit(origin, level, kind, msg)
            } else if (kind) {
                write(origin + ": " + kind + ": " + msg)
            } else {
                write(origin + ": " + level + ": " + msg)
            }
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Math.es"
 */
/************************************************************************/

/*
    Math.es -- Math class 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Math class provides a set of static methods for performing common arithmetic, exponential and 
        trigonometric functions. It also provides commonly used constants such as PI. See also the Number class.
        Depending on the method and the supplied argument, return values may be real numbers, NaN (not a number) 
        or positive or negative infinity.
        @stability evolving
     */
    class Math {

        use default namespace public

        /**
            Base of natural logarithms (Euler's number).
         */
        static const E: Number = 2.7182818284590452354

        /**
            Natural log of 2.
         */
        static const LN2: Number = 0.6931471805599453

        /**
            Natural log of 10.
         */
        static const LN10: Number = 2.302585092994046

        /**
            Base 2 log of e.
         */
        static const LOG2E: Number = 1.4426950408889634

        /**
            Base 10 log of e.
         */
        static const LOG10E: Number = 0.4342944819032518

        /**
            The ratio of the circumference to the diameter of a circle.
         */
        static const PI: Number = 3.1415926535897932

        /**
            Reciprocal of the square root of 2.
         */
        static const SQRT1_2: Number = 0.7071067811865476

        /**
            Square root of 2.
         */
        static const SQRT2: Number = 1.4142135623730951

        /**
            Returns the absolute value of a number (which is equal to its magnitude).
            @param value Number value to examine
            @return the absolute value.
         */
        native static function abs(value: Number): Number 

        /**
            Calculates the arc cosine of an angle (in radians).
            @param angle In radians 
            @return The arc cosine of the argument 
         */
        native static function acos(angle: Number): Number 

        /**
            Calculates the arc sine of an angle (in radians).
            @param oper The operand.
            @return The arc sine of the argument 
         */
        native static function asin(oper: Number): Number 

        /**
            Calculates the arc tangent of an angle (in radians).
            @param oper The operand.
            @return The arc tanget of the argument 
         */
        native static function atan(oper: Number): Number 

        /**
            Calculates the arc tangent of the quotient of its arguments
            @param x the x operand.
            @param y the y operand.
            @return The arc tanget of the argument 
         */
        native static function atan2(y: Number, x: Number): Number 

        /**
            Return the smallest integer greater then this number.
            @return The ceiling
         */
        native static function ceil(oper: Number): Number

        /**
            Calculates the cosine of an angle (in radians).
            @param angle In radians 
            @return The cosine of the argument 
         */
        native static function cos(angle: Number): Number 
        
        /**
            Calculate E to the power of the argument
         */
        native static function exp(power: Number): Number 

        /**
            Returns the largest integer smaller then the argument.
            @param oper The operand.
            @return The floor
         */
        native static function floor(oper: Number): Number

        /**
            Calculates the log (base 10) of a number.
            @param oper The operand.
            @return The natural log of the argument
            @return The base 10 log of the argument
            @spec ejs
         */
        native static function log10(oper: Number): Number 
        
        /**
            Calculates the natural log (ln) of a number.
            @param oper The operand.
            @return The natural log of the argument
         */
        native static function log(oper: Number): Number 

        /**
            Returns the greater of the number or the argument.
            @param x First number to compare
            @param y Second number to compare
            @return A number
         */
        native static function max(x: Number, y: Number): Number

        /**
            Returns the lessor of the number or the argument.
            @param x First number to compare
            @param y Second number to compare
            @return A number
         */
        native static function min(x: Number, y: Number): Number

        /**
            Returns a number which is equal to this number raised to the power of the argument.
            @param num The number to raise to the power
            @param pow The exponent to raise @num to
            @return A number
         */
        native static function pow(num: Number, pow: Number): Number

        /**
            Generates a random number (a Number) inclusively between 0.0 and 1.0.
            @return A random number
         */
        native static function random(): Number 

        /**
            Round this number down to the closes integral value.
            @param num Number to round
            @return A rounded number
         */
        native static function round(num: Number): Number

        /**
            Calculates the sine of an angle (in radians).
            @param angle In radians 
            @return The sine of the argument 
         */
        native static function sin(angle: Number): Number 
        
        /**
            Calculates the square root of a number.
            @param oper The operand.
            @return The square root of the argument
         */
        native static function sqrt(oper: Number): Number 

        /**
            Calculates the tangent of an angle (in radians).
            @param angle In radians 
            @return The tangent of the argument 
         */
        native static function tan(angle: Number): Number 
    }
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

/************************************************************************/
/*
    Start of file "src/core/Memory.es"
 */
/************************************************************************/

/*
    Memory.es -- Memory statistics
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Singleton class to monitor and report on memory allocation and using.
        @stability evolving
        @spec ejs
     */
    class Memory {

        use default namespace public

        /**
            Total heap memory currently allocated by the application in bytes. This includes memory currently in use and 
            also memory that has been freed but is still retained by the application for future use. It does not include 
            code, static data or stack memory. If you require these, use the $resident call.
         */
        native static function get allocated(): Number

        //  TODO -- should use observers not callbacks
        /**
            Memory redline callback. When the memory redline limit is exceeded, the callback will be invoked. 
            If no callback is defined and the redline limit is exceeded, a MemoryError exception is thrown. This callback
            enables the application detect low memory conditions before they become critical and to recover by freeing 
            memory or to gracefully exit.  While the callback is active subsequent invocations of the callback are 
            suppressed.  The callback is invoked with the following signature:
                function callback(size: Number, total: Number): Void
         */
        native static function get callback(): Void
        native static function set callback(fn: Function): Void


        /**
            Maximum amount of heap memory the application may use in bytes. This defines the upper limit for heap memory 
            usage by the entire hosting application. If this limit is reached, subsequent memory allocations will fail and 
            a $MemoryError exception will be thrown. Setting it to zero will allow unlimited memory allocations up 
            to the system imposed maximum. If $redline is defined and non-zero, the redline callback will be invoked 
            when the $redline is exceeded. By default, maximum is set to unlimited.
         */
        native static function get maximum(): Number

        /**
            @duplicate Memory.maximum
            @param value New maximum value in bytes
         */
        native static function set maximum(value: Number): Void

        /**
            Memory redline value in bytes. When the memory redline limit is exceeded, the redline $callback will be invoked. 
            If no callback is defined, a MemoryError exception is thrown. The redline limit enables the application detect 
            low memory conditions before they become critical and to recover by freeing memory or to gracefully exit. 
            Note: the redline applies to the entire hosting application.
         */
        native static function get redline(): Number

        /**
            @duplicate Memory.redline
            @param value New memory redline limit in bytes
         */
        native static function set redline(value: Number): Void

        //  MOB BUG
        /**
            Application's current resident set in bytes. This is the total memory used to host the application and 
            includes all the the application code, data and heap. It is measured by the O/S.
            NOTE: this is currently reporting the peak resident memory and not the current resident memory.
         */
        native static function get resident(): Number

        /**
            System RAM. This is the total amount of RAM installed in the system in bytes.
         */
        native static function get system(): Number
        
        //  TODO - should go to stream
        /**
            Prints memory statistics to stdout. This is primarily used during development for performance measurement.
         */
        native static function stats(): void
    }
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

/************************************************************************/
/*
    Start of file "src/core/MprLog.es"
 */
/************************************************************************/

/*
    MprLog.es - Application Log File class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The MprLog class manages the Application's internal MPR log mechanism. If the Application was started with a 
        "--log" command line switch, the application will write log messages to the specified file, otherwise 
        messages will be sent to the standard error console. The Application will have a single instance of the 
        MprLog class created and stored in the App.mprLog property.

        The Logger class can be used to create higher level logging filters and aggregators and use the MprLog as the
        final output stream.

        @spec ejs
        @stability prototype
     */
    class MprLog implements Stream {

        use default namespace public

        /** 
            Sync/async mode. Not supported.
            @hide
         */
        function get async(): Boolean
            false

        function set async(enable: Boolean): Void {
            throw "Async mode not supported"
        }

        /** 
            Close the MprLog and stop logging
            @hide
         */
        function close(): Void {}

        /** 
            Emit messages to the MprLog stream at a given level
            @param level Verbosity level at which to emit the message (0-9).
            @param data Data messages to emit
         */
        native function emit(level: Number, ...data): Number

        /**
            Is the MPR logging fixed. Set to true if the application logging has been fixed by a hosting environment or
            if the application initiated logging via a command line --log switch.
         */
        native function get fixed(): Boolean
        native function set fixed(on: Boolean): Void

        /**
            @hide
         */
        function flush(dir: Number = Stream.BOTH): Void { }

        /** 
            The numeric verbosity setting (0-9) of this MprLog. Zero is least verbose, nine is the most verbose.
            Messages with a lower (or equal) verbosity level than the MprLog's level are emitted.
            WARNING: Changing the logging verbosity level will affect logging for all interpreters.
         */
        native function get level(): Number
        native function set level(level: Number): void 

        /** @hide */
        function off(name, observer: Function): Void {
            throw "off is not supported"
        }

        /** @hide */
        function on(name, observer: Function): MprLogger {
            throw "on is not supported"
            return this
        }

        /** @hide */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? {
            throw "Read not supported"
            return null
        }

        /** 
            Redirect log output. WARNING: Redirecting the logging output will redirect messages for all interpreters.
            @param location Output location to send messages to.
            @param level Verbosity level for logging. 
         */
        native function redirect(location: String, level: Number? = null): Void

        /** 
            Write messages to the MprLog stream at level 0
            @duplicate Stream.write
         */
        function write(...data): Number
            emit(0, ...data)
    }
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

/************************************************************************/
/*
    Start of file "src/core/Name.es"
 */
/************************************************************************/

/*
    Name.es -- Name class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        @hide
        @stability prototype
     */
    # FUTURE
    final class Name {
        use default namespace public

        const qualifier: Namespace
        const identifier: Namespace

        native function Name(qual: String, id: String? = undefined)
    }
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

/************************************************************************/
/*
    Start of file "src/core/Namespace.es"
 */
/************************************************************************/

/*
    Namespace.es -- Namespace class

    Copyright (c) All Rights Reserved. See details at the end of the file.

    NOTE: this is only partially implemented.
 */

module ejs {

    /**
        Namespaces are used to qualify names into discrete spaces
        @hide
        @stability prototype
     */
    final class Namespace { }
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

/************************************************************************/
/*
    Start of file "src/core/Null.es"
 */
/************************************************************************/

/*
    Null.es -- Null class used for the null value.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Base type for the null value. There is only one instance of the Null type and that is the null value.
        @spec ejs
        @stability evolving
     */
    final class Null {

        /* WARNING: Do not add properties here. Null must have no properties beyond those inherited by Object */

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function get(): Iterator

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function getValues(): Iterator
    }
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

/************************************************************************/
/*
    Start of file "src/core/Number.es"
 */
/************************************************************************/

/*
    Number.es - Number class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Number type is used by all numeric values in Ejscript. Depending on how Ejscript is configured, the underlying
        number representation may be based on either an int, long, int64 or double data type. If the underlying type is
        integral (not double) then some of these routines may not be relevant.
        @stability evolving
     */
    final class Number {

        use default namespace public

        /**
            Number constructor.
            @param value Value to use in creating the Number object. If the value cannot be converted to a number, 
                the value will ba NaN (or 0 if using integer numerics).
         */
        native function Number(value: Object? = null)

        /**
            Return the maximim value this number type can assume. Alias for MaxValue.
            An object of the appropriate number with its value set to the maximum value allowed.
         */
        static const MAX_VALUE: Number = MaxValue

        /**
            Return the minimum value this number type can assume. Alias for MinValue.
            An object of the appropriate number with its value set to the minimum value allowed.
         */
        static const MIN_VALUE: Number = MinValue

        /**
            Not a Number. This is the result of an arithmetic expression which has no value.
         */
        static const NaN : Number = NaN

        /**
            Return a unique value which is less than or equal then any value which the number can assume. 
            @return A number with its value set to -Infinity. If the numeric type is integral, then return zero.
         */
        static const NEGATIVE_INFINITY: Number = NegativeInfinity

        /**
            Return a unique value which is greater then any value which the number can assume. 
            @return A number with its value set to Infinity. If the numeric type is integral, then return MaxValue.
         */
        static const POSITIVE_INFINITY: Number = Infinity

        /**
            Return the maximim value this number type can assume.
            @return A number with its value set to the maximum value allowed.
            @spec ejs
         */
        native static const MaxValue: Number

        /** @hide */
        static const MaxInt32: Number = 2147483647

        /**
            Return the minimum value this number type can assume.
            @return A number with its value set to the minimum value allowed.
            @spec ejs
         */
        native static const MinValue: Number

        /**
            The absolute value of a number (which is equal to its magnitude).
            @spec ejs
         */
        function get abs(): Number
            Math.abs(this)

        /**
            The smallest integral number that is greater or equal to the number value. 
            @spec ejs
         */
        function get ceil(): Number 
            Math.ceil(this)

        /**
            The largest integral number that is smaller than the number value.
            @spec ejs
         */
        function get floor(): Number
            Math.floor(this)

        /**
            Is the number is not Infinity or -Infinity. This is set to true or false.
            @spec ejs
         */
        native function get isFinite(): Boolean

        /**
            Is the number equal to the NaN value. If the numeric type is integral, this will always return false.
            @spec ejs
         */
        native function get isNaN(): Boolean

        /**
            Integral number that is closest to this number. ie. rounded up or down to the closest integer.
            @spec ejs
         */
        function get round(): Number
            Math.round(this)

        /**
            Returns the number formatted as a string in scientific notation with one digit before the decimal point 
            and the argument number of digits after it.
            @param fractionDigits The number of digits in the fraction.
            @return A string representing the number.
         */
        native function toExponential(fractionDigits: Number = 0): String

        /**
            Returns the number formatted as a string with the specified number of digits after the decimal point.
            @param fractionDigits The number of digits in the fraction.
            @return A string representing the number 
         */
        native function toFixed(fractionDigits: Number = 0): String

        /**
            Returns the number formatted as a string in either fixed or exponential notation with argument number of digits.
            @param numDigits The number of digits in the result. If omitted, the entire number is returned.
            @return A string
         */
        native function toPrecision(numDigits: Number = MAX_VALUE): String

        /**
            Byte sized integral number. Numbers are rounded and truncated as necessary.
            @return A byte
            @spec ejs
         */
        function get byte(): Number
            integral(8)

        /**
            Convert this number to an integral value of the specified number of bits. Floating point numbers are 
                converted to integral values using truncation.
            @size Size in bits of the value
            @return An integral number
            @spec ejs
         */
        native function integral(size: Number = 32): Number

        /**
            Return an iterator that can be used to iterate a given number of times. This is used in for/in statements.
            @return an iterator
            @example
                for (i in 5) 
                    print(i)
            @spec ejs
         */
        override iterator native function get(): Iterator

        /**
            Return an iterator that can be used to iterate a given number of times. This is used in for/each statements.
            @return an iterator
            @example
                for each (i in 5) 
                    print(i)
            @spec ejs
         */
        override iterator native function getValues(): Iterator

        /**
            Returns the greater of the number and the arguments.
            @param other Other numbers to compare with
            @return A number
            @spec ejs
         */
        function max(...other): Number {
            let result = this
            for each (n in other) {
                n = n cast Number
                if (n > result) {
                    result = n
                }
            }
            return result
        }

        /**
            Returns the lessor of the number and the arguments.
            @param other Numbers to compare with
            @return A number
            @spec ejs
         */
        function min(...other): Number {
            let result = this
            for each (n in other) {
                n = n cast Number
                if (n < result) {
                    result = n
                }
            }
            return result
        }

        /**
            Returns a number which is equal to this number raised to the power of the argument.
            @param nth Nth Power to be raised to
            @return A number
            @spec ejs
         */
        function power(nth: Number): Number
            Math.pow(this, nth)

        //  TODO - radix
        /**
            This function converts the number to a string representation.
            @param radix Radix to use for the conversion. Defaults to 10. Non-default radixes are currently not supported.
            @returns a string representation of the number.
         */ 
        override native function toString(radix: Number = 10): String
    }
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

/************************************************************************/
/*
    Start of file "src/core/Object.es"
 */
/************************************************************************/

/*
    Object.es -- Object class. Base class for all types.
    Copyright (c) All Rights Reserved. See details at the end of this file.
 */

module ejs {
    /** 
        The Object Class is the root class from which all objects are based. It provides a foundation set of functions 
        and properties which are available to all objects. It provides for: copying objects, evaluating equality of 
        objects, providing information about base classes, serialization and deserialization and iteration. 
        @stability evolving
     */
    dynamic class Object implements Iterable {

        use default namespace public

        /**
            The constructor function from which this object was created.
         */
        native function get constructor(): Function

        /** 
            Clone the object
            @param deep If true, do a deep copy where all object references are also copied, and so on, recursively.
            A shallow clone will do 1 level deep. Deep clones are N-level deep.
            @spec ejs
         */
        native function clone(deep: Boolean = true): Object

        /** 
            Create a new object, set the object prototype and properties.
            @param prototype Prototype object
            @param properties Properties for the new object
            @return The created object
         */
        static native function create(prototype: Object, properties: Object? = undefined): Object 

        /** 
            Define or redefine a property on the given object
            @param obj Object on which to define the property
            @param name Property name
            @param options Property descriptor object. The descriptor has properties for: configurable, enumerable, get, 
                namespace, set, value, and writable.
            @option configurable If true, make the property configurable. This means it can be deleted and have its
                properties modified.
            @option enumerable If true, make the property visible to for/in enumerations.
            @option get Getter function to return the property value
            @option namespace Property namespace qualifier
            @option set Setter function to update the property value
            @option value Initial property value. Cannot use if a get or set property is defined.
            @option writable If true, the property value may be updated after the initial definition.
        */
        static native function defineProperty(obj: Object, name: String, options: Object): Void

        /**
            Define a set of properties on the given object
            @param obj Object on which to define the property
            @param properties Hash of property descriptors
         */
        static function defineProperties(obj: Object, properties: Object): Void {
            for (p in properties) {
                defineProperty(obj, p, properties[p])
            }
        }

        /** 
            Freeze an object. This freezes the object in its current configuration. It makes all properties readonly 
            and thens seals the object preventing any properties from changing their configuration. It is useful as 
            the ultimate lock-down for objects.
            @param obj Object to freeze
         */
        static native function freeze(obj: Object): Void

/* FUTURE 
        get options
        @option baseClasses Boolean determining if base class properties will be serialized. Defaults to false.
        @option depth Number indiciating the depth to recurse when converting properties to literals. If set to zero, 
            the depth is infinite. Defaults to zero.
        @option hidden Booean determining if hidden properties are serialized. If true, hidden properties will be
            serialized. Defaults to false.
*/
        /** 
            Get an iterator for this object to be used by "for (v in obj)"
            @param options Iteration options. Reserved for future use
            @return An iterator object.
            @spec ejs
         */
        iterator native function get(options: Object? = null): Iterator

        /** 
            Get the prototype property descriptor. This returns the property descriptor for the named property in
            the given object. Property names are interpreted using the set of currently open namespaces. 
            @param obj Object to inspect
            @param prop Property name
            @return The a property descriptor object. Return null if the property is not found
         */
        static native function getOwnPropertyDescriptor(obj: Object, prop: String): Object?

//  MOB -- inconsistent with JSON.baseClasses
        /** 
            Return an array of all property names including non-enumerable properties. This returns the bare names
            and does not include the namespace portions of the names. Use getOwnPropertyDescriptor to access property
            namespace values.
            @param obj Object to inspect
            @param options Property selection options
            @option includeBases Boolean determining if base class properties will included. Defaults to false.
            @option excludeFunctions Boolean determining if function properties will included. Defaults to false.
            @return Array of enumerable property names
         */ 
        static native function getOwnPropertyNames(obj: Object, options = null): Array

        /** 
            The number of properties in the object including non-enumerable properties.
         */
        static native function getOwnPropertyCount(obj: Object): Number

        /** 
            Get the prototype object
            @param obj Object to inspect
            @return The prototype object for the given object.
         */
        static native function getOwnPrototypeOf(obj: Object): Object

        /** 
            Get an iterator for this object to be used by "for each (v in obj)"
            @return An iterator object.
            @spec ejs
         */
        iterator native function getValues(): Iterator

        /** 
            Check if an object has a property. The property names is interpreted using the set of 
            currently open namespaces. 
            @param name Name of property to check for.
            @returns true if the object contains the specified property.
         */
        native function hasOwnProperty(name: String): Boolean

        /** 
            Test if an object is extensible
            @return True if the extensible trait of the object is true
         */
        static native function isExtensible(obj: Object): Boolean

        /** 
            Test if an object is frozen.
            @param obj Object to inspect
            @return True if the object is frozen.
         */
        static native function isFrozen(obj: Object): Boolean

        /** 
            Is this object a prototype of the nominated argument object.
            @param obj Target object to use in test.
            @returns true if this is a prototype of obj.
            TODO - should be on the prototype chain - Need a prototype ns
         */
        native function isPrototypeOf(obj: Object): Boolean

        /** 
            Test if an object is sealed.
            @param obj Object to inspect
            @return True if the object is sealed.
         */
        static native function isSealed(obj: Object): Boolean

        /** 
            Return an array of enumerable property names 
            @param obj Object to inspect
            @return Array of enumerable property names
         */ 
        static function keys(obj: Object): Array {
            let result = []
            for (key in obj) {
                result.append(key)
            }
            return result
        }

        /** 
            Prevent extensions to object. Sets the extensible property to false 
            @return The object argument to permit chaining.
         */
        static native function preventExtensions(obj: Object): Object

        /** 
            Test if the property is enumerable and visible in for/in traversals.  The property names is interpreted 
            using the set of currently open namespaces. 
            @param property Name of property to test.
            @returns true if this is a prototype of obj.
            TODO - prototype property
         */
        native function propertyIsEnumerable(property: String): Boolean

        /**
            The prototype object for objects of this type. The prototype object provides the template for instance 
            properties shared by all objects of a given type.
         */
        static native function get prototype(): Object
        static native function set prototype(v: Object): Void

        /** 
            Seal an object. This prevents changing the configuration of any property. This includes preventing property 
            deletion, changes to property descriptors or adding new properties to the object.
            @param obj Object to seal
         */ 
        static native function seal(obj: Object): Void

        /**
            Sort and object's properties 
            @hide
         */
        static native function sortProperties(obj: Object, ascending: Boolean = true): Void

        /** 
            Convert an object to an equivalent JSON encoded string.
            @return This function returns an object literal string.
         */ 
        native function toJSON(): String

        /** 
            This function converts an object to a localized string representation. For many objects, this simply calls
            toString. If a class overrides, it may perform customized localization. For example: Date.toLocaleString
            will use the native O/S routines to encode localized dates.
            @returns a localized string representation of the object. 
         */ 
        native function toLocaleString(): String

        /** 
            This function converts an object to a string representation. Types typically override this to provide 
            the best string representation.
            @returns a string representation of the object. For Objects "[object className]" will be returned, 
            where className is set to the name of the class on which the object was based.
         */ 
        native function toString(): String

        /** 
          Return the value of the object
          @returns this object.
         */ 
        function valueOf(): String
            this


        /**
            Get the base type of a type object.
            @return A type object
         */
        native static function getBaseType(obj: Type): Type
  
        /**
            Get the name of the object if it is a function or type.
            @return The string name of the function or type
         */
        native static function getName(obj: Object): String

        /**
            Get the type for an object. If the object is an instance, this is the class type object. If the object is a
            type, this value is Type.
            @return A type object
         */
        native static function getType(obj: Object): Type
  
        /**
            Get the name of the type for an object. If the object is an instance, this is the name of the class type object.
            If the object is a type, this value is "Type". 
            @return A type object
         */
        native static function getTypeName(obj: Object): String
  
        /**
            Test if the object is a prototype object
            @return True if the object is being used as a prototype object
         */
        native static function isPrototype(obj: Object): Boolean
  
        /**
            Test if the object is a type object
            @return True if the object is a type object
         */
        native static function isType(obj: Object): Boolean
    }
  
    /**
        Return the name of a type. This is a fixed version of the standard "typeof" operator. It returns the real
        Ejscript underlying type name. This call is an alias for Object.getTypeName(o).
        @param o Object or value to examine. 
        @return A string type name. 
        @spec ejs
     */
    native function typeOf(o): String
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

/************************************************************************/
/*
    Start of file "src/core/Path.es"
 */
/************************************************************************/

/*
    Path.es --  Path class. Path's represent files in a file system. The file may or may not exist.  

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    FUTURE
        Path.expand(hash: Object = null)        Expand $VARS or ${vars} in path. Also expand ~user
        Path.split(): Array                     Return dir + base
        Path.splitRoot(): Array                 Return root + rest
        Path.splitExtension(): Array            Return rest + extension
        Path.normalizeCase                      Map to lower case
        Path.canonicalize()                     Resolve symbolic links and return normalized, validated, fully resolved path.
                                                Error if the path does not exist.
        Path.crc(): Number                      Calculate a 32 bit crc of the file contents
        Path.md5(): Number                      Calculate an MD5 digest of the file contents
        Path.touch(date: Date = null): Void     Update modified time. Create if non-existant
        Path.perms = NNN                        Permissions getter / setter
        Path.isReadable, isWritable, isHidden, isSpecial, is Mount
        Path.rmtree()                           Recursive removal
        Path.validate                           Validate a path.
 */

module ejs {

    /**
        Paths are filename objects and may or may not represent physical files in a file system. That is, the 
        corresponding file or directory for the Path  may or may not exist. Once created, Paths are immutable and their
        path value cannot be changed.
        @spec ejs
        @stability evolving
     */
    final class Path {

        use default namespace public

        /**
            Create a new Path object and set the path's location.
            @param path Name of the path to associate with this path object. 
         */
        native function Path(path: String = ".")

        /**
            An equivalent absolute path equivalent for the current path. The path is normalized and uses the native system
            directory separator.
            On Windows, this path with contain a drive specifier and will have back-slash directory separators.
            On Cygwin, this path will be a Cygwin style path without a drive specifier and with forward-slash 
            directory separators. If the path is outside the Cygwin filesystem (outside c:/cygwin), the path 
            will have a /cygdrive/DRIVE prefix. 
            To get a Windows style path, use the $windows property accessor.
         */
        native function get absolute(): Path

        /**
            When the file represented by the path was last accessed. Set to null if the file does not exist.
         */
        native function get accessed(): Date?

        /**
            Append data to a file.
            @param data Data to append to the file
            @param options File open options. Defaults to "atw"
         */
        function append(data, options = "atw"): Void {
            let file = open(options)
            file.write(data)
            file.close()
        }

        /**
            File security permissions.
            @return the file attributes object hash. Fields include: 
            @options user String representing the file user name
            @options group String representing the file group name
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @options permissions Number File Posix permissions mask
         */
        native function get attributes(): Object

        /**
            Define file security permissions.
            @options user String representing the file user name If both user and uid are specified, user takes precedence. 
            @options group String representing the file group name If both group and gid are specified, group takes
                precedence.  
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @options permissions Number File Posix permissions mask
         */
        native function setAttributes(attributes: Object): Void

        /**
            The base of portion of the path. The base portion is the trailing portion without any directory elements.
         */
        native function get basename(): Path
        
        /**
            Path components. The path is normalized and then broken into components for each directory level. 
            The result is an array object with an element for each segment of the path. If the path is an absolute path, 
            the first element represents the file system root and will be the empty string or drive spec 
            on a Windows like systems. Array.join("/") can be used to put components back into a complete path.
         */
        native function get components(): Array
  
        /**
            Test if the path name contains a substring
            @param pattern String pattern to search for
            @return Boolean True if found.
         */
        function contains(pattern: String): Boolean
            name.contains(pattern)

        /**
            Copy a file to the destination
            @param destination New file location
            @param options Object hash
            @options permissions Set to a numeric Posix permissions mask. Not implemented.
            @options user String representing the file user name
                If both user and uid are specified, user takes precedence. 
            @options group String representing the file group name
                If both group and gid are specified, group takes precedence.
            @options uid Number representing the file user id
            @options gid Number representing the file group id
         */
        native function copy(destination: Object, options: Object? = null): Void

        /**
            When the file represented by the path was created. Set to null if the file does not exist.
         */
        native function get created(): Date?

        /**
            The directory portion of a file. The directory portion is the leading portion including all 
            directory elements and excluding the base name. On some systems, it will include a drive specifier.
            See also $parent which will determine the parent directory of relative paths.
         */
        native function get dirname(): Path

        /**
            Return true if the path ends with the given suffix
            @param suffix String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: String): Boolean
            portable.name.endsWith(Path(suffix).portable)

        /**
            Does a file exist for this path.
         */
        native function get exists(): Boolean 

        /**
            File extension portion of the path. The file extension is the portion after (and not including) the last ".".
         */
        native function get extension(): String 

        /**
            TODO - should do pattern matching
            @hide
         */
        function findAbove(name: String): Path? {
            let dir: Path = this
            do {
                if (dir.join(name).exists) {
                    return dir.join(name)
                }
                dir = dir.parent
            } while (dir != dir.parent)
            return null
        }

        /**
            Do Posix glob style file matching.
            @param patterns Pattern to match files. This can be a String, Path or array of String/Paths. 
            The wildcard '?' matches any single character, '*' matches zero or more characters in a filename or 
                directory, '** /' matches zero or more files or directories and matches recursively in a directory
                tree.  If a pattern terminates with "/" it will only match directories. 
                The pattern '**' is equivalent to '** / *' (ignore spaces). 
                The Posix "[]" and "{a,b}" style expressions are not supported.
            @param options Optional properties to control the matching.
            @option depthFirst Do a depth first traversal. If "dirs" is specified, the directories will be shown after
                the files in the directory. Otherwise, directories will be listed first.
            @option exclude Regular expression pattern of files to exclude from the results. Matches the entire path.
                Only for the purpose of this match, directories will have "/" appended. To exclude directories in the
                results, use {exclude: /\/$/}. The trailing "/" will not be returned in the results.
            @option hidden Include hidden files starting with "."
            @option include Regular expression pattern of files to include in the results. Matches the entire returned path.
                Only for the purpose of this match, directories will have "/" appended. To include only directories in the
                results, use {include: /\/$/}
            @option missing Set to undefined to report patterns that don't resolve into any files or directories 
                by throwing an exception. Set to any non-null value to be used in the results when there are no matching
                files or directories. Set to the empty string to use the patterns in the results and set
                to null to do nothing.
            @option relative Return paths relative to the Path, otherwise result entries include the Path. Defaults to false.
            @return An Array of Path objects for each file in the directory.
         */
        native function files(patterns: Object! = '*', options: Object? = null): Array 

        /**
            The file system containing this Path 
            @return The FileSystem object for this path
         */
        function get fileSystem(): FileSystem
            FileSystem(this)

        /**
            Get iterate over any files contained under this path (assuming it is a directory) "for (v in files)". 
                This operates the same as getValues on a Path object.
            @return An iterator object.
            @example:
                for (f in Path("."))
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this file to be used by "for each (v in obj)". Return 
                This operates the same as $get on a Path object.
            @return An iterator object.
            @example
                for each (f in Path("."))
         */
        override iterator native function getValues(): Iterator

        /**
            Does the file path have a drive spec (C:) in it's name. Only relevant on Windows like systems.
            @return True if the file path has a drive spec
         */
        native function get hasDrive(): Boolean 

        /**
            Is the path absolute, i.e. Beginning at the file system's root.
         */
        native function get isAbsolute(): Boolean

        //  MOB - need isExecutable
        /**
            Is the path a directory. If the path is a symbolic link, the target of the link is tested.
            @return true if the file is a directory
         */
        native function get isDir(): Boolean

        /**
            Is the path is a symbolic link file. Not available on some platforms such as Windows and VxWorks.
            @return true if the file is a symbolic link
         */
        native function get isLink(): Boolean

        /**
            Is the path a regular file. If the path is a symbolic link, the target of the link is tested.
            @return true if the file is a regular file and not a directory
         */
        native function get isRegular(): Boolean

        /**
            Is the path is relative and not absolute.
         */
        native function get isRelative(): Boolean

        /**
            Join paths. Joins one more more paths together. If the other path is absolute, return it. Otherwise
            add a separator, and continue joining. The separator is chosen to match the first separator found in 
            either path. If none found, the default file system path separator is used.
            @return A new joined, normalized Path object.
         */
        native function join(...other): Path

        /**
            Join an extension to a path. If the base name of the path already has an extension, this call does nothing.
            @param ext Extension to add. The extension may or may not contain a leading period. Best practice is to 
                not have leading periods on extensions.
            @param force Set to true to always add the extension.
            @return A new Path object with the given extension if the path did not already have one.
         */
        native function joinExt(ext: String, force: Boolean = false): Path

        /**
            The length of the path name in bytes. Note: this is not the file size. For that, use Path.size
         */
        native function get length(): Number 

        /**
            Create a target link to refer to the path
            This will remove any pre-existing target link and then create a symbolic link at the target to refer to the
            path.
            @param hard Set to true to create a hard link. Otherwise the default is to create a symbolic link.
            @param target Target the path will refer to.
          */
        native function link(target: Path, hard: Boolean = false): Void

        /**
            The target pointed to if this path is a symbolic link. Not available on some platforms such as Windows and 
            VxWorks. If the path is not a symbolic link, it is set to null.
         */
        native function get linkTarget(): Path?

        /**
            Make a new directory and all required intervening directories. If the directory already exists, 
                the function returns without any further action. 
            @param options
            @options permissions Directory permissions to use for all created directories. Set to a numeric 
                Posix permissions mask.
            @options user String representing the file user.
                If both user and uid are specified, user takes precedence. 
            @options group String representing the file group.
                If both group and gid are specified, group takes precedence.
            @options uid Number representing the file user id
            @options gid Number representing the file group id
            @return True if the directory can be made or already exists
         */
        native function makeDir(options: Object? = null): Boolean

        /**
            Create a link to a file. Not available on some platforms such as Windows and VxWorks.
            @param target Path to an existing file to link to.
            @param hard Set to true to create a hard link. Otherwise the default is to create a symbolic link.
            @returns this path
            @hide
            @deprecate 2.3.0
         */
        native function makeLink(target: Path, hard: Boolean = false): Void

        /**
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            If the path object is created with an empty path value, the temporary file will be created in the standard
            system location for temp files. This is typically /tmp or /temp.
            @returns a new Path object for the temp file.
         */
        native function temp(): Path

        /**
            Get a path after mapping the path directory separator
            @param sep Path directory separator to use. Defaults to the separator for this path.
            @return a new Path after mapping separators
         */
        native function map(sep: String = separator): Path

        /**
            Match the path to a regular expression pattern
            @param pattern The regular expression pattern to search for
            @return Returns an array of matching substrings in the path
         */
        function match(pattern: RegExp): Array?
            this.toString().match(pattern)

        /** 
            Get the mime type for a path or extension string. The path's extension is used to lookup the corresponding
            mime type.
            @returns The mime type string
         */
        native function get mimeType(): String

        /**
            When the file represented by the path was last modified. Set to null if the file does not exist.
         */
        native function get modified(): Date?

        /**
            Name of the Path as a string. This is the same as $toString().
         */
        native function get name(): String 

        /**
            Natural (native) respresentation of the path for the platform. This uses the platform file system path 
            separator, this is "\" on Windows and "/" on unix and Cygwin. The returned path is normalized. 
            See also $portable for a portable representation.
         */
        native function get natural(): Path 

        /**
            Normalized representation of the path. This removes "segment/.." and "./" components. Separators are made 
            uniform by converting all separators to be the same as the first separator found in the path. Note: the 
            result is not converted to an absolute path.
         */
        native function get normalize(): Path

        /**
            Open a path and return a File object.
            @param options
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a File object which implements the Stream interface.
            @throws IOError if the path or file cannot be created.
         */
        function open(options: Object? = null): File
            new File(this, options)

        /**
            Open a file and return a TextStream object.
            @param options Optional options object
            @options mode optional file access mode string. Use "r" for read, "w" for write, "a" for append to existing
                content, "+" never truncate.
            @options encoding Text encoding format
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a TextStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openTextStream(options: Object? = null): TextStream
            new TextStream(open(options))

        /**
            Open a file and return a BinaryStream object.
            @param options Optional options object
            @options mode optional file access mode with values ored from: Read, Write, Append, Create, Open, Truncate. 
                Defaults to Read.
            @options permissions optional Posix permissions number mask. Defaults to 0664.
            @options user String representing the file user
            @options group String representing the file group
            @return a BinaryStream object which implements the Stream interface.
            @throws IOError if the path or file cannot be opened or created.
         */
        function openBinaryStream(options: Object? = null): BinaryStream
            new BinaryStream(open(options))

        /**
            The parent directory of path. This may be an absolute path if there are no parent directories in the path.
         */
        native function get parent(): Path

        /*  MOB -- different to File.permissions. Should have something that returns an object with full path/file
            attributes including group/user. The perms should be broken down into world:group:user */
        //  MOB - should not use a setter should be functions
        /**
            The file permissions of a path. This number contains the Posix style permissions value or null if the file 
            does not exist. NOTE: this is not a string representation of an octal posix mask. 
         */
        native function get perms(): Number?
        native function set perms(perms: Number): Void

        /**
            The path in a portable (like Unix) representation. This uses "/" separators. The value is is normalized and 
            the separators are mapped to "/". See also $natural for convertion to the O/S native path representation.
         */
        native function get portable(): Path 

        /**
            Read the file contents as a ByteArray. This method opens the file, reads the contents, closes the file and
                returns a new ByteArray with the contents.
            @return A ByteArray
            @throws IOError if the file cannot be read
            @example:
                var b: ByteArray = Path("/tmp/a.txt").readBytes()
         */
        function readBytes(): ByteArray? {
            let file: File = File(this).open()
            result = file.readBytes()
            file.close()
            return result
        }

        /**
            Read the file containing a JSON string and return a deserialized object. 
            This method opens the file, reads the contents, deserializes the object and closes the file.
            @return An object.
            @throws IOError if the file cannot be read
            @example:
                data = Path("/tmp/a.json").readJson()
         */
        function readJSON(): Object? {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return deserialize(result)
        }

        /**
            Read the file contents as an array of lines. Each line is a string. This method opens the file, reads the 
                contents and closes the file.
            @return An array of strings.
            @throws IOError if the file cannot be read
            @example:
                for each (line in Path("/tmp/a.txt").readLines())
         */
        function readLines(): Array? {
            let stream: TextStream = TextStream(open(this))
            result = stream.readLines()
            stream.close()
            return result
        }

        /**
            Read the file contents as a string. This method opens the file, reads the contents and closes the file.
            @return A string.
            @throws IOError if the file cannot be read
            @example:
                data = Path("/tmp/a.txt").readString()
         */
        function readString(): String? {
            let file: File = open(this)
            result = file.readString()
            file.close()
            return result
        }

        /**
            Read the file contents as an XML object.  This method opens the file, reads the contents and closes the file.
            @return An XML object
            @throws IOError if the file cannot be read
         */
        function readXML(): XML? {
            let file: File = open(this)
            let data = file.readString()
            file.close()
            if (data == null) {
                return null
            }
            return new XML(data)
        }

        /**
            That path in a form relative to the application's current working directory. The value is normalized.
         */
        native function get relative(): Path

        /**
            Get a relative path to an origin
            @param origin The origin path to use when calculating a relative path to the destination (this path).
                If origin is null, the current working directory is used as the origin.
            @return A new relative path
         */
        native function relativeTo(origin: Path? = null): Path

        /**
            Delete the file associated with the Path object. If this is a directory without contents it will be removed.
            @return True if the file is sucessfully deleted or does not exist.
         */
        native function remove(): Boolean 

        //  MOB - consider remove({contents: true})
        /**
            Removes a directory and its contents
            If the path is a directory, this call will remove all subdirectories and their contents and finally the
            directory itself. If the directory does not exist, this call does not error and does nothing.
            @return True if all the contents are sucessfully deleted or if the directory does not exist. 
                Otherwise return false.
         */
        function removeAll(): Boolean {
            let passed = true
            if (name == "" || name == "/") {
                throw new ArgError("Bad path for removeAll")
            }
            for each (f in files('**', {depthFirst: true, hidden: true})) {
                if (!f.remove()) {
                    passed = false
                }
            }
            if (!remove()) {
                passed = false
            }
            return passed
        }

        /**
            Remove the drive specifier from a path
            @hide
         */
        function removeDrive(): Path {
            let sep = separator
            return hasDrive ? (sep + components.slice(1).join(sep)) : this
        }

        /**
            Rename a file. If the new path exists it is removed before the rename.
            @param target New name of the path
            @return True if the file is renamed.
         */
        native function rename(target: Path): Boolean
        
        /**
            Replace the path extension and return a new path.
            @return A new path with the given extension.
         */
        function replaceExt(ext: String): Path {
            if (ext && ext[0] != '.') {
                ext = "." + ext
            }
            return this.trimExt() + ext
        }

        /**
            Search and replace. Search for the given pattern which can be either a string or a regular expression 
            and replace it with the replace text. If the pattern is a string, only the first occurrence is replaced.
            @param pattern The regular expression or string pattern to search for.
            @param replacement The string to replace the match with or a function to generate the replacement text. The
                replacement string can contain special replacement patterns: "$$" inserts a "\$", "\$&" inserts the
                matched substring, "\$`" inserts the portion that preceeds the matched substring, "\$'" inserts the
                portion that follows the matched substring, and "\$N" inserts the Nth parenthesized substring.
                The replacement parameter can also be a function which will be invoked and the function return value 
                will be used as the resplacement text. The function will be invoked multiple times for each match to be 
                replaced if the regular expression is global. The function will be invoked with the signature:

                function (matched, submatch_1, submatch_2, ..., matched_offset, original_string)
            @return Returns a new string.
            @spec ejs
         */
        function replace(pattern, replacement): Path
            Path(name.replace(pattern, replacement))

        /**
            Resolve paths in the neighborhood of this path. Resolve operates like join, except that it joins the 
            given paths to the directory portion of the current ("this") path. For example: 
            Path("/usr/bin/ejs/bin").resolve("lib") will return "/usr/lib/ejs/lib". i.e. it will return the
            sibling directory "lib".

            Resolve operates by determining a virtual current directory for this Path object. It then successively 
            joins the given paths to the directory portion of the current result. If the next path is an absolute path, 
            it is used unmodified.  The effect is to find the given paths with a virtual current directory set to the 
            directory containing the prior path.

            Resolve is useful for creating paths in the region of the current path and gracefully handles both 
            absolute and relative path segments.
            @param otherPaths Paths to resolve in the region of this path.
            @return A new Path object that is resolved against the prior path. 
         */
        native function resolve(...otherPaths): Path

        /**
            That root directory component of the path. On windows, this includes the drive spec if present in the path.
          */
        native function get root(): Path

        /**
            Compare two paths test if they represent the same file
            @param other Other path to compare with
            @return True if the paths represent the same underlying filename
         */
        native function same(other: Object): Boolean

        /**
            The path separator for this path. This will return the first valid path separator used by the path
            or the default file system path separator if the path has no separators. On Windows, a path may contain
            "/" and "\" separators.  This will be set to a string containing the first separator found in the path.
            Will typically be either "/" or "/\\" depending on the path, platform and file system.
            Use $natural, $portable or $windows to create a new path with different path separators.
         */
        native function get separator(): String

        /**
            Size of the file associated with this Path object. Set to the number of bytes in the file or -1 if the
            size determination failed.
         */
        native function get size(): Number 

        /**
            Return true if the path starts with the given prefix
            @param prefix String prefix to compare with the path.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: String): Boolean
            portable.name.startsWith(Path(prefix).portable)

        //  MOB - symlink is backwards
        /**
            Create the path as a symbolic link to refer to the target
            This will remove any pre-existing path and then create a symbolic link at path to refer to the target.
            @param target Target the path will refer to.
            @hide
            @deprecate 2.3.0
          */
        native function symlink(target: Path): Void

        /**
            Convert the path to a JSON string. 
            @return a JSON string representing the path.
         */
        native override function toJSON(): String

        /**
            Convert the path to lower case
            @return a new Path mapped to lower case
         */
        function toLowerCase(): Path
            new Path(name.toString().toLowerCase())

        /**
            Return the path as a string. The path is not translated.
            @return a string representing the path.
         */
        native override function toString(): String

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the end of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimEnd(pat: String): Path {
            pat = Path(pat).portable
            if (name.endsWith(pat)) {
                loc = name.lastIndexOf(pat)
                if (loc >= 0) {
                    return new Path(name.slice(0, loc))
                }
            }
            return this
        }

        /**
            Trim a file extension from a path
            @return a Path with no extension
         */
        native function trimExt(): Path

        //  TODO - should support reg expressions
        /**
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match
            @return a Path containing the trimmed path name
         */
        function trimStart(pat: String): Path {
            if (name.startsWith(pat)) {
                return new Path(name.slice(pat.length))
            }
            return this
        }

        /**
            Reduce the size of the file by truncating it. 
            @param size The new size of the file. If the truncate argument is greater than or equal to the 
                current file size nothing happens.
            @throws IOError if the truncate failed.
         */
        native function truncate(size: Number): Void

        /**
            An equivalent normalized Windows path equivalent for the current path.
         */
        native function get windows(): Path

        //  MOB rename? - bit confusing "write". This really does a "save"
        //  MOB - rename to writecContents or save
        //  MOB - last arg should be permissions?
        /**
            Write the file contents. This method opens the file, writes the contents and closes the file.
            @param args The data to write to the file. Data is serialized in before writing. Note that numbers will not 
                be written in a cross platform manner. If that is required, use the BinaryStream class to write Numbers.
            @throws IOError if the data cannot be written
         */
        function write(...args): Void {
            var file: File = new File(this, { mode: "w", permissions: 0644 })
            try {
                for each (item in args) {
                    if (item is String) {
                        file.write(item)
                    } else {
                        file.write(serialize(item))
                    }
                }
            } 
            catch (es) {
                throw new IOError("Cannot write to file")
            }
            finally {
                file.close()
            }
        }

        /** 
            Create a new temporary file. The temp file is located in the directory specified by the Path object. 
            @returns a new Path object for the temp file.
            @hide
            @deprecated 1.0.0
         */
        function makeTemp(): Path
            temp()

        /**
            Return a compact representation of the path
            This returns a relative path if the path is under the given home directory. Otherwise it returns an 
            absolute path.
            @param home Absolute path to a base home directory. Defaults to the current directory.
            @return an equivalent Path
            @hide
         */
        function compact(home: Path = App.dir): Path {
            let p = absolute
            return p.startsWith(home) ? p.relativeTo(home) : p
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Promise.es"
 */
/************************************************************************/

/*
    Promise.es -- Promise keeper for deferred execution of async APIs.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        The Promise class permits deferred processing for async APIs. A Promise encapsulates callbacks and state for an API
        that will take some time to execute. The API can return the promise and the caller can register callbacks for events
        of interest.
        WARNING: The CommonJS spec for promises is still changing 
        @spec commonjs
        @stability prototype
        @hide
     */
    dynamic class Promise extends Emitter {
        private var timer: Timer?
        private var complete: Boolean

        use default namespace public 

        /** 
            Add a callback observer for the "success" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onSuccess(observer: Function): Promise {
            on("success", observer)
            return this
        }

        /** 
            Add a cancel callback observer for the "cancel" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onCancel(observer: Function): Promise {
            on("cancel", observer)
            return this
        }

        /** 
            Add an error callback observer for the "error" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onError(observer: Function): Promise {
            on("error", observer)
            return this
        }

        /** 
            Add a progress callback observer for the "progress" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onProgress(observer: Function): Promise {
            on("progress", observer)
            return this
        }

        /** 
            Add a timeout callback observer for the "timeout" event. Returns this promise object.
            @param observer Callback function
            @return Returns this promise
         */
        function onTimeout(observer: Function): Promise {
            on("timeout", observer)
            return this
        }

        /** 
            Issue a "success" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitSuccess(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("success", ...args)
            } catch (e) {
                //  MOB
                print("CATCH", e)
                emitError(e)
            }
        }

        /** 
            Issue an "error" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitError(...args): Void {
            if (complete) {
                return
            }
            complete = true
            try {
                issue("error", ...args)
            } catch (e) {
                //  MOB -- use logging
                print("EmitError CATCH", e)
            }
        }

        /** 
            Issue an "cancel" event with the given arguments. Once a result for the promise has been emitted via emitSucces,
            emitError or emitCancel, the Promise in completed and will not emit further events.
            @param args Args to pass to the observer
         */
        function emitCancel(...args): Void
            issue("cancel", ...args)

//  MOB -- why have cancel and emitCancel
        /** 
            Cancels the promise and removes "success" and "error" observers then issues a cancel event.
            @param args Args to pass to the "cancel" event observer
         */
        function cancel(...args): Void {
            complete = true
            if (timer) {
                timer.stop()
            }
            clearObservers(["success", "error"])
            issue("cancel", ...args)
        }

//  MOB -- what about cancel?
        /** 
            Convenience function to register callbacks. 
            @param success Success callback passed to onSuccess
            @param error error callback passed to onError
            @param cancel Cancel callback passed to onCancel
            @param progress Progress callback passed to onProgress
            @return this promise
         */
        function then(success: Function, error: Function? = null, cancel: Function? = null, 
                progress: Function? = null): Promise {
            on("success", success)
            if (error) {
                on("error", error)
            }
            if (cancel) {
                on("cancel", cancel)
            }
            if (progress) {
                on("progress", progress)
            }
            return this
        }

        /** 
            Create a timeout for the current promise. If the timeout expires before the promise completes or is cancelled, a
            "timeout" event and then an "error" event will be issued.
            @param msec Timeout in milliseconds
            @return this promise 
         */
        function timeout(msec: Number): Promise {
            if (timer) {
                timer.stop()
            }
            let timeoutComplete
            function wakeup(arg) {
                timeoutComplete = true
                if (timer) {
                    timer.stop()
                }
            }
            on("success", wakeup)
            on("error", wakeup)
            on("cancel", wakeup)
            timer = new Timer(msec, function() {
                if (complete || timeoutComplete) {
                    return;
                }
                timeoutComplete = true
                timer = null
                issue("timeout")
                issue("error")
            })
            timer.start()
            return this
        }
        
        //  MOB - fix MaxInt
        /** 
            Wait for the promise to complete for a given period. This blocks execution until the promise completes or 
            is cancelled.
            @param timeout Time to wait in milliseconds
            @return The arguments array provided to emitSuccess
         */
        function wait(timeout: Number = Number.MaxInt32): Object {
            let waitComplete = false
            let result
            function wakeup(event, ...args) {
                waitComplete = true
                result = args
            }
            on(["cancel", "error", "success"], wakeup)
            let t = new Timer(timeout, wakeup)
            t.start()
            while (!waitComplete && !complete) {
                App.run(timeout, true)
            }
            t.stop()
            return result
        }

        private function issue(name: String, ...args): Void {
            if (timer) {
                timer.stop()
            }
            if (args) {
                fire(name, ...args)
            } else {
                fire(name)
            }
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/RegExp.es"
 */
/************************************************************************/

/*
    Regex.es -- Regular expression class.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Regular expressions per ECMA-262. The following special characters are supported:
        <table class="itemTable">
            <tr><td>\\</td><td>Reverse whether a character is treated literally or not.</td></tr>
            <tr><td>^</td><td>Match to the start of input. If multiline, match starting after a line break.</td></tr>
            <tr><td>\$ </td><td>Match to the end of input. If matching multiline, match before after a line break.</td></tr>
            <tr><td>*</td><td>Match the preceding item zero or more times.</td></tr>
            <tr><td>+</td><td>Match the preceding item one or more times.</td></tr>
            <tr><td>?</td><td>Match the preceding item zero or one times.</td></tr>
            <tr><td>(mem)</td><td>Match inside the parenthesis (i.e. "mem") and store the match.</td></tr>
            <tr><td>(?:nomem)</td><td>Match "nomem" and do not store the match.</td></tr>
            <tr><td>oper(?=need)</td><td>Match "oper" only if it is followed by "need".</td></tr>
            <tr><td>oper(?!not)</td><td>Match "oper" only if it is not followed by "not".</td></tr>
            <tr><td>either|or</td><td>Match "either" or "or".</td></tr>
            <tr><td>{int}</td><td>Match exactly int occurences of the preceding item.</td></tr>
            <tr><td>{int,}</td><td>Match at least int occurences of the preceding item.</td></tr>
            <tr><td>{int1,int2}</td><td>Match at least int1 occurences of the preceding item but no more then int2.</td></tr>
            <tr><td>[pqr]</td><td>Match any one of the enclosed characters. Use a hyphen to specify a range of characters.</td></tr>
            <tr><td>[^pqr]</td><td>Match anything except the characters in brackets.</td></tr>
            <tr><td>[\b]</td><td>Match a backspace.</td></tr>
            <tr><td>\b</td><td>Match a word boundary.</td></tr>
            <tr><td>\B</td><td>Match a non-word boundary.</td></tr>
            <tr><td>\cQ</td><td>Match a control string, e.g. Control-Q</td></tr>
            <tr><td>\d</td><td>Match a digit.</td></tr>
            <tr><td>\D</td><td>Match any non-digit character.</td></tr>
            <tr><td>\f</td><td>Match a form feed.</td></tr>
            <tr><td>\n</td><td>Match a line feed.</td></tr>
            <tr><td>\r</td><td>Match a carriage return.</td></tr>
            <tr><td>\s</td><td>Match a single white space.</td></tr>
            <tr><td>\S</td><td>Match a non-white space.</td></tr>
            <tr><td>\t</td><td>Match a tab.</td></tr>
            <tr><td>\v</td><td>Match a vertical tab.</td></tr>
            <tr><td>\w</td><td>Match any alphanumeric character.</td></tr>
            <tr><td>\W</td><td>Match any non-word character.</td></tr>
            <tr><td>\int</td><td>A reference back int matches.</td></tr>
            <tr><td>\0</td><td>Match a null character.</td></tr>
            <tr><td>\xYY</td><td>Match the character code YY.</td></tr>
            <tr><td>\xYYYY</td><td>Match the character code YYYY.</td></tr>
        </table>
        @spec evolving
     */
    final class RegExp {

        use default namespace public

        /**
            Create a regular expression object that can be used to process strings.
            @param pattern The pattern to associated with this regular expression.
            @param flags "g" for global match, "i" to ignore case, "m" match over multiple lines, "y" for sticky match.
         */
        native function RegExp(pattern: String, flags: String? = null)

        /**
            The integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns. This is only set if the "g" flag was used.
            It is set to the match ending index plus one. Set to zero if no match.
         */
        native function get lastIndex(): Number

        /**
            Set the integer index of the end of the last match plus one. This is the index to start the next match for
            global patterns.
            @return Match end plus one. Set to zero if no match.
         */
        native function set lastIndex(value: Number): Void

        /**
            Match this regular expression against the supplied string. By default, the matching starts at the beginning 
            of the string.
            @param str String to match.
            @param start Optional starting index for matching.
            @return Array of results. The first element is the entire match. Subsequent
                elements correspond to the matching sub-expressions. Returns null if not match.
            @spec ejs Adds start argument.
         */
        native function exec(str: String, start: Number = 0): Array

        /**
            Global flag. If the global modifier was specified, the regular expression will search through the entire 
            input string looking for matches.
            @spec ejs
         */
        native function get global(): Boolean

        /**
            Ignore case flag. If the ignore case modifier was specified, the regular expression is case insensitive.
            @spec ejs
         */
        native function get ignoreCase(): Boolean

        /**
            Multiline flag. If the multiline modifier was specified, the regular expression will search through carriage 
            return and new line characters in the input.
         */
        native function get multiline(): Boolean

        /**
            Regular expression source pattern currently set.
         */
        native function get source(): String

        /**
            Substring last matched. Set to the matched string or null if there were no matches.
            @spec ejs
         */
        native function get matched(): String?

        /**
            Replace all the matches. This call replaces all matching substrings with the corresponding array element.
            If the array element is not a string, it is converted to a string before replacement.
            @param str String to match and replace.
            @param replacement Replacement text. This can contain $& to insert the matched substring,
                $` to insert the portion preceeding that matched substring, $' to insert the portion following the 
                matched substring, $n where n is the nth parentesized substring or $$ to insert a literal $ character.
            @return A string with zero, one or more substitutions in it.
            @spec ejs
         */
        function replace(str: String, replacement: Object): String
            str.replace(this, replacement)

        /**
            Split the target string into substrings around the matching sections.
            @param target String to split.
            @return Array containing the matching substrings
            @spec ejs
         */
        function split(target: String): Array
            target.split(this)

        /**
            Get the integer index of the start of the last match. This is only set of the "g" global flax is used.
            @return Match start index.
            @spec ejs
         */
        native function get start(): Number

        /**
            Sticky flag. If the sticky modifier was specified, the regular expression will only match from the $lastIndex.
            @spec ejs
         */
        native function get sticky(): Boolean

        /**
            Test whether this regular expression will match against a string.
            @param str String to search.
            @return True if there is a match, false otherwise.
            @spec ejs
         */
        native function test(str: String): Boolean

        /**
            Convert the regular expression to a string.
            This form will wrap the expression with slash delimiters and append the regular expression flags.
            For example: "/pattern/g"
            @returns a string representation of the regular expression.
         */
        override native function toString(): String
    }
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

/************************************************************************/
/*
    Start of file "src/core/Socket.es"
 */
/************************************************************************/

/*
    Socket.es -- Socket I/O class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Client and server side TCP/IP support for IPv4 and IPv6 communications. This class supports broadcast, datagram and
        byte-stream socket sevices for both client and server endpoints. Aynchronous and asynchronous operation is supported.
        @spec ejs
        @stability prototype
     */
    class Socket implements Stream {

        use default namespace public

        /** 
            Create a socket object
         */
        native function Socket()

        /** 
            Receive a client socket. Accept must be called after invoking $listen.
            @returns A socket connected to the client endpoint.
         */
        native function accept(): Socket

        /** 
            Local IP address bound to this socket. Set to the address in dot notation or empty string if it is not bound.
         */
        native function get address(): String 

        /** @duplicate Stream.async */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** @duplicate Stream.close */
        native function close(): Void

        //  MOB - what about ipv6
        /** 
            Establish a connection to a client from this socket to the supplied address. After a successful call to 
            connect() the socket may be used for sending and receiving.
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string, a composite "IP:PORT" string or a port number string. If only a port number is provided, 
                the socket will listen on all interfaces.
            @throws IOError if the connection fails. Reasons may include the socket is already bound or the host is unknown.
            @events Issues a "writable" event when the connection is complete.
         */
        native function connect(address: Object): Void

        /** 
            Current encoding scheme for serializing strings. Defaults to "utf-8"
         */
        function get encoding(): String
            "utf-8"

        function set encoding(enc: String): Void {
            throw "Not yet implemented"
        }

        /** @duplicate Stream.flush */
        function flush(dir: Number = Stream.BOTH): Void {}

        /**
            Is the socket at end of input. i.e. Is closed or has the other end sent a FIN packet.
         */
        native function get isEof(): Boolean

        /** 
            Listen on a socket for client connections. This will put the socket into a server role for communcations.
            If the socket is in sync mode, the listen call will block until a client connection is received after which
            accept() should be called to receive the socket instance for the new connection.
            If a the listening socket is in async mode, the listen call will return immediately and 
            client connections will be notified via "accept" events. 
            @param address The endpoint address on which to listen. The address can be either a port number, an IP address
                string or a composite "IP:PORT" string. If only a port number is provided, the socket will listen on
                all interfaces.
            @throws ArgError if the specified listen address is not valid, and IOError for network errors.
            @event Issues a "accept" event when there is a new connection available. In response, the $accept method
                should be called.
         */
        native function listen(address): Void

        /** @duplicate Stream.off */
        native function off(name: Object, observer: Function): Void

        /** 
            @duplicate Stream.on 
            @event readable Issued when the response headers have been fully received and some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
         */
        native function on(name, observer: Function): Socket

        /** 
            The port bound to this socket. Set to the integer port number or zero if not bound.
         */
        native function get port(): Number 

        /** @duplicate Stream.read */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        //  FUTURE - readString, readBytes, readXML

        /** 
            The remote address bound to this socket. Set to the remote address in dot notation or empty string if it 
            is not bound.
         */
        native function get remoteAddress(): String 

        /** 
            @duplicate Stream.write 
         */
        native function write(...data): Number
    }
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

/************************************************************************/
/*
    Start of file "src/core/Stream.es"
 */
/************************************************************************/

/*
    Stream.es -- Stream interface.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /** 
        Stream objects represent streams of data that pass data elements between an endpoint known as a source or sink 
        and a consumer or producer. Streams are full-duplex and may be stacked where intermediate streams are be used 
        as filters or data mutators. Example endpoints are the File, Socket, and Http classes. The TextStream is an 
        example of a filter stream. The data elements passed by streams may be any series of objects including: bytes, 
        lines of text, numbers or objects. Streams may buffer the incoming data or not. Streams may issue events to 
        registered observers for topics of interest. Streams may offer synchronous and asynchronous APIs. 
        @spec ejs
        @spec evolving
     */
    interface Stream {

        use default namespace public

        /** Read direction constant for $flush() */
        static const READ = 0x1

        /** Write direction constant for $flush() */
        static const WRITE = 0x2

        /** Both directions constant for $flush() */
        static const BOTH = 0x3

        /** 
            The current async mode. Set to true if the stream is in async mode.
         */
        function get async(): Boolean

        /** 
            Set the current sync/async mode. The async mode affects the blocking APIs: close, read and write.
            If in async mode, all Stream calls will not block. If observers have been registered, they can be used to
            respond to events to interface with the stream.
            @param enable If true, set the stream into async mode
         */
        function set async(enable: Boolean): Void

        /** 
            Close the stream. 
            @event close A close event is issued before closing the stream.
         */
        function close(): Void

        /**
            Flush the stream and underlying streams. A supplied flush $direction argument modifies the effect of this call.
            If direction is set to Stream.READ, then all read data is discarded. If direction is set to Stream.WRITE, 
            any buffered data is written. Stream.BOTH will cause both directions to be flushed. If the stream is in 
            sync mode, this call will block until all data is written. If the stream is in async mode, it will attempt 
            to write all data but will return immediately. Defaults to Stream.WRITE.
            @param dir direction to flush. Set to $READ, $WRITE or $BOTH.
         */
        function flush(dir: Number = BOTH): Void 

        /** 
            Remove an observer from the stream. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        function off(name, observer: Function): Void

        //  MOB - define what this is set to in the callback
        /** 
            Add an observer to the stream for the named events. 
            @param name :[String|Array] Name of the event to listen for. The name may be an array of events.
            @param observer Callback observer function. The function is called with the following signature:
                function observer(event: String, ...args): Void
            @event readable Issued when the stream becomes readable. 
            @event writable Issued when the stream becomes writable.
            @event close Issued when stream is being closed.
         */
        function on(name, observer: Function): Stream

        /** 
            Read a data from the stream. 
            If data is available, the call will return immediately. 
            If no data is available and the stream is in sync mode, the call will block until data is available.
            If no data is available and the stream is in async mode, the call will not block and will return immediately.
            In this case a "readable" event will be issued when data is available for reading.
            @param buffer Destination byte array for read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. If offset is >= 0, the data is read 
                to the offset and the read pointer is set to the offset and the write pointer to one past the end of 
                the data just read.
            @param count Read up to this number of bytes. If -1, read as much as the buffer will hold up. If the 
                stream is of fixed and known length (such as a file) and the buffer is of sufficient size or is growable, 
                read the entire stream. If the buffer is of a fixed size, ready only what will fit into the buffer.
            @returns a count of the bytes actually read. Returns null on EOF or errors.
            @event readable Issued when there is new read data available.
            @event writable Issued when the stream becomes empty.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? 

        /** 
            Write data to the stream.
            If the stream can accept all the write data, the call returns immediately with the number of bytes written. 
            If writing more data than the stream can absorb in sync mode, the call will block until the data is written.
            If writing more data than the stream can absorb in async mode, the call will not block and will buffer the
            data and return immediately. Some streams will require a $flush() call to actually send the data.
            A "writable" event will be issued when the stream can again absorb more data.
            @param data Data to write. 
            @returns a count of the bytes actually written.
            @throws IOError if there is an I/O error.
            @event readable Issued when data is written and a consumer can read without blocking.
            @event writable Issued when the stream becomes empty and it is ready to be written to.
         */
        function write(...data): Number
    }

    /*
        Workaround for Interface constants
     */
    Stream.READ = 0x1
    Stream.WRITE = 0x2
    Stream.BOTH = 0x3
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

/************************************************************************/
/*
    Start of file "src/core/String.es"
 */
/************************************************************************/

/*
    String.es -- String class
 *
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Each String object represents a single immutable linear sequence of characters. Strings have operators 
        for: comparison, concatenation, copying, searching, conversion, matching, replacement, and, subsetting.
        @spec evolving
     */
    final class String {

        use default namespace public

        /**
            String constructor. This can take two forms:
            <ul>
                <li>String()</li>
                <li>String(str: String)</li>
            </ul>
            @param str The args can be either empty or a string. If a non-string arg is supplied, the VM will automatically
                cast to a string.
         */
        native function String(...str)

        /**
            Do a case sensitive comparison between this string and another.
            @param compare The string to compare against
            @return -1 if this string is less than the compare string, zero if equal and 1 if greater than.
            @spec ejs
         */
        native function caseCompare(compare: String): Number

        /**
            Do a case insensitive comparison between this string and another.
            @param compare The string to compare against
            @return -1 if this string is less than the compare string, zero if equal and 1 if greater than.
            @spec ejs
         */
        native function caselessCompare(compare: String): Number

        /**
            Return the character at a given position in the string
            @returns a new string containing the selected character. If the index is out of range, returns the empty string.
         */
        native function charAt(index: Number): String

        /**
            Get a character code. 
            @param index The index of the character to retrieve
            @return Return the character code at the specified index. If the index is -1, get the last character.
                Return NaN if the index is out of range.
         */
        native function charCodeAt(index: Number = 0): Number

        /**
            Concatenate strings and returns a new string. 
            @param args Strings to append to this string
            @return Return a new string.
         */
        native function concat(...args): String

        /**
            Check if a string contains a pattern.
            @param pattern The pattern can be either a string or regular expression.
            @return Returns true if the pattern is found.
            @spec ejs
         */
        native function contains(pattern: Object): Boolean

        /**
            Determine if this string ends with a given string
            @param test The string to test with
            @return True if the string matches.
            @spec ejs
         */
        native function endsWith(test: String): Boolean

        /**
            Replace tokens in the string and return an expanded string.
            Tokens are represented by '${field}' where field may contain '.'. For example ${user.name}.
            To preserve an ${token} unmodified, preceed the token with an extra '$'. For example: $${token}.
            @param obj containing tokens to expand
            @param options Options hash
            @option fill Set to a string to use for missing properties. Set to undefined or omit options to 
                throw an exception for missing properties. Set fill to '${}' to preserve undefined tokens as-is. 
                This permits multi-pass expansions.
            @option join Character to use to join array elements. Defaults to space.
            @return Expanded string
         */ 
        native function expand(obj: Object, options: Object? = null): String

        /**
            Format arguments as a string. Use the string as a format specifier. The format specifier has the form:
            %[-+ #,][width][precision][type]. See printf(1) for the meaning of the various fields.
            @param args Array containing the data to format. 
            @return -1 if less than, zero if equal and 1 if greater than.
            @example
                "%5.3f".format(num)
            \n\n
                "%s Arg1 %d, arg2 %d".format("Hello World", 1, 2)
            @spec ejs
         */
        native function format(...args): String

        /**
            Create a string from the character code arguments
            @param codes Character codes from which to create the string
            @returns a new string
         */
        native static function fromCharCode(...codes): String

        /**
            Get an iterator for this array to be used by "for (v in string)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this array to be used by "for each (v in string)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Search for an item using strict equality "===". This call searches from the start of the string for 
            the specified element. 
            @param pattern The string to search for.
            @param startIndex Where in the array (zero based) to start searching for the object.
            @return The items index into the array if found, otherwise -1.
         */
        native function indexOf(pattern: String, startIndex: Number = 0): Number

        //  MOB - should be isNumber
        /**
            Is there is at least one character in the string and all characters are digits.
            @spec ejs
         */
        native function get isDigit(): Boolean

        /**
            Is there is at least one character in the string and all characters are alphabetic.
            Uses latin-1 for comparisons.
            @spec ejs
         */
        native function get isAlpha(): Boolean

        /**
            Is there is at least one character in the string and all characters are alphabetic or numeric.
            Uses latin-1 for comparisons.
            @spec ejs
         */
        native function get isAlphaNum(): Boolean

        /**
            Is there is at least one character in the string and there are no upper case characters.
            @return False otherwise
            @spec ejs
         */
        native function get isLower(): Boolean

        /**
            Is there is at least one character in the string and all characters are white space.
            @spec ejs
         */
        native function get isSpace(): Boolean

        /**
            Is there is at least one character in the string and there are no lower case characters.
            @spec ejs
         */
        native function get isUpper(): Boolean

        /**
            Search right to left for a substring starting at a given index.
            @param pattern The string to search for
            @param location The character index at which to start the search.
            @return Return the starting index of the last match found.
         */
        native function lastIndexOf(pattern: String, location: Number = -1): Number

//  TODO - change this to characters when supporting UNICODE.
        /**
            The length of the string in bytes.
         */
        native function get length(): Number

        /**
            Compare another string with the this string object
            @returns zero if the strings are equal, -1 if this string is lexically before @str and 1 if after.
         */
        # UNUSED
        function localeCompare(str: String): Number {
            if (this < string) {
                return -1
            } else if (string == this) {
                return 0
            } else {
                return 1
            }
        }

        //  TODO - should this allow a string?
        /**
            Match a pattern using a regular expression
            @param pattern The regular expression pattern to search for
            @return Returns an array of matching substrings.
         */
        native function match(pattern: RegExp): Array

        /**
            Parse the current string object as a JSON string object. The @filter is an optional filter with the 
            following signature:
                function filter(key: String, value: String): Boolean
            @param filter Optional function to call for each element of objects and arrays. Not yet supported.
            @returns an object representing the JSON string.
         */
        function parseJSON(filter: Function? = null): Object
            deserialize(this)

        /**
            Copy the string into a new string and lower case the first character if there is one. If the first non-white 
            character is not a character or if it is already lower there is no change.
            @return A new String
            @spec ejs
         */
        native function toCamel(): String

        /**
            Copy the string into a new string and capitalize the first character if there is one. If the first non-white 
            character is not a character or if it is already capitalized there is no change.
            @return A new String
            @spec ejs
         */
        native function toPascal(): String

        /**
            Create a new string with all nonprintable characters replaced with unicode hexadecimal equivalents (e.g. \uNNNN).
            @return The new string
            @spec ejs
         */
        native function printable(): String

        /**
            Wrap a string in double quotes.
            @return The new string
         */
        native function quote(): String

        /**
            Remove characters from a string. Remove the elements from @start to @end inclusive. 
            @param start Numeric index of the first element to remove. Negative indicies measure from the end of 
            the string.  -1 is the last character element.
            @param end Numeric index of one past the last element to remove
            @return A new string with the characters removed
            @spec ejs
         */
        native function remove(start: Number, end: Number = -1): String

        /**
            Search and replace. Search for the given pattern which can be either a string or a regular expression 
            and replace it with the replace text. If the pattern is a string, only the first occurrence is replaced.
            @param pattern The regular expression or string pattern to search for.
            @param replacement The string to replace the match with or a function to generate the replacement text. The
                replacement string can contain special replacement patterns: "$$" inserts a "\$", "\$&" inserts the
                matched substring, "\$`" inserts the portion that preceeds the matched substring, "\$'" inserts the
                portion that follows the matched substring, and "\$N" inserts the Nth parenthesized substring.
                The replacement parameter can also be a function which will be invoked and the function return value 
                will be used as the resplacement text. The function will be invoked multiple times for each match to be 
                replaced if the regular expression is global. The function will be invoked with the signature:

                function (matched, submatch_1, submatch_2, ..., matched_offset, original_string)
            @return Returns a new string.
            @spec ejs
         */
        native function replace(pattern: Object, replacement: Object = ''): String

        /**
            Reverse a string. 
            @return Returns a new string with the order of all characters reversed.
            @spec ejs
         */
        native function reverse(): String

        /**
            Search for a pattern.
            @param pattern Regular expression or string pattern to search for in the string.
            @return Return the starting index of the pattern in the string. Return -1 if not found.
         */
        native function search(pattern: Object): Number

        /**
            Extract a substring.
            @param start The position of the first character to slice.
            @param end The position one after the last character. Negative indicies are measured from the end of the 
                string.  The -1 index means the last character, so slice(0, -1) will return all characters except the 
                last.
            @param step Extract every "step" character.
         */ 
        native function slice(start: Number, end: Number = -1, step: Number = 1): String

        /**
            Split a string into an array of substrings. Tokenizes a string using a specified delimiter.
            @param delimiter String or regular expression object separating the tokens.
            @param limit At most limit strings are extracted. If limit is set to -1, then unlimited strings are extracted.
            @return Returns an array of strings.
         */
        native function split(delimiter: Object, limit: Number = -1): Array

        /**
            Tests if this string starts with the string specified in the argument.
            @param test String to compare against
            @return True if it does, false if it doesn't
            @spec ejs
         */
        native function startsWith(test: String): Boolean

        /**
            Extract a substring. Similar to slice but only allows positive indicies.
            If the end index is larger than start, then the effect of substring is as if the two arguments were swapped.
            @param startIndex Integer location to start copying
            @param end Postitive index of one past the last character to extract.
            @return Returns a new string
         */
        native function substring(startIndex: Number, end: Number = -1): String

        /**
            Replication. Replicate the string N times.
            @param times The number of times to copy the string
            @return A new String with the copies concatenated together. Returns empty string if times is <= zero.
            @spec ejs
         */
        function times(times: Number): String {
            var s: String = ""
            for (i in times) {
                s += this
            }
            return s
        }

        //  TODO. Should this be the reverse?   for (s in "%s %s %s".tokenize(value))
        //  that would be more consistent with format()
        /**
            Scan the input and tokenize according to a string format specifier.
            @param format Tokenizing format specifier
            @returns array containing the tokenized elements
            @example
                for (s in string.tokenize("%s %s %s")) {
                    print(s)
                }
            @spec ejs
         */
        native function tokenize(format: String): Array

        /**
            Convert the string to an equivalent JSON encoding.
            @return A quoted string.
            @throws TypeError If the object could not be converted to a string.
         */ 
        override native function toJSON(): String

        /**
            Convert the string to lower case.
            @return Returns a new lower case version of the string.
            @spec ejs
         */
        native function toLowerCase(): String


        /**
            This function converts the string to a localized lower case string representation. 
            @returns a lower case localized string representation of the string. 
         */ 
        # FUTURE
        function toLocaleLower(): String
            //  TODO should be getting from App.Locale not from date
            toLowerCase(Date.LOCAL)

        /**
            This function converts the string to a localized string representation. 
            @returns a localized string representation of the string. 
         */ 
        # FUTURE
        override function toLocaleString(): String
            toString()

        /**
            This function converts an object to a string representation. Types typically override this to provide 
            the best string representation.
            @returns a string representation of the object. For Objects "[object className]" will be returned, 
            where className is set to the name of the class on which the object was based.
         */ 
        override native function toString(): String

        /**
            Convert the string to a path
            @return Returns an equivalent Path object
            @spec ejs
         */
        function toPath(): Path
            this cast Path

        /**
            Convert the string to upper case.
            @return Returns a new upper case version of the string.
            @spec ejs
         */
        native function toUpperCase(): String

        /**
            Convert the string to localized upper case string
            @return Returns a new localized upper case version of the string.
         */
        # FUTURE
        function toLocaleUpperCase(): String
            //  TODO should be getting from App.Locale not from date
            toUpperCase(Date.LOCAL)


        //  TODO - great if this could take a regexp
        /**
            Returns a trimmed copy of the string. Normally used to trim white space, but can be used to trim any 
            substring from the start and end of the string.
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trim(str: String? = null): String

        /**
            Trim text from the start of a string. 
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trimStart(str: String? = null): String

        /**
            Trim text from the end of a string. 
            @param str May be set to a substring to trim from the string. If not set, it defaults to any white space.
            @return Returns a (possibly) modified copy of the string
         */
        native function trimEnd(str: String? = null): String

        /**
            Return the value of the object
            @returns this object.
         */ 
        # FUTURE
        override function valueOf(): String {
            return this
        }

        /*
            Overloaded operators
         */

        /**
            String subtraction. Remove the first occurance of str.
            @param str The string to remove from this string
            @return Return a new string.
            @spec ejs
         */
        function - (str: String): String {
            var i: Number = indexOf(str)
            if (i < 0) {
                return this
            }
            return remove(i, i + str.length)
        }
        
        /**
            Compare strings. 
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function < (str: String): Number {
            return localeCompare(str) < 0
        }

        /**
            Compare strings.
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function > (str: String): Number {
            return localeCompare(str) > 0
        }

        /**
            Compare strings.
            @param str The string to compare to this string
            @return -1 if less than, zero if equal and 1 if greater than.
            @spec ejs
         */
        # DOC_ONLY
        function == (str: String): Number {
            return localeCompare(str) == 0
        }

        /**
            Format arguments as a string. Use the string as a format specifier.
            @param arg The argument to format. Pass an array if multiple arguments are required.
            @return -1 if less than, zero if equal and 1 if greater than.
            @example
                "%5.3f" % num
            <br/>
                "Arg1 %d, arg2 %d" % [1, 2]
            @spec ejs
         */
        function % (arg: Object): String
            format(arg)

//  MOB - Cleanup
        /** 
            @hide 
            @deprecated 2.0.0
         */
        # Config.Legacy || 1
        function toLower(): String
            toLowerCase()

        /** 
            @hide 
            @deprecated 2.0.0
         */
        # Config.Legacy || 1
        function toUpper(): String
            toUpperCase()
    }
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

/************************************************************************/
/*
    Start of file "src/core/System.es"
 */
/************************************************************************/

/*
    System.es - System class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        FUTURE
            endian
     */
    /**
        System is a utility class providing methods to interact with the operating system.
        @spec ejs
        @stability prototype
     */
    enumerable class System {

        use default namespace public

        /**
            Default buffer size 
         */
        public static const Bufsize: Number = 1024

        /**
            The fully qualified system hostname
         */
        native static function get hostname(): String

        /**
            The system IP Address
            @hide
         */
        native static function get ipaddr(): String

        /** @hide */
        native static function get tmpdir(): String
    }
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

/************************************************************************/
/*
    Start of file "src/core/TextStream.es"
 */
/************************************************************************/

/*
    TextStream.es -- TextStream class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        TextStreams interpret data as a stream of characters. They provide methods to read and write data
        in various text encodings and to read/write lines of text appending appropriate system dependent new line 
        terminators. TextStreams can be stacked upon other Streams such as files, byte arrays, sockets, or Http objects.
        @spec ejs
        @stability evolving
     */
    class TextStream implements Stream {

        use default namespace public

        /** 
            System dependent newline terminator
         */
        private var newline: String

        /* 
            Data input buffer
         */
        private var inbuf: ByteArray

        /** @hide */
        private var format: String = "utf-8"

        /* 
            Provider Stream
         */
        private var nextStream: Stream

        /** 
            Create a text filter stream. A Text filter stream must be stacked upon a stream source such as a File.
            @param stream stream data source/sink to stack upon.
         */
        function TextStream(stream: Stream?) {
            if (stream == null) {
                throw new ArgError("Must supply a Stream argument")
            }
            inbuf = new ByteArray
            nextStream = stream
            newline = FileSystem("/").newline
        }

        /** 
            @duplicate Stream.async 
         */
        function get async(): Boolean
            false

        /** 
            @duplicate Stream.async 
         */
        function set async(enable: Boolean): Void {
            if (enable) {
                throw new ArgError("Async mode not supported")
            }
        }

        /** 
            @duplicate Stream.close
         */
        function close(): Void {
            inbuf.flush(2 /* MOB Stream.WRITE */)
            nextStream.close()
        }

        /** 
            The current text encoding.
            Not implemented.
            @hide
         */
        function get encoding(): String
            format

        /** 
            Set the current text encoding. Currently not supported or used.
            Not implemented.
            @param encoding string containing the current text encoding. Supported encodings are: utf-8.
            @hide
         */
        function set encoding(encoding: String): Void {
            format = encoding
        }

        /**  
            Fill the input buffer from upstream
            @returns The number of new characters added to the input bufer
         */
        function fill(): Number? {
            inbuf.compact()
            return nextStream.read(inbuf, -1)
        }

        /** 
            @duplicate Stream.flush
         */
        function flush(dir: Number = Stream.BOTH): Void {
            if (dir & Stream.WRITE)
                inbuf.flush()
            if (!(nextStream is ByteArray)) {
                nextStream.flush(dir)
            }
        }

        /** 
            The number of bytes available to read without blocking. This is the number of bytes buffered internally
            by this stream. It does not include any data buffered downstream.
            @return the number of available bytes
         */
        function get length(): Number
            inbuf.length

        /** 
            @duplicate Stream.off
         */
        function off(name, observer: Function): Void {
            throw new ArgError("Observers are not supported")
        }

        /** 
            @duplicate Stream.on 
         */
        function on(name, observer: Function): TextStream {
            throw new ArgError("Observers are not supported")
            return this
        }

        /** 
            Read characters from the stream into the supplied byte array. 
            @param buffer Destination byte array for the read data.
            @param offset Offset in the byte array to place the data. If the offset is -1, then data is
                appended to the buffer write $position which is then updated. 
            @param count Number of characters to read. 
            @returns a count of characters actually read
            @throws IOError if an I/O error occurs.
         */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number? {
            let total = 0
            if (count < 0) {
                count = Number.MaxValue
            }
            if (offset < 0) {
                buffer.reset()
            } else {
                buffer.flush(1 /* MOB Stream.READ */)
            }
            let where = buffer.writePosition
            while (count > 0) {
                if (inbuf.length == 0) {
                    if (fill() <= 0) {
                        if (total == 0) {
                            return null
                        }
                        break
                    }
                }
                let len = count.min(inbuf.length)
                len = len.min(buffer.size - where)
                if (len == 0) break
                len = buffer.copyIn(where, inbuf, inbuf.readPosition, len)
                inbuf.readPosition += len
                where += len
                total += len
                count -= len
            }
            buffer.writePosition += total
            return total
        }

        /** 
            Read a line from the stream.
            @returns A string containing the next line without newline characters ("\r", "\n"). Return null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readLine(): String? {
            if (inbuf.length == 0 && fill() <= 0) {
                return null
            }
            //  All systems strip both \n and \r\n to normalize text lines
            //  MOB -- this should be a configurable option on a TextStream
            let nl = "\r\n"
            let nlchar = nl.charCodeAt(nl.length - 1)
            let nlchar0 = nl.charCodeAt(0)
            while (true) {
                for (let i = inbuf.readPosition; i < inbuf.writePosition; i++) {
                    //  MOB OPT. If ByteArray had indexOf(nl), then this could be MUCH faster
                    if (inbuf[i] == nlchar) {
                        if (nl.length == 2 && i > inbuf.readPosition && nlchar0 == inbuf[i-1]) {
                            result = inbuf.readString(i - inbuf.readPosition - 1)
                            inbuf.readPosition += 2
                        } else {
                            result = inbuf.readString(i - inbuf.readPosition)
                            inbuf.readPosition++
                        }
                        return result
                    }
                }
                if (fill() <= 0) {
                    /* Missing a line terminator, so return any last portion of text */
                    if (inbuf.length) {
                        return inbuf.readString(inbuf.length)
                    }
                }
            }
            return null
        }

        /** 
            Read a required number of lines of data from the stream.
            @param numLines of lines to read. Defaults to read all lines.
            @returns Array containing the read lines. Lines are stripped of newline characters ("\r", "\n"). 
            Return null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readLines(numLines: Number = -1): Array? {
            var result: Array
            if (numLines <= 0) {
                result = new Array
                numLines = Number.MaxValue
            } else {
                result = new Array(numLines)
            }
            for (let i in numLines) {
                if ((line = readLine()) == null) {
                    if (i == 0) {
                        return null
                    }
                    break
                }
                result[i] = line
            }
            return result
        }

        /** 
            Read a string from the stream. 
            @param count of bytes to read. Returns the entire stream contents if count is -1.
            @returns a string or null on EOF.
            @throws IOError if an I/O error occurs.
         */
        function readString(count: Number = -1): String?
            inbuf.readString(count)

        /** 
            Write characters to the stream.
            @param data String to write. 
            @returns The total number of bytes that were written.
         */
        function write(...data): Number
            nextStream.write(data)

        /** 
            Write text lines to the stream. The text line is written after appending the system text newline character(s).
            @param lines Text lines to write.
            @return The number of characters written or -1 if unsuccessful.
         */
        function writeLine(...lines): Number {
            let written = 0
            if (lines.length == 0) {
                nextStream.write("\n");
                written++
            } else {
                for each (let line in lines) {
                    nextStream.write(line)
                    /* If the file is opened in text mode, the lower layers will add "\r" for windows */
                    nextStream.write("\n");
                    written += line.length + 1
                }
            }
            return written
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Timer.es"
 */
/************************************************************************/

/*
    Timer.es -- Timer Services

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    //  MOB - need a simple way to construct a timer and start in one step
    /**
        Timers manage the execution of functions at some point in the future. Timers may run once, or they can be 
        scheduled to run repeatedly, until stopped by calling the $stop() method. Timers are scheduled with a granularity 
        of 1 millisecond. However, many systems are not capable of supporting this granularity and make only best efforts 
        to schedule events at the desired time.
        @Example
            timer = Timer(200, function (args) { })
            timer.repeat = true
            timer.start()
        @stability prototype
     */
    class Timer {

        use default namespace public

        /**
            Constructor for Timer. The timer is will not be called until $start is called.
            @param period Delay in milliseconds before the timer will run
            @param callback Function to invoke when the timer is due. The callback is invoked with the following signature:
                function callback(error: Error): Void
            @param args Callback arguments
         */
        native function Timer(period: Number, callback: Function, ...args)

        /**
            The current drift setting. If drift is true, the timer is allowed to drift its execution time due to 
            other system events when attempting to optimize overall system performance.
            If the drift value is set to false, the timer is scheduled so that the period between callback start 
            times does not gradually drift. When using non-drifting timers, the timer subsystem will 
            delay other low priority events or timers, if necessary to ensure non-drifting timers are scheduled exactly. 
         */
        native function get drift(): Boolean
        native function set drift(enable: Boolean): Void

        /**
            Timer delay period in milliseconds. Changing a timer period will not reschedule a currently scheduled timer.
            The period will be used to schedule the next invocation. To change the current period, stop the timer and
            restart it.
         */
        native function get period(): Number
        native function set period(period: Number): Void

        /**
            Error callback function for exceptions inside the Timer callback
            The callback is invoked with the following signature:
                function callback(error: Error): Void
         */
        native function get onerror(): Function
        native function set onerror(callback: Function): Void

        /**
            Timer repeatability. If true, the timer will be repeated invoked every $period milliseconds
         */
        native function get repeat(): Boolean
        native function set repeat(enable: Boolean): Void

        /**
            Reschedule a timer. This will stop the timer if it is currently scheduled, then reschedule the timer.
            If the $when argument is provided, the timer period will be set before starting.
            @param when Time period for when to next run the timer
         */
        function restart(when: Number? = null): Void {
            stop()
            if (when) {
                period = when
            }
            start()
        }

        /**
            Start a timer running. The timer will be repeatedly invoked if the $repeat property is true, otherwise it 
            will be invoked once.
            When the timer callback is invoked, it will be invoked with the value of "this" set to the timer unless the
                function has bound a "this" value via Function.bind.
            @return The timer instance. This helps chaining. For example: var timer = Timer(1000, sayHello).start()
         */
        native function start(): Timer

        /**
            Stop a timer running. Once stopped a timer can be restarted by calling $start.
         */
        native function stop(): Void
    }


    /**
        Create an interval timer. This will invoke the callback every $delay milliseconds.
        @param callback Function to invoke when the timer is due.
        @param delay Time period in milliseconds between invocations of the callback
        @param args Function arguments to provide to the callback
        @return Timer ID that can be used with $clearInterval
     */
    function setInterval(callback: Function, delay: Number, ...args): Timer {
        let timer = new Timer(delay, callback, ...args)
        timer.repeat = true
        timer.start()
        return timer
    }

    /**
        Clear and dispose of an interval timer
        @param timer Interval timer returned from $setInterval
     */
    function clearInterval(timer: Timer): Void
        timer.stop()

    /**
        Run a function after a delay
        @param callback Function to invoke when the timer expires
        @param delay Time in milliseconds until the timer expires and the callback is invoked
        @param args Function arguments
        @return Timer that can be used with $clearTimeout
      */
    function setTimeout(callback: Function, delay: Number, ...args): Number {
        let timer = new Timer(delay, callback, ...args)
        timer.start()
        return timer
    }

    /**
        Clear and dispose of a timeout
        @param timer Timeout timer returned from $setTimeout
     */
    function clearTimeout(timer: Timer): Void 
        timer.stop()
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

/************************************************************************/
/*
    Start of file "src/core/Type.es"
 */
/************************************************************************/

/*
    Type.es -- Type class. Base class for all type objects.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /*
        MOB - REVISE
        The Type class is unlike other class definitions. Rather than provide definitions that are accessed via
        the base-class and prototype chain, these properties are cloned directly into all types.
     */

    /**
        Base class for all type objects.
        @spec ejs
        @stability evolving
     */
    final class Type { }
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

/************************************************************************/
/*
    Start of file "src/core/Uri.es"
 */
/************************************************************************/

/*
    Uri.es -- Uri parsing and management class
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {
    /**
        The URI class to provides the ability to create, and manipulate URIs and their constituent components.
        Uri objects are mutable.
        @spec ejs
        @stability evolving
     */
    class Uri {
        use default namespace public

        /** 
            Create and parse a Uri object. 
            @param uri A string or object hash that describes the URI. The $uri specify a complete absolute URI string or
            it may specify a partial URI where missing elements take the normal defaults. The $uri argument may also be 
            an object hash with the following properties.
            @option scheme String URI protocol scheme (http or https)
            @option host String URI host name or IP address.
            @option port Number TCP/IP port number for communications
            @option path String URI path 
            @option query String URI query parameters. Does not include "?"
            @option reference String URI path reference. Does not include "#"
         */
        native function Uri(uri: Object)

        /** 
            Create a complete absolute URI from "this" URI with all mandatory components present including 
            scheme and host.  The resulting URI path will be normalized and any missing components will be 
            completed with values from the given $base URI. If "this" URI path is relative, it will be joined to base 
            URI's path.
            Any query component of "this" URI is discarded in the result. This is because the query component of "this" URI
            is regarded as POST data and not integral to the base URI.
            @param base Optional URI to provide missing components and a base URI path if the current URI path is relative. 
            The base argument can be a string, URI or an object hash of URI component. If the base argument is not supplied,
            the current URI will be completed as much as possible. See $complete.
            @return A complete, absolute URI.
          */
        native function absolute(base = null): Uri

        /**
            Return the TCP/IP address of the URI. This is the IP:PORT portion.
         */
        function get address(): String
            (host || '127.0.0.1') + (port ? (':' + port) : '')

        /** 
            The base of portion of the URI. The base portion is the trailing portion of the path without any directory 
            elements. Set to null if the Uri has no path.
         */
        native function get basename(): Uri
        
        /** 
            Create a complete absolute URI from "this" URI with all mandatory components present including scheme and host. 
            The resulting URI path will be normalized and missing (mandatory) components will be completed with values 
            from the given "missing" argument.
            @param missing Optional URI to provide the missing components. The missing argument can be a string, URI or
                an object hash of URI component. If this missing argument is not provided, the following defaults are used:
                "http://localhost:80/".
            @return A complete, absolute URI.
         */
        native function complete(missing = null): Uri

        /** 
            Break a URI into its components by converting the URI to an absolute URI and breaking into components.
            The components are: scheme, host, port, path, reference and query.
            @return an object hash defining the following fields:
            @option scheme String URI protocol scheme (http or https)
            @option host String URI host name or IP address.
            @option port Number TCP/IP port number for communications
            @option path String URI path 
            @option query String URI query parameters. Does not include "?"
            @option reference String URI path reference. Does not include "#"
         */
        native function get components(): Object 

        /** 
            Decode a URI encoded string using www-url encoding. See encode for details about the character mappings.
            @param str string to decode
            @returns a decoded string
         */
        native static function decode(str: String): String

        /** 
            Decode a URI encoded component using www-url encoding. 
            See encodeComponent for details about the character mappings.
            @param str string to decode
            @returns a decoded string
         */
        native static function decodeComponent(str: String): String

        /** 
            The directory portion of a URI path. The directory portion is the leading portion including all directory 
            elements of the URI path excluding the base name. Set to null if the Uri has no path.
         */
        native function get dirname(): Uri

        /*
            URI encoding notes: (RFC3986). See http://labs.apache.org/webarch/uri/rfc/rfc3986.html.
            Reserved characters (and should be encoded):   : / ? # [ ] @    ! $ & ' ( ) * + , ; =
            Unreserved characters (and must not be encoded): Alpha Digits - . _ ~

            NOTE: ! , ( ) * do not yet have a formalized URI delimiting role.

            encodeComponent preserves:  ! * ' ( )
            encode preserves:           ! * ' ( ) # ; , / ? : @ 

            NOTE: encodeComponent is encoding [] which is hard for IPv6
            MOB:
                Don't encode [] for IPv6
                Encode ! ' ( ) *
         */

        /** 
            Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
            The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", 
            "*", "'", "(", ")", "#", ";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocde does not encode
            "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURI routine does not. It does not
            encode "[" and "]" which may be used in IPv6 numeric hostnames.
            @param str string to encode
            @returns an encoded string
         */
        native static function encode(str: String): String

        /** 
            Encode a URI component suitable for the "application/x-www-form-urlencoded" mime type. This replaces 
            special characters with encoded alternative sequence. The encode call replaces all characters 
            except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", "'", "(", ")". It also maps space to "+".
            Compared with the $encode call, encodeComponent additionally encodes: "#", ";", ",", "/", "?", ":", "@", 
            "&", "=", "+", "$".  Note that this call encodes "=" and "&" which are used to separate and delimit 
            data form name/value pairs.
            NOTE: This routine encodes "!", "'", "(", ")" and "*", whereas the $encodeURIComponent routine does not.
            @note See http://labs.apache.org/webarch/uri/rfc/rfc3986.html for details.
            @param str string to encode
            @returns an encoded string
         */
        native static function encodeComponent(str: String): String

        /** 
            Return true if the URI path ends with the given suffix
            @param suffix URI or String suffix to compare with the path.
            @return true if the path does begin with the suffix
         */
        function endsWith(suffix: Object): Boolean
            path.toString().endsWith(suffix.toString())

        /** 
            The URI extension portion of the path. Set to a String containing the URI extension without the ".".
            Set to null if no extension. If this is modified, the path property will be updated accordingly.
         */
        native function get extension(): String?
        native function set extension(value: String): Void

        /**
            Set to a Path type corresponding to the Uri path property. The Path is relative to the current directory.
            i.e. this removes the leading "/" from the path property.
         */
        function get filename(): Path
            Path(path.slice(1))

        /** 
            Does the URI has an explicit extension
         */
        native function get hasExtension(): Boolean 

        /** 
            Does the URI have an explicit host component. For example: "www.example.com"
         */
        native function get hasHost(): Boolean 

        /** 
            Does the URI have an explicit port number.
         */
        native function get hasPort(): Boolean 

        /** 
            Does the URI have an explicit query component
         */
        native function get hasQuery(): Boolean 

        /** 
            Does the URI have an explicit reference component
         */
        native function get hasReference(): Boolean 

        /** 
            Does the URI have an explicit scheme (protocol) specifier. For example: "http://"
         */
        native function get hasScheme(): Boolean 

        /** 
            The host portion of the URI. Set to null if there is no host component.
         */
        native function get host(): String?
        native function set host(value: String?): Void

        /** 
            Is the URI is an absolute path. Set to true if the URI path component beginning with a "/".
         */
        native function get isAbsolute(): Boolean

        /** 
            Is the URI is a directory URI. Set to true if the URI ends with "/". NOTE: this only tests the URI and 
            not any physical resource associated with the URI.
         */
        native function get isDir(): Boolean

        /** 
            Is the URI is a regular resource and not a directory. Set to true if the URI does not end with "/". 
            NOTE: this only tests the URI and not any physical resource associated with the URI.
         */
        function get isRegular(): Boolean
            isDir == false

        /** 
            Is if the URI path is relative. Set to true if the URI's path component does not begin with a "/".
         */
        function get isRelative(): Boolean
            isAbsolute == false

        /** 
            Join URIs. URI argument are joined in turn starting with "this" URI" as the base. If a URI argument is absolute,
            the progressive result is replaced with the absolute URI and joining continues. If a URI argument is relative, 
            a "/" is appended followed by the argument, and joining continues. For example:  
                Uri("/admin").join("/display") will result in "/display"
                Uri("/admin").join("logout") will result in "/admin/logout"
                Uri("/admin").join("private", "profile") will result in "/admin/private/profile"
            @param others Other URIs to join. These can be URIs, strings or object hashes of URI components.
            @return A new URI with the arguments joined to the current URI.
         */
        native function join(...others): Uri

        /** 
            Join an extension to a URI. If the basename of the URI already has an extension, this call does nothing.
            @return A URI with an extension.
         */
        native function joinExt(ext: String): Uri

        /** 
            Create a local URI. A local URI has no scheme, host or port components.
            @return A complete, local URI.
          */
        native function get local(): Uri

        /** 
            The mime type of the URI. This is set to a mime type string by examining the URI extension. 
            Set to "text/html" if the mime type cannot be determined from the extension. 
            Set to null if the URI has no extension.
         */
        native function get mimeType(): String?

        /** 
            Normalized URI by removing all redundant and invalid URI components. Set to a URI with "segment/.." 
            and "./" components removed. The value will not be converted to an absolute URI nor will it map character case.
         */
        native function get normalize(): Uri

        /** 
            The URI path portion after the hostname
            Set to null if there is no path.
         */
        native function get path(): String?
        native function set path(value: String?): Void

        /** 
            The port number of the URI. Set to null if the URI does not have an explicit port.
         */
        native function get port(): Number?
        native function set port(value: Number?): Void

        /** 
            The URI query string. The query string is the fragment after a "?" character in the URI.
            Set to null if there is no query.
         */
        native function get query(): String?
        native function set query(value: String?): Void

        /** 
            The URI reference portion. The reference portion is sometimes called the "anchor" and is the the fragment 
            after a "#" character in the URI.  Set to null if there is no reference.
         */
        native function get reference(): String?
        native function set reference(value: String?): Void

        /** 
            Create a URI with a relative path from the $base URI to "this" URI. 
            If the base URI has a different scheme, host or port to that of "this" URI, then a relative URI cannot be 
            formed and the current URI is returned.
            Any query component of the $base URI is ignored in the result. This is because the query component 
            is regarded as POST data and not integral to the base URI.
            @param base Base URI to use in calculating the relative path to "this" URI. The base argument can be a string, 
            URI or an object hash of URI component.
            @return a new URI object with a relative path from the $base URI to "this" URI.
         */
        native function relative(base): Uri

        /** 
            Replace the extension and return a new URI.
            @return A path with extension.
         */
        native function replaceExt(ext: String): Uri

        /**
            Resolve a URI in the neighborhood of this URI. Resolve operates like join, except that it joins the 
            given URI to the directory portion of the current ("this") URI. For example: 

                Uri("/a/b.html").resolve("c.html") will return "/a/c.html".

            Resolve operates by determining a virtual current directory for this URI (dirname). It then joins the 
            given URI path to the directory portion of the current result. If the resolving URI is an absolute URI, it is 
            used unmodified. 

            Resolve is useful for creating URIs in the region of the current URI and gracefully handles both absolute 
            and relative URI segments.
            Any query component of "this" URI is discarded in the result. This is because the query component of "this" URI
            is regarded as POST data and not integral to the base URI.
            @param target Other URI to resolve in the region of this path. Target can be a URI, string or object hash
                of URI components.
            @return A new URI object that resolves given URI args using the "this" URI as a base. 
         */
        native function resolve(target): Uri

        /** 
            Compare two URIs test if they represent the same resource
            @param other Other URI to compare with
            @param exact If exact is true, then the query and reference portions must match
            @return True if the URIs represent the same underlying resource
         */
        native function same(other: Object, exact: Boolean = false): Boolean

        /** 
            The URI protocol scheme. Set to null if no scheme defined.
         */
        native function get scheme(): String?
        native function set scheme(value: String?): Void

        /** 
            Return true if the URI path starts with the given prefix. This skips the scheme, host and port portions
            and examines the URI path only.
            @param prefix URI or String prefix to compare with the URI.
            @return true if the path does begin with the prefix
         */
        function startsWith(prefix: Object): Boolean
            path.toString().startsWith(prefix.toString()) 

        /**
            Create a URI based on a template. The template is a subset of the URI-templates specification and supports
            simple {tokens} only. Each token is looked for in the set of provided option objects. The search stops with
            the first object providing a value.
            @param pattern URI-Template with {word} tokens.
            @param options Set of option objects with token properties to complete the URI.
            @return A URI
         */
        native static function template(pattern: String, ...options): Uri

        /** 
            Convert the URI to a JSON string. 
            @return a JSON string representing the URI.
         */
        override function toJSON(): String
            JSON.stringify(this.toString())

        /** 
            Convert the URI to a string.
            @return a string representing the URI.
         */
        native override function toString(): String

        /** 
            Convert the local portion of the URI to a string. This will include only the path, query and reference
            components of the URI. The scheme, host and port portions of the URI will be ignored.
            @return a string representing the URI's path, query and reference portions.
         */
        native override function toLocalString(): String

        /** 
            Trim a pattern from the end of the URI path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a new URI containing the trimmed URI
            TODO - should support reg expressions
         */
        function trimEnd(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimEnd(pat)
            return u
        }

        /** 
            Trim the extension portion off the URI path
            @return a URI with no extension
         */
        native function trimExt(): Uri

        /** 
            Trim a pattern from the start of the path
            NOTE: this does a case-sensitive match. MOB - is this right?
            @return a URI containing the trimmed path name
            TODO - should support reg expressions
         */
        function trimStart(pat: String): Uri {
            let u = this.clone()
            u.path = u.path.toString().trimStart(pat)
            return u
        }

        /** 
            The full URI as a string
         */
        native function get uri(): String
        native function set uri(value: String): Void
    }

    /** 
        Decode an encoded URI using www-url encoding
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURI(str: String): String

    /** 
        Decode an encoded URI component.
        @param str encoded string
        @returns a decoded string
     */
    native function decodeURIComponent(str: String): String

    /** 
        Encode a URI using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")", "#",";", ",", "/", "?", ":", "@", "&", "=", "+", "$". Note that encocdeURI does not encode
        "&", "+" and "=". If you require these to be encoded, use encodeComponents. 
        @see Uri.encode for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURI(str: String): String

    /** 
        Encode a URI component using www-url encoding. This replaces special characters with encoded alternative sequence.
        The encode call replaces all characters except: alphabetic, decimal digits, "-", "_", ".", "!", "~", "*", 
        "'", "(", ")". Note that this call encodes "=" and "&" which are often used in URL query name/key pairs.
        @see Uri.encodeComponent for RFC3986 compliant encoding.
        @param str String to encode
        @returns an encoded string
     */
    native function encodeURIComponent(str: String): String

    /** 
        Encode objects using using www-url encoding. Each object is encoded as a "key=value" pair. Each pair is separated
        by a "&" character. 
        @param items Strings to encode
        @returns an encoded string
     */
    function encodeObjects(...items) {
        let result = ""
        for each (item in items) {
            for (let [key, value] in item) {
                if (result) result += "&"
                result += encodeURIComponent(key) + "=" + encodeURICompnent(value)
            }
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/Void.es"
 */
/************************************************************************/

/*
    Void.es -- Void class used for undefined value.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        The Void type is the base class for the undefined value. One instance of this type is created for the 
        system's undefined value.
        @spec ejs
        @spec evolving
     */
    final class Void {

        /* WARNING: Do not add properties here. Null must have no properties beyond those inherited by Object */

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function get(): Iterator

        /**
            Implementation artifacts
            @hide
         */
        override iterator native function getValues(): Iterator
    }
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

/************************************************************************/
/*
    Start of file "src/core/WebSocket.es"
 */
/************************************************************************/

/**
    WebSocket.es -- WebSockets class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        WebSocket class to implement the WebSockets RFC 6455 specification for client-side communications. 

        WebSockets is a technology providing interactive communication between a server and client. Normal HTML connections
        follow a request / response paradigm and do not easily support asynchronous communications or unsolicited data
        pushed from the server to the client. WebSockets solves this by supporting bi-directional, full-duplex
        communications over persistent connections. A WebSocket connection is established over a standard HTTP connection
        and is then upgraded without impacting the original connection. This means it will work with existing networking
        infrastructure including firewalls and proxies.
        @spec ejs
        @stability prototype
        @see Http ByteArray
     */
    class WebSocket implements Stream {

        use default namespace public

        /** The $readyState value while waiting for the WebSockets open handshake to complete with the peer */
        static const CONNECTING = 0              

        /** The $readyState value once the WebSockets open handshake is complete and ready for communications */
        static const OPEN = 1

        /** The $readyState value when a closing handshake has commenced */
        static const CLOSING = 2

        /** The $readyState value when the WebSockets connection is closed */
        static const CLOSED = 3

        /**
            Create a new WebSocket and connect to the server using the supplied URI.
            @param uri URL to connect to
            @param protocols Optional set of protocols supported by the application. These are application-level
                protocols. This argument may be an array or a string of comma separated values.
            @param options
            @options certificate Path|String Optional path to file containing certificates to validate the 
                server certificate.
            @options verify Boolean Set to true to verify the server certificate. 
                The certificate is verified and the issuer chain is verified. Defaults to true.
                Set to false to accept a self-signed certificate.
            @options frames Preserve individual message frames to onmessage.
         */
        native function WebSocket(uri: Uri, protocols = null, options = null)

        /**
            Event callback invoked when the WebSocket is ready to send and receive data. The $readyState will be set to OPEN.
          */
        var onopen: Function

        /**
            Event callback invoked when a message has been received from the server. The $readyState will be OPEN.
          */
        var onmessage: Function

        /**
            Event callback invoked when an error occurs.
          */
        var onerror: Function

        /**
            Event callback invoked when the connection is closed. The $readyState will be CLOSED.
          */
        var onclose: Function

        /**
            Type of binary data. Set to "ByteArray".
          */
        native function get binaryType(): String

        /**
            The number of bytes of buffered data to send
          */
        native function get bufferedAmount(): Number

        /**
            Send a close message and close the web socket connection
            @param code WebSocket status code to send to the peer explaining why the connection is being closed.
                Defaults to 1000 which means a successful, graceful closure.
            @param reason Optional reason string to indicate why the connection was closed. Must be less than 
                124 bytes of UTF-8 text in length.
         */
        native function close(code: Number = 1000, reason: String? = ""): Void

        /**
            Return the list of supported extensions
          */
        native function get extensions() : String

        /** 
            @duplicate Stream.off
            @hide
         */
        native function off(name, observer: Function): Void

        /** 
            @duplicate Stream.on
            All events are called with the following signature.  The "this" object will be set to the instance object
            if the callback is a method. Otherwise, "this" will be set to the Http instance. If Function.bind may also
            be used to define the "this" object and to inject additional callback arguments. 
                function (event: String, http: Http): Void
            The event has properties: "data" containing the message, "last" set to true if this is the last portion of data for
                a logical message, and "type" set to the WebSockets message type.
            @event headers Issued when the response headers have been fully received.
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event complete Issued when the request completes. Complete is always issued whether the request errors or not.
            @event error Issued if the request does not complete successfully. This is not issued if the request 
                ompletes successfully but with a non 200 Http status code.
            @hide
         */
        native function on(name, observer: Function): Http

        /**
            Return the application-level protocol selected by the server. Protocols are defined when 
            creating the $WebSocket
          */
        native function get protocol() : String

        /**
            The readystate value. This value can be compared with the WebSocket constants: 
            $CONNECTING = 0, $OPEN = 1, $CLOSING = 2, $CLOSED = 3
         */
        native function get readyState() : Number

        /**
            Send data with the request.
            @param content Data to send with the request.
                If the content is a ByteArray, the message will be sent as a WebSocket Binary message.
                Otherwise, the message is converted to a String and sent as a WebSockets UTF-8 Text message.
               If multiple arguments are provided, each is sent as a separate message. 
         */
        native function send(...content): Void

        /*
            @param options
            @options more Set to false for the last frame in a message. If omitted, assumed to be true.
            @options type Message type. Masked to the valid WebSockets message types.
         */
        native function sendBlock(content, options): Void

        /**
            The URI provided to the constructor
         */
        native function get url(): Uri

        /** 
            Wait for the WebSocket to achieve a desired state. 
            @param state Desired ready state value
            @param timeout Timeout in milliseconds to wait for the desired state. A timeout of zero means don't block.
            A timeout of < 0 (default), means wait forever.
            @return True if the desired state was achieved
         */
        native function wait(state: Number, timeout: Number = -1): Boolean
    }
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

/************************************************************************/
/*
    Start of file "src/core/Worker.es"
 */
/************************************************************************/

/*
    Worker -- Worker classes

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        Worker Class. Worker threads are medium-weight thread-based virtual machine instances. They run separate 
        interpreters with tightly controlled data interchange. This class is currently being specified by the
        WebWorker task group (See http://www.whatwg.org/specs/web-workers/current-work/#introduction).
        This class is prototype and highly likely to change
        @spec WebWorker
        @stability prototype.
     */
    class Worker {
        use default namespace public

        /**
            Callback function invoked when the worker exits. 
            The "this" object is set to the worker object.
         */
        var onclose: Function

        /**
            Callback function to receive incoming messages. This is invoked when postMessage is called in another Worker. 
            The "this" object is set to the worker object.
            This is invoked as: function (event) { }
         */
        var onmessage: Function

        /**
            Callback function to receive incoming error events. This is invoked when the Worker thows an exception. 
            The "this" object is set to the worker object.
         */
        var onerror: Function

        /**
            Worker name. This name is initialized but workers can update as required.
            @spec ejs
         */
        var name: String

//  MOB -- should the worker exit when the script completes. Same question for terminate
        /**
            Create a new Worker instance. This call returns an outside worker object for using in the calling interpreter.
                Inside the worker interpreter, a corresponding "inside" worker object is created that is paired to the
                outside worker.
            @params script Optional path to a script or module to execute. If supplied, then a new Worker instance will
                invoke load() to execute the script.
            @params options Options hash
            @options search Search path
            @options name Name of the Worker instance.
            @spec WebWorker and ejs
         */
        native function Worker(script: Path? = null, options: Object? = null)

        /** 
            Clone the worker and underlying interpreter
            @param deep Ignored
            @spec ejs
         */
        native function clone(deep: Boolean = true): Worker

        /** 
            Create a new worker by cloning the current interpreter
            @spec ejs
         */
        native static function fork(): Worker

        /**
            Load the script. The literal script is compiled as a JavaScript program and loaded and run.
            This is similar to the global eval() command but the script is run in its own interpreter and does not
            share any data the the invoking interpreter. The result is serialized in the worker and then deserialized
            (using JSON) in the current interpreter. The call returns undefined if the timeout expires.
            @param script Literal JavaScript program string.
            @param timeout If the timeout is non-zero, this call blocks and will return the value of the last expression in
                the script. Otherwise, this call will not block and join() can be used to wait for completion. Set the
                timeout to -1 to block until the script completes. The default is -1.
            @returns The value of the last expression evaluated in the script. Returns undefined if the timeout 
                expires before the script completes.
            @throws an exception if the script can't be compiled or if it thows a run-time exception.
            @spec ejs
         */
        native function eval(script: String, timeout: Number = -1): Object?

        /**
            Exit the worker.
            @spec ejs
         */
        native static function exit(): Void

        /**
            Wait for Workers to exit.
            @param workers Set of Workers to wait for. Can be a single Worker object or an array of Workers. If null or 
                if the array is empty, then all workers are waited for.
            @param timeout Timeout to wait in milliseconds. The value -1 disables the timeout.
            @spec ejs
         */
        native static function join(workers: Object? = null, timeout: Number = -1): Boolean

        /**
            Load and run a script in a dedicated worker thread. 
            @params script Filename of a script or module to load and run. 
            @param timeout If the timeout is non-zero, this call blocks and will return the value of the last expression in
                the script. Otherwise, this call will not block and join() can be used to wait for completion. Set the
                timeout to -1 to block until the script completes. The default is to not block.
            @spec ejs
         */
        native function load(script: Path, timeout: Number = 0): Void

        /**
            Preload the specified script or module file to initialize the worker. This will run a script using the current
            thread.  This call will load the script/module and initialize and run global code. The call will block until 
            all global code has completed and the script/module is initialized.  To run a worker using its own thread, 
            use load() or Worker(script) instead.
            @param path Filename path for the module or script to load. This should include the file extension.
            @returns the value of the last expression in the script or module.
            @throws an exception if the script or module can't be loaded or initialized or if it thows an exception.
            @spec ejs
         */
        native function preload(path: Path): Object

        /** @hide TODO */
        native function preeval(script: String): Object

        /**
            Lookup a Worker by name
            @param name Lookup a Worker
            @spec ejs
         */
        native static function lookup(name: String): Worker

        /**
            Post a message to the Worker's parent
            @param data Data to pass to the worker's onmessage callback.
            @param ports Not implemented
         */
        native function postMessage(data: Object, ports: Array? = null): Void

        //  MOB - more description?
        /**
            Terminate the worker
         */
        native function terminate(): Void

        /**
            Wait for receipt of a message
            @param timeout Timeout to wait in milliseconds
            @returns True if a message was received
            @stability prototype
         */
        native function waitForMessage(timeout: Number = -1): Boolean
    }

    /** 
        Event for Web Workers
        @spec WebWorker
     */
    class Event extends Error {}

    /** 
        Error event for Web Workers
        @spec WebWorker
     */
    class ErrorEvent extends Error { }


    /*
        Globals for inside workers.
     */
    use default namespace "ejs.worker"

    /**
        Reference to the Worker object for use inside a worker script
        This is only present inside Worker scripts.
     */
    var self: Worker

    /**
        Exit the worker. This is only valid inside Worker scripts.
        @spec ejs
     */
    function exit(): Void
        Worker.exit()

    /**
        Post a message to the Worker's parent. This is only valid inside Worker scripts.
        @param data Data to pass to the worker's onmessage callback.
        @param ports Not implemented
     */
    function postMessage(data: Object, ports: Array? = null): Void
        self.postMessage(data, ports)

    /**
        The error callback function.  This is the callback function to receive incoming data from postMessage() calls.
        This is only present inside Worker scripts.
     */
    function get onerror(): Function
        self.onerror

    function set onerror(fun: Function): Void
        self.onerror = fun

    /**
        The callback function configured to receive incoming messages. 
        This is the callback function to receive incoming data from postMessage() calls.
        This is only present inside Worker scripts.
     */
    function get onmessage(): Function
        self.onmessage

    function set onmessage(fun: Function): Void
        self.onmessage = fun

    # WebWorker         // Only relevant in browsers 
    var location: WorkerLocation
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

/************************************************************************/
/*
    Start of file "src/core/XML.es"
 */
/************************************************************************/

/*
    XML.es - XML class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/*
    TODO - Methods must not be found unless doing a call(). Need some namespace magic for a space that is only open
    for call()
 */

module ejs {

    /**
        The XML class provides a simple ability to load, parse and save XML documents.
     */
    final class XML {

        use default namespace public

        /**
            XML Constructor. Create an empty XML object.
            @param value An optional XML or XMLList object to clone.
         */
        native function XML(value: Object? = null)

        //  TODO - should this be a Path
        /**
            Load an XML document
            @param filename Name of the file containing the XML document to load
         */
        native function load(filename: String): Void

        //  TODO - should this be a Path
        /**
            Save the XML object to a file
            @param filename Name of the file to save the XML document to
         */
        native function save(filename: String): Void

        # FUTURE
        static var ignoreComments: Boolean

        # FUTURE
        static var ignoreProcessingInstructions: Boolean

        # FUTURE
        static var ignoreWhitespace: Boolean

        # FUTURE
        static var prettyPrinting: Boolean

        # FUTURE
        static var prettyIndent: Boolean

        # FUTURE
        native function addNamespace(ns: Namespace): XML

        //  MOB - should be native
        /**
            Append a child to this XML object.
            @param child The child to add.
            @return This object with the added child.
         */
        function appendChild(child: XML): XML {
            this[name()][length()] = child
            return this
        }

        /**
            Get an XMLList containing all of the attributes of this object with the given name.
            @param name The name to search on.
            @return An XMLList with all the attributes (zero or more).
         */
        function attribute(name: String): XMLList
            this["@" + name]

        /**
            Get an XMLList containing all of the attributes of this object.
            @return An XMLList with all the attributes (zero or more).
         */
        function attributes(): XMLList
            this.@*
        
        /**
            Get an XMLList containing the list of children (properties) in this XML object with the given name.
            @param name The name to search on.
            @return An XMLList with all the children names (zero or more).
         */
        function child(name: String): XMLList {
            if (name.isDigit) {
                return this[name cast Number]
            } else {
                return this[name]
            }
        }
        
        /**
            Get the position of this object within the context of its parent.
            @return A number with the zero-based position.
         */
        function childIndex(): Number {
            let p = parent()
            for (i in p) {
                if (p[i] === this) {
                    return i
                }
            }
            return -1
        }
        
        /**
            Get an XMLList containing the children (properties) of this object in order.
            @return An XMLList with all the properties.
         */
        function children(): XMLList 
            this.*

        /**
            Get an XMLList containing the properties of this object that are
            comments.
            @return An XMLList with all the comment properties.
         */
        # FUTURE
        native function comments(): XMLList
        
        /**
            Compare an XML object to another one or an XMLList with only one
            XML object in it. If the comparison operator is true, then one
            object is said to contain the other.
            @return True if this object contains the argument.
         */
        function contains(obj: Object): Boolean {
            for each (item in this) {
                if (item == obj) {
                    return true
                }
            }
            return false
        }

        /**
            Deep copy an XML object. The new object has its parent set to null.
            @return Then new XML object.
         */
        function copy(): XML
            this.clone(true)

        /**
            Get the defaults settings for an XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function defaultSettings(): Object

        /**
            Get all the descendants of this XML object with a given name. The optional argument defaults 
            to getting all descendants.
            @param name The (optional) name to search on.
            @return The list of descendants.
         */
        function descendants(name: String = "*"): Object
            this["." + name]
        
        /**
            Get all the children of this XML node that are elements having the
            given name. The optional argument defaults to getting all elements.
            @param name The (optional) name to search on.
            @return The list of elements.
         */
        function elements(name: String = "*"): XMLList
            this[name]

        /**
            Get an iterator for this node to be used by "for (v in node)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this node to be used by "for each (v in node)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Determine whether this XML object has complex content. If the object has child elements it is 
            considered complex.
            @return True if this object has complex content.
         */
        function hasComplexContent(): Boolean
            this.*.length() > 0

        /**
            Determine whether this object has its own property of the given name.
            @param name The property to look for.
            @return True if this object does have that property.
         */
        override function hasOwnProperty(name: String): Boolean
            this[name] != null
        
        /**
            Determine whether this XML object has simple content. If the object is a text node, an attribute node or 
            an XML element that has no children it is considered simple.
            @return True if this object has simple content.
         */
        function hasSimpleContent(): Boolean
            this.*.length() == 0

        # FUTURE
        native function inScopeNamespaces(): Array

        //  MOB TODO - if marker is null. Insert at beginning or end?
        /**
            Insert a child object into an XML object immediately after a specified marker object. If the marker object 
            is null then the new object is inserted at the end. If the marker object is not found then the insertion 
            is not made.
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildAfter(marker: Object?, child: Object): XML

            // MOB - if marker is null. Insert at beginning or end?
        /**
            Insert a child object into an XML object immediately before a specified marker object. If the marker 
            object is null then the new object is inserted at the end. If the marker object is not found then the
            insertion is not made.
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildBefore(marker: Object?, child: Object): XML

        /**
            Return the length of an XML object in elements. NOTE: this is a method not a property.
            @return A number indicating the number of child elements.
         */
        native function length(): Number
        
        /**
            Get the local name portion of the complete name of this XML object.
            @return The local name.
         */
        # FUTURE
        native function localName(): String

        /**
            Get the qualified name of this XML object.
            @return The qualified name.
         */
        native function name(): String

        # FUTURE
        native function namespace(prefix: String? = null): Object

        # FUTURE
        native function namespaceDeclarations(): Array

        /**
            Get the kind of node this XML object is.
            @return The node kind.
         */
        # FUTURE
        native function nodeKind(): String
        
        /**
            Merge adjacent text nodes into one text node and eliminate empty text nodes.
            @return This XML object.
         */
        # FUTURE
        native function normalize(): XML
        
        /**
            Get the parent of this XML object.
            @return The parent.
         */
        native function parent(): XML

        /**
            Insert a child object into an XML object immediately before all existing properties.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function prependChild(child: Object): XML
        
        /**
            Get all the children of this XML node that are processing instructions having the given name. 
            The optional argument defaults to getting all processing instruction nodes.
            @param name The (optional) name to search on.
            @return The list of processing instruction nodes.
         */
        # FUTURE
        native function processingInstructions(name: String = "*"): XMLList

        /**
            Test a property to see if it will be included in an enumeration over the XML object.
            @param property The property to test.
            @return True if the property will be enumerated.
         */
        # FUTURE
        override native function propertyIsEnumerable(property: Object): Boolean

        /**
            Change the value of a property. If the property is not found, nothing happens.
            @param property The property to change.
            @param value The new value.
            @return True if the property will be enumerated.
         */
        # FUTURE
        native function replace(property: Object, value: Object): void
        
        /**
            Replace all the current properties of this XML object with a new set. The argument can be 
            another XML object or an XMLList.
            @param properties The new property or properties.
            @return This XML object.
         */
        # FUTURE
        native function setChildren(properties: Object): XML
        
        /**
            Set the local name portion of the complete name of this XML object.
            @param The local name.
         */
        # FUTURE
        native function setLocalName(name: String): void

        /**
            Set the qualified name of this XML object.
            @param The qualified name.
         */
        # FUTURE
        native function setName(name: String): void

        /**
            Get the settings for this XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function settings(): Object
        
        /**
            Configure the settings for this XML object.
            @param settings A "settings" object previously returned from a call to the "settings" method.
         */
        # FUTURE
        native function setSettings(settings: Object): void

        /**
            Get all the properties of this XML node that are text nodes having the given name. The optional 
            argument defaults to getting all text nodes.
            @param name The (optional) name to search on.
            @return The list of text nodes.
         */
        # FUTURE
        native function text(name: String = "*"): XMLList

        /**
            Convert to a JSON encoding
            @return A JSON string 
         */
        override native function toJSON(): String 

        /**
            Provides an XML-encoded version of the XML object that includes the tags.
            @return A string with the encoding.
         */
        # FUTURE
        native function toXMLString(): String 

        /**
            Provides a string representation of the XML object.
            @return A string with the encoding.
         */
        override native function toString(): String 
        
        /**
            Return this XML object.
            @return This object.
         */
        override function valueOf(): XML
            this
    }
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

/************************************************************************/
/*
    Start of file "src/core/XMLHttp.es"
 */
/************************************************************************/

/**
    XMLHttp.es -- XMLHttp class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs {

    /**
        XMLHttp compatible method to retrieve HTTP data
        This code is prototype and is not yet supported
        @spec ejs
        @hide
        @stability evolving
     */
    class XMLHttp {

        use default namespace public

        private var hp: Http
        private var state: Number = 0
        private var responseBuf: ByteArray

        //  TODO spec UNSENT
        /** readyState value (UNSENT) */
        static const Uninitialized = 0              

        /** readyState value (OPENED) */
        static const Open = 1

        /** readyState value (SENT) */
        static const Sent = 2

        /** readyState value (LOADING) */
        static const Receiving = 3

        /** readyState value (DONE) */
        static const Loaded = 4

        /**
            Call back function for when the HTTP state changes.
         */
        public var onreadystatechange: Function

        function XMLHttp(http: Http? = null) {
            hp = http || (new Http)
            hp.async = true
        }

        /**
            Abort the connection
         */
        function abort(): void
            hp.close

        /**
            The underlying Http object
            @spec ejs
         */
        function get http() : Http
            hp

        /**
            The readystate value. This value can be compared with the XMLHttp constants: Uninitialized, Open, Sent,
            Receiving or Loaded Set to: Uninitialized = 0, Open = 1, Sent = 2, Receiving = 3, Loaded = 4
         */
        function get readyState() : Number
            state

        /**
            HTTP response body as a string.
         */
        function get responseText(): String
            responseBuf.toString()

        /**
            HTTP response payload as an XML document. Set to an XML object that is the root of the HTTP request 
            response data.
         */
        function get responseXML(): XML
            XML(responseBuf)

        /**
            Not implemented. Only for ActiveX on IE
            @hide
         */
        function get responseBody(): String {
            throw new Error("Unsupported API")
            return ""
        }

        /**
            The HTTP status code. Set to an integer Http status code between 100 and 600.
        */
        function get status(): Number
            hp.status

        /**
            Return the HTTP status code message
         */
        function get statusText() : String
            hp.statusMessage

        /**
            Return the response headers
            @returns a string with the headers catenated together.
         */
        function getAllResponseHeaders(): String {
            let result: String = ""
            for (key in hp.headers) {
                result = result.concat(key + ": " + hp.headers[key] + '\n')
            }
            return result
        }

        /**
            Return a response header. Not yet implemented.
            @param key The name of the response key to be returned.
            @returns the header value as a string
         */
        function getResponseHeader(key: String)
            header(key)

        /**
            Open a connection to the web server using the supplied URL and method.
            @param method HTTP method to use. Valid methods include "GET", "POST", "PUT", "DELETE", "OPTIONS" and "TRACE"
            @param uri URL to retrieve
            @param async If true, don't block after issuing the requeset. By defining an $onreadystatuschange callback 
                function, the request progress can be monitored. NOTE: async mode is not supported. All calls will block.
            @param user Optional user name if authentication is required.
            @param password Optional password if authentication is required.
         */
        function open(method: String, uri: String, async: Boolean = true, user: String? = null, 
                password: String? = null): Void {
            responseBuf = new ByteArray(System.Bufsize)
            hp.async = async
            if (user && password) {
                hp.setCredentials(user, password)
            }
            hp.on(["close", "error"], function (event, ...args) {
                state = Loaded
                notify()
            })
            hp.on("readable", function (event, ...args) {
                while (hp.read(responseBuf, -1) > 0) {
                    state = Receiving
                    notify()
                }
            })
            hp.connect(method, uri)
            state = Open
            notify()
            if (!async) {
                hp.finalize()
                App.waitForEvent(hp, "close", hp.timeout)
            }
        }

        /**
            Send data with the request.
            @param content Data to send with the request.
         */
        function send(content: String? = null): Void {
            if (!hp.async) {
                throw new IOError("Cannot call send in sync mode")
            }
            if (content == null) {
                hp.finalize()
            } else {
                hp.write(content)
            }
        }

        /**
            Set an HTTP header with the request
            @param key Key value for the header
            @param value Value of the header
            @example:
                setRequestHeader("Keep-Alive", "none")
         */
        function setRequestHeader(key: String, value: String): Void
            hp.addHeader(key, value, 1)

        /*
            Invoke the user's state change handler
         */
        private function notify() {
            if (onreadystatechange) {
                if (onreadystatechange.bound) {
                    onreadystatechange(this)
                } else {
                    onreadystatechange.call(this, this)
                }
            }
        }
    }
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

/************************************************************************/
/*
    Start of file "src/core/XMLList.es"
 */
/************************************************************************/

/*
    XMLList.es - XMLList class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

module ejs {

    /**
        The XMLList class is a helper class for the XML class.
        @spec ejs
        @hide
     */
    final class XMLList {

        use default namespace public

        /**
            XML Constructor. Create an empty XML object.
         */
        native function XMLList() 

        # FUTURE
        native function addNamespace(ns: Namespace): XML

        /**
            Append a child to this XML object.
            @param child The child to add.
            @return This object with the added child.
         */
        function appendChild(child: XML): XML {
            this[child.name()] = child
            return this
        }

        /**
            Get an XMLList containing all of the attributes of this object with the given name.
            @param name The name to search on.
            @return An XMLList with all the attributes (zero or more).
         */
        function attribute(name: String): XMLList
            this["@" + name]

        /**
            Get an XMLList containing all of the attributes of this object.
            @return An XMLList with all the attributes (zero or more).
         */
        function attributes(): XMLList
            this.@*

        /**
            Get an XMLList containing the list of children (properties) in this XML object with the given name.
            @param name The name to search on.
            @return An XMLList with all the children names (zero or more).
         */
        function child(name: String): XMLList {
            if (name.isDigit) {
                return this[name cast Number]
            } else {
                return this[name]
            }
        }

        /**
            Get the position of this object within the context of its parent.
            @return A number with the zero-based position.
         */
        function childIndex(): Number {
            let p = parent()
            for (i in p) {
                if (p[i] === this) {
                    return i
                }
            }
            return -1
        }

        /**
            Get an XMLList containing the children (properties) of this object in order.
            @return An XMLList with all the properties.
         */
        function children(): XMLList
            this.*
        
        /**
            Get an XMLList containing the properties of this object that are
            comments.
            @return An XMLList with all the comment properties.
         */
        # FUTURE
        native function comments(): XMLList
        
        /**
            Compare an XML object to another one or an XMLList with only one XML object in it. If the 
            comparison operator is true, then one object is said to contain the other.
            @return True if this object contains the argument.
         */
        # FUTURE
        native function contains(obj: Object): Boolean

        /**
            Deep copy an XML object. The new object has its parent set to null.
            @return Then new XML object.
         */
        function copy(): XML
            this.clone(true)

        /**
            Get the defaults settings for an XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting. 
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function defaultSettings(): Object

        /**
            Get all the descendants (that have the same name) of this XML object. The optional argument defaults 
            to getting all descendants.
            @param name The (optional) name to search on.
            @return The list of descendants.
         */
        function descendants(name: String = "*"): Object
            this["." + name]

        /**
            Get all the children of this XML node that are elements having the
            given name. The optional argument defaults to getting all elements.
            @param name The (optional) name to search on.
            @return The list of elements.
         */
        function elements(name: String = "*"): XMLList
            this[name]

        /**
            Get an iterator for this node to be used by "for (v in node)"
            @return An iterator object.
         */
        override iterator native function get(): Iterator

        /**
            Get an iterator for this node to be used by "for each (v in node)"
            @return An iterator object.
         */
        override iterator native function getValues(): Iterator

        /**
            Determine whether this XML object has complex content. If the object has child elements it is 
            considered complex.
            @return True if this object has complex content.
         */
        function hasComplexContent(): Boolean
            this.*.length() > 0

        /**
            Determine whether this object has its own property of the given name.
            @param name The property to look for.
            @return True if this object does have that property.
         */
        override function hasOwnProperty(name: String): Boolean
            this[name] != null

        /**
            Determine whether this XML object has simple content. If the object
            is a text node, an attribute node or an XML element that has no
            children it is considered simple.
            @return True if this object has simple content.
         */
        # FUTURE
        native function hasSimpleContent(): Boolean

        /**
            Return the namespace in scope
            @return Array of namespaces
         */
        function inScopeNamespaces(): Array
            this.*.length() == 0

        /**
            Insert a child object into an XML object immediately after a specified marker object. If the marker 
            object is null then the new object is inserted at the beginning. If the marker object is not found 
            then the insertion is not made.
            @param marker Insert the new element after this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildAfter(marker: Object?, child: Object): XML

        /**
            Insert a child object into an XML object immediately before a specified marker object. If the marker 
            object is null then the new object is inserted at the end. If the marker object is not found then the
            insertion is not made.
            @param marker Insert the new element before this one.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function insertChildBefore(marker: Object?, child: Object): XML

        /**
            Return the length of an XML object in elements. NOTE: this is a method and must be invoked with ().
            @return A number indicating the number of child elements.
         */
        native function length(): Number

        /**
            Get the local name portion of the complete name of this XML object.
            @return The local name.
         */
        # FUTURE
        native function localName(): String

        /**
            Get the qualified name of this XML object.
            @return The qualified name.
         */
        native function name(): String

        # FUTURE
        native function namespace(prefix: String? = null): Object

        # FUTURE
        native function namespaceDeclarations(): Array

        /**
            Get the kind of node this XML object is.
            @return The node kind.
         */
        # FUTURE
        native function nodeKind(): String

        /**
            Merge adjacent text nodes into one text node and eliminate empty text nodes.
            @return This XML object.
         */
        # FUTURE
        native function normalize(): XML

        /**
            Get the parent of this XML object.
            @return The parent.
         */
        native function parent(): XML

        /**
            Insert a child object into an XML object immediately before all existing properties.
            @param child Child element to be inserted.
            @return This XML object - modified or not.
         */
        # FUTURE
        native function prependChild(child: Object): XML

        /**
            Get all the children of this XML node that are processing instructions having the given name. 
            The optional argument defaults to getting all processing instruction nodes.
            @param name The (optional) name to search on.
            @return The list of processing instruction nodes.
         */
        # FUTURE
        native function processingInstructions(name: String = "*"): XMLList

        /**
            Test a property to see if it will be included in an enumeration over the XML object.
            @param property The property to test.
            @return True if the property will be enumerated.
         */
        # FUTURE
        override native function propertyIsEnumerable(property: Object): Boolean

        /**
            Change the value of a property. If the property is not found, nothing happens.
            @param property The property to change.
            @param value The new value.
            @return True if the property will be enumerated.
         */
        # FUTURE
        native function replace(property: Object, value: Object): void
        
        /**
            Replace all the current properties of this XML object with a new set. The argument can be 
            another XML object or an XMLList.
            @param properties The new property or properties.
            @return This XML object.
         */
        # FUTURE
        native function setChildren(properties: Object): XML

        /**
            Set the local name portion of the complete name of this XML object.
            @param The local name.
         */
        # FUTURE
        native function setLocalName(name: String): void

        /**
            Set the qualified name of this XML object.
            @param The qualified name.
         */
        # FUTURE
        native function setName(name: String): void

        /**
            Get the settings for this XML object. The settings include boolean values for: ignoring comments, 
            ignoring processing instruction, ignoring white space and pretty printing and indenting.
            for details.
            @return Get the settings for this XML object.
         */
        # FUTURE
        native function settings(): Object

        /**
            Configure the settings for this XML object.
            @param settings A "settings" object previously returned from a call to the "settings" method.
         */
        # FUTURE
        native function setSettings(settings: Object): void
        
        /**
            Get all the properties of this XML node that are text nodes having the given name. The optional 
            argument defaults to getting all text nodes.
            @param name The (optional) name to search on.
            @return The list of text nodes.
         */
        # FUTURE
        native function text(name: String = "*"): XMLList
        
        /**
            Convert to a JSON encoding
            @return A JSON string 
         */
        override native function toJSON(): String 

        /**
            Provides a string representation of the XML object.
            @return A string with the encoding.
         */
        override native function toString(): String 

        /**
            Provides an XML-encoded version of the XML object that includes the tags.
            @return A string with the encoding.
         */
        # FUTURE
        native function toXMLString(): String 

        /**
            Return this XML object.
            @return This object.
         */
        override function valueOf(): XML
            this

        /*
           XML
                .prototype
                .ignoreComments
                .ignoreProcessingInstructions
                .ignoreWhitespace
                .prettyPrinting
                .prettyIndent
                .settings()
                .setSettings(settings)
                .defaultSettings()

               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
           XMLList
               function domNode()
               function domNodeList()
               function xpath(XPathExpression)
         */
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.db/Database.es"
 */
/************************************************************************/

/**
    Database.es -- Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB
module ejs.db {

    /**
        SQL Database support. The Database class provides an interface over other database adapter classes such as 
        SQLite or MySQL. Not all the functionality expressed by this API may be implemented by a specific database adapter.
        @spec ejs
        @stability evolving
     */
    class Database {
        private static var defaultDb: Database

        private var adapter: Object
        private var options: Object

        use default namespace public

        /**
            Initialize a database connection using the supplied database connection string. The first opened database
            will also be defined as the default database.
            @param adapter Database adapter to use. E.g. "sqlite". Sqlite is currently the only supported adapter.
            @param options Connection options. This may be filename or an object hash of properties. If set to a filename,
             it should contain the filename of the database on the local system. If options is an object hash, it should 
             contain adapter specific properties that specify how to attach to the database. 
            @option name Database name
            @option username Database username
            @option password Database password
            @option trace Trace database commands to the log
            @option socket Database communications socket
            @option module Module name containing the database connector class. This is a bare module name without ".mod"
                or any leading path.
            @option class Class name containing the database backend.
         */
        function Database(adapter: String, options: Object) {
            Database.defaultDb ||= this
            if (options is String || options is Path) {
                let name = Path(options)
                options = { name: name }
            }
            options.trace ||= false
            this.options = options
            adapter ||= "sqlite"
            options.module ||= ("ejs.db." + adapter)
            let adapterClass = options["class"] || adapter.toPascal()
            //BUG - should be able to use (options.module) below
            let module = options.module
            if (!global.module::[adapterClass]) {
                load(module + ".mod", {reload: false})
                if (!global.module::[adapterClass]) {
                    throw "Cannot find database connector \"" + module + "::" + adapter + "\""
                }
            }
            this.adapter = new global.module::[adapterClass](options)
        }

        //  MOB - inconsistent naming. Add column,index vs create table
        /**
            Add a column to a table.
            @param table Name of the table
            @param column Name of the column to add
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function addColumn(table: String, column: String, datatype: String, options = null): Void
            adapter.addColumn(table, column, datatype, options)

        /**
            Add an index on a column
            @param table Name of the table
            @param column Name of the column to add
            @param index Name of the index
         */
        function addIndex(table: String, column: String, index: String): Void
            adapter.addIndex(table, column, index)

        /**
            Change a column
            @param table Name of the table holding the column
            @param column Name of the column to change
            @param datatype Database independant type of the column. Valid types are: binary, boolean, date,
                datetime, decimal, float, integer, number, string, text, time and timestamp.
            @param options Optional parameters
         */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void
            adapter.changeColumn(table, column, datatype, options)

        /**
            Close the database connection. Database connections should be closed when no longer needed rather than waiting
            for the garbage collector to automatically close the connection when disposing the database instance.
         */
        function close(): Void
            adapter.close()

        /**
            Commit a database transaction
            @hide
         */
        function commit(): Void
            adapter.commit()

        //  TODO - implement
        /**
            Reconnect to the database using new connection options
            @param connectionString See Database() for information about connection options.
            @hide
         */
        function connect(options: Object): Void
            adapter.connect(options)

        /**
            The database connection options
         */
        function get connectionOptions(): Object
            options

        /**
            Create a new database
            @param name Name of the database
            @options Optional parameters
         */
        function createDatabase(name: String, options: Object? = null): Void
            adapter.createDatabase(name, options)

        //  MOB - should be addTable for consistency
        /**
            Create a new table
            @param table Name of the table
            @param columns Array of column descriptor tuples consisting of name:datatype
         */
        function createTable(table: String, columns: Array? = null): Void
            adapter.createTable(table, columns)

        /**
            Map the database independant data type to a database dependant SQL data type
            @param dataType Data type to map
            @returns A string containing the name of the the corresponding SQL database type
         */
        function dataTypeToSqlType(dataType:String): String
            adapter.dataTypeToSqlType(dataType)

        /**
            The default database for the application.
         */
        static function get defaultDatabase(): Database
            defaultDb

        /**
            Set the default database for the application.
            @param db the default database to define
         */
        static function set defaultDatabase(db: Database): Void 
            defaultDb = db

        /**
            Destroy a database
            @param name Name of the database to remove
         */
        function destroyDatabase(name: String): Void
            adapter.destroyDatabase(name)

        //  MOB - should be removeTable for consistency
        /**
            Destroy a table
            @param table Name of the table to destroy
         */
        function destroyTable(table: String): Void
            adapter.destroyTable(table)

        /**
            End a transaction
            @hide
         */
        function endTransaction(): Void
            adapter.endTransaction()

        /**
            Get column information 
            @param table Name of the table to examine
            @return An array of column data. This is database specific content and will vary depending on the
                database connector in use.
         */
        function getColumns(table: String): Array
            adapter.getColumns(table)

        /**
            Return list of tables in a database
            @returns an array containing list of table names present in the currently opened database.
         */
        function getTables(): Array
            adapter.getTables()

        /**
            Return the number of rows in a table
            @returns the count of rows in a table in the currently opened database.
         */
        function getNumRows(table: String): Number
            adapter.getNumRows(table)

        /**
            The name of the database
         */
        function get name(): String
            options.name

        /**
            Execute a SQL command on the database.
            @param cmd SQL command string
            @param tag Debug tag to use when logging the command
            @param trace Set to true to eanble logging this command.
            @returns An array of row results where each row is represented by an Object hash containing the 
                column names and values
            @TODO Refactor logging when Log class implemented
         */
        function query(cmd: String, tag: String = "SQL", trace: Boolean~ = false): Array {
            let mark, size
            trace ||= options.trace
            if (trace) {
                App.log.activity(tag, cmd)
                mark = new Date
                size = Memory.resident
            }
            let result = adapter.sql(cmd)
            if (trace) {
                App.log.activity("Stats", "Elapsed %5.2f msec, memory %5.2f".format(mark.elapsed, 
                    (Memory.resident - size) / (1024 * 1024)))
            }
            return result
        }

        /**
            Remove columns from a table
            @param table Name of the table to modify
            @param columns Array of column names to remove
         */
        function removeColumns(table: String, columns: Array): Void
            adapter.removeColumns(table, columns)

        /**
            Remove an index
            @param table Name of the table to modify
            @param index Name of the index to remove
         */
        function removeIndex(table: String, index: String): Void
            adapter.removeIndex(table, index)

        /**
            Rename a column
            @param table Name of the table to modify
            @param oldColumn Old column name
            @param newColumn New column name
         */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void
            adapter.renameColumn(table, oldColumn, newColumn)

        /**
            Rename a table
            @param oldTable Old table name
            @param newTable New table name
         */
        function renameTable(oldTable: String, newTable: String): Void
            adapter.renameTable(oldTable, newTable)

        /**
            Rollback an uncommited database transaction
            @hide
         */
        function rollback(): Void
            adapter.rollback()

        /**
            Execute a SQL command on the database. This is a low level SQL command interface that bypasses logging.
                Use @query instead.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of row results where each row is represented by an Object hash containing the column 
                names and values
         */
        function sql(cmd: String): Array
            adapter.sql(cmd)

        /**
            Map the SQL type to a database independant data type
            @param sqlType Data type to map
            @returns The corresponding database independant type
         */
        function sqlTypeToDataType(sqlType: String): String
            adapter.sqlTypeToDataType(sqlType)

        /**
            Map the SQL type to an Ejscript type class
            @param sqlType Data type to map
            @returns The corresponding type class
         */
        function sqlTypeToEjsType(sqlType: String): Type
            adapter.sqlTypeToEjsType(sqlType)

        /**
            Start a new database transaction
            @hide
         */
        function startTransaction(): Void
            adapter.startTransaction()

//  MOB -- should be setter/getter
        /**
            Trace all SQL statements on this database. Control whether trace is enabled for all SQL statements 
            issued against the database.
            @param on If true, display each SQL statement to the log
         */
        function trace(on: Boolean): void
            options.trace = on

        /**
            Execute a database transaction
            @param code Function to run inside a database transaction
            @hide
         */
        function transaction(code: Function): Void {
            startTransaction()
            try {
                code()
            } catch (e: Error) {
                rollback();
            } finally {
                endTransaction()
            }
        }

        /**
            Quote ", ', --, ;
            @hide
         */
        static function quote(str: String): String  {
            // str.replace(/'/g, "''").replace(/[#;\x00\x1a\r\n",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#;",;\\-]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[#";\\]/g, "\\$0")
            // return str.replace(/'/g, "''").replace(/[;\\]/g, "\\$0")
            return str.toString().replace(/'/g, "''")
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.db/DatabaseConnector.es"
 */
/************************************************************************/

/*
    DatabaseConnector.es -- Database Connector interface

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB
module ejs.db {

    /**
        Database Connector interface. The database connector interface is the contract implemented by 
        concrete database implementations. This interface is implemented by database connectors such as SQLite and MYSQL.
        @spec ejs
        @stability evolving
     */
    interface DatabaseConnector {

        use default namespace public

        // function DatabaseConnector(connectionString: String)

        /** @duplicate ejs.db::Database.addColumn */
        function addColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.addIndex */
        function addIndex(table: String, column: String, index: String): Void

        /** @duplicate ejs.db::Database.changeColumn */
        function changeColumn(table: String, column: String, datatype: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.close */
        function close(): Void

        /** @duplicate ejs.db::Database.commit 
            @hide
         */
        function commit(): Void

        /** @duplicate ejs.db::Database.connect 
            @hide
         */
        function connect(connectionString: String): Void

        /** @duplicate ejs.db::Database.createDatabase */
        function createDatabase(name: String, options: Object? = null): Void

        /** @duplicate ejs.db::Database.createTable */
        function createTable(table: String, columns: Array? = null): Void

        /** @duplicate ejs.db::Database.dataTypeToSqlType */
        function dataTypeToSqlType(dataType:String): String

        /** @duplicate ejs.db::Database.destroyDatabase */
        function destroyDatabase(name: String): Void

        /** @duplicate ejs.db::Database.destroyTable */
        function destroyTable(table: String): Void

        /** @duplicate ejs.db::Database.getColumns */
        function getColumns(table: String): Array

        /** @duplicate ejs.db::Database.getTables */
        function getTables(): Array

        /** @duplicate ejs.db::Database.removeColumns */
        function removeColumns(table: String, columns: Array): Void 

        /** @duplicate ejs.db::Database.removeIndex */
        function removeIndex(table: String, index: String): Void

        /** @duplicate ejs.db::Database.renameColumn */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void

        /** @duplicate ejs.db::Database.renameTable */
        function renameTable(oldTable: String, newTable: String): Void

        /** @duplicate ejs.db::Database.rollback 
            @hide
         */
        function rollback(): Void

        /** @duplicate ejs.db::Database.sql */
        function sql(cmd: String): Array

        /** @duplicate ejs.db::Database.sqlTypeToDataType */
        function sqlTypeToDataType(sqlType: String): String

        /** @duplicate ejs.db::Database.sqlTypeToEjsType */
        function sqlTypeToEjsType(sqlType: String): String

        /** @duplicate ejs.db::Database.startTransaction 
            @hide */
        function startTransaction(): Void
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.db.mapper/Record.es"
 */
/************************************************************************/

/**
    Record.es -- Record class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB && Config.MAPPER
module ejs.db.mapper {

    require ejs.db

    /**
        Database record class. A record instance corresponds to a row in the database. This class provides a low level 
        Object Relational Mapping (ORM) between the database and Ejscript objects. This class provides methods to create,
        read, update and delete rows in the database. When read or initialized object properties are dynamically created 
        in the Record instance for each column in the database table. Users should subclass the Record class for each 
        database table to manage. When users subclass Record to create models, they should use "implement" rather than
        extend.
        @example 
        public dynamic class MyModel implements Record {}
        @spec ejs
        @stability prototype
     */
    public class Record {
        

        static var  _assocName: String          //  Name for use in associations. Lower case class name
        static var  _belongsTo: Array? = null    //  List of belonging associations

        /*
            Queries to cache. Indexed by model name and optionally query string. Contains cache options for this model.
            Created on demand if cache() is called.
         */
        static var _cacheOptions: Object = {}
        static var _caching: Boolean?

        static var  _className: String          //  Model class name
        static var  _columns: Object            //  List of columns in this database table
        static var  _hasOne: Array? = null      //  List of 1-1 containment associations
        static var  _hasMany: Array? = null     //  List of 1-many containment  associations

        static var  _db: Database               //  Hosting database
        static var  _foreignId: String          //  Camel case class name with "Id". (userCartId))
        static var  _keyName: String            //  Name of the key column (typically "id")
        static var  _model: Type                //  Model class
        static var  _tableName: String          //  Name of the database table. Plural, PascalCase
        static var  _trace: Boolean             //  Trace database SQL statements
        static var  _validations: Array? = null

        static var  _beforeFilters: Array? = null//  Filters that run before saving data
        static var  _afterFilters: Array? = null //  Filters that run after saving data
        static var  _wrapFilters: Array? = null  //  Filters that run before and after saving data

        var _keyValue: Object?                  //  Record key column value
        var _errors: Object                     //  Error message aggregation
        var _cacheAssoc: Object                 //  Cached association data
        var _imodel: Type                       //  Model class

        static var ErrorMessages = {
            accepted: "must be accepted",
            blank: "can't be blank",
            confirmation: "doesn't match confirmation",
            empty: "can't be empty",
            invalid: "is invalid",
            missing: "is missing",
            notNumber: "is not a number",
            notUnique: "is not unique",
            taken: "already taken",
            tooLong: "is too long",
            tooShort: "is too short",
            wrongLength: "wrong length",
            wrongFormat: "wrong format",
        }

        /*
            Initialize the model. This should be called by the model as its very first call.
         */
        if (Object.getName(this) != "Record") {
            _keyName = "id"
            _className = Object.getName(this)
            _model = this
            _assocName = _className.toCamel()
            _foreignId = _className.toCamel() + _keyName.toPascal()
            _tableName = plural(_className).toPascal()
        }

        use default namespace public

        /**
            Constructor for use when instantiating directly from Record. Typically, use models will implement this
            class and will provdie their own constructor which calls initialize().
         */
        function Record(fields: Object? = null) {
            initialize(fields)
        }

        /**
            Construct a new record instance. This is really a constructor function, because the Record class is 
            implemented by user models, no constructor will be invoked when a new user model is instantiated. 
            The record may be initialized by optionally supplying field data. However, the record will not be 
            written to the database until $save is called. To read data from the database into the record, use 
            one of the $find methods.
            @param fields An optional object set of field names and values may be supplied to initialize the record.
         */
        function initialize(fields: Object? = null): Void {
            _imodel = Object.getType(this)
            if (fields) {
                blend(this, fields, {deep: false, overwrite: true})
            }
        }

        /**
            Run filters after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function afterFilter(fn, options: Object? = null): Void {
            _afterFilters ||= []
            _afterFilters.append([fn, options])
        }

        /**
            Run filters before saving data
            @param fn Function to run
            @param options - reserved
         */
        static function beforeFilter(fn, options: Object? = null): Void {
            _beforeFilters ||= []
            _beforeFilters.append([fn, options])
        }

        /**
            Define a belonging reference to another model class. When a model belongs to another, it has a foreign key
            reference to another class.
            @param owner Referenced model class that logically owns this model.
            @param options Optional options hash
            @option className Name of the class
            @option foreignKey Key name for the foreign key
            @option conditions SQL conditions for the relationship to be satisfied
         */
        static function belongsTo(owner, options: Object? = null): Void {
            _belongsTo ||= []
            _belongsTo.append(owner)
        }

        /**
            Database query caching. This caches controls the caching of database records. If enabled, the results
            of queries are cached with a given lifetime. If the lifetime has not expired, subsequent queries will
            be optimized by retrieving cached data. If the record is updated, the cached data will be removed so that
            the next query retrieves fresh data.
            Caching is disabled/enabled via the ejsrc config.cache.database.enable field. It is enabled by default.
            Caching may be used for any Database model, though typically it is most useful for state-less GET requests.
            @param model Model class. This can be a Model class object, "this" or a String model class name.
                You can specify "this" in static code or can also use "this" in class instance
                code and this routine will determine the underlying model class name.
            @param options Cache control options. Default options for all model caching can also be provided by the 
                ejsrc config.cache.database field.
            @option lifespan Time in seconds for the cached output to persist.
            @option query SQL query command to further differentiate cached content. If supplied, different cache data
                can be stored for each query that applies to the given model. If the URI is set to "*" all 
                URIs for the specified model will be uniquely cached. 
            @example 
                cache()
                cache("Store", {lifespan: 200})
                cache(this, {query: "SELECT * from Products"})
                cache(this, {query: "*"})
         */
        static function cache(model = null, options: Object = {}): Void {
            _caching = App.config.cache.database.enable
            if (!_caching) {
                return
            }
            let mname
            if (model == null) {
                mname = _className
            } else if (model is String) {
                mname = model
            } else if (!(model is Type)) {
                model = Object.getType(model)
                mname = Object.getName(model)
            } else {
                mname = Object.getName(model)
            }
            blend(options, App.config.cache.database, {overwrite: false})
            cacheIndex = getCacheIndex(mname)
            _cacheOptions[cacheIndex] = options
            if (options.lifespan is Number) {
                let cacheName = cacheIndex
                if (options.query) {
                    cacheName += "::" + options.query
                }
                /* Invalidate cache data for app reloads */
                App.cache.expire(cacheName, null)
                App.cache.expire(cacheName, Date().future(options.lifespan * 1000))
            }
        }

        private static function getCacheIndex(model: String): String
            "::ejs.db.mapper::" + model

        private static function getCacheName(name: String, options: Object?, query: String?): String {
            if (options && options.query && query) {
                name += "::" + query
            }
            return name
        }


        /*
            Read a single record of kind "model" by the given "key". Data is cached for subsequent reuse.
            Read into rec[field] from table[key]
         */
        private static function cachedRead(rec: Record, field: String, model, key: String, options: Object): Object {
            rec._cacheAssoc ||= {}
            if (rec._cacheAssoc[field] == null) {
                rec._cacheAssoc[field] =  model.readRecords(key, options); 
            }
            return rec._cacheAssoc[field]
        }

        private static function checkFormat(thisObj: Record, field: String, value, options: Object): Void {
            if (! RegExp(options.format).test(value)) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.wrongFormat
            }
        }

        private static function checkNumber(thisObj: Record, field: String, value, options): Void {
            //  MOB - what about decimal, +-
            if (! RegExp(/^[0-9]+$/).test(value)) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.notNumber
            }
        }

        private static function checkPresent(thisObj: Record, field: String, value, options): Void {
            if (value == undefined) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.blank
            } else if (value.length == 0 || value.trim() == "" && thisObj._errors[field] == undefined) {
                thisObj._errors[field] = ErrorMessages.blank
            }
        }

        private static function checkUnique(thisObj: Record, field: String, value, options): Void {
            let grid: Array
            if (thisObj._keyValue) {
                grid = findWhere(field + ' = "' + value + '" AND id <> ' + thisObj._keyValue)
            } else {
                grid = findWhere(field + ' = "' + value + '"')
            }
            if (grid.length > 0) {
                thisObj._errors[field] = (options && options.message) ? options.message : ErrorMessages.notUnique
            }
        }

        /*
            Map types from SQL to ejs when reading from the database
         */
        private function coerceToEjsTypes(): Void {
            for (let field: String in this) {
                let col: Column? = _imodel._columns[field]
                if (col == undefined) {
                    continue
                }
                if (col.ejsType == Object.getType(this[field])) {
                    continue
                }
                let value: String = this[field]
                switch (col.ejsType) {
                case Boolean:
                    if (value is String) {
                        this[field] = (value.trim().toLowerCase() == "true")
                    } else if (value is Number) {
                        this[field] = (value == 1)
                    } else {
                        this[field] = value cast Boolean
                    }
                    this[field] = (this[field]) ? true : false
                    break

                case Date:
                    this[field] = new Date(value)
                    break

                case Number:
                    this[field] = this[field] cast Number
                    break
                }
            }
        }

        /*
            Create associations for a record
         */
        private static function createAssociations(rec: Record, set: Array, preload, options): Void {
            for each (let model in set) {
                if (model is Array) {
                    model = model[0]
                }
                if (preload == true || (preload && preload.contains(model))) {
                    /*
                        Query did table join, so rec already has the data. Extract the fields for the referred model and
                        then remove from rec and replace with an association reference. 
                     */
                    let association = {}
                    if (!model._columns) {
                        model.getSchema()
                    }
                    for (let field: String in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        association[field] = rec[f]
                        delete rec.public::[f]
                    }
                    rec[model._assocName] = model.createRecord(association, options)

                } else {
                    let reader = makeLazyReader(rec, model._assocName, model, rec[model._foreignId])
                    Object.defineProperty(rec, model._assocName, { get: reader })
                    if (!model._columns) model.getSchema()
                    for (let field: String  in model._columns) {
                        let f: String = "_" + model._className + field.toPascal()
                        if (rec[f]) {
                            delete rec.public::[f]
                        }
                    }
                }
            }
        }

        /*
            Create a new record instance and apply the row data
            Process a sql result and add properties for each field in the row
         */
        private static function createRecord(data: Object, options: Object = {}) {
// var before = Memory.resident
            let rec: Record = new global[_className]
            rec.initialize(data)
            rec._keyValue = data[_keyName]

            let subOptions = {}
            if (options.depth) {
                subOptions.depth = options.depth
                subOptions.depth--
            }
            if (options.include) {
                createAssociations(rec, options.include, true, subOptions)
            }
            if (options.depth != 0) {
                if (_belongsTo) {
                    createAssociations(rec, _belongsTo, options.preload, subOptions)
                }
                if (_hasOne) {
                    for each (model in _hasOne) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName] + " LIMIT 1"})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
                if (_hasMany) {
                    for each (model in _hasMany) {
                        if (!rec[model._assocName]) {
                            let reader = makeLazyReader(rec, model._assocName, model, null,
                                {conditions: rec._foreignId + " = " + data[_keyName]})
                            Object.defineProperty(rec, model._assocName, { get: reader })
                        }
                    }
                }
            }
            rec.coerceToEjsTypes()
// print("Record.createRecord(" + _className + ") consumed " + (Memory.resident - before))
            return rec
        }

        /**
            Set an error message. This defines an error message for the given field in a record.
            @param field Name of the field to associate with the error message
            @param msg Error message
         */
        function error(field: String, msg: String): Void {
            field ||= ""
            _errors ||= {}
            _errors[field] = msg
        }

        /*
            Fetch cached data from the cache if present
            @return a response object
         */
        private static function fetchCachedResponse(query: String? = null): Object? {
            let cacheIndex = getCacheIndex(_className)
            let options = _cacheOptions[cacheIndex]
            if (options) {
                let cacheName = getCacheName(cacheIndex, options, query)
                if (!options.query || options.query == "*" || cacheName == (cacheIndex + "::" + options.query)) {
                    let item = App.cache.readObj(cacheName)
                    if (item) {
                        App.log.debug(6, "Use cached database query: " + cacheName)
                        return item
                    }
                }
                App.log.debug(6, "No cached database query for: " + cacheName)
            }
            return null
        }

        //  MOB - should not throw when a record is not found. 
        /**
            Find a record. Find and return a record identified by its primary key if supplied or by the specified options. 
            If more than one record matches, return the first matching record.
            @param key Key Optional key value. Set to null if selecting via the options 
            @param options Optional search option values
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @option columns List of columns to retrieve
            @option conditions { field: value, ...}   or [ "SQL condition", "id == 23", ...]
            @option from Low level from clause (not fully implemented)
            @option keys [set of matching key values]
            @option order ORDER BY clause
            @option group GROUP BY clause
            @option include [Model, ...] Models to join in the query and create associations for. Always preloads.
                The include Model entry may also be an array of [Model, "Join Condition"]
            @option joins Low level join statement "LEFT JOIN vists on stockId = visits.id". Low level joins do not
                create association objects (or lazy loaders). The names of the joined columns are prefixed with the
                appropriate table name using camel case (tableColumn).
            @option limit LIMIT count
            @option depth Specify the depth for which to create associations for belongsTo, hasOne and hasMany relationships.
                 Depth of 1 creates associations only in the immediate fields of the result. Depth == 2 creates in the 
                 next level and so on. Defaults to one.
            @option offset OFFSET count
            @option preload [Model1, ...] Preload "belongsTo" model associations rather than creating lazy loaders. This can
                reduce the number of database queries if iterating through associations.
            @option readonly
            @option lock
         */
        static function find(key: Object?, options: Object = {}): Object? {
            let grid: Array = innerFind(key, 1, options)
            if (grid.length >= 1) {
                let results = createRecord(grid[0], options)
                if (options && options.debug) {
                    print("RESULTS: " + serialize(results))
                }
                return results
            } 
            return null
        }

        /**
            Find all the matching records
            @param options Optional set of options. See $find for list of possible options.
            @returns An array of model records. The array may be empty if no matching records are found
            @throws IOError on internal SQL errors
         */
        static function findAll(options: Object = {}): Array {
            let grid: Array = innerFind(null, null, options)
            // start = new Date
            for (let i = 0; i < grid.length; i++) {
                grid[i] = createRecord(grid[i], options)
            }
            // print("findAll - create records TIME: " + start.elapsed())
            if (options && options.debug) {
                print("RESULTS: " + serialize(grid))
            }
            return grid
        }

        /**
            Find the first record matching a condition. Select a record using a given SQL where clause.
            @param where SQL WHERE clause to use when selecting rows.
            @returns a model record or null if the record cannot be found.
            @throws IOError on internal SQL errors
            @example
                rec = findOneWhere("cost < 200")
         */
        static function findOneWhere(where: String): Object? {
            let grid: Array = innerFind(null, 1, { conditions: [where]})
            if (grid.length >= 1) {
                return createRecord(grid[0])
            } 
            return null
        }

        /**
            Find records matching a condition. Select a set of records using a given SQL where clause
            @param where SQL WHERE clause to use when selecting rows.
            @returns An array of objects. Each object represents a matching row with fields for each column.
            @example
                list = findWhere("cost < 200")
         */
        static function findWhere(where: String, count: Number? = null): Array {
            let grid: Array = innerFind(null, null, { conditions: [where]})
            for (i in grid.length) {
                grid[i] = createRecord(grid[i])
            }
            return grid
        }

        /**
            Return the column names for the table
            @returns an array containing the names of the database columns. This corresponds to the set of properties
                that will be created when a row is read using $find.
         */
        static function getColumnNames(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col)
            }
            return result
        }

        /**
            Return the column names for the record
            @returns an array containing the Pascal case names of the database columns. The names have the first letter
                capitalized. 
         */
        static function getColumnTitles(): Array { 
            if (!_columns) _model.getSchema()
            let result: Array = []
            for (let col: String in _columns) {
                result.append(col.toPascal())
            }
            return result
        }

        /** 
            Get the type of a column
            @param field Name of the field to examine.
            @return A string with the data type of the column
         */
        static function getColumnType(field: String): String {
            if (!_columns) _model.getSchema()
            return _db.sqlTypeToDataType(_columns[field].sqlType)
        }

        /**
            Get the database connection for this record class
            @returns Database instance object created via new $Database
         */
        static function getDb(): Database {
            if (!_db) {
                _db = Database.defaultDatabase
            }
            return _db
        }

        /**
            Get the errors for the record. 
            @return The error message collection for the record.  
         */
        function getErrors(): Object
            _errors

        /**
            Get the key name for this record
         */
        static function getKeyName(): String
            _keyName

        /**
            Return the number of rows in the table
         */
        static function getNumRows(): Number {
            if (!_columns) _model.getSchema()
            let cmd: String = "SELECT COUNT(*) FROM " + _tableName + " WHERE " + _keyName + " <> '';"
            let grid: Array = _db.query(cmd, "numRows", _trace)
            return grid[0]["COUNT(*)"]
        }

        /*
            Read the table schema and return the column hash
         */
        private static function getSchema(): Void {
            if (!_db) {
                _db = Database.defaultDatabase
                if (!_db) {
                    throw new Error("Cannot get schema, database connection has not yet been established")
                }
            }
            let sql: String = 'PRAGMA table_info("' + _tableName + '");'
            let grid: Array = _db.query(sql, "schema", _trace)
            _columns = {}
            for each (let row in grid) {
                let name = row["name"]
                let sqlType = row["type"].toLowerCase()
                let ejsType = mapSqlTypeToEjs(sqlType)
                _columns[name] = new Column(name, false, ejsType, sqlType)
            }
        }

        /**
            Get the associated name for this record
            @returns the database table name backing this record class. Normally this is simply a plural class name. 
         */
        static function getTableName(): String
            _tableName

        /**
            Define a containment relationship to another model class. When using "hasAndBelongsToMany" on another model, it 
            means that other models have a foreign key reference to this class and this class can "contain" many instances 
            of the other models.
            @param model Model. (TODO - not implemented).
            @param options Object hash of options. (TODO - not implemented).
            @option foreignKey Key name for the foreign key. (TODO - not implemented).
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented).
            @option joinTable. (TODO - not implemented).
         */
        static function hasAndBelongsToMany(model: Object, options: Object = {}): Void {
            belongsTo(model, options)
            hasMany(model, options)
        }

        /**
            Check if the record has any errors.
            @return True if the record has errors.
         */
        function hasError(field: String? = null): Boolean {
            if (field) {
                return (_errors && _errors[field])
            }
            if (_errors) {
                return (Object.getOwnPropertyCount(_errors) > 0)
            } 
            return false
        }

        /**
            Define a containment relationship to another model class. When using "hasMany" on another model, it means 
            that other model has a foreign key reference to this class and this class can "contain" many instances of 
            the other.
            @param model Model class that is contained by this class. 
            @param options Options parameter
            @option things Model object that is posessed by this. (TODO - not implemented)
            @option through String Class name which mediates the many to many relationship. (TODO - not implemented)
            @option foreignKey Key name for the foreign key. (TODO - not implemented)
         */
        static function hasMany(model: Object, options: Object = {}): Void {
            _hasMany ||= []
            _hasMany.append(model)
        }

        /**
            Define a containment relationship to another model class. When using "hasOne" on another model, 
            it means that other model has a foreign key reference to this class and this class can "contain" 
            only one instance of the other.
            @param model Model class that is contained by this class. 
            @option thing Model that is posessed by this. (TODO - not implemented).
            @option foreignKey Key name for the foreign key (TODO - not implemented).
            @option as String  (TODO - not implemented).
         */
        static function hasOne(model: Object, options: Object? = null): Void {
            _hasOne ||= []
            _hasOne.append(model)
        }

        /*
            Common find implementation. See find/findAll for doc.
         */
        static private function innerFind(key: Object?, limit: Number? = null, options: Object? = {}): Array {
            let cmd: String
            let columns: Array
            let from: String
            let conditions: String
            let where: Boolean
            let results: Array?

            if (!_columns) _model.getSchema()
            if (_caching && (results = fetchCachedResponse())) {
                return results
            }
            if (options == null) {
                options = {}
            }
            //  LEGACY 1.0.0-B2
            if (options.noassoc) {
                options.depth = 0
            }
            if (options.columns) {
                columns = options.columns
                /*
                    Qualify "id" so it won't clash when doing joins. If the "click" option is specified, must have an ID
                    TODO - Should not modify the parameter. This is actually modifying the options passed in.
                 */
                let index: Number = columns.indexOf("id")
                if (index >= 0) {
                    columns[index] = _tableName + ".id"
                } else if (!columns.contains(_tableName + ".id")) {
                    columns.insert(0, _tableName + ".id")
                }
            } else {
                columns = ["*"]
            }

            conditions = ""
            from = ""
            where = false

            if (options.from) {
                from = options.from
            } else {
                from = _tableName
            }

            if (options.include) {
                let model
                if (options.include is Array) {
                    for each (entry in options.include) {
                        if (entry is Array) {
                            model = entry[0]
                            from += " LEFT OUTER JOIN " + model._tableName
                            from += " ON " + entry[1]
                        } else {
                            model = entry
                            from += " LEFT OUTER JOIN " + model._tableName
                        }
                    }
                } else {
                    model = options.include
                    from += " LEFT OUTER JOIN " + model._tableName
                    // conditions = " ON " + model._tableName + ".id = " + _tableName + "." + model._assocName + "Id"
                }
            }

            if (options.depth != 0) {
                if (_belongsTo) {
                    conditions = " ON "
                    for each (let owner in _belongsTo) {
                        from += " INNER JOIN " + owner._tableName
                    }
                    for each (let owner in _belongsTo) {
                        let tname: String = Object.getName(owner)
                        tname = tname[0].toLowerCase() + tname.slice(1) + "Id"
                        conditions += _tableName + "." + tname + " = " + owner._tableName + "." + owner._keyName + " AND "
                    }
                    if (conditions == " ON ") {
                        conditions = ""
                    }
                }
            }

            if (options.joins) {
                if (conditions == "") {
                    conditions = " ON "
                }
                let parts: Array = options.joins.split(/ ON | on /)
                from += " " + parts[0]
                if (parts.length > 1) {
                    conditions += parts[1] + " AND "
                }
            }
            conditions = conditions.trim(" AND ")

            if (options.conditions) {
                let whereConditions: String = " WHERE "
                if (options.conditions is Array) {
                    for each (cond in options.conditions) {
                        whereConditions += cond + " " + " AND "
                    }
                    whereConditions = whereConditions.trim(" AND ")

                } else if (options.conditions is String) {
                    whereConditions += options.conditions + " " 

                } else if (options.conditions is Object) {
                    for (field in options.conditions) {
                        //  Remove quote from options.conditions[field]
                        whereConditions += field + " = '" + options.conditions[field] + "' " + " AND "
                    }
                }
                whereConditions = whereConditions.trim(" AND ")
                if (whereConditions != " WHERE ") {
                    where = true
                    conditions += whereConditions
                } else {
                    whereConditions = ""
                    from = from.trim(" AND ")
                }

            } else {
                from = from.trim(" AND ")
            }

            if (key || options.key) {
                if (!where) {
                    conditions += " WHERE "
                    where = true
                } else {
                    conditions += " AND "
                }
                conditions += (_tableName + "." + _keyName + " = ") + ((key) ? key : options.key)
            }

            //  Removed quote from "from"
            cmd = "SELECT " + Database.quote(columns) + " FROM " + from + conditions
            if (options.group) {
                cmd += " GROUP BY " + options.group
            }
            if (options.order) {
                cmd += " ORDER BY " + options.order
            }
            if (limit) {
                cmd += " LIMIT " + limit
            } else if (options.limit) {
                cmd += " LIMIT " + options.limit
            }
            if (options.offset) {
                cmd += " OFFSET " + options.offset
            }
            cmd += ";"

            if (_db == null) {
                throw new Error("Database connection has not yet been established")
            }
            if (_caching && (results = fetchCachedResponse(cmd))) {
                return results
            }
            if (results == null) {
                try {
                    if (_trace) {
                        let start = new Date
                        results = _db.query(cmd, "find", _trace)
                        App.log.activity("TIME", "Query Time:", start.elapsed)
                        App.log.info("Query Time:", start.elapsed)
                    } else {
                        results = _db.query(cmd, "find", _trace)
                    }
                    if (_caching) {
                        saveQuery(results, cmd)
                    }
                } 
                catch (e) {
                    throw e
                }
            }
            return results
        }

        /*
            Make a getter function to lazily (on-demand) read associated records (belongsTo)
         */
        private static function makeLazyReader(rec: Record, field: String, model, key: String?, 
                options: Object = {}): Function {
            // print("Make lazy reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
            var lazyReader: Function = function(): Object {
                // print("Run reader for " + _tableName + "[" + field + "] for " + model._tableName + "[" + key + "]")
                return cachedRead(rec, field, model, key, options)
            }
            return lazyReader
        }

        private static function mapSqlTypeToEjs(sqlType: String): Type {
            sqlType = sqlType.replace(/\(.*/, "")
            let ejsType: Type? = _db.sqlTypeToEjsType(sqlType)
            if (ejsType == undefined) {
                throw new Error("Unsupported SQL type: \"" + sqlType + "\"")
            }
            return ejsType
        }

        /*
            Prepare a value to be written to the database
         */
        private static function prepareValue(field: String, value: Object?): String {
            let col: Column? = _columns[field]
            if (col == undefined) {
                return undefined
            }
            if (value == undefined) {
                throw new Error("Field \"" + field + "\" is undefined")
            }
            if (value == null) {
                throw new Error("Field \"" + field + "\" is null")
            }
            switch (col.ejsType) {
            case Boolean:
                if (value is String) {
                    value = (value.toLowerCase() == "true")
                } else if (value is Number) {
                    value = (value == 1)
                } else {
                    value = value cast Boolean
                }
                return value

            case Date:
                return "%d".format((new Date(value)).time)

            case Number:
                return value cast Number
             
            case String:
                return Database.quote(value)
            }
            return Database.quote(value.toString())
        }

        /*
            Read records for an assocation. Will return one or an array of records matching the supplied key and options.
         */
        private static function readRecords(key: String, options: Object): Object {
            let data: Array = innerFind(key, null, options)
            if (data.length > 1) {
                let result: Array = new Array
                for each (row in data) {
                    result.append(createRecord(row))
                }
                return result

            } else if (data.length == 1) {
                return createRecord(data[0])
            }
            return null
        }

        /**
            Remove records from the database
            @param ids Set of keys identifying the records to remove
         */
        static function remove(...ids): Void {
            for each (let key: Object in ids) {
                let cmd: String = "DELETE FROM " + _tableName + " WHERE " + _keyName + " = " + key + ";"
                getDb().query(cmd, "remove", _trace)
            }
        }

        private function runFilters(filters): Void {
            for each (filter in filters) {
                let fn = filter[0]
                let options = filter[1]
                if (options) {
                    let only = options.only
/* TODO
                    if (only) {
                        if (only is String && actionName != only) {
                            continue
                        }
                        if (only is Array && !only.contains(actionName)) {
                            continue
                        }
                    } 
                    except = options.except
                    if (except) {
                        if (except is String && actionName == except) {
                            continue
                        }
                        if (except is Array && except.contains(actionName)) {
                            continue
                        }
                    }
*/
                }
                fn.call(this)
            }
        }

        /**
            Save the record to the database.
            @returns True if the record is validated and successfully written to the database
            @throws IOError Throws exception on sql errors
         */
        function save(): Boolean {
            var sql: String
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            if (!validateRecord()) {
                return false
            }
            runFilters(_imodel._beforeFilters)
            
            if (_keyValue == null) {
                sql = "INSERT INTO " + _imodel._tableName + " ("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + ", "
                    }
                }
                sql = sql.trim(', ')
                sql += ") VALUES("
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += "'" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += ")"

            } else {
                sql = "UPDATE " + _imodel._tableName + " SET "
                for (let field: String in this) {
                    if (_imodel._columns[field]) {
                        sql += field + " = '" + prepareValue(field, this[field]) + "', "
                    }
                }
                sql = sql.trim(', ')
                sql += " WHERE " + _imodel._keyName + " = " +  _keyValue
            }
            if (!_keyValue) {
                sql += "; SELECT last_insert_rowid();"
            } else {
                sql += ";"
            }

            let result: Array = _imodel._db.query(sql, "save", _imodel._trace)
            if (!_keyValue) {
                _keyValue = this["id"] = result[0]["last_insert_rowid()"] cast Number
            }
            runFilters(_imodel._afterFilters)
            if (_imodel._caching) {
                let cacheIndex = getCacheIndex(_imodel._className)
                let options = _cacheOptions[cacheIndex]
                if (options == null || options.query == null) {
                    let cacheName = getCacheName(cacheIndex, options)
                    App.log.debug(6, "Expire database query cache " + cacheName)
                    App.cache.writeObj(cacheName, null, options)
                } else {
                }
            }
            return true
        }

        /*
            Save the output from a database query for future reuse
         */
        private static function saveQuery(results, query: String? = null): Void {
            let cacheIndex = getCacheIndex(_className)
            let options = _cacheOptions[cacheIndex]
            if (options) {
                let cacheName = getCacheName(cacheIndex, options, query)
                App.cache.writeObj(cacheName, results, options)
                App.log.debug(6, "Cache database query " + cacheName)
            }
        }

        /**
            Update a record based on the supplied fields and values.
            @param fields Hash of field/value pairs to use for the record update.
            @returns True if the database is successfully updated. Returns false if validation fails. In this case,
                the record is not saved.
            @throws IOError on database SQL errors
         */
        function saveUpdate(fields: Object): Boolean {
            for (field in fields) {
                if (this[field] != undefined) {
                    this[field] = fields[field]
                }
            }
            return save()
        }

        /**
            Set the database connection for this record class
            @param database Database instance object created via new $Database
         */
        static function setDb(database: Database) {
            _db = database
        }

        /**
            Set the key name for this record
         */
        static function setKeyName(name: String): Void {
            _keyName = name
        }

        /**
            Set the associated table name for this record
            @param name Name of the database table to backup the record class.
         */
        static function setTableName(name: String): Void {
            if (_tableName != name) {
                _tableName = name
                if (_db) {
                    _model.getSchema()
                }
            }
        }

        /**
            Run an SQL statement and return selected records.
            @param cmd SQL command to issue. Note: "SELECT" is automatically prepended and ";" is appended for you.
            @returns An array of objects. Each object represents a matching row with fields for each column.
         */
        static function sql(cmd: String, count: Number? = null): Array {
            cmd = "SELECT " + cmd + ";"
            return getDb().query(cmd, "select", _trace)
        }

        /**
            Trace SQL statements. Control whether trace is enabled for the actual SQL statements issued against the database.
            @param on If true, display each SQL statement to the log
         */
        static function trace(on: Boolean): void
            _trace = on

        /** @hide TODO */
        static function validateFormat(fields: Object, options = null) {
            _validations ||= []
            _validations.append([checkFormat, fields, options])
        }

        /** @hide TODO */
        static function validateNumber(fields: Object, options = null) {
            _validations ||= []
            _validations.append([checkNumber, fields, options])
        }

        /** @hide TODO */
        static function validatePresence(fields: Object, options = null) {
            _validations ||= []
            _validations.append([checkPresent, fields, options])
        }

        /**
            Validate a record. This call validates all the fields in a record.
            @returns True if the record has no errors.
         */
        function validateRecord(): Boolean {
            _imodel ||= Object.getType(this)
            if (!_imodel._columns) _imodel.getSchema()
            _errors = {}
            if (_imodel._validations) {
                for each (let validation in _imodel._validations) {
                    let [check, fields, options] = validation
                    if (fields is Array) {
                        for each (let field in fields) {
                            if (_errors[field]) {
                                continue
                            }
                            check(this, field, this[field], options)
                        }
                    } else {
                        check(this, fields, this[fields], options)
                    }
                }
            }
            let thisType = Object.getType(this)
            if (this.validate) {
                this.validate.call(this)
            }
            coerceToEjsTypes()
            return Object.getOwnPropertyCount(_errors) == 0
        }

        /** @hide TODO */
        static function validateUnique(fields: Object, options = null) {
            _validations ||= []
            _validations.append([checkUnique, fields, options])
        }

        /**
            Run filters before and after saving data
            @param fn Function to run
            @param options - reserved
         */
        static function wrapFilter(fn, options: Object? = null): Void {
            _wrapFilters ||= []
            _wrapFilters.append([fn, options])
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get columnNames(): Array {
            return getColumnNames()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get columnTitles(): Array {
            return getColumnTitles()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get db(): Datbase {
            return getDb()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get keyName(): String {
            return getKeyName()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get numRows(): String {
            return getNumRows()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        static function get tableName(): String {
            return getTableName()
        }

        /** 
            @hide 
            @deprecated 1.0.0B2
         */
        # Config.Legacy
        function constructor(fields: Object? = null): Void {
            initialize(fields)
        }
    }


    /**
        Database column class. A database record is comprised of one or mor columns
        @hide
     */
    class Column {
        //  TODO - workaround. Make these public, see ticket 1227: ejsweb generate was not finding them when internal
        //  missing the internal namespace.
        public var ejsType: Object 
        public var sqlType: Object 

        function Column(name: String, accessor: Boolean = false, ejsType: Type? = null, sqlType: String? = null) {
            this.ejsType = ejsType
            this.sqlType = sqlType
        }
    }

    /** @hide */
    function plural(name: String): String
        name + "s"

    /** @hide */
    function singular(name: String): String {
        var s: String = name + "s"
        if (name.endsWith("ies")) {
            return name.slice(0,-3) + "y"
        } else if (name.endsWith("es")) {
            return name.slice(0,-2)
        } else if (name.endsWith("s")) {
            return name.slice(0,-1)
        }
        return name.toPascal()
    }

    /**
        Map a type assuming it is already of the correct ejs type for the schema
        @hide
     */
    function mapType(value: Object): String {
        if (value is Date) {
            return "%d".format((new Date(value)).time)
        } else if (value is Number) {
            return "%d".format(value)
        }
        return value
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.db.sqlite/Sqlite.es"
 */
/************************************************************************/

/**
    Sqlite.es -- SQLite Database class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

# Config.DB && Config.SQLITE
module ejs.db.sqlite {

    /**
        SQLite database support
        @spec ejs
        @stabilitiy prototype
     */
    class Sqlite {

        /*
            Map independent types to SQL types
         */
        static var DataTypeToSqlType: Object = {
            "binary":       "blob",
            "boolean":      "tinyint",
            "date":         "date",
            "datetime":     "datetime",
            "decimal":      "decimal",
            "float":        "float",
            "integer":      "int",
            "number":       "decimal",
            "string":       "varchar",
            "text":         "text",
            "time":         "time",
            "timestamp":    "datetime",
        }

        /*
            Map independent types to SQL types
         */
        static var SqlTypeToDataType: Object = {
            "blob":         "binary",
            "tinyint":      "boolean",
            "date":         "date",
            "datetime":     "datetime",
            "decimal":      "decimal",
            "float":        "float",
            "int":          "integer",
            "varchar":      "string",
            "text":         "text",
            "time":         "time",
        }

        /*
            Map SQL types to Ejscript native types
         */
        static var SqlTypeToEjsType: Object = {
            "blob":         String,
            "date":         Date,
            "datetime":     Date,
            "decimal":      Number,
            "int":          Number,
            "integer":      Number,
            "float":        Number,
            "time":         Date,
            "tinyint":      Boolean,
            "text":         String,
            "varchar":      String,
        }

        /*
            Map Ejscript native types back to SQL types. INCOMPLETE.
         
        static var EjsToDataType: Object = {
            "string":       "varchar",
            "number":       "decimal",
            "date":         "datetime",
            "bytearray":    "Blob",
            "boolean":      "tinyint",
        }
         */

        use default namespace public

        /**
            Initialize a database connection using the supplied database connection string
            @param options Connection options stipulating how to connect to the database. This can be a simple
                String or Path specifying the filename for the database, or it can be an object with a "name" property
                containing an access URI for the database.
            @options name Database name URI specifying the SQLite database to open. 
                Example: file://var/spool/db/database.db
         */
        native function Sqlite(options: Object)

        /** @duplicate ejs.db::Database.addColumn */
        function addColumn(table: String, column: String, datatype: String, options = null): Void {
            let mapped = DataTypeToSqlType[datatype.toLowerCase()]
            if (mapped == undefined) {
                throw "Bad Ejscript column type: " + datatype
            }
            query("ALTER TABLE " + table + " ADD " + column + " " + mapped)
        }

        /** @duplicate ejs.db::Database.addIndex */
        function addIndex(table: String, column: String, index: String): Void
            query("CREATE INDEX " + index + " ON " + table + " (" + column + ");")

        /** 
            @duplicate ejs.db::Database.changeColumn 
            @hide
            SQLite cannot change or rename columns.
         */
        function changeColumn(table: String, column: String, datatype: String, options = null): Void {
            datatype = datatype.toLowerCase()
            if (DataTypeToSqlType[datatype] == undefined) {
                throw "Bad column type: " + datatype
            }
            /* query("ALTER TABLE " + table + " CHANGE " + column + " " + datatype) */
            throw "SQLite does not support column changes"
        }

        /** @duplicate ejs.db::Database.close */
        native function close(): Void

        /** @duplicate ejs.db::Database.commit 
            @hide*/
        function commit(): Void {}

        //  TODO - implement in native code
        /** 
            @duplicate ejs.db::Database.connect 
            @hide
         */
        native function connect(connectionString: String): Void

        /** @duplicate ejs.db::Database.createDatabase */
        function createDatabase(name: String, options = null): Void {
            /* Nothing to do for sqlite */
        }

        /** @duplicate ejs.db::Database.createTable */
        function createTable(table: String, columns: Array? = null): Void {
            query("DROP TABLE IF EXISTS " + table + ";")
            query("CREATE TABLE " + table + "(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL);")
            if (columns) {
                for each (let colspec: String in columns) {
                    //  TODO - destructuring assignment would be good here
                    let spec: Array = colspec.split(":")
                    if (spec.length != 2) {
                        throw "Bad column spec: " + spec
                    }
                    let column: String = spec[0]
                    let datatype: String = spec[1]
                    addColumn(table, column.trim(), datatype.trim())
                }
            }
        }

        /** @duplicate ejs.db::Database.dataTypeToSqlType */
        function dataTypeToSqlType(dataType:String): String
            Object.getType(this).DataTypeToSqlType[dataType]

        /** @duplicate ejs.db::Database.destroyDatabase */
        function destroyDatabase(name: String): Void
            Path(name).remove()

        /** @duplicate ejs.db::Database.destroyTable */
        function destroyTable(table: String): Void
            query("DROP TABLE IF EXISTS " + table + ";")

        /** @duplicate ejs.db::Database.endTransaction 
            @hide */
        function endTransaction(): Void {}

        /** @duplicate ejs.db::Database.getColumns */
        function getColumns(table: String): Array {
            grid = query('PRAGMA table_info("' + table + '");')
            let names = []
            for each (let row in grid) {
                let name: String = row["name"]
                names.append(name)
            }
            return names
        }

        /**
            @duplicate ejs.db::Database.getNumRows
         */
        function getNumRows(table: String): Number {
            let cmd: String = "SELECT COUNT(*) FROM " + table + ";"
            let grid: Array = query(cmd, "numRows")
            return grid[0]["COUNT(*)"] cast Number
        }

        /** @duplicate ejs.db::Database.getTables */
        function getTables(): Array {
            let cmd: String = "SELECT name from sqlite_master WHERE type = 'table' order by NAME;"
            let grid: Array = query(cmd)
            let result: Array = new Array
            for each (let row: Object in grid) {
                let name: String = row["name"]
                if (!name.contains("sqlite_") && !name.contains("_Ejs")) {
                    result.append(row["name"])
                }
            }
            return result
        }

        /** @duplicate ejs.db::Database.removeColumns */
        function removeColumns(table: String, columns: Array): Void {
            /*
                This is a dumb SQLite work around because it doesn't have drop column
             */
            backup = "_backup_" + table
            keep = getColumns(table)
            for each (column in columns) {
                if ((index = keep.indexOf(column)) < 0) {
                    throw "Column \"" + column + "\" does not exist in " + table
                } 
                keep.remove(index)
            }

            //  TODO - good to have a utility routine for this
            schema = 'PRAGMA table_info("' + table + '");'
            grid = query(schema)
            types = {}
            for each (let row in grid) {
                let name: String = row["name"]
                types[name] = row["type"]
            }

            columnSpec = []
            for each (k in keep) {
                if (k == "id") {
                    columnSpec.append(k + " INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL")
                } else {
                    columnSpec.append(k + " " + types[k])
                }
            }

            cmd = "BEGIN TRANSACTION;
                CREATE TEMPORARY TABLE " + backup + "(" + columnSpec + ");
                INSERT INTO " + backup + " SELECT " + keep + " FROM " + table + ";
                DROP TABLE " + table + ";
                CREATE TABLE " + table + "(" + columnSpec + ");
                INSERT INTO " + table + " SELECT " + keep + " FROM " + backup + ";
                DROP TABLE " + backup + ";
                COMMIT;"
            query(cmd)
        }

        /** @duplicate ejs.db::Database.removeIndex */
        function removeIndex(table: String, index: String): Void
            query("DROP INDEX " + index + ";")

        /** 
            @duplicate ejs.db::Database.renameColumn 
            @hide
            SQLite does not support renaming columns.
         */
        function renameColumn(table: String, oldColumn: String, newColumn: String): Void {
            throw "SQLite does not support renaming columns"
            // query("ALTER TABLE " + table + " RENAME " + oldColumn + " TO " + newColumn + ";")
        }

        /** @duplicate ejs.db::Database.renameTable */
        function renameTable(oldTable: String, newTable: String): Void
            query("ALTER TABLE " + oldTable + " RENAME TO " + newTable + ";")

        /** 
            @duplicate ejs.db::Database.rollback 
            @hide
         */
        function rollback(): Void {}

        //  MOB - why have query and sql

        /** @duplicate ejs.db::Database.query */
        function query(cmd: String, tag: String = "SQL", trace: Boolean~ = false): Array {
            //  TODO - need to access Database.traceAll
            let mark, size
            //  MOB - rationalize Sqlite.query with Database.query and Record.innerFind
            if (trace) {
                App.log.debug(0, tag + ": " + cmd)
                mark = new Date
                size = Memory.resident
            }
            let result = sql(cmd)
            if (trace) {
                App.log.activity("Stats", "Sqlite query %.2f msec, memory %.2f MB, resident %.2f".format(mark.elapsed, 
                    (Memory.resident - size) / (1024 * 1024), Memory.resident / (1024 * 1024)))
            }
            return result
        }

        /** @duplicate ejs.db::Database.sql */
        native function sql(cmd: String): Array

        /** @duplicate ejs.db::Database.sqlTypeToDataType */
        function sqlTypeToDataType(sqlType: String): String
            Sqlite.SqlTypeToDataType[sqlType]

        /** @duplicate ejs.db::Database.sqlTypeToEjsType */
        function sqlTypeToEjsType(sqlType: String): Type
            Sqlite.SqlTypeToEjsType[sqlType]

        /** @duplicate ejs.db::Database.startTransaction 
            @hide */
        function startTransaction(): Void {}
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.mail/Mail.es"
 */
/************************************************************************/

/*
   Mail.es -- Simple mail using sendmail
 */

module ejs.mail {
    /** 
        Send mail using sendmail
        @param options Object hash with properties for: from, to, subject, date and Content-Type.
        @param msg String message to send
        @example:
            sendmail({
            to: 'john@example.com',
            subject: 'Hello',
            from: 'judy@example.com',
            type: 'text/plain',
            date: new Date(),
            files: [],
            attach: [],
            }, 'Welcome to Example.com')
     */
    public function sendmail(options: Object, message: String): Void {
        if (!options['to']) {
            throw "Missing to"
        }
        options.from ||= 'anonymous'
        options.subject ||= 'Mail message'
        let boundary = 'ejs.mail=' + md5('BOUNDARY')
        let msg = []
        msg.push('To: ' + options['to'])
        msg.push('From: ' + options.from)
        msg.push('Date: ' + (options.date || new Date()))
        msg.push('Subject: ' + options.subject)
        msg.push('MIME-Version: 1.0')
        msg.push('Content-Type: multipart/mixed; boundary=' + boundary)
        msg.push('')
        boundary = '--' + boundary
        msg.push(boundary)
        msg.push('Content-Type: ' + (options.type || 'text/plain') + '\n\n' + message)
        msg.push('')

        for each (let file: Path in options.files) {
            msg.push(boundary)
            let mime = Uri(file).mimeType || 'application/octet-stream'
            msg.push('Content-Transfer-Encoding: base64')
            msg.push('Content-Disposition: inline; filename=' + file.basename + '')
            msg.push('Content-Type: ' + mime + '; name="' + file.basename + '"')
            msg.push('')
            let encoded = base64(file.readString())
            for (i = 0; i < encoded.length; i += 76) {
                msg.push(encoded.slice(i, i + 76))
            }
        }
        for each (let data in options.attach) {
            msg.push(boundary)
            msg.push('Content-Type: application/octet-stream')
            msg.push('Content-Transfer-Encoding: base64')
            if (data is ByteArray) {
                data = data.readString()
            }
            let encoded = base64(data)
            for (i = 0; i < encoded.length; i += 76) {
                msg.push(encoded.slice(i, i + 76))
            }
        }
        msg.push(boundary + '--')
        Cmd.run(['sendmail', '-t'], {}, msg.join('\n'))
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.tar/Tar.es"
 */
/************************************************************************/

/**
    Tar.es -- Tar archive management
 */

# Config.TAR
module ejs.tar {

    const BlockSize = 512

    /*
        Operations
     */
    const Extract: Number = 1
    const Info: Number = 2
    const List: Number = 3
    const Read: Number = 4

    /*
        Types
     */
    const Regular: Number = 0
    const HardLink: Number = 1
    const SymLink: Number = 2
    const CharSpecial: Number = 3
    const BlockSpecial: Number = 4
    const Directory: Number = 5
    const Fifo: Number = 6

    class Tar {
        private var path: Path
        private var options: Object

        use default namespace public

        /*
            Create a new Tar object
            @param path Pathname to the tar archive
            @param options Configurable options
            @option chdir Directory to change to before executing the desired operation. In create mode,
                the archive is created, then the directory is changed before adding files. In extract mode,
                the archive is opened, then the directory is changed before extracting files.
            @option relativeTo Path
            @option uid User id to use instead of the file user id
            @option gid Group id to use instead of the file group id
            @option user User name to use instead of the file user name
            @option group Group name to use instead of the file group name
         */
        function Tar(path: Path, options: Object = {}) {
            this.path = path
            this.options = options
        }

        function get name(): Path
            this.path

        private function flatten(args: Array): Array {
            let files = []
            for each (file in args) {
                if (file is Array) {
                    files += file
                } else {
                    files.push(file)
                }
            }
            for (i in files) {
                files[i] = Path(files[i])
            }
            return files
        }

        function create(...args): Void {
            var archive: File  = File(path, {mode: 'w', permissions: 0644})
            let home = App.dir
            try {
                if (options.chdir) {
                    App.chdir(options.chdir)
                }
                for each (file in flatten(args)) {
                    if (!file.exists) {
                        throw 'File does not exist: ' + file
                    }
                    let header = new TarHeader(options)
                    header.createHeader(file)
                    header.write(archive)
                    if (header.type == Regular) {
                        let fp = File(file, 'r')
                        let data = new ByteArray
                        while (fp.read(data)) {
                            archive.write(data)
                        }
                        data.flush()
                        let remainder = 512 - (file.size % 512)
                        if (remainder < 512) {
                            data.writePosition = remainder
                            archive.write(data)
                        }
                        fp.close()
                    }
                }
            } finally {
                App.chdir(home)
            }
            archive.close()
        }

        function operate(files: Array, operation) {
            let archive = File(path, 'r')
            let home = App.dir
            let result = []
            try {
                if (options.chdir) {
                    App.chdir(options.chdir)
                }
                let data: ByteArray?
                while ((data = archive.readBytes(BlockSize)) != null && data[0]) {
                    let header = new TarHeader(options)
                    header.parse(data)
                    let path = header.path
                    if (files.contains(path) || files.length == 0) {
                        if (operation == Extract) {
                            Path(header.name).dirname.makeDir()
                            if (header.type == HardLink) {
                                Path(header.linkName).link(header.name, true)
                            } else if (header.type == SymLink) {
                                Path(header.linkName).link(header.name)
                            } else {
                                let fp = new File(header.name, 'w')
                                let len = header.size
                                let buf = new ByteArray(32 * 1024)
                                while (len > 0) {
                                    buf.flush()
                                    count = len.min(buf.size)
                                    bytes = archive.read(buf, 0, count)
                                    fp.write(buf)
                                    len -= count
                                }
                                fp.close()
                            }
                            try {
                                if (App.uid == 0) {
                                    path.setAttributes(header.attributes)
                                }
                            } catch {}

                        } else if (operation == Info) {
                            result.push({
                                path: header.path,
                                mode: header.mode,
                                uid: header.uid,
                                gid: header.gid,
                                size: header.size,
                                modified: header.modified,
                                user: header.user,
                                group: header.group,
                                type: header.type,
                            })
                            archive.position += header.size

                        } else if (operation == List) {
                            result.push(path)
                            archive.position += header.size

                        } else if (operation == Read) {
                            let result = new ByteArray(header.size)
                            archive.read(result, 0, header.size)
                            return result
                        }
                    } else {
                        archive.position += header.size
                    }
                    let remainder = 512 - (header.size % 512)
                    if (remainder < 512) {
                        archive.position += remainder;
                    }
                }
            } finally {
                App.chdir(home)
            }
            archive.close()
            return result
        }

        function extract(...args): Void
            operate(flatten(args), Extract)

        function info(...args): Array
            operate(flatten(args), Info)

        function list(...args): Array
            operate(flatten(args), List)

        function read(...args): ByteArray
            operate(flatten(args), Read)

        function readString(...args): String
            operate(flatten(args), Read)
    }

    internal class TarHeader {
        var name: String            /* File name. Up to 100 characters. Null terminated if room */
        var mode: Number            /* File permission (octal) */
        var uid: Number             /* User ID (octal) */
        var gid: Number             /* (octal) */
        var size: Number            /* File size (octal) */
        var modified: Date          /* Last modified time in Seconds since Jan 1 1970 (octal) */
        var type: Number            /* Type flag */
        var linkName: String        /* Link target name. Up to 100 characters. Null terminate if room */
        var magic: String           /* Set to 'ustar' null terminated */
        var user: String?           /* User name. Up to 32 chars, null terminated */
        var group: String?          /* Group name. Up to 32 chars, null terminated */
        var major: Number           /* Device major (octal) */
        var magic: String           /* Magic string */
        var minor: Number           /* Device minor (octal) */
        var prefix: String          /* Prefix to file name. Null-terminated if room */
        var sum: Number             /* Header checksum */
        var version: String         /* Version string */
        var options: Object         /* Tar object options */

        function TarHeader(options) {
            this.options = options
        }

        /*
            Create a new header entry
         */
        function createHeader(path: Path) {
            let attributes = path.attributes
            mode = attributes.permissions
            uid = options.uid || attributes.uid
            gid = options.gid || attributes.gid
            user = options.user || attributes.user
            group = options.group || attributes.group
            size = path.size
            modified = path.modified
            link = path.isLink
            if (link) {
                type = SymLink
                linkName = path.linkTarget
                mode = path.dirname.attributes.permissions
                modified = path.dirname.modified
                size = 0
            } else {
                type = Regular
            }
            if (options.relativeTo) {
                path = path.relativeTo(options.relativeTo)
            }
            if (path.length > 100) {
                name = path.basename.portable
                prefix = path.dirname.portable
                if (name.length > 100 || prefix.length > 155) {
                    throw 'Path name ' + path + ' is too long'
                }
            } else {
                name = path.portable
            }
        }

        function parse(header: ByteArray) {
            let csum = checksum(header)
            name = parseStr(header, 100)
            mode = parseNum(header, 8)
            uid = parseNum(header, 8)
            gid = parseNum(header, 8)
            size = parseNum(header, 12)
            modified = Date(parseNum(header, 12) * 1000)
            sum = parseNum(header, 8)
            type = parseNum(header, 1)
            linkName = parseStr(header, 100)
            magic = parseStr(header, 6)
            version = parseStr(header, 2)
            user = parseStr(header, 32)
            group = parseStr(header, 32)
            major = parseNum(header, 8)
            minor = parseNum(header, 8)
            prefix = parseStr(header, 155)
            if (sum != csum) {
                throw 'Bad file format. Checksum mismatch: ' + sum + ' ' + csum
            }
        }

        function get attributes() {
            let att = {}
            if (uid) att.uid = uid
            if (gid) att.gid = gid
            if (user) att.user = user
            if (group) att.group = group
            return att
        }

        function get path(): Path {
            return Path((prefix) ? (Path(prefix).join(name)) : name)
        }

        function show(ba) {
            for (i = 0; i < ba.length; i += 16) {
                stdout.write(['%07o    ' % [i]])
                for (j in 16) {
                    stdout.write(['%02X  ' % ba[i+j]])
                }
                stdout.write('\n')
            }
        }

        /*
            See: 
                http://www.subspacefield.org/~vax/tar_format.html
                http://www.fileformat.info/format/tar/corion.htm
         */
        function write(fp: File) {
            let ba = new ByteArray
            ba.write(name)
            ba.writePosition = 100
            ba.write('%06o ' % [mode]) ; ba.writePosition++
            ba.write('%06o ' % [uid]) ; ba.writePosition++
            ba.write('%06o ' % [gid]) ; ba.writePosition++
            ba.write('%011o ' % [size])
            ba.write('%011o ' % [modified.time / 1000])
            ba.writePosition = 156
            ba.write('%1d' % [type])
            if (linkName) {
                ba.write(linkName) ; ba.writePosition += linkName.length
                ba.writeByte(0)
            }
            ba.writePosition = 257
            ba.write('ustar')
            ba.writePosition = 263
            ba.write('00')
            ba.writePosition = 265
            ba.write('%s' % [user])
            ba.writePosition = 297
            ba.write('%s' % [group])
            if (prefix) {
                ba.writePosition = 345
                ba.write(prefix)
            }
            ba.writePosition = 512
            /* 
                The checksum is stored as as 6 octal digits followed by a null + space 
             */
            let csum = checksum(ba)
            ba.writePosition = 148
            ba.write('%06o' % [csum])
            ba.writePosition++
            ba.write(' ')
            ba.writePosition = 512
            fp.write(ba)
        }

        /*
            Calculate a checksum of the 512-byte header
         */
        private function checksum(data: ByteArray): Number {
            let total = 0
            for (i = 0; i < 512; i++) {
                if (i == 148) {
                    /* Skip the checksum field */
                    total += ' '.charCodeAt() * 8
                    i += 7
                } else {
                    total += data[i]
                }
            }
            return total
        }

        private function parseStr(data: ByteArray, count: Number): String {
            let i, j
            for (i = 0, j = data.readPosition; i < count; i++, j++) {
                if (data[j] == 0) {
                    break
                }
            }
            let s = data.readString(i)
            data.readPosition += count - i
            return s
        }

        private function parseNum(data: ByteArray, count: Number) {
            field = data.readString(count)
            return parseInt(field, 8)
        }
    }

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

/************************************************************************/
/*
    Start of file "src/jems/ejs.template/TemplateParser.es"
 */
/************************************************************************/

/*
    TemplateParser.es -- Ejscript web templating parser. 
 */

# Config.TEMPLATE
module ejs.template  {

    /*
        TODO implement these directives
          -%>  Omit newline after tag
          <%s   For safe output Html escape the output
     */

    /** 
        Web Page Template Parser - Parse an ejs web page and emit an Ejscript source file (.es).
      
        This parser handles embedded Ejscript using <% %> directives. It supports:
        <ul>
          <li>&lt;%       Begin an ejs directive section containing statements</li>
          <li>&lt;%=      Begin an ejs directive section that contains an expression to evaluate and substitute</li>
          <li>%&gt;          End an ejs directive</li>
          <li>&lt;%&#64; include "file" %> Include an ejs file</li>
          <li>&lt;%&#64 layout "file" %>  Specify a layout page to use. Use layout "" to disable layout management.</li>
          <li>&lt;%&#64 view "[module::]class" %>  Specify a view class to use with optional module.</li>
          <li>&lt;%&#64 content Mark the location to substitute content in a layout page
        </ul>
        Directives for use outside of &lt;% %&gt; 
        <ul>
          <li>&#64;&#64var &mdash; To expand the value of "var". Var can also be simple expressions (without spaces).</li>
        </ul>

        @spec ejs
        @stability prototype
     */
    public class TemplateParser {
        private const ContentMarker: String  = "__ejs:CONTENT:ejs__"
        private const ContentPattern: RegExp = new RegExp(ContentMarker)

        private var script: String
        private var pos: Number = 0
        private var lineNumber: Number = 0
        private var viewClass: String = "View"
        private var viewModule: String

        /**
            Build a templated page
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
         */
        public function build(script: String, options: Object = {}): String {
            let code = parse(script, options)
            return "require ejs.web\n" + 
                ((viewModule) ? ("require " + viewModule + "\n") : "") + 
                "\nexports.app = function (request: Request) {\n" + 
                "    " + viewClass + "(request).render(function(request: Request) {\n" + code + "\n    })\n}\n"
        }

        /**
            Template parser. Parse the given script and return the compiled (Ejscript) result
            @param script String containing the script to parse
            @param options Object hash with options to control parsing
            @options layout Path Layout file
            @options dir Path Base directory to use for including files and for resolving layout directives
            @return The parsed and expanded template 
         */
        public function parse(script: String, options: Object = {}): String {
            var token: ByteArray = new ByteArray
            var out: ByteArray = new ByteArray
            var dir: Path = options.dir || Path(".")
            var tid: Number
            var layoutPage: Path?

            if (options.layout) {
                layoutPage = Path(options.layout)
            }
            this.script = script
            while ((tid = getToken(token)) != Token.Eof) {
                // print("getToken => " + Token.tokens[tid + 1] + " TOKEN => \"" + token + "\"")

                switch (tid) {
                case Token.Literal:
                    //  OPT -- should amalgamate writes
                    out.write("\n        write(\"" + token + "\");")
                    break

                case Token.Var:
                    /*
                        Trick to get undefined variables to evaluate to "".
                        Catenate with "" to cause toString to run.
                        Write safely by HTML escaping the expression
                     */
                    out.write("\n        writeSafe(\"\" + ", token, ");\n")
                    break

                case Token.Equals:
                    /* Write safely by HTML escaping the expression */
                    out.write("\n        writeSafe(\"\" + (", token, "));\n")
                    break

                case Token.EjsTag:
                    /*
                        Just copy the Ejscript code straight through
                     */
                    out.write(token.toString())
                    break

                case Token.Control:
                    let args: Array = token.toString().split(/\s/g)
                    let cmd: String = args[0]

                    switch (cmd) {
                    case "include":
                        let path = args[1].trim("'").trim('"')
                        let incPath = dir.join(path)
                        /*
                            Recurse and process the include script
                         */
                        let inc: TemplateParser = new TemplateParser
                        out.write(inc.parse(incPath.readString(), options))
                        break

                    case "layout":
                        let layouts = options.layouts || App.config.dirs.layouts || "layouts"
                        let path = args[1]
                        if (path == "" || path == '""') {
                            layoutPage = undefined
                        } else {
                            layoutPage = Path(args[1].trim("'").trim('"').trim('.ejs') + ".ejs")
                            if (!layoutPage.exists) {
                                layoutPage = Path(layouts).join(layoutPage)
                                if (!layoutPage.exists) {
                                    throw "Cannot find layout page " + layoutPage
                                }
                            }
                        }
                        break

                    case "view":
                        viewClass = args[1].trim("'").trim('"')
                        if (viewClass.contains("::")) {
                            [viewModule, viewClass] = viewClass.split("::")
                        }
                        viewClass ||= "View"
                        break

                    case "content":
                        out.write(ContentMarker)
                        break

                    default:
                        throw "Bad control directive: " + cmd
                    }
                    break

                default:
                case Token.Err:
                    //  TODO - should report line numbers
                    throw "Bad input token: " + token

                }
            }
            if (layoutPage && layoutPage != options.currentLayout) {
                let layoutOptions = blend(options.clone(), { currentLayout: layoutPage })
                let layoutText: String = new TemplateParser().parse(layoutPage.readString(), layoutOptions)
                return layoutText.replace(ContentPattern, out.toString().replace(/\$/g, "$$$$"))
            }
            return out.toString()
        }

        /*
         *  Get the next input token. Read from script[pos]. Return the next token ID and update the token byte array
         */
        function getToken(token: ByteArray): Number {
            var tid = Token.Literal
            token.flush(Stream.BOTH)
            var c
            while (pos < script.length) {
                c = script[pos++]
                switch (c) {
                case '<':
                    if (script[pos] == '%' && (pos < 2 || script[pos - 2] != '\\')) {
                        if (token.length > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        eatSpace()
                        if (script[pos] == '=') {
                            /*
                                <%=  directive
                             */
                            pos++
                            eatSpace()
                            while (pos < script.length && (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>' || script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Equals

                        } else if (script[pos] == '@') {
                            /*
                                <%@  directive
                             */
                            pos++
                            eatSpace()
                            while (pos < script.length && (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.Control

                        } else {
                            while (pos < script.length && 
                                    (c = script[pos]) != undefined && 
                                    (c != '%' || script[pos+1] != '>' || script[pos-1] == '\\')) {
                                token.write(c)
                                pos++
                            }
                            pos += 2
                            return Token.EjsTag
                        }
                    }
                    token.write(c)
                    break

                case '@':
                    if (script[pos] == '@' && (pos < 1 || script[pos-1] != '\\')) {
                        if (token.length > 0) {
                            pos--
                            return Token.Literal
                        }
                        pos++
                        c = script[pos++]
                        while (c.isAlpha || c.isDigit || c == '[' || c == ']' || c == '.' || c == '$' || c == '_' || 
                                c == "'") {
                            token.write(c)
                            c = script[pos++]
                        }
                        pos--
                        return Token.Var
                    }
                    token.write(c)
                    break

                case "\n":
                    lineNumber++
                case "\r":
                    token.write(c)
                    tid = Token.Literal
                    break

                default:
                    //  TODO - triple quotes would eliminate the need for this
                    if (c == '\"' || c == '\\') {
                        token.write('\\')
                    }
                    token.write(c)
                    break
                }
            }
            if (token.length == 0 && pos >= script.length) {
                return Token.Eof
            }
            return tid
        }

        private function eatSpace(): Void {
            while (pos < script.length && script[pos].isSpace) {
                pos++
            }
        }
    }

    /**
        Parser tokens
        @hide
     */
    class Token {
        public static const Err         = -1        /* Any input error */
        public static const Eof         = 0         /* End of file */
        public static const EjsTag      = 1         /* <% text %> */
        public static const Var         = 2         /* @@var */
        public static const Literal     = 3         /* literal HTML */
        public static const Equals      = 4         /* <%= expression */
        public static const Control     = 6         /* <%@ control */

        public static var tokens = [ "Err", "Eof", "EjsTag", "Var", "Literal", "Equals", "Control" ]
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.unix/Unix.es"
 */
/************************************************************************/

/*
    Unix.es -- Unix compatibility functions

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module ejs.unix {

    // use default namespace public

    /**
        Get the base name of a file. Returns the base name portion of a file name. The base name portion is the 
        trailing portion without any directory elements.
        @return A string containing the base name portion of the file name.
     */
    function basename(path: String): Path
        Path(path).basename
    
    //  TODO - should this be cd()
    /**
        Change the current working directory
        @param dir Directory String or path to change to
     */
    function chdir(dir: Object): Void
        App.chdir(dir)

    /**
        Set the permissions of a file or directory
        @param path File or directory to modify
        @param perms Posix style permission mask
     */
    function chmod(path: String, perms: Number): Void
        Path(path).perms = perms

    /**
        Copy files
        The src argument can ba an array of Strings/Paths representing files/directories to copy. The may include
        embedded wildcards. If a src element is a directory, then it all all files and subdirectories will be copied
        preserving its directory structure under dest. 

        @param src Source files/directories to copy. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory and file. The Posix "[]" and "{a,b}" style expressions are not supported.
            If src is an existing directory, then the pattern is converted to 'dir/ * *' (without spaces) 
            and the tree option is enabled.
        @param dest Destination file or directory. If multiple files are copied, dest is assumed to be a directory and 
            will be created if required.  If dest has a trailing "/", it is assumed to be a directory.
        @param options Processing and file attributes
        @options owner String representing the file owner                                                     
        @options group String representing the file group                                                     
        @options permissions Number File Posix permissions mask
        @options tree Copy the src subtree and preserve the directory structure under the destination.
        @return Number of files copied
    */
    function cp(src, dest: Path, options = {}): Number {
        if (!(src is Array)) src = [src]
        let count = 0
        for each (let pattern: Path in src) {
            let base: Path = Path('.')
            if (pattern.isDir) {
                base = pattern
                pattern = Path('**')
                options = blend({tree: true, relative: true}, options)
            }
            list = base.files(pattern, options)

            if (!list || list.length == 0) {
                throw 'cp: Cannot find files to copy "' + pattern + '" to ' + dest
            }
            destIsDir = (dest.isDir || list.length > 1 || dest.name.endsWith('/'))

            for each (let file: Path in list) {
                let to, from = base.join(file)
                if (options.tree) {
                    to = dest.join(base, file).normalize
                } else if (destIsDir) {
                    to = dest.join(file.basename)
                } else {
                    to = dest
                }
                to.dirname.makeDir()
                if (from.isDir) {
                    to.makeDir()
                } else {
                    from.copy(to, options)
                }
                count++
            }
        }
        return count
    }

    /**
        Get the directory name portion of a file. The dirname name portion is the leading portion including all 
        directory elements and excluding the base name. On some systems, it will include a drive specifier.
        @return A string containing the directory name portion of the file name.
     */
    function dirname(path: String): Path
        Path(path).dirname

    /**
        Does a file exist. Return true if the specified file exists and can be accessed.
        @param path Filename path to examine.
        @return True if the file can be accessed
     */
    function exists(path: String): Boolean
        Path(path).exists

    /**
        Get the file extension portion of the file name. The file extension is the portion after the last "."
        in the path.
        @param path Filename path to examine
        @return String containing the file extension. It excludes "." as the first character.
     */
    function extension(path: String): String
        Path(path).extension

    /**
        Return the free space in the file system.
        @return The number of 1M blocks (1024 * 1024 bytes) of free space in the file system.
     */
    function freeSpace(path: String? = null): Number
        FileSystem(path).freeSpace()

    /**
        Is a file a directory. Return true if the specified path exists and is a directory
        @param path Directory path to examine.
        @return True if the file can be accessed
     */
    function isDir(path: String): Boolean
        Path(path).isDir

    /**
        Kill a process
        @param pid Process ID to kill
        @param signal Signal number to use when killing the process.
        @hide
        @deprecated 2.0.0
     */
    function kill(pid: Number, signal: Number = 2): Void 
        Cmd.kill(pid, signal)

    /**
        Get a list of files in a directory.
        @param patterns Pattern to match files. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options If set to true, then files will include sub-directories in the returned list of files.
        @option dirs Include directories in the file list
        @option depthFirst Do a depth first traversal. If "dirs" is specified, the directories will be shown after
        the files in the directory. Otherwise, directories will be listed first.
        @option exclude Regular expression pattern of files to exclude from the results. Matches the entire path.
        @option hidden Show hidden files starting with "."
        @option include Regular expression pattern of files to include in the results. Matches the entire returned path.
        @return An Array of Path objects for each matching file.
     */
    function ls(patterns = "*", options: Object? = null): Array {
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        let results = []
        for each (let pat: Path in patterns) {
            if (pat.exists && pat.isDir) {
                pat = pat.join("*")
            }
            results += Path(".").files(pat, options)
        }
        return results
    }


    /**
        Find files under a directory.
        This uses Path.files to implement the functionality.
        @param path Path or array of paths from which to search
        @param patterns Pattern to match files. This can be a String, Path or array of String/Paths. 
        The wildcard '?' matches any single character, '*' matches zero or more characters in a filename or 
            directory, '** /' matches zero or more files or directories and matches recursively in a directory
            tree.  If a pattern terminates with "/" it will only match directories. 
            The pattern '**' is equivalent to '** / *' (ignore spaces). 
            The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options Optional properties to control the matching.
        @option depthFirst Do a depth first traversal. If "dirs" is specified, the directories will be shown after
            the files in the directory. Otherwise, directories will be listed first.
        @option exclude Regular expression pattern of files to exclude from the results. Matches the entire path.
            Only for the purpose of this match, directories will have "/" appended. To exclude directories in the
            results, use {exclude: /\/$/}. The trailing "/" will not be returned in the results.
        @option hidden Include hidden files starting with "."
        @option include Regular expression pattern of files to include in the results. Matches the entire returned path.
            Only for the purpose of this match, directories will have "/" appended. To include only directories in the
            results, use {include: /\/$/}
        @option missing Set to undefined to report patterns that don't resolve into any files or directories 
            by throwing an exception. Set to any non-null value to be used in the results when there are no matching
            files or directories. Set to the empty string to use the patterns in the results and set
            to null to do nothing.
        @option relative Return paths relative to the Path, otherwise result entries include the Path. Defaults to false.
        @return An Array of Path objects for each file in the directory.
     */
    function find(path: Object, patterns: Object! = "*", options = {}): Array {
        let result = []
        if (path is Array) {
            let paths = path
            for each (path in paths) {
                result += Path(path).files(patterns, options)
            }
        } else {
            result += Path(path).files(patterns, options)
        }
        return result
    }

    /**
        Make a new directory. Makes a new directory and all required intervening directories. If the directory 
        already exists, the function returns without throwing an exception.
        @param path Filename path to use.
        @param permissions Optional posix permissions mask number. e.g. 0664.
        @throws IOError if the directory cannot be created.
     */
    function mkdir(path: String, permissions: Number = 0755): void
        Path(path).makeDir({permissions: permissions})
    
    //  MOB - should allow toFile to be a directory
    //  MOB - both args should be Paths
    /**
        Rename a file. If the new file name exists it is removed before the rename.
        @param fromFile Original file name.
        @param toFile New file name.
        @throws IOError if the original file does not exist or cannot be renamed.
     */
    function mv(fromFile: String, toFile: String): void
        Path(fromFile).rename(toFile)

    /**  
        Open or create a file
        @param path Filename path to open
        @param mode optional file access mode with values values from: Read, Write, Append, Create, Open, Truncate. 
            Defaults to Read.
        @param permissions optional permissions. Defaults to App.permissions
        @return a File object which implements the Stream interface
        @throws IOError if the path or file cannot be opened or created.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function open(path: String, mode: String = "r", permissions: Number = 0644): File
        new File(path, { mode: mode, permissions: permissions})

    /**
        Get the current working directory
        @return A Path containing the current working directory
     */
    function pwd(): Path
        App.dir

    /**  
        Read data bytes from a file and return a byte array containing the data.
        @param file Open file object previously opened via $open or $File
        @param count Number of bytes to read
        @return A byte array containing the read data
        @throws IOError if the file could not be read.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function read(file: File, count: Number): ByteArray?
        file.read(count)

    /**
        Remove files from the file system.
        @param patterns Pattern to match files to remove. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options Options to modify the removal
        @option exclude Regular expression pattern of files to exclude from removal. Matches the entire path.
        @option hidden Remove hidden files starting with "." Defaults to true.
        @option include Regular expression pattern of files to remove. Matches the entire returned path.
        @option nodirs Exclude directories from removal. The default is to include directories.
        @return True if all the contents are sucessfully deleted. Otherwise return false.
    */
    function rm(patterns, options = {}): Boolean {
        options = blend({depthFirst: true, hidden: true}, options)
        let success = true
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        for each (let pat:Path in patterns) {
            for each (let path: Path in Path('.').files(pat, options)) {
                if (!path.remove()) {
                    success = false
                }
            }
        }
        return success
    }

    /**
        Removes a directory and contents
        @param patterns Pattern to match files to delete. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory. The Posix "[]" and "{a,b}" style expressions are not supported.
        @param options Options to modify the removal
        @option exclude Regular expression pattern of files to exclude from removal. Matches the entire path.
        @option hidden Remove hidden files starting with "."
        @option include Regular expression pattern of files to remove. Matches the entire returned path.
        @return True if all the contents are sucessfully deleted or if the directory does not exist. Otherwise return false.
     */
    function rmdir(patterns, options = {}): Boolean {
        options = blend({depthFirst: true, hidden: true}, options)
        let success = true
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        for each (let pat:Path in patterns) {
            for each (let path: Path in Path('.').files(pat)) {
                if (path.isDir) {
                    rmdir(path.join('*'), options)
                }
                if (!path.remove()) {
                    success = false
                }
            }
        }
        return success
    }

    /** 
        Sleep the application for the given number of milliseconds. Events will be serviced while asleep.
        @param delay Time in milliseconds to sleep. Set to -1 to sleep forever.
     */
    function sleep(delay: Number): void
        App.sleep(delay)

    /**
        Create a temporary file. Creates a new, uniquely named temporary file.
        @param directory Directory in which to create the temp file.
        @returns a closed File object after creating an empty temporary file.
     */
    function tempname(directory: String? = null): File
        Path(directory).makeTemp()

    /**
        Write data to the file. If the stream is in sync mode, the write call blocks until the underlying stream or 
        endpoint absorbes all the data. If in async-mode, the call accepts whatever data can be accepted immediately 
        and returns a count of the elements that have been written.
        @param file Open file object previously opened via $open or $File
        @param items The data argument can be ByteArrays, strings or Numbers. All other types will call serialize
        first before writing. Note that numbers will not be written in a cross platform manner. If that is required, use
        the BinaryStream class to write Numbers.
        @returns the number of bytes written.  
        @throws IOError if the file could not be written.
        @hide
        @deprecated 2.0.0
     */
    # Config.Legacy
    function write(file: File, ...items): Number
        file.write(items)
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Cascade.es"
 */
/************************************************************************/

/*
    Cascade slices and return the response from the first non-404 slice
    Usage:
        require ejs.web
        exports.app = Cascade(app)
 */

# Config.WEB
module ejs.web {
    /** 
        Cascade middleware script for web apps.
        @param apps Array of applications to try in turn. The first app to return a valid status (!= Http.NotFound) 
        will conclude the request.
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = Cascade(app1, app2, app3)
     */
    function Cascade(...apps): Object? {
        return function(request) {
            for each (app in apps) {
                let response = app(request)
                if (response.status != Http.NotFound) {
                    return response
                }
            }
            return null
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/CommonLog.es"
 */
/************************************************************************/

/*
    CommonLog.es -- Common Log Format logger.
 */

# Config.WEB
module ejs.web {
    /** 
        Common Log web server logging. 
        This function is a middleware constructor that returns a web application function.
        @param app Application servicing the request and generating the response.
        @param logger Stream to use for writing access log information
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = CommonLog(app)
        @spec ejs
        @stability prototype
     */
    function CommonLog(app, logger: Stream = App.log): Object {
        return (new CommonLogClass(app, logger)).app
    }

    //  TODO - not a great name having Class in the name
    /**
        Common Log web server logging.
        This logs each HTTP request to a file in the Common Log format defined by the Apache web server.
     */
    class CommonLogClass {
        var innerApp: Function
        var logger: Stream

        /**
            Constructor for the CommonLogClass
            @param app Application function
            @param logger Optional logger. Defaults to App.log
         */
        function CommonLogClass(app, logger: Stream = App.log) {
            this.innerApp = app
            this.logger = logger
        }

        //  MOB -- rename
        function app(request: Request): Object {
            let start = new Date
            let response = innerApp.call(request, request)
            let size = (response.body is String) ? response.body.length : 0
            /*
                Sample:  10.0.0.5 - - [16/Mar/2013:15:40:36 -0700] "GET /index.html HTTP/1.1" 200 44
             */
            let user = request.authUser || "-"
            let uri = request.pathInfo + (request.query ? ("?" + request.query) : "")
            logger.write(request.remoteAddress + ' - ' + user + ' [' + Date().format("%d/%b/%Y %T %Z") + 
                '] "' + request.method + ' ' + uri + ' ' + request.protocol + '" ' + response.status + ' ' + 
                size + ' ' + start.elapsed + "\n")
            return response
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/ContentType.es"
 */
/************************************************************************/

/*
    ContentType.es -- Define Content-Type headers
 */

# Config.WEB
module ejs.web {

    /**
        ContentType wrapper middleware. This defines a Content-Type Http header for the content based on an inferred
        mime type from the Request.pathInfo extension.
        @param app Application generating the response. 
        @param options Options providing an optional mimeTypes lookup table and default mime type value.
        @option mimeTypes Hash table of extension keys with mime type values. If not provided, the Uri.mimeType lookup
            table is used.
        @option defaultType Default mime type to use if no matching type is found
        @return A web application function that services a web request and when invoked with the request object will 
        yield a response object.
        @example:
            export.app = ContentType(app, { mimeTypes: { "md": "text/x-markdown" }})
     */
    function ContentType (app, options) {
        options ||= {}
        return function(request: Request) {
            var response = app(request)
            response.headers ||= {}
            let mimeType
            if (options.mimeTypes) {
                mimeType = options.mimeTypes[request.extension]
            } else {
                mimeType pathInfo.mimeType
            }
            if (mimeType) {
                response.headers["content-type"] ||= mimeType
            }
            return response
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Controller.es"
 */
/************************************************************************/

/*
    Controller.es -- MVC Controller class.
 */

# Config.WEB
module ejs.web {
    /**
        Namespace for all action methods 
     */
    namespace action = "action"

    /**
        Request for which a controller is being constructed
     */
    var _initRequest: Request?

    //  DOC - need more doc here on controllers
    /** 
        Web framework controller class. The Controller class is part of the Ejscript Model View Controller (MVC) web
        framework. Controller class instances can accept web requests and direct them to action methods for servicing
        which generates the response. Controllers are responsible for either generating output for the client or invoking
        a View which will create the response. By convention, all Controllers should be defined with a "Controller" 
        suffix. This permits similar Controller and Model classes to exist in the same namespace.

        Action methods will autoFinalize by calling Request.autoFinalize unless Request.dontAutoFinalize has been called.
        If the Controller action wants to keep the request connection to the client open, you must call dontAutoFinalize
        before returning from the action.
        @stability prototype
        @spec ejs
     */
    enumerable class Controller {
        /*  
            Define properties and functions in the ejs.web namespace so that user controller variables don't clash. 
            Override with "public" the specific properties that must be copied to views.
         */
        use default namespace module

        /*
            Actions to cache. Indexed by controller+action name plus optionally, post and query data. 
            Contains cache options for this action. Created on demand if cache() is called.
         */
        private static var _allOptions: Object = {}

        private var _afterCheckers: Array?
        private var _beforeCheckers: Array?

        /** Name of the Controller action method being run for this request */
        var actionName: String 

        /** Configuration settings. This is a reference to $ejs.web::Request.config */
        var config: Object 

//  TODO -- rename to "name"
        /** Pascal case controller name */
        var controllerName: String

        /** Logger stream - reference to $ejs.web::Request.log */
        var log: Logger

        /** Form and query parameters. This is a reference to the $ejs.web::Request.params object. */
        var params: Object

        /** Reference to the current $ejs.web::Request object */
        var request: Request?

        var cacheIndex: String
        var cacheOptions: Object
        var cacheName: String
        var cacheItem: Object

        /***************************************** Convenience Getters  ***************************************/

        /** @duplicate ejs.web::Request.absHome */
        function get absHome(): Uri?
            request ? request.absHome : null

        /** @duplicate ejs.web::Request.home */
        function get home(): Uri?
            request ? request.home : null

        /** @duplicate ejs.web::Request.pathInfo */
        function get pathInfo(): String?
            request ? request.pathInfo : null

        /** @duplicate ejs.web::Request.session */
        function get session(): Session?
            request ? request.session : null

        /** @duplicate ejs.web::Request.uri */
        function get uri(): Uri?
            request ? request.uri : null

        /********************************************* Methods *******************************************/
//  MOB - better name
        /** 
            Static factory method to create and initialize a controller. The controller class is specified by 
            params["controller"] which should be set to the controller name without the "Controller" suffix. 
            This call expects the controller class to be loaded. Called by Mvc.load().
            @param request Web request object
            @param cname Controller class name. This should be the name of the Controller class without the "Controller"
                suffix.
         */
        static function create(request: Request, cname: String? = null): Controller {
            request.params.controller = request.params.controller.toPascal()
            cname ||= (request.params.controller + "Controller")
            _initRequest = request
            if (!global[cname]) {
                throw "Cannot locate controller: '" + cname + "'"
            }
            let c: Controller = new global[cname](request)
            _initRequest = null
            return c
        }

        /** 
            Create and initialize a controller. This may be called directly or via the Controller.create factory method.
            @param req Web request object
         */
        function Controller(req: Request? = null) {
            /*  _initRequest may be set by create() to allow subclasses to omit constructors */
            controllerName = typeOf(this).trim("Controller") //MOB || "-DefaultController-"
            request = req || _initRequest
            if (request) {
                request.controller = this
                log = request.log
                params = request.params
                config = request.config
                if (request.method == "POST") {
                    before(checkSecurityToken)
                }
            }
        }

        /** 
            Run an check function after running the action
            @param fn Function callback to invoke
            @param options Check function options. 
            @option only [String|Array] Only run the checker for this action name. This can be a string action name or
                an array of action names.
            @option except [String|Array] Run the check function for all actions except this name.
                This can be a string action name or an array of action names.
         */
        function after(fn, options: Object? = null): Void {
            _afterCheckers ||= []
            _afterCheckers.append([fn, options])
        }

        private function cacheTag(name: String): String
            md5(name + Date.now + Date.ticks)

        /*
            Fetch cached data from the cache if present.
            @return a response object
         */
        private function fetchCachedResponse(): Object? {
            cacheIndex = getCacheIndex(controllerName, actionName)
            cacheOptions = _allOptions[cacheIndex]
            if (cacheOptions) {
                cacheName = getCacheName(cacheIndex, cacheOptions)
                if ((!cacheOptions.uri || cacheOptions.uri == "*" || cacheName == (cacheIndex + "::" + cacheOptions.uri)) && 
                        cacheOptions.mode != "manual") {
                    let hdr
                    if ((hdr = request.header("Cache-Control")) && (hdr.contains("max-age=0") || hdr.contains("no-cache"))) {
                        App.log.debug(5, "Cache-control header rejects use of cached content")
                    } else {
                        let item = App.cache.readObj(cacheName)
                        if (item) {
                            /*
                                Observe headers
                                If-None-Match: "ec18d-54-4d706a63"
                                If-Modified-Since: Fri, 04 Mar 2013 04:28:19 GMT
                             */
                            let status = Http.Ok
                            if ((hdr = request.header("If-None-Match")) && hdr == item.tag) {
                                /* 
                                    RFC2616 requires returning PrecondFailed, but chrome doesn't send an If-Modified-Since
                                    header and so returning PrecondFailed caused Chrome to fail.
                                 */
                                // SPEC REQUIRES THIS BUG CHROME FAILS status = Http.PrecondFailed
                                status = Http.NotModified
                            }
                            if ((hdr = request.header("If-Modified-Since"))) {
                                if (item.modified <= Date.parse(hdr)) {
                                    status = Http.NotModified
                                } else {
                                    status = Http.Ok
                                }
                            }
                            if (cacheOptions.client) {
                                setHeader("Cache-Control", cacheOptions.client, false)
                            }
                            setHeader("Last-Modified", Date(item.modified).toUTCString())
                            if (status == Http.Ok) {
                                setHeader("Etag", cacheTag(cacheName))
                                //  MOB - change this trace to just use "actionName"
                                App.log.debug(5, "Use cached: " + cacheName)
                                write(item.data)
                            } else {
                                setHeader("Etag", item.tag)
                                App.log.debug(5, "Use cached content, status: " + status + ", " + cacheName)
                            }
                            return {status: status}
                        }
                        App.log.debug(5, "No cached content for: " + cacheName)
                    }
                    request.writeBuffer = new ByteArray
                    let when = new Date
                    cacheItem = { tag: cacheTag(cacheName), modified: Math.floor(when.time / 1000) * 1000}
                    setHeader("Etag", cacheItem.tag)
                    setHeader("Last-Modified", when.toUTCString())
                    if (cacheOptions.client) {
                        setHeader("Cache-Control", cacheOptions.client, false)
                    }
                } else {
                    setHeader("Last-Modified", Date().toUTCString())
                }
            } else {
                setHeader("Last-Modified", Date().toUTCString())
            }
            return null
        }

        /*
            Save the output from the action for future requests
         */
        private function saveCachedResponse(): Void {
            if (request.finalized) {
                let etag = request.responseHeaders["Etag"] || cacheItem.tag
                cacheItem.data = request.writeBuffer
                App.cache.writeObj(cacheName, cacheItem, cacheOptions)
                App.log.debug(5, "Cache action " + cacheName + ", " + request.writeBuffer.length + " bytes")
            }
            let data = request.writeBuffer
            request.writeBuffer = null
            request.write(data)
            if (request.finalized) {
                /* Now that writeBuffer is cleared, finalize will actually finalize the request */
                request.finalize()
            }
        }

        /**
            Manually write out cached content to the client.
            This routine will write non-expired cached data to the client. Caching for actions is enabled by
            calling $cache() in the Controller.
            @return True if valid cached content was found to write to the client.
         */
        function writeCached(): Boolean {
            if (request.finalized || !cacheOptions) {
                return false
            }
            if ((!cacheOptions.uri || cacheOptions.uri == "*" || cacheName == cacheOptions.uri)) {
                let item
                if (item = App.cache.readObj(cacheName)) {
                    App.log.debug(5, "Use cached: " + cacheName)
                    setHeader("Etag", cacheTag(cacheName))
                    setHeader("Last-Modified", Date(item.modified).toUTCString())
                    if (cacheOptions.client) {
                        setHeader("Cache-Control", cacheOptions.client, false)
                    }
                    request.writeBuffer = null
                    write(item.data)
                    request.finalize()
                    return true
                }
            }
            App.log.debug(5, "no cached: " + cacheName)
            return false
        }

        //  MOB - rename
        /** 
            Controller web application. This function will run a controller action method and return a response object. 
            The action method may be specified by the $aname parameter or it may be supplied via 
            $ejs.web::Request.params.action.
            @param request Request object
            @param aname Optional action method name. If not supplied, params.action is consulted. If that is absent too, 
                "index" is used as the action method name.
            @return A response object hash {status, headers, body} or null if writing directly using the request object.
         */
        function app(request: Request, aname: String? = null): Object? {
            let response, cacheIndex, cacheName
            let ns = params.namespace || "action"

            actionName ||= aname || params.action || "index"
            params.action = actionName
            runCheckers(_beforeCheckers)

            if (!request.finalized && request.autoFinalizing) {
                if (App.config.cache.actions.enable) {
                    if (response = fetchCachedResponse()) {
                        return response
                    }
                }
                if (!(ns)::[actionName]) {
                    if (!viewExists(actionName)) {
                        response = "action"::missing()
                    }
                } else {
                    App.log.debug(4, "Run action " + actionName)
                    response = (ns)::[actionName]()
                }
                if (response && !response.body) {
                    throw "Response object is missing a \"body\""
                }
                if (!response && !request.responded && request.autoFinalizing) {
                    /* Run a default view */
                    writeView()
                }
            }
            runCheckers(_afterCheckers)
            if (!response) {
                request.autoFinalize()
            }
            if (request.writeBuffer) {
                saveCachedResponse()
            }
            return response
        }

        /** @duplicate ejs.web::Request.autoFinalize */
        function autoFinalize(): Void
            request.autoFinalize()

        /** @duplicate ejs.web::Request.autoFinalizing */
        function get autoFinalizing(): Boolean
            request.autoFinalizing

        /** 
            Run an action checker before running the action. If the checker function writes a response, the normal
            processing of the requested action will be prevented. Note that checkers do not autoFinalize so if the
            checker does write a response, it must call finalize.
            @param fn Function callback to invoke
            @param options Checker options. 
            @option only [String|Array] Only run the checker for this action name. This can be a string action name or
                an array of action names.
            @option except [String|Array] Run the checker for all actions except this name
                This can be a string action name or an array of action names.
         */
        function before(fn, options: Object? = null): Void {
            _beforeCheckers ||= []
            _beforeCheckers.append([fn, options])
        }

        /**
            Controler action caching. This caches the entire output of an action (including generated view).
            Caching is disabled/enabled via the ejsrc config.cache.actions.enable field. It is enabled by default.
            Caching may be used for any HTTP method, though typically it is most useful for state-less GET requests.
            Output data is uniquely cached for requests with different URI post data or query parameters.
            @param controller Controller class. This can be a Controller class object, "this" or a String controller name.
                You can specify "this" in static code or can also use "this" in class instance
                code and this routine will determine the underlying controller class.
            @param actions Action string or array of actions
            @param options Cache control options. Default options for all action caching can be provided via the 
                ejsrc config.cache.actions field. This is frequently used to specifiy a default lifespan for cached data.
            @option mode Client caching mode. Defaults to "server" if unset. If mode is set to "client", a Cache-Control 
                header will be sent to the client with the caching "max-age" set to the lifespan. This causes the client 
                to serve client-cached content and to not contact the server at all until the max-age expires. 
                If mode is set to "manual", the output from the action will be cached, but the action routine will 
                always be called. To use the cached content in this mode, call $writeCached() in the action method. 
                The default mode is "server" which caches content at the server for the specified lifespan. In server mode, 
                the client will cache requests, but will always revalidate the request with the server. If the server-side 
                content has not expired, a HTTP Not-Modified (304) response will be given and the client will use its 
                client-side cached content.

                Use "client" mode for static content that will rarely change and for which using "reload" in the browser
                is an adequate solution to force a refresh. Use "server" mode for dynamic content in conjunction with 
                $updateCache to expire or update cache contents. Use "manual" mode when the action routine needs to 
                determine if cached content can be used on a case by case basis.

                If a client browser clicks reload, the client cached and server cached content will be ignored and the 
                action method will be always invoked.

            @option lifespan Time in seconds for the cached output to persist.
            @option client Cache-Control header to send to the client to control caching in the client.
                Use this for explicit control of the Cache-Control header and thus control of caching in the client.
                This can be used to set a "max-age" for cached data in the client.
                These are some of the HTTP/1.1 Cache-Control keywords that can be used in the client option are:
                "max-age" Max time in seconds the resource is considered fresh.
                "s-maxage" Max time in seconds the resource is considered fresh from a shared cache.
                "public" marks authenticated responses as cacheable.
                "private" shared caches may not store the response.
                "no-cache" cache must re-submit request for validation before using cached copy.
                "no-store" response may not be stored in a cache.
                "must-revalidate" forces clients to revalidate the request with the server.
                "proxy-revalidate" similar to must-revalidate except only for proxy caches>
//  MOB - unclear, clarify
            @option uri URI and parameter to further differentiate cached content. If supplied, different cache data
                can be stored for each URI that applies to the given controller/action. If the URI is set to "*" all 
                URIs for that action/controller are uniquely cached. If the request has POST data, the URI may include
                such post data in a query format. E.g. {uri: /buy?item=scarf&quantity=1}.
            @example 
                cache(DashController, "index", {lifespan: 200})
                cache(this, ["index", "edit", "show"])
                cache(this, "index", false)
         */
        static function cache(controller, actions: Object, options: Object = {}): Void {
            let cname
            if (controller is String) {
                cname = controller.trim("Controller")
            } else if (!(controller is Type)) {
                controller = Object.getType(controller)
                cname = Object.getName(controller).trim("Controller")
            } else {
                cname = Object.getName(controller).trim("Controller")
            }
            if (!App.config.cache.actions.enable) {
                return
            }
            if (actions is String || actions is Function) {
                actions = [actions]
            }
            blend(options, App.config.cache.actions, {overwrite: false})
            if (options.mode == "client") {
                options.client ||= "max-age=" + options.lifespan
            }
            for each (name in actions) {
                let cacheIndex = getCacheIndex(cname, name)
                _allOptions[cacheIndex] = options
                if (options.lifespan is Number) {
                    let cacheName = cacheIndex
                    if (options.uri) {
                        cacheName += "::" + options.uri
                    }
                    /* Invalidate cache data when the app is reloaded */
                    App.cache.expire(cacheName, null)
                    App.cache.expire(cacheName, Date().future(options.lifespan * 1000))
                }
            }
        }

        /**
            Update the cache contents.
            This will manually update the cache contents for the given actions with the supplied data. If data is null,
            then cached content will be immediately expired.
            @param controller Controller class. This can be a Controller class object, "this" or a String controller name.
                You can specify "this" in static code or can also use "this" in class instance
                code and this routine will determine the underlying controller class.
            @param actions Action string or array of actions
            @param data Object data to cache. Data is serialized using JSON and stored in the cache. Set to null to
                invalidate/expire cached data.
            @param options Cache control options.
            @option uri URI and parameter to further differentiate cached content. If supplied, different cache data
                can be stored for each URI that applies to the given controller/action. If the URI is set to "*" all 
                URIs for that action/controller are uniquely cached. If the request has POST data, the URI may include
                such post data in a query format. E.g. {uri: /buy?item=scarf&quantity=1}
          */
        static function updateCache(controller, actions: Object, data: Object?, options: Object = {}): Void {
            let cname
            if (controller is String) {
                cname = controller.trim("Controller")
            } else if (!(controller is Type)) {
                controller = Object.getType(controller)
                cname = Object.getName(controller).trim("Controller")
            } else {
                cname = Object.getName(controller).trim("Controller")
            }
            if (!App.config.cache.actions.enable) {
                return
            }
            if (actions is String || actions is Function) {
                actions = [actions]
            }
            for each (name in actions) {
                let cacheIndex = getCacheIndex(cname, name)
                let cacheName = cacheIndex
                if (options.uri) {
                    cacheName += "::" + options.uri
                    //  MOB - this is not clearing cache data with queries
                }
                if (data == null) {
                    App.log.debug(6, "Expire " + cacheName)
                    App.cache.expire(cacheName, Date())
                } else {
                    let etag = cacheTag(cacheName)
                    App.cache.writeObj(cacheName, 
                        { tag: etag, modified: Math.floor(Date.now() / 1000) * 1000, data: data}, _allOptions[cacheIndex])
                    App.log.debug(6, "Update cache " + cacheName)
                }
            }
        }

        /** @duplicate ejs.web::Request.clearFlash */
        function clearFlash(): Void
            request.clearFlash()

        private static function getCacheIndex(cname: String, name: String = "*"): String
            "::ejs.web.action::" + cname + "::" + name

        /*
            Create a full cache key name by combining the name prefix from getCacheIndex with URI information 
            URI information is added if cache() is called with options.uri set to something
         */
        private function getCacheName(name: String, options: Object): String {
            if (options && options.uri) {
                name += "::" + request.pathInfo
                if (request.formData) {
                    name += "?" + request.formData
                }
            }
            return name
        }

        /** @duplicate ejs.web::Request.dontAutoFinalize */
        function dontAutoFinalize(): Void
            request.dontAutoFinalize()

        /** 
            @duplicate ejs.web::Request.error
         */
        function error(msg: String): Void
            request.error(msg)

        /** @duplicate ejs.web::Request.finalize */
        function finalize(): Void
            request.finalize()

        /** @duplicate ejs.web::Request.finalized */
        function get finalized(): Boolean
            request.finalized

        /** 
            @duplicate ejs.web::Request.flash
         */
        function get flash(): Object
            request.flash

        /** @duplicate ejs.web::Request.flush */
        function flush(dir: Number = Stream.WRITE): Void
            request.flush(dir)

        /** 
            @duplicate ejs.web::Request.header
         */
        function header(key: String): String
            request.header(key)

        /** @duplicate ejs.web::Request.inform */
        function inform(msg: String): Void
            request.inform(msg)

        /** @duplicate ejs.web::Request.link */
        function link(target: Object): Uri
            request.link(target)

        /** 
            Missing action method. This method will be called if the requested action routine does not exist.
            It should be overridden in user controller classes by using the "override" keyword.
         */
        action function missing() {
            throw "Missing Action: " + params.action + ": could not be found for controller: " + controllerName
            return ""
        }

        /** @duplicate ejs.web::Request.notify */
        function notify(key: String, msg: String): Void
            request.notify(key, msg)

        /** @duplicate ejs.web::Request.on */
        function on(name, observer: Function): Controller {
            request.on(name, observer)
            return this
        }

        /** @duplicate ejs.web::Request.read */
        function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?
            request.read(buffer, offset, count)

        /** 
            Redirect the client to the given URL
            @param where Url to redirect the client toward. This can be a relative or absolute string URL or it can be
                a hash of URL components. For example, the following are valid inputs: "../index.ejs", 
                "http://www.example.com/home.html", {action: "list"}.
            @param status Http status code to use in the redirection response. Defaults to 302.
         */
        function redirect(where: Object, status: Number = Http.MovedTemporarily): Void
            request.redirect(where, status)

        /** 
            Remove all defined checkers on the Controller.
         */
        function removeCheckers(): Void {
            _beforeCheckers = null
            _afterCheckers = null
        }

        /** @duplicate ejs.web::Request.setHeader */
        function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /** @duplicate ejs.web::Request.setHeaders */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void
            request.setHeaders(headers, overwrite)

        /** @duplicate ejs.web::Request.setStatus */
        function setStatus(status: Number): Void
            request.status = status

        /** 
            Low-level write data to the client. This will buffer the written data until either flush() or finalize() 
            is called.
            If an action method does call a write data back to the client and has not called finalize() or 
            dontAutoFinalize(), a default view template will be generated when the action method returns. 
            @param args Arguments to write to the client. The args are converted to strings.
            @return The number of bytes written to the client
         */
        function write(...args): Number
            request.write(...args)

        /** 
            @duplicate ejs.web::Request.warn
         */
        function warn(msg: String): Void
            request.warn(msg)

        /**
            @duplicate ejs.web::Request.writeContent
         */
        function writeContent(data): Void
            request.writeContent(data)

        /**
            Render an error message as the response.
            This call sets the response status and writes a HTML error message with the given arguments back to the client.
            @param status Http response status code
            @param msgs Error messages to send with the response
         */
        function writeError(status: Number, ...msgs): Void
            request.writeError(status, ...msgs)

        /** 
            Render file content back to the client.
            This call writes the given file contents back to the client.
            @param filename Path to the filename to send to the client
         */
        function writeFile(filename: Path): Void
            request.writeFile(filename)

        /** 
            Render a partial response using template file.
            @param path Path to the template to render to the client
            @param options Additional options.
            @option layout Optional layout template. Defaults to config.dirs.layouts/default.ejs.
         */
        function writePartialTemplate(path: Path, options: Object = {}): Void { 
            request.filename = path
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = Path(config.dirs.layouts).join(config.web.views.layout)
            }
            log.debug(4, "writePartialTemplate: \"" + path + "\"")
            request.server.process(TemplateBuilder(request, options), request, false)
        }

        /** 
            Render a view template.
            This call writes the result of running the view template file back to the client.
            @param viewName Name of the view to render to the client. The view template filename will be constructed by 
                joining the views directory with the controller name and view name. E.g. views/Controller/list.ejs.
            @param options Additional options.
            @option controller Optional controller for the view.
            @option layout Optional layout template. Defaults to config.dirs.layouts/default.ejs.
         */
        function writeView(viewName = null, options: Object = {}): Void {
            let controller = options.controller || controllerName
            viewName ||= options.action || actionName
            if (options.layout === undefined) {
                options.layout = config.dirs.layouts.join(config.web.views.layout)
            }
            writeTemplate(config.dirs.views.join(controller, viewName).joinExt(config.extensions.ejs), options)
        }

        /** 
            Render a view template from a path.
            This call writes the result of running the view template file back to the client.
            @param path Path to the view template to render and write to the client.
            @param options Additional options.
            @option layout Optional layout template. Defaults to config.dirs.layouts/default.ejs.
         */
        function writeTemplate(path: Path, options: Object = {}): Void {
            log.debug(4, "writeTemplate: \"" + path + "\"")
            let saveFilename = request.filename
            request.filename = path
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = config.dirs.layouts.join(config.web.views.layout)
            }
            request.server.process(TemplateBuilder(request, options), request, false)
            request.filename = saveFilename
        }

        /** 
            Render a view template from a string literal.
            This call writes the result of running the view template file back to the client.
            @param page String literal containing the view template to render and write to the client.
            @param options Additional options.
            @option layout Path layout template. Defaults to config.dirs.layouts/default.ejs.
         */
        function writeTemplateLiteral(page: String, options: Object = {}): Void {
            log.debug(4, "writeTemplateLiteral")
            request.setHeader("Content-Type", "text/html")
            if (options.layout === undefined) {
                options.layout = config.dirs.layouts.join(config.web.views.layout)
            }
            options.literal = page
            request.server.process(TemplateBuilder(request, options), request, false)
        }

        /**************************************** Private ******************************************/

        private function checkSecurityToken()
            request.checkSecurityToken()

        /* 
            Run the before/after checkers. These are typically used to handle authorization and similar tasks
         */
        private function runCheckers(checkers: Array?): Void {
            for each (checker in checkers) {
                let [fn, options] = checker
                if (options) {
                    if (only = options.only) {
                        if ((only is String && actionName != only) || (only is Array && !only.contains(actionName))) {
                            continue
                        }
                    } 
                    if (except = options.except) {
                        if ((except is String && actionName == except) || (except is Array && except.contains(actionName))) {
                            continue
                        }
                    }
                }
                fn.call(this)
            }
        }

        private function viewExists(name: String): Boolean? {
            let viewClass = controllerName + "_" + actionName + "View"
            if (global[viewClass]) {
                return true
            }
            let path = config.dirs.views.join(controllerName, name).joinExt(config.extensions.ejs)
            if (path.exists) {
                return true
            }
            return null
        }

        /********************************************  LEGACY 1.0.2 ****************************************/

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get absUrl()
            absHome

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function afterFilter(fn, options: Object? = null): Void
            after(fn, options)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get appUrl()
            home.trimEnd("/")

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function beforeFilter(fn, options: Object? = null): Void
            before(fn, options)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function createSession(timeout: Number): Void
            request.createSession(timeout)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function destroySession(): Void
            request.destroySession()

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function discardOutput(): Void {
            //  No supported
            true
        }
            
        /**
            escapeHtml, html is now a global in Utils.es
         */

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get host()
            request.server

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function keepAlive(on: Boolean): Void {
            // Not supported 
            true
        }

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function loadView(path): Void
            writeTemplate(path)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function makeUrl(action: String, id: String? = null, options: Object = {}, query: Object? = null): String
            link({ action, id, query })

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function redirectUrl(uri: String, status: Number = 302): Void
            redirect(uri, status)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function render(...args): Void
            write(...args)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderError(status, ...msgs): Void
            writeError(status, ...msgs)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderFile(filename: String): Void
            writeFile(filename)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderRaw(...args): Void
            write(...args)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function renderView(name: String? = null): Void
            writeView(name)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function reportError(status: Number, msg: String, e: Object? = null): Void
            writeError(status, msg + e)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function resetFilters(): Void
            removeCheckers()

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function sendError(status, ...msgs): Void
            writeError(status, ...msgs)

        /**
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function setCookie(name: String, value: String, path: String? = null, domain: String? = null,
                lifetime: Number = 0, secure: Boolean = false): Void  {
            request.setCookie(name, 
                { value: value, path: path, domain: domain, lifetime: Date().future(lifetime * 1000), secure: secure})
        }

    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Dir.es"
 */
/************************************************************************/

/*
    Dir.es - Directory content handler
 */

# Config.WEB
module ejs.web {

    /** 
        Directory content handler. This redirects requests for directories and serves directory index files.
        If the request pathInfo ends with "/", the request is transparently redirected to an index file if one is present.
        The set of index files is defined by HttpServer.indicies. If the request is a directory but does not end in "/",
        the client is redirected to a URL equal to the pathInfo with a "/" appended.
        @param request Request object
        @returns A response hash object
        @spec ejs
        @stability prototype
     */
    function DirApp(request: Request): Object {
        if (request.pathInfo.endsWith("/")) {
            for each (index in request.server.indicies) {
                let path = request.filename.join(index)
                if (path.exists) {
                    /* Return a String containing the new pathInfo to serve */
                    request.pathInfo += index 
                    let route = request.route.router.route(request)
                    return request.server.process(route.response, request)
                }
            }
            return { 
                status: Http.NotFound, 
                body: errorBody("Not Found", "Cannot locate " + escapeHtml(request.pathInfo))
            }
        }
        return { 
            status: Http.MovedPermanently,
            headers: { location: request.uri + "/" }
            body: errorBody("Moved Permanently", 'The document has moved <a href="' + request.pathInfo + "/" + '">here</a>.')
        }
    }

    /** 
        Directory builder for use in routing tables to service requests for directories. 
        @param request Request object. 
        @return A web application function that services a web request and when invoked with the request object will 
        yield a response object.
        @example:
          { name: "index", builder: DirBuilder, match: Router.isDir }
        @spec ejs
        @stability prototype
        UNUSED
    function DirBuilder(request: Request): Function {
        return DirApp
    }
     */
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Google.es"
 */
/************************************************************************/

/**
   GoogleConnector.es -- View connector for the Google Visualization library
 */

# Config.WEB
module ejs.web {

    /**
        @hide
     */
    # FUTURE
    class GoogleConnector {

        use default namespace module

        function GoogleConnector(controller) {
            // this.controller = controller
        }

//  TODO -- state should be stored in View.viewState. This should be stateless
//  TODO -- all methods must become static
        private var nextId: Number = 0

        private function scriptHeader(kind: String, id: String): Void {
            write('<script type="text/javascript" src="http://www.google.com/jsapi"></script>')
            write('<script type="text/javascript">')
            write('  google.load("visualization", "1", {packages:["' + kind + '"]});')
            write('  google.setOnLoadCallback(' + 'draw_' + id + ');')
        }

        //  TODO - must support Ejs options: bands, columns, data, onClick, refresh, pageSize, pivot, 
        //      sortColumn, sortOrder, style, styleHead, styleEvenRow, styleOddRow, styleCell, visible, widths
        //      Support @options
        function table(data, options: Object): Void {
            var id: String = "GoogleTable_" + nextId++

            if (data == null || data.length == 0) {
                write("<p>No Data</p>")
                return
            }
            let columns: Array = options["columns"]

            scriptHeader("table", id)
            
            write('  function ' + 'draw_' + id + '() {')
            write('    var data = new google.visualization.DataTable();')

            let firstLine: Object = data[0]
            if (columns) {
                if (columns[0] != "id") {
                    columns.insert(0, "id")
                }
                for (let i = 0; i < columns.length; ) {
                    if (firstLine[columns[i]]) {
                        i++
                    } else {
                        columns.remove(i, i)
                    }
                }
            } else {
                columns = []
                for (let name in firstLine) {
                    columns.append(name)
                }
            }

            for each (name in columns) {
                write('    data.addColumn("string", "' + name.toPascal() + '");')
            }
            write('    data.addRows(' + data.length + ');')

            for (let row: Object in data) {
                let col: Number = 0
                for each (name in columns) {
                    write('    data.setValue(' + row + ', ' + col + ', "' + data[row][name] + '");')
                    col++
                }
            }

            write('    var table = new google.visualization.Table(document.getElementById("' + id + '"));')

            let goptions = getOptions(options, { 
                height: null, 
                page: null,
                pageSize: null,
                showRowNumber: null,
                sort: null,
                title: null,
                width: null, 
            })

            write('    table.draw(data, ' + serialize(goptions) + ');')

            if (options.click) {
                write('    google.visualization.events.on(table, "select", function() {')
                write('        var row = table.getSelection()[0].row;')
                write('        window.location = "' + view.makeUrl(options.click, "", options) + '?id=" + ' + 
                    'data.getValue(row, 0);')
                write('    });')
            }

            write('  }')
            write('</script>')

            write('<div id="' + id + '"></div>')
        }

        //  TODO - must support Ejs options: columns, kind, onClick, refresh, style, visible
        //  TODO - use @options

        function chart(grid: Array?, options: Object): Void {
            var id: String = "GoogleChart_" + nextId++

            if (grid == null || grid.length == 0) {
                write("<p>No Data</p>")
                return
            }

            let columns: Array = options["columns"]

            scriptHeader("piechart", id)
            
            write('  function ' + 'draw_' + id + '() {')
            write('    var data = new google.visualization.DataTable();')

            let firstLine: Object = grid[0]
            let col: Number = 0
            //  TODO - need to get data types
            let dataType: String = "string"
            for (let name: String in firstLine) {
                if  (columns && columns.contains(name)) {
                    write('    data.addColumn("' + dataType + '", "' + name.toPascal() + '");')
                    col++
                    if (col >= 2) {
                        break
                    }
                    dataType = "number"
                }
            }
            write('    data.addRows(' + grid.length + ');')

            for (let row: Object in grid) {
                let col2: Number = 0
                //  TODO - workaround
                for (let name2: String in grid[row]) {
                    if  (columns && columns.contains(name2)) {
                        if (col2 == 0) {
                            write('    data.setValue(' + row + ', ' + col2 + ', "' + grid[row][name2] + '");')
                        } else if (col2 == 1) {
                            write('    data.setValue(' + row + ', ' + col2 + ', ' + grid[row][name2] + ');')
                        }
                        //  else break. TODO should do this but it is bugged
                        col2++
                    }
                }
            }

            //  PieChart, Table
            write('    var chart = new google.visualization.PieChart(document.getElementById("' + id + '"));')

            let goptions = getOptions(options, { width: 400, height: 400, is3D: true, title: null })
            write('    chart.draw(data, ' + serialize(goptions) + ');')

            write('  }')
            write('</script>')

            write('<div id="' + id + '"></div>')
        }

        private function getOptions(options: Object, defaults: Object): Object {
            var result: Object = {}
            for (let word: String in defaults) {
                if (options[word]) {
                    result[word] = options[word]
                } else if (defaults[word]) {
                    result[word] = defaults[word]
                }
            }
            return result
        }

        private function write(str: String): Void {
            view.write(str)
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Head.es"
 */
/************************************************************************/

/**
    Head.es - Respond to HEAD requesets and Return just the headers and omit the body.
    NOTE: This is typically done by good web servers anyway.
    Usage:
        require ejs.web
        exports.app = Head(app)
 */

# Config.WEB
module ejs.web {
    /** 
        Head wrapper middleware. Return the headers and omit the body for HTTP HEAD requests. HEAD requests should still
        preserve the original Content-Length header value even though they transmit no body content.
        This version is limited, in that it will only define a Content-Length if the response body is a string. 
        @param app Application servicing the request and generating the response.
        @return A web application function that when invoked with the request will yield a response object.
        @example:
            export.app = Head(app)
     */
    function Head(app: Function) {
        return function(request) {
            var response = app(request)
            if (request.method == "HEAD") {
                if (response.body is String) {
                    let length = response.body.length
                    response.headers ||= {}
                    blend(response.headers, {"Content-Length": length})
                }
                response.body = null
            }
            return response
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Html.es"
 */
/************************************************************************/

/**
   Html.es -- HtmlViewConnector. This provides HTML view support.
 */

# Config.WEB
module ejs.web {

    /**
        The Html Connector provides bare HTML encoding of View controls.

        TODO Style conventions???
            -ejs- prefixes all internal styles
            -ejs-alert
            -ejs-flash
            -ejs-flash-inform
            -ejs-flash-warn
            -ejs-flash-error
            -ejs-hidden
            -ejs-tabs
            -ejs-field-error
            -ejs-progress
            -ejs-progress-inner
            -ejs-table
            -ejs-table-download
            -ejs-even-row
            -ejs-field-error
            -ejs-form-error
        @stability prototype
        @spec ejs
        @hide
     */
    public class HtmlViewConnector {

        use default namespace module

        private var request: Request
        private var view: View

        /* Sequential DOM ID generator */
        private static var lastDomID: Number = 0

        /*
            Mapping of helper options to HTML attributes.
            NOTE: data-*, click and remote are handled specially in getAttributes.
         */
        private static const htmlOptions: Object = { 
            "apply":                "data-apply",
            "background":           "background",
            "class":                "class",
            "color":                "color",
            "colour":               "color",
            "domid":                "id",
            "effects":              "data-effects",
            "height":               "height",
            "method":               "data-method",
            "modal":                "data-modal",
            "period":               "data-refresh-period",
            "pivot":                "data-pivot",
            "rel":                  "rel",
            "size":                 "size",
            "sort":                 "data-sort",
            "sortOrder":            "data-sort-order",
            "style":                "class",
            "width":                "width",
        }

        //  MOB -- need to be able to control treeview or not
        private static const defaultStylesheets = [
            "/layout.css", 
            "/themes/default.css", 
            "/js/treeview.css", 
        ]

        //  MOB -- Should offer all-in-one script
        private static const defaultScripts = [
            "/js/jquery", 
            "/js/jquery.tablesorter",
            "/js/jquery.simplemodal",
            "/js/jquery.treeview",
            "/js/jquery.ejs",
        ]

        function HtmlViewConnector(view) {
            this.view = view
            this.request = view.request
        }

        function alert(text: String, options: Object): Void {
            options.style = append(options.style, "-ejs-alert")
            write('<div' + getAttributes(options) + '>' +  text + '</div>\r\n')
        }

        function anchor(text: String, options: Object): Void {
            let att = getAttributes(options, {"data-click": true})
            write('<a href="' + options.data-click + '"' + att + '>' + text + '</a>\r\n')
        }

        function button(name: String, value: String, options: Object): Void {
            write('    <input name="' + name + '" type="submit" value="' + value + '"' + getAttributes(options) + ' />\r\n')
        }

        function buttonLink(text: String, options: Object): Void {
            let attributes = getAttributes(options)
            write('<button' + attributes + '>' + text + '</button>\r\n')
        }

        function chart(data: Array, options: Object): Void {
            //  TODO
            throw 'HtmlConnector control "chart" not implemented.'
        }

        function checkbox(field: String, value: Object, checkedValue: Object, options: Object): Void {
            let checked = (value == checkedValue) ? ' checked="yes"' : ''
            write('    <input name="' + field + '" type="checkbox"' + getAttributes(options) + checked + 
                ' value="' + checkedValue + '" />\r\n')
            write('    <input name="' + field + '" type="hidden"' + getAttributes(options) + ' value="" />\r\n')
        }

        function div(body: String, options: Object): Void {
            write('<div' + getAttributes(options) + '>' +  body + '</div>\r\n')
        }

        function endform(): Void {
            write('</form>\r\n')
        }

        function flash(kind: String, msg: String, options: Object): Void {
            options.style = append(options.style, "-ejs-flash -ejs-flash-" + kind)
            write('<div' + getAttributes(options) + '>' + msg + '</div>\r\n')
        }

        function form(record: Object, options: Object): Void {
            let method ||= options.method || ((record && options.id) ? "PUT" : "POST")
            options.action ||= options.click
            options.action ||= ((record && options.id) ? "@update" : "@create")
            if (method != "GET" && method != "POST") {
                options.method = method
                method = "POST"
            }
            let uri = request.link(options)
            emitFormErrors(record, options)
            let attributes = getAttributes(options, {action: true, "data-click": true})
            write('<form method="' + method + '" action="' + uri + '"' + attributes + '>\r\n')
            if (options.id != undefined) {
                write('    <input name="id" type="hidden" value="' + options.id + '" />\r\n')
            }
            if (!options.insecure) {
                let token = options.securityToken || request.securityToken
                write('    <input name="' + Request.SecurityTokenName + '" type="hidden" value="' + token + '" />\r\n')
            }
        }

        function icon(uri: String, options: Object): Void {
            //  MOB - what other places should support use request.link
            uri = request.link(uri)
            write('    <link href="' + uri + '" rel="shortcut icon" />\r\n')
        }

        function image(src: String, options: Object): Void {
            src = request.link(src)
            write('<img src="' + src + '"' + getAttributes(options) + '/>\r\n')
        }

        function label(text: String, options: Object): Void {
            write('<span' + getAttributes(options) + '>' +  text + '</span>\r\n')
        }

        function list(name: String, choices: Object, defaultValue: String?, options: Object): Void {
            let selected
            write('    <select name="' + name + '" ' + getAttributes(options) + '>\r\n')
            if (choices is Array) {
                let i = 0
                for each (choice in choices) {
                    if (choice is Array) {
                        /* list("priority", [["low", "3"], ["med", "5"], ["high", "9"]]) */
                        /* Note: value is first */
                        let [value, key] = choice
                        selected = (key == defaultValue) ? ' selected="yes"' : ''
                        write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')

                    } else if (choice && choice.id) {
                        /* list("priority", [{id: 77, field: "value", ...}, ...]) */
                        selected = (choice.id == defaultValue) ? ' selected="yes"' : ''
                        for (let [key, value] in choice) {
                            if (key != "id" && !(value is Function)) {
                                write('      <option value="' + choice.id + '"' + selected + '>' + value + '</option>\r\n')
                                break
                            }
                        }
                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* list("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [value, key] in choice) {
                            selected = (key == defaultValue) ? ' selected="yes"' : ''
                            write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')
                        }
                    } else {
                        /* list("priority", ["low", "med", "high"]) */
                        selected = (choice == defaultValue) ? ' selected="yes"' : ''
                        write('      <option value="' + i + '"' + selected + '>' + choice + '</option>\r\n')
                    }
                    i++
                }
            } else {
                /* list("priority", {low: 0, med: 1, high: 2}) */
                for (let [value, key]  in choices) {
                    selected = (key == defaultValue) ? ' selected="yes"' : ''
                    write('      <option value="' + key + '"' + selected + '>' + value + '</option>\r\n')
                }
            }
            write('    </select>\r\n')
        }

        function mail(name: String, address: String, options: Object): Void  {
            write('<a href="mailto:' + address + '"' + getAttributes(options) + '>' + name + '</a>\r\n')
        }

        function progress(data: Number, options: Object): Void {
            options["data-progress"] = data
            write('<div class="-ejs-progress">\r\n    <div class="-ejs-progress-inner"' + getAttributes(options) + 
                '>' + data + '%</div>\r\n</div>>\r\n')
        }

        function radio(name: String, actual: String, choices: Object, options: Object): Void {
            let checked: String
            let attributes = getAttributes(options)
            if (choices is Array) {
                for each (choice in choices) {
                    if (choice is Array) {
                        /* radio("priority", [["3", "low"], ["5", "med"], ["9", "high"]]) */
                        let [key, value] = choice
                        checked = (value == actual) ? " checked" : ""
                        write('    ' + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + 
                            ' value="' + value + '"' + checked + ' />\r\n')

                    } else if (Object.getOwnPropertyCount(choice) > 0) {
                        /* radio("priority", [{low: 3}, {med: 5}, {high: 9}]) */
                        for (let [key, value] in choice) {
                            checked = (value == actual) ? " checked" : ""
                            write('  ' + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + 
                                ' value="' + value + '"' + checked + ' />\r\n')
                        }

                    } else {
                        /* radio("priority", ["low", "med", "high"]) */
                        checked = (choice == actual) ? " checked" : ""
                        write("    " + choice.toPascal() + 
                            ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                            choice + '"' + checked + ' />\r\n')
                    }
                }
            } else {
                /* radio("priority", {low: 0, med: 1, high: 2}) */
                for (let [key, value] in choices) {
                    checked = (value == actual) ? " checked" : ""
                    write("    " + key.toPascal() + ' <input type="radio" name="' + name + '"' + attributes + ' value="' + 
                        value + '"' + checked + ' />\r\n')
                }
            }
        }

        function refresh(on: String, off, options: Object): Void {
            write('    <img src="' + on + '" data-on="' + on + '" data-off="' + off + '" class="-ejs-refresh" />')
        }

        function script(uri: String?, options: Object): Void {
            if (uri == null) {
                if (options.minified == undefined) {
                    options.minified = true
                }
                /* MVC directory */
                let dirs = request.config.dirs
                let sdir = (dirs && dirs.static) ? dirs.static.basename : "static"
                for each (uri in defaultScripts) {
                    uri = request.link("/" + sdir + uri)
                    uri += (options.minified) ? ".min.js" : ".js"
                    write('    <script src="' + uri + '" type="text/javascript"></script>\r\n')
                }
            } else {
                //MOB uri = request.link(uri)
                write('    <script src="' + uri + '" type="text/javascript"></script>\r\n')
            }
        }

        function securityToken(options: Object): Void {
            write('    <meta name="SecurityTokenName" content="' + Request.SecurityTokenName + '" />\r\n')
            write('    <meta name="' + Request.SecurityTokenName + '" content="' + request.securityToken + '" />\r\n')
        }

        function stylesheet(uri: String?, options: Object): Void {
            if (uri == null) {
                /* MVC directory */
                let dirs = request.config.dirs
                let sdir = (dirs && dirs.static) ? dirs.static.basename : "static"
                for each (uri in defaultStylesheets) {
                    uri = request.link("/" + sdir + uri)
                    write('    <link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
                }
            } else {
                write('    <link rel="stylesheet" type="text/css" href="' + uri + '" />\r\n')
            }
        }

        function table(data, options: Object): Void {
            if (!data is Array) {
                data = [data]
            }
            if (data.length == 0) {
                write("<p>No Data</p>\n")
                return
            }
            options.style = append(options.style, "-ejs-table")
    /*
            let attributes = getAttributes({
                apply: options.apply,
                period: options.period,
                refresh: options.refresh,
                sort: options.sort,
                sortOrder: options.sortOrder || "ascending",
                style: options.style,
            })
    */
            let attributes = getAttributes(options, {"data-click": true})
            let columns = getColumns(data, options)

            write('  <table' + attributes + '>\r\n')

            /*
                Table title header and column headings
             */
            if (options.showHeader != false) {
                write('    <thead>\r\n')
                if (options.title) {
                    let length = Object.getOwnPropertyCount(columns)
                    write('        <tr class="-ejs-table-title"><td colspan="' + length + '">' + 
                        options.title + '</td></tr>\r\n')
                }
                write('        <tr class="-ejs-table-header">\r\n')
                for (let [name, column] in columns) {
                    if (name == null) continue
                    let header = (column.header) ? (column.header) : name.toPascal()
                    let width = (column.width) ? ' width="' + column.width + '"' : ''
                    write('            <th' + width + '>' + header + '</th>\r\n')
                }
                write("        </tr>\r\n    </thead>\r\n")
            }
            write('    <tbody>\r\n')

            /*
                Render each row
             */
            let row = 0
            for each (let r: Object in data) {
                let values = {}
                for (name in columns) {
                    values[name] = view.getValue(r, name, options)
                }
                let rowOptions = {
                    click: options.click,
                    edit: options.edit,
                    id: r.id,
                    key: options.key,
                    params: options.params,
                    remote: options.remote,
                }
                let styleRow = ""
                if (options.styleRows && options.styleRows[row]) {
                    styleRow = ' class="' + options.styleRows[row] + '"'
                }
                //  MOB - why not set these above when defining rowOptions
                rowOptions.record = r
                rowOptions.field = null
                rowOptions.values = values
                let att = getAttributes(rowOptions)
                write('        <tr' + att + styleRow + '>\r\n')

                let col = 0
                for (let [name, column] in columns) {
                    let value = values[name]
                    let styleCell: String = ""
                    if (options.styleColumns) {
                        styleCell = append(styleCell, options.styleColumns[col])
                    }
                    if (column.style) {
                        if (column.style is Array) {
                            styleCell = append(styleCell, column.style[r])
                        } else {
                            styleCell = append(styleCell, column.style)
                        }
                    }
                    if (options.styleCells && options.styleCells[row]) {
                        styleCell = append(styleCell, options.styleCells[row][col])
                    }
                    let attr = ""
                    if (styleCell) {
                        attr = ' class="' + styleCell + '"'
                    }
                    if (column.align) {
                        attr = append(attr, ' align="' + column.align + '"')
                    } else if (value is Number) {
                        attr = append(attr, ' align="right"')
                    }
                    if (column.click || column.edit /* MOB options.cell */) {
                        /* really cell options */
                        rowOptions.record = r
                        rowOptions.row = row
                        rowOptions.field = name
                        rowOptions.values = values
                        rowOptions.key = column.key
                        rowOptions.click = column.click
                        rowOptions.edit = column.edit
                        attr = append(attr, getAttributes(rowOptions))
                    }
                    value = view.formatValue(value, { formatter: column.formatter} )
                    write('            <td' + attr + '>' + value + '</td>\r\n')
                    col++
                }
                row++
                write('        </tr>\r\n')
            }
            write('    </tbody>\r\n</table>\r\n')
        }

        function tabs(data: Object, options: Object): Void {
            let attributes = getAttributes(options, {"data-remote": true})
            let att
            if (options.toggle) {
                att = "data-toggle"
            } else if (options.remote) {
                att = "data-remote"
            } else {
                att = "data-click"
            }
            write('<div class="-ejs-tabs"' + attributes + '>\r\n    <ul>\r\n')
            if (data is Array) {
                for each (tuple in data) {
                    for (let [name, target] in tuple) {
                        let uri = (att == "data-toggle") ? target : request.link(target)
                        write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                    }
                }
            } else {
                for (let [name, target] in data) {
                    let uri = (att == "data-toggle") ? target : request.link(target)
                    write('      <li ' + att + '="' + uri + '">' + name + '</li>\r\n')
                }
            }
            write('    </ul>\r\n</div>\r\n')
        }

        function text(field: String, value: String, options: Object): Void {
            if (options.rows) {
                let cols = options.size
                if (cols == undefined) {
                    cols = 60
                }
                let rows = options.rows
                if (rows == undefined) {
                    rows = 10
                }
                att = getAttributes(options, {size: true})
                write('    <textarea name="' + field + '" type="' + getTextKind(options) + '"' + att + 
                    ' cols="' + cols + '" rows="' + rows + '">' + value + '</textarea>\r\n')
            } else {
                write('    <input name="' + field + '"' + getAttributes(options) + ' type="' + getTextKind(options) + 
                    '" value="' + value + '" />\r\n')
            }
        }

        function tree(data: Object, options: Object): Void {
            options.style = append(options.style, "-ejs-tree")
            let attributes = getAttributes(options)
            let columns = getColumns(data, options)

            write('<div' + attributes + '>\r\n')
            write(serialize(data, {pretty: true}))
            write('</div>\r\n')
        }

        /************************************************** Support ***************************************************/

        private function emitFormErrors(record, options): Void {
            if (!record || !record.getErrors || options.hideErrors) {
                return
            }
            let errors = record.getErrors()
            if (errors) {
                let count = Object.getOwnPropertyCount(errors)
                write('<div class="-ejs-form-error"><h2>The ' + typeOf(record).toLowerCase() + ' has ' + 
                    count + (count > 1 ? ' errors' : ' error') + ' that ' +
                    ((count > 1) ? 'prevent' : 'prevents') + '  it being saved.</h2>\r\n')
                write('    <p>There were problems with the following fields:</p>\r\n')
                write('    <ul>\r\n')
                for (e in errors) {
                    write('        <li>' + e.toPascal() + ' ' + errors[e] + '</li>\r\n')
                }
                write('    </ul>\r\n')
                write('</div>\r\n')
            }
        }

/*
        //  FUTURE -- never called. -- better to push to client via data-filter
        //  TODO - this actually modifies the grid. Need to doc this.
        private function filter(data: Array): Array {
            data = data.clone()
            pattern = request.params.filter.toLowerCase()
            for (let i = 0; i < data.length; i++) {
                let found: Boolean = false
                for each (f in data[i]) {
                    if (f.toString().toLowerCase().indexOf(pattern) >= 0) {
                        found = true
                    }
                }
                if (!found) {
                    data.remove(i, i)
                    i--
                }
            }
            return data
        }
*/

        /*
            Set the template key fields based on the record data
            This defines target[property] = record[property]. The property name is defined in "keyFields"
            Key fields can be an array of property names e.g. ["name", "id"], or they can provide mapping:
            e.g. ["name", {buildId: "id"}]
        */
        private function setKeyFields(target: Object, keyFields: Array, options: Object): Void {
            let record = options.record
            let row = options.row
            let values = options.values
            for (name in record) {
                // Add missing values incase columns are not being displayed 
                values[name] ||= record[name]
            }
            for each (key in keyFields) {
                if (key is String) {
                    // Array of key names corresponding to the columns 
                    let value = (values[key] != null) ? Uri.encodeComponent(values[key]) : row
                    target[key] = value
                } else {
                    // Hash of key:mapped names corresponding to the columns
                    for (field in key) {
                        let value = (values[field] != null) ? Uri.encodeComponent(values[field]) : row
                        target[key[field]] = value
                    }
                }
            }
        }

        /**
            Map options to a HTML attribute string. See htmlOptions and $View for a discussion on standard options.
            @param options Control options
            @returns a string containing the HTML attributes to emit. Will return an empty string or a string with a 
                leading space (and not trailing space)
         */
        function getAttributes(options: Object, exclude: Object? = null): String {
            if (options.hasError) {
                options.style = append(options.style, "-ejs-field-error")
            }
            if (options.click) {
                setLinkOptions(options.click, options, "data-click")

            } else if (options.remote) {
                //  MOB cleanup - data-remote only ever allow a boolean value.
                if (options.remote == true && options.action) {
                    options.remote = options.action
                }
                setLinkOptions(options.remote, options, "data-remote")

            } else if (options.edit) {
                setLinkOptions(options.edit, options, "data-edit")

            }
            if (options.refresh) {
                options.domid ||= getNextID()
                setLinkOptions(options.refresh, options, "data-refresh")
            }
            return mapAttributes(options, exclude)
        }

        private function getColumns(data: Object, options: Object): Object {
            let columns
            if (options.columns) {
                if (options.columns is Array) {
                    columns = {}
                    for each (name in options.columns) {
                        columns[name] = name
                    }
                } else {
                    columns = options.columns
                }
            } else {
                /*
                    No supplied columns, so infer from data.
                 */
                columns = {}
                if (data is Array) {
                    for (let name in data[0]) {
                        if (name == "id" && !options.showId) continue
                        columns[name] = name
                    }
                }
            }
            return columns
        }
    
        private function getTextKind(options): String {
            var kind: String
            if (options.password) {
                kind = "password"
            } else if (options.hidden) {
                kind = "hidden"
            } else {
                kind = "text"
            }
            return kind
        }

        /*
            Get the next usable DOM ID for view controls
         */
        private function getNextID(): String
            "id_" + lastDomID++

        /*
            Map options to an HTML attribute string
         */
        private function mapAttributes(options: Object, exclude: Object? = null): String {
            let result: String = ""
            if (options.method) {
                options.method = options.method.toUpperCase();
            }
            for (let [key, value] in options) {
                if (exclude && exclude[key]) continue
                if (value != undefined) {
                    if (htmlOptions[key] || key.startsWith("data-")) {
                        let mapped = htmlOptions[key] ? htmlOptions[key] : key
                        result += mapped + '="' + value + '" '
                    } else if (key == "pass") {
                        result += value + ' '
                    }
                }
            }
            return (result == "") ? result : (" " + result.trimEnd())
        }

//  MOB
        /*
            Set option fields for a target URI:
                options[prefix] == uri
                options[prefix-method]
                options[prefix-params]
            @param target The URI target
            @param options The options object hash
            @param prefix The property prefix to use in options
         */
        private function setLinkOptions(target: Object, options: Object, prefix: String) {
            if (typeOf(target) != "Object") {
                target = target.toString()
                if (target[0] == '@') {
                    target = {action: target}
                } else {
                    /* Non-mvc URI string */
                    target = {uri: (target[0] == '/') ? (request.scriptName + target) : target}
                }
            } else {
                target = target.clone()
            }
            blend(target, options, {overwrite: false})
            if (options.key && options.record) {
                /* Set template key fields */
                setKeyFields(target, options.key, options)
            }
            target.uri ||= request.link(target)
            if (target.params) {
//  MOB - should be a utility routine
                /* Process params and convert to an encoded query string */
                let list = []
                for (let [k,v] in target.params) {
                    list.push(Uri.encodeComponent(k) + "=" + Uri.encodeComponent(v))
                }
                target.params = list.join("&")
            }
            options[prefix] = target.uri
            if (target.method) {
                options[prefix + "-" + "method"] = target.method
                delete options.method
            }
            if (target.params) {
                options[prefix + "-" + "params"] = target.params
            }
        }

        private function write(str: String): Void
            request.write(str)

        //  TODO OPT
        private function append(str: String?, suffix: String?): String {
            if (suffix) {
                return (str) ? (str + " " + suffix) : suffix
            }
            return str
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/HttpServer.es"
 */
/************************************************************************/

/*
    HttpServer.es -- Http Server class.
  
    Copyright (c) All Rights Reserved. See details at the end of the file.
*/

# Config.WEB
module ejs.web {

    /**
        HttpServer objects represents the server-side of Hypertext Transfer Protocol (HTTP) version 1.1 connections. 
        The HttpServer class is used to receive HTTP requests and generate responses. It supports the HTTP/1.1 standard
        including methods for GET, POST, PUT, DELETE, OPTIONS, and TRACE. It also supports Keep-Alive and SSL connections.
        @spec ejs
        @stability evolving
     */
    enumerable dynamic class HttpServer {
        use default namespace public

        /** Frequency to check and release excess worker threads */
        static var PrunePeriod = 60 * 1000

        private var idleWorkers: Array = []
        private var activeWorkers: Array = []
        private var workerImage: Worker

        private static const defaultConfig = {
            dirs: {
                cache: Path("cache"),
                layouts: Path("layouts"),
            },
            extensions: {
                es:  "es",
                ejs: "ejs",
                mod: "mod",
            },
            log: {
                showErrors: true,
            },
            cache: {
                adapter: "local",
                "class": "LocalCache",
                module: "ejs",
                lifespan: 3600,
                app:     { enable: true, reload: true },
                actions: { enable: true },
                records: { enable: true },
                workers: { enable: true, limit: 10 },
            },
            web: {
                limits: {},
                views: {
                    connectors: {
                        table: "html",
                        chart: "google",
                        rest: "html",
                    },
                    formats: {
                        Date: "%a %e %b %H:%M",
                    },
                    layout: "default.ejs",
                },
            },
        }

        /**
            One time initialization. Blend mandatory config into App.config.
            @hide
         */
        static function initHttpServer() {
            blend(App.config, defaultConfig, {overwrite: false})
            /* Overwrite some keys that exist in the default App.config */
            App.config.cache.app = { enable: true, reload: true }
            let dirs = App.config.dirs
            for (let [key, value] in dirs) {
                dirs[key] = Path(value)
            }
            if (!App.cache) {
                App.cache = new Cache(null, blend({shared: true}, App.config.cache))
            }
        }
        initHttpServer()

        /**
            Index files list. Index files are used by various handlers when requests to directories are made. The 
            indicies are tried in turn for the first valid index file.
          */
        static var indicies: Array = ["index.ejs", "index.html"]

        /** 
            Get the local IP address bound to this socket.
            @returns A string containing the address in dot notation. Returns the empty string if listening on all
            interfaces and returns null if the server is not bound (listening) to any address.
         */
        native function get address(): String?

        /** 
            @duplicate Stream.async
            If the server is put into sync mode, it must be done before calling listen.
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            Server configuration. Initially refers to App.config which is filled with the aggregated "ejsrc" content.
            If a "ejsrc" configuration file exists, it is loaded once at startup and is blended (overwrites) existing 
            App.config settings.
         */
        enumerable var config: Object

        /** 
            Default local directory for web documents to serve. This is used as the default Request.dir value.
         */
        var documents: Path

        /** @hide */
        native function get hostedDocuments(): Path
        /** @hide */
        native function get hostedHome(): Path

        /** 
            Flag indicating if the server is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native function get isSecure(): Boolean

        /**
            Flag indicating if the server is running hosted inside a web server
         */
        native function get hosted(): Boolean
        native function set hosted(value: Boolean): Void

        /**
            Resource limits for the server and for initial resource limits for requests.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option clients Maximum number of unique client IP addresses.
            @option connections Maximum number of simultaneous client endpoints.
            @option connReuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            @option headers Maximum number of headers in a request.
            @option header Maximum size of headers.
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requests Maximum number of simultaneous requests. This is actually a limit per client IP.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option sessionTimeout Default time to preserve session state for new requests. Set to zero for no timeout.
            @option stageBuffer Maximum stage buffer size for each direction.
            @option transmission Maximum size of outgoing body data.
            @option upload Maximum size of uploaded files.
            @option uri Maximum size of URIs.
            @option workers Maximum number of Worker threads to utilize for threaded requests. This value is initialized
                from the ejsrc cache.workers.limit field.
            @see setLimits
          */
        native function get limits(): Object

        /**
            The authorized public host name for the server. If defined, this name will be used in preference for 
            request redirections. If no name is defined, redirections will use to the listening IP address by default.
         */
        native function get name(): String 
        native function set name(hostname: String): Void

        /**
            HttpServer constructor options
         */
        var options: Boolean

        /** 
            Get the port bound to this Http endpoint.
            @return The port number or 0 if it is not bound.
         */
        native function get port(): Number 

        /**
            Function to invoke inside a worker when serving a threaded request. The function is invoked with the signature:
            function onrequest(request: Request): Void
         */
        public var onrequest = defaultOnRequest

        /*
            Default onrequest function to handle threaded requests
            @param request Request object
         */
        private function defaultOnRequest(request: Request): Void {
            App.log.debug(6, "Multithreaded request")
            try {
                process(request.route.response, request)
            } catch (e) {
                request.writeError(Http.ServerError, e)
            }
        }

        /** 
            Default root directory for the server. The app does not change its current directory to this path.
         */
        var home: Path

        /** 
            Software description for the web server
            @return A string containing the name and version of the web server software
         */
        native function get software(): String

        /** 
            Create a HttpServer object. The server is set to async mode by default.
            If an "ejsrc" file exists in the server root, it will be loaded.
            @param options. Set of options to configure the server.
            @option documents Directory containing web documents to serve. If unset and the HttpServer is hosted,
                the $documents property will be defined by the web server.
            @option home Base directory for the server and the "ejsrc" configuration file. If unset and the HttpServer is 
                hosted, the $home property will be defined by the web server.
            @option ejsrc Alternate path to the "ejsrc" configuration file
            @option config Alternate App.config settings
            @option unhosted If hosted inside a web server, set to true to bypass any web server listening endpoints and 
                create a new stand-alone (unhosted) listening connection.
            @spec ejs
            @stability prototype
            @example: This is a fully async server:
let server: HttpServer = new HttpServer({documents: "web"})
let router = Router(Router.Restful)
server.on("readable", function (event: String, request: Request) {
    // "this" object is also set to request
    request.status = 200
    request.setHeaders({"Content-Type": "text/plain"})
    request.on("readable", function (event, request) {
        let data = new ByteArray
        if (request.read(data) == null) {
            print("EOF")
        }
    })
    request.on("writable", function (event) {
        request.write("Hello World")
        request.finalize()
    })
}
server.listen("127.0.0.1:7777")
         */
        function HttpServer(options: Object = {}) {
            if (options.unhosted) {
                hosted = false
            }
            if (hosted) {
                documents = options.documents || hostedDocuments
                home = options.home || hostedHome
            } else {
                documents = options.documents || "."
                home = options.home || "."
            }
            home = home.relative
            config = options.config || App.config
            this.options = options
            let ejsrc = options.ejsrc || config.files.ejsrc
            if (ejsrc.exists && !App.config.files.ejsrc.same(ejsrc)) {
                blend(config, Path(config.files.ejsrc).readJSON())
                App.updateLog()
            } else if (home != ".") {
                let path = home.join("ejsrc")
                if (path.exists) {
                    blend(config, path.readJSON())
                    App.updateLog()
                }
            }
            /* Prefix dirs with the home directory */
            let dirs = config.dirs
            for (let [key, value] in dirs) {
                dirs[key] = home.join(value)
            }
            let web = config.web
            if (web.trace) {
                trace(web.trace)
            }
            web.limits.workers ||= config.cache.workers.limit
            setLimits(web.limits)
            if (web.session) {
                openSession()
            }
            //  MOB - BUG. Need this.fun to bind the function
            setInterval(this.pruneWorkers, PrunePeriod, this)
        }

        private function openSession() {
            let sconfig = config.session
            let sclass = sconfig["class"]
            if (sclass) {
                if (sconfig.module && !global.(module)::[sclass]) {
                    global.load(sconfig.module + ".mod", {reload: false})
                }
                let module = sconfig.module || "public"
                new (module)::[sclass](sconfig.adapter, sconfig.options)
            }
        }

        /** 
            Accept a new incoming for sync servers.  This call creates a request object in response to an 
            incoming client connection on the current HttpServer object.  In async mode, the accept() call is not needed
            as the HttpServer automatically creates the Request object and passes it on "readable" events.
            @return A Request object if in sync mode. No return value if in async mode. 
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new HttpServer
                server.listen("8080")
                while (request = server.accept()) {
                    server.serve(request)
                }
         */
        native function accept(): Request

        /** 
            @duplicate Stream.close 
         */
        native function close(): Void

        /*
            Get a worker for a multithreaded request. This will clone a worker from the workerImage and will
            enforce the configured limits.workers value.
         */
        private function getWorker(): Worker? {
            let w = idleWorkers.pop()
            if (w == undefined) {
                if (limits.workers && activeWorkers.length >= limits.workers) {
                    App.log.debug(1, "Too many workers " + activeWorkers.length + "/" + limits.workers)
                    return null
                }
                if (workerImage == null) {
                    workerImage = new Worker
                    workerImage.preeval("require ejs.web")
                }
                w = workerImage.clone()
                if (config.web.workers && config.web.workers.init) {
                    w.preeval(config.web.workers.init)
                }
            }
            activeWorkers.push(w)
            App.log.debug(4, "HttpServer.getWorker idle: " + idleWorkers.length + " active:" + activeWorkers.length)
            return w
        }

        //  MOB - should take an array of endpoints (like GoAhead) and allow https:///

        /** 
            Listen for client connections. This creates a HTTP server listening on a single socket endpoint. It can
            also be used to attach to an existing listening connection if embedded in a web server. 
            
            When hosted inside a web server, the web server will define the listening endpoints and ensure the 
            EjsScript startup script is executed. Then, when listen() is called, the HttpServer object will be bound to
            the actual web server's listening connection. In this case, the endpoint argument is not required and 
            is ignored.

            HttpServer supports both sync and async modes of operation.  In sync mode, after listen call is made, 
            $accept must be called to wait for and receive client connections. The $accept call will create the 
            Request object.  In async mode, Request objects will be created automatically and passed to registered 
            observers via "readable" events.

            @param endpoint The endpoint address on which to listen. An endoint may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            The IP portion may also be "*" to indicate all interfaces.
            If the server is hosted in a web server, an appropriate existing web server listening connection will be 
            used. Otherwise, if not hosted, a server socket will be opened on the endpoint.
            If hosted and an endpoint is not provided, the server will listen on all appropriate web server connections.
            @throws ArgError if the specified endpoint address is not valid or available for binding.
            @event Issues a "accept" event when there is a new connection available.
            @example:
                server = new Http({home: ".", documents: "web"})
                server.on("readable", function (event, request) {
                    //  NOTE: "this" is set to the request
                    server.serve(request)
                })
                server.listen("80")
         */
        native function listen(endpoint: String? = null): Void

        /** 
            Remove an observer from the server. 
            @param name Event name previously used with observe. The name may be an array of events.
            @param observer Observer function previously used with observe.
         */
        native function off(name: Object, observer: Function): Void

        /** 
            Add an observer for server events. 
            @param name Name of the event to listen for. The name may be an array of events.
            @param observer Callback listening function. The function is called with the following signature:
                function on(event: String, ...args): Void
            @event readable Issued when there is a new request available. This readable event will explicitly set the
                value of "this" to the request regardless of whether the function has a bound "this" value.
            @event close Issued when server is being closed.
         */
        native function on(name, observer: Function): HttpServer

        /**
            Pass a request into a worker VM. The onrequest callback receives the request. This routine clones stub 
            Request and HttpServer objects into the worker VM.
            @param request Request object
            @param worker Worker to handle the request
         */
        native function passRequest(request: Request, worker: Worker)

        /** 
            Process a web request
            @param request Request object
            @param app Web application function 
         */
        function process(app: Function, request: Request, finalize: Boolean = true): Void {
// let mark = new Date
            request.config = config
            try {
                if (request.route && request.route.middleware) {
                    app = Middleware(app, request.route.middleware)
                }
                if (finalize) {
                    request.setupFlash()
                }
                let response
                if (app.bound) {
                    response = app(request)
                } else {
                    response = app.call(request, request)
                }
                if (response is Function) {
                    /* Functions don't auto finalize. The user is responsible for calling finalize() */
                    response.call(request, request)

                } else if (response) {
                    request.status = response.status || 200
                    let headers = response.headers || { "Content-Type": "text/html" }
                    request.setHeaders(headers)
                    if (response.body) {
                        processBody(request, response.body)
                    }

                } else {
                    let file = request.responseHeaders["X-Sendfile"]
                    if (file && !request.isSecure && !request.finalized) {
                        request.writeFile(file)
                    }
                }
                if (finalize) {
                    request.finalizeFlash()
                    request.autoFinalize()
                }
            } catch (e) {
                App.log.debug(1, e)
                request.writeError(Http.ServerError, e)
            }
// App.log.debug(2, "LEAVE PROCESSING  " + mark.elapsed + " msec for " + request.uri)
        }

        private function processBody(request: Request, body: Object): Void {
            if (body is Path) {
                if (request.isSecure) {
                    body = File(body, "r")
                } else {
                    request.writeFile(body)
                    return
                }
            }
            if (body is Array) {
                for each (let item in body) {
//  MOB -- what about async? what if can't accept all the data?
                    request.write(item)
                }
                request.autoFinalize()

            } else if (body is Stream) {
                if (body.async) {
                    request.async = true
                    //  Should we wait on request being writable or on the body stream being readable?
//  MOB Must detect EOF and do a finalize()
                    request.on("readable", function(event, request) {
                        let data = new ByteArray
                        if (request.read(data)) {
//  MOB -- what about async? what if can't accept all the data?
                            request.write(body)
                        } else {
                            request.autoFinalize()
                        }
                    })
                    //  MOB -- or this? but what about error events
                    request.on("complete", function(event, body) {
                        request.autoFinalize()
                    })
                } else {
                    ba = new ByteArray
                    while (body.read(ba)) {
//  MOB -- exceptions on all these writes should be caught --- normal situation for client to disappear
                        request.write(ba)
                    }
                    request.autoFinalize()
                }
            } else if (body && body.forEach) {
                body.forEach(function(block) {
                    request.write(block)
                })
                request.autoFinalize()

            } else if (body is Function) {
                /* Functions don't auto finalize. The user is responsible for calling finalize() */
                body.call(request, request)

            } else if (body) {
                request.write(body)
                request.autoFinalize()

            } else {
                let file = request.responseHeaders["X-Sendfile"]
                if (file && !request.isSecure) {
                    request.writeFile(file)
                } else {
                    request.autoFinalize()
                }
            }
        }

        /** 
            Prune idle worker threads. This will release cached worker interpreters and reduce memory footprint. 
            After calling, the next request will be a little slower as it will need to recreate a worker interpreter.
            This is normally run every PrunePeriod. It may be also be called manually at any time.
         */
        function pruneWorkers(): Void {
            if (idleWorkers.length > 0) {
                App.log.debug(6, "HttpServer prune " + idleWorkers.length + " workers")
                idleWorkers = []
                GC.run()
            }
        }

        private function releaseWorker(w: Worker): Void {
            activeWorkers.remove(w)
            if (config.cache.workers.enable) {
                idleWorkers.push(w)
            }
            App.log.debug(4, "HttpServer.releaseWorker idle: " + idleWorkers.length + " active:" + activeWorkers.length)
        }

        /** 
            Run the application event loop to service requests.
            If the HttpServer is hosted in a web server, this call does nothing as the web server will service events and
            will return immediately. 
         */
        native function run(): Void

        /** 
            Define the Secure Sockets Layer (SSL) protocol credentials. This must be done before calling $listen.
            @param keyFile Path of the file containing the server's private key. This file
            contains the PEM encoded private key file for the server. Set to null if the private key is combined with 
            the certificate file. If the private key is encrypted, you will be prompted at the console to enter the 
            pass-phrase to decript the private key on system reboot. There is a delima here. If you use a crypted 
            private key, the server will pause until you enter the pass-phrase which makes headless operation impossible. 
            If you do not encrypt the private key, your private key is more vulnerable should the server be compromised. 
            Which option you choose depends on whether headless operation is essential or not.
            @param certFile Path of the file containing the SSL certificate
            The certificate file contains the PEM encoded X.509 certificate for the server. The file may also contain 
            the private key in which case you should set the key parameter to null.
            The path may be an absolute path or it may be relative to the ServerRoot.
            @param protocols Optional arary of SSL protocols to support. Select from: SSLv2, SSLv3, TLSv1, ALL. 
                Each protocol can be prefixed by "+" or "-" to add or subtract from the prior set.
                For example: ["ALL", "-SSLv2"], or ["SSLv3", "TLSv1"] or "[ALL]"
            @param ciphers Optional array of ciphers to use when negotiating the SSL connection. Not yet supported.
            @throws ArgError for invalid arguments
         */
        native function secure(keyFile: Path?, certFile: Path!, protocols: Array? = null, ciphers: Array? = null): Void

        /** 
            Serve a web request. Convenience function to route, load and start a web application. 
            Called by web application start script
            @param request Request object
            @param router Configured router instance. If omitted, a default Router will be created using the Router.Top
                routing table.
         */
        function serve(request: Request, router: Router = Router()): Void {
            request.mark = new Date
            try {
                let w: Worker?
                let route: Route = router.route(request)
                if (route.workers) {
                    if ((w = getWorker()) == null) {
                        request.writeError(Http.ServiceUnavailable, "Server busy")
                        return
                    }
                    request.on("close", function() {
                        releaseWorker(w) 
                        App.log.debug(3, "Elapsed " + request.mark.elapsed + " msec for " + request.uri)
                    })
                    passRequest(request, w)
                    /* Must not touch request from here on - the worker owns it now */
                } else {
                    //  MOB - rename response => responder
                    let mark = new Date
                    process(route.response, request)
                    App.log.debug(3, "Elapsed " + mark.elapsed + " msec for " + request.uri)
                }
            } catch (e) {
                let status = request.status != Http.Ok ? request.status : Http.ServerError
                request.writeError(status, e)
            }
        }

        /**
            Define the stages of the Http processing pipeline. Data flows through the processing pipeline and is
            filtered or transmuted by filter stages. A communications connector is responsible for transmitting to 
            the network.
            @param incoming Array of stages for the incoming pipeline: default: ["chunk", "range", "upload"]
            @param outgoing Array of stages for the outgoing pipeline: default: ["auth", "range", "chunk"]
            @param connector Network connector to use for I/O. Defaults to the network connector "net". Other values: "send".
                The "net" connector transparently upgrades to the "send" connector if transmitting static data and 
                not using SSL, ranged or chunked transfers.
         */
        native function setPipeline(incoming: Array, outgoing: Array, connector: String): Void

        /**
            Update the server resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /**
            Define a worker that will serve as the base for cloning workers to serve web requests
            @param worker Configured worker 
         */
        function setWorkerImage(worker: Worker): Void
            workerImage = worker

        /**
            Configure request tracing for the server. The default is to trace the first line of requests and responses at
            level 2 and to trace headers at level 3. The options argument contains optional properties: rx and tx 
            (for receive and transmit tracing). The rx and tx properties may contain an object hash which describes 
            the tracing for that direction and includes any of the following fields:
            @param options. Set of trace options with properties "rx" and "tx" for receive and transmit direction tracing.
                The include property is an array of file extensions to include in tracing.
                The include property is an array of file extensions to exclude from tracing.
                The all property specifies that everything for this direction should be traced.
                The conn property specifies that new connections should be traced.
                The first property specifies that the first line of the request should be traced.
                The headers property specifies that the headers (including first line) of the request should be traced.
                The body property specifies that the body content of the request should be traced.
                The size property specifies a maximum body size in bytes that will be traced. Content beyond this limit 
                    will not be traced.
            @option transmit. Object hash with optional properties: include, exclude, first, headers, body, size.
            @option receive. Object hash with optional properties: include, exclude, conn, first, headers, body, size.
            @example:
                trace({
                    transmit: { exclude: ["gif", "png"], "headers": 3, "body": 4, size: 1000000 }
                    receive:  { "conn": 1, "headers": 2 , "body": 4, size: 1024 }
                })
          */
        native function trace(options: Object): Void

        /**
            Verify client certificates. This ensures that the clients must provide a client certificate for to verify 
            the their identity. You can choose to use either the caCertPath or caCertFile argument. If both are provided
            caCertPath takes precedence.
            @param caCertPath Defines the directory containing the certificates to use for client authentication.
            The path may be an absolute path or it may be relative to the ServerRoot.
            Set to null if you are using $caCertFile.
            @param caCertFile Defines the location of the certificate file or bundle to use for client authentication.
                Use this if you have a single certificate or a bundle of certificates.
                Set to null if you are using $caCertPath.
         */
        native function verifyClients(caCertPath: Path?, caCertFile: Path?): Void

        /**
            Convenience routine to create a web server. This will start a routing web server that will serve a 
            variety of content using the given specified route tables.
            @param address The IP endpoint address on which to listen. The address may be a port number or a composite 
            "IP:PORT" string. If only a port number is provided, the socket will listen on all interfaces on that port. 
            If null is provided for an endpoint value, an existing web server listening connection will be used. In this
            case, the web server will typically be the virtual host that specifies the EjsStartup script. See the
            hosting web server documentation for specifics.
            @param options. Set of options to configure the server.
            @option documents Directory containing web documents to serve. If unset and the HttpServer is hosted,
                the $documents property will be defined by the web server.
            @option home Base directory for the server and the "ejsrc" configuration file. If unset and the HttpServer is 
                hosted, the $home property will be defined by the web server.
            @option routes Route table to use. Defaults to Router.Top
         */
        static function create(address: String?, options: Object = {}): Void {
            let server: HttpServer = new HttpServer(options)
            let routes = options.routes || Router.Top
            var router = Router(routes)
            server.on("readable", function (event, request) {
                server.serve(request, router)
            })
            server.listen(address)
            App.run()
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/MethodOverride.es"
 */
/************************************************************************/

/*
    MethodOverride.es - Override the method type based on the Method-Override header
 */

# Config.WEB
module ejs.web {

    /**
        Method override wrapper middleware. Provides HTTP method overriding via a "-ejs-method-" POST parameter or via
            a X-HTTP-METHOD-OVERRIDE Http header.
        @param app Application generating the response. 
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
        @example:
            export.app = MethodOverride(app)
     */
    function MethodOverride(app: Function): Function {
        return function(request: Request) {
            if (request.method == "POST") {
                let method = request.params["-ejs-method-"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method) {
                    request.method = method
                }
            }
            return app(request)
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Middleware.es"
 */
/************************************************************************/

/*
    Middleware.es - Wrap a web app with defined middleware
 */

# Config.WEB
module ejs.web {

    /** 
        Define middleware for a web app. This wrapps the web application function with defined middleware filters.
        @param app Base web app function object
        @param middleware Array of middleware wrapper applications
        @returns A top level web application function object
        @spec ejs
        @stability prototype
     */
    function Middleware(app: Function, middleware: Array? = null): Function {
        for each (mid in middleware) {
            app = mid[i](app)
        }
        return function (request: Request)
            app(request)
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Mvc.es"
 */
/************************************************************************/

/**
    Mvc.es -- Model View Controller (MVC) web app management
 */

# Config.WEB
module ejs.web {

    /** 
        The Mvc class manages the loading and initialization of MVC web applications
        @stability prototype
        @spec ejs
     */
    class Mvc {

        static var apps = {}

        /*  
            Default configuration for MVC apps. This layers over App.defaultConfig and ejs.web::defaultConfig.
         */
        private static var defaultConfig = {
            dirs: {
                bin: Path("bin"),
                db: Path("db"),
                controllers: Path("controllers"),
                models: Path("models"),
                src: Path("src"),
                static: Path("static"),
                views: Path("views"),
            },
            mvc: {
                //  MOB - should be moved to files
                appmod: "App.mod",
                view: {
                    connectors: { },
                    formats: { },
                },
            },
        }

        private var config: Object
        private static var loaded: Object = {}
        private static const EJSRC = "ejsrc"

        /* App.config.dirs should be not be prefixed with any application paths */
        blend(App.config, defaultConfig, {overwrite: false})

        /** 
            Load an MVC application and the optional application specific ejsrc file
            @param home Base directory for the MVC app
            @param config Configuration object
         */
        function Mvc(home: Path, config = App.config) {
            this.config = config
            let path = home.join(EJSRC)
            if (path.exists) {
                let econfig = path.readJSON()
                let dirs = econfig.dirs
                for (let [key,value] in econfig.dirs) {
                    dirs[key] = home.join(value)
                }
                config = blend(config.clone(), econfig)
                App.updateLog()
            }
            if (config.database) {
                openDatabase(home)
            }
        }

        /**
            Factory to load an MVC application.
            @param request Request object
            @param dir Base directory containing the MVC application. Defaults to "."
            @param config Default configuration for the application
            @return An Mvc application object
          */
        public static function load(request: Request?, dir: Path = ".", config = App.config): Mvc {
            if ((mvc = Mvc.apps[dir]) == null) {
                App.log.debug(2, "Load MVC application from \"" + dir + "\"")
                mvc = Mvc.apps[dir] = new Mvc(dir, config)
                let appmod = config.dirs.cache.join(config.mvc.appmod)

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.cache.app.reload) {
                    let dirs = config.dirs
                    let ext = config.extensions
                    deps = [dir.join(EJSRC)]
                    files = dirs.models.files("*" + ext.es)
                    files += dirs.src.files("*" + ext.es)
                    files += [dirs.controllers.join("Base").joinExt(ext.es)]
                }
                request ||= new Request("/")
                request.config = config
                mvc.loadComponent(request, appmod, files, deps)
                loaded[appmod] = new Date
            }
            return mvc
        }

        /*
            Load the required parts of the Mvc app
         */
        function loadRequest(request: Request) {
            request.config = config
            let dirs = config.dirs
            let appmod = dirs.cache.join(config.mvc.appmod)
            if (config.web.flat) {
                //  MOB - implement flat
                if (!global.BaseController) {
                    global.load(appmod)
                }
            } else {
                let ext = config.extensions
                let dir = request.dir
                request.log.debug(4, "MVC init at \"" + dir + "\"")

                /* Load App. Touch ejsrc triggers a complete reload */
                let files, deps
                if (config.cache.app.reload) {
                    deps = [dir.join(EJSRC)]
                    files = dirs.models.files("*" + ext.es)
                    files += dirs.src.files("*" + ext.es)
                    files += [dirs.controllers.join("Base").joinExt(ext.es)]
                }
                loadComponent(request, appmod, files, deps)

                /* Load controller */
                let params = request.params
                if (!params.controller) {
                    throw "No controller specified by route: " + request.route.name
                }
                let controller = params.controller = params.controller.toPascal()
                let mod = dirs.cache.join(controller).joinExt(ext.mod)
                if (controller != "Base") {
                    if (!global[controller + "Controller"] && mod.exists && !config.cache.app.reload) {
                        loadComponent(request, mod)
                    } else {
                        files = [dirs.controllers.join(controller).joinExt(ext.es)]
                        deps = [dirs.controllers.join("Base").joinExt(ext.es)]
                        loadComponent(request, mod, files, deps)
                    }
                }
            }
        }

        /** 
            Load a component. This will load a module and optionally recompile if the given dependency paths are
            more recent than the module itself. If recompilation occurs, the result will be cached in the supplied module.
            @param request Request object
            @param mod Path to the module to load
            @param files Files to compile into the module
            @param deps Extra file dependencies
         */
        public function loadComponent(request: Request, mod: Path, files: Array? = null, deps: Array? = null) {
            let rebuild = false
            if (mod.exists && request.config.cache.app.reload) {
                let when = mod.modified
                for each (file in (files + deps)) {
                    if (file.exists && file.modified > when) {
                        rebuild = true
                    }
                }
            }
            if (rebuild) {
                rebuildComponent(request, mod, files)
            } else if (loaded[mod]) {
                request.log.debug(4, "Mvc.loadComponent: component already loaded: " + mod)
            } else {
                try {
                    request.log.debug(4, "Mvc.loadComponent: load component : " + mod)
                    global.load(mod)
                    loaded[mod] = new Date
                } catch (e) {
                    request.log.debug(4, "Mvc.loadComponent: Load failed, rebuild component: " + mod)
                    rebuildComponent(request, mod, files)
                }
            }
        }

        /**
            Open database. Expects ejsrc configuration:
                mode: "debug",
                database: {
                    adapter: "sqlite",
                    module: "ejs.db.sqlite",
                    class: "Sqlite",
                    debug: { name: "db/blog.sdb", trace: true },
                    test: { name: "db/blog.sdb", trace: true },
                    production: { name: "db/blog.sdb", trace: true },
                }
            @hide
         */
        public static function openDatabase(home: Path) {
            let dbconfig = App.config.database
            if (dbconfig) {
                for each (kind in ["debug", "test", "production"]) {
                    if (dbconfig[kind]) {
                        dbconfig[kind].name = home.join(dbconfig[kind].name)
                    }
                }
                global.load("ejs.db.mod", {reload: false})
                blend(dbconfig, dbconfig[App.config.mode])
                new "ejs.db"::["Database"](dbconfig.adapter, dbconfig)
            }
        }

        private function rebuildComponent(request: Request, mod: Path, files: Array) {
            let code = "require ejs.web\n"
            for each (file in files) {
                let path = Path(file)
                if (!path.exists) {
                    request.status = Http.NotFound
                    throw "Cannot find required component: \"" + path + "\""
                }
                code += path.readString()
            }
            request.log.debug(4, "Rebuild component: " + mod + " files: " + files)
            eval(code, mod)
        }

    }

    //  MOB - who uses this?
    /**
        MVC request handler.  
        @param request Request object
        @return A response hash (empty). MVC apps use Request methods directly to set status, headers and response body.
        @spec ejs
        @stability prototype
     */
    function MvcApp(request: Request): Object {
        let app = MvcBuilder(request)
        return app(request)
    }

//  MOB -- update doc. Rename from builder?
    /** 
        MVC builder for use in routing tables. The MVC builder function can be included directly in Route table entries.
        @param request Request object. 
        @return A web script function that services a web request.
        @spec ejs
        @stability prototype
     */
    function MvcBuilder(request: Request): Function {
        let mvc: Mvc = Mvc.load(request, request.dir, request.config)
        mvc.loadRequest(request)
        return Controller.create(request, request.params.controller + "Controller").app
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Request.es"
 */
/************************************************************************/

/**
    Request.es -- Ejscript web request object. The Request object respresents a single HTTP request and provides
    low-level properties and methods to inspect and control the request. Request objects support the CommonJS JSGI 
    protocol specification. 
 */
# Config.WEB
module ejs.web {

    /**
        Web request class. Request objects manage the state and execution of a web request. The HttpServer class creates
        instances of Request in response to incoming client requests. The Request object holds the client request state
        including the request URL and headers. It provides low-level properties and methods to inspect and control the
        request. The Request object is a Stream and by calling the read() and write() methods, request body 
        content can be read and written.

        The response to send back to the client can be defined by setting status and calling setHeaders() and write() to 
        set the response status, headers and body content respectively.
        @spec ejs
        @stability prototype
     */
    dynamic class Request implements Stream {
        use default namespace public

        /** Security Token for use to help mitigate CSRF security threats */
        static const SecurityTokenName = "__ejs_security_token__"

        private var lastFlash: Object

        /** 
            Uri for the top-level of the application. This is an absolute Uri that includes scheme and host components.
            See $home to get a relative Uri.
         */ 
        native enumerable var absHome: Uri

        //  MOB - update
        /** 
            Authentication group. This property is set to the value of the authentication group header. 
            This field is read-only.
         */
        native enumerable var authGroup: String

        /** 
            Authentication method if authorization is being used (basic or digest). Set to null if not using authentication. 
            This field is read-only.
         */
        native enumerable var authType: String?

        /** 
            Authentication user name. This property is set to the value of the authentication user header. Set to null if
            not yet defined.
            This field is read-only.
         */
        native enumerable var authUser: String?

        /** 
            Control whether auto-finalizing will be used for the request. If autoFinalizing is false, a request must
            call $finalize() to signal the end of write data. If autoFinalize is true, the Request class will automatically 
            call finalize at the conclusion of the request.
            Applications that wish to keep the request open to the client can suppress auto-finalization by 
            setting autoFinalizing to false or by calling $dontAutoFinalize(). This property defaults to true.
         */
        native enumerable var autoFinalizing: Boolean

        /** 
            Request configuration. Initially refers to $App.config which is filled with the aggregated "ejsrc" content.
            Middleware may modify to refer to a request local configuration object.
         */
        native enumerable var config: Object

        /** 
            Associated Controller object. Set to null if no associated controller.
         */
        native enumerable var controller: Controller?

        /** 
            Get the request content length. This is the length of body data sent by the client with the request. 
            This property is read-only and is set to the length of the request content body in bytes or -1 if not known.
            Body data is readable by using $read() or by using the request object as a stream.
         */
        native enumerable var contentLength: Number

        /** 
            The request content type as specified by the "Content-Type" Http request header. This is set to null 
            if not defined. This is the content type of the request body content sent with the request.
         */
        native enumerable var contentType: String?

        /** 
            Cookie headers. Cookies are sent by the client browser via the Set-Cookie Http header. They are typically 
            used used to specify the session state. If sessions are being used, an Ejscript session cookie will be 
            sent to and from the browser with each request. 
         */
        native enumerable var cookies: Object

        /** 
            Application web document directory on the local file system. This is set to the directory containing the
            application. For MVC applications, this is set to the base directory of the application. For non-MVC apps, 
            it is set to the document root directory specified when the HttpServer was instantiated.
         */
        native enumerable var dir: Path

        /** 
            Get the encoding scheme for serializing strings. The default encoding is "utf-8".
            @hide
         */
        native var encoding: String

        /** 
            Descriptive error message for the request. This message is defined internally by the Request if a request
            times out or has a communications error.
         */
        native enumerable var errorMessage: String

        /** 
            Files uploaded as part of the request. For each uploaded file, an instance of UploadFile is created in files. 
            Each element is named by the file upload HTML input element ID in the HTML page form. 
         */
        native enumerable var files: Object

        /**
            Physical filename for the resource supplying the response content for the request. Virtual requests where
            the Request $uri does not correspond to any physical resource may not define this property.
         */
        native enumerable var filename: Path

//  MOB -- why public here
        /** 
            Notification "flash" messages to pass to the next request (only). By convention, the following keys are used.
            error: Negative errors (Warnings and errors), inform: Informational / postitive feedback (note),
            warn: Negative feedback (Warnings and errors), *: Other feedback (reminders, suggestions...)
        */
        public var flash: Object?

        /** 
            The request form parameters as a string. This parameters are www-url decoded from the POST request body data. 
            This is useful to get a stable, sorted string representing the form parameters.
         */
        native enumerable var formData: String

        /** 
            Request Http headers. This is an object hash filled with lower-case request headers from the client. If multiple 
            headers of the same key value are defined, their contents will be catenated with a ", " separator as per the 
            HTTP/1.1 specification. Use the header() method if you want to retrieve a single header.
            Headers defined on the server-side by creating new header entries in $headers will preserve case. 
            Use $header() if you want to match headers using a mixed case key. e.g. header("Content-Length").
         */
        native enumerable var headers: Object

        /** 
            Home URI for the application. This is a relative Uri from the current URI to the 
            the top-most directory level of the application. 
         */ 
        native enumerable var home: Uri

        /** 
            Host serving the request. This is initialized to the authorized server hostname (HttpServer.name) if one is 
            configured.  Otherwise it will use Http "Host" header value if supplied by the client else the server IP 
            address of the accepting interface. This algorithm attempts to use the most public address available for 
            the server.
         */
        native enumerable var host: String

        /** 
            Flag indicating if the request is using secure communications. This means that TLS/SSL is the underlying
            protocol scheme.
         */
        native enumerable var isSecure: Boolean

        /**
            Resource limits for the request. The limits have initial default values defined by the owning HttpServer.
            @param limits. Limits is an object hash with the following properties:
            @option chunk Maximum size of a chunk when using chunked transfer encoding.
            @option connReuse Maximum number of times to reuse a connection for requests (KeepAlive count).
            @option inactivityTimeout Maximum time in seconds to keep a connection open if idle. Set to zero for no timeout.
            @option receive Maximum size of incoming body data.
            @option requestTimeout Maximum time in seconds for a request to complete. Set to zero for no timeout.
            @option sessionTimeout Maximum time to preserve session state. Set to zero for no timeout.
            @option transmission Maximum size of outgoing body data.
            @option upload Maximum size of uploaded files.
            @see setLimits
          */
        native enumerable var limits: Object

        /** 
            Server IP address of the accepting interface
         */
        native enumerable var localAddress: String

        /** 
            Logger object. Set to App.log. This is configured from the "log" section of the "ejsrc" config file.
         */
        native var log: Logger 

        /** 
            Request HTTP method. String containing the Http method (DELETE | GET | POST | PUT | OPTIONS | TRACE)
         */
        native enumerable var method: String

        /** 
            Original HTTP request method used by the client. If the method is overridden by including a "-ejs-method-" 
            parameter in a POST request or by defining an X-HTTP-METHOD-OVERRIDE Http header, the original method used by
            the client is stored in this property and the method property reflects the newly defined value. If method is
            not overridden, originalMethod will be null.
         */
        native enumerable var originalMethod: String?

        /**
            The original request URI supplied by the client. This is the Uri path supplied by the client on the first
            line of the Http request. It is combined with the HttpServer scheme, host and port components to yield a 
            fully qualified URI. The "uri" property has fields for: scheme, host, port, path, query and reference.
            The "uri" property is read-only.
         */
        native enumerable var originalUri: Uri

        /** 
            The request form parameters plus other routing parameters. 
            The form parameters are www-url decoded from the POST request body data. See also $form.
         */
        native enumerable var params: Object

        /** 
            Location of the request within the application. This is the portion of the request URL after the scriptName. 
            The pathInfo is originally derrived from uri.path after splitting off the scriptName. Changes to the uri or 
            scriptName properties will not affect the pathInfo property.
         */
        native enumerable var pathInfo: String

        /** 
            TCP/IP port number for the server of this request.
         */
        native enumerable var port: Number

        /** 
            Http request protocol (HTTP/1.0 | HTTP/1.1)
         */
        native enumerable var protocol: String

        /** 
            Request query string. This is the portion of the Uri after the "?". Set to null if there is no query.
         */
        native enumerable var query: String?

        /** 
            Request reference string. This is the portion of the Uri after the "#". Set to null if there is no reference.
         */
        native enumerable var reference: String?

        /** 
            Name of the referring URL. This comes from the request "Referrer" Http header. Set to null if there is
            no defined referrer. Note: the HTTP header is misspelt as "referer"
         */
        native enumerable var referrer: String?

        /** 
            IP address of the client issuing the request. 
         */
        native enumerable var remoteAddress: String

        /**
            The application has responded in some way. The application has commenced a response by doing some 
            output or setting status.
         */
        native var responded: Boolean

        /** 
            Http response headers. This is the proposed set of headers to send with the response.
            The case of header keys is preserved.
         */
        native enumerable var responseHeaders: Object

        /** 
            Route used for the request. The route is the matching entry in the route table for the request.
            The route has properties two properties of particular interest: "name" which is the name of the route and
            and "type" which classifies the type of request. 
         */
        enumerable var route: Route

        /**
            Count of times the request has been routed. Used to prevent recursive loops.
            @hide
         */
        native var routed: Number

        /** 
            Http request scheme (http | https)
         */
        native enumerable var scheme: String

        /** 
            Script name for the current application serving the request. This is typically the leading Uri portion 
            corresponding to the application, but middleware may modify this to be an arbitrary string representing 
            the application.  The script name is often determined by the Router as it parses the request using 
            the routing tables. The scriptName will be set to the empty string if not defined, otherwise is should begin
            with a "/" character. NOTE: changing script name will not update the home property.
         */
        native enumerable var scriptName: String

        /** 
            Owning server for the request. This is the HttpServer object that created this request.
         */
        native enumerable var server: HttpServer

        /** 
            Session state object. The session state object can be used to share state between requests.
            If a session has not already been created, accessing this property automatically creates a new session 
            and sets the $ejs.web::Request.sessionID property and a cookie containing a session ID sent to the client 
            with the response.
            To test if a session has been created, test the sessionID property which will not auto-create a session.
            Objects are stored in the session state using JSON serialization.
         */
        native var session: Session 

        /** 
            Current session ID. Index into the $sessions object. Set to null if no session is defined.
         */
        native enumerable var sessionID: String?

        /** 
            Set to the (proposed) Http response status code.
         */
        native enumerable var status: Number

        /**
            The current request URI. This property is read-only and is dynamically computed from the originalUri combined
            with the current scheme, host, port, scriptName, pathInfo, query and reference property values. 
            The "uri" property has fields for: scheme, host, port, path, query and reference.
         */
        native enumerable var uri: Uri

        /**
            Write buffer when capturing output
         */
        var writeBuffer: ByteArray?

        /*************************************** Methods ******************************************/
        /**
            Construct the a Request object. Request objects are typically created by HttpServers and not constructed
            manually.
            @param uri Request URI
            @param dir Default directory containing web documents
         */
        function Request(uri: Uri, dir: Path = ".") {
            this.uri = uri
            this.dir = dir
        }

        /** 
            @duplicate Stream.async
            Request does not support sync mode and only supports async mode.
         */
        native function get async(): Boolean
        native function set async(enable: Boolean): Void

        /** 
            Finalize the request if $autoFinalizing is true. Finalization signals the end of any write data 
            and flushes any buffered write data to the client. This routine is used by frameworks to finalize
            requests if required. 
         */
        native function autoFinalize(): Void 

        /** 
            @duplicate Stream.close
            This closes the current request by finalizing all transmission data and sending a "close" event. It may 
            not actually close the socket connection if the reuse limit has not been exceeded (see limits).
            It is normally not necessary to explicitly call close a request as the web framework will automatically 
            close finalized requests when all input data has fully been read. Calling close on an already closed
            request is silently ignored. 
         */
        native function close(): Void

        /**
            Check the request security token. If a required security token is defined in the session state, the
            request must supply the same token with all POST requests. This helps mitigate potential CSRF threats.
            @throw Throws a SecurityError if the token does not match.
         */
        function checkSecurityToken() {
            if (session[SecurityTokenName] && session[SecurityTokenName] != params[SecurityTokenName]) {
                throw new SecurityError("Security token does not match. Potential CSRF attack. Denying request")
            }
        }

        /**
            Clear previous flags messages. Useful when using client mode caching for an action.
            @stability prototype
            @hide
         */
        function clearFlash(): Void
            flash = null

        /**
            Create a session state object. The session state object can be used to share state between requests.
            If a session has not already been created, this call will create a new session and initialize the 
            $session property with the new session. It will also set the $sessionID property and a cookie containing 
            a session ID that will be sent to the client with the response. Sessions can also be used/created by simply
            accessing the session property.  Objects are stored in the session state using JSON serialization.
            @param timeout Optional session state timeout in seconds. Set to zero for no timeout. After the timeout has 
                expired, the session will be deleted. 
         */
        function createSession(timeout: Number = -1): Session {
            if (timeout >= 0) {
// - MOB - does this change the timeout for just this session or for all?
                setLimits({ sessionTimeout: timeout })
            }
            return session
        }

        /**
            Stop auto-finalizing the request. This will set $autoFinalizing to false. Some web frameworks will 
            "auto-finalize" requests by calling finalize() automatically at the conclusion of the request. 
            Applications that wish to keep the connection open to the client can defeat this auto-finalization by 
            calling dontAutoFinalize().
         */
        native function dontAutoFinalize(): Void

        /** 
            Destroy a session. This call destroys the session state store that is being used for the current client. 
            If no session exists, this call has no effect. Sessions are created by reading or writing to the $session 
            property.
         */
        native function destroySession(): Void

        /** 
            Set an error flash notification message.
            Flash messages persist for only one request and are a convenient way to pass state information or 
            feedback messages to the next request. To use flash messages, setupFlash() and finalizeFlash() must 
            be called before and after the request is processed. HttpServer.process will call setupFlash and finalizeFlash 
            automatically.
            @param msg Message to store
         */
        function error(msg: String): Void
            notify("error", msg)

        /** 
            The request pathInfo file extension
         */
        function get extension(): String
            Uri(pathInfo).extension

        /** 
            Signals the end of any and all response data and flushes any buffered write data to the client. 
            If the request has already been finalized, this call has no additional effect.
         */
        native function finalize(): Void 

        /** 
            Has the request output been finalized. 
            @return True if the all the output has been written.
         */
        native function get finalized(): Boolean 

        /**
            Save flash messages for the next request and delete old flash messages.
         */
        function finalizeFlash() {
            if (flash) {
                if (lastFlash) {
                    for (item in flash) {
                        for each (old in lastFlash) {
                            if (hashcode(flash[item]) == hashcode(old)) {
                                delete flash[item]
                            }
                        }
                    }
                }
                if (Object.getOwnPropertyCount(flash) > 0) {
                    session["__flash__"] = flash
                }
            }
        }

        /** 
            Flush request data. Calling flush(Sream.WRITE) or finalize() is required to ensure buffered write data is sent 
            to the client. Flushing the read direction is ignored.
            @duplicate Stream.flush
         */
        native function flush(dir: Number = Stream.WRITE): Void

        /** 
            Get a request header by keyword. Headers supplied by the remote client are stored in lower-case. 
            Headers defined on the server-side preserve case. This routine supports both by doing a case-insensitive lookup.
            @param key Header key value to retrieve. The key match is case insensitive.
            @return The header value
         */
        native function header(key: String): String

        /** 
            Set an informational flash notification message.
            Flash messages persist for only one request and are a convenient way to pass state information or 
            feedback messages to the next request. To use flash messages, setupFlash() and finalizeFlash() must 
            be called before and after the request is processed. HttpServer.process will call setupFlash and finalizeFlash 
            automatically.
            @param msg Message to store
         */
        function inform(msg: String): Void
            notify("inform", msg)

        // MOB - OPT make native
        /** 
            Create a URI link. The target parameter may contain partial or complete URI information. The missing parts 
            are supplied using the current request and route tables. The resulting URI is a normalized, server-local 
            URI (begins with "/"). The URI will include any defined scriptName, but will not include scheme, host or 
            port components.

            @param target The URI target. The target parameter can be a URI string or object hash of components. If the 
                target is a string, it is may contain an absolute or relative URI. If the target has an absolute URI path, 
                that path is used unmodified. If the target is a relative URI, it is appended to the current request URI 
                path.  The target can also be an object hash of URI components: scheme, host, port, path, reference and
                query. If these component properties are supplied, these will be combined to create a URI.

                The URI will be created according to the route URI template. The template may be explicitly specified
                via a "route" target property. Otherwise, if an "action" property is specified, the route of the same
                name will be used. If these don't result in a usable route, the "default" route will be used. See the
                Router for more details.
               
                If the target is a string that begins with "\@" it will be interpreted as a controller/action pair of the 
                form "\@Controller/action". If the "controller/" portion is absent, the current controller is used. If 
                the action component is missing, the "index" action is used. A bare "\@" refers to the "index" action 
                of the current controller.

                Lastly, the target object hash may contain an override "uri" property. If specified, the value of the 
                "uri" property will be returned and all other properties will be ignored.

            @option scheme String URI scheme portion
            @option host String URI host portion
            @option port Number URI port number
            @option path String URI path portion
            @option reference String URI path reference. Does not include "#"
            @option query String URI query parameters. Does not include "?"
            @option controller String Controller name if using a Controller-based route. This can also be specified via
                the action option.
            @option action String Action to invoke. This can be a URI string or a Controller action of the form
                @Controller/action.
            @option route String Route name to use for the URI template
            @return A normalized, server-local Uri object. The returned URI will be an absolute URI.
            @example
Given a current request of http://example.com/samples/demo" and "r" == the current request:

r.link("images/splash.png")                  # "/samples/images/splash.png"
r.link("images/splash.png").complete(r.uri)  # "http://example.com/samples/images/splash.png"
r.link("images/splash.png").relative(r.uri)  # "images/splash.png"

r.link("http://example.com/index.html")
r.link("/path/to/index.html")
r.link("\@Controller/checkout")
r.link("\@Controller/")
r.link("\@checkout")
r.link("\@")
r.link({action: "checkout")
r.link({action: "logout", controller: "Admin")
r.link({action: "Admin/logout")
r.link({action: "\@Admin/logout")
r.link({uri: "http://example.com/checkout"})
r.link({route: "default", action: "\@checkout")
r.link({product: "candy", quantity: "10", template: "/cart/{product}/{quantity}}")
         */
        function link(target: Object): Uri {
            /*
                MOB - refactor:
                Don't update target as the object hash. Don't use target.uri as the intermediate form. 
                Don't support outside use of target.uri to tunnel a URI
             */
            if (target is Uri) {
                target = target.toString()
            }
            if (target is String) {
                if (target[0] == '@') {
                    //  MOB - what about a possible controller in the target?
                    target = {action: target}
                } else {
                    if (target[0] == '~') {
                        target = scriptName + target.slice(1)
                    } else if (target[0] == '/') {
                        target = scriptName + target
            /*
                    } else if (!target.contains("/")) {
                        return new Uri(target)
            */
                    }
                    target = {uri: target}
                }
            }
            //  MOB - remove target.uri tunneling except internally in this routine
            if (!target.uri) {
                target = target.clone()
                if (target.action) {
                    //  MOB - this should be genericized and take request.params to get a default action / controller
                    //  and all other params
                    if (target.action[0] == '@') {
                        target.action = target.action.slice(1)
                    }
                    if (target.action.contains("/")) {
                        [target.controller, target.action] = target.action.split("/")
                    }
                    if (!target.controller && controller) {
                        target.controller = controller.controllerName
                    }
                    target.route ||= target.action || "default"
                }
                if (target.route) {
                    target.scriptName ||= scriptName
                    if (!target.template && route) {
                        target.template = route.getTemplate(target.controller, target.route)
                    }
                }
            }
            if (target.route && target.template) {
                target.scriptName = scriptName
                target = Uri.template(target.template, target).path
            }
            if (!(target is String)) {
                target.path ||= "/"
            }
            return uri.local.resolve(target).normalize
        }

        /**
            Select the response content type based on the request "Accept" header . See RFC-2616.
            @param formats Array of server supported mime types
            @return formats The selected mime types mime type string

            Accept: "application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png"
         */
        function matchContent(...formats): String {
            let accept = header("Accept")
            if (accept) {
                let media = accept.split(",")
                for (let [i, m] in media) {
                    let [mime, quality] = m.split(";")
                    quality = (quality || "q=1").trimStart("q=") cast Number
                    media[i] = { mime: mime, quality: quality || 1}
                }
                media = media.sort(function(a, i, j) {
                    if (a[i].quality < a[j].quality) {
                        return -1
                    } else if (a[i].quality > a[j].quality) {
                        return 1
                    } else {
                        return 0
                    }
                }, -1)
                for each (m in media) {
                    if (m.mime.contains("*")) {
                        let pat = RegExp(m.mime.replace(/\*/g, ".*"))
                        for each (f in formats) {
                            if (f.match(pat)) {
                                return f
                            }
                        }
                    } else if (formats.contains(m.mime)) {
                        return m.mime
                    }
                }
            }
            return formats[0]
        }

        /** 
            Set a transient flash notification message. Flash messages persist for only one request and are a convenient
                way to pass state information or feedback messages to the next request. To use flash messages, 
                setupFlash() and finalizeFlash() must be called before and after the request is processed. HttpServer.process
                will call setupFlash and finalizeFlash automatically.
            @param key Flash message key
            @param msg Message to store
         */
        function notify(key: String, msg: String): Void {
            if (!flash) {
                createSession()
                flash = {}
            }
            flash[key] = msg
        }

        /** 
            @duplicate Stream.off 
         */
        native function off(name, observer: Function): Void

        /** 
            @duplicate Stream.on
            @event readable Issued when some body content is available.
            @event writable Issued when the connection is writable to accept body data (PUT, POST).
            @event close Issued when the request completes
            @event error Issued if the request does not complete or the connection disconnects. An error event is not 
                caused by non-200 status codes, these are regarded as valid return results. Rather, an error event will
                be issued when the request cannot return a complete, valid Http response to the client.
            All events are called with the signature:
            function (event: String, http: Http): Void
         */
        native function on(name, observer: Function): Request

//  MOB - should there be a blocking read option?
        /** 
            @duplicate Stream.read
            If the request is posting a form, i.e. the Http ContentType header is set to 
            "application/x-www-form-urlencoded", then the request object will not be created by the HttpServer until
            all the form data is read and the $params collection created and populated with the form data. This permits form
            data to be processed synchronously without having to use async/observer techniques to respond to readable
            events. With all other content types, the Request object will be created and run before the incoming client 
            data has been read. To read data in these situations, register an observer function to run when the
            connection becomes "readable".
            @example:
                request.on("readable", function(event, request) {
                    var data = new byteArray
                    if (read(data)) {
                        print("Got " + data)
                    } else {
                        //  End of input, call finalize() when all output has been written
                        request.finalize()
                    }
                })
         */
        native function read(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /** 
            Redirect the client to a new URL. This call redirects the client's browser to a new target specified 
            by the $url.  Optionally, a redirection code may be provided. Normally this code is set to be the HTTP 
            code 302 which means a temporary redirect. A 301, permanent redirect code may be explicitly set.
            @param target Uri to redirect the client toward. This can be a relative or absolute string URI or it can be
                a hash of URI components. For example, the following are valid inputs: ../index.ejs, 
                http://www.example.com/home.html, \@list.
            @param status Optional HTTP redirection status
         */
        function redirect(target: *, status: Number = Http.MovedTemporarily): Void {
            this.status = status
            target = link(target).complete(uri)
            setHeader("Location", target)
            write("<!DOCTYPE html>\r\n" +
                   "<html><head><title>Redirect (" + status + ")</title></head>\r\n" +
                    "<body><h1>Redirect (" + status + ")</h1>\r\n" + 
                    "<p>The document has moved <a href=\"" + target + 
                    "\">here</a>.</p>\r\n" +
                    "<address>" + server.software + " at " + host + " Port " + server.port + 
                    "</address></body>\r\n</html>\r\n")
        }

        /**
            Get a security token to help mitigate CSRF threats. The security token is submitted by forms and requests and
            can be validated by controllers. The token is stored in session["__ejs-security-token__"]. 
         */
        function get securityToken(): Object {
            session[SecurityTokenName] ||= md5(Math.random()) 
            return session[SecurityTokenName]
        }

        /** 
            Define a cookie header to send with the response. The Path, domain and expires properties can be set to null for 
                default values.
            @param name Cookie name
            @param options Cookie field options
            @options value Cookie value
            @options path Uri path to which the cookie applies
            @options domain String Domain in which the cookie applies. Must have 2-3 "." and begin with a leading ".". 
                For example: domain: .example.com
            @options expires Date When the cookie expires
            @options secure Boolean Set to true if the cookie only applies for SSL based connections
         */
        function setCookie(name: String, options: Object) {
            options.path ||= "/"
            let cookie = Uri.encodeComponent(name) + "=" + options.value
            cookie += "; path=" + options.path
            if (options.domain)
                cookie += "; domain=" + options.domain
            if (options.expires)
                cookie += "; expires= " + options.expires.toUTCString()
            if (options.secure)
                cookie += "; secure"
            setHeader("Set-Cookie", cookie)
            setHeader("Cache-control", "no-cache=\"set-cookie\"")
        }

        /** 
            Convenience routine to set a Http response header in $responseHeaders. If a header has already been 
            defined and $overwrite is true, the header will be overwritten. NOTE: case is ignored in the header keyword.
            Access $responseHeaders to inspect the proposed response header set.
            @param key The header keyword for the request, e.g. "accept".
            @param value The value to associate with the header, e.g. "yes"
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        native function setHeader(key: String, value: String, overwrite: Boolean = true): Void

        /**
            Convenience routine to set multiple Http response headers in $responseHeaders. Access $responseHeaders to 
            inspect the proposed response header set.
            @param headers Set of headers to use
            @param overwrite If the header is already defined and overwrite is true, then the new value will
                overwrite the old. If overwrite is false, the new value will be catenated to the old value with a ", "
                separator.
         */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void {
            for (let [key,value] in headers) {
                setHeader(key, value, overwrite)
            }
        }

        /**
            Update the request resource limits. The supplied limit fields are updated.
            See the $limits property for limit field details.
            @param limits Object hash of limit fields and values
            @see limits
         */
        native function setLimits(limits: Object): Void

        /**
            Convenience routine to define an application at a given Uri prefix and directory location. This is typically
                called from routing tables. This sets the $pathInfo, $scriptName and $dir properties.
            @param prefix The leading Uri prefix for the application. This prefix is removed from the pathInfo and the
                $scriptName property is set to the prefix. The script name should begin with "/".
            @param location Path to where the application home directory is. This sets the $dir property to the $location
                argument.
        */
        function setLocation(prefix: String, location: Path): Void {
            prefix = prefix.trimEnd("/")
            pathInfo = pathInfo.trimStart(prefix)
            scriptName = prefix
            dir = location
        }

        /** 
            Convenience routine to set the (proposed) Http response status code. This is equivalent to assigning 
            to the $status property.
         */
        function setStatus(status: Number): Void
            this.status = status

        /**
            Prepare the flash message area. This copies flash messages from the session state store into the flash store.
         */
        function setupFlash() {
            if (sessionID) {
                lastFlash = null
                flash = session["__flash__"]
                if (flash) {
                    session["__flash__"] = undefined
                    lastFlash = flash.clone()
                } else {
                    flash = null
                }
            }
        }

        /** 
            Dump objects for debugging
            @param args List of arguments to print.
            @hide
         */
        function show(...args): Void {
            for each (var e: Object in args) {
                write(serialize(e, {pretty: true}) + "\r\n")
            }
        }

        /**
            @hide
         */
        function showRequest(): Void {
            write("request: {\r\n") 
            for (let [key,value] in this) {
                if (key == "server" || key == "config") {
                    continue
                }
                write("  " + key + ": " +  serialize(value, {pretty: true}) + ",\r\n")
            }
            write("}\r\n")
        }

        /**
            Configure tracing for this request. Tracing is initialized by the owning HttpServer and is typically
            defined to trace the first line of requests and responses at level 2, headers at level 3 and body content
            at level 4. Once the request has been created however, the first line and headers of the request are 
            already parsed and traced by the HttpServer, so modifying the trace level via trace() on the request object
            will only impact the tracing of response body content.
            
            The options argument contains optional properties: rx and tx 
            (for receive and transmit tracing). The rx and tx properties may contain an object hash which describes 
            the tracing for that direction and includes any of the following fields:
            @param options. Set of trace options with properties "rx" and "tx" for receive and transmit direction tracing.
                The include property is an array of file extensions to include in tracing.
                The include property is an array of file extensions to exclude from tracing.
                The all property specifies that everything for this direction should be traced.
                The conn property specifies that new connections should be traced. (Rx only)
                The first property specifies that the first line of the request should be traced.
                The headers property specifies that the headers (including first line) of the request should be traced.
                The body property specifies that the body content of the request should be traced.
                The size property specifies a maximum body size in bytes that will be traced. Content beyond this limit 
                    will not be traced.
            @option tx. Object hash with optional properties: include, exclude, first, headers, body, size.
            @option rx. Object hash with optional properties: include, exclude, conn, first, headers, body, size.
            @example:
                trace({
                    tx: { exclude: ["gif", "png"], "first": 2, "headers": 3, "body": 4, size: 1000000 }
                    rx: { "conn": 1, "first": 2, "headers": 3 , "body": 4, size: 1024 }
                })
          */
        native function trace(options: Object): Void

        /** 
            Set a warning flash notification.
            Flash messages persist for only one request and are a convenient way to pass state information or 
            feedback messages to the next request. To use flash messages, setupFlash() and finalizeFlash() must 
            be called before and after the request is processed. HttpServer.process will call setupFlash and finalizeFlash 
            automatically.
            @param msg Message to store
         */
        function warn(msg: String): Void
            notify("warn", msg)

        /** 
            Write data to the client. This will buffer the written data until either flush() or finalize() is called. 
            @duplicate Stream.write
         */
        native function write(...data): Number

//  MOB - add this to ByteArray, Http, Socket (not to Stream)
        /** 
            Write a block of data to the client. This will buffer the written data which will be flushed when either 
            close(), flush() or finalize() is called or the underlying pipeline is full. 
            @param buffer Destination byte array for read data.
            @param offset Offset in the byte array from which to write. If the offset is -1, then data is
                written from the buffer read $position which is then updated. 
            @param count Read up to this number of bytes. If -1, write all available data in the buffer. 
            @returns a count of the bytes actually written. Returns null on EOF.
            @event writable Issued when the connection can absorb more data.

         */
        # FUTURE
        native function writeBlock(buffer: ByteArray, offset: Number = 0, count: Number = -1): Number?

        /**
MOB - DEBUG
            Write content based on the requested accept mime type
            @param data Data to send to the client
            @hide
         */
        function writeContent(data): Void {
            let mime = matchContent("application/json", "text/html", "application/xml", "text/plain")
            setHeader("Content-Type", mime)
            switch (mime) {
            case "application/json":
                write(serialize(data, {pretty: true}) + "\n")
                break
            case "application/xml":
                write("XML\n")
                break
            case "text/html":
                if (controller) {
                    controller.writeView("edit")
                } else {
                    write("HTML\n")
                }
                break
            default:
            case "text/plain":
                write("PLAIN " + serialize(data, {pretty: true}) + "\n")
                break
            }
        }

        /** 
            Write an error message back to the user and finalize the request.  The output is Html escaped for security.
            @param status Http status code
            @param msgs Messages to send with the response. The messages may be modified for readability if it 
                contains an exception backtrace.
         */
        function writeError(status: Number, ...msgs): Void {
            if (!finalized) {
                this.status = status || Http.ServerError
                let msg = msgs.join(" ").replace(/.*Error Exception: /, "")
                let title = "Request Error for \"" + pathInfo + "\""
                if (config.log.showErrors) {
                    let text = "<pre>" + escapeHtml(msg) + "</pre>\r\n" +
                        '<p>To prevent errors being displayed in the browser, ' + 
                        'set <b>log.showErrors</b> to false in the ejsrc file.</p>\r\n'
                    try {
                        setHeader("Content-Type", "text/html")
                        write(errorBody(title, text))
                    } catch {}
                }
                finalize()
                log.debug(4, "Request error (" + status + ") for: \"" + uri + "\". " + msg)
            }
        }

        /**
            Send a static file back to the client. This is a high performance way to send static content to the client
            by using the "sendConnector". To be effective, this call must be invoked prior to sending any data or 
            headers to the client, otherwise it will be ignored and the slower "netConnector" will be used instead.
            @param file Path to the file to send back to the client
            @return True if the Send connector can successfully be used. 
         */
        native function writeFile(file: Path): Boolean

        /** 
            Send a response to the client. This can be used instead of setting status and calling setHeaders() and write(). 
            The $response argument is an object hash containing status, headers and
            body properties. The respond method replaces previously defined status and headers.
            @option status Numeric Http status code (e.g. 200 for a successful response)
            @option header Object hash of Http headers
            @option body Body content
        */
        function writeResponse(response: Object): Void {
            status = response.status || 200
            if (response.headers) {
                setHeaders(response.headers)
            }
            if (response.body) {
                write(response.body)
            }
            autoFinalize()
        }

        /** 
            Write safely. Write HTML escaped data back to the client.
            @param data Objects to HTML encode and write back to the client.
         */
        function writeSafe(...data): Number
            write(html(...data))

        /**
            The number of bytes written to the client. This is the count of bytes passed to $write and buffered, 
            not the actual number of bytes sent to the network connection.
            @return
         */
        native function get written(): Number

        /********************************************** JSGI  ********************************************************/
        //   MOB - Deprecate
        /** 
            JSGI specification configuration object.
            @spec jsgi-0.3
         */
        static var jsgi: Object = {
            errors: App.log,
            version: [0,3],
            multithread: true,
            multiprocess: false,
            runonce: false,
        }

        /** 
            Storage for middleware specific state. Added for JSGI compliance.
            @spec jsgi-0.3
         */
        native enumerable var env: Object

        /**
            Request content stream. This is equivalent to using "this" as Request objects are streams connected to the
            input content. Added for JSGI compliance.
            @spec jsgi-0.3
            @returns Stream object equal to the value of "this" request instance.
        */
        function get input(): Stream
            this

        /** 
            Decoded query string (URL query string). Eqivalent to the $query property. Added for JSGI compliance
            @spec jsgi-0.3
            @return A string containing the request query. Returns an empty string if there is no query.
         */
        function get queryString(): String
            query

        /**
            Listening port number for the server
            @returns A number set to the TCP/IP port for the listening socket.
         */
        function get serverPort(): Number
            server.port

        /*************************************** Deprecated ***************************************/

//  MOB - remove all this legacy stuff
        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get accept(): String
            header("accept")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get acceptCharset(): String
            header("accept-charset")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get acceptEncoding(): String
            header("accept-encoding")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get authAcl(): String? {
            throw new Error("Not supported")
            return null
        }

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get body(): String {
            let data = new ByteArray
            while (read(data));
            return data
        }

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get connection(): String
            header("connection")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get hostName(): String
            host

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get mimeType(): String
            header("content-type")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get pathTranslated(): String
            dir.join(pathInfo)

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get pragma(): String
            header("pragma")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get remoteHost(): String
            header("host")

        /** 
            @hide
            @deprecated 2.0.0
          */
        # Config.Legacy
        function get url(): String
            pathInfo

        /** 
            Get the name of the client browser software set in the "User-Agent" Http header 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get userAgent(): String
            header("user-agent")

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function writeHtml(...args): Void
            writeSafe(...args)
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Router.es"
 */
/************************************************************************/

/**
    Router.es - Web Request router. Route incoming client HTTP requests.
 */

# Config.WEB
module ejs.web {

    /** 
        The Router class manages incoming HTTP requests to the appropriate location application for servicing. 
        The Route class supports configurable user-defined routes. Each application should create a Router 
        instance and then attach matching routes.

        The Router works by defining routes in a route table. For rapid routing, routes are grouped into sets of 
        routes with the same leading URI path segment. For example: the route template "/User/login" would be put into
        the "User" route set. If a route template is a function or regular expression, the route is added to the "Global"
        route set.
        
        The $ejs.web::Request.pathInfo and other Request properties are examined when selecting a matching route. 
        The request's leading URI pathInfo segment is used to select a route set and then the request is matched 
        against each route in that set. Routes are matched in the order in which they are defined.

        @example:
        var r = new Router
        
        //  Match /some/path and run myCustomApp to generate a response. 
        //  Target is data for myCustomApp.
        r.add("/some/path", {response: myCustomApp, target: "/other/path"})

        //  Match /User/register and run MvcApp with controller == User and 
        //  action == "register"
        r.add("\@/User/register")

        //  Add route for files with a ".es" extension and use the ScriptApp 
        //  to generate the response
        r.add(/\.es$/i, {response: ScriptApp})

        //  Add route for directories and use the DirApp to generate the response
        r.add(Router.isDir, {name: "dir", response: DirApp})

        //  Add routes for RESTful routes for URIs starting with "/User" and 
        //  respond using MvcApp
        r.addResources("User")

        //  Manually create restful routes using the given URI template patterns
        r.add("/{controller}",           {action: "create", method: "POST"})
        r.add("/{controller}/init",      {action: "init"})
        r.add("/{controller}",           {action: "index"})
        r.add("/{controller}/{id}/edit", {action: "edit"})
        r.add("/{controller}/{id}",      {action: "show"})
        r.add("/{controller}/{id}",      {action: "update", method: "PUT"})
        r.add("/{controller}/{id}",      {action: "destroy", method: "DELETE"})
        r.add("/{controller}(/do/{action})")
        
        //  Add route for upper or lower case "D" or "d". Run the default app: MvcApp, 
        //  Dash contoller, refresh action.
        r.add("/[Dd]ash/refresh", "\@Dash/refresh")

        //  Add route for an "admin" application. This sets the scriptName to "admin" 
        //  and expects an application to be located at the directory "myApp"
        r.add("/admin/", {location: { scriptName: "/control", dir: "my"})

        //  Rewrite a request for "old.html" to new.html
        r.add("/web/old.html", 
            {rewrite: function(request) { request.pathInfo = "/web/new.html"}})  

        //  Handle a request with a literal response
        r.add("/oldStuff/", {response: {body: "Not found"} })

        //  Handle a request with an inline function
        r.add("/oldStuff/", {response: function(request) { return {body: "Not found"} }})

        //  A custom matching function to match SSL requests
        r.add(function (request) {
            if (request.scheme == "https") {
                request.params["security"] = "high"
                return true
            }, {
                name: "secure", action: "private" 
            }
        })

        //  A matching function that rewrites a request and then continues matching other routes
        r.add(function (request) {
            if (request.uri.startsWith("/old")) {
                request.uri = request.uri.toString().trimStart("/old")
                return false
            }
        })

        //  Route based on header values
        r.add(function (request) {
            if (request.header("user-agent").contains("Chrome")) {
                return true
            }
        })

        //  Set request parameters with values from request
        r.add("/custom", {action: "display", params: { from: "{uri}", transport: "{scheme}" })

        //  Nest matching routes
        let outer = r.add("/blog", {target: "/post/index"})
        r.add("/comment", {target: "/comment/{action}/{id}", outer: outer})

        //  Match with regular expression. The sub-match is available via $N parameters
        r.add(/^\/Dash-((Mini)|(Full))$/, 
            {controller: "post", action: "list", params: {kind: "$1"}}
        )
        
        //  Conditional matching. Surround optional tokens in "()"
        r.add("/Dash(/{product}(/{branch}(/{configuration})))", {   
            name: "dash", 
            method: "POST", 
            controller: "Dash", 
            action: "index",
        })

        //  Replace the home page route
        r.addHome("/Status")

        //  Display the route table to the console
        r.show()

        @stability prototype
        @spec ejs
     */
    class Router {

        /**
            Symbolic constant for Router() to add top-level routes for directory, *.es, *.ejs and a catchall 
            route for static pages. 
            Use of this constant will not add routes for MVC content or RESTful resources. 
         */ 
        public static const Top: String = "top"

        /**
            Max calls to route() per request
         */
        static const MaxRoute = 20

        /**
            Symbolic constant for Router() to add top-level routes for directory, *.es, *.ejs, generic routes for
            RESTful resources and a catchall route for static pages
         */ 
        public static const Restful: String = "restful"

        /**
            Default application to use when unspecified by a route
         */
        public var defaultApp: Function = MvcApp

        /* Router options provide to constructor */
        private var routerOptions: Object

        /**
            Routes indexed by first component of the URI path/template
         */
        public var routes: Object = {}
        
        /**
            Function to test if the Request.filename is a directory.
            @param request Request object to consider
            @return True if request.filename is a directory
         */
        public static function isDir(request) request.filename.isDir

        /**
            Add a catch-all route for static content
         */
        public function addCatchall(): Void
            add(/^\/.*$/, {name: "catchall", response: StaticApp, method: "*"})

        /**
            Add a default MVC controller/action route. This consists of a "/{controller}/{action}" route.
            All HTTP method verbs are supported.
         */
        public function addDefault(response: Object): Void
            add("/{controller}(/{action}(/.*))", {name: "default", method: "*", response: response})

        /**
            Add routes to handle static content, directories, "es" scripts and stand-alone ejs templated pages.
         */
        public function addHandlers(): Void {
            let staticPattern = "\/" + (App.config.dirs.static.basename || "static") + "\/.*"
            //  MOB - why test here?
            if (staticPattern) {
                add(staticPattern, {name: "default", response: StaticApp})
            }
/*  FUTURE
            add(/\.html$|\.css$|\.jpg$|\.gif$|\.png$|\.ico$|\.css$|\.js$/i,  {name: "static",  response: StaticApp})
 */
            add(/\.es$/i,  {name: "es",  response: ScriptApp, method: "*"})
            add(/\.ejs$/i, {name: "ejs", module: "ejs.template", response: TemplateApp, method: "*"})
            add(isDir,    {name: "dir", response: DirApp})
        }

        /**
            Add a home page route. This will add or update the "home" page route.
            @param target Target to invoke when the home page is accessed.
         */
        public function addHome(target: Object): Void
            add("/", { name: "home", target: target})

        /**
            Add restful routes for a singleton resource. 
            Supports member CRUD actions: edit, show, update, and custom actions.
            The restful routes defined are:
            <pre>
                Method  URL                   Action
                GET     /controller/edit      edit        Display a resource form suitable for editing
                GET     /controller           show        Display a resource (not editable)
                PUT     /controller           update      Update a resource (idempotent)
                ANY     /controllers/action   *           Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
            @param name Name of the resource to route. Can also be an array of names.
         */
        public function addResource(name: *): Void {
            if (name is Array) {
                for each (n in name) {
                    addResource(n)
                }
                return
            } 
            add('/' + name + "/edit",      {controller: name, action: "edit"})
            add('/' + name,                {controller: name, action: "show"})
            add('/' + name,                {controller: name, action: "update", method: "PUT"})
            add('/' + name + "/{action}",  {controller: name, name: "default",  method: "*"})
        }

        /** 
            Add restful routes for a resource collection. 
            Supports CRUD actions: edit, index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET     /controller             index       Display an overview of the resource
                GET     /controller/init        init        Initialize and display a blank form for a new resource
                POST    /controller             create      Accept a form creating a new resource

                GET     /controller/1/edit      edit        Display a resource form suitable for editing
                GET     /controller/1           show        Display a resource (not editable)
                PUT     /controller/1           update      Update a resource (idempotent)
                DELETE  /controller/1           destroy     Destroy a resource (idempotent)

                ANY     /controller/action      default     Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
            @param name Name of the resource to route. Can also be an array of names.
        */
        public function addResources(name: *): Void {
            if (name is Array) {
                for each (n in name) {
                    addResources(n)
                }
                return
            } 
            add('/' + name + "/init",       {controller: name, action: "init"})
            add('/' + name,                 {controller: name, action: "index"})
            add('/' + name,                 {controller: name, action: "create", method: "POST"})

            let id = {id: "[0-9]+"}
            add('/' + name + "/{id}/edit",  {controller: name, action: "edit", constraints: id})
            add('/' + name + "/{id}",       {controller: name, action: "show", constraints: id})
            add('/' + name + "/{id}",       {controller: name, action: "update", , constraints: id, method: "PUT"})
            add('/' + name + "/{id}",       {controller: name, action: "destroy", , constraints: id, method: "DELETE"})

            add('/' + name + "/{action}",   {controller: name, name: "default", method: "*"})
        }

        /** 
            Add default restful routes for resources. This adds default routes for generic resources.
            Supports CRUD actions: edit, index, show, create, update, destroy. The restful routes defined are:
            <pre>
                Method  URL                     Action
                GET     /controller             index       Display an overview of the resource
                GET     /controller/init        init        Initialize and display a blank form for a new resource
                POST    /controller             create      Accept a form creating a new resource

                GET     /controller/1/edit      edit        Display a resource form suitable for editing
                GET     /controller/1           show        Display a resource (not editable)
                PUT     /controller/1           update      Update a resource (idempotent)
                DELETE  /controller/1           destroy     Destroy a resource (idempotent)

                ANY     /controller/action      default     Other custom actions
            </pre>
            The default route is used when constructing URIs via Request.link.
        */
        public function addRestful(): Void {
            add("/{controller}/init",               {action: "init"})
            add("/{controller}",                    {action: "index"})
            add("/{controller}",                    {action: "create", method: "POST"})

            let id = {id: "[0-9]+"}
            add("/{controller}/{id}/edit",          {action: "edit", constraints: id})
            add("/{controller}/{id}",               {action: "show", constraints: id})
            add("/{controller}/{id}",               {action: "update", constraints: id, method: "PUT"})
            add("/{controller}/{id}",               {action: "destroy", constraints: id, method: "DELETE"})

            //  Same as addDefault()
            add("/{controller}(/{action})",         {name: "default", method: "*"})
        }

        /**
            Create a Router instance and initialize routes.
            @param routeSet Optional name of the route set to add. Supports sets include:
                Router.Top and Router.Restful. The Top routes provide top level routes for pages with extensions ".ejs", 
                and ".es" as well as for static content (see $addHandlers, $addCatchall). The Restful routes provide 
                default Controller/Action routes according to a RESTful paradigm (see $addRestful). The routeSet can
               also be set to null to add not routes. This is useful for creating a bare Router instance. Defaults 
               to Top.
            @param options Options to apply to all routes
            @option workers Boolean If true, requests should be execute in a worker thread if possible. The worker thread 
                will be pooled when the request completes and will be available for subsequent requests.  
           @throws Error for an unknown route set.
         */
        function Router(routeSet: String? = Restful, options: Object = {}) {
            routerOptions = options
            switch (routeSet) {
            case Top:
                addHandlers()
                addDefault(StaticApp)
                addCatchall()
                break
            case Restful:
                addHome("@Base/")
                add("/favicon.ico", { redirect: "/static/images/favicon.ico" })
                addHandlers()
                addRestful()
                addCatchall()
                break
            case null:
                break
            default:
                throw new ArgError("Unknown route set: " + routeSet)
            }
        }

        private function insertRoute(r: Route): Void {
            let routeSet = routes[r.routeSetName] ||= {}
            routeSet[r.name] = r
            if (r.workers == null) {
                r.workers = routerOptions.workers
            }
        }

        /**
            Add a route
            @param template String or Regular Expression defining the form of a matching URI (Request.pathInfo).
                If options are not provided and the template arg is a string starting with "\@", the template is
                interpreted both as a URI and as providing the options. See $options below for more details.
            @param options Route options representing the URI and options to use when servicing the request. If it
                is a string, it may begin with a "\@" and be of the form "\@[controller/]action". In this case, if there
                is a "/" delimiter, the first portion is a controller and the second is the controller action to invoke.
                The controller or action may be absent. For example: "\@Controller/", "\@action", "\@controller/action".
                If the string does not begin with an "\@", it is interpreted as a literal URI. 
                For example: "/web/index.html". If the options is an object hash, it may contain the options below:
            @option action Action method to service the request if using controllers. This may also be of the form 
                "controller/action" to set both the action and controller in one property.
            @option constraints Object Object hash of properties whose values are constrained. The property names are
                the field names to be constrained and their values are regular expressions for which the actual URI
                values must match for the route to match.
            @option controller Controller to service the request.
            @option name Name to give to the route. If absent, the name is created from the controller and action names.
                The route naming rules are:
                1. Use options.name if provided, else
                2. Use any action name, else
                3. Use "index"
                The action name is sleuthed from the template if no options are given.
            @option outer Parent route. The parent's template and parameters are appended to this route.
            @option params Override parameter to provide to the request in the Request.params.
            @examples:
                Route("/{controller}(/{action}(/{id}))/", { method: "POST" })
                Route("/User/login", {name: "login" })
                Route("\@/User/login")
            @option name Name for the route
            @option method String|RegExp HTTP methods to support.
            @option limits Limits object for the requests on this route. See HttpServer.limits.
            @option location Application location to serve the request. Location contains two properties: scriptName 
                which is the string URI prefix for the application and dir which is a Path to the physical file system 
                directory containing the applciation.
            @option params Override request parameters.
            @option parent Outer parent route
            @option redirect Redirect requests on this route to this URI.
            @option response (Function|Object) This can be either a function to serve the request or it can be a response 
                hash with status, headers and body properties. The function should have this signature:
                    function (request: Request): Object
            @option rewrite Rewrite function. This can rewrite request properties.
            @option set Route set name in which to add this route. Defaults to the first component of the template if
                the template is a string, otherwise "".
            @option target Target for the route. This can be a Uri path or a controller/action pair: "\@[controller/]action".
            @example:
                r.add("/User/{action}", {controller: "User"})
         */
        public function add(template: Object, options: Object? = null): Route {
            let r = new Route(template, options, this)
            insertRoute(r)
            return r
        }

        /**
            Lookup a route by name. The route name is determined by the options provided when the route was created.
            Action names will be sleuthed from the template if no options provided.
            Outer routes are pre-pended if defined.
            @param options Route description. This can be either string or object hash. If it is a string, it should be
                of the form "controller/action". If the options is an object hash, it should have properties
                controller and action. The controller is used as the index for the route set. The action property is
                the index for the route name.
            @return Route object or null if the route is not found
         */
        public function lookup(options: Object): Route? {
            if (options is String) {
                if (options[0] == "@") {
                    options = options.slice(1)
                }
                if (options.contains("/")) {
                    let [controller, action] = options.split("/")
                    let routeSet = routes[controller]
                    if (routeSet) {
                        return routeSet[action]
                    }
                }
                return routes[""][options]
            }
            let controller = options.controller || ""
            let routeSet = routes[controller]
            let routeName = options.route || options.action || "default"
            return routeSet[routeName]
        }

        /**
            Remove a route
            @param name Name of the route to remove. Name should be of the form "controller/action" where controller
            is the index for the route set and action is the index for the route name.
         */
        public function remove(name: String): Void {
            let [controller, action] = name.split("/")
            let routeSet = routes[controller]
            for (let routeName in routeSet) {
                if (routeName == action) {
                    delete routeSet[action]
                    return
                }
            }
            throw "Cannot find route \"" + name + "\" to remove"
        }

        /**
            Reset the request routing table by removing all routes
         */
        public function reset(request): Void {
            routes = {}
        }

        private function reroute(request): Route {
            request.routed ||= 1
            if (request.routed++ > MaxRoute) {
                throw "Too many route calls. Route table may have a loop."
            }
            return route(request)
        }

        private function secondStageRoute(request: Request, r: Route): Route {
            let params = request.params
            let pathInfo = request.pathInfo
            let log = request.log

            for (field in r.params) {
                /*  Apply override params */
                let value = r.params[field]
                if (value.toString().contains("$")) {
                    value = pathInfo.replace(r.pattern, value)
                }
                if (value.toString().contains("{")) {
                    value = Uri.template(value, params, request)
                }
                params[field] = value
            }
            if (r.rewrite && !r.rewrite(request)) {
                log.debug(5, "Request rewritten as \"" + request.pathInfo + "\" (reroute)")
                return reroute(request)
            }
            if (r.redirect) {
                //  TODO OPT - could this this via a custom app function
                request.pathInfo = r.redirect
                log.debug(5, "Route redirected to \"" + request.pathInfo + "\" (reroute)")
                return reroute(request)
            }
            let location = r.location
            if (location && location.scriptName && location.scriptName != request.scriptName && location.dir) {
                request.setLocation(location.scriptName, location.dir)
                log.debug(4, "Set location scriptName \"" + location.scriptName + "\" dir \"" + 
                    location.dir + "\" (reroute)")
                return reroute(request)
            }
            if (r.module && !r.initialized) {
                global.load(r.module + ".mod")
                r.initialized = true
            }
            if (log.level >= 3) {
                log.debug(4, "Matched route \"" + r.routeSetName + "/" + r.name + "\"")
                if (log.level >= 5) {
                    log.debug(5, "  Route params " + serialize(params, {pretty: true}))
                }
                if (log.level >= 6) {
                    log.debug(6, "  Route " + serialize(r, {pretty: true}))
                    log.debug(6, "  REQUEST\n" + serialize(request, {pretty: true}))
                }
            }
            if (r.limits) {
                request.setLimits(r.limits)
            }
            if (r.trace) {
                if (r.trace.include && (!r.trace.include.contains(request.extension)) ||
                    r.trace.exclude && r.trace.exclude.contains(request.extension)) {
                    request.trace(99)
                } else {
                    request.trace(r.trace.level || 0, r.trace.options, r.trace.size)
                }
            }
            request.route = r
            return r
        }

        /** 
            Route a request. The request is matched against the configured route table. 
            The call returns the web application to execute.
            @param request The current request object
            @return The web application function of the signature: 
                function app(request: Request): Object
         */
        public function route(request): Route {
            let log = request.log
            log.debug(5, "Routing " + request.pathInfo)

            //  MOB - this is now done by http for "-http-method" and X-HTTP-METHOD-OVERRIDE
            if (request.method == "POST") {
                let method = request.params["-ejs-method-"] || request.header("X-HTTP-METHOD-OVERRIDE")
                if (method && method.toUpperCase() != request.method) {
                    log.debug(3, "Change method from " + request.method + " TO " + method + " for " + request.uri)
                    request.method = method
                }
            }
            let routeSet = routes[request.pathInfo.split("/")[1]]
            for each (r in routeSet) {
                log.debug(5, "Test route \"" + r.name + "\"")
                if (r.match(request)) {
                    return secondStageRoute(request, r)
                }
            }
            routeSet = routes[""]
            for each (r in routeSet) {
                log.debug(5, "Test route \"" + r.name + "\"")
                if (r.match(request)) {
                    return secondStageRoute(request, r)
                }
            }
            throw "No route for " + request.pathInfo
        }

        /**
            Set the default application function for the route
            @hide
         */
        public function setDefaultApp(app: Function): Void
            defaultApp = app

        //  MOB - rethink the "full" arg
        /**
            Show the route table
            @param extra Set to "full" to display extra route information
         */
        public function show(extra: String? = null): Void {
            let lastController
            for each (name in Object.getOwnPropertyNames(routes).sort()) {
                print("\n" + (name || "Global")+ "/")
                for each (r in routes[name]) {
                    showRoute(r, extra)
                }
            }
            print()
        }

        private function showRoute(r: Route, extra: String? = null): Void {
            let method = r.method || "*"
            let target
            let tokens = r.tokens
            let params = r.params || {}
            if (params.controller || params.action || 
                    (tokens && (tokens.contains("action") || tokens.contains("controller")))) {
                let controller = params.controller || "*"
                let action = params.action || "*"
                target = controller + "/" + action
            } else if (r.response) {
                if (r.response is Function) {
                    target = r.response.name
                } else {
                    target = "Response Object"
                }
            } else {
                target = "UNKNOWN"
            }
            let template = r.template
            if (template is String) {
                template = "%s  " + template
            } else if (template is RegExp) {
                template = "%r  " + template
            } else if (template is Function) {
                template = "%f  " + template.name
            } else if (!template) {
                template = "*"
            }
            let line = "  %-24s %s %-24s %-7s %s".format(r.name, r.workers ? "W": " ", target, method, template)
            if (extra == "full") {
                if (params && Object.getOwnPropertyCount(params) > 0) {
                    if (!(params.action && Object.getOwnPropertyCount(params) == 1)) {
                        line += "\n                                                      %s".format(serialize(params))
                    }
                }
                line += "\n                                                      pattern: " + r.pattern + "\n"
            }
            print(line)
        }
    }

    /** 
        A Route describes a mapping from a URI to a web application. A route has a URI template for matching with
        candidate request URIs and a serving function to respond to the request.

        If the URI template is a regular expression, it is used to match against the request pathInfo. If it matches,
        the pathInfo is matched and sub-expressions may be referenced in the override parameters by using $1, $2 and
        so on. e.g. { priority: "$1" }
        
        If the URL template is a function, it is run to test for a request match. It should return true to 
        accept the request. The function can set parameters in request.params.

        The optional override params hash provides parameters which will be defined in params[] overriding any tokenized 
        parameters previously set.
     */
    enumerable dynamic class Route {
        use default namespace public

        /* 
            Seed for generating route names 
         */
        private static var nameSeed: Number = 0

        /**
            Resource limits for the request. See HttpServer.limits for details.
         */
        var limits: Object

        /**
            Application location to serve the request. Location contains two properties: scriptName which is the string 
            URI prefix for the application and dir which is a Path to the physical file system directory containing 
            the applciation.
         */
        var location: Object

        /**
            HTTP method to match. If set to "" or "*", all methods are matched.
         */
        var method: String

        /**
            Middleware to run on requests for this route. Middleware wraps the application function filtering and 
            modifying its inputs and outputs.
         */
        var middleware: Array

        /**
            Route name. This is local to the route set (controller)
         */
        var name: String?

        /**
            Name of a required module containing code to serve requests on this route.  
         */
        var moduleName: String?

        /**
            Original template as supplied by caller
         */
        private var originalTemplate: Object

        /**
            Outer route for a nested route. A nested route prepends the outer route template to its template. 
            The param set of the outer route is appended to the inner route.
            @hide
         */
        var outer: Route

        /**
            Request parameters to add to the Request.params. This optional override hash provides parameters which will 
            be defined in Request.params[].
         */
        var params: Object

        /**
            Rewrite function to rewrite the request before serving. It may update the request scriptName, pathInfo 
            and other Request properties. Return true to continue serving the request on this route. Otherwise re-route
            after rewriting the request. 
         */
        var rewrite: Function?

        /**
            URI to redirect the request toward.
         */
        var redirect: String?

        /**
            Response object hash or function to serve the request
         */
        var response: Object

        /** 
          Router instance reference
         */
        var router: Router

        /**
            Route set owning the route. This is the first component of the template.
         */
        var routeSetName: String?

        /**
            Target mapping for the route. The route maps from the template to the target.
         */
        var target: String?

        /**
            Template pattern for creating links. The template is used to match the request pathInfo. The template can be a 
            string, a regular expression or a function. If it is a string, it may contain tokens enclosed in braces 
            "{}" and it is converted to a regular expression for fast matching. At run-time, request tokens 
            are extracted and stored as items in the Request.params collection.

            If the template is a regular expression, it is used to match against the request pathInfo. If it matches, 
            then sub-expressions may be referenced in the $params values by using $1, $2 and so on. 
            e.g. params: { priority: "$1" }
            
            If the template is a function, it is run to test for a request match. The function should return true to 
            match the request. The function can directly set parameters in request.params.
        */
        var template: Object

        /**
            If true, requests should execute using a worker thread if possible. The worker thread will be pooled when
            the request completes and be available for use by subsequent requests.
         */
        var workers: Boolean?

        /**
            Key tokens in the route template
         */
        var tokens: Array?

        /**
            Trace options for the request. Note: the route is created after the Request object is created so the tracing 
            of the connections and request headers will be controlled by the owning server. 
            See HttpServer.trace for trace property fields.
         */
        var trace: Object

        /*
            Match function
         */
        internal var match: Function

        /*
            Regular expression pattern. This matches the pathInfo for the route.
         */
        internal var pattern: Object

        /*
            Splitter. This is used as the replacement argument to extract tokens from the pathInfo
         */
        internal var splitter: String

        /**
            Create a new Route instance. This is normally not invoked directly. Rather Router.add() is used to
            create and install routes into the Router.
            @param template String or Regular Expression defining the form of a matching URI (Request.pathInfo).
            @param options Route options representing the URI and options to use when servicing the request. If it
                is a string, it may begin with an "\@" and be of the form "\@[controller/]action". In this case, if there
                is a "/" delimiter, the first portion is a controller and the second is the controller action to invoke.
                The controller or action may be absent. For example: "\@Controller/", "\@action", "\@controller/action".
                If the string does not begin with an "\@", it is interpreted as a literal URI. 
                For example: "/web/index.html". If the options is an object hash, it may contain the options below:
            @option action Action method to service the request. This may be of the form "action", "controller/action" or 
                "controller/".  If the action portion omitted, the default action (index) will be used.
            @option controller Controller to service the request.
            @option method HTTP Method for which the route is valid. Set to "*" for all methods.
            @option name Name to give to the route. If absent, the name is created from the controller and action names.
            @option outer Parent route. The parent's template and parameters are appended to this route.
            @option params Override parameter to provide to the request in the Request.params.
            @param router Owning router object
            @examples:
                Route("/{controller}(/{action}(/{id}))/", { method: "POST" })
                Route("/User/login", {name: "login" })
         */
        function Route(template: Object, options: Object, router: Router) {
            this.router = router
            this.template = template
            options = parseOptions(options)
            makeParams(options)
            inheritRoutes(options)
            compileTemplate(options)
            setName(options)
            setRouteSetName(options)
            setRouteProperties(options)
        }

        /**
            Get the template pattern for a route given a controller and a route name. If the specified controller 
            cannot be found, the Global route set is used. If the specified route name cannot be found, the "default"
            route is used. Use Uri.template to expand the template with URI components.
            @param controller Controller name
            @param routeName Route name to look for
            @return A template URI string
            @hide
         */
        public function getTemplate(controller: String~, routeName: String~): String {
            let routes = router.routes
            let routeSet = routes[controller] || routes[""]
            let route = routeSet[routeName] || routeSet["default"] || routes[""]["default"]
            return "/{scriptName}" + route.template
        }

        private function inheritRoutes(options: Object): Void {
            let parent = options.outer
            if (parent) {
                let ptem = (parent.originalTemplate is RegExp) ? parent.originalTemplate.source : parent.originalTemplate
                let tem = (template is RegExp) ? template.source : template
                if (template is RegExp) {
                    template = RegExp(ptem + tem)
                } else {
                    template = ptem + tem
                }
                for (p in parent.params) {
                    params[p] ||= parent.params[p]
                }
            }
            this.originalTemplate = template
        }

        //  MOB - see ESP. It does this a better way

        private function compileTemplate(options: Object): Void {
            if (template is String) {
                let t = template
                /*  
                    For string templates, Create a regular expression splitter template so :TOKENS can be referenced
                    positionally in the override hash via $N args.
                    Allow () expressions, these are made into non-capturing parentheses.
                 */
                if (t.contains("(")) {
                    t = t.replace(/\(/g, "(?:")
                    t = t.replace(/\)/g, ")?")
                }
                tokens = t.match(/\{([^\}]+)\}/g)
                for (i in tokens) {
                    tokens[i] = tokens[i].trimStart('{').trimEnd('}')
                }
                let constraints = options.constraints
                for each (token in tokens) {
                    if (constraints && constraints[token]) {
                        t = t.replace("{" + token + "}", "(" + constraints[token] + ")")
                    } else {
                        t = t.replace("{" + token + "}", "([^/]*)")
                    }
                } 
                //  MOB - is this required?
                t = t.replace(/\//g, "\\/")
                pattern = RegExp("^" + t + "$")
                /*  Splitter ends up looking like "$1:$2:$3:$4" */
                let count = 1
                if (!splitter) {
                    splitter = ""
                    for (c in tokens) {
                        splitter += "$" + count++ + ":"
                    }
                    splitter = splitter.trim(":")
                }
                match = matchAndSplit
                template = template.replace(/[\(\)]/g, "").replace(/\/\.\*/g, "")
            } else {
                if (template is Function) {
                    match = template
                } else if (template is RegExp) {
                    pattern = template
                    match = matchRegExp
                } else if (template) {
                    pattern = RegExp(template.toString())
                    match = matchRegExp
                }
            }
        }

        /*
            Match a request and apply splitter to create request params
         */
        private function matchAndSplit(request: Request): Boolean {
            if (method && !request.method.contains(method)) {
                return false
            }
            let pathInfo = request.pathInfo
            if (!pathInfo.match(pattern)) {
                return false
            }
            let parts = pathInfo.replace(pattern, splitter).split(":")
            for (i in tokens) {
                request.params[tokens[i]] ||= parts[i].trimStart("/")
            }
            return true
        }

        /*
            Match a request using a regular expression without splitter
         */
        private function matchRegExp(request: Request): Boolean~ {
            if (method && !request.method.contains(method)) {
                return false
            }
            return request.pathInfo.match(pattern)
        }

        /*
            Make the params object based on input options
         */
        private function makeParams(options: Object): Void {
            params = options.params || {}
            if (options.action) {
                params.action = options.action
            }
            if (options.controller) {
                params.controller = options.controller
            }
            if (options.namespace) {
                params.namespace = options.namespace
            }
        }

        /*
            If no options provided, sleuth the action from the template. This will probably also end up setting 
            the name to the action component
         */
        private function parseOptions(options: Object): Object {
            if (!options) {
                if (template is String) {
                    if (template[0] == '@') {
                        let t = template.replace(/[\(\)]/g, "")
                        options = {target: t.split("{")[0]}
                        template = template.trimStart("@")
                    }
                }
                options ||= {}
            } else if (options is String) {
                options = {target: options}
            }
            target = options.target
            if (target) {
                if (target[0] == '@') {
                    if (target.contains("/")) {
                        [options.controller, options.action] = target.slice(1).trimStart("/").split("/")
                    } else {
                        options.action = target.slice(1)
                    }
                }
            }
            if (options.action) {
                if (options.action.contains("/")) {
                    [options.controller, options.action] = options.action.trimStart("/").split("/")
                } 
                if (options.action.contains("::")) {
                    [options.namespace, options.action] = options.action.split("::")
                }
            }
            if (options.middleware) {
                middleware = options.middleware.reverse()
            }
            return options
        }

        /*
            Create a useful (deterministic) name for the route. Rules are:
            1. Use options.name if provided, else
            2. Use any action name, else
            3. Use "default"

            Action names will be sleuthed from the template if no options provided.
            Outer routes are pre-pended if defined.
         */
        private function setName(options: Object): Void {
            name = options.name
            if (!name && options.action) {
                name = options.action
            }
            if (!name) {
                if (template is String) {
                    /* Take second component */
                    name = template.split("{")[0].split("/")[2]
                } else if (template is Function) {
                    name = template.name
                }
            }
            name ||= "default"
            if (outer && !options.name) {
                name = options.name + "/" + name
            }
            if (!name) {
                throw "Route has no name defined"
            }
        }

        private function setRouteSetName(options: Object): Void {
            if (options) {
                routeSetName = options.set
            }
            if (!routeSetName && template is String) {
                routeSetName = template.split("{")[0].split("/")[1]
            }
            routeSetName ||= ""
        }

        private function setRouteProperties(options: Object): Void {
            limits = options.limits
            linker = options.linker
            location = options.location
            moduleName = options.module
            rewrite = options.rewrite
            redirect = options.redirect
            workers = options.workers
            trace = options.trace
            if (options.method == "" || options.method == "*") {
                method = options.method = ""
            } else {
                method = options.method || "GET"
            }
            options.response ||= router.defaultApp
            if (!(options.response is Function)) {
                response = function (request) {
                    return options.response
                }
            } else {
                response = options.response
            }
        }

    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Script.es"
 */
/************************************************************************/

/*
    Script.es -- Ejscript templated web content handler
 */

# Config.WEB
module ejs.web {

    /**
        Run a script app. Run the script specified by request.filename
        @param request Request object
        @return A response object
        @example:
          { name: "index", builder: ScriptBuilder, match: "\.es$" }
        @spec ejs
        @stability prototype
     */
    function ScriptApp(request: Request) {
        let app = ScriptBuilder(request)
        return app.call(request, request)
    }


    //  MOB -- make all builder functions lower case: scriptBuilder -- or inline above
    /** 
        Script builder to create a function to serve a script request (*.es).  
        @param request Request object. 
        @return A request response
        @spec ejs
        @stability prototype
     */
    function ScriptBuilder(request: Request): Object {
        if (!request.filename.exists) {
            request.writeError(Http.NotFound, "Cannot find " + escapeHtml(request.pathInfo)) 
            //  MOB - should not need throw, just return
            throw true
        }
        try {
            return Loader.require(request.filename, request.config).app
        } catch (e) {
            request.writeError(Http.ServerError, e)
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Session.es"
 */
/************************************************************************/

/**
    Session.es -- Session state management
 */

# Config.WEB
module ejs.web {
    /** 
        Session state storage class. 
        The session state objects can be used to share state between requests. The $ejs.web::Request object has
        a $ejs.web::Request.session property that stores a Session object instance. Values stored into the session 
        state are serialized and deserialized upon retrieval.
        @spec ejs
    */
    dynamic class Session { 
        use default namespace public

        /**
            Create a new session state object. This uses the Store in-memory caching service.
            @param key Optional key to provide. If omitted or null, a new key will be generated. If the given key has
                expired, a new key will be generated.
            @param options
            @option lifespan Lifespan in seconds for the session
         */
        native function Session(key, options: Object? = null)

        /**
            Get the session ID key for a session. This is a static method so as to not clash with session properties.
            @param session Session to examine
            @return The session ID key.
         */
        native static function key(session: Session!): String

        /**
            Destroy a session
            @param session Session to destroy
         */
        native static function destorySession(session: Session!): Void
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/ShowExceptions.es"
 */
/************************************************************************/

/*
    Show exceptions to the client 
 */

# Config.WEB
module ejs.web {
    /** 
        ShowExceptions middleware wrapper. This catches exceptions and formats the result back to the client.
        @param app Application servicing the request and generating the response.
        @return A web application function that services a web request and when invoked with the request object will 
            yield a response object.
     */
    function ShowExceptions(app: Function): Function {
        return function(request: Request) {
            try {
                return app(request)
            } catch (e) {
                return {
                    status: Http.ServerError,
                    body: errorBody(typeOf(e), e.message + "\r\n" + e.formatStack()),
                }
            }
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Static.es"
 */
/************************************************************************/

/*
    Static.es - Static content handler
 */

# Config.WEB
module ejs.web {

    /** 
        Static content handler. This supports DELETE, GET, POST and PUT methods. It handles directory redirection
        and will use X-SendFile for efficient transmission of static content. The If-Match, If-None-Match,
        If-Modified-Since and If-Unmodified-Since headers are supported.
        @param request Request objects
        @returns A response hash object
        @example:
          { name: "index", builder: StaticApp }
        @spec ejs
        @stability prototype
     */
    function StaticApp(request: Request): Object {
        let filename = request.filename
        let status = Http.Ok, body
        let hdr

        let headers = {
            "Content-Type": Uri(request.uri).mimeType,
        }
        if (request.method != "PUT") {
            if ((encoding = request.header("Accept-Encoding")) && encoding.contains("gzip")) {
                let compressed = Path(filename + ".gz")
                if (compressed.exists) {
                    filename = request.filename = compressed
                    headers["Content-Encoding"] = "gzip"
                }
            }
            if (!filename.exists) {
                return {
                    status: Http.NotFound, 
                    body: errorBody("Not Found", "Cannot find " + escapeHtml(request.pathInfo))
                }
            }
        }
        let etag
        if (filename.exists) {
            etag = "%x-%x".format(filename.size, filename.modified)
            headers["ETag"] = etag
            headers["Last-Modified"] = filename.modified.toUTCString()
        }

        /*
            If a specified tag matches, then return the full resource
         */
        let match = false
        if (hdr = request.header("If-Match")) {
            for each (rtag in hdr.split(",")) {
                if (rtag == etag || (rtag == "*" && filename.exists)) {
                    match = true
                    break
                }
            }
            if (!match) {
                return { status: Http.PrecondFailed }
            }
        }
        /*
            If a specified etag matches, then no need to process the request. Respond with Not-Modified.
         */
        match = true
        if (hdr = request.header("If-None-Match")) {
            for each (rtag in hdr.split(",")) {
                if (rtag != etag || (rtag == "*" && !filename.exists)) {
                    match = false
                }
            }
            if (!match) {
                status = Http.PrecondFailed
                /* Keep going to allow If-Modified-Since to be analysed */
            }
        }

        /*
            If the resource has not been modified since, return Not-Modified
         */
        if (match && (when = request.header("If-Modified-Since"))) {
            if (request.method == "GET" || request.method == "HEAD") {
                if (filename.exists && filename.modified <= Date.parse(when) && !request.header("Range")) {
                    status = Http.NotModified
                }
            }
        }
        if (when = request.header("If-Unmodified-Since")) {
            if (!filename.exists || Date.parse(when) < filename.modified) {
                status = Http.PrecondFailed
            }
        }
        if (status == Http.NotModified &&
                (hdr = request.header("Cache-Control")) && (hdr.contains("max-age=0") || hdr.contains("no-cache"))) {
            status = Http.Ok
        }
        if (status != Http.Ok) {
            return { headers: headers, status: status }
        }
        let expires = request.config.web.expires
        if (expires) {
            let lifetime = expires[request.extension] || expires[""]
            if (lifetime) {
                headers["Cache-Control"] = "max-age=" + lifetime
                /* OPT - don't really need to do expires. Cache-Control should be sufficient */
                let when = new Date
                when.time += (lifetime * 1000)
                headers["Expires"] = when.toUTCString()
            }
        }
        if (request.method == "GET" || request.method == "POST") {
            headers["Content-Length"] = filename.size
            if (request.config.web.nosend) {
                body = File(filename, "r")
            } else {
                body = filename
            }

        } else if (request.method == "DELETE") {
            status = Http.NoContent
            try {
                if (!filename.remove()) {
                    status = Http.NotFound
                }
            } catch {
                status = Http.NotFound
            }

        } else if (request.method == "PUT") {
            request.dontAutoFinalize()
            return { body: put }

        } else if (request.method == "HEAD") {
            /* Just need the content length */
            headers["Content-Length"] = filename.size

        } else if (request.method == "OPTIONS") {
            headers["Allow"] = "OPTIONS,GET,HEAD,POST,PUT,DELETE"

        } else if (request.method == "TRACE") {
            // body = "TRACE " + request.pathInfo + " " + request.protocol + "\r\n"
            body = "<!DOCTYPE html>\r\n" +
                "<html><head><title>Trace Request Denied</title></head>\r\n" +
                "<body>The TRACE method is disabled on this server.</body>\r\n" +
                "</html>\r\n"
            status = Http.NotAcceptable

        } else {
            status = Http.BadMethod
            body = errorBody("Unsupported method ", "Method " + escapeHtml(request.method) + " is not supported")
        }
        return {
            status: status,
            headers: headers,
            body: body
        }

        /* Inline function to handle put requests */
        function put(request: Request) {
            let path = request.dir.join(request.pathInfo.trimStart('/'))
            request.status = path.exists ? Http.NoContent : Http.Created

            let file = new File(path, "w")
            file.position = 0;

            request.input.on("readable", function () {
                buf = new ByteArray
                if (request.read(buf)) {
                    file.write(buf)
                } else {
                    file.close()
                    request.finalize()
                }
            })
            request.input.on(["close", "complete", "error"], function (event, request) {
                if (event == "error") {
                    file.close()
                    file.remove()
                }
            })
        }
    }

    //  MOB -- rename to scriptBuilder
    /** 
        Static builder for use in routing tables to serve static file content.
        @param request Request object. 
        @return A web script function that services a web request for static content
        @spec ejs
        @stability prototype
     */
    function StaticBuilder(request: Request): Function {
        //  MOB - should not need "ejs.web"
        return "ejs.web"::StaticApp
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Template.es"
 */
/************************************************************************/

/*
    Template.es -- Ejscript templated web content handler
 */

# Config.WEB
module ejs.web {
    /** 
        Template middleware filter. This interprets the output of an inner web app as a template page which is processed.
        @param app Application function object
        @returns A response object hash
        @spec ejs
        @stability prototype
     */
    function TemplateFilter(app: Function): Object {
        return function(request) {
            let response = app(request)
            let id = md5(request.id)
            return Loader.load(id, id, request.config, function (id, path) {
                if (!global.TemplateParser) {
                    load("ejs.template.mod", {reload: false})
                }
                let data = TemplateParser().build(response.body)
                return Loader.wrap(id, data)
            }).app(request)
        }
    }

    /** 
        Template web page handler. The template web page at request.filename will be processed and run.
        @param request Request objects 
        @returns A response hash object
        @spec ejs
        @example: Example use in a Route table entry
          { name: "index", builder: TemplateApp, match: "\.ejs$" }
        @stability prototype
     */
    function TemplateApp(request: Request): Object {
        let app = TemplateBuilder(request)
        return app(request)
    }

    /** 
        Template builder. This routine creates a template function to serve requests for template files. 
        The template path can be supplied via the request.filename or a literal template can be provided via options.literal.
        @param request Request object. 
        @param options Object hash of options
        @options layout Path Layout file
        @options literal String containing the template to render. In this case request.filename is ignored.
        @options dir Path Base directory to use for including files and for resolving layout directives
        @return A web script function that services a web request.
        @spec ejs
        @stability prototype
     */
    function TemplateBuilder(request: Request, options: Object = {}): Function {
        let path
        if (!options.literal) {
            path = request.filename
            if (path && !path.exists) {
                request.writeError(Http.NotFound, "Cannot find " + escapeHtml(request.pathInfo))
                //  MOB - is this a generic need for a function like this?
                return function() {}
            }
        }
        return Loader.load(path, path, request.config, function (id, path) {
            if (!global.TemplateParser) {
                global.load("ejs.template.mod", {reload: false})
            }
            options.dir = path.dirname
            let data = options.literal || TemplateParser().build(path.readString(), options)
            return Loader.wrap(path, data)
        }).app
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/UploadFile.es"
 */
/************************************************************************/

/**
    uploadedFile.es - Description of an uploaded file. Instances are created and stored in Request.files.
 */

# Config.WEB
module ejs.web {

    /** 
        Instances of UploadFile are created for each uploaded file.
        The UploadFile instances are stored in the request.files property.
        Users should not create instances of UploadFile manually.
        @spec ejs
        @stability evolving
     */
    class UploadFile {

        use default namespace public

        /** 
            Name of the uploaded file given by the client
         */
        var clientFilename: String

        /** 
            Mime type of the encoded data
         */
        var contentType: String

        /** 
            Name of the uploaded file. This is a temporary file in the upload directory.
         */
        var filename: Path

        /** 
            HTML input ID for the upload file element
         */
        var name: String
 
        /** 
            Size of the uploaded file in bytes
         */
        var size: Number
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/UrlMap.es"
 */
/************************************************************************/

/*
    UrlMap.es - Simple Url Router
 */

# Config.WEB
module ejs.web {

    /** 
        Directory content handler. This redirects requests for directories and serves directory index files.
        If the request pathInfo ends with "/", the request is transparently redirected to an index file if one is present.
        The set of index files is defined by HttpServer.indicies. If the request is a directory but does not end in "/",
        the client is redirected to a URL equal to the pathInfo with a "/" appended.
        @param map Map of URLs to use for routing. TODO
        @param options TODO
        @returns A response hash object
        @spec ejs
        @stability prototype
     */
    function UrlMap(map, options): Object {
        var options ||= { longestMatchFirst: true }
        var mappings = []
        for (var location in map) {
            var app = map[location],
                host = null,
                match
            
            if (match = location.match(/^https?:\/\/(.*?)(\/.*)/)) {
                host = match[1]
                location = match[2]
            }
            if (location.charAt(0) != "/") {
                throw new Error("paths need to start with / (was: " + location + ")")
            }
            mapping.push([host, location.replace(/\/+$/,""), app])
        }
        if (options.longestMatchFirst) {
            mapping = mapping.sort(function(a, b) {
                return (b[1].length - a[1].length) || ((b[0]||"").length - (a[0]||"").length)
            })
        }
        return function(env) {
            var path  = env["PATH_INFO"] ? env["PATH_INFO"].replace(/\/+$/,"") : "",
                hHost = env['HTTP_HOST'], sName = env['SERVER_NAME'], sPort = env['SERVER_PORT']

            for (var i = 0; i < mapping.length; i++) {
                var host = mapping[i][0], location = mapping[i][1], app = mapping[i][2]
                if ((host === hHost || host === sName || (host === null )) &&
                    (location === path.substring(0, location.length)) &&
                    (path.charAt(location.length) === "" || path.charAt(location.length) === "/")) {
                    env = Object.create(env)
                    env["SCRIPT_NAME"] += location
                    env["PATH_INFO"]    = path.substring(location.length)

                    return app(env)
                }
            }
            return exports.notFound(env)
        }
    }
/*
    exports.notFound = function (env) {
        return utils.responseForStatus(404, env.PATH_INFO)
    }
*/
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/Utils.es"
 */
/************************************************************************/

/**
    Utils.es -- Utility routines for the web framework.
 */

# Config.WEB
module ejs.web {

    /** @hide */
    function errorBody(title: String, msg: String = ""): String {
        msg ||= ""
        return '<!DOCTYPE html>\r\n<html>\r\n<head><title>' + title + '</title></head>\r\n' + 
           '<body>\r\n<h1>' + title + '</h1>\r\n' +
           '    <p>' + msg + '</p>\r\n' +
           '</body>\r\n</html>\r\n'
    }

    /**
        Return the parsed cookie headers. Cookies are used to specify the session state. If sessions are being used, 
        a session cookie will be sent to and from the browser with each request. 
        @spec ejs
        @stability prototype
     */
    function parseCookies(cookieHeader: String): Object {
        let cookies = {}
        let cookie
        /*
            Input example: 
            $Version="1"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; NAME="value"; $Path="PATH"; $Domain="DOMAIN"; 
         */
        for each (c in cookieHeader.split(";")) {
            let [key,value] = c.trim().split("=")
            key = key.toLowerCase().trimStart("$")
            if (key == "domain" || key == "path" || key == "version") {
                if (cookie) {
                    cookie[key] = value
                }
            } else {
                cookie = { name: key, value: value }
                cookies[key] = cookie
            }
        }
        return cookies
    }

    /*
        Return the request form parameters. This creates the params object on demand.
        @returns Object hash of user parameters
        @spec ejs
        @stability prototype

        UNUSED && KEEP

    function parseParams(form: Object): Object {
        params = {}
        for each (f in form) {
            let parts = f.split("=")
            let key = parts[0]
            let value = parts[1]
            if (!key.contains('.')) {
                params[key] = value
            } else {
                let keyParts = key.split(".")
                o = params
                let count = keyParts.length - 2;
                let i
                for (i = 0; i < count; i++) {
                    o[keyParts[i]] = {}
                }
                o[keyParts[i]] = value
            }
        }
        return params
    }
     */

    /**
        Transform a string to be safe for output into an HTML web page. It does this by changing the
            & > < " ' characters into their ampersand HTML equivalents. This is equivalent to:

        str.replace(/&/g,'&amp;').replace(/\>/g,'&gt;').replace(/</g,'&lt;').replace(/"/g,'&quot;').replace(/'/g,'&#39;')

        @param str input string
        @returns an new escaped HTML string
        @spec ejs
        @stability prototype
     */
    native function escapeHtml(str: String): String

    /** 
        HTML encode the arguments. This escapes HTML directives to be safe for inclusion in a web page.
        @param args Variable arguments that will be converted to safe html
        @return A string containing the encoded arguments catenated together
        @spec ejs
        @stability prototype
     */
    function html(...args): String {
        result = ""
        for each (let s: String in args) {
            result += escapeHtml(s)
        }
        return result
    }

    /**
        Transform an escaped string into its original contents. This reverses the transformation done by $escapeHtml.
        It does this by changing &quot, &gt, &lt back into ", < and >.
        @param s input string
        @returns a transformed string
        @spec ejs
        @stability prototype
     */
    function unescapeHtml(s: String): String
        s.replace(/&amp;/g,'&').replace(/&gt;/g,'>').replace(/&lt;/g,'<').replace(/&quot;/g,'"')
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.web/View.es"
 */
/************************************************************************/

/**
    View.es -- View class 
 */
# Config.WEB
module ejs.web {

    //  MOB - is this necessary?
    require ejs.web

    //  MOB - what does option click Boolean mean below??
    /*
        data-remote should only take true. Then data-click=URI data-remote=true. Otherwise can define both click and remote.
     */
    /**
        Base class for web framework Views. This class provides the core functionality for templated Ejscript view 
        web pages. Ejscript web pages are compiled to create a new View class which extends the View base class.  
        This class provides a suite of high-level control methods that generate HTML for input, output and 
        presentation needs.  In addition to the properties defined by this class, user view classes will typically 
        inherit at runtime, all public properites of any associated controller object defined in 
        $ejs.web::Request.controller.

        <h4>Control Methods</h4>
        Control methods are grouped into two families: input form controls and general output controls. Input controls
        are typically located inside a form/endform control pair that defines a current data record from which data
        will be presented. Output controls can be used anywhere on a page.

        Input controls are generally of the form: function(field, options) where field is the name of the property
        in the current record that contains the data to display. The options is an object hash that controls and modifies
        how the control will render. The options hash can also be a plain string, in which case it is interpreted as an 
        object hash with a single "action" property set to the value of the options string. i.e. {action: options}. Note
        that controls will modify the options object and so sharing one option set over many controls is not advisable.

        Various controls have custom options, but most share the following common set of option properties. 
        @option action String Action to invoke. This can be a URI string or a Controller action of the form
            \@Controller/action.
        @option apply String Client JQuery selector identifying the element to apply the remote update.
            Typically "div.ID" where ID is the DOM ID for the element.
        @option background String Background color. This is a CSS RGB color specification. For example "FF0000" for red.
        @option click (Boolean|Uri|String) URI to invoke if the control is clicked.
        @option color String Foreground color. This is a CSS RGB color specification. For example "FF0000" for red.
        @option confirm String Message to prompt the user to requeset confirmation before submitting a form or request.
        @option controller Controller owning the action to invoke when clicked. Defaults to the current controller.
        @option data-* All data-* names are passed through to the HTML unmodified.
        @option domid String Client-side DOM-ID to use for the control
        @option effects String Transition effects to apply when updating a control. Select from: "fadein", "fadeout",
            "highlight".
        @option escape Boolean Escape the text before rendering. This converts HTML reserved tags and delimiters into
            an encoded form.
        @option height (Number|String) Height of the control. Can be a number of pixels or a percentage string. 
            Defaults to unlimited.
        @option key Array List of fields to set as the key values to uniquely identify the clicked or edited element. 
            The key will be rendered as a "data-key" HTML attribute and will be passed to the receiving controller 
            when the entry is clicked or edited. Each entry of the key option can be a simple
            string field name or it can be an Object with a single property, where the property name is a simple
            string field name and the property value is the mapped field name to use as the actual key name. This 
            supports using custom key names. NOTE: this option cannot be used if using cell clicks or edits. In that
            case, set click/edit to a callback function and explicitly construct the required URI and parameters.
        @option keyFormat String Define how the keys will be handled for click and edit URIs. 
            Set to one of the set: ["params", "path", "query"]. Default is "path".
            Set to "query" to add the key/value pairs to the request URI. Each pair is separated using "&" and the
                key and value are formatted as "key=value".
            Set to "params" to add the key/value pair to the request body parameters. 
            Set to "path" to add the key values in order to the request URI. Each value is separated using "/". This
                provides "pretty" URIs that can be easily tokenized by router templates.
            If you require more complex key management, set click or edit to a callback function and format the 
            URI and params manually.
        @option id Number Numeric database ID for the record that originated the data for the view element.
        @option method String HTTP method to invoke.
        @option pass String attributes to pass through unaltered to the client
        @option params Request parameters to include with a click or remote request
        @option period Number Period in milliseconds to invoke the $refresh URI to update the control data. If period
            is zero (or undefined), then refresh will be done using a perisistent connection.
        @option query URI query string to add to click URIs.
        @option rel String HTML rel attribute. Can be used to generate "rel=nofollow" on links.
        @option remote (String|URI|Object) Perform the request in the background without changing the browser location.
        @option refresh (String|URI|Object) URI to invoke in the background to refresh the control's data every $period.
            milliseconds. If period is undefined or zero, a persistent connection may be used to refresh data.
            The refresh option may use the "\@Controller/action" form.
        @option size (Number|String) Size of the element.
        @option style String CSS Style to use for the element.
        @option value Object Override value to display if used without a form control record.
        @option width (Number|String) Width of the control. Can be a number of pixels or a percentage string. Defaults to
            unlimited.

        <h4>Dynamic Data</h4>
        Most controls can perform background updates of their data after the initial presentation. This is done via
        the refresh and period options.

        @spec ejs
        @stability prototype
     */
    enumerable dynamic class View {
        /*
            Define properties and functions are (by default) in the ejs.web namespace rather than internal to avoid clashes.
         */
        use default namespace module

        /* Cache of connector objects */
        private var connectors: Object = {}

        /* Current record being used inside a form */
        private var currentRecord: Object?

        /* Data value presentation formats */
        public var formats: Object

        /* Configuration from the ejsrc configuration files */
        public var config: Object

        /** Optional controller object */
        public var controller

        /** Form and query parameters - reference to the Request.params object. */
        public var params: Object

        /** Current request object */
        public var request: Object

        /**
            Constructor method to initialize a new View
            @param request Request object
         */
        function View(request: Object) {
            if (request) {
                controller = request.controller
                blend(this, controller, {overwrite: true, deep: false})
                /* Manual construction may not have a controller */
                config = request.config
                this.request = request
                formats = config.web.views.formats
            } else {
                request = {}
                config = App.config
            }

        /*  FUTURE
            for each (helper in config.web.views.helpers) {
                if (helper.contains("::")) {
                    [mod, klass] = helper.split("::")
                    global.load(mod + ".mod")
                    //  MOB -- should use blend(this, global.[mod]::[klass])
                    blend(this, global[klass])
                } else {
                    blend(this, global[helper])
                }
            }
         */
        }

        /**
            Process and emit a view to the client. This invokes the given render function with "this" set to the view.
            @param renderer Rendering function. This may be any external function. The function will have its scope
                modified so that it executes as if it were a member method of the View class.
         */
        function render(renderer: Function): Void {
            renderer.call(this, request)
        }

        /************************************************ View Controls ***************************************************/

        /** 
            Emit a status alert area
MOB - review?
            @param text Initial message text to display. Status messages may be updated by calling the 
                $Controller.status function.
            @param options Optional extra options. See $View for a list of the standard options.
            @option period Polling period in milliseconds for the client to check the server for status message 
                updates. If this is not specifed, the connection to the server will be kept open. This permits the 
                server to "push" alerts to the console, but will consume a connection at the server for each client.
            @example
                <% alert("Status Message", { refresh: "/getData", period: 2000" }) %>
         */
        function alert(text: String, options: Object = {}): Void {
            options = getOptions(options)
            text = formatValue(text, options)
            getConnector("alert", options).alert(text, options)
        }

        //  MOB - should have a URI argument (ESP)
        /**
            Emit an anchor. This is a label inside an anchor reference. 
            @param text Link text to display
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function anchor(text: String, options: Object = {}): Void {
            options = getOptions(options)
            //  MOB - should got to anchor
            getConnector("label", options).label(text, options)
        }

        /**
            Render a form button. This creates a button suitable for use inside an input form. When the button is clicked,
            the input form will be submitted.
            @param name Name for the input button. This defines the HTML element name and provides the source of the
                initial value to display.
            @param label Text label to display in the button and value to send when the form is submitted.
            @param options Optional extra options. See $View for a list of the standard options.
            Examples:
                button("commit", "OK")
                button("commit", "Cancel")
         */
        function button(name: String, label: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("button", options).button(name, label, options)
        }

        //  MOB - this really should have a URI parameter instead of relying on options conversion via options.click 
        /**
            Render a link button. This creates a button suitable for use outside an input form. When the button 
            is clicked, the associated URI will be invoked.
            @param text Text to display in the button. The text can contain embedded HTML.
            @param options Options specifying the target URI to invoke. See $View for a list of the standard options.
            @example
                buttonLink("Cancel", "\@")
         */
        function buttonLink(text: String, options: Object = {}): Void {
            options = getOptions(options)
            //  MOB - why is this here - inconsistent
            if (currentRecord) {
                options.id ||= currentRecord.id
            }
            getConnector("buttonLink", options).buttonLink(text, options)
        }

        /**
            Render a chart. The chart control can display static or dynamic tabular data. The client chart control manages
                sorting by column, dynamic data refreshes, pagination and clicking on rows.
    MOB -- update
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data.
            @param options Object Optional extra options. See also $View for a list of the standard options.
            @option columns Object hash of column entries. Each column entry is in-turn an object hash of options. If unset, 
                all columns are displayed using defaults.
            @option kind String Type of chart. Select from: piechart, table, linechart, annotatedtimeline, guage, map, 
                motionchart, areachart, intensitymap, imageareachart, barchart, imagebarchart, bioheatmap, columnchart, 
                linechart, imagelinechart, imagepiechart, scatterchart (and more)
            @example
                <% chart(grid, { refresh: "/getData", period: 2000" }) %>
                <% chart(data, { click: "\@update" }) %>
         */
        function chart(data: Array, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("chart", options).chart(data, options)
        }

        /**
            Render an input checkbox. This creates a checkbox suitable for use within an input form. 
            @param name Name for the input checkbox. This defines the HTML element name and provides the source of the
                initial value for the checkbox. The field should be a property of the $form control record. It can be a 
                simple property of the record or it can have multiple parts, such as: field.field.field. If this call 
                is used without a form control record, the actual data value should be supplied via the options.value 
                property.
            @param checkedValue Value for which the checkbox will be checked. Defaults to true.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function checkbox(name: String, checkedValue: Object = true, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("checkbox", options).checkbox(name, value, checkedValue, options)
        }

        /**
            Render a HTML division. This creates an HTML element with the required options. It is useful to generate
                a dynamically refreshing division.
            @param body Division body content
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% div({ refresh: "/getData", period: 2000}) %>
         */
        function div(body: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("div", options).div(body, options)
        }

        /**
            End an input form. This closes an input form initiated by calling the $form method.
         */
        function endform(): Void {
            getConnector("endform").endform()
            currentRecord = undefined
        }

        /** 
            Emit a flash message area. 
            @param kinds Kinds of flash messages to display. May be a single string 
                ("error", "inform", "message", "warning"), an array of strings or null. If set to null (or omitted), 
                then all flash messages will be displayed.
            @param options Optional extra options. See $View for a list of the standard options.
            @option retain Number. Number of seconds to retain the message. If <= 0, the message is retained until another
                message is displayed. Default is 0.
                MOB - this default implies it is displayed for zero seconds
            @example
                <% flash("status") %>
                <% flash() %>
                <% flash(["error", "warning"]) %>
         */
        function flash(kinds: Object? = null, options: Object = {}): Void {
            options = getOptions(options)
            let cflash ||= request.flash
            if (cflash == null || cflash.length == 0) {
                return
            }
            let msgs: Object
            if (kinds is String) {
                msgs = {}
                msgs[kinds] = cflash[kinds]
            } else if (kinds is Array) {
                msgs = {}
                for each (kind in kinds) {
                    msgs[kind] = cflash[kind]
                }
            } else {
                msgs = cflash
            }
            let connector = getConnector("flash", options)
            for (kind in msgs) {
                let msg = msgs[kind]
                if (msg && msg != "") {
                    connector.flash(kind, msg, options.clone())
                }
            }
        }

        /**
            Render a form.
            The generated form HTML will include by default a security token definition to guard against CSRF threats.
            This token will automatically be included when the form is submitted and will be validated by the 
            receiving Controller. To disable this functionality, set options.nosecurity to true. This security token
            will be generated once for the view and the same token will be used by all forms on the view page. To use
            security tokens outside a form, you need to manually call $securityToken in the &lt;head> section of the page.

MOB -- much more doc here
    - Talk about controllers updating the record
            @param record Record to display and optionally update
            @param options Optional extra options. See $View for a list of the standard options.
            @option hideErrors Don't display model errors. Models retain error diagnostics from a failed write. Setting
                this option will prevent their display.
            @option modal String Make a form a modal dialog.
            @option nosecurity Don't generate a security token for the form.
            @option securityToken String Override CSRF security token to include when the form is submitted. A default 
                security token will always be generated unless options.nosecurity is defined to be true.
         */
        function form(record: Object, options: Object = {}): Void {
            options = getOptions(options, "action")
            currentRecord = record
            if (record) {
                options.id ||= record.id
            }
            let connector = getConnector("form", options)
            connector.form(record, options)
        }

        //  MOB - is this required if we have image()
        /** 
            Emit an icon link.
            @param src Source name for the icon.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function icon(src: Object, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("icon", options).icon(src, options)
        }

        /**
            Render an image
            @param src Source name for the image.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% image("pic.gif") %>
                <% image("pic.gif", { refresh: "\@store/getData", period: 2000, style: "myStyle" }) %>
                <% image("pic.gif", { click: "\@foreground/click" }) %>
                <% image("checkout.gif", { click: "\@checkout" }) %>
                <% image("pic.gif", { remote: "\@store/update" }) %>
         */
        function image(src: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("image", options).image(src, options)
        }

        /**
            Render an input field as part of a form. This is a smart input control that will call the appropriate
                input control based on the model field data type.
            @param name Name for the input field. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% input("phone") %>
         */
        function input(name: String, options: Object = {}): Void {
            try {
                let datatype
                if (currentRecord && Object.getType(currentRecord).getColumnType) {
                    datatype = Object.getType(currentRecord).getColumnType(name)
                } else {
                    let value = getValue(currentRecord, name, options)
                    datatype = Object.getTypeName(value).toLowerCase()
                }
                //  MOB TODO - needs fleshing out for each type
                switch (datatype) {
                case "binary":
                case "date":
                case "datetime":
                case "decimal":
                case "float":
                case "integer":
                case "number":
                case "string":
                case "time":
                case "timestamp":
                    text(name, options)
                    break
                case "text":
                    options.cols ||= 60
                    options.rows ||= 10
                    text(name, options)
                    break
                case "boolean":
                    checkbox(name, "true", options)
                    break
                default:
                    throw "input control: Unknown field type: " + datatype + " for field " + name
                }
            } catch (e) {
                text(name, options)
            }
        }

        /**
            Render a text label field. This renders an output-only text field. Use text() for input fields.
            @param text Label text to display.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% label("Hello World") %>
                <% label("Hello", { refresh: "/getData", period: 2000, style: "myStyle" }) %>
                <% label("Hello", { click: "/foreground/link" }) %>
                <% label("Checkout", { click: "\@checkout" }) %>
         */
        function label(text: String, options: Object = {}): Void {
            options = getOptions(options)
            text = formatValue(text, options)
            getConnector("label", options).label(text, options)
        }

        /**
            Emit a selection list. 
            @param name Field name to provide the default value for the list. The field should be a property of the 
                form control record. The field can be a simple property of the record or it can have multiple parts, 
                i.e. field.field.field. The field name is used to create the HTML input control name.
                If this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of key/value array tuples where the 
                first entry is the value to display and the second is the value to send to the app. Or it can be an 
                array of objects such as those returned from a table lookup. Lastly, it can be an object where the
                property key is the value to display and the property value is the value to send to the app.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                list("stockId", Stock.stockList) 
                list("priority", ["low", "med", "high"])
                list("priority", [["low", 0], ["med", 0.5], ["high", 1]])
                list("priority", [{low: 3}, {med: 5}, {high: 9}])
                list("priority", {low: 0, med: 1, high: 2})
                list("priority", [{id: 77, field: "value", ...}, ...])
         */
        function list(name: String, choices: Object, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("list", options).list(name, choices, value, options)
        }

        /**
            Emit a mail link
            @param name Recipient name to display
            @param address Mail recipient address link
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function mail(name: String, address: String, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("mail", options).mail(name, address, options)
        }

//  MOB -- redo progress using a commet style
        /** 
            Emit a progress bar.
            @param percent Progress percentage (0-100) 
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                <% progress(percent, { refresh: "/getData", period: 2000" }) %>
         */
        function progress(percent: Number, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("progress", options).progress(percent, options)
        }

        /** 
            Render a radio button. This creates a radio button suitable for use within an input form. 
            @param name Name for the input radio button. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property.
            @param choices Choices to select from. This can be an array list where each element is displayed and the value 
                returned is an element index (origin zero). It can also be an array of key/value array tuples where the 
                first entry is the value to display and the second is the value to send to the app and store in the 
                database. Or it can be an array of objects such as those returned from a table lookup. Lastly, it can 
                be an object where the property key is the value to display and the property value is the value to 
                send to the app and store in the database.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                radio("priority", ["low", "med", "high"])
                radio("priority", [["low", 0], ["med", 0.5], ["high", 1]])
                radio("priority", [{low: 3}, {med: 5}, {high: 9}])
                radio("priority", {low: 0, med: 1, high: 2})
                radio("priority", Message.priorities)
         */
        function radio(name: String, choices: Object, options: Object = {}): Void {
            options = getOptions(options)
            let value = getValue(currentRecord, name, options)
            name = getFieldName(name, options) 
            getConnector("radio", options).radio(name, value, choices, options)
        }

        /**
            Refresh control
            @hide
         */
        function refresh(on: Uri, off: Uri, options: Object = {}): Void {
            let connector = getConnector("refresh", options)
            options = getOptions(options)
            getConnector("refresh", options).refresh(on, off, options)
        }

        /** 
            Emit a script link.
            @param target Script URI to load. URI or array of scripts. Call with no arguments or set to null to 
                get a default set of scripts.
            @param options Optional extra options. See $View for a list of the standard options.
            @option minified If the target is null, a minified option will determine if compressed (minifed) 
                or uncompressed versions of the scripts will be used.
         */
        function script(target: Object?, options: Object = {}): Void {
            let connector = getConnector("script", options)
            if (target is Array) {
                for each (uri in target) {
                    connector.script(uri, options)
                }
            } else {
                connector.script(target, options)
            }
        }

        /**
            Generate a security token for the page and emit a &lt;meta HTML element for the security token.
            Security tokens are used to help guard against CSRF threats.
            This token will automatically be included whenever forms are submitted and the token be validated by the 
            receiving Controller. Forms will normally automatically generate the security token and that explicitly
            calling this routine is not required unless a security token is required for non-form requests such as AJAX
            requests. The $securityToken control should be called inside the &lt;head section of the view page.
            @param options Optional extra options. See $View for a list of the standard options.
            @example
                &lt;head>
                    <% securityToken() %>
                &lt;/head>
        */
        function securityToken(options: Object = {}): Void
            getConnector("securityToken", options).securityToken(options)

        /** 
            Emit a style sheet link.
            @param target Stylesheet URI or array of stylesheets. Call with no arguments or set to null to get a 
                default set of stylesheets.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function stylesheet(target: Object?, options: Object = {}): Void {
            let connector = getConnector("stylesheet", options)
            if (target is Array) {
                for each (uri in target) {
                    connector.stylesheet(uri, options)
                }
            } else {
                connector.stylesheet(target, options)
            }
        }

        /*
            TODO table
            - in-cell editing
            - per-row click URIs
            - pagination
         */
        /**
            Render a table. The table control can display static or dynamic tabular data. The client table control 
                manages sorting by column, dynamic data refreshes and clicking on rows or cells.
            @param data Data to display. The data must be a grid of data, ie. an Array of objects where each object 
                represents the data for a row. The column names are the object property names and the cell text is 
                the object property values.
            @param options Optional extra options. See $View for a list of the standard options.
            @option cell Boolean Set to true to make click or edit links apply per cell instead of per row. 
                The default is false.
            @option columns (Array|Object) The columns list can be either an array of column names or an object hash 
                of column objects where each column entry is hash of column options. 
                Column options: align, formatter, header, sort, sortOrder, style, width.
            @option params Object Hash of post parameters to include in the request. This is a hash of key/value items.
            @option pivot Boolean Pivot the table by swaping rows for columns and vice-versa
            @option showHeader Boolean Control if column headings are displayed.
            @option showId Boolean If a columns option is not provided, the id column is normally hidden. 
                To display, set showId to be true.
            @option sort String Enable row sorting and define the column to sort by. Defaults to the first column.
            @option sortOrder String Default sort order. Set to "ascending" or "descending".Defaults to ascending.
            @option style String CSS class to use for the table. The ultimate style to use for a table cell is the 
                combination of style, styleCells, styleColumns and style Rows.
            @option styleCells 2D Array of styles to use for the table body cells. Can also provide an array to the 
                column.style property.
            @option styleColumns Array of styles to use for the table body columns. Can also use the style option in the
                columns option.
            @option styleRows Array of styles to use for the table body rows
            @option title String Table title.
            Column options:
            <ul>
                <li>align - Will right-align numbers by default</li>
                <li>click - URI to invoke if the cell is clicked</li>
                <li>edit - MOB </li>
                <li>formatter - Function to invoke to format the value to display</li>
                <li>header - Header text for the column</li>
                <li>style - Cell styles</li>
                <li>width - Column width. Can be a string percentage or numeric pixel width</li>
            </ul>
        
            @example
                <% table(gridData, { refresh: "\@update", period: 1000, pivot: true" }) %>
                <% table(gridData, { click: "\@edit" }) %>
                <% table(Table.findAll()) %>
                <% table(gridData, {
                    click: "\@edit",
                    sort: "Product",
                    columns: {
                        product:    { header: "Product", width: "20%" }
                        date:       { format: date('%m-%d-%y) }
                    }
                 }) %>
         */
        function table(data, options: Object = {}): Void {
            options = getOptions(options)
            //  MOB - move to client side (data-pivot). No good here as it can't be refreshed!
            if (options.pivot) {
                data = pivot(data)
            }
            getConnector("table", options).table(data, options)
        }

        /**
            Render a tab control. 
            The tab control can manage a set of panes and will selectively show and hide or invoke the selected panes. 
            If the click option is defined (default), the selected pane will be invoked via a foreground click. If the
            remote option is defined, the selected pane will be invoked via a background click. If the toggle option is
            defined the selected pane will be made visible and other panes will be hidden.
            If using show/hide tabs, define the initial visible pane to be of the class "-ejs-pane-visible" and define
            other panes to be "-ejs-pane-hidden". The control's client side code will toggle these classes to make panes
            visible or hidden.
            @param data Tab data for the control. Tab data can be be a single object where the tab text is the property 
                key and the target to invoke is the property value. It can also be an an array of objects, one per tab. 
            @param options Optional extra options. See $View for a list of the standard options.
            @option click Set to true to invoke the selected pane via a foreground click.
            @option remote Set to true to invoke the selected pane via a background click.
            @option toggle Set to true to show the selected pane and hide other panes.
            @example
                tabs({Status: "pane-1", "Edit: "pane-2"})
                tabs([{Status: "/url-1"}, {"Edit: "/url-2"}], { click: "\@someAction"})
         */
        function tabs(data: Object, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("tabs", options).tabs(data, options)
        }

        /**
            Render a text input field as part of a form.
            @param name Name for the input text field. This defines the HTML element name and provides the source 
                of the initial value to display. The field should be a property of the form control record. It can 
                be a simple property of the record or it can have multiple parts, such as: field.field.field. If 
                this call is used without a form control record, the actual data value should be supplied via the 
                options.value property. If the cols or rows option is defined, then a textarea HTML element will be used for
                multiline input.
            @param options Optional extra options. See $View for a list of the standard options.
            @option cols Number number of text columns
            @option rows Number number of text rows
            @option password Boolean The data to display is a password and should be obfuscated.
            @examples
                <% text("name") %>
                <% text("product.name") %>
                <% text("address", { escape: true }) %>
                <% text("password", {value: params.password, password: true}) %>
                <% text("password", {size: 20}) %>
         */
        function text(name: String, options: Object = {}): Void {
            let value = getValue(currentRecord, name, options)
            value = formatValue(value, options)
            name = getFieldName(name, options) 
            getConnector("text", options).text(name, value, options)
        }

        /**
            Render a tree control. The tree control can display static or dynamic tree data.
            @param data Optional initial data for the control. The data option may be used with the refresh option to 
                dynamically refresh the data. The tree data is typically an XML document.
            @param options Optional extra options. See $View for a list of the standard options.
         */
        function tree(data: Object, options: Object = {}): Void {
            options = getOptions(options)
            getConnector("tree", options).tree(data, options)
        }

//MOB -- review and rethink this
        /**
            Render a partial view. This creates an HTML element with the required options. It is useful to generate
                a dynamically refreshing division.
            @param viewPath (String|Object) If a string, it is a view template name or the name of an action. If an object,
                it should have properties: controller and action. Action can be the name of a view template.
            @param options Optional extra options. See $View for a list of the standard options.
            @examples
                <% view(viewName) %>
         */
        function view(viewPath, options: Object = {}): Void {
            let cname = controllerName
            let action = viewPath
            let ext = config.extensions.ejs
            if (Object.getOwnPropertyCount(viewPath) > 0) {
                if (viewPath.controller) {
                    cname = viewPath.controller
                }
                if (viewPath.action) {
                    action = viewPath.action
                }
                if (viewPath.ext) {
                    ext = viewPath.ext
                }
            }
            controller.writeTemplate(request.dir.join(config.dirs.views.basename, cname, action).joinExt(ext))
        }

        // MOB TODO - need a rich text editor. Wiki style.  wiki()
        // MOB TODO - need markdown style output?

        /***************************************** Wrapped Request Functions **********************************************/
        /**
            @duplicate ejs.web::Request.link
         */
        function link(target: Object): Uri
            request.link(target)

        /** 
            @duplicate ejs.web::Request.redirect
         */
        function redirect(target: *, status: Number = Http.MovedTemporarily): Void
            request.redirect(target)

        /** 
            @duplicate ejs.web::Request.session 
         */
        function get session(): Session 
            request.session

        /** 
            @duplicate ejs.web::Request.setHeader
         */
        function setHeader(key: String, value: String, overwrite: Boolean = true): Void
            request.setHeader(key, value, overwrite)

        /**
            @duplicate ejs.web::Request.setHeaders
         */
        function setHeaders(headers: Object, overwrite: Boolean = true): Void
            request.setHeaders(headers, overwrite)

        /**
            @duplicate ejs.web::Request.setStatus
         */
        function setStatus(status: Number): Void
            request.setStatus(status)

        /** 
            @duplicate ejs.web::Request.show
            @hide
         */
        function show(...args): Void
            request.show(...args)

        /**
            @duplicate ejs.web::Request.write
         */
        function write(...data): Number
            request.write(...data)

        /**
            @duplicate ejs.web::Request.writeSafe
         */
        function writeSafe(...data): Number
            request.writeSafe(...data)

        /*********************************************** Support ****************************************************/
        /**
            Get the data value for presentation.
            @param value Data to present
            @param options Formatting options
            @option formatter Optional data formatter. If undefined, defaults to a basic formatter based on the value's
                data type.
            @return The formatted data.
         */
        function formatValue(value: Object?, options: Object): String {
            if (value == undefined) {
                value = ""
            }
            let formatter = options.formatter || formatters[typeOf(value)] || plainFormatter
            let result = formatter(this, value, options)
            if (options.escape) {
                result = escapeHtml(result)
            }
            return result
        }

        private function getConnector(kind: String, options: Object = {}) {
            let vc = config.web.views
            let connectorName = options.connector || vc.connectors[kind] || vc.connectors["rest"]
            let name = (connectorName + "ViewConnector").toPascal()
            if (connectors[name]) {
                return connectors[name]
            }
            try {
                //  MOB - what is this doing?
                return connectors[name] = new global[name](this)
            } catch (e: Error) {
                throw new Error("Undefined view connector: " + name)
            }
        }

        /*
            Get a field name for an input control. This will use the Record type name if not a plain-old-object.
            If options.field is defined, it is used in preference of the given field.
         */
        private function getFieldName(field: Object, options: Object): String {
            if (options.field) {
                return options.field
            }
            if (currentRecord) {
                if (Object.getType(currentRecord) != Object) {
                    return typeOf(currentRecord).toCamel() + '.' + field
                }
            }
            return field
        }

        private function getOptions(options: Object, name: String = "click"): Object {
            if (options is Uri) {
                options = options.toString()
            }
            if (options is String) {
                let str = options
                options = {}
                options[name] = str
            }
            return options
        }

//  MOB -- refactor - poor API
        /**
            @param record Optional record holding the data to display
            @param data String field name in record or object hash {field: field}. If record is not defined, "data" is
                the actual data.
            @param options Optional extra options. See $View for a list of the standard options.
            @hide
         */
        function getValue(record: Object, data: Object, options: Object): Object {
            let value
            if (options.value !== undefined) {
                value = options.value
            } else if (record) {
                let field = data.field || data
                value = record[field]
                if (value == undefined) {
                    /* Support field.field.field ... */
                    let parts = field 
                    value = record
                    for each (field in parts.split(".")) {
                        value = value[field]
                    }
                }
                if (record.hasError && record.hasError(field)) {
                    options.hasError = true
                }
            } else {
                value = data
            }
            return value
        }

        /*
            TODO - could move client side?
            Pivot the data grid. Returns a new grid, original not modified.
         */
        private function pivot(grid: Array, options: Object = {}) {
            if (!grid || grid.length == 0) return grid
            let headers = []
            let i = 0
            for (name in grid[0]) {
                headers[i++] = name
            }
            let table = []
            let row = 0
            table = []
            for each (name in headers) {
                let r = {}
                i = 1
                r[0] = name
                for (j = 0; j < grid.length; j++) {
                    r[i++] = grid[j][name]
                }
                table[row++] = r
            }
            return table
        }

        /************************************************ View Renderers **************************************************/

        var formatters = {
            Date: dateFormatter,
            String: plainFormatter,
            Number: plainFormatter,
            Boolean: plainFormatter,
            //  MOB -- put all standard types here -- faster
        }

        private static function dateFormatter(view: View, value: Object, options: Object): String
            new Date(value).format(view.formats.Date)

        private static function plainFormatter(view: View, value: Object, options: Object): String
            value.toString()
       
        /************************************************ View Helpers ****************************************************/
        /**
            Temporary helper function to format the date. MOB - should move to helpers somewhere
            @param fmt Format string
            @returns a formatted string
         */
        function date(fmt: String): String {
            return function (data: String): String {
                return new Date(data).format(fmt)
            }
        }

        /**
            Temporary helper function to format a number as currency. MOB
            @param fmt Format string
            @returns a formatted string
         */
        function currency(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /**
            Temporary helper function to format a number. MOB
            @param fmt Format string
            @returns a formatted string
         */
        function number(fmt: String): String {
            return function (data: String): String {
                return fmt.format(data)
            }
        }

        /*************************************************** Deprecated ***************************************************/
        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function makeUrl(action: String, id: String? = null, options: Object = {}, query: Object? = null): String
            request.makeUrl(action, id, options, query)

        /** 
            @hide
            @deprecated 2.0.0
         */
        # Config.Legacy
        function get appUrl()
            request.appUrl
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.zip/Zip.es"
 */
/************************************************************************/

/**
    Zip.es -- Zip class
    @hide
 */

module ejs.tar {

    //  MOB -- need some ability to trace commands  (tar tvf)

    /** @hide */
    class Zip {
        private var path: Path
        private var files = []

        use default namespace public

        function Zip(path: Path) {
            this.path = path.portable
        }
        function add(path: Path): Void {
            path = path.portable
            if (!files.contains(path)) {
                files.append(path)
            }
        }
        function create(): Void {
            let cmd = "zip -q " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            Cmd.sh(cmd)
        }
        function extract(...files): Void {
            let cmd = "unzip " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            Cmd.sh(cmd)
        }

        //  MOB -- incomplete
        function cat(...files): String {
            let cmd = "tar xOzf " + path + " " + files.join(" ")
            cmd = cmd.replace(/\\/g, "\\\\")
            return Cmd.sh(cmd)
        }
    }
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

/************************************************************************/
/*
    Start of file "src/jems/ejs.zlib/Zlib.es"
 */
/************************************************************************/

/**
    Zlib.es -- Zlib compression class

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

//  MOB - enabled while boostraping
# Config.ZLIB || 1
module ejs.zlib {

    /**
        Zlib database support
        @spec ejs
        @stabilitiy prototype
     */
    class Zlib {
        use default namespace public

        static native function compress(src: Path, dest: Path? = null)
        static native function uncompress(src: Path, dest: Path? = null)

        static native function compressBytes(data: ByteArray): ByteArray
        static native function uncompressBytes(data: ByteArray): ByteArray

        static native function compressString(data: String): String
        static native function uncompressString(data: String): String
    }
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
