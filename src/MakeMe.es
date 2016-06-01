#!usr/bin/env ejs
/*
    MakeMe.es -- Embedthis MakeMe main program

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

require ejs.unix
require ejs.zlib
require ejs.version

/**
    MakeMe Class
    This implements the MakeMe 'me' tool and provides constant definitions.

    @stability Prototype
  */
public class MakeMe {
    static const SupportedOS = ['freebsd', 'linux', 'macosx', 'solaris', 'vxworks', 'windows']
    static const SupportedArch = ['arm', 'i64', 'mips', 'ppc', 'sparc', 'x64', 'x86']
    
    /** Singleton $Builder reference */
    public var builder: Builder

    /** Singleton $Configure reference */
    public var configure

    /** Configured directories to use */
    public var directories

    /** Singleton $Loader reference */
    public var loader: Loader

    /** Application command line options from Args.options */
    public var options: Object = {}

    /** Singleton $Generate reference */
    public var generate

    /** Set to the project type being generated */
    public var generating

    /** Project generators */
    public var generators = {}

    /** Default exit status */
    public var status = 0

    private var out: Stream
    private var args: Args
    private var goals: Array

    private var argTemplate = {
        options: {
            benchmark: { alias: 'b' },
            /* Implemented in me.c */
            chdir: { range: String },
            configure: { range: String },
            configuration: { },
            'continue': { alias: 'c' },
            debug: {},
            depth: { range: Number},
            deploy: { range: String },
            diagnose: { alias: 'd' },
            dump: { },
            endian: { range: ['little', 'big'] },
            file: { range: String },
            force: { alias: 'f' },
            gen: { range: String, separator: Array, commas: true },
            get: { range: String },
            help: { },
            import: { },
            init: { },
            keep: { alias: 'k' },
            log: { alias: 'l', range: String },
            name: { range: String },
            overwrite: { },
            out: { range: String },
            more: {alias: 'm'},
            nocross: {},
            nolocal: {},
            pre: { range: String, separator: Array },
            platform: { range: String, separator: Array },
            pre: { },
            prefix: { range: String, separator: Array },
            prefixes: { range: String },
            rebuild: { alias: 'r'},
            release: {},
            rom: { },
            quiet: { alias: 'q' },
            'set': { range: String, separator: Array },
            sets: { range: String },
            show: { alias: 's'},
            showPlatform: { },
            static: { },
            unicode: {},
            unset: { range: String, separator: Array },
            verbose: { alias: 'v' },
            version: { alias: 'V' },
            watch: { range: Number },
            why: { alias: 'w' },
            'with': { range: String, separator: Array },
            without: { range: String, separator: Array },
        },
        unknown: unknownArg,
        usage: usage
    }

    /** 
        MakeMe constructor 
        @hide 
     */
    function MakeMe() {
        global.makeme = this
        let start = new Date
        args = Args(argTemplate)
        options = args.options
        loader = Loader()
        builder = Builder()
    }

    function main() {
        try {
            App.log.name = 'me'
            Me()
            parseArgs(args)
            options.file = Path(options.file || findMakeMeFile() || Loader.START)
            if (!options.configure && options.gen != 'start') {
                /* Must not change directory for init or out-of-tree source */
                App.chdir(options.file.dirname)
            }
            if (options.import) {
                import()
                App.exit()
            }
            if (options.gen == 'start' || options.gen == 'main') {
                loader.initPlatform()
                load(me.dir.me.join('Generate.es'))
                Generate().init(options.gen)
                return
            }
            let configured 
            if (goals.contains('configure')) {
                /* The configure goal is special, must be done separately and first */
                load(me.dir.me.join('Configure.es'))
                Configure().configure()
                goals.removeElements('configure')
                if (goals.length == 0) {
                    return
                }
                options.file = options.file.dirname.join(Loader.START)
                configured = true
            }
            if (goals.contains('generate')) {
                /* The generate goal is special, must be done separately */
                load(me.dir.me.join('Generate.es'))
                makeme.generate = Generate()
                let options = makeme.options
                if (!configured) {
                    /* Load current configuration */
                    builder.process(options.file)
                }
                makeme.generate.projects()
                goals.removeElements('generate')
                if (goals.length == 0) {
                    return
                }
                options.file = options.file.dirname.join(Loader.START)
            }
            if (options.watch) {
                builder.watch(options.file, goals)
            } else {
                builder.process(options.file, goals)
            }
        } catch (e) {
            let msg: String
            if (e is String) {
                App.log.error('' + e + '\n')
            } else {
                App.log.error('' + ((options.diagnose) ? e : e.message) + '\n')
            }
            App.exit(2)
        }
        if (options.benchmark) {
            trace('Benchmark', 'Elapsed time %.2f' % ((start.elapsed / 1000)) + ' secs.')
        }
        if (status) {
            trace('Error', 'Exiting with non-zero status')
            App.exit(status)
        }
    }

    function findMakeMeFile(): Path? {
        let start = Loader.START
        let main = Loader.MAIN
        /*
            Look up the tree for a start/main
         */
        let base: Path = options.configure || '.'
        for (let d: Path = base; d.parent != d; d = d.parent) {
            let names = options.configure ? ([main]) : ([start, main])
            for each (name in names) {
                let path = d.join(name)
                if (path.exists) {
                    if (name == main && !options.configure) {
                        throw 'Cannot find suitable ' + start + '.\nRun "me configure" first.'
                    }
                    vtrace('Info', 'Using: ' + path)
                    return path
                }
            }
        }
        if (main.exists) {
            throw 'Cannot find suitable ' + start + '.\nRun "me configure" first.'

        } else if (options.gen != 'start' && options.gen != 'main') {
            throw 'Cannot find suitable ' + start + '.\nRun "me --gen start" to create stub start.me'
        }
        return null
    }

    function import() {
        me = Me()
        loader.loadFile(Loader.START)
        mkdir(me.dir.top.join('makeme'), 0755)
        for each (src in Config.Bin.files('**', {relative: true})) {
            let dest = me.dir.top.join('makeme', src)
            if (Config.Bin.join(src).isDir) {
                mkdir(dest.dirname, 0755)
            } else {
                safeCopy(Config.Bin.join(src), dest)
            }
        }
    }

    function parseArgs(args: Args) {
        if (options.help || args.rest.contains('help')) {
            usage()
            App.exit(0)
        }
        if (options.showPlatform) {
            me = Me()
            let platforms = loader.readFile(Loader.START).platforms || [loader.localPlatform]
            print(platforms[0].replace('local-', Config.OS + '-' + Config.CPU + '-'))
            App.exit(0)
        }
        if (options.more) {
            let cmd = App.exePath + ' ' +
                App.args.slice(1).join(' ').replace(/[ \t]*-*more[ \t]*|[ \t]*-m[ \t]*/, ' ') + ' 2>&1 | more'
            if (options.show) {
                print(cmd)
            }
            Cmd.sh(cmd, {noio: true})
            App.exit(0)
        }
        if (options.log) {
            App.log.redirect(options.log)
            App.mprLog.redirect(options.log)
        }
        out = (options.out) ? File(options.out, 'w') : stdout

        if (options.init || args.rest.contains('init')) {
            options.gen ||= 'start'
        }
        /*
            --configure /path/to/source
         */
        if (args.rest.contains('configure')) {
            options.configure = Path('.')
        } else if (options.configure) {
            args.rest.push('configure')
            options.configure = Path(options.configure)
        }
        if (options.configuration) {
            options.configuration = true
        } else if (args.rest.contains('configuration')) {
            options.configuration = true
        }
        if (args.rest.contains('generate')) {
            if (Config.OS == 'windows') {
                options.gen = ['nmake', 'vs']
            } else if (Config.OS == 'macosx') {
                options.gen = ['make', 'xcode']
            } else {
                options.gen = ['make']
            }
        } else if (options.gen) {
            args.rest.push('generate')
        }
        if (args.rest.contains('dump')) {
            options.dump = true
        } else if (options.dump) {
            args.rest.push('dump')
            options.dump = true
        }
        if (args.rest.contains('watch')) {
            options.watch = 1000
            args.rest.remove('watch')
            args.rest.push('all')
        }
        if (args.rest.contains('rebuild')) {
            options.rebuild = true
            args.rest.push('all')
        }
        if (args.rest.contains('import')) {
            options.import = true
        }
        if (options.platform) {
            if (!(options.configure || options.gen)) {
                App.log.error('Can only set platform when configuring or generating')
                usage()
            }
            for each (platform in options.platform) {
                validatePlatform(platform)
            }
        }
        if (args.rest.contains('deploy')) {
            let platforms = Me().readFile(Loader.START).platforms
            options.deploy = Path(platforms[0]).join('deploy')
        }
        if (options.deploy) {
            options.deploy = Path(options.deploy).absolute
            options.prefix ||= []
            options.prefix.push('root=' + options.deploy)
            args.rest.push('installBinary')
        }
        /*
            The --set|unset|with|without switches apply to the previous --platform switch
         */
        let platform = loader.localPlatform
        let poptions
        options.platforms = {}
        if (options.depth) {
            poptions = options.platforms[platform] ||= {}
            poptions.enable ||= []
            poptions.enable.push('depth=' + options.depth)
        }
        for (i = 1; i < App.args.length; i++) {
            let arg = App.args[i]
            if (arg == '--platform' || arg == '-platform') {
                platform = verifyPlatform(App.args[++i])
                if (!platform.match(/\w*-\w*-\w*/)) {
                    throw 'Bad platform: ' + platform
                }
                poptions = options.platforms[platform] ||= {}
            } else if (arg == '--with' || arg == '-with') {
                poptions = options.platforms[platform] ||= {}
                poptions['with'] ||= []
                poptions['with'].push(App.args[++i])
            } else if (arg == '--without' || arg == '-without') {
                poptions = options.platforms[platform] ||= {}
                poptions.without ||= []
                poptions.without.push(App.args[++i])
            } else if (arg == '--set' || arg == '-set') {
                /* Map set to enable */
                poptions = options.platforms[platform] ||= {}
                poptions.enable ||= []
                poptions.enable.push(App.args[++i])
            } else if (arg == '--unset' || arg == '-unset') {
                /* Map set to disable */
                poptions = options.platforms[platform] ||= {}
                poptions.disable ||= []
                poptions.disable.push(App.args[++i])
            } else if (arg == '--static' || arg == '-static') {
                poptions = options.platforms[platform] ||= {}
                poptions.enable ||= []
                poptions.enable.push('static=true')
            } else if (arg == '--rom' || arg == '-rom') {
                poptions = options.platforms[platform] ||= {}
                poptions.enable ||= []
                poptions.enable.push('rom=true')
            } else if (arg == '--unicode' || arg == '-unicode') {
                poptions = options.platforms[platform] ||= {}
                poptions.enable ||= []
                poptions.enable.push(Config.OS == 'windows' ? 'charLen=2' : 'charLen=4')
            }
        }
        goals = args.rest
    }

    /**
        The MakeMe location on windows under Program Files. 
        Will be either /Program Files for 32-bit, or /Program Files (x86) for 64-bit
     */
    public function programFiles32(): Path {
        /*
            If we are a 32 bit program, we don't get to see /Program Files (x86)
         */
        let programs: Path
        if (Config.OS != 'windows') {
            return Path("/Program Files")
        } else {
            programs = Path(App.getenv('PROGRAMFILES'))
            if (App.getenv('PROCESSOR_ARCHITECTURE') == 'AMD64' || App.getenv('PROCESSOR_ARCHITEW6432') == 'AMD64') {
                let pf32 = Path(programs + ' (x86)')
                if (pf32.exists) {
                    programs = pf32
                }
            }
            if (!programs) {
                for each (drive in (FileSystem.drives() - ['A', 'B'])) {
                    let pf = Path(drive + ':\\').files('Program Files*')
                    if (pf.length > 0) {
                        return pf[0].portable
                    }
                }
            }
        }
        return programs.portable
    }

    /** @hide */
    public function setSetting(obj, key, value) {
        if (key.contains('.')) {
            let [,name,rest] = (key.match(/([^\.]*)\.(.*)/))
            obj[name] ||= {}
            setSetting(obj[name], rest, value)
        } else {
            obj[key] = value
        }
    }        

    /**
        Emit "show" trace.
        This is trace that is displayed if me --show is invoked.
        @param tag Informational tag emitted before the message
        @param args Message args to display
    */
    public function strace(tag: String, ...args) {
        if (options.show) {
            trace(tag, ...args)
        }
    }

    /**
        Emit trace
        @param tag Informational tag emitted before the message
        @param args Message args to display
     */
    public function trace(tag: String, ...args): Void {
        if (!options.quiet) {
            let msg = args.join(" ")
            let msg = "%12s %s" % (["[" + tag + "]"] + [msg]) + "\n"
            if (out) {
                out.write(msg)
            } else {
                stdout.write(msg)
            }
        }
    }

    /** @hide */
    public function traceFile(msg: String, path: String): Void
        trace(msg, '"' + path + '"')

/*
    Unknown args callback
        Support Autoconf style args:
            --prefix, --bindir, --libdir, --sysconfdir, --includedir, --libexec
            --with-component
            --without-component
            --enable-feature
            --disable-feature
     */
    function unknownArg(argv, i) {
        let map = {
            bindir: 'bin',
            libdir: 'lib',
            includedir: 'inc',
            sysconfdir: 'etc',
            libexec: 'app',
            logfiledir: 'log',
            htdocsdir: 'web',
            manualdir: 'man',
        }
        let arg = argv[i].slice(argv[i].startsWith("--") ? 2 : 1)
        for (let [from, to] in map) {
            if (arg.startsWith(from)) {
                let value = arg.split('=')[1]
                argv.splice(i, 1, '--prefix', to + '=' + value)
                return --i
            }
            if (arg.startsWith('enable-')) {
                let feature = arg.trimStart('--enable-')
                argv.splice(i, 1, '--set', feature + '=true')
                return --i
            }
            if (arg.startsWith('disable-')) {
                let feature = arg.trimStart('--disable-')
                argv.splice(i, 1, '--set', feature + '=false')
                return --i
            }
            if (arg.startsWith('with-')) {
                let component = arg.trimStart('--with-')
                argv.splice(i, 1, '--with', component)
                return --i
            }
            if (arg.startsWith('without-')) {
                let component = arg.trimStart('--without-')
                argv.splice(i, 1, '--without', component)
                return --i
            }
        }
        if (arg == '?') {
            MakeMe.usage()
        } else if (!isNaN(parseInt(arg))) {
            return i+1
        }
        throw "Undefined option '" + arg + "'"
    }

    static function usage(): Void {
        print('\nUsage: me [options] [targets|goals] ...\n' +
            '  Options:\n' +
            '  --benchmark                               # Measure elapsed time\n' +
            '  --chdir directory                         # Change to directory first\n' +
            '  --configure /path/to/source/tree          # Configure product\n' +
            '  --configuration                           # Display current configuration\n' +
            '  --continue                                # Continue on errors\n' +
            '  --debug                                   # Same as --profile debug\n' +
            '  --deploy directory                        # Install to deploy directory\n' +
            '  --depth level                             # Set utest depth level\n' +
            '  --diagnose                                # Emit diagnostic trace \n' +
            '  --dump                                    # Dump the full project\n' +
            '  --endian [big|little]                     # Define the CPU endianness\n' +
            '  --file file.me                            # Use the specified MakeMe file\n' +
            '  --force                                   # Override warnings\n' +
            '  --gen [make|nmake|sh|vs|xcode|main|start] # Generate project file\n' +
            '  --get field                               # Get and display a me field value\n' +
            '  --help                                    # Print help message\n' +
            '  --import                                  # Import standard me environment\n' +
            '  --keep                                    # Keep intermediate files\n' +
            '  --log logSpec                             # Save errors to a log file\n' +
            '  --more                                    # Pass output through "more"\n' +
            '  --nocross                                 # Build natively only\n' +
            '  --nolocal                                 # Build cross only\n' +
            '  --overwrite                               # Overwrite existing files\n' +
            '  --out path                                # Save output to a file\n' +
            '  --platform os-arch-profile                # Build for specified platform\n' +
            '  --pre                                     # Pre-process a source file to stdout\n' +
            '  --prefix dir=path                         # Define installation path prefixes\n' +
            '  --prefixes [debian|opt|embedthis]         # Use a given prefix set\n' +
            '  --profile [debug|release|...]             # Use the build profile\n' +
            '  --quiet                                   # Quiet operation. Suppress trace \n' +
            '  --rebuild                                 # Rebuild all specified targets\n' +
            '  --release                                 # Same as --profile release\n' +
            '  --rom                                     # Build for ROM without a file system\n' +
            '  --set [feature=value]                     # Enable and a feature\n' +
            '  --sets [set,set,..]                       # File set to install/deploy\n' +
            '  --show                                    # Show commands executed\n' +
            '  --static                                  # Make static libraries\n' +
            '  --unicode                                 # Set char size to wide (unicode)\n' +
            '  --unset feature                           # Unset a feature\n' +
            '  --version                                 # Display the me version\n' +
            '  --verbose                                 # Trace operations\n' +
            '  --watch [sleep time]                      # Watch for changes and rebuild\n' +
            '  --why                                     # Why a target was or was not built\n' +
            '  --with NAME[=PATH]                        # Build with package at PATH\n' +
            '  --without NAME                            # Build without a package\n' +
            '')

        let me = Me()
        makeme.options = {}
        let loader = makeme.loader = Loader()
        makeme.builder = Builder()
        loader.initPlatform()

        if (Loader.MAIN.exists) {
            try {
                loader.loadFile(Loader.MAIN)
            } catch (e) { print('CATCH: ' + e)}
        }
        if (me.usage) {
            print('Feature Selection:')
            for (let [item,msg] in me.usage) {
                msg ||= ''
                print('    --set %-32s # %s' % [item + '=value', msg])
            }
            print('')
        }
        if (me.targets) {
            Object.sortProperties(me.targets)
            let header
            for each (target in me.targets) {
                if (!target.configurable) continue
                let desc = target.description
                if (!desc) {
                    let path = loader.findPlugin(target.name, false)
                    if (path) {
                        let matches = path.readString().match(/description:.*'(.*)'|program\(.*, '(.*)'/m)
                        if (matches) {
                            desc = matches[1]
                        }
                    } else {
                        let path = me.dir.paks.join(target.name, Loader.PACKAGE)
                        if (path.exists) {
                            desc = path.readJSON().description
                        }
                    }
                }
                if (!me.configure.requires.contains(target.name) && desc) {
                    if (!header) {
                        print('Components (--with NAME, --without NAME):')
                        header = true
                    }
                    desc ||= ''
                    print('    %-38s # %s'.format([target.name, desc]))
                }
            }
        }
        App.exit(1)
    }

    function validatePlatform(platform) {
        let [os, arch] = platform.split('-')
        if (!SupportedOS.contains(os)) {
            throw 'Unsupported or unknown operating system: ' + os + '. Select from: ' + SupportedOS.join(' ')
        }
        if (!SupportedArch.find(function(a) {
            return arch.startsWith(a)
        })) {
            throw 'Unsupported or unknown architecture: ' + arch + '. Select from: ' + SupportedArch.join(' ')
        }
    }

    /** @hide */
    public function verifyPlatform(platform) {
        let [os, arch, profile] = platform.split('-')
        if (!arch) {
            arch = Config.CPU
        }
        if (!profile) {
            profile = (options.release) ? 'release' : 'debug'
        }
        return os + '-' + arch + '-' + profile
    }

    /**
        Emit "verbose" trace.
        This is trace that is displayed if me --verbose is invoked.
        @param tag Informational tag emitted before the message
        @param args Message args to display
     */
    public function vtrace(tag, ...args) {
        if (options.verbose) {
            trace(tag, ...args)
        }
    }

} /* MakeMe class */

} /* embedthis.me module */

require embedthis.me

MakeMe().main()

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


