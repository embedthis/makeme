/*
    ejs.testme.me - Client-side testme ejs library
 */

module ejs.testme {
    require ejs.unix

    const PIDFILE = '.testme-pidfile'

    function tdebug(...args) {
        print('debug', ...args)
    }

    function tdepth(): Number
        tget('TM_DEPTH') - 0

    function teq(a, b, msg) {
        if (a != b) {
            let error = new Error('Assertion failed')
            let top = error.stack[1]
            print('fail in ' + top.filename + '@' + top.lineno + ' for ' + top.code)
            if (msg) {
                print('info ' + msg)
            }
        }
    }
        
    function tfalse(cond: Boolean) {
        ttrue(!cond)
    }

    function tfail(msg = null) {
        let error = new Error('Assertion failed')
        let top = error.stack[1]
        print('fail in ' + top.filename + '@' + top.lineno + ' for ' + top.code)
        if (msg) {
            print('info ' + msg)
        }
    }

    function tget(key: String, def = null) {
        let value = App.getenv(key)
        if (value == null) {
            value = def
        }
        return value
    }

    function thas(key: String): Boolean
        tget(key) - 0

    //  Shown with verbose only
    //  Would be better if tinfo was always output and tdebug was output in verbose mode
    function tinfo(...args) {
        print('info', ...args)
    }

    function tmatch(s: String, pattern: String) {
        if (smatch(s, pattern)) {
            let error = new Error('String "' + s + '"' + ' does not match pattern "' + pattern + '"')
            let top = error.stack[1]
            print('fail ' + top.filename + '@' + top.lineno + ' for ' + top.code)
        } else {
            let ok = new Error()
            let top = ok.stack[1]
            print('pass ' + top.filename + '@' + top.lineno + ' for ' + top.code)
        }
    }

    function tphase(): String?
        tget('TM_PHASE')
        
    function tset(key: String, value: String) {
        App.putenv(key, value)
        print('set', key, value)
    }

    function tskip(...msg) {
        print('skip', ...msg)
        App.exit(0)
    }

    function ttrue(cond: Boolean) {
        if (!cond) {
            let error = new Error('Assertion failed')
            let top = error.stack[1]
            print('fail ' + top.filename + '@' + top.lineno + ' for ' + top.code)
        } else {
            let ok = new Error()
            let top = ok.stack[1]
            print('pass ' + top.filename + '@' + top.lineno + ' for ' + top.code)
        }
    }

    function tverbose(...args) {
        print('verbose', ...args)
    }

    //  Shown always
    function twrite(...args) {
        print('write', ...args)
    }

    public function connectToService(cmdline: String, options = {}, retries: Number = 1): Boolean {
        let pidfile = Path(options.pidfile || PIDFILE)
        let pid
        if (pidfile.exists) {
            pid = pidfile.readString()
        } else {
            for each (program in Cmd.ps(Path(cmdline).basename)) {
                pid = program.pid
            }
        }
        let address: Uri = Uri(options.address || tget('TM_HTTP') || App.config.uris.http).complete()
        let connected
        for (i in retries) {
            try {
                let sock = new Socket
                sock.connect(address.host + ':' + address.port)
                sock.close()
                connected = true
                tinfo('Connected', 'to ' + Path(cmdline).basename + ' (' + pid + ')')
                return true
            } catch (e) {
                App.sleep(250)
            }
        }
        return false
    }

    public function startService(cmdline: String, options = {}): Boolean {
        let connected = connectToService(cmdline, options, 1)
        if (!connected && !tget('TM_NOSERVER')) {
            let pidfile = options.pidfile || PIDFILE
            let address: Uri = Uri(options.address || tget('TM_HTTP') || App.config.uris.http).complete()
            let cmd = new Cmd
            blend(options, {detach: true})
            cmd.start(cmdline, options)
            cmd.finalize()
            let pid = cmd.pid
            Path(pidfile).write(pid)
            tinfo('Started', cmdline + ' (' + pid + ')')
            App.sleep(250)
            if ((connected = connectToService(cmdline, options, 30)) != true) {
                tfail('Cannot connect to service: ' + cmdline + ' on ' + address)
                return false
            }
        }
        ttrue(connected)
        return true
    }

    public function stopService(options = {}): Boolean {
        if (tget('TM_NOSERVER')) return true
        let pidfile = options.pidfile || PIDFILE
        if (Path(pidfile).exists) {
            pid = Path(pidfile).readString()
            Path(pidfile).remove()
            try { kill(pid, 9); } catch (e) { }
            App.sleep(500);
            tinfo('Stopped', 'Process (' + pid + ')')
        }
        return true
    }

    function failSafeKill(cmd) {
        for each (program in Cmd.ps(cmd)) {
            kill(program.pid, 9)
        }
    }

    public function startStopService(cmd: String, options = {}): Boolean {
        if (tget('TM_NOSERVER')) return true
        if (tphase() == 'Setup') {
            return startService(cmd, options)
        } else {
            return stopService()
        }
    }

    function cleanDir(dir: Path) {
        for each (file in dir.files()) {
            file.remove()
        }
    }
}
