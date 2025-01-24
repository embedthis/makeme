/*
    doc.es - me-doc support functions

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

require ejs.tar
require ejs.unix
require ejs.zlib

var origin = me.globals.ORIGIN

public function apidoc(dox: Path, headers, title: String, tags) {
    let name = dox.basename.trimExt().name
    let dir, doc
    if (Path(me.dir.top.join('doc')).exists) {
        dir = me.dir.top.join('doc/api')
        doc = 'doc'
    } else {
        dir = me.dir.top.join('api')
        doc = '.'
    }
    let output
    if (headers is Array) {
        output = dir.join(name + '.h')
        copyFiles(headers, output, { append: true })
        headers = output
    }
    rmdir([dir.join('html'), dir.join('xml')])

    dox.replaceExt('tags').remove()
    tags ||= me.dir.paks.join('*/doc/api/*.tags')
    tags = Path('.').files(tags)

    let doxtmp = Path('').temp()
    let data = me.dir.top.join(doc, name + '.dox').readString().replace(/^INPUT .*=.*$/m, 'INPUT = ' + headers)

    Path(doxtmp).write(data)
    trace('Generate', name.toPascal() + ' documentation')

    run(['doxygen', doxtmp], {dir: dir})
    if (output) {
        output.remove()
    }
    if (!me.options.keep) {
        doxtmp.remove()
    }
    trace('Process', name.toPascal() + ' documentation (may take a while)')
    let files = [dir.join('xml/' + name + '_8h.xml')]

    files += ls(dir.join('xml/group*')) + ls(dir.join('xml/struct_*.xml'))
    let tstr = tags ? tags.map(function(i) '--tags ' + Path(i).absolute).join(' ') : ''

    run('ejs ' + origin.join('gendoc.es') + ' --bare ' + '--title \"' +
        title + ' API\" --out ' + name + '.html ' +
        tstr + ' ' + files.join(' '), {dir: dir})

    for each (tag in tags) {
        let dir = tag.dirname
        cp(dir.join('*'), 'api')
    }
    if (!me.options.keep) {
        rmdir([dir.join('html'), dir.join('xml')])
    }
}


public function apiLayout(from: Path, to: Path, options = {})
{
    trace('Generate', to)
    let contents = from.readString().replace(/\$/mg, '$$$$')
    let data = to.readString()
    to.write(data.
        replace(/DOC_CONTENT/g, contents).
        replace(/Bare.html/g, 'html'))

    if (options && options.fixup == 'md') {
        let data = to.readString()
        data = data.replace(/\<h2\>Functions\<\/h2\>\n\<h2\>Typedefs\<\/h2\>/, '<h2>Typedefs</h2>')
        data = data.replace(/\<h1\>(.*)\<\/h1\>/g, '\n')
        data = data.replace(/\<h2\>(.*)\<\/h2\>/g, '\n## $1\n')
        to.write(data)
    }
}

/*
    Copyright (c) Embedthis Software. All Rights Reserved.
 */
