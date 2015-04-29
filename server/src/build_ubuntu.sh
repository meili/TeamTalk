#!/bin/bash


build() {
	export CPLUS_INCLUDE_PATH=$PWD/slog
	export LD_LIBRARY_PATH=$PWD/slog
	export LIBRARY_PATH=$PWD/slog:$LIBRARY_PATH


    apt-get -y install cmake
    apt-get -y install libuu-dev 
    apt-get -y install libcurl4-openssl-dev
#    apt-get -y install openssl-devel
    apt-get -y  install libcurl-dev 
    apt-get -y  install liblog4cxx10-dev libprotobuf-lite8 libhiredis-dev protobuf-compiler cmake g++  libprotobuf-dev

	echo "#ifndef __VERSION_H__" > base/version.h
	echo "#define __VERSION_H__" >> base/version.h
	echo "#define VERSION \"$1\"" >> base/version.h
	echo "#endif" >> base/version.h

    if [ ! -d lib ]
    then
        mkdir lib
    fi

CURPWD=$PWD	


	 for i in base slog  route_server msg_server http_msg_server file_server push_server tools db_proxy_server msfs login_server ; do     
		cd $CURPWD/$i
		cmake .
		make
    		if [ $? -eq 0 ]; then
    		    echo "make msfs successed";
  		  else
       		 echo "make msfs failed";
       		 exit;
    		fi
		


	done
	
	cd $CURPWD

        cp base/libbase.a lib/
	 
    	mkdir base/slog/lib
        cp slog/slog_api.h base/slog/
        cp slog/libslog.so base/slog/lib/

    mkdir -p ../run/login_server
    mkdir -p ../run/route_server
    mkdir -p ../run/msg_server
    mkdir -p ../run/file_server
    mkdir -p ../run/msfs
    mkdir -p ../run/push_server
    mkdir -p ../run/http_msg_server
    mkdir -p ../run/db_proxy_server



	#copy executables to run/ dir
	cp login_server/login_server ../run/login_server/

	cp route_server/route_server ../run/route_server/

	cp msg_server/msg_server ../run/msg_server/

    cp http_msg_server/http_msg_server ../run/http_msg_server/

    cp file_server/file_server ../run/file_server/

    cp push_server/push_server ../run/push_server/

    cp db_proxy_server/db_proxy_server ../run/db_proxy_server/

    cp msfs/msfs ../run/msfs/

    cp tools/daeml ../run/

    build_version=im-server-$1
    build_name=$build_version.tar.gz
	if [ -e "$build_name" ]; then
		rm $build_name
	fi
    mkdir -p ../$build_version
    mkdir -p ../$build_version/login_server
    mkdir -p ../$build_version/route_server
    mkdir -p ../$build_version/msg_server
    mkdir -p ../$build_version/file_server
    mkdir -p ../$build_version/msfs
    mkdir -p ../$build_version/http_msg_server
    mkdir -p ../$build_version/push_server
    mkdir -p ../$build_version/db_proxy_server
    mkdir -p ../$build_version/lib

    cp login_server/loginserver.conf ../$build_version/login_server/
    cp login_server/login_server ../$build_version/login_server/

    cp route_server/route_server ../$build_version/route_server/
    cp route_server/routeserver.conf ../$build_version/route_server/

    cp msg_server/msg_server ../$build_version/msg_server/
    cp msg_server/msgserver.conf ../$build_version/msg_server/

    cp http_msg_server/http_msg_server ../$build_version/http_msg_server/
    cp http_msg_server/httpmsgserver.conf ../$build_version/http_msg_server/

    cp file_server/file_server ../$build_version/file_server/
    cp file_server/fileserver.conf ../$build_version/file_server/

    cp push_server/push_server ../$build_version/push_server/
    cp push_server/pushserver.conf ../$build_version/push_server/

    cp db_proxy_server/db_proxy_server ../$build_version/db_proxy_server/
    cp db_proxy_server/dbproxyserver.conf ../$build_version/db_proxy_server/

    cp msfs/msfs ../$build_version/msfs/
    cp msfs/msfs.conf.example ../$build_version/msfs/

    cp slog/log4cxx.properties ../$build_version/lib/
    cp slog/libslog.so  ../$build_version/lib/
    cp -a slog/lib/liblog4cxx.so* ../$build_version/lib/
    cp sync_lib_for_zip.sh ../$build_version/

    cp tools/daeml ../$build_version/
    cp ../run/restart.sh ../$build_version/

    cd ../
    tar zcvf    $build_name $build_version

    rm -rf $build_version
} 

clean() {
	cd base
	make clean
	cd ../login_server
	make clean
	cd ../route_server
	make clean
	cd ../msg_server
	make clean
	cd ../http_msg_server
    make clean
	cd ../file_server
    make clean
    cd ../push_server
    make clean
	cd ../db_proxy_server
	make clean
    cd ../push_server
    make clean
}

print_help() {
	echo "Usage: "
	echo "  $0 clean --- clean all build"
	echo "  $0 version version_str --- build a version"
}

case $1 in
	clean)
		echo "clean all build..."
		clean
		;;
	version)
		if [ $# != 2 ]; then 
			echo $#
			print_help
			exit
		fi

		echo $2
		echo "build..."
		build $2
		;;
	*)
		print_help
		;;
esac
