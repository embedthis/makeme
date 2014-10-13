/*
    Me.es -- Embedthis MakeMe Me class

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */
module embedthis.me {

dynamic enumerable public class Me {
    use default namespace ''

    /*
        Top level Me file properties
     */
    var blend: Array?
    var configure: Object = { }
    var customize: Array?
    var customSettings: Object = {}
    var defaults: Object?
    var dir: Object = { top: Path() }
    var env: Object = {}
    var ext: Object = {}
    var globals: Object = {}
    var internal: Object
/* UNUSED
    var localBin: Path
*/
    var modules: Array
    var mixin: Array
    var newTargets: Array = []
    var options: Object
    var package: Object
    var platform: Object = {}
    var prefixes: Object = {}
    var profiles: Object = {}
    var rules: Object = {}
    var settings: Object = {}
    var scripts: Object = {}
    var target: Target?
    var targets: Object = {}
    var usage: Object = {}

    /* Temporary 
    var optPrefixes: Object
    var debianPrefixes: Object
    var windowsPrefixes: Object
    var packageManifest: Object
    var packagePrefixes: Object
    */
    var manifest: Object

    function Me() {
        global.me = this
        options = makeme.options
        dir.home = Path(App.getenv('HOME') || App.getenv('HOMEPATH') || '.').portable.absolute
        dir.top = Path('.').absolute
        dir.src = Path(options.configure || App.dir).absolute
        dir.me = App.exeDir
        // UNUSED dir.me = dir.src.join('makeme/standard.me').exists ? dir.src.join('me') : Config.Bin.portable
    }

    /*
        Called by MakeMe files via "Me.load()" to load the MakeMe literal definition
        Simply saves a reference to the object that is processed later
     */
    public static function load(obj: Object) {
        Loader.loading(obj)
    }

} /* me class */

} /* embedthis.me module */

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
