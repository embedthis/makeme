/*
    configure.es -- Me configuration

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

    require ejs.unix
    require ejs.zlib

    /** @hide */
    public var currentExtension: String?

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
        if (settings.me && b.makeVersion(Config.Version) < b.makeVersion(settings.makeme)) {
            throw 'This product requires a newer version of Me. Please upgrade Me to ' + settings.me + '\n'
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
            findExtensions()
            // UNUSED setRequiredExtensions()
            captureEnv()
            b.castDirTypes()
            if (b.options.configure) {
                createPlatformMeFile()
                b.makeOutDirs()
                createMeHeader()
                importExtensionFiles()
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

    internal function importExtensionFiles() {
        for (let [pname, extension] in me.extensions) {
            if (extension.enable) {
                for each (file in extension.imports) {
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
            extensions: me.extensions,
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
        if (me.dir.makeme != Config.Bin) {
            nme.dir.makeme = me.dir.makeme
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
                '\nextensions = ' +
                serialize(nme.extensions, {pretty: true, indent: 4, commas: true, quotes: false}))
        }
    }

    /*
        Set extensions required for generation
        UNUSED
    internal function setRequiredExtensions() { 
        if (me.options.gen == 'make' || me.options.gen == 'nmake') {
            for each (target in me.targets) {
                for each (pname in target.extensions) {
                    if (!me.extensions[pname]) {
                        me.extensions[pname] ||= {}
                        me.extensions[pname].name = pname
                        me.extensions[pname].enable = true
                        if (b.options.why) {
                            trace('Create', 'Extension "' + pname + '", required for target "' + target.name + '"')
                        }
                    }
                }
            }
        }
    }
     */

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
            runTargetScript(target, 'postconfig')
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

        f.writeLine('\n/* Extensions */')
        let extensions = me.extensions.clone()
        Object.sortProperties(extensions)
        for (let [pname, extension] in extensions) {
            if (pname == 'compiler') {
                pname = 'cc'
            }
            def(f, 'ME_EXT_' + pname.toUpper(), extension.enable ? '1' : '0')
        }
        for (let [pname, extension] in extensions) {
            if (extension.enable) {
                /* Must test b.options.gen and not me.generating */
                if (!b.options.gen && extension.path) {
                    def(f, 'ME_EXT_' + pname.toUpper() + '_PATH', '"' + extension.path.relative + '"')
                }
                if (extension.definitions) {
                    for each (define in extension.definitions) {
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
        Search for an extension probe. Search in:
        - dir.makeme/probe
        - paks/NAME/package.json extensions:NAME.file

        @hide 
     */
    function findProbe(extension) {
        let path = Path(me.dir.makeme.join('probe', extension + '.me'))
        if (path.exists) {
            return path
        }
        let path = me.dir.paks.join(extension).join(Me.PACKAGE)
        if (path.exists) {
            let spec = path.readJSON()
            if (spec.extensions && spec.extensions[extension] && spec.extensions[extension].file) {
                return spec.extensions[extension].file
            }
        }
        return null
    }

    /**
        @hide
     */
    function findExtensions() {
        let settings = me.settings
        if (!settings.extensions) {
            return
        }
        let extensions = settings.extensions.require + settings.extensions.discover + settings.extensions.generate
        vtrace('Find', 'Extensions: ' + extensions.join(' '))
        createExtensionStubs(extensions)
        loadProbes(extensions)
        enableExtensions()
        configureExtensions()
        Object.sortProperties(me.extensions)
        checkExtensions()
        traceExtensions()
        resetExtensions()
    }

    internal function loadProbe(extension) {
        if (extension.loading || extension.enable === false) {
            return
        }
        let cname = extension.name
        try {
            extension.loading = true
            let pak = me.dir.paks.join(cname, cname + '.me')
            if (pak.exists) {
                /*
                    src/paks/NAME/NAME.me
                 */
                let pme = b.loadMe(pak)
                if (pme.probe && pme.probe[cname]) {
                    Me.extension(pme.probe)
                    extension.path ||= extension.withpath || me.dir.paks.join(cname)
                    extension.description ||= ''
                    extension.probed = 'Probe from pak: ' + pak
                }
                if (extension.enable == null) {
                    extension.enable ||= true
                }
            }
            if (!extension.path) {
                let probe: Path?
                if (me.probe && me.probe[cname]) {
                    extension.probed = 'Inline probe for : ' + cname
                    probe = me.probe[cname].file
                } 
                if (!probe) {
                    probe = findProbe(cname)
                    extension.probed = 'Found probe: ' + probe
                }
                if (probe) {
                    extension.file = probe.portable
                    currentExtension = cname
                    b.loadMeFile(probe)

                } else if (pak.exists) {
                    extension.path = pak

                } else if (me.extensions[cname]) {
                    extension.enable = false
                    extension.diagnostic = 'Cannot find extension: ' + cname + '.me'
                    throw extension.diagnostic
                }
            }

            if (!extension.description) {
                let path = me.dir.paks.join(cname)
                if (extension.enable && path.join(Me.PACKAGE).exists) {
                    let spec = path.join(Me.PACKAGE).readJSON()
                    extension.description = spec.description
                }
            }
            if (extension.extensions) {
                loadProbes(extension.extensions)
            }
            if (extension.discover) {
                loadProbes(extension.discover)
            }
            loadProbes(extension.depends)

            if (extension.enable === undefined) {
                extension.enable = true
            }
        } catch (e) {
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            extension.enable = false
            extension.diagnostic = '' + e
            vtrace('Probe', cname + ' probe failed: ' + extension.diagnostic)
        }
    }

    internal function createExtensionStubs(extensions) {
        for each (name in extensions) {
            let extension = me.extensions[name] ||= {}
            extension.name ||= name
        }
    }

    internal function loadProbes(extensions) {
        for each (name in extensions) {
            if (me.extensions[name]) {
                loadProbe(me.extensions[name])
            }
        }
    }

    internal function enableExtensions() {
        for each (extension in me.extensions) {
            if (extension.enabling) {
                continue
            }
            enableExtension(extension)
        }
    }

    /*
        Check for --without, and run enable scripts/functions
        Enable scripts do not run in dependency order. This are meant for simple scripts without extension dependencies.
     */
    internal function enableExtension(extension) {
        extension.enabling = true
        global.COMP = extension
        if (extension.enable === false && extension.explicit) {
            runExtensionScript(extension, 'without')

        } else if (extension.enable is Function) {
            vtrace('Run', 'Probe: ' + extension.name)
            extension.enable = extension.enable.call(b, extension)

        } else if (extension.enable) {                                                                           
            if (!(extension.enable is Boolean)) {
                let script = expand(extension.enable)
                if (!eval(script)) {
                    extension.enable = false
                } else {
                    extension.enable = true
                }
            }
        }
        if (extension.explicit) {
            for each (dname in extension.depends) {
                let dep = me.extensions[dname]
                if (dep) {
                    dep.explicit = true
                }
            }
        }
        if (me.settings.extensions.generate.contains(extension.name)) {
            if (!extension.explicit) {
                extension.enable = false
                extension.probed = 'Extension must be explicitly included via --with'
            }
        }
        delete global.COMP
    }

    /*
        Configure extensions in recursive dependency order
     */
    internal function configureExtensions() {
        for (let [cname, extension] in me.extensions) {
            extension.name ||= cname
            configureExtension(extension)
        }
    }

    internal function configureExtension(extension) {
        if (extension.configuring) {
            return
        }
        extension.configuring = true

        for each (dname in extension.discover) {
            let dext = me.extensions[dname] ||= {}
            dext.name = dname
            configureExtension(me.extensions[dname])
        }
        for each (pname in extension.extensions) {
            if (me.extensions[pname]) {
                configureExtension(me.extensions[pname])
            } else if (!me.targets[pname]) {
throw 'UNUSED - MISSING COMPONENT TARGET'
                me.extensions[pname] = { name: pname, enable: false, diagnostic: 'Extension not defined' }
            }
        }
        currentExtension = extension.name
        b.currentMeFile = extension.file
        global.COMP = extension
        try {
            if (extension.path is Function) {
                //  MOB - should use runExtensionScript - check all other call()
                let result = extension.path.call(b, extension)
                if (result is String || result is Path) {
                    extension.path = result
                } else if (Object.getOwnPropertyCount(result) > 0) {
                    blend(extension, result)
                }
            }
            if (extension.path is Path) {
                extension.path = extension.path.compact()
            }
            runExtensionScript(extension, 'config')
            
            if (extension.scripts && extension.scripts.generate) {
                print("WARNING: generate scripts in probes are deprecated: ", extension.name)
            }
            if (extension.path) {
                extension.path = Path(extension.path)
            }

        } catch (e) {
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            extension.path = null
            extension.enable = false
            extension.diagnostic = '' + e
            vtrace('Omit', 'Extension "' + extension.name + '": ' + extension.diagnostic + '\n')
        }
        delete global.COMP
    }

    internal function traceExtensions() {
        let disabled = {}
        if (!b.options.configure && !b.options.verbose) return
        for (let [pname, extension] in me.extensions) {
            if (extension.enable && !extension.silent) {
                if (extension.path) {
                    if (b.options.verbose) {
                        trace('Found', pname + ': ' + extension.description + ' at:\n                 ' + 
                            extension.path.compact())
                    } else if (!extension.quiet) {
                        trace('Found', pname + ': ' + extension.description + ': ' + extension.path.compact())
                    }
                } else {
                    trace('Found', pname + ': ' + extension.description)
                }
            } else {
                disabled[pname] = extension
                vtrace('Omit', pname + ': ' + extension.description + ':\n                 ' + extension.diagnostic)
            }
        }
        if (b.options.why) {
            for (let [pname, extension] in disabled) {
                extension.diagnostic ||= 'Not configured'
                trace('Omit', pname + ': ' + extension.diagnostic)
            }
        }
    }

    internal function checkExtension(extension) {
        if (extension.checking) {
            return
        }
        extension.checking = true
        extension.type ||= 'extension'

        /* Recursive descent checking */
        for each (pname in extension.extensions) {
            let p = me.extensions[pname]
            if (p) {
                checkExtension(p)
                if (!p.enable) {
                    extension.enable = false
                    extension.diagnostic ||= 'required extension ' + p.name + ' is not enabled'
                }
            }
        }
        if (!extension.enable && extension.essential) {
            if (!b.options['continue']) {
                throw 'Required extension "' + extension.name + '" is not enabled: ' + extension.diagnostic
            }
        }
        if (extension.enable) {
            for each (o in extension.conflicts) {
                let other = me.extensions[o]
                if (other && other.enable) {
                    other.enable = false
                    other.diagnostic ||= 'conflicts with ' + extension.name
                }
            }
            for (let [i, path] in extension.libpaths) {
                extension.libpaths[i] = Path(path).natural
            }
            for (let [i, path] in extension.includes) {
                extension.includes[i] = Path(path).natural
            }
        }
    }

    internal function checkExtensions() {
        for each (extension in me.extensions) {
            checkExtension(extension)
        }
    }

    internal function resetExtensions() {
        for each (extension in me.extensions) {
            delete extension.loading 
            delete extension.enabling 
            delete extension.configuring 
            delete extension.inheriting 
            delete extension.checking 
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
            if ((dir = me.extensions[currentExtension].path) && !(dir is Function)) {
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
                trace('Missing', 'Extension "' + currentExtension + '" cannot find: "' + file + '"\n')
            }
            if (b.options['continue'] && control.default) {
                return control.default
            }
            throw 'Cannot find "' + file + '" for extension "' + currentExtension + '" on ' + b.currentPlatform + '. '
        }
        vtrace('Probe', 'Extension "' + currentExtension + '" found: "' + path + '"\n')
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

    /**
        Define an extension for a command line program.
        This registers the extension and loads the Me DOM with the extension configuration.
        @param name Program name. Can be either a path or a basename with optional extension
        @param description Short, single-line program description.
        @param options Extra options to pass to Me.extension when defining the program.
     */
    public function program(name: Path, description = null, options = {}): Path {
        let extension = me.extensions[currentExtension]
        let path
        try {
            path = (me.options.gen) ? name : probe(extension.withpath || name, {fullpath: true})
        } catch (e) {
            throw e
        }
        let cfg = {}
        cfg[name] = {
            name: name,
            description: description,
            path: path,
        }
        blend(cfg[name], options)
        Me.extension(cfg)
        return path
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
        envSettings = { extensions: { compiler: {} } }
        for (let [key, tool] in envTools) {
            let path = App.getenv(key)
            if (path) {
                envSettings.extensions[tool] ||= {}
                envSettings.extensions[tool].path = path
                envSettings.extensions[tool].enable = true
            }
        }
        for (let [flag, option] in envFlags) {
            let value = App.getenv(flag)
            if (value) {
                envSettings.extensions.compiler[option] ||= []
                envSettings.extensions.compiler[option] += [value]
            }
        }
        blend(me, envSettings, {combine: true})
    }

    internal function runExtensionScript(extension, when) {
        if (!extension.scripts) return
        for each (item in extension.scripts[when]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                App.chdir(expand(item.home))
            }
            global.COMP = extension
            try {
                if (item.interpreter == 'ejs') {
                    if (item.script is Function) {
                        item.script.call(b, extension)
                    } else {
                        let script = expand(item.script).expand(target.vars, {fill: ''})
                        script = 'require ejs.unix\n' + script
                        eval(script)
                    }
                } else {
                    throw 'Only ejscripts are support for extensions'
                }
            } finally {
                App.chdir(pwd)
                global.COMP = null
            }
        }
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
