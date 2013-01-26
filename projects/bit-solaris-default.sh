#
#   bit-solaris-default.sh -- Build It Shell Script to build Embedthis Bit
#

PRODUCT="bit"
VERSION="0.8.0"
BUILD_NUMBER="0"
PROFILE="default"
ARCH="x86"
ARCH="`uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/'`"
OS="solaris"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="/usr/bin/gcc"
LD="/usr/bin/ld"
CFLAGS="-fPIC -Os -w"
DFLAGS="-D_REENTRANT -DPIC"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS=""
LIBPATHS="-L${CONFIG}/bin"
LIBS="-llxnet -lrt -lsocket -lpthread -lm -ldl"

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

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mprLib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libmpr.so ${LIBPATHS} ${CONFIG}/obj/mprLib.o ${LIBS}

rm -rf ${CONFIG}/inc/est.h
cp -r src/deps/est/est.h ${CONFIG}/inc/est.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/mprSsl.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libmprssl.so ${LIBPATHS} ${CONFIG}/obj/mprSsl.o -lmpr ${LIBS}

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/pcre.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libpcre.so ${LIBPATHS} ${CONFIG}/obj/pcre.o ${LIBS}

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC -Os ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libsqlite3.so ${LIBPATHS} ${CONFIG}/obj/sqlite3.o ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/sqlite.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/sqlite ${LIBPATHS} ${CONFIG}/obj/sqlite.o -lsqlite3 ${LIBS} -lsqlite3 -llxnet -lrt -lsocket -lpthread -lm -ldl 

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/httpLib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libhttp.so ${LIBPATHS} ${CONFIG}/obj/httpLib.o -lpcre -lmpr ${LIBS}

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/http.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/http ${LIBPATHS} ${CONFIG}/obj/http.o -lhttp ${LIBS} -lpcre -lmpr -lhttp -llxnet -lrt -lsocket -lpthread -lm -ldl -lpcre -lmpr 

rm -rf ${CONFIG}/bin/http-ca.crt
cp -r src/deps/http/http-ca.crt ${CONFIG}/bin/http-ca.crt

rm -rf ${CONFIG}/inc/ejs.slots.h
cp -r src/deps/ejs/ejs.slots.h ${CONFIG}/inc/ejs.slots.h

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/deps/ejs/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejsByteGoto.h
cp -r src/deps/ejs/ejsByteGoto.h ${CONFIG}/inc/ejsByteGoto.h

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsLib.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsLib.c

${LDFLAGS}${LDFLAGS}${CC} -shared -o ${CONFIG}/bin/libejs.so ${LIBPATHS} ${CONFIG}/obj/ejsLib.o -lsqlite3 -lmpr -lpcre -lhttp ${LIBS} -lpcre -lmpr

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejs.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejs.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejs ${LIBPATHS} ${CONFIG}/obj/ejs.o -lejs ${LIBS} -lsqlite3 -lmpr -lpcre -lhttp -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lsqlite3 -lmpr -lpcre -lhttp 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/ejsc.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsc.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/ejsc ${LIBPATHS} ${CONFIG}/obj/ejsc.o -lejs ${LIBS} -lsqlite3 -lmpr -lpcre -lhttp -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lsqlite3 -lmpr -lpcre -lhttp 

cd src/deps/ejs >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.mod --optimize 9 --bind --require null ejs.es ;\
cd - >/dev/null 

rm -rf ${CONFIG}/bin/bit.es
cp -r src/bit.es ${CONFIG}/bin/bit.es

cd . >/dev/null ;\
rm -fr ./${CONFIG}/bin/bits ;\
cp -r bits ./${CONFIG}/bin ;\
cd - >/dev/null 

${LDFLAGS}${LDFLAGS}${CC} -c -o ${CONFIG}/obj/bit.o ${CFLAGS} ${DFLAGS} -I${CONFIG}/inc src/bit.c

${LDFLAGS}${LDFLAGS}${CC} -o ${CONFIG}/bin/bit ${LIBPATHS} ${CONFIG}/obj/bit.o ${CONFIG}/obj/mprLib.o ${CONFIG}/obj/pcre.o ${CONFIG}/obj/httpLib.o ${CONFIG}/obj/sqlite3.o ${CONFIG}/obj/ejsLib.o ${LIBS} -llxnet -lrt -lsocket -lpthread -lm -ldl 

#  Omit build script undefined
