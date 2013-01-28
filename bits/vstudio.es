/*
   vstudio.es -- Support functions for generating VS projects
        
   Exporting: vstudio()

   Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
*/     
    
require ejs.unix
    
var out: Stream

const TOOLS_VERSION = '4.0'
const PROJECT_FILE_VERSION = 10.0.30319.1
const SOL_VERSION = '11.00'
const XID = '{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}'
var PREP = 'if not exist "$(ObjDir)" md "$(ObjDir)"
if not exist "$(BinDir)" md "$(BinDir)"
if not exist "$(IncDir)" md "$(IncDir)"
if not exist "$(IncDir)\\bitos.h" copy "..\\..\\src\\bitos.h" "$(IncDir)\\bitos.h"
if not exist "$(IncDir)\\bit.h" copy "..\\${settings.product}-${platform.os}-${platform.profile}-bit.h" "$(IncDir)\\bit.h"
'
var prepTarget
var Base 

public function vstudio(base: Path) {
    //  TODO refactor
    Base = base
    bit.TOOLS_VERSION = TOOLS_VERSION
    bit.PROJECT_FILE_VERSION = PROJECT_FILE_VERSION
    let saveDir = []
    for each (n in ["BIN", "CFG", "FLAT", "INC", "LIB", "OBJ", "PACKS", "PKG", "REL", "SRC", "TOP"]) {
        saveDir[n] = bit.globals[n]
        bit.globals[n] = bit.globals[n].relativeTo(base)
    }
    let projects = []
/* UNUSED
    if (bit.dir.bin.files('*.def').length > 0) {
        PREP += 'if not exist "$(BinDir)\\libmpr.def" xcopy /Y /S *.def "$(BinDir)"'
    }
 */
    /* Create a temporary prep target as the first target */
    prepTarget = {
        type: 'vsprep',
        path: Path('always'),
        name: 'prep',
        enable: true,
        custom: PREP,
        includes: [], libraries: [], libpaths: [],
    }
    projBuild(projects, base, prepTarget)
    for each (target in bit.targets) {
        projBuild(projects, base, target)
    }
    propBuild(base)
    solBuild(projects, base)
    for (n in saveDir) {
        bit.globals[n] = saveDir[n]
    }
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
        for each (dname in target.depends) {
            let dep = bit.targets[dname]
            if (!dep || !dep.guid) {
                continue
            }
            dep.guid = dep.guid.toUpper()
            output('\tProjectSection(ProjectDependencies) = postProject')
            output('\t\t{' + dep.guid + '} = {' + dep.guid + '}')
            output('\tEndProjectSection')
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
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
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

    //  TODO - remove MultiThreadedDebugDll
    output('<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ImportGroup Label="PropertySheets" />
  <PropertyGroup Label="UserMacros">
    <Cfg>vsdebug</Cfg>
  </PropertyGroup>
  <ItemDefinitionGroup>
    <ClCompile>
      <PreprocessorDefinitions>_DEBUG;BIT_DEBUG;DEBUG_IDE;%(PreprocessorDefinitions)</PreprocessorDefinitions>
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
</Project>')
    out.close()
}

function releasePropBuild(base: Path) {
    let path = base.join('release.props').relative
    trace('Generate', path)
    out = TextStream(File(path, 'wt'))
    //  TODO - remove MultiThreadedDebugDll
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
    <CfgDir>..\\..\\windows-' + arch + '-$(Cfg)</CfgDir>
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
    if (target.vsbuilt || !target.enable || target.nogen) {
        return
    }
    //  TODO - refactor
    if (target.type != 'exe' && target.type != 'lib' && target.type != 'vsprep') {
        if (!(target.type == 'build' || target.type == 'file')) {
            return
        }
    }
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
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
    bit.SUBSYSTEM = (target.rule == 'gui') ? 'Windows' : 'Console'
    bit.INC = target.includes ? target.includes.map(function(path) wpath(path.relativeTo(base))).join(';') : ''

    output('<?xml version="1.0" encoding="utf-8"?>
<Project DefaultTargets="Build" ToolsVersion="${TOOLS_VERSION}" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">')


    if (target.type == 'lib' || target.type == 'exe') {
        output('
  <ItemDefinitionGroup>
    <ClCompile>
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
    bit.PTYPE = (target.type == 'exe') ? 'Application' : 'DynamicLibrary'
    let guid = bit.dir.proj.join('.' + target.name + '.guid')
    if (guid.exists) {
        target.guid = guid.readString().trim()
    } else {
        target.guid = Cmd('uuidgen').response.toLower().trim()
        guid.write(target.guid)
    }
    bit.GUID = target.guid
    bit.CTOK = '$(Configuration)'
    bit.PTOK = '$(Platform)'
    bit.STOK = '$(SolutionDir)'
    bit.OTOK = '$(OutDir)'
    bit.UTOK = '$(UserRootDir)'
    bit.VTOK = '$(VCTargetsPath)'
    bit.NAME = target.name
    bit.OUTDIR = wpath(bit.dir.cfg.relativeTo(base))

    output('
  <PropertyGroup Label="Globals">
    <ProjectGuid>{${GUID}}</ProjectGuid>
    <RootNamespace />
    <Keyword>Win32Proj</Keyword>
  </PropertyGroup>')

    output('
  <ItemGroup Label="ProjectConfigurations">')
    for each (vtype in ['Win32', 'x64']) {
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg

            output('    <ProjectConfiguration Include="${VCFG}|${VTYPE}">
      <Configuration>${VCFG}</Configuration>
      <Platform>${VTYPE}</Platform>
    </ProjectConfiguration>')
        }
    }
    output('  </ItemGroup>
')

    for each (vtype in ['Win32', 'x64']) {
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg
            output('  <PropertyGroup Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'" Label="Configuration">
    <ConfigurationType>${PTYPE}</ConfigurationType>
    <CharacterSet>NotSet</CharacterSet>
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
        for each (vcfg in ['Debug', 'Release']) {
            bit.VTYPE = vtype
            bit.VCFG = vcfg
            output('
    <OutDir Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">$(BinDir)\\</OutDir>
    <IntDir Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">$(ObjDir)\\${NAME}\\</IntDir>
    <CustomBuildBeforeTargets Condition="\'${CTOK}|${PTOK}\'==\'${VCFG}|${VTYPE}\'">PreBuildEvent</CustomBuildBeforeTargets>')
        }
    }
    output('  </PropertyGroup>')
}

function projSourceHeaders(base, target) {
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
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
            let obj = bit.targets[file]
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

//  TODO - rename. Does more than just link. Also does 'files' and 'scripts'
function projLink(base, target) {
/* UNUSED
    if (target.type == 'lib') {
        let def = base.join(target.path.basename.toString().replace(/dll$/, 'def'))
        let newdef = Path(target.path.toString().replace(/dll$/, 'def'))
        if (newdef.exists) {
            cp(newdef, def)
            trace('Copy', def)
        }
        if (def.exists) {
            bit.DEF = wpath(def.relativeTo(base))
            output('
  <ItemDefinitionGroup>
    <Link>
      <ModuleDefinitionFile>${DEF}</ModuleDefinitionFile>
    </Link>
  </ItemDefinitionGroup>')
        } else {
            trace('Warn', 'Missing ' + def)
        }
    }
*/
    bit.LIBS = target.libraries ? mapLibs(target.libraries - bit.defaults.libraries).join(';') : ''
    bit.LIBPATHS = target.libpaths ? target.libpaths.map(function(p) wpath(p)).join(';') : ''
    output('
  <ItemDefinitionGroup>
  <Link>
    <AdditionalDependencies>${LIBS};%(AdditionalDependencies)</AdditionalDependencies>
    <AdditionalLibraryDirectories>$(OutDir);${LIBPATHS};%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>
  </Link>')
    projCustomBuildStep(base, target)
    output('  </ItemDefinitionGroup>')
}

/*
    Emit a custom build step for exporting headers and the prep build step
 */
function projCustomBuildStep(base, target) {
    let outfile
    if (target.path) {
        bit.OUT = outfile = wpath(target.path.relativeTo(base))
    } else {
        outfile = 'always'
    }
    let prefix, suffix
    if (target.home) {
        bit.WIN_HOME = wpath(target.home.relativeTo(base))
        bit.HOME = target.home.relativeTo(base)
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
            let path = target.path.relativeTo(Base)
            command += 'if exist ' + wpath(path) + ' del /Q ' + wpath(path) + '\n'
            if (file.isDir) {
                command += '\tif not exist ' + wpath(path) + ' md ' + wpath(path) + '\n'
                command += '\txcopy /S /Y ' + wpath(file.relativeTo(target.home)) + ' ' + wpath(path) + '\n'
            } else {
                command += '\tcopy /Y ' + wpath(file.relativeTo(target.home)) + ' ' + wpath(path) + '\n'
            }
        }
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
        let saveBin = bit.BIN, saveLib = bit.LIB
        bit.BIN = '$(BinDir)'
        bit.LIB = '$(BinDir)'
        output('
  <CustomBuildStep>
    <Command>' + command + '</Command>
    <Outputs>' + wpath(outfile) + '</Outputs>
  </CustomBuildStep>')
        bit.BIN = saveBin
        bit.LIB = saveLib
    }
}

function exportHeaders(base, target) {
    let cmd = ''
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep || dep.type != 'header') continue
        for each (file in dep.files) {
            /* Use the directory in the destination so Xcopy won't ask if file or directory */
            if (file.isDir) {
                cmd += 'xcopy /Y /S /D ' + wpath(file.relativeTo(target.home)) + ' ' + 
                    wpath(dep.path.relativeTo(base).parent) + '\n'
            } else {
                cmd += '\tcopy /Y ' + wpath(file.relativeTo(target.home)) + ' ' + 
                    wpath(dep.path.relativeTo(base).parent) + '\n'
            }
        }
    }
    return cmd
}

function projDeps(base, target) {
    for each (dname in target.depends) {
        let dep = bit.targets[dname]
        if (!dep) {
            if (bit.packs[dname] || dname == 'build') {
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
        bit.DEP = dname
        bit.GUID = dep.guid
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
    out.writeLine(line.expand(bit))
}

function wpath(path): Path {
    path = path.relative.name
    path = path.replace(bit.dir.inc.relativeTo(Base), '$(IncDir)')
    path = path.replace(bit.dir.obj.relativeTo(Base), '$(ObjDir)')
    path = path.replace(bit.dir.bin.relativeTo(Base), '$(BinDir)')
    path = path.replace(bit.platform.name, '$(Cfg)')
    return Path(path.toString().replace(/\//g, '\\'))
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
