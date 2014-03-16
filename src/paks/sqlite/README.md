Embedthis SQLite Library
===

This is custom build of the SQLite SQL engine.

Licensing
---

This software is dual-licensed under a GPLv2 license and commercial licenses are offered by Embedthis Software.
See LICENSE.md for details.

### Prerequisites: 
    Bit (http://embedthis.com/downloads/bit/download.ejs) for Bit to configure and build.
    Ejscript (http://ejscript.org/downloads/ejs/download.ejs) for utest to test.

### To Build:

    ./configure
    bit

    Alternatively to build without Bit:

    make 

Images are built into */bin. The build configuration is saved in */inc/bit.h.

### To Test:

    bit test 

### To Run:

    bit run

This will run appweb in the src/server directory using the src/server/appweb.conf configuration file.

### To Install: 

    bit install

### To Create Packages:

    bit package

### Copyright

Copyright (c) 2003-2014 Embedthis Software, LLC. All Rights Reserved.
Embedthis and MPR are trademarks of Embedthis Software, LLC. Other 
brands and their products are trademarks of their respective holders.
