#
#   bit-solaris-static.sh -- Build It Shell Script to build Embedthis Bit
#

PRODUCT="bit"
VERSION="0.8.0"
BUILD_NUMBER="0"
PROFILE="static"
ARCH="x86"
ARCH="`uname -m | sed 's/i.86/x86/;s/x86_64/x64/;s/arm.*/arm/;s/mips.*/mips/'`"
OS="solaris"
CONFIG="${OS}-${ARCH}-${PROFILE}"
CC="/usr/bin/gcc"
LD="/usr/bin/ld"
CFLAGS="-fPIC  -w"
DFLAGS="-D_REENTRANT -DPIC -DBIT_DEBUG"
IFLAGS="-I${CONFIG}/inc"
LDFLAGS="-g"
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

${CC} -c -o ${CONFIG}/obj/mprLib.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprLib.c

/usr/bin/ar -cr ${CONFIG}/bin/libmpr.a ${CONFIG}/obj/mprLib.o

rm -rf ${CONFIG}/inc/est.h
cp -r src/deps/est/est.h ${CONFIG}/inc/est.h

${CC} -c -o ${CONFIG}/obj/mprSsl.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/mprSsl.c

/usr/bin/ar -cr ${CONFIG}/bin/libmprssl.a ${CONFIG}/obj/mprSsl.o

${CC} -c -o ${CONFIG}/obj/makerom.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/mpr/makerom.c

${CC} -o ${CONFIG}/bin/makerom ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/makerom.o -lmpr ${LIBS} -lmpr -llxnet -lrt -lsocket -lpthread -lm -ldl ${LDFLAGS}

rm -rf ${CONFIG}/inc/pcre.h
cp -r src/deps/pcre/pcre.h ${CONFIG}/inc/pcre.h

${CC} -c -o ${CONFIG}/obj/pcre.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/pcre/pcre.c

/usr/bin/ar -cr ${CONFIG}/bin/libpcre.a ${CONFIG}/obj/pcre.o

rm -rf ${CONFIG}/inc/sqlite3.h
cp -r src/deps/sqlite/sqlite3.h ${CONFIG}/inc/sqlite3.h

${CC} -c -o ${CONFIG}/obj/sqlite3.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite3.c

/usr/bin/ar -cr ${CONFIG}/bin/libsqlite3.a ${CONFIG}/obj/sqlite3.o

${CC} -c -o ${CONFIG}/obj/sqlite.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/sqlite/sqlite.c

${CC} -o ${CONFIG}/bin/sqlite ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/sqlite.o -lsqlite3 ${LIBS} -lsqlite3 -llxnet -lrt -lsocket -lpthread -lm -ldl ${LDFLAGS}

rm -rf ${CONFIG}/inc/http.h
cp -r src/deps/http/http.h ${CONFIG}/inc/http.h

${CC} -c -o ${CONFIG}/obj/httpLib.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/httpLib.c

/usr/bin/ar -cr ${CONFIG}/bin/libhttp.a ${CONFIG}/obj/httpLib.o

${CC} -c -o ${CONFIG}/obj/http.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/http/http.c

${CC} -o ${CONFIG}/bin/http ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/http.o -lhttp ${LIBS} -lpcre -lmpr -lhttp -llxnet -lrt -lsocket -lpthread -lm -ldl -lpcre -lmpr ${LDFLAGS}

rm -rf ${CONFIG}/bin/http-ca.crt
cp -r src/deps/http/http-ca.crt ${CONFIG}/bin/http-ca.crt

rm -rf ${CONFIG}/inc/ejs.slots.h
cp -r src/deps/ejs/ejs.slots.h ${CONFIG}/inc/ejs.slots.h

rm -rf ${CONFIG}/inc/ejs.h
cp -r src/deps/ejs/ejs.h ${CONFIG}/inc/ejs.h

rm -rf ${CONFIG}/inc/ejsByteGoto.h
cp -r src/deps/ejs/ejsByteGoto.h ${CONFIG}/inc/ejsByteGoto.h

${CC} -c -o ${CONFIG}/obj/ejsLib.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsLib.c

/usr/bin/ar -cr ${CONFIG}/bin/libejs.a ${CONFIG}/obj/ejsLib.o

${CC} -c -o ${CONFIG}/obj/ejs.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejs.c

${CC} -o ${CONFIG}/bin/ejs ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejs.o -lejs ${LIBS} -lsqlite3 -lmpr -lpcre -lhttp -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lsqlite3 -lmpr -lpcre -lhttp ${LDFLAGS}

${CC} -c -o ${CONFIG}/obj/ejsc.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/deps/ejs/ejsc.c

${CC} -o ${CONFIG}/bin/ejsc ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/ejsc.o -lejs ${LIBS} -lsqlite3 -lmpr -lpcre -lhttp -lejs -llxnet -lrt -lsocket -lpthread -lm -ldl -lsqlite3 -lmpr -lpcre -lhttp ${LDFLAGS}

cd src/deps/ejs >/dev/null ;\
../../../${CONFIG}/bin/ejsc --out ../../../${CONFIG}/bin/ejs.mod --optimize 9 --bind --require null ejs.es ;\
cd - >/dev/null 

rm -rf ${CONFIG}/bin/bit.es
cp -r src/bit.es ${CONFIG}/bin/bit.es

rm -fr ./${CONFIG}/bin/bits ;\
cp -r bits ./${CONFIG}/bin 

${CC} -c -o ${CONFIG}/obj/bit.o -fPIC ${LDFLAGS} ${DFLAGS} -I${CONFIG}/inc src/bit.c

${CC} -o ${CONFIG}/bin/bit ${LDFLAGS} ${LIBPATHS} ${CONFIG}/obj/bit.o ${CONFIG}/obj/mprLib.o ${CONFIG}/obj/pcre.o ${CONFIG}/obj/httpLib.o ${CONFIG}/obj/sqlite3.o ${CONFIG}/obj/ejsLib.o ${LIBS} -llxnet -lrt -lsocket -lpthread -lm -ldl ${LDFLAGS}

#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
#  Omit build script undefined
