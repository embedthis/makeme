Embedthis SQLite Library
===

This is custom build of the SQLite SQL engine.

Licensing
---

This software is dual-licensed under a GPLv2 license and commercial licenses are offered by Embedthis Software.
See LICENSE.md for details.

### Prerequisites: 
    MakeMe (http://embedthis.com/downloads/makeme/download.esp) for MakeMe to configure and build.
    Ejscript (http://ejscript.org/downloads/ejs/download.esp) for utest to test.

### To Build:

    ./configure
    me

    Alternatively to build without MakeMe:

    make 

Images are built into */bin. The build configuration is saved in */inc/me.h.

### To Test:

    me test 

### To Run:

    me run

This will run appweb in the src/server directory using the src/server/appweb.conf configuration file.

### To Install: 

    me install

### To Create Packages:

    me package

### Copyright

Copyright (c) 2003-2014 Embedthis Software, LLC. All Rights Reserved.
Embedthis and MPR are trademarks of Embedthis Software, LLC. Other 
brands and their products are trademarks of their respective holders.
