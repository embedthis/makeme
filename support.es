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
public function packageBinaryFiles(formats = ['tar', 'native']) {
    let settings = bit.settings
    let bin = bit.dir.pkg.join('bin')
    safeRemove(bit.dir.pkg)
    let vname = settings.product + '-' + settings.version + '-' + settings.buildNumber
    let pkg = bin.join(vname)
    pkg.makeDir()

    let contents = pkg.join('contents')

    let prefixes = bit.prefixes;
    let p = {}
    for (prefix in bit.prefixes) {
        if (prefix == 'config' || prefix == 'log' || prefix == 'spool' || prefix == 'src' || prefix == 'web' || prefix == 'inc') {
            continue
        }
        p[prefix] = Path(contents.portable.name + bit.prefixes[prefix].removeDrive().portable)
        p[prefix].makeDir()
    }
    let strip = bit.platform.profile == 'debug'

    if (!bit.cross) {
        /* These three files are replicated outside the data directory */
        install('doc/product/README.TXT', pkg, {fold: true, expand: true})
        install('package/install.sh', pkg.join('install'), {permissions: 0755, expand: true})
        install('package/uninstall.sh', pkg.join('uninstall'), {permissions: 0755, expand: true})
        if (bit.platform.os == 'windows') {
            install('package/windows/LICENSE.TXT', bin, {fold: true, expand: true})
        }
        /* Move bit-license to the front */
        let files = Path('doc/licenses').files('*.txt').reject(function(p) p.contains('bit-license.txt'))
        install(['doc/licenses/bit-license.txt'] + files, p.product.join('LICENSE.TXT'), {
            cat: true,
            textfile: true,
            fold: true,
            title: bit.settings.title + ' Licenses',
        })
        install('doc/product/README.TXT', p.product, {fold: true, expand: true})
        install('package/uninstall.sh', p.bin.join('uninstall'), {permissions: 0755, expand: true})
        install('package/linkup', p.bin, {permissions: 0755})
    }
    install(bit.dir.bin + '/*', p.bin, {
        include: /bit|\.dll/,
        exclude: /ejs\$|ejsc\$|http\\$|makerom|\.dylib|\.so|\.pdb|\.exp|\.lib|\.def|\.suo|\.old/,
        permissions: 0755, 
    })
    install(bit.dir.bin.join('bits'), p.bin)
    install(bit.dir.bin.join('ca.crt'), p.bin)

    if (bit.targets.libmprssl.enable && bit.platform.os == 'linux') {
        install(bit.dir.bin.join('*.' + bit.ext.shobj + '*'), p.bin, {strip: strip, permissions: 0755})
        for each (f in p.bin.files('*.so.*')) {
            let withver = f.basename
            let nover = withver.name.replace(/\.[0-9]*.*/, '.so')
            let link = p.bin.join(nover)
            f.remove()
            f.symlink(link.basename)
        }
    }
    if (!bit.cross) {
        if (bit.platform.os == 'windows') {
            let version = bit.packs.compiler.version.replace('.', '')
            if (bit.platform.arch == 'x64') {
                install(bit.packs.compiler.dir.join('VC/redist/x64/Microsoft.VC' +
                    version + '.CRT/msvcr' + version + '.dll'), p.bin)                                     
            } else {
                install(bit.packs.compiler.dir.join('VC/redist/x86/Microsoft.VC' + 
                    version + '.CRT/msvcr' + version + '.dll'), p.bin)
            }
            /*
                install(bit.packs.compiler.path.join('../../lib/msvcrt.lib'), p.bin)
             */
            install(bit.dir.bin.join('removeFiles' + bit.globals.EXE), p.bin)
        }
        if (bit.platform.like == 'posix') {
            install('doc/man/bit.1', p.productver.join('doc/man/man1/bit.1'), {compress: true})
        }
    }
    let files = contents.files('**', {exclude: /\/$/, relative: true})
    files = files.map(function(f) Path("/" + f))
    p.productver.join('files.log').append(files.join('\n') + '\n')

    if (formats && bit.platform.last) {
        package(bit.dir.pkg.join('bin'), formats)
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

public function installBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit install\"'
    }
    packageBinaryFiles(null)
    /* Preserve bit */
    let path = App.exePath
    if (path.same(bit.prefixes.bin.join('bit'))) {
        active = path.replaceExt('old')
        active.remove()
        path.rename(active)
    }
    package(bit.dir.pkg.join('bin'), 'install')
    if (Config.OS != 'windows') {
        createLinks()                                                                                          
        updateLatestLink()                                                                                          
    }
    if (!bit.options.keep) {
        bit.dir.pkg.join('bin').removeAll()
    } else {
        trace('Keep', bit.dir.pkg.join('bin'))
    }
    trace('Complete', bit.settings.title + ' installed')
}

public function uninstallBinary() {
    if (Config.OS != 'windows' && App.uid != 0) {
        throw 'Must run as root. Use \"sudo bit uninstall\"'
    }
    let fileslog = bit.prefixes.productver.join('files.log')
    if (fileslog.exists) {
        for each (let file: Path in fileslog.readLines()) {
            strace('Remove', file)
            file.remove()
        }
    }
    fileslog.remove()
    for each (file in bit.prefixes.log.files('*.log*')) {
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
            dir.remove()
        }
        strace('Remove', prefix)
        prefix.remove()
    }
    updateLatestLink()
}

/*
    Create symlinks for binaries and man pages
 */
public function createLinks() {
    let log = []
    let localbin = Path('/usr/local/bin')
    if (localbin.exists) {
        let programs = ['bit' ]
        let bin = bit.prefixes.bin
        let target: Path
        for each (program in programs) {
            let link = Path(localbin.join(program))
            link.symlink(bin.join(program + bit.globals.EXE))
            log.push(link)
        }
        for each (page in bit.prefixes.productver.join('doc/man').files('**/*.1.gz')) {
            let link = Path('/usr/share/man/man1/' + page.basename)
            link.symlink(page)
            log.push(link)
        }
    }
}

function updateLatestLink() {
    let latest = bit.prefixes.product.join('latest')
    let version = bit.prefixes.product.files('*', {include: /\d+\.\d+\.\d+/}).sort().pop()
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
