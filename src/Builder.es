/*
    Builder.es -- Embedthis MakeMe Builder

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

require ejs.unix
require ejs.zlib
require ejs.version
require embedthis.me.script

/**
    The Builder Class implements the core building functions for MakeMe.
    @stability Prototype
  */
public class Builder {

    /** Default goal to build all. Set to 'all' */
    public static const ALL = 'all'

    /** List of targets to build by default. Defaults to 'exe', 'file', 'lib' and 'header' */
    public static const TargetsToBuildByDefault = { exe: true, file: true, lib: true, header: true }

    /** List of targets to clean by default. Defaults to 'exe', 'file', 'lib' and 'obj' */
    public static const TargetsToClean = { exe: true, file: true, lib: true, obj: true }

    /** Current goal */
    public var goal: String

    /** Top-level targets to build */
    public var topTargets: Array

    private var expandMissing = undefined
    private var gates: Object = {}
    private var loader: Loader
    private var options: Object
    private var selectedTargets: Array

    /** Builder constructor
        @hide
    */
    function Builder() {
        this.loader = makeme.loader
        options = makeme.options
    }

    function admit(target, prefix: String) {
        let gate = gates[prefix]
        if (gate[target.name]) {
            return false
        }
        return gate[target.name] = true
    }

    function admitLeave(target, prefix: String) {
        let gate = gates[prefix]
        gate[target.name] = false
    }

    function admitSetup(...prefixes) {
        for each (prefix in prefixes) {
            gates[prefix] = {}
        }
    }

    /**
        Build all required targets for the requested goals
     */
    public function build(goals: Array = []) {
        if (goals.length == 0) {
            goals.push(Builder.ALL)
        }
        if (goals != 'configure') {
            makeDirs()
        }
        let save = gates['build']
        for each (goal in goals) {
            vtrace('Build', goal)
            admitSetup('build')
            this.goal = goal
            for each (target in selectTargets(goal)) {
                buildTarget(target)
            }
        }
        gates['build'] = save
    }

    function buildFileList(target, patterns) {
        let options = blend({directories: false, expand: loader.expand, missing: expandMissing}, target)
        /*
            Files must be absolute at this stage because scripts may change directory before executing.
            BuildFile applies the relative property
         */
        delete options.relative
        if (!(patterns is Array)) {
            patterns = [patterns]
        }
        let list = me.dir.top.files(patterns, options)
        if (list.length == 0 && makeme.generating) {
            list = patterns
        }
        return list
    }

    /*
        Build a target and all required dependencies
     */
    function buildTarget(target) {
        if (!admit(target, 'build')) {
            App.log.error('Possible recursive dependancy: target ' + target.name + ' is already building')
            return
        }
        global.TARGET = me.target = target

        if (target.files) {
            global.FILES = target.files.map(function(f) f.relativeTo(target.home).portable).join(' ')
        } else {
            global.FILES = ''
        }
        me.globals['FILES'] = global.FILES

        try {
            if (!stale(target)) {
                whySkip(target.name, 'is up to date')
            } else {
                if (target.message && !makeme.generating) {
                    if (target.message.contains(':')) {
                        let [,tag, msg] = target.message.match(/([^:]*) *: *(.*)/)
                        trace(tag || 'Info', loader.expand(msg))
                    } else {
                        trace('Info', loader.expand(target.message))
                    }
                }
                if (options.diagnose) {
                    App.log.debug(3, "Target => " +
                        serialize(target, {pretty: true, commas: true, indent: 4, quotes: false}))
                }
                runTargetScript(target, 'prebuild')

                if (makeme.generating) {
                    if (makeme.generate.generator.target) {
                        makeme.generate.generator.target(target)
                    }
                } else {
                    if (target.mkdir) {
                        buildDirs(target)
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
                    }
                }
                runTargetScript(target, 'postbuild')
            }
        } catch (e) {
            throw 'Building target ' + target.name + '\n' + e
        }
        global.TARGET = me.target = null
    }

    function buildDirs(target) {
        for each (dir in target.mkdir) {
            makeDirectory(loader.expand(dir), target)
        }
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
        run(command, {filter: /Creating library /})
    }

    /*
        Copy files[] to path. Used for headers too.
     */
    function buildFile(target) {
        /*
            The target.files list is absolute. Convert to be relative to the target.home so the
            various Path.operate properties will be interpreted locally.
        */
        let files = target.files.map(function(e) e.relativeTo(target.home))
        if (files.length) {
            let tag = target.append ? 'Append' : 'Copy'
            if (target.modify) {
                trace(tag, target.name + ' => ' + target.path.compact())
            } else {
                trace(tag, target.path.compact())
            }
            target.path.dirname.makeDir()
            target.verbose = true
            copyFiles(files, target.path, target)
            if (target.modify) {
                target.modify.remove()
                target.modify.write()
            }
        }
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
                run(command, {filter: /^[a-zA-Z0-9-]*.c\s*$/})
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

    function buildScript(target) {
        setRuleVars(target, target.home)
        if (target.scripts) {
            vtrace(target.type.toPascal(), target.name)
            runTargetScript(target, 'build')
        }
        if (target.path && target.path.isDir && !makeme.generating) {
            touchDir(target.path)
        }
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
        run(command, {filter: /Creating library /})
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
        run(command, {filter: /has no symbols|Creating library /})
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

    function enableTarget(target: Target) {
        let reported = false
        for each (item in target.ifdef) {
            if (!me.targets[item] || !me.targets[item].enable) {
                let configure = me.configure
                if (!(configure.extras && configure.extras.contains(item) && me.options.configurableProject)) {
                    target.why = 'disabled because the required target ' + item + ' is not enabled'
                    target.enable = false
                    reported = true
                }
            }
        }
        global.TARGET = me.target = target
        if (target.enable == undefined) {
            target.enable = true
        } else if (target.enable is Function) {
            target.enable = target.enable(this)

        } else if (!(target.enable is Boolean)) {
            let script = loader.expand(target.enable)
            try {
                if (!eval(script)) {
                    target.why = 'disabled on this platform'
                    target.enable = false
                } else {
                    target.enable = true
                }
            } catch (e) {
                vtrace('Enable', 'Cannot run enable script for ' + target.name)
                App.log.debug(3, e)
                target.enable = false
            }

        } else if (!target.enable) {
            if (!reported) {
                target.why = 'disabled'
            }
        }
        if (target.platforms && !options.gen) {
            let pname = me.platform.name
            if (!target.platforms.contains(pname) &&
                !(loader.samePlatform(pname, loader.localPlatform) && target.platforms.contains('local')) &&
                !(!loader.samePlatform(pname, loader.localPlatform) && target.platforms.contains('cross'))) {
                    target.enable = false
            }
        }
    }

    function enableTargets() {
        for each (target in me.targets) {
            enableTarget(target)
        }
    }

    /**
        @hide
     */
    public function expandRule(target, rule) {
        setRuleVars(target)
        return loader.expand(rule).expand(target.vars, {missing: ''})
    }

    /*
        Expand resources, sources and headers. Support include+exclude and create target.files[]
     */
    function expandWildcards() {
        let target
        admitSetup('depend')
        for each (target in me.targets) {
            if (!target.enable && !(target.ifdef && makeme.generating && options.configurableProject)) {
                continue
            }
            runTargetScript(target, 'presource')
            target.files = buildFileList(target, target.files)
            if (target.headers) {
                let files = buildFileList(target, target.headers)
                for each (file in files) {
                    let header = me.dir.inc.join(file.basename)
                    loader.createTarget({ name: header, enable: true, path: header, type: 'header', home: target.home,
                        goals: [target.name], files: [ file ], includes: target.includes, generate: true,
                        belongs: target.name })
                    target.depends.push(header)
                }
                if (target.type == 'header') {
                    target.generate = false
                }
            }
        }
        for each (target in me.targets) {
            if (!target.enable && !(target.ifdef && makeme.generating && options.configurableProject)) {
                continue
            }
            if (target.resources) {
                let files = buildFileList(target, target.resources)
                for each (file in files) {
                    let res = me.dir.obj.join(file.replaceExt(me.ext.res).basename)
                    loader.createTarget({ name: res, enable: true, path: res, enable: true, home: target.home,
                        type: 'resource', goals: [target.name], files: [ file ], includes: target.includes,
                        defines: target.defines, generate: true, belongs: target.name })
                    target.files.push(res)
                    target.depends.push(res)
                }
            }
            if (target.sources) {
                let files = buildFileList(target, target.sources)
                for each (file in files) {
                    /*
                        Create a target for each source file
                     */
                    let obj = me.dir.obj.join(file.replaceExt(me.ext.o).basename)
                    let props = { name : obj, enable: true, path: obj, type: 'obj', home: target.home,
                        goals: [target.name], files: [ file ],
                        generate: true, belongs: target.name }
                    for each (n in ['compiler', 'defines', 'includes', 'libraries', 'linker', 'libpaths']) {
                        if (target[n] && target[n].length > 0) {
                            props[n] = target[n]
                        }
                    }
                    let objTarget = loader.createTarget(props)
                    /*
                        Inherit pre-compile options from target
                     */
                    let precompile = (target.scripts && target.scripts.precompile) ? target.scripts.precompile : null
                    if (precompile) {
                        objTarget.scripts = {precompile: precompile}
                    }
                    target.files.push(obj)
                    target.depends.push(obj)

                    /*
                        Create targets for each header (if not already present)
                     */
                    makeSourceDepends(objTarget)
                }
            }
            runTargetScript(target, 'postsource')
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

    function makeDirs() {
        for (let [name, dir] in me.dir) {
            if (dir.startsWith(me.dir.bld)) {
                if (name == 'bin' || name == 'inc' || name == 'obj') {
                    dir.makeDir()
                }
            }
        }
    }

    /*
        Create an array of header dependencies for source files
     */
    function makeSourceDepends(target) {
        let includes: Array = []
        let files = target.files
        for each (path in files) {
            if (path.exists) {
                let str = path.readString()
                let more = str.match(/^#include.*"$/gm)
                if (more) {
                    includes += more
                }
            } else {
                vtrace('Warn', 'Cannot find', path, 'in target', target.name)
            }
        }
        let depends = [ ]
        /*
            Resolve includes
         */
        for each (item in includes) {
            let path
            let ifile = item.replace(/#include.*"(.*)"/, '$1')
            target.includes ||= []
            let search = target.includes + [target.home, me.dir.inc]
            for each (dir in search) {
                path = Path(dir).join(ifile)
                if (path.exists && !path.isDir) {
                    break
                }
                path = null
            }
            if (!path && options.why) {
                trace('Warn', 'Cannot resolve include: ' + ifile + ' for ' + target.name + ' search: ' + search)
            }
            if (!path) {
                path = me.dir.inc.join(ifile)
            }
            if (path && !depends.contains(path)) {
                depends.push(path)
            }
        }
        admit(target, 'depend')

        for each (header in depends) {
            if (!me.targets[header]) {
                /* Create a stub header target */
                loader.createTarget({ name: header, enable: true, path: Path(header), home: target.home,
                    type: 'header', goals: [target.name], includes: target.includes, generate: true, belongs: target.name })
            }
            let h = me.targets[header]
            if (admit(h, 'depend')) {
                makeSourceDepends(h)
            }
            if (false && h.depends && target.path.extension != 'h') {
                /* Copy up nested headers */
                depends = (depends + h.depends).unique()
            }
        }
        if (depends.length > 0) {
            target.depends = depends
        }
    }

    /**
        Map library paths to a base
        @param libpaths Array of library search paths
        @param base Base directory from which the paths should be resolved. Defaults to the current directory.
        @return Linker library search path options
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
        libs ||= []
        if (me.platform.os == 'windows') {
            libs = libs.clone()
            for (let [i,name] in libs) {
                let libname = Path('lib' + name).joinExt(me.ext.shlib, true)
                if (me.targets['lib' + name] || me.dir.bin.join(libname).exists) {
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
                let llib = me.dir.bin.join("lib" + libs[i]).joinExt(me.ext.shlib).relative
                if (llib.exists) {
                    libs[i] = llib
                } else {
                    libs[i] = '-l' + Path(libs[i]).trimExt().toString().replace(/^lib/, '')
                }
            }
        } else {
            let mapped = []
            for each (let lib:Path in libs) {
                if (lib.extension == me.ext.shlib || lib.extension == me.ext.shlib) {
                    lib = lib.trimExt()
                }
                mapped.push('-l' + lib.relative.toString().replace(/^lib/, ''))
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

    /**
        Prepare for a build. Must be called before calling $build.
        @hide
     */
    public function prepBuild() {
        if (!me.settings.configured && !options.configure) {
            /*
                Auto configure
             */
            load(me.dir.me.join('Configure.es'))
            Configure().autoConfigure()
            me.settings.configured = true
            for each (target in me.targets) {
                if (target.type == 'exe' || target.type == 'lib' || target.type == 'obj') {
                    loader.inheritCompSettings(target, me.targets.compiler, true)
                }
            }
        }
        setPathEnvVar()
        if (options.configuration) {
            showConfiguration()
        }
        /*
            When cross generating, certain wild cards can't be resolved.
            Setting missing to empty will cause missing glob patterns to be replaced with the pattern itself
         */
        if (options.gen || options.configure) {
            expandMissing = ''
        }
        if (options.gen == 'make' || options.gen == 'nmake') {
            /* Generated project is configurable via Make variables */
            options.configurableProject = true
        }
        enableTargets()
        resolveDependencies()
        expandWildcards()

        Object.sortProperties(me.targets)
        Object.sortProperties(me)

        if (options.dump) {
            let path = Path(me.platform.name + '.dmp')
            if (me.configure) {
                Object.sortProperties(me.configure)
            }
            if (me.targets) {
                Object.sortProperties(me.targets)
            }
            if (me.settings) {
                Object.sortProperties(me.settings)
            }
            for each (target in me.targets) {
                Object.sortProperties(target)
            }
            path.write(serialize(me, {pretty: true, commas: true, indent: 4, nulls: false, quotes: false}))
            trace('Dump', path)
        }
    }

    /**
        Watch for changes and rebuild as required.
        The sleep period is defined via --watch.
     */
    public function watch(start: Path, goals = []) {
        if (!start.exists) {
            throw 'Cannot find ' + start
        }
        if (options.configure || options.gen) {
            throw 'Cannot watch and configure or generate'
        }
        if (!(goals is Array)) {
            goals = [goals]
        }
        let files = loader.getPlatformFiles(start)
        if (files.length > 1) {
            throw 'Cannot watch multiple platforms'
        }
        Me()
        loader.loadFile(files[0])
        prepBuild()
        while (true) {
            vtrace('Check', 'for changes')
            try {
                build(goals)
            } catch (e) {
                print(e)
            }
            App.sleep(options.watch || 1000)
        }
    }

    /**
        Process a top level MakeMe file
        @hide
     */
    public function process(first: Path, goals = []) {
        if (!first.exists) {
            throw 'Cannot find ' + first
        }
        if (!(goals is Array)) {
            goals = [goals]
        }
        let pfiles = loader.getPlatformFiles(first)
        for each (path in pfiles) {
            if (!path.exists) {
                if (path != Loader.START) {
                    throw 'Cannot find ' + path + '.\nRun "me configure" to repair.'
                } else {
                    throw 'Cannot find ' + path
                }
            }
            Me()
            vtrace('Process', path)
            loader.reset()
            loader.loadFile(path)
            if (!options.configure && (pfiles.length > 1 || me.platform.cross)) {
                trace('Build', me.platform.name)
                vtrace('Targets', me.platform.name + ': ' + ((selectedTargets != '') ? selectedTargets: 'nothing to do'))
            }
            if (options.hasOwnProperty('get')) {
                eval('print(serialize(me.' + options.get + ', {pretty: true, quotes: false}))')
                break
            }
            prepBuild()
            build(goals)
            if (goals != 'version') {
                trace('Complete', goals)
            }
        }
    }

    /*
        Resolve a target by inheriting dependent libraries from dependent targets
     */
    function resolve(target) {
        if (!admit(target, 'resolve')) {
            return
        }
        runTargetScript(target, 'preresolve')
        for each (dname in (target.depends + target.uses)) {
            let dep = getDep(dname)
            if (dep) {
                if (!dep.enable) {
                    continue
                }
                resolve(dep)

                if (dep.type == 'lib') {
                    /*
                        Put dependent libraries first so system libraries are last (matters on linux)
                        Convert to a canonical form without a leading 'lib'.
                     */
                    let lpath
                    let libname = dep.path.basename.trimExt() || Path(dep.name)
                    if (libname.extension == me.ext.shlib || libname.extension == me.ext.shlib) {
                        libname = libname.trimExt()
                    }
                    if (dep.static) {
                        if (libname.startsWith('lib')) {
                            lpath = libname.replace(/^lib/, '')
                        } else {
                            lpath = libname
                        }
                    } else {
                        if (libname.startsWith('lib')) {
                            lpath = libname.replace(/^lib/, '')
                        } else {
                            lpath = libname
                        }
                    }
                    target.libraries ||= []
                    if (!target.libraries.contains(lpath)) {
                        target.libraries = [lpath] + target.libraries
                    }
                } else if (dep.configurable) {
                    if (dep.libraries) {
                        target.libraries ||= []
                        target.libraries = (dep.libraries + target.libraries).unique()
                    }
                }
                loader.inheritCompSettings(target, dep)
            }
        }
        runTargetScript(target, 'postresolve')
    }

    function resolveDependencies() {
        admitSetup('resolve')
        for each (target in me.targets) {
            if (target.enable) {
                resolve(target)
            }
        }
    }

    /**
        Run a command and trace output if copt.show or options.show
        @param command Command to run. May be an array of args or a string.
        @param copt Options. These are also passed to $Cmd.
        @option dir Change to given directory to run the command.
        @option filter Do not display output to the console if it contains the specified filter pattern.
            Set to true to filter (not display) any output. Note the command always returns the command output
            as the function result.
        @option generate Generate in projects. Defaults to true.
        @option noshow Do not show the command line before executing. Useful to override me --show for one command.
        @option nostop Continue processing even if this command is not successful.
        @option show Show the command line before executing. Similar to me --show, but operates on just this command.
        @option timeout Timeout for the command to complete

        Note: do not use the Cmd options: noio, detach. Use Cmd APIs directly.
     */
    public function run(command, copt = {}, data = null): String {
        if ((options.show && !copt.noshow) || copt.show) {
            let cmdline: String
            if (command is Array) {
                cmdline = command.join(' ')
            } else {
                cmdline = command
            }
            trace('Run', cmdline)
        }
        if (makeme.generating && copt.generate !== false) {
            genRun(command)
            return ''
        }
        if (copt.noio || copt.nothrow) {
            throw 'run option noio and nothrow options are not supported. Use filter and exceptions instead.'
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
                    if (!me.targets.compiler.vsdir.contains('$')) {
                        value = value.replace(/\$\(VS\)/g, me.targets.compiler.vsdir)
                    }
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

        let results = new ByteArray
        cmd.on('readable', function(event, cmd) {
            let buf = new ByteArray
            cmd.read(buf, -1)
            if (!copt.filter) {
                prints(buf)
            }
            results.write(buf)
        })
        if (data) {
            copt = blend({detach: true}, copt)
        }
        cmd.start(command, copt)
        if (data) {
            let written = cmd.write(data)
            cmd.finalize()
        }
        cmd.wait()
        let response = results.toString()

        if (cmd.status != 0) {
            let msg
            if (!cmd.error || cmd.error == '') {
                msg = response + '\nCommand failure: ' + response + '\nCommand: ' + command
            } else {
                msg = response + '\nCommand failure: ' + cmd.error + '\n' + response + '\nCommand: ' + command
            }
            //  DEPRECATED - continue, nonstop, continueOnErrors
            if (copt.nostop || copt.nonstop || copt.continueOnErrors || options['continue']) {
                if (!copt.filter) {
                    trace('Error', msg)
                }
            } else {
                throw response + '\nCommand failure: ' + cmd.error
            }
        } else if (copt.filter) {
            if (!copt.noshow) {
                if (copt.filter !== true) {
                    if (!(copt.filter is RegExp)) {
                        copt.filter = RegExp(copt.filter, "g")
                    }
                    if (response && !copt.filter.test(response)) {
                        prints(response)
                    }
                    if (cmd.error && !copt.filter.test(cmd.error)) {
                        App.errorStream.write(cmd.error)
                    }
                }
            }
        } else if (cmd.error) {
            App.errorStream.write(cmd.error)
        }
        return response
    }

    function runShell(target, interpreter, script) {
        run(Cmd.locate(interpreter), {}, script )
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
                let path = loader.expand(item.home)
                changeDir(path)
            }
            me.globals.HOME = App.dir
            me.globals.ORIGIN = target.origin
            try {
                if (item.interpreter == 'ejs') {
                    if (item.script is Function) {
                        result = item.script(target)
                    } else {
                        result = eval('require ejs.unix\nrequire embedthis.me.script\n' +
                            loader.expand(item.script, {missing: ''}))
                    }
                } else {
                    runShell(target, item.interpreter, item.script)
                }
            } catch (e) {
                if (options.rethrow) {
                    throw e
                } else {
                    if (options.show) {
                        App.log.error('Error with target: ' + target.name + '\nCommand: ' + item.script + '\n' + e + '\n')
                    } else {
                        App.log.error('Error with target: ' + target.name + '\n\n' + e + '\n')
                    }
                    throw "Exiting"
                }
            } finally {
                changeDir(pwd)
                delete me.target
                delete me.globals.HOME
            }
        }
        return result
    }


    /*
        Called with the desired goal. Goal will be set to true when being called for a required dependent.
     */
    function selectDependentTargets(target, goal) {
        /*
            Optimize by only processing dependents once
         */
        if (goal === true && !admit(target, 'select')) {
            return
        }
        if (target.selected) {
            return
        }
        if (!target.enable && !(target.ifdef && makeme.generating && options.configurableProject)) {
            return
        }
        if (goal === true || target.goals.contains(goal)) {
            target.selected = true
            for each (dname in target.depends) {
                if (dname == Builder.ALL) {
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
            if (goal !== true && target.topLevel) {
                topTargets.push(target)
            }
        } else {
            if (!options.verbose) {
                target.why = null
            }
        }
        if (target.why) {
            whySkip(target.name, target.why)
        }
    }

    /**
        Select the targets to build for a goal
        @return A list of targets
        @hide
     */
    public function selectTargets(goal): Array {
        selectedTargets = []
        topTargets = []
        admitSetup('select')
        for each (target in me.targets) {
            target.selected = false
        }
        for each (target in me.targets) {
            if (target.first) {
                selectDependentTargets(target, goal)
            }
        }
        for each (target in me.targets) {
            if (!target.first) {
                selectDependentTargets(target, goal)
            }
        }
        if (selectedTargets.length == 0) {
            if (goal != 'all') {
                trace('Info', 'No enabled targets for goal "' + goal + '"')
            }
        }
        return selectedTargets
    }

    function setPathEnvVar() {
        let bin = me.dir.bin.absolute
        let sep = App.SearchSeparator
        if (makeme.generating) {
            bin = bin.relative
        }
        App.putenv('PATH', bin + sep + App.getenv('PATH'))
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
            let entry = target.entry || (me.targets.compiler && me.targets.compiler.entry)
            if (entry) {
                tv.ENTRY = entry[target.rule || target.type]
            }
            let subsystem = target.subsystem || (me.targets.compiler && me.targets.compiler.subsystem)
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
            if (makeme.generating) {
                /*
                    Back quote quotes
                    Use abs paths to reppath can substitute as much as possible
                 */
                tv.DEFINES = target.defines.map(function(e) '-D' + e.replace(/"/g, '\\"')).join(' ')
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '"-I' + p + '"') : ''
            } else {
                /* Use relative paths to shorten trace output */
                tv.DEFINES = target.defines.map(function(e) '-D' + e).join(' ')
                tv.INCLUDES = (target.includes) ? target.includes.map(function(p) '"-I' + p.compact(base).portable + '"') : ''
            }
            tv.PDB = tv.OUTPUT.replaceExt('pdb')
            let home = App.home.portable.absolute
            if (home.join('.embedthis').exists && !makeme.generating) {
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

    /**
        @hide
     */
    public function sh(commands, copt = {}): String {
        let lines = commands.match(/^.*$/gm)
        for each (cmd in lines) {
            if (Config.OS == 'windows') {
                response = run('cmd /c "' + cmd + '"', copt)
            } else {
                response = run('bash -c "' + cmd + '"', copt)
            }
        }
        return response
    }

    function showConfiguration() {
        if (me.settings.configure && !options.verbose) {
            trace('Config', me.settings.configure)
        } else {
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
    }

    /*
        Test if a target is stale vs dependencies
     */
    function stale(target): Boolean {
        if (makeme.generating) {
            return target.generate !== false
        }
        if (!target.path) {
            if (target.scripts && target.scripts.build) {
                return true
            }
        }
        if (options.rebuild) {
            return true
        }
        let path = target.modify || target.path
        let name = path || target.name
        let modified = path ? path.modified : Date()

        if (path && !path.exists) {
            whyRebuild(name, 'Rebuild', path + ' is missing.')
            return true
        }
        for each (file in target.files) {
            if (file.isDir) {
                for each (f in file.files('**')) {
                     if (f.modified > modified) {
                        whyRebuild(name, 'Rebuild', 'file ' + f + ' has been modified.')
                        if (options.why && options.verbose) {
                            print(f, f.modified)
                            print(name, path, modified)
                        }
                        return true
                    }
                }
            } else {
                if (file.modified > modified) {
                    whyRebuild(name, 'Rebuild', 'file ' + file + ' has been modified.')
                    if (options.why && options.verbose) {
                        print(file, file.modified)
                        print(name, path, modified)
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
                    whyRebuild(name, 'Rebuild', 'missing dependency ' + dname)
                    return true
                }
                if (dname.modified > modified) {
                    whyRebuild(name, 'Rebuild', 'dependency ' + dname + ' has been modified.')
                    return true
                }
                return false

            } else if (dep.configurable) {
                if (!dep.enable) {
                    continue
                }
                for each (let sname: Path in (dep.depends + dep.uses)) {
                    let sub = getDep(sname)
                    if (sub && sub.enable && sub.name != target.name) {
                        if (stale(sub)) {
                            whyRebuild(name, 'Rebuild', 'dependent target ' + sname + ' is stale, for "' + dname + '"')
                            return true
                        }
                        if (sub.path && sub.path.modified > modified) {
                            whyRebuild(name, 'Rebuild', 'dependent target ' + sname + ' has been modified, for "' +
                                dname + '"')
                            return true
                        }
                    }
                }
                file = dep.path
                if (!file) {
                    continue
                }
                if (!file.exists) {
                    whyRebuild(name, 'Rebuild', 'missing ' + file + ' for "' + dname + '"')
                    return true
                }

            } else {
                file = dep.path
                if (!file) {
                    continue
                }
                if (file.modified > modified) {
                    whyRebuild(name, 'Rebuild', 'dependent ' + file + ' has been modified.')
                    return true
                }
            }
        }
        return false
    }

    /**
        Emit trace for me --why on why a target is being rebuilt.
        @param path Target path being considered.
        @param tag Informational tag emitted before the message.
        @param msg Message to display.
     */
    public function whyRebuild(path: Path, tag: String, msg: String) {
        if (options.why) {
            trace(tag, path + ' because ' + msg)
        }
    }

    /**
        Emit trace for me --why on why a target is being skipped.
        @param name Target path being considered.
        @param msg Message to display.
     */
    public function whySkip(name: String, msg: String) {
        if (options.why) {
            trace('Target', name + ' ' + msg)
        }
    }

    /********** Low Level Cross Platform API *********/

} /* Builder class */

} /* embedthis.me module */

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
