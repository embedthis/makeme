/*
    dev.es - me-dev Support functions

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.unix

public function genProjects(extensions = '', profiles = ["default", "static"], platforms = null) 
{
    if (platforms is String) {
        platforms = [platforms]
    }
    if (profiles is String) {
        profiles = [profiles]
    }
    platforms ||= ['freebsd-x86', 'linux-x86', 'macosx-x64', 'vxworks-x86', 'windows-x86']
    let cmd = Cmd.locate('me').relative
    let runopt = {dir: me.dir.src, show: true}
    if (extensions) {
        extensions +=  ' '
    }
    let home = App.dir
    try {
        App.chdir(me.dir.top)
        let src = me.dir.src.relative
        for each (name in platforms) {
            trace('Generate', me.settings.name + '-' + name.replace(/-.*/, '') + ' projects')
            for each (profile in profiles) {
                let formats = (name == 'windows-x86') ? '-gen nmake' : '-gen make'
                let platform = name + '-' + profile
                let options = (profile == 'static') ? ' -static' : ''
                run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                    ' ' + extensions + formats, runopt)
                /* Xcode and VS use separate profiles */
                if (name == 'macosx-x64') {
                    run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + extensions + '-gen xcode', runopt)
                } else if (name == 'windows-x86') {
                    run(cmd + ' -d -q -platform ' + platform + options + ' -configure ' + src + 
                        ' ' + extensions + '-gen vs', runopt)
                }
                rm(me.dir.top.join(platform + '.me'))
                rmdir(me.dir.top.join(platform))
            }
        }
    }
    finally {
        App.chdir(home)
    }
}



/*
    UNUSED Unfinished
 */
function pushToGit(package, prefixes, fmt) {
    let staging = prefixes.staging.absolute
    let base = staging.join(me.platform.vname)
    let name = me.settings.name
    let package = base.join('package.json')
    let version = package.readJSON().version
    let home = App.dir

    try {
        trace('Publish', me.settings.title + ' ' + version)
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
