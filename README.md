Embedthis MakeMe
===

Embedthis MakeMe. A modern replacement for autoconf/make.

Licensing
---

See [LICENSE.md](https://github.com/embedthis/makeme/blob/master/LICENSE.md) for details.

### Documentation:

    See https://www.embedthis.com/makeme/doc/index.html

### To build

    make boot

On windows, make.bat runs projects/windows.bat to locate the Visual Studio compiler. If you have setup
your CMD environment for Visual Studio by running the Visual Studio vsvarsall.bat, then that edition of 
Visual Studio will be used. If not, windows.bat will attempt to locate the most recent Visual Studio version. 

### To install on Linux or Mac:

    make install

On Windows, add build/*/bin to your PATH.

### Command line to build other things:

    me

Resources
---
  - [MakeMe Web Site](https://www.embedthis.com/makeme/)
  - [MakeMe GitHub repository](https://github.com/embedthis/makeme)
