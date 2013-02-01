Embedthis Bit(TM)

Welcome to the Embedthis Bit distribution. Bit stands for Build It and is a replacement for the autoconf/make build
tools. Bit is a single tool that configures, builds and generated native build projects for applications.
It expresses build rules in the JavaScript language.

This document describes the installation procedure for the binary Bit package. This package contains pre-built language
tools including an interpreter, compiler, vm, documentation generator and assembler listing program as well as the
development libraries and headers needed to embed Bit in an application. 

You can read the documentation for the latest release online at:

    http://embedthis.com/products/bit/doc/index.html

This software is copyrighted and distributed under license. Please read the LICENSE.md for details.


Table of Contents

* System Requirements
* Installation Package Formats
* Development Environment Platform Support
* Installating the Tar Self-Install Image
* Removing Bit
* Apple MAC OS X  Release Details
* Windows Release Details
* Running Bit
* Building the Source Code
* License and Copyright Information

System Requirements

Operating System Support

    * Windows XP, Vista, Windows-7 (x86,x64)
    * Mac OS X 10.4, 10.5 (ppc,x86,x64)
    * Linux 2.4, 2.6 (x86,x64,arm,ppc,sparc,xscale,mips)
    * VxWorks (x86,arm,ppc,sparc,xscale,mips)
    * Solaris (sparc,x86,x64)

To install Bit, your system will need at least the following:

    * 50 MB Disk
    * 1 GB RAM

To deploy Bit in an embedded environment, you will need at least the following:

    * 2 MB Disk / ROM space
    * 4 MB RAM

To rebuild Bit from source code, you will need at least:

    * 80 MB disk space


Installation Package Formats

The Bit binary installation is published in two formats:

    * Native Apple MAC OS X PKG format
    * Native Windows installer format

For other platforms, you will need to download the source code distribution and build and install. The product documentation describes how to build from source. 

See below for operating system specific installation information.


Development Environment Platform Support

If you wish to embed Bit in your application or build Bit from source code, you will need to use a C compiler and associated development tools.  Several development environments are supported.  You may choose any of the following:

    * Linux/Unix/Solaris GNU Tools

    * Windows Visual Studio 2010

    * WindRiver Tornado Development Tools


Apple Mac OS X Release Details

To install the native MAC OS X PKG image:

   1. Download the PKG image.

   2. Double click on the PKG file image.
 
   3. Answer the questions as prompted.


Windows Release Details

To install the Windows Installer image:

   1. Login with administrator privileges. This is necessary as the binary 
      installation needs to install a Windows Service.

   2. Use WinZip or equivalent software to extract the installation image. This
      will extract the README.TXT and setup.exe from the zip archive.

   3. Double click on the setup.exe image to run it.


Removing Bit on Mac OS X

This is the procedure to remove Bit on all systems except Windows.

   1. Change directory to the installation directory for Bit. This is usually /usr/lib/bit.

   2. Run the uninstall script in the install directory for Bit by typing 

      sudo ./uninstall


Removing Bit on Windows

If you installed the Bit Windows Installer image, follow these steps to 
remove Bit. If you installed the Tar self-install image, you will need 
to remove Bit manually. 

   1. Login with administrator privileges

   2. Run the Add / Remove Programs applet from the control panel.

   3. Select the Embedthis Bit and choose remove.


Running Bit

The Bit binary is invoked by by typing:

    bit

Running Bit on Windows

On windows, the installation creates a Programs shortcut for the "bit" binary.  Clicking on it will run "ejs" in
a command window.  To run "ejs" from a command window manually, you may need to add the Bit installation bin directory to
your path. This is usually the C:/Program Files/Embedthis Bit/bin directory. You can do this by running the System applet from the Control Panel and selecting the Advanced tab and then the Environment Variables button at the bottom of the page. Add this directory to the setting for the PATH use variable.


Building from the Source Code

If you wish to rebuild the Bit product from the distributed source code, please download the source code distribution from
the Embedthis web site. Then read "Building from Source" in the Bit product documentation for an explanation of the necessary steps to successfully rebuild the product.


License and Copyright Information

This software is supplied under license. Both a GNU and commercial license are available. Please read LICENSE.md for details. This documentation and the software described in this document may be used and copied only in accordance with the terms of the accompanying license agreement. Product and technical information in this document is subject to change without notice and does not represent a commitment on the part of Embedthis Software LLC.

Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

Embedthis is a trademark of Embedthis Software LLC. Embedthis, Appweb and Embedthis Bit are trademarks of Embedthis Software. All other brand or product names are the trademarks or registered trademarks of their respective holders.


Embedthis Software, LLC.
4616 25th Ave NE, #733
Seattle, WA 98105.
+1 (425) 329-3490
embedthis.com
info@embedthis.com
