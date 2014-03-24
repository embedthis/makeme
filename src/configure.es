/*
    configure.es -- Me configuration

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

    require ejs.unix
    require ejs.zlib

    /** @hide */
    public var currentComponent: String?

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

    /**
        @hide
     */
    function checkMain() {
        let settings = me.settings
        for each (field in ['name', 'description', 'version', 'author']) {
            if (!settings[field]) {
                throw b.MAIN + ' is missing settings.' + field
            }
        }
    }

    /**  
        Configure and initialize for building. This generates platform specific me files.
        @hide
     */
    function configure() {
        vtrace('Load', 'Preload main.me to determine required platforms')
        b.quickLoad(b.options.configure.join(b.MAIN))
        checkMain()
        let settings = me.settings
        if (settings.makeme && b.makeVersion(Config.Version) < b.makeVersion(settings.makeme)) {
            throw 'This product requires a newer version of MakeMe. Please upgrade Me to ' + settings.makeme + '\n'
        }
        if (settings.platforms && !b.options.gen && !b.options.nocross) {
            if (!(settings.platforms is Array)) {
                settings.platforms = [settings.platforms]
            }
            settings.platforms = settings.platforms.transform(function(e) e == 'local' ? b.localPlatform : e)
            b.platforms = (settings.platforms + b.platforms).unique()
        }
        b.verifyPlatforms()
        for each (platform in b.platforms) {
            b.currentPlatform = platform
            trace('Configure', platform)
            b.createMe(platform, b.options.configure.join(b.MAIN))
            findComponents()
            captureEnv()
            b.castDirTypes()
            if (b.options.configure) {
                createPlatformMeFile()
                b.makeOutDirs()
                createMeHeader()
                importComponentFiles()
            }
        }
        if (!b.options.gen) {
            createStartMeFile(b.platforms[0])
            trace('Info', 'Type "me" to build. Use "me configuration" to see current settings"')
        }
    }

    /**
        @hide
     */
    function reconfigure() {
        vtrace('Load', 'Preload main.me to determine required configuration')
        b.quickLoad(Me.START)
        if (me.platforms) {
            platforms = me.platforms
            for (let [index,platform] in me.platforms) {
                let mefile = Path(platform).joinExt('me')
                b.createMe(platform, mefile)
                if (me.settings.configure) {
                    run(me.settings.configure)
                    return
                }
            }
        }
        App.log.error('No prior configuration to use')
    }

    internal function importComponentFiles() {
        for each (target in me.targets) {
            if (target.configurable && target.enable) {
                for each (file in target.imports) {
                    vtrace('Import', file)
                    if (file.extension == 'h') {
                        cp(file, me.dir.inc)
                    } else {
                        if (me.platform.like == 'windows') {
                            let target = me.dir.lib.join(file.basename).relative
                            let old = target.replaceExt('old')
                            vtrace('Preserve', 'Active library ' + target + ' as ' + old)
                            old.remove()
                            try { target.rename(old) } catch {}
                        }
                        cp(file, me.dir.lib)
                    }
                }
            }
        }
    }

    internal function createStartMeFile(platform) {
        let nme = { }
        nme.platforms = b.platforms
        trace('Generate', b.START)
        let data = '/*\n    start.me -- Startup Me File for ' + me.settings.title + 
            '\n */\n\nMe.load(' + serialize(nme, {pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
        b.START.write(data)
    }

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

    internal function createPlatformMeFile() {
        let nme = {}
        blend(nme, {
            blend: [ 
                '${SRC}/main.me',
            ],
            platform: me.platform,
            dir: { 
                src: me.dir.src.absolute.portable,
                top: me.dir.top.portable,
            },
            settings: { configured: true },
            prefixes: me.prefixes,
            targets: getConfigurableTargets(),
            env: me.env,
        })
        for (let [key, value] in me.settings) {
            /* Copy over non-standard settings. These include compiler sleuthing settings */
            nme.settings[key] = value
        }
        blend(nme.settings, me.customSettings)
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
        b.runScript(me.scripts, 'postconfig')
        if (b.options.configure) {
            let path: Path = Path(me.platform.name).joinExt('me')
            trace('Generate', path)
            let data = '/*\n    ' + path + ' -- MakeMe ' + me.settings.title + ' for ' + me.platform.name + 
                '\n */\n\nMe.load(' + 
                serialize(nme, {pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
            path.write(data)
        }
        if (b.options.show && b.options.verbose) {
            trace('Configuration', me.settings.title + 
                '\nsettings = ' +
                serialize(me.settings, {pretty: true, indent: 4, commas: true, quotes: false}) +
                '\ncomponents = ' +
                serialize(nme.targets, {pretty: true, indent: 4, commas: true, quotes: false}))
        }
    }

    internal function createMeHeader() {
        b.runScript(me.scripts, 'preheader')
        let path = me.dir.inc.join('me.h')
        let f = TextStream(File(path, 'w'))
        f.writeLine('/*\n    me.h -- MakeMe Configuration Header for ' + me.platform.name + '\n\n' +
                '    This header is created by Me during configuration. To change settings, re-run\n' +
                '    configure or define variables in your Makefile to override these default values.\n */')
        writeDefinitions(f)
        f.close()
        for (let [tname, target] in me.targets) {
            if (target.configurable) {
                runTargetScript(target, 'postconfig')
            }
        }
    }

    internal function def(f: TextStream, key, value) {
        f.writeLine('#ifndef ' + key)
        f.writeLine('    #define ' + key + ' ' + value)
        f.writeLine('#endif')
    }

    internal function writeSettings(f: TextStream, prefix: String, obj) {
        Object.sortProperties(obj)
        for (let [key,value] in obj) {
            key = prefix + '_' + key.replace(/[A-Z]/g, '_$&').replace(/-/g, '_').toUpper()
            if (value is Number) {
                def(f, key, value)
            } else if (value is Boolean) {
                def(f, key, value cast Number)
            } else if (Object.getOwnPropertyCount(value) > 0 && !(value is Array)) {
                writeSettings(f, key, value)
            } else {
                def(f, key, '"' + value + '"')
            }
        }
    }

    internal function writeDefinitions(f: TextStream) {
        let settings = me.settings.clone()
        if (b.options.endian) {
            settings.endian = b.options.endian == 'little' ? 1 : 2
        }
        f.writeLine('\n/* Settings */')
        writeSettings(f, 'ME', settings)

        f.writeLine('\n/* Prefixes */')
        for (let [name, prefix] in me.prefixes) {
            def(f, 'ME_' + name.toUpper() + '_PREFIX', '"' + prefix.portable + '"')
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
                /* Must test b.options.gen and not me.generating */
                if (!b.options.gen && target.path) {
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

    /** 
        @hide 
     */
    function findComponent(name) {
        let path = Path(me.dir.me.join('configure', name + '.me'))
        if (path.exists) {
            return path
        }
        path = Path(me.dir.me.join('configure', name, name + '.me'))
        if (path.exists) {
            return path
        }
        path = Path(me.dir.home.join('.me/configure', name + '.me'))
        if (path.exists) {
            return path
        }
        path = Path(me.dir.home.join('.me/configure', name, name + '.me'))
        if (path.exists) {
            return path
        }
        return null
    }

    /**
        @hide
     */
    function findComponents() {
        let configure = me.configure.requires + me.configure.discovers + me.configure.extras
        for each (target in me.targets) {
            if (target.configurable && !configure.contains(target.name)) {
                configure.push(target.name)
            }
        }
        configure = configure.unique()
        vtrace('Search', 'Components: ' + configure.join(' '))
        createTargets(configure)
        loadComponents(configure)
        enableComponents()
        configureComponents()
        checkComponents()
        traceComponents()
        resetComponents()
    }

    internal function loadComponents(components) {
        for each (name in components) {
            if (me.targets[name]) {
                loadComponent(me.targets[name])
            }
        }
    }

    internal function loadComponent(target) {
        if (target.loading || !target.configurable) {
            return
        }
        target.loading = true
        try {
            let path: Path?, pak: Path?
            if (target.withPath) {
                path = target.withpath
            } else {
                if (target.loaded) {
                    /* UNUSED if (target.enable == null) {
                        target.enable ||= true
                    }
                    target.path ||= path
                    */
                    target.diagnostic = 'Pre-loaded component'
                } else {
                    path = me.dir.paks.join(target.name)
                    pak = path.join(target.name + '.me')
                    if (pak.exists) {
                        b.loadMeFile(pak)
                        target.path ||= path
                        target.diagnostic = 'Load component from pak: ' + pak
                        /* UNUSED if (target.enable == null) {
                            target.enable ||= true
                        } */
                    } else {
                        path = findComponent(target.name)
                        if (path) {
                            vtrace('Found', 'Component at:' + path)
                            target.diagnostic = 'Found component: ' + path
                            target.file = path.portable
                            currentComponent = target.name
                            b.loadMeFile(path)

                        } else if (me.targets[target.name]) {
                            throw 'Cannot find component: ' + target.name + '.me'

                        } else {
                            print("UNUSED NEVER GET HERE")
                            throw 'BOOM'
                        }
                    }
                }
                target.loaded = true
            }
      
            if (!target.description) {
                let path = me.dir.paks.join(target.name)
                if (path.join(Me.PACKAGE).exists) {
                    let spec = path.join(Me.PACKAGE).readJSON()
                    target.description = spec.description
                } else if (target.name == me.name) {
                    let spec = Me.PACKAGE.readJSON()
                    target.description = spec.description
                }
                target.description ||= target.name.toPascal()
            }
            if (target.ifdef) {
                loadComponents(target.ifdef)
            }
            if (target.requires) {
                createTargets(target.requires)
                loadComponents(target.requires)
            }
            if (target.discovers) {
                createTargets(target.discovers)
                loadComponents(target.discovers)
            }
            loadComponents(target.depends)

            if (target.enable === undefined) {
                target.enable = true
            }
        } catch (e) {
            //  MOB - why try / catch here. What about enableComponents, configureComponents...
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            target.enable = false
            target.diagnostic = '' + e
            vtrace('Configure', target.name + ' failed: ' + target.diagnostic)
        }
    }

    internal function createTargets(components) {
        for each (name in components) {
            let target = me.targets[name]
            if (target) {
                target.loaded = true
            }
            target ||= {}
            me.targets[name] = target
            target.name ||= name
            target.type ||= 'group'
            target.configurable = true
        }
    }

    internal function enableComponents() {
        for each (target in me.targets) {
            //  UNUSED
            assert(!target.enabling)
            if (target.configurable && !target.enabling) {
                enableComponent(target)
            }
        }
    }

    /*
        Check for --without, and run enable scripts/functions
        Enable scripts do not run in dependency order
     */
    internal function enableComponent(target) {
        target.enabling = true
        if (target.explicit) {
            for each (dname in target.depends) {
                let dep = me.targets[dname]
                if (dep) {
                    dep.explicit = true
                }
            }
        }
        if (me.configure.extras.contains(target.name) && !target.explicit) {
            target.enable = false
            target.diagnostic = 'Component must be explicitly included via --with'

        } else if (target.without) {
            vtrace('Run', 'Component call without for: ' + target.name)
            runTargetScript(target, 'without')

        } else if (target.enable is Function) {
            /* This is not documented */
            vtrace('Run', 'Component call enable for: ' + target.name)
            target.enable = target.enable.call(b, target)

        } else if (target.enable && !(target.enable is Boolean)) {
            let script = expand(target.enable)
            vtrace('Run', 'Component eval enable expression for: ' + target.name)
            if (!eval(script)) {
                target.enable = false
            } else {
                target.enable = true
            }
        }
    }

    internal function configureComponents() {
        for each (target in me.targets) {
            if (target.configurable) {
                configureComponent(target)
            }
        }
    }

    internal function configureComponent(target) {
        if (target.configuring) {
            return
        }
        target.configuring = true
        let components = []
        if (target.requires) {
            components += target.requires
        }
        if (target.discovers) {
            components += target.discovers
        }
        for each (dname in components) {
            //  UNUSED - creation
            assert(me.targets[dname])
            let dext = me.targets[dname] ||= {}
            dext.name = dname
            dext.type ||= 'group'
            dext.configurable = true
            configureComponent(dext)
        }
        for each (name in target.ifdef) {
            let et = me.targets[name]
            if (!et) {
                me.targets[name] = { name: name, enable: false, diagnostic: 'Component not defined' }
            } else if (et.configurable) {
                configureComponent(et)
            }
        }
        currentComponent = target.name
        try {
            if (target.scripts && target.scripts.config) {
                let result = runTargetScript(target, 'config', {rethrow: true})
                if (result is String || result is Path) {
                    target.path = result
                } else if (Object.getOwnPropertyCount(result) > 0) {
                    blend(target, result, {combine: true})
                }
            }
            if (target.path is Function) {
                target.path = target.path.call(b, target)
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
    }

    internal function traceComponents() {
        let disabled = {}
        if (!b.options.configure && !b.options.verbose) return
        let components = getConfigurableTargets()
        for (let [name, target] in components) {
            if (!target.configurable) continue
            let description = target.description ? (': ' + target.description) : ''
            let diagnostic = target.diagnostic ? (': ' + target.diagnostic) : ''
            if (target.enable && !target.silent) {
                if (target.path) {
                    if (b.options.verbose) {
                        trace('Found', name + description + ' at: ' + target.path.compact())
                    } else if (!target.quiet) {
                        trace('Found', name + description + ': ' + target.path.compact())
                    }
                } else {
                    trace('Found', name + description)
                }
            } else {
                disabled[name] = target
                vtrace('Omit', name + description + diagnostic)
            }
        }
        if (b.options.why) {
            for (let [name, target] in disabled) {
                trace('Omit', name + diagnostic)
            }
        }
    }

    internal function checkComponent(target) {
        if (target.checking) {
            return
        }
        target.checking = true
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
            if (!b.options['continue']) {
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

    internal function checkComponents() {
        for each (target in me.targets) {
            if (!target.configurable) continue
            checkComponent(target)
        }
        for each (component in me.configure.requires) {
            let target = me.targets[component]
            if (!target) {
                throw 'Required component "' + component + ' cannot be found'
            } else if (!target.enable) {
                throw 'Required component "' + component + ' is not enabled: ' + target.diagnostic
            }
        }
    }

    internal function resetComponents() {
        for each (target in me.targets) {
            if (!target.configurable) continue
            delete target.loaded 
            delete target.loading 
            delete target.enabling 
            delete target.configuring 
            delete target.inheriting 
            delete target.checking 
        }
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
     */
    public function probe(file: Path, control = {}): Path {
        let path: Path?
        let search = [], dir
        if (file.exists) {
            path = file
        } else {
            if ((dir = me.targets[currentComponent].path) && !(dir is Function)) {
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
            if (b.options.why) {
                trace('Missing', 'Component "' + currentComponent + '" cannot find: "' + file + '"\n')
            }
            if (b.options['continue'] && control.default) {
                return control.default
            }
            throw 'Cannot find "' + file + '" for component "' + currentComponent + '" on ' + b.currentPlatform + '. '
        }
        vtrace('Probe', 'Component "' + currentComponent + '" found: "' + path)
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

    /*
        Only used when cross compiling. 
        Note: setting CFLAGS, DFLAGS etc overwrites internal me settings for compiler, defines etc.
     */
    internal function captureEnv() {
        if ((!b.platforms || b.platforms.length > 1) && !me.platform.cross) {
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

    public function getComponentSearch(target, component, objdir = '.') {
        let search = []
        if (target.withpath) {
            search.push(target.withpath)
        } else if (me.dir) {
            if (me.dir.paks) {
                /*
                    src/paks/NAME
                 */
                search.push(me.dir.paks.join(component, objdir))
            }
            /*
                ~/.paks/NAME/OLDEST-VERSION
             */
            let path = me.dir.home.join('.paks', component).files('*').reverse()[0]
            if (path) {
                search.push(path.join(objdir))
            }
            /*
                /usr/local/lib/me/paks/NAME/OLDEST-VERSION
             */
            path = me.dir.me.join('../paks', component).files('*').reverse()[0]
            if (path) {
                search.push(path.join(objdir))
            }
        }
        if (me.platform.like == 'unix') {
            if (me.platform.arch == 'x64') {
                search += ['/usr/lib64', '/lib64']
            }
            search += ['/usr/lib', '/lib' ]
        }
        if (me.platform.os == 'linux') {
            search += Path('/usr/lib').files('*-linux-gnu') + Path('/lib').files('*-linux-gnu')
        }
        return search
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
