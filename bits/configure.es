/*
    configure.es -- Bit configuration

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.bit {

    require ejs.unix
    require ejs.zlib

    public var currentPack: String?

    var envTools = {
        AR: 'lib',
        CC: 'compiler',
        LD: 'linker',
    }

    var envFlags = {
        CFLAGS:  'compiler',
        DFLAGS:  'defines',
        IFLAGS:  'includes',
        LDFLAGS: 'linker',
    }
    var envSettings: Object

    /*  
        Configure and initialize for building. This generates platform specific bit files.
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
            b.castDirTypes()
            createPlatformBitFile()
            b.makeOutDirs()
            b.runScript(bit.scripts, "prebitheader")
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
        b.platforms = bit.platforms = [b.localPlatform]
        b.makeBit(b.localPlatform, b.localPlatform + '.bit')
        if (bit.settings.configure) {
            run(bit.settings.configure)
        } else {
            App.log.error('No prior configuration to use')
        }
    }

    function importPackFiles() {
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

    function createStartBitFile(platform) {
        let nbit = { }
        nbit.platforms = b.platforms
        trace('Generate', b.START)
        let data = '/*\n    start.bit -- Startup Bit File for ' + bit.settings.title + 
            '\n */\n\nBit.load(' + 
            serialize(nbit, {pretty: true, indent: 4, commas: true, quotes: false}) + ')\n'
        b.START.write(data)
    }

    function createPlatformBitFile() {
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
            Object.sortProperties(nbit.settings);
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

    function createBitHeader() {
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

    function def(f: TextStream, key, value) {
        f.writeLine('#ifndef ' + key)
        f.writeLine('    #define ' + key + ' ' + value)
        f.writeLine('#endif')
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
            } else {
                def(f, key, '"' + value + '"')
            }
        }
    }
    function writeDefinitions(f: TextStream) {
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

    /*
        Search for enabled packs in the system
     */
    function findPacks() {
        let settings = bit.settings
        if (!settings.required && !settings.discover) {
            return
        }
        trace('Search', 'For tools and extension packages')
        vtrace('Search', 'Packages: ' + [settings.required + settings.discover].join(' '))
        //  MOB Temp
        if (settings.optional) {
            settings.discover += settings.optional
        }
        let packs = settings.required + settings.discover
        let omitted = []
        for each (pack in packs) {
            if (bit.packs[pack] && bit.packs[pack].enable == false) {
                if (settings.required.contains(pack)) { 
                    throw 'Required pack ' + pack + ' is not enabled'
                }
                continue
            }
            let path = b.findPack(pack)
            if (path.exists) {
                try {
                    bit.packs[pack] ||= {}
                    bit.packs[pack].enable ||= true
                    currentPack = pack
                    b.loadBitFile(path)
                } catch (e) {
                    if (!(e is String)) {
                        App.log.debug(0, e)
                    }
                    let kind = settings.required.contains(pack) ? 'Required' : 'Optional'
                    whyMissing(kind + ' package "' + pack + '". ' + e)
                    let p = bit.packs[pack] ||= {}
                    p.enable = false
                    p.diagnostic = "" + e
                    if ((kind == 'Required' || bit.packs[pack].required) && !b.options['continue']) {
                        throw e
                    }
                }
            } else {
                throw 'Cannot find pack description file: ' + pack + '.pak'
            }
            let p = bit.packs[pack]
            if (p) {
                let desc = p.description || pack
                if (p && p.enable && p.path) {
                    if (b.options.verbose) {
                        trace('Found', desc + ' at:\n                 ' + p.path.portable)
                    } else if (!p.quiet) {
                        trace('Found', desc + ': ' + p.path.portable)
                    }
                } else {
                    omitted.push(desc)
                }
            } else {
                omitted.push(pack)
            }
        }
        for each (item in omitted) {
            trace('Omitted', 'Optional: ' + item)
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
        let path: Path?
        let search = [], dir
        if (file.exists) {
            path = file
        } else {
            if (dir = bit.packs[currentPack].path) {
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

    function captureEnv() {
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

    public function whyMissing(...msg) {
        if (b.options.why) {
            trace('Missing', ...msg)
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
