/*
    doc.es - me-doc support functions

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix
require ejs.zlib

public function apidoc(dox: Path, headers, title: String, tags) {
    let name = dox.basename.trimExt().name
    let api = me.dir.src.join('doc/api')
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
    run(['doxygen', doxtmp], {dir: api})
    if (output) {
        output.remove()
    }
    if (!me.options.keep) {
        doxtmp.remove()
    }
    trace('Process', name.toPascal() + ' documentation (may take a while)')
    let files = [api.join('xml/' + name + '_8h.xml')]
    files += ls(api.join('xml/group*')) + ls(api.join('xml/struct_*.xml'))
    let tstr = tags ? tags.map(function(i) '--tags ' + Path(i).absolute).join(' ') : ''

    run('ejs ' + me.dir.src.join('src/paks/me-doc/gendoc.es') + ' --bare ' + '--title \"' + 
        me.settings.name.toUpper() + ' - ' + title + ' Native API\" --out ' + name + '.html ' +  
        tstr + ' ' + files.join(' '), {dir: api})
    if (!me.options.keep) {
        rmdir([api.join('html'), api.join('xml')])
    }
}


/* UNUSED
public function apiwrap(patterns) {
    let files = Path('.').files(patterns)
    if (files.length == 0) {
        files = [Path(patterns)]
    }
    for each (dfile in files) {
        let name = dfile.replace('.html', '')
        let data = Path(name + 'Bare.html').readString()
        let contents = Path(name + 'Header.tem').readString() + data + 
            Path(name).dirname.join('apiFooter.tem').readString() + '\n'
        dfile.joinExt('html').write(contents)
    }
}
*/

public function apiLayout(from: Path, to: Path)
{
    trace('Generate', to)
    let contents = from.readString().replace(/\$/mg, '$$$$')
    let data = to.readString()
    to.write(data.
        replace(/DOC_CONTENT/g, contents).
        replace(/Bare.html/g, 'html'))
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
