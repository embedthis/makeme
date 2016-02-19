/*
    Script.es -- Embedthis MakeMe Script Environment

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me.script {

require ejs.unix

var sysdirs = {
    '/Applications': true,
    '/Library': true,
    '/Network': true,
    '/System': true,
    '/Program Files': true,
    '/Program Files (x86)': true,
    '/Users': true,
    '/bin': true,
    '/dev': true,
    '/etc': true,
    '/home': true,
    '/opt': true,
    '/opt/bin': true,
    '/sbin': true,
    '/tmp': true,
    '/usr': true,
    '/usr/bin': true,
    '/usr/include': true,
    '/usr/lib': true,
    '/usr/sbin': true,
    '/usr/local': true,
    '/usr/local/bin': true,
    '/usr/local/etc': true,
    '/usr/local/include': true,
    '/usr/local/lib': true,
    '/usr/local/man': true,
    '/usr/local/opt': true,
    '/usr/local/share': true,
    '/usr/local/src': true,
    '/usr/local/x': true,
    '/var': true,
    '/var/cache': true,
    '/var/lib': true,
    '/var/log': true,
    '/var/run': true,
    '/var/spool': true,
    '/var/tmp': true,
    '/': true,
}

/*  KEEP
    Common system directories for safeRemove
    Property white-list for Path.operate
var operate = {
    active: true,
    compress: true,
    contents: true,
    depthFirst: true,
    dir: true,
    directories: true,
    dot: true,
    exclude: true,
    expand: true,
    extension: true,
    filter: true,
    flatten: true,
    footer: true,
    group: true,
    header: true,
    hidden: true,
    include: true,
    include: true,
    keep: true,
    missing: true,
    noneg: true,
    operation: true,
    patch: true,
    perform: true,
    permissions: true,
    post: true,
    pre: true,
    relative: true,
    relative: true,
    rename: true,
    separator: true,
    strip: true,
    symlink: true,
    trim: true,
    user: true,
    verbose: true,
}
*/

public function builtin(cmd: String, actionOptions: Object = {}) {
    switch (cmd) {
    case 'cleanTargets':
        let from = App.dir
        try {
            App.chdir(me.dir.top)
            for each (target in me.targets) {
                if (target.enable && target.path && !target.precious && target.generate !== false &&
                        Builder.TargetsToClean[target.type]) {
                    let path: Path = target.modify || target.path 
                    path = (makeme.generating) ? reppath(path) : path
                    if (sysdirs[path]) {
                        App.log.error("prevent removal of", path)
                        continue
                    }
                    if (!path.childOf(me.dir.bld) && !makeme.generating) {
                        App.log.error("prevent removal of", path)
                        continue
                    }
                    if (path.exists) {
                        trace('Clean', path.relativeTo(me.dir.top))
                    }
                    if (path.toString().endsWith('/') || path.isDir) {
                        removeDir(path)
                    } else {
                        removeFile(path)
                    }
                    if (me.platform.os == 'windows') {
                        let ext = path.extension
                        if (ext == me.ext.shobj || ext == me.ext.exe) {
                            removeFile(path.replaceExt('lib'))
                            removeFile(path.replaceExt('pdb'))
                            removeFile(path.replaceExt('exp'))
                        }
                    }
                }
            }
        } finally {
            App.chdir(from)
        }
        break
    }
}

public function changeDir(path: Path) {
    try {
        App.chdir(path)
    } catch (e) {
        throw new Error("Cannot change directory to " + path)
    }
}

/**
    Copy a file.
    Copy files to a destination.
    The routine uses $copy() to implement the copying.
    This either copies files or if generating, emits code to copy files.
    @param src Source file
    @param dest Destination
    @param options Options to pass to Me.copy(). These include user, group, uid, gid and  permissions.
*/
public function copyFile(src: Path, dest: Path, options = {}) {
    if (!makeme.generating) {
        strace('Copy', 'cp ' + src.portable + ' ' + dest.portable)
        if (src.same(dest)) {
            throw new Error('Cannot copy file. Source is the same as destination: ' + src)
        }
        if (!options.dry) {
            src.copy(dest)
        }
        if ((options.user || options.group || options.uid || options.gid) && App.uid == 0) {
            dest.setAttributes(options)
        } else if (options.permissions) {
            dest.setAttributes({permissions: options.permissions})
        }
    } else {
        let pwd = App.dir
        if (src.startsWith(pwd)) {
            src = src.relativeTo(me.dir.top)
        }
        if (dest.startsWith(me.dir.top)) {
            dest = dest.relativeTo(pwd)
        }
        if (src == dest) {
            throw new Error('Cannot copy file. Source is the same as destination: ' + src)
        }
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            src = src.windows
            if (src.contains(' ')) {
                src = '"' + src + '"'
            }
            dest = dest.windows
            if (dest.contains(' ')) {
                dest = '"' + dest + '"'
            }
            /* Append empty to ensure modified time is updated */
            genCmd('copy /Y /B ' + src + '+modified ' + dest + ' $(LOG)')
        } else {
            if (src.contains(' ')) {
                src = '"' + src + '"'
            }
            if (dest.contains(' ')) {
                dest = '"' + dest + '"'
            }
            genCmd('cp ' + src + ' ' + dest)
            if (options.uid || options.gid) {
                genCmd('[ `id -u` = 0 ] && chown ' + options.uid + ':' + options.gid + ' "' + dest + '"; true')
            } else if (options.user || options.group) {
                genCmd('[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + dest + '"; true')
            }
            if (options.permissions) {
                genCmd('chmod ' + "%0o".format([options.permissions]) + ' "' + dest + '"')
            }
        }
    }
}

public function copy(from, to: Path, options = {}) {
    print("WARNING: Using deprecated copy()", from, to)
    return copyFiles(from, to, options)
}

/*
    Wrapper for Path.operate 
 */
public function copyFiles(from, to: Path, topOptions = {}, base = null) {
    base ||= topOptions.home || me.dir.top
    base = Path(base)
    let options = blend({
        verbose: makeme.options.verbose,
        contents: true,
        directories: true,
        expand: function (str, o) {
            let eo = {missing: true}
            return makeme.loader.expand(str, eo).expand(o, eo)
        }
        /* If generating, keep the pattern */
        missing: makeme.generating ? '' : undefined,

        postPerform: function (from, to, control) {
            if (control.filelist && !to.isDir) {
                control.filelist.push(to)
            }
            if (control.fold) {
                strace('Fold', to)
                foldLines(to)
            }
            if (!control.verbose) {
                if (makeme.options.show) {
                    strace('Copy', 'cp ' + from + ' ' + to)
                }
            }
        }
    }, topOptions, {overwrite: false, functions: true})

    if (makeme.generating) {
        /*
            If generating, provide a perform callback to capture the commands
         */
        options.perform = function(from, to, control) {
            let path = to.dirname
            /*
                Suppress redundant directory operations
             */
            topOptions.made ||= {}
            if (topOptions.made && !topOptions.made[path]) {
                topOptions.made[path] = true
                makeDirectory(path)
            }
          
            /*
                The 'from' path is relative to the control.base path. Must convert the 'from' path to be relative to 
                the 'src' directory. Note: file targets do not change directory to target.home like scripts do.
             */
            if (from.isDir) {
                from = from.relativeTo(me.dir.top)
                if (topOptions.made[from]) {
                    topOptions.made[from] = true
                    makeDirectory(from, control)
                }
            } else {
                from = from.relativeTo(me.dir.top)
                copyFile(from, to, control)
            }
            if (control.symlink && me.platform.like == 'unix') {
                linkFile(to, Path(makeme.loader.expand(control.symlink)).join(to.basename), 
                    blend({symlink: true}, control))
            }
            return true
        }
    }
    if (App.uid != 0) {
        delete options.user
        delete options.group
        delete options.uid
        delete options.gid
    }
    return base.operate(from, to, options)
}

/*
    Fold long lines at column 80. On windows, will also convert line terminatations to <CR><LF>.
 */
function foldLines(path: Path) {
    let lines = path.readLines()
    let out = new TextStream(new File(path, 'wt'))
    for (l = 0; l < lines.length; l++) {
        let line = lines[l]
        if (line.length > 80) {
            for (i = 79; i >= 0; i--) {
                if (line[i] == ' ') {
                    lines[l] = line.slice(0, i)
                    lines.insert(l + 1, line.slice(i + 1))
                    break
                }
            }
            if (i == 0) {
                lines[l] = line.slice(0, 80)
                lines.insert(l + 1, line.slice(80))
            }
        }
        out.writeLine(lines[l])
    }
    out.close()
}

/**
    Link a file.
    This creates a symbolic link on systems that support symlinks.
    The routine uses $Path.link() to implement the linking.
    This either links files or if generating, emits code to link files.
    @param src Source file
    @param dest Destination
    @param options See $copy() for supported options.
*/
public function linkFile(src: Path, dest: Path, options = {}) {
    makeDirectory(dest.parent, options.symlink ? {} : options)
    if (!makeme.generating) {
        if (!options.dry) {
            strace('Remove', 'rm -f', dest)
            dest.remove()
            strace('Link', 'ln -s', src, dest)
            src.link(dest)
        }
    } else if (makeme.generating != 'nmake' && makeme.generating != 'vs') {
        genCmd('rm -f "' + dest + '"')
        genCmd('ln -s "' + src + '" "' + dest + '"')
    }
}

/**
    Make a directory
    This creates a directory and all required parents.
    This either makes a directory or if generating, emits code to make directories.
    @param path Directory path to make
    @param options See $copy() for supported options.
*/
public function makeDirectory(path: Path, options = {}) {
    if (!makeme.generating) {
        if (!options.dry) {
            if (!path.isDir) {
                try {
                    strace('Create', 'mkdir ' + path)
                    if (!path.makeDir()) {
                        throw "Cannot make directory" + path
                    }
                } catch (e) {
                    print(e)
                    print("CANNOT MAKE DIR", path)
                    throw "Cannot make directory" + path
                }
            }
            if ((options.user || options.group || options.uid || options.gid) && App.uid == 0) {
                path.setAttributes(options)
            } else if (options.permissions) {
                path.setAttributes({permissions: options.permissions})
            }
        }
    } else {
        /* Generating */
        let pwd = App.dir
        if (path.startsWith(me.dir.top)) {
            path = path.relativeTo(pwd)
        }
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            if (path.name.endsWith('/')) {
                genCmd('if not exist "' + path.windows + '\\" md "' + path.windows + '\\"')
            } else {
                genCmd('if not exist "' + path.windows + '" md "' + path.windows + '"')
            }
        } else {
            genCmd('mkdir -p "' + path + '"')
            if (options.permissions) {
                genCmd('chmod ' + "%0o".format([options.permissions]) + ' "' + path + '"')
            }
            if (options.user || options.group) {
                genCmd('[ `id -u` = 0 ] && chown ' + options.user + ':' + options.group + ' "' + path + '"; true')
            }
        }
    }
}

//  LEGACY
public function makeDir(path: Path, options = {}) {
    makeDirectory(path, options)
}

/**
    Remove a file.
    This either removes files or if generating, emits code to remove files.
    @param path File to remove
    @param options Control options
*/
public function removeFile(path: Path, options = {}) {
    if (!makeme.generating) {
        strace('Remove', 'rm -f', path)
        if (!options.dry) {
            if (!path.remove()) {
                throw "Cannot remove " + path
            }
        }
    } else {
        let pwd = App.dir
        if (path.startsWith(pwd)) {
            path = path.relative
        }
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            genCmd('if exist "' + path.windows + '" del /Q "' + path.windows + '"')
        } else {
            genCmd('rm -f "' + path + '"')
        }
    }
}

/**
    Remove a directory.
    This removes a file or directory and all its contents include subdirectories. Use the 'empty' option to only remove
    empty directories.
    This either removes directories or if generating, emits code to remove directories.
    @param path Directory to remove
    @param options Control options
    @option empty Remove the directory only if empty.
*/
public function removeDir(path: Path, options = {}) {
    if (!makeme.generating) {
        strace('Remove', path)
        if (!options.dry) {
            if (options.empty) {
                strace('Remove', 'rmdir', path)
                path.remove()
            } else {
                strace('Remove', 'rm -fr', path)
                path.removeAll()
            }
        }
    } else {
        let pwd = App.dir
        if (path.startsWith(pwd)) {
            path = path.relative
        }
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            if (options.empty) {
                genCmd('if exist "' + path.windows + '" rd /Q "' + path.windows + '"')
            } else {
                genCmd('if exist "' + path.windows + '" rd /Q /S "' + path.windows + '"')
            }
        } else {
            if (options.empty) {
                genCmd('rmdir -p "' + path + '" 2>/dev/null ; true')
            } else {
                genCmd('rm -fr "' + path + '"')
            }
        }
    }
}

/**
    Remove a file or directory.
    This removes a file or directory and all its contents including subdirectories.
    @param path File or directory to remove
*/
public function removePath(path: Path) {
    if (!makeme.generating) {
        strace('Remove', path)
        if (!options.dry) {
            strace('Remove', 'rm -fr', path)
            path.removeAll()
        }
    } else {
        let pwd = App.dir
        if (path.startsWith(pwd)) {
            path = path.relative
        }
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            genCmd('if exist "' + path.windows + '\\" rd /Q /S "' + path.windows + '"')
            genCmd('if exist "' + path.windows + '" del /Q "' + path.windows + '"')
        } else {
            genCmd('rm -fr "' + path + '"')
        }
    }
}


/**
    Run a command and trace output if copt.show or options.show
    @param command Command to run. May be an array of args or a string.
    @param copt Options. These are also passed to $Cmd.
    @option dir Change to given directory to run the command.
    @option filter Do not display output to the console if it contains the specified filter pattern.
        Set to true to filter (not display) any output. Note the command always returns the command output
        as the function result.
    @option generate Generate in projects. Defaults to true.
    @option noshow Do not show the command line before executing. Useful to override me --show for one command.
    @option nostop Continue processing even if this command is not successful.
    @option show Show the command line before executing. Similar to me --show, but operates on just this command.
    @option timeout Timeout for the command to complete

    Note: do not use the Cmd options: noio, detach. Use Cmd APIs directly.
 */
public function run(command, copt = {}): String
    makeme.builder.run(command, copt)

/**
    @hide
 */
public function sh(command, copt = {}): String
    makeme.builder.sh(command, copt)

/**
    Safely copy a file. This protects against overwriting the target unless the "overwrite" command line
    option was specified. Copies are traced to the console.
 */
public function safeCopy(from: Path, to: Path) {
    let p: Path = new Path(to)
    if (to.exists && !makeme.options.overwrite) {
        if (!from.isDir) {
            traceFile('Exists', to)
        }
        return
    }
    if (!to.exists) {
        traceFile('Create', to)
    } else {
        traceFile('Overwrite', to)
    }
    if (!to.dirname.isDir) {
        mkdir(to.dirname, 0755)
    }
    cp(from, to)
}

/**
    Safely remove a directory. This protects against removing some major system directories.
 */
public function safeRemove(path: Path) {
    if (sysdirs[path]) {
        App.log.error("Prevent removal of", path)
        return
    }
    if (path.isDir) {
        path.removeAll()
    } else {
        path.remove()
    }
}

/**
    Emit "show" trace
    This is trace that is displayed if me --show is invoked.
    @param tag Informational tag emitted before the message
    @param args Message args to display
*/
public function strace(tag, ...args)
    makeme.strace(tag, ...args)

/**
    Touch a directory and update its last modified time
    @param path Directory path to modify
 */
function touchDir(path: Path) {
    if (path.isDir) {
        let touch = path.join('.touch')
        touch.remove()
        touch.write()
        touch.remove()
    }
}

/**
    Touch a file and update its last modified time
    @param path File path to modify
 */
public function touchFile(path: Path) {
    if (!makeme.generating) {
        if (!options.dry) {
            path.append('')
            strace('Touch', path)
        }
    } else {
        if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
            genCmd('copy /Y /B nul+modified ' + path.windows + ' $(LOG)')
        } else {
            genCmd('touch "' + path + '"')
        }
    }
}

/**
    Emit general trace
    @param tag Informational tag emitted before the message
    @param args Message args to display
 */
public function trace(tag: String, ...args): Void
    makeme.trace(tag, ...args)

/**
    Emit trace for a path
    @param msg Message to display
    @param path Filename to to trace
 */
public function traceFile(msg, path): Void
    makeme.traceFile(msg, path)

/**
    Emit verbose trace
    @param tag Informational tag emitted before the message
    @param args Message args to display
 */
public function vtrace(tag, ...args)
    makeme.vtrace(tag, ...args)


var capture: Array?
var genout: TextStream

public function genStartCapture(target)
    capture = []

public function genStopCapture(target): String {
    let result = ''
    if (capture.length > 0) {
        if (target.message) {
            let [,tag, msg] = target.message.match(/([^:]*) *: *(.*)/)
            let message = repvar(makeme.loader.expand(msg))
            if (makeme.generating == 'nmake') {
                message = '\t@echo ' + '.'.times(9 - tag.length) + ' [' + tag + '] ' + message
            } else {
                message = "echo '%12s %s'" % (["[" + tag + "]"] + [message])
            }
            if (makeme.generating == 'nmake') {
                message = message.replace(/\//g, '\\')
            }
            capture.insert(0, message)
        }
        if (makeme.generating == 'nmake') {
            result = capture.join('\n\t')
        } else {
            result = capture.join(' ; \\\n\t')
        }
    }
    capture = null
    return result
}

public function genClose()
    genout.close()

public function genCmd(s) {
    if (me.target) {
        s = repvar2(s, me.target.home)
    } else {
        s = repvar2(s, me.dir.top)
    }
    if (capture) {
        capture.push(s)
    } else {
        /* Coming here for builtins like clean: */
        genout.writeLine('\t' + s)
    }
}

public function genOpen(path: Path)
    genout = TextStream(File(path, 'w'))

public function genPathTrace(tag: String, path) {
    genTrace(tag, makeme.generating == 'nmake' ? path.windows : path.portable)
}

public function genRun(s) {
    if (me.target) {
        s = repvar2(s, me.target.home)
    } else {
        s = repvar2(s, me.dir.top)
    }
    if (makeme.generating == 'nmake' || makeme.generating == 'vs') {
        if (s[0] != '"') {
            let parts = s.split(' ')
            s = parts[0].replace(/\//g, '\\') + ' ' + parts.slice(1).join(' ')
        }
    }
    if (capture) {
        capture.push(s)
    } else {
        /* Coming here for builtins like clean: */
        genout.writeLine('\t' + s)
    }
}

public function genScript(str: String) {
    capture.push(str)
}

/** 
    Generate a trace line.
    @param tag Informational tag emitted before the message
    @param args Message args to display
 */
public function genTrace(tag: String, ...args): Void {
    let msg = args.join(" ")
    if (makeme.generating == 'nmake') {
        msg = '\t@echo ' + '.'.times(9 - tag.length) + ' [' + tag + '] ' + msg + '\n'
    } else {
        msg = "\t@echo '%12s %s'" % (["[" + tag + "]"] + [msg]) + "\n"
    }
    /* Do not use repvar - messes up windows paths */
    genout.write(repvar(msg))
}

public function genWrite(...args) {
    for each (arg in args) {
        genout.write(repvar(arg))
    }
}

public function genWriteLine(...args) {
    for each (arg in args) {
        genout.write(repvar(arg))
    }
    genout.writeLine()
}

public function genRawWriteLine(...args) {
    for each (arg in args) {
        genout.write(arg)
    }
    genout.writeLine()
}

internal function rep(s: String, pattern, replacement): String {
    if (pattern) {
        s = s.replace(pattern, replacement)
    }
    return s
}

internal function repset(s: String, patterns) {
    for each (pattern in patterns) {
        let [from, to] = pattern
        if (from) {
            s = s.replace(from, to)
        }
    }
    return s
}

internal function repCmd(s: String, pattern, replacement): String {
    if (s.startsWith(pattern)) {
        return s.replace(pattern, replacement)
    }
    if (s.startsWith('"' + pattern + '"')) {
        return s.replace(pattern, replacement)
    }
    return s
}

/**
    Replace default defines, includes, libraries etc with token equivalents. This allows
    Makefiles and script to be use variables to control various flag settings.
    @hide
 */
public function repcmd(command: String): String {
    //  generator.settings == gen
    let mappings = makeme.generate.mappings
    let generating = makeme.generating
    let minimalCflags = makeme.generate.minimalCflags
    if (generating == 'make' || generating == 'nmake') {
        if (mappings.linker != '') {
            /* Linker has -g which is also in minimal C flags */
            command = rep(command, mappings.linker, '$(LDFLAGS)')
        }
        if (mappings.defines != '') {
            command = rep(command, mappings.defines, '$(DFLAGS)')
        } else {
            command = rep(command, ' -c ', ' -c $(DFLAGS) ')
        }
        if (mappings.compiler != '') {
            command = rep(command, mappings.compiler, '$(CFLAGS)')
        } else {
            command = rep(command, ' -c ', ' -c $(CFLAGS) ')
        }
        for each (word in minimalCflags) {
            command = rep(command, word + ' ', ' ')
        }
        command = rep(command, mappings.libpaths, '$(LIBPATHS)')
        command = rep(command, mappings.includes, '$(IFLAGS)')
        command = rep(command, '"$(IFLAGS)"', '$(IFLAGS)')
        /* Twice because libraries are repeated and replace only changes the first occurrence */
        command = rep(command, mappings.libraries, '$(LIBS)')
        command = rep(command, mappings.libraries, '$(LIBS)')
        command = rep(command, RegExp(mappings.build, 'g'), '$$(BUILD)')
        command = rep(command, RegExp(mappings.configuration, 'g'), '$$(CONFIG)')
        if (me.targets.compiler) {
            command = repCmd(command, me.targets.compiler.path, '$(CC)')
        }
        if (me.targets.link) {
            command = repCmd(command, me.targets.link.path, '$(LD)')
        }
        if (me.targets.rc) {
            command = repCmd(command, me.targets.rc.path, '$(RC)')
        }

    } else if (generating == 'sh') {
        if (mappings.linker != '') {
            command = rep(command, mappings.linker, '${LDFLAGS}')
        }
        for each (word in minimalCflags) {
            command = rep(command, word + ' ', ' ')
        }
        if (mappings.defines != '') {
            command = rep(command, mappings.defines, '${DFLAGS}')
        } else {
            command = rep(command, ' -c ', ' -c ${DFLAGS} ')
        }
        if (mappings.compiler != '') {
            command = rep(command, mappings.compiler, '${CFLAGS}')
        } else {
            command = rep(command, ' -c ', ' -c ${CFLAGS} ')
        }
        if (mappings.linker != '') {
            command = rep(command, mappings.linker, '${LDFLAGS}')
        }
        command = rep(command, mappings.libpaths, '${LIBPATHS}')
        command = rep(command, mappings.includes, '${IFLAGS}')
        /* Twice because libraries are repeated and replace only changes the first occurrence */
        command = rep(command, mappings.libraries, '${LIBS}')
        command = rep(command, mappings.libraries, '${LIBS}')
        command = rep(command, RegExp(mappings.build, 'g'), '$${BUILD}')
        command = rep(command, RegExp(mappings.configuration, 'g'), '$${CONFIG}')
        if (me.targets.compiler) {
            command = repCmd(command, me.targets.compiler.path, '${CC}')
        }
        if (me.targets.link) {
            command = repCmd(command, me.targets.link.path, '${LD}')
        }
        for each (word in minimalCflags) {
            command = rep(command, word + ' ', ' ')
        }
    }
    if (generating == 'nmake') {
        command = rep(command, '_DllMainCRTStartup@12', '$(ENTRY)')
    }
    command = rep(command, RegExp(me.dir.top + '/', 'g'), '')
    command = rep(command, /  */g, ' ')
    if (generating == 'nmake') {
        command = rep(command, /\//g, '\\')
    }
    return command
}

/*
    Replace with variables where possible.
    Replaces the top directory and the CONFIGURATION
 */
public function repvar(command: String): String {
    let generating = makeme.generating
    let mappings = makeme.generate.mappings
    command = command.replace(RegExp(me.dir.top + '/', 'g'), '')
    if (generating == 'make') {
        command = command.replace(RegExp(mappings.build, 'g'), '$$(BUILD)')
        command = command.replace(RegExp(mappings.configuration, 'g'), '$$(CONFIG)')
    } else if (generating == 'nmake') {
        command = command.replace(RegExp(mappings.build, 'g'), '$$(BUILD)')
        command = command.replace(RegExp(mappings.configuration, 'g'), '$$(CONFIG)')
    } else if (generating == 'sh') {
        command = command.replace(RegExp(mappings.configuration, 'g'), '$${CONFIG}')
    }
    for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
        if (me.prefixes[p]) {
            if (me.platform.like == 'windows') {
                let pat = me.prefixes[p].windows.replace(/\\/g, '\\\\')
                command = command.replace(RegExp(pat, 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
            }
            command = command.replace(RegExp(me.prefixes[p], 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
        }
    }
    command = command.replace(/\/\//g, '$$(ME_ROOT_PREFIX)/')
    return command
}

public function repvar2(command: String, home: Path? = null): String {
    let generating = makeme.generating
    let mappings = makeme.generate.mappings
    if (home) {
        command = command.replace(RegExp(me.dir.top, 'g'), me.dir.top.relativeTo(home))
    }
    if (home && me.platform.like == 'windows' && generating == 'nmake') {
        let re = RegExp(me.dir.top.windows.name.replace(/\\/g, '\\\\'), 'g')
        command = command.replace(re, me.dir.top.relativeTo(home).windows)
    }
    if (generating == 'make') {
        command = command.replace(RegExp(mappings.build, 'g'), '$$(BUILD)')
        command = command.replace(RegExp(mappings.configuration, 'g'), '$$(CONFIG)')
    } else if (generating == 'nmake') {
        command = command.replace(RegExp(mappings.configuration + '\\\\bin/', 'g'), '$$(CONFIG)\\bin\\')
        command = command.replace(RegExp(mappings.build, 'g'), '$$(BUILD)')
        command = command.replace(RegExp(mappings.configuration, 'g'), '$$(CONFIG)')
    } else if (generating == 'sh') {
        command = command.replace(RegExp(mappings.configuration, 'g'), '$${CONFIG}')
    }
    for each (p in ['vapp', 'app', 'bin', 'inc', 'lib', 'man', 'base', 'web', 'cache', 'spool', 'log', 'etc']) {
        if (mappings[p]) {
            if (me.platform.like == 'windows') {
                let pat = mappings[p].windows.replace(/\\/g, '\\\\')
                command = command.replace(RegExp(pat, 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
            }
            command = command.replace(RegExp(mappings[p], 'g'), '$$(ME_' + p.toUpper() + '_PREFIX)')
        }
    }
    command = command.replace(/\/\//g, '$$(ME_ROOT_PREFIX)/')
    return command
}

public function reppath(path: Path): String {
    path = path.relative
    if (me.platform.like == 'windows') {
        path = (makeme.generating == 'nmake') ? path.windows : path.portable
    } else if (Config.OS == 'windows' && makeme.generating && makeme.generating != 'nmake')  {
        path = path.portable 
    }
    return repvar(path)
}

} /* embedthis.me module */

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


