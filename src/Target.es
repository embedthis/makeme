/*
    Target.es -- Embedthis MakeMe Target class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

/**
    The Target class implements MakeMe buildable targets. Targets are the basic "atoms" for building projects.
    @stability Prototype
 */
dynamic enumerable public class Target {

    use default namespace ''

    /** Compiler switchs to use when building the target */
    var compiler: Array?     /* of Strings */

    /** Defines whether a target is configurable component that can be selected/deselected via configure. */
    var configurable: Boolean?

    /** List of other enabled targets that this target conflicts with this target. Configure will select
        only one target from a set of conflicting targets. */
    var conflicts: Array?        /* of Strings */

    /** Symbolic C-preprocessor defintions to use when building the target */
    var defines: Array?          /* of Strings */

    /** Names of targets on which this target depends to function */
    var depends: Array = []      /* of Paths */

    /** Target description used for configurable components in usage messages. */
    var description: String?

    /** List of configurable components that should be discovered when configuring. */
    var discovers: Array?

    /** Configurable component error message if this target cannot be configured */
    var diagnostic: String?

    /** Whether this target is enabled for building */
    var enable: Object = true         /* Boolean | Function */

    /** Executable entry points, indexed by target type. Used when linking on windows. */
    var entry: Object?

    /** File that defined the target */
    var file: Path

    /** List of input files for this target. This may include wild cards. */
    var files: Array = []        /* of Paths */

    /** Whether to generate this target for projects */
    var generate: Object

    /** List of goal names for which this target will be built */
    var goals: Array = []        /* of Strings */

    /** List of header files required for this target. May include wildcards. */
    var headers: Array?

    /** Home directory for the target */
    var home: Path

    /** List of configurable components that are required for this target to be enabled for building.
        If a specified component is not configured, the target is not built.  */
    var ifdef: Array?

    /** List of external resources to import when configuring */
    var imports: Array?

    /** List of directories for include files */
    var includes: Array?

    /** List of libraries to link with */
    var libraries: Array?

    /** List of linker library include paths to use when linking */
    var libpaths: Array?

    /** List of link switches to use when linking */
    var linker: Array?

    /** 
        Message to emit when building. Should be of the form 'Tag: Message...'
     */
    var message: String?

    /** 
        Directories to make before building the target
     */
    var mkdir: Array?

    /**
        File that is modified by building. Used in stale() to test if a target needs building
     */
    var modify: Path

    /** Name of the target. Same as the entry in Me.targets[] */
    var name: String

    /** 
        Destination path for the target. May be null if the target does build a discrete resource.
        When loaded from a MakeMe file, it may contain '${tokens}' that are expanded on loading.
        May be a function that is run on loading to compute a path.
        May be a directory that holds a collection of resources built by the target.
        The path is used to compute if the target is stale and must be rebuilt if 'files[]' are newer 
        than 'path'.
     */
    var path: Path?

    /** List of platforms to build this target. Includes platform names and 'local' */
    var platforms: Array?

    /** Precious targets are retained and not cleaned via 'me clean' */
    var precious: Boolean?

    /** List of components the configurable target requires to be enabled */
    var requires: Array?

    /** List of resource files required for this target. May include wildcards. */
    var resources: Array?

    /** Name of the build rule. Defaults to 'exe' */
    var rule: String?

    /** 
        Hash of scripts used when building the target. The following MakeMe file properties 
        are converted into script properties on loading:
     */
    var scripts: Object = {}

    /** Target selected for building */
    var selected: Boolean

    /** Suppress configuration trace for this target */
    var silent: Boolean?

    /** List of source files to build with this target. May include wild cards. */
    var sources: Array?

    /** Use static linking for this target */
    var static: Boolean?

    /** 
        Target type: 'exe', 'file', 'group', 'header', 'lib', 'obj', 'resource', 'script'.
     */
    var type: String

    /** List of targets that can be utilized by this target if they are enabled.
        The utilized targets will be built before the target. */
    var uses: Array = []

    /**
        Why the target is not being built
     */
    var why: String?

    /* 
     ****************   Internal properties **************
     */
    /** Bare component created during configuration */
    var bare: Boolean
    /* Target has been enabled or disabled via explicit user configuration command */

    /** Component explicitly excluded or included via configure --with/--without. Set to 'with' or 'without' */
    var explicit: String

    /** List of top level MakeMe collections from which to inherit properties for this target */
    var inherit: Object?         //  String | Array

    /** Component file loaded */
    var loaded: Boolean

    /** List of libraries provided by this target */
    var ownLibraries: Array

    /** Variables for expanding build rules. 
        @hide */
    var vars: Object = {}

    /** Target has been excluded via a configure --without option 
        @hide */
    var without: Boolean

    //  TODO - probably should be Path
    /** Target was configured usinag a --with path */
    var withpath: String

    /*
        Temporary properties used only during loading
            action, build, config, internal,
            prebuild,  precompile,  preconfig,  preresolve,  presource, 
            postblend, postcompile, postconfig, postresolve, postsource,
            run, shell, test
     */

    /* Manifest - Note these are not targets but should not clash properties incase they become targets*/
    /*
        dir, from, to, linkin, set, enable, copy, write, root, user, group, permissions
        append, filter, header, footer, separator
     */

    /* 
        Path.operate options 

        var action: Boolean
        var active: Boolean
        var append: Boolean
        var contents: Boolean
        var compress: Boolean
        var depthFirst: Boolean
        var directories: Boolean
        var dir: Boolean
        var dot: String
        var extension: String
        var exclude: Object      // RegExp | String | Function
        var expand: Object
        var filter: RegExp
        var flatten: Boolean
        var footer: String
        var from: Object         //  Path | String | Object | Array
        var group: Object        //  String | Number
        var header: String
        var hidden: Boolean
        var include: Object      // RegExp | String | Function
        var keep: Boolean
        var missing: String
        var noneg: Boolean
        var operation: String
        var patch: Object
        var pre: Function
        var post: Function
        var relative: Path?
        var rename: Function
        var separator            //  String || Boolean
        var strip: Boolean
        var synmlink: Path
        var to: Path
        var trim: Number
        var user: Object         //  String | Number
        var verbose: Object      //  Boolean | Function
*/


} /* class Target */

} /* module embedthis.me */

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


