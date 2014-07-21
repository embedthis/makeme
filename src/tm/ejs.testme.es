/*
    ejs.testme.me - Client-side testme ejs library
 */

module ejs.testme {
    require ejs.unix

    const PIDFILE = '.testme-pidfile'

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

    function tget(key: String, def = null) {
        let value = App.getenv(key)
        if (value == null) {
            value = def
        }
        return value
    }

    function tphase(): String?
        tget('TM_PHASE')
        
    function tset(key: String, value: String) {
        App.putenv(key, value)
    }

    function tskip(...msg) {
        print('skip', ...msg)
    }

    public function startService(cmdline: String, options = {}): Void {
        stopService()
        let pidfile = options.pidfile || PIDFILE
        let address: Uri = Uri(options.address || tget('TM_HTTP') || App.config.uris.http).complete()
        if (!tget('TM_NOSERVER')) {
            let cmd = new Cmd
            blend(options, {detach: true})
            cmd.start(cmdline, options)
            cmd.finalize()
            let pid = cmd.pid
            Path(pidfile).write(pid)
        }
        let connected
        for (i in 50) {
            try {
                let sock = new Socket
                sock.connect(address.host + ':' + address.port)
                sock.close()
                connected = true
                tinfo('Started', cmdline)
                break
            } catch (e) {
                App.sleep(100)
            }
        }
        if (!connected) {
            ttrue(connected)
            tinfo('Cannot connect to service: ' + cmdline + ' on ' + address)
        }
    }

    public function stopService(options = {}) {
        let pidfile = options.pidfile || PIDFILE
        if (Path(pidfile).exists) {
            pid = Path(pidfile).readString()
            Path(pidfile).remove()
            try { kill(pid, 9); } catch (e) { }
            App.sleep(500);
            tinfo('Stopped', 'Pid ' + pid)
        }
    }

    function failSafeKill(cmd) {
        for each (program in Cmd.ps(cmd)) {
            kill(program.pid, 9)
        }
    }

    public function startStopService(cmd: String, options = {}): Void {
        if (tphase() == 'Setup') {
            failSafeKill(cmd)
            startService(cmd, options)
        } else {
            stopService()
            failSafeKill(cmd)
        }
    }
}
