## 1、依赖

	服务端对pb,hiredis,mysql_client,log4cxx有依赖，所以服务端需要先安装pb，hiredis,mysql,log4cxx。
	在正式编译服务端之前，请先执行server/src目录下的：
	make_hiredis.sh
	make_log4cxx.sh
	make_mariadb.sh
	make_protobuf.sh
	这些脚本会先安装以上依赖。
	
## 2、编译协议文件
	
	所有的协议文件在pb目录下，其中有create.sh以及sync.sh两个shell脚本。
	create.sh的作用是使用protoc将协议文件转化成相应语言的源码。
	sync.sh是将生成的源码拷贝到server的目录下。
	
## 3、编译服务端
	
	经历了编译服务端依赖，pb之后，就可以执行server目录下的build.sh脚本
	
## 4、部署说明
	
	虽然我们已经写了一键部署脚本，但是毕竟TeamTalk是一个完整的解决方案，所以，对于使用者，我们会假设您**会Linux基本操作，能看懂Shell脚本，有一定的Linux操作运维经验**。
	
	一键部署在auto_setup目录下，其中包含了如下几个子目录及文件:
		gcc_setup 与GCC相关的部署，由于我们的服务端极少量的使用了auto等C++11，所以对gcc版本有一定的要求，该目录下的setup.sh会下载安装gcc 4.9.2.
		im_server 与TeamTalk服务端相关的部署，该目录下包含了服务端的配置模板等。
		im_web 与TeamTalk web管理相关的部署，包含了PHP的配置以及php所需nginx相关配置。需要将php目录更名为tt并打包压缩放到此目录下,否则会报如下错误:
		unzip: cannot find or open tt.zip, tt.zip.zip or tt.zip.ZIP。
		mariadb  由于Percona变化太大，所以本次发布我们使用了mariadb作为默认数据库，如果有需要可以根据自己的需求改成mysql，但是请不要在centos 6及以下的系统中使用yum安装的mysql，因为数据库的配置中会设置utf8mb4，以支持emoji表情的存储，而CentOS 6及以下的系统默认使用的是mysql 5.1 默认是不支持utf8mb4的，需要自己编译。
		nginx_php nginx php相关的部署及配置文件。
		redis	redis相关的部署配置文件，redis建议部署主从，防止一台redis奔溃后一些信息的丢失，如果没有条件配置主从，请配置redis的持久化，具体教程请参考Google，但是配置持久化会降低redis的性能。建议配置主从。
		setup.sh 是部署总入口，会分别调用二级目录下的setup.sh执行安装部署过程。

## 5、试用包
	试用包下载地址:
	
	Android:http://s8.mogucdn.com/download/TeamTalk/OpenSource/TeamTalk-Android-0331.apk
	Mac:http://s21.mogucdn.com/download/TeamTalk/OpenSource/TeamTalk-Mac.zip
	Windows:http://s21.mogucdn.com/download/TeamTalk/OpenSource/TeamTalk-win.zip
