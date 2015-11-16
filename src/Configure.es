/*
    Configure.es -- MakeMe Configure support

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

require ejs.unix
require ejs.version

class Configure {

    public static var currentComponent: String?

    /* List of platforms to configure for */
    var platforms: Array = []

    /*
        Recursion protection gates
     */
    private var gates: Object = {}

    /** @hide */
    var envTools = {
        AR: 'lib',
        CC: 'compiler',
        LD: 'linker',
    }

    /** @hide */
    var envFlags = {
        CFLAGS:  'compiler',
        DFLAGS:  'defines',
        IFLAGS:  'includes',
        LDFLAGS: 'linker',
    }
    /** @hide */
    var envSettings: Object
    var loader: Loader
    var builder: Builder

    function Configure() {
        loader = makeme.loader
        builder = makeme.builder
    }

    /*
        Recursion protection when traversing tagets
     */
    function admit(target, prefix: String) {
        let gate = gates[prefix]
        if (gate[target.name]) {
            return false
        }
        return gate[target.name] = true
    }

    function admitSetup(prefix: String) {
        gates[prefix] = {}
    }

    public function autoConfigure() {
        loader.blendFile(loader.findPlugin('components'), true)
        findComponents()
    }

    /*
        Only used when cross compiling. 
        Note: setting CFLAGS, DFLAGS etc overwrites internal me settings for compiler, defines etc.
     */
    function defineEnv() {
        if (platforms.length > 1 && !me.platform.cross) {
            /* If building cross, then only apply env to cross build, not to native dev platform build */
            return
        }
        envSettings = { targets: { compiler: {} } }
        for (let [key, tool] in envTools) {
            let path = App.getenv(key)
            if (path) {
                envSettings.targets[tool] ||= {}
                envSettings.targets[tool].path = path
                envSettings.targets[tool].enable = true
            }
        }
        for (let [flag, option] in envFlags) {
            let value = App.getenv(flag)
            if (value) {
                envSettings.targets.compiler[option] ||= []
                envSettings.targets.compiler[option] += [value]
            }
        }
        blend(me, envSettings, {combine: true})
    }

    function checkComponent(target) {
        if (!admit(target, 'check')) {
            return
        }
        /* Recursive descent checking */
        for each (name in target.ifdef) {
            let p = me.targets[name]
            if (p && p.configurable) {
                checkComponent(p)
                if (!p.enable) {
                    target.enable = false
                    target.diagnostic ||= 'required component ' + p.name + ' is not enabled'
                }
            }
        }
        if (!target.enable && target.essential) {
            if (!makeme.options['continue']) {
                throw 'Required component "' + target.name + '" is not enabled: ' + target.diagnostic
            }
        }
        if (target.enable) {
            for each (o in target.conflicts) {
                let other = me.targets[o]
                if (other && other.configurable && other.enable) {
                    other.enable = false
                    other.diagnostic ||= 'conflicts with ' + target.name
                }
            }
            for (let [i, path] in target.libpaths) {
                target.libpaths[i] = Path(path).natural
            }
            for (let [i, path] in target.includes) {
                target.includes[i] = Path(path).natural
            }
        }
    }

    function checkComponents(toplevel = false) {
        if (toplevel) {
            admitSetup('check')
        }
        for each (target in me.targets) {
            if (!target.configurable) continue
            checkComponent(target)
        }
        for each (component in me.configure.requires) {
            let target = me.targets[component]
            if (!target) {
                throw 'Required component "' + component + '" cannot be found'
            } else if (!target.enable) {
                throw 'Required component "' + component + '" is not enabled: ' + target.diagnostic
            }
        }
    }

    function checkMain() {
        let settings = me.settings
        for each (field in ['name', 'description', 'version']) {
            if (!settings[field]) {
                throw Loader.MAIN + ' is missing settings.' + field
            }
        }
        let package = me.dir.top.join(Loader.PACKAGE)
        let criteria
        if (package.exists) {
            let spec = package.readJSON()
            criteria = spec.devDependencies ? spec.devDependencies.makeme : null
        }
        //  DEPRECATE
        criteria ||= me.makeme || me.me || settings.makeme || settings.me
        if (criteria && !Version(Config.Version).acceptable(criteria)) {
            throw '' + settings.title + ' requires MakeMe ' + criteria + '. MakeMe version ' + Config.Version +
                ' is not compatible with this requirement.' + '\n'
        }
    }

    /**  
        Configure and initialize for building. This generates platform specific me files.
        @hide
     */
    public function configure() {
        let home = App.dir
        App.chdir(me.dir.top)
        let options = makeme.options
        let main = options.configure.join(Loader.MAIN)

        /*
            Compute the platforms from the command line switches and the main.me settings.platforms property.
            If none specified, use the local platform. Ensure local is first to build dev tools.
        */
        platforms = Object.getOwnPropertyNames(options.platforms)
        let obj = loader.readFile(main)
        let settings = obj.settings
        /*
            Configure other platforms only if not generating
            settings.platforms has mandatory platforms like 'local'
         */
        if (settings && settings.platforms && !options.gen) {
            if (!(settings.platforms is Array)) {
                settings.platforms = [settings.platforms]
            }
            settings.platforms = settings.platforms.transform(function(e) e == 'local' ? loader.localPlatform : e)
            if (options.nolocal) {
                settings.platforms.removeElements('local', loader.localPlatform)
            }
            if (platforms.length == 1 && loader.canExecute(platforms[0])) {
                /*
                    Special case. Only one platform specified by the user and it is executable on this system.
                 */
                loader.localPlatform = settings.platforms[0]
            } else {
                platforms = (settings.platforms + platforms).unique()
            }
        }
        if (options.nocross) {
            /* Does not really make sense to do this -- but here for completeness */
            platforms = [loader.localPlatform]
        }
        if (platforms.length == 0) {
            platforms.push(loader.localPlatform)
        }
        for each (platform in platforms) {
            trace('Configure', platform)
            Me()
            me.dir.work = home
            loader.reset()
            loader.initPlatform(platform)
            loader.loadFile(main)
            vtrace('Load', 'Standard configurable components')
            /*
                Load me-components/
             */
            loader.blendFile(loader.findPlugin('components'), true)
            checkMain()
            findComponents()
            defineEnv()
            importComponentFiles()
            createPlatformFile()
            createMeHeader()
            postConfig()
            if (options.nocross) {
                break
            }
        }
        App.chdir(home)
        if (!options.gen) {
            createStartFile(platforms)
            trace('Info', 'Type "me" to build.')
        }
    }

    function configureComponent(target) {
        if (!admit(target, 'configure')) {
            return
        }
        let components = []
        if (target.requires) {
            components += target.requires
        }
        if (target.discovers) {
            components += target.discovers
        }
        createBareTargets(components)
        for each (dname in components) {
            let dext = me.targets[dname]
            configureComponent(dext)
        }
        for each (name in target.ifdef) {
            let et = me.targets[name]
            if (!et) {
                Target.create({ name: name, enable: false, type: 'component', diagnostic: 'Component not defined' })
            }
        }
        /* Just for probe() which needs the context */
        Configure.currentComponent = target.name
        try {
            if (target.scripts && target.scripts.config) {
                let result = makeme.builder.runTargetScript(target, 'config', {rethrow: true})
                if (result is String || result is Path) {
                    target.path = result
                } else if (Object.getOwnPropertyCount(result) > 0) {
                    blend(target, result, {combine: true})
                    if (me.platform.os == 'windows') {
                        for (let [index, lib] in target.libraries) {
                            if (!lib.endsWith('.lib')) {
                                target.libraries[index] = lib + '.lib'
                            }
                        }
                    }
                }
            }
            if (target.path is Function) {
                target.path = target.path(target)
            }
            if (target.path) {
                if (!(target.path is Path)) {
                    target.path = Path(target.path.toString())
                }
                target.path = target.path.compact()
            }
            if (target.env) {
                let env = target.env.clone()
                for each (field in ['PATH', 'INCLUDE', 'LIB']) {
                    if (env[field]) {
                        env['+' + field] = env[field]
                        delete env[field]
                    }
                }
                blend(me.env, env, {combine: true})
            }
            if (target.scripts && target.scripts.generate) {
                print("WARNING: generate scripts are deprecated: ", target.name)
            }
            if (target.path) {
                target.path = Path(target.path)
            }

        } catch (e) {
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            target.path = null
            target.enable = false
            target.diagnostic = '' + e
            vtrace('Omit', 'Component "' + target.name + '": ' + target.diagnostic)
        }
        for each (name in target.ifdef) {
            let et = me.targets[name]
            if (et && et.configurable) {
                configureComponent(et)
            }
        }
    }

    function configureComponents() {
        admitSetup('configure')
        for each (target in me.targets) {
            if (target.configurable) {
                configureComponent(target)
            }
        }
    }

    function createPlatformFile() {
        let nme = {}
        blend(nme, {
            main: '${TOP}/main.me',
            platform: me.platform,
            dir: me.dir,
            prefixes: me.prefixes,
            settings: { configured: true },
            targets: getConfigurableTargets(),
            env: me.env,
        })
        for (let [key, value] in me.settings) {
            /* Copy over non-standard settings. These include compiler sleuthing settings */
            nme.settings[key] = value
        }
        nme.settings.configure = 'me ' + App.args.slice(1).join(' ')
        if (envSettings) {
            blend(nme, envSettings, {combine: true})
        }
        if (me.dir.me != Config.Bin) {
            nme.dir.me = me.dir.me
        }
        if (nme.settings) {
            Object.sortProperties(nme.settings)
        }
        loader.runScript('postconfig')
        if (makeme.options.configure) {
            let path: Path
            if (me.dir.bld != me.dir.out) {
                path = me.dir.bld.join(me.platform.name, Loader.PLATFORM)
            } else {
                path = me.dir.bld.join(me.platform.name).joinExt('me')
            }
            path = path.relative
            trace('Create', path)
            let data = '/*\n    ' + path + ' -- MakeMe ' + me.settings.title + ' for ' + me.platform.name + 
                '\n */\n\nMe.load(' + 
                serialize(nme, {nulls: false, pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
            path.dirname.makeDir()
            path.write(data)
        }
        if (makeme.options.show && makeme.options.verbose) {
            trace('Configuration', me.settings.title + 
                '\nsettings = ' +
                serialize(me.settings, {pretty: true, indent: 4, commas: true, quotes: false}) +
                '\ncomponents = ' +
                serialize(nme.targets, {pretty: true, indent: 4, commas: true, quotes: false}))
        }
    }

    function createMeHeader() {
        loader.runScript('preheader')
        me.dir.inc.makeDir()
        let path = me.dir.inc.join('me.h').relative
        trace('Create', path)
        let f = TextStream(File(path, 'w'))
        f.writeLine('/*\n    me.h -- MakeMe Configure Header for ' + me.platform.name + '\n\n' +
                '    This header is created by Me during configuration. To change settings, re-run\n' +
                '    configure or define variables in your Makefile to override these default values.\n */')
        writeDefinitions(f)
        f.close()
    }

    function createStartFile(platforms) {
        trace('Create', Loader.START)
        let profile = makeme.options.release ? 'release' : 'debug'
        platforms = platforms.transform(function(p) p == (Config.OS + '-' + Config.CPU + '-' + profile) ? 
            'local-' + profile : p)
        let pstr = serialize(platforms, {pretty: true, indent: 4, commas: true, nulls: false, quotes: false})
        pstr = pstr.replace(/^/mg, '    ').trimStart(' ')
        let cmdline = 'me ' + App.args.slice(1).join(' ')
        let meVer = me.makeme ? ('    makeme: "' + me.makeme + '",\n') : ''
        let data = '/*\n    start.me -- MakeMe Startup File\n */\n\nMe.load({\n' +
                   meVer +
                   '    platforms: ' + pstr + ',\n' +
                   '    configure: "' + cmdline + '",\n})\n'
        Loader.START.write(data)
    }

    function createBareTargets(components) {
        for each (name in components) {
            let target = me.targets[name]
            if (target) {
                if (!target.bare) {
                    target.loaded = true
                }
                target.type ||= 'component'
                target.configurable = true
            } else {
                loader.createTarget({
                    name: name,
                    enable: false,
                    home: '.',
                    type: 'component',
                    bare: true,
                    configurable: true
                })
            }
        }
    }

    function def(f: TextStream, key, value) {
        f.writeLine('#ifndef ' + key)
        f.writeLine('    #define ' + key + ' ' + value)
        f.writeLine('#endif')
    }
   
    /*
        Check for --without, and run enable scripts/functions
        Enable scripts do not run in dependency order
     */
    function enableComponent(target) {
        if (!admit(target, 'enable')) {
            return
        }
        if (me.configure.extras.contains(target.name) && !target.explicit) {
            target.enable = false
            target.diagnostic = 'Component must be explicitly included via --with'

        } else if (target.without) {
            vtrace('Run', 'Component call without for: ' + target.name)
            runTargetScript(target, 'without')

        } else if (target.enable is Function) {
            vtrace('Run', 'Component call enable for: ' + target.name)
            target.enable = target.enable.call(me, target)

        } else if (target.enable && !(target.enable is Boolean)) {
            let script = loader.expand(target.enable)
            vtrace('Run', 'Component eval enable expression for: ' + target.name)
            if (!eval(script)) {
                target.enable = false
            } else {
                target.enable = true
            }
        }
    }

    function enableComponents() {
        admitSetup('enable')
        for each (target in me.targets) {
            if (target.explicit) {
                for each (dname in target.depends) {
                    let dep = me.targets[dname]
                    if (dep) {
                        dep.explicit = true
                    }
                }
            }
        }
        for each (target in me.targets) {
            if (target.configurable) {
                enableComponent(target)
            }
        }
    } 

    public function findComponents() {
        let configure = me.configure
        configure.requires ||= []
        configure.discovers ||= []
        configure.extras ||= []
        let components = configure.requires + configure.discovers
        if (me.options.gen) {
            components += configure.extras
        }
        /*
            Add pre-loaded configurable targets. If these are in "extras" only load if generating
         */
        for each (target in me.targets) {
            if (target.configurable && !components.contains(target.name)) {
                if (me.options.gen || !configure.extras.contains(target.name)) {
                    components.push(target.name)
                }
            }
        }
        components = components.unique()
        vtrace('Search', 'Components: ' + components.join(' '))
        createBareTargets(components)
        loadComponents(components, true)
        configureComponents()
        enableComponents()
        checkComponents(true)
        traceComponents()
    }

    /**
        @hide
      */
    function getConfigurableTargets() {
        let targets = {}
        for each (target in me.targets) {
            if (target.configurable) {
                targets[target.name] = target
            }
        }
        Object.sortProperties(targets)
        return targets
    }

    function importComponentFiles() {
        for each (target in me.targets) {
            if (target.configurable && target.enable) {
                for each (let file: Path in target.imports) {
                    if (!file.exists) {
                        throw 'Cannot import: ' + file
                    }
                    vtrace('Import', file)
                    if (file.extension == 'h') {
                        me.dir.inc.makeDir()
                        cp(file, me.dir.inc)
                    } else {
                        if (me.platform.like == 'windows') {
                            let tname = me.dir.bin.join(file.basename).relative
                            let old = tname.replaceExt('old')
                            vtrace('Preserve', 'Active library ' + tname + ' as ' + old)
                            old.remove()
                            try { tname.rename(old) } catch {}
                        }
                        me.dir.bin.makeDir()
                        cp(file, me.dir.bin)
                    }
                }
            }
        }
    }

    function loadComponent(target) {
        if (!target.configurable || !admit(target, 'load')) {
            return
        }
        try {
            let path: Path?, pak: Path?
            if (target.loaded) {
                target.diagnostic = 'Pre-loaded component'

            } else if (target.withpath) {
                pak = Path(target.withpath).join(target.name + '.me')
                if (pak.exists) {
                    loader.blendFile(pak)
                    target.path = target.withpath
                    target.diagnostic = 'Load component from pak: ' + pak
                    target.loaded = true
                }
            }
            if (!target.loaded) {
                path = loader.findPlugin(target.name, false)
                if (path) {
                    vtrace('Found', 'Component at:' + path)
                    target.diagnostic = 'Found component: ' + path
                    target.home = path.dirname
                    target.path = path
                    target.diagnostic = 'Load component from: ' + path
                    Configure.currentComponent = target.name
                    loader.blendFile(path)
                } else {
                    throw 'Cannot find definition for component: ' + target.name + '.me'
                }
                target.loaded = true
            }
            delete target.bare
      
            if (!target.description) {
                let path = me.dir.paks.join(target.name)
                if (path.join(Loader.PACKAGE).exists) {
                    let spec = path.join(Loader.PACKAGE).readJSON()
                    target.description = spec.description
                } else if (target.name == me.name) {
                    let spec = Loader.PACKAGE.readJSON()
                    target.description = spec.description
                }
                target.description ||= target.name.toPascal()
            }
            if (target.ifdef) {
                loadComponents(target.ifdef)
            }
            if (target.requires) {
                createBareTargets(target.requires)
                loadComponents(target.requires)
            }
            if (target.discovers) {
                createBareTargets(target.discovers)
                loadComponents(target.discovers)
            }
            loadComponents(target.depends)
            if (target.enable === undefined) {
                target.enable = true
            }

        } catch (e) {
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            target.enable = false
            target.diagnostic = '' + e
            vtrace('Configure', target.name + ': ' + target.diagnostic)
        }
    }
    
    function loadComponents(components, toplevel = false) {
        if (toplevel) {
            admitSetup('load')
        }
        for each (name in components) {
            if (me.targets[name]) {
                loadComponent(me.targets[name])
            }
        }
    }

    function postConfig() {
        for (let [tname, target] in me.targets) {
            if (target.configurable) {
                makeme.builder.runTargetScript(target, 'postconfig')
            }
        } 
    }

    function traceComponents() {
        let disabled = {}
        if (!makeme.options.configure && !makeme.options.verbose) return
        let components = getConfigurableTargets()
        for (let [name, target] in components) {
            if (!target.enable) continue
            let description = target.description ? (': ' + target.description) : ''
            let diagnostic = target.diagnostic ? (': ' + target.diagnostic) : ''
            if (target.enable && !target.silent) {
                if (target.location || target.path) {
                    let location = Path(target.location || target.path).compact()
                    if (makeme.options.verbose) {
                        trace('Found', name + description + ' at: ' + location)
                    } else if (!target.quiet) {
                        trace('Found', name + description + ': ' + location)
                    }
                } else {
                    trace('Found', name + description)
                }
            } else {
                disabled[name] = target
                vtrace('Omit', name + description + diagnostic)
            }
        }
        if (makeme.options.why) {
            for (let [name, target] in disabled) {
                trace('Omit', name + diagnostic)
            }
        }
    }

    function writeDefinitions(f: TextStream) {
        let settings = me.settings.clone()
        if (makeme.options.endian) {
            settings.endian = makeme.options.endian == 'little' ? 1 : 2
        }
        f.writeLine('\n/* Settings */')
        writeSettings(f, 'ME', settings)

        f.writeLine('\n/* Prefixes */')
        for (let [name, prefix] in me.prefixes) {
            let path = loader.expand(loader.expand(prefix.portable))
            path = loader.expand(path)
            path = Path(path).normalize
            def(f, 'ME_' + name.toUpper() + '_PREFIX', '"' + path + '"')
        }

        /* Suffixes */
        f.writeLine('\n/* Suffixes */')
        def(f, 'ME_EXE', '"' + me.ext.dotexe + '"')
        def(f, 'ME_SHLIB', '"' + me.ext.dotshlib + '"')
        def(f, 'ME_SHOBJ', '"' + me.ext.dotshobj + '"')
        def(f, 'ME_LIB', '"' + me.ext.dotlib + '"')
        def(f, 'ME_OBJ', '"' + me.ext.doto + '"')

        /* Build profile */
        f.writeLine('\n/* Profile */')
        let args = 'me ' + App.args.slice(1).join(' ')
        def(f, 'ME_CONFIG_CMD', '"' + args + '"')
        def(f, 'ME_' + settings.name.toUpper() + '_PRODUCT', '1')
        def(f, 'ME_PROFILE', '"' + me.platform.profile + '"')
        def(f, 'ME_TUNE_' + (me.settings.tune || "size").toUpper(), '1')

        /* Architecture settings */
        f.writeLine('\n/* Miscellaneous */')
        if (settings.charlen) {
            def(f, 'ME_CHAR_LEN', settings.charlen)
            if (settings.charlen == 1) {
                def(f, 'ME_CHAR', 'char')
            } else if (settings.charlen == 2) {
                def(f, 'ME_CHAR', 'short')
            } else if (settings.charlen == 4) {
                def(f, 'ME_CHAR', 'int')
            }
        }
        let ver = settings.version.split('.')
        def(f, 'ME_MAJOR_VERSION',  ver[0])
        def(f, 'ME_MINOR_VERSION', ver[1])
        def(f, 'ME_PATCH_VERSION', ver[2])
        def(f, 'ME_VNUM',  ((((ver[0] * 1000) + ver[1]) * 1000) + ver[2]))

        f.writeLine('\n/* Components */')
        let targets = me.targets.clone()
        Object.sortProperties(targets)
        for each (target in targets) {
            if (!target.configurable) continue
            let name = target.name == 'compiler' ? 'cc' : target.name
            def(f, 'ME_COM_' + name.toUpper(), target.enable ? '1' : '0')
        }
        for each (target in targets) {
            if (!target.configurable) continue
            if (target.enable) {
                /* Must test makeme.options.gen and not makeme.generating */
                if (!makeme.options.gen && target.path) {
                    def(f, 'ME_COM_' + target.name.toUpper() + '_PATH', '"' + target.path.relative + '"')
                }
                if (target.definitions) {
                    for each (define in target.definitions) {
                        if (define.match(/-D(.*)=(.*)/)) {
                            let [key,value] = define.match(/-D(.*)=(.*)/).slice(1)
                            def(f, key, value)
                        } else if (define.match(/(.*)=(.*)/)) {
                            let [key,value] = define.match(/(.*)=(.*)/).slice(1)
                            def(f, key, value)
                        } else {
                            f.writeLine('#define ' + define.trimStart('-D'))
                        }
                    }
                }
            }
        }
    }

    function writeSettings(f: TextStream, prefix: String, obj) {
        Object.sortProperties(obj)
        for (let [key,value] in obj) {
            key = prefix + '_' + key.replace(/[A-Z]/g, '_$&').replace(/-/g, '_').toUpper()
            if (value is Number) {
                def(f, key, value)
            } else if (value is Boolean) {
                def(f, key, value cast Number)
            } else if (Object.getOwnPropertyCount(value) > 0 && !(value is Array)) {
                writeSettings(f, key, value)
            } else if (typeOf(value) != 'Object') {
                def(f, key, '"' + value + '"')
            }
        }
    }

} /* Configure class */
} /* embedthis.me */

module embedthis.me.script {
    require embedthis.me
    require ejs.version

    public function getComponentSearch(target, component, objdir = '.') {
        if (target.withpath) {
            return [Path(target.withpath).join(objdir)]
        } 
        let search = []
        if (me.platform.cross) {
            return search
        }
        if (me.dir) {
            if (me.dir.paks) {
                /*
                    src/paks/NAME
                 */
                let dir = me.dir.paks.join(component, objdir)
                if (dir.exists) {
                    search.push(me.dir.paks.join(component, objdir))
                }
            }
            /*
                ~/.paks/NAME/NEWEST-VERSION
             */
            let home = App.home.portable.absolute
            path = home.join('.paks', component)
            if (path.exists) {
                let versions = Version.sort(path.files('*/*'), -1)
                if (versions && versions.length >= 1) {
                    path = Path(versions[0])
                    if (path) {
                        search.push(path.join(objdir))
                    }
                }
            }
            /*
                /usr/local/lib/me/LATEST/bin/paks/NAME
             */
            path = me.dir.me.join('paks', component)
            if (path.exists) {
                search.push(path.join(objdir))
            }
        }
        if (me.platform.like == 'unix') {
            if (me.platform.arch == 'x64') {
                search += [Path('/usr/lib64'), Path('/lib64')]
            }
            search += [Path('/usr/lib'), Path('/lib') ]
        }
        if (me.platform.os == 'linux') {
            search += Path('/usr/lib').files('*-linux-gnu') + Path('/lib').files('*-linux-gnu')
        }
        return search.transform(function(path) path.absolute)
    }

    /**
        Probe for a file and locate
        Will throw an exception if the file is not found, unless {continue, default} specified in control options
        @param file File to search for
        @param control Control options
        @option default Default path to use if the file cannot be found and me is invoked with --continue
        @option search Array of paths to search for the file
        @option nopath Don't use the system PATH to locate the file
        @option fullpath Return the full path to the located file
        @option nothrow Do not throw an exception
     */
    public function probe(file: Path, control = {}): Path? {
        let path: Path?
        let search = [], dir
        if (file.exists) {
            path = file
        } else {
            if ((dir = me.targets[Configure.currentComponent].path) && !(dir is Function) && Path(dir).exists) {
                search.push(dir)
            }
            if (control.search) {
                if (!(control.search is Array)) {
                    control.search = [control.search]
                }
                search += control.search
            }
            for each (let s: Path in search) {
                if (s.join(file).exists) {
                    path = s.join(file)
                    break
                }
            }
            if (!control.nopath) {
                path ||= Cmd.locate(file)
            }
        }
        if (!path) {
            if (makeme.options.why) {
                trace('Missing', 'Component "' + Configure.currentComponent + '" cannot find: "' + file + '"\n')
            }
            if (makeme.options['continue'] && control.default) {
                return control.default
            }
            if (control.nothrow) {
                return null
            }
            throw 'Cannot find "' + file + '" for component "' + Configure.currentComponent + 
                    '".\n' + 'Using search: ' + serialize(search, {pretty: true})
        }
        vtrace('Probe', 'Component "' + Configure.currentComponent + '" found: "' + path)
        if (control.fullpath) {
            return path.portable
        }
        /*
            Trim the pattern we have been searching for and return the base prefix only
            Need to allow for both / and \ separators
         */
        let pat = RegExp('.' + file.toString().replace(/[\/\\]/g, '.') + '$')
        return path.portable.name.replace(pat, '')
    }
}


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
