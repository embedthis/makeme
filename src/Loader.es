/*
    Loader.es -- Embedthis MakeMe File Loader class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

require ejs.version

/**
    The Loader class loads MakeMe files and creates the MakeMe Document Object Model (DOM).
    @stability Prototype
 */
public class Loader {
    /** Some doc */
    public static const BUILD: Path = Path('build')
    public static const MAIN: Path = Path('main.me')
    public static const PACKAGE: Path = Path('package.json')
    public static const PLATFORM: Path = Path('platform.me')
    public static const START: Path = Path('start.me')
    public static const Unix = ['macosx', 'linux', 'unix', 'freebsd', 'solaris']
    public static const Windows = ['windows', 'wince']

    public var localPlatform: String
    private static var loadObj
    private var loaded: Object
    private var options: Object

    function Loader() {
        options = makeme.options
        localPlatform = options.local || (Config.OS + '-' + Config.CPU + '-' + (options.release ? 'release' : 'debug'))
        reset()
    }

    /**
        Apply command line --with/--without --enable/--disable options
     */
    public function applyCommandLine() {
        let options = me.options
        if (options.debug) {
            me.settings.debug = true
        }
        if (options.release) {
            me.settings.debug = false
        }
        if (me.settings.debug == undefined) {
            me.settings.debug = true
        }
        if (!options.platforms) {
            return
        }
        var poptions = options.platforms[me.platform.name]
        if (!poptions) {
            return
        }
        /*
            Disable/enable was originally --unset|--set
         */
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
            if (me.configure) {
                let configure = me.configure.requires + me.configure.discovers
                if (configure.contains(field)) {
                    App.log.error('Using "--set ' + field + '", but ' + field + ' is a configurable target. ' +
                            'Use --with or --without instead.')
                    App.exit(1)
                }
            }
            makeme.setSetting(me.settings, field, value)
        }
        for each (field in poptions['without']) {
            if (me.configure.requires.contains(field)) {
                throw 'Required component "' + field + '"" cannot be disabled.'
            }
            if (field == 'all' || field == 'default') {
                let list = me.settings['without-' + field] || me.configure.discovers
                for each (f in list) {
                    let target = createTarget({
                        name: f,
                        type: 'component',
                        configurable: true,
                        without: true,
                        enable: false,
                        explicit: 'without',
                        diagnostic: 'Component disabled via --without ' + f + '.'
                    })
                }
                continue
            }
            let target = createTarget({
                name: field,
                type: 'component',
                configurable: true,
                enable: false,
                explicit: 'without',
                diagnostic: 'Component disabled via --without ' + field '.'
            })
        }
        let requires = []
        for each (field in poptions['with']) {
            let [field,value] = field.split('=')
            let target = me.targets[field]
            if (!target) {
                target = createTarget({
                    name: field,
                    type: 'component',
                    explicit: 'with',
                    enable: false,
                    bare: true,
                })
            }
            target.explicit = true
            target.essential = true
            if (value) {
                target.withpath = Path(value)
            }
            target.diagnostic = ''
            if (!(me.configure.requires && me.configure.requires.contains(field)) &&
                !(me.configure.discovers && me.configure.discovers.contains(field))) {
                requires.push(field)
            }
        }
        if (requires.length > 0) {
            /* Insert explicit require first */
            me.configure.requires = requires + me.configure.requires
        }
    }

    /*
        Apply optional platform specific profiles
     */
    function applyPlatformProfile() {
        if (me.profiles && me.profiles[me.platform.profile]) {
            global.blend(me, me.profiles[me.platform.profile], {combine: true})
        }
    }

    function applySettings() {
        for each (target in me.targets) {
            if (target.type == 'exe' || target.type == 'lib') {
                if (target.static == undefined) {
                    target.static = me.settings.static
                }
            }
        }
    }

    /*
        Blend files referenced by the 'blend' property.
        These are blended before the parent file is blended
     */
    function blendBlends(obj) {
        let home = obj.origin
        for each (let name in obj.blend) {
            let optional
            if (name.startsWith('?')) {
                name = name.slice(1)
                optional = true
            }
            name = expand(name, {missing: null})
            let files = home.files(name)
            if (files.length == 0) {
                if ((path = findPlugin(name, false)) != null) {
                    files = [path]
                } else if (!optional) {
                    trace('Warn', 'Cannot find plugin: "' + name + '" ... continuing')
                }
            }
            if (files.length == 0 && !optional) {
                throw 'Cannot find blended module: ' + name
            }
            for each (let file in files) {
                blendFile(file)
            }
        }
    }

    /*
        Blend files nominated in the customize[] property into 'me'.
        These are applied after the parent file is loaded. No errors if the file ddoes not exist.
     */
    function blendCustomize(obj) {
        let home = obj.origin
        for each (cpath in me.customize) {
            cpath = home.join(expand(cpath, {missing: '.'}))
            if (cpath.exists) {
                blendFile(cpath)
            }
        }
    }

    /*
        Blend a MakeMe file and into an existing 'me'. Use loadFile for loading the top level MakeMe file.
     */
    public function blendFile(path: Path): Void {
        path = Path(expand(path, {missing: '.'})).absolute
        if (loaded[path]) {
            // vtrace('Info', 'MakeMe file "' + path.relative + '" already loaded')
            return
        }
        loaded[path] = true
        let priorObj = loadObj
        blendObj(readFile(path))
        loadObj = priorObj
    }

    /*
        Load modules and mixins
     */
    public function blendModules(obj) {
        let home = obj.origin
        if (obj.modules) {
            if (!(obj.modules is Array)) {
                obj.modules = [obj.modules]
            }
            for each (let name in obj.modules) {
                let optional
                if (name.startsWith('?')) {
                    name = name.slice(1)
                    optional = true
                }
                name = expand(name, {missing: null})
                let files = home.files(name)
                if (files.length == 0 && !optional) {
                    throw 'Cannot find blended module: "' + name + '"'
                }
                for each (let path in files) {
                    vtrace('Module', path)
                    try {
                        makeItemGlobals(path.dirname)
                        global.load(path)
                    } catch (e) {
                        throw new Error('When loading: ' + path + '\n' + e)
                    }
                }
            }
        }
        if (obj.mixin) {
            if (!(obj.mixin is Array)) {
                obj.mixin = [obj.mixin]
            }
            for each (let mix in obj.mixin) {
                App.log.debug(2, 'Load mixin from: ' + obj.origin)
                try {
                    makeItemGlobals(obj.origin)
                    global.eval(expand(mix))
                } catch (e) {
                    throw new Error('When loading mixin' + e)
                }
            }
        }
    }

    /*
        Blend a MakeMe object. This blends the object but first blends any included blend files.
     */
    public function blendObj(obj): Object {
        obj.origin ||= me.dir.top
        blendPlugins(obj)
        blendBlends(obj)
        blendModules(obj)
        fixupProperties(obj)

        /*
            Blend properties sans-targets. Targets are specially crafted via create() for inherited defaults.
         */
        let targets = obj.targets
        delete obj.targets

        global.blend(me, obj, {functions: true, combine: true, overwrite: true})
        if (obj.dir) {
            /* Got new directories */
            castDirPaths()
            makeDirectoryGlobals()
        }
        /*
            Create targets
         */
        for (let [name, props] in targets) {
            props.name ||= name
            props.origin = obj.origin
            props.home ||= obj.origin
            props.topLevel = true
            let target = me.targets[name]
            if (target) {
                makeme.builder.runTargetScript(target, 'preblend')
            }
            global.blend(props, me.targets[name], {functions: true, overwrite: false})
            target = createTarget(props)
            makeme.builder.runTargetScript(target, 'postblend')
        }
        blendCustomize(obj)
        return obj
    }

    function blendPlugins(obj) {
        let plugins = obj.plugins || []
        if (obj.master) {
            /*
                Get extra (horizontal) plugins from ejsrc
             */
            if (App.config.makeme) {
                let extras = App.config.makeme.plugins
                if (extras) {
                    extras = castArray(extras)
                    plugins = (plugins + extras).unique()
                }
            }
            if (App.env.MAKEME_PLUGINS) {
                extras = App.env.MAKEME_PLUGINS.split(',')
                plugins = (plugins + extras).unique()
            }
        }
        for each (plugin in plugins) {
            let path = findPlugin(plugin, false)
            if (path) {
                blendFile(path)
            } else if (!plugin.startsWith('?')) {
                trace('Warn', 'Cannot find plugin: "' + plugin + '" ... continuing')
            }
        }
    }

    function castDirPaths() {
        let dir = me.dir
        for (let [key,value] in dir) {
            dir[key] = Path(value)
        }
    }

    function castArray(a) {
        if (a && !(a is Array)) {
            return [a]
        }
        return a
    }

    /*
        Cast all array elements to be Paths. This modifies the array in-situ. Returns the array for chaining.
     */
    function castArrayOfPaths(array) {
        if (array) {
            if (!(array is Array)) {
                array = [Path(array)]
            }
            array = array.transform(function(e) Path(e))
        }
        return array
    }

    function castPath(path) {
        if (path) {
            path = Path(path)
        }
        return path
    }

    function castTargetProperties(target: Target) {
        target.home      = castPath(target.home)
        target.relative  = castPath(target.relative)
        target.includes  = castArrayOfPaths(target.includes)
        target.libpaths  = castArrayOfPaths(target.libpaths)
        target.headers   = castArrayOfPaths(target.headers)
        target.files     = castArrayOfPaths(target.files)
        target.resources = castArrayOfPaths(target.resources)
        target.sources   = castArrayOfPaths(target.sources)
        target.mkdir     = castArrayOfPaths(target.mkdir)

        target.ifdef = castArray(target.ifdef)
        target.uses = castArray(target.uses)
        target.depends = castArray(target.depends)
    }

    /*
        Create a target and add to me.targets
     */
    public function createTarget(properties): Target {
        let target = Target()
        try {
            blend(target, mapTargetProperties(properties), {functions: true, overwrite: true})
        } catch (e) {
            dump(properties)
            throw e
        }
        if (!target.home) {
            target.home = me.dir.top
        }
        castTargetProperties(target)
        if (!target.type) {
            target.type = 'component'
        }
        if (me.targets[target.name]) {
            target = global.blend(me.targets[target.name], target, {functions: true, overwrite: true})
        } else {
            me.targets[target.name] = target
        }
        rebaseTarget(target)
        return target
    }

    /*
        Sleuth the O/S distribution details
     */
    function distro(os) {
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
        Expand tokens in a string.
        Tokens are represented by '${field}' where field may contain '.'. For example ${user.name}.
        To preserve an ${token} unmodified, preceed the token with an extra '$'. For example: $${token}.
        Calls String.expand to expand variables from the me and me.globals objects.
        @param str Input string
        @param options Control options object
        @option missing Set to a string to use for missing properties. Set to undefined or omit options to
        throw an exception for missing properties. Set missing to true to preserve undefined tokens as-is.
        This permits multi-pass expansions.
        @option join Character to use to join array elements. Defaults to space.
        @return Expanded string
     */
    public function expand(str: String, options = {missing: true}) : String {
        /*
            Do twice to allow tokens to use ${vars}
            Last time use real options to handle missing tokens as requested.
         */
        let eo = {missing: true}
        if (global.me) {
            str = str.expand(me.globals, eo)
            str = str.expand(me, eo)
            str = str.expand(me, eo)
            str = str.expand(me.globals, options)
        }
        return str
    }

    /*
        Expand tokens in all fields in an object hash. This is used to expand tokens in me file objects.
     */
    function expandTokens(o) {
        let x
        if (me.targets.removeFiles) {
            if (me.targets.removeFiles.enable is String) x = 1
        }
        for (let [key,value] in o) {
            if (value is String) {
                let newValue
                o[key] = newValue = expand(value)
            } else if (value is Path) {
                o[key] = Path(expand(value))
            } else if (Object.getOwnPropertyCount(value) > 0) {
                o[key] = expandTokens(value)
            }
        }
        return o
    }

    /*
        Fix legacy properties and prepend combine prefixes to properties that must be aggregated
     */
    function fixupProperties(o) {
        plus(o.defaults, 'includes')
        plus(o.internal, 'includes')

        o.settings ||= {}
        o.configure ||= {}
        let settings = o.settings
        let configure = o.configure
        let origin = o.origin

        //  LEGACY
        if (o.extensions) {
            trace('Warn', origin + ' uses "extensions" which is deprecated, use "configure" instead')
        }
        if (settings.extensions) {
            trace('Warn', origin + ' uses "settings.extensions" which is deprecated, use "configure" instead')
        }
        if (o.extensions && o.extensions.generates) {
            trace('Warn', origin + ' uses "extensions.generates" which is deprecated. Use configure.extras instead')
        }
        if (settings.discover) {
            trace('Warn', origin + ' uses "settings.discover" which is deprecated. Use "configure.discovers" instead')
        }
        if (o.scripts) {
            /* Note: these are top level scripts, not target scripts */
            if (o.scripts.preblend) {
                trace('Warn', origin + ' uses "preblend" which is deprecated"')
            }
            if (o.scripts.postblend) {
                trace('Warn', origin + ' uses "postblend" which is deprecated"')
            }
            if (o.scripts.preload) {
                trace('Warn', origin + ' uses "preload" which is deprecated.')
            }
            if (o.scripts.postload) {
                trace('Warn', origin + ' uses "postload" which is deprecated.')
            }
            if (o.scripts.postloadall) {
                trace('Warn', origin + ' uses "postloadall" which is deprecated. Use "loaded" instead"')
                o.scripts.loaded = o.scripts.postloadall
            }
            for (key in o.scripts) {
                plus(o.scripts, key)
            }
        }
        plus(configure, 'requires')
        plus(configure, 'discovers')
        plus(configure, 'extras')

        let home = o.origin.dirname
        rebasePaths(home, o, 'modules')
        rebasePaths(home, o.defaults, 'includes')
        rebasePaths(home, o.internal, 'includes')

        fixScripts(o)
        fixScripts(o.defaults)
        fixScripts(o.internal)
    }

    /*
        Search for a plugin. Search locally first then in the pak cache.
            [lib|src]/name
            paks/name
            paks/name/dist
            ~/.paks/name
            ~/.paks/name
            /usr/local/lib/me/latest/bin/paks/name
     */
    public function findPlugin(name, exceptions = true): Path? {
        let path: Path?
        let optional = false
        if (name.startsWith('?')) {
            name = name.slice(1)
            optional = true
        }
        name = expand(name, {missing: '.'})
        let base = Path(name.trimStart('me-')).joinExt('me')

        if (!name.startsWith('me-')) {
            path = findPlugin('me-' + name, undefined)
        }
        if (!path || !path.exists) {
            path = me.dir.lib.join(name, base)
        }
        if (!path || !path.exists) {
            path = me.dir.paks.join(name, base)
        }
        if (!path || !path.exists) {
            path = me.dir.paks.join(name, 'dist', base)
        }
        if (!path || !path.exists) {
            let home = App.home
            if (home) {
                let pakcache = App.home.join('.paks')
                path = pakcache.join(name, 'embedthis')
                if (!path.exists) {
                    //  TODO - must check all owner directories
                    path = pakcache.join(name)
                }
                //  TODO check dist directory more consistently
                if (!path.exists) {
                    path = pakcache.join(name, 'dist')
                }
                if (path.exists) {
                    let files = path.files('*')
                    if (files.length > 0) {
                        path = Path(Version.sort(files, -1)[0]).join(base)
                    }
                }
            }
        }
        if (!path || !path.exists) {
            path = me.dir.me.join('paks', name, base)
        }
        if (!path || !path.exists) {
            if (!optional) {
                if (exceptions === true) {
                    throw new Error('Cannot find plugin: "' + name + '"')
                }
            }
            path = null
        }
        return path
    }

    /*
        Convert scripts collection into canonical long form
     */
    public function fixScripts(o, topnames) {
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
            let home = o.origin
            for (let [event, item] in o.scripts) {
                if (item is String || item is Function) {
                    o.scripts[event] = [{ home: home, interpreter: 'ejs', script: item }]
                } else if (item is Array) {
                    for (let [key, value] in item) {
                        if ((value is String) || (value is Function)) {
                            item[key] = { home: home, interpreter: 'ejs', script: value }
                        } else if (value is Function) {
                            item[key] = { home: home, interpreter: 'fun', script: value }
                        } else {
                            value.home ||= home
                        }
                    }
                }
            }
            for (let [event, scripts] in o.scripts) {
                for each (item in scripts) {
                    if (item.script is String) {
                        makeItemGlobals(o.origin, item.home)
                        item.script = expand(item.script)
                    }
                }
            }
        }
    }

    public function checkVersion(path, obj) {
        let criteria = (obj.devDependencies) ? obj.devDependencies.makeme : obj.makeme
        if (criteria && !Version(Config.Version).acceptable(criteria)) {
            throw path + ' requires MakeMe ' + criteria + '. MakeMe version ' + Config.Version +
                            ' is not compatible with this requirement.' + '\n'
        }
    }

    public function canExecute(platform) {
        let [os, arch, profile] = platform.split('-')
        if (os == Config.OS) {
            if (arch == Config.CPU) {
                return true
            }
            if ((os == 'windows' || os == 'linux' || os == 'macosx') &&
                (arch == 'x86' && Config.CPU == 'x64')) {
                return true
            }
        }
        return false
    }

    public function getPlatformFiles(path): Array {
        let files = []
        if (path.exists) {
            global.load(expand(path, {missing: '.'}))
            checkVersion(path, loadObj);
            platforms = loadObj.platforms
            if (platforms) {
                if (platforms.length == 1) {
                    if (canExecute(platforms[0])) {
                        localPlatform = platforms[0]
                    }
                }
                for each (platform in platforms) {
                    let local = null
                    let [os, arch, profile ] = localPlatform.split('-')
                    if (platform == 'local') {
                        local = localPlatform
                    } else if (platform == 'local-debug') {
                        local = os + '-' + arch + '-debug'
                    } else if (platform == 'local-release') {
                        local = os + '-' + arch + '-release'
                    }
                    if (local) {
                        if (options.nolocal) {
                            continue
                        }
                        platform = local
                    } else if (options.nocross) {
                        continue
                    }
                    files.push(BUILD.join(platform, PLATFORM))
                }
            } else {
                files.push(path)
            }
        }
        return files
    }

    /*
        Inherit compilation properties from a dependent target (libraries, defines, compiler settings)
        Also used to inhert the compiler default settings
     */
    public function inheritCompSettings(target, dep, inheritCompiler = false) {
        if (!dep) {
            return
        }
        target.defines ||= []
        target.compiler ||= []
        target.includes ||= []
        target.libpaths ||= []
        target.libraries ||= []
        target.linker ||= []

        if (inheritCompiler) {
            for each (option in dep.compiler) {
                if (!target.compiler.contains(option)) {
                    target.compiler.push(option)
                }
            }
        }
        for each (option in dep.defines) {
            if (!target.defines.contains(option)) {
                target.defines.push(option)
            }
        }
        for each (option in dep.includes) {
            if (!target.includes.contains(option)) {
                target.includes.push(option)
            }
        }
        for each (option in dep.libpaths) {
            if (!target.libpaths.contains(option)) {
                target.libpaths.push(option)
            }
        }
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
    }

    /*
        Define the platform. The platform spec is either a platform string or a platform object.
        Also defines directory references for the platform.
     */
    public function initPlatform(spec) {
        let name
        if (spec == null) {
            name = spec = localPlatform
        } else if (spec is String) {
            name = spec
        } else if (spec.platform) {
            name = spec.platform.name
        } else {
            name = localPlatform
        }
        let [os, arch, profile] = name.split('-')
        let [arch, cpu] = (arch || '').split(':')
        let cross = ((os + '-' + arch) != (Config.OS + '-' + Config.CPU))
        let platform = me.platform = {
            name: name,
            os: os,
            arch: arch,
            like: like(os),
            dist: distro(os),
            profile: profile,
            dev: localPlatform,
            cross: cross,
        }
        if (cpu) {
            platform.cpu = cpu
        }
        setDirectories(spec.dir)
    }

    public function like(os) {
        if (Unix.contains(os)) {
            return 'unix'
        } else if (Windows.contains(os)) {
            return 'windows'
        }
        return ''
    }

    /*
        Load a top-level MakeMe file: start.me or main.me.
        This will load all referenced MakeMe files in this order:
            1. O/S file
            2. standard/simple file
            3. Package.json
            4. Plugins
            5. Blend[] files - these load before the invoking file
            6. File itself
            7. Any 'main' file
     */
    public function loadFile(path) {
        let obj = readFile(path)
        checkVersion(path, obj)
        obj.master = true
        if (!me.platform || !me.platform.name) {
            initPlatform(obj)
        }
        /*
            Load me-os/
         */
        blendFile(findPlugin('os'))
        blendFile(me.dir.me.join(!me.dir.bin.same(me.dir.out) ? 'standard.me' : 'simple.me'))
        setExtensions()
        loadPackage(path)
        blendObj(obj)
        if (obj.main) {
            /* Must preserve the original settings and blend over main */
            let settings = me.settings.clone(true)
            blendFile(obj.main)
            blend(me.settings, settings)
        }
        makeGlobals()

        /*
            Loaded, now process
         */
        expandTokens(me)
        setPrefixes()
        applyPlatformProfile()
        applyCommandLine()
        applySettings()
        resolveDirectories()
        runScriptOnce('loaded')
    }

    public static function loading(obj)
        loadObj = obj

    /*
        Load the Package.json and extract product description and definition
     */
    function loadPackage(path) {
        let pfile = path.dirname.join(PACKAGE)
        let settings = me.settings
        let dir = me.dir
        if (pfile.exists) {
            let package
            try {
                package = pfile.readJSON()
            } catch (e) {
                trace('Warn', 'Cannot parse: ' + pfile + '\n' + e)
            }
            try {
                settings.name = package.name
                settings.description = package.description
                settings.title = package.title || package.description
                settings.version = package.version
                settings.author = package.author ? package.author.name : package.name
                settings.company = package.company
                if (package.directories && package.directories.paks) {
                    dir.paks = Path(package.directories.paks)
                }
            } catch {}
        }
        settings.name ||= options.name || ''
        settings.author ||= ''
        settings.company ||= settings.author.split(' ')[0].toLowerCase()
        if (dir.paks && !dir.paks.exists) {
            if (Path('src/paks').exists) {
                dir.paks = Path('src/paks')
            }
        }
        if (settings.version) {
            let ver = settings.version.split('-')[0]
            let majmin = ver.split('.').slice(0,2).join('.')
            settings.compatible ||= majmin
        } else {
            settings.version = '1.0.0'
            settings.compatible = '1.0'
        }
    }

    public function makeDirectoryGlobals(base: Path? = null) {
        let tokens
        for each (n in ['BIN', 'BLD', 'OUT', 'INC', 'LIB', 'OBJ', 'PAKS', 'PKG', 'REL', 'SRC', 'TOP', 'LBIN']) {
            /*
                In portable format so they can be used in build scripts. Windows back-slashes require quoting!
             */
            let path = me.dir[n.toLower()]
            if (!path) continue
            path = Path(path).portable
            if (base) {
                path = path.relativeTo(base)
            }
            global[n] = me.globals[n] = path
            if (path.contains('${') /*}*/) tokens = true
        }
        me.globals.ME = me.dir.me
        return !tokens
    }

    /*
        Make globals used when doing string expansion
        Also called by Xcode
     */
    public function makeGlobals(base: Path? = null) {
        let platform = me.platform
        let g = me.globals
        g.PLATFORM = platform.name
        g.OS = platform.os
        g.CPU = platform.cpu || 'generic'
        g.ARCH = platform.arch
        g.PROFILE = platform.profile
        /* Apple gcc only */
        if (platform['arch-map']) {
            g.CC_ARCH = platform['arch-map'][platform.arch] || platform.arch
        }
        g.CONFIG = platform.name

        if (me.settings.hasMtune && platform.cpu) {
            g.MTUNE = '-mtune=' + platform.cpu
        }
        if (base) {
            /* Called from Xcode */
            makeExtensionGlobals()
            makeDirectoryGlobals(base)
        }
    }

    public function makeExtensionGlobals() {
        let g = me.globals
        let ext = me.ext
        g.LIKE = me.platform.like
        g.EXE = ext.dotexe
        g.O = ext.doto
        g.SHOBJ = ext.dotshobj
        g.SHLIB = ext.dotshlib
        g.ARLIB = ext.dotlib
    }

    function makeItemGlobals(origin: Path?, home: Path?) {
        if (home) {
            me.globals.HOME = Path(home).portable
        }
        if (origin) {
            me.globals.ORIGIN = Path(origin).portable
        }
    }

    function mapTargetProperties(p) {
        for (let [key,value] in p) {
            if (value === null) {
                delete p[key]
            }
        }
        let name = p.name
        if (p.dir) {
            trace('Warn', 'Target "' + name + '" is using "dir" which is deprecated. Use "mkdir" instead')
            p.mkdir = p.dir
            delete p.dir
        }
        if (p.nogen) {
            trace('Warn', 'Target "' + name + '" is using "nogen" which is deprecated. Use "generate" instead')
            p.generate = false
        }
        if (p.subtree) {
            trace('Warn', 'Target "' + name + '" is using "subtree" which is deprecated. Use "relative" instead')
            p.relative = p.subtree
            delete p.subtree
        }
        if (p.expand) {
            trace('Warn', 'Target "' + name + '" using "expand" option in copy, use "patch" instead')
            p.patch = true
            delete p.expand
        }
        if (p.cat) {
            trace('Warn', 'Target "' + name + '" using "cat" option in copy, use "append" instead')
            p.append = p.cat
            delete p.cat
        }
        if (p.title) {
            trace('Warn', 'Target "' + name + '" using "title" option, use "header" instead')
            p.header ||= ''
            p.header = p.title + '\n' + p.header
            delete p.title
        }
        if (p.linkin) {
            trace('Warn', 'Target "' + name + '" using "linkin" option, use "symlink" instead')
            p.symlink = p.linkin
            delete p.linkin
        }
        if (p.message is Array) {
            p.message = p.message.join(':')
        }
        /*
            Map from/to to files/path and set type to 'file'
         */
        if (p.from || p.to) {
            p.type ||= 'file'
            if (p.type == 'file') {
                if (p.to && !p.path) {
                    p.path = p.to
                    delete p.to
                }
                if (p.from && !p.files) {
                    p.files = p.from
                    delete p.from
                }
            }
        }
        setTargetGoals(p)

        /*
            Expand short-form scripts into the long-form. Set the target type if not defined to 'script'.
         */
        if (p.run) {
            let run = p.run
            p.run = null
            p.type ||= 'run'
            if (run is Array) {
                run = run.map(function(a) '"' + a + '"').join(' ')
            }
            run = run.replace(/`/g, '\\`')
            p.action = 'run(`' + run + '`)'
            p.message ||= 'Run: ' + run
        }
        /* These are target events */
        for each (n in ['action', 'build', 'shell', 'preblend', 'postblend', 'prebuild', 'postbuild',
                'precompile', 'postcompile', 'preresolve', 'postresolve', 'presource', 'postsource', 'test']) {
            if (p[n] != undefined) {
                p.type ||= 'script'
                let script = p[n]
                let event = (n == 'action' || n == 'shell') ? 'build' : n
                p.scripts ||= {}
                p.scripts[event] ||= []
                p.scripts[event]  += [{
                    home: p.home,
                    interpreter: (n == 'shell') ? 'bash' : 'ejs',
                    script: script
                }]
                delete p[n]
            }
        }
        /*
            Top level target scripts. Used in components for: config, without and postconfig functions ONLY.
         */
        fixScripts(p, ['config', 'postconfig', 'without'])

        if (p.libraries) {
            /* Own libraries are the libraries defined by a target, but not inherited from dependents */
            p.ownLibraries = p.libraries.clone()
        }
        if (p.type == 'lib') {
            p.ownLibraries ||= []
            p.ownLibraries += [p.name.replace(/^lib/, '')]
        }
        for (let [key,value] in p.defines) {
            p.defines[key] = value.trimStart('-D')
        }
        p.static ||= me.settings.static

        let base = {}
        if (p.type == 'exe' || p.type == 'lib' || p.type == 'obj') {
            inheritCompSettings(base, me.targets.compiler, true)
        }
        /*
            Inherit defaults
         */
        if (Object.getOwnPropertyCount(me.defaults)) {
            for (let [key,value] in me.defaults) {
                if (!key.startsWith('+')) {
                    me.defaults['+' + key] = me.defaults[key]
                    delete me.defaults[key]
                }
            }
            base = blend(base, me.defaults, {combine: true})
        }
        /*
            Inherit internal (file local) properties
         */
        if (p.internal) {
            base = blend(base, p.internal, {combine: true})
        }
        /*
            Inherit specific collections for this target
         */
        if (p.inherit) {
            if (!(p.inherit is Array)) {
                p.inherit = [ p.inherit ]
            }
            for each (from in p.inherit) {
                blend(base, me[from], {combine: true})
            }
        }
        /*  Blend the properties over the base */
        return blend(base, p, {combine: true, functions: true})
    }

    function plus(o, field) {
        if (o && o[field]) {
            o['+' + field] = o[field]
            delete o[field]
        }
    }

    /*
        Read a MakeMe file. This reads the file and returns a reference to the MakeMe definition object.
        The file is not processed or blended.
     */
    public function readFile(path: Path): Object {
        path = expand(path, {missing: '.'})
        if (!path.exists) {
            throw new Error('Cannot open ' + path)
        }
        let result
        try {
            vtrace('Load', path.compact())
            global.load(path)
            result = loadObj
            result.origin = path.dirname
        }
        return result
    }

    /*
        Rebase paths to the specified home directory
     */
    public function rebasePaths(home: Path, o: Object, field: String) {
        if (!o) return
        if (!o[field]) {
            field = '+' + field
            if (!o[field]) {
                return
            }
        }
        if (o[field] is Array) {
            for (let [key,value] in o[field]) {
                if ((!value.startsWith('${') && !value.startsWith('$(')) || value.startsWith('${OS}')) {
                    let exclude = ''
                    if (value.startsWith('!')) {
                        exclude = '!'
                        value = value.name.slice(1)
                    }
                    let path = Path(exclude + home.join(value))
                    if (value.endsWith('/')) {
                        o[field][key] = Path(path + '/')
                    } else {
                        o[field][key] = path
                    }
                }
                /* Comment to balance } */
            }
        } else if (o[field] && o[field].startsWith) {
            if (!o[field].startsWith('${') && !o[field].startsWith('$(')) {
                if (o[field].endsWith('/')) {
                    o[field] = Path(home.join(o[field]) + '/')
                } else {
                    o[field] = home.join(o[field])
                }
            }
            /* Comment to balance } */
        }
    }

    public function rebaseTarget(target: Target) {
        target.home = Path(expand(target.home || me.dir.top))
        setTargetPath(target)

        let home = target.home
        rebasePaths(home.relative, target, 'includes')
        rebasePaths(home.relative, target, 'headers')
        rebasePaths(home.relative, target, 'resources')
        rebasePaths(home.relative, target, 'sources')
        rebasePaths(home.relative, target, 'files')

        rebasePaths(home, target, 'relative')

        for (let [when, item] in target.scripts) {
            for each (script in item) {
                if (script.home) {
                    script.home = Path(expand(script.home))
                }
            }
        }
        target.files ||= []
        if (target.type == 'exe' || target.type == 'lib') {
            target.defines ||= []
            target.compiler ||= []
            target.includes ||= []
            target.libraries ||= []
            target.linker ||= []
            target.libpaths ||= []
        }
        Object.sortProperties(target)
    }

    public function reset() {
        loaded = {}
    }

    /*
        Resolve directory properties to be absolute Paths so they can apply anywhere in the source tree.
        Scripts change directory so it is essential that all script-accessible properties be absolute.
     */
    function resolveDirectories() {
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
        let defaults = me.defaults
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

    public function runScriptOnce(event) {
        if (me.scripts && me.scripts[event]) {
            runScriptFromObj(me.scripts, event)
            delete me.scripts[event]
        }
    }

    public function runScript(event)
        runScriptFromObj(me.scripts, event)

    public function runScriptFromObj(obj, event) {
        if (!obj) {
            return
        }
        for each (item in obj[event]) {
            let pwd = App.dir
            if (item.home && item.home != pwd) {
                App.chdir(expand(item.home))
            }
            try {
                if (item.script is Function) {
                    item.script(event)
                } else {
                    eval('require ejs.unix\nrequire embedthis.me.script\n' + expand(item.script, {missing: ''}))
                }

            } finally {
                App.chdir(pwd)
            }
        }
    }

    public function samePlatform(p1, p2): Boolean {
        if (!p1 || !p2) return false
        let [os1, arch1] = p1.split('-')
        let [os2, arch2] = p2.split('-')
        return os1 == os2 && arch1 == arch2
    }

    /*
        Only called from initPlatform
     */
    function setDirectories(directories) {
        let dir = me.dir
        if (directories) {
            dir = blend(dir, directories)
        }
        castDirPaths()
        if (options.configure) {
            /* Before configure - set default directories */
            dir.bld  ||= dir.work.join(Loader.BUILD)
            dir.out  ||= dir.bld.join(me.platform.name)
            dir.bin  ||= dir.out.join('bin')
            dir.inc  ||= dir.out.join('inc')
            dir.lbin ||= (options.gen || (me.platform.os == Config.OS && me.platform.arch == Config.CPU)) ?
                dir.bin : dir.bld.join(localPlatform, 'bin')
            if (!dir.top.join('lib').exists && dir.top.join('src').exists) {
                dir.lib  ||= dir.top.join('src')
            } else {
                dir.lib  ||= dir.top.join('lib')
            }
            dir.obj  ||= dir.out.join('obj')
            dir.src  ||= dir.top.join('src')
            //  DEPRECATE
            if (dir.top.join('src/paks').exists) {
                dir.paks ||= dir.top.join('src/paks')
            } else {
                dir.paks ||= dir.top.join('paks')
            }
            dir.proj ||= dir.top.join('projects')
            dir.pkg  ||= dir.out.join('pkg')
            dir.rel  ||= dir.out.join('img')

        } else {
            dir.bld  ||= Path('.')
            dir.out  ||= dir.bld
            dir.bin  ||= dir.out
            dir.lbin ||= dir.bin
            dir.lib  ||= dir.top.join('lib')
            dir.inc  ||= dir.out
            dir.obj  ||= dir.out
            dir.src  ||= dir.top
            dir.paks ||= dir.top.join('paks')
            dir.proj ||= dir.out
            dir.pkg  ||= dir.out.join('pkg')
            dir.rel  ||= dir.out.join('img')
        }
        dir.me = App.exeDir
        if (me.platform.like == 'windows') {
            dir.programFiles32 = makeme.programFiles32()
            dir.programFiles = Path(dir.programFiles32.name.replace(' (x86)', ''))
        }
        for (let [key,value] in dir) {
            dir[key] = Path(value.toString().expand(this)).absolute
        }
        makeDirectoryGlobals()
    }

    function setExtensions() {
        let ext = me.ext
        for (let [key,value] in ext.clone()) {
            if (value) {
                ext['dot' + key] = '.' + value
            } else {
                ext['dot' + key] = value
            }
        }
        makeExtensionGlobals()
    }

    public function setPrefixes() {
        let options = me.options
        let prefixes = me.prefixes
        let settings = me.settings
        if (options.prefixes) {
            let pset = options.prefixes + '-prefixes'
            if (!me[pset]) {
                throw 'Cannot find prefix set for ' + pset
            }
            settings.prefixes = pset
            global.blend(prefixes, me[pset])
        } else {
            if (!prefixes || Object.getOwnPropertyCount(prefixes) == 0) {
                me.prefixes = {}
                settings.prefixes ||= 'debian-prefixes'
                global.blend(me.prefixes, me[settings.prefixes])
                prefixes = me.prefixes
            }
        }
        if (options.prefix) {
            for each (p in options.prefix) {
                let [prefix, path] = p.split('=')
                let prior = prefixes[prefix]
                if (path) {
                    prefixes[prefix] = Path(path)
                } else {
                    /* Map --prefix=/opt to --prefix base=/opt */
                    prefixes.base = Path(prefix)
                }
                if (prefix == 'root') {
                    for (let [key,value] in prefixes) {
                        if (key != 'root' && value.startsWith(prior)) {
                            prefixes[key] = Path(value.replace(prior, path + '/')).normalize
                        }
                    }
                }
            }
        }
    }

    function setTargetGoals(target) {
        let goals = target.goals || []
        let type = target.type
        if (goals.length == 0) {
            if ((Builder.TargetsToBuildByDefault[type] || target.build) && !target.action) {
                goals = [Builder.ALL]
                if (target.generate !== false) {
                    target.generate ||= true
                    goals.push('gen')
                }
            } else {
                goals = []
            }
        }
        for (field in target) {
            if (field.startsWith('generate-')) {
                if (target.generate !== false) {
                    target.generate ||= true
                }
            }
        }
        if (target.generate && !goals.contains('gen')) {
            goals.push('gen')
        }
        if (type && type != 'script' && !goals.contains(type)) {
            goals.push(type)
        }
        if (!goals.contains(target.name)) {
            goals.push(target.name.toString())
        }
        target.goals ||= goals
    }

    function setTargetPath(target) {
        let name = target.name
        if (target.path) {
            if (!(target.path is Function)) {
                target.path = Path(expand(target.path))
                if (target.path.isRelative && !target.configurable) {
                    target.path = target.home.join(target.path).absolute
                }
            }
        } else {
            let type = target.type
            if (type == 'lib') {
                //  TODO - remove libname here - not used anymore
                name = target.libname || target.name
                if (target.static) {
                    target.path = me.dir.bin.join(name).joinExt(me.ext.lib, true)
                } else {
                    target.path = me.dir.bin.join(name).joinExt(me.ext.shobj, true)
                }
            } else if (type == 'obj') {
                target.path = me.dir.obj.join(name).joinExt(me.ext.o, true)
            } else if (type == 'exe') {
                target.path = me.dir.bin.join(name).joinExt(me.ext.exe, true)
            } else if (type == 'file') {
                target.path = me.dir.bin.join(name)
            } else if (type == 'res') {
                target.path = me.dir.res.join(name).joinExt(me.ext.res, true)
            } else if (type == 'header') {
                target.path = me.dir.inc.join(name)
            }
        }
        if (target.path && target.type == 'file' && (target.path.isDir || target.path.name.endsWith('/'))) {
            target.modify = target.path.dirname.join('.' + Path(name).basename + '-modified')
        }
    }

} /* class Loader */

} /* module embedthis.me */

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
