/*
    httpLib.c -- Http Library Library Source

    This file is a catenation of all the source code. Amalgamating into a
    single file makes embedding simpler and the resulting application faster.

    Prepared by: magnetar.local
 */

#include "http.h"

/************************************************************************/
/*
    Start of file "src/actionHandler.c"
 */
/************************************************************************/

/*
    actionHandler.c -- Action handler

    This handler maps URIs to actions that are C functions that have been registered via httpDefineAction.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Code *************************************/

static void startAction(HttpQueue *q)
{
    HttpConn    *conn;
    HttpAction     action;
    cchar       *name;

    mprLog(5, "Start actionHandler");
    conn = q->conn;
    assert(!conn->error);
    assert(!conn->tx->finalized);

    name = conn->rx->pathInfo;
    if ((action = mprLookupKey(conn->tx->handler->stageData, name)) == 0) {
        mprError("Cannot find action: %s", name);
    } else {
        (*action)(conn);
    }
}


PUBLIC void httpDefineAction(cchar *name, HttpAction action)
{
    HttpStage   *stage;

    if ((stage = httpLookupStage(MPR->httpService, "actionHandler")) == 0) {
        mprError("Cannot find actionHandler");
        return;
    }
    mprAddKey(stage->stageData, name, action);
}


PUBLIC int httpOpenActionHandler(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateHandler(http, "actionHandler", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->actionHandler = stage;
    if ((stage->stageData = mprCreateHash(0, MPR_HASH_STATIC_VALUES)) == 0) {
        return MPR_ERR_MEMORY;
    }
    stage->start = startAction;
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/auth.c"
 */
/************************************************************************/

/*
    auth.c - Authorization and access management

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************* Forwards ***********************************/

static void computeAbilities(HttpAuth *auth, MprHash *abilities, cchar *role);
static void manageAuth(HttpAuth *auth, int flags);
static void manageRole(HttpRole *role, int flags);
static void manageUser(HttpUser *user, int flags);
static void formLogin(HttpConn *conn);
static bool internalVerifyUser(HttpConn *conn);

/*********************************** Code *************************************/

PUBLIC void httpInitAuth(Http *http)
{
    httpAddAuthType(http, "basic", httpBasicLogin, httpBasicParse, httpBasicSetHeaders);
    httpAddAuthType(http, "digest", httpDigestLogin, httpDigestParse, httpDigestSetHeaders);
    httpAddAuthType(http, "form", formLogin, NULL, NULL);

#if BIT_HAS_PAM && BIT_HTTP_PAM
    /*
        Pam must be actively selected during configuration
     */
    httpAddAuthStore(http, "pam", httpPamVerifyUser);
#endif
    httpAddAuthStore(http, "internal", internalVerifyUser);
}


PUBLIC int httpAuthenticate(HttpConn *conn)
{
    HttpRx      *rx;
    HttpAuth    *auth;
    HttpRoute   *route;
    HttpSession *session;
    cchar       *version;
    bool        cached;

    rx = conn->rx;
    if (rx->flags & HTTP_AUTH_CHECKED) {
        return rx->authenticated;
    }
    rx->flags |= HTTP_AUTH_CHECKED;

    route = rx->route;
    auth = route->auth;
    assert(auth);
    mprLog(5, "Checking user authentication user %s on route %s", conn->username, route->name);

    cached = 0;
    if (rx->cookie && (session = httpGetSession(conn, 0)) != 0) {
        if ((conn->username = (char*) httpGetSessionVar(conn, HTTP_SESSION_USERNAME, 0)) != 0) {
            version = httpGetSessionVar(conn, HTTP_SESSION_AUTHVER, 0);
            if (stoi(version) == auth->version) {
                mprLog(5, "Using cached authentication data for user %s", conn->username);
                cached = 1;
            }
        }
    }
    if (!cached) {
        if (conn->authType && !smatch(conn->authType, auth->type->name)) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Access denied. Wrong authentication protocol type.");
            return 0;
        }
        if (rx->authDetails && (auth->type->parseAuth)(conn) < 0) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Access denied. Bad authentication data.");
            return 0;
        }
        if (!conn->username) {
            return 0;
        }
        if (!(auth->store->verifyUser)(conn)) {
            return 0;
        }
        /*
            Store authentication state and user in session storage
         */
        if ((session = httpCreateSession(conn)) != 0) {
            httpSetSessionVar(conn, HTTP_SESSION_AUTHVER, itos(auth->version));
            httpSetSessionVar(conn, HTTP_SESSION_USERNAME, conn->username);
        }
    }
    rx->authenticated = 1;
    return 1;
}


PUBLIC bool httpCanUser(HttpConn *conn)
{
    HttpAuth    *auth;
    MprKey      *kp;

    auth = conn->rx->route->auth;
    if (auth->permittedUsers && !mprLookupKey(auth->permittedUsers, conn->username)) {
        mprLog(2, "User \"%s\" is not specified as a permitted user to access %s", conn->username, conn->rx->pathInfo);
        return 0;
    }
    if (!auth->requiredAbilities) {
        /* No abilities are required */
        return 1;
    }
    if (!conn->username) {
        /* User not authenticated */
        return 0;
    }
    if (!conn->user) {
        if (auth->users == 0 || (conn->user = mprLookupKey(auth->users, conn->username)) == 0) {
            mprLog(2, "Cannot find user %s", conn->username);
            return 0;
        }
    }
    for (ITERATE_KEYS(auth->requiredAbilities, kp)) {
        if (!mprLookupKey(conn->user->abilities, kp->key)) {
            mprLog(2, "User \"%s\" does not possess the required ability: \"%s\" to access %s", 
                conn->username, kp->key, conn->rx->pathInfo);
            return 0;
        }
    }
    return 1;
}


PUBLIC bool httpLogin(HttpConn *conn, cchar *username, cchar *password)
{
    HttpAuth    *auth;
    HttpSession *session;

    auth = conn->rx->route->auth;
    if (!username || !*username) {
        mprTrace(5, "httpLogin missing username");
        return 0;
    }
    conn->username = sclone(username);
    conn->password = sclone(password);
    conn->encoded = 0;
    if (!(auth->store->verifyUser)(conn)) {
        return 0;
    }
    if ((session = httpCreateSession(conn)) == 0) {
        return 0;
    } else {
        httpSetSessionVar(conn, HTTP_SESSION_AUTHVER, itos(auth->version));
        httpSetSessionVar(conn, HTTP_SESSION_USERNAME, conn->username);
    }
    return 1;
}


PUBLIC bool httpIsAuthenticated(HttpConn *conn)
{
    return conn->rx->authenticated;
}


PUBLIC HttpAuth *httpCreateAuth()
{
    HttpAuth    *auth;
    
    if ((auth = mprAllocObj(HttpAuth, manageAuth)) == 0) {
        return 0;
    }
    httpSetAuthStore(auth, "internal");
    return auth;
}


PUBLIC HttpAuth *httpCreateInheritedAuth(HttpAuth *parent)
{
    HttpAuth      *auth;

    if ((auth = mprAllocObj(HttpAuth, manageAuth)) == 0) {
        return 0;
    }
    if (parent) {
        //  OPT. Structure assignment
        auth->allow = parent->allow;
        auth->deny = parent->deny;
        auth->type = parent->type;
        auth->store = parent->store;
        auth->flags = parent->flags;
        auth->qop = parent->qop;
        auth->realm = parent->realm;
        auth->permittedUsers = parent->permittedUsers;
        auth->requiredAbilities = parent->requiredAbilities;
        auth->users = parent->users;
        auth->roles = parent->roles;
        auth->version = parent->version;
        auth->loggedIn = parent->loggedIn;
        auth->loginPage = parent->loginPage;
        auth->parent = parent;
    }
    return auth;
}


static void manageAuth(HttpAuth *auth, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(auth->allow);
        mprMark(auth->deny);
        mprMark(auth->loggedIn);
        mprMark(auth->loginPage);
        mprMark(auth->permittedUsers);
        mprMark(auth->qop);
        mprMark(auth->realm);
        mprMark(auth->requiredAbilities);
        mprMark(auth->store);
        mprMark(auth->type);
        mprMark(auth->users);
        mprMark(auth->roles);
    }
}


static void manageAuthType(HttpAuthType *type, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(type->name);
    }
}


PUBLIC int httpAddAuthType(Http *http, cchar *name, HttpAskLogin askLogin, HttpParseAuth parseAuth, HttpSetAuth setAuth)
{
    HttpAuthType    *type;

    if ((type = mprAllocObj(HttpAuthType, manageAuthType)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    type->name = sclone(name);
    type->askLogin = askLogin;
    type->parseAuth = parseAuth;
    type->setAuth = setAuth;

    if (mprAddKey(http->authTypes, name, type) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


static void manageAuthStore(HttpAuthStore *store, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(store->name);
    }
}


/*
    Add a password store backend
 */
PUBLIC int httpAddAuthStore(Http *http, cchar *name, HttpVerifyUser verifyUser)
{
    HttpAuthStore    *store;

    if ((store = mprAllocObj(HttpAuthStore, manageAuthStore)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    store->name = sclone(name);
    store->verifyUser = verifyUser;
    if (mprAddKey(http->authStores, name, store) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    return 0;
}


PUBLIC void httpSetAuthAllow(HttpAuth *auth, cchar *allow)
{
    if (auth->allow == 0 || (auth->parent && auth->parent->allow == auth->allow)) {
        auth->allow = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    mprAddKey(auth->allow, sclone(allow), auth);
}


PUBLIC void httpSetAuthAnyValidUser(HttpAuth *auth)
{
    auth->permittedUsers = 0;
}


PUBLIC void httpSetAuthAutoLogin(HttpAuth *auth, bool on)
{
    auth->flags &= ~HTTP_AUTO_LOGIN;
    auth->flags |= on ? HTTP_AUTO_LOGIN : 0;
}


/*
    Can supply a roles or abilities in the "abilities" parameter 
 */
PUBLIC void httpSetAuthRequiredAbilities(HttpAuth *auth, cchar *abilities)
{
    char    *ability, *tok;

    auth->requiredAbilities = mprCreateHash(0, 0);
    for (ability = stok(sclone(abilities), " \t,", &tok); abilities; abilities = stok(NULL, " \t,", &tok)) {
        computeAbilities(auth, auth->requiredAbilities, ability);
    }
}


PUBLIC void httpSetAuthDeny(HttpAuth *auth, cchar *client)
{
    if (auth->deny == 0 || (auth->parent && auth->parent->deny == auth->deny)) {
        auth->deny = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
    }
    mprAddKey(auth->deny, sclone(client), auth);
}


PUBLIC void httpSetAuthOrder(HttpAuth *auth, int order)
{
    auth->flags &= (HTTP_ALLOW_DENY | HTTP_DENY_ALLOW);
    auth->flags |= (order & (HTTP_ALLOW_DENY | HTTP_DENY_ALLOW));
}


/*
    Internal login service routine. Called in response to a form-based login request.
 */
static void loginServiceProc(HttpConn *conn)
{
    HttpAuth    *auth;
    cchar       *username, *password, *referrer;

    auth = conn->rx->route->auth;
    username = httpGetParam(conn, "username", 0);
    password = httpGetParam(conn, "password", 0);

    if (httpLogin(conn, username, password)) {
        if ((referrer = httpGetSessionVar(conn, "referrer", 0)) != 0) {
            /*
                Preserve protocol scheme from existing connection
             */
            HttpUri *where = httpCreateUri(referrer, 0);
            httpCompleteUri(where, conn->rx->parsedUri);
            referrer = httpUriToString(where, 0);
            httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, referrer);
        } else {
            if (auth->loggedIn) {
                httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, auth->loggedIn);
            } else {
                httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, httpLink(conn, "~", 0));
            }
        }
    } else {
        httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, auth->loginPage);
    }
}


static void logoutServiceProc(HttpConn *conn)
{
    HttpAuth    *auth;

    auth = conn->rx->route->auth;
    httpRemoveSessionVar(conn, HTTP_SESSION_USERNAME);
    httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, auth->loginPage);
}


PUBLIC void httpSetAuthForm(HttpRoute *parent, cchar *loginPage, cchar *loginService, cchar *logoutService, cchar *loggedIn)
{
    HttpAuth    *auth;
    HttpRoute   *route;
    bool        secure;

    secure = 0;
    auth = parent->auth;
    auth->loginPage = sclone(loginPage);
    if (loggedIn) {
        auth->loggedIn = sclone(loggedIn);
    }
    /*
        Create routes without auth for the loginPage, loginService and logoutService
     */
    if ((route = httpCreateInheritedRoute(parent)) != 0) {
        if (sstarts(loginPage, "https:///")) {
            loginPage = &loginPage[8];
            secure = 1;
        }
        httpSetRoutePattern(route, loginPage, 0);
        route->auth->type = 0;
        if (secure) {
            httpAddRouteCondition(route, "secure", 0, 0);
        }
        httpFinalizeRoute(route);
    }
    if (loginService && *loginService) {
        if (sstarts(loginService, "https:///")) {
            loginService = &loginService[8];
            secure = 1;
        }
        route = httpCreateActionRoute(parent, loginService, loginServiceProc);
        httpSetRouteMethods(route, "POST");
        route->auth->type = 0;
        if (secure) {
            httpAddRouteCondition(route, "secure", 0, 0);
        }
    }
    if (logoutService && *logoutService) {
        if (sstarts(logoutService, "https://")) {
            logoutService = &logoutService[8];
            secure = 1;
        }
        //  MOB - should be only POST
        httpSetRouteMethods(route, "GET, POST");
        route = httpCreateActionRoute(parent, logoutService, logoutServiceProc);
        route->auth->type = 0;
        if (secure) {
            httpAddRouteCondition(route, "secure", 0, 0);
        }
    }
}


PUBLIC void httpSetAuthQop(HttpAuth *auth, cchar *qop)
{
    auth->qop = sclone(qop);
}


PUBLIC void httpSetAuthRealm(HttpAuth *auth, cchar *realm)
{
    auth->realm = sclone(realm);
    auth->version = ((Http*) MPR->httpService)->nextAuth++;
}


PUBLIC void httpSetAuthPermittedUsers(HttpAuth *auth, cchar *users)
{
    char    *user, *tok;

    auth->permittedUsers = mprCreateHash(0, 0);
    for (user = stok(sclone(users), " \t,", &tok); users; users = stok(NULL, " \t,", &tok)) {
        mprAddKey(auth->permittedUsers, user, user);
    }
}


PUBLIC int httpSetAuthStore(HttpAuth *auth, cchar *store)
{
    Http    *http;

    http = MPR->httpService;
    if ((auth->store = mprLookupKey(http->authStores, store)) == 0) {
        return MPR_ERR_CANT_FIND;
    }
    if (smatch(store, "pam")) {
#if BIT_HAS_PAM && BIT_HTTP_PAM
        if (auth->type && smatch(auth->type->name, "digest")) {
            mprError("Cannot use the PAM password store with digest authentication");
            return MPR_ERR_BAD_ARGS;
        }
#else
        mprError("PAM is not supported in the current configuration");
        return MPR_ERR_BAD_ARGS;
#endif
    }
    auth->version = ((Http*) MPR->httpService)->nextAuth++;
    return 0;
}


PUBLIC int httpSetAuthType(HttpAuth *auth, cchar *type, cchar *details)
{
    Http    *http;

    http = MPR->httpService;
    if ((auth->type = mprLookupKey(http->authTypes, type)) == 0) {
        mprError("Cannot find auth type %s", type);
        return MPR_ERR_CANT_FIND;
    }
    auth->version = ((Http*) MPR->httpService)->nextAuth++;
    return 0;
}


PUBLIC HttpAuthType *httpLookupAuthType(cchar *type)
{
    Http    *http;

    http = MPR->httpService;
    return mprLookupKey(http->authTypes, type);
}


PUBLIC HttpRole *httpCreateRole(HttpAuth *auth, cchar *name, cchar *abilities)
{
    HttpRole    *role;
    char        *ability, *tok;

    if ((role = mprAllocObj(HttpRole, manageRole)) == 0) {
        return 0;
    }
    role->name = sclone(name);
    role->abilities = mprCreateHash(0, 0);
    for (ability = stok(sclone(abilities), " \t", &tok); ability; ability = stok(NULL, " \t", &tok)) {
        mprAddKey(role->abilities, ability, role);
    }
    return role;
}


static void manageRole(HttpRole *role, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(role->name);
        mprMark(role->abilities);
    }
}


PUBLIC int httpAddRole(HttpAuth *auth, cchar *name, cchar *abilities)
{
    HttpRole    *role;

    if (auth->roles == 0) {
        auth->roles = mprCreateHash(0, 0);
        auth->version = ((Http*) MPR->httpService)->nextAuth++;

    } else if (auth->parent && auth->parent->roles == auth->roles) {
        /* Inherit parent roles */
        auth->roles = mprCloneHash(auth->parent->roles);
        auth->version = ((Http*) MPR->httpService)->nextAuth++;
    }
    if (mprLookupKey(auth->roles, name)) {
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((role = httpCreateRole(auth, name, abilities)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (mprAddKey(auth->roles, name, role) == 0) {
        return MPR_ERR_MEMORY;
    }
    mprLog(5, "Role \"%s\" has abilities: %s", role->name, abilities);
    return 0;
}


PUBLIC int httpRemoveRole(HttpAuth *auth, cchar *role)
{
    if (auth->roles == 0 || !mprLookupKey(auth->roles, role)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveKey(auth->roles, role);
    return 0;
}


PUBLIC HttpUser *httpCreateUser(HttpAuth *auth, cchar *name, cchar *password, cchar *roles)
{
    HttpUser    *user;

    if ((user = mprAllocObj(HttpUser, manageUser)) == 0) {
        return 0;
    }
    user->name = sclone(name);
    user->password = sclone(password);
    if (roles) {
        user->roles = sclone(roles);
        httpComputeUserAbilities(auth, user);
    }
    return user;
}


static void manageUser(HttpUser *user, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(user->password);
        mprMark(user->name);
        mprMark(user->abilities);
        mprMark(user->roles);
    }
}


PUBLIC int httpAddUser(HttpAuth *auth, cchar *name, cchar *password, cchar *roles)
{
    HttpUser    *user;

    if (auth->users == 0) {
        auth->users = mprCreateHash(-1, 0);
        auth->version = ((Http*) MPR->httpService)->nextAuth++;

    } else if (auth->parent && auth->parent->users == auth->users) {
        auth->users = mprCloneHash(auth->parent->users);
        auth->version = ((Http*) MPR->httpService)->nextAuth++;
    }
    if (mprLookupKey(auth->users, name)) {
        return MPR_ERR_ALREADY_EXISTS;
    }
    if ((user = httpCreateUser(auth, name, password, roles)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (mprAddKey(auth->users, name, user) == 0) {
        return MPR_ERR_MEMORY;
    }
    return 0;
}


PUBLIC int httpRemoveUser(HttpAuth *auth, cchar *user)
{
    if (auth->users == 0 || !mprLookupKey(auth->users, user)) {
        return MPR_ERR_CANT_ACCESS;
    }
    mprRemoveKey(auth->users, user);
    return 0;
}


/*
    Compute the set of user abilities from the user roles. Role strings can be either roles or abilities. 
    Expand roles into the equivalent set of abilities.
 */
static void computeAbilities(HttpAuth *auth, MprHash *abilities, cchar *role)
{
    MprKey      *ap;
    HttpRole    *rp;

    if ((rp = mprLookupKey(auth->roles, role)) != 0) {
        /* Interpret as a role */
        for (ITERATE_KEYS(rp->abilities, ap)) {
            if (!mprLookupKey(abilities, ap->key)) {
                mprAddKey(abilities, ap->key, MPR->emptyString);
            }
        }
    } else {
        /* Not found as a role: Interpret role as an ability */
        mprAddKey(abilities, role, MPR->emptyString);
    }
}


/*
    Compute the set of user abilities from the user roles. User ability strings can be either roles or abilities. Expand
    roles into the equivalent set of abilities.
 */
PUBLIC void httpComputeUserAbilities(HttpAuth *auth, HttpUser *user)
{
    char        *ability, *tok;

    user->abilities = mprCreateHash(0, 0);
    for (ability = stok(sclone(user->roles), " \t,", &tok); ability; ability = stok(NULL, " \t,", &tok)) {
        computeAbilities(auth, user->abilities, ability);
    }
#if BIT_DEBUG
    {
        MprBuf *buf = mprCreateBuf(0, 0);
        MprKey *ap;
        for (ITERATE_KEYS(user->abilities, ap)) {
            mprPutToBuf(buf, "%s ", ap->key);
        }
        mprAddNullToBuf(buf);
        mprLog(5, "User \"%s\" has abilities: %s", user->name, mprGetBufStart(buf));
    }
#endif
}


PUBLIC void httpComputeAllUserAbilities(HttpAuth *auth)
{
    MprKey      *kp;
    HttpUser    *user;

    for (ITERATE_KEY_DATA(auth->users, kp, user)) {
        httpComputeUserAbilities(auth, user);
    }
}


/*
    Verify the user password based on the internal users set. This is used when not using PAM or custom verification.
 */
static bool internalVerifyUser(HttpConn *conn)
{
    HttpRx      *rx;
    HttpAuth    *auth;
    bool        success;

    rx = conn->rx;
    auth = rx->route->auth;
    if (!conn->encoded) {
        conn->password = mprGetMD5(sfmt("%s:%s:%s", conn->username, auth->realm, conn->password));
        conn->encoded = 1;
    }
    if (!conn->user && (conn->user = mprLookupKey(auth->users, conn->username)) == 0) {
        mprLog(5, "internalVerifyUser: Unknown user \"%s\" for route %s", conn->username, rx->route->name);
        return 0;
    }
    if (rx->passDigest) {
        /* Digest authentication computes a digest using the password as one ingredient */
        success = smatch(conn->password, rx->passDigest);
    } else {
        success = smatch(conn->password, conn->user->password);
    }
    if (success) {
        mprLog(5, "User \"%s\" authenticated for route %s", conn->username, rx->route->name);
    } else {
        mprLog(5, "Password for user \"%s\" failed to authenticate for route %s", conn->username, rx->route->name);
    }
    return success;
}


/*
    Web form-based authentication callback to ask the user to login via a web page
 */
static void formLogin(HttpConn *conn)
{
    httpRedirect(conn, HTTP_CODE_MOVED_TEMPORARILY, conn->rx->route->auth->loginPage);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/basic.c"
 */
/************************************************************************/

/*
    basic.c - Basic Authorization 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Code *************************************/
/*
    Parse the client 'Authorization' header and the server 'Www-Authenticate' header
 */
PUBLIC int httpBasicParse(HttpConn *conn)
{
    HttpRx  *rx;
    char    *decoded, *cp;

    if (conn->endpoint) {
        rx = conn->rx;
        if ((decoded = mprDecode64(rx->authDetails)) == 0) {
            return MPR_ERR_BAD_FORMAT;
        }
        if ((cp = strchr(decoded, ':')) != 0) {
            *cp++ = '\0';
        }
        conn->username = sclone(decoded);
        conn->password = sclone(cp);
        conn->encoded = 0;
    }
    return 0;
}


/*
    Respond to the request by asking for a client login
 */
PUBLIC void httpBasicLogin(HttpConn *conn)
{
    HttpAuth    *auth;

    auth = conn->rx->route->auth;
    httpSetHeader(conn, "WWW-Authenticate", "Basic realm=\"%s\"", auth->realm);
    httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access Denied. Login required");
}


/*
    Add the client 'Authorization' header for authenticated requests
    NOTE: Can do this without first getting a 401 response
 */
PUBLIC bool httpBasicSetHeaders(HttpConn *conn)
{
    httpAddHeader(conn, "Authorization", "basic %s", mprEncode64(sfmt("%s:%s", conn->username, conn->password)));
    return 1;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/cache.c"
 */
/************************************************************************/

/*
    cache.c -- Http request route caching 

    Caching operates as both a handler and an output filter. If acceptable cached content is found, the 
    cacheHandler will serve it instead of the normal handler. If no content is acceptable and caching is enabled
    for the request, the cacheFilter will capture and save the response.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static void cacheAtClient(HttpConn *conn);
static bool fetchCachedResponse(HttpConn *conn);
static HttpCache *lookupCacheControl(HttpConn *conn);
static char *makeCacheKey(HttpConn *conn);
static void manageHttpCache(HttpCache *cache, int flags);
static int matchCacheFilter(HttpConn *conn, HttpRoute *route, int dir);
static int matchCacheHandler(HttpConn *conn, HttpRoute *route, int dir);
static void outgoingCacheFilterService(HttpQueue *q);
static void readyCacheHandler(HttpQueue *q);
static void saveCachedResponse(HttpConn *conn);
static cchar *setHeadersFromCache(HttpConn *conn, cchar *content);

/************************************ Code ************************************/

PUBLIC int httpOpenCacheHandler(Http *http)
{
    HttpStage     *handler, *filter;

    /*
        Create the cache handler to serve cached content 
     */
    if ((handler = httpCreateHandler(http, "cacheHandler", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->cacheHandler = handler;
    handler->match = matchCacheHandler;
    handler->ready = readyCacheHandler;

    /*
        Create the cache filter to capture and cache response content
     */
    if ((filter = httpCreateFilter(http, "cacheFilter", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->cacheFilter = filter;
    filter->match = matchCacheFilter;
    filter->outgoingService = outgoingCacheFilterService;
    return 0;
}


/*
    See if there is acceptable cached content to serve
 */
static int matchCacheHandler(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpCache   *cache;

    assert(route->caching);

    if ((cache = conn->tx->cache = lookupCacheControl(conn)) == 0) {
        /* Caching not configured for this route */
        return HTTP_ROUTE_REJECT;
    }
    if (cache->flags & HTTP_CACHE_CLIENT) {
        cacheAtClient(conn);
    }
    if (cache->flags & HTTP_CACHE_SERVER) {
        if (!(cache->flags & HTTP_CACHE_MANUAL) && fetchCachedResponse(conn)) {
            /* Found cached content */
            return HTTP_ROUTE_OK;
        }
        /*
            Caching is configured but no acceptable cached content. Create a capture buffer for the cacheFilter.
         */
        conn->tx->cacheBuffer = mprCreateBuf(-1, -1);
    }
    return HTTP_ROUTE_REJECT;
}


static void readyCacheHandler(HttpQueue *q) 
{
    HttpConn    *conn;
    HttpTx      *tx;
    cchar       *data;

    conn = q->conn;
    tx = conn->tx;

    if (tx->cachedContent) {
        mprTrace(3, "cacheHandler: write cached content for '%s'", conn->rx->uri);
        if ((data = setHeadersFromCache(conn, tx->cachedContent)) != 0) {
            tx->length = slen(data);
            httpWriteString(q, data);
        }
    }
    httpFinalize(conn);
}


static int matchCacheFilter(HttpConn *conn, HttpRoute *route, int dir)
{
    if ((dir & HTTP_STAGE_TX) && conn->tx->cacheBuffer) {
        mprTrace(3, "cacheFilter: Cache response content for '%s'", conn->rx->uri);
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    This will be enabled when caching is enabled for the route and there is no acceptable cache data to use.
    OR - manual caching has been enabled.
 */
static void outgoingCacheFilterService(HttpQueue *q)
{
    HttpPacket  *packet, *data;
    HttpConn    *conn;
    HttpTx      *tx;
    MprKey      *kp;
    cchar       *cachedData;
    ssize       size;
    int         foundDataPacket;

    conn = q->conn;
    tx = conn->tx;
    foundDataPacket = 0;
    cachedData = 0;

    if (tx->status < 200 || tx->status > 299) {
        tx->cacheBuffer = 0;
    }

    /*
        This routine will save cached responses to tx->cacheBuffer.
        It will also send cached data if the X-SendCache header is present. Normal caching is done by cacheHandler
     */
    if (mprLookupKey(conn->tx->headers, "X-SendCache") != 0) {
        if (fetchCachedResponse(conn)) {
            mprLog(3, "cacheFilter: write cached content for '%s'", conn->rx->uri);
            cachedData = setHeadersFromCache(conn, tx->cachedContent);
            tx->length = slen(cachedData);
        }
    }
    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!httpWillNextQueueAcceptPacket(q, packet)) {
            httpPutBackPacket(q, packet);
            return;
        }
        if (packet->flags & HTTP_PACKET_HEADER) {
            if (!cachedData && tx->cacheBuffer) {
                /*
                    Add defined headers to the start of the cache buffer. Separate with a double newline.
                 */
                mprPutToBuf(tx->cacheBuffer, "X-Status: %d\n", tx->status);
                for (kp = 0; (kp = mprGetNextKey(tx->headers, kp)) != 0; ) {
                    mprPutToBuf(tx->cacheBuffer, "%s: %s\n", kp->key, kp->data);
                }
                mprPutCharToBuf(tx->cacheBuffer, '\n');
            }

        } else if (packet->flags & HTTP_PACKET_DATA) {
            if (cachedData) {
                /*
                    Using X-SendCache. Replace the data with the cached response.
                 */
                mprFlushBuf(packet->content);
                mprPutBlockToBuf(packet->content, cachedData, (ssize) tx->length);

            } else if (tx->cacheBuffer) {
                /*
                    Save the response packet to the cache buffer. Will write below in saveCachedResponse.
                 */
                size = mprGetBufLength(packet->content);
                if ((tx->cacheBufferLength + size) < conn->limits->cacheItemSize) {
                    mprPutBlockToBuf(tx->cacheBuffer, mprGetBufStart(packet->content), mprGetBufLength(packet->content));
                    tx->cacheBufferLength += size;
                } else {
                    tx->cacheBuffer = 0;
                    mprLog(3, "cacheFilter: Item too big to cache %d bytes, limit %d", tx->cacheBufferLength + size,
                        conn->limits->cacheItemSize);
                }
            }
            foundDataPacket = 1;

        } else if (packet->flags & HTTP_PACKET_END) {
            if (cachedData && !foundDataPacket) {
                /*
                    Using X-SendCache but there was no data packet to replace. So do the write here.
                 */
                data = httpCreateDataPacket((ssize) tx->length);
                mprPutBlockToBuf(data->content, cachedData, (ssize) tx->length);
                httpPutPacketToNext(q, data);

            } else if (tx->cacheBuffer) {
                /*
                    Save the cache buffer to the cache store
                 */
                saveCachedResponse(conn);
            }
        }
        httpPutPacketToNext(q, packet);
    }
}


/*
    Find a qualifying cache control entry. Any configured uri,method,extension,type must match.
 */
static HttpCache *lookupCacheControl(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpCache   *cache;
    cchar       *mimeType, *ukey;
    int         next;

    rx = conn->rx;
    tx = conn->tx;

    /*
        Find first qualifying cache control entry. Any configured uri,method,extension,type must match.
     */
    for (next = 0; (cache = mprGetNextItem(rx->route->caching, &next)) != 0; ) {
        if (cache->uris) {
            if (cache->flags & HTTP_CACHE_ONLY) {
                ukey = sfmt("%s?%s", rx->pathInfo, httpGetParamsString(conn));
            } else {
                ukey = rx->pathInfo;
            }
            if (!mprLookupKey(cache->uris, ukey)) {
                continue;
            }
        }
        if (cache->methods && !mprLookupKey(cache->methods, rx->method)) {
            continue;
        }
        if (cache->extensions && !mprLookupKey(cache->extensions, tx->ext)) {
            continue;
        }
        if (cache->types) {
            if ((mimeType = (char*) mprLookupMime(rx->route->mimeTypes, tx->ext)) == 0) {
                continue;
            }
            if (!mprLookupKey(cache->types, mimeType)) {
                continue;
            }
        }
        /* All match */
        break;
    }
    return cache;
}


static void cacheAtClient(HttpConn *conn)
{
    HttpTx      *tx;
    HttpCache   *cache;
    cchar       *value;

    tx = conn->tx;
    cache = conn->tx->cache;

    if (!mprLookupKey(tx->headers, "Cache-Control")) {
        if ((value = mprLookupKey(conn->tx->headers, "Cache-Control")) != 0) {
            if (strstr(value, "max-age") == 0) {
                httpAppendHeader(conn, "Cache-Control", "max-age=%d", cache->clientLifespan / MPR_TICKS_PER_SEC);
            }
        } else {
            httpAddHeader(conn, "Cache-Control", "max-age=%d", cache->clientLifespan / MPR_TICKS_PER_SEC);
        }
#if KEEP
        {
            /* Old HTTP/1.0 clients don't understand Cache-Control */
            struct tm   tm;
            mprDecodeUniversalTime(&tm, conn->http->now + (expires * MPR_TICKS_PER_SEC));
            httpAddHeader(conn, "Expires", "%s", mprFormatTime(MPR_HTTP_DATE, &tm));
        }
#endif
    }
}


/*
    See if there is acceptable cached content for this request. If so, return true.
    Will setup tx->cacheBuffer as a side-effect if the output should be captured and cached.
 */
static bool fetchCachedResponse(HttpConn *conn)
{
    HttpTx      *tx;
    MprTime     modified, when;
    cchar       *value, *key, *tag;
    int         status, cacheOk, canUseClientCache;

    tx = conn->tx;

    /*
        Transparent caching. Manual caching must manually call httpWriteCached()
     */
    key = makeCacheKey(conn);
    if ((value = httpGetHeader(conn, "Cache-Control")) != 0 && 
            (scontains(value, "max-age=0") == 0 || scontains(value, "no-cache") == 0)) {
        mprLog(3, "Client reload. Cache-control header '%s' rejects use of cached content.", value);

    } else if ((tx->cachedContent = mprReadCache(conn->host->responseCache, key, &modified, 0)) != 0) {
        /*
            See if a NotModified response can be served. This is much faster than sending the response.
            Observe headers:
                If-None-Match: "ec18d-54-4d706a63"
                If-Modified-Since: Fri, 04 Mar 2013 04:28:19 GMT
            Set status to OK when content must be transmitted.
         */
        cacheOk = 1;
        canUseClientCache = 0;
        tag = mprGetMD5(key);
        if ((value = httpGetHeader(conn, "If-None-Match")) != 0) {
            canUseClientCache = 1;
            if (scmp(value, tag) != 0) {
                cacheOk = 0;
            }
        }
        if (cacheOk && (value = httpGetHeader(conn, "If-Modified-Since")) != 0) {
            canUseClientCache = 1;
            mprParseTime(&when, value, 0, 0);
            if (modified > when) {
                cacheOk = 0;
            }
        }
        status = (canUseClientCache && cacheOk) ? HTTP_CODE_NOT_MODIFIED : HTTP_CODE_OK;
        mprLog(3, "cacheHandler: Use cached content for %s, status %d", key, status);
        httpSetStatus(conn, status);
        httpSetHeader(conn, "Etag", mprGetMD5(key));
        httpSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
        return 1;
    }
    mprLog(3, "cacheHandler: No cached content for %s", key);
    return 0;
}


static void saveCachedResponse(HttpConn *conn)
{
    HttpTx      *tx;
    MprBuf      *buf;
    MprTime     modified;

    tx = conn->tx;
    assert(tx->finalizedOutput && tx->cacheBuffer);

    buf = tx->cacheBuffer;
    mprAddNullToBuf(buf);
    tx->cacheBuffer = 0;
    /* 
        Truncate modified time to get a 1 sec resolution. This is the resolution for If-Modified headers.  
     */
    modified = mprGetTime() / MPR_TICKS_PER_SEC * MPR_TICKS_PER_SEC;
    mprWriteCache(conn->host->responseCache, makeCacheKey(conn), mprGetBufStart(buf), modified, 
        tx->cache->serverLifespan, 0, 0);
}


PUBLIC ssize httpWriteCached(HttpConn *conn)
{
    MprTime     modified;
    cchar       *cacheKey, *data, *content;

    if (!conn->tx->cache) {
        return MPR_ERR_CANT_FIND;
    }
    cacheKey = makeCacheKey(conn);
    if ((content = mprReadCache(conn->host->responseCache, cacheKey, &modified, 0)) == 0) {
        mprLog(3, "No cached data for ", cacheKey);
        return 0;
    }
    mprLog(5, "Used cached ", cacheKey);
    data = setHeadersFromCache(conn, content);
    httpSetHeader(conn, "Etag", mprGetMD5(cacheKey));
    httpSetHeader(conn, "Last-Modified", mprFormatUniversalTime(MPR_HTTP_DATE, modified));
    conn->tx->cacheBuffer = 0;
    httpWriteString(conn->writeq, data);
    httpFinalize(conn);
    return slen(data);
}


PUBLIC ssize httpUpdateCache(HttpConn *conn, cchar *uri, cchar *data, MprTicks lifespan)
{
    cchar   *key;
    ssize   len;

    len = slen(data);
    if (len > conn->limits->cacheItemSize) {
        return MPR_ERR_WONT_FIT;
    }
    if (lifespan <= 0) {
        lifespan = conn->rx->route->lifespan;
    }
    key = sfmt("http::response-%s", uri);
    if (data == 0 || lifespan <= 0) {
        mprRemoveCache(conn->host->responseCache, key);
        return 0;
    }
    return mprWriteCache(conn->host->responseCache, key, data, 0, lifespan, 0, 0);
}


/*
    Add cache configuration to the route. This can be called multiple times.
    Uris, extensions and methods may optionally provide a space or comma separated list of items.
    If URI is NULL or "*", cache all URIs for this route. Otherwise, cache only the given URIs. 
    The URIs may contain an ordered set of request parameters. For example: "/user/show?name=john&posts=true"
    Note: the URI should not include the route prefix (scriptName)
    The extensions should not contain ".". The methods may contain "*" for all methods.
 */
PUBLIC void httpAddCache(HttpRoute *route, cchar *methods, cchar *uris, cchar *extensions, cchar *types, 
        MprTicks clientLifespan, MprTicks serverLifespan, int flags)
{
    HttpCache   *cache;
    char        *item, *tok;

    cache = 0;
    if (!route->caching) {
        httpAddRouteHandler(route, "cacheHandler", "");
        httpAddRouteFilter(route, "cacheFilter", "", HTTP_STAGE_TX);
        route->caching = mprCreateList(0, 0);

    } else if (flags & HTTP_CACHE_RESET) {
        route->caching = mprCreateList(0, 0);

    } else if (route->parent && route->caching == route->parent->caching) {
        route->caching = mprCloneList(route->parent->caching);
    }
    if ((cache = mprAllocObj(HttpCache, manageHttpCache)) == 0) {
        return;
    }
    if (extensions) {
        cache->extensions = mprCreateHash(0, 0);
        for (item = stok(sclone(extensions), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                extensions = 0;
            } else {
                mprAddKey(cache->extensions, item, cache);
            }
        }
    } else if (types) {
        cache->types = mprCreateHash(0, 0);
        for (item = stok(sclone(types), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                extensions = 0;
            } else {
                mprAddKey(cache->types, item, cache);
            }
        }
    }
    if (methods) {
        cache->methods = mprCreateHash(0, MPR_HASH_CASELESS);
        for (item = stok(sclone(methods), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (smatch(item, "*")) {
                methods = 0;
            } else {
                mprAddKey(cache->methods, item, cache);
            }
        }
    }
    if (uris) {
        cache->uris = mprCreateHash(0, 0);
        for (item = stok(sclone(uris), " \t,", &tok); item; item = stok(0, " \t,", &tok)) {
            if (flags & HTTP_CACHE_ONLY && route->prefix && !scontains(item, sfmt("prefix=%s", route->prefix))) {
                /*
                    Auto-add ?prefix=ROUTE_NAME if there is no query
                 */
                if (!schr(item, '?')) {
                    item = sfmt("%s?prefix=%s", item, route->prefix); 
                }
            }
            mprAddKey(cache->uris, item, cache);
        }
    }
    if (clientLifespan <= 0) {
        clientLifespan = route->lifespan;
    }
    cache->clientLifespan = clientLifespan;
    if (serverLifespan <= 0) {
        serverLifespan = route->lifespan;
    }
    cache->serverLifespan = serverLifespan;
    cache->flags = flags;
    mprAddItem(route->caching, cache);

#if KEEP
    mprTrace(3, "Caching route %s for methods %s, URIs %s, extensions %s, types %s, client lifespan %d, server lifespan %d", 
        route->name,
        (methods) ? methods: "*",
        (uris) ? uris: "*",
        (extensions) ? extensions: "*",
        (types) ? types: "*",
        cache->clientLifespan / MPR_TICKS_PER_SEC);
        cache->serverLifespan / MPR_TICKS_PER_SEC);
#endif
}


static void manageHttpCache(HttpCache *cache, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(cache->extensions);
        mprMark(cache->methods);
        mprMark(cache->types);
        mprMark(cache->uris);
    }
}


static char *makeCacheKey(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (conn->tx->cache->flags & (HTTP_CACHE_ONLY | HTTP_CACHE_UNIQUE)) {
        return sfmt("http::response-%s?%s", rx->pathInfo, httpGetParamsString(conn));
    } else {
        return sfmt("http::response-%s", rx->pathInfo);
    }
}


/*
    Parse cached content of the form:  headers \n\n data
    Set headers in the current requeset and return a reference to the data portion
 */
static cchar *setHeadersFromCache(HttpConn *conn, cchar *content)
{
    cchar   *data;
    char    *header, *headers, *key, *value, *tok;

    if ((data = strstr(content, "\n\n")) == 0) {
        data = content;
    } else {
        headers = snclone(content, data - content);
        data += 2;
        for (header = stok(headers, "\n", &tok); header; header = stok(NULL, "\n", &tok)) {
            key = stok(header, ": ", &value);
            if (smatch(key, "X-Status")) {
                conn->tx->status = (int) stoi(value);
            } else {
                httpAddHeader(conn, key, value);
            }
        }
    }
    return data;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/chunkFilter.c"
 */
/************************************************************************/

/*
    chunkFilter.c - Transfer chunk endociding filter.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static int matchChunk(HttpConn *conn, HttpRoute *route, int dir);
static void openChunk(HttpQueue *q);
static void outgoingChunkService(HttpQueue *q);
static void setChunkPrefix(HttpQueue *q, HttpPacket *packet);

/*********************************** Code *************************************/
/* 
   Loadable module initialization
 */
PUBLIC int httpOpenChunkFilter(Http *http)
{
    HttpStage     *filter;

    mprTrace(5, "Open chunk filter");
    if ((filter = httpCreateFilter(http, "chunkFilter", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->chunkFilter = filter;
    filter->match = matchChunk; 
    filter->open = openChunk; 
    filter->outgoingService = outgoingChunkService; 
    return 0;
}


/*
    This is called twice: once for TX and once for RX
 */
static int matchChunk(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpTx  *tx;

    tx = conn->tx;

    if (conn->upgraded || (!conn->endpoint && tx->parsedUri && tx->parsedUri->webSockets)) {
        return HTTP_ROUTE_REJECT;
    }
    if (dir & HTTP_STAGE_TX) {
        /* 
            If content length is defined, don't need chunking. Also disable chunking if explicitly turned off vi 
            the X_APPWEB_CHUNK_SIZE header which may set the chunk size to zero.
         */
        if (tx->length >= 0 || tx->chunkSize == 0) {
            return HTTP_ROUTE_REJECT;
        }
        return HTTP_ROUTE_OK;
    } else {
        return HTTP_ROUTE_OK;
    }
}


static void openChunk(HttpQueue *q)
{
    HttpConn    *conn;

    conn = q->conn;
    q->packetSize = min(conn->limits->bufferSize, q->max);
}


/*  
    Filter chunk headers and leave behind pure data. This is called for chunked and unchunked data.
    Chunked data format is:
        Chunk spec <CRLF>
        Data <CRLF>
        Chunk spec (size == 0) <CRLF>
        <CRLF>
    Chunk spec is: "HEX_COUNT; chunk length DECIMAL_COUNT\r\n". The "; chunk length DECIMAL_COUNT is optional.
    As an optimization, use "\r\nSIZE ...\r\n" as the delimiter so that the CRLF after data does not special consideration.
    Achive this by parseHeaders reversing the input start by 2.

    Return number of bytes available to read.
    NOTE: may set rx->eof and return 0 bytes on EOF.
 */
PUBLIC ssize httpFilterChunkData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *buf;
    ssize       chunkSize;
    char        *start, *cp;
    int         bad;

    conn = q->conn;
    rx = conn->rx;
    assert(packet);
    buf = packet->content;
    assert(buf);

    switch (rx->chunkState) {
    case HTTP_CHUNK_UNCHUNKED:
        assert(0);
        return -1;

    case HTTP_CHUNK_DATA:
        mprTrace(7, "chunkFilter: data %d bytes, rx->remainingContent %d", 
            httpGetPacketLength(packet), rx->remainingContent);
        if (rx->remainingContent > 0) {
            return (ssize) min(rx->remainingContent, mprGetBufLength(buf));
        }
        /* End of chunk - prep for the next chunk */
        rx->remainingContent = BIT_MAX_BUFFER;
        rx->chunkState = HTTP_CHUNK_START;
        /* Fall through */

    case HTTP_CHUNK_START:
        /*  
            Validate:  "\r\nSIZE.*\r\n"
         */
        if (mprGetBufLength(buf) < 5) {
            return MPR_ERR_NOT_READY;
        }
        start = mprGetBufStart(buf);
        bad = (start[0] != '\r' || start[1] != '\n');
        for (cp = &start[2]; cp < buf->end && *cp != '\n'; cp++) {}
        if (*cp != '\n' && (cp - start) < 80) {
            return MPR_ERR_NOT_READY;
        }
        bad += (cp[-1] != '\r' || cp[0] != '\n');
        if (bad) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return 0;
        }
        chunkSize = (int) stoiradix(&start[2], 16, NULL);
        if (!isxdigit((uchar) start[2]) || chunkSize < 0) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad chunk specification");
            return 0;
        }
        if (chunkSize == 0) {
            /*
                Last chunk. Consume the final "\r\n".
             */
            if ((cp + 2) >= buf->end) {
                return MPR_ERR_NOT_READY;
            }
            cp += 2;
            bad += (cp[-1] != '\r' || cp[0] != '\n');
            if (bad) {
                httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad final chunk specification");
                return 0;
            }
        }
        mprAdjustBufStart(buf, (cp - start + 1));
        /* Remaining content is set to the next chunk size */
        rx->remainingContent = chunkSize;
        rx->chunkState = (chunkSize == 0) ? HTTP_CHUNK_EOF : HTTP_CHUNK_DATA;
        mprTrace(7, "chunkFilter: start incoming chunk of %d bytes", chunkSize);
        return min(chunkSize, mprGetBufLength(buf));

    default:
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad chunk state %d", rx->chunkState);
    }
    return 0;
}


static void outgoingChunkService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet, *finalChunk;
    HttpTx      *tx;
    cchar       *value;

    conn = q->conn;
    tx = conn->tx;

    if (!(q->flags & HTTP_QUEUE_SERVICED)) {
        /*
            If we don't know the content length (tx->length < 0) and if the last packet is the end packet. Then
            we have all the data. Thus we can determine the actual content length and can bypass the chunk handler.
         */
        if (tx->length < 0 && (value = mprLookupKey(tx->headers, "Content-Length")) != 0) {
            tx->length = stoi(value);
        }
        if (tx->length < 0 && tx->chunkSize < 0) {
            if (q->last->flags & HTTP_PACKET_END) {
                if (q->count > 0) {
                    tx->length = q->count;
                }
            } else {
                tx->chunkSize = min(conn->limits->chunkSize, q->max);
            }
        }
        if (tx->flags & HTTP_TX_USE_OWN_HEADERS) {
            tx->chunkSize = -1;
        }
    }
    if (tx->chunkSize <= 0 || conn->upgraded) {
        httpDefaultOutgoingServiceStage(q);
    } else {
        for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
            if (packet->flags & HTTP_PACKET_DATA) {
                httpPutBackPacket(q, packet);
                httpJoinPackets(q, tx->chunkSize);
                packet = httpGetPacket(q);
                if (httpGetPacketLength(packet) > tx->chunkSize) {
                    httpResizePacket(q, packet, tx->chunkSize);
                }
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            if (packet->flags & HTTP_PACKET_DATA) {
                setChunkPrefix(q, packet);

            } else if (packet->flags & HTTP_PACKET_END) {
                /* Insert a packet for the final chunk */
                finalChunk = httpCreateDataPacket(0);
                setChunkPrefix(q, finalChunk);
                httpPutPacketToNext(q, finalChunk);
            }
            httpPutPacketToNext(q, packet);
        }
    }
}


static void setChunkPrefix(HttpQueue *q, HttpPacket *packet)
{
    if (packet->prefix) {
        return;
    }
    packet->prefix = mprCreateBuf(32, 32);
    /*  
        NOTE: prefixes don't count in the queue length. No need to adjust q->count
     */
    if (httpGetPacketLength(packet)) {
        mprPutToBuf(packet->prefix, "\r\n%x\r\n", httpGetPacketLength(packet));
    } else {
        mprPutStringToBuf(packet->prefix, "\r\n0\r\n\r\n");
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/client.c"
 */
/************************************************************************/

/*
    client.c -- Client side specific support.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************* Forwards ***********************************/

static void setDefaultHeaders(HttpConn *conn);

/*********************************** Code *************************************/

static HttpConn *openConnection(HttpConn *conn, struct MprSsl *ssl)
{
    Http        *http;
    HttpUri     *uri;
    MprSocket   *sp;
    char        *ip;
    int         port, rc, level;

    assert(conn);

    http = conn->http;
    uri = conn->tx->parsedUri;

    if (!uri->host) {
        ip = (http->proxyHost) ? http->proxyHost : http->defaultClientHost;
        port = (http->proxyHost) ? http->proxyPort : http->defaultClientPort;
    } else {
        ip = (http->proxyHost) ? http->proxyHost : uri->host;
        port = (http->proxyHost) ? http->proxyPort : uri->port;
    }
    if (port == 0) {
        port = (uri->secure) ? 443 : 80;
    }
    if (conn && conn->sock) {
        if (--conn->keepAliveCount < 0 || port != conn->port || strcmp(ip, conn->ip) != 0 || 
                uri->secure != (conn->sock->ssl != 0) || conn->sock->ssl != ssl) {
            httpCloseConn(conn);
        } else {
            mprLog(4, "Http: reusing keep-alive socket on: %s:%d", ip, port);
        }
    }
    if (conn->sock) {
        return conn;
    }
    if ((sp = mprCreateSocket()) == 0) {
        httpError(conn, HTTP_CODE_COMMS_ERROR, "Cannot create socket for %s", uri->uri);
        return 0;
    }
    if ((rc = mprConnectSocket(sp, ip, port, MPR_SOCKET_NODELAY)) < 0) {
        httpError(conn, HTTP_CODE_COMMS_ERROR, "Cannot open socket on %s:%d", ip, port);
        return 0;
    }
    conn->sock = sp;
    conn->ip = sclone(ip);
    conn->port = port;
    conn->secure = uri->secure;
    conn->keepAliveCount = (conn->limits->keepAliveMax) ? conn->limits->keepAliveMax : -1;

#if BIT_SSL
    /* Must be done even if using keep alive for repeat SSL requests */
    if (uri->secure) {
        char *peerName;
        if (ssl == 0) {
            ssl = mprCreateSsl(0);
        }
        peerName = isdigit(uri->host[0]) ? 0 : uri->host;
        if (mprUpgradeSocket(sp, ssl, peerName) < 0) {
            conn->errorMsg = sp->errorMsg;
            mprLog(4, "Cannot upgrade socket for SSL: %s", conn->errorMsg);
            return 0;
        }
    }
#endif
    if (uri->webSockets && httpUpgradeWebSocket(conn) < 0) {
        conn->errorMsg = sp->errorMsg;
        return 0;
    }
    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_CONN, NULL)) >= 0) {
        mprLog(level, "### Outgoing connection from %s:%d to %s:%d", 
            conn->ip, conn->port, conn->sock->ip, conn->sock->port);
    }
    return conn;
}


static void setDefaultHeaders(HttpConn *conn)
{
    HttpAuthType    *ap;

    assert(conn);

    if (smatch(conn->protocol, "HTTP/1.0")) {
        conn->http10 = 1;
    }
    if (conn->username && conn->authType) {
        if ((ap = httpLookupAuthType(conn->authType)) != 0) {
            if ((ap->setAuth)(conn)) {
                conn->authRequested = 1;
            }
        }
    }
    if (conn->port != 80) {
        httpAddHeader(conn, "Host", "%s:%d", conn->ip, conn->port);
    } else {
        httpAddHeaderString(conn, "Host", conn->ip);
    }
    if (conn->keepAliveCount > 0) {
        httpSetHeaderString(conn, "Connection", "Keep-Alive");
    } else {
        httpSetHeaderString(conn, "Connection", "close");
    }
}


PUBLIC int httpConnect(HttpConn *conn, cchar *method, cchar *uri, struct MprSsl *ssl)
{
    assert(conn);
    assert(method && *method);
    assert(uri && *uri);

    if (conn->endpoint) {
        httpError(conn, HTTP_CODE_BAD_GATEWAY, "Cannot call connect in a server");
        return MPR_ERR_BAD_STATE;
    }
    mprLog(4, "Http: client request: %s %s", method, uri);

    if (conn->tx == 0 || conn->state != HTTP_STATE_BEGIN) {
        /* WARNING: this will erase headers */
        httpPrepClientConn(conn, 0);
    }
    assert(conn->state == HTTP_STATE_BEGIN);
    httpSetState(conn, HTTP_STATE_CONNECTED);
    conn->authRequested = 0;
    conn->tx->method = supper(method);
    conn->tx->parsedUri = httpCreateUri(uri, HTTP_COMPLETE_URI_PATH);
#if BIT_DEBUG
    conn->startMark = mprGetHiResTicks();
#endif
    /*
        The receive pipeline is created when parsing the response in parseIncoming()
     */
    httpCreateTxPipeline(conn, conn->http->clientRoute);
    if (openConnection(conn, ssl) == 0) {
        return MPR_ERR_CANT_OPEN;
    }
    setDefaultHeaders(conn);
    if (conn->upgraded) {
        /* Push out headers */
        httpServiceQueues(conn);
    }
    return 0;
}


/*  
    Check the response for authentication failures and redirections. Return true if a retry is requried.
 */
PUBLIC bool httpNeedRetry(HttpConn *conn, char **url)
{
    HttpAuthType    *authType;
    HttpRx          *rx;

    assert(conn->rx);

    *url = 0;
    rx = conn->rx;

    if (conn->state < HTTP_STATE_FIRST) {
        return 0;
    }
    if (rx->status == HTTP_CODE_UNAUTHORIZED) {
        if (conn->username == 0 || conn->authType == 0) {
            httpFormatError(conn, rx->status, "Authentication required");
        } else if (conn->authRequested) {
            httpFormatError(conn, rx->status, "Authentication failed");
        } else {
            if (conn->authType && (authType = httpLookupAuthType(conn->authType)) != 0) {
                (authType->parseAuth)(conn);
            }
            return 1;
        }
    } else if (HTTP_CODE_MOVED_PERMANENTLY <= rx->status && rx->status <= HTTP_CODE_MOVED_TEMPORARILY && 
            conn->followRedirects) {
        if (rx->redirect) {
            *url = rx->redirect;
            return 1;
        }
        httpFormatError(conn, rx->status, "Missing location header");
        return -1;
    }
    return 0;
}


/*  
    Set the request as being a multipart mime upload. This defines the content type and defines a multipart mime boundary
 */
PUBLIC void httpEnableUpload(HttpConn *conn)
{
    conn->boundary = sfmt("--BOUNDARY--%Ld", conn->http->now);
    httpSetHeader(conn, "Content-Type", "multipart/form-data; boundary=%s", &conn->boundary[2]);
}


static int blockingFileCopy(HttpConn *conn, cchar *path)
{
    MprFile     *file;
    char        buf[BIT_MAX_BUFFER];
    ssize       bytes, nbytes, offset;

    file = mprOpenFile(path, O_RDONLY | O_BINARY, 0);
    if (file == 0) {
        mprError("Cannot open %s", path);
        return MPR_ERR_CANT_OPEN;
    }
    mprAddRoot(file);
    while ((bytes = mprReadFile(file, buf, sizeof(buf))) > 0) {
        offset = 0;
        while (bytes > 0) {
            if ((nbytes = httpWriteBlock(conn->writeq, &buf[offset], bytes, HTTP_BLOCK)) < 0) {
                mprCloseFile(file);
                mprRemoveRoot(file);
                return MPR_ERR_CANT_WRITE;
            }
            bytes -= nbytes;
            offset += nbytes;
            assert(bytes >= 0);
        }
        mprYield(0);
    }
    httpFlushQueue(conn->writeq, 1);
    mprCloseFile(file);
    mprRemoveRoot(file);
    return 0;
}


/*  
    Write upload data. This routine blocks. If you need non-blocking ... cut and paste.
 */
PUBLIC ssize httpWriteUploadData(HttpConn *conn, MprList *fileData, MprList *formData)
{
    char    *path, *pair, *key, *value, *name;
    cchar   *type;
    ssize   rc;
    int     next;

    rc = 0;
    if (formData) {
        for (rc = next = 0; rc >= 0 && (pair = mprGetNextItem(formData, &next)) != 0; ) {
            key = stok(sclone(pair), "=", &value);
            rc += httpWrite(conn->writeq, "%s\r\nContent-Disposition: form-data; name=\"%s\";\r\n", conn->boundary, key);
            rc += httpWrite(conn->writeq, "Content-Type: application/x-www-form-urlencoded\r\n\r\n%s\r\n", value);
        }
    }
    if (fileData) {
        for (rc = next = 0; rc >= 0 && (path = mprGetNextItem(fileData, &next)) != 0; ) {
            if (!mprPathExists(path, R_OK)) {
                httpFormatError(conn, 0, "Cannot open %s", path);
                return MPR_ERR_CANT_OPEN;
            }
            name = mprGetPathBase(path);
            rc += httpWrite(conn->writeq, "%s\r\nContent-Disposition: form-data; name=\"file%d\"; filename=\"%s\"\r\n", 
                conn->boundary, next - 1, name);
            if ((type = mprLookupMime(MPR->mimeTypes, path)) != 0) {
                rc += httpWrite(conn->writeq, "Content-Type: %s\r\n", mprLookupMime(MPR->mimeTypes, path));
            }
            httpWrite(conn->writeq, "\r\n");
            if (blockingFileCopy(conn, path) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            rc += httpWrite(conn->writeq, "\r\n");
        }
    }
    rc += httpWrite(conn->writeq, "%s--\r\n--", conn->boundary);
    return rc;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/conn.c"
 */
/************************************************************************/

/*
    conn.c -- Connection module to handle individual HTTP connections.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/***************************** Forward Declarations ***************************/

static void manageConn(HttpConn *conn, int flags);
static HttpPacket *getPacket(HttpConn *conn, ssize *bytesToRead);
static bool prepForNext(HttpConn *conn);
static void readEvent(HttpConn *conn);
static void writeEvent(HttpConn *conn);

/*********************************** Code *************************************/
/*
    Create a new connection object
 */
PUBLIC HttpConn *httpCreateConn(Http *http, HttpEndpoint *endpoint, MprDispatcher *dispatcher)
{
    HttpConn    *conn;
    HttpHost    *host;
    HttpRoute   *route;

    if ((conn = mprAllocObj(HttpConn, manageConn)) == 0) {
        return 0;
    }
    conn->http = http;
    conn->protocol = http->protocol;
    conn->port = -1;
    conn->retries = HTTP_RETRIES;
    conn->endpoint = endpoint;
    conn->lastActivity = http->now;
    conn->ioCallback = httpEvent;

    if (endpoint) {
        conn->notifier = endpoint->notifier;
        host = mprGetFirstItem(endpoint->hosts);
        if (host && (route = host->defaultRoute) != 0) {
            conn->limits = route->limits;
            conn->trace[0] = route->trace[0];
            conn->trace[1] = route->trace[1];
        } else {
            conn->limits = http->serverLimits;
            httpInitTrace(conn->trace);
        }
    } else {
        conn->limits = http->clientLimits;
        httpInitTrace(conn->trace);
    }
    conn->keepAliveCount = conn->limits->keepAliveMax;
    conn->serviceq = httpCreateQueueHead(conn, "serviceq");

    if (dispatcher) {
        conn->dispatcher = dispatcher;
    } else if (endpoint) {
        conn->dispatcher = endpoint->dispatcher;
    } else {
        conn->dispatcher = mprGetDispatcher();
    }
    httpSetState(conn, HTTP_STATE_BEGIN);
    httpAddConn(http, conn);
    return conn;
}


/*
    Destroy a connection. This removes the connection from the list of connections. Should GC after that.
 */
PUBLIC void httpDestroyConn(HttpConn *conn)
{
    if (conn->http) {
        assert(conn->http);
        HTTP_NOTIFY(conn, HTTP_EVENT_DESTROY, 0);
        httpRemoveConn(conn->http, conn);
        if (conn->endpoint) {
            if (conn->rx) {
                httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_REQUEST, conn);
            }
            httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_CONN, conn);
        }
        conn->input = 0;
        if (conn->tx) {
            httpDestroyPipeline(conn);
            conn->tx->conn = 0;
            conn->tx = 0;
        }
        if (conn->rx) {
            conn->rx->conn = 0;
            conn->rx = 0;
        }
        httpCloseConn(conn);
        conn->http = 0;
    }
    if (conn->dispatcher->flags & MPR_DISPATCHER_AUTO_CREATE) {
        mprDisableDispatcher(conn->dispatcher);
    }
}


static void manageConn(HttpConn *conn, int flags)
{
    assert(conn);

    if (flags & MPR_MANAGE_MARK) {
        mprMark(conn->rx);
        mprMark(conn->tx);
        mprMark(conn->endpoint);
        mprMark(conn->host);
        mprMark(conn->limits);
        mprMark(conn->http);
        mprMark(conn->dispatcher);
        mprMark(conn->newDispatcher);
        mprMark(conn->oldDispatcher);
        mprMark(conn->sock);
        mprMark(conn->serviceq);
        mprMark(conn->currentq);
        mprMark(conn->input);
        mprMark(conn->readq);
        mprMark(conn->writeq);
        mprMark(conn->connectorq);
        mprMark(conn->timeoutEvent);
        mprMark(conn->workerEvent);
        mprMark(conn->context);
        mprMark(conn->ejs);
        mprMark(conn->pool);
        mprMark(conn->mark);
        mprMark(conn->data);
        mprMark(conn->grid);
        mprMark(conn->record);
        mprMark(conn->boundary);
        mprMark(conn->errorMsg);
        mprMark(conn->ip);
        mprMark(conn->protocol);
        mprMark(conn->protocols);
        httpManageTrace(&conn->trace[0], flags);
        httpManageTrace(&conn->trace[1], flags);
        mprMark(conn->headersCallbackArg);

        mprMark(conn->authType);
        mprMark(conn->authData);
        mprMark(conn->username);
        mprMark(conn->password);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyConn(conn);
    }
}


/*  
    Close the connection but don't destroy the conn object.
 */
PUBLIC void httpCloseConn(HttpConn *conn)
{
    assert(conn);

    if (conn->sock) {
        mprLog(5, "Closing connection");
        mprCloseSocket(conn->sock, 0);
        conn->sock = 0;
    }
}


PUBLIC void httpConnTimeout(HttpConn *conn)
{
    HttpLimits  *limits;
    MprTicks    now;

    if (!conn->http) {
        return;
    }
    now = conn->http->now;
    limits = conn->limits;
    assert(limits);
    mprLog(5, "Inactive connection timed out");

    if (conn->timeoutCallback) {
        (conn->timeoutCallback)(conn);
    }
    if (!conn->connError) {
        if (HTTP_STATE_CONNECTED < conn->state && conn->state < HTTP_STATE_PARSED && 
                (conn->started + limits->requestParseTimeout) < now) {
            httpError(conn, HTTP_CODE_REQUEST_TIMEOUT, "Exceeded parse headers timeout of %Ld sec", 
                limits->requestParseTimeout  / 1000);
            if (conn->rx) {
                mprTrace(2, "  State %d, uri %s", conn->state, conn->rx->uri);
            }
        } else {
            if ((conn->lastActivity + limits->inactivityTimeout) < now) {
                if (conn->state > HTTP_STATE_BEGIN) {
                    httpError(conn, HTTP_CODE_REQUEST_TIMEOUT,
                        "Exceeded inactivity timeout of %Ld sec", limits->inactivityTimeout / 1000);
                    if (conn->rx) {
                        mprTrace(2, "  State %d, uri %s", conn->state, conn->rx->uri);
                    }
                }

            } else if ((conn->started + limits->requestTimeout) < now) {
                httpError(conn, HTTP_CODE_REQUEST_TIMEOUT, "Exceeded timeout %d sec", limits->requestTimeout / 1000);
                if (conn->rx) {
                    mprTrace(2, "  State %d, uri %s", conn->state, conn->rx->uri);
                }
            }
        }
    }
    if (conn->endpoint) {
        httpDestroyConn(conn);
    } else {
        httpDisconnect(conn);
    }
}


static void commonPrep(HttpConn *conn)
{
    if (conn->timeoutEvent) {
        mprRemoveEvent(conn->timeoutEvent);
        conn->timeoutEvent = 0;
    }
    conn->lastActivity = conn->http->now;
    conn->error = 0;
    conn->errorMsg = 0;
    conn->state = 0;
    conn->authRequested = 0;

    if (conn->endpoint) {
        conn->authType = 0;
        conn->username = 0;
        conn->password = 0;
        conn->user = 0;
        conn->authData = 0;
        conn->encoded = 0;
    }
    httpSetState(conn, HTTP_STATE_BEGIN);
    httpInitSchedulerQueue(conn->serviceq);
}


/*
    Prepare for another request
    Return true if there is another request ready for serving
 */
static bool prepForNext(HttpConn *conn)
{
    assert(conn->endpoint);
    assert(conn->state == HTTP_STATE_COMPLETE);
    if (conn->tx) {
        assert(conn->tx->finalized && conn->tx->finalizedConnector && conn->tx->finalizedOutput);
        conn->tx->conn = 0;
    }
    if (conn->rx) {
        conn->rx->conn = 0;
    }
    conn->rx = 0;
    conn->tx = 0;
    conn->readq = 0;
    conn->writeq = 0;
    commonPrep(conn);
    assert(conn->state == HTTP_STATE_BEGIN);
    return conn->input && (httpGetPacketLength(conn->input) > 0) && !conn->connError;
}


PUBLIC void httpConsumeLastRequest(HttpConn *conn)
{
    MprTicks    mark;
    char        junk[4096];

    if (!conn->sock) {
        return;
    }
    if (conn->state >= HTTP_STATE_FIRST) {
        mark = conn->http->now;
        while (!httpIsEof(conn) && mprGetRemainingTicks(mark, conn->limits->requestTimeout) > 0) {
            if (httpRead(conn, junk, sizeof(junk)) <= 0) {
                break;
            }
        }
    }
    if (HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_COMPLETE) {
        conn->keepAliveCount = -1;
    }
}
 

PUBLIC void httpPrepClientConn(HttpConn *conn, bool keepHeaders)
{
    MprHash     *headers;

    assert(conn);
    conn->connError = 0;
    if (conn->keepAliveCount >= 0 && conn->sock) {
        /* Eat remaining input incase last request did not consume all data */
        httpConsumeLastRequest(conn);
    } else {
        conn->input = 0;
    }
    conn->input = 0;
    if (conn->tx) {
        conn->tx->conn = 0;
    }
    if (conn->rx) {
        conn->rx->conn = 0;
    }
    headers = (keepHeaders && conn->tx) ? conn->tx->headers: NULL;
    conn->tx = httpCreateTx(conn, headers);
    conn->rx = httpCreateRx(conn);
    commonPrep(conn);
}


PUBLIC void httpCallEvent(HttpConn *conn, int mask)
{
    MprEvent    e;

    if (conn->http) {
        e.mask = mask;
        e.timestamp = conn->http->now;
        httpEvent(conn, &e);
    }
}


PUBLIC void httpAfterEvent(HttpConn *conn)
{
    if (conn->endpoint) {
        if (conn->keepAliveCount < 0 && (conn->state < HTTP_STATE_PARSED || conn->state == HTTP_STATE_COMPLETE)) {
            httpDestroyConn(conn);
            return;
        } else if (conn->state == HTTP_STATE_COMPLETE) {
            prepForNext(conn);
        }
    } else if (mprIsSocketEof(conn->sock)) {
        return;
    }
    if (!conn->state != HTTP_STATE_RUNNING) {
        httpEnableConnEvents(conn);
    }
}


/*  
    IO event handler. This is invoked by the wait subsystem in response to I/O events. It is also invoked via 
    relay when an accept event is received by the server. Initially the conn->dispatcher will be set to the
    server->dispatcher and the first I/O event will be handled on the server thread (or main thread). A request handler
    may create a new conn->dispatcher and transfer execution to a worker thread if required.
 */
PUBLIC void httpEvent(HttpConn *conn, MprEvent *event)
{
    assert(conn->sock);
    mprTrace(6, "httpEvent for fd %d, mask %d", conn->sock->fd, event->mask);
    conn->lastActivity = conn->http->now;

    if (event->mask & MPR_WRITABLE) {
        writeEvent(conn);
    }
    if (event->mask & MPR_READABLE) {
        readEvent(conn);
    }
    httpAfterEvent(conn);
}


static void readEvent(HttpConn *conn)
{
    HttpPacket  *packet;
    ssize       nbytes, size;

    if ((packet = getPacket(conn, &size)) == 0) {
        return;
    }
    assert(conn->input == packet);

    nbytes = mprReadSocket(conn->sock, mprGetBufEnd(packet->content), size);
    mprTrace(7, "http: read event. Got %d", nbytes);

    if (nbytes > 0) {
        mprAdjustBufEnd(packet->content, nbytes);

    } else if (nbytes == 0) {
        return;

    } else if (nbytes < 0 && mprIsSocketEof(conn->sock)) {
        conn->errorMsg = conn->sock->errorMsg;
        conn->keepAliveCount = -1;
        if (conn->state < HTTP_STATE_PARSED || conn->state == HTTP_STATE_COMPLETE) {
            return;
        }
    }
    do {
        if (!httpPumpRequest(conn, conn->input)) {
            break;
        }
    } while (conn->endpoint && prepForNext(conn));
}


static void writeEvent(HttpConn *conn)
{
    mprTrace(6, "httpProcessWriteEvent, state %d", conn->state);

    if (conn->tx) {
        conn->tx->writeBlocked = 0;
        httpResumeQueue(conn->connectorq);
        httpServiceQueues(conn);
        httpPumpRequest(conn, NULL);
    }
}


PUBLIC void httpUseWorker(HttpConn *conn, MprDispatcher *dispatcher, MprEvent *event)
{
    lock(conn->http);
    conn->oldDispatcher = conn->dispatcher;
    conn->dispatcher = dispatcher;
    conn->worker = 1;
    assert(!conn->workerEvent);
    conn->workerEvent = event;
    unlock(conn->http);
}


PUBLIC void httpUsePrimary(HttpConn *conn)
{
    lock(conn->http);
    assert(conn->worker);
    assert(conn->state == HTTP_STATE_BEGIN);
    assert(conn->oldDispatcher && conn->dispatcher != conn->oldDispatcher);
    conn->dispatcher = conn->oldDispatcher;
    conn->oldDispatcher = 0;
    conn->worker = 0;
    unlock(conn->http);
}


/*
    Steal a connection with open socket from Http and disconnect it from management by Http.
    It is the callers responsibility to call mprCloseSocket when required.
 */
PUBLIC MprSocket *httpStealConn(HttpConn *conn)
{
    MprSocket   *sock;

    sock = conn->sock;
    conn->sock = 0;

    mprRemoveSocketHandler(conn->sock);
    if (conn->http) {
        lock(conn->http);
        httpRemoveConn(conn->http, conn);
        httpDiscardData(conn, HTTP_QUEUE_TX);
        httpDiscardData(conn, HTTP_QUEUE_RX);
        httpSetState(conn, HTTP_STATE_COMPLETE);
        unlock(conn->http);
    }
    return sock;
}


PUBLIC void httpEnableConnEvents(HttpConn *conn)
{
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    MprEvent    *event;
    MprSocket   *sp;
    int         eventMask;

    mprTrace(7, "EnableConnEvents");
    sp = conn->sock;
    if (!conn->async || !sp) {
        return;
    }
    tx = conn->tx;
    rx = conn->rx;
    eventMask = 0;
    conn->lastActivity = conn->http->now;

    if (conn->workerEvent) {
        event = conn->workerEvent;
        conn->workerEvent = 0;
        mprQueueEvent(conn->dispatcher, event);

    } else {
        if (tx) {
            /*
                Three cases for writable:
                - Connector is blocked on a write
                - The connector queue has data and is not doing a SSL handshake
                - The SSL stack has buffered write data that needs to be sent
             */
            if (tx->writeBlocked || (conn->connectorq && conn->connectorq->count > 0 && !mprSocketHandshaking(sp)) || 
                    mprSocketHasBufferedWrite(sp)) {
                eventMask |= MPR_WRITABLE;
            }
            /*
                Cases for readable: (Not eof) and ...
                - Room in the read queue
                - Reading a form 
                - Buffered data in the SSL stack
             */
            q = conn->readq;
            if (!rx->eof && (q->count < q->max || rx->form || mprSocketHasBufferedRead(sp))) {
                eventMask |= MPR_READABLE;
            }
        } else {
            eventMask |= MPR_READABLE;
        }
        httpSetupWaitHandler(conn, eventMask);
    }
    if (tx && tx->handler && tx->handler->module) {
        tx->handler->module->lastActivity = conn->lastActivity;
    }
}


PUBLIC void httpSetupWaitHandler(HttpConn *conn, int eventMask)
{
    MprSocket   *sp;

    sp = conn->sock;
    if (sp == 0) {
        return;
    }
    if (eventMask) {
        if (sp->handler == 0) {
            mprAddSocketHandler(sp, eventMask, conn->dispatcher, conn->ioCallback, conn, 0);
        } else {
            sp->handler->dispatcher = conn->dispatcher;
            mprWaitOn(sp->handler, eventMask);
        }
    } else if (sp->handler) {
        mprWaitOn(sp->handler, eventMask);
    }
}


PUBLIC void httpFollowRedirects(HttpConn *conn, bool follow)
{
    conn->followRedirects = follow;
}


/*  
    Get the packet into which to read data. Return in *size the length of data to attempt to read.
 */
static HttpPacket *getPacket(HttpConn *conn, ssize *size)
{
    HttpPacket  *packet;
    MprBuf      *content;

    if ((packet = conn->input) == NULL) {
        conn->input = packet = httpCreateDataPacket(BIT_MAX_BUFFER);
    } else {
        content = packet->content;
        mprResetBufIfEmpty(content);
        mprAddNullToBuf(content);
        if (mprGetBufSpace(content) < BIT_MAX_BUFFER && mprGrowBuf(content, BIT_MAX_BUFFER) < 0) {
            return 0;
        }
    }
    *size = mprGetBufSpace(packet->content);
    assert(*size > 0);
    return packet;
}


PUBLIC int httpGetAsync(HttpConn *conn)
{
    return conn->async;
}


PUBLIC ssize httpGetChunkSize(HttpConn *conn)
{
    if (conn->tx) {
        return conn->tx->chunkSize;
    }
    return 0;
}


PUBLIC void *httpGetConnContext(HttpConn *conn)
{
    return conn->context;
}


PUBLIC void *httpGetConnHost(HttpConn *conn)
{
    return conn->host;
}


PUBLIC ssize httpGetWriteQueueCount(HttpConn *conn)
{
    return conn->writeq ? conn->writeq->count : 0;
}


PUBLIC void httpResetCredentials(HttpConn *conn)
{
    conn->authType = 0;
    conn->username = 0;
    conn->password = 0;
    httpRemoveHeader(conn, "Authorization");
}


PUBLIC void httpSetAsync(HttpConn *conn, int enable)
{
    conn->async = (enable) ? 1 : 0;
}


PUBLIC void httpSetConnNotifier(HttpConn *conn, HttpNotifier notifier)
{
    conn->notifier = notifier;
}


/*
    password and authType can be null
    User may be a combined user:password
 */
PUBLIC void httpSetCredentials(HttpConn *conn, cchar *username, cchar *password, cchar *authType)
{
    httpResetCredentials(conn);
    conn->username = sclone(username);
    if (password == NULL && strchr(username, ':') != 0) {
        conn->username = stok(conn->username, ":", &conn->password);
        conn->password = sclone(conn->password);
    } else {
        conn->password = sclone(password);
    }
    if (authType) {
        conn->authType = sclone(authType);
    }
}


PUBLIC void httpSetKeepAliveCount(HttpConn *conn, int count)
{
    conn->keepAliveCount = count;
}


PUBLIC void httpSetChunkSize(HttpConn *conn, ssize size)
{
    if (conn->tx) {
        conn->tx->chunkSize = size;
    }
}


PUBLIC void httpSetHeadersCallback(HttpConn *conn, HttpHeadersCallback fn, void *arg)
{
    conn->headersCallback = fn;
    conn->headersCallbackArg = arg;
}


PUBLIC void httpSetIOCallback(HttpConn *conn, HttpIOCallback fn)
{
    conn->ioCallback = fn;
}


PUBLIC void httpSetConnContext(HttpConn *conn, void *context)
{
    conn->context = context;
}


PUBLIC void httpSetConnHost(HttpConn *conn, void *host)
{
    conn->host = host;
}


/*  
    Set the protocol to use for outbound requests
 */
PUBLIC void httpSetProtocol(HttpConn *conn, cchar *protocol)
{
    if (conn->state < HTTP_STATE_CONNECTED) {
        conn->protocol = sclone(protocol);
    }
}


PUBLIC void httpSetRetries(HttpConn *conn, int count)
{
    conn->retries = count;
}


PUBLIC void httpSetState(HttpConn *conn, int targetState)
{
    int     state;

    if (targetState == conn->state) {
        return;
    }
    if (targetState < conn->state) {
        /* Prevent regressions */
        return;
    }
    for (state = conn->state + 1; state <= targetState; state++) { 
        conn->state = state;
        HTTP_NOTIFY(conn, HTTP_EVENT_STATE, state);
    }
}


#if BIT_MPR_TRACING
static char *events[] = {
    "undefined", "state-change", "readable", "writable", "error", "destroy", "app-open", "app-close",
};
static char *states[] = {
    "undefined", "begin", "connected", "first", "parsed", "content", "ready", "running", "finalized", "complete",
};
#endif

PUBLIC void httpNotify(HttpConn *conn, int event, int arg)
{
    if (conn->notifier) {
        if (MPR->logLevel >= 6) {
            if (event == HTTP_EVENT_STATE) {
                mprTrace(6, "Event: change to state \"%s\"", states[conn->state]);
            } else if (event < 0 || event > HTTP_EVENT_MAX) {
                mprTrace(6, "Event: \"%d\" in state \"%s\"", event, states[conn->state]);
            } else {
                mprTrace(6, "Event: \"%s\" in state \"%s\"", events[event], states[conn->state]);
            }
        }
        (conn->notifier)(conn, event, arg);
    }
}


/*
    Set each timeout arg to -1 to skip. Set to zero for no timeout. Otherwise set to number of msecs
 */
PUBLIC void httpSetTimeout(HttpConn *conn, MprTicks requestTimeout, MprTicks inactivityTimeout)
{
    if (requestTimeout >= 0) {
        if (requestTimeout == 0) {
            conn->limits->requestTimeout = MAXINT;
        } else {
            conn->limits->requestTimeout = requestTimeout;
        }
    }
    if (inactivityTimeout >= 0) {
        if (inactivityTimeout == 0) {
            conn->limits->inactivityTimeout = MAXINT;
        } else {
            conn->limits->inactivityTimeout = inactivityTimeout;
        }
    }
}


PUBLIC HttpLimits *httpSetUniqueConnLimits(HttpConn *conn)
{
    HttpLimits      *limits;

    if ((limits = mprAllocStruct(HttpLimits)) != 0) {
        *limits = *conn->limits;
        conn->limits = limits;
    }
    return limits;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/digest.c"
 */
/************************************************************************/

/*
    digest.c - Digest Authorization

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Defines ***********************************/
/*
    Per-request digest authorization data
 */
typedef struct DigestData 
{
    char    *algorithm;
    char    *cnonce;
    char    *domain;
    char    *nc;
    char    *nonce;
    char    *opaque;
    char    *qop;
    char    *realm;
    char    *stale;
    char    *uri;
} DigestData;

/********************************** Forwards **********************************/

static char *calcDigest(HttpConn *conn, DigestData *dp);
static char *createDigestNonce(HttpConn *conn, cchar *secret, cchar *realm);
static void manageDigestData(DigestData *dp, int flags);
static int parseDigestNonce(char *nonce, cchar **secret, cchar **realm, MprTime *when);

/*********************************** Code *************************************/
/*
    Parse the client 'Authorization' header and the server 'Www-Authenticate' header
 */
PUBLIC int httpDigestParse(HttpConn *conn)
{
    HttpRx      *rx;
    DigestData  *dp;
    MprTime     when;
    char        *value, *tok, *key, *cp, *sp;
    cchar       *secret, *realm;
    int         seenComma;

    dp = conn->authData = mprAllocObj(DigestData, manageDigestData);
    rx = conn->rx;
    key = sclone(rx->authDetails);

    while (*key) {
        while (*key && isspace((uchar) *key)) {
            key++;
        }
        tok = key;
        while (*tok && !isspace((uchar) *tok) && *tok != ',' && *tok != '=') {
            tok++;
        }
        *tok++ = '\0';

        while (isspace((uchar) *tok)) {
            tok++;
        }
        seenComma = 0;
        if (*tok == '\"') {
            value = ++tok;
            while (*tok != '\"' && *tok != '\0') {
                tok++;
            }
        } else {
            value = tok;
            while (*tok != ',' && *tok != '\0') {
                tok++;
            }
            seenComma++;
        }
        *tok++ = '\0';

        /*
            Handle back-quoting
         */
        if (strchr(value, '\\')) {
            for (cp = sp = value; *sp; sp++) {
                if (*sp == '\\') {
                    sp++;
                }
                *cp++ = *sp++;
            }
            *cp = '\0';
        }

        /*
            user, response, oqaque, uri, realm, nonce, nc, cnonce, qop
         */
        switch (tolower((uchar) *key)) {
        case 'a':
            if (scaselesscmp(key, "algorithm") == 0) {
                dp->algorithm = sclone(value);
                break;
            } else if (scaselesscmp(key, "auth-param") == 0) {
                break;
            }
            break;

        case 'c':
            if (scaselesscmp(key, "cnonce") == 0) {
                dp->cnonce = sclone(value);
            }
            break;

        case 'd':
            if (scaselesscmp(key, "domain") == 0) {
                dp->domain = sclone(value);
                break;
            }
            break;

        case 'n':
            if (scaselesscmp(key, "nc") == 0) {
                dp->nc = sclone(value);
            } else if (scaselesscmp(key, "nonce") == 0) {
                dp->nonce = sclone(value);
            }
            break;

        case 'o':
            if (scaselesscmp(key, "opaque") == 0) {
                dp->opaque = sclone(value);
            }
            break;

        case 'q':
            if (scaselesscmp(key, "qop") == 0) {
                dp->qop = sclone(value);
            }
            break;

        case 'r':
            if (scaselesscmp(key, "realm") == 0) {
                dp->realm = sclone(value);
            } else if (scaselesscmp(key, "response") == 0) {
                /* Store the response digest in the password field. This is MD5(user:realm:password) */
                conn->password = sclone(value);
                conn->encoded = 1;
            }
            break;

        case 's':
            if (scaselesscmp(key, "stale") == 0) {
                break;
            }
        
        case 'u':
            if (scaselesscmp(key, "uri") == 0) {
                dp->uri = sclone(value);
            } else if (scaselesscmp(key, "username") == 0 || scaselesscmp(key, "user") == 0) {
                conn->username = sclone(value);
            }
            break;

        default:
            /*  Just ignore keywords we don't understand */
            ;
        }
        key = tok;
        if (!seenComma) {
            while (*key && *key != ',') {
                key++;
            }
            if (*key) {
                key++;
            }
        }
    }
    if (conn->username == 0 || conn->password == 0) {
        return MPR_ERR_BAD_FORMAT;
    }
    if (dp->realm == 0 || dp->nonce == 0 || dp->uri == 0) {
        return MPR_ERR_BAD_FORMAT;
    }
    if (dp->qop && (dp->cnonce == 0 || dp->nc == 0)) {
        return MPR_ERR_BAD_FORMAT;
    }
    if (conn->endpoint) {
        realm = secret = 0;
        when = 0;
        parseDigestNonce(dp->nonce, &secret, &realm, &when);
        if (!smatch(secret, secret)) {
            //  How should this be reported
            //  MOB - could this set conn->errorMsg?
            mprTrace(2, "Access denied: Nonce mismatch\n");
            return MPR_ERR_BAD_STATE;

        } else if (!smatch(realm, rx->route->auth->realm)) {
            mprTrace(2, "Access denied: Realm mismatch\n");
            return MPR_ERR_BAD_STATE;

        } else if (dp->qop && !smatch(dp->qop, "auth")) {
            mprTrace(2, "Access denied: Bad qop\n");
            return MPR_ERR_BAD_STATE;

        } else if ((when + (5 * 60)) < time(0)) {
            mprTrace(2, "Access denied: Nonce is stale\n");
            return MPR_ERR_BAD_STATE;
        }
        rx->passDigest = calcDigest(conn, dp);
    } else {
        if (dp->domain == 0 || dp->opaque == 0 || dp->algorithm == 0 || dp->stale == 0) {
            return MPR_ERR_BAD_FORMAT;
        }
    }
    return 0;
}


static void manageDigestData(DigestData *dp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(dp->algorithm);
        mprMark(dp->cnonce);
        mprMark(dp->domain);
        mprMark(dp->nc);
        mprMark(dp->nonce);
        mprMark(dp->opaque);
        mprMark(dp->qop);
        mprMark(dp->realm);
        mprMark(dp->stale);
        mprMark(dp->uri);
    }
}


/*
    Respond to the request by asking for a client login
 */
PUBLIC void httpDigestLogin(HttpConn *conn)
{
    HttpAuth    *auth;
    char        *nonce, *opaque;

    auth = conn->rx->route->auth;
    nonce = createDigestNonce(conn, conn->http->secret, auth->realm);
    /* Opaque is unused, set to anything */
    opaque = "799d5";

    if (smatch(auth->qop, "none")) {
        httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", nonce=\"%s\"", auth->realm, nonce);
    } else {
        /* Value of null defaults to "auth" */
        httpSetHeader(conn, "WWW-Authenticate", "Digest realm=\"%s\", domain=\"%s\", "
            "qop=\"auth\", nonce=\"%s\", opaque=\"%s\", algorithm=\"MD5\", stale=\"FALSE\"", 
            auth->realm, conn->host->name, nonce, opaque);
    }
    httpSetContentType(conn, "text/plain");
    httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access Denied. Login required");
}


/*
    Add the client 'Authorization' header for authenticated requests
    Must first get a 401 response to get the authData.
 */
PUBLIC bool httpDigestSetHeaders(HttpConn *conn)
{ 
    Http        *http;
    HttpTx      *tx;
    DigestData  *dp;
    char        *ha1, *ha2, *digest, *cnonce;

    http = conn->http;
    tx = conn->tx;
    if ((dp = conn->authData) == 0) {
        /* Need to await a failing auth response */
        return 0;
    }
    cnonce = sfmt("%s:%s:%x", http->secret, dp->realm, (int) http->now);
    ha1 = mprGetMD5(sfmt("%s:%s:%s", conn->username, dp->realm, conn->password));
    ha2 = mprGetMD5(sfmt("%s:%s", tx->method, tx->parsedUri->path));
    if (smatch(dp->qop, "auth")) {
        digest = mprGetMD5(sfmt("%s:%s:%08x:%s:%s:%s", ha1, dp->nonce, dp->nc, cnonce, dp->qop, ha2));
        httpAddHeader(conn, "Authorization", "Digest username=\"%s\", realm=\"%s\", domain=\"%s\", "
            "algorithm=\"MD5\", qop=\"%s\", cnonce=\"%s\", nc=\"%08x\", nonce=\"%s\", opaque=\"%s\", "
            "stale=\"FALSE\", uri=\"%s\", response=\"%s\"", conn->username, dp->realm, dp->domain, dp->qop, 
            cnonce, dp->nc, dp->nonce, dp->opaque, tx->parsedUri->path, digest);
    } else {
        digest = mprGetMD5(sfmt("%s:%s:%s", ha1, dp->nonce, ha2));
        httpAddHeader(conn, "Authorization", "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", "
            "uri=\"%s\", response=\"%s\"", conn->username, dp->realm, dp->nonce, tx->parsedUri->path, digest);
    }
    return 1;
}


/*
    Create a nonce value for digest authentication (RFC 2617)
 */ 
static char *createDigestNonce(HttpConn *conn, cchar *secret, cchar *realm)
{
    static int64 next = 0;

    assert(realm && *realm);
    return mprEncode64(sfmt("%s:%s:%Lx:%Lx", secret, realm, mprGetTime(), next++));
}


static int parseDigestNonce(char *nonce, cchar **secret, cchar **realm, MprTime *when)
{
    char    *tok, *decoded, *whenStr;

    if ((decoded = mprDecode64(nonce)) == 0) {
        return MPR_ERR_CANT_READ;
    }
    *secret = stok(decoded, ":", &tok);
    *realm = stok(NULL, ":", &tok);
    whenStr = stok(NULL, ":", &tok);
    *when = (MprTime) stoiradix(whenStr, 16, NULL); 
    return 0;
}


/*
    Get a Digest value using the MD5 algorithm -- See RFC 2617 to understand this code.
 */ 
static char *calcDigest(HttpConn *conn, DigestData *dp)
{
    HttpAuth    *auth;
    char        *digestBuf, *ha1, *ha2;

    auth = conn->rx->route->auth;
    if (!conn->user) {
        conn->user = mprLookupKey(auth->users, conn->username);
    }
    assert(conn->user && conn->user->password);
    if (conn->user == 0 || conn->user->password == 0) {
        return 0;
    }

    /*
        Compute HA1. Password is already expected to be in the HA1 format MD5(username:realm:password).
     */
    ha1 = sclone(conn->user->password);

    /*
        HA2
     */ 
    ha2 = mprGetMD5(sfmt("%s:%s", conn->rx->method, dp->uri));

    /*
        H(HA1:nonce:HA2)
     */
    if (scmp(dp->qop, "auth") == 0) {
        digestBuf = sfmt("%s:%s:%s:%s:%s:%s", ha1, dp->nonce, dp->nc, dp->cnonce, dp->qop, ha2);
    } else {
        digestBuf = sfmt("%s:%s:%s", ha1, dp->nonce, ha2);
    }
    return mprGetMD5(digestBuf);
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/endpoint.c"
 */
/************************************************************************/

/*
    endpoint.c -- Create and manage listening endpoints.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static HttpConn *acceptConn(MprSocket *sock, MprDispatcher *dispatcher, HttpEndpoint *endpoint);
static int manageEndpoint(HttpEndpoint *endpoint, int flags);
static int destroyEndpointConnections(HttpEndpoint *endpoint);

/************************************ Code ************************************/
/*
    Create a listening endpoint on ip:port. NOTE: ip may be empty which means bind to all addresses.
 */
PUBLIC HttpEndpoint *httpCreateEndpoint(cchar *ip, int port, MprDispatcher *dispatcher)
{
    HttpEndpoint    *endpoint;
    Http            *http;

    if ((endpoint = mprAllocObj(HttpEndpoint, manageEndpoint)) == 0) {
        return 0;
    }
    http = MPR->httpService;
    endpoint->http = http;
    endpoint->clientLoad = mprCreateHash(BIT_MAX_CLIENTS_HASH, MPR_HASH_STATIC_VALUES);
    endpoint->async = 1;
    endpoint->http = MPR->httpService;
    endpoint->port = port;
    endpoint->ip = sclone(ip);
    endpoint->dispatcher = dispatcher;
    endpoint->hosts = mprCreateList(-1, 0);
    endpoint->mutex = mprCreateLock();
    httpAddEndpoint(http, endpoint);
    return endpoint;
}


PUBLIC void httpDestroyEndpoint(HttpEndpoint *endpoint)
{
    destroyEndpointConnections(endpoint);
    if (endpoint->sock) {
        mprCloseSocket(endpoint->sock, 0);
        endpoint->sock = 0;
    }
    httpRemoveEndpoint(MPR->httpService, endpoint);
}


static int manageEndpoint(HttpEndpoint *endpoint, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(endpoint->http);
        mprMark(endpoint->hosts);
        mprMark(endpoint->limits);
        mprMark(endpoint->clientLoad);
        mprMark(endpoint->ip);
        mprMark(endpoint->context);
        mprMark(endpoint->sock);
        mprMark(endpoint->dispatcher);
        mprMark(endpoint->ssl);
        mprMark(endpoint->mutex);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyEndpoint(endpoint);
    }
    return 0;
}


/*  
    Convenience function to create and configure a new endpoint without using a config file.
 */
PUBLIC HttpEndpoint *httpCreateConfiguredEndpoint(cchar *home, cchar *documents, cchar *ip, int port)
{
    Http            *http;
    HttpHost        *host;
    HttpEndpoint    *endpoint;
    HttpRoute       *route;

    http = MPR->httpService;

    if (ip == 0 && port <= 0) {
        /*  
            If no IP:PORT specified, find the first endpoint
         */
        if ((endpoint = mprGetFirstItem(http->endpoints)) != 0) {
            ip = endpoint->ip;
            port = endpoint->port;
        } else {
            ip = "localhost";
            if (port <= 0) {
                port = BIT_HTTP_PORT;
            }
            if ((endpoint = httpCreateEndpoint(ip, port, NULL)) == 0) {
                return 0;
            }
        }
    } else {
        if ((endpoint = httpCreateEndpoint(ip, port, NULL)) == 0) {
            return 0;
        }
    }
    if ((host = httpCreateHost(home)) == 0) {
        return 0;
    }
    if ((route = httpCreateRoute(host)) == 0) {
        return 0;
    }
    httpSetHostDefaultRoute(host, route);
    httpSetHostIpAddr(host, ip, port);
    httpAddHostToEndpoint(endpoint, host);
    httpSetRouteDir(route, documents);
    httpFinalizeRoute(route);
    return endpoint;
}


static int destroyEndpointConnections(HttpEndpoint *endpoint)
{
    HttpConn    *conn;
    Http        *http;
    int         next;

    http = endpoint->http;
    lock(http->connections);
    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->endpoint == endpoint) {
            conn->endpoint = 0;
            httpDestroyConn(conn);
            next--;
        }
    }
    unlock(http->connections);
    return 0;
}


static bool validateEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;

    if ((host = mprGetFirstItem(endpoint->hosts)) == 0) {
        mprError("Missing host object on endpoint");
        return 0;
    }
    return 1;
}


PUBLIC int httpStartEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;
    cchar       *proto, *ip;
    int         next, rc;

    if (!validateEndpoint(endpoint)) {
        return MPR_ERR_BAD_ARGS;
    }
    for (ITERATE_ITEMS(endpoint->hosts, host, next)) {
        httpStartHost(host);
    }
    if ((endpoint->sock = mprCreateSocket()) == 0) {
        return MPR_ERR_MEMORY;
    }
    if ((rc = mprListenOnSocket(endpoint->sock, endpoint->ip, endpoint->port, MPR_SOCKET_NODELAY | MPR_SOCKET_THREAD)) < 0) {
        if (rc == MPR_ERR_ALREADY_EXISTS) {
            mprError("Cannot open a socket on %s:%d, socket already bound.", *endpoint->ip ? endpoint->ip : "*", endpoint->port);
        } else {
            mprError("Cannot open a socket on %s:%d", *endpoint->ip ? endpoint->ip : "*", endpoint->port);
        }
        return MPR_ERR_CANT_OPEN;
    }
    if (endpoint->http->listenCallback && (endpoint->http->listenCallback)(endpoint) < 0) {
        return MPR_ERR_CANT_OPEN;
    }
    if (endpoint->async && !endpoint->sock->handler) {
        mprAddSocketHandler(endpoint->sock, MPR_SOCKET_READABLE, endpoint->dispatcher, httpAcceptConn, endpoint, 
            (endpoint->dispatcher) ? 0 : MPR_WAIT_NEW_DISPATCHER);
    } else {
        mprSetSocketBlockingMode(endpoint->sock, 1);
    }
    proto = endpoint->ssl ? "HTTPS" : "HTTP ";
    ip = *endpoint->ip ? endpoint->ip : "*";
    if (mprIsSocketV6(endpoint->sock)) {
        mprLog(2, "Started %s service on \"[%s]:%d\"", proto, ip, endpoint->port);
    } else {
        mprLog(2, "Started %s service on \"%s:%d\"", proto, ip, endpoint->port);
    }
    return 0;
}


PUBLIC void httpStopEndpoint(HttpEndpoint *endpoint)
{
    HttpHost    *host;
    int         next;

    for (ITERATE_ITEMS(endpoint->hosts, host, next)) {
        httpStopHost(host);
    }
    if (endpoint->sock) {
        mprCloseSocket(endpoint->sock, 0);
        endpoint->sock = 0;
    }
}


/*
    OPT
 */
PUBLIC bool httpValidateLimits(HttpEndpoint *endpoint, int event, HttpConn *conn)
{
    HttpLimits  *limits;
    Http        *http;
    int         count, level, dir;

    limits = conn->limits;
    dir = HTTP_TRACE_RX;
    assert(conn->endpoint == endpoint);
    http = endpoint->http;

    lock(endpoint);

    switch (event) {
    case HTTP_VALIDATE_OPEN_CONN:
        /*
            This measures active client systems with unique IP addresses.
         */
        if (endpoint->activeClients >= limits->clientMax) {
            unlock(endpoint);
            /*  Abort connection */
            httpError(conn, HTTP_ABORT | HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent clients %d/%d", endpoint->activeClients, limits->clientMax);
            return 0;
        }
        count = (int) PTOL(mprLookupKey(endpoint->clientLoad, conn->ip));
        mprTrace(7, "Connection count for client %s, count %d limit %d\n", conn->ip, count, limits->requestsPerClientMax);
        if (count >= limits->requestsPerClientMax) {
            unlock(endpoint);
            /*  Abort connection */
            httpError(conn, HTTP_ABORT | HTTP_CODE_SERVICE_UNAVAILABLE, 
                "Too many concurrent requests for this client %s %d/%d", conn->ip, count, limits->requestsPerClientMax);
            return 0;
        }
        mprAddKey(endpoint->clientLoad, conn->ip, ITOP(count + 1));
        endpoint->activeClients = (int) mprGetHashLength(endpoint->clientLoad);
        dir = HTTP_TRACE_RX;
        break;

    case HTTP_VALIDATE_CLOSE_CONN:
        count = (int) PTOL(mprLookupKey(endpoint->clientLoad, conn->ip));
        if (count > 1) {
            mprAddKey(endpoint->clientLoad, conn->ip, ITOP(count - 1));
        } else {
            mprRemoveKey(endpoint->clientLoad, conn->ip);
        }
        endpoint->activeClients = (int) mprGetHashLength(endpoint->clientLoad);
        dir = HTTP_TRACE_TX;
        break;
    
    case HTTP_VALIDATE_OPEN_REQUEST:
        assert(conn->rx);
        if (endpoint->activeRequests >= limits->requestMax) {
            unlock(endpoint);
            httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Server overloaded");
            mprLog(2, "Too many concurrent requests %d/%d", endpoint->activeRequests, limits->requestMax);
            return 0;
        }
        endpoint->activeRequests++;
        conn->rx->flags |= HTTP_LIMITS_OPENED;
        dir = HTTP_TRACE_RX;
        break;

    case HTTP_VALIDATE_CLOSE_REQUEST:
        if (conn->rx && conn->rx->flags & HTTP_LIMITS_OPENED) {
            /* Requests incremented only when conn->rx is assigned */
            endpoint->activeRequests--;
            assert(endpoint->activeRequests >= 0);
            dir = HTTP_TRACE_TX;
            conn->rx->flags &= ~HTTP_LIMITS_OPENED;
        }
        break;

    case HTTP_VALIDATE_OPEN_PROCESS:
        http->activeProcesses++;
        if (http->activeProcesses > limits->processMax) {
            unlock(endpoint);
            httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE, "Server overloaded");
            mprLog(2, "Too many concurrent processes %d/%d", http->activeProcesses, limits->processMax);
            return 0;
        }
        dir = HTTP_TRACE_RX;
        break;

    case HTTP_VALIDATE_CLOSE_PROCESS:
        http->activeProcesses--;
        assert(http->activeProcesses >= 0);
        break;
    }
    if (event == HTTP_VALIDATE_CLOSE_CONN || event == HTTP_VALIDATE_CLOSE_REQUEST) {
        if ((level = httpShouldTrace(conn, dir, HTTP_TRACE_LIMITS, NULL)) >= 0) {
            mprTrace(4, "Validate request for %d. Active connections %d, active requests: %d/%d, active client IP %d/%d", 
                event, mprGetListLength(http->connections), endpoint->activeRequests, limits->requestMax, 
                endpoint->activeClients, limits->clientMax);
        }
    }
#if KEEP
    mprTrace(0, "Validate Active connections %d, requests: %d/%d, IP %d/%d, Processes %d/%d", 
        mprGetListLength(http->connections), endpoint->activeRequests, limits->requestMax, 
        endpoint->activeClients, limits->clientMax, http->activeProcesses, limits->processMax);
#endif
    unlock(endpoint);
    return 1;
}


PUBLIC HttpConn *httpAcceptConn(HttpEndpoint *endpoint, MprEvent *event)
{
    MprSocket   *sock;

    /*
        In sync mode, this will block until a connection arrives
     */
    sock = mprAcceptSocket(endpoint->sock);

    /*
        Immediately re-enable because acceptConn can block while servicing a request. Must always re-enable even if
        the sock acceptance above fails.
     */
    if (endpoint->sock->handler) {
        mprEnableSocketEvents(endpoint->sock, MPR_READABLE);
    }
    if (sock == 0) {
        return 0;
    }
    return acceptConn(sock, event->dispatcher, endpoint);
}


/*  
    Accept a new client connection on a new socket. This will come in on a worker thread dedicated to this connection. 
 */
static HttpConn *acceptConn(MprSocket *sock, MprDispatcher *dispatcher, HttpEndpoint *endpoint)
{
    Http        *http;
    HttpConn    *conn;
    MprEvent    e;
    int         level;

    assert(dispatcher);
    assert(endpoint);
    http = endpoint->http;

    if (endpoint->ssl) {
        if (mprUpgradeSocket(sock, endpoint->ssl, 0) < 0) {
            mprError("Cannot upgrade socket for SSL: %s", sock->errorMsg);
            mprCloseSocket(sock, 0);
            return 0;
        }
    }
    if (mprShouldDenyNewRequests()) {
        mprCloseSocket(sock, 0);
        return 0;
    }
#if FUTURE
    static int  warnOnceConnections = 0;
    int count;
    /* 
        Client connections are entered into http->connections. Need to split into two lists 
        Also, ejs pre-allocates connections in the Http constructor.
     */
    if ((count = mprGetListLength(http->connections)) >= endpoint->limits->requestMax) {
        /* To help alleviate DOS - we just close without responding */
        if (!warnOnceConnections) {
            warnOnceConnections = 1;
            mprLog(2, "Too many concurrent connections %d/%d", count, endpoint->limits->requestMax);
        }
        mprCloseSocket(sock, 0);
        http->underAttack = 1;
        return 0;
    }
#endif
    if ((conn = httpCreateConn(http, endpoint, dispatcher)) == 0) {
        mprCloseSocket(sock, 0);
        return 0;
    }
    conn->notifier = endpoint->notifier;
    conn->async = endpoint->async;
    conn->endpoint = endpoint;
    conn->sock = sock;
    conn->port = sock->port;
    conn->ip = sclone(sock->ip);
    conn->secure = (endpoint->ssl != 0);

    if (!httpValidateLimits(endpoint, HTTP_VALIDATE_OPEN_CONN, conn)) {
        conn->endpoint = 0;
        httpDestroyConn(conn);
        return 0;
    }
    assert(conn->state == HTTP_STATE_BEGIN);
    httpSetState(conn, HTTP_STATE_CONNECTED);

    if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_CONN, NULL)) >= 0) {
        mprLog(level, "### Incoming connection from %s:%d to %s:%d %s", 
            conn->ip, conn->port, sock->acceptIp, sock->acceptPort, conn->secure ? "(secure)" : "");
        if (endpoint->ssl) {
            mprLog(level, "Upgrade to TLS");
        }
    }
    e.mask = MPR_READABLE;
    e.timestamp = conn->http->now;
    (conn->ioCallback)(conn, &e);
    return conn;
}


PUBLIC void httpMatchHost(HttpConn *conn)
{ 
    MprSocket       *listenSock;
    HttpEndpoint    *endpoint;
    HttpHost        *host;
    Http            *http;

    http = conn->http;
    listenSock = conn->sock->listenSock;

    if ((endpoint = httpLookupEndpoint(http, listenSock->ip, listenSock->port)) == 0) {
        mprError("No listening endpoint for request from %s:%d", listenSock->ip, listenSock->port);
        mprCloseSocket(conn->sock, 0);
        return;
    }
    if (httpHasNamedVirtualHosts(endpoint)) {
        host = httpLookupHostOnEndpoint(endpoint, conn->rx->hostHeader);
    } else {
        host = mprGetFirstItem(endpoint->hosts);
    }
    if (host == 0) {
        httpSetConnHost(conn, 0);
        httpError(conn, HTTP_CODE_NOT_FOUND, "No host to serve request. Searching for %s", conn->rx->hostHeader);
        conn->host = mprGetFirstItem(endpoint->hosts);
        return;
    }
    if (conn->rx->traceLevel >= 0) {
        mprLog(conn->rx->traceLevel, "Use endpoint: %s:%d", endpoint->ip, endpoint->port);
        mprLog(conn->rx->traceLevel, "Use host %s", host->name);
    }
    conn->host = host;
}


PUBLIC void *httpGetEndpointContext(HttpEndpoint *endpoint)
{
    assert(endpoint);
    if (endpoint) {
        return endpoint->context;
    }
    return 0;
}


PUBLIC int httpIsEndpointAsync(HttpEndpoint *endpoint) 
{
    assert(endpoint);
    if (endpoint) {
        return endpoint->async;
    }
    return 0;
}


PUBLIC void httpSetEndpointAddress(HttpEndpoint *endpoint, cchar *ip, int port)
{
    assert(endpoint);

    if (ip) {
        endpoint->ip = sclone(ip);
    }
    if (port >= 0) {
        endpoint->port = port;
    }
    if (endpoint->sock) {
        httpStopEndpoint(endpoint);
        httpStartEndpoint(endpoint);
    }
}


PUBLIC void httpSetEndpointAsync(HttpEndpoint *endpoint, int async)
{
    if (endpoint->sock) {
        if (endpoint->async && !async) {
            mprSetSocketBlockingMode(endpoint->sock, 1);
        }
        if (!endpoint->async && async) {
            mprSetSocketBlockingMode(endpoint->sock, 0);
        }
    }
    endpoint->async = async;
}


PUBLIC void httpSetEndpointContext(HttpEndpoint *endpoint, void *context)
{
    assert(endpoint);
    endpoint->context = context;
}


PUBLIC void httpSetEndpointNotifier(HttpEndpoint *endpoint, HttpNotifier notifier)
{
    assert(endpoint);
    endpoint->notifier = notifier;
}


PUBLIC int httpSecureEndpoint(HttpEndpoint *endpoint, struct MprSsl *ssl)
{
#if BIT_SSL
    endpoint->ssl = ssl;
    return 0;
#else
    return MPR_ERR_BAD_STATE;
#endif
}


PUBLIC int httpSecureEndpointByName(cchar *name, struct MprSsl *ssl)
{
    HttpEndpoint    *endpoint;
    Http            *http;
    char            *ip;
    int             port, next, count;

    http = MPR->httpService;
    mprParseSocketAddress(name, &ip, &port, NULL, -1);
    if (ip == 0) {
        ip = "";
    }
    for (count = 0, next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            assert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                httpSecureEndpoint(endpoint, ssl);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


PUBLIC void httpAddHostToEndpoint(HttpEndpoint *endpoint, HttpHost *host)
{
    mprAddItem(endpoint->hosts, host);
    if (endpoint->limits == 0) {
        endpoint->limits = host->defaultRoute->limits;
    }
}


PUBLIC bool httpHasNamedVirtualHosts(HttpEndpoint *endpoint)
{
    return endpoint->flags & HTTP_NAMED_VHOST;
}


PUBLIC void httpSetHasNamedVirtualHosts(HttpEndpoint *endpoint, bool on)
{
    if (on) {
        endpoint->flags |= HTTP_NAMED_VHOST;
    } else {
        endpoint->flags &= ~HTTP_NAMED_VHOST;
    }
}


/*
    Only used for named virtual hosts
 */
PUBLIC HttpHost *httpLookupHostOnEndpoint(HttpEndpoint *endpoint, cchar *name)
{
    HttpHost    *host;
    int         next;

    if (name == 0 || *name == '\0') {
        return mprGetFirstItem(endpoint->hosts);
    }
    for (next = 0; (host = mprGetNextItem(endpoint->hosts, &next)) != 0; ) {
        if (smatch(host->name, name)) {
            return host;
        }
        if (*host->name == '*') {
            if (host->name[1] == '\0') {
                return host;
            }
            if (scontains(name, &host->name[1])) {
                return host;
            }
        }
    }
    return 0;
}


PUBLIC int httpConfigureNamedVirtualEndpoints(Http *http, cchar *ip, int port)
{
    HttpEndpoint    *endpoint;
    int             next, count;

    if (ip == 0) {
        ip = "";
    }
    for (count = 0, next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            assert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                httpSetHasNamedVirtualHosts(endpoint, 1);
                count++;
            }
        }
    }
    return (count == 0) ? MPR_ERR_CANT_FIND : 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/error.c"
 */
/************************************************************************/

/*
    error.c -- Http error handling
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static void errorv(HttpConn *conn, int flags, cchar *fmt, va_list args);
static char *formatErrorv(HttpConn *conn, int status, cchar *fmt, va_list args);

/*********************************** Code *************************************/

PUBLIC void httpDisconnect(HttpConn *conn)
{
    if (conn->sock) {
        mprDisconnectSocket(conn->sock);
    }
    conn->connError = 1;
    conn->error = 1;
    conn->keepAliveCount = -1;
    if (conn->rx) {
        conn->rx->eof = 1;
    }
    if (conn->tx) {
        conn->tx->finalized = 1;
        conn->tx->finalizedOutput = 1;
        conn->tx->finalizedConnector = 1;
    }
}


PUBLIC void httpError(HttpConn *conn, int flags, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt);
    errorv(conn, flags, fmt, args);
    va_end(args);
}


/*
    The current request has an error and cannot complete as normal. This call sets the Http response status and 
    overrides the normal output with an alternate error message. If the output has alread started (headers sent), then
    the connection MUST be closed so the client can get some indication the request failed.
 */
static void errorv(HttpConn *conn, int flags, cchar *fmt, va_list args)
{
    HttpRx      *rx;
    HttpTx      *tx;
    cchar       *uri, *statusMsg;
    int         status;

    assert(fmt);
    rx = conn->rx;
    tx = conn->tx;

    if (conn == 0) {
        return;
    }
    status = flags & HTTP_CODE_MASK;
    if (status == 0) {
        status = HTTP_CODE_INTERNAL_SERVER_ERROR;
    }
    if (flags & (HTTP_ABORT | HTTP_CLOSE)) {
        conn->keepAliveCount = -1;
    }
    if (flags & HTTP_ABORT) {
        conn->connError = 1;
        if (rx) {
            rx->eof = 1;
        }
    }
    if (!conn->error) {
        conn->error = 1;
        httpOmitBody(conn);
        conn->errorMsg = formatErrorv(conn, status, fmt, args);
        HTTP_NOTIFY(conn, HTTP_EVENT_ERROR, 0);
        if (conn->endpoint && tx && rx) {
            if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                /* 
                    If the response headers have been sent, must let the other side of the failure ... aborting
                    the request is the only way as the status has been sent.
                 */
                flags |= HTTP_ABORT;
            } else {
                if (rx->route && (uri = httpLookupRouteErrorDocument(rx->route, tx->status))) {
                    /*
                        If the response has started or it is an external redirect ... do a redirect
                     */
                    if (sstarts(uri, "http") || tx->flags & HTTP_TX_HEADERS_CREATED) {
                        httpRedirect(conn, HTTP_CODE_MOVED_PERMANENTLY, uri);
                    } else {
                        /*
                            No response started and it is an internal redirect, so we can rerun the request.
                            Set finalized to "cap" any output. processCompletion() in rx.c will rerun the request using
                            the errorDocument.
                         */
                        tx->errorDocument = uri;
                        tx->finalized = tx->finalizedOutput = tx->finalizedConnector = 1;
                    }
                } else {
                    httpAddHeaderString(conn, "Cache-Control", "no-cache");
                    statusMsg = httpLookupStatus(conn->http, status);
                    if (scmp(conn->rx->accept, "text/plain") == 0) {
                        tx->altBody = sfmt("Access Error: %d -- %s\r\n%s\r\n", status, statusMsg, conn->errorMsg);
                    } else {
                        tx->altBody = sfmt("<!DOCTYPE html>\r\n"
                            "<html><head><title>%s</title></head>\r\n"
                            "<body>\r\n<h2>Access Error: %d -- %s</h2>\r\n<pre>%s</pre>\r\n</body>\r\n</html>\r\n",
                            statusMsg, status, statusMsg, mprEscapeHtml(conn->errorMsg));
                    }
                    tx->length = slen(tx->altBody);
                    tx->flags |= HTTP_TX_NO_BODY;
                    httpDiscardData(conn, HTTP_QUEUE_TX);
                }
            }
        }
        httpFinalize(conn);
    }
    if (flags & HTTP_ABORT) {
        httpDisconnect(conn);
    }
}


/*
    Just format conn->errorMsg and set status - nothing more
    NOTE: this is an internal API. Users should use httpError()
 */
static char *formatErrorv(HttpConn *conn, int status, cchar *fmt, va_list args)
{
    if (conn->errorMsg == 0) {
        conn->errorMsg = sfmtv(fmt, args);
        if (status) {
            if (status < 0) {
                status = HTTP_CODE_INTERNAL_SERVER_ERROR;
            }
            if (conn->endpoint && conn->tx) {
                conn->tx->status = status;
            } else if (conn->rx) {
                conn->rx->status = status;
            }
        }
        if (conn->rx == 0 || conn->rx->uri == 0) {
            mprLog(2, "\"%s\", status %d: %s.", httpLookupStatus(conn->http, status), status, conn->errorMsg);
        } else {
            mprLog(2, "Error: %s", conn->errorMsg);
        }
    }
    return conn->errorMsg;
}


/*
    Just format conn->errorMsg and set status - nothing more
    NOTE: this is an internal API. Users should use httpError()
 */
PUBLIC void httpFormatError(HttpConn *conn, int status, cchar *fmt, ...)
{
    va_list     args;

    va_start(args, fmt); 
    conn->errorMsg = formatErrorv(conn, status, fmt, args);
    va_end(args); 
}


PUBLIC cchar *httpGetError(HttpConn *conn)
{
    if (conn->errorMsg) {
        return conn->errorMsg;
    } else if (conn->state >= HTTP_STATE_FIRST) {
        return httpLookupStatus(conn->http, conn->rx->status);
    } else {
        return "";
    }
}


PUBLIC void httpMemoryError(HttpConn *conn)
{
    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Memory allocation error");
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/host.c"
 */
/************************************************************************/

/*
    host.c -- Host class for all HTTP hosts

    The Host class is used for the default HTTP server and for all virtual hosts (including SSL hosts).
    Many objects are controlled at the host level. Eg. URL handlers.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Locals ***********************************/

static HttpHost *defaultHost;

/********************************** Forwards **********************************/

static void manageHost(HttpHost *host, int flags);

/*********************************** Code *************************************/

PUBLIC HttpHost *httpCreateHost()
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;
    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    if ((host->responseCache = mprCreateCache(MPR_CACHE_SHARED)) == 0) {
        return 0;
    }
    mprSetCacheLimits(host->responseCache, 0, BIT_MAX_CACHE_DURATION, 0, 0);

    host->mutex = mprCreateLock();
    host->routes = mprCreateList(-1, 0);
    host->flags = HTTP_HOST_NO_TRACE;
    host->protocol = sclone("HTTP/1.1");
    httpAddHost(http, host);
    return host;
}


PUBLIC HttpHost *httpCloneHost(HttpHost *parent)
{
    HttpHost    *host;
    Http        *http;

    http = MPR->httpService;

    if ((host = mprAllocObj(HttpHost, manageHost)) == 0) {
        return 0;
    }
    host->mutex = mprCreateLock();

    /*  
        The dirs and routes are all copy-on-write.
        Don't clone ip, port and name
     */
    host->parent = parent;
    host->responseCache = parent->responseCache;
    host->routes = parent->routes;
    host->flags = parent->flags | HTTP_HOST_VHOST;
    host->protocol = parent->protocol;
    httpAddHost(http, host);
    return host;
}


static void manageHost(HttpHost *host, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(host->name);
        mprMark(host->ip);
        mprMark(host->parent);
        mprMark(host->responseCache);
        mprMark(host->routes);
        mprMark(host->defaultRoute);
        mprMark(host->protocol);
        mprMark(host->mutex);
        mprMark(host->defaultEndpoint);
        mprMark(host->secureEndpoint);

    } else if (flags & MPR_MANAGE_FREE) {
        /* The http->hosts list is static. ie. The hosts won't be marked via http->hosts */
        httpRemoveHost(MPR->httpService, host);
    }
}


PUBLIC int httpStartHost(HttpHost *host)
{
    HttpRoute   *route;
    int         next;

    for (ITERATE_ITEMS(host->routes, route, next)) {
        httpStartRoute(route);
    }
    for (ITERATE_ITEMS(host->routes, route, next)) {
        if (!route->log && route->parent && route->parent->log) {
            route->log = route->parent->log;
        }
    }
    return 0;
}


PUBLIC void httpStopHost(HttpHost *host)
{
    HttpRoute   *route;
    int         next;

    for (ITERATE_ITEMS(host->routes, route, next)) {
        httpStopRoute(route);
    }
}


PUBLIC HttpRoute *httpGetHostDefaultRoute(HttpHost *host)
{
    return host->defaultRoute;
}


static void printRoute(HttpRoute *route, int next, bool full)
{
    HttpStage   *handler;
    MprKey      *kp;
    cchar       *methods, *pattern, *target, *index;
    int         nextIndex;

    methods = httpGetRouteMethods(route);
    methods = methods ? methods : "*";
    pattern = (route->pattern && *route->pattern) ? route->pattern : "^/";
    target = (route->target && *route->target) ? route->target : "$&";
    if (full) {
        mprRawLog(0, "\n%d. %s\n", next, route->name);
        mprRawLog(0, "    Pattern:      %s\n", pattern);
        mprRawLog(0, "    StartSegment: %s\n", route->startSegment);
        mprRawLog(0, "    StartsWith:   %s\n", route->startWith);
        mprRawLog(0, "    RegExp:       %s\n", route->optimizedPattern);
        mprRawLog(0, "    Methods:      %s\n", methods);
        mprRawLog(0, "    Prefix:       %s\n", route->prefix);
        mprRawLog(0, "    Target:       %s\n", target);
        mprRawLog(0, "    Directory:    %s\n", route->dir);
        if (route->indicies) {
            mprRawLog(0, "    Indicies      ");
            for (ITERATE_ITEMS(route->indicies, index, nextIndex)) {
                mprRawLog(0, "%s ", index);
            }
        }
        mprRawLog(0, "\n    Next Group    %d\n", route->nextGroup);
        if (route->handler) {
            mprRawLog(0, "    Handler:      %s\n", route->handler->name);
        }
        if (full) {
            if (route->extensions) {
                for (ITERATE_KEYS(route->extensions, kp)) {
                    handler = (HttpStage*) kp->data;
                    mprRawLog(0, "    Extension:    %s => %s\n", kp->key, handler->name);
                }
            }
            if (route->handlers) {
                for (ITERATE_ITEMS(route->handlers, handler, nextIndex)) {
                    mprRawLog(0, "    Handler:      %s\n", handler->name);
                }
            }
        }
        mprRawLog(0, "\n");
    } else {
        mprRawLog(0, "%-20s %-12s %-40s %-14s\n", route->name, methods ? methods : "*", pattern, target);
    }
}


PUBLIC void httpLogRoutes(HttpHost *host, bool full)
{
    HttpRoute   *route;
    int         next, foundDefault;

    if (!full) {
        mprRawLog(0, "%-20s %-12s %-40s %-14s\n", "Name", "Methods", "Pattern", "Target");
    }
    for (foundDefault = next = 0; (route = mprGetNextItem(host->routes, &next)) != 0; ) {
        printRoute(route, next - 1, full);
        if (route == host->defaultRoute) {
            foundDefault++;
        }
    }
    /*
        Add the default so LogRoutes can print the default route which has yet been added to host->routes
     */
    if (!foundDefault && host->defaultRoute) {
        printRoute(host->defaultRoute, next - 1, full);
    }
    mprRawLog(0, "\n");
}


/*
    IP may be null in which case the host is listening on all interfaces. Port may be set to -1 and ip may contain a port
    specifier, ie. "address:port".
 */
PUBLIC void httpSetHostIpAddr(HttpHost *host, cchar *ip, int port)
{
    char    *pip;

    if (port < 0 && schr(ip, ':')) {
        mprParseSocketAddress(ip, &pip, &port, NULL, -1);
        ip = pip;
    }
    host->ip = sclone(ip);
    host->port = port;
    if (!host->name) {
        if (ip) {
            if (port > 0) {
                host->name = sfmt("%s:%d", ip, port);
            } else {
                host->name = sclone(ip);
            }
        } else {
            assert(port > 0);
            host->name = sfmt("*:%d", port);
        }
    }
}


PUBLIC void httpSetHostName(HttpHost *host, cchar *name)
{
    host->name = sclone(name);
}


PUBLIC void httpSetHostProtocol(HttpHost *host, cchar *protocol)
{
    host->protocol = sclone(protocol);
}


PUBLIC int httpAddRoute(HttpHost *host, HttpRoute *route)
{
    HttpRoute   *prev, *item, *lastRoute;
    int         i, thisRoute;

    assert(route);
    
    if (host->parent && host->routes == host->parent->routes) {
        host->routes = mprCloneList(host->parent->routes);
    }
    if (mprLookupItem(host->routes, route) < 0) {
        if ((lastRoute = mprGetLastItem(host->routes)) && lastRoute->pattern[0] == '\0') {
            /* Insert before default route */
            thisRoute = mprInsertItemAtPos(host->routes, mprGetListLength(host->routes) - 1, route);
        } else {
            thisRoute = mprAddItem(host->routes, route);
        }
        if (thisRoute > 0) {
            prev = mprGetItem(host->routes, thisRoute - 1);
            if (!smatch(prev->startSegment, route->startSegment)) {
                prev->nextGroup = thisRoute;
                for (i = thisRoute - 2; i >= 0; i--) {
                    item = mprGetItem(host->routes, i);
                    if (smatch(item->startSegment, prev->startSegment)) {
                        item->nextGroup = thisRoute;
                    } else {
                        break;
                    }
                }
            }
        }
    }
    httpSetRouteHost(route, host);
    return 0;
}


PUBLIC HttpRoute *httpLookupRoute(HttpHost *host, cchar *name)
{
    HttpRoute   *route;
    int         next;

    if (name == 0 || *name == '\0') {
        name = "default";
    }
    if (!host && (host = httpGetDefaultHost()) == 0) {
        return 0;
    }
    for (next = 0; (route = mprGetNextItem(host->routes, &next)) != 0; ) {
        assert(route->name);
        if (smatch(route->name, name)) {
            return route;
        }
    }
    return 0;
}


PUBLIC void httpResetRoutes(HttpHost *host)
{
    host->routes = mprCreateList(-1, 0);
}


PUBLIC void httpSetHostDefaultRoute(HttpHost *host, HttpRoute *route)
{
    host->defaultRoute = route;
}


PUBLIC void httpSetDefaultHost(HttpHost *host)
{
    defaultHost = host;
}


PUBLIC void httpSetHostSecureEndpoint(HttpHost *host, HttpEndpoint *endpoint)
{
    host->secureEndpoint = endpoint;
}


PUBLIC void httpSetHostDefaultEndpoint(HttpHost *host, HttpEndpoint *endpoint)
{
    host->defaultEndpoint = endpoint;
}


PUBLIC HttpHost *httpGetDefaultHost()
{
    return defaultHost;
}


PUBLIC HttpRoute *httpGetDefaultRoute(HttpHost *host)
{
    if (host) {
        return host->defaultRoute;
    } else if (defaultHost) {
        return defaultHost->defaultRoute;
    }
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/httpService.c"
 */
/************************************************************************/

/*
    httpService.c -- Http service. Includes timer for expired requests.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Locals ************************************/
/**
    Standard HTTP error code table
 */
typedef struct HttpStatusCode {
    int     code;                           /**< Http error code */
    char    *codeString;                    /**< Code as a string (for hashing) */
    char    *msg;                           /**< Error message */
} HttpStatusCode;


PUBLIC HttpStatusCode HttpStatusCodes[] = {
    { 100, "100", "Continue" },
    { 101, "101", "Switching Protocols" },
    { 200, "200", "OK" },
    { 201, "201", "Created" },
    { 202, "202", "Accepted" },
    { 204, "204", "No Content" },
    { 205, "205", "Reset Content" },
    { 206, "206", "Partial Content" },
    { 301, "301", "Moved Permanently" },
    { 302, "302", "Moved Temporarily" },
    { 304, "304", "Not Modified" },
    { 305, "305", "Use Proxy" },
    { 307, "307", "Temporary Redirect" },
    { 400, "400", "Bad Request" },
    { 401, "401", "Unauthorized" },
    { 402, "402", "Payment Required" },
    { 403, "403", "Forbidden" },
    { 404, "404", "Not Found" },
    { 405, "405", "Method Not Allowed" },
    { 406, "406", "Not Acceptable" },
    { 408, "408", "Request Timeout" },
    { 409, "409", "Conflict" },
    { 410, "410", "Gone" },
    { 411, "411", "Length Required" },
    { 412, "412", "Precondition Failed" },
    { 413, "413", "Request Entity Too Large" },
    { 414, "414", "Request-URI Too Large" },
    { 415, "415", "Unsupported Media Type" },
    { 416, "416", "Requested Range Not Satisfiable" },
    { 417, "417", "Expectation Failed" },
    { 500, "500", "Internal Server Error" },
    { 501, "501", "Not Implemented" },
    { 502, "502", "Bad Gateway" },
    { 503, "503", "Service Unavailable" },
    { 504, "504", "Gateway Timeout" },
    { 505, "505", "Http Version Not Supported" },
    { 507, "507", "Insufficient Storage" },

    /*
        Proprietary codes (used internally) when connection to client is severed
     */
    { 550, "550", "Comms Error" },
    { 551, "551", "General Client Error" },
    { 0,   0 }
};

/****************************** Forward Declarations **************************/

static void httpTimer(Http *http, MprEvent *event);
static bool isIdle();
static void manageHttp(Http *http, int flags);
static void terminateHttp(int how, int status);
static void updateCurrentDate(Http *http);

/*********************************** Code *************************************/

PUBLIC Http *httpCreate(int flags)
{
    Http            *http;
    HttpStatusCode  *code;

    mprGlobalLock();
    if (MPR->httpService) {
        mprGlobalUnlock();
        return MPR->httpService;
    }
    if ((http = mprAllocObj(Http, manageHttp)) == 0) {
        mprGlobalUnlock();
        return 0;
    }
    MPR->httpService = http;
    http->software = sclone(BIT_HTTP_SOFTWARE);
    http->protocol = sclone("HTTP/1.1");
    http->mutex = mprCreateLock();
    http->stages = mprCreateHash(-1, 0);
    http->hosts = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->connections = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
    http->authTypes = mprCreateHash(-1, MPR_HASH_CASELESS | MPR_HASH_UNIQUE);
    http->authStores = mprCreateHash(-1, MPR_HASH_CASELESS | MPR_HASH_UNIQUE);
    http->booted = mprGetTime();

    updateCurrentDate(http);
    http->statusCodes = mprCreateHash(41, MPR_HASH_STATIC_VALUES | MPR_HASH_STATIC_KEYS);
    for (code = HttpStatusCodes; code->code; code++) {
        mprAddKey(http->statusCodes, code->codeString, code);
    }
    httpCreateSecret(http);
    httpInitAuth(http);
    httpOpenNetConnector(http);
    httpOpenSendConnector(http);
    httpOpenRangeFilter(http);
    httpOpenChunkFilter(http);
    httpOpenWebSockFilter(http);

    mprSetIdleCallback(isIdle);
    mprAddTerminator(terminateHttp);

    if (flags & HTTP_SERVER_SIDE) {
        http->endpoints = mprCreateList(-1, MPR_LIST_STATIC_VALUES);
        http->routeTargets = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
        http->routeConditions = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
        http->routeUpdates = mprCreateHash(-1, MPR_HASH_STATIC_VALUES);
        http->sessionCache = mprCreateCache(MPR_CACHE_SHARED);
        httpOpenUploadFilter(http);
        httpOpenCacheHandler(http);
        httpOpenPassHandler(http);
        httpOpenActionHandler(http);
        http->serverLimits = httpCreateLimits(1);
        httpDefineRouteBuiltins();
    }
    if (flags & HTTP_CLIENT_SIDE) {
        http->defaultClientHost = sclone("127.0.0.1");
        http->defaultClientPort = 80;
        http->clientLimits = httpCreateLimits(0);
        http->clientRoute = httpCreateConfiguredRoute(0, 0);
        http->clientHandler = httpCreateHandler(http, "client", 0);
    }
    mprGlobalUnlock();
    return http;
}


static void manageHttp(Http *http, int flags)
{
    HttpConn    *conn;
    int         next;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(http->endpoints);
        mprMark(http->hosts);
        mprMark(http->connections);
        mprMark(http->stages);
        mprMark(http->statusCodes);
        mprMark(http->routeTargets);
        mprMark(http->routeConditions);
        mprMark(http->routeUpdates);
        mprMark(http->sessionCache);
        /* Don't mark convenience stage references as they will be in http->stages */
        
        mprMark(http->clientLimits);
        mprMark(http->serverLimits);
        mprMark(http->clientRoute);
        mprMark(http->clientHandler);
        mprMark(http->timer);
        mprMark(http->timestamp);
        mprMark(http->mutex);
        mprMark(http->software);
        mprMark(http->forkData);
        mprMark(http->context);
        mprMark(http->currentDate);
        mprMark(http->secret);
        mprMark(http->defaultClientHost);
        mprMark(http->protocol);
        mprMark(http->proxyHost);
        mprMark(http->authTypes);
        mprMark(http->authStores);

        /*
            Endpoints keep connections alive until a timeout. Keep marking even if no other references.
         */
        lock(http->connections);
        for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
            if (conn->endpoint) {
                mprMark(conn);
            }
        }
        unlock(http->connections);
    }
}


PUBLIC void httpDestroy(Http *http)
{
    if (http->timer) {
        mprRemoveEvent(http->timer);
        http->timer = 0;
    }
    if (http->timestamp) {
        mprRemoveEvent(http->timestamp);
        http->timestamp = 0;
    }
    MPR->httpService = NULL;
}


PUBLIC void httpAddEndpoint(Http *http, HttpEndpoint *endpoint)
{
    mprAddItem(http->endpoints, endpoint);
}


PUBLIC void httpRemoveEndpoint(Http *http, HttpEndpoint *endpoint)
{
    mprRemoveItem(http->endpoints, endpoint);
}


/*  
    Lookup a host address. If ipAddr is null or port is -1, then those elements are wild.
 */
PUBLIC HttpEndpoint *httpLookupEndpoint(Http *http, cchar *ip, int port)
{
    HttpEndpoint    *endpoint;
    int             next;

    if (ip == 0) {
        ip = "";
    }
    for (next = 0; (endpoint = mprGetNextItem(http->endpoints, &next)) != 0; ) {
        if (endpoint->port <= 0 || port <= 0 || endpoint->port == port) {
            assert(endpoint->ip);
            if (*endpoint->ip == '\0' || *ip == '\0' || scmp(endpoint->ip, ip) == 0) {
                return endpoint;
            }
        }
    }
    return 0;
}


PUBLIC HttpEndpoint *httpGetFirstEndpoint(Http *http)
{
    return mprGetFirstItem(http->endpoints);
}


/*
    WARNING: this should not be called by users as httpCreateHost will automatically call this.
 */
PUBLIC void httpAddHost(Http *http, HttpHost *host)
{
    mprAddItem(http->hosts, host);
}


PUBLIC void httpRemoveHost(Http *http, HttpHost *host)
{
    mprRemoveItem(http->hosts, host);
}


PUBLIC HttpHost *httpLookupHost(Http *http, cchar *name)
{
    HttpHost    *host;
    int         next;

    for (next = 0; (host = mprGetNextItem(http->hosts, &next)) != 0; ) {
        if (smatch(name, host->name)) {
            return host;
        }
    }
    return 0;
}


PUBLIC void httpInitLimits(HttpLimits *limits, bool serverSide)
{
    memset(limits, 0, sizeof(HttpLimits));
    limits->bufferSize = BIT_MAX_QBUFFER;
    limits->cacheItemSize = BIT_MAX_CACHE_ITEM;
    limits->chunkSize = BIT_MAX_CHUNK;
    limits->clientMax = BIT_MAX_CLIENTS;
    limits->headerMax = BIT_MAX_NUM_HEADERS;
    limits->headerSize = BIT_MAX_HEADERS;
    limits->keepAliveMax = BIT_MAX_KEEP_ALIVE;
    limits->receiveFormSize = BIT_MAX_RECEIVE_FORM;
    limits->receiveBodySize = BIT_MAX_RECEIVE_BODY;
    limits->processMax = BIT_MAX_REQUESTS;
    limits->requestsPerClientMax = BIT_MAX_REQUESTS_PER_CLIENT;
    limits->requestMax = BIT_MAX_REQUESTS;
    limits->sessionMax = BIT_MAX_SESSIONS;
    limits->transmissionBodySize = BIT_MAX_TX_BODY;
    limits->uploadSize = BIT_MAX_UPLOAD;
    limits->uriSize = BIT_MAX_URI;

    limits->inactivityTimeout = BIT_MAX_INACTIVITY_DURATION;
    limits->requestTimeout = BIT_MAX_REQUEST_DURATION;
    limits->requestParseTimeout = BIT_MAX_PARSE_DURATION;
    limits->sessionTimeout = BIT_MAX_SESSION_DURATION;

    limits->webSocketsMax = BIT_MAX_WSS_SOCKETS;
    limits->webSocketsMessageSize = HTTP_MAX_WSS_MESSAGE;
    limits->webSocketsFrameSize = BIT_MAX_WSS_FRAME;
    limits->webSocketsPacketSize = BIT_MAX_WSS_PACKET;
    limits->webSocketsPing = BIT_MAX_PING_DURATION;

#if FUTURE
    mprSetMaxSocketClients(endpoint, atoi(value));

    if (scaselesscmp(key, "LimitClients") == 0) {
        mprSetMaxSocketClients(endpoint, atoi(value));
        return 1;
    }
    if (scaselesscmp(key, "LimitMemoryMax") == 0) {
        mprSetAllocLimits(endpoint, -1, atoi(value));
        return 1;
    }
    if (scaselesscmp(key, "LimitMemoryRedline") == 0) {
        mprSetAllocLimits(endpoint, atoi(value), -1);
        return 1;
    }
#endif
}


PUBLIC HttpLimits *httpCreateLimits(int serverSide)
{
    HttpLimits  *limits;

    if ((limits = mprAllocStruct(HttpLimits)) != 0) {
        httpInitLimits(limits, serverSide);
    }
    return limits;
}


PUBLIC void httpEaseLimits(HttpLimits *limits)
{
    limits->receiveFormSize = MAXOFF;
    limits->receiveBodySize = MAXOFF;
    limits->transmissionBodySize = MAXOFF;
    limits->uploadSize = MAXOFF;
}


PUBLIC void httpAddStage(Http *http, HttpStage *stage)
{
    mprAddKey(http->stages, stage->name, stage);
}


PUBLIC HttpStage *httpLookupStage(Http *http, cchar *name)
{
    return mprLookupKey(http->stages, name);
}


PUBLIC void *httpLookupStageData(Http *http, cchar *name)
{
    HttpStage   *stage;
    if ((stage = mprLookupKey(http->stages, name)) != 0) {
        return stage->stageData;
    }
    return 0;
}


PUBLIC cchar *httpLookupStatus(Http *http, int status)
{
    HttpStatusCode  *ep;
    char            *key;
    
    key = itos(status);
    ep = (HttpStatusCode*) mprLookupKey(http->statusCodes, key);
    if (ep == 0) {
        return "Custom error";
    }
    return ep->msg;
}


PUBLIC void httpSetForkCallback(Http *http, MprForkCallback callback, void *data)
{
    http->forkCallback = callback;
    http->forkData = data;
}


PUBLIC void httpSetListenCallback(Http *http, HttpListenCallback fn)
{
    http->listenCallback = fn;
}


/*  
    The http timer does maintenance activities and will fire per second while there are active requests.
    This is run in both servers and clients.
    NOTE: Because we lock the http here, connections cannot be deleted while we are modifying the list.
 */
static void httpTimer(Http *http, MprEvent *event)
{
    HttpConn    *conn;
    HttpStage   *stage;
    HttpLimits  *limits;
    MprModule   *module;
    int         next, active, abort;

    assert(event);
    
    updateCurrentDate(http);
    if (mprGetDebugMode()) {
        return;
    }
    /* 
       Check for any inactive connections or expired requests (inactivityTimeout and requestTimeout)
       OPT - could check for expired connections every 10 seconds.
     */
    lock(http->connections);
    mprTrace(7, "httpTimer: %d active connections", mprGetListLength(http->connections));
    for (active = 0, next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; active++) {
        limits = conn->limits;
        if (!conn->timeoutEvent) {
            abort = 0;
            if (conn->endpoint && HTTP_STATE_BEGIN < conn->state && conn->state < HTTP_STATE_PARSED && 
                    (conn->started + limits->requestParseTimeout) < http->now) {
                abort = 1;
            } else if ((conn->lastActivity + limits->inactivityTimeout) < http->now || 
                    (conn->started + limits->requestTimeout) < http->now) {
                abort = 1;
            }
            if (abort) {
                conn->timeoutEvent = mprCreateEvent(conn->dispatcher, "connTimeout", 0, httpConnTimeout, conn, 0);
            }
        }
    }

    /*
        Check for unloadable modules
        OPT - could check for modules every minute
     */
    if (mprGetListLength(http->connections) == 0) {
        for (next = 0; (module = mprGetNextItem(MPR->moduleService->modules, &next)) != 0; ) {
            if (module->timeout) {
                if (module->lastActivity + module->timeout < http->now) {
                    mprLog(2, "Unloading inactive module %s", module->name);
                    if ((stage = httpLookupStage(http, module->name)) != 0) {
                        if (mprUnloadModule(module) < 0)  {
                            active++;
                        } else {
                            stage->flags |= HTTP_STAGE_UNLOADED;
                        }
                    } else {
                        mprUnloadModule(module);
                    }
                } else {
                    active++;
                }
            }
        }
    }
    if (active == 0) {
        mprRemoveEvent(event);
        http->timer = 0;
    }
    unlock(http->connections);

    /*
        This will trigger a GC if worthwhile, but will defer yielding
     */
    mprRequestGC(MPR_GC_NO_YIELD);
}


static void timestamp()
{
    mprLog(0, "Time: %s", mprGetDate(NULL));
}


PUBLIC void httpSetTimestamp(MprTicks period)
{
    Http    *http;

    http = MPR->httpService;
    if (period < (10 * MPR_TICKS_PER_SEC)) {
        period = (10 * MPR_TICKS_PER_SEC);
    }
    if (http->timestamp) {
        mprRemoveEvent(http->timestamp);
    }
    if (period > 0) {
        http->timestamp = mprCreateTimerEvent(NULL, "httpTimestamp", period, timestamp, NULL, 
            MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
    }
}


static void terminateHttp(int how, int status)
{
    Http            *http;
    HttpEndpoint    *endpoint;
    int             next;

    /*
        Stop listening for new requests
     */
    http = (Http*) mprGetMpr()->httpService;
    if (http) {
        for (ITERATE_ITEMS(http->endpoints, endpoint, next)) {
            httpStopEndpoint(endpoint);
        }
    }
}


static bool isIdle()
{
    HttpConn        *conn;
    Http            *http;
    MprTicks        now;
    int             next;
    static MprTicks lastTrace = 0;

    http = (Http*) mprGetMpr()->httpService;
    now = http->now;

    lock(http->connections);
    for (next = 0; (conn = mprGetNextItem(http->connections, &next)) != 0; ) {
        if (conn->state != HTTP_STATE_BEGIN) {
            if (lastTrace < now) {
                if (conn->rx) {
                    mprLog(1, "  Request %s is still active", conn->rx->uri ? conn->rx->uri : conn->rx->pathInfo);
                } else {
                    mprLog(1, "Waiting for connection to close");
                    conn->started = 0;
                    conn->timeoutEvent = mprCreateEvent(conn->dispatcher, "connTimeout", 0, httpConnTimeout, conn, 0);
                }
                lastTrace = now;
            }
            unlock(http->connections);
            return 0;
        }
    }
    unlock(http->connections);
    if (!mprServicesAreIdle()) {
        if (lastTrace < now) {
            mprLog(3, "Waiting for MPR services complete");
            lastTrace = now;
        }
        return 0;
    }
    return 1;
}


PUBLIC void httpAddConn(Http *http, HttpConn *conn)
{
    http->now = mprGetTicks();
    assert(http->now >= 0);
    conn->started = http->now;
    mprAddItem(http->connections, conn);
    updateCurrentDate(http);

    //  OPT - use a less contentions mutex
    lock(http);
    conn->seqno = (int) http->totalConnections++;
    if (!http->timer) {
        http->timer = mprCreateTimerEvent(NULL, "httpTimer", HTTP_TIMER_PERIOD, httpTimer, http, 
            MPR_EVENT_CONTINUOUS | MPR_EVENT_QUICK);
    }
    unlock(http);
}


PUBLIC void httpRemoveConn(Http *http, HttpConn *conn)
{
    mprRemoveItem(http->connections, conn);
}


/*  
    Create a random secret for use in authentication. Create once for the entire http service. Created on demand.
    Users can recall as required to update.
 */
PUBLIC int httpCreateSecret(Http *http)
{
    MprTicks    now;
    char        *hex = "0123456789abcdef";
    char        bytes[HTTP_MAX_SECRET], ascii[HTTP_MAX_SECRET * 2 + 1], *ap, *cp, *bp;
    int         i, pid;

    if (mprGetRandomBytes(bytes, sizeof(bytes), 0) < 0) {
        now = http->now;
        pid = (int) getpid();
        cp = (char*) &now;
        bp = bytes;
        for (i = 0; i < sizeof(now) && bp < &bytes[HTTP_MAX_SECRET]; i++) {
            *bp++= *cp++;
        }
        cp = (char*) &now;
        for (i = 0; i < sizeof(pid) && bp < &bytes[HTTP_MAX_SECRET]; i++) {
            *bp++ = *cp++;
        }
        assert(0);
        return MPR_ERR_CANT_INITIALIZE;
    }
    ap = ascii;
    for (i = 0; i < (int) sizeof(bytes); i++) {
        *ap++ = hex[((uchar) bytes[i]) >> 4];
        *ap++ = hex[((uchar) bytes[i]) & 0xf];
    }
    *ap = '\0';
    http->secret = sclone(ascii);
    return 0;
}


PUBLIC char *httpGetDateString(MprPath *sbuf)
{
    MprTicks    when;

    if (sbuf == 0) {
        when = mprGetTime();
    } else {
        when = (MprTicks) sbuf->mtime * MPR_TICKS_PER_SEC;
    }
    return mprFormatUniversalTime(HTTP_DATE_FORMAT, when);
}


PUBLIC void *httpGetContext(Http *http)
{
    return http->context;
}


PUBLIC void httpSetContext(Http *http, void *context)
{
    http->context = context;
}


PUBLIC int httpGetDefaultClientPort(Http *http)
{
    return http->defaultClientPort;
}


PUBLIC cchar *httpGetDefaultClientHost(Http *http)
{
    return http->defaultClientHost;
}


PUBLIC void httpSetDefaultClientPort(Http *http, int port)
{
    http->defaultClientPort = port;
}


PUBLIC void httpSetDefaultClientHost(Http *http, cchar *host)
{
    http->defaultClientHost = sclone(host);
}


PUBLIC void httpSetSoftware(Http *http, cchar *software)
{
    http->software = sclone(software);
}


PUBLIC void httpSetProxy(Http *http, cchar *host, int port)
{
    http->proxyHost = sclone(host);
    http->proxyPort = port;
}


static void updateCurrentDate(Http *http)
{
    http->now = mprGetTicks();
    assert(http->now >= 0);
    if (http->now > (http->currentTime + MPR_TICKS_PER_SEC - 1)) {
        /*
            Optimize and only update the string date representation once per second
         */
        http->currentTime = http->now;
        http->currentDate = httpGetDateString(NULL);
    }
}


PUBLIC void httpGetStats(HttpStats *sp)
{
    MprMemStats         *ap;
    MprWorkerStats      wstats;
    Http                *http;
    HttpEndpoint        *ep;
    int                 next;

    memset(sp, 0, sizeof(*sp));
    http = MPR->httpService;
    ap = mprGetMemStats();

    sp->cpus = ap->numCpu;
    sp->regions = ap->regions;
    sp->pendingRequests = MPR->eventService->pendingCount;

    sp->mem = ap->rss;
    sp->memRedline = ap->redLine;
    sp->memMax = ap->maxMemory;

    sp->heap = ap->bytesAllocated + ap->bytesFree;
    sp->heapUsed = ap->bytesAllocated;
    sp->heapFree = ap->bytesFree;

    mprGetWorkerStats(&wstats);
    sp->workersBusy = wstats.busy;
    sp->workersIdle = wstats.idle;
    sp->workersYielded = wstats.yielded;
    sp->workersMax = wstats.max;

    sp->activeVMs = http->activeVMs;
    sp->activeConnections = mprGetListLength(http->connections);
    sp->activeProcesses = http->activeProcesses;
    sp->activeSessions = http->activeSessions;
    for (ITERATE_ITEMS(http->endpoints, ep, next)) {
        sp->activeRequests += ep->activeRequests;
        sp->activeClients += ep->activeClients;
    }

    sp->totalRequests = http->totalRequests;
    sp->totalConnections = http->totalConnections;
    sp->totalSweeps = MPR->heap->iteration;

}


PUBLIC char *httpStatsReport(int flags)
{
    MprTime             now;
    MprBuf              *buf;
    HttpStats           s;
    double              elapsed;
    static MprTime      lastTime;
    static HttpStats    last;
    double              mb;

    mb = 1024.0 * 1024;
    now = mprGetTime();
    elapsed = (now - lastTime) / 1000.0;
    httpGetStats(&s);
    buf = mprCreateBuf(0, 0);

    mprPutToBuf(buf, "\nHttp Report: at %s\n\n", mprGetDate("%D %T"));
    mprPutToBuf(buf, "Memory      %8.1f MB, %5.1f%% max\n", s.mem / mb, s.mem / (double) s.memMax * 100.0);
    mprPutToBuf(buf, "Heap        %8.1f MB, %5.1f%% mem\n", s.heap / mb, s.heap / (double) s.mem * 100.0);
    mprPutToBuf(buf, "Heap-used   %8.1f MB, %5.1f%% used\n", s.heapUsed / mb, s.heapUsed / (double) s.heap * 100.0);
    mprPutToBuf(buf, "Heap-free   %8.1f MB, %5.1f%% free\n", s.heapFree / mb, s.heapFree / (double) s.heap * 100.0);

    mprPutCharToBuf(buf, '\n');
    mprPutToBuf(buf, "Regions     %8d\n", s.regions);
    mprPutToBuf(buf, "CPUs        %8d\n", s.cpus);
    mprPutCharToBuf(buf, '\n');

    mprPutToBuf(buf, "Connections %8.1f per/sec\n", (s.totalConnections - last.totalConnections) / elapsed);
    mprPutToBuf(buf, "Requests    %8.1f per/sec\n", (s.totalRequests - last.totalRequests) / elapsed);
    mprPutToBuf(buf, "Sweeps      %8.1f per/sec\n", (s.totalSweeps - last.totalSweeps) / elapsed);
    mprPutCharToBuf(buf, '\n');

    mprPutToBuf(buf, "Clients     %8d active\n", s.activeClients);
    mprPutToBuf(buf, "Connections %8d active\n", s.activeConnections);
    mprPutToBuf(buf, "Processes   %8d active\n", s.activeProcesses);
    mprPutToBuf(buf, "Requests    %8d active\n", s.activeRequests);
    mprPutToBuf(buf, "Sessions    %8d active\n", s.activeSessions);
    mprPutToBuf(buf, "VMs         %8d active\n", s.activeVMs);
    mprPutToBuf(buf, "Pending     %8d requests\n", s.pendingRequests);
    mprPutCharToBuf(buf, '\n');

    mprPutToBuf(buf, "Workers     %8d busy - %d yielded, %d idle, %d max\n", 
        s.workersBusy, s.workersYielded, s.workersIdle, s.workersMax);
    mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);

    last = s;
    lastTime = now;
    return sclone(mprGetBufStart(buf));
}



/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/log.c"
 */
/************************************************************************/

/*
    log.c -- Http request access logging

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/************************************ Code ************************************/

PUBLIC int httpSetRouteLog(HttpRoute *route, cchar *path, ssize size, int backup, cchar *format, int flags)
{
    char    *src, *dest;

    assert(route);
    assert(path && *path);
    assert(format);
    
    if (format == NULL || *format == '\0') {
        format = BIT_HTTP_LOG_FORMAT;
    }
    route->logFlags = flags;
    route->logSize = size;
    route->logBackup = backup;
    route->logPath = sclone(path);
    route->logFormat = sclone(format);

    for (src = dest = route->logFormat; *src; src++) {
        if (*src == '\\' && src[1] != '\\') {
            continue;
        }
        *dest++ = *src;
    }
    *dest = '\0';
    if (route->logBackup > 0) {
        httpBackupRouteLog(route);
    }
    route->logFlags &= ~MPR_LOG_ANEW;
    if (!httpOpenRouteLog(route)) {
        return MPR_ERR_CANT_OPEN;
    }
    return 0;
}


PUBLIC void httpBackupRouteLog(HttpRoute *route)
{
    MprPath     info;

    assert(route->logBackup);
    assert(route->logSize > 100);

    if (route->parent && route->parent->log == route->log) {
        httpBackupRouteLog(route->parent);
        return;
    }
    lock(route);
    mprGetPathInfo(route->logPath, &info);
    if (info.valid && ((route->logFlags & MPR_LOG_ANEW) || info.size > route->logSize || route->logSize <= 0)) {
        if (route->log) {
            mprCloseFile(route->log);
            route->log = 0;
        }
        mprBackupLog(route->logPath, route->logBackup);
        route->logFlags &= ~MPR_LOG_ANEW;
    }
    unlock(route);
}


PUBLIC MprFile *httpOpenRouteLog(HttpRoute *route)
{
    MprFile     *file;
    int         mode;

    assert(route->log == 0);
    mode = O_CREAT | O_WRONLY | O_TEXT;
    if ((file = mprOpenFile(route->logPath, mode, 0664)) == 0) {
        mprError("Cannot open log file %s", route->logPath);
        return 0;
    }
    route->log = file;
    return file;
}


PUBLIC void httpWriteRouteLog(HttpRoute *route, cchar *buf, ssize len)
{
    lock(MPR);
    if (route->logBackup > 0) {
        //  OPT - don't check this on every write
        httpBackupRouteLog(route);
        if (!route->log && !httpOpenRouteLog(route)) {
            unlock(MPR);
            return;
        }
    }
    if (mprWriteFile(route->log, (char*) buf, len) != len) {
        mprError("Cannot write to access log %s", route->logPath);
        mprCloseFile(route->log);
        route->log = 0;
    }
    unlock(MPR);
}


PUBLIC void httpLogRequest(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprBuf      *buf;
    char        keyBuf[80], *timeText, *fmt, *cp, *qualifier, *value, c;
    int         len;

    if ((rx = conn->rx) == 0) {
        return;
    }
    tx = conn->tx;
    if ((route = rx->route) == 0 || route->log == 0) {
        return;
    }
    fmt = route->logFormat;
    if (fmt == 0) {
        fmt = BIT_HTTP_LOG_FORMAT;
    }
    len = BIT_MAX_URI + 256;
    buf = mprCreateBuf(len, len);

    while ((c = *fmt++) != '\0') {
        if (c != '%' || (c = *fmt++) == '%') {
            mprPutCharToBuf(buf, c);
            continue;
        }
        switch (c) {
        case 'a':                           /* Remote IP */
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'A':                           /* Local IP */
            mprPutStringToBuf(buf, conn->sock->listenSock->ip);
            break;

        case 'b':
            if (tx->bytesWritten == 0) {
                mprPutCharToBuf(buf, '-');
            } else {
                mprPutIntToBuf(buf, tx->bytesWritten);
            } 
            break;

        case 'B':                           /* Bytes written (minus headers) */
            mprPutIntToBuf(buf, (tx->bytesWritten - tx->headerSize));
            break;

        case 'h':                           /* Remote host */
            mprPutStringToBuf(buf, conn->ip);
            break;

        case 'n':                           /* Local host */
            mprPutStringToBuf(buf, rx->parsedUri->host);
            break;

        case 'O':                           /* Bytes written (including headers) */
            mprPutIntToBuf(buf, tx->bytesWritten);
            break;

        case 'r':                           /* First line of request */
            mprPutToBuf(buf, "%s %s %s", rx->method, rx->uri, conn->protocol);
            break;

        case 's':                           /* Response code */
            mprPutIntToBuf(buf, tx->status);
            break;

        case 't':                           /* Time */
            mprPutCharToBuf(buf, '[');
            timeText = mprFormatLocalTime(MPR_DEFAULT_DATE, mprGetTime());
            mprPutStringToBuf(buf, timeText);
            mprPutCharToBuf(buf, ']');
            break;

        case 'u':                           /* Remote username */
            mprPutStringToBuf(buf, conn->username ? conn->username : "-");
            break;

        case '{':                           /* Header line */
            qualifier = fmt;
            if ((cp = strchr(qualifier, '}')) != 0) {
                fmt = &cp[1];
                *cp = '\0';
                c = *fmt++;
                scopy(keyBuf, sizeof(keyBuf), "HTTP_");
                scopy(&keyBuf[5], sizeof(keyBuf) - 5, qualifier);
                switch (c) {
                case 'i':
                    value = (char*) mprLookupKey(rx->headers, supper(keyBuf));
                    mprPutStringToBuf(buf, value ? value : "-");
                    break;
                default:
                    mprPutStringToBuf(buf, qualifier);
                }
                *cp = '}';

            } else {
                mprPutCharToBuf(buf, c);
            }
            break;

        case '>':
            if (*fmt == 's') {
                fmt++;
                mprPutIntToBuf(buf, tx->status);
            }
            break;

        default:
            mprPutCharToBuf(buf, c);
            break;
        }
    }
    mprPutCharToBuf(buf, '\n');
    mprAddNullToBuf(buf);
    httpWriteRouteLog(route, mprGetBufStart(buf), mprGetBufLength(buf));
}



/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/netConnector.c"
 */
/************************************************************************/

/*
    netConnector.c -- General network connector. 

    The Network connector handles output data (only) from upstream handlers and filters. It uses vectored writes to
    aggregate output packets into fewer actual I/O requests to the O/S. 

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/**************************** Forward Declarations ****************************/

static void addPacketForNet(HttpQueue *q, HttpPacket *packet);
static void adjustNetVec(HttpQueue *q, ssize written);
static MprOff buildNetVec(HttpQueue *q);
static void freeNetPackets(HttpQueue *q, ssize written);
static void netClose(HttpQueue *q);
static void netOutgoingService(HttpQueue *q);

/*********************************** Code *************************************/
/*  
    Initialize the net connector
 */
PUBLIC int httpOpenNetConnector(Http *http)
{
    HttpStage     *stage;

    mprTrace(5, "Open net connector");
    if ((stage = httpCreateConnector(http, "netConnector", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->close = netClose;
    stage->outgoingService = netOutgoingService;
    http->netConnector = stage;
    return 0;
}


static void netClose(HttpQueue *q)
{
    HttpTx      *tx;

    tx = q->conn->tx;
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
}


static void netOutgoingService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       written;
    int         errCode;

    conn = q->conn;
    tx = conn->tx;
    conn->lastActivity = conn->http->now;
    assert(conn->sock);
    
    if (!conn->sock || tx->finalizedConnector) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardQueueData(q, 1);
    }
    if ((tx->bytesWritten + q->count) > conn->limits->transmissionBodySize) {
        httpError(conn, HTTP_CODE_REQUEST_TOO_LARGE | ((tx->bytesWritten) ? HTTP_ABORT : 0),
            "Http transmission aborted. Exceeded transmission max body of %,Ld bytes", conn->limits->transmissionBodySize);
        if (tx->bytesWritten) {
            httpFinalizeConnector(conn);
            return;
        }
    }
#if !BIT_ROM
    if (tx->flags & HTTP_TX_SENDFILE) {
        /* Relay via the send connector */
        if (tx->file == 0) {
            if (tx->flags & HTTP_TX_HEADERS_CREATED) {
                tx->flags &= ~HTTP_TX_SENDFILE;
            } else {
                tx->connector = conn->http->sendConnector;
                httpSendOpen(q);
            }
        }
        if (tx->file) {
            httpSendOutgoingService(q);
            return;
        }
    }
#endif
    while (q->first || q->ioIndex) {
        if (q->ioIndex == 0 && buildNetVec(q) <= 0) {
            break;
        }
        /*  
            Issue a single I/O request to write all the blocks in the I/O vector
         */
        assert(q->ioIndex > 0);
        written = mprWriteSocketVector(conn->sock, q->iovec, q->ioIndex);
        if (written < 0) {
            errCode = mprGetError(q);
            mprTrace(6, "netConnector: wrote %d, errno %d, qflags %x", (int) written, errCode, q->flags);
            if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
                /*  Socket full, wait for an I/O event */
                httpSocketBlocked(conn);
                break;
            }
            if (errCode == EPROTO && conn->secure) {
                httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Can't negotiate SSL with server: %s",
                    conn->sock->errorMsg);
            } else if (errCode != EPIPE && errCode != ECONNRESET && errCode != ECONNABORTED && errCode != ENOTCONN) {
                httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "netConnector: Can't write. errno %d", errCode);
            } else {
                httpDisconnect(conn);
            }
            httpFinalizeConnector(conn);
            break;

        } else if (written > 0) {
            mprTrace(6, "netConnector: wrote %d, ask %d, qflags %x", (int) written, q->ioCount, q->flags);
            tx->bytesWritten += written;
            freeNetPackets(q, written);
            adjustNetVec(q, written);

        } else {
            break;
        }
    }
    if (q->first && q->first->flags & HTTP_PACKET_END) {
        mprTrace(6, "netConnector: end of stream. Finalize connector");
        httpFinalizeConnector(conn);
    } else {
        HTTP_NOTIFY(conn, HTTP_EVENT_WRITABLE, 0);
    }
}


/*
    Build the IO vector. Return the count of bytes to be written. Return -1 for EOF.
 */
static MprOff buildNetVec(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    HttpPacket  *packet, *prev;

    conn = q->conn;
    tx = conn->tx;

    /*
        Examine each packet and accumulate as many packets into the I/O vector as possible. Leave the packets on the queue 
        for now, they are removed after the IO is complete for the entire packet.
     */
    for (packet = prev = q->first; packet && !(packet->flags & HTTP_PACKET_END); packet = packet->next) {
        if (packet->flags & HTTP_PACKET_HEADER) {
            if (tx->chunkSize <= 0 && q->count > 0 && tx->length < 0) {
                /* Incase no chunking filter and we've not seen all the data yet */
                conn->keepAliveCount = 0;
            }
            httpWriteHeaders(q, packet);
        }
        if (q->ioIndex >= (BIT_MAX_IOVEC - 2)) {
            break;
        }
        if (httpGetPacketLength(packet) > 0 || packet->prefix) {
            addPacketForNet(q, packet);
        } else {
            /* Remove empty packets */
            prev->next = packet->next;
            continue;
        }
        prev = packet;
    }
    return q->ioCount;
}


/*
    Add one entry to the io vector
 */
static void addToNetVector(HttpQueue *q, char *ptr, ssize bytes)
{
    assert(bytes > 0);

    q->iovec[q->ioIndex].start = ptr;
    q->iovec[q->ioIndex].len = bytes;
    q->ioCount += bytes;
    q->ioIndex++;
}


/*
    Add a packet to the io vector. Return the number of bytes added to the vector.
 */
static void addPacketForNet(HttpQueue *q, HttpPacket *packet)
{
    HttpTx      *tx;
    HttpConn    *conn;
    int         item;

    conn = q->conn;
    tx = conn->tx;

    assert(q->count >= 0);
    assert(q->ioIndex < (BIT_MAX_IOVEC - 2));

    if (packet->prefix) {
        addToNetVector(q, mprGetBufStart(packet->prefix), mprGetBufLength(packet->prefix));
    }
    if (httpGetPacketLength(packet) > 0) {
        addToNetVector(q, mprGetBufStart(packet->content), mprGetBufLength(packet->content));
    }
    item = (packet->flags & HTTP_PACKET_HEADER) ? HTTP_TRACE_HEADER : HTTP_TRACE_BODY;
    if (httpShouldTrace(conn, HTTP_TRACE_TX, item, tx->ext) >= 0) {
        httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, (ssize) tx->bytesWritten);
    }
}


static void freeNetPackets(HttpQueue *q, ssize bytes)
{
    HttpPacket  *packet;
    ssize       len;

    assert(q->count >= 0);
    assert(bytes > 0);

    /*
        Loop while data to be accounted for and we have not hit the end of data packet.
        Chunks will have the chunk header in the packet->prefix.
        The final chunk trailer will be in a packet->prefix with no other data content.
        Must leave this routine with the end packet still on the queue and all bytes accounted for.
     */
    while ((packet = q->first) != 0 && !(packet->flags & HTTP_PACKET_END) && bytes > 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes don't count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if (packet->content) {
            len = mprGetBufLength(packet->content);
            len = min(len, bytes);
            mprAdjustBufStart(packet->content, len);
            bytes -= len;
            q->count -= len;
            assert(q->count >= 0);
        }
        if (httpGetPacketLength(packet) == 0) {
            /* Done with this packet - consume it */
            assert(!(packet->flags & HTTP_PACKET_END));
            httpGetPacket(q);
        } else {
            break;
        }
    }
    assert(bytes == 0);
}


/*
    Clear entries from the IO vector that have actually been transmitted. Support partial writes.
 */
static void adjustNetVec(HttpQueue *q, ssize written)
{
    MprIOVec    *iovec;
    ssize       len;
    int         i, j;

    /*
        Cleanup the IO vector
     */
    if (written == q->ioCount) {
        /*
            Entire vector written. Just reset.
         */
        q->ioIndex = 0;
        q->ioCount = 0;

    } else {
        /*
            Partial write of an vector entry. Need to copy down the unwritten vector entries.
         */
        q->ioCount -= written;
        assert(q->ioCount >= 0);
        iovec = q->iovec;
        for (i = 0; i < q->ioIndex; i++) {
            len = iovec[i].len;
            if (written < len) {
                iovec[i].start += written;
                iovec[i].len -= written;
                break;
            } else {
                written -= len;
            }
        }
        /*
            Compact
         */
        for (j = 0; i < q->ioIndex; ) {
            iovec[j++] = iovec[i++];
        }
        q->ioIndex = j;
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/packet.c"
 */
/************************************************************************/

/*
    packet.c -- Queue support routines. Queues are the bi-directional data flow channels for the pipeline.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static void managePacket(HttpPacket *packet, int flags);

/************************************ Code ************************************/
/*  
    Create a new packet. If size is -1, then also create a default growable buffer -- 
    used for incoming body content. If size > 0, then create a non-growable buffer 
    of the requested size.
 */
PUBLIC HttpPacket *httpCreatePacket(ssize size)
{
    HttpPacket  *packet;

    if ((packet = mprAllocObj(HttpPacket, managePacket)) == 0) {
        return 0;
    }
    if (size != 0) {
        if ((packet->content = mprCreateBuf(size < 0 ? BIT_MAX_BUFFER: (ssize) size, -1)) == 0) {
            return 0;
        }
    }
    return packet;
}


static void managePacket(HttpPacket *packet, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(packet->prefix);
        mprMark(packet->content);
        /* Don't mark next packet. List owner will mark */
    }
}


PUBLIC HttpPacket *httpCreateDataPacket(ssize size)
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(size)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_DATA;
    return packet;
}


PUBLIC HttpPacket *httpCreateEntityPacket(MprOff pos, MprOff size, HttpFillProc fill)
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_DATA;
    packet->epos = pos;
    packet->esize = size;
    packet->fill = fill;
    return packet;
}


PUBLIC HttpPacket *httpCreateEndPacket()
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_END;
    return packet;
}


PUBLIC HttpPacket *httpCreateHeaderPacket()
{
    HttpPacket    *packet;

    if ((packet = httpCreatePacket(BIT_MAX_BUFFER)) == 0) {
        return 0;
    }
    packet->flags = HTTP_PACKET_HEADER;
    return packet;
}


PUBLIC HttpPacket *httpClonePacket(HttpPacket *orig)
{
    HttpPacket  *packet;

    if ((packet = httpCreatePacket(0)) == 0) {
        return 0;
    }
    if (orig->content) {
        packet->content = mprCloneBuf(orig->content);
    }
    if (orig->prefix) {
        packet->prefix = mprCloneBuf(orig->prefix);
    }
    packet->flags = orig->flags;
    packet->type = orig->type;
    packet->last = orig->last;
    packet->esize = orig->esize;
    packet->epos = orig->epos;
    packet->fill = orig->fill;
    return packet;
}


PUBLIC void httpAdjustPacketStart(HttpPacket *packet, MprOff size)
{
    if (packet->esize) {
        packet->epos += size;
        packet->esize -= size;
    } else if (packet->content) {
        mprAdjustBufStart(packet->content, (ssize) size);
    }
}


PUBLIC void httpAdjustPacketEnd(HttpPacket *packet, MprOff size)
{
    if (packet->esize) {
        packet->esize += size;
    } else if (packet->content) {
        mprAdjustBufEnd(packet->content, (ssize) size);
    }
}


PUBLIC HttpPacket *httpGetPacket(HttpQueue *q)
{
    HttpQueue     *prev;
    HttpPacket    *packet;

    while (q->first) {
        if ((packet = q->first) != 0) {
            q->first = packet->next;
            packet->next = 0;
            q->count -= httpGetPacketLength(packet);
            assert(q->count >= 0);
            if (packet == q->last) {
                q->last = 0;
                assert(q->first == 0);
            }
            if (q->first == 0) {
                assert(q->last == 0);
            }
        }
        if (q->count < q->low) {
            prev = httpFindPreviousQueue(q);
            if (prev && prev->flags & HTTP_QUEUE_SUSPENDED) {
                /*
                    This queue was full and now is below the low water mark. Back-enable the previous queue.
                 */
                httpResumeQueue(prev);
            }
        }
        return packet;
    }
    return 0;
}


/*  
    Test if the packet is too too large to be accepted by the downstream queue.
 */
PUBLIC bool httpIsPacketTooBig(HttpQueue *q, HttpPacket *packet)
{
    ssize   size;
    
    size = mprGetBufLength(packet->content);
    return size > q->max || size > q->packetSize;
}


/*  
    Join a packet onto the service queue. This joins packet content data.
 */
PUBLIC void httpJoinPacketForService(HttpQueue *q, HttpPacket *packet, bool serviceQ)
{
    if (q->first == 0) {
        /*  Just use the service queue as a holding queue while we aggregate the post data.  */
        httpPutForService(q, packet, HTTP_DELAY_SERVICE);

    } else {
        /* Skip over the header packet */
        if (q->first && q->first->flags & HTTP_PACKET_HEADER) {
            packet = q->first->next;
            q->first = packet;
        } else {
            /* Aggregate all data into one packet and free the packet.  */
            httpJoinPacket(q->first, packet);
        }
        q->count += httpGetPacketLength(packet);
    }
    VERIFY_QUEUE(q);
    if (serviceQ && !(q->flags & HTTP_QUEUE_SUSPENDED))  {
        httpScheduleQueue(q);
    }
}


/*  
    Join two packets by pulling the content from the second into the first.
    WARNING: this will not update the queue count. Assumes the either both are on the queue or neither. 
 */
PUBLIC int httpJoinPacket(HttpPacket *packet, HttpPacket *p)
{
    ssize   len;

    assert(packet->esize == 0);
    assert(p->esize == 0);
    assert(!(packet->flags & HTTP_PACKET_SOLO));
    assert(!(p->flags & HTTP_PACKET_SOLO));

    len = httpGetPacketLength(p);
    if (mprPutBlockToBuf(packet->content, mprGetBufStart(p->content), (ssize) len) != len) {
        assert(0);
        return MPR_ERR_MEMORY;
    }
    return 0;
}


/*
    Join queue packets up to the maximum of the given size and the downstream queue packet size.
    WARNING: this will not update the queue count.
 */
PUBLIC void httpJoinPackets(HttpQueue *q, ssize size)
{
    HttpPacket  *packet, *first;
    ssize       len;

    if (size < 0) {
        size = MAXINT;
    }
    if ((first = q->first) != 0 && first->next) {
        if (first->flags & HTTP_PACKET_HEADER) {
            /* Step over a header packet */
            first = first->next;
        }
        for (packet = first->next; packet; packet = packet->next) {
            if (packet->content == 0 || (len = httpGetPacketLength(packet)) == 0) {
                break;
            }
            assert(!(packet->flags & HTTP_PACKET_END));
            httpJoinPacket(first, packet);
            /* Unlink the packet */
            first->next = packet->next;
            if (q->last == packet) {
                q->last = first;
            }
        }
    }
}


PUBLIC void httpPutPacket(HttpQueue *q, HttpPacket *packet)
{
    assert(packet);
    assert(q->put);

    q->put(q, packet);
}


/*  
    Pass to the next stage in the pipeline
 */
PUBLIC void httpPutPacketToNext(HttpQueue *q, HttpPacket *packet)
{
    assert(packet);
    assert(q->nextQ->put);

    q->nextQ->put(q->nextQ, packet);
}


PUBLIC void httpPutPackets(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        httpPutPacketToNext(q, packet);
    }
}


/*  
    Put the packet back at the front of the queue
 */
PUBLIC void httpPutBackPacket(HttpQueue *q, HttpPacket *packet)
{
    assert(packet);
    assert(packet->next == 0);
    assert(q->count >= 0);
    
    if (packet) {
        packet->next = q->first;
        if (q->first == 0) {
            q->last = packet;
        }
        q->first = packet;
        q->count += httpGetPacketLength(packet);
    }
}


/*  
    Put a packet on the service queue.
 */
PUBLIC void httpPutForService(HttpQueue *q, HttpPacket *packet, bool serviceQ)
{
    assert(packet);
   
    q->count += httpGetPacketLength(packet);
    packet->next = 0;
    
    if (q->first) {
        q->last->next = packet;
        q->last = packet;
    } else {
        q->first = packet;
        q->last = packet;
    }
    if (serviceQ && !(q->flags & HTTP_QUEUE_SUSPENDED))  {
        httpScheduleQueue(q);
    }
}


/*  
    Resize and possibly split a packet so it fits in the downstream queue. Put back the 2nd portion of the split packet 
    on the queue. Ensure that the packet is not larger than "size" if it is greater than zero. If size < 0, then
    use the default packet size. 
 */
PUBLIC int httpResizePacket(HttpQueue *q, HttpPacket *packet, ssize size)
{
    HttpPacket  *tail;
    ssize       len;
    
    if (size <= 0) {
        size = MAXINT;
    }
    if (packet->esize > size) {
        if ((tail = httpSplitPacket(packet, size)) == 0) {
            return MPR_ERR_MEMORY;
        }
    } else {
        /*  
            Calculate the size that will fit downstream
         */
        len = packet->content ? httpGetPacketLength(packet) : 0;
        size = min(size, len);
        size = min(size, q->nextQ->max);
        size = min(size, q->nextQ->packetSize);
        if (size == 0 || size == len) {
            return 0;
        }
        if ((tail = httpSplitPacket(packet, size)) == 0) {
            return MPR_ERR_MEMORY;
        }
    }
    httpPutBackPacket(q, tail);
    return 0;
}


/*
    Split a packet at a given offset and return a new packet containing the data after the offset.
    The prefix data remains with the original packet. 
 */
PUBLIC HttpPacket *httpSplitPacket(HttpPacket *orig, ssize offset)
{
    HttpPacket  *packet;
    ssize       count, size;

    /* Must not be in a queue */
    assert(orig->next == 0);

    if (orig->esize) {
        if ((packet = httpCreateEntityPacket(orig->epos + offset, orig->esize - offset, orig->fill)) == 0) {
            return 0;
        }
        orig->esize = offset;

    } else {
        if (offset >= httpGetPacketLength(orig)) {
            assert(offset < httpGetPacketLength(orig));
            return 0;
        }
        /*
            OPT - A large packet will often be resized by splitting into chunks that the
            downstream queues will accept. This causes many allocations that are a small delta less than the large
            packet 
            Better:
                - If offset is < size/2
                    - Allocate packet size == offset
                    - Set orig->content =   
                    - packet->content = orig->content
                        httpAdjustPacketStart(packet, offset) 
                    - orig->content = Packet(size == offset)
                        copy from packet
                Adjust the content->start
         */
        count = httpGetPacketLength(orig) - offset;
        size = max(count, BIT_MAX_BUFFER);
        size = HTTP_PACKET_ALIGN(size);
        if ((packet = httpCreateDataPacket(size)) == 0) {
            return 0;
        }
        httpAdjustPacketEnd(orig, (ssize) -count);
        if (mprPutBlockToBuf(packet->content, mprGetBufEnd(orig->content), (ssize) count) != count) {
            return 0;
        }
#if BIT_DEBUG
        mprAddNullToBuf(orig->content);
#endif
    }
    packet->flags = orig->flags;
    packet->type = orig->type;
    packet->last = orig->last;
    return packet;
}


bool httpIsLastPacket(HttpPacket *packet) 
{
    return packet->last;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/pam.c"
 */
/************************************************************************/

/*
    authPam.c - Authorization using PAM (Pluggable Authorization Module)

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



#if BIT_HAS_PAM && BIT_HTTP_PAM
 #include    <security/pam_appl.h>

/********************************* Defines ************************************/

typedef struct {
    char    *name;
    char    *password;
} UserInfo;

#if MACOSX
    typedef int Gid;
#else
    typedef gid_t Gid;
#endif

/********************************* Forwards ***********************************/

static int pamChat(int msgCount, const struct pam_message **msg, struct pam_response **resp, void *data);

/*********************************** Code *************************************/

PUBLIC bool httpPamVerifyUser(HttpConn *conn)
{
    MprBuf              *abilities;
    pam_handle_t        *pamh;
    UserInfo            info;
    struct pam_conv     conv = { pamChat, &info };
    struct group        *gp;
    int                 res, i;
   
    assert(conn->username);
    assert(conn->password);
    assert(!conn->encoded);

    info.name = (char*) conn->username;
    info.password = (char*) conn->password;
    pamh = NULL;
    if ((res = pam_start("login", info.name, &conv, &pamh)) != PAM_SUCCESS) {
        return 0;
    }
    if ((res = pam_authenticate(pamh, PAM_DISALLOW_NULL_AUTHTOK)) != PAM_SUCCESS) {
        pam_end(pamh, PAM_SUCCESS);
        mprTrace(5, "httpPamVerifyUser failed to verify %s", conn->username);
        return 0;
    }
    pam_end(pamh, PAM_SUCCESS);
    mprTrace(5, "httpPamVerifyUser verified %s", conn->username);

    if (!conn->user) {
        conn->user = mprLookupKey(conn->rx->route->auth->users, conn->username);
    }
    if (!conn->user) {
        Gid     groups[32];
        int     ngroups;
        /* 
            Create a temporary user with a abilities set to the groups 
         */
        ngroups = sizeof(groups) / sizeof(Gid);
        if ((i = getgrouplist(conn->username, 99999, groups, &ngroups)) >= 0) {
            abilities = mprCreateBuf(0, 0);
            for (i = 0; i < ngroups; i++) {
                if ((gp = getgrgid(groups[i])) != 0) {
                    mprPutToBuf(abilities, "%s ", gp->gr_name);
                }
            }
            mprAddNullToBuf(abilities);
            mprTrace(5, "Create temp user \"%s\" with abilities: %s", conn->username, mprGetBufStart(abilities));
            /*
                Create a user and map groups to roles and expand to abilities
             */
            conn->user = httpCreateUser(conn->rx->route->auth, conn->username, 0, mprGetBufStart(abilities));
        }
    }
    return 1;
}

/*  
    Callback invoked by the pam_authenticate function
 */
static int pamChat(int msgCount, const struct pam_message **msg, struct pam_response **resp, void *data) 
{
    UserInfo                *info;
    struct pam_response     *reply;
    int                     i;
    
    i = 0;
    reply = 0;
    info = (UserInfo*) data;

    if (resp == 0 || msg == 0 || info == 0) {
        return PAM_CONV_ERR;
    }
    if ((reply = calloc(msgCount, sizeof(struct pam_response))) == 0) {
        return PAM_CONV_ERR;
    }
    for (i = 0; i < msgCount; i++) {
        reply[i].resp_retcode = 0;
        reply[i].resp = 0;
        
        switch (msg[i]->msg_style) {
        case PAM_PROMPT_ECHO_ON:
            reply[i].resp = strdup(info->name);
            break;

        case PAM_PROMPT_ECHO_OFF:
            /* Retrieve the user password and pass onto pam */
            reply[i].resp = strdup(info->password);
            break;

        default:
            free(reply);
            return PAM_CONV_ERR;
        }
    }
    *resp = reply;
    return PAM_SUCCESS;
}
#endif /* BIT_HAS_PAM */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/passHandler.c"
 */
/************************************************************************/

/*
    passHandler.c -- Pass through handler

    This handler simply relays all content to a network connector. It is used for the ErrorHandler and 
    when there is no handler defined. It is configured as the "passHandler" and "errorHandler".

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Code *************************************/

static void startPass(HttpQueue *q)
{
    mprTrace(5, "Start passHandler");
    if (q->conn->rx->flags & (HTTP_OPTIONS | HTTP_TRACE)) {
        httpHandleOptionsTrace(q->conn, "");
    }
}


static void readyPass(HttpQueue *q)
{
    httpFinalize(q->conn);
}


static void readyError(HttpQueue *q)
{
    if (!q->conn->error) {
        /*
            The ErrorHandler emits this error always
         */
        httpError(q->conn, HTTP_CODE_SERVICE_UNAVAILABLE, "The requested resource is not available");
    }
    httpFinalize(q->conn);
}


/*
    Handle Trace and Options requests. Handlers can do this themselves if they desire, but typically
    all Trace/Options requests come here.
 */
PUBLIC void httpHandleOptionsTrace(HttpConn *conn, cchar *methods)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    HttpQueue   *q;
    HttpPacket  *traceData, *headers;
    MprKey      *method;

    tx = conn->tx;
    rx = conn->rx;
    route = rx->route;

    if (rx->flags & HTTP_TRACE) {
        /* The trace method is disabled by default unless 'TraceMethod on' is specified */
        if (!(route->flags & HTTP_ROUTE_TRACE_METHOD)) {
            tx->status = HTTP_CODE_NOT_ACCEPTABLE;
            httpFormatResponseBody(conn, "Trace Request Denied", "The TRACE method is disabled for this resource.");
        } else {
            /*
                Create a dummy set of headers to use as the response body. Then reset so the connector will
                create the headers in the normal fashion. Need to be careful not to have a content length in the
                headers in the body.
             */
            q = conn->writeq;
            headers = q->first;
            tx->flags |= HTTP_TX_NO_LENGTH;
            httpWriteHeaders(q, headers);
            traceData = httpCreateDataPacket(httpGetPacketLength(headers) + 128);
            tx->flags &= ~(HTTP_TX_NO_LENGTH | HTTP_TX_HEADERS_CREATED);
            q->count -= httpGetPacketLength(headers);
            assert(q->count == 0);
            mprFlushBuf(headers->content);
            mprPutToBuf(traceData->content, mprGetBufStart(q->first->content));
            httpSetContentType(conn, "message/http");
            httpPutForService(q, traceData, HTTP_DELAY_SERVICE);
        }

    } else if (rx->flags & HTTP_OPTIONS) {
        if (rx->route->methods) {
            methods = 0;
            for (ITERATE_KEYS(route->methods, method)) {
                methods = (methods) ? sjoin(methods, ",", method->key, 0) : method->key;
            }
            httpSetHeader(conn, "Allow", "%s", methods);
        } else {
            httpSetHeader(conn, "Allow", "OPTIONS,%s%s", (route->flags & HTTP_ROUTE_TRACE_METHOD) ? "TRACE," : "", methods);
        }
        assert(tx->length <= 0);
    }
    httpFinalize(conn);
}


PUBLIC int httpOpenPassHandler(Http *http)
{
    HttpStage     *stage;

    if ((stage = httpCreateHandler(http, "passHandler", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->passHandler = stage;
    stage->start = startPass;
    stage->ready = readyPass;

    /*
        PassHandler is an alias as the ErrorHandler too
     */
    if ((stage = httpCreateHandler(http, "errorHandler", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->start = startPass;
    stage->ready = readyError;
    return 0;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/pipeline.c"
 */
/************************************************************************/

/*
    pipeline.c -- HTTP pipeline processing. 
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forward ***********************************/

static bool matchFilter(HttpConn *conn, HttpStage *filter, HttpRoute *route, int dir);
static void openQueues(HttpConn *conn);
static void pairQueues(HttpConn *conn);
static void httpStartHandler(HttpConn *conn);

/*********************************** Code *************************************/

PUBLIC void httpCreateTxPipeline(HttpConn *conn, HttpRoute *route)
{
    Http        *http;
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    HttpStage   *stage, *filter;
    int         next, hasOutputFilters;

    assert(conn);
    assert(route);

    http = conn->http;
    rx = conn->rx;
    tx = conn->tx;

    tx->outputPipeline = mprCreateList(-1, 0);
    if (conn->endpoint) {
        if (tx->handler == 0 || tx->finalized) {
            tx->handler = http->passHandler;
        }
        mprAddItem(tx->outputPipeline, tx->handler);
    }
    hasOutputFilters = 0;
    if (route->outputStages) {
        for (next = 0; (filter = mprGetNextItem(route->outputStages, &next)) != 0; ) {
            if (matchFilter(conn, filter, route, HTTP_STAGE_TX) == HTTP_ROUTE_OK) {
                mprAddItem(tx->outputPipeline, filter);
                if (rx->traceLevel >= 0) {
                    mprLog(rx->traceLevel, "Select output filter: \"%s\"", filter->name);
                }
                hasOutputFilters = 1;
            }
        }
    }
    if (tx->connector == 0) {
#if !BIT_ROM
        if (tx->handler == http->fileHandler && (rx->flags & HTTP_GET) && !hasOutputFilters && 
                !conn->secure && httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_BODY, tx->ext) < 0) {
            tx->connector = http->sendConnector;
        } else 
#endif
        if (route && route->connector) {
            tx->connector = route->connector;
        } else {
            tx->connector = http->netConnector;
        }
    }
    mprAddItem(tx->outputPipeline, tx->connector);
    if (rx->traceLevel >= 0) {
        mprLog(rx->traceLevel + 1, "Select connector: \"%s\"", tx->connector->name);
    }
    /*  Create the outgoing queue heads and open the queues */
    q = tx->queue[HTTP_QUEUE_TX];
    for (next = 0; (stage = mprGetNextItem(tx->outputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_TX, q);
    }
    conn->connectorq = tx->queue[HTTP_QUEUE_TX]->prevQ;

    /*
        Double the connector max hi-water mark. This optimization permits connectors to accept packets without 
        unnecesary flow control.
     */
    conn->connectorq->max *= 2;

    pairQueues(conn);

    /*
        Put the header before opening the queues incase an open routine actually services and completes the request
        httpHandleOptionsTrace does this when called from openFile() in fileHandler.
     */
    httpPutForService(conn->writeq, httpCreateHeaderPacket(), HTTP_DELAY_SERVICE);
    openQueues(conn);

#if FUTURE
    if (rx->upgrade && !conn->upgraded) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Cannot upgrade communications protocol");
    }
#endif
    if (tx->pendingFinalize) {
        tx->finalizedOutput = 0;
        httpFinalizeOutput(conn);
    }
}


PUBLIC void httpCreateRxPipeline(HttpConn *conn, HttpRoute *route)
{
    HttpTx      *tx;
    HttpRx      *rx;
    HttpQueue   *q;
    HttpStage   *stage, *filter;
    int         next;

    assert(conn);
    assert(route);

    rx = conn->rx;
    tx = conn->tx;
    rx->inputPipeline = mprCreateList(-1, 0);
    if (route) {
        for (next = 0; (filter = mprGetNextItem(route->inputStages, &next)) != 0; ) {
            if (matchFilter(conn, filter, route, HTTP_STAGE_RX) == HTTP_ROUTE_OK) {
                mprAddItem(rx->inputPipeline, filter);
            }
        }
    }
    mprAddItem(rx->inputPipeline, tx->handler ? tx->handler : conn->http->clientHandler);
    /*  Create the incoming queue heads and open the queues.  */
    q = tx->queue[HTTP_QUEUE_RX];
    for (next = 0; (stage = mprGetNextItem(rx->inputPipeline, &next)) != 0; ) {
        q = httpCreateQueue(conn, stage, HTTP_QUEUE_RX, q);
    }
    if (!conn->endpoint) {
        pairQueues(conn);
        openQueues(conn);
    }
}


static void pairQueues(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead, *rq, *rqhead;

    tx = conn->tx;
    qhead = tx->queue[HTTP_QUEUE_TX];
    rqhead = tx->queue[HTTP_QUEUE_RX];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        if (q->pair == 0) {
            for (rq = rqhead->nextQ; rq != rqhead; rq = rq->nextQ) {
                if (q->stage == rq->stage) {
                    q->pair = rq;
                    rq->pair = q;
                }
            }
        }
    }
}


static int openQueue(HttpQueue *q, ssize chunkSize)
{
    Http        *http;
    HttpConn    *conn;
    HttpStage   *stage;
    MprModule   *module;

    stage = q->stage;
    conn = q->conn;
    http = q->conn->http;

    if (chunkSize > 0) {
        q->packetSize = min(q->packetSize, chunkSize);
    }
    if (stage->flags & HTTP_STAGE_UNLOADED && stage->module) {
        module = stage->module;
        module = mprCreateModule(module->name, module->path, module->entry, http);
        if (mprLoadModule(module) < 0) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot load module %s", module->name);
            return MPR_ERR_CANT_READ;
        }
        stage->module = module;
    }
    if (stage->module) {
        stage->module->lastActivity = http->now;
    }
    return 0;
}


static void openQueues(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    int         i;

    tx = conn->tx;
    for (i = 0; i < HTTP_MAX_QUEUE; i++) {
        qhead = tx->queue[i];
        for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
            if (q->open && !(q->flags & (HTTP_QUEUE_OPEN))) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_OPEN)) {
                    openQueue(q, tx->chunkSize);
                    if (q->open && !tx->finalized) {
                        q->flags |= HTTP_QUEUE_OPEN;
                        q->stage->open(q);
                    }
                }
            }
        }
    }
}


PUBLIC void httpSetSendConnector(HttpConn *conn, cchar *path)
{
#if !BIT_ROM
    HttpTx      *tx;

    tx = conn->tx;
    tx->flags |= HTTP_TX_SENDFILE;
    tx->filename = sclone(path);
#else
    mprError("Send connector not available if ROMFS enabled");
#endif
}


PUBLIC void httpDestroyPipeline(HttpConn *conn)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;
    int         i;

    tx = conn->tx;
    if (tx) {
        for (i = 0; i < HTTP_MAX_QUEUE; i++) {
            qhead = tx->queue[i];
            for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
                if (q->close && q->flags & HTTP_QUEUE_OPEN) {
                    q->flags &= ~HTTP_QUEUE_OPEN;
                    q->stage->close(q);
                }
            }
        }
    }
}


PUBLIC void httpStartPipeline(HttpConn *conn)
{
    HttpQueue   *qhead, *q, *prevQ, *nextQ;
    HttpTx      *tx;
    HttpRx      *rx;
    
    tx = conn->tx;
    tx->started = 1;
    rx = conn->rx;
    if (rx->needInputPipeline) {
        qhead = tx->queue[HTTP_QUEUE_RX];
        for (q = qhead->nextQ; !tx->finalized && q->nextQ != qhead; q = nextQ) {
            nextQ = q->nextQ;
            if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
                if (q->pair == 0 || !(q->pair->flags & HTTP_QUEUE_STARTED)) {
                    q->flags |= HTTP_QUEUE_STARTED;
                    q->stage->start(q);
                }
            }
        }
    }
    qhead = tx->queue[HTTP_QUEUE_TX];
    for (q = qhead->prevQ; !tx->finalized && q->prevQ != qhead; q = prevQ) {
        prevQ = q->prevQ;
        if (q->start && !(q->flags & HTTP_QUEUE_STARTED)) {
            q->flags |= HTTP_QUEUE_STARTED;
            q->stage->start(q);
        }
    }
    /* Start the handler last */
    q = qhead->nextQ;
    httpStartHandler(conn);
    if (!tx->finalized && !tx->finalizedConnector && rx->remainingContent > 0) {
        /* If no remaining content, wait till the processing stage to avoid duplicate writable events */
        HTTP_NOTIFY(conn, HTTP_EVENT_WRITABLE, 0);
    }
}


PUBLIC void httpReadyHandler(HttpConn *conn)
{
    HttpQueue   *q;
    
    q = conn->writeq;
    if (q->stage && q->stage->ready && !conn->tx->finalized && !(q->flags & HTTP_QUEUE_READY)) {
        q->flags |= HTTP_QUEUE_READY;
        q->stage->ready(q);
    }
}


static void httpStartHandler(HttpConn *conn)
{
    HttpQueue   *q;
    
    q = conn->writeq;
    if (q->stage->start && !conn->tx->finalized && !(q->flags & HTTP_QUEUE_STARTED)) {
        q->flags |= HTTP_QUEUE_STARTED;
        q->stage->start(q);
    }
}


/*
    Get more output by invoking the stage 'writable' callback. Called by processRunning.
 */
PUBLIC bool httpGetMoreOutput(HttpConn *conn)
{
    HttpQueue   *q;
    
    q = conn->writeq;
    if (!q->stage || !q->stage->writable) {
       return 0;
    }
    if (!conn->tx->finalizedOutput) {
        q->stage->writable(q);
        if (q->count > 0) {
            httpScheduleQueue(q);
            httpServiceQueues(conn);
        }
    }
    return 1;
}


/*  
    Run the queue service routines until there is no more work to be done. NOTE: all I/O is non-blocking.
 */
PUBLIC bool httpServiceQueues(HttpConn *conn)
{
    HttpQueue   *q;
    int         workDone;

    workDone = 0;
    while (conn->state < HTTP_STATE_COMPLETE && (q = httpGetNextQueueForService(conn->serviceq)) != NULL) {
        if (q->servicing) {
            q->flags |= HTTP_QUEUE_RESERVICE;
        } else {
            assert(q->schedulePrev == q->scheduleNext);
            httpServiceQueue(q);
            workDone = 1;
        }
    }
    return workDone;
}


PUBLIC void httpDiscardData(HttpConn *conn, int dir)
{
    HttpTx      *tx;
    HttpQueue   *q, *qhead;

    tx = conn->tx;
    if (tx == 0) {
        return;
    }
    qhead = tx->queue[dir];
    for (q = qhead->nextQ; q != qhead; q = q->nextQ) {
        httpDiscardQueueData(q, 1);
    }
}


static bool matchFilter(HttpConn *conn, HttpStage *filter, HttpRoute *route, int dir)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (filter->match) {
        return filter->match(conn, route, dir);
    }
    if (filter->extensions && tx->ext) {
        return mprLookupKey(filter->extensions, tx->ext) != 0;
    }
    return 1;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/queue.c"
 */
/************************************************************************/

/*
    queue.c -- Queue support routines. Queues are the bi-directional data flow channels for the pipeline.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static void manageQueue(HttpQueue *q, int flags);

/************************************ Code ************************************/

PUBLIC HttpQueue *httpCreateQueueHead(HttpConn *conn, cchar *name)
{
    HttpQueue   *q;

    if ((q = mprAllocObj(HttpQueue, manageQueue)) == 0) {
        return 0;
    }
    httpInitQueue(conn, q, name);
    httpInitSchedulerQueue(q);
    return q;
}


/*
    Create a queue associated with a connection.
    Prev may be set to the previous queue in a pipeline. If so, then the Conn.readq and writeq are updated.
 */
PUBLIC HttpQueue *httpCreateQueue(HttpConn *conn, HttpStage *stage, int dir, HttpQueue *prev)
{
    HttpQueue   *q;

    if ((q = mprAllocObj(HttpQueue, manageQueue)) == 0) {
        return 0;
    }
    q->conn = conn;
    httpInitQueue(conn, q, sfmt("%s-%s", stage->name, dir == HTTP_QUEUE_TX ? "tx" : "rx"));
    httpInitSchedulerQueue(q);
    httpAssignQueue(q, stage, dir);
    if (prev) {
        httpAppendQueue(prev, q);
        if (dir == HTTP_QUEUE_RX) {
            conn->readq = conn->tx->queue[HTTP_QUEUE_RX]->prevQ;
        } else {
            conn->writeq = conn->tx->queue[HTTP_QUEUE_TX]->nextQ;
        }
    }
    return q;
}


static void manageQueue(HttpQueue *q, int flags)
{
    HttpPacket      *packet;

    if (flags & MPR_MANAGE_MARK) {
        mprMark(q->name);
        for (packet = q->first; packet; packet = packet->next) {
            mprMark(packet);
        }
        mprMark(q->last);
        mprMark(q->nextQ);
        mprMark(q->prevQ);
        mprMark(q->stage);
        mprMark(q->conn);
        mprMark(q->scheduleNext);
        mprMark(q->schedulePrev);
        mprMark(q->pair);
        mprMark(q->queueData);
        if (q->nextQ && q->nextQ->stage) {
            /* Not a queue head */
            mprMark(q->nextQ);
        }
    }
}


PUBLIC void httpAssignQueue(HttpQueue *q, HttpStage *stage, int dir)
{
    q->stage = stage;
    q->close = stage->close;
    q->open = stage->open;
    q->start = stage->start;
    if (dir == HTTP_QUEUE_TX) {
        q->put = stage->outgoing;
        q->service = stage->outgoingService;
    } else {
        q->put = stage->incoming;
        q->service = stage->incomingService;
    }
}


PUBLIC void httpInitQueue(HttpConn *conn, HttpQueue *q, cchar *name)
{
    HttpTx      *tx;

    tx = conn->tx;
    q->conn = conn;
    q->nextQ = q;
    q->prevQ = q;
    q->name = sclone(name);
    q->max = conn->limits->bufferSize;
    q->low = q->max / 100 *  5;    
    if (tx && tx->chunkSize > 0) {
        q->packetSize = tx->chunkSize;
    } else {
        q->packetSize = q->max;
    }
}


PUBLIC void httpSetQueueLimits(HttpQueue *q, ssize low, ssize max)
{
    q->low = low;
    q->max = max;
}


#if KEEP
/*  
    Insert a queue after the previous element
 */
PUBLIC void httpAppendQueueToHead(HttpQueue *head, HttpQueue *q)
{
    q->nextQ = head;
    q->prevQ = head->prevQ;
    head->prevQ->nextQ = q;
    head->prevQ = q;
}
#endif


PUBLIC bool httpIsQueueSuspended(HttpQueue *q)
{
    return (q->flags & HTTP_QUEUE_SUSPENDED) ? 1 : 0;
}


PUBLIC void httpSuspendQueue(HttpQueue *q)
{
    mprTrace(7, "Suspend q %s", q->name);
    q->flags |= HTTP_QUEUE_SUSPENDED;
}


PUBLIC bool httpIsSuspendQueue(HttpQueue *q)
{
    return q->flags & HTTP_QUEUE_SUSPENDED;
}



/*  
    Remove all data in the queue. If removePackets is true, actually remove the packet too.
    This preserves the header and EOT packets.
 */
PUBLIC void httpDiscardQueueData(HttpQueue *q, bool removePackets)
{
    HttpPacket  *packet, *prev, *next;
    ssize       len;

    if (q == 0) {
        return;
    }
    for (prev = 0, packet = q->first; packet; packet = next) {
        next = packet->next;
        if (packet->flags & (HTTP_PACKET_RANGE | HTTP_PACKET_DATA)) {
            if (removePackets) {
                if (prev) {
                    prev->next = next;
                } else {
                    q->first = next;
                }
                if (packet == q->last) {
                    q->last = prev;
                }
                q->count -= httpGetPacketLength(packet);
                assert(q->count >= 0);
                continue;
            } else {
                len = httpGetPacketLength(packet);
                q->conn->tx->length -= len;
                q->count -= len;
                assert(q->count >= 0);
                if (packet->content) {
                    mprFlushBuf(packet->content);
                }
            }
        }
        prev = packet;
    }
}


/*  
    Flush queue data by scheduling the queue and servicing all scheduled queues. Return true if there is room for more data.
    If blocking is requested, the call will block until the queue count falls below the queue max.
    WARNING: Be very careful when using blocking == true. Should only be used by end applications and not by middleware.
 */
PUBLIC bool httpFlushQueue(HttpQueue *q, bool blocking)
{
    HttpConn    *conn;
    HttpQueue   *next;

    conn = q->conn;
    assert(conn->sock);
    do {
        httpScheduleQueue(q);
        next = q->nextQ;
        if (next->count >= next->max) {
            httpScheduleQueue(next);
        }
        httpServiceQueues(conn);
        if (conn->sock == 0) {
            break;
        }
        if (blocking) {
            httpGetMoreOutput(conn);
        }
    } while (blocking && q->count > 0);
    return (q->count < q->max) ? 1 : 0;
}


PUBLIC void httpResumeQueue(HttpQueue *q)
{
    mprTrace(7, "Resume q %s", q->name);
    q->flags &= ~HTTP_QUEUE_SUSPENDED;
    httpScheduleQueue(q);
}


PUBLIC HttpQueue *httpFindPreviousQueue(HttpQueue *q)
{
    while (q->prevQ && q->prevQ->stage && q->prevQ != q) {
        q = q->prevQ;
        if (q->service) {
            return q;
        }
    }
    return 0;
}


PUBLIC HttpQueue *httpGetNextQueueForService(HttpQueue *q)
{
    HttpQueue     *next;
    
    if (q->scheduleNext != q) {
        next = q->scheduleNext;
        next->schedulePrev->scheduleNext = next->scheduleNext;
        next->scheduleNext->schedulePrev = next->schedulePrev;
        next->schedulePrev = next->scheduleNext = next;
        return next;
    }
    return 0;
}


/*  
    Return the number of bytes the queue will accept. Always positive.
 */
PUBLIC ssize httpGetQueueRoom(HttpQueue *q)
{
    assert(q->max > 0);
    assert(q->count >= 0);
    
    if (q->count >= q->max) {
        return 0;
    }
    return q->max - q->count;
}


PUBLIC void httpInitSchedulerQueue(HttpQueue *q)
{
    q->scheduleNext = q;
    q->schedulePrev = q;
}


/*  
    Append a queue after the previous element
 */
PUBLIC void httpAppendQueue(HttpQueue *prev, HttpQueue *q)
{
    q->nextQ = prev->nextQ;
    q->prevQ = prev;
    prev->nextQ->prevQ = q;
    prev->nextQ = q;
}


PUBLIC bool httpIsQueueEmpty(HttpQueue *q)
{
    return q->first == 0;
}


/*  
    Read data. If sync mode, this will block. If async, will never block.
    Will return what data is available up to the requested size. 
    Returns a count of bytes read. Returns zero if not data. EOF if returns zero and conn->state is > HTTP_STATE_CONTENT.
 */
PUBLIC ssize httpRead(HttpConn *conn, char *buf, ssize size)
{
    HttpPacket  *packet;
    HttpQueue   *q;
    MprBuf      *content;
    ssize       nbytes, len;

    q = conn->readq;
    assert(q->count >= 0);
    assert(size >= 0);
    VERIFY_QUEUE(q);

    while (q->count <= 0 && !conn->async && !conn->error && conn->sock && (conn->state <= HTTP_STATE_CONTENT)) {
        httpServiceQueues(conn);
        if (conn->sock) {
            httpWait(conn, 0, MPR_TIMEOUT_NO_BUSY);
        }
    }
    conn->lastActivity = conn->http->now;

    for (nbytes = 0; size > 0 && q->count > 0; ) {
        if ((packet = q->first) == 0) {
            break;
        }
        content = packet->content;
        len = mprGetBufLength(content);
        len = min(len, size);
        assert(len <= q->count);
        if (len > 0) {
            len = mprGetBlockFromBuf(content, buf, len);
            assert(len <= q->count);
        }
        buf += len;
        size -= len;
        q->count -= len;
        assert(q->count >= 0);
        nbytes += len;
        if (mprGetBufLength(content) == 0) {
            httpGetPacket(q);
        }
    }
    assert(q->count >= 0);
    if (nbytes < size) {
        buf[nbytes] = '\0';
    }
    return nbytes;
}


PUBLIC ssize httpGetReadCount(HttpConn *conn)
{
    return conn->readq->count;
}


PUBLIC bool httpIsEof(HttpConn *conn) 
{
    return conn->rx == 0 || conn->rx->eof;
}


/*
    Read data as a string
 */
PUBLIC char *httpReadString(HttpConn *conn)
{
    HttpRx      *rx;
    ssize       sofar, nbytes, remaining;
    char        *content;

    rx = conn->rx;
    remaining = (ssize) min(MAXSSIZE, rx->length);

    if (remaining > 0) {
        if ((content = mprAlloc(remaining + 1)) == 0) {
            return 0;
        }
        sofar = 0;
        while (remaining > 0) {
            nbytes = httpRead(conn, &content[sofar], remaining);
            if (nbytes < 0) {
                return 0;
            }
            sofar += nbytes;
            remaining -= nbytes;
        }
    } else {
        content = mprAlloc(BIT_MAX_BUFFER);
        sofar = 0;
        while (1) {
            nbytes = httpRead(conn, &content[sofar], BIT_MAX_BUFFER);
            if (nbytes < 0) {
                return 0;
            } else if (nbytes == 0) {
                break;
            }
            sofar += nbytes;
            content = mprRealloc(content, sofar + BIT_MAX_BUFFER);
        }
    }
    content[sofar] = '\0';
    return content;
}


PUBLIC void httpRemoveQueue(HttpQueue *q)
{
    q->prevQ->nextQ = q->nextQ;
    q->nextQ->prevQ = q->prevQ;
    q->prevQ = q->nextQ = q;
}


PUBLIC void httpScheduleQueue(HttpQueue *q)
{
    HttpQueue     *head;
    
    assert(q->conn);
    head = q->conn->serviceq;
    
    if (q->scheduleNext == q && !(q->flags & HTTP_QUEUE_SUSPENDED)) {
        q->scheduleNext = head;
        q->schedulePrev = head->schedulePrev;
        head->schedulePrev->scheduleNext = q;
        head->schedulePrev = q;
    }
}


PUBLIC void httpServiceQueue(HttpQueue *q)
{
    q->conn->currentq = q;

    if (q->servicing) {
        q->flags |= HTTP_QUEUE_RESERVICE;
    } else {
        /*  
            Since we are servicing this "q" now, we can remove from the schedule queue if it is already queued.
         */
        if (q->conn->serviceq->scheduleNext == q) {
            httpGetNextQueueForService(q->conn->serviceq);
        }
        if (!(q->flags & HTTP_QUEUE_SUSPENDED)) {
            q->servicing = 1;
            q->service(q);
            if (q->flags & HTTP_QUEUE_RESERVICE) {
                q->flags &= ~HTTP_QUEUE_RESERVICE;
                httpScheduleQueue(q);
            }
            q->flags |= HTTP_QUEUE_SERVICED;
            q->servicing = 0;
        }
    }
}


/*  
    Return true if the next queue will accept this packet. If not, then disable the queue's service procedure.
    This may split the packet if it exceeds the downstreams maximum packet size.
 */
PUBLIC bool httpWillNextQueueAcceptPacket(HttpQueue *q, HttpPacket *packet)
{
    HttpQueue   *nextQ;
    ssize       size;

    nextQ = q->nextQ;
    size = httpGetPacketLength(packet);
    if (size <= nextQ->packetSize && (size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    if (httpResizePacket(q, packet, 0) < 0) {
        return 0;
    }
    size = httpGetPacketLength(packet);
    assert(size <= nextQ->packetSize);
    /* 
        Packet size is now acceptable. Accept the packet if the queue is mostly empty (< low) or if the 
        packet will fit entirely under the max or if the queue.
        NOTE: queue maximums are advisory. We choose to potentially overflow the max here to optimize the case where
        the queue may have say one byte and a max size packet would overflow by 1.
     */
    if (nextQ->count < nextQ->low || (size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    /*  
        The downstream queue cannot accept this packet, so disable queue and mark the downstream queue as full and service 
     */
    httpSuspendQueue(q);
    if (!(nextQ->flags & HTTP_QUEUE_SUSPENDED)) {
        httpScheduleQueue(nextQ);
    }
    return 0;
}


PUBLIC bool httpWillQueueAcceptPacket(HttpQueue *q, HttpPacket *packet, bool split)
{
    ssize       size;

    size = httpGetPacketLength(packet);
    if (size <= q->packetSize && (size + q->count) <= q->max) {
        return 1;
    }
    if (split) {
        if (httpResizePacket(q, packet, 0) < 0) {
            return 0;
        }
        size = httpGetPacketLength(packet);
        assert(size <= q->packetSize);
        if ((size + q->count) <= q->max) {
            return 1;
        }
    }
    /*  
        The downstream queue is full, so disable the queue and mark the downstream queue as full and service 
     */
    if (!(q->flags & HTTP_QUEUE_SUSPENDED)) {
        httpScheduleQueue(q);
    }
    return 0;
}


/*  
    Return true if the next queue will accept a certain amount of data. If not, then disable the queue's service procedure.
    Will not split the packet.
 */
PUBLIC bool httpWillNextQueueAcceptSize(HttpQueue *q, ssize size)
{
    HttpQueue   *nextQ;

    nextQ = q->nextQ;
    if (size <= nextQ->packetSize && (size + nextQ->count) <= nextQ->max) {
        return 1;
    }
    httpSuspendQueue(q);
    if (!(nextQ->flags & HTTP_QUEUE_SUSPENDED)) {
        httpScheduleQueue(nextQ);
    }
    return 0;
}


/*
    Write a block of data. This is the lowest level write routine for data. This will buffer the data and flush if
    the queue buffer is full. Flushing is done by calling httpFlushQueue which will service queues as required. This
    may call the queue outgoing service routine and disable downstream queues if they are overfull.
    This routine will always accept the data and never return "short". 
 */
PUBLIC ssize httpWriteBlock(HttpQueue *q, cchar *buf, ssize len, int flags)
{
    HttpPacket  *packet;
    HttpConn    *conn;
    HttpTx      *tx;
    ssize       totalWritten, packetSize, thisWrite;

    assert(q == q->conn->writeq);
    conn = q->conn;
    tx = conn->tx;
    if (flags == 0) {
        flags = HTTP_BUFFER;
    }
    if (tx == 0 || tx->finalizedOutput) {
        return MPR_ERR_CANT_WRITE;
    }
    tx->responded = 1;

    for (totalWritten = 0; len > 0; ) {
        mprTrace(7, "httpWriteBlock q_count %d, q_max %d", q->count, q->max);
        if (conn->state >= HTTP_STATE_FINALIZED) {
            return MPR_ERR_CANT_WRITE;
        }
        if (q->last && q->last != q->first && q->last->flags & HTTP_PACKET_DATA && mprGetBufSpace(q->last->content) > 0) {
            packet = q->last;
        } else {
            packetSize = (tx->chunkSize > 0) ? tx->chunkSize : q->packetSize;
            if ((packet = httpCreateDataPacket(packetSize)) == 0) {
                return MPR_ERR_MEMORY;
            }
            httpPutForService(q, packet, HTTP_DELAY_SERVICE);
        }
        assert(mprGetBufSpace(packet->content) > 0);
        thisWrite = min(len, mprGetBufSpace(packet->content));
        if (flags & (HTTP_BLOCK | HTTP_NON_BLOCK)) {
            thisWrite = min(thisWrite, q->max - q->count);
        }
        if (thisWrite > 0) {
            if ((thisWrite = mprPutBlockToBuf(packet->content, buf, thisWrite)) == 0) {
                return MPR_ERR_MEMORY;
            }
            buf += thisWrite;
            len -= thisWrite;
            q->count += thisWrite;
            totalWritten += thisWrite;
        }
        if (q->count >= q->max) {
            httpFlushQueue(q, 0);
            if (q->count >= q->max) {
                if (flags & HTTP_NON_BLOCK) {
                    break;
                } else if (flags & HTTP_BLOCK) {
                    while (q->count >= q->max && !tx->finalized) {
                        if (!mprWaitForSingleIO((int) conn->sock->fd, MPR_WRITABLE, conn->limits->inactivityTimeout)) {
                            return MPR_ERR_TIMEOUT;
                        }
                        httpResumeQueue(conn->connectorq);
                        httpServiceQueues(conn);
                    }
                }
            }
        }
    }
    if (conn->error) {
        return MPR_ERR_CANT_WRITE;
    }
    return totalWritten;
}


PUBLIC ssize httpWriteString(HttpQueue *q, cchar *s)
{
    return httpWriteBlock(q, s, strlen(s), HTTP_BUFFER);
}


PUBLIC ssize httpWriteSafeString(HttpQueue *q, cchar *s)
{
    return httpWriteString(q, mprEscapeHtml(s));
}


PUBLIC ssize httpWrite(HttpQueue *q, cchar *fmt, ...)
{
    va_list     vargs;
    char        *buf;
    
    va_start(vargs, fmt);
    buf = sfmtv(fmt, vargs);
    va_end(vargs);
    return httpWriteString(q, buf);
}


#if BIT_DEBUG
PUBLIC bool httpVerifyQueue(HttpQueue *q)
{
    HttpPacket  *packet;
    ssize       count;

    count = 0;
    for (packet = q->first; packet; packet = packet->next) {
        if (packet->next == 0) {
            assert(packet == q->last);
        }
        count += httpGetPacketLength(packet);
    }
    assert(count == q->count);
    return count <= q->count;
}
#endif

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/rangeFilter.c"
 */
/************************************************************************/

/*
    rangeFilter.c - Ranged request filter.
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Defines ***********************************/

#define HTTP_RANGE_BUFSIZE 128              /* Packet size to hold range boundary */

/********************************** Forwards **********************************/

static bool applyRange(HttpQueue *q, HttpPacket *packet);
static void createRangeBoundary(HttpConn *conn);
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range);
static HttpPacket *createFinalRangePacket(HttpConn *conn);
static void outgoingRangeService(HttpQueue *q);
static bool fixRangeLength(HttpConn *conn);
static int matchRange(HttpConn *conn, HttpRoute *route, int dir);
static void startRange(HttpQueue *q);

/*********************************** Code *************************************/

PUBLIC int httpOpenRangeFilter(Http *http)
{
    HttpStage     *filter;

    mprTrace(5, "Open range filter");
    if ((filter = httpCreateFilter(http, "rangeFilter", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->rangeFilter = filter;
    filter->match = matchRange; 
    filter->start = startRange; 
    filter->outgoingService = outgoingRangeService; 
    return 0;
}


/*
    This is called twice: once for TX and once for RX
 */
static int matchRange(HttpConn *conn, HttpRoute *route, int dir)
{
    assert(conn->rx);

    httpSetHeader(conn, "Accept-Ranges", "bytes");
    if ((dir & HTTP_STAGE_TX) && conn->tx->outputRanges) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


static void startRange(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;
    /*
        The httpContentNotModified routine can set outputRanges to zero if returning not-modified.
     */
    if (tx->outputRanges == 0 || tx->status != HTTP_CODE_OK || !fixRangeLength(conn)) {
        httpRemoveQueue(q);
    } else {
        tx->status = HTTP_CODE_PARTIAL;
        if (tx->outputRanges->next) {
            createRangeBoundary(conn);
        }
    }
}


static void outgoingRangeService(HttpQueue *q)
{
    HttpPacket  *packet;
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (packet->flags & HTTP_PACKET_DATA) {
            if (!applyRange(q, packet)) {
                return;
            }
        } else {
            /*
                Send headers and end packet downstream
             */
            if (packet->flags & HTTP_PACKET_END && tx->rangeBoundary) {
                httpPutPacketToNext(q, createFinalRangePacket(conn));
            }
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            httpPutPacketToNext(q, packet);
        }
    }
}


static bool applyRange(HttpQueue *q, HttpPacket *packet)
{
    HttpRange   *range;
    HttpConn    *conn;
    HttpTx      *tx;
    MprOff      endPacket, length, gap, span;
    ssize       count;

    conn = q->conn;
    tx = conn->tx;
    range = tx->currentRange;

    /*  
        Process the data packet over multiple ranges ranges until all the data is processed or discarded.
        A packet may contain data or it may be empty with an associated entityLength. If empty, range packets
        are filled with entity data as required.
     */
    while (range && packet) {
        length = httpGetPacketEntityLength(packet);
        if (length <= 0) {
            break;
        }
        endPacket = tx->rangePos + length;
        if (endPacket < range->start) {
            /* Packet is before the next range, so discard the entire packet and seek forwards */
            tx->rangePos += length;
            break;

        } else if (tx->rangePos < range->start) {
            /*  Packet starts before range so skip some data, but some packet data is in range */
            gap = (range->start - tx->rangePos);
            tx->rangePos += gap;
            if (gap < length) {
                httpAdjustPacketStart(packet, (ssize) gap);
            }
            /* Keep going and examine next range */

        } else {
            /* In range */
            assert(range->start <= tx->rangePos && tx->rangePos < range->end);
            span = min(length, (range->end - tx->rangePos));
            count = (ssize) min(span, q->nextQ->packetSize);
            assert(count > 0);
            if (!httpWillNextQueueAcceptSize(q, count)) {
                httpPutBackPacket(q, packet);
                return 0;
            }
            if (length > count) {
                /* Split packet if packet extends past range */
                httpPutBackPacket(q, httpSplitPacket(packet, count));
            }
            if (packet->fill && (*packet->fill)(q, packet, tx->rangePos, count) < 0) {
                return 0;
            }
            if (tx->rangeBoundary) {
                httpPutPacketToNext(q, createRangePacket(conn, range));
            }
            httpPutPacketToNext(q, packet);
            packet = 0;
            tx->rangePos += count;
        }
        if (tx->rangePos >= range->end) {
            tx->currentRange = range = range->next;
        }
    }
    return 1;
}


/*  
    Create a range boundary packet
 */
static HttpPacket *createRangePacket(HttpConn *conn, HttpRange *range)
{
    HttpPacket  *packet;
    HttpTx      *tx;
    char        *length;

    tx = conn->tx;

    length = (tx->entityLength >= 0) ? itos(tx->entityLength) : "*";
    packet = httpCreatePacket(HTTP_RANGE_BUFSIZE);
    packet->flags |= HTTP_PACKET_RANGE;
    mprPutToBuf(packet->content, 
        "\r\n--%s\r\n"
        "Content-Range: bytes %Ld-%Ld/%s\r\n\r\n",
        tx->rangeBoundary, range->start, range->end - 1, length);
    return packet;
}


/*  
    Create a final range packet that follows all the data
 */
static HttpPacket *createFinalRangePacket(HttpConn *conn)
{
    HttpPacket  *packet;
    HttpTx      *tx;

    tx = conn->tx;

    packet = httpCreatePacket(HTTP_RANGE_BUFSIZE);
    packet->flags |= HTTP_PACKET_RANGE;
    mprPutToBuf(packet->content, "\r\n--%s--\r\n", tx->rangeBoundary);
    return packet;
}


/*  
    Create a range boundary. This is required if more than one range is requested.
 */
static void createRangeBoundary(HttpConn *conn)
{
    HttpTx      *tx;
    int         when;

    tx = conn->tx;
    assert(tx->rangeBoundary == 0);
    when = (int) conn->http->now;
    tx->rangeBoundary = sfmt("%08X%08X", PTOI(tx) + PTOI(conn) * when, when);
}


/*  
    Ensure all the range limits are within the entity size limits. Fixup negative ranges.
 */
static bool fixRangeLength(HttpConn *conn)
{
    HttpTx      *tx;
    HttpRange   *range;
    MprOff      length;

    tx = conn->tx;
    length = tx->entityLength ? tx->entityLength : tx->length;

    for (range = tx->outputRanges; range; range = range->next) {
        /*
                Range: 0-49             first 50 bytes
                Range: 50-99,200-249    Two 50 byte ranges from 50 and 200
                Range: -50              Last 50 bytes
                Range: 1-               Skip first byte then emit the rest
         */
        if (length) {
            if (range->end > length) {
                range->end = length;
            }
            if (range->start > length) {
                range->start = length;
            }
        }
        if (range->start < 0) {
            if (length <= 0) {
                /*
                    Cannot compute an offset from the end as we don't know the entity length and it is not always possible
                    or wise to buffer all the output.
                 */
                httpError(conn, HTTP_CODE_RANGE_NOT_SATISFIABLE, "Cannot compute end range with unknown content length"); 
                return 0;
            }
            /* select last -range-end bytes */
            range->start = length - range->end + 1;
            range->end = length;
        }
        if (range->end < 0) {
            if (length <= 0) {
                return 0;
            }
            range->end = length - range->end - 1;
        }
        range->len = (int) (range->end - range->start);
    }
    return 1;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/route.c"
 */
/************************************************************************/

/*
    route.c -- Http request routing 

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



#if BIT_PACK_PCRE
 #include    "pcre.h"
#endif

/********************************** Forwards **********************************/

#define GRADUATE_LIST(route, field) \
    if (route->field == 0) { \
        route->field = mprCreateList(-1, 0); \
    } else if (route->parent && route->field == route->parent->field) { \
        route->field = mprCloneList(route->parent->field); \
    }

#define GRADUATE_HASH(route, field) \
    assert(route->field) ; \
    if (route->parent && route->field == route->parent->field) { \
        route->field = mprCloneHash(route->parent->field); \
    }

//MOB temp
#if !BIT_PACK_PCRE
    int pcre_exec(void *a, void *b, cchar *c, int d, int e, int f, int *g, int h) { return -1; }
    void *pcre_compile2(cchar *a, int b, int *c, cchar **d, int *e, const unsigned char *f) { return 0; }
#endif
     
/********************************** Forwards **********************************/

static void addUniqueItem(MprList *list, HttpRouteOp *op);
static HttpLang *createLangDef(cchar *path, cchar *suffix, int flags);
static HttpRouteOp *createRouteOp(cchar *name, int flags);
static void definePathVars(HttpRoute *route);
static void defineHostVars(HttpRoute *route);
static char *expandTokens(HttpConn *conn, cchar *path);
static char *expandPatternTokens(cchar *str, cchar *replacement, int *matches, int matchCount);
static char *expandRequestTokens(HttpConn *conn, char *targetKey);
static cchar *expandRouteName(HttpConn *conn, cchar *routeName);
static void finalizeMethods(HttpRoute *route);
static void finalizePattern(HttpRoute *route);
static char *finalizeReplacement(HttpRoute *route, cchar *str);
static char *finalizeTemplate(HttpRoute *route);
static bool opPresent(MprList *list, HttpRouteOp *op);
static void manageRoute(HttpRoute *route, int flags);
static void manageLang(HttpLang *lang, int flags);
static void manageRouteOp(HttpRouteOp *op, int flags);
static int matchRequestUri(HttpConn *conn, HttpRoute *route);
static int testRoute(HttpConn *conn, HttpRoute *route);
static char *qualifyName(HttpRoute *route, cchar *controller, cchar *name);
static int selectHandler(HttpConn *conn, HttpRoute *route);
static int testCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *condition);
static char *trimQuotes(char *str);
static int updateRequest(HttpConn *conn, HttpRoute *route, HttpRouteOp *update);

/************************************ Code ************************************/
/*
    Host may be null
 */
PUBLIC HttpRoute *httpCreateRoute(HttpHost *host)
{
    Http        *http;
    HttpRoute   *route;

    http = MPR->httpService;
    if ((route = mprAllocObj(HttpRoute, manageRoute)) == 0) {
        return 0;
    }
    route->auth = httpCreateAuth();
    route->defaultLanguage = sclone("en");
    route->dir = mprGetCurrentPath(".");
    route->home = route->dir;
    route->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
    route->extensions = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    route->flags = HTTP_ROUTE_GZIP;
    route->handlers = mprCreateList(-1, MPR_LIST_STABLE);
    route->handlersByMatch = mprCreateList(-1, MPR_LIST_STABLE);
    route->host = host;
    route->http = MPR->httpService;
    route->indicies = mprCreateList(-1, 0);
    route->inputStages = mprCreateList(-1, 0);
    route->lifespan = BIT_MAX_CACHE_DURATION;
    route->outputStages = mprCreateList(-1, 0);
    route->vars = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    route->pattern = MPR->emptyString;
    route->targetRule = sclone("run");
    route->autoDelete = 1;
    route->workers = -1;

    if (MPR->httpService) {
        route->limits = mprMemdup(http->serverLimits ? http->serverLimits : http->clientLimits, sizeof(HttpLimits));
    }
    route->mimeTypes = MPR->mimeTypes;
    route->mutex = mprCreateLock();
    httpInitTrace(route->trace);

    if ((route->mimeTypes = mprCreateMimeTypes("mime.types")) == 0) {
        route->mimeTypes = MPR->mimeTypes;
    }  
    definePathVars(route);
    return route;
}


/*  
    Create a new location block. Inherit from the parent. We use a copy-on-write scheme if these are modified later.
 */
PUBLIC HttpRoute *httpCreateInheritedRoute(HttpRoute *parent)
{
    HttpRoute  *route;

    if (!parent && (parent = httpGetDefaultRoute(0)) == 0) {
        return 0;
    }
    if ((route = mprAllocObj(HttpRoute, manageRoute)) == 0) {
        return 0;
    }
    //  OPT. Structure assigment then overwrite.
    route->parent = parent;
    route->auth = httpCreateInheritedAuth(parent->auth);
    route->autoDelete = parent->autoDelete;
    route->caching = parent->caching;
    route->conditions = parent->conditions;
    route->connector = parent->connector;
    route->defaultLanguage = parent->defaultLanguage;
    route->dir = parent->dir;
    route->home = parent->home;
    route->data = parent->data;
    route->eroute = parent->eroute;
    route->errorDocuments = parent->errorDocuments;
    route->extensions = parent->extensions;
    route->handler = parent->handler;
    route->handlers = parent->handlers;
    route->handlersByMatch = parent->handlersByMatch;
    route->headers = parent->headers;
    route->http = MPR->httpService;
    route->host = parent->host;
    route->inputStages = parent->inputStages;
    route->indicies = parent->indicies;
    route->languages = parent->languages;
    route->lifespan = parent->lifespan;
    route->methods = parent->methods;
    route->methodSpec = parent->methodSpec;
    route->outputStages = parent->outputStages;
    route->params = parent->params;
    route->parent = parent;
    route->vars = parent->vars;
    route->pattern = parent->pattern;
    route->patternCompiled = parent->patternCompiled;
    route->optimizedPattern = parent->optimizedPattern;
    route->responseStatus = parent->responseStatus;
    route->script = parent->script;
    route->prefix = parent->prefix;
    route->prefixLen = parent->prefixLen;
    route->scriptPath = parent->scriptPath;
    route->sourceName = parent->sourceName;
    route->sourcePath = parent->sourcePath;
    route->ssl = parent->ssl;
    route->target = parent->target;
    route->targetRule = parent->targetRule;
    route->tokens = parent->tokens;
    route->updates = parent->updates;
    route->uploadDir = parent->uploadDir;
    route->workers = parent->workers;
    route->limits = parent->limits;
    route->mimeTypes = parent->mimeTypes;
    route->trace[0] = parent->trace[0];
    route->trace[1] = parent->trace[1];
    route->log = parent->log;
    route->logFormat = parent->logFormat;
    route->logPath = parent->logPath;
    route->logSize = parent->logSize;
    route->logBackup = parent->logBackup;
    route->logFlags = parent->logFlags;
    route->flags = parent->flags & ~(HTTP_ROUTE_FREE_PATTERN);
    return route;
}


static void manageRoute(HttpRoute *route, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(route->name);
        mprMark(route->pattern);
        mprMark(route->startSegment);
        mprMark(route->startWith);
        mprMark(route->optimizedPattern);
        mprMark(route->prefix);
        mprMark(route->tplate);
        mprMark(route->targetRule);
        mprMark(route->target);
        mprMark(route->dir);
        mprMark(route->home);
        mprMark(route->indicies);
        mprMark(route->methodSpec);
        mprMark(route->handler);
        mprMark(route->caching);
        mprMark(route->auth);
        mprMark(route->http);
        mprMark(route->host);
        mprMark(route->parent);
        mprMark(route->defaultLanguage);
        mprMark(route->extensions);
        mprMark(route->handlers);
        mprMark(route->handlersByMatch);
        mprMark(route->connector);
        mprMark(route->data);
        mprMark(route->eroute);
        mprMark(route->vars);
        mprMark(route->languages);
        mprMark(route->inputStages);
        mprMark(route->outputStages);
        mprMark(route->errorDocuments);
        mprMark(route->context);
        mprMark(route->uploadDir);
        mprMark(route->script);
        mprMark(route->scriptPath);
        mprMark(route->methods);
        mprMark(route->params);
        mprMark(route->headers);
        mprMark(route->conditions);
        mprMark(route->updates);
        mprMark(route->sourceName);
        mprMark(route->sourcePath);
        mprMark(route->tokens);
        mprMark(route->ssl);
        mprMark(route->limits);
        mprMark(route->mimeTypes);
        httpManageTrace(&route->trace[0], flags);
        httpManageTrace(&route->trace[1], flags);
        mprMark(route->log);
        mprMark(route->logFormat);
        mprMark(route->logPath);
        mprMark(route->mutex);
        mprMark(route->webSocketsProtocol);

    } else if (flags & MPR_MANAGE_FREE) {
        if (route->patternCompiled && (route->flags & HTTP_ROUTE_FREE_PATTERN)) {
            free(route->patternCompiled);
            route->patternCompiled = 0;
        }
    }
}


PUBLIC HttpRoute *httpCreateDefaultRoute(HttpHost *host)
{
    HttpRoute   *route;

    assert(host);
    if ((route = httpCreateRoute(host)) == 0) {
        return 0;
    }
    httpSetRouteName(route, "default");
    httpFinalizeRoute(route);
    return route;
}


/*
    Create and configure a basic route. This is mainly used for client side piplines.
    Host may be null.
 */
PUBLIC HttpRoute *httpCreateConfiguredRoute(HttpHost *host, int serverSide)
{
    HttpRoute   *route;
    Http        *http;

    /*
        Create default incoming and outgoing pipelines. Order matters.
     */
    route = httpCreateRoute(host);
    http = route->http;
    httpAddRouteFilter(route, http->rangeFilter->name, NULL, HTTP_STAGE_TX);
    httpAddRouteFilter(route, http->chunkFilter->name, NULL, HTTP_STAGE_RX | HTTP_STAGE_TX);
    httpAddRouteFilter(route, http->webSocketFilter->name, NULL, HTTP_STAGE_RX | HTTP_STAGE_TX);
    if (serverSide) {
        httpAddRouteFilter(route, http->uploadFilter->name, NULL, HTTP_STAGE_RX);
    }
    return route;
}


PUBLIC HttpRoute *httpCreateAliasRoute(HttpRoute *parent, cchar *pattern, cchar *path, int status)
{
    HttpRoute   *route;

    assert(parent);
    assert(pattern && *pattern);

    if ((route = httpCreateInheritedRoute(parent)) == 0) {
        return 0;
    }
    httpSetRoutePattern(route, pattern, 0);
    if (path) {
        httpSetRouteDir(route, path);
    }
    route->responseStatus = status;
    return route;
}


/*
    This routine binds a new route to a URI. It creates a handler, route and binds a callback to that route. 
 */
PUBLIC HttpRoute *httpCreateActionRoute(HttpRoute *parent, cchar *pattern, HttpAction action)
{
    HttpRoute   *route;

    if (!pattern || !action) {
        return 0;
    }
    if ((route = httpCreateInheritedRoute(parent)) != 0) {
        route->handler = route->http->actionHandler;
        httpSetRoutePattern(route, pattern, 0);
        httpDefineAction(pattern, action);
        httpFinalizeRoute(route);
    }
    return route;
}


PUBLIC int httpStartRoute(HttpRoute *route)
{
#if !BIT_ROM
    if (!(route->flags & HTTP_ROUTE_STARTED)) {
        route->flags |= HTTP_ROUTE_STARTED;
        if (route->logPath && (!route->parent || route->logPath != route->parent->logPath)) {
            if (route->logBackup > 0) {
                httpBackupRouteLog(route);
            }
            if ((route->log = mprOpenFile(route->logPath, O_CREAT | O_APPEND | O_WRONLY | O_TEXT, 0664)) == 0) {
                mprError("Cannot open log file %s", route->logPath);
                return MPR_ERR_CANT_OPEN;
            }
        }
    }
#endif
    return 0;
}


PUBLIC void httpStopRoute(HttpRoute *route)
{
    route->log = 0;
}


/*
    Find the matching route and handler for a request. If any errors occur, the pass handler is used to 
    pass errors via the net/sendfile connectors onto the client. This process may rewrite the request 
    URI and may redirect the request.
 */
PUBLIC void httpRouteRequest(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    int         next, rewrites, match;

    rx = conn->rx;
    tx = conn->tx;
    route = 0;

    for (next = rewrites = 0; rewrites < BIT_MAX_REWRITE; ) {
        if (next >= conn->host->routes->length) {
            break;
        }
        route = conn->host->routes->items[next++];
        if (route->startSegment && strncmp(rx->pathInfo, route->startSegment, route->startSegmentLen) != 0) {
            /* Failed to match the first URI segment, skip to the next group */
            assert(next <= route->nextGroup);
            next = route->nextGroup;

        } else if (route->startWith && strncmp(rx->pathInfo, route->startWith, route->startWithLen) != 0) {
            /* Failed to match starting literal segment of the route pattern, advance to test the next route */
            continue;

        } else if ((match = httpMatchRoute(conn, route)) == HTTP_ROUTE_REROUTE) {
            next = 0;
            route = 0;
            rewrites++;
            rx->flags &= ~HTTP_AUTH_CHECKED;

        } else if (match == HTTP_ROUTE_OK) {
            break;
        }
    }
    if (route == 0 || tx->handler == 0) {
        /* Ensure this is emitted to the log */
        mprError("Cannot find suitable route for request %s", rx->pathInfo);
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot find suitable route for request");
        return;
    }
    if (rx->traceLevel >= 0) {
        mprLog(rx->traceLevel, "Select route \"%s\" target \"%s\"", route->name, route->targetRule);
    }
    rx->route = route;
    conn->limits = route->limits;

    conn->trace[0] = route->trace[0];
    conn->trace[1] = route->trace[1];

    if (rewrites >= BIT_MAX_REWRITE) {
        httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Too many request rewrites");
    }
    if (tx->finalized) {
        /* Pass handler can transmit the error */
        tx->handler = conn->http->passHandler;
    }
    if (rx->traceLevel >= 0) {
        mprLog(rx->traceLevel, "Select handler: \"%s\" for \"%s\"", tx->handler->name, rx->uri);
    }
}


PUBLIC int httpMatchRoute(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;
    char        *savePathInfo, *pathInfo;
    int         rc;

    assert(conn);
    assert(route);

    rx = conn->rx;
    savePathInfo = 0;

    /*
        Remove the route prefix. Restore after matching.
     */
    if (route->prefix) {
        savePathInfo = rx->pathInfo;
        pathInfo = &rx->pathInfo[route->prefixLen];
        if (*pathInfo == '\0') {
            pathInfo = "/";
        }
        rx->pathInfo = sclone(pathInfo);
        rx->scriptName = route->prefix;
    }
    if ((rc = matchRequestUri(conn, route)) == HTTP_ROUTE_OK) {
        rc = testRoute(conn, route);
    }
    if (rc == HTTP_ROUTE_REJECT && route->prefix) {
        /* Keep the modified pathInfo if OK or REWRITE */
        rx->pathInfo = savePathInfo;
        rx->scriptName = 0;
    }
    return rc;
}


static int matchRequestUri(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;

    assert(conn);
    assert(route);
    rx = conn->rx;

    if (route->patternCompiled) {
        rx->matchCount = pcre_exec(route->patternCompiled, NULL, rx->pathInfo, (int) slen(rx->pathInfo), 0, 0, 
                rx->matches, sizeof(rx->matches) / sizeof(int));
        mprTrace(6, "Test route pattern \"%s\", regexp %s, pathInfo %s", route->name, route->optimizedPattern, rx->pathInfo);

        if (route->flags & HTTP_ROUTE_NOT) {
            if (rx->matchCount > 0) {
                return HTTP_ROUTE_REJECT;
            }
            rx->matchCount = 1;
            rx->matches[0] = 0;
            rx->matches[1] = (int) slen(rx->pathInfo);

        } else if (rx->matchCount <= 0) {
            return HTTP_ROUTE_REJECT;
        }
    } else if (route->pattern && *route->pattern) {
        /* Pattern compilation failed */
        return HTTP_ROUTE_REJECT;
    }
    mprTrace(6, "Test route methods \"%s\"", route->name);
    if (route->methods && !mprLookupKey(route->methods, rx->method)) {
        return HTTP_ROUTE_REJECT;
    }
    rx->route = route;
    return HTTP_ROUTE_OK;
}


static int testRoute(HttpConn *conn, HttpRoute *route)
{
    HttpRouteOp     *op, *condition, *update;
    HttpRouteProc   *proc;
    HttpRx          *rx;
    HttpTx          *tx;
    cchar           *token, *value, *header, *field;
    int             next, rc, matched[BIT_MAX_ROUTE_MATCHES * 2], count, result;

    assert(conn);
    assert(route);
    rx = conn->rx;
    tx = conn->tx;

    rx->target = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);

    if (route->headers) {
        for (next = 0; (op = mprGetNextItem(route->headers, &next)) != 0; ) {
            mprTrace(6, "Test route \"%s\" header \"%s\"", route->name, op->name);
            if ((header = httpGetHeader(conn, op->name)) != 0) {
                count = pcre_exec(op->mdata, NULL, header, (int) slen(header), 0, 0, 
                    matched, sizeof(matched) / sizeof(int)); 
                result = count > 0;
                if (op->flags & HTTP_ROUTE_NOT) {
                    result = !result;
                }
                if (!result) {
                    return HTTP_ROUTE_REJECT;
                }
            }
        }
    }
    if (route->params) {
        for (next = 0; (op = mprGetNextItem(route->params, &next)) != 0; ) {
            mprTrace(6, "Test route \"%s\" field \"%s\"", route->name, op->name);
            if ((field = httpGetParam(conn, op->name, "")) != 0) {
                count = pcre_exec(op->mdata, NULL, field, (int) slen(field), 0, 0, 
                    matched, sizeof(matched) / sizeof(int)); 
                result = count > 0;
                if (op->flags & HTTP_ROUTE_NOT) {
                    result = !result;
                }
                if (!result) {
                    return HTTP_ROUTE_REJECT;
                }
            }
        }
    }
    if (route->conditions) {
        for (next = 0; (condition = mprGetNextItem(route->conditions, &next)) != 0; ) {
            mprTrace(6, "Test route \"%s\" condition \"%s\"", route->name, condition->name);
            rc = testCondition(conn, route, condition);
            if (rc == HTTP_ROUTE_REROUTE) {
                return rc;
            }
            if (condition->flags & HTTP_ROUTE_NOT) {
                rc = !rc;
            }
            if (rc == HTTP_ROUTE_REJECT) {
                return rc;
            }
        }
    }
    if (route->updates) {
        for (next = 0; (update = mprGetNextItem(route->updates, &next)) != 0; ) {
            mprTrace(6, "Run route \"%s\" update \"%s\"", route->name, update->name);
            if ((rc = updateRequest(conn, route, update)) == HTTP_ROUTE_REROUTE) {
                return rc;
            }
        }
    }
    if (route->prefix) {
        /* This is needed by some handler match routines */
        httpSetParam(conn, "prefix", route->prefix);
    }
    if ((rc = selectHandler(conn, route)) != HTTP_ROUTE_OK) {
        return rc;
    }
    if (route->tokens) {
        for (next = 0; (token = mprGetNextItem(route->tokens, &next)) != 0; ) {
            value = snclone(&rx->pathInfo[rx->matches[next * 2]], rx->matches[(next * 2) + 1] - rx->matches[(next * 2)]);
            httpSetParam(conn, token, value);
        }
    }
    if ((proc = mprLookupKey(conn->http->routeTargets, route->targetRule)) == 0) {
        httpError(conn, -1, "Cannot find route target rule \"%s\"", route->targetRule);
        return HTTP_ROUTE_REJECT;
    }
    if ((rc = (*proc)(conn, route, 0)) != HTTP_ROUTE_OK) {
        return rc;
    }
    if (tx->handler->match && !(tx->flags & HTTP_TX_MATCHED)) {
        if ((rc = tx->handler->match(conn, route, HTTP_QUEUE_TX)) != HTTP_ROUTE_OK) {
            return rc;
        }
        tx->flags |= HTTP_TX_MATCHED;
    }
    return rc;
}


static int selectHandler(HttpConn *conn, HttpRoute *route)
{
    HttpTx      *tx;
    int         next, rc;

    assert(conn);
    assert(route);

    tx = conn->tx;
    if (route->handler) {
        tx->handler = route->handler;
        return HTTP_ROUTE_OK;
    }
    /*
        Handlers that match solely by their match routine are examined first (in-order)
     */
    for (next = 0; (tx->handler = mprGetNextStableItem(route->handlersByMatch, &next)) != 0; ) {
        rc = tx->handler->match(conn, route, 0);
        tx->flags |= HTTP_TX_MATCHED;
        if (rc == HTTP_ROUTE_OK || rc == HTTP_ROUTE_REROUTE) {
            return rc;
        }
    }
    if (!tx->handler) {
        /*
            Now match by extensions
         */
        if (!tx->ext || (tx->handler = mprLookupKey(route->extensions, tx->ext)) == 0) {
            tx->handler = mprLookupKey(route->extensions, "");
        }
    }
    return tx->handler ? HTTP_ROUTE_OK : HTTP_ROUTE_REJECT;
}


/*
    Map the target to physical storage. Sets tx->filename and tx->ext.
 */
PUBLIC void httpMapFile(HttpConn *conn, HttpRoute *route)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLang    *lang;
    MprPath     *info;

    assert(conn);
    assert(route);

    rx = conn->rx;
    tx = conn->tx;
    lang = rx->lang;

    assert(rx->target);
    tx->filename = rx->target;
    if (lang && lang->path) {
        tx->filename = mprJoinPath(lang->path, tx->filename);
    }
    tx->filename = mprJoinPath(route->dir, tx->filename);
#if BIT_ROM
    tx->filename = mprGetRelPath(tx->filename, NULL);
#endif
    tx->ext = httpGetExt(conn);
    info = &tx->fileInfo;
    mprGetPathInfo(tx->filename, info);
    if (info->valid) {
        tx->etag = sfmt("\"%Lx-%Lx-%Lx\"", (int64) info->inode, (int64) info->size, (int64) info->mtime);
    }
    mprTrace(7, "mapFile uri \"%s\", filename: \"%s\", extension: \"%s\"", rx->uri, tx->filename, tx->ext);
}


/************************************ API *************************************/

PUBLIC int httpAddRouteCondition(HttpRoute *route, cchar *name, cchar *details, int flags)
{
    HttpRouteOp *op;
    cchar       *errMsg;
    char        *pattern, *value;
    int         column;

    assert(route);

    GRADUATE_LIST(route, conditions);
    if ((op = createRouteOp(name, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (scaselessmatch(name, "auth") || scaselessmatch(name, "unauthorized")) {
        /* Nothing to do. Route->auth has it all */

    } else if (scaselessmatch(name, "missing")) {
        op->details = finalizeReplacement(route, "${request:filename}");

    } else if (scaselessmatch(name, "directory")) {
        op->details = finalizeReplacement(route, details);

    } else if (scaselessmatch(name, "exists")) {
        op->details = finalizeReplacement(route, details);

    } else if (scaselessmatch(name, "match")) {
        /* 
            Condition match string pattern
            String can contain matching ${tokens} from the route->pattern and can contain request ${tokens}
         */
        if (!httpTokenize(route, details, "%S %S", &value, &pattern)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        if ((op->mdata = pcre_compile2(pattern, 0, 0, &errMsg, &column, NULL)) == 0) {
            mprError("Cannot compile condition match pattern. Error %s at column %d", errMsg, column); 
            return MPR_ERR_BAD_SYNTAX;
        }
        op->details = finalizeReplacement(route, value);
        op->flags |= HTTP_ROUTE_FREE;

    } else if (scaselessmatch(name, "secure")) {
        op->details = finalizeReplacement(route, details);
    }
    addUniqueItem(route->conditions, op);
    return 0;
}


PUBLIC int httpAddRouteFilter(HttpRoute *route, cchar *name, cchar *extensions, int direction)
{
    HttpStage   *stage;
    HttpStage   *filter;
    char        *extlist, *word, *tok;
    int         pos;

    assert(route);
    
    stage = httpLookupStage(route->http, name);
    if (stage == 0) {
        mprError("Cannot find filter %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    /*
        Clone an existing stage because each filter stores its own set of extensions to match against
     */
    filter = httpCloneStage(route->http, stage);

    if (extensions && *extensions) {
        filter->extensions = mprCreateHash(0, MPR_HASH_CASELESS);
        extlist = sclone(extensions);
        word = stok(extlist, " \t\r\n", &tok);
        while (word) {
            if (*word == '*' && word[1] == '.') {
                word += 2;
            } else if (*word == '.') {
                word++;
            } else if (*word == '\"' && word[1] == '\"') {
                word = "";
            }
            mprAddKey(filter->extensions, word, filter);
            word = stok(0, " \t\r\n", &tok);
        }
    }
    if (direction & HTTP_STAGE_RX && filter->incoming) {
        GRADUATE_LIST(route, inputStages);
        mprAddItem(route->inputStages, filter);
    }
    if (direction & HTTP_STAGE_TX && filter->outgoing) {
        GRADUATE_LIST(route, outputStages);
        if (smatch(name, "cacheFilter") && 
                (pos = mprGetListLength(route->outputStages) - 1) >= 0 &&
                smatch(((HttpStage*) mprGetLastItem(route->outputStages))->name, "chunkFilter")) {
            mprInsertItemAtPos(route->outputStages, pos, filter);
        } else {
            mprAddItem(route->outputStages, filter);
        }
    }
    return 0;
}


PUBLIC int httpAddRouteHandler(HttpRoute *route, cchar *name, cchar *extensions)
{
    Http            *http;
    HttpStage       *handler;
    char            *extlist, *word, *tok;

    assert(route);

    http = route->http;
    if ((handler = httpLookupStage(http, name)) == 0) {
        mprError("Cannot find stage %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    GRADUATE_HASH(route, extensions);

    if (extensions && *extensions) {
        /*
            Add to the handler extension hash. Skip over "*." and "."
         */ 
        extlist = sclone(extensions);
        word = stok(extlist, " \t\r\n", &tok);
        while (word) {
            if (*word == '*' && word[1] == '.') {
                word += 2;
            } else if (*word == '.') {
                word++;
            } else if (*word == '\"' && word[1] == '\"') {
                word = "";
            }
            mprAddKey(route->extensions, word, handler);
            word = stok(0, " \t\r\n", &tok);
        }

    } else {
        if (mprLookupItem(route->handlers, handler) < 0) {
            GRADUATE_LIST(route, handlers);
            mprAddItem(route->handlers, handler);
        }
        if (handler->match) {
            if (mprLookupItem(route->handlersByMatch, handler) < 0) {
                GRADUATE_LIST(route, handlersByMatch);
                mprAddItem(route->handlersByMatch, handler);
            }
        } else {
            /*
                Match all extensions if no-match routine provided
             */
            mprAddKey(route->extensions, "", handler);
        }
    }
    return 0;
}


/*
    Header field valuePattern
 */
PUBLIC void httpAddRouteHeader(HttpRoute *route, cchar *header, cchar *value, int flags)
{
    HttpRouteOp     *op;
    cchar           *errMsg;
    int             column;

    assert(route);
    assert(header && *header);
    assert(value && *value);

    GRADUATE_LIST(route, headers);
    if ((op = createRouteOp(header, flags | HTTP_ROUTE_FREE)) == 0) {
        return;
    }
    if ((op->mdata = pcre_compile2(value, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Cannot compile header pattern. Error %s at column %d", errMsg, column); 
    } else {
        mprAddItem(route->headers, op);
    }
}


#if FUTURE && KEEP
PUBLIC void httpAddRouteLoad(HttpRoute *route, cchar *module, cchar *path)
{
    HttpRouteOp     *op;

    GRADUATE_LIST(route, updates);
    if ((op = createRouteOp("--load--", 0)) == 0) {
        return;
    }
    op->var = sclone(module);
    op->value = sclone(path);
    mprAddItem(route->updates, op);
}
#endif


/*
    Param field valuePattern
 */
PUBLIC void httpAddRouteParam(HttpRoute *route, cchar *field, cchar *value, int flags)
{
    HttpRouteOp     *op;
    cchar           *errMsg;
    int             column;

    assert(route);
    assert(field && *field);
    assert(value && *value);

    GRADUATE_LIST(route, params);
    if ((op = createRouteOp(field, flags | HTTP_ROUTE_FREE)) == 0) {
        return;
    }
    if ((op->mdata = pcre_compile2(value, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Cannot compile field pattern. Error %s at column %d", errMsg, column); 
    } else {
        mprAddItem(route->params, op);
    }
}


/*
    Add a route update record. These run to modify a request.
        Update rule var value
        rule == "cmd|param"
        details == "var value"
    Value can contain pattern and request tokens.
 */
PUBLIC int httpAddRouteUpdate(HttpRoute *route, cchar *rule, cchar *details, int flags)
{
    HttpRouteOp *op;
    char        *value;

    assert(route);
    assert(rule && *rule);

    GRADUATE_LIST(route, updates);
    if ((op = createRouteOp(rule, flags)) == 0) {
        return MPR_ERR_MEMORY;
    }
    if (scaselessmatch(rule, "cmd")) {
        op->details = sclone(details);

    } else if (scaselessmatch(rule, "lang")) {
        /* Nothing to do */;

    } else if (scaselessmatch(rule, "param")) {
        if (!httpTokenize(route, details, "%S %S", &op->var, &value)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        op->value = finalizeReplacement(route, value);
    
    } else {
        return MPR_ERR_BAD_SYNTAX;
    }
    addUniqueItem(route->updates, op);
    return 0;
}


PUBLIC void httpClearRouteStages(HttpRoute *route, int direction)
{
    assert(route);

    if (direction & HTTP_STAGE_RX) {
        route->inputStages = mprCreateList(-1, 0);
    }
    if (direction & HTTP_STAGE_TX) {
        route->outputStages = mprCreateList(-1, 0);
    }
}


PUBLIC void httpDefineRouteTarget(cchar *key, HttpRouteProc *proc)
{
    assert(key && *key);
    assert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeTargets, key, proc);
}


PUBLIC void httpDefineRouteCondition(cchar *key, HttpRouteProc *proc)
{
    assert(key && *key);
    assert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeConditions, key, proc);
}


PUBLIC void httpDefineRouteUpdate(cchar *key, HttpRouteProc *proc)
{
    assert(key && *key);
    assert(proc);

    mprAddKey(((Http*) MPR->httpService)->routeUpdates, key, proc);
}


PUBLIC void *httpGetRouteData(HttpRoute *route, cchar *key)
{
    assert(route);
    assert(key && *key);

    if (!route->data) {
        return 0;
    }
    return mprLookupKey(route->data, key);
}


PUBLIC cchar *httpGetRouteDir(HttpRoute *route)
{
    assert(route);
    return route->dir;
}


PUBLIC cchar *httpGetRouteMethods(HttpRoute *route)
{
    assert(route);
    return route->methodSpec;
}


PUBLIC void httpResetRoutePipeline(HttpRoute *route)
{
    assert(route);

    if (route->parent == 0) {
        route->errorDocuments = mprCreateHash(HTTP_SMALL_HASH_SIZE, 0);
        route->caching = 0;
        route->extensions = 0;
        route->handlers = mprCreateList(-1, 0);
        route->handlersByMatch = mprCreateList(-1, 0);
        route->inputStages = mprCreateList(-1, 0);
        route->indicies = mprCreateList(-1, 0);
    }
    route->outputStages = mprCreateList(-1, 0);
}


PUBLIC void httpResetHandlers(HttpRoute *route)
{
    assert(route);
    route->handlers = mprCreateList(-1, 0);
    route->handlersByMatch = mprCreateList(-1, 0);
}


PUBLIC void httpSetRouteAuth(HttpRoute *route, HttpAuth *auth)
{
    assert(route);
    route->auth = auth;
}


PUBLIC void httpSetRouteAutoDelete(HttpRoute *route, bool enable)
{
    assert(route);
    route->autoDelete = enable;
}


PUBLIC void httpSetRouteCompression(HttpRoute *route, int flags)
{
    assert(route);
    route->flags &= (HTTP_ROUTE_GZIP);
    route->flags |= (HTTP_ROUTE_GZIP & flags);
}


PUBLIC int httpSetRouteConnector(HttpRoute *route, cchar *name)
{
    HttpStage     *stage;

    assert(route);
    
    stage = httpLookupStage(route->http, name);
    if (stage == 0) {
        mprError("Cannot find connector %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    route->connector = stage;
    return 0;
}


PUBLIC void httpSetRouteData(HttpRoute *route, cchar *key, void *data)
{
    assert(route);
    assert(key && *key);
    assert(data);

    if (route->data == 0) {
        route->data = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, data);
    }
    mprAddKey(route->data, key, data);
}


PUBLIC void httpSetRouteFlags(HttpRoute *route, int flags)
{
    assert(route);
    route->flags = flags;
}


PUBLIC int httpSetRouteHandler(HttpRoute *route, cchar *name)
{
    HttpStage     *handler;

    assert(route);
    assert(name && *name);
    
    if ((handler = httpLookupStage(route->http, name)) == 0) {
        mprError("Cannot find handler %s", name); 
        return MPR_ERR_CANT_FIND;
    }
    route->handler = handler;
    return 0;
}


PUBLIC void httpSetRouteDir(HttpRoute *route, cchar *path)
{
    assert(route);
    assert(path && *path);
    
    route->dir = httpMakePath(route, path);
    httpSetRouteVar(route, "DOCUMENTS_DIR", route->dir);
    //  DEPRECATE
    httpSetRouteVar(route, "DOCUMENTS", route->dir);
    httpSetRouteVar(route, "DOCUMENT_ROOT", route->dir);
}


PUBLIC void httpSetRouteHome(HttpRoute *route, cchar *path)
{
    assert(route);
    assert(path && *path);
    
    route->home = httpMakePath(route, path);

    httpSetRouteVar(route, "HOME_DIR", route->home);
    //  DEPRECATED
    httpSetRouteVar(route, "ROUTE_HOME", route->home);
}


/*
    WARNING: internal API only. 
 */
PUBLIC void httpSetRouteHost(HttpRoute *route, HttpHost *host)
{
    assert(route);
    assert(host);
    
    route->host = host;
    defineHostVars(route);
}


PUBLIC void httpAddRouteIndex(HttpRoute *route, cchar *index)
{
    cchar   *item;
    int     next;

    assert(route);
    assert(index && *index);
    
    GRADUATE_LIST(route, indicies);
    for (ITERATE_ITEMS(route->indicies, item, next)) {
        if (smatch(index, item)) {
            return;
        }
    }
    mprAddItem(route->indicies, sclone(index));
}


PUBLIC void httpSetRouteMethods(HttpRoute *route, cchar *methods)
{
    assert(route);
    assert(methods && methods);

    route->methodSpec = sclone(methods);
    finalizeMethods(route);
}


PUBLIC void httpSetRouteName(HttpRoute *route, cchar *name)
{
    assert(route);
    assert(name && *name);
    
    route->name = sclone(name);
}


PUBLIC void httpSetRoutePattern(HttpRoute *route, cchar *pattern, int flags)
{
    assert(route);
    assert(pattern && *pattern);
    
    route->flags |= (flags & HTTP_ROUTE_NOT);
    route->pattern = sclone(pattern);
    finalizePattern(route);
}


PUBLIC void httpSetRoutePrefix(HttpRoute *route, cchar *prefix)
{
    assert(route);
    assert(prefix && *prefix);
    
    route->prefix = sclone(prefix);
    route->prefixLen = slen(prefix);
    if (route->pattern) {
        finalizePattern(route);
    }
}


PUBLIC void httpSetRouteSource(HttpRoute *route, cchar *source)
{
    assert(route);
    assert(source);

    /* Source can be empty */
    route->sourceName = sclone(source);
}


PUBLIC void httpSetRouteScript(HttpRoute *route, cchar *script, cchar *scriptPath)
{
    assert(route);
    
    if (script) {
        assert(*script);
        route->script = sclone(script);
    }
    if (scriptPath) {
        assert(*scriptPath);
        route->scriptPath = sclone(scriptPath);
    }
}


/*
    Target names are extensible and hashed in http->routeTargets. 

        Target close
        Target redirect status [URI]
        Target run ${DOCUMENTS}/${request:uri}.gz
        Target run ${controller}-${name} 
        Target write [-r] status "Hello World\r\n"
 */
PUBLIC int httpSetRouteTarget(HttpRoute *route, cchar *rule, cchar *details)
{
    char    *redirect, *msg;

    assert(route);
    assert(rule && *rule);

    route->targetRule = sclone(rule);
    route->target = sclone(details);

    if (scaselessmatch(rule, "close")) {
        route->target = sclone(details);

    } else if (scaselessmatch(rule, "redirect")) {
        if (!httpTokenize(route, details, "%N ?S", &route->responseStatus, &redirect)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        route->target = finalizeReplacement(route, redirect);
        return 0;

    } else if (scaselessmatch(rule, "run")) {
        route->target = finalizeReplacement(route, details);

    } else if (scaselessmatch(rule, "write")) {
        /*
            Write [-r] status Message
         */
        if (sncmp(details, "-r", 2) == 0) {
            route->flags |= HTTP_ROUTE_RAW;
            details = &details[2];
        }
        if (!httpTokenize(route, details, "%N %S", &route->responseStatus, &msg)) {
            return MPR_ERR_BAD_SYNTAX;
        }
        route->target = finalizeReplacement(route, msg);

    } else {
        return MPR_ERR_BAD_SYNTAX;
    }
    return 0;
}


PUBLIC void httpSetRouteTemplate(HttpRoute *route, cchar *tplate)
{
    assert(route);
    assert(tplate && *tplate);
    
    route->tplate = sclone(tplate);
}


PUBLIC void httpSetRouteWorkers(HttpRoute *route, int workers)
{
    assert(route);
    route->workers = workers;
}


PUBLIC void httpAddRouteErrorDocument(HttpRoute *route, int status, cchar *url)
{
    char    *code;

    assert(route);
    GRADUATE_HASH(route, errorDocuments);
    code = itos(status);
    mprAddKey(route->errorDocuments, code, sclone(url));
}


PUBLIC cchar *httpLookupRouteErrorDocument(HttpRoute *route, int code)
{
    char   *num;

    assert(route);
    if (route->errorDocuments == 0) {
        return 0;
    }
    num = itos(code);
    return (cchar*) mprLookupKey(route->errorDocuments, num);
}

/********************************* Route Finalization *************************/

static void finalizeMethods(HttpRoute *route)
{
    char    *method, *methods, *tok;

    assert(route);
    methods = route->methodSpec;
    if (methods && *methods && !scaselessmatch(methods, "ALL") && !smatch(methods, "*")) {
        if ((route->methods = mprCreateHash(-1, 0)) == 0) {
            return;
        }
        methods = sclone(methods);
        while ((method = stok(methods, ", \t\n\r", &tok)) != 0) {
            mprAddKey(route->methods, method, route);
            methods = 0;
        }
    } else {
        route->methodSpec = sclone("*");
    }
}


/*
    Finalize the pattern. 
        - Change "\{n[:m]}" to "{n[:m]}"
        - Change "\~" to "~"
        - Change "(~ PAT ~)" to "(?: PAT )?"
        - Extract the tokens and change tokens: "{word}" to "([^/]*)"
 */
static void finalizePattern(HttpRoute *route)
{
    MprBuf      *pattern;
    cchar       *errMsg;
    char        *startPattern, *cp, *ep, *token, *field;
    ssize       len;
    int         column;

    assert(route);
    route->tokens = mprCreateList(-1, 0);
    pattern = mprCreateBuf(-1, -1);
    startPattern = route->pattern[0] == '^' ? &route->pattern[1] : route->pattern;

    if (route->name == 0) {
        route->name = sclone(startPattern);
    }
    if (route->tplate == 0) {
        /* Do this while the prefix is still in the route pattern */
        route->tplate = finalizeTemplate(route);
    }
    /*
        Create an simple literal startWith string to optimize route rejection.
     */
    len = strcspn(startPattern, "^$*+?.(|{[\\");
    if (len) {
        route->startWith = snclone(startPattern, len);
        route->startWithLen = len;
        if ((cp = strchr(&route->startWith[1], '/')) != 0) {
            route->startSegment = snclone(route->startWith, cp - route->startWith);
        } else {
            route->startSegment = route->startWith;
        }
        route->startSegmentLen = slen(route->startSegment);
    } else {
        /* Pattern has special characters */
        route->startWith = 0;
        route->startWithLen = 0;
        route->startSegmentLen = 0;
        route->startSegment = 0;
    }

    /*
        Remove the route prefix from the start of the compiled pattern.
     */
    if (route->prefix && sstarts(startPattern, route->prefix)) {
        assert(route->prefixLen <= route->startWithLen);
        startPattern = sfmt("^%s", &startPattern[route->prefixLen]);
    } else {
        startPattern = sjoin("^", startPattern, NULL);
    }
    for (cp = startPattern; *cp; cp++) {
        /* Alias for optional, non-capturing pattern:  "(?: PAT )?" */
        if (*cp == '(' && cp[1] == '~') {
            mprPutStringToBuf(pattern, "(?:");
            cp++;

        } else if (*cp == '(') {
            mprPutCharToBuf(pattern, *cp);
        } else if (*cp == '~' && cp[1] == ')') {
            mprPutStringToBuf(pattern, ")?");
            cp++;

        } else if (*cp == ')') {
            mprPutCharToBuf(pattern, *cp);

        } else if (*cp == '{') {
            if (cp > startPattern&& cp[-1] == '\\') {
                mprAdjustBufEnd(pattern, -1);
                mprPutCharToBuf(pattern, *cp);
            } else {
                if ((ep = schr(cp, '}')) != 0) {
                    /* Trim {} off the token and replace in pattern with "([^/]*)"  */
                    token = snclone(&cp[1], ep - cp - 1);
                    if ((field = schr(token, '=')) != 0) {
                        *field++ = '\0';
                        field = sfmt("(%s)", field);
                    } else {
                        field = "([^/]*)";
                    }
                    mprPutStringToBuf(pattern, field);
                    mprAddItem(route->tokens, token);
                    /* Params ends up looking like "$1:$2:$3:$4" */
                    cp = ep;
                }
            }
        } else if (*cp == '\\' && *cp == '~') {
            mprPutCharToBuf(pattern, *++cp);

        } else {
            mprPutCharToBuf(pattern, *cp);
        }
    }
    mprAddNullToBuf(pattern);
    route->optimizedPattern = sclone(mprGetBufStart(pattern));
    if (mprGetListLength(route->tokens) == 0) {
        route->tokens = 0;
    }
    if (route->patternCompiled && (route->flags & HTTP_ROUTE_FREE_PATTERN)) {
        free(route->patternCompiled);
    }
    if ((route->patternCompiled = pcre_compile2(route->optimizedPattern, 0, 0, &errMsg, &column, NULL)) == 0) {
        mprError("Cannot compile route. Error %s at column %d", errMsg, column); 
    }
    route->flags |= HTTP_ROUTE_FREE_PATTERN;
}


static char *finalizeReplacement(HttpRoute *route, cchar *str)
{
    MprBuf      *buf;
    cchar       *item;
    cchar       *tok, *cp, *ep, *token;
    int         next, braced;

    assert(route);

    /*
        Prepare a replacement string. Change $token to $N
     */
    buf = mprCreateBuf(-1, -1);
    if (str && *str) {
        for (cp = str; *cp; cp++) {
            if ((tok = schr(cp, '$')) != 0 && (tok == str || tok[-1] != '\\')) {
                if (tok > cp) {
                    mprPutBlockToBuf(buf, cp, tok - cp);
                }
                if ((braced = (*++tok == '{')) != 0) {
                    tok++;
                }
                if (*tok == '&' || *tok == '\'' || *tok == '`' || *tok == '$') {
                    mprPutCharToBuf(buf, '$');
                    mprPutCharToBuf(buf, *tok);
                    ep = tok + 1;
                } else {
                    if (braced) {
                        for (ep = tok; *ep && *ep != '}'; ep++) ;
                    } else {
                        for (ep = tok; *ep && isdigit((uchar) *ep); ep++) ;
                    }
                    token = snclone(tok, ep - tok);
                    if (schr(token, ':')) {
                        /* Double quote to get through two levels of expansion in writeTarget */
                        mprPutStringToBuf(buf, "$${");
                        mprPutStringToBuf(buf, token);
                        mprPutCharToBuf(buf, '}');
                    } else {
                        for (next = 0; (item = mprGetNextItem(route->tokens, &next)) != 0; ) {
                            if (scmp(item, token) == 0) {
                                break;
                            }
                        }
                        /*  Insert "$" in front of "{token}" */
                        if (item) {
                            mprPutCharToBuf(buf, '$');
                            mprPutIntToBuf(buf, next);
                        } else if (snumber(token)) {
                            mprPutCharToBuf(buf, '$');
                            mprPutStringToBuf(buf, token);
                        } else {
                            mprError("Cannot find token \"%s\" in template \"%s\"", token, route->pattern);
                        }
                    }
                }
                if (braced) {
                    ep++;
                }
                cp = ep - 1;

            } else {
                if (*cp == '\\') {
                    if (cp[1] == 'r') {
                        mprPutCharToBuf(buf, '\r');
                        cp++;
                    } else if (cp[1] == 'n') {
                        mprPutCharToBuf(buf, '\n');
                        cp++;
                    } else {
                        mprPutCharToBuf(buf, *cp);
                    }
                } else {
                    mprPutCharToBuf(buf, *cp);
                }
            }
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


/*
    Convert a route pattern into a usable template to construct URI links
    NOTE: this is heuristic and not perfect. Users can define the template via the httpSetTemplate API or in appweb via the
    EspURITemplate configuration directive.
 */
static char *finalizeTemplate(HttpRoute *route)
{
    MprBuf  *buf;
    char    *sp, *tplate;

    if ((buf = mprCreateBuf(0, 0)) == 0) {
        return 0;
    }
    /*
        Note: the route->pattern includes the prefix
     */
    for (sp = route->pattern; *sp; sp++) {
        switch (*sp) {
        default:
            mprPutCharToBuf(buf, *sp);
            break;
        case '$':
            if (sp[1] == '\0') {
                sp++;
            } else {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case '^':
            if (sp > route->pattern) {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case '+':
        case '?':
        case '|':
        case '[':
        case ']':
        case '*':
        case '.':
            break;
        case '(':
            if (sp[1] == '~') {
                sp++;
            }
            break;
        case '~':
            if (sp[1] == ')') {
                sp++;
            } else {
                mprPutCharToBuf(buf, *sp);
            }
            break;
        case ')':
            break;
        case '\\':
            if (sp[1] == '\\') {
                mprPutCharToBuf(buf, *sp++);
            } else {
                mprPutCharToBuf(buf, *++sp);
            }
            break;
        case '{':
            mprPutCharToBuf(buf, '$');
            while (sp[1] && *sp != '}') {
                if (*sp == '=') {
                    while (sp[1] && *sp != '}') sp++;
                } else {
                    mprPutCharToBuf(buf, *sp++);
                }
            }
            mprPutCharToBuf(buf, '}');
            break;
        }
    }
    if (mprLookAtLastCharInBuf(buf) == '/') {
        mprAdjustBufEnd(buf, -1);
    }
    mprAddNullToBuf(buf);
    if (mprGetBufLength(buf) > 0) {
        tplate = sclone(mprGetBufStart(buf));
    } else {
        tplate = sclone("/");
    }
    return tplate;
}


PUBLIC void httpFinalizeRoute(HttpRoute *route)
{
    /*
        Add the route to the owning host. When using an Appweb configuration file, the order of route finalization 
        will be from the inside out. This ensures that nested routes are defined BEFORE outer/enclosing routes.
        This is important as requests process routes in-order.
     */
    assert(route);
    if (mprGetListLength(route->indicies) == 0) {
        mprAddItem(route->indicies,  sclone("index.html"));
    }
    httpAddRoute(route->host, route);
}


/********************************* Path and URI Expansion *****************************/
/*
    What does this return. Does it return an absolute URI?
    MOB - consider rename httpUri() and move to uri.c
 */
PUBLIC char *httpLink(HttpConn *conn, cchar *target, MprHash *options)
{
    HttpRoute       *route, *lroute;
    HttpRx          *rx;
    HttpUri         *uri;
    cchar           *routeName, *action, *controller, *originalAction, *tplate;
    char            *rest;

    rx = conn->rx;
    route = rx->route;
    controller = 0;

    if (target == 0) {
        target = "";
    }
    if (*target == '@') {
        target = sjoin("{action: '", target, "'}", NULL);
    } 
    if (*target != '{') {
        target = httpTemplate(conn, target, 0);
    } else  {
        if (options) {
            options = mprBlendHash(httpGetOptions(target), options);
        } else {
            options = httpGetOptions(target);
        }
        /*
            Prep the action. Forms are:
                . @action               # Use the current controller
                . @controller/          # Use "list" as the action
                . @controller/action
         */
        if ((action = httpGetOption(options, "action", 0)) != 0) {
            originalAction = action;
            if (*action == '@') {
                action = &action[1];
            }
            if (strchr(action, '/')) {
                controller = stok((char*) action, "/", (char**) &action);
                action = stok((char*) action, "/", &rest);
            }
            if (controller) {
                httpSetOption(options, "controller", controller);
            } else {
                controller = httpGetParam(conn, "controller", 0);
            }
            if (action == 0 || *action == '\0') {
                action = "list";
            }
            if (action != originalAction) {
                httpSetOption(options, "action", action);
            }
        }
        /*
            Find the template to use. Strategy is this order:
                . options.template
                . options.route.template
                . options.action mapped to a route.template, via:
                . /app/STAR/action
                . /app/controller/action
                . /app/STAR/default
                . /app/controller/default
         */
        if ((tplate = httpGetOption(options, "template", 0)) == 0) {
            if ((routeName = httpGetOption(options, "route", 0)) != 0) {
                routeName = expandRouteName(conn, routeName);
                lroute = httpLookupRoute(conn->host, routeName);
            } else {
                lroute = 0;
            }
            if (lroute == 0) {
                if ((lroute = httpLookupRoute(conn->host, qualifyName(route, "{controller}", action))) == 0) {
                    if ((lroute = httpLookupRoute(conn->host, qualifyName(route, controller, action))) == 0) {
                        if ((lroute = httpLookupRoute(conn->host, qualifyName(route, "{controller}", "default"))) == 0) {
                            lroute = httpLookupRoute(conn->host, qualifyName(route, controller, "default"));
                        }
                    }
                }
            }
            if (lroute) {
                tplate = lroute->tplate;
            }
        }
        if (tplate) {
            target = httpTemplate(conn, tplate, options);
        } else {
            mprError("Cannot find template for URI %s", target);
            target = "/";
        }
    }
    //  OPT
    uri = httpCreateUri(target, 0);
    uri = httpResolveUri(httpCreateUri(rx->uri, 0), 1, &uri, 0);
    httpNormalizeUri(uri);
    return httpUriToString(uri, 0);
}


/*
    Limited expansion of route names. Support ~/ and ${app} at the start of the route name
 */
static cchar *expandRouteName(HttpConn *conn, cchar *routeName)
{
    HttpRoute   *route;

    route = conn->rx->route;
    if (routeName[0] == '~') {
        return sjoin(route->prefix, &routeName[1], NULL);
    }
    if (sstarts(routeName, "${app}")) {
        return sjoin(route->prefix, &routeName[6], NULL);
    }
    return routeName;
}


/*
    Expect a route->tplate with embedded tokens of the form: "/${controller}/${action}/${other}"
    The options is a hash of token values.
 */
PUBLIC char *httpTemplate(HttpConn *conn, cchar *tplate, MprHash *options)
{
    MprBuf      *buf;
    HttpRoute   *route;
    cchar       *cp, *ep, *value;
    char        key[BIT_MAX_BUFFER];

    route = conn->rx->route;
    if (tplate == 0 || *tplate == '\0') {
        return MPR->emptyString;
    }
    buf = mprCreateBuf(-1, -1);
    for (cp = tplate; *cp; cp++) {
        if (*cp == '~' && (cp == tplate || cp[-1] != '\\')) {
            if (route->prefix) {
                mprPutStringToBuf(buf, route->prefix);
            }

        } else if (*cp == '$' && cp[1] == '{' && (cp == tplate || cp[-1] != '\\')) {
            cp += 2;
            if ((ep = strchr(cp, '}')) != 0) {
                sncopy(key, sizeof(key), cp, ep - cp);
                if (options && (value = httpGetOption(options, key, 0)) != 0) {
                    mprPutStringToBuf(buf, value);
                } else if ((value = mprLookupKey(conn->rx->params, key)) != 0) {
                    mprPutStringToBuf(buf, value);
                }
                if (value == 0) {
                    /* Just emit the token name if the token can't be found */
                    mprPutStringToBuf(buf, key);
                }
                cp = ep;
            }
        } else {
            mprPutCharToBuf(buf, *cp);
        }
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


PUBLIC void httpSetRouteVar(HttpRoute *route, cchar *key, cchar *value)
{
    assert(route);
    assert(key);
    assert(value);

    GRADUATE_HASH(route, vars);
    if (schr(value, '$')) {
        value = stemplate(value, route->vars);
    }
    mprAddKey(route->vars, key, sclone(value));
}


PUBLIC cchar *httpGetRouteVar(HttpRoute *route, cchar *key)
{
    return mprLookupKey(route->vars, key);
}


PUBLIC cchar *httpExpandRouteVars(HttpRoute *route, cchar *str)
{
    return stemplate(str, route->vars);
}


/*
    Make a path name. This replaces $references, converts to an absolute path name, cleans the path and maps delimiters.
    Paths are resolved relative to the route home.
 */
PUBLIC char *httpMakePath(HttpRoute *route, cchar *file)
{
    char    *path;

    assert(route);
    assert(file);

    if ((path = stemplate(file, route->vars)) == 0) {
        return 0;
    }
    if (mprIsPathRel(path) && route->host) {
        path = mprJoinPath(route->home, path);
    }
    return mprGetAbsPath(path);
}

/********************************* Language ***********************************/
/*
    Language can be an empty string
 */
PUBLIC int httpAddRouteLanguageSuffix(HttpRoute *route, cchar *language, cchar *suffix, int flags)
{
    HttpLang    *lp;

    assert(route);
    assert(language);
    assert(suffix && *suffix);

    if (route->languages == 0) {
        route->languages = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, languages);
    }
    if ((lp = mprLookupKey(route->languages, language)) != 0) {
        lp->suffix = sclone(suffix);
        lp->flags = flags;
    } else {
        mprAddKey(route->languages, language, createLangDef(0, suffix, flags));
    }
    return httpAddRouteUpdate(route, "lang", 0, 0);
}


PUBLIC int httpAddRouteLanguageDir(HttpRoute *route, cchar *language, cchar *path)
{
    HttpLang    *lp;

    assert(route);
    assert(language && *language);
    assert(path && *path);

    if (route->languages == 0) {
        route->languages = mprCreateHash(-1, 0);
    } else {
        GRADUATE_HASH(route, languages);
    }
    if ((lp = mprLookupKey(route->languages, language)) != 0) {
        lp->path = sclone(path);
    } else {
        mprAddKey(route->languages, language, createLangDef(path, 0, 0));
    }
    return httpAddRouteUpdate(route, "lang", 0, 0);
}


PUBLIC void httpSetRouteDefaultLanguage(HttpRoute *route, cchar *language)
{
    assert(route);
    assert(language && *language);

    route->defaultLanguage = sclone(language);
}


/********************************* Conditions *********************************/

static int testCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *condition)
{
    HttpRouteProc   *proc;

    assert(conn);
    assert(route);
    assert(condition);

    if ((proc = mprLookupKey(conn->http->routeConditions, condition->name)) == 0) {
        httpError(conn, -1, "Cannot find route condition rule %s", condition->name);
        return 0;
    }
    mprTrace(6, "run condition on route %s condition %s", route->name, condition->name);
    return (*proc)(conn, route, condition);
}


/*
    Allow/Deny authorization
 */
static int allowDenyCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpRx      *rx;
    HttpAuth    *auth;
    int         allow, deny;

    assert(conn);
    assert(route);

    rx = conn->rx;
    auth = rx->route->auth;
    if (auth == 0) {
        return HTTP_ROUTE_OK;
    }
    allow = 0;
    deny = 0;
    if (auth->flags & HTTP_ALLOW_DENY) {
        if (auth->allow && mprLookupKey(auth->allow, conn->ip)) {
            allow++;
        } else {
            allow++;
        }
        if (auth->deny && mprLookupKey(auth->deny, conn->ip)) {
            deny++;
        }
        if (!allow || deny) {
            httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access denied for this server %s", conn->ip);
            return HTTP_ROUTE_OK;
        }
    } else {
        if (auth->deny && mprLookupKey(auth->deny, conn->ip)) {
            deny++;
        }
        if (auth->allow && !mprLookupKey(auth->allow, conn->ip)) {
            deny = 0;
            allow++;
        } else {
            allow++;
        }
        if (deny || !allow) {
            httpError(conn, HTTP_CODE_UNAUTHORIZED, "Access denied for this server %s", conn->ip);
            return HTTP_ROUTE_OK;
        }
    }
    return HTTP_ROUTE_OK;
}


/*
    This condition is used to implement all user authentication for routes
 */
static int authCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpAuth    *auth;

    assert(conn);
    assert(route);

    auth = route->auth;
    if (!auth || !auth->type || auth->flags & HTTP_AUTO_LOGIN) {
        /* Authentication not required */
        return HTTP_ROUTE_OK;
    }
    if (!httpAuthenticate(conn)) {
        if (!conn->tx->finalized && route->auth && route->auth->type) {
            (route->auth->type->askLogin)(conn);
        }
        /* Request has been denied and fully handled */
        return HTTP_ROUTE_OK;
    }
    if (!httpCanUser(conn)) {
        httpError(conn, HTTP_CODE_FORBIDDEN, "Access denied. User is not authorized for access.");
    }
    return HTTP_ROUTE_OK;
}


/*
    This condition is used for "Condition unauthorized"
 */
static int unauthorizedCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpAuth    *auth;

    auth = route->auth;
    if (!auth || !auth->type || auth->flags & HTTP_AUTO_LOGIN) {
        return HTTP_ROUTE_REJECT;
    }
    return httpAuthenticate(conn) ? HTTP_ROUTE_REJECT : HTTP_ROUTE_OK;
}


static int directoryCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpTx      *tx;
    MprPath     info;
    char        *path;

    assert(conn);
    assert(route);
    assert(op);

    /* 
        Must have tx->filename set when expanding op->details, so map target now 
     */
    tx = conn->tx;
    httpMapFile(conn, route);
    path = mprJoinPath(route->dir, expandTokens(conn, op->details));
    tx->ext = tx->filename = 0;
    mprGetPathInfo(path, &info);
    if (info.isDir) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    Test if a file exists
 */
static int existsCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpTx  *tx;
    char    *path;

    assert(conn);
    assert(route);
    assert(op);

    /* 
        Must have tx->filename set when expanding op->details, so map target now 
     */
    tx = conn->tx;
    httpMapFile(conn, route);
    path = mprJoinPath(route->dir, expandTokens(conn, op->details));
    tx->ext = tx->filename = 0;
    if (mprPathExists(path, R_OK)) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


static int matchCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    char    *str;
    int     matched[BIT_MAX_ROUTE_MATCHES * 2], count;

    assert(conn);
    assert(route);
    assert(op);

    str = expandTokens(conn, op->details);
    count = pcre_exec(op->mdata, NULL, str, (int) slen(str), 0, 0, matched, sizeof(matched) / sizeof(int)); 
    if (count > 0) {
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    Test if the connection is secure
 */
static int secureCondition(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    int64   age;

    assert(conn);
    if (op->details && op->details) {
        /* Negative age means subDomains == true */
        age = stoi(op->details);
        if (age < 0) {
            httpAddHeader(conn, "Strict-Transport-Security", "max-age=%Ld; includeSubDomains", -age / MPR_TICKS_PER_SEC);
        } else {
            httpAddHeader(conn, "Strict-Transport-Security", "max-age=%Ld", age / MPR_TICKS_PER_SEC);
        }
    }
    if (!conn->secure) {
        httpError(conn, HTTP_CODE_UNAUTHORIZED, "SSL required for this route");
    }
    return HTTP_ROUTE_OK;
}

/********************************* Updates ******************************/

static int updateRequest(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpRouteProc   *proc;

    assert(conn);
    assert(route);
    assert(op);

    if ((proc = mprLookupKey(conn->http->routeUpdates, op->name)) == 0) {
        httpError(conn, -1, "Cannot find route update rule %s", op->name);
        return HTTP_ROUTE_OK;
    }
    mprTrace(6, "run update on route %s update %s", route->name, op->name);
    return (*proc)(conn, route, op);
}


static int cmdUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    MprCmd  *cmd;
    char    *command, *out, *err;
    int     status;

    assert(conn);
    assert(route);
    assert(op);

    command = expandTokens(conn, op->details);
    cmd = mprCreateCmd(conn->dispatcher);
    if ((status = mprRunCmd(cmd, command, NULL, &out, &err, -1, 0)) != 0) {
        /* Don't call httpError, just set errorMsg which can be retrieved via: ${request:error} */
        conn->errorMsg = sfmt("Command failed: %s\nStatus: %d\n%s\n%s", command, status, out, err);
        mprError("%s", conn->errorMsg);
        /* Continue */
    }
    return HTTP_ROUTE_OK;
}


static int paramUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    assert(conn);
    assert(route);
    assert(op);

    httpSetParam(conn, op->var, expandTokens(conn, op->value));
    return HTTP_ROUTE_OK;
}


static int langUpdate(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    HttpUri     *prior;
    HttpRx      *rx;
    HttpLang    *lang;
    char        *ext, *pathInfo, *uri;

    assert(conn);
    assert(route);

    rx = conn->rx;
    prior = rx->parsedUri;
    assert(route->languages);

    if ((lang = httpGetLanguage(conn, route->languages, 0)) != 0) {
        rx->lang = lang;
        if (lang->suffix) {
            pathInfo = 0;
            if (lang->flags & HTTP_LANG_AFTER) {
                pathInfo = sjoin(rx->pathInfo, ".", lang->suffix, NULL);
            } else if (lang->flags & HTTP_LANG_BEFORE) {
                ext = httpGetExt(conn);
                if (ext && *ext) {
                    pathInfo = sjoin(mprJoinPathExt(mprTrimPathExt(rx->pathInfo), lang->suffix), ".", ext, NULL);
                } else {
                    pathInfo = mprJoinPathExt(mprTrimPathExt(rx->pathInfo), lang->suffix);
                }
            }
            if (pathInfo) {
                uri = httpFormatUri(prior->scheme, prior->host, prior->port, pathInfo, prior->reference, prior->query, 0);
                httpSetUri(conn, uri);
            }
        }
    }
    return HTTP_ROUTE_OK;
}


/*********************************** Targets **********************************/

static int closeTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    assert(conn);
    assert(route);

    httpError(conn, HTTP_CODE_RESET | HTTP_ABORT, "Route target \"close\" is closing request");
    return HTTP_ROUTE_OK;
}


static int redirectTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    cchar       *target;

    assert(conn);
    assert(route);
    assert(route->target);

    target = expandTokens(conn, route->target);
    httpRedirect(conn, route->responseStatus ? route->responseStatus : HTTP_CODE_MOVED_TEMPORARILY, target);
    return HTTP_ROUTE_OK;
}


static int runTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    /*
        Need to re-compute output string as updates may have run to define params which affect the route->target tokens
     */
    conn->rx->target = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);
    return HTTP_ROUTE_OK;
}


static int writeTarget(HttpConn *conn, HttpRoute *route, HttpRouteOp *op)
{
    char    *str;

    assert(conn);
    assert(route);

    /*
        Need to re-compute output string as updates may have run to define params which affect the route->target tokens
     */
    str = route->target ? expandTokens(conn, route->target) : sclone(&conn->rx->pathInfo[1]);
    if (!(route->flags & HTTP_ROUTE_RAW)) {
        str = mprEscapeHtml(str);
    }
    httpSetStatus(conn, route->responseStatus);
    httpFormatResponse(conn, "%s", str);
    httpFinalize(conn);
    return HTTP_ROUTE_OK;
}


/************************************************** Route Convenience ****************************************************/

PUBLIC HttpRoute *httpDefineRoute(HttpRoute *parent, cchar *name, cchar *methods, cchar *pattern, cchar *target, cchar *source)
{
    HttpRoute   *route;

    if ((route = httpCreateInheritedRoute(parent)) == 0) {
        return 0;
    }
    httpSetRouteName(route, name);
    httpSetRoutePattern(route, pattern, 0);
    if (methods) {
        httpSetRouteMethods(route, methods);
    }
    if (source) {
        httpSetRouteSource(route, source);
    }
    httpSetRouteTarget(route, "run", target);
    httpFinalizeRoute(route);
    return route;
}


/*
    Calculate a qualified route name. The form is: /{app}/{controller}/action
 */
static char *qualifyName(HttpRoute *route, cchar *controller, cchar *action)
{
    cchar   *prefix, *controllerPrefix;

    prefix = route->prefix ? route->prefix : "";
    if (action == 0 || *action == '\0') {
        action = "default";
    }
    if (controller) {
        controllerPrefix = (controller && smatch(controller, "{controller}")) ? "*" : controller;
        return sjoin(prefix, "/", controllerPrefix, "/", action, NULL);
    } else {
        return sjoin(prefix, "/", action, NULL);
    }
}


/*
    Add a restful route. The parent route may supply a route prefix. If defined, the route name will prepend the prefix.
 */
static void addRestful(HttpRoute *parent, cchar *action, cchar *methods, cchar *pattern, cchar *target, cchar *resource)
{
    cchar   *name, *nameResource, *source;

    nameResource = smatch(resource, "{controller}") ? "*" : resource;
    if (parent->prefix) {
        name = sfmt("%s/%s/%s", parent->prefix, nameResource, action);
        pattern = sfmt("^%s/%s%s", parent->prefix, resource, pattern);
    } else {
        name = sfmt("/%s/%s", nameResource, action);
        pattern = sfmt("^/%s%s", resource, pattern);
    }
    if (*resource == '{') {
        target = sfmt("$%s-%s", resource, target);
        source = sfmt("$%s.c", resource);
    } else {
        target = sfmt("%s-%s", resource, target);
        source = sfmt("%s.c", resource);
    }
    httpDefineRoute(parent, name, methods, pattern, target, source);
}


/*
    httpAddResourceGroup(parent, "{controller}")
 */
PUBLIC void httpAddResourceGroup(HttpRoute *parent, cchar *resource)
{
    addRestful(parent, "list",      "GET",    "(/)*$",                   "list",          resource);
    addRestful(parent, "init",      "GET",    "/init$",                  "init",          resource);
    addRestful(parent, "create",    "POST",   "(/)*$",                   "create",        resource);
    addRestful(parent, "edit",      "GET",    "/{id=[0-9]+}/edit$",      "edit",          resource);
    addRestful(parent, "show",      "GET",    "/{id=[0-9]+}$",           "show",          resource);
    addRestful(parent, "update",    "PUT",    "/{id=[0-9]+}$",           "update",        resource);
    addRestful(parent, "destroy",   "DELETE", "/{id=[0-9]+}$",           "destroy",       resource);
    addRestful(parent, "custom",    "POST",   "/{action}/{id=[0-9]+}$",  "${action}",     resource);
    addRestful(parent, "default",   "*",      "/{action}$",              "cmd-${action}", resource);
}


/*
    httpAddResource(parent, "{controller}")
 */
PUBLIC void httpAddResource(HttpRoute *parent, cchar *resource)
{
    addRestful(parent, "init",      "GET",    "/init$",       "init",          resource);
    addRestful(parent, "create",    "POST",   "(/)*$",        "create",        resource);
    addRestful(parent, "edit",      "GET",    "/edit$",       "edit",          resource);
    addRestful(parent, "show",      "GET",    "$",            "show",          resource);
    addRestful(parent, "update",    "PUT",    "$",            "update",        resource);
    addRestful(parent, "destroy",   "DELETE", "$",            "destroy",       resource);
    addRestful(parent, "default",   "*",      "/{action}$",   "cmd-${action}", resource);
}


PUBLIC void httpAddHomeRoute(HttpRoute *parent)
{
    cchar   *source, *name, *path, *pattern, *prefix;

    prefix = parent->prefix ? parent->prefix : "";
    source = parent->sourceName;
    name = qualifyName(parent, NULL, "home");
    path = stemplate("${STATIC_DIR}/index.esp", parent->vars);
    pattern = sfmt("^%s%s", prefix, "(/)*$");
    httpDefineRoute(parent, name, "GET,POST", pattern, path, source);
}


PUBLIC void httpAddStaticRoute(HttpRoute *parent)
{
    HttpRoute   *route;
    cchar       *source, *name, *path, *pattern, *prefix;

    prefix = parent->prefix ? parent->prefix : "";
    source = parent->sourceName;
    name = qualifyName(parent, NULL, "static");
    path = stemplate("${STATIC_DIR}/$1", parent->vars);
    pattern = sfmt("^%s%s", prefix, "/static/(.*)");
    route = httpDefineRoute(parent, name, "GET", pattern, path, source);
    httpAddRouteHandler(route, "fileHandler", "");
}


PUBLIC void httpAddRouteSet(HttpRoute *parent, cchar *set)
{
    if (scaselessmatch(set, "simple")) {
        httpAddHomeRoute(parent);

    } else if (scaselessmatch(set, "mvc")) {
        httpAddHomeRoute(parent);
        httpAddStaticRoute(parent);
        httpDefineRoute(parent, "default", NULL, "^/{controller}(~/{action}~)", "${controller}-${action}", 
            "${controller}.c");

    } else if (scaselessmatch(set, "restful")) {
        httpAddHomeRoute(parent);
        httpAddStaticRoute(parent);
        httpAddResourceGroup(parent, "{controller}");

    } else if (!scaselessmatch(set, "none")) {
        mprError("Unknown route set %s", set);
    }
}


/*************************************************** Support Routines ****************************************************/
/*
    Route operations are used per-route for headers and fields
 */
static HttpRouteOp *createRouteOp(cchar *name, int flags)
{
    HttpRouteOp   *op;

    assert(name && *name);

    if ((op = mprAllocObj(HttpRouteOp, manageRouteOp)) == 0) {
        return 0;
    }
    op->name = sclone(name);
    op->flags = flags;
    return op;
}


static void manageRouteOp(HttpRouteOp *op, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(op->name);
        mprMark(op->details);
        mprMark(op->var);
        mprMark(op->value);

    } else if (flags & MPR_MANAGE_FREE) {
        if (op->flags & HTTP_ROUTE_FREE) {
            free(op->mdata);
            op->mdata = 0;
        }
    }
}


static bool opPresent(MprList *list, HttpRouteOp *op)
{
    HttpRouteOp   *last;

    if ((last = mprGetLastItem(list)) == 0) {
        return 0;
    }
    if (smatch(last->name, op->name) && 
        smatch(last->details, op->details) && 
        smatch(last->var, op->var) && 
        smatch(last->value, op->value) && 
        last->mdata == op->mdata && 
        last->flags == op->flags) {
        return 1;
    }
    return 0;
}


static void addUniqueItem(MprList *list, HttpRouteOp *op)
{
    assert(list);
    assert(op);

    if (!opPresent(list, op)) {
        mprAddItem(list, op);
    }
}


static HttpLang *createLangDef(cchar *path, cchar *suffix, int flags)
{
    HttpLang    *lang;

    if ((lang = mprAllocObj(HttpLang, manageLang)) == 0) {
        return 0;
    }
    if (path) {
        lang->path = sclone(path);
    }
    if (suffix) {
        lang->suffix = sclone(suffix);
    }
    lang->flags = flags;
    return lang;
}


static void manageLang(HttpLang *lang, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(lang->path);
        mprMark(lang->suffix);
    }
}


static void definePathVars(HttpRoute *route)
{
    assert(route);

    mprAddKey(route->vars, "PRODUCT", sclone(BIT_PRODUCT));
    mprAddKey(route->vars, "OS", sclone(BIT_OS));
    mprAddKey(route->vars, "VERSION", sclone(BIT_VERSION));

    mprAddKey(route->vars, "BIN_DIR", mprGetAppDir());
    //  DEPRECATED
    mprAddKey(route->vars, "LIBDIR", mprGetAppDir());
    if (route->host) {
        defineHostVars(route);
    }
}


static void defineHostVars(HttpRoute *route) 
{
    assert(route);
    mprAddKey(route->vars, "DOCUMENTS", route->dir);
    mprAddKey(route->vars, "ROUTE_HOME", route->home);
    mprAddKey(route->vars, "SERVER_NAME", route->host->name);
    mprAddKey(route->vars, "SERVER_PORT", itos(route->host->port));

    //  DEPRECATE
    mprAddKey(route->vars, "DOCUMENT_ROOT", route->dir);
    mprAddKey(route->vars, "SERVER_ROOT", route->home);
}


static char *expandTokens(HttpConn *conn, cchar *str)
{
    HttpRx      *rx;

    assert(conn);
    assert(str);

    rx = conn->rx;
    return expandRequestTokens(conn, expandPatternTokens(rx->pathInfo, str, rx->matches, rx->matchCount));
}


/*
    WARNING: str is modified. Result is allocated string.
 */
static char *expandRequestTokens(HttpConn *conn, char *str)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    MprBuf      *buf;
    HttpLang    *lang;
    char        *tok, *cp, *key, *value, *field, *header, *defaultValue, *state, *v, *p;

    assert(conn);
    assert(str);

    rx = conn->rx;
    route = rx->route;
    tx = conn->tx;
    buf = mprCreateBuf(-1, -1);
    tok = 0;

    for (cp = str; cp && *cp; ) {
        if ((tok = strstr(cp, "${")) == 0) {
            break;
        }
        if (tok > cp) {
            mprPutBlockToBuf(buf, cp, tok - cp);
        }
        if ((key = stok(&tok[2], ":}", &value)) == 0) {
            continue;
        }
        stok(value, "}", &cp);

        if (smatch(key, "header")) {
            header = stok(value, "=", &defaultValue);
            if ((value = (char*) httpGetHeader(conn, header)) == 0) {
                value = defaultValue ? defaultValue : "";
            }
            mprPutStringToBuf(buf, value);

        } else if (smatch(key, "param")) {
            field = stok(value, "=", &defaultValue);
            if (defaultValue == 0) {
                defaultValue = "";
            }
            mprPutStringToBuf(buf, httpGetParam(conn, field, defaultValue));

        } else if (smatch(key, "request")) {
            value = stok(value, "=", &defaultValue);
            //  OPT with switch on first char

            if (smatch(value, "authenticated")) {
                mprPutStringToBuf(buf, rx->authenticated ? "true" : "false");

            } else if (smatch(value, "clientAddress")) {
                mprPutStringToBuf(buf, conn->ip);

            } else if (smatch(value, "clientPort")) {
                mprPutIntToBuf(buf, conn->port);

            } else if (smatch(value, "error")) {
                mprPutStringToBuf(buf, conn->errorMsg);

            } else if (smatch(value, "ext")) {
                mprPutStringToBuf(buf, rx->parsedUri->ext);

            } else if (smatch(value, "extraPath")) {
                mprPutStringToBuf(buf, rx->extraPath);

            } else if (smatch(value, "filename")) {
                mprPutStringToBuf(buf, tx->filename);

            } else if (scaselessmatch(value, "language")) {
                if (!defaultValue) {
                    defaultValue = route->defaultLanguage;
                }
                if ((lang = httpGetLanguage(conn, route->languages, defaultValue)) != 0) {
                    mprPutStringToBuf(buf, lang->suffix);
                } else {
                    mprPutStringToBuf(buf, defaultValue);
                }

            } else if (scaselessmatch(value, "languageDir")) {
                lang = httpGetLanguage(conn, route->languages, 0);
                if (!defaultValue) {
                    defaultValue = ".";
                }
                mprPutStringToBuf(buf, lang ? lang->path : defaultValue);

            } else if (smatch(value, "host")) {
                /* Includes port if present */
                mprPutStringToBuf(buf, rx->parsedUri->host);

            } else if (smatch(value, "method")) {
                mprPutStringToBuf(buf, rx->method);

            } else if (smatch(value, "originalUri")) {
                mprPutStringToBuf(buf, rx->originalUri);

            } else if (smatch(value, "pathInfo")) {
                mprPutStringToBuf(buf, rx->pathInfo);

            } else if (smatch(value, "prefix")) {
                mprPutStringToBuf(buf, route->prefix);

            } else if (smatch(value, "query")) {
                mprPutStringToBuf(buf, rx->parsedUri->query);

            } else if (smatch(value, "reference")) {
                mprPutStringToBuf(buf, rx->parsedUri->reference);

            } else if (smatch(value, "scheme")) {
                if (rx->parsedUri->scheme) {
                    mprPutStringToBuf(buf, rx->parsedUri->scheme);
                }  else {
                    mprPutStringToBuf(buf, (conn->secure) ? "https" : "http");
                }

            } else if (smatch(value, "scriptName")) {
                mprPutStringToBuf(buf, rx->scriptName);

            } else if (smatch(value, "serverAddress")) {
                /* Pure IP address, no port. See "serverPort" */
                mprPutStringToBuf(buf, conn->sock->acceptIp);

            } else if (smatch(value, "serverPort")) {
                mprPutIntToBuf(buf, conn->sock->acceptPort);

            } else if (smatch(value, "uri")) {
                mprPutStringToBuf(buf, rx->uri);
            }
        } else if (smatch(key, "ssl")) {
            value = stok(value, "=", &defaultValue);
            if (smatch(value, "state")) {
                mprPutStringToBuf(buf, mprGetSocketState(conn->sock));
            } else {
                state = mprGetSocketState(conn->sock);
                if ((p = scontains(state, value)) != 0) {
                    stok(p, "=", &v);
                    mprPutStringToBuf(buf, stok(v, ", ", NULL));
                }
            }
        }
    }
    assert(cp);
    if (tok) {
        if (tok > cp) {
            mprPutBlockToBuf(buf, tok, tok - cp);
        }
    } else {
        mprPutStringToBuf(buf, cp);
    }
    mprAddNullToBuf(buf);
    return sclone(mprGetBufStart(buf));
}


PUBLIC char *httpExpandUri(HttpConn *conn, cchar *str)
{
    return expandRequestTokens(conn, stemplate(str, conn->rx->route->vars));
}


/*
    Replace text using pcre regular expression match indicies
 */
static char *expandPatternTokens(cchar *str, cchar *replacement, int *matches, int matchCount)
{
    MprBuf  *result;
    cchar   *end, *cp, *lastReplace;
    int     submatch;

    assert(str);
    assert(replacement);
    assert(matches);

    result = mprCreateBuf(-1, -1);
    lastReplace = replacement;
    end = &replacement[slen(replacement)];

    for (cp = replacement; cp < end; ) {
        if (*cp == '$') {
            if (lastReplace < cp) {
                mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
            }
            switch (*++cp) {
            case '$':
                mprPutCharToBuf(result, '$');
                break;
            case '&':
                /* Replace the matched string */
                if (matchCount > 0) {
                    mprPutSubStringToBuf(result, &str[matches[0]], matches[1] - matches[0]);
                }
                break;
            case '`':
                /* Insert the portion that preceeds the matched string */
                if (matchCount > 0) {
                    mprPutSubStringToBuf(result, str, matches[0]);
                }
                break;
            case '\'':
                /* Insert the portion that follows the matched string */
                if (matchCount > 0) {
                    mprPutSubStringToBuf(result, &str[matches[1]], slen(str) - matches[1]);
                }
                break;
            default:
                /* Insert the nth submatch */
                if (isdigit((uchar) *cp)) {
                    submatch = (int) atoi(cp);
                    while (isdigit((uchar) *++cp))
                        ;
                    cp--;
                    if (submatch < matchCount) {
                        submatch *= 2;
                        mprPutSubStringToBuf(result, &str[matches[submatch]], matches[submatch + 1] - matches[submatch]);
                    }
                } else {
                    mprError("Bad replacement $ specification in page");
                    return 0;
                }
            }
            lastReplace = cp + 1;
        }
        cp++;
    }
    if (lastReplace < cp && lastReplace < end) {
        mprPutSubStringToBuf(result, lastReplace, (int) (cp - lastReplace));
    }
    mprAddNullToBuf(result);
    return sclone(mprGetBufStart(result));
}


PUBLIC void httpDefineRouteBuiltins()
{
    /*
        These are the conditions that can be selected. Use httpAddRouteCondition to add to a route.
        The allow and auth conditions are internal and are configured via various Auth APIs.
     */
    httpDefineRouteCondition("allowDeny", allowDenyCondition);
    httpDefineRouteCondition("auth", authCondition);
    httpDefineRouteCondition("directory", directoryCondition);
    httpDefineRouteCondition("exists", existsCondition);
    httpDefineRouteCondition("match", matchCondition);
    httpDefineRouteCondition("secure", secureCondition);
    httpDefineRouteCondition("unauthorized", unauthorizedCondition);

    httpDefineRouteUpdate("param", paramUpdate);
    httpDefineRouteUpdate("cmd", cmdUpdate);
    httpDefineRouteUpdate("lang", langUpdate);

    httpDefineRouteTarget("close", closeTarget);
    httpDefineRouteTarget("redirect", redirectTarget);
    httpDefineRouteTarget("run", runTarget);
    httpDefineRouteTarget("write", writeTarget);
}


/*
    Tokenizes a line using %formats. Mandatory tokens can be specified with %. Optional tokens are specified with ?. 
    Supported tokens:
        %B - Boolean. Parses: on/off, true/false, yes/no.
        %N - Number. Parses numbers in base 10.
        %S - String. Removes quotes.
        %T - Template String. Removes quotes and expand ${PathVars}.
        %P - Path string. Removes quotes and expands ${PathVars}. Resolved relative to host->dir (Home).
        %W - Parse words into a list
        %! - Optional negate. Set value to HTTP_ROUTE_NOT present, otherwise zero.
    Values wrapped in quotes will have the outermost quotes trimmed.
 */
PUBLIC bool httpTokenize(HttpRoute *route, cchar *line, cchar *fmt, ...)
{
    va_list     args;
    bool        rc;

    assert(route);
    assert(line);
    assert(fmt);

    va_start(args, fmt);
    rc =  httpTokenizev(route, line, fmt, args);
    va_end(args);
    return rc;
}


PUBLIC bool httpTokenizev(HttpRoute *route, cchar *line, cchar *fmt, va_list args)
{
    MprList     *list;
    cchar       *f;
    char        *tok, *etok, *value, *word, *end;
    int         quote;

    assert(route);
    assert(fmt);

    if (line == 0) {
        line = "";
    }
    tok = sclone(line);
    end = &tok[slen(line)];

    for (f = fmt; *f && tok < end; f++) {
        for (; isspace((uchar) *tok); tok++) ;
        if (*tok == '\0' || *tok == '#') {
            break;
        }
        if (isspace((uchar) *f)) {
            continue;
        }
        if (*f == '%' || *f == '?') {
            f++;
            quote = 0;
            if (*f != '*' && (*tok == '"' || *tok == '\'')) {
                quote = *tok++;
            }
            if (*f == '!') {
                etok = &tok[1];
            } else {
                if (quote) {
                    for (etok = tok; *etok && !(*etok == quote && etok[-1] != '\\'); etok++) ; 
                    *etok++ = '\0';
                } else if (*f == '*') {
                    for (etok = tok; *etok; etok++) {
                        if (*etok == '#') {
                            *etok = '\0';
                        }
                    }
                } else {
                    for (etok = tok; *etok && !isspace((uchar) *etok); etok++) ;
                }
                *etok++ = '\0';
            }
            if (*f == '*') {
                f++;
                tok = trimQuotes(tok);
                * va_arg(args, char**) = tok;
                tok = etok;
                break;
            }

            switch (*f) {
            case '!':
                if (*tok == '!') {
                    *va_arg(args, int*) = HTTP_ROUTE_NOT;
                } else {
                    *va_arg(args, int*) = 0;
                    continue;
                }
                break;
            case 'B':
                if (scaselesscmp(tok, "on") == 0 || scaselesscmp(tok, "true") == 0 || scaselesscmp(tok, "yes") == 0) {
                    *va_arg(args, bool*) = 1;
                } else {
                    *va_arg(args, bool*) = 0;
                }
                break;
            case 'N':
                *va_arg(args, int*) = (int) stoi(tok);
                break;
            case 'P':
                *va_arg(args, char**) = httpMakePath(route, strim(tok, "\"", MPR_TRIM_BOTH));
                break;
            case 'S':
                *va_arg(args, char**) = strim(tok, "\"", MPR_TRIM_BOTH);
                break;
            case 'T':
                value = strim(tok, "\"", MPR_TRIM_BOTH);
                *va_arg(args, char**) = stemplate(value, route->vars);
                break;
            case 'W':
                list = va_arg(args, MprList*);
                word = stok(tok, " \t\r\n", &tok);
                while (word) {
                    mprAddItem(list, word);
                    word = stok(0, " \t\r\n", &tok);
                }
                break;
            default:
                mprError("Unknown token pattern %%\"%c\"", *f);
                break;
            }
            tok = etok;
        }
    }
    if (tok < end) {
        /*
            Extra unparsed text
         */
        for (; tok < end && isspace((uchar) *tok); tok++) ;
        if (*tok && *tok != '#') {
            mprError("Extra unparsed text: \"%s\"", tok);
            return 0;
        }
    }
    if (*f) {
        /*
            Extra unparsed format tokens
         */
        for (; *f; f++) {
            if (*f == '%') {
                break;
            } else if (*f == '?') {
                switch (*++f) {
                case '!':
                case 'N':
                    *va_arg(args, int*) = 0;
                    break;
                case 'B':
                    *va_arg(args, bool*) = 0;
                    break;
                case 'D':
                case 'P':
                case 'S':
                case 'T':
                case '*':
                    *va_arg(args, char**) = 0;
                    break;
                case 'W':
                    break;
                default:
                    mprError("Unknown token pattern %%\"%c\"", *f);
                    break;
                }
            }
        }
        if (*f) {
            mprError("Missing directive parameters");
            return 0;
        }
    }
    va_end(args);
    return 1;
}


static char *trimQuotes(char *str)
{
    ssize   len;

    assert(str);
    len = slen(str);
    if (*str == '\"' && str[len - 1] == '\"' && len > 2 && str[1] != '\"') {
        return snclone(&str[1], len - 2);
    }
    return sclone(str);
}


PUBLIC MprHash *httpGetOptions(cchar *options)
{
    if (options == 0) {
        return mprCreateHash(-1, 0);
    }
    if (*options == '@') {
        /* Allow embedded URIs as options */
        options = sfmt("{ data-click: '%s'}", options);
    }
    assert(*options == '{');
    if (*options != '{') {
        options = sfmt("{%s}", options);
    }
    return mprDeserialize(options);
}


PUBLIC void *httpGetOption(MprHash *options, cchar *field, cchar *defaultValue)
{
    MprKey      *kp;
    cchar       *value;

    if (options == 0) {
        value = defaultValue;
    } else if ((kp = mprLookupKeyEntry(options, field)) == 0) {
        value = defaultValue;
    } else {
        value = kp->data;
    }
    return (void*) value;
}


PUBLIC MprHash *httpGetOptionHash(MprHash *options, cchar *field)
{
    MprKey      *kp;

    if (options == 0) {
        return 0;
    }
    if ((kp = mprLookupKeyEntry(options, field)) == 0) {
        return 0;
    }
    if (kp->type != MPR_JSON_ARRAY && kp->type != MPR_JSON_OBJ) {
        return 0;
    }
    return (MprHash*) kp->data;
}


/* 
    Prepend an option
 */
PUBLIC void httpInsertOption(MprHash *options, cchar *field, cchar *value)
{
    MprKey      *kp;

    if (options == 0) {
        assert(options);
        return;
    }
    if ((kp = mprLookupKeyEntry(options, field)) != 0) {
        kp = mprAddKey(options, field, sjoin(value, " ", kp->data, NULL));
    } else {
        kp = mprAddKey(options, field, value);
    }
    kp->type = MPR_JSON_STRING;
}


PUBLIC void httpAddOption(MprHash *options, cchar *field, cchar *value)
{
    MprKey      *kp;

    if (options == 0) {
        assert(options);
        return;
    }
    if ((kp = mprLookupKeyEntry(options, field)) != 0) {
        kp = mprAddKey(options, field, sjoin(kp->data, " ", value, NULL));
    } else {
        kp = mprAddKey(options, field, value);
    }
    kp->type = MPR_JSON_STRING;
}


PUBLIC void httpRemoveOption(MprHash *options, cchar *field)
{
    if (options == 0) {
        assert(options);
        return;
    }
    mprRemoveKey(options, field);
}


PUBLIC bool httpOption(MprHash *hash, cchar *field, cchar *value, int useDefault)
{
    return smatch(value, httpGetOption(hash, field, useDefault ? value : 0));
}


PUBLIC void httpSetOption(MprHash *options, cchar *field, cchar *value)
{
    MprKey  *kp;

    if (value == 0) {
        return;
    }
    if (options == 0) {
        assert(options);
        return;
    }
    if ((kp = mprAddKey(options, field, value)) != 0) {
        kp->type = MPR_JSON_STRING;
    }
}


PUBLIC void httpEnableTraceMethod(HttpRoute *route, bool on)
{
    assert(route);
    if (on) {
        route->flags |= HTTP_ROUTE_TRACE_METHOD;
    } else {
        route->flags &= ~HTTP_ROUTE_TRACE_METHOD;
    }
}


PUBLIC HttpLimits *httpGraduateLimits(HttpRoute *route, HttpLimits *limits)
{
    if (route->parent && route->limits == route->parent->limits) {
        if (limits == 0) {
            limits = ((Http*) MPR->httpService)->serverLimits;
        }
        route->limits = mprMemdup(limits, sizeof(HttpLimits));
    }
    return route->limits;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/rx.c"
 */
/************************************************************************/

/*
    rx.c -- Http receiver. Parses http requests and client responses.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/***************************** Forward Declarations ***************************/

static void addMatchEtag(HttpConn *conn, char *etag);
static char *getToken(HttpConn *conn, cchar *delim);
static void manageRange(HttpRange *range, int flags);
static void manageRx(HttpRx *rx, int flags);
static bool parseHeaders(HttpConn *conn, HttpPacket *packet);
static bool parseIncoming(HttpConn *conn, HttpPacket *packet);
static bool parseRange(HttpConn *conn, char *value);
static bool parseRequestLine(HttpConn *conn, HttpPacket *packet);
static bool parseResponseLine(HttpConn *conn, HttpPacket *packet);
static void processCompletion(HttpConn *conn);
static bool processContent(HttpConn *conn, HttpPacket *packet);
static void parseMethod(HttpConn *conn);
static bool processParsed(HttpConn *conn);
static bool processReady(HttpConn *conn);
static bool processRunning(HttpConn *conn);
static void routeRequest(HttpConn *conn);
static int setParsedUri(HttpConn *conn);

/*********************************** Code *************************************/

PUBLIC HttpRx *httpCreateRx(HttpConn *conn)
{
    HttpRx      *rx;

    if ((rx = mprAllocObj(HttpRx, manageRx)) == 0) {
        return 0;
    }
    rx->conn = conn;
    rx->length = -1;
    rx->ifMatch = 1;
    rx->ifModified = 1;
    rx->pathInfo = sclone("/");
    rx->scriptName = mprEmptyString();
    rx->needInputPipeline = !conn->endpoint;
    rx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS);
    rx->chunkState = HTTP_CHUNK_UNCHUNKED;
    rx->traceLevel = -1;
    return rx;
}


static void manageRx(HttpRx *rx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(rx->method);
        mprMark(rx->uri);
        mprMark(rx->pathInfo);
        mprMark(rx->scriptName);
        mprMark(rx->extraPath);
        mprMark(rx->conn);
        mprMark(rx->route);
        mprMark(rx->etags);
        mprMark(rx->headerPacket);
        mprMark(rx->headers);
        mprMark(rx->inputPipeline);
        mprMark(rx->parsedUri);
        mprMark(rx->requestData);
        mprMark(rx->statusMessage);
        mprMark(rx->accept);
        mprMark(rx->acceptCharset);
        mprMark(rx->acceptEncoding);
        mprMark(rx->acceptLanguage);
        mprMark(rx->cookie);
        mprMark(rx->connection);
        mprMark(rx->contentLength);
        mprMark(rx->hostHeader);
        mprMark(rx->pragma);
        mprMark(rx->mimeType);
        mprMark(rx->originalMethod);
        mprMark(rx->originalUri);
        mprMark(rx->redirect);
        mprMark(rx->referrer);
        mprMark(rx->securityToken);
        mprMark(rx->session);
        mprMark(rx->userAgent);
        mprMark(rx->params);
        mprMark(rx->svars);
        mprMark(rx->inputRange);
        mprMark(rx->passDigest);
        mprMark(rx->files);
        mprMark(rx->uploadDir);
        mprMark(rx->paramString);
        mprMark(rx->lang);
        mprMark(rx->target);
        mprMark(rx->upgrade);
        mprMark(rx->webSocket);

    } else if (flags & MPR_MANAGE_FREE) {
        if (rx->conn) {
            rx->conn->rx = 0;
        }
    }
}


PUBLIC void httpDestroyRx(HttpRx *rx)
{
    if (rx->conn) {
        rx->conn->rx = 0;
        rx->conn = 0;
    }
}


/*  
    Pump the Http engine.
    Process an incoming request and drive the state machine. This will process only one request.
    All socket I/O is non-blocking, and this routine must not block. Note: packet may be null.
    Return true if the request is completed successfully.
 */
PUBLIC bool httpPumpRequest(HttpConn *conn, HttpPacket *packet)
{
    bool    canProceed;

    assert(conn);
    if (conn->pumping) {
        return 0;
    }
    canProceed = 1;
    conn->pumping = 1;

    while (canProceed) {
        mprTrace(6, "httpPumpRequest %s, state %d, error %d", conn->dispatcher->name, conn->state, conn->error);
        switch (conn->state) {
        case HTTP_STATE_BEGIN:
        case HTTP_STATE_CONNECTED:
            canProceed = parseIncoming(conn, packet);
            break;

        case HTTP_STATE_PARSED:
            canProceed = processParsed(conn);
            break;

        case HTTP_STATE_CONTENT:
            canProceed = processContent(conn, packet);
            break;

        case HTTP_STATE_READY:
            canProceed = processReady(conn);
            break;

        case HTTP_STATE_RUNNING:
            canProceed = processRunning(conn);
            break;

        case HTTP_STATE_FINALIZED:
            processCompletion(conn);
            break;

        case HTTP_STATE_COMPLETE:
            conn->pumping = 0;
            return !conn->connError;

        default:
            assert(conn->state == HTTP_STATE_COMPLETE);
            break;
        }
        packet = conn->input;
    }
    conn->pumping = 0;
    return 0;
}


/*  
    Parse the incoming http message. Return true to keep going with this or subsequent request, zero means
    insufficient data to proceed.
 */
static bool parseIncoming(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    ssize       len;
    char        *start, *end;

    if (packet == NULL) {
        return 0;
    }
    if (mprShouldDenyNewRequests()) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_NOT_ACCEPTABLE, "The server is terminating");
        return 0;
    }
    if (!conn->rx) {
        conn->rx = httpCreateRx(conn);
        conn->tx = httpCreateTx(conn, NULL);
    }
    rx = conn->rx;
    if ((len = httpGetPacketLength(packet)) == 0) {
        return 0;
    }
    start = mprGetBufStart(packet->content);

#if FUTURE
    while (*start == '\r' || *start == '\n') {
        mprGetCharFromBuf(packet->content);
        start = mprGetBufStart(packet->content);
    }
#endif

    /*
        Don't start processing until all the headers have been received (delimited by two blank lines)
     */
    if ((end = sncontains(start, "\r\n\r\n", len)) == 0) {
        if (len >= conn->limits->headerSize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE, 
                "Header too big. Length %d vs limit %d", len, conn->limits->headerSize);
        }
        return 0;
    }
    len = end - start;
    mprAddNullToBuf(packet->content);

    if (len >= conn->limits->headerSize) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE, 
            "Header too big. Length %d vs limit %d", len, conn->limits->headerSize);
        return 0;
    }
    if (conn->endpoint) {
        /* This will set conn->error if it does not validate - keep going to generate a response */
        if (!parseRequestLine(conn, packet)) {
            return 0;
        }
    } else if (!parseResponseLine(conn, packet)) {
        return 0;
    }
    if (!parseHeaders(conn, packet)) {
        return 0;
    }
    if (conn->endpoint) {
        httpMatchHost(conn);
        setParsedUri(conn);

    } else if (rx->status != HTTP_CODE_CONTINUE) {
        /* 
            Ignore Expect status responses. NOTE: Clients have already created their Tx pipeline.
         */
        httpCreateRxPipeline(conn, conn->http->clientRoute);
    }
    httpSetState(conn, HTTP_STATE_PARSED);
    return 1;
}


static void mapMethod(HttpConn *conn)
{
    HttpRx      *rx;
    cchar       *method;

    rx = conn->rx;
    if (rx->flags & HTTP_POST) {
        if ((method = httpGetParam(conn, "-http-method-", 0)) != 0) {
            if (!scaselessmatch(method, rx->method)) {
                mprLog(3, "Change method from %s to %s for %s", rx->method, method, rx->uri);
                httpSetMethod(conn, method);
            }
        }
    }
}


static void routeRequest(HttpConn *conn)
{
    HttpRx  *rx;

    assert(conn->endpoint);

    rx = conn->rx;
    httpAddParams(conn);
    mapMethod(conn);
    httpRouteRequest(conn);  
    httpCreateRxPipeline(conn, rx->route);
    httpCreateTxPipeline(conn, rx->route);
}


/*
    Only called by parseRequestLine
 */
static void traceRequest(HttpConn *conn, HttpPacket *packet)
{
    MprBuf  *content;
    cchar   *endp, *ext, *cp;
    int     len, level;

    content = packet->content;
    ext = 0;
    /*
        Find the Uri extension:   "GET /path.ext HTTP/1.1"
     */
    if ((cp = schr(content->start, ' ')) != 0) {
        if ((cp = schr(++cp, ' ')) != 0) {
            for (ext = --cp; ext > content->start && *ext != '.'; ext--) ;
            ext = (*ext == '.') ? snclone(&ext[1], cp - ext) : 0;
            conn->tx->ext = ext;
        }
    }
    /*
        If tracing header, do entire header including first line
     */
    if ((conn->rx->traceLevel = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, ext)) >= 0) {
        mprLog(4, "New request from %s:%d to %s:%d", conn->ip, conn->port, conn->sock->acceptIp, conn->sock->acceptPort);
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);

    } else if ((level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, ext)) >= 0) {
        endp = strstr((char*) content->start, "\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 2) : 0;
        if (len > 0) {
            content->start[len - 2] = '\0';
            mprLog(level, "%s", content->start);
            content->start[len - 2] = '\r';
        }
    }
    httpValidateLimits(conn->endpoint, HTTP_VALIDATE_OPEN_REQUEST, conn);
}


static void parseMethod(HttpConn *conn)
{
    HttpRx      *rx;
    cchar       *method;
    int         methodFlags;

    rx = conn->rx;
    method = rx->method;
    methodFlags = 0;

    switch (method[0]) {
    case 'D':
        if (strcmp(method, "DELETE") == 0) {
            methodFlags = HTTP_DELETE;
        }
        break;

    case 'G':
        if (strcmp(method, "GET") == 0) {
            methodFlags = HTTP_GET;
        }
        break;

    case 'H':
        if (strcmp(method, "HEAD") == 0) {
            methodFlags = HTTP_HEAD;
        }
        break;

    case 'O':
        if (strcmp(method, "OPTIONS") == 0) {
            methodFlags = HTTP_OPTIONS;
        }
        break;

    case 'P':
        if (strcmp(method, "POST") == 0) {
            methodFlags = HTTP_POST;
            rx->needInputPipeline = 1;

        } else if (strcmp(method, "PUT") == 0) {
            methodFlags = HTTP_PUT;
            rx->needInputPipeline = 1;
        }
        break;

    case 'T':
        if (strcmp(method, "TRACE") == 0) {
            methodFlags = HTTP_TRACE;
        }
        break;
    }
    rx->flags |= methodFlags;
}


/*  
    Parse the first line of a http request. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Requests look like: METHOD URL HTTP/1.X.
 */
static bool parseRequestLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    char        *uri, *protocol;
    ssize       len;

    rx = conn->rx;
#if BIT_DEBUG
    conn->startMark = mprGetHiResTicks();
#endif
    traceRequest(conn, packet);

    rx->originalMethod = rx->method = supper(getToken(conn, 0));
    parseMethod(conn);

    uri = getToken(conn, 0);
    len = slen(uri);
    if (*uri == '\0') {
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad HTTP request. Empty URI");
        return 0;
    } else if (len >= conn->limits->uriSize) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_URL_TOO_LARGE, 
            "Bad request. URI too long. Length %d vs limit %d", len, conn->limits->uriSize);
        return 0;
    }
    protocol = conn->protocol = supper(getToken(conn, "\r\n"));
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        if (rx->flags & (HTTP_POST|HTTP_PUT)) {
            rx->remainingContent = MAXINT;
            rx->needInputPipeline = 1;
        }
        conn->http10 = 1;
        conn->protocol = protocol;
    } else if (strcmp(protocol, "HTTP/1.1") == 0) {
        conn->protocol = protocol;
    } else {
        conn->protocol = sclone("HTTP/1.1");
        httpError(conn, HTTP_ABORT | HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
        return 0;
    }
    rx->originalUri = rx->uri = sclone(uri);
    conn->http->totalRequests++;
    httpSetState(conn, HTTP_STATE_FIRST);
    return 1;
}


/*  
    Parse the first line of a http response. Return true if the first line parsed. This is only called once all the headers
    have been read and buffered. Response status lines look like: HTTP/1.X CODE Message
 */
static bool parseResponseLine(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprBuf      *content;
    cchar       *endp;
    char        *protocol, *status;
    ssize       len;
    int         level, traced;

    rx = conn->rx;
    tx = conn->tx;
    traced = 0;

    if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, tx->ext) >= 0) {
        content = packet->content;
        endp = strstr((char*) content->start, "\r\n\r\n");
        len = (endp) ? (int) (endp - mprGetBufStart(content) + 4) : 0;
        httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_HEADER, packet, len, 0);
        traced = 1;
    }
    protocol = conn->protocol = supper(getToken(conn, 0));
    if (strcmp(protocol, "HTTP/1.0") == 0) {
        conn->http10 = 1;
        if (!scaselessmatch(tx->method, "HEAD")) {
            rx->remainingContent = MAXINT;
        }
    } else if (strcmp(protocol, "HTTP/1.1") != 0) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_NOT_ACCEPTABLE, "Unsupported HTTP protocol");
        return 0;
    }
    status = getToken(conn, 0);
    if (*status == '\0') {
        httpError(conn, HTTP_ABORT | HTTP_CODE_NOT_ACCEPTABLE, "Bad response status code");
        return 0;
    }
    rx->status = atoi(status);
    rx->statusMessage = sclone(getToken(conn, "\r\n"));

    len = slen(rx->statusMessage);
    if (len >= conn->limits->uriSize) {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_URL_TOO_LARGE, 
            "Bad response. Status message too long. Length %d vs limit %d", len, conn->limits->uriSize);
        return 0;
    }
    if (!traced && (level = httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_FIRST, tx->ext)) >= 0) {
        mprLog(level, "%s %d %s", protocol, rx->status, rx->statusMessage);
    }
    return 1;
}


/*  
    Parse the request headers. Return true if the header parsed.
 */
static bool parseHeaders(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpLimits  *limits;
    MprBuf      *content;
    char        *cp, *key, *value, *tok, *hvalue;
    cchar       *oldValue;
    int         count, keepAlive;

    rx = conn->rx;
    tx = conn->tx;
    content = packet->content;
    conn->rx->headerPacket = packet;
    limits = conn->limits;
    keepAlive = (conn->http10) ? 0 : 1;

    for (count = 0; content->start[0] != '\r' && !conn->error; count++) {
        if (count >= limits->headerMax) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Too many headers");
            return 0;
        }
        if ((key = getToken(conn, ":")) == 0 || *key == '\0') {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad header format");
            return 0;
        }
        value = getToken(conn, "\r\n");
        while (isspace((uchar) *value)) {
            value++;
        }
        mprTrace(8, "Key %s, value %s", key, value);
        if (strspn(key, "%<>/\\") > 0) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad header key value");
            return 0;
        }
        if ((oldValue = mprLookupKey(rx->headers, key)) != 0) {
            hvalue = sfmt("%s, %s", oldValue, value);
        } else {
            hvalue = sclone(value);
        }
        mprAddKey(rx->headers, key, hvalue);

        switch (tolower((uchar) key[0])) {
        case 'a':
            if (strcasecmp(key, "authorization") == 0) {
                value = sclone(value);
                conn->authType = slower(stok(value, " \t", &tok));
                rx->authDetails = sclone(tok);

            } else if (strcasecmp(key, "accept-charset") == 0) {
                rx->acceptCharset = sclone(value);

            } else if (strcasecmp(key, "accept") == 0) {
                rx->accept = sclone(value);

            } else if (strcasecmp(key, "accept-encoding") == 0) {
                rx->acceptEncoding = sclone(value);

            } else if (strcasecmp(key, "accept-language") == 0) {
                rx->acceptLanguage = sclone(value);
            }
            break;

        case 'c':
            if (strcasecmp(key, "connection") == 0) {
                rx->connection = sclone(value);
                if (scaselesscmp(value, "KEEP-ALIVE") == 0) {
                    keepAlive = 1;
                } else if (scaselesscmp(value, "CLOSE") == 0) {
                    /*  Not really required, but set to 0 to be sure */
                    conn->keepAliveCount = 0;
                }

            } else if (strcasecmp(key, "content-length") == 0) {
                if (rx->length >= 0) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_BAD_REQUEST, "Mulitple content length headers");
                    break;
                }
                rx->length = stoi(value);
                if (rx->length < 0) {
                    httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad content length");
                    return 0;
                }
                if (rx->length >= conn->limits->receiveBodySize) {
                    httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                        "Request content length %,Ld bytes is too big. Limit %,Ld", 
                        rx->length, conn->limits->receiveBodySize);
                    return 0;
                }
                rx->contentLength = sclone(value);
                assert(rx->length >= 0);
                if (conn->endpoint || !scaselessmatch(tx->method, "HEAD")) {
                    rx->remainingContent = rx->length;
                    rx->needInputPipeline = 1;
                }

            } else if (strcasecmp(key, "content-range") == 0) {
                /*
                    This headers specifies the range of any posted body data
                    Format is:  Content-Range: bytes n1-n2/length
                    Where n1 is first byte pos and n2 is last byte pos
                 */
                char    *sp;
                MprOff  start, end, size;

                start = end = size = -1;
                sp = value;
                while (*sp && !isdigit((uchar) *sp)) {
                    sp++;
                }
                if (*sp) {
                    start = stoi(sp);
                    if ((sp = strchr(sp, '-')) != 0) {
                        end = stoi(++sp);
                    }
                    if ((sp = strchr(sp, '/')) != 0) {
                        /*
                            Note this is not the content length transmitted, but the original size of the input of which
                            the client is transmitting only a portion.
                         */
                        size = stoi(++sp);
                    }
                }
                if (start < 0 || end < 0 || size < 0 || end <= start) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad content range");
                    break;
                }
                rx->inputRange = httpCreateRange(conn, start, end);

            } else if (strcasecmp(key, "content-type") == 0) {
                rx->mimeType = sclone(value);
                if (rx->flags & (HTTP_POST | HTTP_PUT)) {
                    rx->form = scontains(rx->mimeType, "application/x-www-form-urlencoded") != 0;
                    rx->upload = scontains(rx->mimeType, "multipart/form-data") != 0;
                } else { 
                    rx->form = rx->upload = 0;
                }

            } else if (strcasecmp(key, "cookie") == 0) {
                if (rx->cookie && *rx->cookie) {
                    rx->cookie = sjoin(rx->cookie, "; ", value, NULL);
                } else {
                    rx->cookie = sclone(value);
                }
            }
            break;

        case 'e':
            if (strcasecmp(key, "expect") == 0) {
                /*
                    Handle 100-continue for HTTP/1.1 clients only. This is the only expectation that is currently supported.
                 */
                if (!conn->http10) {
                    if (strcasecmp(value, "100-continue") != 0) {
                        httpError(conn, HTTP_CODE_EXPECTATION_FAILED, "Expect header value \"%s\" is unsupported", value);
                    } else {
                        rx->flags |= HTTP_EXPECT_CONTINUE;
                    }
                }
            }
            break;

        case 'h':
            if (strcasecmp(key, "host") == 0) {
                rx->hostHeader = sclone(value);
            }
            break;

        case 'i':
            if ((strcasecmp(key, "if-modified-since") == 0) || (strcasecmp(key, "if-unmodified-since") == 0)) {
                MprTime     newDate = 0;
                char        *cp;
                bool        ifModified = (tolower((uchar) key[3]) == 'm');

                if ((cp = strchr(value, ';')) != 0) {
                    *cp = '\0';
                }
                if (mprParseTime(&newDate, value, MPR_UTC_TIMEZONE, NULL) < 0) {
                    assert(0);
                    break;
                }
                if (newDate) {
                    rx->since = newDate;
                    rx->ifModified = ifModified;
                    rx->flags |= HTTP_IF_MODIFIED;
                }

            } else if ((strcasecmp(key, "if-match") == 0) || (strcasecmp(key, "if-none-match") == 0)) {
                char    *word, *tok;
                bool    ifMatch = (tolower((uchar) key[3]) == 'm');

                if ((tok = strchr(value, ';')) != 0) {
                    *tok = '\0';
                }
                rx->ifMatch = ifMatch;
                rx->flags |= HTTP_IF_MODIFIED;
                value = sclone(value);
                word = stok(value, " ,", &tok);
                while (word) {
                    addMatchEtag(conn, word);
                    word = stok(0, " ,", &tok);
                }

            } else if (strcasecmp(key, "if-range") == 0) {
                char    *word, *tok;

                if ((tok = strchr(value, ';')) != 0) {
                    *tok = '\0';
                }
                rx->ifMatch = 1;
                rx->flags |= HTTP_IF_MODIFIED;
                value = sclone(value);
                word = stok(value, " ,", &tok);
                while (word) {
                    addMatchEtag(conn, word);
                    word = stok(0, " ,", &tok);
                }
            }
            break;

        case 'k':
            /* Keep-Alive: timeout=N, max=1 */
            if (strcasecmp(key, "keep-alive") == 0) {
                keepAlive = 1;
                if ((tok = scontains(value, "max=")) != 0) {
                    conn->keepAliveCount = atoi(&tok[4]);
                    /*  
                        IMPORTANT: Deliberately close the connection one request early. This ensures a client-led 
                        termination and helps relieve server-side TIME_WAIT conditions.
                     */
                    if (conn->keepAliveCount == 1) {
                        conn->keepAliveCount = 0;
                    }
                }
            }
            break;                
                
        case 'l':
            if (strcasecmp(key, "location") == 0) {
                rx->redirect = sclone(value);
            }
            break;

        case 'o':
            if (strcasecmp(key, "origin") == 0) {
                rx->origin = sclone(value);
            }
            break;

        case 'p':
            if (strcasecmp(key, "pragma") == 0) {
                rx->pragma = sclone(value);
            }
            break;

        case 'r':
            if (strcasecmp(key, "range") == 0) {
                if (!parseRange(conn, value)) {
                    httpError(conn, HTTP_CLOSE | HTTP_CODE_RANGE_NOT_SATISFIABLE, "Bad range");
                }
            } else if (strcasecmp(key, "referer") == 0) {
                /* NOTE: yes the header is misspelt in the spec */
                rx->referrer = sclone(value);
            }
            break;

        case 't':
            if (strcasecmp(key, "transfer-encoding") == 0) {
                if (scaselesscmp(value, "chunked") == 0) {
                    /*  
                        remainingContent will be revised by the chunk filter as chunks are processed and will 
                        be set to zero when the last chunk has been received.
                     */
                    rx->flags |= HTTP_CHUNKED;
                    rx->chunkState = HTTP_CHUNK_START;
                    rx->remainingContent = MAXINT;
                    rx->needInputPipeline = 1;
                }
            }
            break;

        case 'x':
            if (strcasecmp(key, "x-http-method-override") == 0) {
                httpSetMethod(conn, value);
#if BIT_DEBUG
            } else if (strcasecmp(key, "x-chunk-size") == 0) {
                tx->chunkSize = atoi(value);
                if (tx->chunkSize <= 0) {
                    tx->chunkSize = 0;
                } else if (tx->chunkSize > conn->limits->chunkSize) {
                    tx->chunkSize = conn->limits->chunkSize;
                }
#endif
            }
            break;

        case 'u':
            if (scaselesscmp(key, "upgrade") == 0) {
                rx->upgrade = sclone(value);
            } else if (strcasecmp(key, "user-agent") == 0) {
                rx->userAgent = sclone(value);
            }
            break;

        case 'w':
            if (strcasecmp(key, "www-authenticate") == 0) {
                cp = value;
                while (*value && !isspace((uchar) *value)) {
                    value++;
                }
                *value++ = '\0';
                conn->authType = slower(cp);
                rx->authDetails = sclone(value);
            }
            break;
        }
    }
    if (!keepAlive) {
        conn->keepAliveCount = 0;
    }
    if (!(rx->flags & HTTP_CHUNKED)) {
        /*  
            Step over "\r\n" after headers. 
            Don't do this if chunked so chunking can parse a single chunk delimiter of "\r\nSIZE ...\r\n"
         */
        mprAdjustBufStart(content, 2);
    }
    return 1;
}


/*
    Sends an 100 Continue response to the client. This bypasses the transmission pipeline, writing directly to the socket.
 */
static int sendContinue(HttpConn *conn)
{
    cchar      *response;

    assert(conn);
    assert(conn->sock);

    /* Write the response to the socket and flush. */
    response = sfmt("%s 100 Continue\r\n\r\n", conn->protocol);
    mprWriteSocket(conn->sock, response, slen(response));
    mprFlushSocket(conn->sock);
    return 0;
}


/*
    Called once the HTTP request/response headers have been parsed
 */
static bool processParsed(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (!rx->form && conn->endpoint) {
        /*
            Routes need to be able to access form data, so forms will route later after all input is received.
         */
        routeRequest(conn);
    }
    /*
        Send a 100 (Continue) response if the client has requested it. If the connection has an error, that takes
        precedence and 100 Continue will not be sent. Also, if the connector has already written bytes to the socket, we
        do not send 100 Continue to avoid corrupting the response.
     */
    if ((rx->flags & HTTP_EXPECT_CONTINUE) && !conn->tx->finalized && !conn->tx->bytesWritten) {
        sendContinue(conn);
        rx->flags &= ~HTTP_EXPECT_CONTINUE;
    }
    if (!conn->endpoint && conn->upgraded && !httpVerifyWebSocketsHandshake(conn)) {
        httpSetState(conn, HTTP_STATE_FINALIZED);
        return 1;
    }
    httpSetState(conn, HTTP_STATE_CONTENT);

    if (rx->remainingContent == 0) {
        rx->eof = 1;
    }
    if (conn->endpoint && !(rx->form || rx->upload)) {
        httpStartPipeline(conn);
    }
    if (rx->eof) {
        httpSetState(conn, HTTP_STATE_READY);
    }
    return 1;
}


static bool processContent(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpQueue   *q;
    MprBuf      *content;
    ssize       nbytes;

    assert(conn);
    if (!packet) {
        httpServiceQueues(conn);
        return 0;
    }
    rx = conn->rx;
    tx = conn->tx;
    content = packet->content;

    q = tx->queue[HTTP_QUEUE_RX];
    VERIFY_QUEUE(q);
    mprTrace(6, "processContent: packet of %d bytes, remaining %d", mprGetBufLength(content), rx->remainingContent);
    
    /*
        Determine if end of input (end-of-file)
     */
    if (rx->chunkState) {
        nbytes = httpFilterChunkData(q, packet);
        if (rx->chunkState == HTTP_CHUNK_EOF) {
            rx->eof = 1;
            assert(rx->remainingContent == 0);
        }
    } else {
        nbytes = (ssize) min(rx->remainingContent, mprGetBufLength(content));
        if (!conn->upgraded && (rx->remainingContent - nbytes) <= 0) {
            rx->eof = 1;
        }
    }
    if (mprIsSocketEof(conn->sock)) {
        rx->eof = 1;
    }
    if (nbytes > 0) {
        if (!conn->upgraded) {
            rx->remainingContent -= nbytes;
            assert(rx->remainingContent >= 0);
        }
        rx->bytesRead += nbytes;
        if (httpShouldTrace(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, tx->ext) >= 0) {
            httpTraceContent(conn, HTTP_TRACE_RX, HTTP_TRACE_BODY, packet, nbytes, rx->bytesRead);
        }
        /*
            Enforce sandbox limits
         */
        if (!conn->error) {
            if (rx->bytesRead >= conn->limits->receiveBodySize) {
                httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_TOO_LARGE, 
                    "Request body of %,Ld bytes is too big. Limit %,Ld", rx->bytesRead, conn->limits->receiveBodySize);

            } else if (rx->form && rx->bytesRead >= conn->limits->receiveFormSize) {
                httpError(conn, HTTP_CLOSE | HTTP_CODE_REQUEST_TOO_LARGE, 
                    "Request form of %,Ld bytes is too big. Limit %,Ld", rx->bytesRead, conn->limits->receiveFormSize);
            }
        }
        /*
            Send packet upstream toward the handler
         */
        if (packet == rx->headerPacket && nbytes > 0) {
            packet = httpSplitPacket(packet, 0);
        }
        if (httpGetPacketLength(packet) > nbytes) {
            /*  Split excess data belonging to the next chunk or pipelined request */
            mprTrace(7, "processContent: Split packet of %d at %d", httpGetPacketLength(packet), nbytes);
            conn->input = httpSplitPacket(packet, nbytes);
        } else {
            conn->input = 0;
        }
        if (!(tx->finalized && conn->endpoint)) {
            if (rx->form) {
                httpPutForService(q, packet, HTTP_DELAY_SERVICE);
            } else {
                httpPutPacketToNext(q, packet);
            }
        }
    }
    if (rx->eof) {
        if (rx->remainingContent > 0 && !conn->http10) {
            /* Closing is the only way for HTTP/1.0 to signify the end of data */
            httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Connection lost");
        }
        if (rx->form && conn->endpoint) {
            /* Forms wait for all data before routing */
            routeRequest(conn);
            while ((packet = httpGetPacket(q)) != 0) {
                httpPutPacketToNext(q, packet);
            }
        }
        /*
            Send "end" pack to signify eof to the handler
         */
        httpPutPacketToNext(q, httpCreateEndPacket());
        if (!tx->started) {
            httpStartPipeline(conn);
        }
        httpSetState(conn, HTTP_STATE_READY);
        return conn->workerEvent ? 0 : 1;
    }
    if (tx->started) {
        httpServiceQueues(conn);
    }
    if (rx->chunkState && nbytes <= 0) {
        /* Insufficient data */
        return 0;
    }
    if (conn->input && httpGetPacketLength(conn->input) > 0) {
        /* More data to process */
        return 1;
    }
    /* Must wait */
    return 0;
}


/*
    In the ready state after all content has been received
 */
static bool processReady(HttpConn *conn)
{
    httpServiceQueues(conn);
    httpReadyHandler(conn);
    httpSetState(conn, HTTP_STATE_RUNNING);
    return 1;
}


/*
    Note: may be called multiple times in response to output I/O events
 */
static bool processRunning(HttpConn *conn)
{
    HttpQueue   *q;
    HttpTx      *tx;
    int         canProceed;

    q = conn->writeq;
    tx = conn->tx;
    canProceed = 1;
    httpServiceQueues(conn);

    if (conn->endpoint) {
        /* Server side */
        if (tx->finalized) {
            if (tx->finalizedConnector) {
                /* Request complete and output complete */
                httpSetState(conn, HTTP_STATE_FINALIZED);
            } else {
                /* Still got output to do. Wait for Tx I/O event. Do suspend incase handler not using auto-flow routines */
                tx->writeBlocked = 1;
                httpSuspendQueue(q);
                httpEnableConnEvents(q->conn);
                canProceed = 0;
                assert(conn->state < HTTP_STATE_FINALIZED);
            }

        } else if (!httpGetMoreOutput(conn)) {
            /* Request not complete yet. No process callback defined */
            canProceed = 0;
            assert(conn->state < HTTP_STATE_FINALIZED);

        } else if (conn->state >= HTTP_STATE_FINALIZED) {
            /* This happens when httpGetMoreOutput calls writable on windows which then completes the request */
            canProceed = 1;

        } else if (q->count < q->low) {
            if (q->count == 0) {
                /* Queue is empty and data may have drained above in httpServiceQueues. Yield to reclaim memory. */
                mprYield(0);
            }
            if (q->flags & HTTP_QUEUE_SUSPENDED) {
                httpResumeQueue(q);
            }
            /* Need to give events a chance to run. Otherwise can ping/pong suspend to resume */
            canProceed = 0;

        } else {
            /* Wait for output to drain */
            tx->writeBlocked = 1;
            httpSuspendQueue(q);
            httpEnableConnEvents(q->conn);
            canProceed = 0;
            assert(conn->state < HTTP_STATE_FINALIZED);
        }
    } else {
        /* Client side */
        httpServiceQueues(conn);
        if (conn->upgraded) {
            canProceed = 0;
            assert(conn->state < HTTP_STATE_FINALIZED);
        } else {
            httpFinalize(conn);
            if (tx->finalized && conn->rx->eof) {
                httpSetState(conn, HTTP_STATE_FINALIZED);
            } else {
                assert(0);
            }
        }
    }
    return canProceed;
}


#if BIT_DEBUG
static void measure(HttpConn *conn)
{
    MprTicks    elapsed;
    HttpTx      *tx;
    cchar       *uri;
    int         level;

    tx = conn->tx;
    if (conn->rx == 0 || tx == 0) {
        return;
    }
    uri = (conn->endpoint) ? conn->rx->uri : tx->parsedUri->path;
   
    if ((level = httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_TIME, tx->ext)) >= 0) {
        elapsed = mprGetTicks() - conn->started;
#if MPR_HIGH_RES_TIMER
        if (elapsed < 1000) {
            mprTrace(level, "TIME: Request %s took %,d msec %,d ticks", uri, elapsed, mprGetHiResTicks() - conn->startMark);
        } else
#endif
            mprTrace(level, "TIME: Request %s took %,d msec", uri, elapsed);
    }
}
#else
#define measure(conn)
#endif


static void createErrorRequest(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpPacket  *packet;
    MprBuf      *buf;
    char        *cp, *headers;
    int         key;

    rx = conn->rx;
    tx = conn->tx;
    if (!rx->headerPacket) {
        return;
    }
    conn->rx = httpCreateRx(conn);
    conn->tx = httpCreateTx(conn, NULL);

    /* Preserve the old status */
    conn->tx->status = tx->status;
    conn->error = 0;
    conn->errorMsg = 0;
    conn->upgraded = 0;
    conn->worker = 0;

    packet = httpCreateDataPacket(BIT_MAX_BUFFER);
    mprPutToBuf(packet->content, "%s %s %s\r\n", rx->method, tx->errorDocument, conn->protocol);
    buf = rx->headerPacket->content;
    /*
        Sever the old Rx and Tx for GC
     */
    rx->conn = 0;
    tx->conn = 0;

    /*
        Reconstruct the headers. Change nulls to '\r', ' ', or ':' as appropriate
     */
    key = 0;
    headers = 0;
    for (cp = buf->data; cp < &buf->end[-1]; cp++) {
        if (*cp == '\0') {
            if (cp[1] == '\n') {
                if (!headers) {
                    headers = &cp[2];
                }
                *cp = '\r';
                key = 0;
            } else if (!key) {
                *cp = ':';
                key = 1;
            } else {
                *cp = ' ';
            }
        }
    }
    if (headers && headers < buf->end) {
        mprPutStringToBuf(packet->content, headers);
        conn->input = packet;
        conn->state = HTTP_STATE_CONNECTED;
    } else {
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Can't reconstruct headers");
    }
}


static void processCompletion(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;

    rx = conn->rx;
    tx = conn->tx;
    assert(tx->finalized);
    assert(tx->finalizedOutput);

#if BIT_TRACE_MEM
    mprTrace(1, "Request complete, status %d, error %d, connError %d, %s%s, memsize %.2f MB",
        tx->status, conn->error, conn->connError, rx->hostHeader, rx->uri, mprGetMem() / 1024 / 1024.0);
#endif
    httpDestroyPipeline(conn);
    measure(conn);
    if (conn->endpoint && rx) {
        assert(rx->route);
        if (rx->route && rx->route->log) {
            httpLogRequest(conn);
        }
        httpValidateLimits(conn->endpoint, HTTP_VALIDATE_CLOSE_REQUEST, conn);
    }
    assert(conn->state == HTTP_STATE_FINALIZED);
    httpSetState(conn, HTTP_STATE_COMPLETE);
    if (tx->errorDocument && !conn->connError && !smatch(tx->errorDocument, rx->uri)) {
        mprLog(2, "  ErrorDoc %s for %d from %s", tx->errorDocument, tx->status, rx->uri);
        createErrorRequest(conn);
    }
}


/*
    Used by ejscript Request.close
 */
PUBLIC void httpCloseRx(HttpConn *conn)
{
    if (conn->rx && !conn->rx->remainingContent) {
        /* May not have consumed all read data, so can't be assured the next request will be okay */
        conn->keepAliveCount = -1;
    }
    if (conn->state < HTTP_STATE_FINALIZED) {
        httpPumpRequest(conn, NULL);
    }
}


PUBLIC bool httpContentNotModified(HttpConn *conn)
{
    HttpRx      *rx;
    HttpTx      *tx;
    MprTime     modified;
    bool        same;

    rx = conn->rx;
    tx = conn->tx;

    if (rx->flags & HTTP_IF_MODIFIED) {
        /*  
            If both checks, the last modification time and etag, claim that the request doesn't need to be
            performed, skip the transfer.
         */
        assert(tx->fileInfo.valid);
        modified = (MprTime) tx->fileInfo.mtime * MPR_TICKS_PER_SEC;
        same = httpMatchModified(conn, modified) && httpMatchEtag(conn, tx->etag);
        if (tx->outputRanges && !same) {
            tx->outputRanges = 0;
        }
        return same;
    }
    return 0;
}


PUBLIC HttpRange *httpCreateRange(HttpConn *conn, MprOff start, MprOff end)
{
    HttpRange     *range;

    if ((range = mprAllocObj(HttpRange, manageRange)) == 0) {
        return 0;
    }
    range->start = start;
    range->end = end;
    range->len = end - start;
    return range;
}


static void manageRange(HttpRange *range, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(range->next);
    }
}


PUBLIC MprOff httpGetContentLength(HttpConn *conn)
{
    if (conn->rx == 0) {
        assert(conn->rx);
        return 0;
    }
    return conn->rx->length;
}


PUBLIC cchar *httpGetCookies(HttpConn *conn)
{
    if (conn->rx == 0) {
        assert(conn->rx);
        return 0;
    }
    return conn->rx->cookie;
}


PUBLIC cchar *httpGetHeader(HttpConn *conn, cchar *key)
{
    if (conn->rx == 0) {
        assert(conn->rx);
        return 0;
    }
    return mprLookupKey(conn->rx->headers, slower(key));
}


PUBLIC char *httpGetHeadersFromHash(MprHash *hash)
{
    MprKey      *kp;
    char        *headers, *cp;
    ssize       len;

    for (len = 0, kp = 0; (kp = mprGetNextKey(hash, kp)) != 0; ) {
        len += strlen(kp->key) + 2 + strlen(kp->data) + 1;
    }
    if ((headers = mprAlloc(len + 1)) == 0) {
        return 0;
    }
    for (kp = 0, cp = headers; (kp = mprGetNextKey(hash, kp)) != 0; ) {
        strcpy(cp, kp->key);
        cp += strlen(cp);
        *cp++ = ':';
        *cp++ = ' ';
        strcpy(cp, kp->data);
        cp += strlen(cp);
        *cp++ = '\n';
    }
    *cp = '\0';
    return headers;
}


PUBLIC char *httpGetHeaders(HttpConn *conn)
{
    return httpGetHeadersFromHash(conn->rx->headers);
}


PUBLIC MprHash *httpGetHeaderHash(HttpConn *conn)
{
    if (conn->rx == 0) {
        assert(conn->rx);
        return 0;
    }
    return conn->rx->headers;
}


PUBLIC cchar *httpGetQueryString(HttpConn *conn)
{
    return (conn->rx && conn->rx->parsedUri) ? conn->rx->parsedUri->query : 0;
}


PUBLIC int httpGetStatus(HttpConn *conn)
{
    return (conn->rx) ? conn->rx->status : 0;
}


PUBLIC char *httpGetStatusMessage(HttpConn *conn)
{
    return (conn->rx) ? conn->rx->statusMessage : 0;
}


PUBLIC void httpSetMethod(HttpConn *conn, cchar *method)
{
    conn->rx->method = sclone(method);
    parseMethod(conn);
}


static int setParsedUri(HttpConn *conn)
{
    HttpRx      *rx;
    char        *cp;
    cchar       *hostname;

    rx = conn->rx;
    if (httpSetUri(conn, rx->uri) < 0 || rx->pathInfo[0] != '/') {
        httpError(conn, HTTP_ABORT | HTTP_CODE_BAD_REQUEST, "Bad URL");
        return MPR_ERR_BAD_ARGS;
    }
    /*
        Complete the URI based on the connection state.
        Must have a complete scheme, host, port and path.
     */
    rx->parsedUri->scheme = sclone(conn->secure ? "https" : "http");
    hostname = rx->hostHeader ? rx->hostHeader : conn->host->name;
    if (!hostname) {
        hostname = conn->sock->acceptIp;
    }
    rx->parsedUri->host = sclone(hostname);
    if ((cp = strchr(rx->parsedUri->host, ':')) != 0) {
        *cp = '\0';
    }
    rx->parsedUri->port = conn->sock->listenSock->port;
    return 0;
}


PUBLIC int httpSetUri(HttpConn *conn, cchar *uri)
{
    HttpRx      *rx;
    char        *pathInfo;

    rx = conn->rx;
    if ((rx->parsedUri = httpCreateUri(uri, 0)) == 0) {
        return MPR_ERR_BAD_ARGS;
    }
    pathInfo = httpNormalizeUriPath(mprUriDecode(rx->parsedUri->path));
    if (pathInfo[0] != '/') {
        return MPR_ERR_BAD_ARGS;
    }
    rx->pathInfo = pathInfo;
    rx->uri = rx->parsedUri->path;
    conn->tx->ext = httpGetExt(conn);
    /*
        Start out with no scriptName and the entire URI in the pathInfo. Stages may rewrite.
     */
    rx->scriptName = mprEmptyString();
    return 0;
}


/*
    Wait for the connection to reach a given state.
    @param state Desired state. Set to zero if you want to wait for one I/O event.
    @param timeout Timeout in msec. If timeout is zer, wait forever. If timeout is < 0, use default inactivity 
        and duration timeouts.
 */
PUBLIC int httpWait(HttpConn *conn, int state, MprTicks timeout)
{
    MprTicks    mark, remaining, inactivityTimeout;
    int         saveAsync, justOne, workDone;

    if (state == 0) {
        state = HTTP_STATE_FINALIZED;
        justOne = 1;
    } else {
        justOne = 0;
    }
    if (conn->state <= HTTP_STATE_BEGIN) {
        assert(conn->state >= HTTP_STATE_BEGIN);
        return MPR_ERR_BAD_STATE;
    } 
    if (conn->input && httpGetPacketLength(conn->input) > 0) {
        httpPumpRequest(conn, conn->input);
    }
    assert(conn->sock);
    if (conn->error || !conn->sock) {
        return MPR_ERR_BAD_STATE;
    }
    mark = mprGetTicks();
    if (mprGetDebugMode()) {
        inactivityTimeout = timeout = MPR_MAX_TIMEOUT;
    } else {
        inactivityTimeout = timeout < 0 ? conn->limits->inactivityTimeout : MPR_MAX_TIMEOUT;
        if (timeout < 0) {
            timeout = conn->limits->requestTimeout;
        }
    }
    saveAsync = conn->async;
    conn->async = 1;

    if (conn->state < state) {
        httpEnableConnEvents(conn);
    }
    remaining = timeout;
    do {
        workDone = httpServiceQueues(conn);
        if (conn->state < state) {
            mprWaitForEvent(conn->dispatcher, min(inactivityTimeout, remaining));
        }
        if (conn->sock && mprIsSocketEof(conn->sock) && !workDone) {
            break;
        }
        remaining = mprGetRemainingTicks(mark, timeout);
    } while (!justOne && !conn->error && conn->state < state && remaining > 0);

    conn->async = saveAsync;
    if (conn->sock == 0 || conn->error) {
        return MPR_ERR_CANT_CONNECT;
    }
    if (!justOne && conn->state < state) {
        return (remaining <= 0) ? MPR_ERR_TIMEOUT : MPR_ERR_CANT_READ;
    }
    return 0;
}


/*  
    Set the connector as write blocked and can't proceed.
 */
PUBLIC void httpSocketBlocked(HttpConn *conn)
{
    mprTrace(7, "Socket full, waiting to drain.");
    conn->tx->writeBlocked = 1;
}


static void addMatchEtag(HttpConn *conn, char *etag)
{
    HttpRx   *rx;

    rx = conn->rx;
    if (rx->etags == 0) {
        rx->etags = mprCreateList(-1, 0);
    }
    mprAddItem(rx->etags, sclone(etag));
}


/*
    Get the next input token. The content buffer is advanced to the next token. This routine always returns a
    non-zero token. The empty string means the delimiter was not found. The delimiter is a string to match and not
    a set of characters. If null, it means use white space (space or tab) as a delimiter. 
 */
static char *getToken(HttpConn *conn, cchar *delim)
{
    MprBuf  *buf;
    char    *token, *endToken, *nextToken;

    buf = conn->input->content;
    token = mprGetBufStart(buf);
    nextToken = mprGetBufEnd(buf);
    for (token = mprGetBufStart(buf); (*token == ' ' || *token == '\t') && token < mprGetBufEnd(buf); token++) {}

    if (delim == 0) {
        delim = " \t";
        if ((endToken = strpbrk(token, delim)) != 0) {
            nextToken = endToken + strspn(endToken, delim);
            *endToken = '\0';
        }
    } else {
        if ((endToken = strstr(token, delim)) != 0) {
            *endToken = '\0';
            /* Only eat one occurence of the delimiter */
            nextToken = endToken + strlen(delim);
        }
    }
    buf->start = nextToken;
    return token;
}


/*  
    Match the entity's etag with the client's provided etag.
 */
PUBLIC bool httpMatchEtag(HttpConn *conn, char *requestedEtag)
{
    HttpRx  *rx;
    char    *tag;
    int     next;

    rx = conn->rx;
    if (rx->etags == 0) {
        return 1;
    }
    if (requestedEtag == 0) {
        return 0;
    }
    for (next = 0; (tag = mprGetNextItem(rx->etags, &next)) != 0; ) {
        if (strcmp(tag, requestedEtag) == 0) {
            return (rx->ifMatch) ? 0 : 1;
        }
    }
    return (rx->ifMatch) ? 1 : 0;
}


/*  
    If an IF-MODIFIED-SINCE was specified, then return true if the resource has not been modified. If using
    IF-UNMODIFIED, then return true if the resource was modified.
 */
PUBLIC bool httpMatchModified(HttpConn *conn, MprTime time)
{
    HttpRx   *rx;

    rx = conn->rx;

    if (rx->since == 0) {
        /*  If-Modified or UnModified not supplied. */
        return 1;
    }
    if (rx->ifModified) {
        /*  Return true if the file has not been modified.  */
        return !(time > rx->since);
    } else {
        /*  Return true if the file has been modified.  */
        return (time > rx->since);
    }
}


/*  
    Format is:  Range: bytes=n1-n2,n3-n4,...
    Where n1 is first byte pos and n2 is last byte pos

    Examples:
        Range: bytes=0-49             first 50 bytes
        Range: bytes=50-99,200-249    Two 50 byte ranges from 50 and 200
        Range: bytes=-50              Last 50 bytes
        Range: bytes=1-               Skip first byte then emit the rest

    Return 1 if more ranges, 0 if end of ranges, -1 if bad range.
 */
static bool parseRange(HttpConn *conn, char *value)
{
    HttpTx      *tx;
    HttpRange   *range, *last, *next;
    char        *tok, *ep;

    tx = conn->tx;
    value = sclone(value);
    if (value == 0) {
        return 0;
    }
    /*  
        Step over the "bytes="
     */
    stok(value, "=", &value);

    for (last = 0; value && *value; ) {
        if ((range = mprAllocObj(HttpRange, manageRange)) == 0) {
            return 0;
        }
        /*  
            A range "-7" will set the start to -1 and end to 8
         */
        tok = stok(value, ",", &value);
        if (*tok != '-') {
            range->start = (ssize) stoi(tok);
        } else {
            range->start = -1;
        }
        range->end = -1;

        if ((ep = strchr(tok, '-')) != 0) {
            if (*++ep != '\0') {
                /*
                    End is one beyond the range. Makes the math easier.
                 */
                range->end = (ssize) stoi(ep) + 1;
            }
        }
        if (range->start >= 0 && range->end >= 0) {
            range->len = (int) (range->end - range->start);
        }
        if (last == 0) {
            tx->outputRanges = range;
        } else {
            last->next = range;
        }
        last = range;
    }

    /*  
        Validate ranges
     */
    for (range = tx->outputRanges; range; range = range->next) {
        if (range->end != -1 && range->start >= range->end) {
            return 0;
        }
        if (range->start < 0 && range->end < 0) {
            return 0;
        }
        next = range->next;
        if (range->start < 0 && next) {
            /* This range goes to the end, so can't have another range afterwards */
            return 0;
        }
        if (next) {
            if (range->end < 0) {
                return 0;
            }
            if (next->start >= 0 && range->end > next->start) {
                return 0;
            }
        }
    }
    conn->tx->currentRange = tx->outputRanges;
    return (last) ? 1: 0;
}


PUBLIC void httpSetStageData(HttpConn *conn, cchar *key, cvoid *data)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->requestData == 0) {
        rx->requestData = mprCreateHash(-1, 0);
    }
    mprAddKey(rx->requestData, key, data);
}


PUBLIC cvoid *httpGetStageData(HttpConn *conn, cchar *key)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->requestData == 0) {
        return NULL;
    }
    return mprLookupKey(rx->requestData, key);
}


PUBLIC char *httpGetPathExt(cchar *path)
{
    char    *ep, *ext;

    if ((ext = strrchr(path, '.')) != 0) {
        ext = sclone(++ext);
        for (ep = ext; *ep && isalnum((uchar) *ep); ep++) {
            ;
        }
        *ep = '\0';
    }
    return ext;
}


/*
    Get the request extension. Look first at the URI pathInfo. If no extension, look at the filename if defined.
    Return NULL if no extension.
 */
PUBLIC char *httpGetExt(HttpConn *conn)
{
    HttpRx  *rx;
    char    *ext;

    rx = conn->rx;
    if ((ext = httpGetPathExt(rx->pathInfo)) == 0) {
        if (conn->tx->filename) {
            ext = httpGetPathExt(conn->tx->filename);
        }
    }
    return ext;
}


//  MOB - can this just use the default compare
static int compareLang(char **s1, char **s2)
{
    return scmp(*s1, *s2);
}


PUBLIC HttpLang *httpGetLanguage(HttpConn *conn, MprHash *spoken, cchar *defaultLang)
{
    HttpRx      *rx;
    HttpLang    *lang;
    MprList     *list;
    cchar       *accept;
    char        *nextTok, *tok, *quality, *language;
    int         next;

    rx = conn->rx;
    if (rx->lang) {
        return rx->lang;
    }
    if (spoken == 0) {
        return 0;
    }
    list = mprCreateList(-1, 0);
    if ((accept = httpGetHeader(conn, "Accept-Language")) != 0) {
        for (tok = stok(sclone(accept), ",", &nextTok); tok; tok = stok(nextTok, ",", &nextTok)) {
            language = stok(tok, ";", &quality);
            if (quality == 0) {
                quality = "1";
            }
            mprAddItem(list, sfmt("%03d %s", (int) (atof(quality) * 100), language));
        }
        mprSortList(list, (MprSortProc) compareLang, 0);
        for (next = 0; (language = mprGetNextItem(list, &next)) != 0; ) {
            if ((lang = mprLookupKey(rx->route->languages, &language[4])) != 0) {
                rx->lang = lang;
                return lang;
            }
        }
    }
    if (defaultLang && (lang = mprLookupKey(rx->route->languages, defaultLang)) != 0) {
        rx->lang = lang;
        return lang;
    }
    return 0;
}


/*
    Trim extra path information after the uri extension. This is used by CGI and PHP only. The strategy is to 
    heuristically find the script name in the uri. This is assumed to be the original uri up to and including 
    first path component containing a "." Any path information after that is regarded as extra path.
    WARNING: Extra path is an old, unreliable, CGI specific technique. Do not use directories with embedded periods.
 */
PUBLIC void httpTrimExtraPath(HttpConn *conn)
{
    HttpRx      *rx;
    char        *cp, *extra;
    ssize       len;

    rx = conn->rx;
    if (!(rx->flags & (HTTP_OPTIONS | HTTP_TRACE))) { 
        if ((cp = strchr(rx->pathInfo, '.')) != 0 && (extra = strchr(cp, '/')) != 0) {
            len = extra - rx->pathInfo;
            if (0 < len && len < slen(rx->pathInfo)) {
                rx->extraPath = sclone(&rx->pathInfo[len]);
                rx->pathInfo[len] = '\0';
            }
        }
        if ((cp = strchr(rx->target, '.')) != 0 && (extra = strchr(cp, '/')) != 0) {
            len = extra - rx->target;
            if (0 < len && len < slen(rx->target)) {
                rx->target[len] = '\0';
            }
        }
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/sendConnector.c"
 */
/************************************************************************/

/*
    sendConnector.c -- Send file connector. 

    The Sendfile connector supports the optimized transmission of whole static files. It uses operating system 
    sendfile APIs to eliminate reading the document into user space and multiple socket writes. The send connector 
    is not a general purpose connector. It cannot handle dynamic data or ranged requests. It does support chunked requests.

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/**************************** Forward Declarations ****************************/
#if !BIT_ROM

static void addPacketForSend(HttpQueue *q, HttpPacket *packet);
static void adjustSendVec(HttpQueue *q, MprOff written);
static MprOff buildSendVec(HttpQueue *q);
static void freeSendPackets(HttpQueue *q, MprOff written);
static void sendClose(HttpQueue *q);

/*********************************** Code *************************************/

PUBLIC int httpOpenSendConnector(Http *http)
{
    HttpStage     *stage;

    mprTrace(5, "Open send connector");
    if ((stage = httpCreateConnector(http, "sendConnector", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    stage->open = httpSendOpen;
    stage->close = sendClose;
    stage->outgoingService = httpSendOutgoingService; 
    http->sendConnector = stage;
    return 0;
}


/*  
    Initialize the send connector for a request
 */
PUBLIC void httpSendOpen(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;

    conn = q->conn;
    tx = conn->tx;

    if (tx->connector != conn->http->sendConnector) {
        httpAssignQueue(q, tx->connector, HTTP_QUEUE_TX);
        tx->connector->open(q);
        return;
    }
    if (!(tx->flags & HTTP_TX_NO_BODY)) {
        assert(tx->fileInfo.valid);
        if (tx->fileInfo.size > conn->limits->transmissionBodySize) {
            httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE,
                "Http transmission aborted. File size exceeds max body of %,Ld bytes", conn->limits->transmissionBodySize);
            return;
        }
        tx->file = mprOpenFile(tx->filename, O_RDONLY | O_BINARY, 0);
        if (tx->file == 0) {
            httpError(conn, HTTP_CODE_NOT_FOUND, "Cannot open document: %s, err %d", tx->filename, mprGetError());
        }
    }
}


static void sendClose(HttpQueue *q)
{
    HttpTx  *tx;

    tx = q->conn->tx;
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
}


PUBLIC void httpSendOutgoingService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpTx      *tx;
    MprFile     *file;
    MprOff      written;
    int         errCode;

    conn = q->conn;
    tx = conn->tx;
    conn->lastActivity = conn->http->now;
    assert(conn->sock);

    if (!conn->sock || tx->finalizedConnector) {
        return;
    }
    if (tx->flags & HTTP_TX_NO_BODY) {
        httpDiscardQueueData(q, 1);
    }
    if ((tx->bytesWritten + q->ioCount) > conn->limits->transmissionBodySize) {
        httpError(conn, HTTP_ABORT | HTTP_CODE_REQUEST_TOO_LARGE | ((tx->bytesWritten) ? HTTP_ABORT : 0),
            "Http transmission aborted. Exceeded max body of %,Ld bytes", conn->limits->transmissionBodySize);
        if (tx->bytesWritten) {
            httpFinalizeConnector(conn);
            return;
        }
    }
    if (q->ioIndex == 0) {
        buildSendVec(q);
    }
    /*
        No need to loop around as send file tries to write as much of the file as possible. 
        If not eof, will always have the socket blocked.
     */
    file = q->ioFile ? tx->file : 0;
    written = mprSendFileToSocket(conn->sock, file, q->ioPos, q->ioCount, q->iovec, q->ioIndex, NULL, 0);
    if (written < 0) {
        errCode = mprGetError();
        if (errCode == EAGAIN || errCode == EWOULDBLOCK) {
            /*  Socket full, wait for an I/O event */
            httpSocketBlocked(conn);
        } else {
            if (errCode != EPIPE && errCode != ECONNRESET && errCode != ECONNABORTED && errCode != ENOTCONN) {
                httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "sendConnector: error, errCode %d", errCode);
            } else {
                httpDisconnect(conn);
            }
            httpFinalizeConnector(conn);
        }
    } else if (written > 0) {
        tx->bytesWritten += written;
        freeSendPackets(q, written);
        adjustSendVec(q, written);
    }
    mprTrace(6, "sendConnector: wrote %d, qflags %x", (int) written, q->flags);
    if (q->first && q->first->flags & HTTP_PACKET_END) {
        mprTrace(6, "sendConnector: end of stream. Finalize connector");
        httpFinalizeConnector(conn);
    } else {
        HTTP_NOTIFY(conn, HTTP_EVENT_WRITABLE, 0);
    }
}


/*  
    Build the IO vector. This connector uses the send file API which permits multiple IO blocks to be written with 
    file data. This is used to write transfer the headers and chunk encoding boundaries. Return the count of bytes to 
    be written. Return -1 for EOF.
 */
static MprOff buildSendVec(HttpQueue *q)
{
    HttpPacket  *packet, *prev;

    assert(q->ioIndex == 0);
    q->ioCount = 0;
    q->ioFile = 0;

    /*  
        Examine each packet and accumulate as many packets into the I/O vector as possible. Can only have one data packet at
        a time due to the limitations of the sendfile API (on Linux). And the data packet must be after all the 
        vector entries. Leave the packets on the queue for now, they are removed after the IO is complete for the 
        entire packet.
     */
    for (packet = prev = q->first; packet && !(packet->flags & HTTP_PACKET_END); packet = packet->next) {
        if (packet->flags & HTTP_PACKET_HEADER) {
            httpWriteHeaders(q, packet);
        }
        if (q->ioFile || q->ioIndex >= (BIT_MAX_IOVEC - 2)) {
            /* Only one file entry allowed */
            break;
        }
        if (packet->prefix || packet->esize || httpGetPacketLength(packet) > 0) {
            addPacketForSend(q, packet);
        } else {
            /* Remove empty packets */
            prev->next = packet->next;
            continue;
        }
        prev = packet;
    }
    return q->ioCount;
}


/*  
    Add one entry to the io vector
 */
static void addToSendVector(HttpQueue *q, char *ptr, ssize bytes)
{
    assert(ptr > 0);
    assert(bytes > 0);

    q->iovec[q->ioIndex].start = ptr;
    q->iovec[q->ioIndex].len = bytes;
    q->ioCount += bytes;
    q->ioIndex++;
}


/*  
    Add a packet to the io vector. Return the number of bytes added to the vector.
 */
static void addPacketForSend(HttpQueue *q, HttpPacket *packet)
{
    HttpTx      *tx;
    HttpConn    *conn;
    int         item;

    conn = q->conn;
    tx = conn->tx;
    
    assert(q->count >= 0);
    assert(q->ioIndex < (BIT_MAX_IOVEC - 2));

    if (packet->prefix) {
        addToSendVector(q, mprGetBufStart(packet->prefix), mprGetBufLength(packet->prefix));
    }
    if (packet->esize > 0) {
        assert(q->ioFile == 0);
        q->ioFile = 1;
        q->ioCount += packet->esize;

    } else if (httpGetPacketLength(packet) > 0) {
        /*
            Header packets have actual content. File data packets are virtual and only have a count.
         */
        addToSendVector(q, mprGetBufStart(packet->content), httpGetPacketLength(packet));
        item = (packet->flags & HTTP_PACKET_HEADER) ? HTTP_TRACE_HEADER : HTTP_TRACE_BODY;
        if (httpShouldTrace(conn, HTTP_TRACE_TX, item, tx->ext) >= 0) {
            httpTraceContent(conn, HTTP_TRACE_TX, item, packet, 0, tx->bytesWritten);
        }
    }
}


static void freeSendPackets(HttpQueue *q, MprOff bytes)
{
    HttpPacket  *packet;
    ssize       len;

    assert(q->first);
    assert(q->count >= 0);
    assert(bytes >= 0);

    /*
        Loop while data to be accounted for and we have not hit the end of data packet
        There should be 2-3 packets on the queue. A header packet for the HTTP response headers, an optional
        data packet with packet->esize set to the size of the file, and an end packet with no content.
        Must leave this routine with the end packet still on the queue and all bytes accounted for.
     */
    while ((packet = q->first) != 0 && !(packet->flags & HTTP_PACKET_END) && bytes > 0) {
        if (packet->prefix) {
            len = mprGetBufLength(packet->prefix);
            len = (ssize) min(len, bytes);
            mprAdjustBufStart(packet->prefix, len);
            bytes -= len;
            /* Prefixes don't count in the q->count. No need to adjust */
            if (mprGetBufLength(packet->prefix) == 0) {
                packet->prefix = 0;
            }
        }
        if (packet->esize) {
            len = (ssize) min(packet->esize, bytes);
            packet->esize -= len;
            packet->epos += len;
            bytes -= len;
            assert(packet->esize >= 0);

        } else if ((len = httpGetPacketLength(packet)) > 0) {
            /* Header packets come here */
            len = (ssize) min(len, bytes);
            mprAdjustBufStart(packet->content, len);
            bytes -= len;
            q->count -= len;
            assert(q->count >= 0);
        }
        if (packet->esize == 0 && httpGetPacketLength(packet) == 0) {
            /* Done with this packet - consume it */
            assert(!(packet->flags & HTTP_PACKET_END));
            httpGetPacket(q);
        } else {
            break;
        }
    }
    assert(bytes == 0);
}


/*  
    Clear entries from the IO vector that have actually been transmitted. This supports partial writes due to the socket
    being full. Don't come here if we've seen all the packets and all the data has been completely written. ie. small files
    don't come here.
 */
static void adjustSendVec(HttpQueue *q, MprOff written)
{
    MprIOVec    *iovec;
    ssize       len;
    int         i, j;

    iovec = q->iovec;
    for (i = 0; i < q->ioIndex; i++) {
        len = iovec[i].len;
        if (written < len) {
            iovec[i].start += (ssize) written;
            iovec[i].len -= (ssize) written;
            return;
        }
        written -= len;
        q->ioCount -= len;
        for (j = i + 1; i < q->ioIndex; ) {
            iovec[j++] = iovec[i++];
        }
        q->ioIndex--;
        i--;
    }
    if (written > 0 && q->ioFile) {
        /* All remaining data came from the file */
        q->ioPos += written;
    }
    q->ioIndex = 0;
    q->ioCount = 0;
    q->ioFile = 0;
}


#else
PUBLIC int httpOpenSendConnector(Http *http) { return 0; }
PUBLIC void httpSendOpen(HttpQueue *q) {}
PUBLIC void httpSendOutgoingService(HttpQueue *q) {}
#endif /* !BIT_ROM */

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/session.c"
 */
/************************************************************************/

/**
    httpSession.c - Session data storage

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************** Includes **********************************/



/********************************** Forwards  *********************************/

static char *makeKey(HttpSession *sp, cchar *key);
static char *makeSessionID(HttpConn *conn);
static void manageSession(HttpSession *sp, int flags);

/************************************* Code ***********************************/

PUBLIC HttpSession *httpAllocSession(HttpConn *conn, cchar *id, MprTicks lifespan)
{
    HttpSession *sp;

    assert(conn);
#if UNUSED && FUTURE
    Http        *http;
    http = conn->http;

    //  OPT less contentions mutex
    lock(http);
    if (http->activeSessions >= conn->limits->sessionMax) {
        httpError(conn, HTTP_CODE_SERVICE_UNAVAILABLE,
            "Too many sessions %d/%d", http->activeSessions, conn->limits->sessionMax);
        unlock(http);
        return 0;
    }
    http->activeSessions++;
    unlock(http);
#endif

    if ((sp = mprAllocObj(HttpSession, manageSession)) == 0) {
        return 0;
    }
    mprSetName(sp, "session");
    sp->lifespan = lifespan;
    if (id == 0) {
        id = makeSessionID(conn);
    }
    sp->id = sclone(id);
    sp->cache = conn->http->sessionCache;
    sp->conn = conn;
    return sp;
}


PUBLIC void httpDestroySession(HttpSession *sp)
{
    Http    *http;

    http = MPR->httpService;

    assert(sp);
    //  OPT less contentions mutex
    lock(http);
    http->activeSessions--;
    assert(http->activeSessions >= 0);
    unlock(http);
    sp->id = 0;
}


static void manageSession(HttpSession *sp, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(sp->id);
        mprMark(sp->cache);
    }
}


PUBLIC HttpSession *httpCreateSession(HttpConn *conn)
{
    return httpGetSession(conn, 1);
}


PUBLIC HttpSession *httpGetSession(HttpConn *conn, int create)
{
    HttpRx      *rx;
    char        *id;

    assert(conn);
    rx = conn->rx;
    assert(rx);
    if (rx->session || !conn) {
        return rx->session;
    }
    id = httpGetSessionID(conn);
    if (id || create) {
        rx->session = httpAllocSession(conn, id, conn->limits->sessionTimeout);
        if (rx->session && !id) {
            httpSetCookie(conn, HTTP_SESSION_COOKIE, rx->session->id, "/", NULL, 0, 0);
        }
    }
    return rx->session;
}


PUBLIC MprHash *httpGetSessionObj(HttpConn *conn, cchar *key)
{
    cchar   *str;

    if ((str = httpGetSessionVar(conn, key, 0)) != 0 && *str) {
        assert(*str == '{');
        return mprDeserialize(str);
    }
    return 0;
}


PUBLIC cchar *httpGetSessionVar(HttpConn *conn, cchar *key, cchar *defaultValue)
{
    HttpSession  *sp;
    cchar       *result;

    assert(conn);
    assert(key && *key);

    result = 0;
    if ((sp = httpGetSession(conn, 0)) != 0) {
        result = mprReadCache(sp->cache, makeKey(sp, key), 0, 0);
    }
    return result ? result : defaultValue;
}


PUBLIC int httpSetSessionObj(HttpConn *conn, cchar *key, MprHash *obj)
{
    httpSetSessionVar(conn, key, mprSerialize(obj, 0));
    return 0;
}


PUBLIC int httpSetSessionVar(HttpConn *conn, cchar *key, cchar *value)
{
    HttpSession  *sp;

    assert(conn);
    assert(key && *key);
    assert(value);

    if ((sp = httpGetSession(conn, 1)) == 0) {
        return 0;
    }
    if (mprWriteCache(sp->cache, makeKey(sp, key), value, 0, sp->lifespan, 0, MPR_CACHE_SET) == 0) {
        return MPR_ERR_CANT_WRITE;
    }
    return 0;
}


PUBLIC int httpRemoveSessionVar(HttpConn *conn, cchar *key)
{
    HttpSession  *sp;

    assert(conn);
    assert(key && *key);

    if ((sp = httpGetSession(conn, 1)) == 0) {
        return 0;
    }
    return mprRemoveCache(sp->cache, makeKey(sp, key)) ? 0 : MPR_ERR_CANT_FIND;
}


PUBLIC char *httpGetSessionID(HttpConn *conn)
{
    HttpRx  *rx;
    cchar   *cookies, *cookie;
    char    *cp, *value;
    int     quoted;

    assert(conn);
    rx = conn->rx;
    assert(rx);

    if (rx->session) {
        return rx->session->id;
    }
    if (rx->sessionProbed) {
        return 0;
    }
    rx->sessionProbed = 1;
    cookies = httpGetCookies(conn);
    for (cookie = cookies; cookie && (value = strstr(cookie, HTTP_SESSION_COOKIE)) != 0; cookie = value) {
        value += strlen(HTTP_SESSION_COOKIE);
        while (isspace((uchar) *value) || *value == '=') {
            value++;
        }
        quoted = 0;
        if (*value == '"') {
            value++;
            quoted++;
        }
        for (cp = value; *cp; cp++) {
            if (quoted) {
                if (*cp == '"' && cp[-1] != '\\') {
                    break;
                }
            } else {
                if ((*cp == ',' || *cp == ';') && cp[-1] != '\\') {
                    break;
                }
            }
        }
        return snclone(value, cp - value);
    }
    return 0;
}


static char *makeSessionID(HttpConn *conn)
{
    char        idBuf[64];
    static int  nextSession = 0;

    assert(conn);

    /* Thread race here on nextSession++ not critical */
    fmt(idBuf, sizeof(idBuf), "%08x%08x%d", PTOI(conn->data) + PTOI(conn), (int) mprGetTime(), nextSession++);
    return mprGetMD5WithPrefix(idBuf, sizeof(idBuf), "::http.session::");
}


/*
    Make a session cache key. This includes the session cookie, the connection IP address and the variable key
    The IP address is added to prevent hijacking.
    Use ./configure --set sessionWithoutIp=true to create sessions without encoding the client IP
 */
static char *makeKey(HttpSession *sp, cchar *key)
{
#if BIT_SESSION_WITHOUT_IP
    return sfmt("session-%s-%s", sp->id, key);
#else
    return sfmt("session-%s-%s-%s", sp->id, sp->conn->ip, key);
#endif
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/stage.c"
 */
/************************************************************************/

/*
    stage.c -- Stages are the building blocks of the Http request pipeline.

    Stages support the extensible and modular processing of HTTP requests. Handlers are a kind of stage that are the 
    first line processing of a request. Connectors are the last stage in a chain to send/receive data over a network.

    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************* Forwards ***********************************/

static void manageStage(HttpStage *stage, int flags);

/*********************************** Code *************************************/
/*  
    Put packets on the service queue.
 */
static void outgoing(HttpQueue *q, HttpPacket *packet)
{
    int     enableService;

    /*  
        Handlers service routines must only be auto-enabled if in the running state.
     */
    enableService = !(q->stage->flags & HTTP_STAGE_HANDLER) || (q->conn->state >= HTTP_STATE_READY) ? 1 : 0;
    httpPutForService(q, packet, enableService);
}


/*  
    Default incoming data routine.  Simply transfer the data upstream to the next filter or handler.
 */
static void incoming(HttpQueue *q, HttpPacket *packet)
{
    assert(q);
    VERIFY_QUEUE(q);
    assert(packet);
    
    if (q->nextQ->put) {
        httpPutPacketToNext(q, packet);
    } else {
        /* This queue is the last queue in the pipeline */
        if (httpGetPacketLength(packet) > 0) {
            if (packet->flags & HTTP_PACKET_SOLO) {
                httpPutForService(q, packet, HTTP_DELAY_SERVICE);
            } else {
                httpJoinPacketForService(q, packet, 0);
            }
        } else {
            /* Zero length packet means eof */
            httpPutForService(q, packet, HTTP_DELAY_SERVICE);
        }
        HTTP_NOTIFY(q->conn, HTTP_EVENT_READABLE, 0);
    }
}


PUBLIC void httpDefaultOutgoingServiceStage(HttpQueue *q)
{
    HttpPacket    *packet;

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!httpWillNextQueueAcceptPacket(q, packet)) {
            httpPutBackPacket(q, packet);
            return;
        }
        httpPutPacketToNext(q, packet);
    }
}


PUBLIC HttpStage *httpCreateStage(Http *http, cchar *name, int flags, MprModule *module)
{
    HttpStage     *stage;

    assert(http);
    assert(name && *name);

    if ((stage = httpLookupStage(http, name)) != 0) {
        if (!(stage->flags & HTTP_STAGE_UNLOADED)) {
            mprError("Stage %s already exists", name);
            return 0;
        }
    } else if ((stage = mprAllocObj(HttpStage, manageStage)) == 0) {
        return 0;
    }
    stage->flags = flags;
    stage->name = sclone(name);
    stage->incoming = incoming;
    stage->outgoing = outgoing;
    stage->outgoingService = httpDefaultOutgoingServiceStage;
    stage->module = module;
    httpAddStage(http, stage);
    return stage;
}


static void manageStage(HttpStage *stage, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(stage->name);
        mprMark(stage->path);
        mprMark(stage->stageData);
        mprMark(stage->module);
        mprMark(stage->extensions);
    }
}


PUBLIC HttpStage *httpCloneStage(Http *http, HttpStage *stage)
{
    HttpStage   *clone;

    if ((clone = mprAllocObj(HttpStage, manageStage)) == 0) {
        return 0;
    }
    *clone = *stage;
    return clone;
}


PUBLIC HttpStage *httpCreateHandler(Http *http, cchar *name, MprModule *module)
{
    return httpCreateStage(http, name, HTTP_STAGE_HANDLER, module);
}


PUBLIC HttpStage *httpCreateFilter(Http *http, cchar *name, MprModule *module)
{
    return httpCreateStage(http, name, HTTP_STAGE_FILTER, module);
}


PUBLIC HttpStage *httpCreateConnector(Http *http, cchar *name, MprModule *module)
{
    return httpCreateStage(http, name, HTTP_STAGE_CONNECTOR, module);
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/trace.c"
 */
/************************************************************************/

/*
    trace.c -- Trace data
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/*********************************** Code *************************************/

PUBLIC void httpSetRouteTraceFilter(HttpRoute *route, int dir, int levels[HTTP_TRACE_MAX_ITEM], ssize len, 
    cchar *include, cchar *exclude)
{
    HttpTrace   *trace;
    char        *word, *tok, *line;
    int         i;

    trace = &route->trace[dir];
    trace->size = len;
    for (i = 0; i < HTTP_TRACE_MAX_ITEM; i++) {
        trace->levels[i] = levels[i];
    }
    if (include && strcmp(include, "*") != 0) {
        trace->include = mprCreateHash(0, 0);
        line = sclone(include);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(trace->include, word, route);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
    if (exclude) {
        trace->exclude = mprCreateHash(0, 0);
        line = sclone(exclude);
        word = stok(line, ", \t\r\n", &tok);
        while (word) {
            if (word[0] == '*' && word[1] == '.') {
                word += 2;
            }
            mprAddKey(trace->exclude, word, route);
            word = stok(NULL, ", \t\r\n", &tok);
        }
    }
}


PUBLIC void httpManageTrace(HttpTrace *trace, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(trace->include);
        mprMark(trace->exclude);
    }
}


PUBLIC void httpInitTrace(HttpTrace *trace)
{
    int     dir;

    assert(trace);

    for (dir = 0; dir < HTTP_TRACE_MAX_DIR; dir++) {
        trace[dir].levels[HTTP_TRACE_CONN] = 3;
        trace[dir].levels[HTTP_TRACE_FIRST] = 2;
        trace[dir].levels[HTTP_TRACE_HEADER] = 3;
        trace[dir].levels[HTTP_TRACE_BODY] = 4;
        trace[dir].levels[HTTP_TRACE_LIMITS] = 5;
        trace[dir].levels[HTTP_TRACE_TIME] = 6;
        trace[dir].size = -1;
    }
}


/*
    If tracing should occur, return the level
 */
PUBLIC int httpShouldTrace(HttpConn *conn, int dir, int item, cchar *ext)
{
    HttpTrace   *trace;

    assert(0 <= dir && dir < HTTP_TRACE_MAX_DIR);
    assert(0 <= item && item < HTTP_TRACE_MAX_ITEM);

    trace = &conn->trace[dir];
    if (trace->disable || trace->levels[item] > MPR->logLevel) {
        return -1;
    }
    if (ext) {
        if (trace->include && !mprLookupKey(trace->include, ext)) {
            trace->disable = 1;
            return -1;
        }
        if (trace->exclude && mprLookupKey(trace->exclude, ext)) {
            trace->disable = 1;
            return -1;
        }
    }
    return trace->levels[item];
}


//  OPT
static void traceBuf(HttpConn *conn, int dir, int level, cchar *msg, cchar *buf, ssize len)
{
    cchar       *start, *cp, *tag, *digits;
    char        *data, *dp;
    static int  txSeq = 0;
    static int  rxSeq = 0;
    int         seqno, i, printable;

    start = buf;
    if (len > 3 && start[0] == (char) 0xef && start[1] == (char) 0xbb && start[2] == (char) 0xbf) {
        /* Step over UTF encoding */
        start += 3;
    }
    for (printable = 1, i = 0; i < len; i++) {
        if (!isprint((uchar) start[i]) && start[i] != '\n' && start[i] != '\r' && start[i] != '\t') {
            printable = 0;
        }
    }
    if (dir == HTTP_TRACE_TX) {
        tag = "Transmit";
        seqno = txSeq++;
    } else {
        tag = "Receive";
        seqno = rxSeq++;
    }
    if (printable) {
        data = mprAlloc(len + 1);
        memcpy(data, start, len);
        data[len] = '\0';
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>>\n%s", tag, msg, seqno, 
            len, conn->seqno, data);
    } else {
        mprRawLog(level, "\n>>>>>>>>>> %s %s packet %d, len %d (conn %d) >>>>>>>>>> (binary)\n", tag, msg, seqno, 
            len, conn->seqno);
        /* To trace binary, must be two levels higher (typically 6) */
        if (MPR->logLevel < (level + 2)) {
            mprRawLog(level, "    Omitted. Display at log level %d\n", level + 2);
        } else {
            data = mprAlloc(len * 3 + ((len / 16) + 1) + 1);
            digits = "0123456789ABCDEF";
            for (i = 0, cp = start, dp = data; cp < &start[len]; cp++) {
                *dp++ = digits[(*cp >> 4) & 0x0f];
                *dp++ = digits[*cp & 0x0f];
                *dp++ = ' ';
                if ((++i % 16) == 0) {
                    *dp++ = '\n';
                }
            }
            *dp++ = '\n';
            *dp = '\0';
            mprRawLog(level, "%s", data);
        }
    }
    mprRawLog(level, "<<<<<<<<<< End %s packet, conn %d\n", tag, conn->seqno);
}


PUBLIC void httpTraceContent(HttpConn *conn, int dir, int item, HttpPacket *packet, ssize len, MprOff total)
{
    HttpTrace   *trace;
    ssize       size;
    int         level;

    trace = &conn->trace[dir];
    level = trace->levels[item];

    if (trace->size >= 0 && total >= trace->size) {
        mprLog(level, "Abbreviating response trace for conn %d", conn->seqno);
        trace->disable = 1;
        return;
    }
    if (len <= 0) {
        len = MAXINT;
    }
    if (packet->prefix) {
        size = mprGetBufLength(packet->prefix);
        size = min(size, len);
        traceBuf(conn, dir, level, "prefix", mprGetBufStart(packet->prefix), size);
    }
    if (packet->content) {
        size = httpGetPacketLength(packet);
        size = min(size, len);
        traceBuf(conn, dir, level, "content", mprGetBufStart(packet->content), size);
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/tx.c"
 */
/************************************************************************/

/*
    tx.c - Http transmitter for server responses and client requests.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/***************************** Forward Declarations ***************************/

static void manageTx(HttpTx *tx, int flags);

/*********************************** Code *************************************/

PUBLIC HttpTx *httpCreateTx(HttpConn *conn, MprHash *headers)
{
    HttpTx      *tx;

    if ((tx = mprAllocObj(HttpTx, manageTx)) == 0) {
        return 0;
    }
    conn->tx = tx;
    tx->conn = conn;
    tx->status = HTTP_CODE_OK;
    tx->length = -1;
    tx->entityLength = -1;
    tx->chunkSize = -1;

    tx->queue[HTTP_QUEUE_TX] = httpCreateQueueHead(conn, "TxHead");
    tx->queue[HTTP_QUEUE_RX] = httpCreateQueueHead(conn, "RxHead");
    conn->readq = tx->queue[HTTP_QUEUE_RX]->prevQ;
    conn->writeq = tx->queue[HTTP_QUEUE_TX]->nextQ;

    if (headers) {
        tx->headers = headers;
    } else if ((tx->headers = mprCreateHash(HTTP_SMALL_HASH_SIZE, MPR_HASH_CASELESS)) != 0) {
        if (!conn->endpoint) {
            httpAddHeaderString(conn, "User-Agent", sclone(BIT_HTTP_SOFTWARE));
        }
    }
    return tx;
}


PUBLIC void httpDestroyTx(HttpTx *tx)
{
    if (tx->file) {
        mprCloseFile(tx->file);
        tx->file = 0;
    }
    if (tx->conn) {
        tx->conn->tx = 0;
        tx->conn = 0;
    }
}


static void manageTx(HttpTx *tx, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(tx->altBody);
        mprMark(tx->cache);
        mprMark(tx->cacheBuffer);
        mprMark(tx->cachedContent);
        mprMark(tx->conn);
        mprMark(tx->connector);
        mprMark(tx->currentRange);
        mprMark(tx->ext);
        mprMark(tx->etag);
        mprMark(tx->errorDocument);
        mprMark(tx->file);
        mprMark(tx->filename);
        mprMark(tx->handler);
        mprMark(tx->headers);
        mprMark(tx->method);
        mprMark(tx->outputPipeline);
        mprMark(tx->outputRanges);
        mprMark(tx->parsedUri);
        mprMark(tx->queue[0]);
        mprMark(tx->queue[1]);
        mprMark(tx->rangeBoundary);
        mprMark(tx->webSockKey);

    } else if (flags & MPR_MANAGE_FREE) {
        httpDestroyTx(tx);
    }
}


/*
    Add key/value to the header hash. If already present, update the value
*/
static void addHdr(HttpConn *conn, cchar *key, cchar *value)
{
    assert(key && *key);
    assert(value);

    mprAddKey(conn->tx->headers, key, value);
}


PUBLIC int httpRemoveHeader(HttpConn *conn, cchar *key)
{
    assert(key && *key);
    if (conn->tx == 0) {
        return MPR_ERR_CANT_ACCESS;
    }
    return mprRemoveKey(conn->tx->headers, key);
}


/*  
    Add a http header if not already defined
 */
PUBLIC void httpAddHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    char        *value;
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);

    if (!mprLookupKey(conn->tx->headers, key)) {
        addHdr(conn, key, value);
    }
}


/*
    Add a header string if not already defined
 */
PUBLIC void httpAddHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    assert(key && *key);
    assert(value);

    if (!mprLookupKey(conn->tx->headers, key)) {
        addHdr(conn, key, sclone(value));
    }
}


/* 
   Append a header. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void httpAppendHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    va_list     vargs;
    char        *value;
    cchar       *oldValue;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);

    oldValue = mprLookupKey(conn->tx->headers, key);
    if (oldValue) {
        /*
            Set-Cookie has legacy behavior and some browsers require separate headers
         */
        if (scaselessmatch(key, "Set-Cookie")) {
            mprAddDuplicateKey(conn->tx->headers, key, value);
        } else {
            addHdr(conn, key, sfmt("%s, %s", oldValue, value));
        }
    } else {
        addHdr(conn, key, value);
    }
}


/* 
   Append a header string. If already defined, the value is catenated to the pre-existing value after a ", " separator.
   As per the HTTP/1.1 spec.
 */
PUBLIC void httpAppendHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    cchar   *oldValue;

    assert(key && *key);
    assert(value && *value);

    oldValue = mprLookupKey(conn->tx->headers, key);
    if (oldValue) {
        if (scaselessmatch(key, "Set-Cookie")) {
            mprAddDuplicateKey(conn->tx->headers, key, sclone(value));
        } else {
            addHdr(conn, key, sfmt("%s, %s", oldValue, value));
        }
    } else {
        addHdr(conn, key, sclone(value));
    }
}


/*  
    Set a http header. Overwrite if present.
 */
PUBLIC void httpSetHeader(HttpConn *conn, cchar *key, cchar *fmt, ...)
{
    char        *value;
    va_list     vargs;

    assert(key && *key);
    assert(fmt && *fmt);

    va_start(vargs, fmt);
    value = sfmtv(fmt, vargs);
    va_end(vargs);
    addHdr(conn, key, value);
}


PUBLIC void httpSetHeaderString(HttpConn *conn, cchar *key, cchar *value)
{
    assert(key && *key);
    assert(value);

    addHdr(conn, key, sclone(value));
}


/*
    Called by connectors (ONLY) when writing the transmission is complete
 */
PUBLIC void httpFinalizeConnector(HttpConn *conn)
{
    HttpTx      *tx;

    tx = conn->tx;
    tx->finalizedConnector = 1;
    tx->finalizedOutput = 1;
    /*
        Use case: server calling finalize in a timer. Must notify for close event in ejs.web/test/request/events.tst
      */ 
    /* Cannot do this if there is still data to read */
    if (tx->finalized && conn->rx->eof) {
        httpSetState(conn, HTTP_STATE_FINALIZED);
    }
}


PUBLIC void httpFinalize(HttpConn *conn)
{
    HttpTx  *tx;

    tx = conn->tx;
    if (!tx || tx->finalized) {
        return;
    }
    tx->finalized = 1;
    if (!tx->finalizedOutput) {
        httpFinalizeOutput(conn);
    } else {
        httpServiceQueues(conn);
    }
}


PUBLIC void httpFinalizeOutput(HttpConn *conn)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (!tx || tx->finalizedOutput) {
        return;
    }
    tx->responded = 1;
    tx->finalizedOutput = 1;
    assert(conn->writeq);
    if (conn->writeq == tx->queue[HTTP_QUEUE_TX]) {
        /* Tx Pipeline not yet created */
        tx->pendingFinalize = 1;
        return;
    }
    assert(conn->state >= HTTP_STATE_CONNECTED);
    /*
        This may be called from httpError when the connection fails.
     */
    if (conn->sock) {
        httpPutForService(conn->writeq, httpCreateEndPacket(), HTTP_SCHEDULE_QUEUE);
        httpServiceQueues(conn);
    }
}


PUBLIC int httpIsFinalized(HttpConn *conn)
{
    return conn->tx->finalized;
}


PUBLIC int httpIsOutputFinalized(HttpConn *conn)
{
    return conn->tx->finalizedOutput;
}


/*
    Flush the write queue
 */
PUBLIC void httpFlush(HttpConn *conn)
{
    httpFlushQueue(conn->writeq, !conn->async);
}


/*
    This formats a response and sets the altBody. The response is not HTML escaped.
    This is the lowest level for formatResponse.
 */
PUBLIC ssize httpFormatResponsev(HttpConn *conn, cchar *fmt, va_list args)
{
    HttpTx      *tx;
    char        *body;

    tx = conn->tx;
    tx->responded = 1;
    body = fmt ? sfmtv(fmt, args) : conn->errorMsg;
    tx->altBody = body;
    tx->length = slen(tx->altBody);
    tx->flags |= HTTP_TX_NO_BODY;
    httpDiscardData(conn, HTTP_QUEUE_TX);
    return (ssize) tx->length;
}


/*
    This formats a response and sets the altBody. The response is not HTML escaped.
 */
PUBLIC ssize httpFormatResponse(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;
    ssize       rc;

    va_start(args, fmt);
    rc = httpFormatResponsev(conn, fmt, args);
    va_end(args);
    return rc;
}


/*
    This formats a complete response. Depending on the Accept header, the response will be either HTML or plain text.
    The response is not HTML escaped. This calls httpFormatResponse.
 */
PUBLIC ssize httpFormatResponseBody(HttpConn *conn, cchar *title, cchar *fmt, ...)
{
    va_list     args;
    char        *msg, *body;

    va_start(args, fmt);
    body = fmt ? sfmtv(fmt, args) : conn->errorMsg;

    if (scmp(conn->rx->accept, "text/plain") == 0) {
        msg = body;
    } else {
        msg = sfmt(
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body>\r\n%s\r\n</body>\r\n</html>\r\n",
            title, body);
    }
    va_end(args);
    return httpFormatResponse(conn, "%s", msg);
}


PUBLIC void *httpGetQueueData(HttpConn *conn)
{
    HttpQueue     *q;

    q = conn->tx->queue[HTTP_QUEUE_TX];
    return q->nextQ->queueData;
}


PUBLIC void httpOmitBody(HttpConn *conn)
{
    HttpTx  *tx;

    tx = conn->tx;
    if (!tx) {
        return;
    }
    tx->flags |= HTTP_TX_NO_BODY;
    tx->length = -1;
    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        /* Connectors will detect this also and disconnect */
    } else {
        httpDiscardData(conn, HTTP_QUEUE_TX);
    }
}


/*  
    Redirect the user to another web page. The targetUri may or may not have a scheme.
 */
PUBLIC void httpRedirect(HttpConn *conn, int status, cchar *targetUri)
{
    HttpTx          *tx;
    HttpRx          *rx;
    HttpUri         *target, *base;
    HttpEndpoint    *endpoint;
    cchar           *msg;
    char            *dir, *cp;

    assert(targetUri);
    rx = conn->rx;
    tx = conn->tx;

    if (tx->finalized) {
        /* A response has already been formulated */
        return;
    }
    tx->status = status;

    if (schr(targetUri, '$')) {
        targetUri = httpExpandUri(conn, targetUri);
    }
    mprLog(3, "redirect %d %s", status, targetUri);
    msg = httpLookupStatus(conn->http, status);

    if (300 <= status && status <= 399) {
        if (targetUri == 0) {
            targetUri = "/";
        }
        target = httpCreateUri(targetUri, 0);
        base = rx->parsedUri;
        /*
            Support URIs without a host:  https:///path. This is used to redirect onto the same host but with a 
            different scheme. So find a suitable local endpoint to supply the port for the scheme.
        */
        if (!target->port &&
                (!target->host || smatch(base->host, target->host)) &&
                (target->scheme && !smatch(target->scheme, base->scheme))) {
            endpoint = smatch(target->scheme, "https") ? conn->host->secureEndpoint : conn->host->defaultEndpoint;
            if (endpoint) {
                target->port = endpoint->port;
            } else {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot find endpoint for scheme %s", target->scheme);
                return;
            }
        }
        if (target->path && target->path[0] != '/') {
            /*
                Relative file redirection to a file in the same directory as the previous request.
             */
            dir = sclone(rx->pathInfo);
            if ((cp = strrchr(dir, '/')) != 0) {
                /* Remove basename */
                *cp = '\0';
            }
            target->path = sjoin(dir, "/", target->path, NULL);
        }
        target = httpCompleteUri(target, base);
        targetUri = httpUriToString(target, 0);
        httpSetHeader(conn, "Location", "%s", targetUri);
        httpFormatResponse(conn, 
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body><h1>%s</h1>\r\n<p>The document has moved <a href=\"%s\">here</a>.</p></body></html>\r\n",
            msg, msg, targetUri);
    } else {
        httpFormatResponse(conn, 
            "<!DOCTYPE html>\r\n"
            "<html><head><title>%s</title></head>\r\n"
            "<body><h1>%s</h1>\r\n</body></html>\r\n",
            msg, msg);
    }
    httpFinalize(conn);
}


PUBLIC void httpSetContentLength(HttpConn *conn, MprOff length)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }
    tx->length = length;
    httpSetHeader(conn, "Content-Length", "%Ld", tx->length);
}


PUBLIC void httpSetCookie(HttpConn *conn, cchar *name, cchar *value, cchar *path, cchar *cookieDomain, 
        MprTicks lifespan, int flags)
{
    HttpRx      *rx;
    char        *cp, *expiresAtt, *expires, *domainAtt, *domain, *secure, *httponly;

    rx = conn->rx;
    if (path == 0) {
        path = "/";
    }
    domain = (char*) cookieDomain;
    if (!domain) {
        domain = sclone(rx->hostHeader);
        if ((cp = strchr(domain, ':')) != 0) {
            *cp = '\0';
        }
        if (*domain && domain[strlen(domain) - 1] == '.') {
            domain[strlen(domain) - 1] = '\0';
        }
    }
    domainAtt = domain ? "; domain=" : "";
    if (domain && !strchr(domain, '.')) {
        if (smatch(domain, "localhost")) {
            domainAtt = domain = "";
        } else {
            domain = sjoin(".", domain, NULL);
        }
    }
    if (lifespan > 0) {
        expiresAtt = "; expires=";
        expires = mprFormatUniversalTime(MPR_HTTP_DATE, mprGetTime() + lifespan);

    } else {
        expires = expiresAtt = "";
    }
    /* 
       Allow multiple cookie headers. Even if the same name. Later definitions take precedence
     */
    secure = (flags & HTTP_COOKIE_SECURE) ? "; secure" : "";
    httponly = (flags & HTTP_COOKIE_HTTP) ?  "; httponly" : "";
    httpAppendHeader(conn, "Set-Cookie", 
        sjoin(name, "=", value, "; path=", path, domainAtt, domain, expiresAtt, expires, secure, httponly, NULL));
    httpAppendHeader(conn, "Cache-Control", "no-cache=\"set-cookie\"");
}


/*  
    Set headers for httpWriteHeaders. This defines standard headers.
 */
static void setHeaders(HttpConn *conn, HttpPacket *packet)
{
    HttpRx      *rx;
    HttpTx      *tx;
    HttpRoute   *route;
    HttpRange   *range;
    MprOff      length;
    cchar       *mimeType;

    assert(packet->flags == HTTP_PACKET_HEADER);

    rx = conn->rx;
    tx = conn->tx;
    route = rx->route;

    /*
        Mandatory headers that must be defined here use httpSetHeader which overwrites existing values. 
     */
    httpAddHeaderString(conn, "Date", conn->http->currentDate);

    if (tx->ext) {
        if ((mimeType = (char*) mprLookupMime(route->mimeTypes, tx->ext)) != 0) {
            if (conn->error) {
                httpAddHeaderString(conn, "Content-Type", "text/html");
            } else {
                httpAddHeaderString(conn, "Content-Type", mimeType);
            }
        }
    }
    if (tx->etag) {
        httpAddHeader(conn, "ETag", "%s", tx->etag);
    }
    length = tx->length > 0 ? tx->length : 0;
    if (rx->flags & HTTP_HEAD) {
        conn->tx->flags |= HTTP_TX_NO_BODY;
        httpDiscardData(conn, HTTP_QUEUE_TX);
        if (tx->chunkSize <= 0) {
            httpAddHeader(conn, "Content-Length", "%Ld", length);
        }

    } else if (tx->length < 0 && tx->chunkSize > 0) {
        httpSetHeaderString(conn, "Transfer-Encoding", "chunked");

    } else if (conn->endpoint) {
        /* Server must not emit a content length header for 1XX, 204 and 304 status */
        if (!((100 <= tx->status && tx->status <= 199) || tx->status == 204 || 
                tx->status == 304 || tx->flags & HTTP_TX_NO_LENGTH)) {
            httpAddHeader(conn, "Content-Length", "%Ld", length);
        }

    } else if (tx->length > 0) {
        /* client with body */
        httpAddHeader(conn, "Content-Length", "%Ld", length);
    }
    if (tx->outputRanges) {
        if (tx->outputRanges->next == 0) {
            range = tx->outputRanges;
            if (tx->entityLength > 0) {
                httpSetHeader(conn, "Content-Range", "bytes %Ld-%Ld/%Ld", range->start, range->end, tx->entityLength);
            } else {
                httpSetHeader(conn, "Content-Range", "bytes %Ld-%Ld/*", range->start, range->end);
            }
        } else {
            httpSetHeader(conn, "Content-Type", "multipart/byteranges; boundary=%s", tx->rangeBoundary);
        }
        httpSetHeader(conn, "Accept-Ranges", "bytes");
    }
    if (conn->endpoint) {
        httpAddHeaderString(conn, "Server", conn->http->software);
        if (--conn->keepAliveCount > 0) {
            httpAddHeaderString(conn, "Connection", "Keep-Alive");
            httpAddHeader(conn, "Keep-Alive", "timeout=%Ld, max=%d", conn->limits->inactivityTimeout / 1000,
                conn->keepAliveCount);
        } else {
            httpAddHeaderString(conn, "Connection", "close");
        }
    }
}


PUBLIC void httpSetEntityLength(HttpConn *conn, int64 len)
{
    HttpTx      *tx;

    tx = conn->tx;
    tx->entityLength = len;
    if (tx->outputRanges == 0) {
        tx->length = len;
    }
}


PUBLIC void httpSetResponded(HttpConn *conn)
{
    conn->tx->responded = 1;
}


PUBLIC void httpSetStatus(HttpConn *conn, int status)
{
    conn->tx->status = status;
    conn->tx->responded = 1;
}


PUBLIC void httpSetContentType(HttpConn *conn, cchar *mimeType)
{
    httpSetHeaderString(conn, "Content-Type", sclone(mimeType));
}


PUBLIC void httpWriteHeaders(HttpQueue *q, HttpPacket *packet)
{
    Http        *http;
    HttpConn    *conn;
    HttpTx      *tx;
    HttpUri     *parsedUri;
    MprKey      *kp;
    MprBuf      *buf;
    int         level;

    assert(packet->flags == HTTP_PACKET_HEADER);

    conn = q->conn;
    http = conn->http;
    tx = conn->tx;
    buf = packet->content;

    if (tx->flags & HTTP_TX_HEADERS_CREATED) {
        return;
    }    
    tx->flags |= HTTP_TX_HEADERS_CREATED;
    tx->responded = 1;
    if (conn->headersCallback) {
        /* Must be before headers below */
        (conn->headersCallback)(conn->headersCallbackArg);
    }
    if (tx->flags & HTTP_TX_USE_OWN_HEADERS && !conn->error) {
        conn->keepAliveCount = -1;
        return;
    }
    setHeaders(conn, packet);

    if (conn->endpoint) {
        mprPutStringToBuf(buf, conn->protocol);
        mprPutCharToBuf(buf, ' ');
        mprPutIntToBuf(buf, tx->status);
        mprPutCharToBuf(buf, ' ');
        mprPutStringToBuf(buf, httpLookupStatus(http, tx->status));
    } else {
        mprPutStringToBuf(buf, tx->method);
        mprPutCharToBuf(buf, ' ');
        parsedUri = tx->parsedUri;
        if (http->proxyHost && *http->proxyHost) {
            if (parsedUri->query && *parsedUri->query) {
                mprPutToBuf(buf, "http://%s:%d%s?%s %s", http->proxyHost, http->proxyPort, 
                    parsedUri->path, parsedUri->query, conn->protocol);
            } else {
                mprPutToBuf(buf, "http://%s:%d%s %s", http->proxyHost, http->proxyPort, parsedUri->path,
                    conn->protocol);
            }
        } else {
            if (parsedUri->query && *parsedUri->query) {
                mprPutToBuf(buf, "%s?%s %s", parsedUri->path, parsedUri->query, conn->protocol);
            } else {
                mprPutStringToBuf(buf, parsedUri->path);
                mprPutCharToBuf(buf, ' ');
                mprPutStringToBuf(buf, conn->protocol);
            }
        }
    }
    if ((level = httpShouldTrace(conn, HTTP_TRACE_TX, HTTP_TRACE_FIRST, tx->ext)) >= mprGetLogLevel(tx)) {
        mprAddNullToBuf(buf);
        mprLog(level, "  %s", mprGetBufStart(buf));
    }
    mprPutStringToBuf(buf, "\r\n");

    /* 
        Output headers
     */
    kp = mprGetFirstKey(conn->tx->headers);
    while (kp) {
        mprPutStringToBuf(packet->content, kp->key);
        mprPutStringToBuf(packet->content, ": ");
        if (kp->data) {
            mprPutStringToBuf(packet->content, kp->data);
        }
        mprPutStringToBuf(packet->content, "\r\n");
        kp = mprGetNextKey(conn->tx->headers, kp);
    }
    /* 
        By omitting the "\r\n" delimiter after the headers, chunks can emit "\r\nSize\r\n" as a single chunk delimiter
     */
    if (tx->length >= 0 || tx->chunkSize <= 0) {
        mprPutStringToBuf(buf, "\r\n");
    }
    if (tx->altBody) {
        /* Error responses are emitted here */
        mprPutStringToBuf(buf, tx->altBody);
        httpDiscardQueueData(tx->queue[HTTP_QUEUE_TX]->nextQ, 0);
    }
    tx->headerSize = mprGetBufLength(buf);
    tx->flags |= HTTP_TX_HEADERS_CREATED;
    q->count += httpGetPacketLength(packet);
}


PUBLIC bool httpFileExists(HttpConn *conn)
{
    HttpTx      *tx;

    tx = conn->tx;
    if (!tx->fileInfo.checked) {
        mprGetPathInfo(tx->filename, &tx->fileInfo);
    }
    return tx->fileInfo.valid;
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/uploadFilter.c"
 */
/************************************************************************/

/*
    uploadFilter.c - Upload file filter.
    The upload filter processes post data according to RFC-1867 ("multipart/form-data" post data). 
    It saves the uploaded files in a configured upload directory.
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************** Includes **********************************/



/*********************************** Locals ***********************************/
/*
    Upload state machine states
 */
#define HTTP_UPLOAD_REQUEST_HEADER        1   /* Request header */
#define HTTP_UPLOAD_BOUNDARY              2   /* Boundary divider */
#define HTTP_UPLOAD_CONTENT_HEADER        3   /* Content part header */
#define HTTP_UPLOAD_CONTENT_DATA          4   /* Content encoded data */
#define HTTP_UPLOAD_CONTENT_END           5   /* End of multipart message */

/*
    Per upload context
 */
typedef struct Upload {
    HttpUploadFile  *currentFile;       /* Current file context */
    MprFile         *file;              /* Current file I/O object */
    char            *boundary;          /* Boundary signature */
    ssize           boundaryLen;        /* Length of boundary */
    int             contentState;       /* Input states */
    char            *clientFilename;    /* Current file filename */
    char            *tmpPath;           /* Current temp filename for upload data */
    char            *id;                /* Current name keyword value */
} Upload;


/********************************** Forwards **********************************/

static void closeUpload(HttpQueue *q);
static char *getBoundary(void *buf, ssize bufLen, void *boundary, ssize boundaryLen);
static void incomingUpload(HttpQueue *q, HttpPacket *packet);
static void manageHttpUploadFile(HttpUploadFile *file, int flags);
static void manageUpload(Upload *up, int flags);
static int matchUpload(HttpConn *conn, HttpRoute *route, int dir);
static void openUpload(HttpQueue *q);
static int  processContentBoundary(HttpQueue *q, char *line);
static int  processContentHeader(HttpQueue *q, char *line);
static int  processContentData(HttpQueue *q);

/************************************* Code ***********************************/

PUBLIC int httpOpenUploadFilter(Http *http)
{
    HttpStage     *filter;

    if ((filter = httpCreateFilter(http, "uploadFilter", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->uploadFilter = filter;
    filter->match = matchUpload; 
    filter->open = openUpload; 
    filter->close = closeUpload; 
    filter->incoming = incomingUpload; 
    return 0;
}


/*  
    Match if this request needs the upload filter. Return true if needed.
 */
static int matchUpload(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpRx  *rx;
    char    *pat;
    ssize   len;
    
    if (!(dir & HTTP_STAGE_RX)) {
        return HTTP_ROUTE_REJECT;
    }
    rx = conn->rx;
    if (!(rx->flags & HTTP_POST) || rx->remainingContent <= 0) {
        return HTTP_ROUTE_REJECT;
    }
    pat = "multipart/form-data";
    len = strlen(pat);
    if (sncaselesscmp(rx->mimeType, pat, len) == 0) {
        rx->upload = 1;
        mprTrace(5, "matchUpload for %s", rx->uri);
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*  
    Initialize the upload filter for a new request
 */
static void openUpload(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    Upload      *up;
    char        *boundary;

    conn = q->conn;
    rx = conn->rx;

    mprTrace(5, "Open upload filter");
    if ((up = mprAllocObj(Upload, manageUpload)) == 0) {
        return;
    }
    q->queueData = up;
    up->contentState = HTTP_UPLOAD_BOUNDARY;
    rx->uploadDir = rx->route->uploadDir;
    rx->autoDelete = rx->route->autoDelete;

    if (rx->uploadDir == 0) {
#if BIT_WIN_LIKE
        rx->uploadDir = mprNormalizePath(getenv("TEMP"));
#else
        rx->uploadDir = sclone("/tmp");
#endif
    }
    mprTrace(5, "Upload directory is %s", rx->uploadDir);

    if ((boundary = strstr(rx->mimeType, "boundary=")) != 0) {
        boundary += 9;
        up->boundary = sjoin("--", boundary, NULL);
        up->boundaryLen = strlen(up->boundary);
    }
    if (up->boundaryLen == 0 || *up->boundary == '\0') {
        httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad boundary");
        return;
    }
    httpSetParam(conn, "UPLOAD_DIR", rx->uploadDir);
}


static void manageUpload(Upload *up, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(up->currentFile);
        mprMark(up->file);
        mprMark(up->boundary);
        mprMark(up->clientFilename);
        mprMark(up->tmpPath);
        mprMark(up->id);
    }
}


/*  
    Cleanup when the entire request has complete
 */
static void closeUpload(HttpQueue *q)
{
    HttpUploadFile  *file;
    HttpRx          *rx;
    Upload          *up;

    rx = q->conn->rx;
    up = q->queueData;
    
    if (up->currentFile) {
        file = up->currentFile;
        file->filename = 0;
    }
    if (rx->autoDelete) {
        httpRemoveAllUploadedFiles(q->conn);
    }
}


/*  
    Incoming data acceptance routine. The service queue is used, but not a service routine as the data is processed
    immediately. Partial data is buffered on the service queue until a correct mime boundary is seen.
 */
static void incomingUpload(HttpQueue *q, HttpPacket *packet)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *content;
    Upload      *up;
    char        *line, *nextTok;
    ssize       count;
    int         done, rc;
    
    assert(packet);
    
    conn = q->conn;
    rx = conn->rx;
    up = q->queueData;
    
    if (httpGetPacketLength(packet) == 0) {
        if (up->contentState != HTTP_UPLOAD_CONTENT_END) {
            httpError(conn, HTTP_CODE_BAD_REQUEST, "Client supplied insufficient upload data");
        }
        httpPutPacketToNext(q, packet);
        return;
    }
    mprTrace(7, "uploadIncomingData: %d bytes", httpGetPacketLength(packet));
    
    /*  
        Put the packet data onto the service queue for buffering. This aggregates input data incase we don't have
        a complete mime record yet.
     */
    httpJoinPacketForService(q, packet, 0);
    
    packet = q->first;
    content = packet->content;
    mprAddNullToBuf(content);
    count = httpGetPacketLength(packet);

    for (done = 0, line = 0; !done; ) {
        if  (up->contentState == HTTP_UPLOAD_BOUNDARY || up->contentState == HTTP_UPLOAD_CONTENT_HEADER) {
            /*
                Parse the next input line
             */
            line = mprGetBufStart(content);
            stok(line, "\n", &nextTok);
            if (nextTok == 0) {
                /* Incomplete line */
                /* done++; */
                break; 
            }
            mprAdjustBufStart(content, (int) (nextTok - line));
            line = strim(line, "\r", MPR_TRIM_END);
        }
        switch (up->contentState) {
        case HTTP_UPLOAD_BOUNDARY:
            if (processContentBoundary(q, line) < 0) {
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_HEADER:
            if (processContentHeader(q, line) < 0) {
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_DATA:
            rc = processContentData(q);
            if (rc < 0) {
                done++;
            }
            if (httpGetPacketLength(packet) < up->boundaryLen) {
                /*  Incomplete boundary - return to get more data */
                done++;
            }
            break;

        case HTTP_UPLOAD_CONTENT_END:
            done++;
            break;
        }
    }
    /*  
        Compact the buffer to prevent memory growth. There is often residual data after the boundary for the next block.
     */
    if (packet != rx->headerPacket) {
        mprCompactBuf(content);
    }
    q->count -= (count - httpGetPacketLength(packet));
    assert(q->count >= 0);

    if (httpGetPacketLength(packet) == 0) {
        /* 
           Quicker to remove the buffer so the packets don't have to be joined the next time 
         */
        httpGetPacket(q);
        assert(q->count >= 0);
    }
}


/*  
    Process the mime boundary division
    Returns  < 0 on a request or state error
            == 0 if successful
 */
static int processContentBoundary(HttpQueue *q, char *line)
{
    HttpConn    *conn;
    Upload      *up;

    conn = q->conn;
    up = q->queueData;

    /*
        Expecting a multipart boundary string
     */
    if (strncmp(up->boundary, line, up->boundaryLen) != 0) {
        httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad upload state. Incomplete boundary");
        return MPR_ERR_BAD_STATE;
    }
    if (line[up->boundaryLen] && strcmp(&line[up->boundaryLen], "--") == 0) {
        up->contentState = HTTP_UPLOAD_CONTENT_END;
    } else {
        up->contentState = HTTP_UPLOAD_CONTENT_HEADER;
    }
    return 0;
}


/*  
    Expecting content headers. A blank line indicates the start of the data.
    Returns  < 0  Request or state error
    Returns == 0  Successfully parsed the input line.
 */
static int processContentHeader(HttpQueue *q, char *line)
{
    HttpConn        *conn;
    HttpRx          *rx;
    HttpUploadFile  *file;
    Upload          *up;
    char            *key, *headerTok, *rest, *nextPair, *value;

    conn = q->conn;
    rx = conn->rx;
    up = q->queueData;
    
    if (line[0] == '\0') {
        up->contentState = HTTP_UPLOAD_CONTENT_DATA;
        return 0;
    }
    mprTrace(7, "Header line: %s", line);

    headerTok = line;
    stok(line, ": ", &rest);

    if (scaselesscmp(headerTok, "Content-Disposition") == 0) {

        /*  
            The content disposition header describes either a form
            variable or an uploaded file.
        
            Content-Disposition: form-data; name="field1"
            >>blank line
            Field Data
            ---boundary
     
            Content-Disposition: form-data; name="field1" ->
                filename="user.file"
            >>blank line
            File data
            ---boundary
         */
        key = rest;
        up->id = up->clientFilename = 0;
        while (key && stok(key, ";\r\n", &nextPair)) {

            key = strim(key, " ", MPR_TRIM_BOTH);
            stok(key, "= ", &value);
            value = strim(value, "\"", MPR_TRIM_BOTH);

            if (scaselesscmp(key, "form-data") == 0) {
                /* Nothing to do */

            } else if (scaselesscmp(key, "name") == 0) {
                up->id = sclone(value);

            } else if (scaselesscmp(key, "filename") == 0) {
                if (up->id == 0) {
                    httpError(conn, HTTP_CODE_BAD_REQUEST, "Bad upload state. Missing name field");
                    return MPR_ERR_BAD_STATE;
                }
                up->clientFilename = sclone(value);
                /*  
                    Create the file to hold the uploaded data
                 */
                up->tmpPath = mprGetTempPath(rx->uploadDir);
                if (up->tmpPath == 0) {
                    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                        "Cannot create upload temp file %s. Check upload temp dir %s", up->tmpPath, rx->uploadDir);
                    return MPR_ERR_CANT_OPEN;
                }
                mprTrace(5, "File upload of: %s stored as %s", up->clientFilename, up->tmpPath);

                up->file = mprOpenFile(up->tmpPath, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, 0600);
                if (up->file == 0) {
                    httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Cannot open upload temp file %s", up->tmpPath);
                    return MPR_ERR_BAD_STATE;
                }
                /*  
                    Create the files[id]
                 */
                file = up->currentFile = mprAllocObj(HttpUploadFile, manageHttpUploadFile);
                file->clientFilename = sclone(up->clientFilename);
                file->filename = sclone(up->tmpPath);
            }
            key = nextPair;
        }

    } else if (scaselesscmp(headerTok, "Content-Type") == 0) {
        if (up->clientFilename) {
            mprTrace(5, "Set files[%s][CONTENT_TYPE] = %s", up->id, rest);
            up->currentFile->contentType = sclone(rest);
        }
    }
    return 0;
}


static void manageHttpUploadFile(HttpUploadFile *file, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(file->filename);
        mprMark(file->clientFilename);
        mprMark(file->contentType);
    }
}


static void defineFileFields(HttpQueue *q, Upload *up)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    char            *key;

    conn = q->conn;
    if (conn->tx->handler == conn->http->ejsHandler) {
        /*  
            Ejscript manages this for itself
         */
        return;
    }
    up = q->queueData;
    file = up->currentFile;
    key = sjoin("FILE_CLIENT_FILENAME_", up->id, NULL);
    httpSetParam(conn, key, file->clientFilename);

    key = sjoin("FILE_CONTENT_TYPE_", up->id, NULL);
    httpSetParam(conn, key, file->contentType);

    key = sjoin("FILE_FILENAME_", up->id, NULL);
    httpSetParam(conn, key, file->filename);

    key = sjoin("FILE_SIZE_", up->id, NULL);
    httpSetIntParam(conn, key, (int) file->size);
}


static int writeToFile(HttpQueue *q, char *data, ssize len)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    HttpLimits      *limits;
    Upload          *up;
    ssize           rc;

    conn = q->conn;
    limits = conn->limits;
    up = q->queueData;
    file = up->currentFile;

    if ((file->size + len) > limits->uploadSize) {
        httpError(conn, HTTP_CODE_REQUEST_TOO_LARGE, "Uploaded file exceeds maximum %,Ld", limits->uploadSize);
        return MPR_ERR_CANT_WRITE;
    }
    if (len > 0) {
        /*  
            File upload. Write the file data.
         */
        rc = mprWriteFile(up->file, data, len);
        if (rc != len) {
            httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, 
                "Cannot write to upload temp file %s, rc %d, errno %d", up->tmpPath, rc, mprGetOsError());
            return MPR_ERR_CANT_WRITE;
        }
        file->size += len;
        mprTrace(7, "uploadFilter: Wrote %d bytes to %s", len, up->tmpPath);
    }
    return 0;
}


/*  
    Process the content data.
    Returns < 0 on error
            == 0 when more data is needed
            == 1 when data successfully written
 */
static int processContentData(HttpQueue *q)
{
    HttpConn        *conn;
    HttpUploadFile  *file;
    HttpPacket      *packet;
    MprBuf          *content;
    Upload          *up;
    ssize           size, dataLen;
    char            *data, *bp, *key;

    conn = q->conn;
    up = q->queueData;
    content = q->first->content;
    file = up->currentFile;
    packet = 0;

    size = mprGetBufLength(content);
    if (size < up->boundaryLen) {
        /*  Incomplete boundary. Return and get more data */
        return 0;
    }
    bp = getBoundary(mprGetBufStart(content), size, up->boundary, up->boundaryLen);
    if (bp == 0) {
        mprTrace(7, "uploadFilter: Got boundary filename %x", up->clientFilename);
        if (up->clientFilename) {
            /*  
                No signature found yet. probably more data to come. Must handle split boundaries.
             */
            data = mprGetBufStart(content);
            dataLen = ((int) (mprGetBufEnd(content) - data)) - (up->boundaryLen - 1);
            if (dataLen > 0 && writeToFile(q, mprGetBufStart(content), dataLen) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            mprAdjustBufStart(content, dataLen);
            return 0;       /* Get more data */
        }
    }
    data = mprGetBufStart(content);
    dataLen = (bp) ? (bp - data) : mprGetBufLength(content);

    if (dataLen > 0) {
        mprAdjustBufStart(content, dataLen);
        /*  
            This is the CRLF before the boundary
         */
        if (dataLen >= 2 && data[dataLen - 2] == '\r' && data[dataLen - 1] == '\n') {
            dataLen -= 2;
        }
        if (up->clientFilename) {
            /*  
                Write the last bit of file data and add to the list of files and define environment variables
             */
            if (writeToFile(q, data, dataLen) < 0) {
                return MPR_ERR_CANT_WRITE;
            }
            httpAddUploadFile(conn, up->id, file);
            defineFileFields(q, up);

        } else {
            /*  
                Normal string form data variables
             */
            data[dataLen] = '\0'; 
            mprLog(3, "uploadFilter: form[%s] = %s", up->id, data);
            key = mprUriDecode(up->id);
            data = mprUriDecode(data);
            httpSetParam(conn, key, data);

            if (packet == 0) {
                packet = httpCreatePacket(BIT_MAX_BUFFER);
            }
            if (httpGetPacketLength(packet) > 0) {
                /*
                    Need to add www-form-urlencoding separators
                 */
                mprPutCharToBuf(packet->content, '&');
            } else {
                conn->rx->mimeType = sclone("application/x-www-form-urlencoded");

            }
            mprPutToBuf(packet->content, "%s=%s", up->id, data);
        }
    }
    if (up->clientFilename) {
        /*  
            Now have all the data (we've seen the boundary)
         */
        mprCloseFile(up->file);
        up->file = 0;
        up->clientFilename = 0;
    }
    if (packet) {
        httpPutPacketToNext(q, packet);
    }
    up->contentState = HTTP_UPLOAD_BOUNDARY;
    return 0;
}


/*  
    Find the boundary signature in memory. Returns pointer to the first match.
 */ 
static char *getBoundary(void *buf, ssize bufLen, void *boundary, ssize boundaryLen)
{
    char    *cp, *endp;
    char    first;

    assert(buf);
    assert(boundary);
    assert(boundaryLen > 0);

    first = *((char*) boundary);
    cp = (char*) buf;

    if (bufLen < boundaryLen) {
        return 0;
    }
    endp = cp + (bufLen - boundaryLen) + 1;
    while (cp < endp) {
        cp = (char *) memchr(cp, first, endp - cp);
        if (!cp) {
            return 0;
        }
        if (memcmp(cp, boundary, boundaryLen) == 0) {
            return cp;
        }
        cp++;
    }
    return 0;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/uri.c"
 */
/************************************************************************/

/*
    uri.c - URI manipulation routines
    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Forwards **********************************/

static int getPort(HttpUri *uri);
static int getDefaultPort(cchar *scheme);
static void manageUri(HttpUri *uri, int flags);
static void trimPathToDirname(HttpUri *uri);

/************************************ Code ************************************/
/*  
    Create and initialize a URI. This accepts full URIs with schemes (http:) and partial URLs
    Support IPv4 and [IPv6]. Supported forms:

        scheme://[::]:PORT/URI
        scheme://HOST:PORT/URI
        [::]:PORT/URI
        :PORT/URI
        HOST:PORT/URI
        PORT/URI
        /URI
        URI

        NOTE: the following is not supported and requires a scheme prefix. This is because it is ambiguous with URI.
        HOST/URI

    Missing fields are null or zero.
 */
PUBLIC HttpUri *httpCreateUri(cchar *uri, int flags)
{
    HttpUri     *up;
    char        *tok, *next;

    assert(uri);

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    tok = up->uri = sclone(uri);

    if (sncmp(up->uri, "http://", 7) == 0) {
        up->scheme = sclone("http");
        if (flags & HTTP_COMPLETE_URI) {
            up->port = 80;
        }
        tok = &up->uri[7];

    } else if (sncmp(up->uri, "ws://", 5) == 0) {
        up->scheme = sclone("ws");
        if (flags & HTTP_COMPLETE_URI) {
            up->port = 80;
        }
        tok = &up->uri[5];
        up->webSockets = 1;

    } else if (sncmp(up->uri, "https://", 8) == 0) {
        up->scheme = sclone("https");
        up->secure = 1;
        if (flags & HTTP_COMPLETE_URI) {
            up->port = 443;
        }
        tok = &up->uri[8];

    } else if (sncmp(up->uri, "wss://", 6) == 0) {
        up->scheme = sclone("wss");
        up->secure = 1;
        if (flags & HTTP_COMPLETE_URI) {
            up->port = 443;
        }
        tok = &up->uri[6];
        up->webSockets = 1;

    } else {
        up->scheme = 0;
        tok = up->uri;
    }
    if (schr(tok, ':')) {
        /* Has port specifier */
        if (*tok == '[' && ((next = strchr(tok, ']')) != 0)) {
            /* IPv6  [::]:port/uri */
            up->host = snclone(&tok[1], (next - tok) - 1);
            if (*++next == ':') {
                up->port = atoi(++next);
            }
            tok = schr(next, '/');

        } else if ((next = spbrk(tok, ":/")) == NULL) {
            /* hostname */
            if (*tok) {
                up->host = sclone(tok);
            }
            tok = 0;

        } else if (*next == ':') {
            /* hostname:port */
            if (next > tok) {
                up->host = snclone(tok, next - tok);
            }
            up->port = atoi(++next);
            tok = schr(next, '/');

        } else if (*next == '/') {
            /* hostname/uri */
            if (next > tok) {
                up->host = snclone(tok, next - tok);
            }
            tok = next;
        }

    } else if (up->scheme && *tok != '/') {
        /* hostname/uri */
        if ((next = schr(tok, '/')) != 0) {
            if (next > tok) {
                up->host = snclone(tok, next - tok);
            }
            tok = next;
        } else {
            /* hostname */
            if (*tok) {
                up->host = sclone(tok);
            }
            tok = 0;
        }
    }
    if (tok) {
        if ((next = spbrk(tok, "#?")) == NULL) {
            if (*tok) {
                up->path = sclone(tok);
            }
        } else {
            if (next > tok) {
                up->path = snclone(tok, next - tok);
            }
            tok = next + 1;
            if (*next == '#') {
                if ((next = schr(tok, '?')) != NULL) {
                    up->reference = snclone(tok, next - tok);
                    up->query = sclone(++next);
                } else {
                    up->reference = sclone(tok);
                }
            } else {
                up->query = sclone(tok);
            }
        }
        if (up->path && (tok = srchr(up->path, '.')) != NULL) {
            if (tok[1]) {
                if ((next = srchr(up->path, '/')) != NULL) {
                    if (next <= tok) {
                        up->ext = sclone(++tok);
                    }
                } else {
                    up->ext = sclone(++tok);
                }
            }
        }
    }
    if (flags & (HTTP_COMPLETE_URI | HTTP_COMPLETE_URI_PATH)) {
        if (up->path == 0 || *up->path == '\0') {
            up->path = sclone("/");
        }
    }
    if (flags & HTTP_COMPLETE_URI) {
        if (!up->scheme) {
            up->scheme = sclone("http");
        }
        if (!up->host) {
            up->host = sclone("localhost");
        }
        if (!up->port) {
            up->port = 80;
        }
    }
    return up;
}


static void manageUri(HttpUri *uri, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(uri->scheme);
        mprMark(uri->host);
        mprMark(uri->path);
        mprMark(uri->ext);
        mprMark(uri->reference);
        mprMark(uri->query);
        mprMark(uri->uri);
    }
}


/*  
    Create and initialize a URI. This accepts full URIs with schemes (http:) and partial URLs
 */
PUBLIC HttpUri *httpCreateUriFromParts(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, cchar *query, 
        int flags)
{
    HttpUri     *up;
    char        *cp, *tok;

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    if (scheme) {
        up->scheme = sclone(scheme);
        up->secure = (smatch(up->scheme, "https") || smatch(up->scheme, "wss"));
        up->webSockets = (smatch(up->scheme, "ws") || smatch(up->scheme, "wss"));
    } else if (flags & HTTP_COMPLETE_URI) {
        up->scheme = "http";
    }
    if (host) {
        if (*host == '[' && ((cp = strchr(host, ']')) != 0)) {
            up->host = snclone(&host[1], (cp - host) - 2);
            if ((cp = schr(++cp, ':')) && port == 0) {
                port = (int) stoi(++cp);
            }
        } else {
            up->host = sclone(host);
            if ((cp = schr(up->host, ':')) && port == 0) {
                port = (int) stoi(++cp);
            }
        }
    } else if (flags & HTTP_COMPLETE_URI) {
        up->host = sclone("localhost");
    }
    if (port) {
        up->port = port;
    }
    if (path) {
        while (path[0] == '/' && path[1] == '/') {
            path++;
        }
        up->path = sclone(path);
    }
    if (flags & (HTTP_COMPLETE_URI | HTTP_COMPLETE_URI_PATH)) {
        if (up->path == 0 || *up->path == '\0') {
            up->path = sclone("/");
        }
    }
    if (reference) {
        up->reference = sclone(reference);
    }
    if (query) {
        up->query = sclone(query);
    }
    if ((tok = srchr(up->path, '.')) != NULL) {
        if ((cp = srchr(up->path, '/')) != NULL) {
            if (cp <= tok) {
                up->ext = sclone(&tok[1]);
            }
        } else {
            up->ext = sclone(&tok[1]);
        }
    }
    return up;
}


PUBLIC HttpUri *httpCloneUri(HttpUri *base, int flags)
{
    HttpUri     *up;
    char        *path, *cp, *tok;

    if ((up = mprAllocObj(HttpUri, manageUri)) == 0) {
        return 0;
    }
    if (base->scheme) {
        up->scheme = sclone(base->scheme);
    } else if (flags & HTTP_COMPLETE_URI) {
        up->scheme = sclone("http");
    }
    up->secure = (smatch(up->scheme, "https") || smatch(up->scheme, "wss"));
    up->webSockets = (smatch(up->scheme, "ws") || smatch(up->scheme, "wss"));
    if (base->host) {
        up->host = sclone(base->host);
    } else if (flags & HTTP_COMPLETE_URI) {
        up->host = sclone("localhost");
    }
    if (base->port) {
        up->port = base->port;
    } else if (flags & HTTP_COMPLETE_URI) {
        up->port = (smatch(up->scheme, "https") || smatch(up->scheme, "wss"))? 443 : 80;
    }
    path = base->path;
    if (path) {
        while (path[0] == '/' && path[1] == '/') {
            path++;
        }
        up->path = sclone(path);
    }
    if (flags & (HTTP_COMPLETE_URI | HTTP_COMPLETE_URI_PATH)) {
        if (up->path == 0 || *up->path == '\0') {
            up->path = sclone("/");
        }
    }
    if (base->reference) {
        up->reference = sclone(base->reference);
    }
    if (base->query) {
        up->query = sclone(base->query);
    }
    if (up->path && (tok = srchr(up->path, '.')) != NULL) {
        if ((cp = srchr(up->path, '/')) != NULL) {
            if (cp <= tok) {
                up->ext = sclone(&tok[1]);
            }
        } else {
            up->ext = sclone(&tok[1]);
        }
    }
    return up;
}


/*
    Complete the "uri" using missing parts from base
 */
PUBLIC HttpUri *httpCompleteUri(HttpUri *uri, HttpUri *base)
{
    if (!base) {
        if (!uri->scheme) {
            uri->scheme = sclone("http");
        }
        if (!uri->host) {
            uri->host = sclone("localhost");
        }
        if (!uri->path) {
            uri->path = sclone("/");
        }
    } else {
        if (!uri->host) {
            uri->host = base->host;
            if (!uri->port) {
                uri->port = base->port;
            }
        }
        if (!uri->scheme) {
            uri->scheme = base->scheme;
        }
        if (!uri->path) {
            uri->path = base->path;
            if (!uri->query) {
                uri->query = base->query;
            }
            if (!uri->reference) {
                uri->reference = base->reference;
            }
        }
    }
    uri->secure = (smatch(uri->scheme, "https") || smatch(uri->scheme, "wss"));
    uri->webSockets = (smatch(uri->scheme, "ws") || smatch(uri->scheme, "wss"));
    return uri;
}


/*  
    Format a string URI from parts
 */
PUBLIC char *httpFormatUri(cchar *scheme, cchar *host, int port, cchar *path, cchar *reference, cchar *query, int flags)
{
    char    *uri;
    cchar   *portStr, *hostDelim, *portDelim, *pathDelim, *queryDelim, *referenceDelim, *cp;

    portDelim = "";
    portStr = "";

    if ((flags & HTTP_COMPLETE_URI) || host || scheme) {
        if (scheme == 0 || *scheme == '\0') {
            scheme = "http";
        }
        if (host == 0 || *host == '\0') {
            host = "localhost";
        }
        hostDelim = "://";
    } else {
        host = hostDelim = "";
    }
    if (host) {
        if (mprIsIPv6(host)) {
            if (*host != '[') {
                host = sfmt("[%s]", host);
            } else if ((cp = scontains(host, "]:")) != 0) {
                port = 0;
            }
        } else if (schr(host, ':')) {
            port = 0;
        }
    }
    if (port != 0 && port != getDefaultPort(scheme)) {
        portStr = itos(port);
        portDelim = ":";
    }
    if (scheme == 0) {
        scheme = "";
    }
    if (path && *path) {
        if (*hostDelim) {
            pathDelim = (*path == '/') ? "" :  "/";
        } else {
            pathDelim = "";
        }
    } else {
        pathDelim = path = "";
    }
    if (reference && *reference) {
        referenceDelim = "#";
    } else {
        referenceDelim = reference = "";
    }
    if (query && *query) {
        queryDelim = "?";
    } else {
        queryDelim = query = "";
    }
    if (portDelim) {
        uri = sjoin(scheme, hostDelim, host, portDelim, portStr, pathDelim, path, referenceDelim, reference, 
            queryDelim, query, NULL);
    } else {
        uri = sjoin(scheme, hostDelim, host, pathDelim, path, referenceDelim, reference, queryDelim, query, NULL);
    }
    return uri;
}


/*
    This returns a URI relative to the base for the given target

    uri = target.relative(base)
 */
PUBLIC HttpUri *httpGetRelativeUri(HttpUri *base, HttpUri *target, int clone)
{
    HttpUri     *uri;
    char        *basePath, *bp, *cp, *tp, *startDiff;
    int         i, baseSegments, commonSegments;

    if (target == 0) {
        return (clone) ? httpCloneUri(base, 0) : base;
    }
    if (!(target->path && target->path[0] == '/') || !((base->path && base->path[0] == '/'))) {
        /* If target is relative, just use it. If base is relative, can't use it because we don't know where it is */
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (base->scheme && target->scheme && scmp(base->scheme, target->scheme) != 0) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (base->host && target->host && (base->host && scmp(base->host, target->host) != 0)) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    if (getPort(base) != getPort(target)) {
        return (clone) ? httpCloneUri(target, 0) : target;
    }
    basePath = httpNormalizeUriPath(base->path);

    /* Count trailing "/" */
    for (baseSegments = 0, bp = basePath; *bp; bp++) {
        if (*bp == '/') {
            baseSegments++;
        }
    }

    /*
        Find portion of path that matches the base, if any.
     */
    commonSegments = 0;
    for (bp = base->path, tp = startDiff = target->path; *bp && *tp; bp++, tp++) {
        if (*bp == '/') {
            if (*tp == '/') {
                commonSegments++;
                startDiff = tp;
            }
        } else {
            if (*bp != *tp) {
                break;
            }
        }
    }
    if (*startDiff == '/') {
        startDiff++;
    }
    
    if ((uri = httpCloneUri(target, 0)) == 0) {
        return 0;
    }
    uri->host = 0;
    uri->scheme = 0;
    uri->port = 0;

    uri->path = cp = mprAlloc(baseSegments * 3 + (int) slen(target->path) + 2);
    for (i = commonSegments; i < baseSegments; i++) {
        *cp++ = '.';
        *cp++ = '.';
        *cp++ = '/';
    }
    if (*startDiff) {
        strcpy(cp, startDiff);
    } else if (cp > uri->path) {
        /*
            Cleanup trailing separators ("../" is the end of the new path)
         */
        cp[-1] = '\0';
    } else {
        strcpy(uri->path, ".");
    }
    return uri;
}


/*
    result = base.join(other)
 */
PUBLIC HttpUri *httpJoinUriPath(HttpUri *result, HttpUri *base, HttpUri *other)
{
    char    *sep;

    if (other->path[0] == '/') {
        result->path = sclone(other->path);
    } else {
        sep = ((base->path[0] == '\0' || base->path[slen(base->path) - 1] == '/') || 
               (other->path[0] == '\0' || other->path[0] == '/'))  ? "" : "/";
        result->path = sjoin(base->path, sep, other->path, NULL);
    }
    return result;
}


PUBLIC HttpUri *httpJoinUri(HttpUri *uri, int argc, HttpUri **others)
{
    HttpUri     *other;
    int         i;

    uri = httpCloneUri(uri, 0);

    for (i = 0; i < argc; i++) {
        other = others[i];
        if (other->scheme) {
            uri->scheme = sclone(other->scheme);
        }
        if (other->host) {
            uri->host = sclone(other->host);
        }
        if (other->port) {
            uri->port = other->port;
        }
        if (other->path) {
            httpJoinUriPath(uri, uri, other);
        }
        if (other->reference) {
            uri->reference = sclone(other->reference);
        }
        if (other->query) {
            uri->query = sclone(other->query);
        }
    }
    uri->ext = mprGetPathExt(uri->path);
    return uri;
}


PUBLIC HttpUri *httpMakeUriLocal(HttpUri *uri)
{
    if (uri) {
        uri->host = 0;
        uri->scheme = 0;
        uri->port = 0;
    }
    return uri;
}


PUBLIC void httpNormalizeUri(HttpUri *uri)
{
    uri->path = httpNormalizeUriPath(uri->path);
}


/*
    Normalize a URI path to remove redundant "./" and cleanup "../" and make separator uniform. Does not make an abs path.
    It does not map separators nor change case. 
 */
PUBLIC char *httpNormalizeUriPath(cchar *pathArg)
{
    char    *dupPath, *path, *sp, *dp, *mark, **segments;
    int     firstc, j, i, nseg, len;

    if (pathArg == 0 || *pathArg == '\0') {
        return mprEmptyString();
    }
    len = (int) slen(pathArg);
    if ((dupPath = mprAlloc(len + 2)) == 0) {
        return NULL;
    }
    strcpy(dupPath, pathArg);

    if ((segments = mprAlloc(sizeof(char*) * (len + 1))) == 0) {
        return NULL;
    }
    nseg = len = 0;
    firstc = *dupPath;
    for (mark = sp = dupPath; *sp; sp++) {
        if (*sp == '/') {
            *sp = '\0';
            while (sp[1] == '/') {
                sp++;
            }
            segments[nseg++] = mark;
            len += (int) (sp - mark);
            mark = sp + 1;
        }
    }
    segments[nseg++] = mark;
    len += (int) (sp - mark);
    for (j = i = 0; i < nseg; i++, j++) {
        sp = segments[i];
        if (sp[0] == '.') {
            if (sp[1] == '\0')  {
                if ((i+1) == nseg) {
                    segments[j] = "";
                } else {
                    j--;
                }
            } else if (sp[1] == '.' && sp[2] == '\0')  {
                if (i == 1 && *segments[0] == '\0') {
                    j = 0;
                } else if ((i+1) == nseg) {
                    if (--j >= 0) {
                        segments[j] = "";
                    }
                } else {
                    j = max(j - 2, -1);
                }
            }
        } else {
            segments[j] = segments[i];
        }
    }
    nseg = j;
    assert(nseg >= 0);
    if ((path = mprAlloc(len + nseg + 1)) != 0) {
        for (i = 0, dp = path; i < nseg; ) {
            strcpy(dp, segments[i]);
            len = (int) slen(segments[i]);
            dp += len;
            if (++i < nseg || (nseg == 1 && *segments[0] == '\0' && firstc == '/')) {
                *dp++ = '/';
            }
        }
        *dp = '\0';
    }
    return path;
}


PUBLIC HttpUri *httpResolveUri(HttpUri *base, int argc, HttpUri **others, bool local)
{
    HttpUri     *current, *other;
    int         i;

    if ((current = httpCloneUri(base, 0)) == 0) {
        return 0;
    }
    if (local) {
        current->host = 0;
        current->scheme = 0;
        current->port = 0;
    }
    /*
        Must not inherit the query or reference
     */
    current->query = NULL;
    current->reference = NULL;

    for (i = 0; i < argc; i++) {
        other = others[i];
        if (other->scheme) {
            current->scheme = sclone(other->scheme);
        }
        if (other->host) {
            current->host = sclone(other->host);
        }
        if (other->port) {
            current->port = other->port;
        }
        if (other->path) {
            trimPathToDirname(current);
            httpJoinUriPath(current, current, other);
        }
        if (other->reference) {
            current->reference = sclone(other->reference);
        }
        if (other->query) {
            current->query = sclone(other->query);
        }
    }
    current->ext = mprGetPathExt(current->path);
    return current;
}


PUBLIC char *httpUriToString(HttpUri *uri, int flags)
{
    return httpFormatUri(uri->scheme, uri->host, uri->port, uri->path, uri->reference, uri->query, flags);
}


static int getPort(HttpUri *uri)
{
    if (uri->port) {
        return uri->port;
    }
    return (uri->scheme && (smatch(uri->scheme, "https") || smatch(uri->scheme, "wss"))) ? 443 : 80;
}


static int getDefaultPort(cchar *scheme)
{
    return (scheme && (smatch(scheme, "https") || smatch(scheme, "wss"))) ? 443 : 80;
}


static void trimPathToDirname(HttpUri *uri) 
{
    char        *path, *cp;
    int         len;

    path = uri->path;
    len = (int) slen(path);
    if (path[len - 1] == '/') {
        if (len > 1) {
            path[len - 1] = '\0';
        }
    } else {
        if ((cp = srchr(path, '/')) != 0) {
            if (cp > path) {
                *cp = '\0';
            } else {
                cp[1] = '\0';
            }
        } else if (*path) {
            path[0] = '\0';
        }
    }
}


/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/var.c"
 */
/************************************************************************/

/*
    var.c -- Manage the request variables
    Copyright (c) All Rights Reserved. See copyright notice at the bottom of the file.
 */

/********************************* Includes ***********************************/



/********************************** Defines ***********************************/

#define HTTP_VAR_HASH_SIZE  61           /* Hash size for vars and params */

/*********************************** Code *************************************/
/*
    Define standard CGI variables
 */
PUBLIC void httpCreateCGIParams(HttpConn *conn)
{
    HttpRx          *rx;
    HttpTx          *tx;
    HttpHost        *host;
    HttpUploadFile  *up;
    MprSocket       *sock;
    MprHash         *vars, *svars;
    MprKey          *kp;
    int             index;

    rx = conn->rx;
    if ((svars = rx->svars) != 0) {
        /* Do only once */
        return;
    }
    svars = rx->svars = mprCreateHash(HTTP_VAR_HASH_SIZE, 0);
    tx = conn->tx;
    host = conn->host;
    sock = conn->sock;

    mprAddKey(svars, "ROUTE_HOME", rx->route->home);

    mprAddKey(svars, "AUTH_TYPE", conn->authType);
    mprAddKey(svars, "AUTH_USER", conn->username);
    mprAddKey(svars, "AUTH_ACL", MPR->emptyString);
    mprAddKey(svars, "CONTENT_LENGTH", rx->contentLength);
    mprAddKey(svars, "CONTENT_TYPE", rx->mimeType);
    mprAddKey(svars, "DOCUMENTS", rx->route->dir);
    mprAddKey(svars, "GATEWAY_INTERFACE", sclone("CGI/1.1"));
    mprAddKey(svars, "QUERY_STRING", rx->parsedUri->query);
    mprAddKey(svars, "REMOTE_ADDR", conn->ip);
    mprAddKeyFmt(svars, "REMOTE_PORT", "%d", conn->port);

    //  DEPRECATE
    mprAddKey(svars, "DOCUMENT_ROOT", rx->route->dir);
    //  DEPRECATE
    mprAddKey(svars, "SERVER_ROOT", rx->route->home);

    /* Set to the same as AUTH_USER */
    mprAddKey(svars, "REMOTE_USER", conn->username);
    mprAddKey(svars, "REQUEST_METHOD", rx->method);
    mprAddKey(svars, "REQUEST_TRANSPORT", sclone((char*) ((conn->secure) ? "https" : "http")));
    mprAddKey(svars, "SERVER_ADDR", sock->acceptIp);
    mprAddKey(svars, "SERVER_NAME", host->name);
    mprAddKeyFmt(svars, "SERVER_PORT", "%d", sock->acceptPort);
    mprAddKey(svars, "SERVER_PROTOCOL", conn->protocol);
    mprAddKey(svars, "SERVER_SOFTWARE", conn->http->software);
    /*
        For PHP, REQUEST_URI must be the original URI. The SCRIPT_NAME will refer to the new pathInfo
     */
    mprAddKey(svars, "REQUEST_URI", rx->originalUri);
    /*  
        URIs are broken into the following: http://{SERVER_NAME}:{SERVER_PORT}{SCRIPT_NAME}{PATH_INFO} 
        NOTE: Appweb refers to pathInfo as the app relative URI and scriptName as the app address before the pathInfo.
        In CGI|PHP terms, the scriptName is the appweb rx->pathInfo and the PATH_INFO is the extraPath. 
     */
    mprAddKey(svars, "PATH_INFO", rx->extraPath);
    mprAddKeyFmt(svars, "SCRIPT_NAME", "%s%s", rx->scriptName, rx->pathInfo);
    mprAddKey(svars, "SCRIPT_FILENAME", tx->filename);
    if (rx->extraPath) {
        /*  
            Only set PATH_TRANSLATED if extraPath is set (CGI spec) 
         */
        assert(rx->extraPath[0] == '/');
        mprAddKey(svars, "PATH_TRANSLATED", mprNormalizePath(sfmt("%s%s", rx->route->dir, rx->extraPath)));
    }

    if (rx->files) {
        vars = httpGetParams(conn);
        assert(vars);
        for (index = 0, kp = 0; (kp = mprGetNextKey(conn->rx->files, kp)) != 0; index++) {
            up = (HttpUploadFile*) kp->data;
            mprAddKey(vars, sfmt("FILE_%d_FILENAME", index), up->filename);
            mprAddKey(vars, sfmt("FILE_%d_CLIENT_FILENAME", index), up->clientFilename);
            mprAddKey(vars, sfmt("FILE_%d_CONTENT_TYPE", index), up->contentType);
            mprAddKey(vars, sfmt("FILE_%d_NAME", index), kp->key);
            mprAddKeyFmt(vars, sfmt("FILE_%d_SIZE", index), "%d", up->size);
        }
    }
    if (conn->http->envCallback) {
        conn->http->envCallback(conn);
    }
}


/*  
    Add variables to the vars environment store. This comes from the query string and urlencoded post data.
    Make variables for each keyword in a query string. The buffer must be url encoded (ie. key=value&key2=value2..., 
    spaces converted to '+' and all else should be %HEX encoded).
 */
static void addParamsFromBuf(HttpConn *conn, cchar *buf, ssize len)
{
    MprHash     *vars;
    cchar       *oldValue;
    char        *newValue, *decoded, *keyword, *value, *tok;

    assert(conn);
    vars = httpGetParams(conn);
    decoded = mprAlloc(len + 1);
    decoded[len] = '\0';
    memcpy(decoded, buf, len);

    keyword = stok(decoded, "&", &tok);
    while (keyword != 0) {
        if ((value = strchr(keyword, '=')) != 0) {
            *value++ = '\0';
            value = mprUriDecode(value);
        } else {
            value = "";
        }
        keyword = mprUriDecode(keyword);

        if (*keyword) {
            /*  
                Append to existing keywords
             */
            oldValue = mprLookupKey(vars, keyword);
            if (oldValue != 0 && *oldValue) {
                if (*value) {
                    newValue = sjoin(oldValue, " ", value, NULL);
                    mprAddKey(vars, keyword, newValue);
                }
            } else {
                mprAddKey(vars, keyword, sclone(value));
            }
        }
        keyword = stok(0, "&", &tok);
    }
}


static void addParamsFromQueue(HttpQueue *q)
{
    HttpConn    *conn;
    HttpRx      *rx;
    MprBuf      *content;

    assert(q);
    
    conn = q->conn;
    rx = conn->rx;

    if ((rx->form || rx->upload) && q->first && q->first->content) {
        httpJoinPackets(q, -1);
        content = q->first->content;
        mprAddNullToBuf(content);
        mprTrace(6, "Form body data: length %d, \"%s\"", mprGetBufLength(content), mprGetBufStart(content));
        addParamsFromBuf(conn, mprGetBufStart(content), mprGetBufLength(content));
    }
}


static void addQueryParams(HttpConn *conn) 
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->parsedUri->query && !(rx->flags & HTTP_ADDED_QUERY_PARAMS)) {
        addParamsFromBuf(conn, rx->parsedUri->query, slen(rx->parsedUri->query));
        rx->flags |= HTTP_ADDED_QUERY_PARAMS;
    }
}


static void addBodyParams(HttpConn *conn)
{
    HttpRx      *rx;

    rx = conn->rx;
    if (rx->form) {
        if (!(rx->flags & HTTP_ADDED_FORM_PARAMS)) {
            addParamsFromQueue(conn->readq);
            rx->flags |= HTTP_ADDED_FORM_PARAMS;
        }
    }
}


PUBLIC void httpAddParams(HttpConn *conn)
{
    addQueryParams(conn);
    addBodyParams(conn);
}


PUBLIC MprHash *httpGetParams(HttpConn *conn)
{ 
    if (conn->rx->params == 0) {
        conn->rx->params = mprCreateHash(HTTP_VAR_HASH_SIZE, 0);
    }
    return conn->rx->params;
}


PUBLIC int httpTestParam(HttpConn *conn, cchar *var)
{
    MprHash    *vars;
    
    vars = httpGetParams(conn);
    return vars && mprLookupKey(vars, var) != 0;
}


PUBLIC cchar *httpGetParam(HttpConn *conn, cchar *var, cchar *defaultValue)
{
    MprHash     *vars;
    cchar       *value;
    
    vars = httpGetParams(conn);
    value = mprLookupKey(vars, var);
    return (value) ? value : defaultValue;
}


PUBLIC int httpGetIntParam(HttpConn *conn, cchar *var, int defaultValue)
{
    MprHash     *vars;
    cchar       *value;
    
    vars = httpGetParams(conn);
    value = mprLookupKey(vars, var);
    return (value) ? (int) stoi(value) : defaultValue;
}


static int sortParam(MprKey **h1, MprKey **h2)
{
    return scmp((*h1)->key, (*h2)->key);
}


/*
    Return the request parameters as a string. 
    This will return the exact same string regardless of the order of form parameters.
 */
PUBLIC char *httpGetParamsString(HttpConn *conn)
{
    HttpRx      *rx;
    MprHash     *params;
    MprKey      *kp;
    MprList     *list;
    char        *buf, *cp;
    ssize       len;
    int         next;

    assert(conn);

    rx = conn->rx;

    if (rx->paramString == 0) {
        if ((params = conn->rx->params) != 0) {
            if ((list = mprCreateList(mprGetHashLength(params), 0)) != 0) {
                len = 0;
                for (kp = 0; (kp = mprGetNextKey(params, kp)) != NULL; ) {
                    mprAddItem(list, kp);
                    len += slen(kp->key) + slen(kp->data) + 2;
                }
                if ((buf = mprAlloc(len + 1)) != 0) {
                    mprSortList(list, (MprSortProc) sortParam, 0);
                    cp = buf;
                    for (next = 0; (kp = mprGetNextItem(list, &next)) != 0; ) {
                        strcpy(cp, kp->key); cp += slen(kp->key);
                        *cp++ = '=';
                        strcpy(cp, kp->data); cp += slen(kp->data);
                        *cp++ = '&';
                    }
                    cp[-1] = '\0';
                    rx->paramString = buf;
                }
            }
        }
    }
    return rx->paramString;
}


PUBLIC void httpSetParam(HttpConn *conn, cchar *var, cchar *value) 
{
    MprHash     *vars;

    vars = httpGetParams(conn);
    mprAddKey(vars, var, sclone(value));
}


PUBLIC void httpSetIntParam(HttpConn *conn, cchar *var, int value) 
{
    MprHash     *vars;
    
    vars = httpGetParams(conn);
    mprAddKey(vars, var, sfmt("%d", value));
}


PUBLIC bool httpMatchParam(HttpConn *conn, cchar *var, cchar *value)
{
    if (strcmp(value, httpGetParam(conn, var, " __UNDEF__ ")) == 0) {
        return 1;
    }
    return 0;
}


PUBLIC void httpAddUploadFile(HttpConn *conn, cchar *id, HttpUploadFile *upfile)
{
    HttpRx   *rx;

    rx = conn->rx;
    if (rx->files == 0) {
        rx->files = mprCreateHash(-1, 0);
    }
    mprAddKey(rx->files, id, upfile);
}


PUBLIC void httpRemoveUploadFile(HttpConn *conn, cchar *id)
{
    HttpRx    *rx;
    HttpUploadFile  *upfile;

    rx = conn->rx;

    upfile = (HttpUploadFile*) mprLookupKey(rx->files, id);
    if (upfile) {
        mprDeletePath(upfile->filename);
        upfile->filename = 0;
    }
}


PUBLIC void httpRemoveAllUploadedFiles(HttpConn *conn)
{
    HttpRx          *rx;
    HttpUploadFile  *upfile;
    MprKey          *kp;

    rx = conn->rx;

    for (kp = 0; rx->files && (kp = mprGetNextKey(rx->files, kp)) != 0; ) {
        upfile = (HttpUploadFile*) kp->data;
        if (upfile->filename) {
            mprDeletePath(upfile->filename);
            upfile->filename = 0;
        }
    }
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
    Start of file "src/webSock.c"
 */
/************************************************************************/

/*
    webSock.c - WebSockets support

    Copyright (c) All Rights Reserved. See details at the end of the file.
 */

/********************************* Includes ***********************************/



/********************************** Locals ************************************/
/*
    Message frame states
 */
#define WS_BEGIN       0
#define WS_EXT_DATA    1                /* Unused */
#define WS_MSG         2
#define WS_CLOSED      3

#if BIT_MPR_TRACING
static char *codetxt[16] = {
    "continuation", "text", "binary", "reserved", "reserved", "reserved", "reserved", "reserved",
    "close", "ping", "pong", "reserved", "reserved", "reserved", "reserved", "reserved",
};
#endif

/*
    Frame format

     Byte 0          Byte 1          Byte 2          Byte 3
     0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7
    +-+-+-+-+-------+-+-------------+-------------------------------+
    |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    |I|S|S|S|  (4)  |A|     (7)     |             (16/63)           |
    |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    | |1|2|3|       |K|             |                               |
    +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    |     Extended payload length continued, if payload len == 127  |
    + - - - - - - - - - - - - - - - +-------------------------------+
    |                               |Masking-key, if MASK set to 1  |
    +-------------------------------+-------------------------------+
    | Masking-key (continued)       |          Payload Data         |
    +-------------------------------- - - - - - - - - - - - - - - - +
    :                     Payload Data continued ...                :
    + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    |                     Payload Data continued ...                |
    +---------------------------------------------------------------+

    Single message has 
        fin == 1
    Fragmented message has
        fin == 0, opcode != 0
        fin == 0, opcode == 0
        fin == 1, opcode == 0

    Common first byte codes:
        0x9B    Fin | /SET

    NOTE: control frames (opcode >= 8) can be sent between fragmented frames
 */
#define GET_FIN(v)              (((v) >> 7) & 0x1)          /* Final fragment */
#define GET_RSV(v)              (((v) >> 4) & 0x7)          /* Reserved (used for extensions) */
#define GET_CODE(v)             ((v) & 0xf)                 /* Packet opcode */
#define GET_MASK(v)             (((v) >> 7) & 0x1)          /* True if dataMask in frame (client send) */
#define GET_LEN(v)              ((v) & 0x7f)                /* Low order 7 bits of length */

#define SET_FIN(v)              (((v) & 0x1) << 7)
#define SET_MASK(v)             (((v) & 0x1) << 7)
#define SET_CODE(v)             ((v) & 0xf)
#define SET_LEN(len, n)         ((uchar)(((len) >> ((n) * 8)) & 0xff))

/********************************** Forwards **********************************/

static void closeWebSock(HttpQueue *q);
static void incomingWebSockData(HttpQueue *q, HttpPacket *packet);
static void manageWebSocket(HttpWebSocket *ws, int flags);
static int matchWebSock(HttpConn *conn, HttpRoute *route, int dir);
static void openWebSock(HttpQueue *q);
static void outgoingWebSockService(HttpQueue *q);
static void readyWebSock(HttpQueue *q);
static bool validUTF8(cchar *str, ssize len);
static void webSockPing(HttpConn *conn);
static void webSockTimeout(HttpConn *conn);

/*********************************** Code *************************************/
/* 
   WebSocket Filter initialization
 */
PUBLIC int httpOpenWebSockFilter(Http *http)
{
    HttpStage     *filter;

    assert(http);

    mprTrace(5, "Open webSock filter");
    if ((filter = httpCreateFilter(http, "webSocketFilter", NULL)) == 0) {
        return MPR_ERR_CANT_CREATE;
    }
    http->webSocketFilter = filter;
    filter->match = matchWebSock; 
    filter->open = openWebSock; 
    filter->ready = readyWebSock; 
    filter->close = closeWebSock; 
    filter->outgoingService = outgoingWebSockService; 
    filter->incoming = incomingWebSockData; 
    return 0;
}


/*
    Match if the filter is required for this request. This is called twice: once for TX and once for RX. RX first.
 */
static int matchWebSock(HttpConn *conn, HttpRoute *route, int dir)
{
    HttpWebSocket   *ws;
    HttpRx          *rx;
    HttpTx          *tx;
    char            *kind, *tok;
    cchar           *key, *protocols;
    int             version;

    assert(conn);
    assert(route);
    rx = conn->rx;
    tx = conn->tx;
    assert(rx);
    assert(tx);

    if (!conn->endpoint) {
        if (rx->webSocket) {
            return HTTP_ROUTE_OK;
        } else if (tx->parsedUri && tx->parsedUri->webSockets) {
            /* ws:// URI. Client web sockets */
            if ((ws = mprAllocObj(HttpWebSocket, manageWebSocket)) == 0) {
                httpMemoryError(conn);
                return HTTP_ROUTE_OK;
            }
            rx->webSocket = ws;
            ws->state = WS_STATE_CONNECTING;
            return HTTP_ROUTE_OK;
        }
        return HTTP_ROUTE_REJECT;
    }
    if (dir & HTTP_STAGE_TX) {
        return rx->webSocket ? HTTP_ROUTE_OK : HTTP_ROUTE_REJECT;
    }
    if (!rx->upgrade || !scaselessmatch(rx->upgrade, "websocket")) {
        return HTTP_ROUTE_REJECT;
    }
    if (!rx->hostHeader || !smatch(rx->method, "GET")) {
        return HTTP_ROUTE_REJECT;
    }
    version = (int) stoi(httpGetHeader(conn, "sec-websocket-version"));
    if (version < WS_VERSION) {
        httpSetHeader(conn, "Sec-WebSocket-Version", "%d", WS_VERSION);
        httpError(conn, HTTP_CLOSE | HTTP_CODE_BAD_REQUEST, "Unsupported Sec-WebSocket-Version");
        return HTTP_ROUTE_OK;
    }
    if ((key = httpGetHeader(conn, "sec-websocket-key")) == 0) {
        httpError(conn, HTTP_CLOSE | HTTP_CODE_BAD_REQUEST, "Bad Sec-WebSocket-Key");
        return HTTP_ROUTE_OK;
    }
    protocols = httpGetHeader(conn, "sec-websocket-protocol");

    if (dir & HTTP_STAGE_RX) {
        if ((ws = mprAllocObj(HttpWebSocket, manageWebSocket)) == 0) {
            httpMemoryError(conn);
            return HTTP_ROUTE_OK;
        }
        rx->webSocket = ws;
        ws->state = WS_STATE_OPEN;
        /* Just select the first protocol */
        if (route->webSocketsProtocol) {
            for (kind = stok(sclone(protocols), " \t,", &tok); kind; kind = stok(NULL, " \t,", &tok)) {
                if (smatch(route->webSocketsProtocol, kind)) {
                    break;
                }
            }
            if (!kind) {
                httpError(conn, HTTP_CLOSE | HTTP_CODE_BAD_REQUEST, "Unsupported Sec-WebSocket-Protocol");
                return HTTP_ROUTE_OK;
            }
            ws->subProtocol = sclone(kind);
        } else {
            /* Just pick the first protocol */
            ws->subProtocol = stok(sclone(protocols), " ,", NULL);
        }
        httpSetStatus(conn, HTTP_CODE_SWITCHING);
        httpSetHeader(conn, "Connection", "Upgrade");
        httpSetHeader(conn, "Upgrade", "WebSocket");
        httpSetHeader(conn, "Sec-WebSocket-Accept", mprGetSHABase64(sjoin(key, WS_MAGIC, NULL)));
        if (ws->subProtocol && *ws->subProtocol) {
            httpSetHeader(conn, "Sec-WebSocket-Protocol", ws->subProtocol);
        }
        httpSetHeader(conn, "X-Request-Timeout", "%Ld", conn->limits->requestTimeout / MPR_TICKS_PER_SEC);
        httpSetHeader(conn, "X-Inactivity-Timeout", "%Ld", conn->limits->requestTimeout / MPR_TICKS_PER_SEC);

        if (route->webSocketsPingPeriod) {
            ws->pingEvent = mprCreateEvent(conn->dispatcher, "webSocket", route->webSocketsPingPeriod, 
                webSockPing, conn, MPR_EVENT_CONTINUOUS);
        }
        conn->keepAliveCount = -1;
        conn->upgraded = 1;
        rx->eof = 0;
        rx->remainingContent = MAXINT;
        return HTTP_ROUTE_OK;
    }
    return HTTP_ROUTE_REJECT;
}


/*
    Open the filter for a new request
 */
static void openWebSock(HttpQueue *q)
{
    HttpConn        *conn;
    HttpWebSocket   *ws;
    HttpPacket      *packet;

    assert(q);
    conn = q->conn;
    ws = conn->rx->webSocket;
    assert(ws);

    mprTrace(5, "webSocketFilter: Opening a new request ");
    q->packetSize = min(conn->limits->bufferSize, q->max);
    ws->closeStatus = WS_STATUS_NO_STATUS;
    conn->timeoutCallback = webSockTimeout;

    if ((packet = httpGetPacket(conn->writeq)) != 0) {
        assert(packet->flags & HTTP_PACKET_HEADER);
        httpPutForService(q, packet, HTTP_SCHEDULE_QUEUE);
    }
    conn->tx->responded = 0;
}


static void manageWebSocket(HttpWebSocket *ws, int flags)
{
    if (flags & MPR_MANAGE_MARK) {
        mprMark(ws->currentFrame);
        mprMark(ws->currentMessage);
        mprMark(ws->pingEvent);
        mprMark(ws->subProtocol);
        mprMark(ws->closeReason);
    }
}


static void closeWebSock(HttpQueue *q)
{
    HttpWebSocket   *ws;

    if (q->conn && q->conn->rx) {
        ws = q->conn->rx->webSocket;
        assert(ws);
        if (ws && ws->pingEvent) {
            mprRemoveEvent(ws->pingEvent);
            ws->pingEvent = 0;
        }
    }
}


static void readyWebSock(HttpQueue *q)
{
    if (q->conn->endpoint) {
        HTTP_NOTIFY(q->conn, HTTP_EVENT_APP_OPEN, 0);
    }
}


static int processFrame(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpRx          *rx;
    HttpWebSocket   *ws;
    HttpLimits      *limits;
    HttpPacket      *tail;
    MprBuf          *content;
    char            *cp;

    conn = q->conn;
    limits = conn->limits;
    ws = conn->rx->webSocket;
    assert(ws);
    rx = conn->rx;
    assert(packet);
    content = packet->content;
    assert(content);

    mprTrace(4, "webSocketFilter: Process packet type %d, \"%s\", data length %d", packet->type, 
        codetxt[packet->type], mprGetBufLength(content));

    switch (packet->type) {
    case WS_MSG_BINARY:
    case WS_MSG_TEXT:
        if (ws->closing) {
            break;
        }
        if (packet->type == WS_MSG_TEXT && !validUTF8(content->start, mprGetBufLength(content))) {
            if (!rx->route->ignoreEncodingErrors) {
                mprError("webSocketFilter: Text packet has invalid UTF8");
                return WS_STATUS_INVALID_UTF8;
            }
        }
        if (packet->type == WS_MSG_TEXT) {
            mprTrace(5, "webSocketFilter: Text packet \"%s\"", content->start);
        }
        if (ws->currentMessage) {
            httpJoinPacket(ws->currentMessage, packet);
            ws->currentMessage->last = packet->last;
            packet = ws->currentMessage;
        }
        for (tail = 0; packet; packet = tail, tail = 0) {
            if (httpGetPacketLength(packet) > limits->webSocketsPacketSize) {
                tail = httpSplitPacket(packet, limits->webSocketsPacketSize);
                assert(tail);
                packet->last = 0;
            }
            if (packet->last || tail) {
                packet->flags |= HTTP_PACKET_SOLO;
                ws->messageLength += httpGetPacketLength(packet);
                httpPutPacketToNext(q, packet);
                ws->currentMessage = 0;
            } else {
                ws->currentMessage = packet;
                break;
            }
        } 
        break;

    case WS_MSG_CLOSE:
        cp = content->start;
        if (httpGetPacketLength(packet) >= 2) {
            ws->closeStatus = ((uchar) cp[0]) << 8 | (uchar) cp[1];
            if (httpGetPacketLength(packet) >= 4) {
                mprAddNullToBuf(content);
                if (ws->maskOffset >= 0) {
                    for (cp = content->start; cp < content->end; cp++) {
                        *cp = *cp ^ ws->dataMask[ws->maskOffset++ & 0x3];
                    }
                }
                ws->closeReason = sclone(&content->start[2]);
            }
        }
        mprTrace(5, "webSocketFilter: close status %d, reason \"%s\", closing %d", ws->closeStatus, 
                ws->closeReason, ws->closing);
        if (ws->closing) {
            httpDisconnect(conn);
        } else {
            /* Acknowledge the close. Echo the received status */
            httpSendClose(conn, WS_STATUS_OK, NULL);
            rx->eof = 1;
            rx->remainingContent = 0;
        }
        /* Advance from the content state */
        httpSetState(conn, HTTP_STATE_READY);
        ws->state = WS_STATE_CLOSED;
        break;

    case WS_MSG_PING:
        /* Respond with the same content as specified in the ping message */
        httpSendBlock(conn, WS_MSG_PONG, mprGetBufStart(content), mprGetBufLength(content), HTTP_BUFFER);
        break;

    case WS_MSG_PONG:
        /* Do nothing */
        break;

    default:
        mprError("webSocketFilter: Bad message type %d", packet->type);
        ws->state = WS_STATE_CLOSED;
        return WS_STATUS_PROTOCOL_ERROR;
    }
    return 0;
}


static void incomingWebSockData(HttpQueue *q, HttpPacket *packet)
{
    HttpConn        *conn;
    HttpWebSocket   *ws;
    HttpPacket      *tail;
    HttpLimits      *limits;
    MprBuf          *content;
    char            *fp, *cp;
    ssize           len, currentFrameLen, offset, frameLen;
    int             i, error, mask, lenBytes, opcode;

    assert(packet);
    conn = q->conn;
    assert(conn->rx);
    ws = conn->rx->webSocket;
    assert(ws);
    limits = conn->limits;
    VERIFY_QUEUE(q);

    if (packet->flags & HTTP_PACKET_DATA) {
        /*
            The service queue is used to hold data that is yet to be analyzed.
            The ws->currentFrame holds the current frame that is being read from the service queue.
         */
        httpJoinPacketForService(q, packet, 0);
    }
    mprTrace(4, "webSocketFilter: incoming data. State %d, Frame state %d, Length: %d", 
        ws->state, ws->frameState, httpGetPacketLength(packet));

    if (packet->flags & HTTP_PACKET_END) {
        /* EOF packet means the socket has been abortively closed */
        if (ws->state != WS_STATE_CLOSED) {
            ws->closing = 1;
            ws->frameState = WS_CLOSED;
            ws->state = WS_STATE_CLOSED;
            ws->closeStatus = WS_STATUS_COMMS_ERROR;
            HTTP_NOTIFY(conn, HTTP_EVENT_APP_CLOSE, ws->closeStatus);
            httpError(conn, HTTP_ABORT | HTTP_CODE_COMMS_ERROR, "Connection lost");
        }
    }
    while ((packet = httpGetPacket(q)) != 0) {
        content = packet->content;
        error = 0;
        mprTrace(5, "webSocketFilter: incoming data, frame state %d", ws->frameState);
        switch (ws->frameState) {
        case WS_CLOSED:
            if (httpGetPacketLength(packet) > 0) {
                mprTrace(5, "webSocketFilter: closed, ignore incoming packet");
            }
            httpFinalize(conn);
            break;

        case WS_BEGIN:
            if (httpGetPacketLength(packet) < 2) {
                /* Need more data */
                httpPutBackPacket(q, packet);
                return;
            }
            fp = content->start;
            if (GET_RSV(*fp) != 0) {
                error = WS_STATUS_PROTOCOL_ERROR;
                break;
            }
            packet->last = GET_FIN(*fp);
            opcode = GET_CODE(*fp);
            if (opcode) {
                if (opcode > WS_MSG_PONG) {
                    error = WS_STATUS_PROTOCOL_ERROR;
                    break;
                }
                packet->type = opcode;
                if (opcode >= WS_MSG_CONTROL && !packet->last) {
                    /* Control frame, must not be fragmented */
                    error = WS_STATUS_PROTOCOL_ERROR;
                    break;
                }
            }
            fp++;
            len = GET_LEN(*fp);
            mask = GET_MASK(*fp);
            lenBytes = 1;
            if (len == 126) {
                lenBytes += 2;
                len = 0;
            } else if (len == 127) {
                lenBytes += 8;
                len = 0;
            }
            if (httpGetPacketLength(packet) < (lenBytes + (mask * 4))) {
                /* Return if we don't have the required packet control fields */
                httpPutBackPacket(q, packet);
                return;
            }
            fp++;
            while (--lenBytes > 0) {
                len <<= 8;
                len += (uchar) *fp++;
            }
            ws->frameLength = len;
            ws->frameState = WS_MSG;
            ws->maskOffset = mask ? 0 : -1;
            if (mask) {
                for (i = 0; i < 4; i++) {
                    ws->dataMask[i] = *fp++;
                }
            }
            assert(content);
            assert(fp >= content->start);
            mprAdjustBufStart(content, fp - content->start);
            assert(q->count >= 0);
            ws->frameState = WS_MSG;
            mprTrace(5, "webSocketFilter: Begin new packet \"%s\", last %d, mask %d, length %d", codetxt[opcode & 0xf],
                packet->last, mask, len);
            /* Keep packet on queue as we need the packet->type */
            httpPutBackPacket(q, packet);
            if (httpGetPacketLength(packet) == 0) {
                return;
            }
            break;

        case WS_MSG:
            currentFrameLen = httpGetPacketLength(ws->currentFrame);
            len = httpGetPacketLength(packet);
            if ((currentFrameLen + len) > ws->frameLength) {
                /*
                    Split packet if it contains data for the next frame
                 */
                offset = ws->frameLength - currentFrameLen;
                if ((tail = httpSplitPacket(packet, offset)) != 0) {
                    tail->last = 0;
                    tail->type = 0;
                    httpPutBackPacket(q, tail);
                    mprTrace(6, "webSocketFilter: Split data packet, %d/%d", ws->frameLength, httpGetPacketLength(tail));
                    len = httpGetPacketLength(packet);
                }
            }
            if ((currentFrameLen + len) > conn->limits->webSocketsMessageSize) {
                mprError("webSocketFilter: Incoming message is too large %d/%d", len, limits->webSocketsMessageSize);
                error = WS_STATUS_MESSAGE_TOO_LARGE;
                break;
            }
            if (packet->type == WS_MSG_CONT && ws->currentFrame) {
                mprTrace(6, "webSocketFilter: Joining data packet %d/%d", currentFrameLen, len);
                httpJoinPacket(ws->currentFrame, packet);
                packet = ws->currentFrame;
                content = packet->content;
            }
            frameLen = httpGetPacketLength(packet);
            assert(frameLen <= ws->frameLength);
            if (frameLen == ws->frameLength) {
                /*
                    Got a complete frame 
                 */
                assert(packet->type);
                if (ws->maskOffset >= 0) {
                    for (cp = content->start; cp < content->end; cp++) {
                        *cp = *cp ^ ws->dataMask[ws->maskOffset++ & 0x3];
                    }
                } 
                if ((error = processFrame(q, packet)) != 0) {
                    break;
                }
                if (ws->state == WS_STATE_CLOSED) {
                    HTTP_NOTIFY(conn, HTTP_EVENT_APP_CLOSE, ws->closeStatus);
                    httpFinalize(conn);
                    ws->frameState = WS_CLOSED;
                    break;
                }
                ws->currentFrame = 0;
                ws->frameState = WS_BEGIN;
            } else {
                ws->currentFrame = packet;
            }
            break;

#if KEEP
        case WS_EXT_DATA:
            assert(packet);
            mprTrace(5, "webSocketFilter: EXT DATA - RESERVED");
            ws->frameState = WS_MSG;
            break;
#endif

        default:
            error = WS_STATUS_PROTOCOL_ERROR;
            break;
        }
        if (error) {
            /*
                Notify of the error and send a close to the peer. The peer may or may not be still there.
                Want to wait for a possible close response message, so don't finalize here.
             */
            mprError("webSocketFilter: WebSockets error Status %d", error);
            HTTP_NOTIFY(conn, HTTP_EVENT_ERROR, error);
            httpSendClose(conn, error, NULL);
            ws->frameState = WS_CLOSED;
            ws->state = WS_STATE_CLOSED;
            return;
        }
    }
}


/*
    Send a text message. Caller must submit valid UTF8.
    Returns the number of data message bytes written. Should equal the length.
 */
PUBLIC ssize httpSend(HttpConn *conn, cchar *fmt, ...)
{
    va_list     args;
    char        *buf;

    va_start(args, fmt);
    buf = sfmtv(fmt, args);
    va_end(args);
    return httpSendBlock(conn, WS_MSG_TEXT, buf, slen(buf), HTTP_BUFFER);
}


/*
    Send a block of data with the specified message type. Set last to true for the last block of a logical message.
    WARNING: this absorbs all data. The caller should ensure they don't write too much by checking conn->writeq->count.
 */
PUBLIC ssize httpSendBlock(HttpConn *conn, int type, cchar *buf, ssize len, int flags)
{
    HttpPacket  *packet;
    HttpQueue   *q;
    ssize       thisWrite, totalWritten;

    assert(conn);
    assert(buf);

    /*
        Note: we can come here before the handshake is complete. The data is queued and if the connection handshake 
        succeeds, then the data is sent.
     */
    assert(HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_FINALIZED);
    if (!(HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_FINALIZED) || !conn->upgraded) {
        return MPR_ERR_BAD_STATE;
    }
    if (type < 0 || type > WS_MSG_PONG) {
        mprError("webSocketFilter: httpSendBlock: bad message type %d", type);
        return MPR_ERR_BAD_ARGS;
    }
    q = conn->writeq;
    if (flags == 0) {
        flags = HTTP_BUFFER;
    }
    if (len < 0) {
        len = slen(buf);
    }
    if (len > conn->limits->webSocketsMessageSize) {
        mprError("webSocketFilter: Outgoing message is too large %d/%d", len, conn->limits->webSocketsMessageSize);
        return MPR_ERR_WONT_FIT;
    }
    mprTrace(5, "webSocketFilter: Sending message type \"%s\", len %d", codetxt[type & 0xf], len);
    totalWritten = 0;
    do {
        /*
            Break into frames. Note: downstream may also fragment packets.
            The outgoing service routine will convert every packet into a frame.
         */
        thisWrite = min(len, conn->limits->webSocketsFrameSize);
        thisWrite = min(thisWrite, q->packetSize);
        if (flags & (HTTP_BLOCK | HTTP_NON_BLOCK)) {
            thisWrite = min(thisWrite, q->max - q->count);
        }
        if ((packet = httpCreateDataPacket(thisWrite)) == 0) {
            return MPR_ERR_MEMORY;
        }
        packet->type = type;
        if (thisWrite > 0) {
            if (mprPutBlockToBuf(packet->content, buf, thisWrite) != thisWrite) {
                return MPR_ERR_MEMORY;
            }
            len -= thisWrite;
            buf += thisWrite;
            totalWritten += thisWrite;
        }
        packet->last = (len > 0) ? 0 : !(flags & HTTP_MORE);
        httpPutForService(q, packet, HTTP_SCHEDULE_QUEUE);

        if (q->count >= q->max) {
            httpFlushQueue(q, 0);
            if (q->count >= q->max) {
                if (flags & HTTP_NON_BLOCK) {
                    break;
                } else if (flags & HTTP_BLOCK) {
                    while (q->count >= q->max) {
                        assert(conn->limits->inactivityTimeout > 10);
                        mprWaitForEvent(conn->dispatcher, conn->limits->inactivityTimeout);
                    }
                }
            }
        }
    } while (len > 0);
    httpServiceQueues(conn);
    return totalWritten;
}


/*
    The reason string is optional
 */
PUBLIC void httpSendClose(HttpConn *conn, int status, cchar *reason)
{
    HttpWebSocket   *ws;
    char            msg[128];
    ssize           len;

    assert(0 <= status && status <= WS_STATUS_MAX);
    ws = conn->rx->webSocket;
    assert(ws);
    if (ws->closing) {
        return;
    }
    ws->closing = 1;
    ws->state = WS_STATE_CLOSING;

    if (!(HTTP_STATE_CONNECTED <= conn->state && conn->state < HTTP_STATE_FINALIZED) || !conn->upgraded) {
        /* Ignore closes when already finalized or not yet connected */
        return;
    } 
    len = 2;
    if (reason) {
        if (slen(reason) >= 124) {
            reason = "WebSockets reason message was too big";
            mprError(reason);
        }
        len += slen(reason) + 1;
    }
    msg[0] = (status >> 8) & 0xff;
    msg[1] = status & 0xff;
    if (reason) {
        scopy(&msg[2], len - 2, reason);
    }
    mprTrace(5, "webSocketFilter: sendClose, status %d reason \"%s\"", status, reason);
    httpSendBlock(conn, WS_MSG_CLOSE, msg, len, HTTP_BUFFER);
}


/*
    This is the outgoing filter routine. It services packets on the outgoing queue and transforms them into 
    WebSockets frames.
 */
static void outgoingWebSockService(HttpQueue *q)
{
    HttpConn    *conn;
    HttpPacket  *packet;
    char        *ep, *fp, *prefix, dataMask[4];
    ssize       len;
    int         i, mask;

    conn = q->conn;
    mprTrace(6, "webSocketFilter: outgoing service");

    for (packet = httpGetPacket(q); packet; packet = httpGetPacket(q)) {
        if (!(packet->flags & (HTTP_PACKET_END | HTTP_PACKET_HEADER))) {
            httpResizePacket(q, packet, conn->limits->bufferSize);
            if (!httpWillNextQueueAcceptPacket(q, packet)) {
                httpPutBackPacket(q, packet);
                return;
            }
            if (packet->type < 0 || packet->type > WS_MSG_MAX) {
                httpError(conn, HTTP_CODE_INTERNAL_SERVER_ERROR, "Bad WebSocket packet type %d", packet->type);
                break;
            }
            len = httpGetPacketLength(packet);
            packet->prefix = mprCreateBuf(16, 16);
            prefix = packet->prefix->start;
            /*
                Server-side does not mask outgoing data
             */
            mask = conn->endpoint ? 0 : 1;
            *prefix++ = SET_FIN(packet->last) | SET_CODE(packet->type);
            if (len <= 125) {
                *prefix++ = SET_MASK(mask) | SET_LEN(len, 0);
            } else if (len <= 65535) {
                *prefix++ = SET_MASK(mask) | 126;
                *prefix++ = SET_LEN(len, 1);
                *prefix++ = SET_LEN(len, 0);
            } else {
                *prefix++ = SET_MASK(mask) | 127;
                for (i = 7; i >= 0; i--) {
                    *prefix++ = SET_LEN(len, i);
                }
            }
            if (!conn->endpoint) {
                mprGetRandomBytes(dataMask, sizeof(dataMask), 0);
                for (i = 0; i < 4; i++) {
                    *prefix++ = dataMask[i];
                }
                fp = packet->content->start;
                ep = packet->content->end;
                for (i = 0; fp < ep; fp++) {
                    *fp = *fp ^ dataMask[i++ & 0x3];
                }
            }
            *prefix = '\0';
            mprAdjustBufEnd(packet->prefix, prefix - packet->prefix->start);
            mprTrace(6, "webSocketFilter: outgoing service, data packet len %d", httpGetPacketLength(packet));
        }
        httpPutPacketToNext(q, packet);
    }
}


PUBLIC char *httpGetWebSocketCloseReason(HttpConn *conn)
{
    HttpWebSocket   *ws;

    if (!conn || !conn->rx) {
        return 0;
    }
    if ((ws = conn->rx->webSocket) == 0) {
        return 0;
    }
    assert(ws);
    return ws->closeReason;
}


PUBLIC ssize httpGetWebSocketMessageLength(HttpConn *conn)
{
    HttpWebSocket   *ws;

    if (!conn || !conn->rx) {
        return 0;
    }
    if ((ws = conn->rx->webSocket) == 0) {
        return 0;
    }
    assert(ws);
    return ws->messageLength;
}


PUBLIC char *httpGetWebSocketProtocol(HttpConn *conn)
{
    HttpWebSocket   *ws;

    if (!conn || !conn->rx) {
        return 0;
    }
    if ((ws = conn->rx->webSocket) == 0) {
        return 0;
    }
    assert(ws);
    return ws->subProtocol;
}


PUBLIC ssize httpGetWebSocketState(HttpConn *conn)
{
    HttpWebSocket   *ws;

    if (!conn || !conn->rx) {
        return 0;
    }
    if ((ws = conn->rx->webSocket) == 0) {
        return 0;
    }
    assert(ws);
    return ws->state;
}


PUBLIC bool httpWebSocketOrderlyClosed(HttpConn *conn)
{
    HttpWebSocket   *ws;

    if (!conn || !conn->rx) {
        return 0;
    }
    if ((ws = conn->rx->webSocket) == 0) {
        return 0;
    }
    assert(ws);
    return ws->closeStatus != WS_STATUS_COMMS_ERROR;
}


PUBLIC void httpSetWebSocketProtocols(HttpConn *conn, cchar *protocols)
{
    assert(conn);
    assert(protocols && *protocols);
    conn->protocols = sclone(protocols);
}


static bool validUTF8(cchar *str, ssize len)
{
    cuchar      *cp, *end;
    int         nbytes, i;
  
    assert(str);
    cp = (cuchar*) str;
    end = (cuchar*) &str[len];
    for (; cp < end && *cp; cp += nbytes) {
        if (!(*cp & 0x80)) {
            nbytes = 1;
        } else if ((*cp & 0xc0) == 0x80) {
            return 0;
        } else if ((*cp & 0xe0) == 0xc0) {
            nbytes = 2;
        } else if ((*cp & 0xf0) == 0xe0) {
            nbytes = 3;
        } else if ((*cp & 0xf8) == 0xf0) {
            nbytes = 4;
        } else if ((*cp & 0xfc) == 0xf8) {
            nbytes = 5;
        } else if ((*cp & 0xfe) == 0xfc) {
            nbytes = 6;
        } else {
            nbytes = 1;
        }
        for (i = 1; i < nbytes; i++) {
            if ((cp[i] & 0xc0) != 0x80) {
                return 0;
            }
        }
        assert(nbytes >= 1);
    } 
    return 1;
}


static void webSockPing(HttpConn *conn)
{
    assert(conn);
    assert(conn->rx);
    /*
        Send a ping. Optimze by sending no data message with it.
     */
    httpSendBlock(conn, WS_MSG_PING, NULL, 0, HTTP_BUFFER);
}


static void webSockTimeout(HttpConn *conn)
{
    assert(conn);
    httpSendClose(conn, WS_STATUS_POLICY_VIOLATION, "Request timeout");
}


/*
    Upgrade a client socket to use Web Sockets. This is called by the client to request a web sockets upgrade.
 */
PUBLIC int httpUpgradeWebSocket(HttpConn *conn)
{
    HttpTx  *tx;
    char    num[16];

    assert(!conn->endpoint);
    tx = conn->tx;
    mprLog(3, "webSocketFilter: Upgrade socket");
    httpSetStatus(conn, HTTP_CODE_SWITCHING);
    httpSetHeader(conn, "Upgrade", "websocket");
    httpSetHeader(conn, "Connection", "Upgrade");
    mprGetRandomBytes(num, sizeof(num), 0);
    tx->webSockKey = mprEncode64Block(num, sizeof(num));
    httpSetHeader(conn, "Sec-WebSocket-Key", tx->webSockKey);
    httpSetHeader(conn, "Sec-WebSocket-Protocol", conn->protocols ? conn->protocols : "chat");
    httpSetHeader(conn, "Sec-WebSocket-Version", "13");
    httpSetHeader(conn, "X-Request-Timeout", "%Ld", conn->limits->requestTimeout / MPR_TICKS_PER_SEC);
    httpSetHeader(conn, "X-Inactivity-Timeout", "%Ld", conn->limits->requestTimeout / MPR_TICKS_PER_SEC);
    conn->upgraded = 1;
    conn->keepAliveCount = -1;
    conn->rx->remainingContent = MAXINT;
    return 0;
}


/*
    Client verification of the server WebSockets handshake response
 */
PUBLIC bool httpVerifyWebSocketsHandshake(HttpConn *conn)
{
    HttpRx          *rx;
    HttpTx          *tx;
    cchar           *key, *expected;

    assert(!conn->endpoint);
    rx = conn->rx;
    tx = conn->tx;
    assert(rx);
    assert(rx->webSocket);
    assert(conn->upgraded);

    rx->webSocket->state = WS_STATE_CLOSED;

    if (rx->status != HTTP_CODE_SWITCHING) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket handshake status %d", rx->status);
        return 0;
    }
    if (!smatch(httpGetHeader(conn, "connection"), "Upgrade")) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket Connection header");
        return 0;
    }
    if (!smatch(httpGetHeader(conn, "upgrade"), "WebSocket")) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket Upgrade header");
        return 0;
    }
    expected = mprGetSHABase64(sjoin(tx->webSockKey, WS_MAGIC, NULL));
    key = httpGetHeader(conn, "sec-websocket-accept");
    if (!smatch(key, expected)) {
        httpError(conn, HTTP_CODE_BAD_HANDSHAKE, "Bad WebSocket handshake key\n%s\n%s", key, expected);
        return 0;
    }
    rx->webSocket->state = WS_STATE_OPEN;
    mprLog(4, "WebSockets handsake verified");
    return 1;
}

/*
    @copy   default

    Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.

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
