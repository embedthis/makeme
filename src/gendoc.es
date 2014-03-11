/*
    gendoc.es - Generate HTML doc from Doxygen XML files

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

module embedthis.doc {

    var all: Boolean
    var bare: Boolean
    var out: File?
    var outPath: Path
    var symbols = {}
    var reserved = {"int": true, "char": true, "long": true, "void": true, "...": true, "va_list": true, "struct": true,
        "...)": true, ")": true, "": true, "double": true, "HWND": true, "volatile": true }
    var seeAlso = {}
    var tagFile = null
    var title: String = "Documentation"

    function usage(): String {
        return "usage: gendoc [--all] [--bare] [--tags tagfile] [--out outFile] [--title Title] files..."
    }

    function parseArgs(): Array {
        let files = []
        for (argind = 1 ; argind < App.args.length; argind++) {
            arg = App.args[argind]
            switch (arg) {
            case "--all":
            case "-a":
                all = true
                break

            case "--bare":
                bare = true
                break

            case "--out":
                if (++argind >= App.args.length) {
                    throw usage()
                }
                outPath = App.args[argind]
                out = File(outPath, "w")
                break

            case "--tags":
                if (++argind >= App.args.length) {
                    throw usage()
                }
                let tags = blend(symbols, Path(App.args[argind]).readJSON())
                break

            case "--title":
                if (++argind >= App.args.length) {
                    throw usage()
                }
                title = App.args[argind]
                break

            default:
                files = App.args.slice(argind) 
                argind = App.args.length
            }
        }
        if (out == null) {
            throw usage()
        }
        return files
    }


    function emit(...args) {
        out.write(args.toString() + "\n")
    }


    function emitHeader() {
        if (bare) {
            return
        }
        emit('<!DOCTYPE html>')
        emit('<html lang="en">')
        emit('<head>')
        emit('<meta charset="utf-8"/>')
        emit('<title>' + title + ' Documentation</title>')
        emit('<link href="api.css" rel="stylesheet" type="text/css" />')
        emit('</head>\n<body>\n')
        emit('  <div class="contents">')
    }


    function emitFooter() {
        if (bare) {
            return
        }
        emit('<div class="footnote">Generated on ' + new Date + '<br/>')
        emit('  Copyright &copy; <a href="http://embedthis.com">Embedthis Software</a> ' + 
            new Date().year + '.')
        emit('</div></div></body></html>')
    }


    /*
        Emit top level navigation
     */
    function emitNavigation() {
        if (bare) {
            return
        }
        emit('<div>')
        result = []
        for each (kind in ["Extensions", "Typedefs", "Functions", "Defines"]) {
            result.append('<a href="#' + kind + '">' + kind + '</a>')
        }
        emit(result.join(" | "))
        emit('</div>')
    }


    /*
        Parse all symbol references and build an index of symbols
     */
    function parseReferences(xml: XML) {
        for each (def in xml) {
            if (def.@kind == "group" || def.@kind == "struct") {
                symbols[def.compoundname] = def.@id
                for each (sect in def.detaileddescription.para.simplesect) {
                    if (sect.@kind != "see") {
                        continue
                    }
                    seeAlso[def.compoundname] = sect.para
                }
            }
        }
        var sections: XML = xml.compounddef.sectiondef
        for each (section in sections) {
            var members: XML = section.memberdef
            for each (m in members) {
                if (m.name != "" && m.@id != "") {
                    symbols[m.name] = m.@id
                }
            }
        }
    }


    /*
        Emit a reference
     */
    function ref(name: String): String {

        /* Split the core type name off from "*" suffixes */
        parts = name.split(" ")
        typeSpec = parts.slice(0, 1)
        rest = parts.slice(1)
        sym = typeSpec.toString().trim("*")
        let value = symbols[sym]
        if (value) {
            let result
            if (value.toString().contains("#")) {
                result = '<a href="' + symbols[sym] + '" class="ref">' + typeSpec + '</a>'
            } else {
                result = '<a href="#' + symbols[sym] + '" class="ref">' + typeSpec + '</a>'
            }
            if (rest && rest != "") {
               result += " " + rest
            }
            return result
        }
        if (!reserved[sym]) {
            App.log.error("Cannot find link symbol \"" + sym + "\"")
        }
        return name
    }


    function strip(str: String): String {
        if (str == "") {
            return str
        }
        str = str.replace(/<para>|<emphasis>|<title>|<type>|<\/para>|<\/emphasis>|<\/title>|<\/type>/g, "")
        str = str.replace(/<ref refid="[^>]*>/g, '')
        str = str.replace(/<\/ref>/g, '')
        str = str.replace(/ kindref="[^"]*"/g, '')
        str = str.replace(/itemizedlist>/g, '')
        str = str.replace(/listitem>/g, '')
        str = str.replace(/<linebreak\/>/g, '')
        str = str.replace(/bold>/g, '')
        str = str.trim().trim(".").trim().trim(".")
        return str
    }


    /*
        Remove or map XML elements into HTML
     */
    function clean(str: String): String {
        if (str == "") {
            return str
        }
        str = str.replace(/<para>|<emphasis>|<title>|<type>|<\/para>|<\/emphasis>|<\/title>|<\/type>/g, "")
        str = str.replace(/<ref refid="([^"]*#[^"]*)"/g, '<a class="ref" AAA href="$1"')
        str = str.replace(/<ref refid="/g, '<a class="ref" href="#')
        str = str.replace(/<\/ref>/g, '</a>')
        str = str.replace(/ kindref="[^"]*"/g, "")
        str = str.replace(/itemizedlist>/g, 'ul>')
        str = str.replace(/listitem>/g, 'li>')
        str = str.replace(/<linebreak\/>/g, "<br/>")
        str = str.replace(/bold>/g, "b>")
        str = str.replace(/preformatted>/g, "pre>")
        str = str.replace(/<row>/g, "<tr>")
        str = str.replace(/<entry thead="no">/g, "<td>")
        str = str.replace(/<table rows=[^>]*>/g, "<table class='info'>")
        str = str.replace(/<\/entry>/g, "</td>")
        str = str.trim().trim(".").trim().trim(".")
        str = str.replace(/--/g, "&mdash;")
        return str
    }


    /*
        Clean the string of XML elements and append a "."
     */
    function cleanDot(str: String): String {
        s = clean(str)
        if (s == "") {
            return s
        }
        if (!s.endsWith(">")) {
            s = s + "."
        }
        return s
    }


    /*
        Intelligently transform a @return directive
     */
    function cleanReturns(str: String): String {
        str = cleanDot(str)
        str = str.replace(/^Return[s]* the/, "The")
        return str
    }


    /*
        Emit a file level overview
     */
    function emitOverview(xml: XML) {
        emit("<h1>" + title + "</h1>")
        for each (def in xml) {
            if (def.@kind != "file") {
                continue
            }
            name = def.compoundname

            if (!all && def.briefdescription == '' && def.detaileddescription == '') {
                return
            }
            emit('<a name="' +  name + '"></a>')

            if (def.briefdescription != "") {
                emit('<p>' + cleanDot(def.briefdescription.para) + '</p>')
            }
            if (def.detaileddescription != "") {
                for each (node in def.detaileddescription.*) {
                    str = node.toString()
                    str = str.replace(/para>/g, "p>")
                    str = str.replace(/title>/g, "h1>")
                    str = str.replace(/<linebreak\/>/g, "<br/>")
                    str = str.replace(/<sect[^>]*>|<\/sect[0-9]>/g, "")
                    str = str.replace(/<simplesect[^>]*>|<\/simplesect>/g, "")
                    emit(str)
                }
            }
        }
    }


    /*
        Emit an index of all services
     */
    function emitServiceIndex(def: XML) {
        if (all || def.briefdescription != "" || def.detaileddescription != '') {
            emit('<tr class="apiDef">')
            emit('<td class="apiName"><a href="#' + def.@id + '" class="nameRef">' + def.compoundname + '</a></td>')
            emit('<td class="apiBrief">' + cleanDot(def.briefdescription.para) + '</td></tr>')
        }
        symbols[def.compoundname] = def.@id
    }


    /*
        Generate an index of all functions
     */
    function genFunctionIndex(def: XML, section: XML, index: Object) {
        var members: XML = section.memberdef

        for each (m in members) {
            if (m.@kind == 'function') {
                if (!all && m.briefdescription == '' && m.detaileddescription == '') {
                    continue
                }
                if (def.@kind == "file" && m.@id.toString().startsWith("group__")) {
                    continue
                }
                let definition = m.definition.toString().split(" ")
                typedef = definition.slice(0, -1).join(" ")
                name = definition.slice(-1)

                let str = '<tr class="apiDef"><td class="apiType">' + ref(typedef) + '</td><td>'
                str += '<a href="#' + m.@id + '" class="nameRef">' + name + '</a>'

                args = m.argsstring.toString().trim('(').split(",")
                if (args.length > 0) {
                    result = []
                    for (i in args) {
                        arg = args[i].trim()
                        result.append(ref(arg))
                    }
                    str += "(" + result.join(", ")
                }
                str += '</td></tr>'
                if (m.briefdescription != "") {
                    str += '<tr class="apiBrief"><td>&nbsp;</td><td>' + cleanDot(m.briefdescription.para) + '</td></tr>'
                }
                index[name] = str
            }
        }
    }


    /*
        Emit an index of all #define directives
     */
    function emitDefineIndex(section: XML) {
        var members: XML = section.memberdef

        for each (m in members) {
            if (m.@kind == 'define') {
                if (!all && m.briefdescription == '' && m.detaileddescription == '') {
                    continue
                }
                symbols[m.name] = m.@id

                emit('<tr class="apiDef">')
                emit('<td>#define</td><td><a href="#' + m.@id + '" class="nameRef">' + m.name + '</a>' + 
                    '&nbsp;&nbsp;&nbsp;' + m.initializer + '</td>')
                emit('</tr>')
                if (m.briefdescription != "") {
                    emit('<tr class="apiBrief"><td>&nbsp;</td><td>' + 
                        cleanDot(m.briefdescription.para) + '</td></tr>')
                }
            }
        }
    }


    /*
        Emit an index of struct based typedefs
     */
    function emitTypeIndex(def: XML, index: Object) {
        var str
        if (all || def.briefdescription != "" || def.detaileddescription != '') {
            name = def.compoundname
            symbols[name] = def.@id
            str = '<tr class="apiDef">'
            str += '<td class="apiName">'
            str += '<a href="#' + symbols[name] + '" class="nameRef">' + name + '</a></td>'
            str += '<td class="apiBrief">' + cleanDot(def.briefdescription.para) + '</td></tr>'
            index[name] = str
        }
    }


    /*
        Emit an index of all simple (non-struct) typedefs
     */
    function emitStructTypeIndex(section: XML, index: Object) {
        var members: XML = section.memberdef
        var str

        for each (m in members) {
            if (m.@kind == 'typedef') {
                if (!all && m.briefdescription == '' && m.detaileddescription == '') {
                    continue
                }
                symbols[m.name] = m.@id
                def = m.definition.toString()
                if (def.contains("(")) {
                    /* Parse "typedef void(* MprLogHandler)(cchar *file, ... cchar *msg) */
                    name = def.toString().replace(/typedef[^\(]*\([^\w]*([\w]+).*/, "$1")
                } else {
                    def = def.toString().split(" ")
                    typedef = def.slice(0, -1).join(" ")
                    name = def.slice(-1)
                }
                str = '<tr class="apiDef">'
                str += '<td class="apiName">'
                str += '<a href="#' + m.@id + '" class="nameRef">' + name + '</a></td>'
                if (m.briefdescription != "") {
                    str += '<td class="apiBrief">' + cleanDot(m.briefdescription.para) + '</td></tr>'
                }
                index[name] = str
            }
        }
    }


    /*
        Get master group references for a given group name
     */
    function getGroupReferences(group: String): Array {
        references = []
        items = seeAlso[group]
        if (items != undefined) {
            for each (see in items.ref.*) {
                references.append(see)
            }
        }
        return references
    }


    /*
        Emit a See Also section. Handle groups/struct @references
     */
    function emitSeeAlso(name: String, node: XML) {
        if (node.para.toString().startsWith('@-')) {
            let name = node.para.toString().slice(2).trim()
            references = getGroupReferences(name)

        } else {
            references = []
            for each (see in node.para.ref.*) {
                references.append(see)
            }
        }
        emitSeeAlsoReferences(name, references)
    }


    function emitSeeAlsoReferences(name: String, references: Array) {
        emit('    <dl><dt>See Also:</dt><dd>')
        references.sort()
        trimmed = []
        for each (see in references) {
            if (see == name) {
                continue
            }
            trimmed.append(ref(see))
        }
        emit('    ' + trimmed.join(", ") + '</dd></dl>')
    }


    function emitSimpleSect(name: String, node: XML) {
        if (node.@kind == "see") {
            emitSeeAlso(name, node)

        } else if (node.@kind == "return") {
            emit('    <dl><dt>Returns:</dt><dd>' + cleanReturns(node.para).toPascal() + '</dd></dl>')

        } else if (node.@kind == "remark") {
            emit('    <dl><dt>Remarks:</dt><dd>' + cleanDot(node.para).toPascal() + '</dd></dl>')

        } else {
            emit('    <dl><dt>' + clean(node.title) + '</dt><dd>' + cleanDot(node.para).toPascal() + '</dd></dl>')
        }
    }


    /*
        Emit function args
     */
    function emitArgs(args: XML) {
        result = []
        for each (p in args.param) {
            if (p.type == "...") {
                result.append("...")
            } else {
                s = ref(strip(p.type)) + " " + p.declname
                s = s.replace(/ ([\*]*) /, " $1")
                result.append(s)
            }
        }
        emit("(" + result.join(", ") + ")")
    }


    /*
        Used for function and simple typedef details
     */
    function emitDetail(def: XML, section: XML) {
        var members: XML = section.memberdef

        if (section.@kind == "func") {
            kind = "function"

        } else if (section.@kind == "typedef") {
            kind = "typedef"
        }
        for each (m in members) {
            if (m.@kind == kind) {
                if (!all && m.briefdescription == '' && m.detaileddescription == '') {
                    continue
                }
                if (def.@kind == "file" && m.@id.toString().startsWith("group__")) {
                    continue
                }
                emit('<a name="' + m.@id + '"></a>')
                emit('<div class="api">')
                emit('  <div class="prototype">')

                if (kind == "function") {
                    emit('    ' + ref(strip(m.type)))
                    str = m.definition.toString().split(" ").slice(1).join(" ")
                    emit('    ' + clean(str))
                    emitArgs(m)

                } else if (kind == "typedef") {
                    emit('    ' + cleanDot(m.definition))
                }
                emit('  </div>')

                emit('  <div class="apiDetail">')

                if (m.briefdescription != "") {
                    emit('<p>' + cleanDot(m.briefdescription.para) + '</p>')
                }

                seen = false
                for each (n in m.detaileddescription.para.*) {
                    if (n.name() == "simplesect") {
                        if (n.@kind == "see") {
                            seen = true
                        }
                        emitSimpleSect(m.name, n)

                    } else if (n.name() == "parameterlist") {
                        /*
                            Parameters
                         */
                        emit('    <dl><dt>Parameters:</dt><dd>')
                        emit('    <table class="parameters" title="Parameters">')
                        for each (p in n.parameteritem) {
                            emit('    <tr><td class="param">' + p.parameternamelist.parametername + '</td><td>' + 
                                cleanDot(p.parameterdescription.para) + '</td>')
                        }
                        emit('    </table></dd></dl>')

                    } else {
                        emit(clean(n))
                    }
                }
                if (!seen && def.@kind == "group") {
                    references = getGroupReferences(def.compoundname)
                    emitSeeAlsoReferences(m.name, references)
                }
                emit('  </div>')
                emit('</div>')
            }
        }
    }


    function emitFields(name: String, def: XML) {
        let doneHeader = false
        for each (m in def.sectiondef.memberdef) {
            if (!doneHeader) {
                emit('    <dl><dt>Fields:</dt><dd>')
                emit('    <table class="parameters" title="Parameters">')
                doneHeader = true
            }
            if (m.@kind == "variable") {
                field = m.definition.toString().replace(/.*::/, "")
                if (m.briefdescription != "" || m.detaileddescription != "") {
                    emit('    <tr><td class="param">' + clean(m.type) + '</td><td><td>' + field + '</td><td>')
                    if (m.briefdescription != "") {
                        s = cleanDot(m.briefdescription.para)
                        if (m.detaileddescription != "") {
                            s += " "
                        }
                        emit(s)
                    }
                    if (m.detaileddescription != "") {
                        s = cleanDot(m.detaileddescription.para)
                        emit(s)
                    }
                    emit('</td>')
                }
            }
        }
        if (doneHeader) {
            emit('    </table></dd></dl>')
        }
    }


    function emitStructDetail(def: XML, fields: XML?) {
        let name = def.compoundname
        if (!all && def.briefdescription == '' && def.detaileddescription == '') {
            return
        }
        emit('<a name="' + symbols[name] + '"></a>')
        emit('<div class="api">')
        emit('  <div class="prototype">' + clean(name) + '</div>')
        emit('  <div class="apiDetail">')

        if (def.briefdescription != "") {
            emit('<p>' + cleanDot(def.briefdescription.para) + '</p>')
        }
        let doneFields = false
        if (def.detaileddescription != "") {
            for each (n in def.detaileddescription.para.*) {
                if (n.name() == "simplesect") {
                    emitSimpleSect(name, n)

                    if (!doneFields && fields == null) {
                        emitFields(name, def)
                        doneFields = true
                    }
                }
            }
        }
        if (fields) {
            emitFields(name, fields)
        } else if (!doneFields) {
            emitFields(name, def)
        }
        emit('  </div>')
        emit('</div>')
    }


    function emitIndicies(xml: XML) {
        var sections: XML

        /*
            Emit the group indicies
         */
        emit('<a name="Extensions"></a><h1>Extensions</h1>')
        emit('  <table class="apiIndex" title="Extensions">')

        for each (def in xml) {
            if (def.@kind == "group") {
                emitServiceIndex(def)
            }
        }
        emit('</table>')

        /*
            Emit the navigation indicies
         */
        emit('<a name="Functions"></a><h1>Functions</h1>')
        emit('  <table class="apiIndex" title="Functions">')

        let functionIndex = {}
        for each (def in xml) {
            sections = def.sectiondef
            for each (section in sections) {
                if (section.@kind == "func") {
                    genFunctionIndex(def, section, functionIndex)
                }
            }
        }
        Object.sortProperties(functionIndex)
        for each (i in functionIndex) {
            emit(i)
        }
        emit('</table>')

        emit('<a name="Typedefs"></a><h1>Typedefs</h1>')
        emit('<table class="apiIndex" title="typedefs">')
        let typeIndex = {}
        for each (def in xml) {
            if (def.@kind == "struct") {
                emitTypeIndex(def, typeIndex)
            } else {
                sections = def.sectiondef
                for each (section in sections) {
                    if (section.@kind == "typedef") {
                        emitStructTypeIndex(section, typeIndex)
                    }
                }
            }
        }
        Object.sortProperties(typeIndex)
        for each (i in typeIndex) {
            emit(i)
        }
        emit('</table>')

        emit('<a name="Defines"></a><h1>Defines</h1>')
        emit('<table class="apiIndex" title="Defines">')
        for each (def in xml) {
            sections = def.sectiondef
            for each (section in sections) {
                if (section.@kind == "define") {
                    emitDefineIndex(section)
                }
            }
        }
        emit("  </table>")
    }


    function emitDetails(xml: XML) {
        /*
            Emit groups
         */
        for each (def in xml) {
            if (def.@kind != "group") {
                continue
            }
            emit('<h1>' + def.compoundname + '</h1>')
            let foundStruct = false
            for each (d in xml) {
                if (d.@kind == "struct" && d.compoundname.toString() == def.compoundname.toString()) {
                    foundStruct = true
                    emitStructDetail(def, d)
                }
            }
            if (!foundStruct) {
                for each (d in xml) {
                    if (d.compoundname.toString() == def.compoundname.toString()) {
                        emitStructDetail(def, d)
                    }
                }
            }
            let sections: XML = def.sectiondef
            for each (section in sections) {
                if (section.@kind == "func") {
                    emitDetail(def, section)
                }
            }
        }

        /*
            Emit functions
         */
        emit('<h2>Functions</h2>')

        for each (def in xml) {
            if (def.@kind == "group") {
                continue
            }
            let sections: XML = def.sectiondef
            for each (section in sections) {
                if (section.@kind == "func") {
                    emitDetail(def, section)
                }
            }
        }

        emit('<h2>Typedefs</h2>')
        for each (def in xml) {
            if (def.@kind == "struct") {
                emitStructDetail(def, null)
            } else {
                let sections: XML = def.sectiondef
                for each (section in sections) {
                    if (section.@kind == "typedef") {
                        emitDetail(def, section)
                    }
                }
            }
        }
    }


    function parse(xml: XML) {
        parseReferences(xml)
        emitHeader()
        emitNavigation()
        emitOverview(xml)
        emitIndicies(xml)
        emitDetails(xml)
        emitFooter()
    }


    function saveTags() {
        let name = outPath.basename
        let tags = outPath.replaceExt("tags");
        for (s in symbols) {
            symbols[s] = name + "#" + symbols[s]
        }
        Path(tags).write(serialize(symbols))
    }


    function main() {
        var xml: XML

        for each (f in parseArgs()) {
            var tmp = new XML(f)
            if (xml) {
                len = xml.compounddef.length()
                xml.compounddef[len] = tmp.compounddef
            } else {
                xml = tmp
            }
        }
        parse(xml)
        out.close()
        saveTags()
    }

    main()

    function e(...args) {
        App.errorStream.write(args + "\n")
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
