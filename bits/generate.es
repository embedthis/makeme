/*
    generate.es -- Generate Bit projects

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.bit {

    require ejs.unix
    require ejs.zlib

    var gen: Object
    var genout: TextStream
    var capture: Array?

    var minimalCflags = [ 
        '-w', '-g', '-Wall', '-Wno-deprecated-declarations', '-Wno-unused-result', '-Wshorten-64-to-32', '-mtune=generic']

    function generate() {
        if (b.options.gen == 'start') {
            generateStart()
            return
        }
        if (b.options.gen == 'main') {
            generateMain()
            return
        }
        b.makeBit(b.localPlatform, b.localPlatform + '.bit')
        platforms = bit.platforms = [b.localPlatform]
        bit.original = {
            dir: bit.dir.clone(),
            platform: bit.platform.clone(),
        }
        for (d in bit.dir) {
            if (d == 'bits') continue
            bit.dir[d] = bit.dir[d].replace(bit.original.platform.name, bit.platform.name)
        }
        bit.platform.last = true
        bit.generating = true
        b.prepBuild()
        generateProjects()
        bit.generating = null
    }

    function generateProjects() {
        b.selectTargets('all')
        gen = {
            configuration:  bit.platform.name
            compiler:       bit.defaults.compiler.join(' '),
            defines :       bit.defaults.defines.map(function(e) '-D' + e).join(' '),
            includes:       bit.defaults.includes.map(function(e) '-I' + e).join(' '),
            linker:         bit.defaults.linker.join(' '),
            libpaths:       b.mapLibPaths(bit.defaults.libpaths)
            libraries:      b.mapLibs(bit.defaults.libraries).join(' ')
        }
        blend(gen, bit.prefixes)
        for each (item in b.options.gen) {
            bit.generating = item
            let base = bit.dir.proj.join(bit.settings.product + '-' + bit.platform.os + '-' + bit.platform.profile)
            let path = bit.original.dir.inc.join('bit.h')
            let hfile = bit.dir.src.join('projects', 
                    bit.settings.product + '-' + bit.platform.os + '-' + bit.platform.profile + '-bit.h')
            path.copy(hfile)
            trace('Generate', 'project header: ' + hfile.relative)
            if (bit.generating == 'sh') {
                generateShellProject(base)
            } else if (bit.generating == 'make') {
                generateMakeProject(base)
            } else if (bit.generating == 'nmake') {
                generateNmakeProject(base)
            } else if (bit.generating == 'vstudio' || bit.generating == 'vs') {
                generateVstudioProject(base)
            } else if (bit.generating == 'xcode') {
                generateXcodeProject(base)
            } else {
                throw 'Unknown generation format: ' + bit.generating
            }
        }
    }

    function generateTarget(target) {
        if (target.requires) {
            for each (r in target.requires) {
                if (bit.platform.os == 'windows') {
                    genout.writeLine('!IF "$(BIT_PACK_' + r.toUpper() + ')" == "1"')
                } else {
                    genWriteLine('ifeq ($(BIT_PACK_' + r.toUpper() + '),1)')
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
        if (target.requires) {
            for (i in target.requires.length) {
                if (bit.platform.os == 'windows') {
                    genWriteLine('!ENDIF')
                } else {
                    genWriteLine('endif')
                }
            }
        }
        genWriteLine('')
    }

    function generateMain() {
        let bits = Config.Bin.join('bits')
        let cfg = Path('configure')
        if (cfg.exists && !b.options.overwrite) {
            traceFile('Exists', 'configure')
        } else {
            let data = '#!/bin/bash\n#\n#   configure -- Configure for building\n#\n' +
                'if ! type bit >/dev/null 2>&1 ; then\n' +
                    '    echo -e "\\nInstall the \\"bit\\" tool for configuring." >&2\n' +
                    '    echo -e "Download from: http://embedthis.com/downloads/bit/download.ejs." >&2\n' +
                    '    echo -e "Or skip configuring and make a standard build using \\"make\\".\\n" >&2\n' +
                    '    exit 255\n' +
                'fi\n' + 
                'bit configure "$@"'
            traceFile(cfg.exists ? 'Overwrite' : 'Create', cfg)
            cfg.write(data)
            cfg.setAttributes({permissions: 0755})
        }
        safeCopy(bits.join('sample-main.bit'), MAIN)
    }

    function generateStart() {
        safeCopy(Path(Config.Bin).join('bits/sample-start.bit'), 'start.bit')
    }

    function generateShellProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.sh')
        let path = base.joinExt('sh')
        genout = TextStream(File(path, 'w'))
        genout.writeLine('#\n#   ' + path.basename + ' -- Build It Shell Script to build ' + bit.settings.title + '\n#\n')
        genEnv()
        genout.writeLine('PRODUCT="' + bit.settings.product + '"')
        genout.writeLine('VERSION="' + bit.settings.version + '"')
        genout.writeLine('BUILD_NUMBER="' + bit.settings.buildNumber + '"')
        genout.writeLine('PROFILE="' + bit.platform.profile + '"')
        genout.writeLine('ARCH="' + bit.platform.arch + '"')
        genout.writeLine('ARCH="`uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\'`"')
        genout.writeLine('OS="' + bit.platform.os + '"')
        genout.writeLine('CONFIG="${OS}-${ARCH}-${PROFILE}' + '"')
        genout.writeLine('CC="' + bit.packs.compiler.path + '"')
        if (bit.packs.link) {
            genout.writeLine('LD="' + bit.packs.link.path + '"')
        }
        let cflags = gen.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word, '')
        }
        cflags += ' -w'
        genout.writeLine('CFLAGS="' + cflags.trim() + '"')
        genout.writeLine('DFLAGS="' + gen.defines + '"')
        genout.writeLine('IFLAGS="' + 
            repvar(bit.defaults.includes.map(function(path) '-I' + path.relative).join(' ')) + '"')
        genout.writeLine('LDFLAGS="' + repvar(gen.linker).replace(/\$ORIGIN/g, '\\$$ORIGIN') + '"')
        genout.writeLine('LIBPATHS="' + repvar(gen.libpaths) + '"')
        genout.writeLine('LIBS="' + gen.libraries + '"\n')
        genout.writeLine('[ ! -x ${CONFIG}/inc ] && ' + 'mkdir -p ${CONFIG}/inc\n')
        genout.writeLine('[ ! -x ${CONFIG}/bin ] && ' + 'mkdir -p ${CONFIG}/bin\n')
        genout.writeLine('[ ! -x ${CONFIG}/obj ] && ' + 'mkdir -p ${CONFIG}/obj\n')
        genout.writeLine('[ ! -f ${CONFIG}/inc/bit.h ] && ' + 
            'cp projects/' + bit.settings.product + '-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h')
        if (bit.dir.src.join('src/bitos.h')) {
            genout.writeLine('[ ! -f ${CONFIG}/inc/bitos.h ] && cp ${SRC}/src/bitos.h ${CONFIG}/inc/bitos.h')
        }
        genout.writeLine('if ! diff ${CONFIG}/inc/bit.h projects/' + bit.settings.product + 
            '-${OS}-${PROFILE}-bit.h >/dev/null ; then')
        genout.writeLine('\tcp projects/' + bit.settings.product + '-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h')
        genout.writeLine('fi\n')
        b.build()
        genout.close()
        path.setAttributes({permissions: 0755})
    }

    function mapPrefixes() {
        prefixes = {}
        let root = bit.prefixes.root
        let base = bit.prefixes.base
        let app = bit.prefixes.app
        let vapp = bit.prefixes.vapp
        for (let [name,value] in bit.prefixes) {
            if (name.startsWith('programFiles')) continue
            value = expand(value).replace(/\/\//g, '/')
            if (name == 'root') {
                ;
            } else if (name == 'base') {
                if (value.startsWith(root.name)) {
                    if (root.name == '/') {
                        value = value.replace(root.name, '$(BIT_ROOT_PREFIX)/')
                    } else if (bit.platform.like == 'windows') {
                        value = value.replace(root.name, '$(BIT_ROOT_PREFIX)\\')
                    } else {
                        value = value.replace(root.name, '$(BIT_ROOT_PREFIX)')
                    }
                } else {
                    value = '$(BIT_ROOT_PREFIX)' + value
                }
            } else if (name == 'app') {
                if (value.startsWith(base.name)) {
                    value = value.replace(base.name, '$(BIT_BASE_PREFIX)')
                }
            } else if (name == 'vapp') {
                if (value.startsWith(app.name)) {
                    value = value.replace(app.name, '$(BIT_APP_PREFIX)')
                }
            } else if (value.startsWith(vapp.name)) {
                value = value.replace(vapp.name, '$(BIT_VAPP_PREFIX)')
            } else {
                value = '$(BIT_ROOT_PREFIX)' + value
            }
            value = value.replace(bit.settings.version, '$(VERSION)')
            value = value.replace(bit.settings.product, '$(PRODUCT)')
            prefixes[name] = Path(value.toString())
        }
        return prefixes
    }

    function generatePackDefs() {
        let requiredPacks = {}
        /*
            Get packs required by targets
         */
        for each (pack in bit.packs) {
            if (pack.required && pack.explicit) {
                requiredPacks[pack.name] = true
            }
        }
        for each (target in bit.targets) {
            for each (r in target.requires) {
                if (bit.packs[r]) {
                    requiredPacks[r] = true
                }
            }
        }
        for (let [pname, enabled] in bit.packDefaults) {
            if (bit.packs[pname]) {
                bit.packs[pname].enable = enabled
            }
            requiredPacks[pname] = true
        }
        /*
            Follow pack.requires and pack.depends references
         */
        for (let [name, pack] in bit.packs) {
            if (requiredPacks[name]) {
                for each (r in pack.requires) {
                    if (bit.packs[r]) {
                        requiredPacks[r] = true
                    }
                }
            }
        }
        /*
            Emit BIT_PACK_XXX definitions 
         */
        Object.sortProperties(bit.packs)
        for (let [name, pack] in bit.packs) {
            if (requiredPacks[name]) {
                if (bit.platform.os == 'windows' ) {
                    genout.writeLine('%-18s = %s'.format(['BIT_PACK_' + name.toUpper(), pack.enable ? 1 : 0]))
                } else {
                    genout.writeLine('%-18s := %s'.format(['BIT_PACK_' + name.toUpper(), pack.enable ? 1 : 0]))
                }
            }
        }
        genout.writeLine('')

        /*
            Emit pack dependencies
         */
        for (let [pname, pack] in bit.packs) {
            if (pack.requires) {
                if (bit.platform.os == 'windows' ) {
                    genout.writeLine('!IF "$(BIT_PACK_' + pname.toUpper() + ')" == "1"')
                    for each (r in pack.requires) {
                        genout.writeLine('BIT_PACK_' + r.toUpper() + ' = 1')
                    }
                    genout.writeLine('!ENDIF\n')
                } else {
                    genout.writeLine('ifeq ($(BIT_PACK_' + pname.toUpper() + '),1)')
                    for each (r in pack.requires) {
                        genout.writeLine('    BIT_PACK_' + r.toUpper() + ' := 1')
                    }
                    genout.writeLine('endif')
                }
            }
        }
        genout.writeLine('')

        for each (pack in bit.packs) {
            if (pack.path) {
                if (bit.platform.os == 'windows') {
                    genout.writeLine('%-25s = %s'.format(['BIT_PACK_' + pack.name.toUpper() + '_PATH', pack.path]))
                } else {
                    genout.writeLine('%-25s := %s'.format(['BIT_PACK_' + pack.name.toUpper() + '_PATH', pack.path]))
                }
            }
        }
        genout.writeLine('')

        /*
            Compute the dflags 
         */
        let dflags = ''
        for (let [name, pack] in bit.packs) {
            if (requiredPacks[name]) {
                dflags += '-DBIT_PACK_' + name.toUpper() + '=$(BIT_PACK_' + name.toUpper() + ') '
            }
        }
        return dflags
    }

    function generateMakeProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative + '.mk')
        let path = base.joinExt('mk')
        genout = TextStream(File(path, 'w'))
        genout.writeLine('#\n#   ' + path.basename + ' -- Makefile to build ' + 
            bit.settings.title + ' for ' + bit.platform.os + '\n#\n')
        b.runScript(bit.scripts, 'pregen')
        genout.writeLine('PRODUCT            := ' + bit.settings.product)
        genout.writeLine('VERSION            := ' + bit.settings.version)
        genout.writeLine('BUILD_NUMBER       := ' + bit.settings.buildNumber)
        genout.writeLine('PROFILE            := ' + bit.platform.profile)
        if (bit.platform.os == 'vxworks') {
            genout.writeLine("ARCH               := $(shell echo $(WIND_HOST_TYPE) | sed 's/-.*//')")
            genout.writeLine("CPU                := $(subst X86,PENTIUM,$(shell echo $(ARCH) | tr a-z A-Z))")
        } else {
            genout.writeLine('ARCH               := $(shell uname -m | sed \'s/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/\')')
        }
        genout.writeLine('OS                 := ' + bit.platform.os)
        genout.writeLine('CC                 := ' + bit.packs.compiler.path)
        if (bit.packs.link) {
            genout.writeLine('LD                 := ' + bit.packs.link.path)
        }
        genout.writeLine('CONFIG             := $(OS)-$(ARCH)-$(PROFILE)')
        genout.writeLine('LBIN               := $(CONFIG)/bin\n')

        let dflags = generatePackDefs()
        genEnv()

        let cflags = gen.compiler
        for each (word in minimalCflags) {
            cflags = cflags.replace(word, '')
        }
        cflags += ' -w'
        genout.writeLine('CFLAGS             += ' + cflags.trim())
        genout.writeLine('DFLAGS             += ' + gen.defines.replace(/-DBIT_DEBUG/, '') + 
            ' $(patsubst %,-D%,$(filter BIT_%,$(MAKEFLAGS))) ' + dflags)
        genout.writeLine('IFLAGS             += ' + 
            repvar(bit.defaults.includes.map(function(path) '-I' + reppath(path.relative)).join(' ')))
        let linker = defaults.linker.map(function(s) "'" + s + "'").join(' ')
        let ldflags = repvar(linker).replace(/\$ORIGIN/g, '$$$$ORIGIN').replace(/ '-g'/, '')
        genout.writeLine('LDFLAGS            += ' + ldflags)
        genout.writeLine('LIBPATHS           += ' + repvar(gen.libpaths))
        genout.writeLine('LIBS               += ' + gen.libraries + '\n')

        genout.writeLine('DEBUG              := ' + (bit.settings.debug ? 'debug' : 'release'))
        genout.writeLine('CFLAGS-debug       := -g')
        genout.writeLine('DFLAGS-debug       := -DBIT_DEBUG')
        genout.writeLine('LDFLAGS-debug      := -g')
        genout.writeLine('DFLAGS-release     := ')
        genout.writeLine('CFLAGS-release     := -O2')
        genout.writeLine('LDFLAGS-release    := ')
        genout.writeLine('CFLAGS             += $(CFLAGS-$(DEBUG))')
        genout.writeLine('DFLAGS             += $(DFLAGS-$(DEBUG))')
        genout.writeLine('LDFLAGS            += $(LDFLAGS-$(DEBUG))\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name == 'root' && value == '/') {
                value = ''
            }
            genout.writeLine('%-18s := %s'.format(['BIT_' + name.toUpper() + '_PREFIX', value]))
        }
        genout.writeLine('')
        b.runScript(bit.scripts, 'gencustom')
        genout.writeLine('')

        let pop = bit.settings.product + '-' + bit.platform.os + '-' + bit.platform.profile
        genTargets()
        genout.writeLine('unexport CDPATH\n')
        genout.writeLine('ifndef SHOW\n.SILENT:\nendif\n')
        genout.writeLine('all build compile: prep $(TARGETS)\n')
        genout.writeLine('.PHONY: prep\n\nprep:')
        genout.writeLine('\t@echo "      [Info] Use "make SHOW=1" to trace executed commands."')
        genout.writeLine('\t@if [ "$(CONFIG)" = "" ] ; then echo WARNING: CONFIG not set ; exit 255 ; fi')
        genout.writeLine('\t@if [ "$(BIT_APP_PREFIX)" = "" ] ; then echo WARNING: BIT_APP_PREFIX not set ; exit 255 ; fi')
        if (bit.platform.os == 'vxworks') {
            genout.writeLine('\t@if [ "$(WIND_BASE)" = "" ] ; then echo WARNING: WIND_BASE not set. Run wrenv.sh. ; exit 255 ; fi')
            genout.writeLine('\t@if [ "$(WIND_HOST_TYPE)" = "" ] ; then echo WARNING: WIND_HOST_TYPE not set. Run wrenv.sh. ; exit 255 ; fi')
            genout.writeLine('\t@if [ "$(WIND_GNU_PATH)" = "" ] ; then echo WARNING: WIND_GNU_PATH not set. Run wrenv.sh. ; exit 255 ; fi')
        }
        genout.writeLine('\t@[ ! -x $(CONFIG)/bin ] && ' + 'mkdir -p $(CONFIG)/bin; true')
        genout.writeLine('\t@[ ! -x $(CONFIG)/inc ] && ' + 'mkdir -p $(CONFIG)/inc; true')
        genout.writeLine('\t@[ ! -x $(CONFIG)/obj ] && ' + 'mkdir -p $(CONFIG)/obj; true')
        genout.writeLine('\t@[ ! -f $(CONFIG)/inc/bit.h ] && ' + 'cp projects/' + pop + '-bit.h $(CONFIG)/inc/bit.h ; true')
        if (bit.dir.src.join('src/bitos.h')) {
            genout.writeLine('\t@[ ! -f $(CONFIG)/inc/bitos.h ] && cp src/bitos.h $(CONFIG)/inc/bitos.h ; true')
            genout.writeLine('\t@if ! diff $(CONFIG)/inc/bitos.h src/bitos.h >/dev/null ; then\\')
            genout.writeLine('\t\tcp src/bitos.h $(CONFIG)/inc/bitos.h  ; \\')
            genout.writeLine('\tfi; true')
        }
        genout.writeLine('\t@if ! diff $(CONFIG)/inc/bit.h projects/' + pop + '-bit.h >/dev/null ; then\\')
        genout.writeLine('\t\tcp projects/' + pop + '-bit.h $(CONFIG)/inc/bit.h  ; \\')
        genout.writeLine('\tfi; true')
        genout.writeLine('\t@if [ -f "$(CONFIG)/.makeflags" ] ; then \\')
        genout.writeLine('\t\tif [ "$(MAKEFLAGS)" != " ` cat $(CONFIG)/.makeflags`" ] ; then \\')
        genout.writeLine('\t\t\techo "   [Warning] Make flags have changed since the last build: \"`cat $(CONFIG)/.makeflags`\"" ; \\')
        genout.writeLine('\t\tfi ; \\')
        genout.writeLine('\tfi')
        genout.writeLine('\t@echo $(MAKEFLAGS) >$(CONFIG)/.makeflags')

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
        genout.writeLine('#\n#   ' + path.basename + ' -- Makefile to build ' + bit.settings.title + 
            ' for ' + bit.platform.os + '\n#\n')
        b.runScript(bit.scripts, 'pregen')
        genout.writeLine('PRODUCT            = ' + bit.settings.product)
        genout.writeLine('VERSION            = ' + bit.settings.version)
        genout.writeLine('BUILD_NUMBER       = ' + bit.settings.buildNumber)
        genout.writeLine('PROFILE            = ' + bit.platform.profile)
        genout.writeLine('PA                 = $(PROCESSOR_ARCHITECTURE)')
        genout.writeLine('')
        genout.writeLine('!IF "$(PA)" == "AMD64"')
            genout.writeLine('ARCH               = x64')
            genout.writeLine('ENTRY              = _DllMainCRTStartup')
        genout.writeLine('!ELSE')
            genout.writeLine('ARCH               = x86')
            genout.writeLine('ENTRY              = _DllMainCRTStartup@12')
        genout.writeLine('!ENDIF\n')
        genout.writeLine('OS                 = ' + bit.platform.os)
        genout.writeLine('CONFIG             = $(OS)-$(ARCH)-$(PROFILE)')
        genout.writeLine('LBIN               = $(CONFIG)\\bin\n')

        let dflags = generatePackDefs()

        genout.writeLine('CC                 = cl')
        genout.writeLine('LD                 = link')
        genout.writeLine('RC                 = rc')
        genout.writeLine('CFLAGS             = ' + gen.compiler)
        genout.writeLine('DFLAGS             = ' + gen.defines + ' ' + dflags)
        genout.writeLine('IFLAGS             = ' + 
            repvar(bit.defaults.includes.map(function(path) '-I' + reppath(path)).join(' ')))
        genout.writeLine('LDFLAGS            = ' + repvar(gen.linker).replace(/-machine:x86/, '-machine:$$(ARCH)'))
        genout.writeLine('LIBPATHS           = ' + repvar(gen.libpaths).replace(/\//g, '\\'))
        genout.writeLine('LIBS               = ' + gen.libraries + '\n')

        let prefixes = mapPrefixes()
        for (let [name, value] in prefixes) {
            if (name.startsWith('programFiles')) continue
            /* MOB bug - value.windows will change C:/ to C: */
            if (name == 'root') {
                value = value.trimEnd('/')
            } else {
                value = value.map('\\')
            }
            genout.writeLine('%-18s = '.format(['BIT_' + name.toUpper() + '_PREFIX']) + value)
        }
        genout.writeLine('')
        b.runScript(bit.scripts, 'gencustom')
        genout.writeLine('')

        genTargets()
        let pop = bit.settings.product + '-' + bit.platform.os + '-' + bit.platform.profile
        genout.writeLine('!IFNDEF SHOW\n.SILENT:\n!ENDIF\n')
        genout.writeLine('all build compile: prep $(TARGETS)\n')
        genout.writeLine('.PHONY: prep\n\nprep:')
        genout.writeLine('!IF "$(VSINSTALLDIR)" == ""\n\techo "Visual Studio vars not set. Run vcvars.bat."\n\texit 255\n!ENDIF')
        genout.writeLine('!IF "$(BIT_APP_PREFIX)" == ""\n\techo "BIT_APP_PREFIX not set."\n\texit 255\n!ENDIF')
        genout.writeLine('\t@if not exist $(CONFIG)\\bin md $(CONFIG)\\bin')
        genout.writeLine('\t@if not exist $(CONFIG)\\inc md $(CONFIG)\\inc')
        genout.writeLine('\t@if not exist $(CONFIG)\\obj md $(CONFIG)\\obj')
        genout.writeLine('\t@if not exist $(CONFIG)\\inc\\bit.h ' + 'copy projects\\' + pop + '-bit.h $(CONFIG)\\inc\\bit.h\n')
        genout.writeLine('clean:')
        builtin('cleanTargets')
        genout.writeLine('')
        b.build()
        genout.close()
    }

    function generateVstudioProject(base: Path) {
        trace('Generate', 'project file: ' + base.relative)
        mkdir(base)
        global.load(bit.dir.bits.join('vstudio.es'))
        vstudio(base)
    }

    function generateXcodeProject(base: Path) {
        global.load(bit.dir.bits.join('xcode.es'))
        xcode(base)
    }

    function genEnv() {
        let found
        if (bit.platform.os == 'windows') {
            var winsdk = (bit.packs.winsdk && bit.packs.winsdk.path) ? 
                bit.packs.winsdk.path.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft SDKs\\Windows\\v6.1'
            var vs = (bit.packs.compiler && bit.packs.compiler.dir) ? 
                bit.packs.compiler.dir.windows.name.replace(/.*Program Files.*Microsoft/, '$$(PROGRAMFILES)\\Microsoft') :
                '$(PROGRAMFILES)\\Microsoft Visual Studio 9.0'
            if (bit.generating == 'make') {
                /* Not used */
                genout.writeLine('VS             := ' + '$(VSINSTALLDIR)')
                genout.writeLine('VS             ?= ' + vs)
                genout.writeLine('SDK            := ' + '$(WindowsSDKDir)')
                genout.writeLine('SDK            ?= ' + winsdk)
                genout.writeLine('\nexport         SDK VS')
            }
        }
        for (let [key,value] in bit.env) {
            if (bit.platform.os == 'windows') {
                value = value.map(function(item)
                    item.replace(bit.packs.compiler.dir, '$(VS)').replace(bit.packs.winsdk.path, '$(SDK)')
                )
            }
            if (value is Array) {
                value = value.join(App.SearchSeparator)
            }
            if (bit.platform.os == 'windows') {
                if (key == 'INCLUDE' || key == 'LIB') {
                    value = '$(' + key + ');' + value
                } else if (key == 'PATH') {
                    value = value + ';$(' + key + ')'
                } 
            }
            if (bit.generating == 'make') {
                genout.writeLine('export %-18s := %s' % [key, value])

            } else if (bit.generating == 'nmake') {
                value = value.replace(/\//g, '\\')
                genout.writeLine('%-9s = %s' % [key, value])

            } else if (bit.generating == 'sh') {
                genout.writeLine('export ' + key + '="' + value + '"')
            }
            found = true
        }
        if (bit.platform.os == 'vxworks') {
            genout.writeLine('%-25s := %s'.format(['export PATH', '$(WIND_GNU_PATH)/$(WIND_HOST_TYPE)/bin:$(PATH)']))
        }
        if (found) {
            genout.writeLine('')
        }
    }

    function genTargets() {
        let all = []
        for each (target in b.topTargets) {
            if (target.path && target.enable && !target.nogen) {
                if (target.requires) {
                    for each (r in target.requires) {
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('!IF "$(BIT_PACK_' + r.toUpper() + ')" == "1"')
                        } else {
                            genout.writeLine('ifeq ($(BIT_PACK_' + r.toUpper() + '),1)')
                        }
                    }
                    if (bit.platform.os == 'windows') {
                        genout.writeLine('TARGETS            = $(TARGETS) ' + reppath(target.path))
                    } else {
                        genout.writeLine('TARGETS            += ' + reppath(target.path))
                    }
                    for (i in target.requires.length) {
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('!ENDIF')
                        } else {
                            genout.writeLine('endif')
                        }
                    }
                } else {
                    if (bit.platform.os == 'windows') {
                        genout.writeLine('TARGETS            = $(TARGETS) ' + reppath(target.path))
                    } else {
                        genout.writeLine('TARGETS            += ' + reppath(target.path))
                    }
                }
            }
        }
        genout.writeLine('')
    }

    function generateDir(target, solo = false) {
        if (target.dir) {
            if (bit.generating == 'sh') {
                makeDir(target.dir)

            } else if (bit.generating == 'make' || bit.generating == 'nmake') {
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
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (bit.generating == 'sh') {
            command = repcmd(command)
            genout.writeLine(command)

        } else if (bit.generating == 'make' || bit.generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.name)
            generateDir(target)
            genout.writeLine('\t' + command)
        }
    }

    function generateSharedLib(target) {
        let transition = target.rule || 'shlib'
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (bit.generating == 'sh') {
            command = repcmd(command)
            genout.writeLine(command)

        } else if (bit.generating == 'make' || bit.generating == 'nmake') {
            genTargetDeps(target)
            command = genTargetLibs(target, repcmd(command))
            command = command.replace(/-arch *\S* /, '')
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.name)
            generateDir(target)
            genout.writeLine('\t' + command)
        }
    }

    function generateStaticLib(target) {
        let transition = target.rule || 'lib'
        let rule = bit.rules[transition]
        if (!rule) {
            throw 'No rule to build target ' + target.path + ' for transition ' + transition
            return
        }
        let command = b.expandRule(target, rule)
        if (bit.generating == 'sh') {
            command = repcmd(command)
            genout.writeLine(command)

        } else if (bit.generating == 'make' || bit.generating == 'nmake') {
            command = repcmd(command)
            genTargetDeps(target)
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
            gtrace('Link', target.name)
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
            let rule = target.rule || bit.rules[transition]
            if (!rule) {
                rule = bit.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = b.expandRule(target, rule)
            if (bit.generating == 'sh') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '')
                genout.writeLine(command)

            } else if (bit.generating == 'make') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '')
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n    ' + file.relative + getDepsVar() + '\n')
                gtrace('Compile', file.relativeTo('.'))
                generateDir(target)
                genout.writeLine('\t' + command)

            } else if (bit.generating == 'nmake') {
                command = repcmd(command)
                command = command.replace(/-arch *\S* /, '')
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n    ' + file.relative.windows + getDepsVar() + '\n')
                gtrace('Compile', file.relativeTo('.'))
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
            let rule = target.rule || bit.rules[transition]
            if (!rule) {
                rule = bit.rules[target.path.extension]
                if (!rule) {
                    throw 'No rule to build target ' + target.path + ' for transition ' + transition
                    return
                }
            }
            let command = b.expandRule(target, rule)
            if (bit.generating == 'sh') {
                command = repcmd(command)
                genout.writeLine(command)

            } else if (bit.generating == 'make') {
                command = repcmd(command)
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n        ' + file.relative + getDepsVar() + '\n')
                gtrace('Compile', file.relativeTo('.'))
                generateDir(target)
                genout.writeLine('\t' + command)

            } else if (bit.generating == 'nmake') {
                command = repcmd(command)
                genTargetDeps(target)
                genout.write(reppath(target.path) + ': \\\n        ' + file.relative.windows + getDepsVar() + '\n')
                gtrace('Compile', file.relativeTo('.'))
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
        if (bit.generating == 'make' || bit.generating == 'nmake') {
            genTargetDeps(target)
            genout.write(reppath(target.path) + ':' + getDepsVar() + '\n')
        }
        gtrace('Copy', target.path.relative.portable)
        generateDir(target)
        for each (let file: Path in target.files) {
            /* Auto-generated headers targets for includes have file == target.path */
            if (file == target.path) {
                continue
            }
            if (target.subtree) {
                /* File must be abs to allow for a subtree substitution */
                copy(file, target.path, target)
            } else {
                copy(file, target.path, target)
            }
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
        if (bit.generating == 'make' || bit.generating == 'nmake') {
            genTargetDeps(target)
            genout.write(reppath(target.name) + ':' + getDepsVar() + '\n')
        }
        generateDir(target)
        if (command is Array) {
            genout.write('\t' + command.map(function(a) '"' + a + '"').join(' '))
        } else {
            genout.write('\t' + command)
        }
    }

    function generateScript(target) {
        setRuleVars(target, target.home)
        let prefix, suffix
        if (bit.generating) {
            if (bit.generating == 'sh' || bit.generating == 'make') {
                prefix = 'cd ' + target.home.relative
                suffix = 'cd ' + bit.dir.top.relativeTo(target.home)
            } else if (bit.generating == 'nmake') {
                prefix = 'cd ' + target.home.relative.windows + '\n'
                suffix = '\ncd ' + bit.dir.src.relativeTo(target.home).windows
            } else {
                prefix = suffix = ''
            }
            let rhome = target.home.relative
            if (rhome == '.' || rhome.startsWith('..')) {
                /* Don't change directory out of source tree. Necessary for actions in standard.bit */
                prefix = suffix = ''
            }
        }
        if (target['generate-capture']) {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            capture = []
            vtrace(target.type.toPascal(), target.name)
            runTargetScript(target, 'build')
            if (capture.length > 0) {
                genWriteLine('\t' + capture.join('\n\t'))
            }
            capture = null

        } else if (bit.generating == 'sh') {
            let cmd = target['generate-sh-' + bit.platform.os] || target['generate-sh'] ||
                target['generate-make-' + bit.platform.os] || target['generate-make'] || target.generate
            if (cmd) {
                cmd = cmd.trim()
                cmd = cmd.replace(/\\\n/mg, '')
                if (prefix || suffix) {
                    if (cmd.startsWith('@')) {
                        cmd = cmd.slice(1).replace(/^.*$/mg, '\t@' + prefix + '; $& ; ' + suffix)
                    } else {
                        cmd = cmd.replace(/^.*$/mg, '\t' + prefix + '; $& ; ' + suffix)
                    }
                } else {
                    cmd = cmd.replace(/^/mg, '\t')
                }
                bit.globals.LBIN = '$(LBIN)'
                cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                cmd = repvar2(cmd, target.home)
                bit.globals.LBIN = b.localBin
                genWriteLine(cmd)
            } else {
                genout.write('#  Omit build script ' + target.name + '\n')
            }

        } else if (bit.generating == 'make') {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            let cmd = target['generate-make-' + bit.platform.os] || target['generate-make'] ||
                target['generate-sh-' + bit.platform.os] || target['generate-sh'] || target.generate
            if (cmd) {
                cmd = cmd.trim().replace(/^\s*/mg, '\t')
                cmd = cmd.replace(/\\\n\s*/mg, '')
                cmd = cmd.replace(/^\t*(ifeq|ifneq|else|endif)/mg, '$1')
                if (prefix || suffix) {
                    if (cmd.startsWith('\t@')) {
                        cmd = cmd.slice(2).replace(/^\s*(.*)$/mg, '\t@' + prefix + '; $1 ; ' + suffix)
                    } else {
                        cmd = cmd.replace(/^\s(.*)$/mg, '\t' + prefix + '; $1 ; ' + suffix)
                    }
                }
                bit.globals.LBIN = '$(LBIN)'
                cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                cmd = repvar2(cmd, target.home)
                bit.globals.LBIN = b.localBin
                genWriteLine(cmd)
            }

        } else if (bit.generating == 'nmake') {
            genTargetDeps(target)
            if (target.path) {
                genWrite(target.path.relative.windows + ':' + getDepsVar() + '\n')
            } else {
                genWrite(target.name + ':' + getDepsVar() + '\n')
            }
            generateDir(target)
            let cmd = target['generate-namke-' + bit.platform.os] || target['generate-nmake'] || target['generate-make'] ||
                target.generate
            if (cmd) {
                cmd = cmd.replace(/\\\n/mg, '')
                cmd = cmd.trim().replace(/^cp /, 'copy ')
                cmd = prefix + cmd + suffix
                cmd = cmd.replace(/^[ \t]*/mg, '')
                cmd = cmd.replace(/^([^!])/mg, '\t$&')
                let saveDir = []
                if (bit.platform.os == 'windows') {
                    for (n in bit.globals) {
                        if (bit.globals[n] is Path) {
                            saveDir[n] = bit.globals[n]
                            bit.globals[n] = bit.globals[n].windows
                        }
                    }
                }
                bit.globals.LBIN = '$(LBIN)'
                try {
                    cmd = expand(cmd, {fill: null}).expand(target.vars, {fill: '${}'})
                } catch (e) {
                    print('Target', target.name)
                    print('Script:', cmd)
                    throw e
                }
                if (bit.platform.os == 'windows') {
                    for (n in saveDir) {
                        bit.globals[n] = saveDir[n]
                    }
                }
                cmd = repvar2(cmd, target.home)
                bit.globals.LBIN = b.localBin
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
        if (bit.generating == 'make' || bit.generating == 'nmake') {
            /* Twice because ldflags are repeated and replace only changes the first occurrence */
            command = rep(command, gen.linker, '$(LDFLAGS)')
            command = rep(command, gen.linker, '$(LDFLAGS)')
            command = rep(command, gen.libpaths, '$(LIBPATHS)')
            command = rep(command, gen.compiler, '$(CFLAGS)')
            command = rep(command, gen.defines, '$(DFLAGS)')
            command = rep(command, gen.includes, '$(IFLAGS)')
            command = rep(command, gen.libraries, '$(LIBS)')
            command = rep(command, RegExp(gen.configuration, 'g'), '$$(CONFIG)')

            command = repCmd(command, bit.packs.compiler.path, '$(CC)')
            command = repCmd(command, bit.packs.link.path, '$(LD)')
            if (bit.packs.rc) {
                command = repCmd(command, bit.packs.rc.path, '$(RC)')
            }
            for each (word in minimalCflags) {
                command = rep(command, word, '')
            }

        } else if (bit.generating == 'sh') {
            command = rep(command, gen.linker, '${LDFLAGS}')
            command = rep(command, gen.linker, '${LDFLAGS}')
            command = rep(command, gen.libpaths, '${LIBPATHS}')
            command = rep(command, gen.compiler, '${CFLAGS}')
            command = rep(command, gen.defines, '${DFLAGS}')
            command = rep(command, gen.includes, '${IFLAGS}')
            command = rep(command, gen.libraries, '${LIBS}')
            command = rep(command, RegExp(gen.configuration, 'g'), '$${CONFIG}')
            command = repCmd(command, bit.packs.compiler.path, '${CC}')
            command = repCmd(command, bit.packs.link.path, '${LD}')
            for each (word in minimalCflags) {
                command = rep(command, word, '')
            }
        }
        if (bit.generating == 'nmake') {
            command = rep(command, '_DllMainCRTStartup@12', '$(ENTRY)')
        }
        command = rep(command, RegExp(bit.dir.top + '/', 'g'), '')
        command = rep(command, /  */g, ' ')
        if (bit.generating == 'nmake') {
            command = rep(command, /\//g, '\\')
        }
        return command
    }

    /*
        Replace with variables where possible.
        Replaces the top directory and the CONFIGURATION
     */
    function repvar(command: String): String {
        command = command.replace(RegExp(bit.dir.top + '/', 'g'), '')
        if (bit.generating == 'make') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (bit.generating == 'nmake') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (bit.generating == 'sh') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$${CONFIG}')
        }
        for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
            if (bit.platform.like == 'windows') {
                let pat = bit.prefixes[p].windows.replace(/\\/g, '\\\\')
                command = command.replace(RegExp(pat, 'g'), '$$(BIT_' + p.toUpper() + '_PREFIX)')
            }
            command = command.replace(RegExp(bit.prefixes[p], 'g'), '$$(BIT_' + p.toUpper() + '_PREFIX)')
        }
        command = command.replace(/\/\//g, '$$(BIT_ROOT_PREFIX)/')
        return command
    }

    //  MOB - should merge repvar and repvar2
    function repvar2(command: String, home: Path): String {
        command = command.replace(RegExp(bit.dir.top, 'g'), bit.dir.top.relativeTo(home))
        if (bit.platform.like == 'windows' && bit.generating == 'nmake') {
            let re = RegExp(bit.dir.top.windows.name.replace(/\\/g, '\\\\'), 'g')
            command = command.replace(re, bit.dir.top.relativeTo(home).windows)
        }
        if (bit.generating == 'make') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (bit.generating == 'nmake') {
            command = command.replace(RegExp(gen.configuration + '\\\\bin/', 'g'), '$$(CONFIG)\\bin\\')
            command = command.replace(RegExp(gen.configuration, 'g'), '$$(CONFIG)')
        } else if (bit.generating == 'sh') {
            command = command.replace(RegExp(gen.configuration, 'g'), '$${CONFIG}')
        }
        for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
            if (bit.platform.like == 'windows') {
                let pat = gen[p].windows.replace(/\\/g, '\\\\')
                command = command.replace(RegExp(pat, 'g'), '$$(BIT_' + p.toUpper() + '_PREFIX)')
            }
            command = command.replace(RegExp(gen[p], 'g'), '$$(BIT_' + p.toUpper() + '_PREFIX)')
        }
        command = command.replace(/\/\//g, '$$(BIT_ROOT_PREFIX)/')
        return command
    }

    function reppath(path: Path): String {
        path = path.relative
        if (bit.platform.like == 'windows') {
            path = (bit.generating == 'nmake') ? path.windows : path.portable
        } else if (Config.OS == 'windows' && bit.generating && bit.generating != 'nmake')  {
            path = path.portable 
        }
        return repvar(path)
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
            Search the target libraries to find what packs they require.
         */
        for each (lib in target.libraries) {
            let name, dep, requires, pack
            name = pack = null

            dep = b.getDep(lib)
            if (dep && dep.type != 'pack') {
                name = dep.name
                requires = dep.requires
                if (bit.platform.os == 'vxworks' && !target.static) {
                    continue
                }
            } else {
                /*
                    Check packs that provide the library
                 */
                for each (p in bit.packs) {
                    if (p.libraries) {
                        if (p.libraries.contains(lib)) {
                            name = lib
                            dep = target
                        } else if (p.libraries.contains(Path(lib).trimExt())) {
                            name = lib.trimExt()
                            dep = target
                        } else if (p.libraries.contains(Path(lib.replace(/^lib/, '')).trimExt())) {
                            name = Path(lib.replace(/^lib/, '')).trimExt()
                            dep = target
                        }
                        if (name) {
                            requires = (target.requires) ? target.requires.clone() : []
                            if (!requires.contains(p.name)) {
                                requires.push(p.name)
                            }
                            pack = p;
                            break
                        }
                    }
                }
            }

            if (name) {
                if (bit.platform.os == 'windows') {
                    lib = lib.replace(/^lib/, '').replace(/\.lib$/, '')
                }
                if (requires) {
                    for each (r in requires) {
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('!IF "$(BIT_PACK_' + r.toUpper() + ')" == "1"')
                        } else {
                            genout.writeLine('ifeq ($(BIT_PACK_' + r.toUpper() + '),1)')
                        }
                    }
                    if (bit.platform.os == 'windows') {
                        genout.writeLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                        if (pack) {
                            for each (path in pack.libpaths) {
                                if (path != bit.dir.bin) {
                                    genout.writeLine('LIBPATHS_' + nextID + ' = $(LIBPATHS_' + nextID + ') -libpath:' + path)
                                    command = command.replace('-libpath:' + path.windows, '')
                                }
                            }
                        }
                    } else {
                        genout.writeLine('    LIBS_' + nextID + ' += -l' + lib)
                        if (pack) {
                            for each (path in pack.libpaths) {
                                if (path != bit.dir.bin) {
                                    genout.writeLine('    LIBPATHS_' + nextID + ' += -L' + path)
                                    command = command.replace('-L' + path, '')
                                }
                            }
                        }
                    }
                    for each (i in requires.length) {
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('!ENDIF')
                        } else {
                            genout.writeLine('endif')
                        }
                    }
                } else {
                    if (bit.platform.os == 'windows') {
                        genout.writeLine('LIBS_' + nextID + ' = $(LIBS_' + nextID + ') lib' + lib + '.lib')
                    } else {
                        genout.writeLine('LIBS_' + nextID + ' += -l' + lib)
                    }
                }
                found = true
                if (bit.platform.os == 'windows') {
                    command = command.replace(RegExp(' lib' + lib + '.lib ', 'g'), ' ')
                    command = command.replace(RegExp(' ' + lib + '.lib ', 'g'), ' ')
                    command = command.replace(RegExp(' ' + lib + ' ', 'g'), ' ')
                } else {
                    command = command.replace(RegExp(' -l' + lib + ' ', 'g'), ' ')
                }
            } else {
                if (bit.platform.os == 'windows') {
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
                if (bit.platform.os == 'windows') {
                    genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + reppath(file))
                } else {
                    genout.writeLine('DEPS_' + nextID + ' += ' + reppath(file))
                }
                found = true
            }
        }
        if (target.depends && target.depends.length > 0) {
            for each (let dname in target.depends) {
                // let dep = bit.targets[dname]
                if (dep && dep.enable) {
                    let d = (dep.path) ? reppath(dep.path) : dep.name
                    if (dep.requires) {
                        for each (r in dep.requires) {
                            if (bit.platform.os == 'windows') {
                                genout.writeLine('!IF "$(BIT_PACK_' + r.toUpper() + ')" == "1"')
                            } else {
                                genout.writeLine('ifeq ($(BIT_PACK_' + r.toUpper() + '),1)')
                            }
                        }
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                        } else {
                            genout.writeLine('    DEPS_' + nextID + ' += ' + d)
                        }
                        for (i in dep.requires.length) {
                            if (bit.platform.os == 'windows') {
                                genout.writeLine('!ENDIF')
                            } else {
                                genout.writeLine('endif')
                            }
                        }
                    } else {
                        if (bit.platform.os == 'windows') {
                            genout.writeLine('DEPS_' + nextID + ' = $(DEPS_' + nextID + ') ' + d)
                        } else {
                            genout.writeLine('DEPS_' + nextID + ' += ' + d)
                        }
                    }
                    found = true
                }
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

} /* bit module */

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
