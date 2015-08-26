#!/bin/bash
# author: luoning
# date: 03/24/2015

LOG4CXX=apache-log4cxx-0.10.0
LOG4CXX_PATH=http://mirror.bit.edu.cn/apache/logging/log4cxx/0.10.0/$LOG4CXX.tar.gz
CUR_DIR=
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

build_log4cxx(){
    yum -y install apr-devel
    yum -y install apr-util-devel
    cd log4cxx
    download $LOG4CXX.tar.gz $LOG4CXX_PATH
    tar -xf $LOG4CXX.tar.gz
    cd $LOG4CXX
    ./configure --prefix=$CUR_DIR/log4cxx --with-apr=/usr --with-apr-util=/usr
    cp ../inputstreamreader.cpp ./src/main/cpp/
    cp ../socketoutputstream.cpp ./src/main/cpp/
    cp ../console.cpp ./src/examples/cpp/
    make
    make install
    cd ../../
    cp -rf log4cxx/include slog/
    mkdir -p slog/lib/
    cp -f log4cxx/lib/liblog4cxx.so* slog/lib/
}

check_user
get_cur_dir
build_log4cxx
