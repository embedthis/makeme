/*
   Vstudio.es -- Support functions for generating VS projects
        
   Exporting: vstudio()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
module embedthis.me {
    
require ejs.unix
    
class Vstudio {
    var out: Stream

    const TOOLS_VERSION = '4.0'
    const PROJECT_FILE_VERSION = 10.0.30319.1
    const SOL_VERSION = '11.00'
    const XID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
    var PREP

    var loader: Loader
    var builder: Builder
    var options: Object

    var prepTarget
    var Base: Path

    function Vstudio() {
        loader = makeme.loader
        builder = makeme.builder
        options = makeme.options
        PREP = `if not exist "$(ObjDir)" md "$(ObjDir)"
if not exist "$(BinDir)" md "$(BinDir)"
if not exist "$(IncDir)" md "$(IncDir)"
`
        if (me.dir.inc.join('me.h').exists) {
          PREP += `if not exist "$(IncDir)\\me.h" copy "..\\${settings.name}-${platform.os}-${platform.profile}-me.h" "$(IncDir)\\me.h"`
        }
    }

    public function generate(base: Path) {
        let saveGlobals = me.globals.clone()
        let saveDirs = me.dir.clone()
        for each (n in ["BIN", "OUT", "INC", "LIB", "OBJ", "PAKS", "PKG", "REL", "SRC", "TOP"]) {
            if (me.globals[n]) {
                me.globals[n] = me.globals[n].relativeTo(base)
            }
        }
        me.globals.LBIN = me.globals.BIN
        me.dir.bin = me.dir.out.join('bin')
        me.dir.inc = me.dir.out.join('inc')
        me.dir.obj = me.dir.out.join('obj')

        Base = base
        me.TOOLS_VERSION = TOOLS_VERSION
        me.PROJECT_FILE_VERSION = PROJECT_FILE_VERSION

        let projects = []
        /* Create a temporary prep target as the first target */
        prepTarget = loader.createTarget({
            type: 'vsprep',
            name: 'prep',
            home: base,
            enable: true,
            custom: PREP,
            includes: [], libraries: [], libpaths: [],
            generate: true,
        })
        let code = PREP
        for each (target in me.targets) {
            if (target.type == 'header') {
                for each (let file: Path in target.files) {
                    if (file == target.path) {
                        continue
                    }
                    code += '\ncopy /Y /B ' + wpath(file.relativeTo(base)) + ' ' + 
                        wpath(target.path.relativeTo(base).parent)
                }
            }
        }
        prepTarget.custom = code

        projBuild(projects, base, prepTarget)
        for each (target in me.targets) {
            projBuild(projects, base, target)
        }
        propBuild(base)
        solBuild(projects, base)
        me.globals = saveGlobals
        me.dir = saveDirs
    }

    /* TODO - should be a generic fuction */
    function getAllDeps(target): Array {
        let list = []
        for each (name in target.depends) {
            let dep = builder.getDep(name) 
            if (dep.enable) {
                list += getAllDeps(dep)
            }
        }
        for each (name in target.uses) {
            let dep = builder.getDep(name) 
            if (dep && dep.enable) {
                list += getAllDeps(dep)
            }
        }
        for each (name in target.uses) {
            let dep = builder.getDep(name) 
            if (dep && dep.enable && dep.selected) {
                list += getAllDeps(dep)
            }
        }
        if (target.guid) {
            list.push(target.name)
        }
        if (target.type == 'component' && target.depends.length > 0) {
            for each (dep in target.depends) {
                list += getAllDeps(dep)
            }
        }
        return list.unique()
    }

    function solBuild(projects, base: Path) {
        let path = base.joinExt('sln').relative
        trace('Generate', path)
        out = TextStream(File(path, 'wt'))
        output('Microsoft Visual Studio Solution File, Format Version ' + SOL_VERSION)
        output('# Visual Studio 2010')

        for each (target in projects) {
            target.guid = target.guid.toUpper()
            output('Project("' + XID + '") = "' + target.name + '", "' + 
                wpath(base.basename.join(target.name).joinExt('vcxproj', true)) + '", "{' + target.guid + '}"')
            /* Everything depends on prep */
            if (target != prepTarget) {
                let dep = prepTarget
                dep.guid = dep.guid.toUpper()
                output('\tProjectSection(ProjectDependencies) = postProject')
                output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
                output('\tEndProjectSection')
            }
            let depends = getAllDeps(target)
            for each (dname in depends) {
                let dep = builder.getDep(dname)
                if (!dep) continue
                if (dep.type == 'extension') {
                    for each (r in dep.libraries) {
                        let d = me.targets['lib' + r]
                        if (d && d.guid) {
                            output('\tProjectSection(ProjectDependencies) = postProject')
                            output('\t\t{' + d.guid + '} = {' + d.guid + '}')
                            output('\tEndProjectSection')

                        }
                    }
                } else {
                    if (!dep.guid) {
                        continue
                    }
                    dep.guid = dep.guid.toUpper()
                    output('\tProjectSection(ProjectDependencies) = postProject')
                    output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
                    output('\tEndProjectSection')
                }
            }
            output('EndProject')
        }
        output('
    Global
        GlobalSection(SolutionConfigurationPlatforms) = preSolution
            Debug|Win32 = Debug|Win32
            Debug|x64 = Debug|x64
            Release|Win32 = Release|Win32
            Release|x64 = Release|x64
        EndGlobalSection
        GlobalSection(ProjectConfigurationPlatforms) = postSolution')

        for each (target in projects) {
            output('{' + target.guid + '}.Debug|Win32.ActiveCfg = Debug|Win32')
            output('{' + target.guid + '}.Debug|Win32.Build.0 = Debug|Win32')
            output('{' + target.guid + '}.Debug|x64.ActiveCfg = Debug|x64')
            output('{' + target.guid + '}.Debug|x64.Build.0 = Debug|x64')
            output('{' + target.guid + '}.Release|Win32.ActiveCfg = Release|Win32')
            output('{' + target.guid + '}.Release|Win32.Build.0 = Release|Win32')
            output('{' + target.guid + '}.Release|x64.ActiveCfg = Release|x64')
            output('{' + target.guid + '}.Release|x64.Build.0 = Release|x64')
        }
        output('EndGlobalSection

      GlobalSection(SolutionProperties) = preSolution
        HideSolutionNode = FALSE
      EndGlobalSection
    EndGlobal')
        out.close()
    }

    function propBuild(base: Path) {
        productPropBuild(base)
        debugPropBuild(base)
        releasePropBuild(base)
        archPropBuild(base, 'x86')
        archPropBuild(base, 'x64')
    }

    function productPropBuild(base: Path) {
        let path = base.join('product.props').relative
        trace('Generate', path)
        out = TextStream(File(path, 'wt'))
        output('<?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="12.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ImportGroup Label="PropertySheets" />
      <ItemDefinitionGroup>
        <ClCompile>
          <AdditionalIncludeDirectories>$(IncDir);%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>
          <PreprocessorDefinitions>WIN32;_WINDOWS;_REENTRANT;_MT;%(PreprocessorDefinitions)</PreprocessorDefinitions>
        </ClCompile>
        <Link>
          <AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
          <AdditionalLibraryDirectories>$(OutDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
        </Link>
      </ItemDefinitionGroup>
    </Project>')
        out.close()
    }

    function debugPropBuild(base: Path) {
        let path = base.join('debug.props').relative
        trace('Generate', path)
        out = TextStream(File(path, 'wt'))
        let pathenv = ''
        if (Config.OS == 'windows') {
            let defaults = blend({}, me.defaults, {combine: true})
            let paths = defaults.libpaths ? defaults.libpaths.join(';') : []
            pathenv = `
      <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|Win32'">
        <LocalDebuggerEnvironment>PATH=` + paths + `;%PATH%;$(LocalDebugerEnvironment)</LocalDebuggerEnvironment>
      </PropertyGroup>`
        }

        output(`<?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ImportGroup Label="PropertySheets" />
      <PropertyGroup Label="UserMacros">
        <Cfg>` + me.platform.profile + `</Cfg>
      </PropertyGroup>` + pathenv + `
      <ItemDefinitionGroup>
        <ClCompile>
          <PreprocessorDefinitions>_DEBUG;ME_DEBUG;DEBUG_IDE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
          <Optimization>Disabled</Optimization>
          <BasicRuntimeChecks>EnableFastChecks</BasicRuntimeChecks>
          <RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>
        </ClCompile>
        <Link>
          <GenerateDebugInformation>true</GenerateDebugInformation>
        </Link>
      </ItemDefinitionGroup>
      <ItemGroup>
        <BuildMacro Include="Cfg">
        <Value>$(Cfg)</Value>
        <EnvironmentVariable>true</EnvironmentVariable>
      </BuildMacro>
      </ItemGroup>
    </Project>`)
        out.close()
    }

    function releasePropBuild(base: Path) {
        let path = base.join('release.props').relative
        trace('Generate', path)
        out = TextStream(File(path, 'wt'))
        output('<?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ImportGroup Label="PropertySheets" />
      <PropertyGroup Label="UserMacros">
        <Cfg>vsrelease</Cfg>
      </PropertyGroup>
      <ItemDefinitionGroup>
        <ClCompile>
          <Optimization>MinSpace</Optimization>
          <RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>
          <IntrinsicFunctions>true</IntrinsicFunctions>
          <FunctionLevelLinking>true</FunctionLevelLinking>
        </ClCompile>
        <Link>
          <GenerateDebugInformation>false</GenerateDebugInformation>
        </Link>
      </ItemDefinitionGroup>
      <ItemGroup>
        <BuildMacro Include="Cfg">
          <Value>$(Cfg)</Value>
          <EnvironmentVariable>true</EnvironmentVariable>
        </BuildMacro>
      </ItemGroup>
    </Project>')
        out.close()
    }

    function archPropBuild(base: Path, arch) {
        let path = base.join(arch + '.props').relative
        trace('Generate', path)
        out = TextStream(File(path, 'wt'))

        output('<?xml version="1.0" encoding="utf-8"?>
    <Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
      <ImportGroup Label="PropertySheets" />
      <PropertyGroup Label="UserMacros">
        <CfgDir>..\\..\\build\\windows-' + arch + '-$(Cfg)</CfgDir>
        <IncDir>$([System.IO.Path]::GetFullPath($(ProjectDir)\\$(CfgDir)\\inc))</IncDir>
        <ObjDir>$([System.IO.Path]::GetFullPath($(ProjectDir)\\$(CfgDir)\\obj))</ObjDir>
        <BinDir>$([System.IO.Path]::GetFullPath($(ProjectDir)\\$(CfgDir)\\bin))</BinDir>
      </PropertyGroup>
      <ItemGroup>
        <BuildMacro Include="CfgDir">
          <Value>$(CfgDir)</Value>
          <EnvironmentVariable>true</EnvironmentVariable>
        </BuildMacro>
        <BuildMacro Include="BinDir">
          <Value>$(BinDir)</Value>
          <EnvironmentVariable>true</EnvironmentVariable>
        </BuildMacro>
        <BuildMacro Include="IncDir">
          <Value>$(IncDir)</Value>
          <EnvironmentVariable>true</EnvironmentVariable>
        </BuildMacro>
        <BuildMacro Include="ObjDir">
          <Value>$(ObjDir)</Value>
          <EnvironmentVariable>true</EnvironmentVariable>
        </BuildMacro>
      </ItemGroup>
    </Project>')
        out.close()
    }

    function projBuild(projects: Array, base: Path, target) {
        if (target.vsbuilt || !target.enable || !target.generate) {
            return
        }
        if (target.type != 'exe' && target.type != 'lib' && target.type != 'vsprep') {
            if (!(target.type == 'build' || target.type == 'file' || 
                    (target.type == 'script' && target.goals.contains('all')))) {
                return
            }
        }
        global.TARGET = me.target = target
        if (target.files) {
            target.cmdfiles = target.files.join(' ')
        }
        for each (dname in target.depends) {
            let dep = me.targets[dname]
            if (dep && dep.enable && !dep.vsbuilt) {
                projBuild(projects, base, dep)
            }
        }
        target.project = base.join(target.name).joinExt('vcxproj', true).relative
        trace('Generate', target.project)
        projects.push(target)
        out = TextStream(File(target.project, 'wt'))
        projHeader(base, target)
        projConfig(base, target)
        projSources(base, target)
        projSourceHeaders(base, target)
        projResources(base, target)
        projLink(base, target)
        projDeps(base, target)
        projFooter(base, target)
        out.close()
        target.vsbuilt = true
    }

    function projHeader(base, target) {
        me.SUBSYSTEM = (target.rule == 'gui') ? 'Windows' : 'Console'
        me.INC = target.includes ? target.includes.map(function(path) wpath(path.relativeTo(base))).join(';') : ''
        me.DEF = target.defines ? target.defines.join(';') : ''
        output('<?xml version="1.0" encoding="utf-8"?>
    <Project DefaultTargets="Build" ToolsVersion="${TOOLS_VERSION}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')
        if (target.type == 'lib' || target.type == 'exe') {
            output('
      <ItemDefinitionGroup>
        <ClCompile>
          <PreprocessorDefinitions>${DEF};%(PreprocessorDefinitions)</PreprocessorDefinitions>
          <AdditionalIncludeDirectories>${INC};%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>')
            output('    </ClCompile>')
            output('    <Link>
          <AdditionalDependencies>ws2_32.lib;%(AdditionalDependencies)</AdditionalDependencies>
          <AdditionalLibraryDirectories>$(OutDir);%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
          <SubSystem>${SUBSYSTEM}</SubSystem>')
            output('    </Link>
      </ItemDefinitionGroup>')
        }
    }

    function projConfig(base, target) {
        if (target.type == 'exe') {
            me.PTYPE = 'Application'
        } else if (target.type == 'lib') {
            if (target.static) {
                me.PTYPE = 'StaticLibrary'
            } else {
                me.PTYPE = 'DynamicLibrary'
            }
        } else {
            me.PTYPE = ''
        }
        let guid = me.dir.proj.join('.' + target.name + '.guid')
        if (guid.exists) {
            target.guid = guid.readString().trim()
        } else {
            target.guid = Cmd('uuidgen').response.toLower().trim()
            guid.write(target.guid)
        }
        me.GUID = target.guid
        me.CTOK = '$(Configuration)'
        me.PTOK = '$(Platform)'
        me.STOK = '$(SolutionDir)'
        me.OTOK = '$(OutDir)'
        me.UTOK = '$(UserRootDir)'
        me.VTOK = '$(VCTargetsPath)'
        me.NAME = target.type == 'exe' ? target.path.basename.trimExt().name : target.name
        me.OUTDIR = wpath(me.dir.out.relativeTo(base))

        output('
      <PropertyGroup Label="Globals">
        <ProjectGuid>{${GUID}}</ProjectGuid>
        <RootNamespace />
        <Keyword>Win32Proj</Keyword>
      </PropertyGroup>')

        output('
      <ItemGroup Label="ProjectConfigurations">')
        for each (vtype in ['Win32', 'x64']) {
            for each (vout in ['Debug', 'Release']) {
                me.VTYPE = vtype
                me.VOUT = vout

                output('    <ProjectConfiguration Include="${VOUT}|${VTYPE}">
          <Configuration>${VOUT}</Configuration>
          <Platform>${VTYPE}</Platform>
        </ProjectConfiguration>')
            }
        }
        output('  </ItemGroup>
    ')

        for each (vtype in ['Win32', 'x64']) {
            for each (vout in ['Debug', 'Release']) {
                me.VTYPE = vtype
                me.VOUT = vout
                output('  <PropertyGroup Condition="\'${CTOK}|${PTOK}\'==\'${VOUT}|${VTYPE}\'" Label="Configuration">
        <ConfigurationType>${PTYPE}</ConfigurationType>
        <CharacterSet>NotSet</CharacterSet>
        <PlatformToolset>v120</PlatformToolset>
      </PropertyGroup>')
            }
        }

        output('
      <Import Project="${VTOK}\Microsoft.Cpp.Default.props" />
      <Import Project="${VTOK}\Microsoft.Cpp.props" />

      <ImportGroup Label="PropertySheets" />
      <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|Win32\'" Label="PropertySheets">
        <Import Project="product.props" />
        <Import Project="debug.props" />
        <Import Project="x86.props" />
      </ImportGroup>
      <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|Win32\'" Label="PropertySheets">
        <Import Project="product.props" />
        <Import Project="release.props" />
        <Import Project="x86.props" />
      </ImportGroup>
      <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Debug|x64\'" Label="PropertySheets">
        <Import Project="product.props" />
        <Import Project="debug.props" />
        <Import Project="x64.props" />
      </ImportGroup>
      <ImportGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'" Label="PropertySheets">
        <Import Project="product.props" />
        <Import Project="release.props" />
        <Import Project="x64.props" />
      </ImportGroup>

      <PropertyGroup>
        <_ProjectFileVersion>${PROJECT_FILE_VERSION}</_ProjectFileVersion>')
        for each (vtype in ['Win32', 'x64']) {
            for each (vout in ['Debug', 'Release']) {
                me.VTYPE = vtype
                me.VOUT = vout
                output('
        <OutDir Condition="\'${CTOK}|${PTOK}\'==\'${VOUT}|${VTYPE}\'">$(BinDir)\\</OutDir>
        <IntDir Condition="\'${CTOK}|${PTOK}\'==\'${VOUT}|${VTYPE}\'">$(ObjDir)\\${NAME}\\</IntDir>
        <CustomBuildBeforeTargets Condition="\'${CTOK}|${PTOK}\'==\'${VOUT}|${VTYPE}\'">PreBuildEvent</CustomBuildBeforeTargets>')
            }
        }
        output('  </PropertyGroup>')

        let name = target.path ? target.path.basename.trimExt().name : target.name
        if (name != target.name) {
            output('    <PropertyGroup>
        <TargetName>' + name + '</TargetName>
    </PropertyGroup>')
        }
    }

    function projSourceHeaders(base, target) {
        for each (dname in target.depends) {
            let dep = me.targets[dname]
            if (!dep || dep.type != 'header') continue
            output('
      <ItemGroup>')
            output('    <ClInclude Include="' + wpath(dep.path.relativeTo(base)) + '" />')
            output('  </ItemGroup>')
        }
    }

    function projSources(base, target) {
        if (target.sources) {
            output('  
      <ItemGroup>')
            for each (file in target.files) {
                let obj = me.targets[file]
                if (obj && obj.type == 'obj') {
                    for each (src in obj.files) {
                        let path = src.relativeTo(base)
                        output('    <ClCompile Include="' + wpath(path) + '" />')
                    }
                }
            }
            output('  </ItemGroup>')
        }
    }

    function projResources(base, target) {
        if (target.resources) {
            output('  
      <ItemGroup>')
            for each (resource in target.resources) {
                let path = resource.relativeTo(base)
                output('    <ResourceCompile Include="' + wpath(path) + '" />')
            }
            output('  </ItemGroup>')
        }
    }

    function projLink(base, target) {
            output('
      <ItemDefinitionGroup>
        ')
        if (target.type == 'exe' || target.type == 'lib') {
            me.LIBS = target.libraries ? builder.mapLibs(target, target.libraries - me.targets.compiler.libraries).join(';') : ''
            me.LIBPATHS = target.libpaths ? target.libpaths.map(function(p) wpath(p)).join(';') : ''
            output('
      <Link>
        <AdditionalDependencies>${LIBS};%(AdditionalDependencies)</AdditionalDependencies>
        <AdditionalLibraryDirectories>$(OutDir);${LIBPATHS};%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
      </Link>')
        }
        projCustomBuildStep(base, target)
        output('  </ItemDefinitionGroup>')
    }

    /*
        Emit a custom build step for exporting headers and the prep build step
     */
    function projCustomBuildStep(base, target) {
        let outfile: Path
        if (target.path) {
            me.OUT = outfile = wpath(target.path.relativeTo(base))
        } else {
            outfile = 'always'
        }
        let prefix, suffix
        if (target.home) {
            me.WIN_HOME = wpath(target.home.relativeTo(base))
            me.HOME = target.home.relativeTo(base)
            prefix = 'cd ' + target.home.relativeTo(base).windows + '\n'
            suffix = '\ncd ' + base.relativeTo(target.home).windows
        } else {
            prefix = suffix = ''
        }
        let command = target.custom || ''
        if (target.depends) {
            command += exportHeaders(base, target)
        }
        if (target.type == 'file') {
            for each (let file: Path in target.files) {
                let path = target.path
                path = path.relativeTo(Base)
                command += 'if exist ' + wpath(path) + ' del /Q ' + wpath(path) + '\n'
                if (file.isDir) {
                    command += '\tif not exist ' + wpath(path) + ' md ' + wpath(path) + '\n'
                    command += '\txcopy /S /Y ' + wpath(file.relativeTo(target.home)) + ' ' + wpath(path) + '\n'
                } else {
                    command += '\tcopy /Y ' + wpath(file.relativeTo(target.home)) + ' ' + wpath(path) + '\n'
                }
            }
        } else if (target.generate === true) {
            let cmdtmp = Path('me-vs.tmp')
            genOpen(cmdtmp)
            if (target.files) {
                global.FILES = target.files.map(function(f) f.relativeTo(target.home).portable).join(' ')
            } else {
                global.FILES = ''
            }
            me.globals.FILES = global.FILES
            builder.runTargetScript(target, 'build')
            genClose()
            let data = cmdtmp.readString() + '\n'
            command += data.replace(/^[ \t]*/mg, '').trim().replace(/^-md /m, 'md ').replace(/^-rd /m, 'rd ')
            cmdtmp.remove()

        } else if (target['generate-vs']) {
            command += target['generate-vs']

        } else if (target['generate-nmake']) {
            let ncmd = target['generate-nmake']
            ncmd = ncmd.replace(/^[ \t]*/mg, '').trim().replace(/^-md /m, 'md ').replace(/^-rd /m, 'rd ')
            command += ncmd

        } else if (target['generate']) {
            let ncmd = target['generate']
            ncmd = ncmd.replace(/^[ \t]*/mg, '').trim()
            command += ncmd
        }
        command = command.replace(/^[ \t]*/mg, '')
        command = command.replace(/^cp /mg, 'copy ').trim()
        if (command != '') {
            command = prefix + command + suffix
            let saveBin = me.BIN, saveLib = me.LIB, saveLbin = me.LBIN
            me.BIN = '$(BinDir)'
            me.LBIN = '$(BinDir)'
            me.LIB = '$(BinDir)'
            output('
      <CustomBuildStep>
        <Command>' + command + '</Command>
        <Outputs>' + wpath(outfile.relative) + '</Outputs>
      </CustomBuildStep>')
            me.BIN = saveLbin
            me.LBIN = saveBin
            me.LIB = saveLib
        }
    }

    function exportHeaders(base, target) {
        let cmd = ''
        for each (dname in target.depends) {
            let dep = me.targets[dname]
            if (!dep || dep.type != 'header') continue
            for each (file in dep.files) {
                /* Use the directory in the destination so Xcopy won't ask if file or directory */
                if (file.isDir) {
                    cmd += 'xcopy /Y /S /D ' + wpath(file.relativeTo(target.home)) + ' ' + 
                        wpath(dep.path.relativeTo(base).parent) + '\n'
                } else {
                    cmd += '\tcopy /Y /B ' + wpath(file.relativeTo(target.home)) + ' ' + 
                        wpath(dep.path.relativeTo(base).parent) + '\n'
                }
            }
        }
        return cmd
    }

    function projDeps(base, target) {
        for each (dname in target.depends) {
            let dep = me.targets[dname]
            if (!dep) {
                if ((me.targets[dname] && me.targets[dname].type == 'extension') || dname == 'build') {
                    continue
                }
                throw 'Missing dependency ' + dname + ' for target ' + target.name
            }
            if (dep.type != 'exe' && dep.type != 'lib') {
                continue
            }
            if (!dep.enable) {
                continue
            }
            if (!dep.guid) {
                throw 'Missing guid for ' + dname
            }
            me.DEP = dname
            me.GUID = dep.guid
            output('
    <ItemGroup>
      <ProjectReference Include="${DEP}.vcxproj">
      <Project>${GUID}</Project>
      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>
      </ProjectReference>
    </ItemGroup>')
        }
    }

    function projFooter(base, target) {
        output('\n  <Import Project="${VTOK}\Microsoft.Cpp.targets" />')
        output('  <ImportGroup Label="ExtensionTargets">\n  </ImportGroup>\n\n</Project>')
    }

    function output(line: String) {
        out.writeLine(line.expand(me))
    }

    function replacePath(str, path, substitute) {
        if (path == '.') {
            return str
        }
        let pattern = path.replace(/\./g, '\\.')
        return str.replace(RegExp(pattern, 'g'), substitute)
    }

    /*
        Path is relative to Base
     */
    function wpath(path: Path): Path {
        name = path.relative.name
    /* UNUSED
        if (name.startsWith('..\\..')) {
            // Path outside local tree 
            name = Base.join(path).absolute.name
        } else 
    */
        {
            name = replacePath(name, me.dir.inc.relativeTo(Base), '$$(IncDir)')
            name = replacePath(name, me.dir.obj.relativeTo(Base), '$$(ObjDir)')
            name = replacePath(name, me.dir.bin.relativeTo(Base), '$$(BinDir)')
            name = replacePath(name, me.platform.name, '$$(Cfg)')
        }
        return Path(name.replace(/\//g, '\\'))
    }
} /* Class Vstudio */
} /* embedthis.me */

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
