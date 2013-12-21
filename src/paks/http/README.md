Embedthis Http Library
===

Licensing
---
The Http library is dual-licensed under a GPLv2 license and commercial licenses are offered by Embedthis Software.
See http://embedthis.com/licensing/index.html for licensing details.

### To Read Documentation:

  See doc/index.html

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

Resources
---
  - [MPR GitHub repository](http://github.com/embedthis/mpr-4)
  - [Embedthis web site](http://embedthis.com/)
  - [Bit GitHub repository](http://github.com/embedthis/bit)

