/*
    Support functions for Embedthis products

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix
require ejs.zlib

let TempFilter = /\.old$|\.tmp$|xcuserdata|xcworkspace|project.guid|-mine/

//  MOB - update doc
/**
    Install and uninstall files.
    @param src Source path. May contain glob style wild cards including '*', '?' and '**'. May also be an array
        of source paths.
    @param dest Destination path
    @param options Process options
    @option active If destination is an active executable or shared library, rename the active file using a
        '.old' extension and retry the copy.
    @option compress Compress target file
    @option copytemp Copy files that look like temp files
    @option exclude Exclude files that match the pattern. The pattern should be in portable file format.
    @option expand Expand tokens. Set to true or an Object hash containing properties to use when replacing 
        tokens of the form ${token} in the src and dest filenames. If set to true, the 'bit' object is used.
    @option fold Fold long lines on windows at column 80 and convert new line endings.
    @option group Set file group
    @option include Include files that match the pattern. The pattern should be in portable file format.
    @option linkin Add a symbolic link to the destination in this directory
    @option permissions Set file permissions
    @option strip Strip object or executable
    @options tree Copy the entire subtree identified by the patterns by prepending the entire pattern path.
    @option user Set file file user
 */
public function install(src, dest: Path, options = {}) {
    if (!(src is Array)) src = [src]
    if (options.cat) {
        let files = []
        for each (pat in src) {
            files += Path('.').files(pat, {missing: undefined})
        }
        src = files.unique()
    }
    let path = Path('.')
    if (Path(src).isDir) {
        path = Path(src)
        src = '**'
        if (dest.isDir && !options.subtree) {
            dest = dest.join(path.basename)
        }
        options = blend({tree: true, relative: true}, options, {functions: true})
    }
/* UNUSED
    if (options.expand) {
         dest = dest.toString().expand(options.expand, options)
    }
*/
    list = path.files(src, options)
    if (bit.options.verbose) {
        print("Install", src, dest)
        dump('Files', list)
    }
    if (!list || list.length == 0) {
        if (bit.generating) {
            list = src
        } else {
            throw 'cp: Cannot find files to copy for "' + src + '" to ' + dest
        }
    }
    let destIsDir = (dest.isDir || (!options.cat && list.length > 1) || dest.name.endsWith('/'))
    for each (let file: Path in list) {
        let from = path.join(file)
        if (options.expand) {
             file = file.toString().expand(options.expand, options)
        }
        let target
        if (options.tree) {
            target = Path(dest + "/" + file).normalize
        } else if (destIsDir) {
            target = dest.join(file.basename)
        } else {
            target = dest
        }
/* UNUSED
        if (bit.generating) {
        } else if (!target.dirname.makeDir()) {
            throw 'Cannot make directory "' + dest + '"'
        }
*/
        from = from.relative.portable
        if (options.exclude && from.match(options.exclude)) {
            continue
        }
        if (!options.copytemp && from.match(TempFilter)) {
            continue
        }
        if (options.include && !from.match(options.include)) {
            continue
        }
        attributes = options.clone()
        if (!bit.generating) {
            attributes.permissions ||= from.attributes.permissions
        }
        if (bit.generating) {
            if (options.cat || options.expand || options.fold || options.compress) {
                dump("OPTIONS", options)
                throw "Cannot use processing options when generating"
            }
            makeDir(target.parent, options)
            if (from.isDir) {
                makeDir(target, options)
            } else {
                copy(from, target, options)
            }
            if (options.linkin) {
                link(target, Path(expand(options.linkin)).join(target.basename), options)
            }
            continue
        }
        if (options.cat) {
            catenate(from, target, attributes)
        } else {
            if (from.isDir) {
                makeDir(target, options)
            } else {
                try {
                    copy(from, target, attributes)
                } catch {
                    if (options.active) {
                        let active = target.replaceExt('old')
                        active.remove()
                        target.rename(active)
                    }
                    copy(from, target, attributes)
                }
            }
        }
        //  if (target.extension == bit.ext.shobj)
        //  MOB - check this
        //  for each (f in abin.files('*.so.*')) {
        //      let nover = target.basename.name.replace(/\.[0-9]*.*/, '.so')
        //      target.remove()
        //      target.symlink(target.dirname.join(nover).basename)
        //      //MOB - not right
        //      options.filelist.push(target)
        //  }
        //
        if (options.expand) {
            strace('Expand', target)
            let o = bit
            if (options.expand != true) {
                o = options.expand
            }
            target.write(target.readString().expand(o, {fill: '${}'}))
            target.setAttributes(attributes)
        }
        if (options.fold) {
            strace('Fold', target)
            foldLines(target)
            target.setAttributes(attributes)
        }
        if (options.strip && bit.packs.strip && bit.platform.profile == 'release') {
            strace('Strip', target)
            Cmd.run(bit.packs.strip.path + ' ' + target)
        }
        if (options.compress) {
            let zname = Path(target.name + '.gz')
            strace('Compress', zname)
            zname.remove()
            Zlib.compress(target.name, zname)
            target.remove()
            target = zname
        }
    /* MOB UNUSED install only
        if (App.uid == 0 && target.extension == 'so' && Config.OS == 'linux') {
            let ldconfig = Cmd.locate('ldconfig')
            if (ldconfig) {
                Cmd.run('ldconfig ' + target)
            }
        }
    */
        if (options.filelist) {
            options.filelist.push(target)
        }
        if (options.linkin) {
            let linkin = Path(expand(options.linkin))
            linkin.makeDir(options)
            let ltarget = linkin.join(from.basename)
            link(target.relativeTo(ltarget.dirname), ltarget, options)
            if (options.filelist) {
                options.filelist.push(ltarget)
            }
        }
    }
}


public function deploy(manifest, prefixes, package): Array {
    let sets = package.sets
    if (!(sets is RegExp)) {
        sets = RegExp(sets.toString().replace(/[ ,]/g, '|'))
    }
    let filelist = []
    let made = {}
    for each (item in manifest.files) {
        if (bit.options.verbose) {
            //  MOB - remove
            dump("Consider", item)
        }
        let name = item.name
        if (item.from && !(item.from is Array)) {
            item.from = [item.from]
        }
        if (item.dir && !(item.dir is Array)) {
            item.dir = [item.dir]
        }
        let name = serialize(item)
        let enable = true
        if (item.enable) {
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
        }
        if (enable && item.root && App.uid != 0) {
            trace('Skip', 'Must be root to copy ' + name)
            skip(name, 'Must be administrator')
            enable = false
        }
        if (enable) {
            if (sets) {
                if (item.set) {
                    if (!sets.exec(item.set)) {
                        enable = false
                        skip(name, 'Not in the requested file set: ' + sets)
                    }
                }
            }
        }
        if (enable) {
            if (item.dir) {
                for each (let dir:Path in item.dir) {
                    dir = expand(dir)
                    dir.makeDir()
                    strace('Create', dir.relativeTo(bit.dir.top))
                }
            }
            for each (from in item.from) {
                item.filelist = filelist
                item.made = made
                from = Path(expand(from))
                item.to = Path(expand(item.to))
                if (item.precopy) {
                    runScript(item.precopy)
                }
                install(from, item.to, item)
            }
            if (item.postcopy) {
                runScript(item.postcopy)
            }
        }
    }
    return filelist
}


function setupGlobals(manifest, package, prefixes) {
    for (pname in prefixes) {
        if (package.prefixes.contains(pname)) {
            bit.globals[pname] = prefixes[pname]
            if (!bit.generating || bit.target.name != 'uninstall') {
                makeDir(prefixes[pname])
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
        manifest = blend(inherit.clone(), bit.manifest, {combine: true})
        manifest.files = inherit.files + bit.manifest.files
        package.prefixes = (inherit.packages[kind].prefixes + package.prefixes).unique()
    } else {
        manifest = bit.manifest.clone()
    }
    return manifest
}


/*
    Create prefixes for the kind of package
 */
function setupPackagePrefixes(kind, package, doinstall) {
    let prefixes = {}
    if (doinstall) {
        prefixes = bit.prefixes.clone()
        /* MOB just for safety */
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


function setupPackage(kind, doinstall = false) {
    let package = bit.manifest.packages[kind]
    if (package && package.platforms) {
        if (!(package.platforms.contains(bit.platform.os) || package.platforms.contains(bit.platform.like))) {
            package = null
        }
    }
    let prefixes, manifest
    if (package) {
        prefixes = setupPackagePrefixes(kind, package, doinstall)
        manifest = setupManifest(kind, package, prefixes)
        setupGlobals(manifest, package, prefixes)
        if (!doinstall) {
            bit.dir.rel.makeDir()
        }
    } else {
        trace('Info', 'Skip creating ' + kind + ' package')
    }
    return [manifest, package, prefixes]
}


function makeFiles(where, root, files, prefixes) {
    if (!bit.generating) {
        files = files.sort().unique().filter(function(f) f.startsWith(root))
        files = files.map(function(f) '/' + f.relativeTo(root))
        where.join('files.log').write(files.join('\n') + '\n')
    }
}


public function packageBinary() {
    let [manifest, package, prefixes] = setupPackage('binary')
    if (package) {
        trace('Package', bit.settings.title + ' Binary')
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
        trace('Package', bit.settings.title + ' Source')
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
    let [manifest, package, prefixes] = setupPackage('install', true)
    if (package) {
        checkRoot()
        trace('Install', bit.settings.title)
        files = deploy(manifest, bit.prefixes, package) 
        makeFiles(prefixes.vapp, prefixes.root, files, bit.prefixes)
        if (!bit.cross && Config.OS != 'windows') {
            link(bit.settings.version, bit.prefixes.app.join('latest'))
        }
        trace('Complete', bit.settings.title + ' installed')
    }
}


public function uninstallBinary() {
    let [manifest, package, prefixes] = setupPackage('binary', true)
    if (package) {
        checkRoot()
        trace('Uninstall', bit.settings.title)
        let fileslog = bit.prefixes.vapp.join('files.log')
        if (fileslog.exists) {
            for each (let file: Path in fileslog.readLines()) {
                remove(file)
            }
        }
        fileslog.remove()
        if (prefixes.log) {
            for each (file in prefixes.log.files('*.log*')) {
                remove(file)
            }
        }
        let name = (bit.platform.os == 'windows') ? bit.settings.title : bit.settings.product
        for (let [key, prefix] in bit.prefixes) {
            /* Safety, make sure product name is in prefix */
            if (!prefix.name.contains(name) || key == 'src') {
                continue
            }
            if (!package.prefixes.contains(key)) {
                continue
            }
            if (!bit.generating) {
                for each (dir in prefix.files('**', {include: /\/$/}).sort().reverse()) {
                    removeDir(dir)
                }
            }
            strace('Remove', prefix)
            removeDir(prefix)
        }
        updateLatestLink()
        trace('Complete', bit.settings.title + ' uninstalled')
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
        link(version.basename, latest)
    } else {
        latest.remove()
    }
}


function makeSimplePackage(package, prefixes, fmt) {
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
    //  MOB zname.link(generic)
    Path(generic).symlink(zname)
    bit.dir.rel.join('md5-' + bit.platform.vname + '-' + fmt + '.tgz.txt').write(md5(zname.readString()))
    trace('Package', generic)
}


public function packageName() {
    let s = bit.settings
    let p = bit.platform
    if (Config.OS == 'macosx') {
        name = s.product + '-' + s.version + '-' + s.buildNumber + '-' + p.dist + '-' + p.os + '-' + p.arch + '.pkg'
    } else if (Config.OS == 'windows') {
        name = s.product + '-' + s.version + '-' + s.buildNumber + '-' + p.dist + '-' + p.os + '-x86.exe.zip'
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
    let generic = bit.dir.rel.join(s.product + '-tar' + '.tgz')
    generic.remove()
    //MOB   zname.link(generic)
    Path(generic).symlink(zname)
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
/* 
    options.staff = staffDir
*/
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
    let opak = Path('package/' + bit.platform.os)
    install(opak.join('background.png'), staging)
    install(opak.join('license.rtf'), staging)
    install(opak.join('readme.rtf'), staging)
    install(opak.join(pm + '/*'), pmdoc, {expand: true, hidden: true})
    let scripts = staging.join('scripts')
    scripts.makeDir()
    install('package/' + bit.platform.os + '/scripts/*', scripts, {expand: true})
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

    //  MOB change contents to root
    let contents = staging.join(bit.platform.vname, 'contents')
    let RPM = prefixes.media.join('RPM')
    for each (d in ['SOURCES', 'SPECS', 'BUILD', 'RPMS', 'SRPMS']) {
        RPM.join(d).makeDir()
    }
    RPM.join('RPMS', bit.platform.arch).makeDir()

//  MOB - should be put into prefixes
    bit.dir.rpm = RPM
    bit.dir.contents = contents

    let opak = Path('package/' + bit.platform.os)
    let spec = RPM.join('SPECS', base).joinExt('spec', true)
    install(opak.join('rpm.spec'), spec, {expand: true, permissions: 0644})

    let files = contents.files('**')
    let fileList = RPM.join('BUILD/binFiles.txt')
    let cp: File = fileList.open('atw')
    cp.write('%defattr(-,root,root)\n')

    let owndirs = RegExp(bit.settings.product)
    for each (file in contents.files('**/', {relative: true, include: owndirs})) {
        cp.write('%dir /' + file + '\n')
    }
    for each (file in contents.files('**', {exclude: /\/$/})) {
        cp.write('"/' + file.relativeTo(contents) + '"\n')
    }
    for each (file in contents.files('**/.*', {hidden: true})) {
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
    let s = bit.settings
    let cpu = bit.platform.arch
    if (cpu == 'x64') {
        cpu = 'amd64'
    }
    bit.platform.mappedCpu = cpu
    let contents = pkg.join(bit.platform.vname, 'contents')
    let DEBIAN = contents.join('DEBIAN')
    let opak = Path('package/' + bit.platform.os)

    install(opak.join('deb.bin/conffiles'), DEBIAN.join('conffiles'), {expand: true, permissions: 0644})
    install(opak.join('deb.bin/control'), DEBIAN, {expand: true, permissions: 0755})
    install(opak.join('deb.bin/p*'), DEBIAN, {expand: true, permissions: 0755})

    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')
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
    let opak = Path('package/' + bit.platform.os)

    install(bit.dir.top.join('LICENSE.md'), pkg)
    let iss = pkg.join('install.iss')
    install(opak.join('install.iss'), iss, {expand: true})
    let contents = pkg.join(s.product + '-' + s.version + '-' + s.buildNumber, 'contents')
    let files = contents.files('**', {exclude: /\/$/, missing: undefined})

    let appPrefix = bit.prefixes.app.removeDrive().portable
    let top = Path(contents.name + appPrefix)

    let destTop = Path(top.portable.name + appPrefix).windows
    let cp: File = iss.open('atw')
    for each (file in files) {
        let src = file.relativeTo(pkg)
        let dest = file.relativeTo(top).windows
        cp.write('Source: "' + src + '"; DestDir: "{app}\\' + dest.dirname + '"; ' +
            'DestName: "' + dest.basename + '";\n')
    }
    cp.close()
    let base = [s.product, s.version, s.buildNumber, bit.platform.dist, bit.platform.os, bit.platform.arch].join('-')
    let outfile = bit.dir.rel.join(base).joinExt('exe', true)
    run([bit.packs.pmaker.path, iss], {noshow: true})
    pkg.join('Output/setup.exe').copy(outfile)

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
            throw 'Can\'t find package: ' + from
        }
        Zlib.uncompress(from, from.replaceExt('tartemp'))
        from = from.replaceExt('tartemp')
        tartemp = from
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
        install(headers, output, { cat: true, })
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
        let contents = Path(name + 'Header.tem').readString() + data + 
            Path(name).dirname.join('apiFooter.tem').readString() + '\n'
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


public function genProjects(packs = '--without default', profiles = ["default"], platforms = null) 
{
    if (platforms is String) {
        platforms = [platforms]
    }
    if (profiles is String) {
        profiles = [profiles]
    }
    platforms ||= ['freebsd-x86', 'linux-x86', 'macosx-x64', 'vxworks-x86', 'windows-x86']
    let bitcmd = Cmd.locate('bit')

    //  MOB
    let targets = "--nobuild build install uninstall"
    for each (profile in profiles) {
        for each (name in platforms) {
            let formats = (name == 'windows-x86') ? '-gen nmake' : '-gen make'
            trace('Generate', bit.settings.product + '-' + name.replace(/-.*/, '') + ' projects')
            let platform = name + '-' + profile
            run(bitcmd + ' -d -q -platform ' + platform + ' ' + packs + ' -configure . ' + formats, bit.target.runopt)
            /* Xcode and VS use separate profiles */
            if (name == 'macosx-x64') {
                run(bitcmd + ' -d -q -platform ' + platform + ' ' + packs + ' -configure . -gen xcode', bit.target.runopt)
            } else if (name == 'windows-x86') {
                run(bitcmd + ' -d -q -platform ' + platform + ' ' + packs + ' -configure . -gen vs', bit.target.runopt)
            }
        }
    }
    trace('Cleanup', 'Project working directories')
    for each (profile in ['default', 'static']) {
        for each (name in ['freebsd-x86', 'linux-x86', 'macosx-x64', 'windows-x86', 'vxworks-x86']) {
            let platform = name + '-' + profile
            rm(bit.dir.top.join(platform + '.bit'))
            rmdir(bit.dir.top.join(platform))
        }
    }
}


//  MOB - not used
public function getWebUser(): String {
    let passwdFile: Path = Path("/etc/passwd")
    if (passwdFile.exists) {
        let passwords = passwdFile.readString()
        for each (u in ["www-data", "_www", "nobody", "Administrator"]) {
            if (passwords.contains(u + ":")) {
                return u
            }
        }
    }
    return '0'
}


public function getWebGroup(): String {
    let groupFile: Path = Path("/etc/group")
    if (groupFile.exists) {
        let groups = groupFile.readString()
        for each (g in ["www-data", "_www", "nobody", "nogroup", "Administrator"]) {
            if (groups.contains(g + ":")) {
                return g
            }
        }
    }
    return '0'
}


/*
    Fold long lines at column 80. On windows, will also convert line terminatations to <CR><LF>.
 */
function foldLines(path: Path) {
    let lines = path.readLines()
    let out = new TextStream(new File(path, 'wt'))
    for (l = 0; l < lines.length; l++) {
        let line = lines[l]
        if (line.length > 80) {
            for (i = 79; i >= 0; i--) {
                if (line[i] == ' ') {
                    lines[l] = line.slice(0, i)
                    lines.insert(l + 1, line.slice(i + 1))
                    break
                }
            }
            if (i == 0) {
                lines[l] = line.slice(0, 80)
                lines.insert(l + 1, line.slice(80))
            }
        }
        out.writeLine(lines[l])
    }
    out.close()
}


function getatt(attributes) {
    let att = ''
    if (attributes.group) {
        att += '-g ' + attributes.group
    }
    if (attributes.user) {
        att += '-u ' + attributes.user
    }
    if (attributes.permissions) {
        att += '-m ' + attributes.permissions.toString().trimStart('0')
    }
    return (att) ? (att + ' ') : att
}


function link(src: Path, dest: Path, options = {}) {
    if (!bit.generating) {
        strace('Create', 'Link: ' + dest)
        if (!options.dry) {
            dest.remove()
            //  MOB - must reverse
            //  src.link(dest)
            dest.symlink(src)
        }
    } else if (bit.generating != 'nmake') {
        gen('rm -f "' + dest + '"')
        gen('install -d "' + dest.parent + '"')
        gen('ln -s "' + src + '" "' + dest + '"')
    }
}


function makeDir(path: Path, options = {}) {
    if (!bit.generating) {
        strace('Create', 'Directory: ' + path)
        if (!options.dry) {
            if (!path.makeDir(options)) {
                throw "Cannot make directory" + path
            }
        }
    } else {
        if (options.made) {
            if (options.made[path]) {
                return
            }
            options.made[path] = true
        }
        if (bit.generating == 'nmake') {
            gen('if not exist "' + path + '" md "' + path + '"')
        } else {
            gen('install -d ' + getatt(options) + '"' + path + '"')
        }
    }
}


function remove(path: Path, options = {}) {
    if (!bit.generating) {
        strace('Remove', path)
        if (!options.dry) {
            if (!path.remove()) {
                throw "Cannot remove " + path
            }
        }
    } else {
        if (bit.generating == 'nmake') {
            gen('if exist "' + path + '" rd /Q "' + path + '"')
        } else {
            gen('rm -f "' + path + '"')
        }
    }
}


function removeDir(path: Path, options = {}) {
    if (!bit.generating) {
        strace('Remove', path)
        if (!options.dry) {
            if (!path.removeAll()) {
                throw "Cannot remove " + path
            }
        }
    } else {
        if (bit.generating == 'nmake') {
            gen('if exist "' + path + '" rd /Q /S "' + path + '"')
        } else {
            gen('rm -fr "' + path + '"')
        }
    }
}


function copy(src: Path, dest: Path, options = {}) {
    if (!bit.generating) {
        strace('Copy', dest)
        if (!options.dry) {
            src.copy(dest, options)
        }
    } else if (bit.generating == 'nmake') {
        gen('copy "' + src + '" "' + dest + '"')
    } else {
        gen('install ' + getatt(options) + ' "' + src + '" "' + dest + '"')
    }
}


function catenate(from, target, options) {
    strace('Combine', from.relative)
    if (!target.exists) {
        if (options.title) {
            if (options.textfile) {
                target.write('#\n' +
                   '#   ' + target.basename + ' -- ' + options.title + '\n' + 
                   '#\n')
            } else {
                target.write('/*\n' +
                   '    ' + target.basename + ' -- ' + options.title + '\n\n' +
                   '    This file is a catenation of all the source code. Amalgamating into a\n' +
                   '    single file makes embedding simpler and the resulting application faster.\n\n' + 
                   '    Prepared by: ' + System.hostname + '\n */\n\n')
            }
        }
        if (options.header) {
            target.append(options.header + '\n')
        }
    }
    if (options.textfile) {
        target.append('\n' +
           '#\n' +
           '#   Start of file \"' + from.relative + '\"\n' +
           '#\n')
    } else {
        target.append('\n' +
           '/************************************************************************/\n' +
           '/*\n    Start of file \"' + from.relative + '\"\n */\n' +
           '/************************************************************************/\n\n')
    }
    let data = from.readString()
    if (options.filter) {
        data = data.replace(options.filter, '')
    }
    target.append(data)
    target.setAttributes(options)
}


function skip(name, msg) {
    if (bit.options.why) {
        trace('Skip', 'Manifest item "' + name + '", ' + msg)
    }
}


//  MOB - WHO CALLS? should be called after installing?
function fixlibs() {
    let ldconfigSwitch = (bit.platform.os == 'freebsd') ? '-m' : '-n'
    let ldconfig = Cmd.locate('ldconfig')
    if (ldconfig) {
        //  MOB hard coded /usr/lib
        Cmd.run(ldconfig + ' ' + ldconfigSwitch + ' /usr/lib/' + bit.settings.product + '/modules')
    }
    if (bit.platform.dist == 'fedora') {
        Cmd.run('chcon /usr/bin/chcon -t texrel_shlib_t ' + bit.prefixes.vapp.join('bin').files('*.so').join(' '))
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
