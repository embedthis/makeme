/*
 */

module ejs.testme {

function ttrue(cond: Boolean) {
    if (!cond) {
        let error = new Error('Assertion failed')
        let top = error.stack[0]
        print('fail in ' + top.filename + '@' + top.lineno + ' for ' + top.code)
    }
    print('pass')
}

function tfalse(cond: Boolean) {
    ttrue(!cond)
}

function tinfo(...args) {
    print('info', ...args)
}

function twrite(...args) {
    print('write', ...args)
}

function tget(key: String, def) {
    let value = App.getenv(key)
    if (value == null) {
        value = def
    }
    return value
}

function tset(key: String, value: String) {
    App.putenv(key, value)
}

function tskip(...msg) {
    print('skip', ...msg)
}

}
