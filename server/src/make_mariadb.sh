#!/bin/bash
# author: luoning
# date: 03/24/2015

MARIADB_DEVEL=MariaDB-10.0.17-centos6-x86_64-devel
MARIADB_DEVEL_DOWNLOAD_PATH=http://sfo1.mirrors.digitalocean.com/mariadb/mariadb-10.0.17/yum/centos6-amd64/rpms/$MARIADB_DEVEL.rpm
MARIADB_COMMON=MariaDB-10.0.17-centos6-x86_64-common
MARIADB_COMMON_DOWNLOAD_PATH=http://sfo1.mirrors.digitalocean.com/mariadb/mariadb-10.0.17/yum/centos6-amd64/rpms/$MARIADB_COMMON.rpm
MARIADB_COMPAT=MariaDB-10.0.17-centos6-x86_64-compat
MARIADB_COMPAT_DOWNLOAD_PATH=http://sfo1.mirrors.digitalocean.com/mariadb/mariadb-10.0.17/yum/centos6-amd64/rpms/$MARIADB_COMPAT.rpm
CUR_DIR=

check_user() {
    if [ $(id -u) != "0" ]; then
        echo "Error: You must be root to run this script, please use root to install im"
        exit 1
    fi
}

get_cur_dir() {
    # Get the fully qualified path to the script
    case $0 in
        /*)
            SCRIPT="$0"
            ;;
        *)
            PWD_DIR=$(pwd);
            SCRIPT="${PWD_DIR}/$0"
            ;;
    esac
    # Resolve the true real path without any sym links.
    CHANGED=true
    while [ "X$CHANGED" != "X" ]
    do
        # Change spaces to ":" so the tokens can be parsed.
        SAFESCRIPT=`echo $SCRIPT | sed -e 's; ;:;g'`
        # Get the real path to this script, resolving any symbolic links
        TOKENS=`echo $SAFESCRIPT | sed -e 's;/; ;g'`
        REALPATH=
        for C in $TOKENS; do
            # Change any ":" in the token back to a space.
            C=`echo $C | sed -e 's;:; ;g'`
            REALPATH="$REALPATH/$C"
            # If REALPATH is a sym link, resolve it.  Loop for nested links.
            while [ -h "$REALPATH" ] ; do
                LS="`ls -ld "$REALPATH"`"
                LINK="`expr "$LS" : '.*-> \(.*\)$'`"
                if expr "$LINK" : '/.*' > /dev/null; then
                    # LINK is absolute.
                    REALPATH="$LINK"
                else
                    # LINK is relative.
                    REALPATH="`dirname "$REALPATH"`""/$LINK"
                fi
            done
        done

        if [ "$REALPATH" = "$SCRIPT" ]
        then
            CHANGED=""
        else
            SCRIPT="$REALPATH"
        fi
    done
    # Change the current directory to the location of the script
    CUR_DIR=$(dirname "${REALPATH}")
}

download() {
    if [ -f "$1" ]; then
        echo "$1 existed."
    else
        echo "$1 not existed, begin to download..."
        wget $2
        if [ $? -eq 0 ]; then
            echo "download $1 successed";
        else
            echo "Error: download $1 failed";
            return 1;
        fi
    fi
    return 0
}

build_mariadb_devel(){
    CENTOS_VERSION=$(less /etc/redhat-release)
    #echo "$OS_VERSION, $OS_BIT bit..." 
    if [[ $CENTOS_VERSION =~ "7.0" ]]; then
        yum -y install mariadb-devel
    else
        download $MARIADB_DEVEL.rpm $MARIADB_DEVEL_DOWNLOAD_PATH
        if [ $? -eq 1 ]; then
            return 1
        fi

        download $MARIADB_COMMON.rpm $MARIADB_COMMON_DOWNLOAD_PATH
        if [ $? -eq 1 ]; then
            return 1
        fi

        download $MARIADB_COMPAT.rpm $MARIADB_COMPAT_DOWNLOAD_PATH
        if [ $? -eq 1 ]; then
            return 1
        fi

        yum -y install openssl-devel


        rpm -ivh MariaDB-*
        RET=$?
        if [ $RET -eq 0 ]; then
            echo "install mariadb-devel successed";
        elif [ $RET -eq 3 ]; then
            echo "mariadb-devel has installed";
        else
            echo "Error: install mariadb-devel failed";
            return 1;
        fi
    fi

    
}

check_user
get_cur_dir
build_mariadb_devel