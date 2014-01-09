/*
    Support functions for Embedthis products

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix
require ejs.zlib

let TempFilter = /\.old$|\.tmp$|xcuserdata|xcworkspace|project.guid|-mine/

public function deploy(manifest, prefixes, package): Array {
    let sets = bit.options.sets || package.sets
    trace('Copy', 'File sets: ' + sets)
    let home = App.dir
    if (manifest.home) {
        App.chdir(manifest.home)
    }
    if (!(sets is RegExp)) {
        sets = RegExp(sets.toString().replace(/[ ,]/g, '|'))
    }
    let filelist = []
    let made = {}
    for each (item in manifest.files) {
        if (bit.options.verbose) {
            dump("Consider", item)
        }
        let prior = App.dir
        if (item.home) {
            App.chdir(item.home)
        }
        for (let [key,value] in item) {
            if (value is String && value.contains('${')) {
                item[key] = expand(value)
            }
        }
        item.made = made
        item.filelist = filelist
        let name = item.name
        if (item.from && !(item.from is Array)) {
            item.from = [item.from]
        }
        if (item.dir && !(item.dir is Array)) {
            item.dir = [item.dir]
        }
        let name = item.name || serialize(item)
        let enable = true

        for each (r in item.packs) {
            if (!bit.generating) {
                if ((!bit.packs[r] || !bit.packs[r].enable)) {
                    skip(name, 'Required pack ' + r + ' is not enabled')
                    enable = false
                    break
                }
            }
        } 
        if (enable && item.enable) {
            if (!(item.enable is Boolean)) {
                let script = expand(item.enable)
                try {
                    enable = eval(script) cast Boolean
                } catch (e) {
                    vtrace('Enable', 'Cannot run enable script for ' + name)
                    App.log.debug(3, e)
                    skip(name, 'Enable script failed to run')
                    enable = false
                }
            }
        } else if (item.enable === false) {
            enable = false
        }
        if (enable && sets) {
            if (item.set) {
                if (!sets.exec(item.set)) {
                    enable = false
                    skip(name, 'Not in the requested file set: ' + sets)
                }
            }
        }
        if (enable && App.uid != 0 && item.root && bit.installing && !bit.generating) {
            trace('Skip', 'Must be root to copy ' + name)
            skip(name, 'Must be administrator')
            enable = false
        }
        if (enable) {
            if (item.precopy) {
                eval('require ejs.unix\n' + expand(item.precopy))
            }
            if (item.packs && bit.generating) {
                for each (r in item.packs) {
                    if (bit.platform.os == 'windows') {
                        gencmd('!IF "$(BIT_PACK_' + r.toUpper() + ')" == "1"')
                    } else {
                        gencmd('if [ "$(BIT_PACK_' + r.toUpper() + ')" = 1 ]; then true')
                    }
                }
            }
            if (item.dir) {
                for each (let dir:Path in item.dir) {
                    dir = expand(dir)
                    makeDir(dir, item)
                    strace('Create', dir.relativeTo(bit.dir.top))
                }
            }
            if (item.from) {
                copy(item.from, item.to, item)
            }
            if (item.write) {
                item.to = Path(expand(item.to))
                let data = expand(item.write)
                if (bit.generating) {
                    data = data.replace(/\n/g, '\\n')
                    genScript("echo '" + data + "' >" + item.to)
                } else {
                    strace('Create', item.to)
                    item.to.write(data)
                    if (filelist) {
                        filelist.push(item.to)
                    }
                }
            }
            if (item.packs && bit.generating) {
                for each (r in item.packs.length) {
                    if (bit.platform.os == 'windows') {
                        gencmd('!ENDIF')
                    } else {
                        gencmd('fi')
                    }
                }
            }
            if (item.postcopy) {
                eval('require ejs.unix\n' + expand(item.postcopy))
            }
            App.chdir(prior)
        }
    }
    App.chdir(home)
    return filelist
}


function setupGlobals(manifest, package, prefixes) {
    for (pname in prefixes) {
        if (package.prefixes.contains(pname)) {
            bit.globals[pname] = prefixes[pname]
            if (!bit.generating) {
                if (bit.target.name != 'uninstall') {
                    let prefix = Path(prefixes[pname])
                    if (prefix.exists) {
                        if (prefix.toString().contains(bit.settings.product)) {
                            safeRemove(prefix)
                        }
                    }
                    //  TODO - there should be an option in settings to require root
                    if (Config.OS != 'windows' && App.uid != 0 && bit.installing && !bit.generating) {
                        throw 'Must run as root. Use "sudo bit install"'
                    }
                    //  TODO remove generating here 
                    if (bit.generating || !prefixes[pname].exists) {
                        if (prefixes[pname].contains(bit.settings.product)) {
                            makeDir(prefixes[pname])
                        }
                    }
                }
            }
        }
    }
    bit.globals.media = prefixes.media
    bit.globals.staging = prefixes.staging

    if (prefixes.vapp) {
        bit.globals.abin = prefixes.vapp.join('bin')
        bit.globals.adoc = prefixes.vapp.join('doc')
        bit.globals.ainc = prefixes.vapp.join('inc')
    }
}


function setupManifest(kind, package, prefixes) {
    let manifest

    if (package.inherit) {
        let inherit = bit[package.inherit]
        manifest = blend(inherit.clone(), bit.manifest.clone(), {combine: true})
        manifest.files = (inherit.files + bit.manifest.files).clone(true)
        package.prefixes = (inherit.packages[kind].prefixes + package.prefixes).unique()
    } else {
        manifest = bit.manifest.clone()
    }
    for each (item in manifest.files) {
        if (item.packs && !(item.packs is Array)) {
            item.packs = [item.packs]
        }
    }
    return manifest
}


/*
    Create prefixes for the kind of package
 */
function setupPackagePrefixes(kind, package) {
    let prefixes = {}
    if (bit.installing) {
        prefixes = bit.prefixes.clone()
        prefixes.staging = bit.prefixes.app
        prefixes.media = prefixes.app
    } else {
        bit.platform.vname = bit.settings.product + '-' + bit.settings.version + '-' + bit.settings.buildNumber
        prefixes.staging = bit.dir.pkg.join(kind)
        prefixes.media = prefixes.staging.join(bit.platform.vname)
        safeRemove(prefixes.staging)
        for (pname in bit.prefixes) {
            if (package.prefixes.contains(pname)) {
                if (pname == 'src') {
                    prefixes[pname] = prefixes.media.portable.normalize
                } else {
                    prefixes[pname] = Path(prefixes.media.join('contents').portable.name + 
                        bit.prefixes[pname].removeDrive().portable).normalize
                }
            }
        }
    }
    if (bit.options.verbose) {
        dump("Prefixes", prefixes)
    }
    return prefixes
}


function setupPackage(kind) {
    if (bit.settings.manifest) {
        b.loadBitFile(bit.dir.src.join(bit.settings.manifest))
        b.runScript(bit.scripts, "loaded")
    }
    let package = bit.manifest.packages[kind]
    if (package && package.platforms) {
        if (!(package.platforms.contains(bit.platform.os) || package.platforms.contains(bit.platform.like))) {
            package = null
        }
    }
    let prefixes, manifest
    if (package) {
        prefixes = setupPackagePrefixes(kind, package)
        manifest = setupManifest(kind, package, prefixes)
        setupGlobals(manifest, package, prefixes)
        if (!bit.installing) {
            bit.dir.rel.makeDir()
        }
    } else {
        vtrace('Info', 'Skip creating ' + kind + ' package')
    }
    return [manifest, package, prefixes]
}


function makeFiles(where, root, files, prefixes) {
    if (!bit.generating && !bit.options.deploy) {
        let flog = where.join('files.log')
        files += [flog]
        files = files.sort().unique().filter(function(f) f.startsWith(root))
        files = files.map(function(f) '/' + f.relativeTo(root))
        flog.write(files.join('\n') + '\n')
    }
}


public function packageBinary() {
    let [manifest, package, prefixes] = setupPackage('binary')
    if (package) {
        trace('Create', bit.settings.title + ' Binary')
        let files = deploy(manifest, prefixes, package)
        makeFiles(prefixes.vapp, prefixes.root, files, prefixes)
        /* Do Tar first as native package will add files */
        makeTarPackage(prefixes)
        makeNativePackage(prefixes)
    }
}


public function packageSource() {
    let [manifest, package, prefixes] = setupPackage('source')
    if (package) {
        trace('Create', bit.settings.title + ' Source')
        deploy(manifest, prefixes, package)
        makeSimplePackage(package, prefixes, 'src')
    }
}


function flatten(options) {
    let flat: Path = bit.dir.flat
    safeRemove(flat)
    let vflat = flat.join(bit.platform.vname)
    vflat.makeDir()
    for each (f in bit.dir.pkg.files('**', {exclude: /\/$/, missing: undefined})) {
        f.copy(vflat.join(f.basename))
    }
}


public function publish() {
    let [manifest, package, prefixes] = setupPackage('pak')
    if (package) {
        trace('Package', bit.settings.title + ' Pak')
        deploy(manifest, prefixes, package)
        publishPackage(package, prefixes, 'pak')
    }
}

public function cache() {
    let [manifest, package, prefixes] = setupPackage('pak')
    if (package) {
        trace('Package', bit.settings.title + ' Pak')
        deploy(manifest, prefixes, package)
        cachePackage(package, prefixes, 'pak')
    }
}

public function packagePak() {
    let [manifest, package, prefixes] = setupPackage('pak')
    if (package) {
        trace('Package', bit.settings.title + ' Pak')
        deploy(manifest, prefixes, package)
        makeSimplePackage(package, prefixes, 'pak')
    }
}


public function packageCombo() {
    let [manifest, package, prefixes] = setupPackage('combo')
    if (package) {
        trace('Package', bit.settings.title + ' Combo')
        deploy(manifest, prefixes, package)
        makeSimplePackage(package, prefixes, 'combo')
    }
}


public function packageFlat() {
    let [manifest, package, prefixes] = setupPackage('flat')
    if (package) {
        trace('Package', bit.settings.title + ' Flat')
        deploy(manifest, prefixes, package)
        flatten()
        makeSimplePackage(package, prefixes, 'flat')
    }
}


function checkRoot() {
    if (Config.OS != 'windows' && App.uid != 0 && bit.prefixes.root.same('/') && !bit.generating) {
        throw 'Must run as root. Use "sudo bit install"'
    }
}


public function installBinary() {
    bit.installing = true
    let [manifest, package, prefixes] = setupPackage('install')
    if (package) {
        checkRoot()
        if (!bit.generating) {
            if (bit.options.deploy) {
                trace('Deploy', bit.settings.title + ' to "' + bit.prefixes.root + '"')
            } else {
                trace('Install', bit.settings.title)
            }
        }
        files = deploy(manifest, bit.prefixes, package) 
        makeFiles(prefixes.vapp, prefixes.root, files, bit.prefixes)
        if (!bit.generating) {
            trace('Complete', bit.settings.title + ' installed')
        }
    }
    delete bit.installing
}


public function uninstallBinary() {
    let [manifest, package, prefixes] = setupPackage('binary', true)
    let name = (bit.platform.os == 'windows') ? bit.settings.title : bit.settings.product
    if (package) {
        checkRoot()
        if (!bit.generating) {
            trace('Uninstall', bit.settings.title)
        }
        let fileslog = bit.prefixes.vapp.join('files.log')

        if (bit.generating) {
            for each (n in ['web', 'spool', 'cache', 'log']) {
                if (package.prefixes.contains(n)) {
                    removeDir(bit.prefixes[n])
                }
            }
            removeDir(bit.prefixes.vapp)
        } else {
            if (fileslog.exists) {
                for each (let file: Path in fileslog.readLines()) {
                    if (!file.isDir) {
                        removeFile(file)
                    }
                }
                fileslog.remove()
            }
            if (bit.prefixes.log) {
                for each (file in bit.prefixes.log.files('*.log*')) {
                    removeFile(file)
                }
            }
        }
        for (let [key, prefix] in bit.prefixes) {
            /* 
                Safety, make sure product name is in prefix 
             */
            if (!prefix.name.contains(name) || key == 'src' || key == 'app' || !prefixes[key]) {
                continue
            }
            if (!package.prefixes.contains(key)) {
                continue
            }
            if (bit.generating) {
                if (key == 'vapp') {
                    continue
                }
            } else {
                for each (dir in prefix.files('**', {include: /\/$/}).sort().reverse()) {
                    removeDir(dir, {empty: true})
                }
            }
            removeDir(prefix, {empty: true})
        }
        updateLatestLink()
        removeDir(bit.prefixes.app, {empty: true})
        if (!bit.generating) {
            trace('Complete', bit.settings.title + ' uninstalled')
        }
    }
}


/* Only used for uninstalling */
function updateLatestLink() {
    let latest = bit.prefixes.app.join('latest')
    let version
    if (!bit.generating) {
        version = bit.prefixes.app.files('*', {include: /\d+\.\d+\.\d+/}).sort().pop()
    }
    if (version) {
        version.basename.link(latest)
    } else {
        removeFile(latest)
    }
}


function makeSimplePackage(package, prefixes, fmt) {
    if (fmt == 'pak') {
        let base = prefixes.staging.join(bit.platform.vname)
        let package = base.join('package.json')
        package.copy(base.join('bower.json'))
    }
    let name = bit.dir.rel.join(bit.platform.vname + '-' + fmt + '.tar')
    let zname = name.replaceExt('tgz')
    let options = {relativeTo: prefixes.staging, user: 'root', group: 'root', uid: 0, gid: 0}
    let tar = new Tar(name, options)
    tar.create(prefixes.staging.files('**', {exclude: /\/$/, missing: undefined}))
    Zlib.compress(tar.name, zname)
    if (!bit.options.keep) {
        name.remove()
    }
    trace('Package', zname)

    let generic = bit.dir.rel.join(bit.settings.product + '-' + fmt + '.tgz')
    generic.remove()
    zname.link(generic)
    bit.dir.rel.join('md5-' + bit.platform.vname + '-' + fmt + '.tgz.txt').write(md5(zname.readString()))
    trace('Package', generic)
}


function publishPackage(package, prefixes, fmt) {
    let staging = prefixes.staging.absolute
    let base = staging.join(bit.platform.vname)
    let name = bit.settings.product
    let package = base.join('package.json')
    let version = package.readJSON().version
    let home = App.dir

    try {
        trace('Publish', bit.settings.title + ' ' + version)
        package.copy(base.join('bower.json'))
        App.chdir(staging)
        run('git clone git@github.com:embedthis/pak-' + name + '.git', {noshow: true})
        staging.join('pak-' + name, '.git').rename(base.join('.git'))

        App.chdir(base)
        run('git add *')
        run('git commit -q -mPublish-' + version + ' -a', {noshow: true, continueOnErrors: true})
        run('git tag -d v' + version, {noshow: true, continueOnErrors: true })
        run('git push -q origin :refs/tags/v' + version, {noshow: true, continueOnErrors: true})
        run('git tag v' + version)
        run('git push --tags -u origin master', {noshow: true})
        
    } finally {
        App.chdir(home)
    }
}


function cachePackage(package, prefixes, fmt) {
    let staging = prefixes.staging.absolute
    let base = staging.join(bit.platform.vname)
    let name = bit.settings.product
    let package = base.join('package.json')
    let version = package.readJSON().version
    let home = App.dir
    try {
        trace('Cache', bit.settings.title + ' ' + version)
        package.copy(base.join('bower.json'))
        App.chdir(staging)
        run('pak -f cache ' + bit.platform.vname)
    } finally {
        App.chdir(home)
    }
}


public function packageName() {
    let s = bit.settings
    let p = bit.platform
    if (Config.OS == 'macosx') {
        name = s.product + '-' + s.version + '-' + s.buildNumber + '-' + p.dist + '-' + p.os + '-' + p.arch + '.pkg'
    } else if (Config.OS == 'windows') {
        name = s.product + '-' + s.version + '-' + s.buildNumber + '-' + p.dist + '-' + p.os + '-' + p.arch + '.exe.zip'
    } else {
        return null
    }
    return bit.dir.rel.join(name)

}


public function installPackage() {
    let s = bit.settings
    let package = packageName()
    if (Config.OS == 'macosx') {
        checkRoot()
        trace('Install', package.basename)
        run('installer -target / -package ' + package, {noshow: true})

    } else if (Config.OS == 'windows') {
        trace('Install', package.basename)
        package.trimExt().remove()
        run([bit.packs.zip.path.replace(/zip/, 'unzip'), '-q', package], {dir: bit.dir.rel})
        run([package.trimExt(), '/verysilent'], {noshow: true})
        package.trimExt().remove()
    }
}


public function uninstallPackage() {
    if (Config.OS == 'macosx') {
        checkRoot()
        if (bit.prefixes.vapp.join('bin/uninstall').exists) {
            trace('Uninstall', bit.prefixes.vapp.join('bin/uninstall'))
            run([bit.prefixes.vapp.join('bin/uninstall')], {noshow: true})
        }
    } else {
        let uninstall = bit.prefixes.vapp.files('unins*.exe')[0]
        if (uninstall) {
            trace('Uninstall', uninstall)
            run([uninstall, '/verysilent'], {noshow: true})
        }
    }
}


public function whatInstalled() {
    for each (prefix in bit.prefixes) {
        if (prefix.exists) {
            trace('Exists', prefix)
            let files = prefix.files('**')
            if (files.length > 0) {
                vtrace('Exists', files.join(', '))
            }
        }
    }
}


function makeTarPackage(prefixes) {
    let base = [bit.settings.product, bit.settings.version, bit.settings.buildNumber, 
        bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')
    let name = bit.dir.rel.join(base).joinExt('tar', true)
    let zname = name.replaceExt('tgz')
    let files = prefixes.staging.files('**', {exclude: /\/$/, missing: undefined})

    let options = {relativeTo: prefixes.staging, user: 'root', group: 'root', uid: 0, gid: 0}
    let tar = new Tar(name, options)

    trace('Package', zname)
    tar.create(files)

    Zlib.compress(name, zname)
    name.remove()
    bit.dir.rel.join('md5-' + base).joinExt('tgz.txt', true).write(md5(zname.readString()))
    let generic = bit.dir.rel.join(bit.settings.product + '-tar' + '.tgz')
    generic.remove()
    zname.link(generic)
}


function makeNativePackage(prefixes) {
    let os = (bit.cross) ? bit.platform.dev : bit.platform.os
    switch (bit.platform.os) {
    case 'linux':
        if (bit.platform.dist == 'ubuntu') {
            packageUbuntu(prefixes)
        } else if (bit.platform.dist == 'fedora') {
            packageFedora(prefixes)
        } else {
            trace('Info', 'Can\'t package for ' + bit.platform.dist + ' linux distribution')
        }
        break
    case 'macosx':
        packageMacosx(prefixes)
        break
    case 'windows':
        packageWindows(prefixes)
        break
    default:
        trace('Info', 'Cannot package for ' + bit.platform.os)
    }
}


var staffDir = {
    'var/www': true,
}


function createMacContents(prefixes) {
    let staging = prefixes.staging
    let s = bit.settings
    let cp: File = staging.join(s.product + '.pmdoc', '01contents-contents.xml').open('w')
    cp.write('<pkg-contents spec="1.12">')
    cp.write('<f n="contents" o="root" g="wheel" p="16877" pt="' + prefixes.root + '" m="false" t="file">')
    for each (dir in prefixes.root.files('*', {include: /\/$/, missing: undefined})) {
        inner(staging, cp, dir)
    }

    function inner(prefixes, cp: File, dir: Path) {
        let perms = dir.attributes.permissions cast Number
        cp.write('<f n="' + dir.basename + '" o="root" g="wheel" p="' + perms + '" />')
        for each (f in dir.files()) {
            if (f.isDir) {
                inner(staging, cp, f)
            } else {
                perms = f.attributes.permissions cast Number
                cp.write('<f n="' + f.basename + '" o="root" g="wheel" p="' + perms + '" />')
            }
        }
        cp.write('</f>')
    }
    cp.write('</pkg-contents>\n')
    cp.close()
}


function packageMacosx(prefixes) {
    if (!bit.packs.pmaker || !bit.packs.pmaker.path) {
        throw 'Configured without pmaker: PackageMaker'
    }
    let staging = prefixes.staging
    let s = bit.settings
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')
    let name = bit.dir.rel.join(base).joinExt('tar', true)
    let files = staging.files('**', {exclude: /\/$/, missing: undefined})
    let size = 20
    for each (file in staging.files('**', {exclude: /\/$/, missing: undefined})) {
        size += ((file.size + 999) / 1000)
    }
    bit.PACKAGE_SIZE = size
    let pm = s.product + '.pmdoc'
    let pmdoc = staging.join(pm)
    let opak = bit.dir.src.join('package/' + bit.platform.os)
    copy(opak.join('background.png'), staging)
    copy(opak.join('license.rtf'), staging)
    copy(opak.join('readme.rtf'), staging)
    copy(opak.join(pm + '/*'), pmdoc, {expand: true, hidden: true})
    let scripts = staging.join('scripts')
    scripts.makeDir()
    copy(bit.dir.src.join('package/' + bit.platform.os + '/scripts/*'), scripts, {expand: true})
    createMacContents(prefixes)

    /* Remove extended attributes */
    Cmd.sh("cd " + staging + "; for i in $(ls -Rl@ | grep '^    ' | awk '{print $1}' | sort -u); do \
        find . | xargs xattr -d $i 2>/dev/null ; done")

    let outfile = bit.dir.rel.join(base).joinExt('pkg', true)
    trace('Package', outfile)
    run(bit.packs.pmaker.path + ' --target 10.5 --domain system --doc ' + pmdoc + 
        ' --id com.embedthis.' + s.product + '.bin.pkg --root-volume-only --no-relocate' +
        ' --discard-forks --out ' + outfile)
    bit.dir.rel.join('md5-' + base).joinExt('pkg.txt', true).write(md5(outfile.readString()))
}


function packageFedora(prefixes) {
    if (!bit.packs.pmaker || !bit.packs.pmaker.path) {
        throw 'Configured without pmaker: rpmbuild'
    }
    let home = App.getenv('HOME')
    App.putenv('HOME', bit.dir.out)

    let staging = prefixes.staging
    let s = bit.settings
    let cpu = bit.platform.arch
    if (cpu.match(/^i.86$|x86/)) {
        cpu = 'i386'
    } else if (cpu == 'x64') {
        cpu = 'x86_64'
    }
    bit.platform.mappedCpu = cpu
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')

    let RPM = prefixes.media.join('RPM')
    for each (d in ['SOURCES', 'SPECS', 'BUILD', 'RPMS', 'SRPMS']) {
        RPM.join(d).makeDir()
    }
    RPM.join('RPMS', bit.platform.arch).makeDir()
    bit.prefixes.rpm = RPM
    bit.prefixes.content = prefixes.root

    let opak = bit.dir.src.join('package/' + bit.platform.os)
    let spec = RPM.join('SPECS', base).joinExt('spec', true)
    copy(opak.join('rpm.spec'), spec, {expand: true, permissions: 0644})

    let files = prefixes.root.files('**')
    let fileList = RPM.join('BUILD/binFiles.txt')
    let cp: File = fileList.open('atw')
    cp.write('%defattr(-,root,root)\n')

    let owndirs = RegExp(bit.settings.product)
    /* Exclude everything under latest */
    for each (file in prefixes.root.files('**/', {relative: true, include: owndirs, exclude: /\/latest/})) {
        cp.write('%dir /' + file + '\n')
    }
    /* Exclude directories and everything under latest, but include latest itself */
    for each (file in prefixes.root.files('**', {exclude: /\/$|\/latest\//})) {
        cp.write('"/' + file.relativeTo(prefixes.root) + '"\n')
    }
    for each (file in prefixes.root.files('**/.*', {hidden: true})) {
        file.remove()
    }
    cp.close()

    let macros = bit.dir.out.join('.rpmmacros')
    macros.write('%_topdir ' + RPM + '

%__os_install_post /usr/lib/rpm/brp-compress %{!?__debug_package:/usr/lib/rpm/brp-strip %{__strip}} /usr/lib/rpm/brp-strip-static-archive %{__strip} /usr/lib/rpm/brp-strip-comment-note %{__strip} %{__objdump} %{nil}')
    let outfile = bit.dir.rel.join(base).joinExt('rpm', true)
    trace('Package', outfile)
    run(bit.packs.pmaker.path + ' -ba --target ' + cpu + ' ' + spec.basename, {dir: RPM.join('SPECS'), noshow: true})
    let rpmfile = RPM.join('RPMS', cpu, [s.product, s.version, s.buildNumber].join('-')).joinExt(cpu + '.rpm', true)
    rpmfile.rename(outfile)
    bit.dir.rel.join('md5-' + base).joinExt('rpm.txt', true).write(md5(outfile.readString()))
    App.putenv('HOME', home)
}


function packageUbuntu(prefixes) {
    if (!bit.packs.pmaker || !bit.packs.pmaker.path) {
        throw 'Configured without pmaker: dpkg'
    }
    let cpu = bit.platform.arch
    if (cpu.match(/^i.86$|x86/)) {
        cpu = 'i386'
    } else if (cpu == 'x64') {
        cpu = 'amd64'
    }
    bit.platform.mappedCpu = cpu
    let s = bit.settings
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')

    let DEBIAN = prefixes.root.join('DEBIAN')
    let opak = bit.dir.src.join('package/' + bit.platform.os)

    copy(opak.join('deb.bin/conffiles'), DEBIAN.join('conffiles'), {expand: true, permissions: 0644})
    copy(opak.join('deb.bin/control'), DEBIAN, {expand: true, permissions: 0755})
    copy(opak.join('deb.bin/p*'), DEBIAN, {expand: true, permissions: 0755})

    let outfile = bit.dir.rel.join(base).joinExt('deb', true)
    trace('Package', outfile)
    run(bit.packs.pmaker.path + ' --build ' + DEBIAN.dirname + ' ' + outfile, {noshow: true})
    bit.dir.rel.join('md5-' + base).joinExt('deb.txt', true).write(md5(outfile.readString()))
}


function packageWindows(prefixes) {
    if (!bit.packs.pmaker || !bit.packs.pmaker.path) {
        throw 'Configured without pmaker: Inno Setup'
    }
    let s = bit.settings
    let wpak = bit.dir.src.join('package/' + bit.platform.os)
    let media = prefixes.media

    copy(bit.dir.src.join('LICENSE.md'), media)
    let iss = media.join('install.iss')
    copy(wpak.join('install.iss'), iss, {expand: true})

    let files = prefixes.root.files('**', {exclude: /\/$/, missing: undefined})

    let appPrefix = bit.prefixes.app.removeDrive().portable
    let top = Path(prefixes.root.name + appPrefix)
    let cp: File = iss.open('atw')
    for each (file in files) {
        let src = file.relativeTo(media)
        let dest = file.relativeTo(top).windows
        cp.write('Source: "' + src + '"; DestDir: "{app}\\' + dest.dirname + '"; ' +
            'DestName: "' + dest.basename + '";\n')
    }
    cp.close()

    let data = iss.readString()
    if (bit.platform.arch == 'x64') {
        sub = '{pf64}'
    } else {
        sub = '{pf32}'
    }
    data = data.replace(/{pf}/g, sub)
    iss.write(data)

    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')
    let outfile = bit.dir.rel.join(base).joinExt('exe', true)
    run([bit.packs.pmaker.path, iss], {noshow: true})
    media.join('Output/setup.exe').copy(outfile)

    /* Wrap in a zip archive */
    let zipfile = outfile.joinExt('zip', true)
    zipfile.remove()
    trace('Package', zipfile)
    run([bit.packs.zip.path, '-q', zipfile.basename, outfile.basename], {dir: bit.dir.rel})
    bit.dir.rel.join('md5-' + base).joinExt('exe.zip.txt', true).write(md5(zipfile.readString()))
    outfile.remove()
}


public function syncup(from: Path, to: Path) {
    let tartemp: Path
    if (from.name.endsWith('.tgz') || from.name.endsWith('.gz')) {
        if (!from.exists) {
            throw 'Cannot find package: ' + from
        }
        tartemp = Path().temp()
        Zlib.uncompress(from, tartemp)
        from = tartemp
    }
    let tar = new Tar(from)
    for each (item in tar.list()) {
        let path = to.join(item.components.slice(1).join('/'))
        let fromData = tar.readString(item)
        let toData = path.exists ? path.readString() : undefined
        if (fromData != toData) {
            let modified = tar.info(item)[0].modified
            if (path.exists && modified <= path.modified) {
                if (!bit.options.force) {
                    trace('WARNING', path.relative + ' has been modified. Update skipped for this file.')
                    continue
                }
                trace('Update', 'Force update of ' + path)
            } else {
                trace('Update', path)
            }
            path.write(fromData)
        }
    }
    if (tartemp) {
        tartemp.remove()
    }
}


public function apidoc(dox: Path, headers, title: String, tags) {
    let name = dox.basename.trimExt().name
    let api = bit.dir.src.join('doc/api')
    let output
    if (headers is Array) {
        output = api.join(name + '.h')
        copy(headers, output, { cat: true, })
        headers = output
    }
    rmdir([api.join('html'), api.join('xml')])
    tags = Path('.').files(tags)

    let doxtmp = Path('').temp().replaceExt('dox')
    let data = api.join(name + '.dox').readString().replace(/^INPUT .*=.*$/m, 'INPUT = ' + headers)
    Path(doxtmp).write(data)
    trace('Generate', name.toPascal() + ' documentation')
    run([bit.packs.doxygen.path, doxtmp], {dir: api})
    if (output) {
        output.remove()
    }
    if (!bit.options.keep) {
        doxtmp.remove()
    }
    trace('Process', name.toPascal() + ' documentation (may take a while)')
    let files = [api.join('xml/' + name + '_8h.xml')]
    files += ls(api.join('xml/group*')) + ls(api.join('xml/struct_*.xml'))
    let tstr = tags ? tags.map(function(i) '--tags ' + Path(i).absolute).join(' ') : ''

    run('ejs ' + bit.dir.bits.join('gendoc.es') + ' --bare ' + '--title \"' + 
        bit.settings.product.toUpper() + ' - ' + title + ' Native API\" --out ' + name + 
        'Bare.html ' +  tstr + ' ' + files.join(' '), {dir: api})
    if (!bit.options.keep) {
        rmdir([api.join('html'), api.join('xml')])
    }
}


public function apiwrap(patterns) {
    for each (dfile in Path('.').files(patterns)) {
        let name = dfile.name.replace('.html', '')
        let data = Path(name + 'Bare.html').readString()
        let contents = Path(name + 'Header.tem').readString() + data + Path(name).dirname.join('apiFooter.tem').readString() + '\n'
        dfile.joinExt('html').write(contents)
    }
}


public function checkInstalled() {
    let result = []
    for each (key in ['app', 'vapp', 'bin']) {
        let prefix = bit.prefixes[key]
        if (!prefix.exists) {
            result.push(prefix)
        }
    }
    return result.length > 0 ? result.unique() : null
}


public function checkUninstalled() {
    let result = []
    for each (prefix in bit.prefixes) {
        if (!prefix.name.contains(bit.settings.product)) {
            continue
        }
        if (prefix.exists) {
            result.push(prefix)
        }
    }
    return result.length > 0 ? result.unique() : null
}


public function genProjects(packs = '', profiles = ["default", "static"], platforms = null) 
{
    if (platforms is String) {
        platforms = [platforms]
    }
    if (profiles is String) {
        profiles = [profiles]
    }
    platforms ||= ['freebsd-x86', 'linux-x86', 'macosx-x64', 'vxworks-x86', 'windows-x86']
    let bitcmd = Cmd.locate('bit').relative
    let runopt = {dir: bit.dir.src, show: true}
    if (packs) {
        packs +=  ' '
    }
    let home = App.dir
    try {
        App.chdir(bit.dir.top)
        let src = bit.dir.src.relative
        for each (name in platforms) {
            trace('Generate', bit.settings.product + '-' + name.replace(/-.*/, '') + ' projects')
            for each (profile in profiles) {
                let formats = (name == 'windows-x86') ? '-gen nmake' : '-gen make'
                let platform = name + '-' + profile
                let options = (profile == 'static') ? ' -static' : ''
                run(bitcmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                    ' ' + packs + formats, runopt)
                /* Xcode and VS use separate profiles */
                if (name == 'macosx-x64') {
                    run(bitcmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + packs + '-gen xcode', runopt)
                } else if (name == 'windows-x86') {
                    run(bitcmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + packs + '-gen vs', runopt)
                }
                rm(bit.dir.top.join(platform + '.bit'))
                rmdir(bit.dir.top.join(platform))
            }
        }
    }
    finally {
        App.chdir(home)
    }
}


public function getWebUser(): String {
    if (bit.platform.os == 'macosx') {
        return '_www'
    } else if (bit.platform.os == 'windows') {
        return 'Administrator'
    } else if (bit.platform.os == 'linux' || bit.platform.os == 'freebsd') {
        return 'nobody'
    }
    return '0'
}


public function getWebGroup(): String {
    if (bit.platform.os == 'macosx') {
        return '_www'
    } else if (bit.platform.os == 'windows') {
        return 'Administrator'
    } else if (bit.platform.os == 'linux' || bit.platform.os == 'freebsd') {
        let groups = Path('/etc/group').readString()
        if (groups.contains('nogroup:')) {
            return 'nogroup'
        }
        return 'nobody'
    }
    return '0'
}


function skip(name, msg) {
    if (bit.options.why) {
        trace('Skip', 'Manifest item "' + name + '", ' + msg)
    }
}


function fixlibs() {
    throw new Error("fixlibs not supported")
    let ldconfigSwitch = (bit.platform.os == 'freebsd') ? '-m' : '-n'
    let ldconfig = Cmd.locate('ldconfig')
    if (ldconfig) {
        Cmd.run(ldconfig + ' ' + ldconfigSwitch + ' /usr/local/lib/' + bit.settings.product + '/modules')
    }
    if (bit.platform.dist == 'fedora') {
        Cmd.run('chcon /usr/bin/chcon -t texrel_shlib_t ' + bit.prefixes.vapp.join('bin').files('*.so').join(' '))
    }
}

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
