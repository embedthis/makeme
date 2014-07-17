#!/usr/bin/env ejs
/*
    me.es -- Embedthis MakeMe

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

require ejs.unix
require ejs.zlib
require ejs.version

/**
    Me Class
    This implements the "me" tool and provide access via the Me DOM.
    @stability Prototype
  */
public class Me {
    /** @hide */
    public var initialized: Boolean

    /** @hide */
    public var options: Object = { control: {}}

    /** @hide */
    public static const PACKAGE: Path = Path('package.json')

    /** @hide */
    public static const MAIN: Path = Path('main.me')

    /** @hide */
    public var platforms: Array

    /** @hide */
    public static const BUILD: Path = Path('build')

    /** @hide */
    public static const PLATFORM: Path = Path('platform.me')

    /** @hide */
    public static const START: Path = Path('start.me')

    /** @hide */
    public var localPlatform: String

    /** @hide */
    public var selectedTargets: Array

    /** @hide */
    public var topTargets: Array

    /** @hide */
    public var localBin: Path

    /** @hide */
    public var currentPlatform: String?

    /** @hide */
    public var currentMeFile: Path?

    private static const supportedOS = ['freebsd', 'linux', 'macosx', 'solaris', 'vxworks', 'windows']
    private static const supportedArch = ['arm', 'i64', 'mips', 'sparc', 'x64', 'x86']
    private const ALL = 'all'

    /*
        Filter for files that look like temp files and should not be installed
     */
    private const TempFilter = /\.old$|\.tmp$|xcuserdata|xcworkspace|project.guid|-mine|\/sav\/|\/save\//

    private var appName: String = 'me'
    private var args: Args
    private var local: Object
    private var missing = null
    private var out: Stream
    private var rest: Array

    private var home: Path
    private var bareMe: Object = { 
        platforms: [], 
        platform: {}, 
        dir: { top: Path('.') }, 
        configure: { 
            requires: [], 
            discovers: [], 
            extras: [], 
        }, 
        settings: { 
            version: '1.0.0', 
        },
        targets: {}, 
        env: {}, 
        globals: {}, 
        customSettings: {}
    }

    private var me: Object = {}
    private var platform: Object

    private var goals: Array = []

    private var unix = ['macosx', 'linux', 'unix', 'freebsd', 'solaris']
    private var windows = ['windows', 'wince']
    private var start: Date
    private var targetsToBuildByDefault = { exe: true, file: true, lib: true }
    private var targetsToClean = { exe: true, file: true, lib: true, obj: true }

    private var argTemplate = {
        options: {
            benchmark: { alias: 'b' },
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
            keep: { alias: 'k' },
            log: { alias: 'l', range: String },
            name: { range: String },
            overwrite: { },
            out: { range: String },
            more: {alias: 'm'},
            nocross: {},
            pre: { range: String, separator: Array },
            platform: { range: String, separator: Array },
            pre: { },
            prefix: { range: String, separator: Array },
            prefixes: { range: String },
            reconfigure: { },
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

    function usage(): Void {
        print('\nUsage: me [options] [targets|goals] ...\n' +
            '  Options:\n' + 
            '  --benchmark                              # Measure elapsed time\n' +
            '  --configure /path/to/source/tree         # Configure product\n' +
            '  --configuration                          # Display current configuration\n' +
            '  --continue                               # Continue on errors\n' +
            '  --debug                                  # Same as --profile debug\n' +
            '  --deploy directory                       # Install to deploy directory\n' +
            '  --depth level                            # Set utest depth level\n' +
            '  --diagnose                               # Emit diagnostic trace \n' +
            '  --dump                                   # Dump the full project\n' +
            '  --endian [big|little]                    # Define the CPU endianness\n' +
            '  --file file.me                           # Use the specified me file\n' +
            '  --force                                  # Override warnings\n' +
            '  --gen [make|nmake|sh|vs|xcode|main|start]# Generate project file\n' + 
            '  --get field                              # Get and display a me field value\n' + 
            '  --help                                   # Print help message\n' + 
            '  --import                                 # Import standard me environment\n' + 
            '  --keep                                   # Keep intermediate files\n' + 
            '  --log logSpec                            # Save errors to a log file\n' +
            '  --more                                   # Run output through more\n' +
            '  --nocross                                # Build natively\n' +
            '  --overwrite                              # Overwrite existing files\n' +
            '  --out path                               # Save output to a file\n' +
            '  --platform os-arch-profile               # Build for specified platform\n' +
            '  --pre                                    # Pre-process a source file to stdout\n' +
            '  --prefix dir=path                        # Define installation path prefixes\n' +
            '  --prefixes [debian|opt|embedthis]        # Use a given prefix set\n' +
            '  --profile [debug|release|...]            # Use the build profile\n' +
            '  --quiet                                  # Quiet operation. Suppress trace \n' +
            '  --rebuild                                # Rebuild all specified targets\n' +
            '  --reconfigure                            # Reconfigure with existing settings\n' +
            '  --release                                # Same as --profile release\n' +
            '  --rom                                    # Build for ROM without a file system\n' +
            '  --set [feature=value]                    # Enable and a feature\n' +
            '  --sets [set,set,..]                      # File set to install/deploy\n' +
            '  --show                                   # Show commands executed\n' +
            '  --static                                 # Make static libraries\n' +
            '  --unicode                                # Set char size to wide (unicode)\n' +
            '  --unset feature                          # Unset a feature\n' +
            '  --version                                # Display the me version\n' +
            '  --verbose                                # Trace operations\n' +
            '  --watch [sleep time]                     # Watch for changes and rebuild\n' +
            '  --why                                    # Why a target was or was not built\n' +
            '  --with NAME[=PATH]                       # Build with package at PATH\n' +
            '  --without NAME                           # Build without a package\n' +
            '')
        if (START.exists) {
            try {
                b.createMe(START, Config.OS + '-' + Config.CPU)
                global.me = me = b.me
                for (let [index,platform] in me.platforms) {
                    b.createMe(START, platform)
                    b.prepBuild()
                    break
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
                    let header
                    Object.sortProperties(me.targets)
                    b.overlay('configure.es')
                    for each (target in me.targets) {
                        if (!target.configurable) continue
                        let desc = target.description
                        if (!desc) {
                            let path = findComponent(target.name)
                            if (path) {
                                let matches = path.readString().match(/description:.*'(.*)'|program\(.*, '(.*)'/m)
                                if (matches) {
                                    desc = matches[1]
                                }
                            } else {
                                let path = me.dir.paks.join(target.name, PACKAGE)
                                if (path.exists) {
                                    desc = PACKAGE.readJSON().description
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
            } catch (e) { print('CATCH: ' + e)}
        }
        App.exit(1)
    }

    function overlay(name) {
        let src = options.configure || Path('.')
        //  TODO SHOULD be done once centrally
        let base = src.join('me/standard.me').exists ? src.join('me') : Config.Bin
        global.load(base.join(name))
    }

    function main() {
        let start = new Date
        global._b = this
        home = App.dir
        args = Args(argTemplate)
        options = args.options
        try {
            setup(args)
            if (!options.file && !options.configure) {
                let file = findStart()
                if (file) {
                    App.chdir(file.dirname)
                    home = App.dir
                    options.file = file.basename
                }
            }
            if (options.import) {
                import()
                App.exit()
            } 
            if (options.init) {
                init()
                App.exit()
            } 
            if (options.reconfigure) {
                overlay('configure.es')
                reconfigure()
            }
            if (options.configure) {
                overlay('configure.es')
                configure()
                options.file = START
            }
            if (options.gen) {
                overlay('generate.es')
                if (!options.configure) {
                    platforms = me.platforms = [localPlatform]
                    createMe(options.file, localPlatform)
                    prepBuild()
                }
                generate()
            } else if (options.watch) {
                while (true) {
                    vtrace('Check', 'for changes')
                    try {
                        process(options.file)
                    } catch (e) {
                        print(e)
                    }
                    App.sleep(options.watch || 1000)
                }
            } else {
                process(options.file)
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
    }

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
            prefix: 'root',
            bindir: 'bin',
            libdir: 'lib',
            includedir: 'inc',
            sysconfdir: 'etc',
            libexec: 'app',
            logfiledir: 'log',
            htdocsdir: 'web',
            manualdir: 'man',
        }
        let arg = argv[i]
        for (let [from, to] in map) {
            if (arg.startsWith('--' + from)) {
                let value = arg.split('=')[1]
                argv.splice(i, 1, '--prefix', to + '=' + value)
                return --i
            }
            if (arg.startsWith('--enable-')) {
                let feature = arg.trimStart('--enable-')
                argv.splice(i, 1, '--set', feature + '=true')
                return --i
            }
            if (arg.startsWith('--disable-')) {
                let feature = arg.trimStart('--disable-')
                argv.splice(i, 1, '--set', feature + '=false')
                return --i
            }
            if (arg.startsWith('--with-')) {
                let component = arg.trimStart('--with-')
                argv.splice(i, 1, '--with', component)
                return --i
            }
            if (arg.startsWith('--without-')) {
                let component = arg.trimStart('--without-')
                argv.splice(i, 1, '--without', component)
                return --i
            }
        }
        throw "Undefined option '" + arg + "'"
    }

    /*
        Parse arguments
     */
    function setup(args: Args) {
        localPlatform =  Config.OS + '-' + Config.CPU + '-' + (options.release ? 'release' : 'debug')
        options.control = {}
        if (options.version) {
            print(version)
            App.exit(0)
        }
        if (options.help || args.rest.contains('help')) {
            usage()
            App.exit(0)
        }
        if (options.showPlatform) {
            b.createMe(START, Config.OS + '-' + Config.CPU)
            if (me.platforms && me.platforms.length > 0) {
                print(me.platforms[0])
            } else {
                print(localPlatform)
            }
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
        } else if (args.rest.contains('reconfigure')) {
            options.reconfigure = true
        } else if (options.reconfigure) {
            args.rest.push('configure')
        }
        if (args.rest.contains('generate')) {
            if (Config.OS == 'windows') {
                options.gen = ['sh', 'nmake', 'vs']
            } else if (Config.OS == 'macosx') {
                options.gen = ['sh', 'make', 'xcode']
            } else {
                options.gen = ['sh', 'make']
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
        if (options.platform && !(options.configure || options.gen)) {
            App.log.error('Can only set platform when configuring or generating')
            usage()
        }
        platforms = options.platform || []
        if (platforms.length == 0) {
            platforms.insert(0, localPlatform)
        }
        platforms.transform(function(e) e == 'local' ? localPlatform : e).unique()

        if (options.gen && options.gen.toString().match(/make|nmake|sh|vs|xcode/)) {
            if (platforms.length != 1) {
                App.log.error('Can only generate for one platform at a time')
                usage()
            }
            localPlatform = platforms[0]
            /* Must continue if configure can't locate tools, but does know a default */
            options['continue'] = true
        }
        let [os, arch] = localPlatform.split('-') 
        validatePlatform(os, arch)
        local = {
            name: localPlatform,
            os: os,
            arch: arch,
            like: like(os),
        }
        if (args.rest.contains('deploy')) {
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
        let platform = localPlatform
        let poptions = options.control[platform] = {}
        for (i = 1; i < App.args.length; i++) {
            let arg = App.args[i]
            if (arg == '--platform' || arg == '-platform') {
                platform = verifyPlatform(App.args[++i])
                poptions = options.control[platform] = {}
            } else if (arg == '--with' || arg == '-with') {
                poptions['with'] ||= []
                poptions['with'].push(App.args[++i])
            } else if (arg == '--without' || arg == '-without') {
                poptions.without ||= []
                poptions.without.push(App.args[++i])
            } else if (arg == '--set' || arg == '-set') {
                /* Map set to enable */
                poptions.enable ||= []
                poptions.enable.push(App.args[++i])
            } else if (arg == '--unset' || arg == '-unset') {
                /* Map set to disable */
                poptions.disable ||= []
                poptions.disable.push(App.args[++i])
            }
        }
        if (options.depth) {
            poptions.enable ||= []
            poptions.enable.push('depth=' + options.depth)
        }
        if (options.static) {
            poptions.enable ||= []
            poptions.enable.push('static=true')
        }
        if (options.rom) {
            poptions.enable ||= []
            poptions.enable.push('rom=true')
        }
        if (options.unicode) {
            poptions.enable ||= []
            poptions.enable.push(Config.OS == 'windows' ? 'charLen=2' : 'charLen=4')
        }
        goals = args.rest
        bareMe.options = options
    }

    function getValue() {
        eval('print(serialize(me.' + options.get + ', {pretty: true, quotes: false}))')
    }

    function showConfiguration() {
        print("// Configuration for Platform: " + me.platform.name)
        print("\nConfigurable Components:")
        let configurable = []
        for each (target in me.targets) {
            if (target.configurable) {
                configurable.push(target)
            }
        }
        print(serialize(configurable, {pretty: true, quotes: false}))
        print("\nsettings:")
        print(serialize(me.settings, {pretty: true, quotes: false}))
    }

    function setSetting(obj, key, value) {
        if (key.contains('.')) {
            let [,name,rest] = (key.match(/([^\.]*)\.(.*)/))
            obj[name] ||= {}
            setSetting(obj[name], rest, value)
        } else {
            obj[key] = value
        }
    }

    /*
        Apply command line --with/--without --enable/--disable options
     */
    function applyCommandLineOptions(platform) {
        var poptions = options.control[platform]
        if (!poptions) {
            return
        }
        if (options.debug) {
            me.settings.debug = true
        }
        if (options.release) {
            me.settings.debug = false
        }
        if (me.settings.debug == undefined) {
            me.settings.debug = true
        }
        /* Disable/enable was originally --unset|--set */
        for each (field in poptions.disable) {
            me.settings[field] = false
        }
        for each (field in poptions.enable) {
            let [field,value] = field.split('=')
            if (value === undefined) {
                value = true
            } else if (value == 'true') {
                value = true
            } else if (value == 'false') {
                value = false
            } else if (value.isDigit) {
                value = value cast Number
            }
            if (value == undefined) {
                value = true
            }
            let configure = me.configure.requires + me.configure.discovers
            if (configure.contains(field)) {
                App.log.error("Using \"--set " + field + "\", but " + field + " is a configurable target. " + 
                        "Use --with or --without instead.")
                App.exit(1)
            }
            setSetting(me.settings, field, value)
        }
        for each (field in poptions['without']) {
            if (me.configure.requires.contains(field)) { 
                throw 'Required component "' + field + '"" cannot be disabled.'
            }
            if (field == 'all' || field == 'default') {
                let list = me.settings['without-' + field] || me.settings.discover
                for each (f in list) {
                    let target = me.targets[f] ||= {}
                    target.name ||= f
                    target.without = true
                    target.enable = false
                    target.explicit = true
                    target.diagnostic = 'Component disabled via --without ' + f + '.'
                }
                continue
            }
            let target = me.targets[field] ||= {}
            target.name ||= field
            target.without = true
            target.enable = false
            target.explicit = true
            target.diagnostic = 'Component disabled via --without ' + field + '.'
        }
        let requires = []
        for each (field in poptions['with']) {
            let [field,value] = field.split('=')
            let prior = me.targets[field]
            let target = me.targets[field] ||= {}
            target.name ||= field
            target.explicit = true
            target.essential = true
            if (!prior) {
print("SET BARE", field)
                target.bare = true
            }
            if (value) {
                target.withpath = Path(value)
            }
            delete target.diagnostic
            if (!me.configure.requires.contains(field) && !me.configure.discovers.contains(field)) {
                requires.push(field)
            }
        }
        if (requires.length > 0) {
            /* Insert explicit require first */
            me.configure.requires = requires + me.configure.requires
        }
    }

    /*
        Apply the selected build profile
     */
    function applyProfile() {
        if (me.profiles && me.profiles[me.platform.profile]) {
            blend(me, me.profiles[me.platform.profile], {combine: true})
        }
    }

    /*
        Process a given me file
     */
    function processMe(mefile: Path, platform) {
        createMe(mefile, platform)
        if (me.package && me.package.version != me.settings.version) {
            trace('Upgrade', 'Main.me has been updated, reconfiguring ...')
            overlay('configure.es')
            reconfigure()
        }
        if (options.configuration) {
            showConfiguration()
        }
        prepBuild()
        if (options.hasOwnProperty('get')) {
            getValue()
        } else {
            build()
        }
    }

    /*
        This function will do a quick load of a me file and if it has platforms[] then it will load those instead.
     */
    function process(mefile: Path) {
        if (!mefile.exists) {
            throw 'Cannot find ' + mefile
        }
        quickLoad(mefile)
        if (me.platforms) {
            platforms = me.platforms
            for (let [index,platform] in me.platforms) {
                mefile = me.dir.top.join(BUILD, platform, PLATFORM)
                if (!mefile.exists) {
                    mefile = me.dir.top.join(platform).joinExt('me')
                }
                processMe(mefile, platform)
                if (!options.configure && (me.platforms.length > 1 || me.platform.cross)) {
                    trace('Complete', me.platform.name)
                }
            }
        } else {
            platforms = me.platforms = [localPlatform]
            processMe(mefile, localPlatform)
        }
    }

    function loadModules() {
        App.log.debug(2, "Me Modules: " + serialize(me.modules, {pretty: true}))
        for each (let module in me.modules) {
            App.log.debug(2, "Load me module: " + module)
            try {
                global.load(module)
            } catch (e) {
                throw new Error('When loading: ' + module + '\n' + e)
            }
        }
        for each (let mix in me.mixin) {
            try {
                global.eval(mix)
            } catch (e) {
                throw new Error('When loading mixin' + e)
            }
        }
    }

    /**
        @hide
        TODO - should this be static?
     */
    public function loadMeFile(path) {
        let saveCurrent = currentMeFile
        try {
            currentMeFile = path.portable
            vtrace('Loading', currentMeFile)
            global.load(path)
        } finally {
            currentMeFile = saveCurrent
        }
    }

    /*
        Rebase paths to the specified home directory
     */
    function rebase(home: Path, o: Object, field: String) {
        if (!o) return
        if (!o[field]) {
            field = '+' + field 
            if (!o[field]) {
                return
            }
        }
        if (o[field] is Array) {
            for (let [key,value] in o[field]) {
                if (!value.startsWith('${') && !value.startsWith('$(')) {
                    if (value.endsWith('/')) {
                        o[field][key] = Path(home.join(value) + '/')
                    } else {
                        o[field][key] = home.join(value)
                    }
                }
            }
        } else if (o[field] && o[field].startsWith) {
            if (!o[field].startsWith('${') && !o[field].startsWith('$(')) {
                if (o[field].endsWith('/')) {
                    o[field] = Path(home.join(o[field]) + '/')
                } else {
                    o[field] = home.join(o[field])
                }
            }
        }
    }

    /*
        Convert scripts collection into canonical long form
     */
    function fixScripts(o, topnames) {
        if (!o) return 
        /*
            Move top names inside scripts
         */
        for each (name in topnames) {
            if (o[name]) {
                o.scripts ||= {}
                o.scripts[name] = o[name]
                delete o[name]
            }
        }
        if (o.scripts) {
            /*
                Convert to canonical long form
             */
            let home = currentMeFile.dirname
            for (let [event, item] in o.scripts) {
                if (item is String || item is Function) {
                    o.scripts[event] = [{ home: home, interpreter: 'ejs', script: item }]
                } else if (item is Array) {
                    for (let [i,elt] in item) {
                        if ((elt is String) || (elt is Function)) {
                            item[i] = { home: home, interpreter: 'ejs', script: elt }
                        } else if (elt is Function) {
                            item[i] = { home: home, interpreter: 'fun', script: elt }
                        } else {
                            elt.home ||= home
                        }
                    }
                }
            }
        }
    }

    function plus(o, field) {
        if (o && o[field]) {
            o['+' + field] = o[field]
            delete o[field]
        }
    }

    function rename(o, from, to) {
        if (o && o[from]) {
            o[to] = o[from]
            delete o[from]
        }
    }

    function absPath(path: Path?) {
        if (path && !path.startsWith('${')) {
            return path.absolute
        }
        return path
    }

    function makeArray(a) {
        if (a && !(a is Array)) {
            return [a]
        }
        return a
    }

    function fixGoals(target, build) {
        if (!target.goals) {
            if (targetsToBuildByDefault[target.type] || build) {
                target.goals = [ALL, 'generate']
            } else if (target.run) {
                target.goals = ['generate']
            } else {
                target.goals = []
            }
        }
        if (target.type && target.type != 'script' && target.type != 'run' && !target.goals.contains(target.type)) {
            target.goals.push(target.type)
        }
        if (!target.goals.contains(target.name)) {
            target.goals.push(target.name)
        }
        for (field in target) {
            if (field.startsWith('generate')) {
                target.generateScript = true
            }
        }
        if (target.scripts && target.scripts.generate) {
            target.generateScript = true
        }
        if (target.generateScript && !target.goals.contains('generate')) {
            target.goals.push('generate')
        }
    }

    function fixTarget(o, tname, target) {
        target.home = absPath(target.home)
        let home = target.home
        if (target.path && !target.configurable) {
            //  Because compiler.path was changing 'clang'
            rebase(home, target, 'path')
        }
        for (let [key,value] in target.defines) {
            target.defines[key] = value.trimStart('-D')
        }
        if (target.ifdef) {
            target.ifdef = makeArray(target.ifdef)
        }
        if (target.uses) {
            target.uses = makeArray(target.uses)
        } else {
            target.uses = []
        }
        if (target.depends) {
            target.depends = makeArray(target.depends)
        } else {
            target.depends = []
        }
        rebase(home, target, 'includes')
        rebase(home, target, 'headers')
        rebase(home, target, 'resources')
        rebase(home, target, 'sources')
        rebase(home, target, 'files')
        rebase(home, target, 'subtree')

        if (target.run) {
            target.type ||= 'run'
        }
        if (target.test) {
            target.type ||= 'test'
        }
        /*
            Expand short-form scripts into the long-form. Set the target type if not defined to 'script'.
         */
        let build = target.build
        for each (n in ['action', 'build', 'shell', 'postblend', 'preresolve', 'postresolve', 'presource', 'postsource', 
                'precompile', 'postcompile', 'prebuild', 'postbuild', 'test']) {
            if (target[n] != undefined) {
                target.type ||= 'script'
                let script = target[n]
                let event = (n == 'action' || n == 'shell') ? 'build' : n
                target.scripts ||= {}
                target.scripts[event] ||= []
                target.scripts[event]  += [{ home: home, interpreter: (n == 'shell') ? 'bash' : 'ejs', script: script }]
                delete target[n]
            }
        }
        fixScripts(target, ['config', 'without', 'postconfig'])
        fixGoals(target, build)

        if (o.internal) {
            target.internal = o.internal
        }
    }

    function fixup(o, ns) {
        let home = currentMeFile ? currentMeFile.dirname : App.dir
    
        /*
            Arrays must have a +prefix to blend
         */
        if (o.mixin) {
            o.mixin = makeArray(o.mixin)
            plus(o, 'mixin')
        }
        plus(o, 'modules')
        plus(o.defaults, 'includes')
        plus(o.internal, 'includes')

        o.settings ||= {}
        o.configure ||= {}
        let settings = o.settings
        let configure = o.configure

        //  LEGACY
        if (o.modules) {
            throw 'WARNING: modules is deprecated. Use mix instead'
        }
        if (o.extensions) {
            throw 'WARNING: extensions is deprecated. Use "configure" instead'
        }
        if (o.extensions && o.extensions.generates) {
            throw 'WARNING: extensions.generates is deprecated. Use extensions.extras instead'
        }
        if (settings.extensions) {
            throw 'WARNING: settings.extensions is deprecated. Use "configure" instead'
        }
        if (settings.discover) {
            throw 'WARNING: settings.discover is deprecated. Use "discovers" instead'
        }

        plus(configure, 'requires')
        plus(configure, 'discovers')
        plus(configure, 'extras')

        rebase(home, o, 'modules')
        rebase(home, o.defaults, 'includes')
        rebase(home, o.internal, 'includes')

        fixScripts(o)
        fixScripts(o.defaults)
        fixScripts(o.internal)

        for (let [tname,target] in o.targets) {
            target.name ||= tname
            target.home ||= home
            fixTarget(o, tname, target)
        }
    }

    /** @hide */
    public function loadMeObject(o, ns = null) {
        let home = currentMeFile ? currentMeFile.dirname : App.dir
        fixup(o, ns)

        if (o.scripts && o.scripts.preblend) {
            runScript(o.scripts, "preblend")
            delete o.scripts.preblend
        }
        /* 
            Blending is depth-first -- blend this me object after loading me files referenced in blend[]
            Special case for the local plaform me file to provide early definition of platform and dir properties
         */
        if (o.dir) {
            blend(me.dir, o.dir, {combine: true})
        }
        if (o.platform) {
            blend(me.platform, o.platform, {combine: true})
        }
        if (!me.quickLoad) {
            me.globals.ME = me.dir.me
            me.globals.SRC = me.dir.src
            for each (let path in o.blend) {
                let files
                if (path.startsWith('?')) {
                    files = home.files(expand(path.slice(1), {fill: null}))
                } else {
                    path = Path(expand(path, {fill: null}))
                    files = home.files(path)
                    if (files.length == 0) {
                        vtrace('Probe', me.dir.home.join('.paks', path.trimExt(), '*', path))
                        files = me.dir.home.join('.paks', path.trimExt()).files('*/' + path).reverse().slice(0, 1)
                        if (files.length == 0) {
                            throw 'Cannot find blended module: ' + path
                        }
                    }
                }
                for each (let p in files) {
                    loadMeFile(p)
                }
            }
        }
        /*
            Delay blending defaults into targets until blendDefaults. 
            This is because 'combine: true' erases the +/- property prefixes.
         */
        if (o.targets) {
            me.targets ||= {}
            me.targets = blend(me.targets, o.targets, {functions: true})
            delete o.targets
        }
        me = blend(me, o, {combine: true, functions: true})
        if (o.scripts && o.scripts.postload) {
            runScript(me.scripts, "postload")
            delete me.scripts.postload
        }
    }

    function findStart(): Path? {
        let lp = START
        if (lp.exists) {
            return lp
        }
        let base: Path = options.configure || '.'
        for (let d: Path = base; d.parent != d; d = d.parent) {
            let f: Path = d.join(lp)
            if (f.exists) {
                vtrace('Info', 'Using me file ' + f)
                return f
            }
        }
        if (!options.configure) {
            if (Path(MAIN).exists) {
                throw 'Cannot find suitable ' + START + '.\nRun "me configure" first.'
            } else {
                if (options.gen != 'start' && options.gen != 'main') {
                    throw 'Cannot find suitable ' + START + '.\nRun "me --gen start" to create stub start.me'
                }
            }
        }
        return null
    }

    function import() {
        b.createMe(START, Config.OS + '-' + Config.CPU)
        mkdir(me.dir.top.join('me'), 0755)
        for each (src in Config.Bin.files('**', {relative: true})) {
            let dest = me.dir.top.join('me', src)
            if (Config.Bin.join(src).isDir) {
                mkdir(dest.dirname, 0755)
            } else {
                safeCopy(Config.Bin.join(src), dest)
            }
        }
    }

    /**
        @hide
     */
    public function prepBuild() {
        vtrace('Prepare', 'For building')
        if (!options.configure && (me.platforms.length > 1 || me.platform.cross)) {
            trace('Build', me.platform.name)
            vtrace('Targets', me.platform.name + ': ' + ((selectedTargets != '') ? selectedTargets: 'nothing to do'))
        }
        /* 
            When cross generating, certain wild cards can't be resolved.
            Setting missing to empty will cause missing glob patterns to be replaced with the pattern itself 
         */
        if (options.gen || options.configure) {
            missing = ''
        }
        if (me.options.gen == 'make' || me.options.gen == 'nmake') {
            me.options.configurableProject = true
        }
        makeConstGlobals()
        makeDirGlobals()
        enableTargets()
        blendDefaults()
        resolveDependencies()
        expandWildcards()
        castTargetTypes()
        setTargetPaths()

        Object.sortProperties(me.targets)
        Object.sortProperties(me)

        if (options.dump) {
            let o = me.clone()
            delete o.blend
            let path = Path(currentPlatform + '.dmp')
            Object.sortProperties(o)
            if (o.configure) {
                Object.sortProperties(o.configure)
            }
            if (o.targets) {
                Object.sortProperties(o.targets)
            }
            if (o.settings) {
                Object.sortProperties(o.settings)
            }
            for each (target in o.targets) {
                Object.sortProperties(target)
            }
            path.write(serialize(o, {pretty: true, commas: true, indent: 4, quotes: false}))
            trace('Dump', 'Save Me DOM to: ' + path)
        }
    }

    /*
        Determine which targets are enabled for building
     */
    function enableTargets() {
        for (let [tname, target] in me.targets) {
            let reported = false
            target.name ||= tname

            for each (item in target.ifdef) {
                if (!me.targets[item] || !me.targets[item].enable) {
                    if (!(me.configure.extras.contains(item) && me.options.configurableProject)) {
                        whySkip(target.name, 'disabled because the required target ' + item + ' is not enabled')
                        target.enable = false
                        reported = true
                    }
                }
            }
            if (target.enable == undefined) {
                target.enable = true

            } else if (target.enable is Function) {
                target.enable = target.enable.call(this, target)

            } else if (!(target.enable is Boolean)) {
                let script = expand(target.enable)
                try {
                    if (!eval(script)) {
                        whySkip(target.name, 'disabled on this platform')
                        target.enable = false
                    } else {
                        target.enable = true
                    }
                } catch (e) {
                    vtrace('Enable', 'Cannot run enable script for ' + target.name)
                    App.log.debug(3, e)
                    target.enable = false
                }

            } else if (!reported) {
                whySkip(target.name, 'disabled')
            }
            if (target.platforms) {
                if (!target.platforms.contains(currentPlatform) &&
                    !(samePlatform(currentPlatform, localPlatform) && target.platforms.contains('local')) &&
                    !(!samePlatform(currentPlatform, localPlatform) && target.platforms.contains('cross'))) {
                        target.enable = false
                }
            }
        }
    }

    /**
        Search for a target dependency. Search order:
            NAME
            libNAME
            NAME.ext
        @hide
     */
    public function getDep(dname) {
        if (dep = me.targets[dname]) {
            return dep

        } else if (dep = me.targets['lib' + dname]) {
            return dep

        } else if (dep = me.targets[Path(dname).trimExt()]) {
            /* Permits full library */
            return dep
        }
        return null
    }

    function selectDependentTargets(target, goal) {
        if (target.selected) {
            return
        }
        if (!target.enable) {
            return
        }
        if (goal === true || target.goals.contains(goal)) {
            target.selected = true
            for each (dname in target.depends) {
                if (dname == ALL) {
                    for each (target in me.targets) {
                        selectDependentTargets(target, dname)
                    }
                } else {
                    let dep = me.targets[dname]
                    if (dep) {
                        if (!dep.selected) {
                            selectDependentTargets(dep, true)
                        }
                    } else if (!Path(dname).exists && !me.targets[dname]) {
                        throw 'Unknown dependency "' + dname + '" in target "' + target.name + '"'
                    }
                }
            }
            for each (dname in target.uses) {
                let dep = me.targets[dname]
                if (dep && !dep.selected) {
                    selectDependentTargets(dep, true)
                }
            }
            /*
                Select targets used by this target if they are enabled. No error if not enabled.
             */
            for each (dname in target.uses) {
                let dep = me.targets[dname]
                if (dep && dep.enable && !dep.selected) {
                    selectDependentTargets(dep, true)
                }
            }
            selectedTargets.push(target)
            if (goal !== true) {
                topTargets.push(target)
            }
        }
    }

    /**
        @hide
     */
    public function selectTargets(goal): Array {
        selectedTargets = []
        topTargets = []
        for each (target in me.targets) {
            delete target.selected
        }
        for each (target in me.targets) {
            selectDependentTargets(target, goal)
        }
        if (selectedTargets.length == 0) {
            if (goal != 'all') {
                vtrace('Info', 'No enabled targets for goal "' + goal + '"')
            }
        }
        return selectedTargets
    }

    /*
        Set target output paths. Uses the default locations for libraries, executables and files
     */
    function setTargetPaths() {
        for each (target in me.targets) {
            let name = target.pname || target.name
            if (!target.path) {
                if (target.type == 'lib') {
                    if (target.static) {
                        target.path = me.dir.lib.join(name).joinExt(me.ext.lib, true)
                    } else {
                        target.path = me.dir.lib.join(name).joinExt(me.ext.shobj, true)
                    }
                } else if (target.type == 'obj') {
                    target.path = me.dir.obj.join(name).joinExt(me.ext.o, true)
                } else if (target.type == 'exe') {
                    target.path = me.dir.bin.join(name).joinExt(me.ext.exe, true)
                } else if (target.type == 'file') {
                    target.path = me.dir.lib.join(name)
                } else if (target.type == 'res') {
                    target.path = me.dir.res.join(name).joinExt(me.ext.res, true)
                }
            }
            if (target.path) {
                target.path = Path(expand(target.path))
            }
            if (target.home) {
                target.home = Path(expand(target.home))
            }
            for (let [when, item] in target.scripts) {
                for each (script in item) {
                    if (script.home) {
                        script.home = Path(expand(script.home))
                    }
                }
            }
            Object.sortProperties(target)
        }
    }

    /*
        Build a file list and apply include/exclude filters
        Include may be an array. Exclude will only ever be a RegExp|String
     */
    function buildFileList(target, include, exclude = null) {
        if (!target.copytemp) {
            if (exclude) {
                /* Join exclude patterns. Strip leading and trailing slashes and change \/ to / */
                let ex = (TempFilter.toString().slice(1, -1) + '|' + exclude.toString().slice(1, -1)).replace(/\\\//g, '/')
                exclude = RegExp(ex)
            } else {
                exclude = TempFilter
            }
        }
        let files
        if (include is RegExp) {
            /* Fast path */
            if (exclude is RegExp) {
                files = Path(me.dir.src).files('*', {include: include, exclude: exclude, missing: missing})
            } else {
                files = Path(me.dir.src).files('*', {include: include, missing: missing})
            }
        } else {
            if (!(include is Array)) {
                include = [ include ]
            }
            files = []
            for each (ipat in include) {
                ipat = expand(ipat)
                if (exclude is RegExp) {
                    files += Path('.').files(ipat, {exclude: exclude, missing: ''})
                } else {
                    files += Path('.').files(ipat, {missing: ''})
                }
            }
        }
        return files
    }

    function inheritDep(target, dep, inheritCompiler = false) {
        target.defines ||= []
        target.compiler ||= []
        target.includes ||= []
        target.libraries ||= []
        target.linker ||= []
        target.libpaths ||= []
        for each (lib in dep.libraries) {
            if (!target.libraries.contains(lib)) {
                target.libraries = target.libraries + [lib]
            }
        }
        for each (option in dep.linker) {
            if (!target.linker.contains(option)) {
                target.linker.push(option)
            }
        }
        for each (option in dep.libpaths) {
            if (!target.libpaths.contains(option)) {
                target.libpaths.push(option)
            }
        }
        for each (option in dep.includes) {
            if (!target.includes.contains(option)) {
                target.includes.push(option)
            }
        }
        for each (option in dep.defines) {
            if (!target.defines.contains(option)) {
                target.defines.push(option)
            }
        }
        if (inheritCompiler) {
            for each (option in dep.compiler) {
                if (!target.compiler.contains(option)) {
                    target.compiler.push(option)
                }
            }
        }
        return target
    }

    /*
        Resolve a target by inheriting dependent libraries from dependent targets
     */
    function resolve(target) {
        if (target.resolved) {
            return
        }
        runTargetScript(target, 'preresolve')
        target.resolved = true
        for each (dname in (target.depends + target.uses)) {
            let dep = getDep(dname)
            if (dep) {
                if (!dep.enable) {
                    if (!me.options.configurableProject) {
                        continue
                    }
                }
                if (!dep.resolved) {
                    resolve(dep)
                }
                if (dep.type == 'lib') {
                    /* 
                        Put dependent libraries first so system libraries are last (matters on linux) 
                        Convert to a canonical form without a leading 'lib'.
                     */
                    target.libraries ||= []
                    let lpath
                    if (dep.static) {
                        if (dname.startsWith('lib')) {
                            lpath = dname.replace(/^lib/, '')
                        } else {
                            lpath = dname
                        }
                    } else {
                        if (dname.startsWith('lib')) {
                            lpath = dname.replace(/^lib/, '')
                        } else {
                            lpath = dname
                        }
                    }
                    if (!target.libraries.contains(lpath)) {
                        target.libraries = target.libraries + [lpath]
                    }
                } else if (dep.configurable) {
                    if (dep.libraries) {
                        target.libraries ||= []
                        target.libraries = (target.libraries + dep.libraries).unique()
                    }
                }
                inheritDep(target, dep)
            }
        }
        runTargetScript(target, 'postresolve')
    }

    function resolveDependencies() {
        for each (target in me.targets) {
            if (target.enable) {
                resolve(target)
            }
        }
        for each (target in me.targets) {
            delete target.resolved
        }
    }

    /*
        Expand resources, sources and headers. Support include+exclude and create target.files[]
     */
    function expandWildcards() {
        let index
        for each (target in me.targets) {
            if (!target.enable) {
                continue
            }
            runTargetScript(target, 'presource')
            if (target.files) {
                let files = buildFileList(target, target.files, target.exclude)
                if (target.path) {
                    target.path = Path(expand(target.path))
                }
                if (target.type == 'file' && files.length > 1) {
                    if (me.options.gen && !me.options.configurableTarget) {
                        target.dest = target.path
                        target.path = target.path.join('.updated')
                        target.files = files
                    } else {
                        for each (file in files) {
                            let dest, name
                            if (target.subtree) {
                                name = file.relativeTo(target.subtree)
                                dest = Path(target.path).join(name)
                            } else {
                                name = file.relative
                                dest = Path(target.path).join(name)
                            }
                            me.targets[dest] = { name: name, type: 'file', enable: true, path: dest, files: [file],
                                goals: ['all', 'generate', target.name], home: target.home }
                        }
                        target.depends = files
                        target.files = []
                        target.type = 'group'
                    }
                } else {
                    target.files = files
                }
            }
            if (target.headers) {
                /*
                    Create a target for each header file
                 */
                target.files ||= []
                let files = buildFileList(target, target.headers, target.exclude)
                for each (file in files) {
                    let header = me.dir.inc.join(file.basename)
                    /* Always overwrite dynamically created targets created via makeDepends */
                    me.targets[header] = { name: header, enable: true, path: header, type: 'header', 
                        goals: [target.name], files: [ file ], includes: target.includes }
                    target.depends.push(header)
                }
            }
            if (target.resources) {
                target.files ||= []
                let files = buildFileList(target, target.resources, target.exclude)
                for each (file in files) {
                    /*
                        Create a target for each resource file
                     */
                    let res = me.dir.obj.join(file.replaceExt(me.ext.res).basename)
                    let resTarget = { name : res, enable: true, path: res, type: 'resource', 
                        goals: [target.name], files: [ file ], includes: target.includes, defines: target.defines }
                    if (me.targets[res]) {
                        resTarget = blend(me.targets[resTarget.name], resTarget, {combined: true})
                    }
                    me.targets[resTarget.name] = resTarget
                    target.files.push(res)
                    target.depends.push(res)
                }
            }
            if (target.sources) {
                target.files ||= []
                let files = buildFileList(target, target.sources, target.exclude)
                for each (file in files) {
                    /*
                        Create a target for each source file
                     */
                    let obj = me.dir.obj.join(file.replaceExt(me.ext.o).basename)
                    let objTarget = { name : obj, enable: true, path: obj, type: 'obj', 
                        goals: [target.name], files: [ file ], 
                        compiler: target.compiler, defines: target.defines, includes: target.includes}
                    let precompile = (target.scripts && target.scripts.precompile) ?  target.scripts.precompile : null
                    if (precompile) {
                        objTarget.scripts = {precompile: precompile}
                    }
                    if (me.targets[obj]) {
                        objTarget = blend(me.targets[objTarget.name], objTarget, {combined: true})
                    }
                    me.targets[objTarget.name] = objTarget
                    target.files.push(obj)
                    target.depends.push(obj)

                    /*
                        Create targets for each header (if not already present)
                     */
                    makeDepends(objTarget)
                }
            }
            if (target.files) {
                target.cmdfiles = target.files.join(' ')
            } else {
                target.cmdfiles = ''
            }
            runTargetScript(target, 'postsource')
        }
    }

    /*
        Blend me.defaults into targets. The defaults are the base, then the target is blended over the defaults.
        This delayed until all me files are blended so that the target property +/- prefixes are not lost in prior blends.
        Also allows events to modify defaults up to the last minute.
     */
    function blendDefaults() {
        runScript(me.scripts, "preinherit")
        for (let [tname, target] in me.targets) {
            if (target.libraries) {
                /* Own libraries are the libraries defined by a target, but not inherited from dependents */
                target.ownLibraries = target.libraries.clone()
            }
            if (target.type == 'lib') {
                target.ownLibraries ||= []
                target.ownLibraries += [target.name.replace(/^lib/, '')]
            }
            if (target.static == null && me.settings.static) {
                target.static = me.settings.static
            }
            let base = {}
            if (target.type == 'exe' || target.type == 'lib') {
                base = inheritDep(base, me.targets.compiler, true)
            }
            if (Object.getOwnPropertyCount(me.defaults)) {
                for (let [key,value] in me.defaults) {
                    if (!key.startsWith('+')) {
                        me.defaults['+' + key] = me.defaults[key]
                        delete me.defaults[key]
                    }
                }
                base = blend(base, me.defaults, {combine: true})
            }
            if (target.internal) {
                base = blend(base, target.internal, {combine: true})
                delete target.internal
            }
            /* NOTE: this does not blend into existing targets of the same name. It overwrites */
            target = me.targets[tname] = blend(base, target, {combine: true})
            if (target.inherit) {
                if (!(target.inherit is Array)) {
                    target.inherit = [ target.inherit ]
                }
                for each (from in target.inherit) {
                    blend(target, me[from], {combine: true})
                }
            }
            runTargetScript(target, 'postblend')
            if (target.type == 'obj') { 
                delete target.linker 
                delete target.libpaths 
                delete target.libraries 
            }
        }
    }

    /**
        @hide
     */
    public function castDirTypes() {
        /*
            Use absolute patsh so they will apply anywhere in the source tree. Rules change directory and build
            locally for each directory, so it is essential these be absolute.
         */
        for (let [key,value] in me.blend) {
            me.blend[key] = Path(value).absolute.portable
        }
        for (let [key,value] in me.dir) {
            me.dir[key] = Path(value).absolute
        }
        let defaults = me.targets.compiler
        if (defaults) {
            for (let [key,value] in defaults.includes) {
                defaults.includes[key] = Path(value).absolute
            }
            for (let [key,value] in defaults.libpaths) {
                defaults.libpaths[key] = Path(value).absolute
            }
        }
        defaults = me.defaults
        if (defaults) {
            for (let [key,value] in defaults.includes) {
                defaults.includes[key] = Path(value).absolute
            }
            for (let [key,value] in defaults.libpaths) {
                defaults.libpaths[key] = Path(value).absolute
            }
        }
        for (let [pname, prefix] in me.prefixes) {
            me.prefixes[pname] = Path(prefix)
            if (me.platform.os == 'windows') {
                if (Config.OS == 'windows') {
                    me.prefixes[pname] = me.prefixes[pname].absolute
                }
            } else {
                me.prefixes[pname] = me.prefixes[pname].normalize
            }
        }
    }

    function castTargetTypes() {
        for each (target in me.targets) {
            if (target.path) {
                target.path = Path(target.path)
            }
            if (target.home) {
                target.home = Path(target.home)
            }
            if (target.subtree) {
                target.subtree = Path(target.subtree)
            }
            for (i in target.includes) {
                target.includes[i] = Path(target.includes[i])
            }
            for (i in target.libpaths) {
                target.libpaths[i] = Path(target.libpaths[i])
            }
            for (i in target.headers) {
                target.headers[i] = Path(target.headers[i])
            }
            for (i in target.files) {
                target.files[i] = Path(target.files[i])
            }
            for (i in target.resources) {
                target.resources[i] = Path(target.resources[i])
            }
            target.depends ||= []
            target.uses ||= []
        }
    }

    /**
        Build all selected targets
        @hide
     */
    public function build() {
        if (goals.length == 0) {
            goals.push(ALL)
        }
        for each (goal in goals) {
            for each (target in selectTargets(goal)) {
                buildTarget(target)
            }
        }
    }

    /*
        Build a target and all required dependencies (first)
     */
    function buildTarget(target) {
        if (target.building) {
            App.log.error('Possible recursive dependancy: target ' + target.name + ' is already building')
        }
        vtrace('Consider', target.name)
        global.TARGET = me.target = target
        target.building = true
        target.linker ||= []
        target.libpaths ||= []
        target.includes ||= []
        target.libraries ||= []
        target.vars ||= {}

        if (target.message) {
            if (target.message is Array) {
                trace(... target.message)
            } else {
                trace('Info', target.message)
            }
        }
        try {
            if (!stale(target)) {
                whySkip(target.path, 'is up to date')
            } else {
                if (options.diagnose) {
                    App.log.debug(3, "Target => " + 
                        serialize(target, {pretty: true, commas: true, indent: 4, quotes: false}))
                }
                runTargetScript(target, 'prebuild')

                if (me.generating) {
                    generateTarget(target)
                } else {
                    if (target.dir) {
                        buildDir(target)
                    }
                    if (target.scripts && target.scripts['build']) {
                        buildScript(target)
                    }
                    if (target.type == 'lib') {
                        if (target.static) {
                            buildStaticLib(target)
                        } else {
                            buildSharedLib(target)
                        }
                    } else if (target.type == 'exe') {
                        buildExe(target)
                    } else if (target.type == 'obj') {
                        buildObj(target)
                    } else if (target.type == 'file' || target.type == 'header') {
                        buildFile(target)
                    } else if (target.type == 'resource') {
                        buildResource(target)
                    } else if (target.type == 'run') {
                        buildRun(target)
                    }
                }
                runTargetScript(target, 'postbuild')
            }
        } catch (e) {
            throw new Error('Building target ' + target.name + '\n' + e)
        }
        target.building = false
        global.TARGET = me.target = null
    }

    function buildDir(target) {
        makeDir(expand(target.dir), target)
    }

    function buildExe(target) {
        let transition = target.rule || 'exe'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Link', target.path.natural.relative)
        if (target.active && me.platform.like == 'windows') {
            let old = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + old)
            old.remove()
            try { target.path.rename(old) } catch {}
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /Creating library /})
    }

    function buildSharedLib(target) {
        let transition = target.rule || 'shlib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Link', target.path.natural.relative)
        if (target.active && me.platform.like == 'windows') {
            let active = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
            active.remove()
            try { target.path.rename(target.path.replaceExt('old')) } catch {}
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /Creating library /})
    }

    function buildStaticLib(target) {
        let transition = target.rule || 'lib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Archive', target.path.natural.relative)
        if (target.active && me.platform.like == 'windows') {
            let active = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
            active.remove()
            try { target.path.rename(target.path.replaceExt('old')) } catch {}
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /has no symbols|Creating library /})
    }

    /*
        Build symbols file for windows libraries
     */
    function buildSym(target) {
        let rule = me.rules['sym']
        if (!rule) {
            return
        }
        target.vars.INPUT = target.files.join(' ')
        let command = expandRule(target, rule)
        let data = run(command, {noshow: true})
        let result = []
        let lines = data.match(/SECT.*External *\| .*/gm)
        for each (l in lines) {
            if (l.contains('__real')) continue
            if (l.contains('??')) continue
            let sym
            if (me.platform.arch == 'x64') {
                /* Win64 does not have "_" */
                sym = l.replace(/.*\| */, '').replace(/\r$/,'')
            } else {
                sym = l.replace(/.*\| _/, '').replace(/\r$/,'')
            }
            if (sym == 'MemoryBarrier' || sym.contains('_mask@@NegDouble@')) continue
            result.push(sym)
        }
        let def = Path(target.path.toString().replace(/dll$/, 'def'))
        def.write('LIBRARY ' + target.path.basename + '\nEXPORTS\n  ' + result.sort().join('\n  ') + '\n')
    }

    /*
        Build an object from source
     */
    function buildObj(target) {
        runTargetScript(target, 'precompile')

        let ext = target.path.extension
        for each (file in target.files) {
            target.vars.INPUT = file.relative
            let transition = file.extension + '->' + target.path.extension
            if (options.pre) {
                transition = 'c->c'
            }
            let rule = target.rule || me.rules[transition]
            if (!rule) {
                rule = me.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = expandRule(target, rule)
            trace('Compile', target.path.natural.relative)
            if (me.platform.os == 'windows') {
                run(command, {excludeOutput: /^[a-zA-Z0-9-]*.c\s*$/})
            } else {
                run(command)
            }
        }
        runTargetScript(target, 'postcompile')
    }

    function buildResource(target) {
        let ext = target.path.extension
        for each (file in target.files) {
            target.vars.INPUT = file.relative
            let transition = file.extension + '->' + target.path.extension
            let rule = target.rule || me.rules[transition]
            if (!rule) {
                rule = me.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = expandRule(target, rule)
            trace('Compile', target.path.relative)
            run(command)
        }
    }

    function buildRun(target) {
        let command = target.run.clone()
        if (command is Array) {
            for (let [key,value] in command) {
                command[key] = expand(value)
            }
        } else {
            command = expand(command)
        }
        trace('Run', command)
        let pwd = App.dir
        if (target.home && target.home != pwd) {
            App.chdir(expand(target.home))
        }
        try {
            run(command, {noio: true})
        } finally {
            App.chdir(pwd)
        }
    }

    /*
        Copy files[] to path
     */
    function buildFile(target) {
        if (target.path.exists && !target.path.isDir && !target.type == 'header') {
            if (target.active && me.platform.like == 'windows') {
                let active = target.path.relative.replaceExt('old')
                trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
                active.remove()
                try { target.path.rename(target.path.replaceExt('old')) } catch {}
            } else {
                safeRemove(target.path)
            }
        }
        if (target.type != 'header') {
            trace('Copy', target.path.natural.relative)
        }
        for each (let file: Path in target.files) {
            if (file == target.path) {
                /* Auto-generated headers targets for includes have file == target.path */
                continue
            }
            copy(file, target.path, target)
        }
        if (target.path.isDir && !me.generating) {
            touchDir(target.path)
        }
    }

    function touchDir(path: Path) {
        if (path.isDir) {
            let touch = path.join('.touch')
            touch.remove()
            touch.write()
            touch.remove()
        }
    }

    function buildScript(target) {
        setRuleVars(target, target.home)
        if (target.scripts) {
            vtrace(target.type.toPascal(), target.name)
            runTargetScript(target, 'build')
        }
        if (target.path && target.path.isDir && !me.generating) {
            touchDir(target.path)
        }
    }

    /**
        Set top level constant variables. This enables them to be used in token expansion.
        @hide.
     */
    public function makeConstGlobals() {
        let g = me.globals
        g.PLATFORM = me.platform.name
        g.OS = me.platform.os
        g.CPU = me.platform.cpu || 'generic'
        g.ARCH = me.platform.arch
        /* Apple gcc only */
        if (me.platform['arch-map']) {
            g.CC_ARCH = me.platform['arch-map'][me.platform.arch] || me.platform.arch
        }
        g.CONFIG = me.platform.name
        g.EXE = me.ext.dotexe
        g.LIKE = me.platform.like
        g.O = me.ext.doto
        g.SHOBJ = me.ext.dotshobj
        g.SHLIB = me.ext.dotshlib
        if (me.settings.hasMtune && me.platform.cpu) {
            g.MTUNE = '-mtune=' + me.platform.cpu
        }
    }

    /**
        Called in this file and in xcode.es during project generation
        @hide
     */
    public function makeDirGlobals(base: Path? = null) {
        for each (n in ['BIN', 'BLD', 'OUT', 'INC', 'LIB', 'OBJ', 'PAKS', 'PKG', 'REL', 'SRC', 'TOP']) {
            /* 
                These globals are always in portable format so they can be used in build scripts. Windows back-slashes
                require quoting! 
             */ 
            let dir = me.dir[n.toLower()]
            if (!dir) continue
            dir = dir.portable
            if (base) {
                dir = dir.relativeTo(base)
            }
            global[n] = me.globals[n] = dir
        }
        if (base) {
            me.globals.LBIN = localBin.relativeTo(base)
        } else {
            me.globals.LBIN = localBin
        }
    }

    /**
        @hide
     */
    public function setRuleVars(target, base: Path = App.dir) {
        let tv = target.vars || {}
        if (target.home) {
            tv.HOME = Path(target.home).relativeTo(base)
        }
        if (target.path) {
            tv.OUTPUT = target.path.compact(base).portable
        }
        if (target.libpaths) {
            tv.LIBPATHS = mapLibPaths(target.libpaths, base)
        }
        if (me.platform.os == 'windows') {
            let entry = target.entry || me.targets.compiler.entry
            if (entry) {
                tv.ENTRY = entry[target.rule || target.type]
            }
            let subsystem = target.subsystem || me.targets.compiler.subsystem
            if (subsystem) {
                tv.SUBSYSTEM = subsystem[target.rule || target.type]
            }
        }
        if (target.type == 'exe') {
            if (!target.files) {
                throw 'Target ' + target.name + ' has no input files or sources'
            }
            tv.INPUT = target.files.map(function(p) '"' + p.compact(base).portable + '"').join(' ')
            tv.LIBS = mapLibs(target, target.libraries, target.static)
            tv.LDFLAGS = (target.linker) ? target.linker.join(' ') : ''

        } else if (target.type == 'lib') {
            if (!target.files) {
                throw 'Target ' + target.name + ' has no input files or sources'
            }
            tv.INPUT = target.files.map(function(p) '"' + p.compact(base).portable + '"').join(' ')
            tv.LIBNAME = target.path.basename
            tv.DEF = Path(target.path.compact(base).portable.toString().replace(/dll$/, 'def'))
            tv.LIBS = mapLibs(target, target.libraries, target.static)
            tv.LDFLAGS = (target.linker) ? target.linker.join(' ') : ''

        } else if (target.type == 'obj') {
            tv.CFLAGS = (target.compiler) ? target.compiler.join(' ') : ''
            tv.DEFINES = target.defines.map(function(e) '-D' + e).join(' ')
            if (me.generating) {
                /* Use abs paths to reppath can substitute as much as possible */
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '"-I' + p + '"') : ''
            } else {
                /* Use relative paths to shorten trace output */
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '"-I' + p.compact(base).portable + '"') : ''
            }
            tv.PDB = tv.OUTPUT.replaceExt('pdb')
            if (me.dir.home.join('.embedthis').exists && !me.generating) {
                tv.CFLAGS += ' -DEMBEDTHIS=1'
            }

        } else if (target.type == 'resource') {
            tv.OUTPUT = target.path.relative
            tv.CFLAGS = (target.compiler) ? target.compiler.join(' ') : ''
            tv.DEFINES = target.defines.map(function(e) '-D' + e).join(' ')
            tv.INCLUDES = (target.includes) ? target.includes.map(function(path) '"-I' + path.relative + '"') : ''
        }
        target.vars = tv
    }

    function applyEnv(me) {
        let outbin = Path('.').join(me.platform.name, 'bin').absolute
        let sep = App.SearchSeparator
        if (me.generating) {
            outbin = outbin.relative
        }
        App.putenv('PATH', outbin + sep + App.getenv('PATH'))
        App.log.debug(2, "PATH=" + App.getenv('PATH'))
    }

    /**
        Run an event script in the directory of the me file
        @hide
     */
    public function runTargetScript(target, event, options = {}) {
        let result
        if (!target.scripts) {
            return null
        }
        global.TARGET = me.target = target
        for each (item in target.scripts[event]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                changeDir(expand(item.home))
            }
            try {
                if (item.interpreter == 'ejs') {
                    if (item.script is Function) {
                        result = item.script.call(this, target)
                    } else {
                        let script = expand(item.script).expand(target.vars, {fill: ''})
                        script = 'require ejs.unix\n' + script
                        result = eval(script)
                    }
                } else {
                    runShell(target, item.interpreter, item.script)
                }
            } catch (e) {
                if (options.rethrow) {
                    throw e
                } else {
                    App.log.error('Error with target: ' + target.name + '\nCommand: ' + item.script + '\n' + e + '\n')
                    throw "Exiting"
                }
            } finally {
                changeDir(pwd)
                delete me.target
            }
        }
        return result
    }

    /**
        @hide
     */
    public function runScript(scripts, event) {
        if (!scripts) {
            return
        }
        for each (item in scripts[event]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                App.chdir(expand(item.home))
            }
            try {
                if (item.script is Function) {
                    item.script.call(this, event)
                } else {
                    script = 'require ejs.unix\n' + expand(item.script)
                    eval(script)
                }
            } finally {
                App.chdir(pwd)
            }
        }
    }

    function runShell(target, interpreter, script) {
        let lines = script.match(/^.*$/mg).filter(function(l) l.length)
        let command = lines.join(';')
        strace('Run', command)
        let interpreter = Cmd.locate(interpreter)
        let cmd = new Cmd
        cmd.start([interpreter, "-c", command.toString().trimEnd('\n')], {noio: true})
        if (cmd.status != 0 && !options['continue']) {
            throw 'Command failure: ' + command + '\nError: ' + cmd.error
        }
    }

    /**
        @hide
     */
    public function mapLibPaths(libpaths: Array, base: Path = App.dir): String {
        if (me.platform.os == 'windows') {
            return libpaths.map(function(p) '"-libpath:' + p.compact(base).portable + '"').join(' ')
        } else {
            return libpaths.map(function(p) '-L' + p.compact(base).portable).join(' ')
        }
    }

    /**
        Map libraries into the appropriate O/S dependant format
        @hide
     */
    public function mapLibs(target, libs: Array, static = null): Array {
        if (me.platform.os == 'windows') {
            libs = libs.clone()
            for (let [i,name] in libs) {
                let libname = Path('lib' + name).joinExt(me.ext.shlib)
                if (me.targets['lib' + name] || me.dir.lib.join(libname).exists) {
                    libs[i] = libname
                } else {
                    let libpaths = target ? target.libpaths : me.targets.compiler.libpaths
                    for each (dir in libpaths) {
                        if (dir.join(libname).exists) {
                            libs[i] = dir.join(libname)
                            break
                        }
                    }
                }
            }
        } else if (me.platform.os == 'vxworks') {
            libs = libs.clone()
            /*  
                Remove "*.out" libraries as they are resolved at load time only 
             */
            for (i = 0; i < libs.length; i++) {
                let name = libs[i]
                let dep = me.targets['lib' + name]
                if (!dep) {
                    dep = me.targets[name]
                }
                if (dep && dep.type == 'lib' && !dep.static) {
                    libs.remove(i, i)
                    i--
                }
            }
            for (i in libs) {
                let llib = me.dir.lib.join("lib" + libs[i]).joinExt(me.ext.shlib).relative
                if (llib.exists) {
                    libs[i] = llib
                } else {
                    libs[i] = '-l' + Path(libs[i]).trimExt().toString().replace(/^lib/, '')
                }
            }
        } else {
            let mapped = []
            for each (let lib:Path in libs) {
                mapped.push('-l' + lib.trimExt().relative.toString().replace(/^lib/, ''))
            }
            libs = mapped
        }
        for (let [i, lib] in libs) {
            if (lib.contains(' ')) {
                libs[i] = '"' + lib + '"'
            }
        }
        return libs
    }

    /*
        Test if a target is stale vs the inputs AND dependencies
     */
    function stale(target) {
        if (me.generating) {
            return !target.nogen
        }
        if (options.rebuild) {
            return true
        }
        if (!target.path) {
            return true
        }
        let path = target.path
         if (!path.modified) {
            whyRebuild(target.name, 'Rebuild', target.path + ' is missing.')
            return true
        }
        for each (file in target.files) {
             if (target.subtree) {
                let p = path.join(file.trimStart(target.subtree + target.subtree.separator[0]))
                if (!file.isDir && file.modified > p.modified) {
                    whyRebuild(path, 'Rebuild', 'input ' + file + ' has been modified.')
                    if (options.why && options.verbose) {
                        print(file, file.modified)
                        print(path, path.modified)
                    }
                    return true
                }

            } else if (file.isDir) {
                for each (f in file.files('**')) {
                     if (f.modified > path.modified) {
                        whyRebuild(path, 'Rebuild', 'input ' + f + ' has been modified.')
                        if (options.why && options.verbose) {
                            print(f, f.modified)
                            print(path, path.modified)
                        }
                        return true
                    }
                }
            } else {
                if (file.modified > path.modified) {
                    whyRebuild(path, 'Rebuild', 'input ' + file + ' has been modified.')
                    if (options.why && options.verbose) {
                        print(file, file.modified)
                        print(path, path.modified)
                    }
                    return true
                }
            }
        }
        for each (let dname: Path in (target.depends + target.uses)) {
            let file
            let dep = getDep(dname)
            if (!dep) {
                /* Dependency not found as a target , so treat as a file */
                if (!dname.modified) {
                    if (target.uses.contains(dname.toString())) {
                        continue
                    }
                    whyRebuild(path, 'Rebuild', 'missing dependency ' + dname)
                    return true
                }
                if (dname.modified > path.modified) {
                    whyRebuild(path, 'Rebuild', 'dependency ' + dname + ' has been modified.')
                    return true
                }
                return false

            } else if (dep.configurable) {
                if (!dep.enable) {
                    continue
                }
                file = dep.path
                if (!file) {
                    continue
                }
                if (!file.exists) {
                    whyRebuild(path, 'Rebuild', 'missing ' + file + ' for "' + dname + '"')
                    return true
                }

            } else {
                file = dep.path
                if (!file) {
                    continue
                }
                if (file.modified > path.modified) {
                    whyRebuild(path, 'Rebuild', 'dependent ' + file + ' has been modified.')
                    return true
                }
            }
        }
        return false
    }

    /*
        Create an array of header dependencies for source files
     */
    function makeDepends(target) {
        let includes: Array = []
        for each (path in target.files) {
            if (path.exists) {
                let str = path.readString()
                let more = str.match(/^#include.*"$/gm)
                if (more) {
                    includes += more
                }
            }
        }
        let depends = [ ]
        let meheader = me.dir.inc.join('me.h')
        if ((target.type == 'obj' || target.type == 'lib' || target.type == 'exe') && target.name != meheader) {
            depends = [ meheader ]
        }
        /*
            Resolve includes 
         */
        for each (item in includes) {
            let ifile = item.replace(/#include.*"(.*)"/, '$1')
            let path
            for each (dir in target.includes) {
                path = Path(dir).join(ifile)
                if (path.exists && !path.isDir) {
                    break
                }
                if (options.why) {
                    trace('Warn', 'Cannot resolve include: ' + path.relative + ' for ' + target.name)
                }
                path = null
            }
            if (!path) {
                path = me.dir.inc.join(ifile)
            }
            if (path && !depends.contains(path)) {
                depends.push(path)
            }
        }
        target.makedep = true
        for each (header in depends) {
            if (!me.targets[header]) {
                me.targets[header] = { name: header, enable: true, path: Path(header),
                    type: 'header', goals: [target.name], files: [ header ], includes: target.includes }
            }
            let h = me.targets[header]
            if (h && !h.makedep) {
                makeDepends(h)
                if (h.depends && target.path.extension != 'h') {
                    /* Pull up nested headers */
                    depends = (depends + h.depends).unique()
                    delete h.depends
                }
            }
        }
        if (depends.length > 0) {
            target.depends = depends
        }
    }

    /*
        Expand tokens in all fields in an object hash. This is used to expand tokens in me file objects.
     */
    function expandTokens(o) {
        for (let [key,value] in o) {
            if (value is String) {
                o[key] = expand(value)
            } else if (value is Path) {
                o[key] = Path(expand(value))
            } else if (Object.getOwnPropertyCount(value) > 0) {
                o[key] = expandTokens(value)
            }
        }
        return o
    }

    /**
        Run a command and trace output if cmdOptions.true or options.show
        @param command Command to run. May be an array of args or a string.
        @param cmdOptions Options to pass to $Cmd.
        @option show Show the command line before executing. Similar to me --show, but operates on just this command.
        @option noshow Do not show the command line before executing. Useful to override me --show for one command.
            noshow is used to hide command display and to suppress command output.
        @option continueOnErrors Continue processing even if this command is not successful.
     */
    public function run(command, cmdOptions = {}): String {
        if (options.show || cmdOptions.show) {
            let cmdline: String
            if (command is Array) {
                cmdline = command.join(' ')
            } else {
                cmdline = command
            }
            trace('Run', cmdline)
        }
        let cmd = new Cmd
        if (me.env) {
            let env = App.env.clone()
            for (let [key,value] in me.env) {
                if (value is Array) {
                    value = value.join(App.SearchSeparator)
                }
                if (me.platform.os == 'windows') {
                    /* Replacement may contain $(VS) */
                    if (!me.targets.compiler.dir.contains('$'))
                        value = value.replace(/\$\(VS\)/g, me.targets.compiler.dir)
                    if (!me.targets.winsdk.path.contains('$'))
                        value = value.replace(/\$\(SDK\)/g, me.targets.winsdk.path)
                }
                if (env[key] && (key == 'PATH' || key == 'INCLUDE' || key == 'LIB')) {
                    env[key] = value + App.SearchSeparator + env[key]
                } else {
                    env[key] = value
                }
            }
            cmd.env = env
        }
        App.log.debug(2, "Command " + command)
        App.log.debug(3, "Env " + serialize(cmd.env, {pretty: true, indent: 4, commas: true, quotes: false}))
        cmd.start(command, cmdOptions)
        if (cmd.status != 0) {
            let msg
            if (!cmd.error || cmd.error == '') {
                msg = 'Command failure: ' + cmd.response + '\nCommand: ' + command
            } else {
                msg = 'Command failure: ' + cmd.error + '\n' + cmd.response + '\nCommand: ' + command
            }
            if (cmdOptions.continueOnErrors || options['continue']) {
                if (!cmdOptions.noshow) {
                    trace('Error', msg)
                }
            } else {
                throw msg
            }
        } else if (!cmdOptions.noshow) {
            if (!cmdOptions.filter || !cmdOptions.filter.test(command)) {
                if (cmd.error) {
                    if (!cmdOptions.excludeOutput || !cmdOptions.excludeOutput.test(cmd.error)) {
                        print(cmd.error)
                    }
                }
                if (cmd.response) {
                    if (!cmdOptions.excludeOutput || !cmdOptions.excludeOutput.test(cmd.response)) {
                        print(cmd.response)
                    }
                }
            }
        }
        return cmd.response
    }

    /** 
        @hide
     */
    public function sh(commands, cmdOptions = {noio: true}): String {
        let lines = commands.match(/^.*$/gm)
        for each (cmd in lines) {
            if (Config.OS == 'windows') {
                response = run('cmd /c "' + cmd + '"', cmdOptions)
            } else {
                response = run('bash -c "' + cmd + '"', cmdOptions)
            }
        }
        return response
    }

    /**
        Make required output directories (carefully). Only make dirs inside the 'src' or 'top' directories.
        @hide
     */
    public function makeOutDirs() {
        for (let [name, dir] in me.dir) {
            if (dir.startsWith(me.dir.top) || dir.startsWith(me.dir.src)) {
                if (name == 'bin' || name == 'inc' || name == 'obj') {
                    dir.makeDir()
                }
            }
        }
        Path(me.dir.out).join('test.setup').write('test.skip("Skip platform directory")\n')
    }

    /** 
        @hide
     */
    public function safeCopy(from: Path, to: Path) {
        let p: Path = new Path(to)
        if (to.exists && !options.overwrite) {
            if (!from.isDir) {
                traceFile('Exists', to)
            }
            return
        }
        if (!to.exists) {
            traceFile('Create', to)
        } else {
            traceFile('Overwrite', to)
        }
        if (!to.dirname.isDir) {
            mkdir(to.dirname, 0755)
        }
        cp(from, to)
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

    /** 
        Emit "show" trace
        This is trace that is displayed if me --show is invoked.
        @param tag Informational tag emitted before the message
        @param args Message args to display
    */
    public function strace(tag, ...args) {
        if (options.show) {
            trace(tag, ...args)
        }
    }

    /** 
        Emit "verbose" trace
        This is trace that is displayed if me --verbose is invoked.
        @param tag Informational tag emitted before the message
        @param args Message args to display
     */
    public function vtrace(tag, ...args) {
        if (options.verbose) {
            trace(tag, ...args)
        }
    }

    function traceFile(msg: String, path: String): Void
        trace(msg, '"' + path + '"')

    /**
        Emit trace for me --why on why a target is being rebuilt
        @param path Target path being considered
        @param tag Informational tag emitted before the message
        @param msg Message to display
     */
    public function whyRebuild(path, tag, msg) {
        if (options.why) {
            trace(tag, path + ' because ' + msg)
        }
    }

    /**
        Emit trace for me --why on why a target is being skipped
        @param path Target path being considered
        @param msg Message to display
     */
    public function whySkip(path, msg) {
        if (options.why) {
            trace('Target', path + ' ' + msg)
        }
    }

    function whyMissing(...msg) {
        if (options.why) {
            trace('Missing', ...msg)
        }
    }

    function diagnose(...msg) {
        if (options.diagnose) {
            trace('Debug', ...msg)
        }
    }

    /** 
        Built-in commands
        @hide 
     */
    public function builtin(cmd: String, actionOptions: Object = {}) {
        switch (cmd) {
        case 'cleanTargets':
            for each (target in me.targets) {
                if (target.enable && !target.precious && !target.nogen && target.path && targetsToClean[target.type]) {
                    if (options.show) {
                        trace('Clean', target.path.relative)
                    }
                    let path: Path = (me.generating) ? reppath(target.path) : target.path
                    if (target.path.toString().endsWith('/')) {
                        removeDir(path)
                    } else {
                        removeFile(path)
                    }
                    if (me.platform.os == 'windows') {
                        let ext = target.path.extension
                        if (ext == me.ext.shobj || ext == me.ext.exe) {
                            removeFile(path.replaceExt('lib'))
                            removeFile(path.replaceExt('pdb'))
                            removeFile(path.replaceExt('exp'))
                        }
                    }
                }
            }
            break
        }
    }

    function like(os) {
        if (unix.contains(os)) {
            return "unix"
        } else if (windows.contains(os)) {
            return "windows"
        }
        return ""
    }

    /*
        Return the program files for 32 me. Will be either /Program Files for 32-bit, or /Program Files (x86) for 64-bit
     */
    function programFiles32(): Path {
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

    function dist(os) {
        let dist = { macosx: 'apple', windows: 'ms', 'linux': 'ubuntu', 'vxworks': 'WindRiver' }[os]
        if (os == 'linux') {
            let relfile = Path('/etc/redhat-release')
            if (relfile.exists) {
                let rver = relfile.readString()
                if (rver.contains('Fedora')) {
                    dist = 'fedora'
                } else if (rver.contains('Red Hat Enterprise')) {
                    dist = 'rhl'
                } else {
                    dist = 'fedora'
                }
            } else if (Path('/etc/SuSE-release').exists) {
                dist = 'suse'
            } else if (Path('/etc/gentoo-release').exists) {
                dist = 'gentoo'
            }
        }
        return dist
    }

    /**
        Load an object into the Me DOM
        @param obj Object collection to load into the DOM
        @param ns Reserved
     */
    public static function load(obj: Object, ns = null) {
        b.loadMeObject(obj, ns)
    }

    private var sysdirs = {
        '/Applications': true,
        '/Library': true,
        '/Network': true,
        '/System': true,
        '/Program Files': true,
        '/Program Files (x86)': true,
        '/Users': true,
        '/bin': true,
        '/dev': true,
        '/etc': true,
        '/home': true,
        '/opt': true,
        '/sbin': true,
        '/tmp': true,
        '/usr': true,
        '/usr/bin': true,
        '/usr/include': true,
        '/usr/lib': true,
        '/usr/sbin': true,
        '/usr/local': true,
        '/usr/local/bin': true,
        '/usr/local/etc': true,
        '/usr/local/include': true,
        '/usr/local/lib': true,
        '/usr/local/man': true,
        '/usr/local/opt': true,
        '/usr/local/share': true,
        '/usr/local/src': true,
        '/usr/local/x': true,
        '/var': true,
        '/var/cache': true,
        '/var/lib': true,
        '/var/log': true,
        '/var/run': true,
        '/var/spool': true,
        '/var/tmp': true,
        '/': true,
    }

    /** @hide */
    public function safeRemove(dir: Path) {
        if (sysdirs[dir]) {
            App.log.error("prevent removal of", dir)
            return
        }
        dir.removeAll()
    }

    function verifyPlatform(platform) {
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
        @hide
     */
    public function verifyPlatforms() {
        for (i in platforms) {
            platforms[i] = verifyPlatform(platforms[i])
        }
    }

    function setDirs() {
        let dir = me.dir
        dir.top = Path(dir.top)
        if (options.configure || BUILD.exists) {
            dir.bld  ||= BUILD
        } else {
            dir.bld  ||= Path('.')
        }
        if (me.settings.configured || options.configure) {
            dir.out  ||= dir.bld.join(me.platform.name)
            dir.bin  ||= dir.out.join('bin')
            dir.lib  ||= dir.bin
            dir.inc  ||= dir.out.join('inc')
            dir.obj  ||= dir.out.join('obj')
            dir.paks ||= dir.top.join('src/paks')
            dir.pkg  ||= dir.out.join('pkg')
            dir.proj ||= dir.top.join('projects')
            dir.rel  ||= dir.out.join('img')
        } else {
            dir.out  ||= dir.bld
            dir.bin  ||= dir.out
            dir.lib  ||= dir.out
            dir.inc  ||= dir.out
            dir.obj  ||= dir.out
            dir.proj ||= dir.out
            dir.paks ||= dir.top.join('paks')
        }
        dir.me ||=   dir.bin

        for (let [key,value] in dir) {
            dir[key] = Path(value.toString().expand(me)).absolute
        }
    }

    function makeDirs() {
        makeDir(me.dir.obj)
        makeDir(me.dir.bin)
    }

    /**
        Make a me object for the given platform from a me file
        @hide
     */
    public function createMe(mefile: Path, platform: String) {
        let [os, arch, profile] = platform.split('-') 
        let [arch,cpu] = (arch || '').split(":")
        let kind = like(os)
        if (!mefile.exists) {
            throw new Error("Cannot open " + mefile)
        }
        global.me = me = makeBareMe()
        me.dir.src = options.configure || Path('.')
        /*
            If imported, resolve MakeMe files relative to the imported 'me' directory
         */
        me.dir.top = Path('.')
        let home = Path(App.getenv('HOME') || App.getenv('HOMEPATH') || '.')
        me.dir.home = home.portable
        me.options ||= {}
        let cross = ((os + '-' + arch) != (Config.OS + '-' + Config.CPU))

        me.platform = { 
            name: platform, 
            os: os,
            arch: arch,
            like: kind, 
            dist: dist(os),
            profile: profile,
            dev: localPlatform,
            cross: cross,
        }
        if (cpu) {
            me.platform.cpu = cpu
        }
        let base = me.dir.src.join('me/standard.me').exists ?  me.dir.src.join('me') : Config.Bin.portable
        loadMeFile(base.join('standard.me'))
        me.dir.me = base
        me.globals.PLATFORM = currentPlatform = platform

        loadMeFile(mefile)

        /*
            Customize me files must be applied after the enclosing me file is loaded so they can override anything.
         */
        for each (path in me.customize) {
            let path = home.join(expand(path, {fill: '.'}))
            if (path.exists) {
                loadMeFile(path)
            }
        }
        setDirs()
        makeDirs()
        if (!me.settings.configured && !options.configure) {
            loadMeFile(me.dir.me.join('simple.me'))
        }
        loadMeFile(me.dir.me.join('os/' + me.platform.os + '.me'))
        loadPackage(mefile)
        if (options.name) {
            me.settings.name = options.name
        }
        if (me.scripts && me.scripts.postloadall) {
            runScript(me.scripts, "postloadall")
            delete me.scripts.postloadall
        }
        if (kind == 'windows') {
            /*
                If 32 bit, /Program Files
                If 64 bit, /Program Files, for 64 bit programs, /Program Files (x86) for 32 bit programs
             */
            me.dir.programFiles32 = programFiles32()
            me.dir.programFiles = Path(me.dir.programFiles32.name.replace(' (x86)', ''))
        }
        if (options.prefixes) {
            let pset = options.prefixes + '-prefixes'
            if (!me[pset]) {
                throw "Cannot find prefix set for " + pset
            }
            me.prefixes = {}
            me.settings.prefixes = pset
            blend(me.prefixes, me[pset])
        } else {
            if (!me.prefixes || Object.getOwnPropertyCount(me.prefixes) == 0) {
                me.prefixes = {}
                me.settings.prefixes ||= 'debian-prefixes'
                blend(me.prefixes, me[me.settings.prefixes])
            }
        }
        if (options.prefix) {
            me.prefixes ||= {}
            for each (p in options.prefix) {
                let [prefix, path] = p.split('=')
                let prior = me.prefixes[prefix]
                if (path) {
                    me.prefixes[prefix] = Path(path)
                } else {
                    /* Map --prefix=/opt to --prefix base=/opt */
                    me.prefixes.root = Path(prefix)
                }
                if (prefix == 'root') {
                    for (let [key,value] in me.prefixes) {
                        if (key != 'root' && value.startsWith(prior)) {
                            me.prefixes[key] = Path(value.replace(prior, path + '/')).normalize
                        }
                    }
                }
            }
        }
        for (let [key,value] in me.ext.clone()) {
            if (value) {
                me.ext['dot' + key] = '.' + value
            } else {
                me.ext['dot' + key] = value
            }
        }
        if (me.settings.version) {
            let ver = me.settings.version.split('-')[0]
            let majmin = ver.split('.').slice(0,2).join('.')
            me.settings.compatible ||= majmin
        }
        expandTokens(me)
        loadModules()
        applyProfile()
        me.standardSettings = me.settings.clone(true)
        applyCommandLineOptions(platform)
        applyEnv(me)
        castDirTypes()
        if (samePlatform(platform, localPlatform)) {
            me.globals.LBIN = localBin = me.dir.bin.portable
        }
        if (!me.settings.configured && !options.configure) {
            overlay('configure.es')
            findComponents()
            castDirTypes()
        }
        runScript(me.scripts, "loaded")
    }

    function samePlatform(p1, p2): Boolean {
        if (!p1 || !p2) return false
        let [os1, arch1] = p1.split('-')
        let [os2, arch2] = p2.split('-')
        return os1 == os2 && arch1 == arch2
    }

    private function loadPackage(mefile) {
        let pfile = mefile.dirname.join(PACKAGE)
        if (pfile.exists) {
            let package
            try {
                package = me.package = pfile.readJSON()
            } catch (e) {
                trace('WARN', 'Cannot parse: ' + pfile + '\n' + e)
            }
            try {
                me.settings ||= {}
                me.settings.name = package.name
                me.settings.description = package.description
                me.settings.title = package.title || package.description
                me.settings.version = package.version
                me.settings.author = package.author ? package.author.name : package.name
                me.settings.company = package.company
                if (package.dirs && package.dirs.paks) {
                    me.dir.paks = package.dirs.paks
                }
            } catch {}
        }
        me.settings.author ||= ''
        me.settings.company ||= me.settings.author.split(' ')[0].toLowerCase()
        if (me.dir.paks && !me.dir.paks.exists) {
            if (Path('src/paks').exists) {
                me.dir.paks = Path('src/paks')
            } else if (Path('src/deps').exists) {
                me.dir.paks = Path('src/deps')
            }
        }
    }

    /**
        @hide
     */
    public function quickLoad(mefile: Path) {
        global.me = me = makeBareMe()
        me.quickLoad = true
        loadMeFile(mefile)
        loadPackage(mefile)
    }

    /**
        TODO - should this be static?
        Load but don't update "me"
        @hide
     */
    public function loadMe(mefile: Path) {
        let save = me
        global.me = me = makeBareMe()
        me.quickLoad = true
        loadMeFile(mefile)
        let result = me
        global.me = me = save
        return result
    }

    function validatePlatform(os, arch) {
        if (!supportedOS.contains(os)) {
            trace('WARN', 'Unsupported or unknown operating system: ' + os + '. Select from: ' + supportedOS.join(' '))
        }
        if (!supportedArch.contains(arch)) {
            trace('WARN', 'Unsupported or unknown architecture: ' + arch + '. Select from: ' + supportedArch.join(' '))
        }
    }

    function makeBareMe() {
        let old = me
        me = bareMe.clone(true)
        me.platforms = old.platforms
        return me
    }

    /**
        Expand tokens in a string.
        Tokens are represented by '${field}' where field may contain '.'. For example ${user.name}.    
        To preserve an ${token} unmodified, preceed the token with an extra '$'. For example: $${token}.
        Calls $String.expand to expand variables from the me and me.globals objects.
        @param s Input string
        @param options Control options object
        @option fill Set to a string to use for missing properties. Set to undefined or omit options to
        throw an exception for missing properties. Set fill to '${}' to preserve undefined tokens as-is.
        This permits multi-pass expansions.
        @option join Character to use to join array elements. Defaults to space.
        @return Expanded string
     */
    public function expand(s: String, options = {fill: '${}'}) : String {
        /* 
            Do twice to allow tokens to use ${vars} 
            Last time use real options to handle unfulfilled tokens as requested.
         */
        let eo = {fill: '${}'}
        s = s.expand(me, eo)
        s = s.expand(me.globals, eo)
        s = s.expand(me, eo)
        return s.expand(me.globals, options)
    }

    /**
        @hide
     */
    public function expandRule(target, rule) {
        setRuleVars(target)
        let result = expand(rule).expand(target.vars, {fill: ''})
        return result
    }

    let VER_FACTOR = 1000                                                                            

    /**
        @hide
     */
    public function makeVersion(version: String): Number {
        let parts = version.trim().split(".")
        let patch = 0, minor = 0
        let major = parts[0] cast Number
        if (parts.length > 1) {
            minor = parts[1] cast Number
        }
        if (parts.length > 2) {
            patch = parts[2] cast Number
        }
        return (((major * VER_FACTOR) + minor) * VER_FACTOR) + patch
    }

    /**
        Copy files
        @param src Source files/directories to copy. This can be a String, Path or array of String/Paths. 
            The wildcards "*", "**" and "?" are the only wild card patterns supported. The "**" pattern matches
            every directory and file. The Posix "[]" and "{a,b}" style expressions are not supported.
            If a src item is an existing directory, then the pattern appends slash '**' subtree option is enabled.
            if a src item ends with "/", then the contents of that directory are copied without the directory itself.
        @param dest Destination file or directory. If multiple files are copied, dest is assumed to be a directory and
            will be created if required. If dest has a trailing "/", it is assumed to be a directory.
        @param options Processing and file options
        @option active If destination is an active executable or shared library, rename the active file using a
            '.old' extension and retry the copy.
        @option compress Compress target file
        @option copytemp Copy files that look like temp files
        @option exclude Exclude files that match the pattern. The pattern should be in portable file format.
        @option expand Expand tokens. Set to true or an Object hash containing properties to use when replacing 
            tokens of the form ${token} in the src and dest filenames. If set to true, the 'me' object is used.
        @option fold Fold long lines on windows at column 80 and convert new line endings.
        @option group Set file group
        @option include Include files that match the pattern. The pattern should be in portable file format.
        @option linkin Add a symbolic link to the destination in this directory
        @option permissions Set file permissions
        @option strip Strip object or executable
        @option subtree If tree is enabled, trim the subtree portion off destination filenames.
        @option user Set file file user
     */
    public function copy(src, dest: Path, options = {}) {
        let to
        dest = Path(expand(dest))
        if (!(src is Array)) src = [src]
        let subtree = options.subtree

        if (options.cat) {
            let files = []
            for each (pat in src) {
                pat = Path(expand(pat))
                files += Path('.').files(pat, {missing: undefined})
            }
            src = files.unique()
        } 
        for each (let pattern: Path in src) {
            let dir: Path, destBase: Path
            pattern = Path(expand(pattern))
            let doContents = pattern.name.endsWith('/')
            pattern = pattern.trimEnd('/')
            if (pattern.isDir) {
                if (doContents) {
                    subtree = pattern.normalize.portable
                } else {
                    subtree = pattern.normalize.dirname.portable
                }
                pattern = Path(pattern.normalize.name + '/**')
                options = blend({exclude: /\/$/}, options, {overwrite: false})
            }
            /*
                Build file list
             */
            list = me.dir.src.files(pattern, blend({relative: true}, options))
            if (me.options.verbose) {
                dump('Copy-Files', list)
            }
            if (!list || list.length == 0) {
                if (me.generating) {
                    list = [pattern]
                } else if (!options.cat && src.length > 0) {
                    throw 'cp: Cannot find files to copy for "' + pattern + '" to ' + dest
                }
            }
            let destIsDir = (dest.isDir || (!options.cat && list.length > 1) || dest.name.endsWith('/'))

            for each (let from: Path in list) {
                let from = from.portable
                if (subtree) {
                    to = dest.join(from.trimStart(Path(subtree).portable.name + '/'))
                } else if (destIsDir) {
                    to = dest.join(from.basename)
                } else {
                    to = dest
                }
                from = from.relative.portable
                if (!options.copytemp && from.match(TempFilter)) {
                    vtrace('Skip', 'Copying temp file', from)
                    continue
                }
                attributes = options.clone()
                if (!me.generating) {
                    attributes.permissions ||= from.attributes.permissions
                }
                if (me.generating) {
                    if (options.cat || options.expand || options.fold || options.compress) {
                        App.log.error('Cannot use options for copy() when generating')
                        App.log.error('Skipping', src)
                        continue
                    }
                    /* Must not use full options as it contains perms for the dest */
                    makeDir(to.dirname, {made: options.made})
                    if (from.isDir) {
                        makeDir(to, options)
                    } else {
                        copyFile(from, to, options)
                    }
                    if (options.linkin && me.platform.like == 'unix') {
                        linkFile(to, Path(expand(options.linkin)).join(to.basename), options)
                    }

                } else {
                    makeDir(to.dirname)
                    if (options.cat) {
                        catenate(from, to, attributes)
                    } else {
                        if (from.isDir) {
                            makeDir(to, options)
                        } else {
                            try {
                                copyFile(from, to, attributes)
                            } catch (e) {
                                if (options.active) {
                                    let active = to.replaceExt('old')
                                    active.remove()
                                    try { to.rename(active) } catch {}
                                }
                                copyFile(from, to, attributes)
                            }
                        }
                    }
                    if (options.expand) {
                        strace('Expand', to)
                        let o = me
                        if (options.expand != true) {
                            o = options.expand
                        }
                        to.write(to.readString().expand(o, {fill: '${}'}))
                        to.setAttributes(attributes)
                    }
                    if (options.fold) {
                        strace('Fold', to)
                        foldLines(to)
                        to.setAttributes(attributes)
                    }
                    if (options.strip && me.targets.strip && me.platform.profile == 'release') {
                        strace('Strip', to)
                        Cmd.run(me.targets.strip.path + ' ' + to)
                    }
                    if (options.compress) {
                        let zname = Path(to.name + '.gz')
                        strace('Compress', zname)
                        zname.remove()
                        Zlib.compress(to.name, zname)
                        to.remove()
                        to = zname
                    }
                    if (options.filelist) {
                        if (!to.isDir) {
                            options.filelist.push(to)
                        }
                    }
                    if (options.linkin && me.platform.like == 'unix') {
                        let linkin = Path(expand(options.linkin))
                        linkin.makeDir(options)
                        let lto = linkin.join(to.basename)
                        linkFile(to.relativeTo(lto.dirname), lto, options)
                        if (options.filelist) {
                            options.filelist.push(lto)
                        }
                    }
                }
            }
        }
        if (options.cat && options.footer && to) {
            to.append(options.footer + '\n')
        }
    }

    /*
        Fold long lines at column 80. On windows, will also convert line terminatations to <CR><LF>.
     */
    function foldLines(path: Path) {
        let lines = path.readLines()
        let out = new TextStream(new File(path, 'wt'))
        for (l = 0; l < lines.length; l++) {
            let line = lines[l]
            if (line.length > 80) {
                for (i = 79; i >= 0; i--) {
                    if (line[i] == ' ') {
                        lines[l] = line.slice(0, i)
                        lines.insert(l + 1, line.slice(i + 1))
                        break
                    }
                }
                if (i == 0) {
                    lines[l] = line.slice(0, 80)
                    lines.insert(l + 1, line.slice(80))
                }
            }
            out.writeLine(lines[l])
        }
        out.close()
    }

    /**
        Link a file.
        This creates a symbolic link on systems that support symlinks.
        The routine uses $Path.link() to implement the linking.
        This either links files or if generating, emits code to link files.
        @param src Source file 
        @param dest Destination 
        @param options See $copy() for supported options.
    */
    public function linkFile(src: Path, dest: Path, options = {}) {
        makeDir(dest.parent, options)
        if (!me.generating) {
            if (!options.dry) {
                strace('Remove', 'rm -f', dest)
                dest.remove()
                strace('Link', 'ln -s', src, dest)
                src.link(dest)
            }
        } else if (me.generating != 'nmake' && me.generating != 'vs') {
            gencmd('rm -f "' + dest + '"')
            gencmd('ln -s "' + src + '" "' + dest + '"')
        }
    }

    /** @hide */
    public function changeDir(path: Path) {
        try {
            App.chdir(path)
        } catch (e) {
            throw new Error("Cannot change directory to " + path)
        }
    }

    /**
        Make a directory
        This creates a directory and all required parents.
        This either makes a directory or if generating, emits code to make directories.
        @param path Directory path to make
        @param options See $copy() for supported options.
    */
    public function makeDir(path: Path, options = {}) {
        if (!me.generating) {
            if (!options.dry) {
                if (!path.isDir) {
                    try {
                        strace('Create', 'mkdir ' + path)
                        if (!path.makeDir()) {
                            throw "Cannot make directory" + path
                        }
                    } catch (e) {
                        print(e)
                        print("CANNOT MAKE DIR", path)
                        throw "Cannot make directory" + path
                    } 
                }
                if ((options.user || options.group || options.uid || options.gid) && App.uid == 0) {
                    path.setAttributes(options)
                } else if (options.permissions) {
                    path.setAttributes({permissions: options.permissions})
                }
            }
        } else {
            /* Generating */
            if (options.made) {
                if (options.made[path]) {
                    return
                }
                options.made[path] = true
            }
            let pwd = App.dir
            if (path.startsWith(pwd)) {
                path = path.relative
            }
            if (me.generating == 'nmake' || me.generating == 'vs') {
                /* BUG FIX */
                if (path.name.endsWith('/')) {
                    gencmd('if not exist "' + path.windows + '/" md "' + path.windows + '/"')
                } else {
                    gencmd('if not exist "' + path.windows + '" md "' + path.windows + '"')
                }
            } else {
                gencmd('mkdir -p "' + path + '"')
                if (options.permissions) {
                    gencmd('chmod ' + "%0o".format([options.permissions]) + ' "' + path + '"')
                }
                if (options.user || options.group) {
                    gencmd('[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + path + '"; true')
                }
            }
        }
    }


    /**
        Remove a file.
        This either removes files or if generating, emits code to remove files.
        @param path File to remove
        @param options Control options
    */
    public function removeFile(path: Path, options = {}) {
        if (!me.generating) {
            strace('Remove', 'rm -f', path)
            if (!options.dry) {
                if (!path.remove()) {
                    throw "Cannot remove " + path
                }
            }
        } else {
            let pwd = App.dir
            if (path.startsWith(pwd)) {
                path = path.relative
            }
            if (me.generating == 'nmake' || me.generating == 'vs') {
                gencmd('if exist "' + path.windows + '" del /Q "' + path.windows + '"')
            } else {
                gencmd('rm -f "' + path + '"')
            }
        }
    }

    /**
        Remove a directory.
        This removes a file or directory and all its contents include subdirectories. Use the 'empty' option to only remove
        empty directories.
        This either removes directories or if generating, emits code to remove directories.
        @param path Directory to remove
        @param options Control options
        @option empty Remove the directory only if empty. 
    */
    public function removeDir(path: Path, options = {}) {
        if (!me.generating) {
            strace('Remove', path)
            if (!options.dry) {
                if (options.empty) {
                    strace('Remove', 'rmdir', path)
                    path.remove()
                } else {
                    strace('Remove', 'rm -fr', path)
                    path.removeAll()
                }
            }
        } else {
            let pwd = App.dir
            if (path.startsWith(pwd)) {
                path = path.relative
            }
            if (me.generating == 'nmake' || me.generating == 'vs') {
                if (options.empty) {
                    gencmd('if exist "' + path.windows + '" rd /Q "' + path.windows + '"')
                } else {
                    gencmd('if exist "' + path.windows + '" rd /Q /S "' + path.windows + '"')
                }
            } else {
                if (options.empty) {
                    gencmd('rmdir -p "' + path + '" 2>/dev/null ; true')
                } else {
                    gencmd('rm -fr "' + path + '"')
                }
            }
        }
    }

    /**
        Remove a file or directory.
        This removes a file or directory and all its contents including subdirectories.
        @param path File or directory to remove
    */
    public function removePath(path: Path) {
        if (!me.generating) {
            strace('Remove', path)
            if (!options.dry) {
                strace('Remove', 'rm -fr', path)
                path.removeAll()
            }
        } else {
            let pwd = App.dir
            if (path.startsWith(pwd)) {
                path = path.relative
            }
            if (me.generating == 'nmake' || me.generating == 'vs') {
                gencmd('if exist "' + path.windows + '\\" rd /Q /S "' + path.windows + '"')
                gencmd('if exist "' + path.windows + '" del /Q "' + path.windows + '"')
            } else {
                gencmd('rm -fr "' + path + '"')
            }
        }
    }

    /**
        Copy a file.
        Copy files to a destination.
        The routine uses $copy() to implement the copying.
        This either copies files or if generating, emits code to copy files.
        @param src Source file 
        @param dest Destination 
        @param options Options to pass to Me.copy(). These include user, group, uid, gid and  permissions.
    */
    public function copyFile(src: Path, dest: Path, options = {}) {
        if (!me.generating) {
            strace('Copy', 'cp ' + src.portable + ' ' + dest.portable)
            if (src.same(dest)) {
                throw new Error('Cannot copy file. Source is the same as destination: ' + src)
            }
            if (!options.dry) {
                src.copy(dest)
            }
            if ((options.user || options.group || options.uid || options.gid) && App.uid == 0) {
                dest.setAttributes(options)
            } else if (options.permissions) {
                dest.setAttributes({permissions: options.permissions})
            }
        } else {
            let pwd = App.dir
            if (src.startsWith(pwd)) {
                src = src.relativeTo(me.dir.top)
            }
            if (dest.startsWith(pwd)) {
                dest = dest.relativeTo(me.dir.top)
            }
            if (src == dest) {
                throw new Error('Cannot copy file. Source is the same as destination: ' + src)
            }
            if (me.generating == 'nmake' || me.generating == 'vs') {
                src = src.windows
                if (src.contains(' ')) {
                    src = '"' + src + '"'
                }
                dest = dest.windows
                if (dest.contains(' ')) {
                    dest = '"' + dest + '"'
                }
                gencmd('copy /Y ' + src + ' ' + dest.windows)
            } else {
                if (src.contains(' ')) {
                    src = '"' + src + '"'
                }
                if (dest.contains(' ')) {
                    dest = '"' + dest + '"'
                }
                gencmd('cp ' + src + ' ' + dest)
                if (options.uid || options.gid) {
                    gencmd('[ `id -u` = 0 ] && chown ' + options.uid + ':' + options.gid + ' "' + dest + '"; true')
                } else if (options.user || options.group) {
                    gencmd('[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + dest + '"; true')
                }
                if (options.permissions) {
                    gencmd('chmod ' + "%0o".format([options.permissions]) + ' "' + dest + '"')
                }
            }
        }
    }


    /** @hide */
    public function catenate(from, target, options) {
        strace('Combine', from.relative)
        if (!target.exists) {
            if (options.title) {
                if (options.textfile) {
                    target.write('#\n' +
                       '#   ' + target.basename + ' -- ' + options.title + '\n' + 
                       '#\n')
                } else {
                    target.write('/*\n' +
                       '    ' + target.basename + ' -- ' + options.title + '\n\n' +
                       '    This file is a catenation of all the source code. Amalgamating into a\n' +
                       '    single file makes embedding simpler and the resulting application faster.\n\n' + 
                       '    Prepared by: ' + System.hostname + '\n */\n\n')
                }
            }
            if (options.header) {
                target.append(options.header + '\n')
            }
        }
        if (options.textfile) {
            target.append('\n' +
               '#\n' +
               '#   Start of file \"' + from.relative + '\"\n' +
               '#\n')
        } else {
            target.append('\n' +
               '/************************************************************************/\n' +
               '/*\n    Start of file \"' + from.relative + '\"\n */\n' +
               '/************************************************************************/\n\n')
        }
        let data = from.readString()
        if (options.filter) {
            data = data.replace(options.filter, '')
        }
        target.append(data)
        target.setAttributes(options)
    }

} /* me class */

} /* embedthis.me module */

/*
    Global functions for me files
 */
require embedthis.me

/**
    Me DOM object
    @hide
 */
public var b: Me = new Me

b.main()

/** @hide */
public function builtin(command: String, options = null)
    b.builtin(command, options)

/** 
    Emit general trace
    @param tag Informational tag emitted before the message
    @param args Message args to display
 */
public function trace(tag: String, ...args): Void
    b.trace(tag, ...args)

/** 
    Emit "show" trace
    This is trace that is displayed if me --show is invoked.
    @param tag Informational tag emitted before the message
    @param args Message args to display
*/
public function strace(tag, ...args)
    b.strace(tag, ...args)
    
/** @duplicate Me.vtrace */
public function vtrace(tag, ...args)
    b.vtrace(tag, ...args)

/** @duplicate Me.copy */
public function copy(src, dest: Path, options = {})
    b.copy(src, dest, options)

/** @duplicate Me.run */
public function run(command, cmdOptions = {}): String
    b.run(command, cmdOptions)

/** @hide */
public function sh(commands, cmdOptions = {noio: true}): String
    b.sh(commands, cmdOptions)

/** @hide */
public function safeRemove(dir: Path)
    b.safeRemove(dir)

/** @hide */
public function mapLibs(target, libs: Array, static = null)
    b.mapLibs(target, libs, static)

/** @hide */
public function setRuleVars(target, dir = App.dir)
    b.setRuleVars(target, dir)

/** @hide */
public function makeDirGlobals(base: Path? = null)
    b.makeDirGlobals(base)

/** @duplicate Me.makeDir */
public function makeDir(path: Path, options = {})
    b.makeDir(path, options)

/** @duplicate Me.copyFile */
public function copyFile(src: Path, dest: Path, options = {})
    b.copyFile(src, dest, options)

/** @duplicate Me.linkFile */
public function linkFile(src: Path, dest: Path, options = {})
    b.linkFile(src, dest, options)

/** @duplicate Me.removeDir */
public function removeDir(path: Path, options = {})
    b.removeDir(path, options)

/** @duplicate Me.removeFile */
public function removeFile(path: Path, options = {})
    b.removeFile(path, options)

/** @duplicate Me.removePath */
public function removePath(path: Path)
    b.removePath(path)

/** @hide */
public function runTargetScript(target, when, options = {})
    b.runTargetScript(target, when, options)

/** @duplicate Me.whyRebuild */
public function whyRebuild(path, tag, msg)
    b.whyRebuild(path, tag, msg)

/** @duplicate Me.expand */
public function expand(s: String, options = {fill: '${}'}) : String
    b.expand(s, options)

/** @hide */
public function genScript(s)
    b.genScript(s)

/** @hide */
public function genWriteLine(str)
    b.genWriteLine(str)

/** @hide */
public function genWrite(str)
    b.genWrite(str)

/** @hide */
public function whySkip(path, msg)
    b.whySkip(path, msg)

/** @hide */
public function compareVersion(list, a, b) {
    let parts_a = list[a].match(/.*(\d+)[\-\.](\d+)[\-\.](\d+)/)
    let parts_b = list[b].match(/.*(\d+)[\-\.](\d+)[\-\.](\d+)/)
    try {
        for (i = 1; i <= 3; i++) {
            parts_a[i] -= 0
            parts_b[i] -= 0
            if (parts_a[i] < parts_b[i]) {
                return -1
            } else if (parts_a[i] > parts_b[i]) {
                return 1
            }
        }
    } catch {
        if (parts_a == null) {
            return -1
        } else if (parts_b == null) {
            return 1
        }
        return 0
    }
    return 0
}

/** @hide */
public function sortVersions(versions: Array)
    versions.sort(compareVersion).reverse()

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
