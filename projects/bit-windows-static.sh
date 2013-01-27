#
#   bit-windows-static.sh -- Build It Shell Script to build Embedthis Bit
#

export PATH="$(SDK)/Bin:$(VS)/VC/Bin:$(VS)/Common7/IDE:$(VS)/Common7/Tools:$(VS)/SDK/v3.5/bin:$(VS)/VC/VCPackages;$(PATH)"
export INCLUDE="$(INCLUDE);$(SDK)/Include:$(VS)/VC/INCLUDE"
export LIB="$(LIB);$(SDK)/Lib:$(VS)/VC/lib"

PRODUCT="bit"
VERSION="0.8.0"
BUILD_NUMBER="0"
PROFILE="static"
ARCH="x86"
ARCH="`uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/'`"
OS="windows"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="cl.exe"
LD="link.exe"
CFLAGS="-nologo -GR- -W3 -Zi -Od -MDd -w"
DFLAGS="-D_REENTRANT -D_MT -DBIT_DEBUG"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS="-nologo -nodefaultlib -incremental:no -debug -machine:x86"
LIBPATHS="-libpath:${CONFIG}/bin"
LIBS="ws2_32.lib advapi32.lib user32.lib kernel32.lib oldnames.lib msvcrt.lib shell32.lib"

[ ! -x ${CONFIG}/inc ] && mkdir -p ${CONFIG}/inc ${CONFIG}/obj ${CONFIG}/lib ${CONFIG}/bin

[ ! -f ${CONFIG}/inc/bit.h ] && cp projects/bit-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h
[ ! -f ${CONFIG}/inc/bitos.h ] && cp ${SRC}/src/bitos.h ${CONFIG}/inc/bitos.h
if ! diff ${CONFIG}/inc/bit.h projects/bit-${OS}-${PROFILE}-bit.h >/dev/null ; then
	cp projects/bit-${OS}-${PROFILE}-bit.h ${CONFIG}/inc/bit.h
fi

rm -rf ${CONFIG}/bin/ca.crt
cp -r src/deps/est/ca.crt ${CONFIG}/bin/ca.crt

rm -rf ${CONFIG}/inc/bitos.h
cp -r src/bitos.h ${CONFIG}/inc/bitos.h

rm -rf ${CONFIG}/inc/mpr.h
cp -r src/deps/mpr/mpr.h ${CONFIG}/inc/mpr.h

"${CC}" -c -Fo${CONFIG}/obj/mprLib.obj -Fd${CONFIG}/obj/mprLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

"lib.exe" -nologo -out:${CONFIG}/bin/libmpr.lib ${CONFIG}/obj/mprLib.obj

rm -rf ${CONFIG}/inc/est.h
cp -r src/deps/est/est.h ${CONFIG}/inc/est.h

"${CC}" -c -Fo${CONFIG}/obj/mprSsl.obj -Fd${CONFIG}/obj/mprSsl.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

"lib.exe" -nologo -out:${CONFIG}/bin/libmprssl.lib ${CONFIG}/obj/mprSsl.obj

"${CC}" -c -Fo${CONFIG}/obj/makerom.obj -Fd${CONFIG}/obj/makerom.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/makerom.c

"${LD}" -out:${CONFIG}/bin/makerom.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.obj libmpr.lib ${LIBS}

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

"${CC}" -c -Fo${CONFIG}/obj/pcre.obj -Fd${CONFIG}/obj/pcre.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

"lib.exe" -nologo -out:${CONFIG}/bin/libpcre.lib ${CONFIG}/obj/pcre.obj

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

"${CC}" -c -Fo${CONFIG}/obj/sqlite3.obj -Fd${CONFIG}/obj/sqlite3.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

"lib.exe" -nologo -out:${CONFIG}/bin/libsqlite3.lib ${CONFIG}/obj/sqlite3.obj

"${CC}" -c -Fo${CONFIG}/obj/sqlite.obj -Fd${CONFIG}/obj/sqlite.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

"${LD}" -out:${CONFIG}/bin/sqlite.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.obj libsqlite3.lib ${LIBS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

"${CC}" -c -Fo${CONFIG}/obj/httpLib.obj -Fd${CONFIG}/obj/httpLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

"lib.exe" -nologo -out:${CONFIG}/bin/libhttp.lib ${CONFIG}/obj/httpLib.obj

"${CC}" -c -Fo${CONFIG}/obj/http.obj -Fd${CONFIG}/obj/http.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

"${LD}" -out:${CONFIG}/bin/http.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.obj libhttp.lib ${LIBS} libpcre.lib libmpr.lib

rm -rf ${CONFIG}/bin/http-ca.crt
cp -r src/deps/http/http-ca.crt ${CONFIG}/bin/http-ca.crt

rm -rf ${CONFIG}/inc/ejs.slots.h
cp -r src/deps/ejs/ejs.slots.h ${CONFIG}/inc/ejs.slots.h

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/deps/ejs/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejsByteGoto.h
cp -r src/deps/ejs/ejsByteGoto.h ${CONFIG}/inc/ejsByteGoto.h

"${CC}" -c -Fo${CONFIG}/obj/ejsLib.obj -Fd${CONFIG}/obj/ejsLib.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsLib.c

"lib.exe" -nologo -out:${CONFIG}/bin/libejs.lib ${CONFIG}/obj/ejsLib.obj

"${CC}" -c -Fo${CONFIG}/obj/ejs.obj -Fd${CONFIG}/obj/ejs.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejs.c

"${LD}" -out:${CONFIG}/bin/ejs.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.obj libejs.lib ${LIBS} libsqlite3.lib libmpr.lib libpcre.lib libhttp.lib

"${CC}" -c -Fo${CONFIG}/obj/ejsc.obj -Fd${CONFIG}/obj/ejsc.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsc.c

"${LD}" -out:${CONFIG}/bin/ejsc.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsc.obj libejs.lib ${LIBS} libsqlite3.lib libmpr.lib libpcre.lib libhttp.lib

cd src/deps/ejs >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.mod --optimize 9 --bind --require null ejs.es ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/removeFiles.obj -Fd${CONFIG}/obj/removeFiles.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc package/windows/removeFiles.c

"${LD}" -out:${CONFIG}/bin/removeFiles.exe -entry:WinMainCRTStartup -subsystem:Windows ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/removeFiles.obj libmpr.lib ${LIBS}

rm -rf ${CONFIG}/bin/bit.es
cp -r src/bit.es ${CONFIG}/bin/bit.es

cd . >/dev/null ;\
rm -fr ./${CONFIG}/bin/bits ;\
cp -r bits ./${CONFIG}/bin ;\
cd - >/dev/null 

"${CC}" -c -Fo${CONFIG}/obj/bit.obj -Fd${CONFIG}/obj/bit.pdb ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/bit.c

"${LD}" -out:${CONFIG}/bin/bit.exe -entry:mainCRTStartup -subsystem:console ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/bit.obj ${CONFIG}/obj/mprLib.obj ${CONFIG}/obj/pcre.obj ${CONFIG}/obj/httpLib.obj ${CONFIG}/obj/sqlite3.obj ${CONFIG}/obj/ejsLib.obj ${LIBS}

#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
