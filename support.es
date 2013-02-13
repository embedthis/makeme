/*
    Support functions for Embedthis Bit

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix

/*
    Copy binary files to package staging area
    This is run for local and cross platforms. The last platform does the packaging
 */
public function packageDeploy(minimal = false) {
    safeRemove(bit.dir.pkg)

    let settings = bit.settings
    let binrel = bit.dir.pkg.join('binrel')
    let vname = settings.product + '-' + settings.version + '-' + settings.buildNumber
    let pkg = binrel.join(vname)
    pkg.makeDir()

    let contents = pkg.join('contents')
    let prefixes = bit.prefixes

    let p = {}
    for (pname in bit.prefixes) {
        p[pname] = Path(contents.portable.name + bit.prefixes[pname].removeDrive().portable)
        p[pname].makeDir()
    }
    let pbin = p.vapp.join('bin')
    let strip = bit.platform.profile == 'debug'

    trace('Deploy', bit.settings.title)

    if (!bit.cross) {
        /* These three files are replicated outside the data directory */
        install('doc/product/README.TXT', pkg, {fold: true, expand: true})
        install('package/install.sh', pkg.join('install'), {permissions: 0755, expand: true})
        install('package/uninstall.sh', pkg.join('uninstall'), {permissions: 0755, expand: true})
        if (bit.platform.os == 'windows') {
            install('package/windows/LICENSE.TXT', binrel, {fold: true, expand: true})
        }
        /* Move bit-license to the front */
        let files = Path('doc/licenses').files('*.txt').reject(function(p) p.contains('bit-license.txt'))
        install(['doc/licenses/bit-license.txt'] + files, p.app.join('LICENSE.TXT'), {
            cat: true,
            textfile: true,
            fold: true,
            title: bit.settings.title + ' Licenses',
        })
        install('doc/product/README.TXT', p.app, {fold: true, expand: true})
        install('package/uninstall.sh', pbin.join('uninstall'), {permissions: 0755, expand: true})
        install('package/linkup', pbin, {permissions: 0755})
    }
    install(bit.dir.bin + '/*', pbin, {
        include: /bit|ca.crt|ejs.mod|\.dll|lib.*/,
        permissions: 0755, 
        show: true,
    })
    install(bit.dir.bin.join('bits'), pbin)
    install(bit.dir.bin.join('ca.crt'), pbin)

    if (bit.targets.libmprssl.enable && bit.platform.os == 'linux') {
        install(bit.dir.bin.join('*.' + bit.ext.shobj + '*'), pbin, {strip: strip, permissions: 0755})
        for each (f in pbin.files('*.so.*')) {
            let withver = f.basename
            let nover = withver.name.replace(/\.[0-9]*.*/, '.so')
            let link = pbin.join(nover)
            f.remove()
            f.symlink(link.basename)
        }
    }
    if (!bit.cross) {
        if (bit.platform.os == 'windows') {
            let version = bit.packs.compiler.version.replace('.', '')
            if (bit.platform.arch == 'x64') {
                install(bit.packs.compiler.dir.join('VC/redist/x64/Microsoft.VC' +
                    version + '.CRT/msvcr' + version + '.dll'), pbin)                                     
            } else {
                install(bit.packs.compiler.dir.join('VC/redist/x86/Microsoft.VC' + 
                    version + '.CRT/msvcr' + version + '.dll'), pbin)
            }
            /*
                install(bit.packs.compiler.path.join('../../lib/msvcrt.lib'), pbin)
             */
            install(bit.dir.bin.join('removeFiles' + bit.globals.EXE), pbin)
        }
        if (bit.platform.like == 'posix') {
            install('doc/man/bit.1', p.vapp.join('doc/man/man1/bit.1'), {compress: true})
        }
    }
    let files = contents.files('**', {exclude: /\/$/, relative: true})
    files = files.map(function(f) Path("/" + f))
    p.vapp.join('files.log').append(files.join('\n') + '\n')

    /*
        Remove empty prefixes. Will fail if there are any files.
     */
    for (i in 3) {
        for (let [name, prefix] in p) {
            prefix.remove()
        }
    }
}


public function packageSourceFiles() {
    if (bit.cross) {
        return
    }
    let s = bit.settings
    let src = bit.dir.pkg.join('src')
    let pkg = src.join(s.product + '-' + s.version)
    safeRemove(pkg)
    pkg.makeDir()
    install(['Makefile', 'start.bit', 'main.bit'], pkg)
    install('bits', pkg)
    install('*.md', pkg, {fold: true, expand: true})
    install('configure', pkg, {permissions: 0755})
    install('src', pkg, {
        exclude: /\.log$|\.lst$|\.stackdump$|\/cache|huge.txt|\.swp$|\.tmp/,
    })
    install('doc', pkg, {
        exclude: /\/xml\/|\/html\/|Archive|\.mod$|\.so$|\.dylib$|\.o$/,
    })
    install('projects', pkg, {
        exclude: /\/Debug\/|\/Release\/|\.ncb|\.mode1v3|\.pbxuser/,
    })
    install('package', pkg, {})
    package(src, 'src')
}


public function packageComboFiles() {
    if (bit.cross) {
        return
    }
    let s = bit.settings
    let src = bit.dir.pkg.join('src')
    let pkg = src.join(s.product + '-' + s.version)
    safeRemove(pkg)
    pkg.makeDir()
    install('projects/bit-' + bit.platform.os + '-default-bit.h', pkg.join('src/deps/bit/bit.h'))
    install('package/start-flat.bit', pkg.join('src/deps/bit/start.bit'))
    install('package/Makefile-flat', pkg.join('src/deps/bit/Makefile'))
    install(['src/deps/mpr/mpr.h', 'src/deps/http/http.h', 'src/deps/pcre/pcre.h'], 
        pkg.join('src/deps/bit/deps.h'), {
        cat: true,
        filter: /^#inc.*bit.*$|^#inc.*mpr.*$|^#inc.*http.*$|^#inc.*customize.*$|^#inc.*edi.*$|^#inc.*mdb.*$|^#inc.*esp.*$/mg,
        title: bit.settings.title + ' Library Source',
    })
    install(['src/deps/**.c'], pkg.join('src/deps/bit/deps.c'), {
        cat: true,
        filter: /^#inc.*bit.*$|^#inc.*mpr.*$|^#inc.*http.*$|^#inc.*customize.*$|^#inc.*edi.*$|^#inc.*mdb.*$|^#inc.*esp.*$/mg,
        exclude: /pcre|makerom|http\.c|sqlite|manager|ejs/,
        header: '#include \"bit.h\"',
        title: bit.settings.title + ' Library Source',
    })
    install(['src/**.c'], pkg.join('src/deps/bit/bitLib.c'), {
        cat: true,
        filter: /^#inc.*bit.*$|^#inc.*mpr.*$|^#inc.*http.*$|^#inc.*customize.*$|^#inc.*edi.*$|^#inc.*mdb.*$|^#inc.*esp.*$/mg,
        exclude: /deps|server.bit.c|esp\.c|ejs|samples|romFiles|pcre|sqlite|appman|makerom|utils|test|http\.c|sqlite|manager/,
        header: '#include \"bit.h\"',
        title: bit.settings.title + ' Library Source',
    })
    install(['src/bit.c'], pkg.join('src/bit.c'))
    install(['src/bit.es'], pkg.join('src/bit.es'))

    install(['src/deps/pcre/pcre.c', 'src/deps/pcre/pcre.h'], pkg.join('src/deps/bit'))
    package(bit.dir.pkg.join('src'), ['combo', 'flat'])
}


public function packageBinaryFiles(formats = ['tar', 'native'], minimal = false) {
    packageDeploy(minimal)
    if (bit.platform.last) {
        package(bit.dir.pkg.join('binrel'), formats)
    }
}


public function installBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit install\"'
    }
    /* Preserve bit */
    let path = App.exePath
    if (path.same(bit.prefixes.bin.join('bit'))) {
        active = path.replaceExt('old')
        active.remove()
        path.rename(active)
    }
    package(bit.dir.pkg.join('binrel'), 'install')
    if (Config.OS != 'windows') {
        createLinks()                                                                                          
        updateLatestLink()                                                                                          
    }
    if (!bit.options.keep) {
        bit.dir.pkg.join('binrel').removeAll()
    } else {
        trace('Keep', bit.dir.pkg.join('binrel'))
    }
    trace('Complete', bit.settings.title + ' installed')
}


public function uninstallBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit uninstall\"'
    }
    trace('Uninstall', bit.settings.title)
    let fileslog = bit.prefixes.vapp.join('files.log')
    if (fileslog.exists) {
        for each (let file: Path in fileslog.readLines()) {
            strace('Remove', file)
            file.remove()
        }
    }
    fileslog.remove()
    for each (file in bit.prefixes.log.files('*.log*')) {
        strace('Remove', file)
        file.remove()
    }
    for each (prefix in bit.prefixes) {
        if (bit.platform.os == 'windows') {
            if (!prefix.name.contains(bit.settings.title)) continue
        } else {
            if (!prefix.name.contains(bit.settings.product)) continue
        }
        for each (dir in prefix.files('**', {include: /\/$/}).sort().reverse()) {
            strace('Remove', dir)
            dir.removeAll()
        }
        strace('Remove', prefix)
        prefix.removeAll()
    }
    updateLatestLink()
    trace('Complete', bit.settings.title + ' uninstalled')                                                 
}

/*
    Create symlinks for binaries and man pages
 */
public function createLinks() {
    let log = []
    let bin = bit.prefixes.bin
    bin.makeDir()
    let programs = ['bit' ]
    let pbin = bit.prefixes.vapp.join('bin')
    let target: Path
    for each (program in programs) {
        let link = Path(bin.join(program))
        link.symlink(pbin.join(program + bit.globals.EXE))
        strace('Link', link)
        log.push(link)
    }
    for each (page in bit.prefixes.vapp.join('doc/man').files('**/*.1.gz')) {
        let link = bit.prefixes.man.join('man1').join(page.basename)
        link.symlink(page)
        strace('Link', link)
        log.push(link)
    }
}

function updateLatestLink() {
    let latest = bit.prefixes.app.join('latest')
    let version = bit.prefixes.app.files('*', {include: /\d+\.\d+\.\d+/}).sort().pop()
    if (version) {
        latest.symlink(version.basename)
    } else {
        latest.remove()
    }
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
