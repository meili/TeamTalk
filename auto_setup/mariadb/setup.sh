#!/bin/bash
# this is a setup scripts for mysql
# author: luoning
# date: 08/30/2014

# setup mysql

MARIADB_SHARED=MariaDB-10.0.21-centos6-x86_64-shared
MARIADB_SHARED_DOWNLOAD_PATH=http://mirrors.aliyun.com/mariadb/mariadb-10.0.21/yum/centos6-amd64/rpms/$MARIADB_SHARED.rpm

MARIADB_CLIENT=MariaDB-10.0.21-centos6-x86_64-client
MARIADB_CLIENT_DOWNLOAD_PATH=http://mirrors.aliyun.com/mariadb/mariadb-10.0.21/yum/centos6-amd64/rpms/$MARIADB_CLIENT.rpm


MARIADB_SERVER=MariaDB-10.0.21-centos6-x86_64-server
MARIADB_SERVER_DOWNLOAD_PATH=http://mirrors.aliyun.com/mariadb/mariadb-10.0.21/yum/centos6-amd64/rpms/$MARIADB_SERVER.rpm

MARIADB_COMMON=MariaDB-10.0.21-centos6-x86_64-common
MARIADB_COMMON_DOWNLOAD_PATH=http://mirrors.aliyun.com/mariadb/mariadb-10.0.21/yum/centos6-amd64/rpms/$MARIADB_COMMON.rpm

MARIADB_COMPAT=MariaDB-10.0.21-centos6-x86_64-compat
MARIADB_COMPAT_DOWNLOAD_PATH=http://mirrors.aliyun.com/mariadb/mariadb-10.0.21/yum/centos6-amd64/rpms/$MARIADB_COMPAT.rpm

IM_SQL=ttopen.sql
MYSQL_CONF=my.cnf
MYSQL_PASSWORD=12345

CENTOS_VERSION=6

print_hello(){
	echo "==========================================="
	echo "$1 mysql for TeamTalk"
	echo "==========================================="
}

check_user() {
	if [ $(id -u) != "0" ]; then
    	echo "Error: You must be root to run this script, please use root to install mysql"
    	exit 1
	fi
}

check_os() {
	OS_VERSION=$(less /etc/redhat-release)
	OS_BIT=$(getconf LONG_BIT)
	#echo "$OS_VERSION, $OS_BIT bit..." 
	if [[ $OS_VERSION =~ "CentOS" ]]; then
		if [ $OS_BIT == 64 ]; then
			if [[ $OS_VERSION =~ "7" ]]; then
				CENTOS_VERSION=7
			else 
				CENTOS_VERSION=6
			fi
			return 0
		else
			echo "Error: OS must be CentOS 64bit to run this script."
			exit 1
		fi
	else
		echo "Error: OS must be CentOS 64bit to run this script."
		exit 1
	fi
}

check_run() {
	ps -ef | grep -v 'grep' | grep mysqld
	if [ $? -eq 0 ]; then
		echo "Error: mysql is running."
		exit 1
	fi
}

clean_yum() {
	YUM_PID=/var/run/yum.pid
	if [ -f "$YUM_PID" ]; then
		set -x
		rm -f YUM_PID
		killall yum
		set +x
	fi
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

build_ssl() {
	clean_yum
	yum -y install openssl-devel
	if [ $? -eq 0 ]; then
		echo "yum install openssl-devel successed."
	else
		echo "Error: yum install openssl-devel failed."
		return 1;
	fi
}

build_mysql2() {
	download $MARIADB_SERVER.rpm $MARIADB_SERVER_DOWNLOAD_PATH
	if [ $? -eq 1 ]; then
		return 1
	fi
	
	download $MARIADB_CLIENT.rpm $MARIADB_CLIENT_DOWNLOAD_PATH
	if [ $? -eq 1 ]; then
		return 1
	fi

	download $MARIADB_SHARED.rpm $MARIADB_SHARED_DOWNLOAD_PATH
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

	build_ssl 
	if [ $? -eq 0 ]; then
		echo "build ssl successed."
	else
		echo "Error: build ssl failed."
		return 1
	fi

	yum -y install perl-DBI

	rpm -ivh MariaDB-*
	RET=$?
	if [ $RET -eq 0 ]; then
		echo "install mariadb-server successed";
	elif [ $RET -eq 3 ]; then
		echo "mariadb-server has installed";
	else
		echo "Error: install mariadb-server failed";
		return 1;
	fi
	
	if [ -f /usr/share/mysql/my-huge.cnf ]; then
		cp -f /usr/share/mysql/my-huge.cnf /etc/$MYSQL_CONF
	else
		echo "Error: $MYSQL_CONF is not existed";
		return 1;
	fi
}

build_mysql() {
	clean_yum
	yum -y install mariadb
	if [ $? -eq 0 ]; then
		echo "yum install mysql successed."
	else
		echo "Error: yum install mysql failed."
		return 1;
	fi

	clean_yum
	yum -y install mariadb-server
	if [ $? -eq 0 ]; then
		echo "yum install mysql-server successed."
	else
		echo "Error: yum install mysql-server failed."
		return 1;
	fi

	clean_yum
	yum -y install mariadb-devel
	if [ $? -eq 0 ]; then
		echo "yum install mysql-devel successed."
	else
		echo "Error: yum install mysql-devel failed."
		return 1;
	fi


	if [ -f /usr/share/mysql/my-huge.cnf ]; then
		cp -f /usr/share/mysql/my-huge.cnf /etc/$MYSQL_CONF
	else
		echo "Error: $MYSQL_CONF is not existed";
		return 1;
	fi
}

run_mysql() {
	PROCESS=$(pgrep mysql)
	if [ -z "$PROCESS" ]; then 
		echo "no mysql is running..." 
		if [ $CENTOS_VERSION -eq 7 ]; then
			service mariadb start
		else
			service mysql start
		fi
		if [ $? -eq 0 ]; then
			echo "start mysql successed."
		else
			echo "Error: start mysql failed."
			return 1
		fi
	else 
		echo "Warning: mysql is running"
	fi
}	

set_password() {
	mysql_secure_installation
}


create_database() {
	cd ./conf/
	if [ -f "$IM_SQL" ]; then
		echo "$IM_SQL existed, begin to run $IM_SQL"
	else
		echo "Error: $IM_SQL not existed."
		cd ..
		return 1
	fi

	mysql -u root -p$MYSQL_PASSWORD < $IM_SQL
	if [ $? -eq 0 ]; then
		echo "run sql successed."
		cd ..
	else
		echo "Error: run sql failed."
		cd ..
		return 1
	fi
}

build_all() {

	#echo "$OS_VERSION, $OS_BIT bit..." 
	if [ $CENTOS_VERSION -eq 7 ]; then
		build_mysql
		if [ $? -eq 0 ]; then
			echo "build mysql successed."
		else
			echo "Error: build mysql failed."
			exit 1
		fi
	else
		build_mysql2
		if [ $? -eq 0 ]; then
			echo "build mysql successed."
		else
			echo "Error: build mysql failed."
			exit 1
		fi
	fi
	

	run_mysql
	if [ $? -eq 0 ]; then
		echo "run mysql successed."
	else
		echo "Error: run mysql failed."
		exit 1
	fi

	set_password
	if [ $? -eq 0 ]; then
		echo "set password successed."
	else
		echo "Error: set password failed."
		exit 1
	fi

	create_database
	if [ $? -eq 0 ]; then
		echo "create database successed."
	else
		echo "Error: create database failed."
		exit 1
	fi	
}


print_help() {
	echo "Usage: "
	echo "  $0 check --- check environment"
	echo "  $0 install --- check & run scripts to install"
}

case $1 in
	check)
		print_hello $1
		check_user
		check_os
		check_run
		;;
	install)
		print_hello $1
		check_user
		check_os
		check_run
		build_all
		;;
	*)
		print_help
		;;
esac


