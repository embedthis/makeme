/*
    dev.es - me-dev Support functions

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.unix

public function genProjects(extensions = '', profiles = ["default"], platforms = null) {
    if (platforms is String) {
        platforms = [platforms]
    }
    if (profiles is String) {
        profiles = [profiles]
    }
    // platforms ||= ['freebsd-x86', 'linux-x64', 'macosx-arm64', 'vxworks-x86', 'windows-x64']
    platforms ||= ['linux-x64', 'macosx-arm64']

    let cmd = Cmd.locate('me').compact()
    let runopt = {dir: me.dir.top, show: true}
    if (extensions) {
        extensions +=  ' '
    }
    let home = App.dir
    try {
        App.chdir(me.dir.top)
        let src = me.dir.top.relative
        for each (name in platforms) {
            trace('Generate', me.settings.name + '-' + name.replace(/-.*/, '') + ' projects')
            for each (profile in profiles) {
                if (name.startsWith('macosx')) {
                    runopt.env = {ME_COM_OPENSSL_PATH: '/opt/homebrew'}
                }
                let formats = name.startsWith('windows-') ? '-gen nmake' : '-gen make'
                let platform = name + '-' + profile
                let options = (profile == 'static') ? ' -static' : ''
                let pext = extensions
                run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                    ' ' + pext + formats, runopt)
                /* Xcode and VS use separate profiles */
                if (name == 'macosx-x64' || name == 'macosx-arm64') {
                    run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + pext + '-gen xcode', runopt)
                } else if (name.startsWith('windows-')) {
                    run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + pext + '-gen vs', runopt)
                }
                rmdir(me.dir.bld.join(platform))
            }
        }
    }
    finally {
        App.chdir(home)
    }
}

/*
    Commit a repository to a git "pak-NAME" repository
    This is used by repositories to publish "pak" versions of its source code.
 */
public function commitToPakRepo() {
    let name = me.settings.name
    let names = ['appweb', 'ejscript', 'esp', 'http', 'mpr', 'osdep', 'pcre', 'sqlite', 'zlib']
    if (!names.contains(name)) {
        throw 'This product does not have a pak-' + name + ' repository'
    }
    let [manifest, package, prefixes] = setupPackage('pak')
    let staging = prefixes.staging.absolute
    let base = staging.join(me.platform.vname)

    deploy(manifest, prefixes, package)

    let path = base.join('pak.json')
    let version
    if (path.exists) {
        version = path.readJSON().version
    } 
    if (!version) {
        path = base.join('package.json')
        version = path.readJSON().version
    }
    let home = App.dir

    try {
        trace('Publish', me.settings.title + ' ' + version)
        App.chdir(staging)
        run('git clone git@github.com:embedthis/pak-' + name + '.git', {noshow: true})

        /* Steal the .git */
        staging.join('pak-' + name, '.git').rename(base.join('.git'))
        App.chdir(base)
        run('git add -A *')
        run('git commit -q -mPublish-' + version + ' -a', {noshow: true, nostop: true})
        run('git tag -d v' + version, {noshow: true, nostop: true })
        run('git push -q origin :refs/tags/v' + version, {noshow: true, nostop: true})
        run('git tag v' + version)
        run('git push --tags -u origin master', {noshow: true, filter: true})
        
    } finally {
        App.chdir(home)
    }
}

public function publish() {
    let aws = Cmd.locate('aws')
    if (!aws) {
        throw 'Cannot locate aws'
    }
    let awsProfile
    /*
        Prefer: AWS_PROFILE, then AWS_ACCESS_KEY_ID, then legacy PUBLISH_KEYS_AWS_ACCESS
     */
    if (!App.getenv('AWS_PROFILE')) {
        if (!App.getenv('AWS_ACCESS_KEY_ID')) {
            if (!App.getenv('PUBLISH_KEYS_AWS_ACCESS')) {
                throw 'AWS_PROFILE or ACCESS_KEY_ID is not defined'
            }
            //  Legacy
            App.putenv('AWS_ACCESS_KEY_ID',     App.getenv('PUBLISH_KEYS_AWS_ACCESS'))
            App.putenv('AWS_SECRET_ACCESS_KEY', App.getenv('PUBLISH_KEYS_AWS_SECRET'))
            App.putenv('AWS_DEFAULT_REGION',    App.getenv('PUBLISH_REGION'))
        }
    }
    let name = me.settings.name
    let p = me.platform
    let dir = Path('build/' + p.os + '-' + p.arch + '-' + p.profile + '/img')
    let bucket = App.getenv('BUCKET') || App.getenv('PUBLISH_BUCKET') || App.getenv('PUBLISH_PUBLICBUCKET')
    let files

    /*
        Use mac only for source packages
     */
    if (me.settings.title.contains('Community')) {
        files = dir.files(name + '-src.tgz')
    } else if (Config.OS == 'macosx') {
        files = dir.files(name + '-*-src*') + dir.files(name + '*.pkg')
    } else if (Config.OS == 'windows') {
        files = dir.files(name + '*exe.zip')
    } else if (Config.OS == 'linux') {
        /* We don't create linux packages for now */
    }
    if (!files || files.length == 0) {
        print('No files to publish')
    } else {
        let description = me.settings.description.replace(/Embedthis /, '')
        let google = Path(App.home.join('Google Drive').join(description))
        if (google.exists) {
            for each (file in dir.files(name + '-*-src*')) {
                print('cp ' + file + ' "' + google.join(file.basename) + '"')
                run('cp ' + file + ' "' + google.join(file.basename) + '"')
            }
        }
        for each (file in files) {
            let base = file.basename
            if (awsProfile) {
                print(aws + ' s3 --quiet cp ' + file + ' ' + 's3://' + bucket + '/' + base + ' --profile ' + awsProfile)
                run(aws + ' s3 --quiet cp ' + file + ' ' + 's3://' + bucket + '/' + base + ' --profile ' + awsProfile)
            } else {
                print(aws + ' s3 --quiet cp ' + file + ' ' + 's3://' + bucket + '/' + base)
                run(aws + ' s3 --quiet cp ' + file + ' ' + 's3://' + bucket + '/' + base)
            }
        }
    }
}


/*
    This software is distributed under a commercial license.
 */
