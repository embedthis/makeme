Perl Compatible Regular Expressions
===

Licensing
---
See LICENSE.md for details.

### To Read Documentation:

  See doc/index.html

### Prerequisites:
    Ejscript (http://www.ejscript.org/downloads/ejs/download.ejs) for the Bit and Utest tools to configure and build.

### To Build:

    ./configure
    bit

    Alternatively to build without Ejscript:

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
  - [Embedthis web site](http://embedthis.com/)
  - [Appweb web site](http://appwebserver.org/)
  - [MPR GitHub repository](http://github.com/embedthis/mpr-4)
  - [Appweb GitHub repository](http://github.com/embedthis/appweb-4)
