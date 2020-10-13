Embedthis Ejscript Language
===

## Ejscript is archived and not maintained.

A complete, integrated JavaScript environment suitable for education purposes when studying
compilers, language parsers, virtual machines and JavaScript.

I implements most of the abandoned ES4 specification including:

* Classes
* Namespaces
* Optional typing
* Operator overloading

Ejscript has a stand-alone compiler that generates byte code modules. 

See https://www.embedthis.com/ejscript/doc for details.

Licensing
---

See LICENSE.md for details.

### To Read Documentation:

  See https://www.embedthis.com/ejscript/doc/

### To Build:

    ./configure
    make

Images are built into */bin. The build configuration is saved in */inc/me.h.

### To Test:

    make test


### To run a Script:

To run executables locally without installing, you will need to put the target bin 
directory in your PATH. This is under OS-ARCH-PROFILE/bin.

    ejs script

### To Install:

    sudo make install

Resources
---
  - [Ejscript web site](https://www.embedthis.com/ejscript/)
  - [Embedthis web site](https://www.embedthis.com/)
  - [Ejscript GitHub repository](http://github.com/embedthis/ejscript)

