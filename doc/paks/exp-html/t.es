/*
    MOB
    extract extracting styles?
 */

let text = `<script>alert('inline1')</script>
<div id="43" onclick="alert('hi')">outer<p>inner</p></div>
<span before=23 onclick="alert('hello world')" custom='42'></span>
<script src="abc.js"></script>
<script abc="none" id='mob'>alert('inline2')</script>`

print("\nINPUT\n\n" + text, '\n')

/*
    Handle <script> tags
    Extract and remove inline scripts
 */
function handleScripts(state, input: String): String {
    let re = /<script[^>]*>(.*)<\/script>/mg
    let start = 0, end = 0
    let output = ''
    let matches
    state.scripts ||= []

    while (matches = re.exec(input, start)) {
        let elt = matches[0]
        end = re.lastIndex - elt.length
        output += input.slice(start, end)
        if (elt.match(/src=['"]/)) {
            output += matches[0]
        } else {
            state.scripts.push(matches[1])
        }
        start = re.lastIndex
    }
    return output + input.slice(start)
}

/*
    MOB - must handle backquoted quotes
    MOB - would be great to handle existing IDs
    Extract and remove onclick="..." 
    Add ID if not one present of the form exp-NNN
    Add listener to external script file of the name OriginalFilename.js
        document.getElementById(id).addEventListener('click', function() {
        });
*/

function handleClicks(state, text: String): String {
    let nextId = 0
    let result = ''
    let re = /<([\w_\-:.]+)([^>]*>)/g
    let start = 0, end = 0
    let matches
    state.onclick ||= {}

    while (matches = re.exec(text, start)) {
        let elt = matches[0]
        end = re.lastIndex - matches[0].length
        /* Emit prior text */
        result += text.slice(start, end)

        let clickre = /(.*) +(onclick=)(["'])(.*)(\3)(.*)/m
        if ((cmatches = clickre.exec(elt)) != null) {
            elt = cmatches[1] + cmatches[6]
            let id
            if ((ematches = elt.match(/id=['"]([^'"]+)['"]/)) != null) {
                id = ematches[1]
            } else {
                id = 'exp-' + md5(++nextId)
                elt = cmatches[1] + ' id="' + id + '"' + cmatches[6]
            }
            state.onclick[id] = cmatches[4]
        }
        result += elt
        start = re.lastIndex
    }
    return result + text.slice(start)
}

let state = {}
let result = handleClicks(state, handleScripts(state, text))

print("\nOUTPUT")
print(result)

print("\nSCRIPTS")
dump(state.scripts)

print("\nCLICK")
dump(state.onclick)

//  MOB - process scripts[]
