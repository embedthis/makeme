/*
   xcode.es -- Support functions for generating Xcode projects
        
   Exporting: xcode()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     

module embedthis.me {

require ejs.unix
    
class Xcode {
    /*  
        Project output file stream
     */
    var out: Stream             

    /*
        Project prep code. Copy project me.h into 'inc'
     */
    const PREP_CODE = 
    "[ ! -x ${INC_DIR} ] && mkdir -p ${INC_DIR} ${OBJ_DIR} ${LIB_DIR} ${BIN_DIR}
    [ ! -f ${INC_DIR}/me.h -a -f ${settings.name}-macosx-${platform.profile}-me.h ] && cp ${settings.name}-macosx-${platform.profile}-me.h ${INC_DIR}/me.h
    if [ -f ${settings.name}-macosx-${platform.profile}-me.h ] ; then
        if ! diff ${INC_DIR}/me.h ${settings.name}-macosx-${platform.profile}-me.h >/dev/null 2>&1 ; then
            cp ${settings.name}-macosx-${platform.profile}-me.h ${INC_DIR}/me.h
        fi
    fi"

    var ids = {}                //  IDs stores the set of unique IDs used by generated Xcode projects. 
    var eo = {missing: true}    //  Property for expand() that preserves unresolved tokens
    var ibase                   //  Base ID string
    var inext = 0               //  Next sequential ID

    var builder: Builder
    var loader: Loader

    function Xcode() {
        builder = makeme.builder
        loader = makeme.loader
    }

    public function generate(base: Path) {
        let name = base.basename
        base = base.dirname
        xinit(base, name)
        projHeader(base)
        aggregates(base)
        sources(base)
        proxies(base)
        files(base)
        frameworks(base)
        groups(base)
        targets(base)
        scripts(base)
        project(base)
        sourcesBuildPhase(base)
        targetDependencies(base)
        projectConfigSection(base, name)
        targetConfigSection()
        term()
    }

    /*
        Want targets to be sorted in the UI
     */
    function sortTargets(array: Array, first: Number, second: Number): Number {
        let i = 0, j = 0
        if (array[first].name == 'All')
            i = -20000000
        if (array[second].name == 'All')
            j = -20000000
        if (array[first].name == 'Prep')
            i = -10000000
        if (array[second].name == 'Prep')
            j = -10000000
        if (array[first].type == 'lib')
            i = -1000000
        if (array[second].type == 'lib')
            j = -1000000
        if (array[first].type == 'exe')
            i = -1000
        if (array[second].type == 'exe')
            j = -1000
        if (array[first].name < array[second].name)
            i--
        else if (array[first].name > array[second].name)
            j--
        if (i < j) {
            return -1
        } else if (i == j) {
            return 0
        }
        return 1
    }

    function xinit(base, name) {
        let dir = base.join(name).joinExt('xcodeproj').relative
        trace('Generate', dir)
        dir.makeDir()
        let proj = dir.join('project.pbxproj')
        out = TextStream(File(proj, 'wt'))

        /*
            Set the XCode ID hash
         */
        let guidpath = dir.join('project.guid')
        if (guidpath.exists) {
            ibase = guidpath.readString().trim()
        } else {
            ibase = ('%08x%08x' % [Date().ticks, Date().ticks]).toUpper()
            guidpath.write(ibase)
        }
        makeid('ID_Products')
        makeid('ID_Project')
        makeid('ID_ProjectConfigList')
        makeid('ID_ProjectDebug')
        makeid('ID_ProjectRelease')

        /*
            Create the pseudo targets:
            Products - collection of all executables and libraries. Required by xcode.
            All      - Target depending on all executables to build (not libraries). Makes it easy to build everything.
            Prep     - Target to run prepare script. Creates build output directories.
         */
        loader.createTarget({ enable: true, home: base, type: 'group', xgroup: true, name: 'Products' })
        loader.createTarget({ enable: true, home: base, type: 'xbuild', xtarget: true, name: 'All' })
        loader.createTarget({ enable: true, home: base, type: 'xbuild', xtarget: true, name: 'Prep', 'generate-xcode': PREP_CODE }) 

        /*
            Create groups of targets for the xcode project
            NOTE: not yet using the x[] arrays. Should use these instead of iterating through targets[] all the time
         */
        me.xtargets = []
        me.xgroups = []
        me.xbinaries = []
        me.xscripts = []
        for each (target in me.targets) {
            if (!target.enable) continue
            if (target.action) continue
            if (target.generate === false) continue
            let type = target.type
            if (type == 'lib' || type == 'exe') {
                target.xbinary = true
                me.xbinaries.push(target)

            } else if (type == 'xbuild' || type == 'file' || (type == 'script' && target.goals.contains('all'))) {
                target.xscript = true
                me.xscripts.push(target)
            }
            if (type == 'lib' || type == 'exe' || target.name == 'Products') {
                target.xgroup = true
                me.xgroups.push(target)
            }
            if (type == 'lib' || type == 'exe' || type == 'xbuild' || type == 'file' || 
                    (type == 'script' && target.goals.contains('all'))) {
                target.xtarget = true
                me.xtargets.push(target)
            }
        }
        me.xtargets.sort(sortTargets)
        me.xgroups.sort(sortTargets)

        let targets = []
        for each (target in me.targets) {
            if (target.xbinary || target.xscript) {
                targets.push(target.name)
            }
            target.depends ||= []
            if (target.name != 'Prep') {
                target.depends.push('Prep')
            }
            me.target = target
            if (target.type == 'lib' || target.type == 'exe' || target.type == 'file') {
                builder.runTargetScript(target, 'prebuild')
            } else if (target.type == 'obj') {
                builder.runTargetScript(target, 'precompile')
            }
        }
        me.targets.All.depends = targets
        me.targets.Products.depends = targets

        /*
            Add header export to the prep code. Unfortunately can't use target scripts because they run after building the
            target.
         */
        let code = PREP_CODE
        for each (target in me.targets) {
            if (target.type == 'lib' || target.type == 'exe') {
                for each (let hdr in target.depends) {
                    let dep = me.targets[hdr]
                    if (dep && dep.type == 'header') {
                        for each (let file: Path in dep.files) {
                            code += '\nif [ ' + file.relativeTo(base) + ' -nt ' + dep.path.relativeTo(base) + ' ] ; then\n' +
                                   '    cp ' + file.relativeTo(base) + ' ' + dep.path.relativeTo(base) + '\nfi'
                        }
                    }
                }
            } else if (target.type == 'header') {
                for each (let file: Path in target.files) {
                    code += '\nif [ ' + file.relativeTo(base) + ' -nt ' + target.path.relativeTo(base) + ' ] ; then\n' +
                       '    cp ' + file.relativeTo(base) + ' ' + target.path.relativeTo(base) + '\nfi'
                }
            }
        }
        me.targets.Prep['generate-xcode'] = code
    }

    function term() {
        delete me.targets.Products
        delete me.targets.All
        delete me.targets.Prep
        delete me.xtargets
        delete me.xgroups
        delete me.xbinaries
        loader.makeGlobals()
    }

    function projHeader(base: Path) {
        output('// !$*UTF8*$!')
        output('{
        archiveVersion = 1;
        classes = {
        };
        objectVersion = 46;
        objects = {')
    }

    /*
        Aggregates are for scripts, and prep
     */
    function aggregates(base: Path) {
        output('\n/* Begin PBXAggregateTarget section */')
        let section = '\t\t${TID} /* ${TNAME} */ = {
    			isa = PBXAggregateTarget;
    			buildConfigurationList = ${BCL} /* Build configuration list for PBXAggregateTarget "${TNAME}" */;
    			buildPhases = (
    				${SID}
    			);
    			dependencies = ('
        for each (target in me.targets) {
            if (!target.xscript) {
                continue
            }
            let bcl = makeid('ID_BuildConfigList:' + target.name)
            let tid = makeid('ID_NativeTarget:' + target.name)
            let sid = (target.name == 'All') ? '' : (makeid('ID_ShellScript:' + target.name) + ',')
            output(section.expand(ids, eo).expand({TID: tid, BCL: bcl, TNAME: target.name, SID: sid}))

            let depSection = '\t\t\t\t${DID} /* PBXTargetDependency ${DNAME} */,'
            for each (item in target.depends) {
                dep = me.targets[item]
                if (!dep) {
                    continue
                }
                if (!dep.xtarget) {
                    continue
                }
                let did = makeid('ID_TargetDependency:' + target.name + '-on-' + dep.name)
                output(depSection.expand({DID: did, DNAME: dep.name}))
            }
            let footer = '\t\t\t);
    			name = ${TNAME};
    			productName = ${TNAME};
    		};'
            output(footer.expand({TNAME: target.name}))
        }
        output('/* End PBXAggregateTarget section */')
    }

    /*
        Emit all source files
     */
    function sources(base: Path) {
        output('\n/* Begin PBXBuildFile section */')
        section = '\t\t${BID} /* ${PATH} in Sources */ = {isa = PBXBuildFile; fileRef = ${REF} /* ${NAME} */; };'

        for each (target in me.targets) {
            if (!target.xbinary) {
                continue
            } 
            for each (file in target.files) {
                let obj = me.targets[file]
                if (obj) {
                    let bid = getmakeid('ID_BuildFile:' + obj.name)
                    for each (src in obj.files) {
                        let ref = getmakeid('ID_TargetSrc:' + src)
                        output(section.expand({BID: bid, REF: ref, PATH: src.basename, NAME: src.basename}))
                    }
                }
            }
        }
        /*
            Emit a framework reference for library targets that are depended upon. See PBXFrameworksBuildPhase.
         */
        section = '\t\t${BID} /* ${PATH} for ${TNAME} */ = {isa = PBXBuildFile; fileRef = ${REF}; };'
        for each (target in me.targets) {
            if (!target.xbinary) {
               continue
            } 
            for each (item in target.libraries) {
                let dep = me.targets['lib' + item]
                if (dep && dep.type == 'lib') {
                    let bid = getmakeid('ID_TargetFramework:' + target.name + '-on-' + dep.name)
                    let path = dep.path.relativeTo(base)
                    let ref = getmakeid('ID_TargetRef:' + path)
                    output(section.expand({BID: bid, REF: ref, PATH: dep.path.basename, 
                        TNAME: target.name, NAME: dep.name}))
                }
            }
        }
        output('/* End PBXBuildFile section */')
    }

    /*
        There is a proxy for each script, All and prep
     */
    function proxies(base: Path) {
        output('\n/* Begin PBXContainerItemProxy section */')
        let section = '\t\t${PID} /* PBXContainerItemProxy ${DNAME} from ${TNAME} */ = {
    			isa = PBXContainerItemProxy;
    			containerPortal = ${ID_Project} /* Project object */;
    			proxyType = 1;
    			remoteGlobalIDString = ${DID};
    			remoteInfo = ${DNAME};
    		};'
        for each (target in me.targets) {
            if (!target.xtarget) {
                continue
            }
            for each (dep in getAllDeps(target)) {
                let pid = makeid('ID_TargetProxy:' + target.name + '-on-' + dep.name)
                let did = getmakeid('ID_NativeTarget:' + dep.name)
                output(section.expand(ids, eo).expand({PID: pid, DID: did, TNAME: target.name, DNAME: dep.name}))
            }
        }
        output('/* End PBXContainerItemProxy section */')
    }

    /*
        Emit all files for all targets
     */
    function files(base: Path) {
        output('\n/* Begin PBXFileReference section */')
        let lib = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.dylib"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
        let exe = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; explicitFileType = "compiled.mach-o.executable"; includeInIndex = 0; path = ${PATH}; sourceTree = BUILT_PRODUCTS_DIR; };'
        let source = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.c; name = ${NAME}; path = ${PATH}; sourceTree = "<group>"; };'
        let header = '\t\t${REF} /* ${NAME} */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = sourcecode.c.h; name = ${NAME}; path = ${PATH}; sourceTree = "<group>"; };'

        for each (target in me.targets) {
            if (!target.xbinary) {
                continue
            } 
            for each (file in target.files) {
                let obj = me.targets[file]
                /*
                    Emit all sources and headers
                 */
                if (obj && obj.type == 'obj') {
                    for each (src in obj.files) {
                        let path = src.relativeTo(me.dir.top)
                        let ref = getid('ID_TargetSrc:' + src)
                        output(source.expand({REF: ref, NAME: src.basename, PATH: path}))
                    }
                    for each (hdr in obj.depends) {
                        if (hdr is Path && hdr.extension == 'h') {
                            let path = Path(hdr).relativeTo(me.dir.top)
                            if (!ids['ID_TargetHdr:' + hdr]) {
                                let ref = makeid('ID_TargetHdr:' + hdr)
                                output(header.expand({REF: ref, NAME: hdr.basename, PATH: path}))
                            }
                        }
                    }
                }
            }
            let path = target.path.relativeTo(base)
            let ref = getmakeid('ID_TargetRef:' + path)
            if (target.type == 'lib') {
                output(lib.expand({REF: ref, NAME: target.name, PATH: path.basename.joinExt(me.ext.shobj)}))
            } else if (target.type == 'exe') {
                output(exe.expand({REF: ref, NAME: target.name, PATH: path.basename}))
            }
        }
        output('/* End PBXFileReference section */')
    }

    /*
        This is for frameworks and libraries used by targets
     */
    function frameworks(base: Path) {
        output('\n/* Begin PBXFrameworksBuildPhase section */')
        let section = '\t\t${FID} /* Frameworks and Libraries for ${TNAME} */ = {
                isa = PBXFrameworksBuildPhase;
                buildActionMask = 2147483647;
                files = (
    ${LIBS}
                );
                runOnlyForDeploymentPostprocessing = 0;
            };'
        for each (target in me.targets) {
            if (!target.xbinary) {
               continue
            } 
            let fid = makeid('ID_Frameworks:' + target.name)
            let libs = []
/* UNUSED
            if (target.static) {
                continue
            }
*/
            for each (item in target.libraries) {
                let dep = me.targets['lib' + item]
                if (dep && dep.type == 'lib') {
                    let id = getid('ID_TargetFramework:' + target.name + '-on-' + dep.name)
                    libs.push('\t\t\t\t' + id + ' /* ' + dep.name + ' */,')
                }
            }
            output(section.expand(ids, eo).expand({FID: fid, LIBS: libs.join('\n') + '\t\t\t\t', TNAME: target.name}))
        }
        output('/* End PBXFrameworksBuildPhase section */')
    }


    /*
        Groups are listed in the explorer pane for each executable and library and for the 
        Products group (which is required by xcode).
     */
    function groups(base: Path) {
        output('\n/* Begin PBXGroup section */')
        let section = '\t\t${GID} /* ${NAME} */ = {
                isa = PBXGroup;
                children = ('
        output(section.expand({GID: makeid('ID_Group'), NAME: 'Top Group'}))

        let groupItem = '\t\t\t\t${REF} /* ${NAME} */,'
        for each (target in me.xgroups) {
            let ref = makeid('ID_TargetGroup:' + target.name)
            let name = target.name != 'Products' ? (target.name) : target.name
            output(groupItem.expand({REF: ref, NAME: name}))
        }
        output('\t\t\t);
                sourceTree = "<group>";
            };')

        let groupFooter = '\t\t\t);
                name = "${NAME}";
                path = ${PATH};
                sourceTree = SOURCE_ROOT;
            };'

        let headers = {}
        for each (target in me.targets) {
            if (!target.xgroup) {
                continue
            }
            let gid = getid('ID_TargetGroup:' + target.name)
            let name = target.name != 'Products' ? (target.name) : target.name
            output(section.expand({GID: gid, NAME: name}))
            for each (item in target.depends) {
                let dep = me.targets[item]
                if (dep && dep.type == 'obj') {
                    for each (hdr in dep.depends) {
                        if (headers[hdr]) continue
                        if (hdr is Path && hdr.extension == 'h') {
                            let htarget = me.targets[hdr]
                            if (htarget) {
                                if (htarget.belongs != name && htarget.belongs != dep.name) {
                                    continue
                                }
                            }
                            let ref = getid('ID_TargetHdr:' + hdr)
                            output(groupItem.expand({REF: ref, NAME: hdr.basename}))
                        }
                        headers[hdr] = true
                    }
                }
            }
            for each (item in target.depends) {
                let dep = me.targets[item]
                if (dep) {
                    if (dep.type == 'obj') {
                        for each (src in dep.files) {
                            let ref = getid('ID_TargetSrc:' + src)
                            output(groupItem.expand({REF: ref, NAME: src.basename}))
                        }
                    } else if (dep.xbinary && target.name == 'Products') {
                        let path = dep.path.relativeTo(base)
                        let ref = getid('ID_TargetRef:' + path)
                        output(groupItem.expand({REF: ref, NAME: dep.name}))
                    }
                }
            }
            output(groupFooter.expand({NAME: name, PATH: me.dir.top.relativeTo(base)}))
        }
        output('/* End PBXGroup section */')
    }


    /*
        Find all dependencies for a target
     */
    function getAllDeps(target, result = []) {
        if (target._getAllDeps) {
            return
        }
        target._getAllDeps = true
        for each (dname in target.depends) {
            if (dname == target.name) {
                continue
            }
            let dep = me.targets[dname]
            if (dep && !result.contains(dep)) {
                getAllDeps(dep, result)
                if (dep.enable && dep.xtarget) { 
                    result.push(dep)
                }
            }
        }
        for each (dname in target.uses) {
            if (dname == target.name) {
                continue
            }
            let dep = me.targets[dname]
            if (dep && dep.enable && !result.contains(dep)) {
                getAllDeps(dep, result)
                if (dep.enable && dep.xtarget) { 
                    result.push(dep)
                }
            }
        }
        delete target._getAllDeps
        return result
    }


    /*
        Emit native targets for each binary to build
     */
    function targets(base) {
        output('\n/* Begin PBXNativeTarget section */')
        let section = '\t\t${TID} /* ${TNAME} */ = {
    			isa = PBXNativeTarget;
    			buildConfigurationList = ${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */;
    			buildPhases = (
    				${SID} /* Sources */,
    				${FID} /* Frameworks */,
    			);
    			buildRules = (
    			);
    			dependencies = (
    ${DEPS}
    			);
    			name = ${TNAME};
    			productName = ${settings.name};
    			productReference = ${REF} /* ${TNAME} */;
    			productType = "${PTYPE}";
    		};'

        for each (target in me.targets) {
            if (!target.xbinary) {
                continue
            }
            let sid = makeid('ID_NativeSources:' + target.name)
            let fid = getid('ID_Frameworks:' + target.name)
            let tid = getid('ID_NativeTarget:' + target.name)
            let bcl = getmakeid('ID_BuildConfigList:' + target.name)
            let path = target.path.relativeTo(base)
            let ref = getid('ID_TargetRef:' + path)
            let ptype = (target.type == 'exe') ? 'com.apple.product-type.tool' : 'com.apple.product-type.library.dynamic';

            deplist = getAllDeps(target)
            let deps = deplist.map(function(dep) '\t\t\t\t' + 
                getmakeid('ID_TargetDependency:' + target.name + '-on-' + dep.name) + 
                ' /* ' + dep.name + ' */,').join('\n') + '\t\t\t\t'
            output(section.expand(me, eo).expand(ids, eo).expand({
                TNAME: target.name, TID: tid, BCL: bcl, REF: ref, SID: sid, FID: fid, PTYPE: ptype, DEPS: deps,
            }))
        }
        output('/* End PBXNativeTarget section */')
    }

    /*
        Emit all scripts
     */
    function scripts(base) {
        output('\n/* Begin PBXShellScriptBuildPhase section */')
        let section = '\t\t${SID} /* ShellScript for ${TNAME} */ = {
    			isa = PBXShellScriptBuildPhase;
    			buildActionMask = 2147483647;
    			files = (
    			);
    			inputPaths = (
    ${INPUTS}
    			);
    			outputPaths = (
    ${OUTPUTS}
    			);
    			runOnlyForDeploymentPostprocessing = 0;
    			shellPath = ${SHELL};
    			shellScript = ${CMD};
    		};'
        for each (target in me.targets) {
            if (!target.xscript || target.name == 'All') {
                continue
            }
            let shell = '/bin/bash'
            let inputs = ''
            let outputs = ''
            let cmd = 'PATH=$PATH:/usr/local/bin\n'

            if (!target.home.same(base)) {
                if (target.type == 'file') {
                    cmd += 'cd ' + me.dir.top.relativeTo(base) + '\n'
                } else {
                    cmd += 'cd ' + target.home.relativeTo(base) + '\n'
                }
                loader.makeGlobals(target.home)
            }
            if (target.type == 'file') {
                /* Map files relative to home to support full range of Path.operate capabilities */
                let files = target.files.map(function(e) e.relativeTo(target.home))
                let cmdtmp = Path('me-xcode.tmp')
                genOpen(cmdtmp)
                copyFiles(files, target.path, target)
                genClose()
                let data = cmdtmp.readString()
                cmd += replacePath(data, me.dir.out.relativeTo(base), '$${OUT_DIR}')
                cmdtmp.remove()

            } else if (target['generate-xcode'] || target['generate-sh'] || 
                    (target['generate'] && target.generate !== true)) {
                let gencmd = target['generate-xcode'] || target['generate-sh'] || target['generate']
                if (gencmd) {
                    cmd += gencmd
                } else {
                    if (target.scripts && target.scripts.build) {
                        shell = '"/usr/bin/env ' + target.scripts.build[0].interpreter + '"'
                        cmd += target.scripts.build[0].script
                    }
                }
                cmd = cmd.replace(/^[ \t]*[\r\n]+/m, '')
                cmd = cmd.replace(/^[ \t]*/mg, '').trim()

            } else if (target.generate === true) {
                let cmdtmp = Path('me-xcode.tmp')
                genOpen(cmdtmp)
                if (target.files) {
                    global.FILES = target.files.map(function(f) f.relativeTo(target.home).portable).join(' ')
                } else {
                    global.FILES = ''
                }
                me.globals.FILES = global.FILES
                builder.runTargetScript(target, 'build')
                genClose()
                let data = cmdtmp.readString()
                cmd += replacePath(data, me.dir.out.relativeTo(base), '$${OUT_DIR}')
                cmdtmp.remove()
            }
            cmd = replacePath(cmd, me.dir.out.relativeTo(base), '$${OUT_DIR}')

            if (target.files && target.files.length > 0) {
                inputs = target.files.map(function(f) smartPath(f, base)).join(',\n')
            }
            target.vars ||= {}
            if (target.path) {
                outputs = target.path.relativeTo(base)
                target.vars.OUT = outputs
            }
            let sid = getid('ID_ShellScript:' + target.name)
            me.target = target
            cmd = loader.expand(cmd.toJSON())
            cmd = cmd.expand(target.vars, eo)
            output(section.expand({SID: sid, CMD: cmd, INPUTS: inputs, OUTPUTS: outputs, TNAME: target.name, SHELL: shell}))
        }
        loader.makeGlobals()
        output('/* End PBXShellScriptBuildPhase section */')
    }

    /*
        Emit the top level project
     */
    function project(base) {
        let section = '
    /* Begin PBXProject section */
    		${ID_Project} /* Project object */ = {
    			isa = PBXProject;
    			attributes = {
    				LastUpgradeCheck = 0430;
    				ORGANIZATIONNAME = "${settings.company}";
    			};
    			buildConfigurationList = ${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.name}" */;
    			compatibilityVersion = "Xcode 3.2";
    			developmentRegion = English;
    			hasScannedForEncodings = 0;
    			knownRegions = (
    				en,
    			);
    			mainGroup = ${ID_Group};
    			productRefGroup = ${ID_TargetGroup:Products} /* Products */;
    			projectDirPath = "";
    			projectRoot = "";
    			targets = ('

        output(section.expand(me, eo).expand(me.globals, eo).expand(ids))

        section = '\t\t\t\t${TID} /* ${TNAME} */,'
        for each (target in me.xtargets) {
            let tid = getid('ID_NativeTarget:' + target.name)
            output(section.expand({TID: tid, TNAME: target.name}))
        }
        output('\t\t\t);
    		};
    /* End PBXProject section */')
    }

    function sourcesBuildPhase(base: Path) {
        output('\n/* Begin PBXSourcesBuildPhase section */')
        let section = '\t\t${SID} /* ${NAME} Sources */ = {
    			isa = PBXSourcesBuildPhase;
    			buildActionMask = 2147483647;
    			files = (
    ${FILES}
    			);
    			runOnlyForDeploymentPostprocessing = 0;
    		};'

        for each (target in me.targets) {
            if (!target.xbinary) {
               continue
            }
            let lines = []
            for each (file in target.files) {
                let obj = me.targets[file]
                for each (src in obj.files) {
                    let bid = getid('ID_BuildFile:' + obj.name)
                    let fid = getid('ID_TargetSrc:' + src)
                    let srcSection = '\t\t\t\t${BID} /* ${NAME} in Sources */,'
                    lines.push(srcSection.expand({BID: bid, NAME: src.basename}))
                }
            }
            let files = lines.join('\n')
            let sid = getid('ID_NativeSources:' + target.name)
            output(section.expand({FILES: files, NAME: target.name, SID: sid}, eo).expand(ids))
        }
        output('/* End PBXSourcesBuildPhase section */')
    }

    /*
        There is a dependency entry for each dependency relationship
     */
    function targetDependencies(base: Path) {
        output('\n/* Begin PBXTargetDependency section */')
        let section = '\t\t${TDID} /* PBXTargetDependency ${TNAME} depends on ${DNAME} */ = {
    			isa = PBXTargetDependency;
    			target = ${DID} /* ${DNAME} */;
    			targetProxy = ${PID} /* PBXContainerItemProxy */;
    		};'
        for each (target in me.targets) {
            if (!target.xtarget) {
               continue
            }
            for each (dep in getAllDeps(target)) {
                let tdid = getmakeid('ID_TargetDependency:' + target.name + '-on-' + dep.name)
                let pid = getid('ID_TargetProxy:' + target.name + '-on-' + dep.name)
                let did = getid('ID_NativeTarget:' + dep.name)
                output(section.expand({TDID: tdid, DID: did, TNAME: target.name, DNAME: dep.name, PID: pid}))
            }
        }
        output('/* End PBXTargetDependency section */')
    }

    function prepareSettings(base, o, debug: Boolean) {
        let options = {}
        if (!o.linker) {
            return ''
        }
        let libs = []
        for each (lname in o.libraries) {
            dep = me.targets['lib' + lname]
            if (dep && dep.type == 'lib') {
                continue
            }
            dep = me.targets[lname.replace(/\.dylib/, '')]
            if (dep && dep.type == 'lib' && !dep.static) {
                continue
            }
            libs.push(lname)
        }
        if (o.linker) {
            o.linker = o.linker.clone()
            for (let [key,value] in o.linker) {
                if (value.contains('-compatibility_version') || value.contains('-current_version')) {
                    delete o.linker[key]
                }
            }
            o.linker.compact()
            if (me.platform.os == 'macosx') {
                o.linker = o.linker.unique()
            }
        }
        let staticTarget = o.target && o.target.static
        let flags = o.linker.filter(function(e) e != '-g') + 
            libs.map(function(lib: Path) {
                if (staticTarget) {
                    if (o.target && o.target.type == 'exe') {
                        return '-l' + lib.trimExt().toString().replace(/^lib/, '')
                    } else {
                        return ''
                    }
                } else {
                    return '-l' + lib.trimExt().toString().replace(/^lib/, '')
                }
            })
        if (flags.length > 0) {
            if (staticTarget) {
                options.linker = '\n\t\t\t\tOTHER_LDFLAGS = (\n' + 
                    flags.map(function(f) '\t\t\t\t\t"' + f + '",').join('\n') + '\n\t\t\t\t\t);\n'
            } else {
                options.linker = '\n\t\t\t\tOTHER_LDFLAGS = (\n' + 
                    flags.map(function(f) '\t\t\t\t\t"' + f + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);\n'
            }
        } else if (staticTarget) {
            options.linker = '\n\t\t\t\tOTHER_LDFLAGS = ();\n'
        }
        if (o.includes.length > 0) {
            options.includes = '\n\t\t\t\tHEADER_SEARCH_PATHS = (\n' + 
                o.includes.map(function(f) '\t\t\t\t\t"' + smartPath(f, base) + '",').join('\n') +
                    '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);\n'
        }
        if (o.libpaths.length > 0) {
            options.libpaths = '\n\t\t\t\tLIBRARY_SEARCH_PATHS = (\n' + 
                o.libpaths.map(function(f) '\t\t\t\t\t"' + smartPath(f, base) + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);'
        }
        /*
            DEBUG set by me-components/compiler
         */
        let defines = o.defines.clone()
        defines.removeElements('ME_DEBUG')
        defines.removeElements('ME_DEBUG=0')
        defines.removeElements('ME_DEBUG=1')
        if (debug) {
            defines.push('ME_DEBUG=1')
        } else {
            defines.push('ME_DEBUG=0')
        }
        if (defines.length > 0) {
            options.defines = '\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (\n' + 
                defines.map(function(f) '\t\t\t\t\t"' + f.replace('-D', '').replace(/"/g, '\\\\\\"') + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);'
        }
        let result = ''
        if (options.includes) result += options.includes
        if (options.defines) result += options.defines
        if (options.libpaths) result += options.libpaths
        if (options.linker) result += options.linker
        if (staticTarget) {
            if (o.target.type == 'lib') {
                result += '\n\t\t\t\tMACH_O_TYPE = staticlib;\n'
            } else {
                result += '\n\t\t\t\tMACH_O_TYPE = mh_execute;\n'
            }
        }
        return result.trimStart('\n')
    }

    function yesno(o, value) {
        if (value == '-w') {
            return o.contains(value) ? 'YES' : null
        } else {
            if (o.contains('-Wno-' + value)) {
                return 'NO'
            } else if (o.contains('-W' + value)) {
                return 'YES'
            }
        }
        return null
    }

    function appendSetting(result, o, keys, option) {
        if (!(keys is Array)) keys = [keys]
        for each (key in keys) {
            let value = yesno(o, option)
            if (value) {
                result = result +  '\t\t\t\t' + key + ' = ' + value + ';\n'
            }
        }
        return result
    }

    function projectConfigSection(base, name) {
        /*
            Extract $(VAR) defintiions in includes and libpaths
         */
        let defs = []
        for each (target in me.targets) {
            if (!target.enable) {
                continue
            }
            for each (item in target.includes) {
                if (item.contains('$(')) {
                    let key = item.replace(/\$\(([^\)]*)\).*/, '$1')
                    let value = (name.endsWith('-mine') ? App.getenv(key) : '') || ''
                    defs.push(key + ' = "' + value + '";\n')
                }
            }
            for each (item in target.libpaths) {
                if (item.contains('$(')) {
                    let key = item.replace(/\$\(([^\)]*)\).*/, '$1')
                    let value = (name.endsWith('-mine') ? App.getenv(key) : '') || ''
                    defs.push(key + ' = "' + value + '";\n')
                }
            }
        }
        let common_settings = ''
        if (defs.length > 0) {
            common_settings += defs.unique().join('')
        }
        common_settings += '
                    /* Common Settings */
                    COMBINE_HIDPI_IMAGES = YES;
    				ALWAYS_SEARCH_USER_PATHS = NO;
                    ONLY_ACTIVE_ARCH = YES;
                    CURRENT_PROJECT_VERSION = ${settings.compatible};
                    DYLIB_COMPATIBILITY_VERSION = "$(CURRENT_PROJECT_VERSION)";
                    DYLIB_CURRENT_VERSION = "$(CURRENT_PROJECT_VERSION)";

    				GCC_C_LANGUAGE_STANDARD = gnu99;
    				GCC_VERSION = ${COMPILER};
    				GCC_WARN_ABOUT_RETURN_TYPE = YES;

GCC_NO_COMMON_BLOCKS = YES;
GCC_WARN_64_TO_32_BIT_CONVERSION = YES;
GCC_WARN_UNDECLARED_SELECTOR = YES;
CLANG_WARN_BOOL_CONVERSION = YES;
CLANG_WARN_CONSTANT_CONVERSION = YES;
CLANG_WARN_EMPTY_BODY = YES;
CLANG_WARN_ENUM_CONVERSION = YES;
CLANG_WARN_INT_CONVERSION = YES;
CLANG_WARN_UNREACHABLE_CODE = YES;
CLANG_WARN__DUPLICATE_METHOD_MATCH = YES;
ENABLE_STRICT_OBJC_MSGSEND = YES;
ENABLE_TESTABILITY = YES;

                    LD_DYLIB_INSTALL_NAME = "@rpath/$(EXECUTABLE_PATH)";
    				MACOSX_DEPLOYMENT_TARGET = 10.10;
    				SDKROOT = macosx;

                    OUT_DIR = "${OUT}";
                    BIN_DIR = "${BIN}";
                    LIB_DIR = "${LIB}";
                    INC_DIR = "${INC}";
                    OBJ_DIR = "${OBJ}";
                    SRC_DIR = "${SRC}";

                    CONFIGURATION_TEMP_DIR = "$(OBJ_DIR)/${settings.name}-xcode/$(CONFIGURATION)";
                    CONFIGURATION_BUILD_DIR = "$(BIN_DIR)";
                    INSTALL_PATH = "/usr/lib/${settings.name}";
                    DSTROOT = "/tmp/${settings.name}.dst";
                    OBJROOT = "$(OBJ_DIR)";
                    SYMROOT = "$(BIN_DIR)";'


        let section = '
    /* Begin XCBuildConfiguration section */
    		${ID_ProjectDebug} /* Debug */ = {
    			isa = XCBuildConfiguration;
    			buildSettings = {
    ${COMMON_SETTINGS}
    ${OVERRIDABLE_SETTINGS}
                    /* Debug Settings */
    				COPY_PHASE_STRIP = NO;
    				GCC_OPTIMIZATION_LEVEL = 0;
    				GCC_SYMBOLS_PRIVATE_EXTERN = NO;
    ${DEBUG_SETTINGS}
    			};
    			name = Debug;
    		};
    		${ID_ProjectRelease} /* Release */ = {
    			isa = XCBuildConfiguration;
    			buildSettings = {
    ${COMMON_SETTINGS}
    ${OVERRIDABLE_SETTINGS}
                    /* Release Settings */
    				COPY_PHASE_STRIP = YES;
    				DEBUG_INFORMATION_FORMAT = "dwarf-with-dsym";
    				GCC_OPTIMIZATION_LEVEL = s;
                    GCC_WARN_UNINITIALIZED_AUTOS = ${WARN_UNUSED};
    ${RELEASE_SETTINGS}
    			};
    			name = Release;
    		};'

        let compiler = me.targets.compiler.path.basename == 'gcc' ? 
              'com.apple.compilers.llvmgcc42' : 'com.apple.compilers.llvm.clang.1_0'
        let defaults = me.targets.compiler
        let overridable = appendSetting('', defaults.compiler, 
            ['GCC_WARN_64_TO_32_ME_CONVERSION'], 'shorten-64-to-32')
        overridable += appendSetting(overridable, defaults.compiler, 
            ['GCC_WARN_UNUSED_VARIABLE', 'GCC_WARN_UNUSED_FUNCTION', 'GCC_WARN_UNUSED_LABEL'], 'unused-result')
        overridable += appendSetting(overridable, defaults.compiler, ['GCC_WARN_INHIBIT_ALL_WARNINGS'], '-w')
        loader.makeGlobals(base)

        output(section.expand(ids, eo).expand({
            COMMON_SETTINGS: common_settings.expand(me, eo).expand(me.globals, eo).expand(ids, eo).expand({
                COMPILER: compiler,
            }),
            OVERRIDABLE_SETTINGS: overridable,
            DEBUG_SETTINGS: prepareSettings(base, defaults, true),
            RELEASE_SETTINGS: prepareSettings(base, defaults, false),
            WARN_UNUSED: yesno(defaults.compiler, 'unused-result'),
        }))

        /*
            Now the per-target settings
         */
        section = '\t\t${TARGET_DEBUG} /* Debug */ = {
    			isa = XCBuildConfiguration;
    			buildSettings = {
    				PRODUCT_NAME = ${PNAME};
    ${OVERRIDABLE_SETTINGS}
    ${DEBUG_SETTINGS}
    			};
    			name = Debug;
    		};
    		${TARGET_RELEASE} /* Release */ = {
    			isa = XCBuildConfiguration;
    			buildSettings = {
    				PRODUCT_NAME = ${PNAME};
    ${OVERRIDABLE_SETTINGS}
    ${RELEASE_SETTINGS}
    			};
    			name = Release;
    		};'
        for each (target in me.targets) {
            if (!target.xtarget) {
               continue
            }
            let tdid = makeid('ID_TargetDebugConfig:' + target.name)
            let trid = makeid('ID_TargetReleaseConfig:' + target.name)

            for each (n in ['compiler', 'defines', 'includes', 'libpaths', 'linker', 'libraries']) {
                target[n] ||= []
            }
            let ts = {
                target: target,
                compiler: target.compiler - defaults.compiler,
                defines: target.defines - defaults.defines,
                includes: target.includes - defaults.includes,
                libpaths: target.libpaths - defaults.libpaths,
                linker: target.linker - defaults.linker,
                libraries: target.libraries - defaults.libraries,
            }
            let overridable = appendSetting('', ts.compiler, 
                ['GCC_WARN_64_TO_32_ME_CONVERSION'], 'shorten-64-to-32')
            overridable += appendSetting(overridable, ts.compiler, 
                ['GCC_WARN_UNUSED_VARIABLE', 'GCC_WARN_UNUSED_FUNCTION', 'GCC_WARN_UNUSED_LABEL'], 'unused-result')
            overridable += appendSetting(overridable, ts.compiler, ['GCC_WARN_INHIBIT_ALL_WARNINGS'], '-w')

            let pname = Path(target.path ? target.path : target.name).basename.trimExt().name || target.name
            if (target.type == 'lib' && (target.static || me.settings.static)) {
                overridable += '\t\t\t\tEXECUTABLE_EXTENSION = a;\n'
            }
            /*
                target.path may end in "/" so basename may be empty. Use ".name" to convert to a string which is falsy
             */
            output(section.expand({PNAME: pname, TNAME: target.name, TARGET_DEBUG: tdid, TARGET_RELEASE: trid, 
                OVERRIDABLE_SETTINGS: overridable,
                DEBUG_SETTINGS: prepareSettings(base, ts, true),
                RELEASE_SETTINGS: prepareSettings(base, ts, false),
                WARN_UNUSED: yesno(ts.compiler, 'unused-result'),
                WARN_64_TO_32: yesno(ts.compiler, 'shorten-64-to-32'),
            }))
        }
        output('/* End XCBuildConfiguration section */')
        loader.makeGlobals()
    }

    function targetConfigSection() {
        let section = '
    /* Begin XCConfigurationList section */
    		${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.name}" */ = {
    			isa = XCConfigurationList;
    			buildConfigurations = (
    				${ID_ProjectDebug} /* Debug */,
    				${ID_ProjectRelease} /* Release */,
    			);
    			defaultConfigurationIsVisible = 0;
    			defaultConfigurationName = Release;
    		};'
        output(section.expand(me, eo).expand(me.globals, eo).expand(ids))

        section = '\t\t${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */ = {
    			isa = XCConfigurationList;
    			buildConfigurations = (
    				${TARGET_DEBUG} /* Debug */,
    				${TARGET_RELEASE} /* Release */,
    			);
    			defaultConfigurationIsVisible = 0;
    			defaultConfigurationName = Release;
    		};'

        for each (target in me.targets) {
            if (!target.xtarget) {
                continue
            }
            let bcl = getid('ID_BuildConfigList:' + target.name)
            let tid = getid('ID_NativeTarget:' + target.name)
            let tdid = getid('ID_TargetDebugConfig:' + target.name)
            let trid = getid('ID_TargetReleaseConfig:' + target.name)
            output(section.expand(me, eo).expand(me.globals, eo).expand(ids, eo).
                    expand({BCL: bcl, TNAME: target.name, TID: tid, TARGET_DEBUG: tdid, TARGET_RELEASE: trid}))
        }

        section = '/* End XCConfigurationList section */
	};
	rootObject = ${ID_Project} /* Project object */;
}'
        output(section.expand(ids))
    }

    function output(line: String) {
        out.writeLine(line)
    }

    /*
        Generate the next available ID
     */
    function uid()
        (ibase + ("%08x" % [inext++]).toUpper())

    function getid(src) {
        if (!ids[src]) {
            throw new Error('Unknown ID for ' + src)
        }
        return ids[src]
    }

    function makeid(src) {
        if (ids[src]) {
            throw new Error('ID already exists for  ' + src)
        }
        return ids[src] = uid()
    }

    function getmakeid(src) {
        let id = ids[src] || uid()
        return ids[src] = id
    }

    function replacePath(str, path, substitute) {
        if (path == '.') {
            return str
        }
        let pattern = path.replace(/\./g, '\\.')
        return str.replace(RegExp(pattern, 'g'), substitute)
    }

    function smartPath(path: Path, base: Path) {
        if (path.startsWith('$(')) {
            return path
        }
        return path.relativeTo(base)
    }

} /* */
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
