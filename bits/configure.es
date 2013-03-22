/*
    configure.es -- Bit configuration

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.bit {

    require ejs.unix
    require ejs.zlib

    /** @hide */
    public var currentPack: String?

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
        Configure and initialize for building. This generates platform specific bit files.
        @hide
     */
    function configure() {
        vtrace('Load', 'Preload main.bit to determine required platforms')
        b.quickLoad(b.options.configure.join(b.MAIN))
        let settings = bit.settings
        if (settings.bit && b.makeVersion(Config.Version) < b.makeVersion(settings.bit)) {
            throw 'This product requires a newer version of Bit. Please upgrade Bit to ' + settings.bit + '\n'
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
            if (bit.platform.cross) {
                captureEnv()
            }
            b.makeBit(platform, b.options.configure.join(b.MAIN))
            findPacks()
            setRequiredPacks()
            b.castDirTypes()
            createPlatformBitFile()
            b.makeOutDirs()
            createBitHeader()
            importPackFiles()
        }
        if (!b.options.gen) {
            createStartBitFile(b.platforms[0])
            trace('Info', 'Use "bit" to build. Use "bit configuration" to see current settings"')
        }
    }

    function reconfigure() {
        vtrace('Load', 'Preload main.bit to determine required configuration')
        b.quickLoad(b.MAIN)
        b.platforms = bit.platforms = [b.localPlatform]
        b.makeBit(b.localPlatform, b.localPlatform + '.bit')
        if (bit.settings.configure) {
            run(bit.settings.configure)
        } else {
            App.log.error('No prior configuration to use')
        }
    }

    internal function importPackFiles() {
        for (let [pname, pack] in bit.packs) {
            for each (file in pack.imports) {
                vtrace('Import', file)
                if (file.extension == 'h') {
                    cp(file, bit.dir.inc)
                } else {
                    if (bit.platform.like == 'windows') {
                        let target = bit.dir.lib.join(file.basename).relative
                        let old = target.replaceExt('old')
                        vtrace('Preserve', 'Active library ' + target + ' as ' + old)
                        old.remove()
                        target.rename(old)
                    }
                    cp(file, bit.dir.lib)
                }
            }
        }
    }

    internal function createStartBitFile(platform) {
        let nbit = { }
        nbit.platforms = b.platforms
        trace('Generate', b.START)
        let data = '/*\n    start.bit -- Startup Bit File for ' + bit.settings.title + 
            '\n */\n\nBit.load(' + 
            serialize(nbit, {pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
        b.START.write(data)
    }

    internal function createPlatformBitFile() {
        let nbit = {}
        blend(nbit, {
            blend: [ 
                '${SRC}/main.bit',
            ],
            platform: bit.platform,
            dir: { 
                src: bit.dir.src.absolute.portable,
                top: bit.dir.top.portable,
            },
            settings: { configured: true },
            prefixes: bit.prefixes,
            packs: bit.packs,
            env: bit.env,
        })
        for (let [key, value] in bit.settings) {
            /* Copy over non-standard settings. These include compiler sleuthing settings */
            nbit.settings[key] = value
        }
        blend(nbit.settings, bit.customSettings)
        nbit.settings.configure = 'bit ' + App.args.slice(1).join(' ')

        if (envSettings) {
            blend(nbit, envSettings, {combine: true})
        }
        if (bit.dir.bits != Config.Bin.join('bits')) {
            nbit.dir.bits = bit.dir.bits
        }
        if (nbit.settings) {
            Object.sortProperties(nbit.settings)
        }
        b.runScript(bit.scripts, "postconfig")
        if (b.options.configure) {
            let path: Path = Path(bit.platform.name).joinExt('bit')
            trace('Generate', path)
            let data = '/*\n    ' + path + ' -- Build ' + bit.settings.title + ' for ' + bit.platform.name + 
                '\n */\n\nBit.load(' + 
                serialize(nbit, {pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
            path.write(data)
        }
        if (b.options.show && b.options.verbose) {
            trace('Configuration', bit.settings.title + 
                '\nsettings = ' +
                serialize(bit.settings, {pretty: true, indent: 4, commas: true, quotes: false}) +
                '\npacks = ' +
                serialize(nbit.packs, {pretty: true, indent: 4, commas: true, quotes: false}))
        }
    }

    /*
        Set packs required for generation from bit.settings.projects
     */
    internal function setRequiredPacks() { 
        if (bit.options.gen) {
            for (let [pname, enabled] in bit.settings.projects) {
                bit.packs[pname] ||= {}
                bit.packs[pname].enable = enabled
            }
            for each (target in bit.targets) {
                for each (pname in target.requires) {
                    bit.packs[pname] ||= {}
                    bit.packs[pname].enable = true
                }
            }
        }
    }

    internal function createBitHeader() {
        b.runScript(bit.scripts, "prebitheader")
        let path = bit.dir.inc.join('bit.h')
        let f = TextStream(File(path, 'w'))
        f.writeLine('/*\n    bit.h -- Build It Configuration Header for ' + bit.platform.name + '\n\n' +
                '    This header is created by Bit during configuration. To change settings, re-run\n' +
                '    configure or define variables in your Makefile to override these default values.\n */\n')
        writeDefinitions(f)
        f.close()
        for (let [tname, target] in bit.targets) {
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
        let settings = bit.settings.clone()
        if (b.options.endian) {
            settings.endian = b.options.endian == 'little' ? 1 : 2
        }
        f.writeLine('\n/* Settings */')
        writeSettings(f, "BIT", settings)

        f.writeLine('\n/* Prefixes */')
        for (let [name, prefix] in bit.prefixes) {
            def(f, 'BIT_' + name.toUpper() + '_PREFIX', '"' + prefix.portable + '"')
        }

        /* Suffixes */
        f.writeLine('\n/* Suffixes */')
        def(f, 'BIT_EXE', '"' + bit.ext.dotexe + '"')
        def(f, 'BIT_SHLIB', '"' + bit.ext.dotshlib + '"')
        def(f, 'BIT_SHOBJ', '"' + bit.ext.dotshobj + '"')
        def(f, 'BIT_LIB', '"' + bit.ext.dotlib + '"')
        def(f, 'BIT_OBJ', '"' + bit.ext.doto + '"')

        /* Build profile */
        f.writeLine('\n/* Profile */')
        let args = 'bit ' + App.args.slice(1).join(' ')
        def(f, 'BIT_CONFIG_CMD', '"' + args + '"')
        def(f, 'BIT_' + settings.product.toUpper() + '_PRODUCT', '1')
        def(f, 'BIT_PROFILE', '"' + bit.platform.profile + '"')

        /* Architecture settings */
        f.writeLine('\n/* Miscellaneous */')
        if (settings.charlen) {
            def(f, 'BIT_CHAR_LEN', settings.charlen)
            if (settings.charlen == 1) {
                def(f, 'BIT_CHAR', 'char')
            } else if (settings.charlen == 2) {
                def(f, 'BIT_CHAR', 'short')
            } else if (settings.charlen == 4) {
                def(f, 'BIT_CHAR', 'int')
            }
        }
        let ver = settings.version.split('.')
        def(f, 'BIT_MAJOR_VERSION',  ver[0])
        def(f, 'BIT_MINOR_VERSION', ver[1])
        def(f, 'BIT_PATCH_VERSION', ver[2])
        def(f, 'BIT_VNUM',  ((((ver[0] * 1000) + ver[1]) * 1000) + ver[2]))

        f.writeLine('\n/* Packs */')
        let packs = bit.packs.clone()
        Object.sortProperties(packs)
        for (let [pname, pack] in packs) {
            if (pname == 'compiler') {
                pname = 'cc'
            }
            def(f, 'BIT_PACK_' + pname.toUpper(), pack.enable ? '1' : '0')
        }
        for (let [pname, pack] in packs) {
            if (pack.enable) {
                /* Must test b.options.gen and not bit.generating */
                if (!b.options.gen && pack.path) {
                    def(f, 'BIT_PACK_' + pname.toUpper() + '_PATH', '"' + pack.path.relative + '"')
                }
                if (pack.definitions) {
                    for each (define in pack.definitions) {
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

    internal function findPacks() {
        let settings = bit.settings
        if (!settings.requires && !settings.discover) {
            return
        }
        trace('Search', 'For tools and extension packages')
        let packs = settings.requires + settings.discover
        if (bit.options.gen && bit.settings.projects) {
            packs += Object.getOwnPropertyNames(bit.settings.projects)
        }
        loadPacks(packs)
        enablePacks()
        configurePacks()
        Object.sortProperties(bit.packs)
        checkPacks()
        inheritPacks()
        tracePacks()
        resetPacks()
    }

    /*
        Load the packs, but don't run any events. Events must be fired in recursive dependency order
     */
    internal function loadPacks(packs) {
        vtrace('Search', 'Packages: ' + packs.join(' '))
        for each (pname in packs) {
            if (bit.packs[pname]) {
                let pack = bit.packs[pname]
                if (pack.loaded || pack.enable === false) {
                    continue
                }
            }
            let path = b.findPack(pname)
            if (!path.exists) {
                throw 'Cannot find pack description file: ' + pname + '.bit'
            }
            let pack = bit.packs[pname] ||= {}
            pack.name ||= pname
            pack.enable ||= undefined
            try {
                pack.loaded = true
                pack.file = path.portable
                currentPack = pname

                vtrace('Search', 'For pack: ' + pname)
                b.loadBitFile(path)

                if (pack.requires) {
                    loadPacks(pack.requires)
                }
                if (pack.discover) {
                    loadPacks(pack.discover)
                }
                if (pack.enable === undefined) {
                    pack.enable = true
                }
            } catch (e) {
                if (!(e is String)) {
                    App.log.debug(0, e)
                }
                pack.enable = false
                pack.diagnostic = "" + e
            }
        }
    }

    internal function enablePacks() {
        for each (pack in bit.packs) {
            if (pack.enabling) {
                continue
            }
            enablePack(pack)
        }
        for each (r in bit.settings.requires) {
            bit.packs[r].required = true
        }
    }

    /*
        Check for --without pack, and run enable scripts/functions
        Enable scripts do not run in dependency order. This are meant for simple scripts without pack dependencies.
     */
    internal function enablePack(pack) {
        pack.enabling = true
        global.PACK = pack
        if (pack.enable === false && pack.explicit) {
            runPackScript(pack, "without")

        } else if (pack.enable is Function) {
            pack.enable = pack.enable.call(b, pack)

        } else if (pack.enable) {                                                                           
            if (!(pack.enable is Boolean)) {
                let script = expand(pack.enable)
                if (!eval(script)) {
                    pack.enable = false
                } else {
                    pack.enable = true
                }
            }
        }
        delete global.PACK
    }

    /*
        Configure packs in recursive dependency order
     */
    internal function configurePacks() {
        for (let [pname, pack] in bit.packs) {
            pack.name ||= pname
            configurePack(pack)
        }
    }

    internal function configurePack(pack) {
        if (pack.configuring) {
            return
        }
        pack.configuring = true
        for each (pname in pack.requires) {
            configurePack(bit.packs[pname])
        }
        for each (pname in pack.discover) {
            configurePack(bit.packs[pname])
        }
        for each (pname in pack.depends) {
            if (bit.packs[pname]) {
                configurePack(bit.packs[pname])
            } else {
                bit.packs[pname] = { name: pname, enable: false }
            }
        }
        for each (pname in pack.after) {
            if (bit.packs[pname]) {
                configurePack(bit.packs[pname])
            } else {
                bit.packs[pname] = { name: pname, enable: false }
            }
        }
        currentPack = pack.name
        b.currentBitFile = pack.file
        global.PACK = pack
        try {
            if (bit.generating) {
                if (pack.scripts.generate) {
                    runPackScript(pack, "generate")
                } else {
                    pack.path = Path(pname)
                }
            } else {
                if (pack.path is Function) {
                    pack.path = pack.path.call(b, pack)
                }
                runPackScript(pack, "config")
            }
            if (pack.path) {
                pack.path = Path(pack.path)
            }

        } catch (e) {
            // print("CATCH", pack.name, e)
            if (!(e is String)) {
                App.log.debug(0, e)
            }
            pack.enable = false
            pack.diagnostic = "" + e
        }
        delete global.PACK
    }

    internal function inheritFromPack(pack, dep) {
        for each (lib in dep.libraries) {
            pack.libraries ||= []
            if (!pack.libraries.contains(lib)) {
                pack.libraries.push(lib)
            }
        }
        for each (option in dep.linker) {
            pack.linker ||= []
            if (!pack.linker.contains(option)) {
                pack.linker.push(option)
            }
        }
        for each (option in dep.libpaths) {
            pack.libpaths ||= []
            if (!pack.libpaths.contains(option)) {
                pack.libpaths.push(option)
            }
        }
        for each (option in dep.includes) {
            pack.includes ||= []
            if (!pack.includes.contains(option)) {
                pack.includes.push(option)
            }
        }
        for each (option in dep.defines) {
            pack.defines ||= []
            if (!pack.defines.contains(option)) {
                pack.defines.push(option)
            }
        }
    }

    internal function inheritPack(pack) {
        if (pack.inheriting) {
            return
        }
        pack.inheriting = true

        for each (pname in pack.depends) {
            inheritPack(bit.packs[pname])
        }
        for each (pname in pack.depends) {
            let dep = bit.packs[pname]
            if (dep && dep.enable) {
                inheritFromPack(pack, dep)
            }
        }
    }

    internal function inheritPacks() {
        for (let [pname, pack] in bit.packs) {
            inheritPack(pack)
        }
    }

    internal function tracePacks() {
        let omitted = {}
        for (let [pname, pack] in bit.packs) {
            if (pack.enable && !pack.silent) {
                if (pack.path) {
                    if (b.options.verbose) {
                        trace('Found', pname + ': ' + pack.description + ' at:\n                 ' + pack.path.portable)
                    } else if (!pack.quiet) {
                        trace('Found', pname + ': ' + pack.description + ': ' + pack.path.portable)
                    }
                } else {
                    trace('Found', pname + ': ' + pack.description)
                }
            } else {
                omitted[pname] = pack
            }
        }
        if (b.options.why) {
            for (let [pname, pack] in omitted) {
                trace('Omitted', pname + ': ' + pack.description + ': ' + pack.diagnostic)
            }
        }
    }

    internal function checkPack(pack) {
        if (pack.checking) {
            return
        }
        pack.checking = true
        /* Recursive descent checking */
        for each (pname in pack.requires) {
            let p = bit.packs[pname]
            if (p) {
                checkPack(p)
                if (!p.enable) {
                    pack.enable = false
                    pack.diagnostic ||= 'required pack ' + p.name + ' is not enabled'
                }
            }
        }
        if (!pack.enable && pack.required) {
            if (!b.options['continue']) {
                throw 'Required pack ' + pack.name + ' is not enabled.'
            }
        }
        if (pack.enable) {
            for each (o in pack.conflicts) {
                let other = bit.packs[o]
                if (other && other.enable) {
                    other.enable = false
                    other.diagnostic ||= 'conflicts with ' + pack.name
                }
            }
        }
    }

    internal function checkPacks() {
        for each (pack in bit.packs) {
            checkPack(pack)
        }
    }

    internal function resetPacks() {
        for each (pack in bit.packs) {
            delete pack.loaded 
            delete pack.enabling 
            delete pack.configuring 
            delete pack.inheriting 
            delete pack.checking 
        }
    }

    /**
        Probe for a file and locate
        Will throw an exception if the file is not found, unless {continue, default} specified in control options
        @param file File to search for
        @param control Control options
        @option default Default path to use if the file cannot be found and bit is invoked with --continue
        @option search Array of paths to search for the file
        @option nopath Don't use the system PATH to locate the file
        @option fullpath Return the full path to the located file
     */
    public function probe(file: Path, control = {}): Path {
        if (bit.generating) {
            return file
        }
        let path: Path?
        let search = [], dir
        if (file.exists) {
            path = file
        } else {
            if ((dir = bit.packs[currentPack].path) && !(dir is Function)) {
                search.push(dir)
            }
            if (control.search) {
                if (!(control.search is Array)) {
                    control.search = [control.search]
                }
                search += control.search
            }
            App.log.debug(2, "Probe for " + file + ' in search path: ' + search)
            for each (let s: Path in search) {
                App.log.debug(2, "Probe for " + s.join(file) + ' exists: ' + s.join(file).exists)
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
                trace('Probe', 'File ' + file)
                trace('Search', search.join(' '))
            }
            if (b.options['continue'] && control.default) {
                return control.default
            }
            throw 'Cannot find ' + file + ' for package "' + currentPack + '" on ' + b.currentPlatform + '. '
        }
        App.log.debug(2, 'Probe for ' + file + ' found at ' + path)
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
        Define a pack for a command line program.
        This registers the pack and loads the Bit DOM with the pack configuration.
        @param name Program name. Can be either a path or a basename with optional extension
        @param description Short, single-line program description.
        @param options Extra options to pass to Bit.pack when defining the program package.
     */
    public function program(name: Path, description = null, options = {}): Path {
        let pack = bit.packs[currentPack]
        let path
        try {
            if (bit.generating) {
                path = name
            } else {
                path = probe(pack.withpath || name, {fullpath: true})
            }
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
        Bit.pack(cfg)
        return path
    }

    internal function captureEnv() {
        envSettings = { packs: {}, defaults: {} }
        for (let [key, tool] in envTools) {
            let path = App.getenv(key)
            if (path) {
                envSettings.packs[tool] ||= {}
                envSettings.packs[tool].path = path
                envSettings.packs[tool].enable = true
            }
        }
        for (let [flag, option] in envFlags) {
            let value = App.getenv(flag)
            if (value) {
                let flag = ((options.configure) ? '+' : '') + option
                envSettings.defaults[option] ||= []
                envSettings.defaults[option] += value.replace(/^-I/, '').split(' ')
                envSettings.defaults['override-' + option.trim('+')] = true
            }
        }
        blend(bit, envSettings, {combine: true})
    }

    internal function runPackScript(pack, when) {
        if (!pack.scripts) return
        for each (item in pack.scripts[when]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                App.chdir(expand(item.home))
            }
            global.PACK = pack
            try {
                if (item.interpreter == 'ejs') {
                    if (item.script is Function) {
                        item.script.call(b, pack)
                    } else {
                        let script = expand(item.script).expand(target.vars, {fill: ''})
                        script = 'require ejs.unix\n' + script
                        eval(script)
                    }
                } else {
                    throw "Only ejscripts are support for packs"
                    // runShell(target, item.interpreter, item.script)
                }
            } finally {
                App.chdir(pwd)
                global.PACK = null
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
