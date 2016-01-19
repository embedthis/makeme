/*
    testme.es -- MakeMe Unit Test
 */
module ejs.testme {

require ejs.unix

enumerable class TestMe {
    const TIMEOUT: Number = 5 * 60 * 1000

    var cfg: Path?                          //  Path to configuration outputs directory
    var bin: Path                           //  Path to bin directory
    var top: Path                           //  Path to top of source tree
    var topTest: Path                       //  Path to top of test tree
    var original: Path                      //  Original current directory
    var mebin: Path                         //  Directory containing "me"

    //  DEPRECATE remove topDir, topTestDir
    var topDir: Path                        //  Path to top of source tree
    var topTestDir: Path                    //  Path to top of test tree

    var depth: Number = 1                   //  Test level. Higher levels mean deeper testing.

    var keepGoing: Boolean = false          //  Continue on errors 
    var topEnv: Object = {}                 //  Global env to pass to tests
    var filters: Array = []                 //  Filter tests by pattern x.y.z... 
    var noserver: Boolean = false           //  Omit running a server (sets TM_NOSERVER=1)
    var skipTest: Boolean                   //  Skip current test or directory
    var options: Object                     //  Command line options
    var program: String                     //  Program name
    var log: Logger = App.log
    var start = Date.now()
    var startTest
    var verbosity: Number = 0

    var done: Boolean = false
    var failedCount: Number = 0
    var passedCount: Number = 0
    var skippedCount: Number = 0
    var testCount: Number = 0

    //  TODO - remove when ejscript 3 is released
    var needTestMeMod: Boolean

    function unknownArg(argv, i, template) {
        let arg = argv[i].slice(argv[i].startsWith("--") ? 2 : 1)
        if (arg == '?') {
            tm.usage()
        } else if (isNaN(parseInt(arg))) {
            throw 'Unknown option: ' + arg
        } else {
            this.options.trace = 'stdout:' + arg
            this.options.log = 'stdout:' + arg
        }
        return i
    }

    let argsTemplate = {
        options: {
            chdir: { range: Path },
            compile: {},
            clean: {},
            clobber: {},
            'continue': { alias: 's' },
            debug: { alias: 'D' },
            depth: { range: Number, alias: 'd' },
            ide: { alias: 'i' },
            log: { alias: 'l', range: String },
            more: { alias: 'm' },
            noserver: { alias: 'n' },
            project: { },
            projects: { alias: 'p' },
            quiet: { alias: 'q' },
            rebuild: { alias: 'r' },
            show: { alias: 's' },
            trace: { alias: 't' },
            verbose: { alias: 'v' },
            version: { },
            why: { alias: 'w' },
        },
        unknown: unknownArg,
        usage: usage,
    }

    function usage(): Void {
        let program = Path(App.args[0]).basename
        App.log.write('Usage: ' + program + ' [options] [filter patterns...]\n' +
            '  --chdir dir           # Change to directory before testing\n' + 
            '  --clean               # Clean compiled tests\n' + 
            '  --clobber             # Remove testme directories\n' + 
            '  --compile             # Compile all C tests\n' + 
            '  --continue            # Continue on errors\n' + 
            '  --debug               # Run in debug mode. Sets TM_DEBUG\n' + 
            '  --depth number        # Zero == basic, 1 == throrough, 2 extensive\n' + 
            '  --ide                 # Run the test in an IDE debugger\n' + 
            '  --log file:level      # Log output to file at verbosity level\n' + 
            '  --more                # Pass output through "more"\n' + 
            '  --noserver            # Do not run server side of tests\n' + 
            '  --projects            # Generate IDE projects for tests\n' + 
            '  --quiet               # Quiet mode\n' + 
            '  --rebuild             # Rebuild all tests before running\n' + 
            '  --show                # Show commands executed\n' +
            '  --trace file:level    # HTTP request tracing\n' + 
            '  --verbose             # Verbose mode\n' + 
            '  --version             # Output version information\n' +
            '  --why                 # Show why commands are executed\n')
        App.exit(1)
    }

    function parseArgs(): Void {
        let args = Args(argsTemplate, App.args)
        options = args.options
        filters += args.rest

        original = App.dir
        if (options.chdir) {
            topTest = options.chdir
        }
        App.chdir(topTest)
        if (original != App.dir) {
            trace('Chdir', App.dir)
            let current = original.relativeTo(App.dir)
            if (filters.length > 0) {
                filters.transform(function(f) current.join(f))
            } else {
                filters = [current]
            }
        }
        if (options['continue']) {
            keepGoing = true
        }
        if (options.debug) {
            Debug.mode = true
        }
        if (options.depth) {
            depth = options.depth
            if (depth < 1 || depth > 9) {
                depth = 1
            }
        }
        if (options.ide && filters.length == 0) {
            App.log.error('Must specify at least one test')
            App.exit(1)
        }
        if (options.noserver) {
            noserver = true
            topEnv.TM_NOSERVER = '1';
        }
        if (options.verbose) {
            verbosity++
        }
        if (options.project) {
            /* Convenient alias */
            options.projects = options.project
        }
        if (options.projects) {
            if (filters.length == 0) {
                App.log.error('Must specify at least one test')
                App.exit(1)
            }
        }
        if (options.more) {
            let cmd = App.exePath + ' ' +
                App.args.slice(1).join(' ').replace(/[ \t]*-*more[ \t]*|[ \t]*-m[ \t]*/, ' ') + ' 2>&1 | more'
            if (options.show) {
                print(cmd)
            }
            Cmd.sh(cmd, {noio: true})
            App.exit(0)
        }
        if (options.version || options.log || options.trace) {
            /* Handled in C code */
        }
    }

    function TestMe() {
        program = Path(App.args[0]).basename
        if ((path = searchUp('configure')) != null) {
            top = path.dirname.absolute
            try {
                if (top.join('start.me')) {
                    cfg = top.join('build', Cmd.run('me --showPlatform', {dir: top}).trim())
                }
            } catch {}
            if (!cfg) {
                cfg = top.join('build').files(Config.OS + '-' + Config.CPU + '-*').sort()[0]
                if (!cfg) {
                    let hdr = top.files('build/*/inc/me.h').sort()[0]
                    if (hdr) {
                        cfg = hdr.trimEnd('/inc/me.h')
                    }
                }
            }
            bin = cfg.join('bin').portable
            if (cfg.join('inc/me.h').exists) {
                parseMeConfig(cfg.join('inc/me.h'))
            }
        } else {
            top = App.dir
        }
        if ((path = searchUp('TOP.es.set')) != null) {
            topTest = path.dirname.absolute.portable
        } else {
            topTest = top
        }
        //  DEPRECATE
        topDir = top
        topTestDir = topTest
    }

    function setupEnv() {
        let me = Cmd.locate('me')
        if (!me) {
            throw 'Cannot locate "me"'
        }
        if (me.isLink) {
            me = me.dirname.join(me.linkTarget)
        }
        mebin = me.dirname.portable

        /*
            TODO - remove this code when ejscript 3 is released
            The testme.mod is not needed for ejscript 3
         */
        needTestMeMod = true
        let package = topDir.join('package.json')
        if (package.exists) {
            let p = package.readJSON()
            if (p.name.startsWith('ejs.') ||
               (p.name == 'ejscript' && p.version[0] >= '3') ||
               (p.pak && p.pak.testejs == '3')) {
                ejsVersion = Cmd.run('makeme-ejs -V').trim()
                if (ejsVersion[0] < '3') {
                    throw 'Cannot run unit tests with old ejs: ' + Cmd.locate('ejs')
                }
                needTestMeMod = false
            }
        }
        let ejsVer = Cmd.run('makeme-ejs -V').trim()
        if (needTestMeMod && ejsVer[0] >= '3') {
            throw 'This product requires makeme-ejs 2.x to run unit tests. Ejs ' + Cmd.locate('makeme-ejs')
        }

        blend(topEnv, {
            TM_TOP: topDir, 
            TM_TOP_TEST: topTest, 
            //  DEPRECATE
            TM_CFG: cfg, 
            TM_OUT: cfg, 
            TM_BIN: bin, 
            TM_DEPTH: depth, 
            TM_MEBIN: mebin, 
        })
        if (options.debug) {
            topEnv.TM_DEBUG = true
        }
        let sep = App.SearchSeparator
        App.putenv('PATH', bin + sep + me.dirname + sep + App.getenv('PATH'))
        App.log.debug(2, "PATH=" + App.getenv('PATH'))
    }

    /*
        Main test runner
     */
    function runAllTests(): Void {
        trace('Test', 'Starting tests. Test depth: ' + depth)
        setupEnv()
        runDirTests('.', topEnv)
    }

    function runDirTests(dir: Path, parentEnv) {
        skipTest = false
        let env = parentEnv.clone()
        for each (file in dir.files('*.set')) {
            runTest('Setup', file, env)
        }
        try {
            if (skipTest) {
                /* Skip whole directory if skip used in *.set */
                skipTest = false
            } else {
                if (!dir.exists) {
                    log.error('Cannot read directory: ' + dir)
                }
                for each (file in dir.files('*.com')) {
                    runTest('Setup', file, env)
                    if (done) break
                }
                for each (file in dir.files('*')) {
                    if (filters.length > 0) {
                        let found
                        for each (let filter: Path in filters) {
                            if (file.isDir && filter.startsWith(file)) {
                                found = true
                                break
                            }
                            if (file.startsWith(filter)) {
                                found = true
                                break
                            }
                        }
                        if (!found) {
                            continue
                        }
                    }
                    if (file.isDir) {
                        runDirTests(file, env)
                    } else if (file.extension == 'tst') {
                        runTest('Test', file, env)
                    }
                    if (done) break
                }
            }
        } catch (e) {
            /* Exception in finally without this catch */
            if (!keepGoing) {
                done = true
            }
        }
        finally {
            for each (file in dir.files('*.set')) {
                runTest('Finalize', file, env)
            }
        }
    }

    function runTest(phase, file: Path, env) {
        if (!file.exists) {
            return
        }
        blend(env, {
            TM_PHASE: phase,
            TM_DIR: file.dirname,
        })
        let home = App.dir
        let cont = true
        try {
            App.chdir(file.dirname)
            runTestFile(phase, file, file.basename, env)
        } catch (e) {
            failedCount++
            if (!keepGoing) {
                done = true
            }
            throw e
        } finally {
            App.chdir(home)
        }
    }

    /*
        Run a test file with changed directory. topPath is the file from the test top.
     */
    function runTestFile(phase, topPath: Path, file: Path, env) {
        vtrace('Testing', topPath)
        if (phase == 'Test') {
            this.testCount++
        }
        let command = file
        let trimmed = file.trimExt()

        if (options.clean || options.clobber) {
            clean(topPath, file)
            return true
        }
        try {
            command = buildTest(phase, topPath, file, env)
        } catch (e) {
            trace('FAIL', topPath + ' cannot build ' + topPath + '\n\n' + e.message)
            this.failedCount++
            return false
        }
        if (file.extension == 'tst' && trimmed.extension == 'c') {
            if (options.projects) {
                buildProject(phase, topPath, file, env)
            }
            if (options.ide && Config.OS == 'macosx') {
                let proj = Path('testme').join(file.basename.trimExt().trimExt() + '-macosx-debug.xcodeproj')
                if (!proj.exists && !options.projects) {
                    buildProject(phase, topPath, file, env)
                }
                strace('Run', '/usr/bin/open ' + proj)
                Cmd.run('/usr/bin/open ' + proj)
                return false
            }
            if (options.projects) {
                return true
            }
        }
        if (options.compile) {
            if (file.extension != 'set') {
                /* Must continue processing setup files */
                trace('Build', file)
                return true
            }
        }
        let prior = this.failedCount
        if (command) {
            try {
                App.log.debug(5, serialize(env))
                this.startTest = new Date
                let cmd = new Cmd
                cmd.env = env
                strace('Run', command)
                cmd.start(command, blend({detach: true}, options))
                cmd.finalize()
                cmd.wait(TIMEOUT)
                if (cmd.status != 0) {
                    trace('FAIL', topPath + ' with bad exit status ' + cmd.status)
                    if (cmd.error) {
                        trace('Stderr', '\n' + cmd.error)
                    }
                    if (cmd.response) {
                        trace('Stdout', '\n' + cmd.response)
                    }
                    this.failedCount++
                } else {
                    let output = cmd.readString()
                    parseOutput(phase, topPath, file, output, env)
                    if (cmd.error) {
                        trace('Stderr', '\n' + cmd.error)
                    }
                }
            } catch (e) {
                trace('FAIL', topPath + ' ' + e)
                this.failedCount++
            }
        } else {
            trace('FAIL', topPath + ' is not a valid test file')
            this.failedCount++
        }
        if (prior == this.failedCount) {
            if (phase == 'Test') {
                trace('Pass', topPath)
            } else if (!options.verbose && !skipTest) {
                trace(phase, topPath)
            }
        } else if (!keepGoing) {
            done = true
        }
    }

    function parseOutput(phase, topPath, file, output, env) {
        let success
        let lines = output.split('\n')
        for each (line in lines) {
            let tokens = line.trim().split(/ +/)
            let kind = tokens[0]
            let rest = tokens.slice(1).join(' ')

            switch (kind) {
            case 'debug':
                trace('Debug', rest)
                break

            case 'fail':
                success = false
                this.failedCount++
                trace('FAIL', topPath + ' ' + rest)
                break

            case 'pass':
                if (success == null) {
                    success = true
                }
                this.passedCount++
                break

            case 'info':
                if (success) {
                    vtrace('Info', rest)
                } else {
                    trace('Info', rest)
                }
                break

            case 'set':
                let parts = rest.split(' ')
                let key = parts[0]
                let value = parts.slice(1).join(' ')
                env[key] = value
                vtrace('Set', key, value)
                break

            case 'skip':
                success = true
                skippedCount++
                skipTest = true
                if (true || options.verbose || options.why) {
                    if (file.extension == 'set') {
                        if (phase == 'Setup') {
                            trace('Skip', 'Directory "' + topPath.dirname + '", ' + rest)
                        }
                    } else {
                        trace('Skip', topPath + ', ' + rest)
                    }
                }
                break

            case 'verbose':
                vtrace('Info', rest)
                break

            case 'write':
                trace('Info', rest)
                break

            case '':
                break

            default:
                /* 
                    Now just ignore unexpected output

                    success = false
                    this.failedCount++
                    trace('FAIL', 'Unexpected output from ' + topPath + ': ' + kind + ' ' + rest)
                 */
                print(line)
            }
        }
        if (success == null) {
            /* Assume test passed if no result. Allows normal programs to be unit tests */
            this.passedCount++
        }
    }

    function createMakeMe(file: Path, env) {
        let name = file.trimExt().trimExt()
        let tm = Path('testme')
        if (!tm.exists) {
            tm.makeDir()
            tm.join('.GENERATED').write()
        }
        let mefile = tm.join(name).joinExt('me')
        if (!mefile.exists) {
            let libraries = env.libraries ? env.libraries.split(/ /) : []
            libraries = serialize(libraries).replace(/"/g, "'")
            let linker = '[]'
            if (Config.OS != 'windows') {
                linker = "[ '-Wl,-rpath," + bin + "']"
            }
            let instructions = `
Me.load({
    defaults: {
        '+defines': [ 'BIN="` + bin + `"' ],
        '+includes': [ '` + cfg.join('inc').portable + `', '` + App.exeDir.parent.join('inc').portable + `' ],
        '+libpaths': [ '` + bin + `' ],
        '+libraries': ` + libraries + `,
        '+linker': ` + linker + `,
    },
    targets: {
        "` + name + `": {
            type: 'exe',
            sources: [ '` + name + `.c' ],
        }
    }
})
`
            Path(mefile).write(instructions)
        }
    }

    function clean(topPath, file) {
        let tm = Path('testme')
        let ext = file.trimExt().extension
        let name = file.trimExt().trimExt()
        let mefile = tm.join(name).joinExt('me')
        let c = tm.join(name).joinExt('c')
        let exe = tm.join(name)
        let generated = tm.join('.GENERATED').exists
        if (Config.OS == 'windows') {
            exe = exe.joinExt('.exe')
        }
        let base = topPath.dirname
        for each (f in tm.files(['*.o', '*.obj', '*.lib', '*.pdb', '*.exe', '*.mk', '*.sh', '*.mod', '*.me'])) {
            trace('Remove', base.join(f))
            f.remove()
        }
        for each (f in tm.files([name + '-*.xcodeproj'])) {
            trace('Remove', base.join(f))
            f.removeAll()
        }
        if (exe && exe.exists) {
            exe.remove()
            trace('Remove', base.join(exe))
        }
        if (c.exists) {
            c.remove()
            trace('Remove', base.join(c))
        }
        if (options.clobber) {
            if (mefile.exists) {
                mefile.remove()
                trace('Remove', base.join(mefile))
            }
            if (generated) {
                tm.join('.GENERATED').remove()
                if (tm.remove()) {
                    trace('Remove', base.join(tm))
                }
            }
        }
    }

    /*
        Build the test and return the command to run
        For *.es, return a command with 'ejs' prepended
        For *.c, create a testme directory with *.me file
        For *.es.com, use 'ejsc' to precompile.

        Commands run from the directory containing the test.
     */
    function buildTest(phase, topPath: Path, file: Path, env): String? {
        let tm = Path('testme')
        let ext = file.trimExt().extension
        let name = file.trimExt().trimExt()
        let mefile = tm.join(name).joinExt('me')
        let c = tm.join(name).joinExt('c')
        let exe, command, ejs, ejsc

        if (needTestMeMod) {
            ejs = mebin.join('makeme-ejs')
            ejsc = mebin.join('makeme-ejsc')
        } else {
            ejs = Cmd.locate('ejs')
            ejsc = ejs.dirname.join('ejsc')
        }
        if (ext == 'es' || ext == 'js') {
            if (file.extension == 'com') {
                let mod = Path(name).joinExt('mod', true)
                if (needTestMeMod) {
                    command = ejsc + ' --search "testme' + App.SearchSeparator + mebin + '" --out ' + mod + ' ' + file
                } else {
                    command = ejsc + ' --out ' + mod + ' ' + file
                }
                if (options.rebuild || !mod.exists || mod.modified < file.modified) {
                    if (options.rebuild) {
                        why('Rebuild', mod + ' because --rebuild')
                    } else {
                        why('Rebuild', mod + ' because ' + file + ' is newer')
                    }
                } else {
                    why('Target', mod + ' is up to date')
                }
            } else {
                let switches = ''
                if (options.log) {
                    switches += '--log ' + options.log
                }
                if (options.trace) {
                    switches += ' --trace ' + options.trace
                }
                if (needTestMeMod) {
                    command = ejs + ' --require ejs.testme ' + switches + ' ' + file
                } else {
                    command = ejs + switches + ' ' + file
                }
            }
        } else if (ext == 'c') {
            exe = tm.join(name)
            if (Config.OS == 'windows') {
                exe = exe.joinExt('.exe')
            }
            command = exe
            if (!tm.exists) {
                tm.makeDir()
                tm.join('.GENERATED').write()
            }
            createMakeMe(file, env)
            if (options.rebuild) {
                why('Copy', 'Update ' + c + ' because --rebuild')
                file.copy(c)
            } else if (!c.exists || c.modified < file.modified) {
                why('Copy', 'Update ' + c + ' because ' + file + ' is newer')
                file.copy(c)
            }
            if (options.rebuild || !exe.exists || exe.modified < c.modified) {
                let show = options.show ? ' -s ' : ' '
                if (options.rebuild) {
                    why('Rebuild', exe + ' because --rebuild')
                } else {
                    why('Rebuild', exe + ' because ' + c + ' is newer')
                }
                strace('Build', 'me --chdir testme --file ' + mefile.basename + show)
                let ropt = {error: true}
                let result = Cmd.run('me --chdir testme --file ' + mefile.basename + show, ropt)
                if (ropt.error !== true) {
                    log.write(ropt.error)
                }
                if (options.show) {
                    log.write(result)
                }
            } else {
                why('Target', exe + ' is up to date')
            }
        }
        return command
    }

    function buildProject(phase, topPath, file: Path, env) {
        createMakeMe(file, env)
        let tm = Path('testme')
        let name = file.trimExt().trimExt()
        let mefile = tm.join(name).joinExt('me')
        trace('Generate', 'Projects ' + mefile.dirname.join(name))
        try {
            run('me --chdir ' + mefile.dirname + ' --file ' + mefile.basename + ' --name ' + name + ' generate')
        } catch (e) {
            trace('FAIL', topPath + ' cannot generate project for ' + topPath + '\n\n' + e.message)
        }
    }

    function summary() {
        if (!options.projects) {
            if (testCount == 0 && filters.length > 0) {
                trace('Missing', 'No tests match supplied filter: ' + filters.join(' '))
            }
            trace('Summary', ((failedCount == 0) ? 'PASSED' : 'FAILED') + ': ' + 
                failedCount + ' tests(s) failed, ' + 
                testCount + ' tests passed, ' + 
                skippedCount + ' tests(s) skipped. ' + 
                'Elapsed time ' + ('%.2f' % ((Date.now() - start) / 1000)) + ' secs.')
        }
    }

    function exit() {
        App.exit(failedCount > 0 ? 1 : 0)
    }

    function parseMeConfig(path: Path) {
        let data = Path(path).readString()
        let str = data.match(/ME_.*/g)
        for each (item in str) {
            let [key, value] = item.split(' ')
            key = key.replace(/ME_COM_/, '')
            key = key.replace(/ME_/, '').toUpperCase()
            if (value == '1' || value == '0') {
                value = value cast Number
            }
            topEnv['ME_' + key] = value
        }
        str = data.match(/export.*/g)
        env = {}
        for each (item in str) {
            if (!item.contains('=')) {
                continue
            }
            let [key, value] = item.split(':=')
            key = key.replace(/export /, '')
            env[key] = value
        }
    }

    function searchUp(path: Path): Path? {
        if (path.exists && !path.isDir) {
            return path
        }
        path = Path(path).relative
        dir = Path('..')
        while (true) {
            up = Path(dir.relative).join(path)
            if (up.exists && !up.isDir) {
                return up
            }
            if (dir.parent == dir) {
                break
            }
            dir = dir.parent
        }
        return null
    }

    function trace(tag: String, ...args): Void {
        if (!options.quiet) {
            log.activity(tag, ...args)
        }
    }

    function strace(tag: String, ...args): Void {
        if (options.show && !options.quiet) {
            log.activity(tag, ...args)
        }
    }

    function vtrace(tag: String, ...args): Void {
        if (verbosity > 0) {
            log.activity(tag, ...args)
        }
    }

    function why(tag: String, ...args): Void {
        if (options.why) {
            log.activity(tag, ...args)
        }
    }

    function run(cmd) {
        strace('Run', cmd)
        return Cmd.run(cmd)
    }
}

/*
    Main program
 */
var tm: TestMe = new TestMe

try {
    tm.parseArgs()
    tm.runAllTests()
} catch (e) { 
    App.log.error(e)
    tm.failedCount++
    tm.summary()
    tm.exit()
}
tm.summary()
tm.exit()

} /* module ejs.testme */

/*
    @copy   default

    Copyright (c) Embedthis Software. All Rights Reserved.

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
