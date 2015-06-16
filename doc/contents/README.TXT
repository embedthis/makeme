Embedthis MakeMe(TM)

Welcome to Embedthis MakeMe. MakeMe is a modern replacement for the autoconf and make make build tools. MakeMe uses Javascript to simply describe projects with smart defaults. It then builds directly, or generates makefiles, and IDE projects.

This document describes the installation procedure for the binary MakeMe package. This package contains the "me" tool and manual page. Documentation is available online at https://embedthis.com/makeme/doc/. 

You can read the documentation for the latest release online at:

    https://embedthis.com/makeme/doc/index.html

This software is copyrighted and distributed under license. Please read the LICENSE.TXT for details.


Table of Contents

* System Requirements
* Installation Package Formats
* Development Environment Platform Support
* Removing MakeMe
* Apple MAC OS X Release Details
* Windows Release Details
* Running MakeMe
* Building the Source Code
* License and Copyright Information

System Requirements

Operating System Support

    * Windows Windows-7,8 (x86,x64)
    * Mac OS X 10.6+ (x64)
    * Linux 2.6 (x86,x64,arm,ppc,xscale,mips)

To install MakeMe, your system will need at least the following:

    * 5 MB Disk
    * 512 MB RAM

To rebuild MakeMe from source code, you will need at least:

    * 100 MB disk space

Installation Package Formats

The MakeMe binary installation is published in two formats:

    * Native Apple MAC OS X PKG format
    * Native Windows installer format

For other platforms, you will need to download the source code distribution and build and install. The product documentation describes how to build from source. 

See below for operating system specific installation information.


Development Environment Platform Support

If you wish to embed MakeMe in your application or build MakeMe from source code, you will need to use a C compiler and associated development tools.  Several development environments are supported.  You may choose any of the following:

    * Linux/Unix GNU Tools

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


Removing MakeMe on Mac OS X

This is the procedure to remove MakeMe on all systems except Windows.

   1. Change directory to the installation directory for MakeMe. This is usually /usr/local/lib/makeme.

   2. Run the uninstall script in the install directory for MakeMe by typing 

      sudo ./uninstall


Removing MakeMe on Windows

If you installed the MakeMe Windows Installer image, follow these steps to 
remove MakeMe.

   1. Login with administrator privileges

   2. Run the Add / Remove Programs applet from the control panel.

   3. Select the Embedthis MakeMe and choose remove.


Running MakeMe

The MakeMe binary is invoked by by typing:

    me

Running MakeMe on Windows

When installed, "me" will be added to your system PATH. You may need to log out and then log in again to reinitialize your system PATH. 


Building from the Source Code

If you wish to rebuild the MakeMe product from the distributed source code, please download the source code
distribution from the Embedthis web site. Then read "Building from Source" in the MakeMe product documentation for an explanation of the necessary steps to successfully rebuild the product.  

License and Copyright Information

This software is supplied under license. Both a GNU and commercial license are available. Please read LICENSE.TXT for details. This documentation and the software described in this document may be used and copied only in accordance with the terms of the accompanying license agreement. Product and technical information in this document is subject to change without notice and does not represent a commitment on the part of Embedthis Software LLC.

Copyright (c) Embedthis Software. All Rights Reserved.

Embedthis is a trademark of Embedthis Software LLC. Embedthis and Embedthis MakeMe are trademarks of Embedthis Software. All other brand or product names are the trademarks or registered trademarks of their respective holders.


Embedthis Software, LLC.
4616 25th Ave NE, #733
Seattle, WA 98105.
+1 (425) 329-3490
embedthis.com
info@embedthis.com
