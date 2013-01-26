/*
   xcode.es -- Support functions for generating Xcode projects
        
   Exporting: xcode()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream             //  Project output file stream

const PREP_CODE = 
"[ ! -x ${INC_DIR} ] && mkdir -p ${INC_DIR} ${OBJ_DIR} ${LIB_DIR} ${BIN_DIR}
[ ! -f ${INC_DIR}/bit.h ] && cp ${settings.product}-macosx-${platform.profile}-bit.h ${INC_DIR}/bit.h
[ ! -f ${INC_DIR}/bitos.h ] && cp ${SRC_DIR}/src/bitos.h ${INC_DIR}/bitos.h
if ! diff ${INC_DIR}/bit.h ${settings.product}-macosx-${platform.profile}-bit.h >/dev/null ; then
    cp ${settings.product}-macosx-${platform.profile}-bit.h ${INC_DIR}/bit.h
fi"

var ids = {}                //  IDs stores the set of unique IDs used by generated Xcode projects. 
var eo = {fill: '${}'}      //  Property for expand() that preserves unresolved tokens
var ibase                   //  Base ID string
var inext = 0               //  Next sequential ID

public function xcode(base: Path) {
    let name = base.basename
    base = base.dirname
    init(base, name)
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
    projectConfigSection(base)
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

function init(base, name) {
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
    bit.targets._Products_ = { enable: true, home: base, type: 'group', xgroup: true, name: 'Products' }
    bit.targets._All_ =      { enable: true, home: base, type: 'build', xtarget: true, name: 'All' }
    bit.targets._Prep_ =     { enable: true, home: base, type: 'build', xtarget: true, name: 'Prep', 
                               'generate-xcode': PREP_CODE } 

    /*
        Create groups of targets for the xcode project
        NOTE: not yet using the x[] arrays. Should use these instead of iterating through targets[] all the time
     */
    bit.xtargets = []
    bit.xgroups = []
    bit.xbinaries = []
    bit.xscripts = []
    for each (target in bit.targets) {
        if (!target.enable) continue
        let type = target.type
        if (type == 'lib' || type == 'exe') {
            target.xbinary = true
            bit.xbinaries.push(target)
/*UNUSED
            if (type == 'lib' && target.headers) {
                target.xscript = true
                bit.xscripts.push(target)
            }
*/
        } else if (type == 'build' || type == 'file') {
            target.xscript = true
            bit.xscripts.push(target)
        }
        if (type == 'lib' || type == 'exe' || target.name == 'Products') {
            target.xgroup = true
            bit.xgroups.push(target)
        }
        if (type == 'lib' || type == 'exe' || type == 'build' || type == 'file') {
            target.xtarget = true
            bit.xtargets.push(target)
        }
    }
    bit.xtargets.sort(sortTargets)
    bit.xgroups.sort(sortTargets)

    let targets = []
    for each (target in bit.targets) {
        if (target.xbinary || target.xscript) {
            targets.push(target.name)
        }
        target.depends ||= []
        if (target.name != 'Prep') {
            target.depends.push('_Prep_')
        }
        bit.target = target
        if (target.type == 'lib' || target.type == 'exe' || target.type == 'file') {
            runTargetScript(target, 'prebuild')
        } else if (target.type == 'obj') {
            runTargetScript(target, 'precompile')
        }
    }
    bit.targets._All_.depends = targets
    bit.targets._Products_.depends = targets

    /*
        Add header export to the prep code. Unfortunately can't use target scripts because they run after building the
        target.
     */
    let code = PREP_CODE
    for each (target in bit.targets) {
        if (target.type == 'lib' || target.type == 'exe') {
            for each (let hdr in target.depends) {
                let dep = bit.targets[hdr]
                if (dep && dep.type == 'header') {
                    for each (let file: Path in dep.files) {
                        code += '\nif [ ' + file.relativeTo(base) + ' -nt ' + dep.path.relativeTo(base) + ' ] ; then\n' +
                               '    cp ' + file.relativeTo(base) + ' ' + dep.path.relativeTo(base) + '\nfi'
                    }
                }
            }
        }
    }
    bit.targets._Prep_['generate-xcode'] = code
}

function term() {
    delete bit.targets._Products_
    delete bit.targets._All_
    delete bit.targets._Prep_
    delete bit.xtargets
    delete bit.xgroups
    delete bit.xbinaries
    makeDirGlobals()
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
    for each (target in bit.targets) {
        if (!target.xscript) {
            continue
        }
        let bcl = makeid('ID_BuildConfigList:' + target.name)
        let tid = makeid('ID_NativeTarget:' + target.name)
        let sid = (target.name == 'All') ? '' : (makeid('ID_ShellScript:' + target.name) + ',')
        output(section.expand(ids, eo).expand({TID: tid, BCL: bcl, TNAME: target.name, SID: sid}))

        let depSection = '\t\t\t\t${DID} /* PBXTargetDependency ${DNAME} */,'
        for each (item in target.depends) {
            dep = bit.targets[item]
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

    for each (target in bit.targets) {
        if (!target.xbinary) {
            continue
        } 
        for each (file in target.files) {
            let obj = bit.targets[file]
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
    for each (target in bit.targets) {
        if (!target.xbinary) {
           continue
        } 
        for each (item in target.libraries) {
            let dep = bit.targets['lib' + item]
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
    for each (target in bit.targets) {
        if (!target.xtarget) {
            continue
        }
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (!dep || !dep.xtarget) continue
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

    for each (target in bit.targets) {
        if (!target.xbinary) {
            continue
        } 
        for each (file in target.files) {
            let obj = bit.targets[file]
            /*
                Emit all sources and headers
             */
            if (obj && obj.type == 'obj') {
                for each (src in obj.files) {
                    //  TODO - can this emit duplicates?
                    let path = src.relativeTo(bit.dir.src)
                    let ref = getid('ID_TargetSrc:' + src)
                    output(source.expand({REF: ref, NAME: src.basename, PATH: path}))
                }
                for each (hdr in obj.depends) {
                    if (hdr is Path && hdr.extension == 'h') {
                        let path = Path(hdr).relativeTo(bit.dir.src)
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
            output(lib.expand({REF: ref, NAME: target.name, PATH: path.basename.joinExt(bit.ext.shobj)}))
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
    for each (target in bit.targets) {
        if (!target.xbinary) {
           continue
        } 
        let fid = makeid('ID_Frameworks:' + target.name)
        let libs = []
        for each (item in target.libraries) {
            let dep = bit.targets['lib' + item]
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
    for each (target in bit.xgroups) {
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

    for each (target in bit.targets) {
        if (!target.xgroup) {
            continue
        }
        let gid = getid('ID_TargetGroup:' + target.name)
        let name = target.name != 'Products' ? (target.name) : target.name
        output(section.expand({GID: gid, NAME: name}))
        let headers = {}
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (dep && dep.type == 'obj') {
                for each (hdr in dep.depends) {
                    if (headers[hdr]) continue
                    if (hdr is Path && hdr.extension == 'h') {
                        let ref = getid('ID_TargetHdr:' + hdr)
                        output(groupItem.expand({REF: ref, NAME: hdr.basename}))
                    }
                    headers[hdr] = true
                }
            }
        }
        for each (item in target.depends) {
            let dep = bit.targets[item]
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
        output(groupFooter.expand({NAME: name, PATH: bit.dir.src.relativeTo(base)}))
    }
    output('/* End PBXGroup section */')
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
			productName = ${settings.product};
			productReference = ${REF} /* ${TNAME} */;
			productType = "${PTYPE}";
		};'

    for each (target in bit.targets) {
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

        let deplist = []
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (dep && dep.xtarget) {
                deplist.push(dep)
            }
        }
        let deps = deplist.map(function(dep) '\t\t\t\t' + 
            getmakeid('ID_TargetDependency:' + target.name + '-on-' + dep.name) + 
            ' /* ' + dep.name + ' */,').join('\n') + '\t\t\t\t'
        output(section.expand(bit, eo).expand(ids, eo).expand({
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
    for each (target in bit.targets) {
        if (!target.xscript || target.name == 'All') {
            continue
        }
        let shell = '/bin/bash'
        let inputs = ''
        let outputs = ''
        let cmd = 'PATH=$PATH:/usr/local/bin\n'
        if (bit.platform.profile == 'mine' && bit.settings.product == 'appweb') {
            cmd += 'rm -f ${BIN_DIR}/appweb\n'
        }
        if (!target.home.same(base)) {
            cmd += 'cd ' + target.home.relativeTo(base) + '\n'
            makeDirGlobals(target.home)
        }
        if (target.type == 'file') {
            for each (let file: Path in target.files) {
                cmd += 'rm -rf ' + target.path.relativeTo(target.home) + '\n' +
                       'cp -r ' + file.relativeTo(target.home) + ' ' + target.path.relativeTo(target.home) + '\n'
            }
        } else {
            cmd += target['generate-xcode'] || target['generate-sh'] || target['generate']
            if (!cmd) {
                if (target.scripts && target.scripts.build) {
                    shell = '"/usr/bin/env ' + target.scripts.build[0].shell + '"'
                    cmd = target.scripts.build[0].script
                }
            }
            cmd = cmd.replace(/^[ \t]*[\r\n]+/m, '')
            cmd = cmd.replace(/^[ \t]*/mg, '').trim()
        }
        cmd = cmd.replace(RegExp(bit.dir.cfg.relativeTo(base), 'g'), '$${CFG_DIR}')

        if (target.files && target.files.length > 0) {
            inputs = target.files.map(function(f) f.relativeTo(base)).join(',\n')
        }
        if (target.path) {
            outputs = target.path.relativeTo(base)
            target.vars.OUT = outputs
        }
        let sid = getid('ID_ShellScript:' + target.name)
        // TODO cmd = cmd.toJSON().expand(bit, eo).expand(bit.globals, eo).expand(target.vars, eo)
        cmd = expand(cmd.toJSON())
        cmd = cmd.expand(target.vars, eo)
        output(section.expand({SID: sid, CMD: cmd, INPUTS: inputs, OUTPUTS: outputs, TNAME: target.name, SHELL: shell}))
    }
    makeDirGlobals()
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
			buildConfigurationList = ${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.product}" */;
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

    output(section.expand(bit, eo).expand(bit.globals, eo).expand(ids))

    section = '\t\t\t\t${TID} /* ${TNAME} */,'
    for each (target in bit.xtargets) {
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

    for each (target in bit.targets) {
        if (!target.xbinary) {
           continue
        }
        let lines = []
        for each (file in target.files) {
            let obj = bit.targets[file]
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
    for each (target in bit.targets) {
        if (!target.xtarget) {
           continue
        }
        for each (item in target.depends) {
            let dep = bit.targets[item]
            if (!dep || !dep.xtarget) {
                continue
            }
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
        dep = bit.targets['lib' + lname]
        if (dep && dep.type == 'lib') {
            continue
        }
        dep = bit.targets[lname.replace(/\.dylib/, '')]
        if (dep && dep.type == 'lib') {
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
            o.includes.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);\n'
    }
    if (o.libpaths.length > 0) {
        options.libpaths = '\n\t\t\t\tLIBRARY_SEARCH_PATHS = (\n' + 
            o.libpaths.map(function(f) '\t\t\t\t\t"' + f.relativeTo(base) + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);'
    }
    let defines = o.defines.clone()
    if (debug) {
        defines.push('-DBIT_DEBUG')
    } else {
        defines.removeElements('-DDEBUG')
    }
    if (defines.length > 0) {
        options.defines = '\t\t\t\tGCC_PREPROCESSOR_DEFINITIONS = (\n' + 
            defines.map(function(f) '\t\t\t\t\t"' + f.replace('-D', '') + '",').join('\n') + '\n\t\t\t\t\t"$(inherited)"\n\t\t\t\t);'
    }
    let result = ''
    if (options.includes) result += options.includes
    if (options.defines) result += options.defines
    if (options.libpaths) result += options.libpaths
    if (options.linker) result += options.linker
    if (staticTarget) {
        result += '\n\t\t\t\tMACH_O_TYPE = staticlib;\n'
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

function projectConfigSection(base) {
    let common_settings = '
                /* Common Settings */
				ALWAYS_SEARCH_USER_PATHS = NO;
				ARCHS = "$(ARCHS_STANDARD_64_BIT)";
                CURRENT_PROJECT_VERSION = ${settings.version};
                DYLIB_COMPATIBILITY_VERSION = "$(CURRENT_PROJECT_VERSION)";
                DYLIB_CURRENT_VERSION = "$(CURRENT_PROJECT_VERSION)";
				GCC_C_LANGUAGE_STANDARD = gnu99;
				GCC_VERSION = ${COMPILER};
				GCC_WARN_ABOUT_RETURN_TYPE = YES;
                LD_DYLIB_INSTALL_NAME = "@rpath/$(EXECUTABLE_PATH)";
				MACOSX_DEPLOYMENT_TARGET = 10.7;
				SDKROOT = macosx;

                CFG_DIR = "${CFG}";
                BIN_DIR = "${BIN}";
                LIB_DIR = "${LIB}";
                INC_DIR = "${INC}";
                OBJ_DIR = "${OBJ}";
                SRC_DIR = "${SRC}";

                CONFIGURATION_TEMP_DIR = "$(OBJ_DIR)/tmp/$(CONFIGURATION)";
                CONFIGURATION_BUILD_DIR = "$(BIN_DIR)";
                INSTALL_PATH = "/usr/lib/${settings.product}";
                DSTROOT = "/tmp/${settings.product}.dst";
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

    let compiler = bit.packs.compiler.path.basename == 'gcc' ? 
                  'com.apple.compilers.llvmgcc42' : 'com.apple.compilers.llvm.clang.1_0'
    let overridable = appendSetting('', defaults.compiler, 
            ['GCC_WARN_64_TO_32_BIT_CONVERSION'], 'shorten-64-to-32')
    overridable += appendSetting(overridable, defaults.compiler, 
            ['GCC_WARN_UNUSED_VARIABLE', 'GCC_WARN_UNUSED_FUNCTION', 'GCC_WARN_UNUSED_LABEL'], 'unused-result')
    overridable += appendSetting(overridable, defaults.compiler, ['GCC_WARN_INHIBIT_ALL_WARNINGS'], '-w')
    makeDirGlobals(base)
    output(section.expand(ids, eo).expand({
        COMMON_SETTINGS: common_settings.expand(bit, eo).expand(bit.globals, eo).expand(ids, eo).expand({
            COMPILER: compiler,
        }),
        OVERRIDABLE_SETTINGS: overridable,
        DEBUG_SETTINGS: prepareSettings(base, bit.defaults, true),
        RELEASE_SETTINGS: prepareSettings(base, bit.defaults, false),
        WARN_UNUSED: yesno(defaults.compiler, 'unused-result'),
    }))

    /*
        Now the per-target settings
     */
    section = '\t\t${TARGET_DEBUG} /* Debug */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
${OVERRIDABLE_SETTINGS}
${DEBUG_SETTINGS}
			};
			name = Debug;
		};
		${TARGET_RELEASE} /* Release */ = {
			isa = XCBuildConfiguration;
			buildSettings = {
				PRODUCT_NAME = ${TNAME};
${OVERRIDABLE_SETTINGS}
${RELEASE_SETTINGS}
			};
			name = Release;
		};'
    for each (target in bit.targets) {
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
            compiler: target.compiler - bit.defaults.compiler,
            defines: target.defines - bit.defaults.defines,
            includes: target.includes - bit.defaults.includes,
            libpaths: target.libpaths - bit.defaults.libpaths,
            linker: target.linker - bit.defaults.linker,
            libraries: target.libraries - bit.defaults.libraries,
        }
        let overridable = appendSetting('', ts.compiler, 
            ['GCC_WARN_64_TO_32_BIT_CONVERSION'], 'shorten-64-to-32')
        overridable += appendSetting(overridable, ts.compiler, 
            ['GCC_WARN_UNUSED_VARIABLE', 'GCC_WARN_UNUSED_FUNCTION', 'GCC_WARN_UNUSED_LABEL'], 'unused-result')
        overridable += appendSetting(overridable, ts.compiler, ['GCC_WARN_INHIBIT_ALL_WARNINGS'], '-w')
        if (target.type == 'lib' && bit.settings.static) {
            overridable += '\t\t\t\tEXECUTABLE_EXTENSION = a;\n'
        }
        output(section.expand({TNAME: target.name, TARGET_DEBUG: tdid, TARGET_RELEASE: trid, 
            OVERRIDABLE_SETTINGS: overridable,
            DEBUG_SETTINGS: prepareSettings(base, ts, true),
            RELEASE_SETTINGS: prepareSettings(base, ts, false),
            WARN_UNUSED: yesno(ts.compiler, 'unused-result'),
            WARN_64_TO_32: yesno(ts.compiler, 'shorten-64-to-32'),
        }))
    }
    output('/* End XCBuildConfiguration section */')
    makeDirGlobals()
}

function targetConfigSection() {
    let section = '
/* Begin XCConfigurationList section */
		${ID_ProjectConfigList} /* Build configuration list for PBXProject "${settings.product}" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${ID_ProjectDebug} /* Debug */,
				${ID_ProjectRelease} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'
    output(section.expand(bit, eo).expand(bit.globals, eo).expand(ids))

    section = '\t\t${BCL} /* Build configuration list for PBXNativeTarget "${TNAME}" */ = {
			isa = XCConfigurationList;
			buildConfigurations = (
				${TARGET_DEBUG} /* Debug */,
				${TARGET_RELEASE} /* Release */,
			);
			defaultConfigurationIsVisible = 0;
			defaultConfigurationName = Release;
		};'

    for each (target in bit.targets) {
        if (!target.xtarget) {
            continue
        }
        let bcl = getid('ID_BuildConfigList:' + target.name)
        let tid = getid('ID_NativeTarget:' + target.name)
        let tdid = getid('ID_TargetDebugConfig:' + target.name)
        let trid = getid('ID_TargetReleaseConfig:' + target.name)
        output(section.expand(bit, eo).expand(bit.globals, eo).expand(ids, eo).
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

/*
    @copy   default
  
    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.
    Copyright (c) Michael O'Brien, 1993-2013. All Rights Reserved.
  
    This software is distributed under commercial and open source licenses.
    You may use the GPL open source license described below or you may acquire
    a commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details.
  
    This software is open source; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version. See the GNU General Public License for more
    details at: http://embedthis.com/downloads/gplLicense.html
  
    This program is distributed WITHOUT ANY WARRANTY; without even the
    implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  
    This GPL license does NOT permit incorporating this software into
    proprietary programs. If you are unable to comply with the GPL, you must
    acquire a commercial license to use this software. Commercial licenses
    for this software and support services are available from Embedthis
    Software at http://embedthis.com
  
    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
