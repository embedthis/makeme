#!/usr/bin/env ejs
/*
    bit.es -- Build It! -- Embedthis Build It Framework

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.bit {

require ejs.unix
require ejs.zlib


/**
    Bit Class.
    This implements the Bit tool and provide access via the Bit DOM.
    @stability Prototype
  */
public class Bit {
    /** @hide */
    public var initialized: Boolean
    public var options: Object = { control: {}}
    public static const MAIN: Path = Path('main.bit')
    public var platforms: Array
    public static const START: Path = Path('start.bit')
    public var localPlatform: String
    public var selectedTargets: Array
    public var topTargets: Array
    public var localBin: Path
    public var currentPlatform: String?
    public var currentBitFile: Path?

    private static const supportedOS = ['freebsd', 'linux', 'macosx', 'solaris', 'vxworks', 'windows']
    private static const supportedArch = ['arm', 'i64', 'mips', 'sparc', 'x64', 'x86']
    private const ALL = 'all'

    /*
        Filter for files that look like temp files and should not be installed
     */
    private const TempFilter = /\.old$|\.tmp$|xcuserdata|xcworkspace|project.guid|-mine|\/sav\/|\/save\//

    private var appName: String = 'bit'
    private var args: Args
    private var capture: Array?
    private var local: Object
    private var missing = null
    private var out: Stream
    private var rest: Array

    private var home: Path
    private var bareBit: Object = { platforms: [], platform: {}, dir: {}, settings: { requires: [], discover: [], }, 
        packs: {}, targets: {}, env: {}, globals: {}, customSettings: {}}

    private var bit: Object = {}
    private var platform: Object

    private var goals: Array = []

    private var unix = ['macosx', 'linux', 'unix', 'freebsd', 'solaris']
    private var windows = ['windows', 'wince']
    private var start: Date
    private var targetsToBuildByDefault = { exe: true, file: true, lib: true }
    private var targetsToClean = { exe: true, file: true, lib: true, obj: true }
    /*
        Note: actions are converted to a type of build by default
     */
    private var targetsToBlend = { exe: true, lib: true, obj: true }

    private var argTemplate = {
        options: {
            benchmark: { alias: 'b' },
            bit: { range: String },
            chdir: { range: String },
            configure: { range: String },
            configuration: { },
            'continue': { alias: 'c' },
            debug: {},
            depth: { range: Number},
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
            overwrite: { },
            out: { range: String },
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
            show: { alias: 's'},
            static: { },
            unicode: {},
            unset: { range: String, separator: Array },
            verbose: { alias: 'v' },
            version: { alias: 'V' },
            why: { alias: 'w' },
            'with': { range: String, separator: Array },
            without: { range: String, separator: Array },
        },
        unknown: unknownArg,
        usage: usage
    }

    function usage(): Void {
        print('\nUsage: bit [options] [targets|goals] ...\n' +
            '  Options:\n' + 
            '  --benchmark                              # Measure elapsed time\n' +
            '  --chdir dir                              # Directory to build from\n' +
            '  --configure /path/to/source/tree         # Configure product\n' +
            '  --configuration                          # Display current configuration\n' +
            '  --continue                               # Continue on errors\n' +
            '  --debug                                  # Same as --profile debug\n' +
            '  --depth level                            # Set utest depth level\n' +
            '  --diagnose                               # Emit diagnostic trace \n' +
            '  --dump                                   # Dump the full project bit file\n' +
            '  --endian [big|little]                    # Define the CPU endianness\n' +
            '  --file file.bit                          # Use the specified bit file\n' +
            '  --force                                  # Override warnings\n' +
            '  --gen [make|nmake|sh|vs|xcode|main|start]# Generate project file\n' + 
            '  --get field                              # Get and display a bit field value\n' + 
            '  --help                                   # Print help message\n' + 
            '  --import                                 # Import standard bit environment\n' + 
            '  --keep                                   # Keep intermediate files\n' + 
            '  --log logSpec                            # Save errors to a log file\n' +
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
            '  --show                                   # Show commands executed\n' +
            '  --static                                 # Make static libraries\n' +
            '  --unicode                                # Set char size to wide (unicode)\n' +
            '  --unset feature                          # Unset a feature\n' +
            '  --version                                # Display the bit version\n' +
            '  --verbose                                # Trace operations\n' +
            '  --with PACK[=PATH]                       # Build with package at PATH\n' +
            '  --without PACK                           # Build without a package\n' +
            '')
        if (START.exists) {
            try {
                b.makeBit(Config.OS + '-' + Config.CPU, START)
                global.bit = bit = b.bit
                let bitfile: Path = START
                for (let [index,platform] in bit.platforms) {
                    bitfile = bitfile.dirname.join(platform).joinExt('bit')
                    if (bitfile.exists) {
                        b.makeBit(platform, bitfile)
                        b.prepBuild()
                    }
                    break
                }
                if (bit.usage) {
                    print('Feature Selection:')
                    for (let [item,msg] in bit.usage) {
                        print('    --set %-32s # %s' % [item + '=value', msg])
                    }
                    print('')
                }
                if (bit.packs) {
                    let header
                    Object.sortProperties(bit.packs)
                    for (name in bit.packs) {
                        let pack = bit.packs[name]
                        let desc = pack.description
                        if (!desc) {
                            let path = b.findPack(name)
                            if (path.exists) {
                                let matches = path.readString().match(/(pack|program)\(.*, '(.*)'/m)
                                if (matches) {
                                    desc = matches[2]
                                }
                            }
                        }
                        if (!bit.settings.requires.contains(name)) {
                            if (!header) {
                                print('Extension Packages (--with PACK, --without PACK):')
                                header = true
                            }
                            print('    %-38s # %s'.format([name, desc]))
                        }
                    }
                }
            } catch (e) { print('CATCH: ' + e)}
        }
        App.exit(1)
    }

    function overlay(name) {
        let src = options.configure || Path('.')
        let bits = src.join('bits/standard.bit').exists ? src.join('bits') : Config.Bin.join('bits')
        global.load(bits.join(name))
    }

    function main() {
        let start = new Date
        global._b = this
        home = App.dir
        args = Args(argTemplate)
        options = args.options
        try {
            setup(args)
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
            }
            if (options.gen) {
                overlay('generate.es')
                generate()
            } else {
                if (!options.file) {
                    let file = findStart()
                    App.chdir(file.dirname)
                    home = App.dir
                    options.file = file.basename
                }
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
            --with-pack
            --without-pack
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
                let pack = arg.trimStart('--with-')
                argv.splice(i, 1, '--with', pack)
                return --i
            }
            if (arg.startsWith('--without-')) {
                let pack = arg.trimStart('--without-')
                argv.splice(i, 1, '--without', pack)
                return --i
            }
        }
        throw "Undefined option '" + arg + "'"
    }

    /*
        Parse arguments
     */
    function setup(args: Args) {
        options.control = {}
        if (options.chdir) {
            App.chdir(options.chdir)
        }
        if (options.version) {
            print(version)
            App.exit(0)
        }
        if (options.help || args.rest.contains('help')) {
            usage()
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
        localPlatform =  Config.OS + '-' + Config.CPU + '-' + (options.release ? 'release' : 'debug')
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
            if (!Path(localPlatform + '.bit').exists) {
                trace('Generate', 'Create platform bit file: ' + localPlatform + '.bit')
            }
            /* Must continue if probe can't locate tools, but does know a default */
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
        if (goals[0] == 'version') {
            print(bit.settings.version + '-' + bit.settings.buildNumber)
            App.exit()
        }
        bareBit.options = options
    }

    function getValue() {
        eval('print(serialize(bit.' + options.get + ', {pretty: true, quotes: false}))')
    }

    function showConfiguration() {
        print("// Configuration for Platform: " + bit.platform.name)
        print("\npacks:")
        print(serialize(bit.packs, {pretty: true, quotes: false}))
        print("\nsettings:")
        print(serialize(bit.settings, {pretty: true, quotes: false}))
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
            bit.settings.debug = true
        }
        if (options.release) {
            bit.settings.debug = false
        }
        if (bit.settings.debug == undefined) {
            bit.settings.debug = true
        }
        /* Disable/enable was originally --unset|--set */
        for each (field in poptions.disable) {
            bit.settings[field] = false
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
            Object.getOwnPropertyNames(bit.packs)
            let packs = bit.settings.requires + bit.settings.discover + Object.getOwnPropertyNames(bit.packs)
            if (packs.contains(field)) {
                App.log.error("Using \"--set " + field + "\", but " + field + " is an extension package. " + 
                        "Use --with or --without instead.")
                App.exit(1)
            }
            setSetting(bit.settings, field, value)
        }
        let requires = []
        for each (field in poptions['with']) {
            let [field,value] = field.split('=')
            bit.packs[field] ||= {}
            let pack = bit.packs[field]
            if (value) {
                pack.enable = true
                pack.withpath = Path(value)
            }
            pack.explicit = true
            pack.required = true
            if (!bit.settings.requires.contains(field) && !bit.settings.discover.contains(field)) {
                let path = findPack(field)
                if (!path || !path.exists) {
                    throw 'Cannot find pack description file: ' + field + '.pak'
                }
                requires.push(field)
            }
        }
        if (requires.length > 0) {
            /* Insert explicit requires first */
            bit.settings.requires = requires + bit.settings.requires
        }
        for each (field in poptions['without']) {
            if (bit.settings.requires.contains(field)) { 
                throw 'Required pack ' + field + ' cannot be disabled.'
            }
            if (field != 'all' && field != 'default') {
                let path = findPack(field)
                if (!path || !path.exists) {
                    throw 'Cannot find pack description file: ' + field + '.pak'
                }
            }
            bit.packs[field] ||= {}
            let pack = bit.packs[field]
            if ((field == 'all' || field == 'default') && bit.settings['without-' + field]) {
                for each (f in bit.settings['without-' + field]) {
                    bit.packs[f] ||= {}
                    let pack = bit.packs[f]
                    pack.enable = false
                    pack.explicit = true
                    pack.diagnostic = 'configured --without ' + f + '.'
                }
                continue
            }
            pack.enable = false
            pack.diagnostic = 'configured --without ' + field + '.'
            pack.explicit = true
        }
    }

    /*
        Apply the selected build profile
     */
    function applyProfile() {
        if (bit.profiles && bit.profiles[bit.platform.profile]) {
            blend(bit, bit.profiles[bit.platform.profile], {combine: true})
        }
    }

    /** @hide */
    public function findPack(pack) {
        let path = Path(bit.dir.bits).join('packs', pack + '.bit')
        if (!path.exists) {
            for each (d in bit.settings.packs) {
                path = Path(bit.dir.src).join(d, pack + '.bit')
                if (path.exists) {
                    break
                }
            }
        }
        return path
    }

    function process(bitfile: Path) {
        if (!bitfile.exists) {
            throw 'Cannot find ' + bitfile
        }
        let ver
        if (MAIN.exists) {
            quickLoad(MAIN)
            ver = bit.settings.version + '-' + bit.settings.buildNumber
        }
        quickLoad(bitfile)
        if (bit.platforms) {
            platforms = bit.platforms
            for (let [index,platform] in bit.platforms) {
                bitfile = bitfile.dirname.join(platform).joinExt('bit')
                makeBit(platform, bitfile)
                if (index == (bit.platforms.length - 1)) {
                    bit.platform.last = true
                }
                if (ver && (ver != (bit.settings.version + '-' + bit.settings.buildNumber))) {
                    trace('Upgrade', 'Main.bit has been updated, reconfiguring ...')
                    reconfigure()
                }
                if (options.configuration) {
                    showConfiguration()
                    continue
                }
                prepBuild()
                if (options.hasOwnProperty('get')) {
                    getValue()
                } else {
                    build()
                }
                if (!options.configure && (bit.platforms.length > 1 || bit.platform.cross)) {
                    trace('Complete', bit.platform.name)
                }
            }
        } else {
            platforms = bit.platforms = [localPlatform]
            makeBit(localPlatform, bitfile)
            bit.platform.last = true
            prepBuild()
            if (options.hasOwnProperty('get')) {
                getValue()
            } else {
                build()
            }
        }
    }

    function loadModules() {
        App.log.debug(2, "Bit Modules: " + serialize(bit.modules, {pretty: true}))
        for each (let module in bit.modules) {
            App.log.debug(2, "Load bit module: " + module)
            try {
                global.load(module)
            } catch (e) {
                throw new Error('When loading: ' + module + '\n' + e)
            }
        }
    }

    public function loadBitFile(path) {
        let saveCurrent = currentBitFile
        try {
            currentBitFile = path.portable
            vtrace('Loading', currentBitFile)
            global.load(path)
        } finally {
            currentBitFile = saveCurrent
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
            let home = currentBitFile.dirname
            for (let [event,item] in o.scripts) {
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
            /*
                for (let [event,item] in o.scripts) {
                    plus(o.scripts, event)
                }
             */
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
        if (target.generateScript && !target.goals.contains('generate')) {
            target.goals.push('generate')
        }
    }

    function fixTarget(o, tname, target) {
        target.home = absPath(target.home)
        let home = target.home
        if (target.path) {
            // target.path = absPath(target.path)
            rebase(home, target, 'path')
        }

        //  LEGACY
        if (target.require) {
            trace('Warn', 'Target ' + target.name + ' in ' + currentBitFile + ' is using require instead of requires')
            rename(target, 'require', 'requires')
        }
        for (let [key,value] in target.defines) {
            target.defines[key] = value.trimStart('-D')
        }
        if (target.requires) {
            target.requires = makeArray(target.requires)
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
        /*
            Expand short-form scripts into the long-form. Set the target type if not defined to 'script'.
         */
        let build = target.build
        for each (n in ['action', 'build', 'shell', 'postblend', 'preresolve', 'postresolve', 'postsource', 
                'precompile', 'postcompile', 'prebuild', 'postbuild']) {
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
        fixScripts(target)
        fixGoals(target, build)

        /*
            Blend internal for only the targets in this file. Delay blending defaults.
         */
        if (o.internal) {
            // blend(target, o.internal, {combine: true})
            let base = blend({}, o.internal, {combine: true})
            target = o.targets[tname] = bit.targets[tname] = blend(base, target, {combine: true})
        }
    }

    function fixup(o, ns) {
        let home = currentBitFile.dirname
    
        //  LEGACY
        for each (f in ['+defaults', '+internal']) {
            if (o[f]) {
                throw "Using " + f + " in " + currentBitFile + ". Do not use a plus prefix."
            }
        }
        //  LEGACY
        if (o.settings) {
            if (o.settings.required) {
                trace("Warn", "Using settings.required in " + currentBitFile + ". Use settings.requires instead.")
            }
            //  LEGACY property renaming
            rename(o.settings, 'required', 'requires')
        }
        /*
            Arrays must have a +prefix to blend
         */
        plus(o, 'modules')
        plus(o.settings, 'requires')
        plus(o.defaults, 'includes')
        plus(o.internal, 'includes')

        rebase(home, o, 'modules')
        rebase(home, o.defaults, 'includes')
        rebase(home, o.internal, 'includes')

        fixScripts(o)
        fixScripts(o.defaults)
        fixScripts(o.internal)
        for each (pack in o.packs) {
            fixScripts(pack, ['config', 'without', 'postconfig'])
        }

        for (let [tname,target] in o.targets) {
            target.name ||= tname
            target.home ||= home
            fixTarget(o, tname, target)
        }
    }

    /** @hide */
    public function loadBitObject(o, ns = null) {
        let home = currentBitFile.dirname
        fixup(o, ns)

        if (o.scripts && o.scripts.preblend) {
            runScript(o.scripts, "preblend")
            delete o.scripts.preblend
        }
        /* 
            Blending is depth-first -- blend this bit object after loading bit files referenced in blend[]
            Special case for the local plaform bit file to provide early definition of platform and dir properties
         */
        if (o.dir) {
            blend(bit.dir, o.dir, {combine: true})
        }
        if (o.platform) {
            blend(bit.platform, o.platform, {combine: true})
        }
        if (!bit.quickLoad) {
            for each (path in o.blend) {
                bit.globals.BITS = bit.dir.bits
                bit.globals.SRC = bit.dir.src
                if (path.startsWith('?')) {
                    path = home.join(expand(path.slice(1), {fill: null}))
                    if (path.exists) {
                        loadBitFile(path)
                    } else {
                        vtrace('SKIP', 'Skip blending optional ' + path.relative)
                    }
                } else {
                    path = home.join(expand(path, {fill: null}))
                    loadBitFile(path)
                }
            }
        }
        /*
            Delay blending defaults into targets until blendDefaults. 
            This is because 'combine: true' erases the +/- property prefixes.
         */
        if (o.targets) {
            bit.targets ||= {}
            bit.targets = blend(bit.targets, o.targets, {functions: true})
            delete o.targets
        }
        bit = blend(bit, o, {combine: true, functions: true})
        if (o.scripts && o.scripts.postload) {
            runScript(bit.scripts, "postload")
            delete bit.scripts.postload
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
                vtrace('Info', 'Using bit file ' + f)
                return f
            }
        }
        if (Path(MAIN).exists) {
            throw 'Cannot find suitable ' + START + '.\nRun "bit configure" first.'
        } else {
            throw 'Cannot find suitable ' + START + '.\nRun "bit --gen start" to create stub start.bit'
        }
        return null
    }

    function import() {
        let bin = Path(Config.Bin)
        for each (dest in bin.files('bits/**', {relative: true})) {
            let src = bin.join(dest)
            if (src.isDir) {
                mkdir(dest.dirname, 0755)
            } else {
                safeCopy(src, dest)
            }
        }
    }

    public function prepBuild() {
        vtrace('Prepare', 'For building')
        if (!options.configure && (bit.platforms.length > 1 || bit.platform.cross)) {
            trace('Build', bit.platform.name)
            vtrace('Targets', bit.platform.name + ': ' + ((selectedTargets != '') ? selectedTargets: 'nothing to do'))
        }
        /* 
            When cross generating, certain wild cards can't be resolved.
            Setting missing to empty will cause missing glob patterns to be replaced with the pattern itself 
         */
        if (options.gen || options.configure) {
            missing = ''
        }
        makeConstGlobals()
        makeDirGlobals()
        enableTargets()
        blendDefaults()
        resolveDependencies()
        expandWildcards()
        castTargetTypes()
        setTargetPaths()
        inlineStatic()
        Object.sortProperties(bit)

        if (options.dump) {
            let o = bit.clone()
            delete o.blend
            let path = Path(currentPlatform + '.dmp')
            path.write(serialize(o, {pretty: true, commas: true, indent: 4, quotes: false}))
            trace('Dump', 'Save Bit DOM to: ' + path)
        }
    }

    /*
        Determine which targets are enabled for building on this platform
     */
    function enableTargets() {
        for (let [tname, target] in bit.targets) {
            let reported = false
            for each (item in target.requires) {
                if (!bit.packs[item] || !bit.packs[item].enable) {
                    whySkip(target.name, 'disabled because the pack ' + item + ' is not enabled')
                    target.enable = false
                    reported = true
                }
            }
            if (target.enable) {
                if (!(target.enable is Boolean)) {
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
                }
                target.name ||= tname
            } else if (target.enable == undefined) {
                target.enable = true
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

    function getDependentTargets(target, goal) {
        if (target.selected || !target.enable) {
            return
        }
        if (goal === true || target.goals.contains(goal)) {
            target.selected = true
            for each (dname in target.depends) {
                if (dname == ALL) {
                    for each (target in bit.targets) {
                        getDependentTargets(target, dname)
                    }
                } else {
                    let dep = bit.targets[dname]
                    if (dep) {
                        if (!dep.selected) {
                            getDependentTargets(dep, true)
                        }
                    } else if (!Path(dname).exists && !bit.packs[dname]) {
                        throw 'Unknown dependency "' + dname + '" in target "' + target.name + '"'
                    }
                }
            }
            selectedTargets.push(target)
            if (goal !== true) {
                topTargets.push(target)
            }
        }
    }

    public function selectTargets(goal): Array {
        selectedTargets = []
        topTargets = []
        for each (target in bit.targets) {
            delete target.selected
        }
        for each (target in bit.targets) {
            getDependentTargets(target, goal)
        }
        if (selectedTargets.length == 0) {
            if (goal != 'all') {
                trace('Info', 'No qualifying targets for goal "' + goal + '"')
                // throw 'No targets for goal "' + goal + '"'
            }
        }
        return selectedTargets
    }

    /*
        Set target output paths. Uses the default locations for libraries, executables and files
     */
    function setTargetPaths() {
        for each (target in bit.targets) {
            if (!target.path) {
                if (target.type == 'lib') {
                    if (target.static) {
                        target.path = bit.dir.lib.join(target.name).joinExt(bit.ext.lib, true)
                    } else {
                        target.path = bit.dir.lib.join(target.name).joinExt(bit.ext.shobj, true)
                    }
                } else if (target.type == 'obj') {
                    target.path = bit.dir.obj.join(target.name).joinExt(bit.ext.o, true)
                } else if (target.type == 'exe') {
                    target.path = bit.dir.bin.join(target.name).joinExt(bit.ext.exe, true)
                } else if (target.type == 'file') {
                    target.path = bit.dir.lib.join(target.name)
                } else if (target.type == 'res') {
                    target.path = bit.dir.res.join(target.name).joinExt(bit.ext.res, true)
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
        }
    }

    function getDepLibs(target): Array {
        let libs = []
        for each (dname in target.depends) {
            let dep = bit.targets[dname]
            if (dep && dep.type == 'lib' && dep.enable) {
                libs += getDepLibs(dep)
                libs.push(dname)
            }
        }
        return libs
    }

    /*
        Implement static linking by inlining all libraries
     */
    function inlineStatic() {
        for each (target in selectedTargets) {
            if (target.static) {
                let resolved = []
                let includes = []
                let defines = []
                if (target.type == 'exe') {
                    for each (dname in getDepLibs(target).unique()) {
                        let dep = bit.targets[dname]
                        if (dep && dep.type == 'lib' && dep.enable) {
                            /* Add the dependent files to the target executables */
                            target.files += dep.files
                            includes += dep.includes
                            defines += dep.defines
                            if (dep.static) {
                                resolved.push(Path(dname).joinExt(bit.ext.lib, true))
                            } else if (dname.startsWith('lib')) {
                                resolved.push(dname.replace(/^lib/g, ''))
                            } else {
                                resolved.push(Path(dname).joinExt(bit.ext.shlib, true))
                            }
                        }
                    }
                }
                target.libraries -= resolved
                target.includes += includes
                target.defines += defines
                target.includes = target.includes.unique()
                target.defines = target.defines.unique()
            }
        }
    }

    /*
        Build a file list and apply include/exclude filters
        Include may be an array. Exclude will only ever be a RegExp|String
     */
    function buildFileList(target, include, exclude = null) {
        if (!target.copytemp) {
            if (exclude) {
                exclude = TempFilter + '|' + exclude
            } else {
                exclude = TempFilter
            }
        }
        let files
        if (include is RegExp) {
            /* Fast path */
            if (exclude is RegExp) {
                files = Path(bit.dir.src).files('*', {include: include, exclude: exclude, missing: missing})
            } else {
                files = Path(bit.dir.src).files('*', {include: include, missing: missing})
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

    /*
        Resolve a target by inheriting dependent libraries
     */
    function resolve(target) {
        if (target.resolved) {
            return
        }
        runTargetScript(target, 'preresolve')
        target.resolved = true
        for each (dname in target.depends) {
            let dep = bit.targets[dname]
            if (dep) {
                if (!dep.enable) continue
                if (!dep.resolved) {
                    resolve(dep)
                }
                if (dep.type == 'lib') {
                    target.libraries
                    target.libraries ||= []
                    /* Put dependent libraries first so system libraries are last (matters on linux) */
                    if (dep.static) {
                        target.libraries = [Path(dname).joinExt(bit.ext.lib)] + target.libraries
                    } else {
                        if (dname.startsWith('lib')) {
                            target.libraries = [dname.replace(/^lib/, '')] + target.libraries
                        } else {
                            target.libraries = [Path(dname).joinExt(bit.ext.shlib, true)] + target.libraries
                        }
                    }
                    for each (lib in dep.libraries) {
                        if (!target.libraries.contains(lib)) {
                            target.libraries.push(lib)
                        }
                    }
                    for each (option in dep.linker) {
                        target.linker ||= []
                        if (!target.linker.contains(option)) {
                            target.linker.push(option)
                        }
                    }
                    for each (option in dep.libpaths) {
                        target.libpaths ||= []
                        if (!target.libpaths.contains(option)) {
                            target.libpaths.push(option)
                        }
                    }
                }
            } else {
                let pack = bit.packs[dname]
                if (pack) {
                    if (!pack.enable) continue
                    if (pack.includes) {
                        target.includes ||= []
                        target.includes += pack.includes
                    }
                    if (pack.defines) {
                        target.defines ||= []
                        target.defines += pack.defines
                    }
                    if (pack.libraries) {
                        target.libraries ||= []
                        target.libraries += pack.libraries
                    }
                    if (pack.linker) {
                        target.linker ||= []
                        target.linker += pack.linker
                    }
                    if (pack.libpaths) {
                        target.libpaths ||= []
                        target.libpaths += pack.libpaths
                    }
                }
            }
        }
        runTargetScript(target, 'postresolve')
    }

    function resolveDependencies() {
        for each (target in bit.targets) {
            if (target.enable) {
                resolve(target)
            }
        }
        for each (target in bit.targets) {
            delete target.resolved
        }
    }

    /*
        Expand resources, sources and headers. Support include+exclude and create target.files[]
     */
    function expandWildcards() {
        let index
        for each (target in bit.targets) {
            if (!target.enable) {
                continue
            }
            runTargetScript(target, 'presource')
            if (target.files) {
                target.files = buildFileList(target, target.files, target.exclude)
            }
            if (target.headers) {
                /*
                    Create a target for each header file
                 */
                target.files ||= []
                let files = buildFileList(target, target.headers, target.exclude)
                for each (file in files) {
                    let header = bit.dir.inc.join(file.basename)
                    /* Always overwrite dynamically created targets created via makeDepends */
                    bit.targets[header] = { name: header, enable: true, path: header, type: 'header', 
                        goals: [target.name], files: [ file ], includes: target.includes }
                    target.depends ||= []
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
                    let res = bit.dir.obj.join(file.replaceExt(bit.ext.res).basename)
                    let resTarget = { name : res, enable: true, path: res, type: 'resource', 
                        goals: [target.name], files: [ file ], includes: target.includes }
                    if (bit.targets[res]) {
                        resTarget = blend(bit.targets[resTarget.name], resTarget, {combined: true})
                    }
                    bit.targets[resTarget.name] = resTarget
                    target.files.push(res)
                    target.depends ||= []
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
                    let obj = bit.dir.obj.join(file.replaceExt(bit.ext.o).basename)
                    let objTarget = { name : obj, enable: true, path: obj, type: 'obj', 
                        goals: [target.name], files: [ file ], 
                        compiler: target.compiler, defines: target.defines, includes: target.includes}
                    let precompile = (target.scripts && target.scripts.precompile) ?  target.scripts.precompile : null
                    if (precompile) {
                        objTarget.scripts = {precompile: precompile}
                    }
                    if (bit.targets[obj]) {
                        objTarget = blend(bit.targets[objTarget.name], objTarget, {combined: true})
                    }
                    bit.targets[objTarget.name] = objTarget
                    target.files.push(obj)
                    target.depends ||= []
                    target.depends.push(obj)

                    /*
                        Create targets for each header (if not already present)
                     */
                    makeDepends(objTarget)
                }
            }
            runTargetScript(target, 'postsource')
        }
    }

    /*
        Blend bit.defaults into targets. The defaults are the base, then the target is blended over the defaults.
        This delayed until all bit files are blended so that the target property +/- prefixes are not lost in prior blends.
        Also allows events to modify defaults up to the last minute.
     */
    function blendDefaults() {
        runScript(bit.scripts, "preinherit")
        for (let [tname, target] in bit.targets) {
            if (targetsToBlend[target.type]) {
                let def = blend({}, bit.defaults, {combine: true})
                target = bit.targets[tname] = blend(def, target, {combine: true})
                if (target.inherit) {
                    if (!(target.inherit is Array)) {
                        target.inherit = [ target.inherit ]
                    }
                    for each (from in target.inherit) {
                        blend(target, bit[from], {combine: true})
                    }
                }
                runTargetScript(target, 'postblend')
                if (target.type == 'obj') { 
                    delete target.linker 
                    delete target.libpaths 
                    delete target.libraries 
                }
            }
            if (target.type == 'lib' && target.static == null && bit.settings.static) {
                target.static = bit.settings.static
            }
        }
    }

    public function castDirTypes() {
        /*
            Use absolute patsh so they will apply anywhere in the source tree. Rules change directory and build
            locally for each directory, so it is essential these be absolute.
         */
        for (let [key,value] in bit.blend) {
            bit.blend[key] = Path(value).absolute.portable
        }
        for (let [key,value] in bit.dir) {
            bit.dir[key] = Path(value).absolute
        }
        let defaults = bit.defaults
        if (defaults) {
            for (let [key,value] in defaults.includes) {
                defaults.includes[key] = Path(value).absolute
            }
            for (let [key,value] in defaults.libpaths) {
                defaults.libpaths[key] = Path(value).absolute
            }
        }
        for (let [pname, prefix] in bit.prefixes) {
            bit.prefixes[pname] = Path(prefix)
            if (bit.platform.os == 'windows') {
                if (Config.OS == 'windows') {
                    bit.prefixes[pname] = bit.prefixes[pname].absolute
                }
            } else {
                bit.prefixes[pname] = bit.prefixes[pname].normalize
            }
        }
        for each (pack in bit.packs) {
            if (pack.dir) {
                pack.dir = Path(pack.dir).absolute
            }
            if (pack.path) {
                /* Must not make absolute incase pack resolves using PATH at run-time. e.g. cc */
                pack.path = Path(pack.path)
            }
            for (let [key,value] in pack.includes) {
                if (!value.startsWith('$')) {
                    pack.includes[key] = Path(value).absolute
                } else {
                    pack.includes[key] = Path(value)
                }
            }
            for (let [key,value] in pack.libpaths) {
                if (!value.startsWith('$')) {
                    pack.libpaths[key] = Path(value).absolute
                } else {
                    pack.includes[key] = Path(value)
                }
            }
        }
    }

    function castTargetTypes() {
        for each (target in bit.targets) {
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
        }
    }

    /*
        Build all selected targets
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

                if (bit.generating) {
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
    }

    function buildDir(target) {
        makeDir(expand(target.dir), target)
    }

    function buildExe(target) {
        let transition = target.rule || 'exe'
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Link', target.name)
        if (target.active && bit.platform.like == 'windows') {
            let old = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + old)
            old.remove()
            target.path.rename(old)
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /Creating library /})
    }

    function buildSharedLib(target) {
        let transition = target.rule || 'shlib'
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Link', target.name)
        if (target.active && bit.platform.like == 'windows') {
            let active = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
            active.remove()
            target.path.rename(target.path.replaceExt('old'))
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /Creating library /})
    }

    function buildStaticLib(target) {
        let transition = target.rule || 'lib'
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = expandRule(target, rule)
        trace('Archive', target.name)
        if (target.active && bit.platform.like == 'windows') {
            let active = target.path.relative.replaceExt('old')
            trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
            active.remove()
            target.path.rename(target.path.replaceExt('old'))
        } else {
            safeRemove(target.path)
        }
        run(command, {excludeOutput: /has no symbols|Creating library /})
    }

    /*
        Build symbols file for windows libraries
     */
    function buildSym(target) {
        let rule = bit.rules['sym']
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
            if (bit.platform.arch == 'x64') {
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
            let rule = target.rule || bit.rules[transition]
            if (!rule) {
                rule = bit.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = expandRule(target, rule)
            trace('Compile', file.relativeTo('.'))
            if (bit.platform.os == 'windows') {
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
            let rule = target.rule || bit.rules[transition]
            if (!rule) {
                rule = bit.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = expandRule(target, rule)
            trace('Compile', file.relativeTo('.'))
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
        run(command)
    }

    /*
        Copy files[] to path
     */
    function buildFile(target) {
        if (target.path.exists && !target.path.isDir && !target.type == 'header') {
            if (target.active && bit.platform.like == 'windows') {
                let active = target.path.relative.replaceExt('old')
                trace('Preserve', 'Active target ' + target.path.relative + ' as ' + active)
                active.remove()
                target.path.rename(target.path.replaceExt('old'))
            } else {
                safeRemove(target.path)
            }
        }
        trace('Copy', target.path.relative.portable)
        for each (let file: Path in target.files) {
            if (file == target.path) {
                /* Auto-generated headers targets for includes have file == target.path */
                continue
            }
            copy(file, target.path, target)
        }
        if (target.path.isDir && !bit.generating) {
            let touch = Path(target.path).join('.touch')
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
    }

    /**
        Set top level constant variables. This enables them to be used in token expansion.
        @hide.
     */
    public function makeConstGlobals() {
        let g = bit.globals
        g.PLATFORM = bit.platform.name
        g.OS = bit.platform.os
        g.CPU = bit.platform.cpu || 'generic'
        g.ARCH = bit.platform.arch
        /* Apple gcc only */
        if (bit.platform['arch-map']) {
            g.CC_ARCH = bit.platform['arch-map'][bit.platform.arch] || bit.platform.arch
        }
        g.CONFIG = bit.platform.name
        g.EXE = bit.ext.dotexe
        g.LIKE = bit.platform.like
        g.O = bit.ext.doto
        g.SHOBJ = bit.ext.dotshobj
        g.SHLIB = bit.ext.dotshlib
        if (bit.settings.hasMtune && bit.platform.cpu) {
            g.MTUNE = '-mtune=' + bit.platform.cpu
        }
    }

    /**
        Called in this file and in xcode.es during project generation
        @hide
     */
    public function makeDirGlobals(base: Path? = null) {
        for each (n in ['BIN', 'OUT', 'BITS', 'FLAT', 'INC', 'LIB', 'OBJ', 'PACKS', 'PKG', 'REL', 'SRC', 'TOP']) {
            /* 
                These globals are always in portable format so they can be used in build scripts. Windows back-slashes
                require quoting! 
             */ 
            let dir = bit.dir[n.toLower()]
            if (!dir) continue
            dir = dir.portable
            if (base) {
                dir = dir.relativeTo(base)
            }
            global[n] = bit.globals[n] = dir
        }
        if (base) {
            bit.globals.LBIN = localBin.relativeTo(base)
        } else {
            bit.globals.LBIN = localBin
        }
    }

    public function setRuleVars(target, base: Path = App.dir) {
        let tv = target.vars || {}
        if (target.home) {
            tv.HOME = Path(target.home).relativeTo(base)
        }
        if (target.path) {
            tv.OUTPUT = target.path.relativeTo(base)
        }
        if (target.libpaths) {
            tv.LIBPATHS = mapLibPaths(target.libpaths, base)
        }
        if (target.entry) {
            tv.ENTRY = target.entry[target.rule || target.type]
        }
        if (target.type == 'exe') {
            if (!target.files) {
                throw 'Target ' + target.name + ' has no input files or sources'
            }
            tv.INPUT = target.files.map(function(p) p.relativeTo(base)).join(' ')
            tv.LIBS = mapLibs(target.libraries, target.static)
            tv.LDFLAGS = (target.linker) ? target.linker.join(' ') : ''

        } else if (target.type == 'lib') {
            if (!target.files) {
                throw 'Target ' + target.name + ' has no input files or sources'
            }
            tv.INPUT = target.files.map(function(p) p.relativeTo(base)).join(' ')
            tv.LIBNAME = target.path.basename
            tv.DEF = Path(target.path.relativeTo(base).toString().replace(/dll$/, 'def'))
            tv.LIBS = mapLibs(target.libraries, target.static)
            tv.LDFLAGS = (target.linker) ? target.linker.join(' ') : ''

        } else if (target.type == 'obj') {
            tv.CFLAGS = (target.compiler) ? target.compiler.join(' ') : ''
            tv.DEFINES = target.defines.map(function(e) '-D' + e).join(' ')
            if (bit.generating) {
                /* Use abs paths to reppath can substitute as much as possible */
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '-I' + p) : ''
            } else {
                /* Use relative paths to shorten trace output */
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '-I' + p.relativeTo(base)) : ''
            }
            tv.PDB = tv.OUTPUT.replaceExt('pdb')
            if (bit.dir.home.join('.embedthis').exists && !bit.generating) {
                tv.CFLAGS += ' -DEMBEDTHIS=1'
            }

        } else if (target.type == 'resource') {
            tv.OUTPUT = target.path.relative
            tv.CFLAGS = (target.compiler) ? target.compiler.join(' ') : ''
            target.defines ||= []
            tv.DEFINES = target.defines.map(function(e) '-D' + e).join(' ')
            tv.INCLUDES = (target.includes) ? target.includes.map(function(path) '-I' + path.relative) : ''
        }
        target.vars = tv
    }

    function applyEnv(bit) {
        let outbin = Path('.').join(bit.platform.name, 'bin').absolute
        let sep = App.SearchSeparator
        if (bit.generating) {
            outbin = outbin.relative
        }
        App.putenv('PATH', outbin + sep + App.getenv('PATH'))
        App.log.debug(2, "PATH=" + App.getenv('PATH'))
    }

    /**
        Run an event script in the directory of the bit file
        @hide
     */
    public function runTargetScript(target, when) {
        if (!target.scripts) return
        for each (item in target.scripts[when]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                App.chdir(expand(item.home))
            }
            global.TARGET = bit.target = target
            try {
                if (item.interpreter == 'ejs') {
                    if (item.script is Function) {
                        item.script.call(this, target)
                    } else {
                        let script = expand(item.script).expand(target.vars, {fill: ''})
                        script = 'require ejs.unix\n' + script
                        eval(script)
                    }
                } else {
                    runShell(target, item.interpreter, item.script)
                }
            } finally {
                App.chdir(pwd)
                global.TARGET = null
                delete bit.target
            }
        }
    }

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

    public function mapLibPaths(libpaths: Array, base: Path = App.dir): String {
        if (bit.platform.os == 'windows') {
            return libpaths.map(function(p) '-libpath:' + p.relativeTo(base)).join(' ')
        } else {
            return libpaths.map(function(p) '-L' + p.relativeTo(base)).join(' ')
        }
    }

    /**
        Map libraries into the appropriate O/S dependant format
        @hide
     */
    public function mapLibs(libs: Array, static = null): Array {
        if (bit.platform.os == 'windows') {
            libs = libs.clone()
            for (let [i,name] in libs) {
                let libname = Path('lib' + name).joinExt(bit.ext.shlib)
                if (bit.targets['lib' + name] || bit.dir.lib.join(libname).exists) {
                    libs[i] = libname
                }
            }
        } else if (bit.platform.os == 'vxworks') {
            libs = libs.clone()
            for (i = 0; i < libs.length; i++) {
                if (libs.contains(libs[i])) {
                    libs.remove(i)
                    i--
                }
            }
            for (i in libs) {
                let llib = bit.dir.lib.join("lib" + libs[i]).joinExt(bit.ext.shlib).relative
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
        return libs
    }

    /*
        Test if a target is stale vs the inputs AND dependencies
     */
    function stale(target) {
        if (bit.generating) {
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
                let p = path.join(file.trimStart(target.subtree + '/'))
                if (!file.isDir && file.modified > p.modified) {
                    whyRebuild(path, 'Rebuild', 'input ' + file + ' has been modified.')
                    if (options.why && options.verbose) {
                        print(file, file.modified)
                        print(path, path.modified)
                    }
                    return true
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
        for each (let dname: Path in target.depends) {
            let file
            if (!bit.targets[dname]) {
                let pack = bit.packs[dname]
                if (pack) {
                /* UNUSED - don't make stale if pack has no path
                    if (!pack.enable) {
                        continue
                    }
                    file = pack.path
                    if (!file) {
                        whyRebuild(path, 'Rebuild', 'missing ' + file + ' for package ' + dname)
                        return true
                    }
                    */
                } else {
                    /* If dependency is not a target, then treat as a file */
                    if (!dname.modified) {
                        whyRebuild(path, 'Rebuild', 'missing dependency ' + dname)
                        return true
                    }
                    if (dname.modified > path.modified) {
                        whyRebuild(path, 'Rebuild', 'dependency ' + dname + ' has been modified.')
                        return true
                    }
                    return false
                }
            } else {
                file = bit.targets[dname].path
            }
            if (file.modified > path.modified) {
                whyRebuild(path, 'Rebuild', 'dependent ' + file + ' has been modified.')
                return true
            }
        }
        return false
    }

    /*
        Create an array of dependencies for a target
     */
    function makeDepends(target): Array {
        let includes: Array = []
        /*
            FUTURE - persist dependencies

            for each (path in target.files) {
                if (path.modified <= bit.settings.lastBuild) {
                    continue
                }
                let dep = bit.targets[path]
                if (dep) {
                    for each (h in dep.depends) {
                    let str = path.readString()
                    let more = str.match(/^#include.*"$/gm)
                    if (more) {
                        includes += more
                    }
                }
            }
         */
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
        let bith = bit.dir.inc.join('bit.h')
        if ((target.type == 'obj' || target.type == 'lib' || target.type == 'exe') && target.name != bith) {
            depends = [ bith ]
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
                path = bit.dir.inc.join(ifile)
            }
            if (path && !depends.contains(path)) {
                depends.push(path)
            }
        }
        target.makedep = true
        for each (header in depends) {
            if (!bit.targets[header]) {
                bit.targets[header] = { name: header, enable: true, path: Path(header),
                    type: 'header', goals: [target.name], files: [ header ], includes: target.includes }
            }
            let h = bit.targets[header]
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
        return depends
    }

    /*
        Expand tokens in all fields in an object hash. This is used to expand tokens in bit file objects.
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
        @option show Show the command line before executing. Similar to bit --show, but operates on just this command.
        @option noshow Do not show the command line before executing. Useful to override bit --show for one command.
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
        if (bit.env) {
            let env = App.env.clone()
            for (let [key,value] in bit.env) {
                if (value is Array) {
                    value = value.join(App.SearchSeparator)
                }
                if (bit.platform.os == 'windows') {
                    /* Replacement may contain $(VS) */
                    if (!bit.packs.compiler.dir.contains('$'))
                        value = value.replace(/\$\(VS\)/g, bit.packs.compiler.dir)
                    if (!bit.packs.winsdk.path.contains('$'))
                        value = value.replace(/\$\(SDK\)/g, bit.packs.winsdk.path)
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
                trace('Error', msg)
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

    /*
        Make required output directories (carefully). Only make dirs inside the 'src' or 'top' directories.
     */
    public function makeOutDirs() {
        for each (d in bit.dir) {
            if (d.startsWith(bit.dir.top) || d.startsWith(bit.dir.src)) {
                d.makeDir()
            }
        }
        Path(bit.dir.out).join('test.setup').write('test.skip("Skip platform directory")\n')
    }

    function safeCopy(from: Path, to: Path) {
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
            out.write(msg)
        }
    }

    /** 
        Emit "show" trace
        This is trace that is displayed if bit --show is invoked.
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
        This is trace that is displayed if bit --verbose is invoked.
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
        Emit trace for bit --why on why a target is being rebuilt
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
        Emit trace for bit --why on why a target is being skipped
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
            for each (target in bit.targets) {
                if (target.enable && !target.precious && !target.nogen && target.path && targetsToClean[target.type]) {
                    /* UNUSED
                        if (bit.generating == 'make') {
                            genWriteLine('\trm -rf ' + reppath(target.path))
                        } else if (bit.generating == 'nmake') {
                            genout.writeLine('\t-if exist ' + reppath(target.path) + ' del /Q ' + reppath(target.path))
                        } else if (bit.generating == 'sh') {
                            genWriteLine('rm -rf ' + target.path.relative)
                        }
                    */
                    if (bit.generating) {
                        removeFile(reppath(target.path))
                    } else {
                        if (target.path.exists) {
                            if (options.show) {
                                trace('Clean', target.path.relative)
                            }
                            safeRemove(target.path)
                        }
                        if (Config.OS == 'windows') {
                            let ext = target.path.extension
                            if (ext == bit.ext.shobj || ext == bit.ext.exe) {
                                target.path.replaceExt('lib').remove()
                                target.path.replaceExt('pdb').remove()
                                target.path.replaceExt('exp').remove()
                            }
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
        Return the program files for 32 bit. Will be either /Program Files for 32-bit, or /Program Files (x86) for 64-bit
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
        Load an object into the Bit DOM
        @param obj Object collection to load into the DOM
        @param ns Reserved
     */
    public static function load(obj: Object, ns = null) {
        b.loadBitObject(obj, ns)
    }

    //  MOB DOC
    public static function pack(obj: Object) {
        b.loadBitObject({packs: obj} )
    }

    /**
        Define a pack for a command line program.
        This registers the pack and loads the Bit DOM with the pack configuration.
        @param name Program name. Can be either a path or a basename with optional extension
        @param description Short, single-line program description.
     */
    public static function program(name: Path, description = null): Path {
        let path = bit.packs[name.trimExt()].path || name
        let pack = {}
        let cfg = {description: description}
        pack[name] = cfg
        try {
            cfg.path = probe(name, {fullpath: true})
        } catch (e) {
            throw e
        }
        Bit.load({packs: pack})
        return cfg.path
    }

    /** @hide */
    public function safeRemove(dir: Path) {
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

    public function verifyPlatforms() {
        for (i in platforms) {
            platforms[i] = verifyPlatform(platforms[i])
        }
    }

    /*
        Make a bit object. This will load the required bit files.
     */
    public function makeBit(platform: String, bitfile: Path) {
        let [os, arch, profile] = platform.split('-') 
        let [arch,cpu] = (arch || '').split(":")
        let kind = like(os)
        global.bit = bit = makeBareBit()
        bit.dir.src = options.configure || Path('.')
        bit.dir.bits = bit.dir.src.join('bits/standard.bit').exists ? 
            bit.dir.src.join('bits') : Config.Bin.join('bits').portable
        bit.dir.top = '.'
        bit.dir.home = Path(App.getenv('HOME')).portable
        let cross = ((os + '-' + arch) != (Config.OS + '-' + Config.CPU))

        bit.platform = { 
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
            bit.platform.cpu = cpu
        }
        loadBitFile(bit.dir.bits.join('standard.bit'))
        loadBitFile(bit.dir.bits.join('os/' + bit.platform.os + '.bit'))
        bit.globals.PLATFORM = currentPlatform = platform
        if (bitfile) {
            loadBitFile(bitfile)
            /*
                Customize bit files must be applied after the enclosing bit file is loaded so they can override anything.
             */
            for each (path in bit.customize) {
                let path = home.join(expand(path, {fill: '.'}))
                if (path.exists) {
                    loadBitFile(path)
                }
            }
        }
        if (kind == 'windows') {
            /*
                If 32 bit, /Program Files
                If 64 bit, /Program Files, for 64 bit programs, /Program Files (x86) for 32 bit programs
             */
            bit.dir.programFiles32 = programFiles32()
            bit.dir.programFiles = Path(bit.dir.programFiles32.name.replace(' (x86)', ''))
        }

        if (options.prefixes) {
            let pset = options.prefixes + '-prefixes'
            if (!bit[pset]) {
                throw "Cannot find prefix set for " + pset
            }
            bit.prefixes = {}
            bit.settings.prefixes = pset
            blend(bit.prefixes, bit[pset])
        } else {
            if (!bit.prefixes) {
                bit.prefixes = {}
                bit.settings.prefixes ||= 'debian-prefixes'
                blend(bit.prefixes, bit[bit.settings.prefixes])
            }
        }
        if (options.configure) {
            if (options.prefix) {
                bit.prefixes ||= {}
                for each (p in options.prefix) {
                    let [prefix, path] = p.split('=')
                    if (path) {
                        bit.prefixes[prefix] = Path(path)
                    } else {
                        /* Map --prefix=/opt to --prefix base=/opt */
                        bit.prefixes.root = Path(prefix)
                    }
                }
            }
        }
        for (let [key,value] in bit.ext.clone()) {
            if (value) {
                bit.ext['dot' + key] = '.' + value
            } else {
                bit.ext['dot' + key] = value
            }
        }
        if (!bit.settings.configured && !options.configure) {
            loadBitFile(bit.dir.bits.join('simple.bit'))
        }
        expandTokens(bit)
        if (!bit.dir.packs.exists) {
            let pdir = bit.dir.home.join('packages-' + bit.platform.os + '-' + bit.platform.arch)
            if (pdir.exists) {
                bit.dir.packs = pdir
            }
        }
        loadModules()
        applyProfile()
        bit.standardSettings = bit.settings.clone(true)
        applyCommandLineOptions(platform)
        applyEnv(bit)
        castDirTypes()
        if (samePlatform(platform, localPlatform)) {
            bit.globals.LBIN = localBin = bit.dir.bin.portable
        }
        runScript(bit.scripts, "loaded")
    }

    function samePlatform(p1, p2): Boolean {
        if (!p1 || !p2) return false
        let [os1, arch1] = p1.split('-')
        let [os2, arch2] = p2.split('-')
        return os1 == os2 && arch1 == arch2
    }

    public function quickLoad(bitfile: Path) {
        global.bit = bit = makeBareBit()
        bit.quickLoad = true
        loadBitFile(bitfile)
    }

    function validatePlatform(os, arch) {
        if (!supportedOS.contains(os)) {
            trace('WARN', 'Unsupported or unknown operating system: ' + os + '. Select from: ' + supportedOS.join(' '))
        }
        if (!supportedArch.contains(arch)) {
            trace('WARN', 'Unsupported or unknown architecture: ' + arch + '. Select from: ' + supportedArch.join(' '))
        }
    }

    function makeBareBit() {
        let old = bit
        bit = bareBit.clone(true)
        bit.platforms = old.platforms
        return bit
    }

    /**
        Expand tokens in a string.
        Tokens are represented by '${field}' where field may contain '.'. For example ${user.name}.    
        To preserve an ${token} unmodified, preceed the token with an extra '$'. For example: $${token}.
        Calls $String.expand to expand variables from the bit and bit.globals objects.
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
        s = s.expand(bit, eo)
        s = s.expand(bit.globals, eo)
        s = s.expand(bit, eo)
        return s.expand(bit.globals, options)
    }

    public function expandRule(target, rule) {
        setRuleVars(target)
        let result = expand(rule).expand(target.vars, {fill: ''})
        // target.vars = {}
        return result
    }

    let VER_FACTOR = 1000                                                                            

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
            tokens of the form ${token} in the src and dest filenames. If set to true, the 'bit' object is used.
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
            list = Path('.').files(pattern, options)
            if (bit.options.verbose) {
                dump('Copy-Files', list)
            }
            if (!list || list.length == 0) {
                if (bit.generating) {
                    list = [pattern]
                } else if (!options.cat && src.length > 0) {
                    throw 'cp: Cannot find files to copy for "' + pattern + '" to ' + dest
                }
            }
            let destIsDir = (dest.isDir || (!options.cat && list.length > 1) || dest.name.endsWith('/'))

            for each (let from: Path in list) {
                let from = from.portable
                if (subtree) {
                    to = dest.join(from.trimStart(Path(subtree).name + '/'))
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
                if (!bit.generating) {
                    attributes.permissions ||= from.attributes.permissions
                }
                if (bit.generating) {
                    if (options.cat || options.expand || options.fold || options.compress) {
                        dump("OPTIONS", options)
                        throw "Cannot use processing options when generating"
                    }
                    /* Must not use full options as it contains perms for the dest */
                    makeDir(to.dirname, {made: options.made})
                    if (from.isDir) {
                        makeDir(to, options)
                    } else {
                        copyFile(from, to, options)
                    }
                    if (options.linkin && bit.platform.like == 'unix') {
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
                                    to.rename(active)
                                }
                                copyFile(from, to, attributes)
                            }
                        }
                    }
                    if (options.expand) {
                        strace('Expand', to)
                        let o = bit
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
                    if (options.strip && bit.packs.strip && bit.platform.profile == 'release') {
                        strace('Strip', to)
                        Cmd.run(bit.packs.strip.path + ' ' + to)
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
                    if (options.linkin && bit.platform.like == 'unix') {
                        let linkin = Path(expand(options.linkin))
                        linkin.makeDir(options)
                        let lto = linkin.join(from.basename)
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

    function genrep(s) {
        genout.writeLine(repvar(s))
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
        if (!bit.generating) {
            if (!options.dry) {
                strace('Remove', 'rm -f', dest)
                dest.remove()
                strace('Link', 'ln -s', src, dest)
                src.link(dest)
            }
        } else if (bit.generating != 'nmake') {
            genrep('\trm -f "' + dest + '"')
            genrep('\tln -s "' + src + '" "' + dest + '"')
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
        if (!bit.generating) {
            if (!options.dry) {
                if (!path.isDir) {
                    strace('Create', 'mkdir -p' + path)
                    if (!path.makeDir()) {
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
            if (bit.generating == 'nmake') {
                /* BUG FIX */
                if (path.name.endsWith('/')) {
                    genrep('\tif not exist "' + path.windows + '/" md "' + path.windows + '/"')
                } else {
                    genrep('\tif not exist "' + path.windows + '" md "' + path.windows + '"')
                }
            } else {
                genrep('\tmkdir -p "' + path + '"')
                if (options.permissions) {
                    genrep('\tchmod ' + "%0o".format([options.permissions]) + ' "' + path + '"')
                }
                if (options.user || options.group) {
                    genrep('\t[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + path + '"')
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
        if (!bit.generating) {
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
            if (bit.generating == 'nmake') {
                genrep('\tif exist "' + path.windows + '" rd /Q "' + path.windows + '"')
            } else {
                genrep('\trm -f "' + path + '"')
            }
        }
    }

    /**
        Remove a directory.
        This removes a directory and all its contents include subdirectories. Use the 'empty' option to only remove
        empty directories.
        This either removes directories or if generating, emits code to remove directories.
        @param path Directory to remove
        @param options Control options
        @option empty Remove the directory only if empty. 
    */
    public function removeDir(path: Path, options = {}) {
        if (!bit.generating) {
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
            if (bit.generating == 'nmake') {
                if (options.empty) {
                    genrep('\tif exist "' + path.windows + '" rd /Q "' + path.windows + '"')
                } else {
                    genrep('\tif exist "' + path.windows + '" rd /Q /S"' + path.windows + '"')
                }
            } else {
                if (options.empty) {
                    genrep('\trmdir -p "' + path + '" 2>/dev/null ; true')
                } else {
                    genrep('\trm -fr "' + path + '"')
                }
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
        @param options Options to pass to Bit.copy(). These include user, group, uid, gid and  permissions.
    */
    public function copyFile(src: Path, dest: Path, options = {}) {
        if (!bit.generating) {
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
                src = src.relative
            }
            if (dest.startsWith(pwd)) {
                dest = dest.relative
            }
            if (src == dest) {
                throw new Error('Cannot copy file. Source is the same as destination: ' + src)
            }
            if (bit.generating == 'nmake') {
                genrep('\tcopy /Y "' + src.windows + '" "' + dest.windows + '"')
            } else {
                genrep('\tcp "' + src + '" "' + dest + '"')
                if (options.uid || options.gid) {
                    genrep('\t[ `id -u` = 0 ] && chown ' + options.uid + ':' + options.gid + ' "' + dest + '"')
                } else if (options.user || options.group) {
                    genrep('\t[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + dest + '"')
                }
                if (options.permissions) {
                    genrep('\tchmod ' + "%0o".format([options.permissions]) + ' "' + dest + '"')
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


} /* bit class */

} /* bit module */

/*
    Global functions for bit files
 */
require embedthis.bit

/**
    Bit DOM object
 */
public var b: Bit = new Bit

b.main()

/* UNUSED
    Define a pack. This registers a pack with a unique name and description
    This calls Bit.load to initialize a pack collection in the Bit DOM.
    @param name Unique pack name
    @param description Short, single-line pack description
public function pack(name: String, description: String) {
    let pack = {}
    pack[name] = {description: description}
    Bit.load({packs: pack})
}
 */

/*
   UNUSED
    Probe for a file and locate
    Will throw an exception if the file is not found, unless {continue, default} specified in control options
    @param file File to search for
    @param options Control options
    @option default Default path to use if the file cannot be found and bit is invoked with --continue
    @option search Array of paths to search for the file
    @option nopath Don't use the system PATH to locate the file
    @option fullpath Return the full path to the located file
public function probe(file: Path, options = {}): Path {
    return b.probe(file, options)
}
 */

/** MOVED
    Define a pack for a command line program.
    This registers the pack and loads the Bit DOM with the pack configuration.
    @param name Program name. Can be either a path or a basename with optional extension
    @param description Short, single-line program description.
    @hide
    @deprecate
public function program(name: Path, description = null): Path {
    let path = bit.packs[name.trimExt()].path || name
    let pack = {}
    let cfg = {description: description}
    pack[name] = cfg
    try {
        cfg.path = probe(name, {fullpath: true})
    } catch (e) {
        throw e
    }
    Bit.load({packs: pack})
    return cfg.path
}
*/

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
    This is trace that is displayed if bit --show is invoked.
    @param tag Informational tag emitted before the message
    @param args Message args to display
*/
public function strace(tag, ...args)
    b.strace(tag, ...args)
    
/** @duplicate Bit.vtrace */
public function vtrace(tag, ...args)
    b.vtrace(tag, ...args)

/** @duplicate Bit.copy */
public function copy(src, dest: Path, options = {})
    b.copy(src, dest, options)

/** @duplicate Bit.run */
public function run(command, cmdOptions = {}): String
    b.run(command, cmdOptions)

/** @hide */
public function safeRemove(dir: Path)
    b.safeRemove(dir)

/** @hide */
public function mapLibs(libs: Array, static = null)
    b.mapLibs(libs, static)

/** @hide */
public function setRuleVars(target, dir = App.dir)
    b.setRuleVars(target, dir)

/** @hide */
public function makeDirGlobals(base: Path? = null)
    b.makeDirGlobals(base)

/** @duplicate Bit.makeDir */
public function makeDir(path: Path, options = {})
    b.makeDir(path, options)

/** @duplicate Bit.copyFile */
public function copyFile(src: Path, dest: Path, options = {})
    b.copyFile(src, dest, options)

/** @duplicate Bit.linkFile */
public function linkFile(src: Path, dest: Path, options = {})
    b.linkFile(src, dest, options)

/** @duplicate Bit.removeDir */
public function removeDir(path: Path, options = {})
    b.removeDir(path, options)

/** @duplicate Bit.removeFile */
public function removeFile(path: Path, options = {})
    b.removeFile(path, options)

/** @hide */
public function runTargetScript(target, when)
    b.runTargetScript(target, when)

/** @duplicate Bit.whyRebuild */
public function whyRebuild(path, tag, msg)
    b.whyRebuild(path, tag, msg)

/** @duplicate Bit.expand */
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
