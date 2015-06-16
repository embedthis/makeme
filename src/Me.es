/*
    Me.es -- Embedthis MakeMe Me class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

/**
    The Me class is the MakeMe Document Object Model (DOM).
    This class defined core properties and exposes no methods. Instances are dynamic and may create new
    properties at runtime. 
    @stability Prototype
 */
dynamic enumerable public class Me {
    use default namespace ''

    /** Array of files to load. These paths may include the wild-card "*" character to 
        match a filename portion and "**" to match any file or directory at any depth in the directory 
        tree. If the filename is not found, "~/.paks" is searched for a matching Pak.
     */
    var blend: Array?

    /**
        The configure collection specifies the configurable targets used to extend the project.
        The 'discovers' property
        The 'discovers' property lists optional configurable targets to discover when configuring.
        The 'requires' property lists required targets to discover when configuring.
        The 'extras' property lists components that will not be discovered but can be included via an explicit
            'configure --with NAME'.
     */
    var configure: Object = { }

    /** Optional MakeMe files to load after fully loading all blended MakeMe files */
    var customize: Array?
    
    /** Default properties to be inherited by all targets */
    var defaults: Object?

    /** Directories used by MakeMe. These must be absolute paths so that they can be used when the 
        current directory is changed while processing. */
    var dir: Object = { top: Path() }

    /** Environment variables to set when compiling and linking */
    var env: Object = {}

    /** File extensions indexed by file type */
    var ext: Object = {}

    /** Global values to use in property token expansion */
    var globals: Object = {}

    /** Default properties to be inherited by only the targets in the same MakeMe file */
    var internal: Object

    /** Literal script code to mixin. The script is injected into the global scope.  */
    var mixin: Array

    /** List of scripts or compiled scripts to pre-load */
    var modules: Array

    /** Reference to the MakeMe command line options object */
    var options: Object

    /** Description of the current platform. Has properties for operating system and CPU architecture. */
    var platform: Object = {}

    /** Installation directory prefixes and default values on Unix/Linux systems */
    var prefixes: Object = {}

    /** Build profiles. Standard profiles are: debug and release */
    var profiles: Object = {}

    /** Command rules for compiling, building libraries and executables */
    var rules: Object = {}

    /** Top-level scripts to execute */
    var scripts: Object = {}

    /** Project top-level settings. This includes name, version, description. See settings collection below for 
        full details. */
    var settings: Object = {}

    /** Reference to the currently building target */
    var target: Target?

    /** Targets to build. Each target is its own collection of properties */
    var targets: Object = {}

    /** Additional usage message to emit for MakeMe command line errors */
    var usage: Object = {}

    /** File manifest for use by me-package 
        @hide */
    var manifest: Object

    /** 
        Me constructor
        @hide
     */
    function Me() {
        /*
            Do bare minimum initialization. The loader does the initialization
         */
        global.me = this
        options = makeme.options
        dir.me = App.exeDir
        dir.work = Path('.').absolute
        dir.top = Path(options.configure || App.dir).absolute
        let path = me.dir.top.join(Loader.PACKAGE)
        if (path.exists) {
            try {
                package = path.readJSON()
                if (package.directories && package.directories.paks) {
                    dir.paks = Path(package.directories.paks).absolute
                }
            } catch (e) {
                trace('Warn', 'Cannot parse: ' + path + '\n' + e)
            }
        }
        dir.paks ||= dir.top.join('paks')
        makeme.directories = dir
    }

    /**
        Load a MakeMe literal object.
        Called by MakeMe files via "Me.load()" to load the MakeMe literal definition.
        This simply saves a reference to the object that is processed later.
     */
    public static function load(obj: Object) {
        Loader.loading(obj)
    }

} /* me class */

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
