/*
    generate.es -- Generate Me projects

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

    require ejs.unix
    require ejs.zlib

    var gen: Object
    var genout: TextStream
    var capture: Array?

    var minimalCflags = [ 
        '-O2', '-O', '-w', '-g', '-Wall', '-Wno-deprecated-declarations', '-Wno-unused-result', 
        '-Wshorten-64-to-32', '-Wunreachable-code', '-mtune=generic']

    function generate() {
        if (b.options.gen == 'start') {
            generateStart()
            return
        }
        if (b.options.gen == 'main') {
            generateMain()
            return
        }
        if (Path(b.localPlatform + '.me').exists) {
            b.createMe(b.localPlatform, b.localPlatform + '.me')
        } else {
            b.createMe(b.localPlatform, b.options.file)
        }
        me.settings.name ||= App.dir.basename.replace(/-/g, '_')

        platforms = me.platforms = [b.localPlatform]
        me.original = {
            dir: me.dir.clone(),
            platform: me.platform.clone(),
        }
        for (d in me.dir) {
            if (d == 'makeme') continue
            me.dir[d] = me.dir[d].replace(me.original.platform.name, me.platform.name)
        }
        me.platform.last = true
        b.prepBuild()
        me.generating = true
        generateProjects()
        me.generating = null
    }

    function generateProjects() {
        b.selectTargets('all')
        let cpack = me.targets.compiler
        let cflags = cpack.compiler.join(' ')
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags = cflags.replace(/^ *$/, '')
        gen = {
            configuration:  me.platform.name
            compiler:       cflags,
            defines :       cpack.defines.map(function(e) '-D' + e).join(' '),
            includes:       cpack.includes.map(function(e) '-I' + e).join(' '),
            linker:         cpack.linker.join(' '),
            libpaths:       b.mapLibPaths(cpack.libpaths)
            libraries:      b.mapLibs(null, cpack.libraries).join(' ')
        }
        blend(gen, me.prefixes)
        for each (item in b.options.gen) {
            me.generating = item
            me.settings.name ||= 'app'
            let base = me.dir.proj.join(me.settings.name + '-' + me.platform.os + '-' + me.platform.profile)
            let path = me.original.dir.inc.join('me.h')
            let hfile = me.dir.src.join('projects', 
                    me.settings.name + '-' + me.platform.os + '-' + me.platform.profile + '-me.h')
            if (path.exists) {
                trace('Generate', 'project header: ' + hfile.relative)
                path.copy(hfile)
            }
            base.dirname.makeDir()
            if (me.generating == 'sh') {
                generateShellProject(base)
            } else if (me.generating == 'make') {
                generateMakeProject(base)
            } else if (me.generating == 'nmake') {
                generateNmakeProject(base)
            } else if (me.generating == 'vstudio' || me.generating == 'vs') {
                generateVstudioProject(base)
            } else if (me.generating == 'xcode') {
                generateXcodeProject(base)
            } else {
                throw 'Unknown generation format: ' + me.generating
            }
        }
    }

    function generateTarget(target) {
        if (target.configurable) {
            return
        }
        global.TARGET = me.target = target
        if (target.files) {
            target.cmdfiles = target.files.join(' ')
        }
        if (target.ifdef) {
            for each (r in target.ifdef) {
                if (me.platform.os == 'windows') {
                    genout.writeLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                } else {
                    genWriteLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                }
            }
        }
        if (target.generateScript) {
            generateScript(target)
        }
        if (target.type == 'lib') {
            if (target.static) {
                generateStaticLib(target)
            } else {
                generateSharedLib(target)
            }
        } else if (target.type == 'exe') {
            generateExe(target)
        } else if (target.type == 'obj') {
            generateObj(target)
        } else if (target.type == 'file' || target.type == 'header') {
            generateFile(target)
        } else if (target.type == 'resource') {
            generateResource(target)
        } else if (target.type == 'run') {
            generateRun(target)
        } else if (target.dir) {
            generateDir(target, true)
        }
        if (target.ifdef) {
            for (i in target.ifdef.length) {
                if (me.platform.os == 'windows') {
                    genWriteLine('!ENDIF')
                } else {
                    genWriteLine('endif')
                }
            }
        }
        genWriteLine('')
        global.TARGET = me.target = null
    }

    function generateMain() {
        let makeme = Config.Bin.join('makeme')
        let cfg = Path('configure')
        if (cfg.exists && !b.options.overwrite) {
            trace('Exists', 'configure')
        } else {
            let data = '#!/bin/bash\n#\n#   configure -- Configure for building\n#\n' +
                'if ! type me >/dev/null 2>&1 ; then\n' +
                    '    echo -e "\\nInstall the \\"me\\" tool for configuring." >&2\n' +
                    '    echo -e "Download from: http://embedthis.com/downloads/me/download.ejs." >&2\n' +
                    '    echo -e "Or skip configuring and make a standard build using \\"make\\".\\n" >&2\n' +
                    '    exit 255\n' +
                'fi\n' + 
                'me configure "$@"'
            trace(cfg.exists ? 'Overwrite' : 'Create', cfg)
            cfg.write(data)
            cfg.setAttributes({permissions: 0755})
        }
        b.safeCopy(makeme.join('sample-main.me'), b.MAIN)
    }

    function generateStart() {
        b.safeCopy(Path(Config.Bin).join('makeme/sample-start.me'), 'start.me')
    }

    function generateShellProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.sh')
        let path = base.joinExt('sh')
        genout = TextStream(File(path, 'w'))
        genout.writeLine('#\n#   ' + path.basename + ' -- MakeMe Shell Script to build ' + me.settings.title + '\n#\n')
        genEnv()
        genout.writeLine('NAME="' + me.settings.name + '"')
        genout.writeLine('VERSION="' + me.settings.version + '"')
        genout.writeLine('PROFILE="' + me.platform.profile + '"')
        genout.writeLine('ARCH="' + me.platform.arch + '"')
        genout.writeLine('ARCH="`uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\'`"')
        genout.writeLine('OS="' + me.platform.os + '"')
        genout.writeLine('CONFIG="${OS}-${ARCH}-${PROFILE}' + '"')
        genout.writeLine('CC="' + me.targets.compiler.path + '"')
        if (me.targets.link) {
            genout.writeLine('LD="' + me.targets.link.path + '"')
        }
        let cflags = gen.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags += ' -w'
        genout.writeLine('CFLAGS="' + cflags.trim() + '"')
        genout.writeLine('DFLAGS="' + gen.defines + '"')
        genout.writeLine('IFLAGS="' + 
            repvar(me.targets.compiler.includes.map(function(path) '-I' + path.relative).join(' ')) + '"')
        genout.writeLine('LDFLAGS="' + repvar(gen.linker).replace(/\$ORIGIN/g, '\\$$ORIGIN') + '"')
        genout.writeLine('LIBPATHS="' + repvar(gen.libpaths) + '"')
        genout.writeLine('LIBS="' + gen.libraries + '"\n')
        genout.writeLine('[ ! -x ${CONFIG}/inc ] && ' + 'mkdir -p ${CONFIG}/inc\n')
        genout.writeLine('[ ! -x ${CONFIG}/bin ] && ' + 'mkdir -p ${CONFIG}/bin\n')
        genout.writeLine('[ ! -x ${CONFIG}/obj ] && ' + 'mkdir -p ${CONFIG}/obj\n')
        if (me.dir.src.join('src/paks/osdep/osdep.h').exists) {
            genout.writeLine('[ ! -f ${CONFIG}/inc/osdep.h ] && cp ${SRC}/src/paks/osdep/osdep.h ${CONFIG}/inc/osdep.h')
        }
        if (me.dir.inc.join('me.h').exists) {
            genout.writeLine('[ ! -f ${CONFIG}/inc/me.h ] && ' + 
                'cp projects/' + me.settings.name + '-${OS}-${PROFILE}-me.h ${CONFIG}/inc/me.h')
            genout.writeLine('if ! diff ${CONFIG}/inc/me.h projects/' + me.settings.name + 
                '-${OS}-${PROFILE}-me.h >/dev/null ; then')
        }
        genout.writeLine('\tcp projects/' + me.settings.name + '-${OS}-${PROFILE}-me.h ${CONFIG}/inc/me.h')
        genout.writeLine('fi\n')
        b.build()
        genout.close()
        path.setAttributes({permissions: 0755})
    }

    function mapPrefixes() {
        prefixes = {}
        let root = me.prefixes.root
        let base = me.prefixes.base
        let app = me.prefixes.app
        let vapp = me.prefixes.vapp
        for (let [name,value] in me.prefixes) {
            if (name.startsWith('programFiles')) continue
            value = expand(value).replace(/\/\//g, '/')
            if (name == 'root') {
                ;
            } else if (name == 'base') {
                if (value.startsWith(root.name)) {
                    if (root.name == '/') {
                        value = value.replace(root.name, '$(ME_ROOT_PREFIX)/')
                    } else if (me.platform.like == 'windows') {
                        value = value.replace(root.name, '$(ME_ROOT_PREFIX)\\')
                    } else {
                        value = value.replace(root.name, '$(ME_ROOT_PREFIX)')
                    }
                } else {
                    value = '$(ME_ROOT_PREFIX)' + value
                }
            } else if (name == 'app') {
                if (value.startsWith(base.name)) {
                    value = value.replace(base.name, '$(ME_BASE_PREFIX)')
                }
            } else if (name == 'vapp') {
                if (value.startsWith(app.name)) {
                    value = value.replace(app.name, '$(ME_APP_PREFIX)')
                }
            } else if (value.startsWith(vapp.name)) {
                value = value.replace(vapp.name, '$(ME_VAPP_PREFIX)')
            } else {
                value = '$(ME_ROOT_PREFIX)' + value
            }
            value = value.replace(me.settings.version, '$(VERSION)')
            value = value.replace(me.settings.name, '$(NAME)')
            prefixes[name] = Path(value.toString())
        }
        return prefixes
    }

    function generateComponentDefs() {
        let needed = {}
        for each (target in me.targets) {
            if (target.configurable) continue
            if (target.explicit || target.enable) {
                needed[target.name] = true
            }
            for each (r in target.ifdef) {
                if (me.targets[r]) {
                    needed[r] = true
                }
            }
            for each (r in target.depends) {
                if (me.targets[r]) {
                    needed[r] = true
                }
            }
            for each (r in target.uses) {
                if (me.targets[r] && me.targets[r].enable) {
                    needed[r] = true
                }
            }
        }
        /* UNUSED - replaced with below so you don't have to repeat in extras
        for each (let name in (me.configure.discovers + me.configure.extras)) {
            me.targets[name] ||= {}
            me.targets[name].enable ||= false
            needed[name] = true
        }
        */
        for each (let target in me.targets) {
            if (!target.configurable) continue
            if (me.configure.requires.contains(target.name)) continue
            needed[target.name] = true
        }
        for each (let target in me.targets) {
            if (!target.configurable) continue
            if (needed[target.name]) {
                for each (r in target.ifdef) {
                    if (me.targets[r]) {
                        needed[r] = true
                    }
                }
            }
        }
        /*
            Emit ME_COM_* definitions 
         */
        Object.sortProperties(me.targets)
        for each (let target in me.targets) {
            if (!target.configurable) continue
            let name = target.name
            if (needed[name]) {
                if (me.platform.os == 'windows' ) {
                    genout.writeLine('!IF "$(ME_COM_' + name.toUpper() + ')" == ""')
                    genout.writeLine('%-21s = %s'.format(['ME_COM_' + name.toUpper(), target.enable ? 1 : 0]))
                    genout.writeLine('!ENDIF')
                } else {
                    genout.writeLine('%-21s ?= %s'.format(['ME_COM_' + name.toUpper(), target.enable ? 1 : 0]))
                }
            }
        }
        genout.writeLine('')

        /*
            Emit configurable definitions
         */
        for each (let target in me.targets) {
            if (!target.configurable) continue
            let name = target.name
            if (needed[name] && target.ifdef) {
                if (me.platform.os == 'windows' ) {
                    for each (r in target.ifdef) {
                        genout.writeLine('!IF "$(ME_COM_' + r.toUpper() + ')" == ""')
                        genout.writeLine('%-21s = 1'.format(['ME_COM_' + r.toUpper()]))
                        genout.writeLine('!ENDIF\n')
                    }
                } else {
                    for each (r in target.ifdef) {
                        genout.writeLine('ifeq ($(ME_COM_' + name.toUpper() + '),1)')
                        for each (r in target.ifdef) {
                            genout.writeLine('    ME_COM_' + r.toUpper() + ' := 1')
                        }
                        genout.writeLine('endif')
                    }
                }
            }
        }
        // genout.writeLine('#####\n')

        /*
            Emit configurable depends[]
         */
        let emitted = {}
        for (let [name, target] in me.targets) {
            if (!target.configurable) continue
            if (needed[name] && target.depends && !emitted[name]) {
                emitted[name] = true
                let seenItem = false
                if (me.platform.os == 'windows' ) {
                    for each (r in target.depends) {
                        if (me.targets[r] && me.targets[r].configurable) {
                            if (!seenItem) {
                                genout.writeLine('!IF "$(ME_COM_' + r.toUpper() + ')" == ""')
                                seenItem = true
                            }
                            genout.writeLine('%-21s = 1'.format(['ME_COM_' + r.toUpper()]))
                        }
                    }
                    if (seenItem) {
                        genout.writeLine('!ENDIF\n')
                    }
                } else {
                    for each (r in target.depends) {
                        if (me.targets[r] && me.targets[r].configurable) {
                            if (!seenItem) {
                                genout.writeLine('ifeq ($(ME_COM_' + name.toUpper() + '),1)')
                                seenItem = true
                            }
                            genout.writeLine('    ME_COM_' + r.toUpper() + ' := 1')
                        }
                    }
                    if (seenItem) {
                        genout.writeLine('endif')
                    }
                }
            }
        }
        genout.writeLine('')

        /*
            Emit configurable paths
         */
        for each (let target in me.targets) {
            if (!target.configurable) continue
            if (target.path) {
                if (me.platform.os == 'windows') {
                    genout.writeLine('%-21s = %s'.format(['ME_COM_' + target.name.toUpper() + '_PATH', target.path]))
                } else {
                    genout.writeLine('%-21s ?= %s'.format(['ME_COM_' + target.name.toUpper() + '_PATH', target.path]))
                }
            }
        }
        genout.writeLine('')

        /*
            Compute the dflags 
         */
        let dflags = ''
        for (let [name, target] in me.targets) {
            if (!target.configurable) continue
            if (needed[name]) {
                dflags += '-DME_COM_' + name.toUpper() + '=$(ME_COM_' + name.toUpper() + ') '
            }
        }
        return dflags
    }

    function generateMakeProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.mk')
        let path = base.joinExt('mk')
        genout = TextStream(File(path, 'w'))
        genout.writeLine('#\n#   ' + path.basename + ' -- Makefile to build ' + 
            me.settings.title + ' for ' + me.platform.os + '\n#\n')
        b.runScript(me.scripts, 'pregen')
        genout.writeLine('NAME                  := ' + me.settings.name)
        genout.writeLine('VERSION               := ' + me.settings.version)
        genout.writeLine('PROFILE               ?= ' + me.platform.profile)
        if (me.platform.os == 'vxworks') {
            genout.writeLine("ARCH                  ?= $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*//')")
            genout.writeLine("CPU                   ?= $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))")
        } else {
            genout.writeLine('ARCH                  ?= $(shell uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\')')
            genout.writeLine('CC_ARCH               ?= $(shell echo $(ARCH) | sed \'s/x86/i686/;s/x64/x86_64/\')')
        }
        genout.writeLine('OS                    ?= ' + me.platform.os)
        genout.writeLine('CC                    ?= ' + me.targets.compiler.path)
        if (me.targets.link) {
            genout.writeLine('LD                    ?= ' + me.targets.link.path)
        }
        genout.writeLine('CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)')
        genout.writeLine('LBIN                  ?= $(CONFIG)/bin')
        genout.writeLine('PATH                  := $(LBIN):$(PATH)\n')

        let dflags = generateComponentDefs()
        genEnv()

        let cflags = gen.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags += ' -w'
        genout.writeLine('CFLAGS                += ' + cflags.trim())
        genout.writeLine('DFLAGS                += ' + gen.defines.replace(/-DME_DEBUG */, '') + 
            ' $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) ' + dflags)
        genout.writeLine('IFLAGS                += "' + 
            repvar(me.targets.compiler.includes.map(function(path) '-I' + reppath(path.relative)).join(' ')) + '"')
        let linker = me.targets.compiler.linker.map(function(s) "'" + s + "'").join(' ')
        let ldflags = repvar(linker).replace(/\$ORIGIN/g, '$$$$ORIGIN').replace(/'-g' */, '')
        genout.writeLine('LDFLAGS               += ' + ldflags)
        genout.writeLine('LIBPATHS              += ' + repvar(gen.libpaths))
        genout.writeLine('LIBS                  += ' + gen.libraries + '\n')

        genout.writeLine('DEBUG                 ?= ' + (me.settings.debug ? 'debug' : 'release'))
        genout.writeLine('CFLAGS-debug          ?= -g')
        genout.writeLine('DFLAGS-debug          ?= -DME_DEBUG')
        genout.writeLine('LDFLAGS-debug         ?= -g')
        genout.writeLine('DFLAGS-release        ?= ')
        genout.writeLine('CFLAGS-release        ?= -O2')
        genout.writeLine('LDFLAGS-release       ?= ')
        genout.writeLine('CFLAGS                += $(CFLAGS-$(DEBUG))')
        genout.writeLine('DFLAGS                += $(DFLAGS-$(DEBUG))')
        genout.writeLine('LDFLAGS               += $(LDFLAGS-$(DEBUG))\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name == 'root' && value == '/') {
                value = ''
            }
            genout.writeLine('%-21s ?= %s'.format(['ME_' + name.toUpper() + '_PREFIX', value]))
        }
        genout.writeLine('')
        b.runScript(me.scripts, 'gencustom')
        genout.writeLine('')

        let pop = me.settings.name + '-' + me.platform.os + '-' + me.platform.profile
        genTargets()
        genout.writeLine('unexport CDPATH\n')
        genout.writeLine('ifndef SHOW\n.SILENT:\nendif\n')
        genout.writeLine('all build compile: prep $(TARGETS)\n')
        genout.writeLine('.PHONY: prep\n\nprep:')
        genout.writeLine('\t@echo "      [Info] Use "make SHOW=1" to trace executed commands."')
        genout.writeLine('\t@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi')
        if (me.prefixes.app) {
            genout.writeLine('\t@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi')
        }
        if (me.platform.os == 'vxworks') {
            genout.writeLine('\t@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi')
            genout.writeLine('\t@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi')
            genout.writeLine('\t@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi')
        }
        genout.writeLine('\t@[ ! -x $(CONFIG)/bin ] && ' + 'mkdir -p $(CONFIG)/bin; true')
        genout.writeLine('\t@[ ! -x $(CONFIG)/inc ] && ' + 'mkdir -p $(CONFIG)/inc; true')
        genout.writeLine('\t@[ ! -x $(CONFIG)/obj ] && ' + 'mkdir -p $(CONFIG)/obj; true')
        if (me.dir.src.join('src/paks/osdep/osdep.h').exists) {
            genout.writeLine('\t@[ ! -f $(CONFIG)/inc/osdep.h ] && cp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h ; true')
            genout.writeLine('\t@if ! diff $(CONFIG)/inc/osdep.h src/paks/osdep/osdep.h >/dev/null ; then\\')
            genout.writeLine('\t\tcp src/paks/osdep/osdep.h $(CONFIG)/inc/osdep.h  ; \\')
            genout.writeLine('\tfi; true')
        }
        if (me.dir.inc.join('me.h').exists) {
            genout.writeLine('\t@[ ! -f $(CONFIG)/inc/me.h ] && ' + 'cp projects/' + pop + '-me.h $(CONFIG)/inc/me.h ; true')
            genout.writeLine('\t@if ! diff $(CONFIG)/inc/me.h projects/' + pop + '-me.h >/dev/null ; then\\')
            genout.writeLine('\t\tcp projects/' + pop + '-me.h $(CONFIG)/inc/me.h  ; \\')
            genout.writeLine('\tfi; true')
        }
        genout.writeLine('\t@if [ -f "$(CONFIG)/.makeflags" ] ; then \\')
        genout.writeLine('\t\tif [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \\')
        genout.writeLine('\t\t\techo "   [Warning] Make flags have changed since the last build: \"`cat $(CONFIG)/.makeflags`\"" ; \\')
        genout.writeLine('\t\tfi ; \\')
        genout.writeLine('\tfi')
        genout.writeLine('\t@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags\n')

        genout.writeLine('clean:')
        builtin('cleanTargets')
        genout.writeLine('\nclobber: clean\n\trm -fr ./$(CONFIG)\n')
        b.build()
        genout.close()
    }

    function generateNmakeProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.nmake')
        let path = base.joinExt('nmake')
        genout = TextStream(File(path, 'w'))
        genout.writeLine('#\n#   ' + path.basename + ' -- Makefile to build ' + me.settings.title + 
            ' for ' + me.platform.os + '\n#\n')
        b.runScript(me.scripts, 'pregen')
        genout.writeLine('NAME                  = ' + me.settings.name)
        genout.writeLine('VERSION               = ' + me.settings.version + '\n')
        genout.writeLine('OS                    = ' + me.platform.os)
        genout.writeLine('PA                    = $(PROCESSOR_ARCHITECTURE)')

        genout.writeLine('!IF "$(PROFILE)" == ""')
        genout.writeLine('PROFILE               = ' + me.platform.profile)
        genout.writeLine('!ENDIF\n')

        genout.writeLine('')
        genout.writeLine('!IF "$(PA)" == "AMD64"')
            genout.writeLine('ARCH                  = x64')
            genout.writeLine('ENTRY                 = _DllMainCRTStartup')
        genout.writeLine('!ELSE')
            genout.writeLine('ARCH                  = x86')
            genout.writeLine('ENTRY                 = _DllMainCRTStartup@12')
        genout.writeLine('!ENDIF\n')

        genout.writeLine('!IF "$(CONFIG)" == ""')
        genout.writeLine('CONFIG                = $(OS)-$(ARCH)-$(PROFILE)')
        genout.writeLine('!ENDIF\n')
        genout.writeLine('LBIN                  = $(CONFIG)\\bin\n')

        let dflags = generateComponentDefs()

        genout.writeLine('CC                    = cl')
        genout.writeLine('LD                    = link')
        genout.writeLine('RC                    = rc')
        genout.writeLine('CFLAGS                = ' + gen.compiler)
        genout.writeLine('DFLAGS                = ' + gen.defines + ' ' + dflags)
        genout.writeLine('IFLAGS                = ' + 
            repvar(me.targets.compiler.includes.map(function(path) '-I' + reppath(path)).join(' ')))
        genout.writeLine('LDFLAGS               = ' + repvar(gen.linker).replace(/-machine:x86/, '-machine:$$(ARCH)'))
        genout.writeLine('LIBPATHS              = ' + repvar(gen.libpaths).replace(/\//g, '\\'))
        genout.writeLine('LIBS                  = ' + gen.libraries + '\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name.startsWith('programFiles')) continue
            /* TODO value.windows will change C:/ to C: */
            if (name == 'root') {
                value = value.trimEnd('/')
            } else {
                value = value.map('\\')
            }
            genout.writeLine('%-21s = '.format(['ME_' + name.toUpper() + '_PREFIX']) + value)
        }
        genout.writeLine('')
        b.runScript(me.scripts, 'gencustom')
        genout.writeLine('')

        genTargets()
        let pop = me.settings.name + '-' + me.platform.os + '-' + me.platform.profile
        genout.writeLine('!IFNDEF SHOW\n.SILENT:\n!ENDIF\n')
        genout.writeLine('all build compile: prep $(TARGETS)\n')
        genout.writeLine('.PHONY: prep\n\nprep:')
        genout.writeLine('!IF "$(VSINSTALLDIR)" == ""\n\techo "Visual Studio vars not set. Run vcvars.bat."\n\texit 255\n!ENDIF')
        if (me.prefixes.app) {
            genout.writeLine('!IF "$(ME_APP_PREFIX)" == ""\n\techo "ME_APP_PREFIX not set."\n\texit 255\n!ENDIF')
        }
        genout.writeLine('\t@if not exist $(CONFIG)\\bin md $(CONFIG)\\bin')
        genout.writeLine('\t@if not exist $(CONFIG)\\inc md $(CONFIG)\\inc')
        genout.writeLine('\t@if not exist $(CONFIG)\\obj md $(CONFIG)\\obj')
        if (me.dir.inc.join('me.h').exists) {
            genout.writeLine('\t@if not exist $(CONFIG)\\inc\\me.h ' + 'copy projects\\' + pop + '-me.h $(CONFIG)\\inc\\me.h\n')
        }
        genout.writeLine('clean:')
        builtin('cleanTargets')
        genout.writeLine('')
        b.build()
        genout.close()
    }

    function generateVstudioProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative)
        mkdir(base)
        global.load(me.dir.me.join('vstudio.es'))
        vstudio(base)
    }

    function generateXcodeProject(base: Path) {
        global.load(me.dir.me.join('xcode.es'))
        xcode(base)
    }

    function genEnv() {
        let found
        if (me.platform.os == 'windows') {
            var winsdk = (me.targets.winsdk && me.targets.winsdk.path) ? 
                me.targets.winsdk.path.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft SDKs\\Windows\\v6.1'
            var vs = (me.targets.compiler && me.targets.compiler.dir) ? 
                me.targets.compiler.dir.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft Visual Studio 9.0'
            if (me.generating == 'make') {
                /* Not used */
                genout.writeLine('VS             := ' + '$(VSINSTALLDIR)')
                genout.writeLine('VS             ?= ' + vs)
                genout.writeLine('SDK            := ' + '$(WindowsSDKDir)')
                genout.writeLine('SDK            ?= ' + winsdk)
                genout.writeLine('\nexport         SDK VS')
            }
        }
        for (let [key,value] in me.env) {
            if (me.platform.os == 'windows') {
                value = value.map(function(item)
                    item.replace(me.targets.compiler.dir, '$(VS)').replace(me.targets.winsdk.path, '$(SDK)')
                )
            }
            if (value is Array) {
                value = value.join(App.SearchSeparator)
            }
            if (me.platform.os == 'windows') {
                if (key == 'INCLUDE' || key == 'LIB') {
                    value = '$(' + key + ');' + value
                } else if (key == 'PATH') {
                    value = value + ';$(' + key + ')'
                } 
            }
            if (me.generating == 'make') {
                genout.writeLine('export %-14s ?= %s' % [key, value])

            } else if (me.generating == 'nmake') {
                value = value.replace(/\//g, '\\')
                genout.writeLine('%-9s = %s' % [key, value])

            } else if (me.generating == 'sh') {
                genout.writeLine('export ' + key + '="' + value + '"')
            }
            found = true
        }
        if (me.platform.os == 'vxworks') {
            genout.writeLine('%-21s := %s'.format(['export PATH', '$(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)']))
        }
        if (found) {
            genout.writeLine('')
        }
    }

    function genTargets() {
        let all = []
        for each (target in b.topTargets) {
            if (target.path && target.enable && !target.nogen) {
                let path = target.path
                if (target.ifdef) {
                    for each (pname in target.ifdef) {
                        if (me.platform.os == 'windows') {
                            genout.writeLine('!IF "$(ME_COM_' + pname.toUpper() + ')" == "1"')
                        } else {
                            genout.writeLine('ifeq ($(ME_COM_' + pname.toUpper() + '),1)')
                        }
                    }
                    if (me.platform.os == 'windows') {
                        genout.writeLine('TARGETS               = $(TARGETS) ' + reppath(path))
                    } else {
                        genout.writeLine('    TARGETS           += ' + reppath(path))
                    }
                    for (i in target.ifdef.length) {
                        if (me.platform.os == 'windows') {
                            genout.writeLine('!ENDIF')
                        } else {
                            genout.writeLine('endif')
                        }
                    }
                } else {
                    if (me.platform.os == 'windows') {
                        genout.writeLine('TARGETS               = $(TARGETS) ' + reppath(path))
                    } else {
                        genout.writeLine('TARGETS               += ' + reppath(path))
                    }
                }
            }
        }
        genout.writeLine('')
    }

    function generateDir(target, solo = false) {
        if (target.dir) {
            if (me.generating == 'sh') {
                makeDir(target.dir)

            } else if (me.generating == 'make' || me.generating == 'nmake') {
                if (solo) {
                    genTargetDeps(target)
                    genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
                }
                makeDir(target.dir)
            }
        }
    }

    function generateExe(target) {
        let transition = target.rule || 'exe'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (me.generating == 'sh') {
            command = repcmd(command)
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genout.writeLine(command)

        } else if (me.generating == 'make' || me.generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.path.natural.relative)
            generateDir(target)
            genout.writeLine('\t' + command)
        }
    }

    function generateSharedLib(target) {
        let transition = target.rule || 'shlib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (me.generating == 'sh') {
            command = repcmd(command)
            genout.writeLine(command)

        } else if (me.generating == 'make' || me.generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.path.natural.relative)
            generateDir(target)
            genout.writeLine('\t' + command)
        }
    }

    function generateStaticLib(target) {
        let transition = target.rule || 'lib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (me.generating == 'sh') {
            command = repcmd(command)
            genout.writeLine(command)

        } else if (me.generating == 'make' || me.generating == 'nmake') {
            command = repcmd(command)
            genTargetDeps(target)
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.path.natural.relative)
            generateDir(target)
            genout.writeLine('\t' + command)
        }
    }

    /*
        Build symbols file for windows libraries
     */
    function generateSym(target) {
        throw "Not supported to generate sym targets yet"
    }

    /*
        Build an object from source
     */
    function generateObj(target) {
        runTargetScript(target, 'precompile')

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
            let command = b.expandRule(target, rule)
            if (me.generating == 'sh') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genout.writeLine(command)

            } else if (me.generating == 'make') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n    ' + file.relative + getDepsVar() + '\n')
                gtrace('Compile', target.path.natural.relative)
                generateDir(target)
                genout.writeLine('\t' + command)

            } else if (me.generating == 'nmake') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n    ' + file.relative.windows + getDepsVar() + '\n')
                gtrace('Compile', target.path.natural.relative)
                generateDir(target)
                genout.writeLine('\t' + command)
            }
        }
        runTargetScript(target, 'postcompile')
    }

    function generateResource(target) {
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
            let command = b.expandRule(target, rule)
            if (me.generating == 'sh') {
                command = repcmd(command)
                genout.writeLine(command)

            } else if (me.generating == 'make') {
                command = repcmd(command)
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n        ' + file.relative + getDepsVar() + '\n')
                gtrace('Compile', target.path.natural.relative)
                generateDir(target)
                genout.writeLine('\t' + command)

            } else if (me.generating == 'nmake') {
                command = repcmd(command)
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n        ' + file.relative.windows + getDepsVar() + '\n')
                gtrace('Compile', target.path.natural.relative)
                generateDir(target)
                genout.writeLine('\t' + command)
            }
        }
    }

    /*
        Copy files[] to path
     */
    function generateFile(target) {
        target.made ||= {}
        if (me.generating == 'make' || me.generating == 'nmake') {
            genTargetDeps(target)
            if (target.touch) {
                genout.write(reppath(target.touch) + ':' + getDepsVar() + '\n')
            } else {
                genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            }
        }
        let dest = target.dest || target.path
        gtrace('Copy', dest.relative.portable)

        generateDir(target)
        for each (let file: Path in target.files) {
            /* Auto-generated headers targets for includes have file == target.path */
            if (file == target.path) {
                continue
            }
            copy(file, dest, target)
        }
        if (target.dest) {
            removeDir(target.path)
            makeDir(target.path)
        }
        delete target.made
    }

    function generateRun(target) {
        let command = target.run.clone()
        if (command is Array) {
            for (let [key,value] in command) {
                command[key] = expand(value)
            }
        } else {
            command = expand(command)
        }
        if (command is Array) {
            command = command.map(function(a) '"' + a + '"').join(' ')
        }
        let prefix, suffix
        if (me.generating == 'sh' || me.generating == 'make') {
            prefix = 'cd ' + target.home.relative
            suffix = 'cd ' + me.dir.top.relativeTo(target.home)
        } else if (me.generating == 'nmake') {
            prefix = 'cd ' + target.home.relative.windows + '\n'
            suffix = '\ncd ' + me.dir.src.relativeTo(target.home).windows
        } else {
            prefix = suffix = ''
        }
        let rhome = target.home.relative
        if (rhome == '.' || rhome.startsWith('..')) {
            /* Don't change directory out of source tree. Necessary for actions in standard.me */
            prefix = suffix = ''
        }
        if (me.generating == 'make' || me.generating == 'nmake') {
            genTargetDeps(target)
            genout.write(reppath(target.name) + ':' + getDepsVar() + '\n')
        }
        if (me.generating == 'make' || me.generating == 'sh') {
            if (prefix || suffix) {
                if (command.startsWith('@')) {
                    command = command.slice(1).replace(/^.*$/mg, '\t@' + prefix + '; $& ; ' + suffix)
                } else {
                    command = command.replace(/^.*$/mg, '\t' + prefix + '; $& ; ' + suffix)
                }
            } else {
                command = command.replace(/^/mg, '\t')
            }
        } else if (me.generating == 'nmake') {
            command = prefix + command + suffix
            command = command.replace(/^[ \t]*/mg, '')
            command = command.replace(/^([^!])/mg, '\t$&')
        }
        generateDir(target)
        genout.write(command)
    }

    function generateScript(target) {
        setRuleVars(target, target.home)
        let prefix = ''
        let suffix = ''
        assert(me.generating)
        assert(me.generating)

        let kind = me.generating
        //  TODO - always true
        if (me.generating) {
            if (me.generating == 'sh' || me.generating == 'make') {
                prefix = 'cd ' + target.home.relative
                suffix = 'cd ' + me.dir.top.relativeTo(target.home)
            } else if (me.generating == 'nmake') {
                prefix = 'cd ' + target.home.relative.windows + '\n'
                suffix = '\ncd ' + me.dir.src.relativeTo(target.home).windows
            } else {
                prefix = suffix = ''
            }
            let rhome = target.home.relative
            if (/* UNUSED rhome == '.' || */ rhome.startsWith('..')) {
                /* Don't change directory out of source tree. Necessary for actions in standard.me */
                prefix = suffix = ''
            }
        }
        let cmd
        if (target['generate-capture']) {
            capture = []
            runTargetScript(target, 'build')
            if (capture.length > 0) {
                if (prefix && me.generating != 'nmake') {
                    cmd = capture.join(' ; \\\n\t')
                } else {
                    cmd = capture.join('\n\t')
                }
            } else {
                prefix = suffix = ''
            }
            capture = null
        } else {
            let sh = (me.generating == 'make' | me.generating == 'sh' || me.generating == 'xcode') ? target['generate-sh'] : null
            cmd = target['generate-' + kind + '-' + me.platform.os] || target['generate-' + kind] || 
                target['generate-make-' + me.platform.os] || target['generate-make'] || sh || target.generate
            if (cmd && me.generating != 'nmake') {
                cmd = cmd.trim().replace(/\n/mg, ' ; \\\n')
            }
        } 
        if (me.generating == 'sh') {
            if (cmd) {
                cmd = cmd.trim()
                cmd = cmd.replace(/\\\n/mg, '')
                if (prefix) {
                    if (cmd.startsWith('@')) {
                        cmd = cmd.slice(1).replace(/^.*$/mg, '\t@' + prefix + '; $& ; ' + suffix)
                    } else {
                        cmd = cmd.replace(/^.*$/mg, '\t' + prefix + '; $& ; ' + suffix)
                    }
                } else {
                    cmd = cmd.replace(/^/mg, '\t')
                }
                me.globals.LBIN = '$(LBIN)'
                cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                cmd = repvar2(cmd, target.home)
                me.globals.LBIN = b.localBin
                genWriteLine(cmd)
            } else {
                genout.write('#  Omit build script ' + target.name + '\n')
            }

        } else if (me.generating == 'make') {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            if (cmd) {
                cmd = cmd.trim().replace(/^\s*/mg, '\t')
                /*
                    cmd = cmd.replace(/^\t*(ifeq|ifneq|else|endif)/mg, '$1')
                    if (prefix || suffix) {
                        if (cmd.startsWith('\t@')) {
                            cmd = cmd.slice(2).replace(/^\s*(.*)$/mg, '\t@' + prefix + '; $1 ; ' + suffix)
                        } else {
                            cmd = cmd.replace(/^\s(.*)$/mg, '\t' + prefix + '; $1 ; ' + suffix)
                        }
                    }
                */
                if (prefix) {
                    cmd = '\t( \\\n\t' + prefix + '; \\\n' + cmd + ' ; \\\n\t)'
                }
                me.globals.LBIN = '$(LBIN)'
                cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                cmd = repvar2(cmd, target.home)
                me.globals.LBIN = b.localBin
                genWriteLine(cmd)
            }

        } else if (me.generating == 'nmake') {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative.windows + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            if (cmd && cmd.match(/^[ \t]*$/)) {
               cmd = null
            }
            if (cmd) {
                cmd = cmd.trim().replace(/^cp /, 'copy ')
                cmd = prefix + cmd + suffix
                cmd = cmd.replace(/^[ \t]*/mg, '')
                cmd = cmd.replace(/^([^!])/mg, '\t$&')
                let saveDir = []
                if (me.platform.os == 'windows') {
                    for (n in me.globals) {
                        if (me.globals[n] is Path) {
                            saveDir[n] = me.globals[n]
                            me.globals[n] = me.globals[n].windows
                        }
                    }
                }
                me.globals.LBIN = '$(LBIN)'
                try {
                    cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                } catch (e) {
                    print('Target', target.name)
                    print('Script:', cmd)
                    throw e
                }
                if (me.platform.os == 'windows') {
                    for (n in saveDir) {
                        me.globals[n] = saveDir[n]
                    }
                }
                cmd = repvar2(cmd, target.home)
                me.globals.LBIN = b.localBin
                genWriteLine(cmd)
            } else {
                genout.write('#  Omit build script ' + target.name + '\n')
            }
        }
    }

    function rep(s: String, pattern, replacement): String {
        if (pattern) {
            return s.replace(pattern, replacement)
        }
        return s
    }

    function repCmd(s: String, pattern, replacement): String {
        if (s.startsWith(pattern)) {
            return s.replace(pattern, replacement)
        }
        if (s.startsWith('"' + pattern + '"')) {
            return s.replace(pattern, replacement)
        }
        return s
    }

    /*
        Replace default defines, includes, libraries etc with token equivalents. This allows
        Makefiles and script to be use variables to control various flag settings.
     */
    function repcmd(command: String): String {
        if (me.generating == 'make' || me.generating == 'nmake') {
            if (gen.linker != '') {
                /* Linker has -g which is also in minimal C flags */
                command = rep(command, gen.linker, '$(LDFLAGS)')
            }
            for each (word in minimalCflags) {
                command = rep(command, word + ' ', ' ')
            }
            if (gen.defines != '') {
                command = rep(command, gen.defines, '$(DFLAGS)')
            } else {
                command = rep(command, ' -c ', ' -c $(DFLAGS) ')
            }
            if (gen.compiler != '') {
                command = rep(command, gen.compiler, '$(CFLAGS)')
            } else {
                command = rep(command, ' -c ', ' -c $(CFLAGS) ')
            }
            command = rep(command, gen.libpaths, '$(LIBPATHS)')
            command = rep(command, gen.includes, '$(IFLAGS)')
            command = rep(command, '"$(IFLAGS)"', '$(IFLAGS)')
            /* Twice because libraries are repeated and replace only changes the first occurrence */
            command = rep(command, gen.libraries, '$(LIBS)')
            command = rep(command, gen.libraries, '$(LIBS)')
            command = rep(command, RegExp(gen.configuration, 'g'), '$$(CONFIG)')
            if (me.targets.compiler) {
                command = repCmd(command, me.targets.compiler.path, '$(CC)')
            }
            if (me.targets.link) {
                command = repCmd(command, me.targets.link.path, '$(LD)')
            }
            if (me.targets.rc) {
                command = repCmd(command, me.targets.rc.path, '$(RC)')
            }

        } else if (me.generating == 'sh') {
            if (gen.linker != '') {
                command = rep(command, gen.linker, '${LDFLAGS}')
            }
            for each (word in minimalCflags) {
                command = rep(command, word + ' ', ' ')
            }
            if (gen.defines != '') {
                command = rep(command, gen.defines, '${DFLAGS}')
            } else {
                command = rep(command, ' -c ', ' -c ${DFLAGS} ')
            }
            if (gen.compiler != '') {
                command = rep(command, gen.compiler, '${CFLAGS}')
            } else {
                command = rep(command, ' -c ', ' -c ${CFLAGS} ')
            }
            if (gen.linker != '') {
                command = rep(command, gen.linker, '${LDFLAGS}')
            }
            command = rep(command, gen.libpaths, '${LIBPATHS}')
            command = rep(command, gen.includes, '${IFLAGS}')
            /* Twice because libraries are repeated and replace only changes the first occurrence */
            command = rep(command, gen.libraries, '${LIBS}')
            command = rep(command, gen.libraries, '${LIBS}')
            command = rep(command, RegExp(gen.configuration, 'g'), '$${CONFIG}')
            if (me.targets.compiler) {
                command = repCmd(command, me.targets.compiler.path, '${CC}')
            }
            if (me.targets.link) {
                command = repCmd(command, me.targets.link.path, '${LD}')
            }
            for each (word in minimalCflags) {
                command = rep(command, word + ' ', ' ')
            }
        }
        if (me.generating == 'nmake') {
            command = rep(command, '_DllMainCRTStartup@12', '$(ENTRY)')
        }
        command = rep(command, RegExp(me.dir.top + '/', 'g'), '')
        command = rep(command, /  */g, ' ')
        if (me.generating == 'nmake') {
            command = rep(command, /\//g, '\\')
        }
        return command
    }

    /*
        Replace with variables where possible.
        Replaces the top directory and the CONFIGURATION
     */
    function repvar(command: String): String {
        command = command.replace(RegExp(me.dir.top + '/', 'g'), '')
        if (me.generating == 'make') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (me.generating == 'nmake') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (me.generating == 'sh') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$${CONFIG}')
        }
        for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
            if (me.prefixes[p]) {
                if (me.platform.like == 'windows') {
                    let pat = me.prefixes[p].windows.replace(/\\/g, '\\\\')
                    command = command.replace(RegExp(pat, 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
                }
                command = command.replace(RegExp(me.prefixes[p], 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
            }
        }
        command = command.replace(/\/\//g, '$$(ME_ROOT_PREFIX)/')
        return command
    }

    function repvar2(command: String, home: Path? = null): String {
        if (home) {
            command = command.replace(RegExp(me.dir.top, 'g'), me.dir.top.relativeTo(home))
        }
        if (home && me.platform.like == 'windows' && me.generating == 'nmake') {
            let re = RegExp(me.dir.top.windows.name.replace(/\\/g, '\\\\'), 'g')
            command = command.replace(re, me.dir.top.relativeTo(home).windows)
        }
        if (me.generating == 'make') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (me.generating == 'nmake') {
            command = command.replace(RegExp(gen.configuration + '\\\\bin/', 'g'), '$$(CONFIG)\\bin\\')
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (me.generating == 'sh') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$${CONFIG}')
        }
        for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
            if (gen[p]) {
                if (me.platform.like == 'windows') {
                    let pat = gen[p].windows.replace(/\\/g, '\\\\')
                    command = command.replace(RegExp(pat, 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
                }
                command = command.replace(RegExp(gen[p], 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
            }
        }
        command = command.replace(/\/\//g, '$$(ME_ROOT_PREFIX)/')
        return command
    }

    function reppath(path: Path): String {
        path = path.relative
        if (me.platform.like == 'windows') {
            path = (me.generating == 'nmake') ? path.windows : path.portable
        } else if (Config.OS == 'windows' && me.generating && me.generating != 'nmake')  {
            path = path.portable 
        }
        return repvar(path)
    }

    public function gencmd(s) {
        if (me.target) {
            s = repvar2(s, me.target.home)
        } else {
            s = repvar2(s, me.dirs.top)
        }
        if (capture) {
            capture.push(s)
        } else {
            /* Coming here for builtins like clean: */
            genout.writeLine('\t' + s)
        }
    }

    function findLib(libraries, lib) {
        let name
        if (libraries) {
            if (libraries.contains(lib)) {
                name = lib
            } else if (libraries.contains(Path(lib).trimExt())) {
                name = lib.trimExt()
            } else if (libraries.contains(Path(lib.replace(/^lib/, '')).trimExt())) {
                name = Path(lib.replace(/^lib/, '')).trimExt()
            }
        }
        return name
    }

    function getLib(lib) {
        if (dep = me.targets['lib' + lib]) {
            return dep

        } else if (dep = me.targets[lib]) {
            return dep

        } else if (dep = me.targets[Path(lib).trimExt()]) {
            /* Permits full library */
            return dep
        }
        return null
    }

    var nextID: Number = 0

    function getTargetLibs(target)  {
        return ' $(LIBS_' + nextID + ')'
    }

    function genTargetLibs(target, command): String {
        let found
        /* This makes matching easier */
        command += ' '

        /*
            Search the target libraries to find what configurable targets they require.
         */
        for each (lib in target.libraries) {
            let name, dep, ifdef, component
            name = component = null
            if (me.targets.compiler.libraries.contains(lib)) {
                continue
            }
            dep = getLib(lib)
            if (dep && !dep.configurable) {
                name = dep.name
                ifdef = dep.ifdef
                if (me.platform.os == 'vxworks' && !target.static) {
                    continue
                }
            } else {
                /*
                    Check components that provide the library
                 */
                for each (p in me.targets) {
                    if (!p.configurable) continue
                    /* Own libraries are the libraries defined by a target, but not inherited from dependents */
                    name = findLib(p.ownLibraries, lib)
                    if (name) {
                        ifdef = (target.ifdef) ? target.ifdef.clone() : []
                        if (!ifdef.contains(p.name)) {
                            ifdef.push(p.name)
                        }
                        component = p
                        break
                    }
                }
            }
            if (name) {
                if (me.platform.os == 'windows') {
                    lib = lib.replace(/^lib/, '').replace(/\.lib$/, '')
                }
                if (ifdef) {
                    let indent = ''
                    for each (r in ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genout.writeLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                            } else {
                                genout.writeLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                            }
                            indent = '    '
                        }
                    }
                    if (me.platform.os == 'windows') {
                        genout.writeLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                        if (component) {
                            for each (path in component.libpaths) {
                                if (path != me.dir.bin) {
                                    genout.writeLine('LIBPATHS_' + nextID + ' = $(LIBPATHS_' + nextID + ') -libpath:' + path)
                                    command = command.replace('"-libpath:' + path.windows + '"', '')
                                }
                            }
                        }
                    } else {
                        genout.writeLine(indent + 'LIBS_' + nextID + ' += -l' + lib)
                        if (component) {
                            for each (path in component.libpaths) {
                                if (path != me.dir.bin) {
                                    genout.writeLine(indent + 'LIBPATHS_' + nextID + ' += -L' + path)
                                    command = command.replace('-L' + path, '')
                                }
                            }
                        }
                    }
                    for each (r in ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genout.writeLine('!ENDIF')
                            } else {
                                genout.writeLine('endif')
                            }
                        }
                    }
                } else {
                    if (me.platform.os == 'windows') {
                        genout.writeLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                    } else {
                        genout.writeLine('LIBS_' + nextID + ' += -l' + lib)
                    }
                }
                found = true
                if (me.platform.os == 'windows') {
                    command = command.replace(RegExp(' lib' + lib + '.lib ', 'g'), ' ')
                    command = command.replace(RegExp(' ' + lib + '.lib ', 'g'), ' ')
                    command = command.replace(RegExp(' ' + lib + ' ', 'g'), ' ')
                } else {
                    command = command.replace(RegExp(' -l' + lib + ' ', 'g'), ' ')
                }
            } else {
                if (me.platform.os == 'windows') {
                    command = command.replace(RegExp(' lib' + lib + '.lib ', 'g'), ' ')
                } else {
                    /* Leave as is */
                    // command = command.replace(RegExp(' -l' + lib + ' ', 'g'), ' ')
                }
            }
        }
        if (found) {
            genout.writeLine('')
            if (command.contains('$(LIBS)')) {
                command = command.replace('$(LIBS)', '$(LIBPATHS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS)')
            } else {
                command += ' $(LIBPATHS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS_' + nextID + ')'
            }
        }
        return command
    }

    function getAllDeps(top, target, result = []) {
        for each (dname in (target.depends + target.uses)) {
            if (dname == target.name) {
                continue
            }
            if (!result.contains(dname)) {
                let dep = me.targets[dname]
                if (dep && dep.enable) {
                    getAllDeps(top, dep, result)
                }
                if (!dep || !dep.configurable) {
                    result.push(dname)
                }
            }
        }
        return result
    }

    function getDepsVar(target)  {
        return ' $(DEPS_' + nextID + ')'
    }

    /*
        Get the dependencies of a target as a string
     */
    function genTargetDeps(target) {
        nextID++
        genout.writeLine('#\n#   ' + Path(target.name).basename + '\n#')
        let found
        if (target.type == 'file' || target.type == 'script') {
            for each (file in target.files) {
                if (me.platform.os == 'windows') {
                    genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + reppath(file))
                } else {
                    genout.writeLine('DEPS_' + nextID + ' += ' + reppath(file))
                }
                found = true
            }
        }
        let depends = getAllDeps(target, target)
        for each (let dname in depends) {
            dep = b.getDep(dname)
            if (dep && dep.enable) {
                let d = (dep.path) ? reppath(dep.path) : dep.name
                if (dep.ifdef) {
                    let indent = ''
                    for each (r in dep.ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genout.writeLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                            } else {
                                genout.writeLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                            }
                            indent = '    '
                        }
                    }
                    if (me.platform.os == 'windows') {
                        genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                    } else {
                        genout.writeLine(indent + 'DEPS_' + nextID + ' += ' + d)
                    }
                    for each (r in dep.ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genout.writeLine('!ENDIF')
                            } else {
                                genout.writeLine('endif')
                            }
                        }
                    }
                } else {
                    if (me.platform.os == 'windows') {
                        genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                    } else {
                        genout.writeLine('DEPS_' + nextID + ' += ' + d)
                    }
                }
                found = true
            }
        }
        if (found) {
            genout.writeLine('')
        }
    }

    /** 
        Generate a trace line.
        @param tag Informational tag emitted before the message
        @param args Message args to display
     */
    public function gtrace(tag: String, ...args): Void {
        let msg = args.join(" ")
        let msg = "\t@echo '%12s %s'" % (["[" + tag + "]"] + [msg]) + "\n"
        genout.write(repvar(msg))
    }

    /** @hide */
    public function genWriteLine(str) {
        genout.writeLine(repvar(str))
    }

    /** @hide */
    public function genWrite(str) {
        genout.write(repvar(str))
    }

    public function genOpen(path) {
        genout = TextStream(File(path, 'w'))
    }

    public function genClose() {
        genout.close()
    }

    /** @hide */
    public function genScript(str: String) {
        capture.push(str)
    }

    function like(os) {
        if (unix.contains(os)) {
            return "unix"
        } else if (windows.contains(os)) {
            return "windows"
        }
        return ""
    }

} /* me module */

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
