/*
    mprSsl.c -- Embedthis MPR SSL Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: orion
 */


/************************************************************************/
/*
    Start of file "src/ssl/matrixssl.c"
 */
/************************************************************************/

/*
    matrixssl.c -- Support for secure sockets via MatrixSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "bit.h"

#if BIT_PACK_MATRIXSSL

#define MATRIX_USE_FILE_SYSTEM

/* 
    Work-around to allow the windows 7.* SDK to be used with VS 2014 
 */
#if _MSC_VER >= 1700
    #define SAL_SUPP_H
    #define SPECSTRING_SUPP_H
#endif
/*
    Matrixssl defines int*, uint*, but does not provide HAS_XXX or any other means to disable. Ugh!
    So must include matrixsslApi.h first and then workaround. 
 */
#if WIN32
 #include   <winsock2.h>
 #include   <windows.h>
 #define    WIN32 1
#else
 #define    POSIX 1
#endif
 #include   "matrixsslApi.h"

#define     HAS_INT16 1
#define     HAS_UINT16 1
#define     HAS_INT32 1
#define     HAS_UINT32 1
#define     HAS_INT64 1
#define     HAS_UINT64 1

#include    "mpr.h"

/************************************* Defines ********************************/
/*
    Per SSL configuration structure
 */
typedef struct MatrixConfig {
    sslKeys_t       *keys;
    sslSessionId_t  *session;
} MatrixConfig;

/*
    Per socket extended state
 */
typedef struct MatrixSocket {
    MprSocket       *sock;              /* Underlying socket */
    MatrixConfig    *cfg;               /* SSL config */
    ssl_t           *ctx;               /* MatrixSSL ssl_t structure */
    char            *outbuf;            /* Pending output data */
    char            *peerName;          /* Desired peer name */
    ssize           outlen;             /* Length of outbuf */
    ssize           written;            /* Number of unencoded bytes written */
    int             more;               /* MatrixSSL stack has buffered data */
    MprBuf          *peerCert;          /* Parsed peer certificate */
} MatrixSocket;

/***************************** Forward Declarations ***************************/

static void     closeMss(MprSocket *sp, bool gracefully);
static void     disconnectMss(MprSocket *sp);
static ssize    flushMss(MprSocket *sp);
static char     *getMssState(MprSocket *sp);
static int      handshakeMss(MprSocket *sp, short cipherSuite);
static ssize    innerRead(MprSocket *sp, char *userBuf, ssize len);
static void     manageMatrixSocket(MatrixSocket *msp, int flags);
static void     manageMatrixConfig(MatrixConfig *cfg, int flags);
static ssize    processMssData(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore);
static ssize    readMss(MprSocket *sp, void *buf, ssize len);
static int      upgradeMss(MprSocket *sp, MprSsl *ssl, cchar *peerName);
static int      verifyCert(ssl_t *ssl, psX509Cert_t *cert, int32 alert);
static ssize    writeMss(MprSocket *sp, cvoid *buf, ssize len);

/************************************ Code ************************************/

PUBLIC int mprCreateMatrixSslModule()
{
    MprSocketProvider   *provider;

    if ((provider = mprAllocObj(MprSocketProvider, 0)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    provider->closeSocket = closeMss;
    provider->disconnectSocket = disconnectMss;
    provider->flushSocket = flushMss;
    provider->readSocket = readMss;
    provider->socketState = getMssState;
    provider->writeSocket = writeMss;
    provider->upgradeSocket = upgradeMss;
    mprAddSocketProvider("matrixssl", provider);

    if (matrixSslOpen() < 0) {
        return 0;
    }
    return 0;
}


static void manageMatrixConfig(MatrixConfig *cfg, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;
    } else if (flags & MPR_MANAGE_FREE) {
        if (cfg->keys) {
            matrixSslDeleteKeys(cfg->keys);
            cfg->keys = 0;
        }
        matrixSslClose();
    }
}


static void manageMatrixSocket(MatrixSocket *msp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(msp->sock);
        mprMark(msp->peerName);
        mprMark(msp->peerCert);

    } else if (flags & MPR_MANAGE_FREE) {
        if (msp->ctx) {
            matrixSslDeleteSession(msp->ctx);
        }
    }
}


/*
    Close the socket
 */
static void closeMss(MprSocket *sp, bool gracefully)
{
    MatrixSocket    *msp;
    uchar           *obuf;
    int             nbytes;

    assert(sp);

    lock(sp);
    msp = sp->sslSocket;
    assert(msp);

    if (!(sp->flags & MPR_SOCKET_EOF) && msp->ctx) {
        /*
            Flush data. Append a closure alert to any buffered output data, and try to send it.
            Don't bother retrying or blocking, we're just closing anyway.
         */
        matrixSslEncodeClosureAlert(msp->ctx);
        if ((nbytes = matrixSslGetOutdata(msp->ctx, &obuf)) > 0) {
            sp->service->standardProvider->writeSocket(sp, obuf, nbytes);
        }
    }
    sp->service->standardProvider->closeSocket(sp, gracefully);
    mprRemoveItem(sp->service->secureSockets, msp->sock);
    unlock(sp);
}


static int upgradeMss(MprSocket *sp, MprSsl *ssl, cchar *peerName)
{
    MprSocketService    *ss;
    MatrixSocket        *msp;
    MatrixConfig        *cfg;
    char                *password;
    uint32              cipherSuite;
    int                 flags;

    ss = sp->service;
    assert(ss);
    assert(sp);

    if ((msp = (MatrixSocket*) mprAllocObj(MatrixSocket, manageMatrixSocket)) == 0) {
        return MPR_ERR_MEMORY;
    }
    lock(sp);
    msp->sock = sp;
    sp->sslSocket = msp;
    sp->ssl = ssl;
    password = 0;

    mprAddItem(ss->secureSockets, sp);

    if (ssl->config) {
        msp->cfg = cfg = ssl->config;

    } else {
        if ((cfg = mprAllocObj(MatrixConfig, manageMatrixConfig)) == 0) {
            unlock(sp);
            return MPR_ERR_MEMORY;
        }
        if (matrixSslNewKeys(&cfg->keys) < 0) {
            mprError("MatrixSSL: Cannot create new MatrixSSL keys");
            unlock(sp);
            return MPR_ERR_CANT_INITIALIZE;
        }
        /*
            Read the certificate and the key file for this server.
         */
        password = NULL;
        if (matrixSslLoadRsaKeys(cfg->keys, ssl->certFile, ssl->keyFile, password, ssl->caFile) < 0) {
            mprError("MatrixSSL: Could not read or decode certificate or key file."); 
            unlock(sp);
            return MPR_ERR_CANT_READ;
        }
        msp->cfg = ssl->config = cfg;
    }
    unlock(sp);

    /* 
        Associate a new ssl session with this socket. The session represents the state of the ssl protocol 
        over this socket. Session caching is handled automatically by this api.
     */
    if (sp->flags & MPR_SOCKET_SERVER) {
        flags = (ssl->verifyPeer) ? SSL_FLAGS_CLIENT_AUTH : 0;
        if (matrixSslNewServerSession(&msp->ctx, cfg->keys, NULL, flags) < 0) {
            mprError("MatrixSSL: Cannot create new server session");
            unlock(sp);
            return MPR_ERR_CANT_CREATE;
        }
        matrixSslSetCertValidator(msp->ctx, (sslCertCb_t)verifyCert);

    } else {
        msp->peerName = sclone(peerName);
        cipherSuite = 0;
        if (matrixSslNewClientSession(&msp->ctx, cfg->keys, NULL, cipherSuite, verifyCert, NULL, NULL, 0) < 0) {
            mprError("MatrixSSL: Cannot create new client session");
            unlock(sp);
            return MPR_ERR_CANT_CONNECT;
        }
        if (handshakeMss(sp, 0) < 0) {
            unlock(sp);
            return MPR_ERR_CANT_CONNECT;
        }
    }
    unlock(sp);
    return 0;
}


/*
    Store the name in printable form into buf; no more than (end - buf) characters will be written
 */
static void parseCert(MprBuf *buf, char *prefix, psX509Cert_t *cert)
{
    mprPutToBuf(buf, "%s_S_CN=%s,", prefix, cert->subject.commonName);
    mprPutToBuf(buf, "%s_S_C=%s,", prefix, cert->subject.country);
    mprPutToBuf(buf, "%s_S_L=%s,", prefix, cert->subject.locality);
    mprPutToBuf(buf, "%s_S_ST=%s,", prefix, cert->subject.state);
    mprPutToBuf(buf, "%s_S_O=%s,", prefix, cert->subject.organization);
    mprPutToBuf(buf, "%s_S_OU=%s,", prefix, cert->subject.orgUnit);

    mprPutToBuf(buf, "%s_I_CN=%s,", prefix, cert->issuer.commonName);
    mprPutToBuf(buf, "%s_I_C=%s,", prefix, cert->issuer.country);
    mprPutToBuf(buf, "%s_I_L=%s,", prefix, cert->issuer.locality);
    mprPutToBuf(buf, "%s_I_ST=%s,", prefix, cert->issuer.state);
    mprPutToBuf(buf, "%s_I_O=%s,", prefix, cert->issuer.organization);
    mprPutToBuf(buf, "%s_I_OU=%s,", prefix, cert->issuer.orgUnit);
}


/*
    Get the SSL state with key=values for both peer and own certs
 */
static char *getMssState(MprSocket *sp)
{
    MatrixSocket    *msp;
    MprBuf          *buf;
    ssl_t           *ctx;
    cchar           *cipherName;

    msp = sp->sslSocket;
    ctx = msp->ctx;
    cipherName = mprGetSslCipherName(msp->ctx->cipher->ident);
    buf = mprCreateBuf(0, 0);
    mprPutToBuf(buf, "PROVIDER=matrixssl,CIPHER=%s,", cipherName ? cipherName: "unknown");
    mprPutBlockToBuf(buf, mprGetBufStart(msp->peerCert), mprGetBufLength(msp->peerCert));
    parseCert(buf, sp->acceptIp ? "CLIENT" : "SERVER", ctx->keys->cert);
    mprLog(5, "MatrixSSL certs: %s", mprGetBufStart(buf));
    return mprGetBufStart(buf);
}


/*
    Validate certificates
 */
static int verifyCert(ssl_t *ssl, psX509Cert_t *cert, int32 alert)
{
    MprSocketService    *ss;
    MprSocket           *sp;
    MatrixSocket        *msp;
    struct tm           t;
    cchar               *cipherName;
    char                *c;
    int                 next, y, m, d, cipherCode;

    ss = MPR->socketService;

    /*
        Find our handle. This is really ugly because the matrix api does not provide a handle.
     */
    lock(ss);
    sp = 0;
    for (ITERATE_ITEMS(ss->secureSockets, sp, next)) {
        if (sp->ssl && ((MatrixSocket*) sp->sslSocket)->ctx == ssl) {
            break;
        }
    }
    unlock(ss);
    msp = sp->sslSocket;
    cipherCode = msp->ctx->cipher->ident;
    cipherName = mprGetSslCipherName(cipherCode);
    if (cipherName) {
        mprLog(3, "MatrixSSL connected using cipher: %s, %x", cipherName, cipherCode);
    } else {
        mprLog(3, "MatrixSSL connected using cipher: %x", cipherCode);
    }
    if (!sp) {
        /* Should not get here */
        assert(sp);
        return SSL_ALLOW_ANON_CONNECTION;
    }
    if (!sp->ssl->verifyPeer) {
        return SSL_ALLOW_ANON_CONNECTION;
    }
    if (alert > 0) {
        if (alert == SSL_ALERT_CERTIFICATE_REVOKED) {
            sp->errorMsg = sclone("Certificate revoked");
        } else if (alert == SSL_ALERT_UNKNOWN_CA) {
            if (memcmp(cert->issuer.hash, cert->subject.hash, SHA1_HASH_SIZE) == 0) {
                sp->errorMsg = sclone("Self-signed certificate");
            } else {
                sp->errorMsg = sclone("Certificate not trusted");
            }
            if (!sp->ssl->verifyIssuer) {
                //  ssl->sec.anon = 1;
                return SSL_ALLOW_ANON_CONNECTION;
            }
        } else {
            sp->errorMsg = sfmt("Cannot handshake: error %d", alert);
        }
        return alert;
    }
    if (msp->peerName && !smatch(msp->peerName, cert->subject.commonName)) {
        sp->errorMsg = sclone("Certificate Common name mismatch");
        return PS_FAILURE;
    }
    /* 
        Validate the 'not before' date 
     */
    mprDecodeUniversalTime(&t, mprGetTime());
    if ((c = cert->notBefore) != NULL) {
        if (strlen(c) < 8) {
            sp->errorMsg = sclone("Corrupt certificate");
            return PS_FAILURE;
        }
        /* 
            UTCTIME, defined in 1982, has just a 2 digit year 
         */
        if (cert->notBeforeTimeType == ASN_UTCTIME) {
            y =  2000 + 10 * (c[0] - '0') + (c[1] - '0'); 
            c += 2;
        } else {
            y = 1000 * (c[0] - '0') + 100 * (c[1] - '0') + 10 * (c[2] - '0') + (c[3] - '0'); 
            c += 4;
        }
        m = 10 * (c[0] - '0') + (c[1] - '0'); 
        c += 2;
        d = 10 * (c[0] - '0') + (c[1] - '0'); 
        y -= 1900;
        m -= 1;
        if (t.tm_year < y) {
            sp->errorMsg = sclone("Corrupt certificate");
            return PS_FAILURE; 
        }
        if (t.tm_year == y) {
            if (t.tm_mon < m || (t.tm_mon == m && t.tm_mday < d)) {
                sp->errorMsg = sclone("Certificate not yet active");
                return PS_FAILURE;
            }
        }
    }

    /* 
        Validate the 'not after' date 
     */
    if ((c = cert->notAfter) != NULL) {
        if (strlen(c) < 8) {
            sp->errorMsg = sclone("Corrupt certificate");
            return PS_FAILURE;
        }
        if (cert->notAfterTimeType == ASN_UTCTIME) {
            y =  2000 + 10 * (c[0] - '0') + (c[1] - '0'); 
            c += 2;
        } else {
            y = 1000 * (c[0] - '0') + 100 * (c[1] - '0') + 10 * (c[2] - '0') + (c[3] - '0'); 
            c += 4;
        }
        m = 10 * (c[0] - '0') + (c[1] - '0'); 
        c += 2;
        d = 10 * (c[0] - '0') + (c[1] - '0'); 
        y -= 1900;
        m -= 1;
        if (t.tm_year > y) {
            sp->errorMsg = sclone("Corrupt certificate");
            return PS_FAILURE; 
        } else if (t.tm_year == y) {
            if (t.tm_mon > m || (t.tm_mon == m && t.tm_mday > d)) {
                sp->errorMsg = sclone("Certificate expired");
                return PS_FAILURE;
            }
        }
    }
    /*
        Must parse here as MatrixSSL frees this if you have both client and server enabled in the library
     */
    msp->peerCert = mprCreateBuf(0, 0);
    parseCert(msp->peerCert, sp->acceptIp ? "SERVER" : "CLIENT", cert);
    mprLog(3, "MatrixSSL: Certificate verified");
    mprLog(4, "MatrixSSL: %s", mprGetBufStart(msp->peerCert));
    return PS_SUCCESS;
}


static void disconnectMss(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


static ssize blockingWrite(MprSocket *sp, cvoid *buf, ssize len)
{
    MprSocketProvider   *standard;
    ssize               written, bytes;
    int                 prior;

    standard = sp->service->standardProvider;
    prior = mprSetSocketBlockingMode(sp, 1);
    for (written = 0; len > 0; ) {
        if ((bytes = standard->writeSocket(sp, buf, len)) < 0) {
            mprSetSocketBlockingMode(sp, prior);
            return bytes;
        }
        buf = (char*) buf + bytes;
        len -= bytes;
        written += bytes;
    }
    mprSetSocketBlockingMode(sp, prior);
    return written;
}


static int32 handshakeIsComplete(ssl_t *ssl)
{   
    return (ssl->hsState == SSL_HS_DONE) ? PS_TRUE : PS_FALSE;
}


/*
    Construct the initial HELLO message to send to the server and initiate
    the SSL handshake. Can be used in the re-handshake scenario as well.
    This is a blocking routine.
 */
static int handshakeMss(MprSocket *sp, short cipherSuite)
{
    MatrixSocket    *msp;
    ssize           rc, written, toWrite;
    char            *obuf, buf[BIT_MAX_BUFFER];
    int             mode;

    msp = sp->sslSocket;

    toWrite = matrixSslGetOutdata(msp->ctx, (uchar**) &obuf);
    if ((written = blockingWrite(sp, obuf, toWrite)) < 0) {
        mprError("MatrixSSL: Error in socketWrite");
        return MPR_ERR_CANT_INITIALIZE;
    }
    matrixSslSentData(msp->ctx, (int) written);
    mode = mprSetSocketBlockingMode(sp, 1);

    while (1) {
        /*
            Reading handshake records should always return 0 bytes, we aren't expecting any data yet.
         */
        if ((rc = innerRead(sp, buf, sizeof(buf))) == 0) {
            if (mprIsSocketEof(sp)) {
                mprSetSocketBlockingMode(sp, mode);
                return MPR_ERR_CANT_INITIALIZE;
            }
            if (handshakeIsComplete(msp->ctx)) {
                break;
            }
        } else {
            mprError("MatrixSSL: sslRead error in sslDoHandhake, rc %d", rc);
            mprSetSocketBlockingMode(sp, mode);
            return MPR_ERR_CANT_INITIALIZE;
        }
    }
    mprSetSocketBlockingMode(sp, mode);
    return 0;
}


/*
    Process incoming data. Some is app data, some is SSL control data.
 */
static ssize processMssData(MprSocket *sp, char *buf, ssize size, ssize nbytes, int *readMore)
{
    MatrixSocket    *msp;
    uchar           *data, *obuf;
    ssize           toWrite, written, copied, sofar;
    uint32          dlen;
    int                 rc;

    msp = (MatrixSocket*) sp->sslSocket;
    *readMore = 0;
    sofar = 0;

    /*
        Process the received data. If there is application data, it is returned in data/dlen
     */
    rc = matrixSslReceivedData(msp->ctx, (int) nbytes, &data, &dlen);

    while (1) {
        switch (rc) {
        case PS_SUCCESS:
            return sofar;

        case MATRIXSSL_REQUEST_SEND:
            toWrite = matrixSslGetOutdata(msp->ctx, &obuf);
            if ((written = blockingWrite(sp, obuf, toWrite)) < 0) {
                mprError("MatrixSSL: Error in process");
                return MPR_ERR_CANT_INITIALIZE;
            }
            matrixSslSentData(msp->ctx, (int) written);
            if (msp->ctx->err != SSL_ALERT_NONE && msp->ctx->err != SSL_ALLOW_ANON_CONNECTION) {
                return -1;
            }
            *readMore = 1;
            return 0;

        case MATRIXSSL_REQUEST_RECV:
            /* Partial read. More read data required */
            *readMore = 1;
            msp->more = 1;
            return 0;

        case MATRIXSSL_HANDSHAKE_COMPLETE:
            *readMore = 0;
            return 0;

        case MATRIXSSL_RECEIVED_ALERT:
            assert(dlen == 2);
            if (data[0] == SSL_ALERT_LEVEL_FATAL) {
                return MPR_ERR;
            } else if (data[1] == SSL_ALERT_CLOSE_NOTIFY) {
                //  ignore - graceful close
                return 0;
            } else {
                //  ignore
            }
            rc = matrixSslProcessedData(msp->ctx, &data, &dlen);
            break;

        case MATRIXSSL_APP_DATA:
            copied = min((ssize) dlen, size);
            memcpy(buf, data, copied);
            buf += copied;
            size -= copied;
            data += copied;
            dlen = dlen - (int) copied;
            sofar += copied;
            msp->more = ((ssize) dlen > size) ? 1 : 0;
            if (!msp->more) {
                /* The MatrixSSL buffer has been consumed, see if we can get more data */
                rc = matrixSslProcessedData(msp->ctx, &data, &dlen);
                break;
            }
            return sofar;

        default:
            return MPR_ERR;
        }
    }
}


/*
    Return number of bytes read. Return -1 on errors and EOF
 */
static ssize innerRead(MprSocket *sp, char *buf, ssize size)
{
    MprSocketProvider   *standard;
    MatrixSocket        *msp;
    uchar               *mbuf;
    ssize               nbytes;
    int                 msize, readMore;

    msp = (MatrixSocket*) sp->sslSocket;
    standard = sp->service->standardProvider;
    do {
        /*
            Get the MatrixSSL read buffer to read data into
         */
        if ((msize = matrixSslGetReadbuf(msp->ctx, &mbuf)) < 0) {
            return MPR_ERR_BAD_STATE;
        }
        readMore = 0;
        if ((nbytes = standard->readSocket(sp, mbuf, msize)) < 0) {
            return nbytes;
        } else if (nbytes > 0) {
            nbytes = processMssData(sp, buf, size, nbytes, &readMore);
            if (nbytes < 0 || nbytes > 0) {
                return nbytes;
            }
        }
    } while (readMore);
    return 0;
}


/*
    Return number of bytes read. Return -1 on errors and EOF.
 */
static ssize readMss(MprSocket *sp, void *buf, ssize len)
{
    MatrixSocket    *msp;
    ssize           bytes;

    if (len <= 0) {
        return -1;
    }
    lock(sp);
    bytes = innerRead(sp, buf, len);
    msp = (MatrixSocket*) sp->sslSocket;
    if (msp->more) {
        mprHiddenSocketData(sp, msp->more, MPR_READABLE);
    }
    unlock(sp);
    return bytes;
}


/*
    Non-blocking write data. Return the number of bytes written or -1 on errors.
    Returns zero if part of the data was written.

    Encode caller's data buffer into an SSL record and write to socket. The encoded data will always be 
    bigger than the incoming data because of the record header (5 bytes) and MAC (16 bytes MD5 / 20 bytes SHA1)
    This would be fine if we were using blocking sockets, but non-blocking presents an interesting problem.  Example:

        A 100 byte input record is encoded to a 125 byte SSL record
        We can send 124 bytes without blocking, leaving one buffered byte
        We can't return 124 to the caller because it's more than they requested
        We can't return 100 to the caller because they would assume all data
        has been written, and we wouldn't get re-called to send the last byte

    We handle the above case by returning 0 to the caller if the entire encoded record could not be sent. Returning 
    0 will prompt us to select this socket for write events, and we'll be called again when the socket is writable.  
    We'll use this mechanism to flush the remaining encoded data, ignoring the bytes sent in, as they have already 
    been encoded.  When it is completely flushed, we return the originally requested length, and resume normal 
    processing.
 */
static ssize writeMss(MprSocket *sp, cvoid *buf, ssize len)
{
    MatrixSocket    *msp;
    uchar           *obuf;
    ssize           encoded, nbytes, written;

    msp = (MatrixSocket*) sp->sslSocket;

    while (len > 0 || msp->outlen > 0) {
        if ((encoded = matrixSslGetOutdata(msp->ctx, &obuf)) <= 0) {
            if (msp->outlen <= 0) {
                msp->outbuf = (char*) buf;
                msp->outlen = len;
                msp->written = 0;
                len = 0;
            }
            nbytes = min(msp->outlen, SSL_MAX_PLAINTEXT_LEN);
            if ((encoded = matrixSslEncodeToOutdata(msp->ctx, (uchar*) buf, (int) nbytes)) < 0) {
                return encoded;
            }
            msp->outbuf += nbytes;
            msp->outlen -= nbytes;
            msp->written += nbytes;
        }
        if ((written = sp->service->standardProvider->writeSocket(sp, obuf, encoded)) < 0) {
            return written;
        } else if (written == 0) {
            break;
        }
        matrixSslSentData(msp->ctx, (int) written);
    }
    /*
        Only signify all the data has been written if MatrixSSL has absorbed all the data
     */
    return msp->outlen == 0 ? msp->written : 0;
}


/*
    Blocking flush
 */
static ssize flushMss(MprSocket *sp)
{
    return blockingWrite(sp, 0, 0);
}


/*
    Cleanup for all-in-one distributions
 */
#undef SSL_RSA_WITH_3DES_EDE_CBC_SHA
#undef TLS_RSA_WITH_AES_128_CBC_SHA
#undef TLS_RSA_WITH_AES_256_CBC_SHA

#else
void matrixsslDummy() {}
#endif /* BIT_PACK_MATRIXSSL */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ssl/est.c"
 */
/************************************************************************/

/*
    est.c - Embedded Secure Transport

    Individual sockets are not thread-safe. Must only be used by a single thread.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "mpr.h"

#if BIT_PACK_EST

#include    "est.h"

/************************************* Defines ********************************/
/*
    Per-route SSL configuration
 */
typedef struct EstConfig {
    rsa_context     rsa;                /* RSA context */
    x509_cert       cert;               /* Certificate (own) */
    x509_cert       ca;                 /* Certificate authority bundle to verify peer */
    int             *ciphers;           /* Set of acceptable ciphers */
    char            *dhKey;             /* DH keys */
} EstConfig;

/*
    Per socket state
 */
typedef struct EstSocket {
    MprSocket       *sock;              /* MPR socket object */
    MprTicks        started;            /* When connection begun */
    EstConfig       *cfg;               /* Configuration */
    havege_state    hs;                 /* Random HAVEGE state */
    ssl_context     ctx;                /* SSL state */
    ssl_session     session;            /* SSL sessions */
} EstSocket;

static MprSocketProvider *estProvider;  /* EST socket provider */
static EstConfig *defaultEstConfig;     /* Default configuration */

/*
    Regenerate using: dh_genprime
    Generated on 1/1/2014
 */
static char *dhG = "4";
static char *dhKey =
    "E4004C1F94182000103D883A448B3F80"
    "2CE4B44A83301270002C20D0321CFD00"
    "11CCEF784C26A400F43DFB901BCA7538"
    "F2C6B176001CF5A0FD16D2C48B1D0C1C"
    "F6AC8E1DA6BCC3B4E1F96B0564965300"
    "FFA1D0B601EB2800F489AA512C4B248C"
    "01F76949A60BB7F00A40B1EAB64BDD48" 
    "E8A700D60B7F1200FA8E77B0A979DABF";

/*
    Thread-safe session list
 */
static MprList *sessions;

/***************************** Forward Declarations ***************************/

static void     closeEst(MprSocket *sp, bool gracefully);
static void     disconnectEst(MprSocket *sp);
static void     estTrace(void *fp, int level, char *str);
static int      handshakeEst(MprSocket *sp);
static char     *getEstState(MprSocket *sp);
static void     manageEstConfig(EstConfig *cfg, int flags);
static void     manageEstProvider(MprSocketProvider *provider, int flags);
static void     manageEstSocket(EstSocket *ssp, int flags);
static ssize    readEst(MprSocket *sp, void *buf, ssize len);
static int      upgradeEst(MprSocket *sp, MprSsl *sslConfig, cchar *peerName);
static ssize    writeEst(MprSocket *sp, cvoid *buf, ssize len);

static int      setSession(ssl_context *ssl);
static int      getSession(ssl_context *ssl);

/************************************* Code ***********************************/
/*
    Create the EST module. This is called only once
 */
PUBLIC int mprCreateEstModule()
{
    if ((estProvider = mprAllocObj(MprSocketProvider, manageEstProvider)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    estProvider->upgradeSocket = upgradeEst;
    estProvider->closeSocket = closeEst;
    estProvider->disconnectSocket = disconnectEst;
    estProvider->readSocket = readEst;
    estProvider->writeSocket = writeEst;
    estProvider->socketState = getEstState;
    mprAddSocketProvider("est", estProvider);
    sessions = mprCreateList(0, 0);

    if ((defaultEstConfig = mprAllocObj(EstConfig, manageEstConfig)) == 0) {
        return MPR_ERR_MEMORY;
    }
    defaultEstConfig->dhKey = dhKey;
    return 0;
}


static void manageEstProvider(MprSocketProvider *provider, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(provider->name);
        mprMark(defaultEstConfig);
        mprMark(sessions);

    } else if (flags & MPR_MANAGE_FREE) {
        defaultEstConfig = 0;
        sessions = 0;
    }
}


static void manageEstConfig(EstConfig *cfg, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;

    } else if (flags & MPR_MANAGE_FREE) {
        rsa_free(&cfg->rsa);
        x509_free(&cfg->cert);
        x509_free(&cfg->ca);
        free(cfg->ciphers);
    }
}


/*
    Destructor for an EstSocket object
 */
static void manageEstSocket(EstSocket *est, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(est->cfg);
        mprMark(est->sock);

    } else if (flags & MPR_MANAGE_FREE) {
        ssl_free(&est->ctx);
    }
}


static void closeEst(MprSocket *sp, bool gracefully)
{
    EstSocket       *est;

    est = sp->sslSocket;
    if (!(sp->flags & MPR_SOCKET_EOF)) {
        ssl_close_notify(&est->ctx);
    }
    sp->service->standardProvider->closeSocket(sp, gracefully);
}


/*
    Upgrade a standard socket to use TLS
 */
static int upgradeEst(MprSocket *sp, MprSsl *ssl, cchar *peerName)
{
    EstSocket   *est;
    EstConfig   *cfg;
    int         verifyMode;

    assert(sp);

    if (ssl == 0) {
        ssl = mprCreateSsl(sp->flags & MPR_SOCKET_SERVER);
    }
    if ((est = (EstSocket*) mprAllocObj(EstSocket, manageEstSocket)) == 0) {
        return MPR_ERR_MEMORY;
    }
    est->sock = sp;
    sp->sslSocket = est;
    sp->ssl = ssl;
    verifyMode = (sp->flags & MPR_SOCKET_SERVER && !ssl->verifyPeer) ? SSL_VERIFY_NO_CHECK : SSL_VERIFY_OPTIONAL;

    lock(ssl);
    if (ssl->config && !ssl->changed) {
        est->cfg = cfg = ssl->config;
    } else {
        ssl->changed = 0;

        /*
            One time setup for the SSL configuration for this MprSsl
         */
        if ((cfg = mprAllocObj(EstConfig, manageEstConfig)) == 0) {
            unlock(ssl);
            return MPR_ERR_MEMORY;
        }
        if (ssl->certFile) {
            /*
                Load a PEM format certificate file
             */
            if (x509parse_crtfile(&cfg->cert, ssl->certFile) != 0) {
                sp->errorMsg = sfmt("Unable to parse certificate %s", ssl->certFile); 
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
        }
        if (ssl->keyFile) {
            /*
                Load a decrypted PEM format private key
                Last arg is password if you need to use an encrypted private key
             */
            if (x509parse_keyfile(&cfg->rsa, ssl->keyFile, 0) != 0) {
                sp->errorMsg = sfmt("Unable to parse key file %s", ssl->keyFile); 
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
        }
        if (verifyMode != SSL_VERIFY_NO_CHECK) {
            if (!ssl->caFile) {
                sp->errorMsg = sclone("No defined certificate authority file");
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            if (x509parse_crtfile(&cfg->ca, ssl->caFile) != 0) {
                sp->errorMsg = sfmt("Unable to open or parse certificate authority file %s", ssl->caFile); 
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
        }
        est->cfg = ssl->config = cfg;
        cfg->dhKey = defaultEstConfig->dhKey;
        cfg->ciphers = ssl_create_ciphers(ssl->ciphers);
    }
    unlock(ssl);

    //  TODO - convert to proper entropy source API
    //  TODO - can't put this in cfg yet as it is not thread safe
    ssl_free(&est->ctx);
    havege_init(&est->hs);
    ssl_init(&est->ctx);
    ssl_set_endpoint(&est->ctx, sp->flags & MPR_SOCKET_SERVER ? SSL_IS_SERVER : SSL_IS_CLIENT);
    ssl_set_authmode(&est->ctx, verifyMode);
    ssl_set_rng(&est->ctx, havege_rand, &est->hs);
    ssl_set_dbg(&est->ctx, estTrace, NULL);
    ssl_set_bio(&est->ctx, net_recv, &sp->fd, net_send, &sp->fd);

    //  TODO - better if the API took a handle (est)
    ssl_set_scb(&est->ctx, getSession, setSession);
    ssl_set_ciphers(&est->ctx, cfg->ciphers);

    ssl_set_session(&est->ctx, 1, 0, &est->session);
    memset(&est->session, 0, sizeof(ssl_session));

    ssl_set_ca_chain(&est->ctx, ssl->caFile ? &cfg->ca : NULL, (char*) peerName);
    if (ssl->keyFile && ssl->certFile) {
        ssl_set_own_cert(&est->ctx, &cfg->cert, &cfg->rsa);
    }
    ssl_set_dh_param(&est->ctx, dhKey, dhG);
    est->started = mprGetTicks();

    if (handshakeEst(sp) < 0) {
        return -1;
    }
    return 0;
}


static void disconnectEst(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


/*
    Initiate or continue SSL handshaking with the peer. This routine does not block.
    Return -1 on errors, 0 incomplete and awaiting I/O, 1 if successful
 */
static int handshakeEst(MprSocket *sp)
{
    EstSocket   *est;
    int         rc, vrc, trusted;

    est = (EstSocket*) sp->sslSocket;
    assert(!(est->ctx.state == SSL_HANDSHAKE_OVER));
    rc = 0;
    trusted = 1;

    sp->flags |= MPR_SOCKET_HANDSHAKING;
    while (est->ctx.state != SSL_HANDSHAKE_OVER && (rc = ssl_handshake(&est->ctx)) != 0) {
        if (rc == EST_ERR_NET_TRY_AGAIN) {
            if (!mprGetSocketBlockingMode(sp)) {
                return 0;
            }
            continue;
        }
        /* Error */
        break;
    }
    sp->flags &= ~MPR_SOCKET_HANDSHAKING;
    mprLog(4, "Est handshake complete in %,d msec", mprGetTicks() - est->started);

    /*
        Analyze the handshake result
     */
    if (rc < 0) {
        //  TODO - more codes here or have est set a textual message (better)
        if (rc == EST_ERR_SSL_PRIVATE_KEY_REQUIRED && !(sp->ssl->keyFile || sp->ssl->certFile)) {
            sp->errorMsg = sclone("Peer requires a certificate");
        } else {
            sp->errorMsg = sfmt("Cannot handshake: error -0x%x", -rc);
        }
        mprLog(4, "%s", sp->errorMsg);
        sp->flags |= MPR_SOCKET_EOF;
        errno = EPROTO;
        return -1;
       
    } else if ((vrc = ssl_get_verify_result(&est->ctx)) != 0) {
        if (vrc & BADCERT_EXPIRED) {
            sp->errorMsg = sclone("Certificate expired");

        } else if (vrc & BADCERT_REVOKED) {
            sp->errorMsg = sclone("Certificate revoked");

        } else if (vrc & BADCERT_CN_MISMATCH) {
            sp->errorMsg = sclone("Certificate common name mismatch");

        } else if (vrc & BADCERT_NOT_TRUSTED) {
            if (vrc & BADCERT_SELF_SIGNED) {
                sp->errorMsg = sclone("Self-signed certificate");
            } else {
                sp->errorMsg = sclone("Certificate not trusted");
            }
            trusted = 0;

        } else {
            if (est->ctx.client_auth && !sp->ssl->certFile) {
                sp->errorMsg = sclone("Server requires a client certificate");
            } else if (rc == EST_ERR_NET_CONN_RESET) {
                sp->errorMsg = sclone("Peer disconnected");
            } else {
                sp->errorMsg = sfmt("Cannot handshake: error -0x%x", -rc);
            }
        }
        mprLog(4, "%s", sp->errorMsg);
        if (sp->ssl->verifyPeer) {
            /* 
               If not verifying the issuer, permit certs that are only untrusted (no other error).
               This allows self-signed certs.
             */
            if (!sp->ssl->verifyIssuer && !trusted) {
                return 1;
            } else {
                sp->flags |= MPR_SOCKET_EOF;
                errno = EPROTO;
                return -1;
            }
        }
#if UNUSED
    } else {
        /* TODO - being emitted when no cert supplied */
        mprLog(3, "EST: Certificate verified");
#endif
    }
    return 1;
}


/*
    Return the number of bytes read. Return -1 on errors and EOF. Distinguish EOF via mprIsSocketEof.
    If non-blocking, may return zero if no data or still handshaking.
 */
static ssize readEst(MprSocket *sp, void *buf, ssize len)
{
    EstSocket   *est;
    int         rc;

    est = (EstSocket*) sp->sslSocket;
    assert(est);
    assert(est->cfg);

    if (est->ctx.state != SSL_HANDSHAKE_OVER) {
        if ((rc = handshakeEst(sp)) <= 0) {
            return rc;
        }
    }
    while (1) {
        rc = ssl_read(&est->ctx, buf, (int) len);
        mprLog(5, "EST: ssl_read %d", rc);
        if (rc < 0) {
            if (rc == EST_ERR_NET_TRY_AGAIN)  {
                rc = 0;
                break;
            } else if (rc == EST_ERR_SSL_PEER_CLOSE_NOTIFY) {
                mprLog(5, "EST: connection was closed gracefully\n");
                sp->flags |= MPR_SOCKET_EOF;
                return -1;
            } else if (rc == EST_ERR_NET_CONN_RESET) {
                mprLog(5, "EST: connection reset");
                sp->flags |= MPR_SOCKET_EOF;
                return -1;
            } else {
                mprLog(4, "EST: read error -0x%", -rc);
                sp->flags |= MPR_SOCKET_EOF;
                return -1;
            }
        }
        break;
    }
    mprHiddenSocketData(sp, ssl_get_bytes_avail(&est->ctx), MPR_READABLE);
    return rc;
}


/*
    Write data. Return the number of bytes written or -1 on errors or socket closure.
    If non-blocking, may return zero if no data or still handshaking.
 */
static ssize writeEst(MprSocket *sp, cvoid *buf, ssize len)
{
    EstSocket   *est;
    ssize       totalWritten;
    int         rc;

    est = (EstSocket*) sp->sslSocket;
    if (len <= 0) {
        assert(0);
        return -1;
    }
    if (est->ctx.state != SSL_HANDSHAKE_OVER) {
        if ((rc = handshakeEst(sp)) <= 0) {
            return rc;
        }
    }
    totalWritten = 0;
    rc = 0;
    do {
        rc = ssl_write(&est->ctx, (uchar*) buf, (int) len);
        mprLog(7, "EST: written %d, requested len %d", rc, len);
        if (rc <= 0) {
            if (rc == EST_ERR_NET_TRY_AGAIN) {                                                          
                break;
            }
            if (rc == EST_ERR_NET_CONN_RESET) {                                                         
                mprLog(0, "ssl_write peer closed");
                return -1;
            } else {
                mprLog(0, "ssl_write failed rc -0x%x", -rc);
                return -1;
            }
        } else {
            totalWritten += rc;
            buf = (void*) ((char*) buf + rc);
            len -= rc;
            mprLog(7, "EST: write: len %d, written %d, total %d", len, rc, totalWritten);
        }
    } while (len > 0);
    mprHiddenSocketData(sp, est->ctx.out_left, MPR_WRITABLE);

    if (totalWritten == 0 && rc == EST_ERR_NET_TRY_AGAIN) {                                                          
        mprSetOsError(EAGAIN);
        return -1;
    }
    return totalWritten;
}


static char *getEstState(MprSocket *sp)
{
    EstSocket       *est;
    ssl_context     *ctx;
    MprBuf          *buf;
    char            *own, *peer;
    char            cbuf[5120];         //  TODO - must not be a static buffer

    if ((est = sp->sslSocket) == 0) {
        return 0;
    }
    ctx = &est->ctx;
    buf = mprCreateBuf(0, 0);
    mprPutToBuf(buf, "PROVIDER=est,CIPHER=%s,", ssl_get_cipher(ctx));

    own =  sp->acceptIp ? "SERVER_" : "CLIENT_";
    peer = sp->acceptIp ? "CLIENT_" : "SERVER_";
    if (ctx->peer_cert) {
        x509parse_cert_info(peer, cbuf, sizeof(cbuf), ctx->peer_cert);
        mprPutStringToBuf(buf, cbuf);
    }
    if (ctx->own_cert) {
        x509parse_cert_info(own, cbuf, sizeof(cbuf), ctx->own_cert);
        mprPutStringToBuf(buf, cbuf);
    }
    mprTrace(5, "EST state: %s", mprGetBufStart(buf));
    return mprGetBufStart(buf);
}


/*
    Thread-safe session management
 */
static int getSession(ssl_context *ssl)
{
    ssl_session     *sp;
    time_t          t;
    int             next;

    t = time(NULL);
    if (!ssl->resume) {
        return 1;
    }
    for (ITERATE_ITEMS(sessions, sp, next)) {
        if (ssl->timeout && (t - sp->start) > ssl->timeout) {
            continue;
        }
        if (ssl->session->cipher != sp->cipher || ssl->session->length != sp->length) {
            continue;
        }
        if (memcmp(ssl->session->id, sp->id, sp->length) != 0) {
            continue;
        }
        memcpy(ssl->session->master, sp->master, sizeof(ssl->session->master));
        return 0;
    }
    return 1;
}


static int setSession(ssl_context *ssl)
{
    time_t          t;
    ssl_session     *sp;
    int             next;

    t = time(NULL);
    for (ITERATE_ITEMS(sessions, sp, next)) {
        if (ssl->timeout != 0 && (t - sp->start) > ssl->timeout) {
            /* expired, reuse this slot */
            break;  
        }
        if (memcmp(ssl->session->id, sp->id, sp->length) == 0) {
            /* client reconnected */
            break;  
        }
    }
    if (sp == NULL) {
        if ((sp = mprAlloc(sizeof(ssl_session))) == 0) {
            return 1;
        }
        mprAddItem(sessions, sp);
    }
    memcpy(sp, ssl->session, sizeof(ssl_session));
    return 0;
}


static void estTrace(void *fp, int level, char *str)
{
    level += 3;
    if (level <= MPR->logLevel) {
        mprRawLog(level, "%s: %d: EST: %s", MPR->name, level, str);
    }
}

#else
void estDummy() {}
#endif /* BIT_PACK_EST */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ssl/openssl.c"
 */
/************************************************************************/

/*
    openssl.c - Support for secure sockets via OpenSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "mpr.h"

#if BIT_PACK_OPENSSL

/* Clashes with WinCrypt.h */
#undef OCSP_RESPONSE
#include    <openssl/ssl.h>
#include    <openssl/evp.h>
#include    <openssl/rand.h>
#include    <openssl/err.h>
#include    <openssl/dh.h>

/************************************* Defines ********************************/

typedef struct OpenConfig {
    SSL_CTX         *context;
    RSA             *rsaKey512;
    RSA             *rsaKey1024;
    DH              *dhKey512;
    DH              *dhKey1024;
} OpenConfig;

typedef struct OpenSocket {
    MprSocket       *sock;
    OpenConfig      *cfg;
    char            *peerName;
    SSL             *handle;
    //  TODO - is the bio used?
    BIO             *bio;
} OpenSocket;

typedef struct RandBuf {
    MprTime     now;
    int         pid;
} RandBuf;

static int      numLocks;
static MprMutex **olocks;
static MprSocketProvider *openProvider;
static OpenConfig *defaultOpenConfig;

struct CRYPTO_dynlock_value {
    MprMutex    *mutex;
};
typedef struct CRYPTO_dynlock_value DynLock;

/***************************** Forward Declarations ***************************/

static void     closeOss(MprSocket *sp, bool gracefully);
static int      configureCertificateFiles(MprSsl *ssl, SSL_CTX *ctx, char *key, char *cert);
static OpenConfig *createOpenSslConfig(MprSocket *sp);
static DH       *dhCallback(SSL *ssl, int isExport, int keyLength);
static void     disconnectOss(MprSocket *sp);
static ssize    flushOss(MprSocket *sp);
static char     *getOssState(MprSocket *sp);
static void     manageOpenConfig(OpenConfig *cfg, int flags);
static void     manageOpenProvider(MprSocketProvider *provider, int flags);
static void     manageOpenSocket(OpenSocket *ssp, int flags);
static ssize    readOss(MprSocket *sp, void *buf, ssize len);
static RSA      *rsaCallback(SSL *ssl, int isExport, int keyLength);
static int      upgradeOss(MprSocket *sp, MprSsl *ssl, cchar *peerName);
static int      verifyX509Certificate(int ok, X509_STORE_CTX *ctx);
static ssize    writeOss(MprSocket *sp, cvoid *buf, ssize len);

static DynLock  *sslCreateDynLock(const char *file, int line);
static void     sslDynLock(int mode, DynLock *dl, const char *file, int line);
static void     sslDestroyDynLock(DynLock *dl, const char *file, int line);
static void     sslStaticLock(int mode, int n, const char *file, int line);
static ulong    sslThreadId(void);

static DH       *get_dh512();
static DH       *get_dh1024();

/************************************* Code ***********************************/
/*
    Create the Openssl module. This is called only once
 */
PUBLIC int mprCreateOpenSslModule()
{
    RandBuf     randBuf;
    int         i;

    randBuf.now = mprGetTime();
    randBuf.pid = getpid();
    RAND_seed((void*) &randBuf, sizeof(randBuf));
#if BIT_UNIX_LIKE
    mprLog(6, "OpenSsl: Before calling RAND_load_file");
    RAND_load_file("/dev/urandom", 256);
    mprLog(6, "OpenSsl: After calling RAND_load_file");
#endif

    if ((openProvider = mprAllocObj(MprSocketProvider, manageOpenProvider)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    openProvider->upgradeSocket = upgradeOss;
    openProvider->closeSocket = closeOss;
    openProvider->disconnectSocket = disconnectOss;
    openProvider->flushSocket = flushOss;
    openProvider->socketState = getOssState;
    openProvider->readSocket = readOss;
    openProvider->writeSocket = writeOss;
    mprAddSocketProvider("openssl", openProvider);

    /*
        Pre-create expensive keys
     */
    if ((defaultOpenConfig = mprAllocObj(OpenConfig, manageOpenConfig)) == 0) {
        return MPR_ERR_MEMORY;
    }
    defaultOpenConfig->rsaKey512 = RSA_generate_key(512, RSA_F4, 0, 0);
    defaultOpenConfig->rsaKey1024 = RSA_generate_key(1024, RSA_F4, 0, 0);
    defaultOpenConfig->dhKey512 = get_dh512();
    defaultOpenConfig->dhKey1024 = get_dh1024();

    /*
        Configure the SSL library. Use the crypto ID as a one-time test. This allows
        users to configure the library and have their configuration used instead.
     */
    if (CRYPTO_get_id_callback() == 0) {
        numLocks = CRYPTO_num_locks();
        if ((olocks = mprAlloc(numLocks * sizeof(MprMutex*))) == 0) {
            return MPR_ERR_MEMORY;
        }
        for (i = 0; i < numLocks; i++) {
            olocks[i] = mprCreateLock();
        }
        CRYPTO_set_id_callback(sslThreadId);
        CRYPTO_set_locking_callback(sslStaticLock);

        CRYPTO_set_dynlock_create_callback(sslCreateDynLock);
        CRYPTO_set_dynlock_destroy_callback(sslDestroyDynLock);
        CRYPTO_set_dynlock_lock_callback(sslDynLock);
#if !BIT_WIN_LIKE
        /* OPT - Should be a configure option to specify desired ciphers */
        OpenSSL_add_all_algorithms();
#endif
        /*
            WARNING: SSL_library_init() is not reentrant. Caller must ensure safety.
         */
        SSL_library_init();
        SSL_load_error_strings();
    }
    return 0;
}


static void manageOpenConfig(OpenConfig *cfg, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;
    } else if (flags & MPR_MANAGE_FREE) {
        if (cfg->context != 0) {
            SSL_CTX_free(cfg->context);
            cfg->context = 0;
        }
        if (cfg == defaultOpenConfig) {
            if (cfg->rsaKey512) {
                RSA_free(cfg->rsaKey512);
                cfg->rsaKey512 = 0;
            }
            if (cfg->rsaKey1024) {
                RSA_free(cfg->rsaKey1024);
                cfg->rsaKey1024 = 0;
            }
            if (cfg->dhKey512) {
                DH_free(cfg->dhKey512);
                cfg->dhKey512 = 0;
            }
            if (cfg->dhKey1024) {
                DH_free(cfg->dhKey1024);
                cfg->dhKey1024 = 0;
            }
        }
    }
}


static void manageOpenProvider(MprSocketProvider *provider, int flags)
{
    int     i;

    if (flags & MPR_MANAGE_MARK) {
        /* Mark global locks */
        if (olocks) {
            mprMark(olocks);
            for (i = 0; i < numLocks; i++) {
                mprMark(olocks[i]);
            }
        }
        mprMark(defaultOpenConfig);
        mprMark(provider->name);

    } else if (flags & MPR_MANAGE_FREE) {
        olocks = 0;
    }
}


/*
    Create an SSL configuration for a route. An application can have multiple different SSL 
    configurations for different routes. There is default SSL configuration that is used
    when a route does not define a configuration and also for clients.
 */
static OpenConfig *createOpenSslConfig(MprSocket *sp)
{
    MprSsl          *ssl;
    OpenConfig      *cfg;
    SSL_CTX         *context;
    uchar           resume[16];
    int             verifyMode;

    ssl = sp->ssl;
    assert(ssl);

    if ((ssl->config = mprAllocObj(OpenConfig, manageOpenConfig)) == 0) {
        return 0;
    }
    cfg = ssl->config;
    cfg->rsaKey512 = defaultOpenConfig->rsaKey512;
    cfg->rsaKey1024 = defaultOpenConfig->rsaKey1024;
    cfg->dhKey512 = defaultOpenConfig->dhKey512;
    cfg->dhKey1024 = defaultOpenConfig->dhKey1024;

    cfg = ssl->config;
    assert(cfg);

    if ((context = SSL_CTX_new(SSLv23_method())) == 0) {
        mprError("OpenSSL: Unable to create SSL context"); 
        return 0;
    }
    SSL_CTX_set_app_data(context, (void*) ssl);
    SSL_CTX_sess_set_cache_size(context, 512);
    RAND_bytes(resume, sizeof(resume));
    SSL_CTX_set_session_id_context(context, resume, sizeof(resume));

    verifyMode = (sp->flags & MPR_SOCKET_SERVER && !ssl->verifyPeer) ? SSL_VERIFY_NONE : 
        SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT;

    /*
        Configure the certificates
     */
    if (ssl->keyFile || ssl->certFile) {
        if (configureCertificateFiles(ssl, context, ssl->keyFile, ssl->certFile) != 0) {
            SSL_CTX_free(context);
            return 0;
        }
    }
    SSL_CTX_set_cipher_list(context, ssl->ciphers);

    if (verifyMode != SSL_VERIFY_NONE) {
        if (!(ssl->caFile || ssl->caPath)) {
            sp->errorMsg = sclone("No defined certificate authority file");
            SSL_CTX_free(context);
            return 0;
        }
        if ((!SSL_CTX_load_verify_locations(context, ssl->caFile, ssl->caPath)) ||
                (!SSL_CTX_set_default_verify_paths(context))) {
            sp->errorMsg = sclone("OpenSSL: Unable to set certificate locations"); 
            SSL_CTX_free(context);
            return 0;
        }
        if (ssl->caFile) {
            STACK_OF(X509_NAME) *certNames;
            certNames = SSL_load_client_CA_file(ssl->caFile);
            if (certNames) {
                /*
                    Define the list of CA certificates to send to the client
                    before they send their client certificate for validation
                 */
                SSL_CTX_set_client_CA_list(context, certNames);
            }
        }
        if (sp->flags & MPR_SOCKET_SERVER) {
            SSL_CTX_set_verify_depth(context, ssl->verifyDepth);
        }
    }
    SSL_CTX_set_verify(context, verifyMode, verifyX509Certificate);

    /*
        Define callbacks
     */
    SSL_CTX_set_tmp_rsa_callback(context, rsaCallback);
    SSL_CTX_set_tmp_dh_callback(context, dhCallback);

    SSL_CTX_set_options(context, SSL_OP_ALL);
#ifdef SSL_OP_NO_TICKET
    SSL_CTX_set_options(context, SSL_OP_NO_TICKET);
#endif
#ifdef SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION
    SSL_CTX_set_options(context, SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION);
#endif
    SSL_CTX_set_mode(context, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_AUTO_RETRY);
#ifdef SSL_OP_MSIE_SSLV2_RSA_PADDING
    SSL_CTX_set_options(context, SSL_OP_MSIE_SSLV2_RSA_PADDING);
#endif
#ifdef SSL_OP_NO_COMPRESSION                                                                                         
    SSL_CTX_set_options(context, SSL_OP_NO_COMPRESSION);                                                            
#endif
#ifdef SSL_MODE_RELEASE_BUFFERS                                                                                      
    SSL_CTX_set_mode(context, SSL_MODE_RELEASE_BUFFERS);                                                            
#endif
#if 0
    SSL_CTX_set_read_ahead(context, 1);                                                                             
    SSL_CTX_set_info_callback(context, info_callback); 
#endif

    /*
        Select the required protocols
        Disable SSLv2 by default -- it is insecure.
     */
    SSL_CTX_set_options(context, SSL_OP_NO_SSLv2);
    if (!(ssl->protocols & MPR_PROTO_SSLV3)) {
        SSL_CTX_set_options(context, SSL_OP_NO_SSLv3);
    }
    if (!(ssl->protocols & MPR_PROTO_TLSV1)) {
        SSL_CTX_set_options(context, SSL_OP_NO_TLSv1);
    }
    /* 
        Ensure we generate a new private key for each connection
     */
    SSL_CTX_set_options(context, SSL_OP_SINGLE_DH_USE);
    cfg->context = context;
    return cfg;
}


/*
    Configure the SSL certificate information using key and cert files
 */
static int configureCertificateFiles(MprSsl *ssl, SSL_CTX *ctx, char *key, char *cert)
{
    assert(ctx);

    if (cert == 0) {
        return 0;
    }
    if (cert && SSL_CTX_use_certificate_chain_file(ctx, cert) <= 0) {
        if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_ASN1) <= 0) {
            mprError("OpenSSL: Cannot open certificate file: %s", cert); 
            return -1;
        }
    }
    key = (key == 0) ? cert : key;
    if (key) {
        if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
            /* attempt ASN1 for self-signed format */
            if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_ASN1) <= 0) {
                mprError("OpenSSL: Cannot open private key file: %s", key); 
                return -1;
            }
        }
        if (!SSL_CTX_check_private_key(ctx)) {
            mprError("OpenSSL: Check of private key file failed: %s", key);
            return -1;
        }
    }
    return 0;
}


static void manageOpenSocket(OpenSocket *osp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(osp->sock);
        mprMark(osp->peerName);

    } else if (flags & MPR_MANAGE_FREE) {
        if (osp->handle) {
            SSL_set_shutdown(osp->handle, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
            SSL_free(osp->handle);
            osp->handle = 0;
        }
    }
}


static void closeOss(MprSocket *sp, bool gracefully)
{
    OpenSocket    *osp;

    osp = sp->sslSocket;
    SSL_free(osp->handle);
    osp->handle = 0;
    sp->service->standardProvider->closeSocket(sp, gracefully);
}


/*
    Upgrade a standard socket to use SSL/TLS
 */
static int upgradeOss(MprSocket *sp, MprSsl *ssl, cchar *peerName)
{
    OpenSocket      *osp;
    OpenConfig      *cfg;
    char            ebuf[BIT_MAX_BUFFER];
    ulong           error;
    int             rc;

    assert(sp);

    if (ssl == 0) {
        ssl = mprCreateSsl(sp->flags & MPR_SOCKET_SERVER);
    }
    if ((osp = (OpenSocket*) mprAllocObj(OpenSocket, manageOpenSocket)) == 0) {
        return MPR_ERR_MEMORY;
    }
    osp->sock = sp;
    sp->sslSocket = osp;
    sp->ssl = ssl;

    if (!ssl->config || ssl->changed) {
        if ((ssl->config = createOpenSslConfig(sp)) == 0) {
            return MPR_ERR_CANT_INITIALIZE;
        }
        ssl->changed = 0;
    }

    /*
        Create and configure the SSL struct
     */
    cfg = osp->cfg = sp->ssl->config;
    if ((osp->handle = (SSL*) SSL_new(cfg->context)) == 0) {
        return MPR_ERR_BAD_STATE;
    }
    SSL_set_app_data(osp->handle, (void*) osp);

    /*
        Create a socket bio
     */
    osp->bio = BIO_new_socket((int) sp->fd, BIO_NOCLOSE);
    SSL_set_bio(osp->handle, osp->bio, osp->bio);
    if (sp->flags & MPR_SOCKET_SERVER) {
        SSL_set_accept_state(osp->handle);
    } else {
        if (peerName) {
            osp->peerName = sclone(peerName);
        }
        /* Block while connecting */
        mprSetSocketBlockingMode(sp, 1);
        sp->errorMsg = 0;
        if ((rc = SSL_connect(osp->handle)) < 1) {
            if (sp->errorMsg) {
                mprLog(4, "%s", sp->errorMsg);
            } else {
                error = ERR_get_error();
                ERR_error_string_n(error, ebuf, sizeof(ebuf) - 1);
                sp->errorMsg = sclone(ebuf);
                mprLog(4, "SSL_read error %s", ebuf);
            }
            return MPR_ERR_CANT_CONNECT;
        }
        mprSetSocketBlockingMode(sp, 0);
    }
#if defined(BIT_MPR_SSL_RENEGOTIATE) && !BIT_MPR_SSL_RENEGOTIATE
    /*
        Disable renegotiation after the initial handshake if renegotiate is explicitly set to false (CVE-2009-3555).
        Note: this really is a bogus CVE as disabling renegotiation is not required nor does it enhance security if
        used with up-to-date (patched) SSL stacks 
     */
    if (osp->handle->s3) {
        osp->handle->s3->flags |= SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS;
    }
#endif
    return 0;
}


/*
    Parse the cert info and write properties to the buffer
    Modifies the info argument
 */
static void parseCertFields(MprBuf *buf, char *prefix, char *prefix2, char *info)
{
    char    c, *cp, *term, *key, *value;

    term = cp = info;
    do {
        c = *cp;
        if (c == '/' || c == '\0') {
            *cp = '\0';
            key = stok(term, "=", &value);
            if (smatch(key, "emailAddress")) {
                key = "EMAIL";
            }
            mprPutToBuf(buf, "%s%s%s=%s,", prefix, prefix2, key, value);
            term = &cp[1];
            *cp = c;
        }
    } while (*cp++ != '\0');
}


static char *getOssState(MprSocket *sp)
{
    OpenSocket      *osp;
    MprBuf          *buf;
    X509            *cert;
    char            *prefix;
    char            subject[512], issuer[512];

    osp = sp->sslSocket;
    buf = mprCreateBuf(0, 0);
    mprPutToBuf(buf, "PROVIDER=openssl,CIPHER=%s,", SSL_get_cipher(osp->handle));

    if ((cert = SSL_get_peer_certificate(osp->handle)) != 0) {
        prefix = sp->acceptIp ? "CLIENT_" : "SERVER_";
        X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject) -1);
        parseCertFields(buf, prefix, "S_", &subject[1]);

        X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) -1);
        parseCertFields(buf, prefix, "I_", &issuer[1]);
        X509_free(cert);
    }
    if ((cert = SSL_get_certificate(osp->handle)) != 0) {
        prefix =  sp->acceptIp ? "SERVER_" : "CLIENT_";
        X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject) -1);
        parseCertFields(buf, prefix, "S_", &subject[1]);

        X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) -1);
        parseCertFields(buf, prefix, "I_", &issuer[1]);
        /* Don't call X509_free on own cert */
    }
    mprTrace(5, "OpenSSL state: %s", mprGetBufStart(buf));
    return mprGetBufStart(buf);
}


static void disconnectOss(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


static int checkCert(MprSocket *sp)
{
    MprSsl      *ssl;
    OpenSocket  *osp;
    X509        *cert;
    X509_NAME   *xSubject;
    char        subject[512], issuer[512], peer[512], *dp, *pp;

    ssl = sp->ssl;
    osp = (OpenSocket*) sp->sslSocket;

    mprLog(4, "OpenSSL connected using cipher: \"%s\"", SSL_get_cipher(osp->handle));
    if (ssl->caFile) {
        mprLog(4, "OpenSSL: Using certificates from %s", ssl->caFile);
    } else if (ssl->caPath) {
        mprLog(4, "OpenSSL: Using certificates from directory %s", ssl->caPath);
    }
    cert = SSL_get_peer_certificate(osp->handle);
    if (cert == 0) {
        mprLog(4, "OpenSSL: client supplied no certificate");
        peer[0] = '\0';
    } else {
        xSubject = X509_get_subject_name(cert);
        X509_NAME_oneline(xSubject, subject, sizeof(subject) -1);
        X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) -1);
        X509_NAME_get_text_by_NID(xSubject, NID_commonName, peer, sizeof(peer) - 1);
        mprLog(4, "OpenSSL Subject %s", subject);
        mprLog(4, "OpenSSL Issuer: %s", issuer);
        mprLog(4, "OpenSSL Peer: %s", peer);
        X509_free(cert);
    }
    if (ssl->verifyPeer && osp->peerName) {
        if (smatch(peer, osp->peerName)) {
            /* simple match */
        } else if (*peer == '*' && peer[1] == '.') {
            pp = &peer[2];
            /* Cert peer must be of the form *.domain.tld. i.e. *.com is not valid */
            if (!strchr(pp, '.')) {
                sp->errorMsg = sfmt("Peer CN is not valid %s", peer);
                return -1;
            }
            /* Required peer name must have a domain portion. i.e. domain.tld */
            if ((dp = strchr(osp->peerName, '.')) != 0) {
                /* Strip the host portion and just test the domain portion */
                if (!smatch(pp, &dp[1])) {
                    sp->errorMsg = sfmt("Certificate common name mismatch CN \"%s\" vs required \"%s\"", 
                        peer, osp->peerName);
                    return -1;
                }
            }
        } else {
            sp->errorMsg = sfmt("Certificate common name mismatch CN \"%s\" vs required \"%s\"", peer, osp->peerName);
        }
    }
    return 0;
}


/*
    Return the number of bytes read. Return -1 on errors and EOF. Distinguish EOF via mprIsSocketEof.
    If non-blocking, may return zero if no data or still handshaking.
 */
static ssize readOss(MprSocket *sp, void *buf, ssize len)
{
    OpenSocket      *osp;
    char            ebuf[BIT_MAX_BUFFER];
    ulong           serror;
    int             rc, error, retries, i;

    //  OPT - should not need these locks
    lock(sp);
    osp = (OpenSocket*) sp->sslSocket;
    assert(osp);

    if (osp->handle == 0) {
        assert(osp->handle);
        unlock(sp);
        return -1;
    }
    /*  
        Limit retries on WANT_READ. If non-blocking and no data, then this can spin forever.
     */
    retries = 5;
    for (i = 0; i < retries; i++) {
        rc = SSL_read(osp->handle, buf, (int) len);
        if (rc < 0) {
            error = SSL_get_error(osp->handle, rc);
            if (error == SSL_ERROR_WANT_READ || error == SSL_ERROR_WANT_CONNECT || error == SSL_ERROR_WANT_ACCEPT) {
                continue;
            }
            serror = ERR_get_error();
            ERR_error_string_n(serror, ebuf, sizeof(ebuf) - 1);
            mprLog(5, "SSL_read %s", ebuf);
        }
        break;
    }
    if (rc > 0 && !(sp->flags & MPR_SOCKET_CHECKED)) {
        if (checkCert(sp) < 0) {
            return MPR_ERR_BAD_STATE;
        }
        sp->flags |= MPR_SOCKET_CHECKED;
    }
    if (rc <= 0) {
        error = SSL_get_error(osp->handle, rc);
        if (error == SSL_ERROR_WANT_READ) {
            rc = 0;
        } else if (error == SSL_ERROR_WANT_WRITE) {
            mprNap(10);
            rc = 0;
        } else if (error == SSL_ERROR_ZERO_RETURN) {
            sp->flags |= MPR_SOCKET_EOF;
            rc = -1;
        } else if (error == SSL_ERROR_SYSCALL) {
            sp->flags |= MPR_SOCKET_EOF;
            rc = -1;
        } else if (error != SSL_ERROR_ZERO_RETURN) {
            /* SSL_ERROR_SSL */
            serror = ERR_get_error();
            ERR_error_string_n(serror, ebuf, sizeof(ebuf) - 1);
            mprLog(4, "OpenSSL: connection with protocol error: %s", ebuf);
            rc = -1;
            sp->flags |= MPR_SOCKET_EOF;
        }
    } else if (SSL_pending(osp->handle) > 0) {
        sp->flags |= MPR_SOCKET_BUFFERED_READ;
        mprRecallWaitHandlerByFd(sp->fd);
    }
    unlock(sp);
    return rc;
}


/*
    Write data. Return the number of bytes written or -1 on errors.
 */
static ssize writeOss(MprSocket *sp, cvoid *buf, ssize len)
{
    OpenSocket  *osp;
    ssize       totalWritten;
    int         rc;

    //  OPT - should not need these locks
    lock(sp);
    osp = (OpenSocket*) sp->sslSocket;

    if (osp->bio == 0 || osp->handle == 0 || len <= 0) {
        assert(0);
        unlock(sp);
        return -1;
    }
    totalWritten = 0;
    ERR_clear_error();
    rc = 0;

    do {
        rc = SSL_write(osp->handle, buf, (int) len);
        mprLog(7, "OpenSSL: written %d, requested len %d", rc, len);
        if (rc <= 0) {
            if (SSL_get_error(osp->handle, rc) == SSL_ERROR_WANT_WRITE) {
                break;
            }
            unlock(sp);
            return -1;
        }
        totalWritten += rc;
        buf = (void*) ((char*) buf + rc);
        len -= rc;
        mprLog(7, "OpenSSL: write: len %d, written %d, total %d, error %d", len, rc, totalWritten, SSL_get_error(osp->handle, rc));
    } while (len > 0);
    unlock(sp);

    if (totalWritten == 0 && rc == SSL_ERROR_WANT_WRITE) {
        mprSetOsError(EAGAIN);
        return -1;
    }
    return totalWritten;
}

/*
    Called to verify X509 client certificates
 */
static int verifyX509Certificate(int ok, X509_STORE_CTX *xContext)
{
    X509            *cert;
    SSL             *handle;
    OpenSocket      *osp;
    MprSocket       *sp;
    MprSsl          *ssl;
    char            subject[512], issuer[512], peer[512];
    int             error, depth;
    
    subject[0] = issuer[0] = '\0';

    handle = (SSL*) X509_STORE_CTX_get_app_data(xContext);
    osp = (OpenSocket*) SSL_get_app_data(handle);
    sp = osp->sock;
    ssl = sp->ssl;

    cert = X509_STORE_CTX_get_current_cert(xContext);
    depth = X509_STORE_CTX_get_error_depth(xContext);
    error = X509_STORE_CTX_get_error(xContext);

    ok = 1;
    if (X509_NAME_oneline(X509_get_subject_name(cert), subject, sizeof(subject) - 1) < 0) {
        sp->errorMsg = sclone("Cannot get subject name");
        ok = 0;
    }
    if (X509_NAME_oneline(X509_get_issuer_name(cert), issuer, sizeof(issuer) - 1) < 0) {
        sp->errorMsg = sclone("Cannot get issuer name");
        ok = 0;
    }
    if (X509_NAME_get_text_by_NID(X509_get_subject_name(cert), NID_commonName, peer, sizeof(peer) - 1) == 0) {
        sp->errorMsg = sclone("Cannot get peer name");
        ok = 0;
    }
    if (ok && ssl->verifyDepth < depth) {
        if (error == 0) {
            error = X509_V_ERR_CERT_CHAIN_TOO_LONG;
        }
    }
    switch (error) {
    case X509_V_OK:
        break;
    case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
    case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
        /* Normal self signed certificate */
        if (ssl->verifyIssuer) {
            sp->errorMsg = sclone("Self-signed certificate");
            ok = 0;
        }
        break;

    case X509_V_ERR_CERT_UNTRUSTED:
    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
        if (ssl->verifyIssuer) {
            /* Issuer can't be verified */
            sp->errorMsg = sclone("Certificate not trusted");
            ok = 0;
        }
        break;

    case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
        if (ssl->verifyIssuer) {
            /* Issuer can't be verified */
            sp->errorMsg = sclone("Certificate not trusted");
            ok = 0;
        }
        break;

    case X509_V_ERR_CERT_CHAIN_TOO_LONG:
    case X509_V_ERR_CERT_HAS_EXPIRED:
    case X509_V_ERR_CERT_NOT_YET_VALID:
    case X509_V_ERR_CERT_REJECTED:
    case X509_V_ERR_CERT_SIGNATURE_FAILURE:
    case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
    case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
    case X509_V_ERR_INVALID_CA:
    default:
        sp->errorMsg = sfmt("Certificate verification error %d", error);
        ok = 0;
        break;
    }
    if (ok) {
        mprLog(3, "OpenSSL: Certificate verified");
    } else {
        mprLog(3, "OpenSSL: Certificate cannot be verified (more trace at level 4)");
        mprLog(3, "OpenSSL: %s", sp->errorMsg);
    }
    mprLog(4, "OpenSSL: Subject: %s", subject);
    mprLog(4, "OpenSSL: Issuer: %s", issuer);
    mprLog(4, "OpenSSL: Peer: %s", peer);
    return ok;
}


static ssize flushOss(MprSocket *sp)
{
    return 0;
}

 
static ulong sslThreadId()
{
    return (long) mprGetCurrentOsThread();
}


//  OPT - should not need these locks

static void sslStaticLock(int mode, int n, const char *file, int line)
{
    assert(0 <= n && n < numLocks);

    if (olocks) {
        if (mode & CRYPTO_LOCK) {
            mprLock(olocks[n]);
        } else {
            mprUnlock(olocks[n]);
        }
    }
}


//  OPT - should not need these locks
static DynLock *sslCreateDynLock(const char *file, int line)
{
    DynLock     *dl;

    dl = mprAllocZeroed(sizeof(DynLock));
    dl->mutex = mprCreateLock(dl);
    mprHold(dl->mutex);
    return dl;
}


static void sslDestroyDynLock(DynLock *dl, const char *file, int line)
{
    if (dl->mutex) {
        mprRelease(dl->mutex);
        dl->mutex = 0;
    }
}


static void sslDynLock(int mode, DynLock *dl, const char *file, int line)
{
    if (mode & CRYPTO_LOCK) {
        mprLock(dl->mutex);
    } else {
        mprUnlock(dl->mutex);
    }
}


/*
    Used for ephemeral RSA keys
 */
static RSA *rsaCallback(SSL *handle, int isExport, int keyLength)
{
    MprSocket       *sp;
    OpenSocket      *osp;
    OpenConfig      *cfg;
    RSA             *key;

    osp = (OpenSocket*) SSL_get_app_data(handle);
    sp = osp->sock;
    assert(sp);
    cfg = sp->ssl->config;

    key = 0;
    switch (keyLength) {
    case 512:
        key = cfg->rsaKey512;
        break;

    case 1024:
    default:
        key = cfg->rsaKey1024;
    }
    return key;
}


/*
    Used for ephemeral DH keys
 */
static DH *dhCallback(SSL *handle, int isExport, int keyLength)
{
    MprSocket       *sp;
    OpenSocket      *osp;
    OpenConfig      *cfg;
    DH              *key;

    osp = (OpenSocket*) SSL_get_app_data(handle);
    sp = osp->sock;
    cfg = sp->ssl->config;

    key = 0;
    switch (keyLength) {
    case 512:
        key = cfg->dhKey512;
        break;

    case 1024:
    default:
        key = cfg->dhKey1024;
    }
    return key;
}


/*
    openSslDh.c - OpenSSL DH get routines. Generated by openssl.
    Use bit gendh to generate new content.
 */
static DH *get_dh512()
{
    static unsigned char dh512_p[] = {
        0x8E,0xFD,0xBE,0xD3,0x92,0x1D,0x0C,0x0A,0x58,0xBF,0xFF,0xE4,
        0x51,0x54,0x36,0x39,0x13,0xEA,0xD8,0xD2,0x70,0xBB,0xE3,0x8C,
        0x86,0xA6,0x31,0xA1,0x04,0x2A,0x09,0xE4,0xD0,0x33,0x88,0x5F,
        0xEF,0xB1,0x70,0xEA,0x42,0xB6,0x0E,0x58,0x60,0xD5,0xC1,0x0C,
        0xD1,0x12,0x16,0x99,0xBC,0x7E,0x55,0x7C,0xE4,0xC1,0x5D,0x15,
        0xF6,0x45,0xBC,0x73,
    };
    static unsigned char dh512_g[] = {
        0x02,
    };
    DH *dh;
    if ((dh = DH_new()) == NULL) {
        return(NULL);
    }
    dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), NULL);
    dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL)) { 
        DH_free(dh); 
        return(NULL); 
    }
    return dh;
}


static DH *get_dh1024()
{
    static unsigned char dh1024_p[] = {
        0xCD,0x02,0x2C,0x11,0x43,0xCD,0xAD,0xF5,0x54,0x5F,0xED,0xB1,
        0x28,0x56,0xDF,0x99,0xFA,0x80,0x2C,0x70,0xB5,0xC8,0xA8,0x12,
        0xC3,0xCD,0x38,0x0D,0x3B,0xE1,0xE3,0xA3,0xE4,0xE9,0xCB,0x58,
        0x78,0x7E,0xA6,0x80,0x7E,0xFC,0xC9,0x93,0x3A,0x86,0x1C,0x8E,
        0x0B,0xA2,0x1C,0xD0,0x09,0x99,0x29,0x9B,0xC1,0x53,0xB8,0xF3,
        0x98,0xA7,0xD8,0x46,0xBE,0x5B,0xB9,0x64,0x31,0xCF,0x02,0x63,
        0x0F,0x5D,0xF2,0xBE,0xEF,0xF6,0x55,0x8B,0xFB,0xF0,0xB8,0xF7,
        0xA5,0x2E,0xD2,0x6F,0x58,0x1E,0x46,0x3F,0x74,0x3C,0x02,0x41,
        0x2F,0x65,0x53,0x7F,0x1C,0x7B,0x8A,0x72,0x22,0x1D,0x2B,0xE9,
        0xA3,0x0F,0x50,0xC3,0x13,0x12,0x6C,0xD2,0x17,0xA9,0xA5,0x82,
        0xFC,0x91,0xE3,0x3E,0x28,0x8A,0x97,0x73,
    };
    static unsigned char dh1024_g[] = {
        0x02,
    };
    DH *dh;
    if ((dh = DH_new()) == NULL) {
        return(NULL);
    }
    dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), NULL);
    dh->g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), NULL);
    if ((dh->p == NULL) || (dh->g == NULL)) {
        DH_free(dh); 
        return(NULL); 
    }
    return dh;
}

#else
void opensslDummy() {}
#endif /* BIT_PACK_OPENSSL */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ssl/nanossl.c"
 */
/************************************************************************/

/*
    nanossl.c - Mocana NanoSSL

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/

#include    "mpr.h"

#if WINDOWS
    #define __RTOS_WIN32__
#elif MACOSX
    #define __RTOS_OSX__
#elif VXWORKS
    #define __RTOS_VXWORKS__
#else
    #define __RTOS_LINUX__
#endif

#define __ENABLE_MOCANA_SSL_SERVER__        1
#define __ENABLE_MOCANA_PEM_CONVERSION__    1
#define __ENABLE_ALL_DEBUGGING__            1
#define __ENABLE_MOCANA_DEBUG_CONSOLE__     1
#define __MOCANA_DUMP_CONSOLE_TO_STDOUT__   1

#if UNUSED
    __ENABLE_MOCANA_SSL_ASYNC_SERVER_API__ 
    __ENABLE_MOCANA_SSL_ASYNC_CLIENT_API__
    __ENABLE_MOCANA_SSL_ASYNC_API_EXTENSIONS__
    __ENABLE_MOCANA_SSL_CLIENT__
#endif

#if BIT_PACK_NANOSSL
 #include "common/moptions.h"
 #include "common/mdefs.h"
 #include "common/mtypes.h"
 #include "common/merrors.h"
 #include "common/mrtos.h"
 #include "common/mtcp.h"
 #include "common/mocana.h"
 #include "common/random.h"
 #include "common/vlong.h"
 #include "crypto/hw_accel.h"
 #include "crypto/crypto.h"
 #include "crypto/pubcrypto.h"
 #include "crypto/ca_mgmt.h"
 #include "ssl/ssl.h"
 #include "asn1/oiddefs.h"

/************************************* Defines ********************************/
/*
    Per-route SSL configuration
 */
typedef struct NanoConfig {
    certDescriptor  cert;
    certDescriptor  ca;
} NanoConfig;

/*
    Per socket state
 */
typedef struct NanoSocket {
    MprSocket       *sock;
    NanoConfig       *cfg;
    sbyte4          handle;
    int             connected;
} NanoSocket;

static MprSocketProvider *nanoProvider;
static NanoConfig *defaultNanoConfig;

#if BIT_DEBUG
    #define SSL_HELLO_TIMEOUT   15000
    #define SSL_RECV_TIMEOUT    300000
#else
    #define SSL_HELLO_TIMEOUT   15000000
    #define SSL_RECV_TIMEOUT    30000000
#endif

#define KEY_SIZE                1024
#define MAX_CIPHERS             16

/***************************** Forward Declarations ***************************/

static void     nanoClose(MprSocket *sp, bool gracefully);
static void     nanoDisconnect(MprSocket *sp);
static Socket   nanoListen(MprSocket *sp, cchar *host, int port, int flags);
static void     nanoLog(sbyte4 module, sbyte4 severity, sbyte *msg);
static void     manageNanoConfig(NanoConfig *cfg, int flags);
static void     manageNanoProvider(MprSocketProvider *provider, int flags);
static void     manageNanoSocket(NanoSocket *ssp, int flags);
static ssize    nanoRead(MprSocket *sp, void *buf, ssize len);
static int      setNanoCiphers(MprSocket *sp, cchar *cipherSuite);
static int      nanoUpgrade(MprSocket *sp, MprSsl *sslConfig, cchar *peerName);
static ssize    nanoWrite(MprSocket *sp, cvoid *buf, ssize len);

static void     DEBUG_PRINT(void *where, void *msg);
static void     DEBUG_PRINTNL(void *where, void *msg);

/************************************* Code ***********************************/
/*
    Create the Openssl module. This is called only once
 */
PUBLIC int mprCreateNanoSslModule()
{
    sslSettings     *settings;

    if ((nanoProvider = mprAllocObj(MprSocketProvider, manageNanoProvider)) == NULL) {
        return MPR_ERR_MEMORY;
    }
    nanoProvider->upgradeSocket = nanoUpgrade;
    nanoProvider->closeSocket = nanoClose;
    nanoProvider->disconnectSocket = nanoDisconnect;
    nanoProvider->readSocket = nanoRead;
    nanoProvider->writeSocket = nanoWrite;
    mprAddSocketProvider("nanossl", nanoProvider);

    if ((defaultNanoConfig = mprAllocObj(NanoConfig, manageNanoConfig)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (MOCANA_initMocana() < 0) {
        mprError("NanoSSL initialization failed");
        return MPR_ERR_CANT_INITIALIZE;
    }
    MOCANA_initLog(nanoLog);
    if (SSL_init(SOMAXCONN, 0) < 0) {
        mprError("SSL_init failed");
        return MPR_ERR_CANT_INITIALIZE;
    }
    settings = SSL_sslSettings();
    settings->sslTimeOutHello = SSL_HELLO_TIMEOUT;
    settings->sslTimeOutReceive = SSL_RECV_TIMEOUT;
    return 0;
}


static void manageNanoProvider(MprSocketProvider *provider, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(provider->name);
        mprMark(defaultNanoConfig);

    } else if (flags & MPR_MANAGE_FREE) {
        defaultNanoConfig = 0;
        SSL_releaseTables();
        MOCANA_freeMocana();
    }
}


static void manageNanoConfig(NanoConfig *cfg, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        ;
    } else if (flags & MPR_MANAGE_FREE) {
        CA_MGMT_freeCertificate(&cfg->cert);
    }
}


static void manageNanoSocket(NanoSocket *np, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(np->cfg);
        mprMark(np->sock);

    } else if (flags & MPR_MANAGE_FREE) {
        if (np->handle) {
            SSL_closeConnection(np->handle);
            np->handle = 0;
        }
    }
}


static void nanoClose(MprSocket *sp, bool gracefully)
{
    NanoSocket      *np;

    np = sp->sslSocket;
    if (np->handle) {
        SSL_closeConnection(np->handle);
        np->handle = 0;
    }
    sp->service->standardProvider->closeSocket(sp, gracefully);
}


/*
    Upgrade a standard socket to use TLS
 */
static int nanoUpgrade(MprSocket *sp, MprSsl *ssl, cchar *peerName)
{
    NanoSocket   *np;
    NanoConfig   *cfg;
    int         rc;

    assert(sp);

    if (ssl == 0) {
        ssl = mprCreateSsl(sp->flags & MPR_SOCKET_SERVER);
    }
    if ((np = (NanoSocket*) mprAllocObj(NanoSocket, manageNanoSocket)) == 0) {
        return MPR_ERR_MEMORY;
    }
    np->sock = sp;
    sp->sslSocket = np;
    sp->ssl = ssl;

    lock(ssl);
    if (ssl->config && !ssl->changed) {
        np->cfg = cfg = ssl->config;
    } else {
        ssl->changed = 0;
        /*
            One time setup for the SSL configuration for this MprSsl
         */
        if ((cfg = mprAllocObj(NanoConfig, manageNanoConfig)) == 0) {
            unlock(ssl);
            return MPR_ERR_MEMORY;
        }
        if (ssl->certFile) {
            certDescriptor tmp;
            if ((rc = MOCANA_readFile((sbyte*) ssl->certFile, &tmp.pCertificate, &tmp.certLength)) < 0) {
                mprError("NanoSSL: Unable to read certificate %s", ssl->certFile); 
                CA_MGMT_freeCertificate(&tmp);
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            assert(__ENABLE_MOCANA_PEM_CONVERSION__);
            if ((rc = CA_MGMT_decodeCertificate(tmp.pCertificate, tmp.certLength, &cfg->cert.pCertificate, 
                    &cfg->cert.certLength)) < 0) {
                mprError("NanoSSL: Unable to decode PEM certificate %s", ssl->certFile); 
                CA_MGMT_freeCertificate(&tmp);
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            MOCANA_freeReadFile(&tmp.pCertificate);
        }
        if (ssl->keyFile) {
            certDescriptor tmp;
            if ((rc = MOCANA_readFile((sbyte*) ssl->keyFile, &tmp.pKeyBlob, &tmp.keyBlobLength)) < 0) {
                mprError("NanoSSL: Unable to read key file %s", ssl->keyFile); 
                CA_MGMT_freeCertificate(&cfg->cert);
                unlock(ssl);
            }
            if ((rc = CA_MGMT_convertKeyPEM(tmp.pKeyBlob, tmp.keyBlobLength, 
                    &cfg->cert.pKeyBlob, &cfg->cert.keyBlobLength)) < 0) {
                mprError("NanoSSL: Unable to decode PEM key file %s", ssl->keyFile); 
                CA_MGMT_freeCertificate(&tmp);
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            MOCANA_freeReadFile(&tmp.pKeyBlob);    
        }
        if (ssl->caFile) {
            certDescriptor tmp;
            if ((rc = MOCANA_readFile((sbyte*) ssl->caFile, &tmp.pCertificate, &tmp.certLength)) < 0) {
                mprError("NanoSSL: Unable to read CA certificate file %s", ssl->caFile); 
                CA_MGMT_freeCertificate(&tmp);
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            if ((rc = CA_MGMT_decodeCertificate(tmp.pCertificate, tmp.certLength, &cfg->ca.pCertificate, 
                    &cfg->ca.certLength)) < 0) {
                mprError("NanoSSL: Unable to decode PEM certificate %s", ssl->caFile); 
                CA_MGMT_freeCertificate(&tmp);
                unlock(ssl);
                return MPR_ERR_CANT_READ;
            }
            MOCANA_freeReadFile(&tmp.pCertificate);
        }
        if (SSL_initServerCert(&cfg->cert, FALSE, 0)) {
            mprError("SSL_initServerCert failed");
            unlock(ssl);
            return MPR_ERR_CANT_INITIALIZE;
        }
        np->cfg = ssl->config = cfg;
    }
    unlock(ssl);

    if (sp->flags & MPR_SOCKET_SERVER) {
        if ((np->handle = SSL_acceptConnection(sp->fd)) < 0) {
            return -1;
        }
    } else {
        mprError("NanoSSL does not support client side SSL");
    }
    return 0;
}


static void nanoDisconnect(MprSocket *sp)
{
    sp->service->standardProvider->disconnectSocket(sp);
}


static void checkCert(MprSocket *sp)
{
    NanoSocket  *np;
    MprCipher   *cp;
    MprSsl      *ssl;
    ubyte2      cipher;
    ubyte4      ecurve;
   
    ssl = sp->ssl;
    np = (NanoSocket*) sp->sslSocket;

    if (ssl->caFile) {
        mprLog(4, "NanoSSL: Using certificates from %s", ssl->caFile);
    } else if (ssl->caPath) {
        mprLog(4, "NanoSSL: Using certificates from directory %s", ssl->caPath);
    }
    /*
        Trace cipher being used
     */
    if (SSL_getCipherInfo(np->handle, &cipher, &ecurve) < 0) {
        mprLog(0, "Cannot get cipher info");
        return;
    }
    for (cp = mprCiphers; cp->name; cp++) {
        if (cp->code == cipher) {
            break;
        }
    }
    if (cp) {
        mprLog(0, "NanoSSL connected using cipher: %s, %x", cp->name, (int) cipher);
    } else {
        mprLog(0, "NanoSSL connected using cipher: %x", (int) cipher);
    }
}


/*
    Initiate or continue SSL handshaking with the peer. This routine does not block.
    Return -1 on errors, 0 incomplete and awaiting I/O, 1 if successful
*/
static int nanoHandshake(MprSocket *sp)
{
    NanoSocket  *np;
    ubyte4      flags;
    int         rc;

    np = (NanoSocket*) sp->sslSocket;
    sp->flags |= MPR_SOCKET_HANDSHAKING;
    if (setNanoCiphers(sp, sp->ssl->ciphers) < 0) {
        return 0;
    }
    SSL_getSessionFlags(np->handle, &flags);
    if (sp->ssl->verifyPeer) {
        flags |= SSL_FLAG_REQUIRE_MUTUAL_AUTH;
    } else {
        flags |= SSL_FLAG_NO_MUTUAL_AUTH_REQUEST;
    }
    SSL_setSessionFlags(np->handle, flags);
    rc = 0;

    while (!np->connected) {
        if ((rc = SSL_negotiateConnection(np->handle)) < 0) {
            break;
        }
        np->connected = 1;
        break;
    }
    sp->flags &= ~MPR_SOCKET_HANDSHAKING;

    /*
        Analyze the handshake result
    */
    if (rc < 0) {
        if (rc == ERR_SSL_UNKNOWN_CERTIFICATE_AUTHORITY) {
            sp->errorMsg = sclone("Unknown certificate authority");

        /* Common name mismatch, cert revoked */
        } else if (rc == ERR_SSL_PROTOCOL_PROCESS_CERTIFICATE) {
            sp->errorMsg = sclone("Bad certificate");
        } else if (rc == ERR_SSL_NO_SELF_SIGNED_CERTIFICATES) {
            sp->errorMsg = sclone("Self-signed certificate");
        } else if (rc == ERR_SSL_CERT_VALIDATION_FAILED) {
            sp->errorMsg = sclone("Certificate does not validate");
        }
        DISPLAY_ERROR(0, rc); 
        mprLog(4, "NanoSSL: Cannot handshake: error %d", rc);
        errno = EPROTO;
        return -1;
    }
    checkCert(sp);
    return 1;
}


/*
    Return the number of bytes read. Return -1 on errors and EOF. Distinguish EOF via mprIsSocketEof
 */
static ssize nanoRead(MprSocket *sp, void *buf, ssize len)
{
    NanoSocket  *np;
    sbyte4      nbytes, count;
    int         rc;

    np = (NanoSocket*) sp->sslSocket;
    assert(np);
    assert(np->cfg);

    if (!np->connected && (rc = nanoHandshake(sp)) <= 0) {
        return rc;
    }
    while (1) {
        nbytes = 0;
        rc = SSL_recv(np->handle, buf, (sbyte4) len, &nbytes, 0);
        mprLog(5, "NanoSSL: ssl_read %d", rc);
        if (rc < 0) {
            if (rc != ERR_TCP_READ_ERROR) {
                sp->flags |= MPR_SOCKET_EOF;
            }
            nbytes = -1;
        }
        break;
    }
    SSL_recvPending(np->handle, &count);
    mprHiddenSocketData(sp, count, MPR_READABLE);
    return nbytes;
}


/*
    Write data. Return the number of bytes written or -1 on errors.
 */
static ssize nanoWrite(MprSocket *sp, cvoid *buf, ssize len)
{
    NanoSocket  *np;
    ssize       totalWritten;
    int         rc, count, sent;

    np = (NanoSocket*) sp->sslSocket;
    if (len <= 0) {
        assert(0);
        return -1;
    }
    if (!np->connected && (rc = nanoHandshake(sp)) <= 0) {
        return rc;
    }
    totalWritten = 0;
    rc = 0;
    do {
        rc = sent = SSL_send(np->handle, (sbyte*) buf, (int) len);
        mprLog(7, "NanoSSL: written %d, requested len %d", sent, len);
        if (rc <= 0) {
            break;
        }
        totalWritten += sent;
        buf = (void*) ((char*) buf + sent);
        len -= sent;
        mprLog(7, "NanoSSL: write: len %d, written %d, total %d", len, sent, totalWritten);
    } while (len > 0);

    SSL_sendPending(np->handle, &count);
    mprHiddenSocketData(sp, count, MPR_WRITABLE);
    if (totalWritten == 0 && rc < 0 && errno == EAGAIN) {
        return -1;
    }
    return totalWritten;
}


static int setNanoCiphers(MprSocket *sp, cchar *cipherSuite)
{
    NanoSocket  *np;
    char        *suite, *cipher, *next;
    ubyte2      *ciphers;
    int         count, cipherCode;


    np = sp->sslSocket;
    ciphers = mprAlloc((MAX_CIPHERS + 1) * sizeof(ubyte2));

    if (!cipherSuite || cipherSuite[0] == '\0') {
        return 0;
    }
    suite = strdup(cipherSuite);
    count = 0;
    while ((cipher = stok(suite, ":, \t", &next)) != 0 && count < MAX_CIPHERS) {
        if ((cipherCode = mprGetSslCipherCode(cipher)) < 0) {
            mprError("Requested cipher %s is not supported by this provider", cipher);
        } else {
            ciphers[count++] = cipherCode;
        }
        suite = 0;
    }
    if (SSL_enableCiphers(np->handle, ciphers, count) < 0) {
        mprError("Requested cipher suite %s is not supported by this provider", cipherSuite);
        return MPR_ERR_BAD_STATE;
    }
    return 0;
}


static void DEBUG_PRINT(void *where, void *msg)
{
    mprRawLog(4, "%s", msg);
}

static void DEBUG_PRINTNL(void *where, void *msg)
{
    mprLog(4, "%s", msg);
}

static void nanoLog(sbyte4 module, sbyte4 severity, sbyte *msg)
{
    mprLog(3, "%s", (cchar*) msg);
}

#else
void nanosslDummy() {}
#endif /* BIT_PACK_NANOSSL */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */

/************************************************************************/
/*
    Start of file "src/ssl/ssl.c"
 */
/************************************************************************/

/**
    ssl.c -- Initialization for libmprssl. Load the SSL provider.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/

#include    "mpr.h"

/********************************** Globals ***********************************/

/*
    See: http://www.iana.org/assignments/tls-parameters/tls-parameters.xml
*/
MprCipher mprCiphers[] = {
    { 0x0001, "SSL_RSA_WITH_NULL_MD5" },
    { 0x0002, "SSL_RSA_WITH_NULL_SHA" },
    { 0x0004, "TLS_RSA_WITH_RC4_128_MD5" },
    { 0x0005, "TLS_RSA_WITH_RC4_128_SHA" },
    { 0x0009, "SSL_RSA_WITH_DES_CBC_SHA" },
    { 0x000A, "SSL_RSA_WITH_3DES_EDE_CBC_SHA" },
    { 0x0015, "SSL_DHE_RSA_WITH_DES_CBC_SHA" },
    { 0x0016, "SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA" },
    { 0x001A, "SSL_DH_ANON_WITH_DES_CBC_SHA" },
    { 0x001B, "SSL_DH_ANON_WITH_3DES_EDE_CBC_SHA" },
    { 0x002F, "TLS_RSA_WITH_AES_128_CBC_SHA" },
    { 0x0033, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA" },
    { 0x0034, "TLS_DH_ANON_WITH_AES_128_CBC_SHA" },
    { 0x0035, "TLS_RSA_WITH_AES_256_CBC_SHA" },
    { 0x0039, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA" },
    { 0x003A, "TLS_DH_ANON_WITH_AES_256_CBC_SHA" },
    { 0x003B, "SSL_RSA_WITH_NULL_SHA256" },
    { 0x003C, "TLS_RSA_WITH_AES_128_CBC_SHA256" },
    { 0x003D, "TLS_RSA_WITH_AES_256_CBC_SHA256" },
    { 0x0041, "TLS_RSA_WITH_CAMELLIA_128_CBC_SHA" },
    { 0x0067, "TLS_DHE_RSA_WITH_AES_128_CBC_SHA256" },
    { 0x006B, "TLS_DHE_RSA_WITH_AES_256_CBC_SHA256" },
    { 0x006C, "TLS_DH_ANON_WITH_AES_128_CBC_SHA256" },
    { 0x006D, "TLS_DH_ANON_WITH_AES_256_CBC_SHA256" },
    { 0x0084, "TLS_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA" },
    { 0x0088, "TLS_RSA_WITH_CAMELLIA_256_CBC_SHA" },
    { 0x008B, "TLS_PSK_WITH_3DES_EDE_CBC_SHA" },
    { 0x008C, "TLS_PSK_WITH_AES_128_CBC_SHA" },
    { 0x008D, "TLS_PSK_WITH_AES_256_CBC_SHA" },
    { 0x008F, "SSL_DHE_PSK_WITH_3DES_EDE_CBC_SHA" },
    { 0x0090, "TLS_DHE_PSK_WITH_AES_128_CBC_SHA" },
    { 0x0091, "TLS_DHE_PSK_WITH_AES_256_CBC_SHA" },
    { 0x0093, "TLS_RSA_PSK_WITH_3DES_EDE_CBC_SHA" },
    { 0x0094, "TLS_RSA_PSK_WITH_AES_128_CBC_SHA" },
    { 0x0095, "TLS_RSA_PSK_WITH_AES_256_CBC_SHA" },
    { 0xC001, "TLS_ECDH_ECDSA_WITH_NULL_SHA" },
    { 0xC003, "SSL_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA" },
    { 0xC004, "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA" },
    { 0xC005, "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA" },
    { 0xC006, "TLS_ECDHE_ECDSA_WITH_NULL_SHA" },
    { 0xC008, "SSL_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA" },
    { 0xC009, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA" },
    { 0xC00A, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA" },
    { 0xC00B, "TLS_ECDH_RSA_WITH_NULL_SHA" },
    { 0xC00D, "SSL_ECDH_RSA_WITH_3DES_EDE_CBC_SHA" },
    { 0xC00E, "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA" },
    { 0xC00F, "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA" },
    { 0xC010, "TLS_ECDHE_RSA_WITH_NULL_SHA" },
    { 0xC012, "SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA" },
    { 0xC013, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA" },
    { 0xC014, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA" },
    { 0xC015, "TLS_ECDH_anon_WITH_NULL_SHA" },
    { 0xC017, "SSL_ECDH_anon_WITH_3DES_EDE_CBC_SHA" },
    { 0xC018, "TLS_ECDH_anon_WITH_AES_128_CBC_SHA" },
    { 0xC019, "TLS_ECDH_anon_WITH_AES_256_CBC_SHA " },
    { 0xC023, "TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256" },
    { 0xC024, "TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384" },
    { 0xC025, "TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256" },
    { 0xC026, "TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384" },
    { 0xC027, "TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256" },
    { 0xC028, "TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384" },
    { 0xC029, "TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256" },
    { 0xC02A, "TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384" },
    { 0xC02B, "TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256" },
    { 0xC02C, "TLS_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384" },
    { 0xC02D, "TLS_ECDH_ECDSA_WITH_AES_128_GCM_SHA256" },
    { 0xC02E, "TLS_ECDH_ECDSA_WITH_AES_256_GCM_SHA384" },
    { 0xC02F, "TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256" },
    { 0xC030, "TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384" },
    { 0xC031, "TLS_ECDH_RSA_WITH_AES_128_GCM_SHA256" },
    { 0xC032, "TLS_ECDH_RSA_WITH_AES_256_GCM_SHA384" },
    { 0xFFF0, "TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8" },
    { 0x0, 0 },
};

/************************************ Code ************************************/
/*
    Module initialization entry point
 */
PUBLIC int mprSslInit(void *unused, MprModule *module)
{
#if BIT_PACK_SSL
    assert(module);

    /*
        Order matters. The last enabled stack becomes the default.
     */
#if BIT_PACK_MATRIXSSL
    if (mprCreateMatrixSslModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->sslProvider = sclone("matrixssl");
#endif
#if BIT_PACK_NANOSSL
    if (mprCreateNanoSslModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->sslProvider = sclone("nanossl");
#endif
#if BIT_PACK_OPENSSL
    if (mprCreateOpenSslModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->sslProvider = sclone("openssl");
#endif
#if BIT_PACK_EST
    if (mprCreateEstModule() < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    MPR->socketService->sslProvider = sclone("est");
#endif
    return 0;
#else
    return MPR_ERR_BAD_STATE;
#endif /* BLD_PACK_SSL */
}


PUBLIC cchar *mprGetSslCipherName(int code) 
{
    MprCipher   *cp;

    for (cp = mprCiphers; cp->name; cp++) {
        if (cp->code == code) {
            return cp->name;
        }
    }
    return 0;
}


PUBLIC int mprGetSslCipherCode(cchar *cipher) 
{
    MprCipher   *cp;

    for (cp = mprCiphers; cp->name; cp++) {
        if (smatch(cp->name, cipher)) {
            return cp->code;
        }
    }
    return 0;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2014. All Rights Reserved.

    This software is distributed under commercial and open source licenses.
    You may use the Embedthis Open Source license or you may acquire a 
    commercial license from Embedthis Software. You agree to be fully bound
    by the terms of either license. Consult the LICENSE.md distributed with
    this software for full details and other copyrights.

    Local variables:
    tab-width: 4
    c-basic-offset: 4
    End:
    vim: sw=4 ts=4 expandtab

    @end
 */
