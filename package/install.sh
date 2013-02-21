#!/bin/bash
#
#   install: Installation script
#
#   Copyright (c) Embedthis Software LLC, 2003-2013. All Rights Reserved.
#
#   Usage: install [configFile]
#
################################################################################
#
#   The configFile is of the format:
#       FMT=[rpm|deb|tar]               # Package format to use
#       srcDir=sourcePath               # Where to install the src
#       installbin=[YN]                 # Install binary package
#

HOME=`pwd`
FMT=

HOSTNAME=`hostname`
COMPANY="${settings.company}"
PRODUCT="${settings.product}"
NAME="${settings.title}"
VERSION="${settings.version}"
NUMBER="${settings.buildNumber}"
OS="${platform.os}"
CPU="${platform.arch}"
DIST="${platform.dist}"

BIN_PREFIX="${prefixes.bin}"
INC_PREFIX="${prefixes.inc}"
PRD_PREFIX="${prefixes.productver}"
VER_PREFIX="${prefixes.productver}"

installbin=Y
headless=${HEADLESS:-0}

PATH=$PATH:/sbin:/usr/sbin
unset CDPATH
export CYGWIN=nodosfilewarning

###############################################################################

setup() {
    umask 022
    if [ $OS != windows -a `id -u` != "0" ] ; then
        echo "You must be root to install this product."
        exit 255
    fi

    #
    #   Headless install
    #
    if [ $# -ge 1 ] ; then
        if [ ! -f $1 ] ; then
            echo "Could not find installation config file \"$1\"." 1>&2
            exit 255
        else
            . $1 
            installFiles $FMT
        fi
        exit 0
    fi
    sleuthPackageFormat
    [ "$headless" != 1 ] && echo -e "\n$NAME ${VERSION}-${NUMBER} Installation\n"

}

#
#   Try to guess if we should default to using RPM
#
sleuthPackageFormat() {
    local name

    FMT=
    name=`createPackageName ${PRODUCT}-bin`
    if [ -x contents ] ; then
        FMT=tar
    else
        for f in deb rpm tgz ; do
            if [ -f ${name}.${f} ] ; then
                FMT=${f%.gz}
                break
            fi
        done
    fi
    if [ "$FMT" = "" ] ; then
        echo -e "\nYou may be be missing a necessary package file. "
        echo "Check that you have the correct $NAME package".
        exit 255
    fi
}

askUser() {
    local finished

    #
    #   Confirm the configuration
    #
    finished=N
    while [ "$finished" = "N" ]
    do
        installbin=`yesno "Install binary package" "$installbin"`
    
        if [ "$headless" != 1 ] ; then
            echo -e "\nInstalling with this configuration:" 
            echo -e "    Install binary package: $installbin"
            echo
        fi
        finished=`yesno "Accept this configuration" "Y"`
    done
    
    if [ "$installbin" = "N" ] ; then
        echo -e "\nNothing to install, exiting. "
        exit 0
    fi
}

createPackageName() {
    echo ${1}-${VERSION}-${NUMBER}-${DIST}-${OS}-${CPU}
}

# 
#   Get a yes/no answer from the user. Usage: ans=`yesno "prompt" "default"`
#   Echos 1 for Y or 0 for N
#
yesno() {
    local ans

    if [ "$headless" = 1 ] ; then
        echo "Y"
        return
    fi
    echo -n "$1 [$2] : " 1>&2
    while [ 1 ] 
    do
        read ans
        if [ "$ans" = "" ] ; then
            echo $2 ; break
        elif [ "$ans" = "Y" -o "$ans" = "y" ] ; then
            echo "Y" ; break
        elif [ "$ans" = "N" -o "$ans" = "n" ] ; then
            echo "N" ; break
        fi
        echo -e "\nMust enter a 'y' or 'n'\n " 1>&1
    done
}

# 
#   Get input from the user. Usage: ans=`ask "prompt" "default"`
#   Returns the answer or default if <ENTER> is pressed
#
ask() {
    local ans

    default=$2

    if [ "$headless" = 1 ] ; then
        echo "$default"
        return
    fi

    echo -n "$1 [$default] : " 1>&2
    read ans
    if [ "$ans" = "" ] ; then
        echo $default
    fi
    echo $ans
}

saveSetup() {
    local firstChar

    mkdir -p "$VER_PREFIX"
    echo -e "FMT=$FMT\nbinDir=$VER_PREFIX\ninstallbin=$installbin\n" >"${VER_PREFIX}/install.conf"
}

installFiles() {
    local dir pkg doins NAME upper target

    [ "$headless" != 1 ] && echo -e "\nExtracting files ...\n"

    for pkg in bin ; do
        doins=`eval echo \\$install${pkg}`
        if [ "$doins" = Y ] ; then
            upper=`echo $pkg | tr '[:lower:]' '[:upper:]'`
            suffix="-${pkg}"
            NAME=`createPackageName ${PRODUCT}${suffix}`.$FMT
            if [ "$runDaemon" != "Y" ] ; then
                export APPWEB_DONT_START=1
            fi
            if [ "$FMT" = "rpm" ] ; then
                [ "$headless" != 1 ] && echo -e "rpm -Uhv $NAME"
                rpm -Uhv $HOME/$NAME
            elif [ "$FMT" = "deb" ] ; then
                [ "$headless" != 1 ] && echo -e "dpkg -i $NAME"
                dpkg -i $HOME/$NAME >/dev/null
            elif [ "$FMT" = "tar" ] ; then
                target=/
                [ $OS = windows ] && target=`cygpath ${HOMEDRIVE}/`
                (cd contents ; tar cf - . | (cd $target && tar xBf -))
            fi
        fi
    done

    if [ -f /etc/redhat-release -a -x /usr/bin/chcon ] ; then 
        if sestatus | grep enabled >/dev/nulll ; then
            for f in $BIN_PREFIX/*.so ; do
                chcon /usr/bin/chcon -t texrel_shlib_t $f 2>&1 >/dev/null
            done
        fi
    fi

#   if [ "$OS" = "freebsd" ] ; then
#       LDCONFIG_OPT=-m
#   else
#       LDCONFIG_OPT=-n
#   fi
#   if which ldconfig >/dev/null 2>&1 ; then
#       ldconfig /usr/lib/lib${PRODUCT}.so.?.?.?
#       ldconfig $LDCONFIG_OPT /usr/lib/${PRODUCT}
#       ldconfig $LDCONFIG_OPT /usr/lib/${PRODUCT}/modules
#   fi

    if [ $OS = windows ] ; then
        [ "$headless" != 1 ] && echo -e "\nSetting file permissions ..."
        find "$PRD_PREFIX" -type d -exec chmod 755 {} \;
        find "$PRD_PREFIX" -type f -exec chmod g+r,o+r {} \;
        chmod 755 "$BIN_PREFIX"/*.dll "$BIN_PREFIX"/*.exe
    fi
    [ "$headless" != 1 ] && echo
}

removeOld() {
    if [ -x /usr/lib/bit/bin/uninstall ] ; then
        HEADLESS=1 /usr/lib/bit/bin/uninstall </dev/null 2>&1 >/dev/null
    else 
        for v in `ls $prefix 2>/dev/null | egrep -v '[a-zA-Z@!_\-]' | sort -n -r`
        do
            if [ -x "$prefix/$v/bin/bit" ] ; then
                version=$v
                break
            fi
        done
        if [ -x /usr/lib/bit/bin/$version/uninstall ] ; then
            HEADLESS=1 /usr/lib/bit/bin/$version/uninstall </dev/null 2>&1 >/dev/null
        fi
    fi
}

###############################################################################
#
#   Main program for install script
#

setup $*
askUser
removeOld
saveSetup
installFiles $FMT

[ "$headless" != 1 ] && echo
echo -e "$NAME installation successful."
exit 0
