#!/bin/bash
# this is a setup scripts for gcc and gdb
# author: luoning
# date: 09/11/2014

# setup gcc

VERSION=5.2.0
GCC=gcc-$VERSION
GCC_DOWNLOAD_PATH=http://mirrors.ustc.edu.cn/gnu/gcc/$GCC/$GCC.tar.gz
GDB=gdb-7.9
TERMCAP=termcap-1.3.1
TERMCAP_DOWNLOAD_PATH=ftp://prep.ai.mit.edu/pub/gnu/termcap/$TERMCAP.tar.gz
GDB_DOWNLOAD_PATH=http://ftp.gnu.org/gnu/gdb/$GDB.tar.gz
CUR_DIR=

CPU_NUM=`cat /proc/cpuinfo | grep processor | wc -l`
check_user() {
	if [ $(id -u) != "0" ]; then
    	echo "Error: You must be root to run this script, please use root to install $GCC"
    	exit 1
	fi
}

download() {
	if [ -f "$1" ]; then
		echo "$1 existed."
	else
		echo "$1 not existed, begin to download..."
		wget $2
		if [ $? -eq 0 ]; then
			echo "download $1 successed"
		else
			echo "download $1 failed"
			return 1
		fi
	fi
	return 0
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


build_g++(){
	check_user
	yum -y install gcc-c++
	download $GCC.tar.gz $GCC_DOWNLOAD_PATH
	tar -xf $GCC.tar.gz
	cd $GCC
	./contrib/download_prerequisites
	mkdir -p gcc-build-$VERSION
	cd gcc-build-$VERSION
	../configure --prefix=/usr -enable-checking=release -enable-languages=c,c++ -disable-multilib 
	make -j $CPU_NUM
	make install
}

build_termcap(){
	# building termcap
	cd $CUR_DIR
	download $TERMCAP.tar.gz $TERMCAP_DOWNLOAD_PATH
	tar -xzvf $TERMCAP.tar.gz
	cd $TERMCAP
	./configure --prefix=/usr
    make
	if [ $? -eq 0 ]; then
  	echo "make termcap successed";
	else
  	echo "make termcap failed";
  	exit;
	fi
    make install
	if [ $? -eq 0 ]; then
  	echo "install termcap successed";
	else
  	echo "install termcap failed";
  	exit;
	fi
}

build_gdb(){
	# building gdb
    yum -y install texinfo
	cd $CUR_DIR
	download $GDB.tar.gz $GDB_DOWNLOAD_PATH
	tar -xzvf $GDB.tar.gz
	cd $GDB
	./configure --prefix=/usr
    make
	if [ $? -eq 0 ]; then
  	echo "make gdb successed";
	else
  	echo "make gdb failed";
  	exit;
	fi
    make install
	if [ $? -eq 0 ]; then
  	echo "install gdb successed";
	else
  	echo "install gdb failed";
  	exit;
	fi
}


get_cur_dir
build_g++
build_termcap
build_gdb
