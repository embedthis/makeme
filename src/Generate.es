/*
    Generate.es -- Generate MakeMe projects

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

class Generate {

    public var mappings: Object = {}
    public var generator: Object

    var builder: Builder
    var loader: Loader
    var options: Object

    public var minimalCflags = [ 
        '-O2', '-O', '-w', '-g', '-Wall', '-Wno-deprecated-declarations', '-Wno-unused-result', 
        '-Wshorten-64-to-32', '-Wunreachable-code', '-mtune=generic']

    var targetsToClean = { exe: true, file: true, lib: true, obj: true }

    function Generate() {
        loader = makeme.loader
        builder = makeme.builder
        options = makeme.options
    }

    public function projects() {
        makeme.generating = true
        let platforms = Object.getOwnPropertyNames(options.platforms)
        if (platforms.length == 0) {
            generateProjects()
        } else {
            for each (platform in platforms) {
                Me()
                loader.reset()
                loader.initPlatform(platform)
                let path = Loader.BUILD.join(platform, Loader.PLATFORM)
                if (path.exists) {
                    loader.loadFile(path)
                }
                builder.prepBuild()
                generateProjects()
            }
        }
        makeme.generating = null
    }

    function generateProjects() {
        me.settings.name ||= 'app'
        let base = me.dir.proj.join(me.settings.name + '-' + me.platform.os + '-' + me.platform.profile)
        base.makeDir()
        prepProject()

        /*
            Create project per-platform prototype me.h header
         */
        let path = me.dir.inc.join('me.h')
        let hfile = me.dir.top.join('projects', 
                me.settings.name + '-' + me.platform.os + '-' + me.platform.profile + '-me.h')
        if (path.exists) {
            trace('Generate', 'project header: ' + hfile.relative)
            path.copy(hfile)
        }
        for each (item in options.gen) {
            let generating = makeme.generating = item
            if (!makeme.generators[generating]) {
                let plugin = generating
                if (plugin == 'nmake' || plugin == 'sh') {
                    plugin = 'make'
                } else if (plugin == 'vs') {
                    plugin = 'vstudio'
                }
                let path = loader.findPlugin(plugin, false)
                if (path) {
                    let obj = loader.blendObj(loader.readFile(path))
                    loader.runScriptFromObj(obj.scripts, '+generator')
                }
            }
            generator = makeme.generators[generating]
            if (!generator) {
                throw 'Unknown generation format: ' + makeme.generating
            }
            trace('Generate', makeme.generating + ' file: ' + base.relative)
            generator.project(base, generating)
        }
    }

    function prepProject() {
        let cpack = me.targets.compiler
        let cflags = cpack.compiler.join(' ')
        for each (word in minimalCflags) {
            cflags = cflags.replace(word + ' ', ' ')
        }
        cflags = cflags.replace(/^ */, '')
        mappings = {
            configuration:  me.platform.name
            compiler:       cflags,
            defines :       cpack.defines.map(function(e) '-D' + e.replace(/"/, '\\"')).join(' '),
            includes:       cpack.includes.map(function(e) '-I' + e).join(' '),
            linker:         cpack.linker.join(' '),
            libpaths:       builder.mapLibPaths(cpack.libpaths),
            libraries:      builder.mapLibs(null, cpack.libraries).join(' '),
            build:          loader.BUILD + '/' + me.platform.name,
            lbin:           me.globals.LBIN ? me.globals.LBIN.relative : me.globals.BIN,
        }
        blend(mappings, me.prefixes)
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

    public function start() {
        safeCopy(Config.Bin.join('master-start.me'), 'start.me')
    }

    public function init(kind: String) {
        if (kind == 'start') {
            start()
        } else {
            main()
        }
    }

} /* class Generate */

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
