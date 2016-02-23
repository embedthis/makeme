/*
    Make.es -- Generate Makefiles

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

class Make {

    var builder: Builder
    var generating: String?
    var gen: Object
    var loader: Loader
    var options: Object

    public var minimalCflags = [
        '-O2', '-O', '-w', '-g', '-Wall', '-Wno-deprecated-declarations', '-Wno-unused-result',
        '-Wshorten-64-to-32', '-Wunreachable-code', '-mtune=generic']

    var targetsToClean = { exe: true, file: true, lib: true, obj: true }

    public function Make() {
        loader = makeme.loader
        builder = makeme.builder
        options = makeme.options
    }

    public function generate(base: Path, kind) {
        generating = kind
        if (kind == 'make') {
            generateMakeProject(base)

        } else if (kind == 'nmake') {
            generateNmakeProject(base)

        } else if (kind == 'shell') {
            generateNmakeProject(base)
        }
    }

    /*
        Generate conditional definitions for component targets
        TODO - this should really work recursively and go N deep
     */
    function componentDefs() {
        let needed = {}
        for each (target in me.targets) {
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
        Object.sortProperties(me.targets)

        /*
            Emit ME_COM_* definitions
         */
        for each (let target in me.targets) {
            if (!target.configurable) {
                continue
            }
            let name = target.name
            if (needed[name]) {
                let enable = target.enable
                if (me.platform.os == 'windows' || me.platform.os == 'vxworks') {
                    if (target.name == 'ssl' || target.name == 'openssl') {
                        enable = false
                    }
                }
                if (me.platform.os == 'windows' ) {
                    genWriteLine('!IF "$(ME_COM_' + name.toUpper() + ')" == ""')
                    genWriteLine('%-21s = %s'.format(['ME_COM_' + name.toUpper(), enable ? 1 : 0]))
                    genWriteLine('!ENDIF')
                } else {
                    genWriteLine('%-21s ?= %s'.format(['ME_COM_' + name.toUpper(), enable ? 1 : 0]))
                }
            }
        }
        genWriteLine('')

        /*
            Emit defines
         */
        let defined = {}
        for each (let target in me.targets) {
            if (!target.configurable) {
                continue
            }
            let name = target.name
            if (needed[name]) {
                for each (define in target.defines) {
                    let [key,value] = define.split('=')
                    if (!key.contains('ME_COM')) continue
                    value ||= ''
                    if (!defined[key]) {
                        if (me.platform.os == 'windows' ) {
                            genWriteLine('!IF "$(ME_COM_' + name.toUpper() + ')" == ""')
                            genWriteLine('%-21s = %s'.format([key, '"' + value + '"']))
                            genWriteLine('!ENDIF')
                        } else {
                            genWriteLine('%-21s ?= %s'.format([key, '"' + value + '"']))
                        }
                        defined[key] = true
                    }
                }
            }
        }
        genWriteLine('')

        /*
            Emit configurable definitions
         */
        for each (let target in me.targets) {
            if (!target.configurable) continue
            let name = target.name
            if (needed[name] && target.ifdef) {
                if (me.platform.os == 'windows' ) {
                    for each (r in target.ifdef) {
                        genWriteLine('!IF "$(ME_COM_' + name.toUpper() + ')" == "1"')
                        genWriteLine('%-21s = 1'.format(['ME_COM_' + r.toUpper()]))
                        genWriteLine('!ENDIF\n')
                    }
                } else {
                    for each (r in target.ifdef) {
                        genWriteLine('ifeq ($(ME_COM_' + name.toUpper() + '),1)')
                        for each (r in target.ifdef) {
                            genWriteLine('    ME_COM_' + r.toUpper() + ' := 1')
                        }
                        genWriteLine('endif')
                    }
                }
            }
        }

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
                                genWriteLine('!IF "$(ME_COM_' + r.toUpper() + ')" == ""')
                                seenItem = true
                            }
                            genWriteLine('%-21s = 1'.format(['ME_COM_' + r.toUpper()]))
                        }
                    }
                    if (seenItem) {
                        genWriteLine('!ENDIF\n')
                    }
                } else {
                    for each (r in target.depends) {
                        if (me.targets[r] && me.targets[r].configurable) {
                            if (!seenItem) {
                                genWriteLine('ifeq ($(ME_COM_' + name.toUpper() + '),1)')
                                seenItem = true
                            }
                            genWriteLine('    ME_COM_' + r.toUpper() + ' := 1')
                        }
                    }
                    if (seenItem) {
                        genWriteLine('endif')
                    }
                }
            }
        }
        genWriteLine('')

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

    /*
        Generate environment variable defintions
     */
    function environment() {
        let found
        if (me.platform.os == 'windows') {
            var winsdk = (me.targets.winsdk && me.targets.winsdk.path) ?
                me.targets.winsdk.path.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft SDKs\\Windows\\v6.1'
            var vs = (me.targets.compiler && me.targets.compiler.dir) ?
                me.targets.compiler.dir.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft Visual Studio 9.0'
            if (generating == 'make') {
                /* Not used */
                genWriteLine('VS             := ' + '$(VSINSTALLDIR)')
                genWriteLine('VS             ?= ' + vs)
                genWriteLine('SDK            := ' + '$(WindowsSDKDir)')
                genWriteLine('SDK            ?= ' + winsdk)
                genWriteLine('\nexport         SDK VS')
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
            if (generating == 'make') {
                genWriteLine('export %-14s ?= %s' % [key, value])

            } else if (generating == 'nmake') {
                value = value.replace(/\//g, '\\')
                genWriteLine('%-9s = %s' % [key, value])

            } else if (generating == 'sh') {
                genWriteLine('export ' + key + '="' + value + '"')
            }
            found = true
        }
        if (me.platform.os == 'vxworks') {
            genWriteLine('%-21s := %s'.format(['export PATH', '$(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)']))
        }
        if (found) {
            genWriteLine('')
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

    function generateDir(target, solo = false) {
        if (target.mkdir) {
            if (generating == 'sh') {
                makeDir(target.mkdir)

            } else if (generating == 'make' || generating == 'nmake') {
                if (solo) {
                    genTargetDeps(target)
                    let path = target.path
                    genWrite(reppath(path) + ':' + getDepsVar() + '\n')
                }
                makeDir(target.mkdir)
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
        let command = builder.expandRule(target, rule)
        if (generating == 'sh') {
            command = repcmd(command)
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genWriteLine(command)

        } else if (generating == 'make' || generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genWrite(reppath(target.path) + ':' + getDepsVar() + '\n')
            genPathTrace('Link', target.path.natural.relative)
            generateDir(target)
            if (generating == 'nmake') {
                genWriteLine('\t' + command + ' $(LOG)')
            } else {
                genWriteLine('\t' + command)
            }
        }
    }

    public function generateMakeProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.mk')
        let path = base.joinExt('mk')
        genOpen(path)
        genWriteLine('#\n#   ' + path.basename + ' -- Makefile to build ' +
            me.settings.title + ' for ' + me.platform.os + '\n#\n')
        loader.runScript('pregen')
        genWriteLine('NAME                  := ' + me.settings.name)
        genWriteLine('VERSION               := ' + me.settings.version)
        genWriteLine('PROFILE               ?= ' + me.platform.profile)
        if (me.platform.os == 'vxworks') {
            genWriteLine("ARCH                  ?= $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*//')")
            genWriteLine("CPU                   ?= $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))")
        } else {
            genWriteLine('ARCH                  ?= $(shell uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\')')
            genWriteLine('CC_ARCH               ?= $(shell echo $(ARCH) | sed \'s/x86/i686/;s/x64/x86_64/\')')
        }
        genWriteLine('OS                    ?= ' + me.platform.os)
        genWriteLine('CC                    ?= ' + me.targets.compiler.path)
        if (me.targets.link) {
            genWriteLine('LD                    ?= ' + me.targets.link.path)
        }
        genWriteLine('CONFIG                ?= $(OS)-$(ARCH)-$(PROFILE)')
        genWriteLine('BUILD                 ?= ' + loader.BUILD + '/$(CONFIG)')
        genWriteLine('LBIN                  ?= $(BUILD)/bin')
        genWriteLine('PATH                  := $(LBIN):$(PATH)\n')

        let dflags = componentDefs()
        environment()

        let mappings = makeme.generate.mappings
        let cflags = mappings.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags += ' -w'
        genWriteLine('CFLAGS                += ' + cflags.trim())
        genWriteLine('DFLAGS                += ' + mappings.defines.replace(/-DME_DEBUG +/, '') +
            ' $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) ' + dflags)
        genWriteLine('IFLAGS                += "' +
            repvar(me.targets.compiler.includes.map(function(path) '-I' + reppath(path.relative)).join(' ')) + '"')
        let linker = me.targets.compiler.linker.map(function(s) "'" + s + "'").join(' ')
        let ldflags = repvar(linker).replace(/\$ORIGIN/g, '$$$$ORIGIN').replace(/'-g' */, '')
        genWriteLine('LDFLAGS               += ' + ldflags)
        genWriteLine('LIBPATHS              += ' + repvar(mappings.libpaths))
        genWriteLine('LIBS                  += ' + mappings.libraries + '\n')

        genWriteLine('DEBUG                 ?= ' + (me.settings.debug ? 'debug' : 'release'))
        genWriteLine('CFLAGS-debug          ?= -g')
        genWriteLine('DFLAGS-debug          ?= -DME_DEBUG')
        genWriteLine('LDFLAGS-debug         ?= -g')
        genWriteLine('DFLAGS-release        ?= ')
        genWriteLine('CFLAGS-release        ?= -O2')
        genWriteLine('LDFLAGS-release       ?= ')
        genWriteLine('CFLAGS                += $(CFLAGS-$(DEBUG))')
        genWriteLine('DFLAGS                += $(DFLAGS-$(DEBUG))')
        genWriteLine('LDFLAGS               += $(LDFLAGS-$(DEBUG))\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name == 'root' && value == '/') {
                value = ''
            }
            genRawWriteLine('%-21s ?= %s'.format(['ME_' + name.toUpper() + '_PREFIX', value]))
        }
        genWriteLine('')
        loader.runScript('gencustom')
        genWriteLine('')

        let pop = me.settings.name + '-' + me.platform.os + '-' + me.platform.profile
        genTargets()

        genWriteLine('unexport CDPATH\n')
        genWriteLine('ifndef SHOW\n.SILENT:\nendif\n')
        genWriteLine('all build compile: prep $(TARGETS)\n')
        genWriteLine('.PHONY: prep\n\nprep:')
        genWriteLine('\t@echo "      [Info] Use "make SHOW=1" to trace executed commands."')
        genWriteLine('\t@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi')
        if (me.prefixes.app) {
            genWriteLine('\t@if [ "$(ME_APP_PREFIX)" = "" ] ; then echo WARNING: ME_APP_PREFIX not set ; exit 255 ; fi')
        }
        if (me.platform.os == 'vxworks') {
            genWriteLine('\t@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi')
            genWriteLine('\t@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi')
            genWriteLine('\t@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi')
        }
        genWriteLine('\t@[ ! -x $(BUILD)/bin ] && ' + 'mkdir -p $(BUILD)/bin; true')
        genWriteLine('\t@[ ! -x $(BUILD)/inc ] && ' + 'mkdir -p $(BUILD)/inc; true')
        genWriteLine('\t@[ ! -x $(BUILD)/obj ] && ' + 'mkdir -p $(BUILD)/obj; true')
        if (me.dir.inc.join('me.h').exists) {
            genWriteLine('\t@[ ! -f $(BUILD)/inc/me.h ] && ' + 'cp projects/' + pop + '-me.h $(BUILD)/inc/me.h ; true')
            genWriteLine('\t@if ! diff $(BUILD)/inc/me.h projects/' + pop + '-me.h >/dev/null ; then\\')
            genWriteLine('\t\tcp projects/' + pop + '-me.h $(BUILD)/inc/me.h  ; \\')
            genWriteLine('\tfi; true')
        }
        genWriteLine('\t@if [ -f "$(BUILD)/.makeflags" ] ; then \\')
        genWriteLine('\t\tif [ "$(MAKEFLAGS)" != "`cat $(BUILD)/.makeflags`" ] ; then \\')
        genWriteLine('\t\t\techo "   [Warning] Make flags have changed since the last build" ; \\')
        genWriteLine('\t\t\techo "   [Warning] Previous build command: \"`cat $(BUILD)/.makeflags`\"" ; \\')
        genWriteLine('\t\tfi ; \\')
        genWriteLine('\tfi')
        genWriteLine('\t@echo "$(MAKEFLAGS)" >$(BUILD)/.makeflags\n')

        genWriteLine('clean:')
        builtin('cleanTargets')
        genWriteLine('\nclobber: clean\n\trm -fr ./$(BUILD)\n')
        builder.build(['gen'])
        genClose()
    }

    public function generateNmakeProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.nmake')
        let mappings = makeme.generate.mappings
        let path = base.joinExt('nmake')
        genOpen(path)
        genWriteLine('#\n#   ' + path.basename + ' -- Makefile to build ' + me.settings.title +
            ' for ' + me.platform.os + '\n#\n')
        loader.runScript('pregen')
        genWriteLine('NAME                  = ' + me.settings.name)
        genWriteLine('VERSION               = ' + me.settings.version + '\n')
        genWriteLine('OS                    = ' + me.platform.os)
        genWriteLine('PA                    = $(PROCESSOR_ARCHITECTURE)')

        genWriteLine('!IF "$(PROFILE)" == ""')
        genWriteLine('PROFILE               = ' + me.platform.profile)
        genWriteLine('!ENDIF\n')

        genWriteLine('')
        genWriteLine('!IF "$(PA)" == "AMD64"')
            genWriteLine('ARCH                  = x64')
            genWriteLine('ENTRY                 = _DllMainCRTStartup')
        genWriteLine('!ELSE')
            genWriteLine('ARCH                  = x86')
            genWriteLine('ENTRY                 = _DllMainCRTStartup@12')
        genWriteLine('!ENDIF\n')

        genWriteLine('!IF "$(CONFIG)" == ""')
        genWriteLine('CONFIG                = $(OS)-$(ARCH)-$(PROFILE)')
        genWriteLine('!ENDIF\n')

        genWriteLine('!IF "$(BUILD)" == ""')
        genWriteLine('BUILD                 = ' + loader.BUILD + '\\$(CONFIG)')
        genWriteLine('!ENDIF\n')

        genWriteLine('LBIN                  = $(BUILD)\\bin\n')

        let dflags = componentDefs()
        genWriteLine('CC                    = cl')
        genWriteLine('LD                    = link')
        genWriteLine('RC                    = rc')
        genWriteLine('CFLAGS                = ' + mappings.compiler)
        genWriteLine('DFLAGS                = ' + mappings.defines + ' ' + dflags)
        genWriteLine('IFLAGS                = ' +
            repvar(me.targets.compiler.includes.map(function(path) '-I' + reppath(path)).join(' ')))
        genWriteLine('LDFLAGS               = ' + repvar(mappings.linker).replace(/-machine:x86/, '-machine:$$(ARCH)'))
        genWriteLine('LIBPATHS              = ' + repvar(mappings.libpaths).replace(/\//g, '\\'))
        genWriteLine('LIBS                  = ' + mappings.libraries + '\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name.startsWith('programFiles')) continue
            /* TODO value.windows will change C:/ to C: */
            if (name == 'root') {
                value = value.trimEnd('/')
            } else {
                value = value.map('\\')
            }
            genWriteLine('%-21s = '.format(['ME_' + name.toUpper() + '_PREFIX']) + value)
        }
        genWriteLine('')
        loader.runScript('gencustom')
        genWriteLine('')

        genTargets()
        let pop = me.settings.name + '-' + me.platform.os + '-' + me.platform.profile
        genWriteLine('!IFNDEF SHOW\n.SILENT:\n!ENDIF\n')
        genWriteLine('all build compile: prep $(TARGETS)\n')
        genWriteLine('.PHONY: prep\n\nprep:')
        genWriteLine('!IF "$(VSINSTALLDIR)" == ""\n\techo "Visual Studio vars not set. Run vcvars.bat."\n\texit 255\n!ENDIF')
        if (me.prefixes.app) {
            genWriteLine('!IF "$(ME_APP_PREFIX)" == ""\n\techo "ME_APP_PREFIX not set."\n\texit 255\n!ENDIF')
        }
        genWriteLine('\t@if not exist $(BUILD)\\bin md $(BUILD)\\bin')
        genWriteLine('\t@if not exist $(BUILD)\\inc md $(BUILD)\\inc')
        genWriteLine('\t@if not exist $(BUILD)\\obj md $(BUILD)\\obj')
        if (me.dir.inc.join('me.h').exists) {
            genWriteLine('\t@if not exist $(BUILD)\\inc\\me.h ' + 'copy projects\\' + pop + '-me.h $(BUILD)\\inc\\me.h\n')
        }
        genWriteLine('!IF "$(SHOW)" != ""')
        genTrace('Info', 'Use "make SHOW=1" to trace executed commands and errors.')
        genWriteLine('LOG =')
        genWriteLine('!ELSE')
        genWriteLine('LOG = >nul')
        genWriteLine('!ENDIF\n')

        genWriteLine('clean:')
        builtin('cleanTargets')
        genWriteLine('')
        builder.build()
        genClose()
    }

    public function generateShellProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.sh')
        let path = base.joinExt('sh')
        genOpen(path)
        genWriteLine('#\n#   ' + path.basename + ' -- MakeMe Shell Script to build ' + me.settings.title + '\n#\n')
        environment()
        genWriteLine('NAME="' + me.settings.name + '"')
        genWriteLine('VERSION="' + me.settings.version + '"')
        genWriteLine('PROFILE="' + me.platform.profile + '"')
        genWriteLine('ARCH="' + me.platform.arch + '"')
        genWriteLine('ARCH="`uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\'`"')
        genWriteLine('OS="' + me.platform.os + '"')
        genWriteLine('CONFIG="${OS}-${ARCH}-${PROFILE}' + '"')
        genWriteLine('BUILD="' + loader.BUILD + '/${CONFIG}')
        genWriteLine('CC="' + me.targets.compiler.path + '"')
        if (me.targets.link) {
            genWriteLine('LD="' + me.targets.link.path + '"')
        }
        let mappings = makeme.generate.mappings
        let cflags = mappings.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags += ' -w'
        genWriteLine('CFLAGS="' + cflags.trim() + '"')
        genWriteLine('DFLAGS="' + mappings.defines + '"')
        genWriteLine('IFLAGS="' +
            repvar(me.targets.compiler.includes.map(function(path) '-I' + path.relative).join(' ')) + '"')
        genWriteLine('LDFLAGS="' + repvar(mappings.linker).replace(/\$ORIGIN/g, '\\$$ORIGIN') + '"')
        genWriteLine('LIBPATHS="' + repvar(mappings.libpaths) + '"')
        genWriteLine('LIBS="' + mappings.libraries + '"\n')
        genWriteLine('[ ! -x ${BUILD}/inc ] && ' + 'mkdir -p ${BUILD}/inc\n')
        genWriteLine('[ ! -x ${BUILD}/bin ] && ' + 'mkdir -p ${BUILD}/bin\n')
        genWriteLine('[ ! -x ${BUILD}/obj ] && ' + 'mkdir -p ${BUILD}/obj\n')
        if (me.dir.inc.join('me.h').exists) {
            genWriteLine('[ ! -f ${BUILD}/inc/me.h ] && ' +
                'cp projects/' + me.settings.name + '-${OS}-${PROFILE}-me.h ${BUILD}/inc/me.h')
            genWriteLine('if ! diff ${BUILD}/inc/me.h projects/' + me.settings.name +
                '-${OS}-${PROFILE}-me.h >/dev/null ; then')
        }
        genWriteLine('\tcp projects/' + me.settings.name + '-${OS}-${PROFILE}-me.h ${BUILD}/inc/me.h')
        genWriteLine('fi\n')
        builder.build()
        genClose()
        path.setAttributes({permissions: 0755})
    }

    public function generateTarget(target) {
        if (target.type == 'component' && !target.generate) {
            return
        }
        global.TARGET = me.target = target
        if (target.ifdef) {
            for each (r in target.ifdef) {
                if (me.platform.os == 'windows') {
                    genWriteLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                } else {
                    genWriteLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                }
            }
        }
        if (target.configurable) {
            if (me.platform.os == 'windows') {
                genWriteLine('!IF "$(ME_COM_' + target.name.toUpper() + ')" == "1"')
            } else {
                genWriteLine('ifeq ($(ME_COM_' + target.name.toUpper() + '),1)')
            }
        }
        target.prorDefines = target.defines
        if (target.defines) {
            target.defines = target.defines.map(function(e) {
                let [key,value] = e.split('=')
                if (key.contains('ME_COM')) {
                    /*
                        Ignore the value as the key will be a definition at the top of the Makefile
                        Quotes should be added above in the definition
                     */
                    return key + '=$(' + key + ')'
                } else {
                    return e
                }
            })
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
        } else if (target.mkdir) {
            generateDir(target, true)
        } else if (target.generate) {
            generateScript(target)
        }
        if (target.configurable) {
            if (me.platform.os == 'windows') {
                genWriteLine('!ENDIF')
            } else {
                genWriteLine('endif')
            }
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
        target.defines = target.priorDefines
        global.TARGET = me.target = null
    }

    public function main() {
        let cfg = Path('configure')
        if (cfg.exists && !options.overwrite) {
            trace('Exists', 'configure')
        } else {
            let data = '#!/bin/bash\n#\n#   configure -- Configure for building\n#\n' +
                'if ! type me >/dev/null 2>&1 ; then\n' +
                    '    echo -e "\\nInstall the \\"me\\" tool for configuring." >&2\n' +
                    '    echo -e "Download from: https://embedthis.com/downloads/me/download.ejs." >&2\n' +
                    '    echo -e "Or skip configuring and make a standard build using \\"make\\".\\n" >&2\n' +
                    '    exit 255\n' +
                'fi\n' +
                'me configure "$@"'
            trace(cfg.exists ? 'Overwrite' : 'Create', cfg)
            cfg.write(data)
            cfg.setAttributes({permissions: 0755})
        }
        safeCopy(Config.Bin.join('master-main.me'), Loader.MAIN)
    }

    function mapPrefixes() {
        prefixes = {}
        let root = me.prefixes.root
        let base = me.prefixes.base
        let app = me.prefixes.app
        let vapp = me.prefixes.vapp
        for (let [name,value] in me.prefixes) {
            if (name.startsWith('programFiles')) continue
            value = loader.expand(value).replace(/\/\//g, '/')
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

    public function start() {
        safeCopy(Config.Bin.join('master-start.me'), 'start.me')
    }

    function genTargets() {
        builder.selectTargets('gen')
        let topTargets = builder.topTargets

        let all = []
        for each (target in topTargets) {
            let path = target.modify || target.path
            if (path && target.enable && target.generate) {
                if (target.ifdef) {
                    for each (pname in target.ifdef) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!IF "$(ME_COM_' + pname.toUpper() + ')" == "1"')
                        } else {
                            genWriteLine('ifeq ($(ME_COM_' + pname.toUpper() + '),1)')
                        }
                    }
                    if (me.platform.os == 'windows') {
                        genWriteLine('TARGETS               = $(TARGETS) ' + reppath(path))
                    } else {
                        genWriteLine('    TARGETS           += ' + reppath(path))
                    }
                    for (i in target.ifdef.length) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!ENDIF')
                        } else {
                            genWriteLine('endif')
                        }
                    }
                } else {
                    if (me.platform.os == 'windows') {
                        genWriteLine('TARGETS               = $(TARGETS) ' + reppath(path))
                    } else {
                        genWriteLine('TARGETS               += ' + reppath(path))
                    }
                }
            }
        }
        genWriteLine('')
    }

    function generateSharedLib(target) {
        let transition = target.rule || 'shlib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = builder.expandRule(target, rule)
        if (generating == 'sh') {
            command = repcmd(command)
            genWriteLine(command)

        } else if (generating == 'make' || generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
            genWrite(reppath(target.path) + ':' + getDepsVar() + '\n')
            genPathTrace('Link', target.path.natural.relative)
            generateDir(target)
            if (generating == 'nmake') {
                genWriteLine('\t' + command + ' $(LOG)')
            } else {
                genWriteLine('\t' + command)
            }
        }
    }

    function generateStaticLib(target) {
        let transition = target.rule || 'lib'
        let rule = me.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = builder.expandRule(target, rule)
        if (generating == 'sh') {
            command = repcmd(command)
            genWriteLine(command)

        } else if (generating == 'make' || generating == 'nmake') {
            command = repcmd(command)
            genTargetDeps(target)
            genWrite(reppath(target.path) + ':' + getDepsVar() + '\n')
            genPathTrace('Link', target.path.natural.relative)
            generateDir(target)
            if (generating == 'nmake') {
                genWriteLine('\t' + command + ' $(LOG)')
            } else {
                genWriteLine('\t' + command)
            }
        }
    }

    /*
        Build symbols file for windows libraries
     */
    function generateSym(target) {
        throw 'Not supported to generate sym targets yet'
    }

    /*
        Build an object from source
     */
    function generateObj(target) {
        builder.runTargetScript(target, 'precompile')

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
            let command = builder.expandRule(target, rule)
            if (generating == 'sh') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genWriteLine(command)

            } else if (generating == 'make') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genTargetDeps(target)
                genWrite(reppath(target.path) + ': \\\n    ' + file.relative + getDepsVar() + '\n')
                genPathTrace('Compile', target.path.natural.relative)
                generateDir(target)
                genWriteLine('\t' + command)

            } else if (generating == 'nmake') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '-arch $$(CC_ARCH) ')
                genTargetDeps(target)
                genWrite(reppath(target.path) + ': \\\n    ' + file.relative.windows + getDepsVar() + '\n')
                genPathTrace('Compile', target.path.natural.relative)
                generateDir(target)
                genWriteLine('\t' + command + ' $(LOG)')
            }
        }
        builder.runTargetScript(target, 'postcompile')
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
            let command = builder.expandRule(target, rule)
            if (generating == 'sh') {
                command = repcmd(command)
                genWriteLine(command)

            } else if (generating == 'make') {
                command = repcmd(command)
                genTargetDeps(target)
                genWrite(reppath(target.path) + ': \\\n        ' + file.relative + getDepsVar() + '\n')
                genPathTrace('Compile', target.path.natural.relative)
                generateDir(target)
                genWriteLine('\t' + command)

            } else if (generating == 'nmake') {
                command = repcmd(command)
                genTargetDeps(target)
                genWrite(reppath(target.path) + ': \\\n        ' + file.relative.windows + getDepsVar() + '\n')
                genPathTrace('Compile', target.path.natural.relative)
                generateDir(target)
                genWriteLine('\t' + command)
            }
        }
    }

    /*
        Copy files[] to path
     */
    function generateFile(target) {
        if (generating == 'make' || generating == 'nmake') {
            genTargetDeps(target)
            if (target.modify) {
                genWrite(reppath(target.modify) + ':' + getDepsVar() + '\n')
            } else {
                genWrite(reppath(target.path) + ':' + getDepsVar() + '\n')
            }
        }
        let dest = target.path
        if (target.files.length > 0) {
            genPathTrace('Copy', dest.relative)
        }
        generateDir(target)
        copyFiles(target.files, dest, target)
        if (target.modify) {
            touchFile(target.modify.relativeTo(me.dir.top))
        }
    }

    function generateScript(target) {
        let prefix = ''
        let suffix = ''
        builder.setRuleVars(target, target.home)
        if (target.message) {
            let message = target.message
            if (message is Array) {
                tag = message[0]
                message = message.slice(1)
            } else {
                tag = 'Info'
            }
            message = repvar(loader.expand(message))
            if (generating == 'nmake') {
                message = '\t@echo ' + '.'.times(9 - tag.length) + ' [' + tag + '] ' + message
            } else {
                message = "echo '%12s %s'" % (["[" + tag + "]"] + [message])
            }
            if (generating == 'nmake') {
                message = message.replace(/\//g, '\\')
            }
        }
        let kind = makeme.generating
        if (!target.home.same('.')) {
            if (generating == 'sh' || generating == 'make') {
                prefix = 'cd ' + target.home.relative
                suffix = 'cd ' + me.dir.top.relativeTo(target.home)
            } else if (generating == 'nmake') {
                prefix = 'cd ' + target.home.relative.windows + '\n'
                suffix = '\ncd ' + me.dir.top.relativeTo(target.home).windows
            }
        }
        /*
            Strategy: If target.generate is true, then run script and capture commands
            otherwise, interpret generate* as a command. Where FORMAT is: make, nmake, sh, vs or xcode:
                generate-FORMAT-OS
                generate-FORMAT
                generate-make-OS
                generate-make
                generate-sh
                generate
         */
        let sh
        if (generating == 'make' | generating == 'sh' || generating == 'xcode') {
            sh = target['generate-sh']
        }
        cmd = target['generate-' + kind + '-' + me.platform.os] || target['generate-' + kind] ||
            target['generate-make-' + me.platform.os] || target['generate-make'] || sh || ''
        if (!cmd && target.generate is String) {
            cmd = target.generate
        }
        if (target.generate === true && !cmd) {
            genStartCapture(target)
            builder.runTargetScript(target, 'build')
            cmd = genStopCapture(target)
            if (cmd == '') {
                prefix = suffix = ''
            }
        } else {
            if (cmd && makeme.generating != 'nmake') {
                /* TODO - alternative would be to pipe cmd into a shell */
                cmd = cmd.trim().replace(/\n/mg, ' ; \\\n')
            }
        }

        cmd = '' + cmd
        if (generating == 'sh') {
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
                cmd = loader.expand(cmd, {missing: null}).expand(target.vars, {missing: true})
                cmd = repvar2(cmd, target.home)
                me.globals.LBIN = me.localBin
                genWriteLine(cmd)
            } else {
                genWrite('#  Omit build script ' + target.name + '\n')
            }

        } else if (generating == 'make') {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            if (cmd) {
                cmd = cmd.trim().replace(/^\s*/mg, '\t')
                if (prefix) {
                    cmd = '\t( \\\n\t' + prefix + '; \\\n' + cmd + ' ; \\\n\t)'
                }
                me.globals.LBIN = me.dir.top.relativeTo(target.home).join('$(LBIN)')
                cmd = loader.expand(cmd, {missing: true}).expand(target.vars, {missing: ''})
                cmd = repvar2(cmd, target.home)
                genWriteLine(cmd)
                me.globals.LBIN = me.localBin
            }

        } else if (generating == 'nmake') {
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

                me.globals.LBIN = me.dir.top.relativeTo(target.home).join('$(LBIN)').windows
                try {
                    cmd = loader.expand(cmd, {missing: null}).expand(target.vars, {missing: true})
                } catch (e) {
                    print('Target', target.name)
                    print('Script:', cmd)
                    throw e
                }
                cmd = repvar2(cmd, target.home)
                me.globals.LBIN = me.localBin
                genWriteLine(cmd)
            } else {
                genWrite('#  Omit build script ' + target.name + '\n')
            }
        }
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

    function getAllLibraries(base, target) {
        let libraries = []
        if (!target.enable && !target.ifdef) {
            return libraries
        }
        for each (dname in (target.depends + target.uses)) {
            let dep = builder.getDep(dname)
            if (dep) {
                libraries += getAllLibraries(base, dep)
            }
        }
        if (target.type == 'lib') {
            if (target != base) {
                libraries.push(target.libname || target.name)
            }
            if (target.libraries) {
                libraries += target.libraries
            }
        }
        return libraries
    }

    /*
        This should use the same strategy as genTargetDepItems and not look at target.libraries
     */
    function genTargetLibs(target, command): String {
        let found
        command += ' '

        let libraries = getAllLibraries(target, target).unique()

        /*
            Search the libraries to find what configurable targets they require.
         */
        for each (lib in libraries) {
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
                    if (p.libname == lib || p.libname == ('lib' + lib)) {
                        component = p
                        name = p.libname
                        if (p.configurable) {
                            ifdef = [p.name]
                        }
                        break
                    }
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
                lib = lib.replace(/^lib/, '').replace(/\.lib$/, '')
                if (ifdef) {
                    let indent = ''
                    for each (r in ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genWriteLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                            } else {
                                genWriteLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                            }
                            indent = '    '
                        }
                    }
                    if (dep && dep.configurable && (!target.ifdef || !target.ifdef.contains(dep.name))) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!IF "$(ME_COM_' + dep.name.toUpper() + ')" == "1"')
                        } else {
                            genWriteLine('ifeq ($(ME_COM_' + dep.name.toUpper() + '),1)')
                        }
                        indent = '    '
                    }
                    if (me.platform.os == 'windows') {
                        genWriteLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                        if (component) {
                            for each (path in component.libpaths) {
                                if (path != me.dir.bin) {
                                    genWriteLine('LIBPATHS_' + nextID + ' = $(LIBPATHS_' + nextID + ') -libpath:' + path)
                                    command = command.replace('"-libpath:' + path.windows + '"', '')
                                }
                            }
                        }
                    } else {
                        genWriteLine(indent + 'LIBS_' + nextID + ' += -l' + lib)
                        if (component) {
                            for each (path in component.libpaths) {
                                if (path != me.dir.bin) {
                                    genWriteLine(indent + 'LIBPATHS_' + nextID + ' += -L"' + path + '"')
                                    command = command.replace('-L' + path, '')
                                }
                            }
                        }
                    }
                    if (dep && dep.configurable && (!target.ifdef || !target.ifdef.contains(dep.name))) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!ENDIF')
                        } else {
                            genWriteLine('endif')
                        }
                    }
                    for each (r in ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genWriteLine('!ENDIF')
                            } else {
                                genWriteLine('endif')
                            }
                        }
                    }
                } else {
                    if (me.platform.os == 'windows') {
                        genWriteLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                    } else {
                        genWriteLine('LIBS_' + nextID + ' += -l' + lib)
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
            genWriteLine('')
            if (command.contains('$(LIBS)')) {
                command = command.replace('$(LIBS)',
                    '$(LIBPATHS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS)')
            } else {
                command += ' $(LIBPATHS_' + nextID + ') $(LIBS_' + nextID + ') $(LIBS_' + nextID + ')'
            }
        }
        return command
    }

    function getDepsVar(target)  {
        return ' $(DEPS_' + nextID + ')'
    }

    function genTargetDepItems(target, depends) {
        for each (let dname in depends) {
            dep = builder.getDep(dname)
            if (dep && (dep.enable || options.configurableProject)) {
                let path = dep.modify || dep.path
                let d = (path) ? reppath(path) : dep.name
                if (dep.ifdef && dep.type != 'component') {
                    let indent = ''
                    for each (r in dep.ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genWriteLine('!IF "$(ME_COM_' + r.toUpper() + ')" == "1"')
                            } else {
                                genWriteLine('ifeq ($(ME_COM_' + r.toUpper() + '),1)')
                            }
                            indent = '    '
                        }
                    }
                    if (dep.configurable && (!target.ifdef || !target.ifdef.contains(dep.name))) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!IF "$(ME_COM_' + dep.name.toUpper() + ')" == "1"')
                        } else {
                            genWriteLine('ifeq ($(ME_COM_' + dep.name.toUpper() + '),1)')
                        }
                        indent = '    '
                    }
                    if (me.platform.os == 'windows') {
                        genWriteLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                    } else {
                        genWriteLine(indent + 'DEPS_' + nextID + ' += ' + d)
                    }
                    if (dep.configurable && (!target.ifdef || !target.ifdef.contains(dep.name))) {
                        if (me.platform.os == 'windows') {
                            genWriteLine('!ENDIF')
                        } else {
                            genWriteLine('endif')
                        }
                    }
                    for each (r in dep.ifdef) {
                        if (!target.ifdef || !target.ifdef.contains(r)) {
                            if (me.platform.os == 'windows') {
                                genWriteLine('!ENDIF')
                            } else {
                                genWriteLine('endif')
                            }
                        }
                    }
                } else if (dep.type != 'component') {
                    if (me.platform.os == 'windows') {
                        genWriteLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                    } else {
                        genWriteLine('DEPS_' + nextID + ' += ' + d)
                    }
                } else if (dname == 'compile') {
                    if (me.platform.os == 'windows') {
                        genWriteLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                    } else {
                        genWriteLine('DEPS_' + nextID + ' += ' + d)
                    }
                }
                genTargetDepItems(target, dep.uses)
                if (dep.type == 'component' && dep.depends.length > 0) {
                    genTargetDepItems(target, dep.depends)
                }
            }
        }
    }

    /*
        Get the dependencies of a target as a string
     */
    function genTargetDeps(target) {
        nextID++
        genWriteLine('#\n#   ' + Path(target.name).basename + '\n#')
        if (target.type == 'file' || target.type == 'script' || target.type == 'header') {
            for each (file in target.files) {
                if (file == target.path) continue
                if (me.platform.os == 'windows') {
                    genWriteLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + reppath(file))
                } else {
                    genWriteLine('DEPS_' + nextID + ' += ' + reppath(file))
                }
            }
        }
        let depends = []
        for each (dname in (target.depends + target.uses)) {
            if (dname == target.name) {
                continue
            }
            if (!depends.contains(dname)) {
                depends.push(dname)
            }
        }
        genTargetDepItems(target, depends)
        genWriteLine('')
    }

} /* class Project */

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
