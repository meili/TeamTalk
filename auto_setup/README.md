#安装说明:
	TeamTalk整套服务提供模块部署脚本和一键部署方案，主要模块有NGINX,PHP,MARIADB(MYSQL),REDIS,IM_WEB,IM_SERVER,其中IM_WEB,IM_SERVER为自主开发模块,其余均为开源解决方案,各个模块	需要手动改动的地方如下:
######NGINX: 
	无需改动

######PHP: 
	在conf目录下包含两个配置文件 php-fpm.conf php.ini, 可以自行进行优化配置, 也可以选择不改动这两个文件

######MARIADB(MYSQL): 
	在安装脚本setup.sh中默认设置了mysql root用户的初始密码为12345,在安装mariadb的过程中,会提示输入密码,如果想用自定义密码, 需要修改“MYSQL_PASSWORD=12345”对密码进行设置保持一致，一旦对密码进行更改,需要同时在IM_WEB与IM_SERVER中进行更改,详见IM_WEB和IM_SERVER配置说明。
	如果使用的是已存在的mariadb(mysql),可以直接使用"mysql -u $USER -p$PASSWORD < ttopen.sql"进行库与表的创建。
	

######REDIS: 
	在conf目录下包含了redis.conf的配置文件, 可以自行进行优化配置,也可以选择不改动这个文件使用默认配置。
	建议对redis进行主从配置，避免redis的重启导致数据紊乱。
 
######IM_WEB: 
	在conf目录下包含了config.php, database.php, im.com.conf三个配置文件,其中im.com.conf为NGINX所需要的配置文件,建议不改动
	config.php文件主要配置了msfs服务器的地址，修改$config['msfs_url']参数即可
	dababase.php文件主要配置了链接MARIADB所需要的参数,根据自己的需求修改'hostname','username','password'这三个参数。
	如果使用的是现有的nginx+php环境,可以修改setup.sh中的 PHP_WEB_SETUP_PATH为nginx放置web代码的路径,
	并且将PHP_NGINX_CONF_PATH修改为nginx配置文件的路径然后执行setup.sh脚本即可

######IM_SERVER: 
	IM_SERVER下共有8种服务器,所以也需要对这些服务器进行分别配置

	1.LOGIN_SERVER: 
	ClientListenIP为提供给Client端监听的本地地址，走的是pb协议
	HttpListenIP为提供给Client端监听的http协议本地地址(功能与ClientListenIP一样)
	MsgServerListenIP为提供给Msg Server端监听的本地地址
	msfs为提供客户端上传下载图片使用的msfs地址
	discovery为mac,ios,android客户端中“发现”所使用的地址,此处地址填写的是php后台的地址
	
	2.MSG_SERVER: 
	ListenIP为本机监听的IP,用于Client端的消息收发; 
	DBServerIP用于链接DB_PROXY,此处至少填两个数据库地址,也可以是同一个实例
	LoginServerIP用于链接LoginServer
	RouteServerIP用于链接RouteServer
	FileServerIP用于链接FileServer
	PushServerIP用于链接PushServer
	IpAddr填写的是Client端可以直接访问的地址,对于需要公网访问的情况下,如果是路由器映射,则需要填路由器映射在公网上的地址;此处需要填写两个Client端可以访问的地址,如果只有一个,则填写相同的地址即可
	aesKey为msg_server用于消息解密使用,与各个客户端保持一致

	3.ROUTE_SERVER: 
	根据说明配置需要监听的对应IP Port即可

	4.FILE_SERVER: 
	Address为Client端可以直接可以访问的IP地址,对于需要公网访问的情况下,如果是路由器映射,则需要填路由器映射在公网上的地址

	5.MSFS_SERVER: 
	ListenIP和Port填写的是监听的本地IP, BaseDir为默认保存图片文件的路径,如有必要可以更改

	6.DB_PROXY_SERVER	
	在安装配置脚本setup.sh中, DB_PROXY的默认监听Port设置为10600,如果被更改需要同时对IM_SERVER中其他服务器的配置进行更改，详见MSG_SERVER, HTTP_MSG_SERVER配置说明;
	对于mariadb与redis的配置可以根据自己的mysql,redis进行设置

	7.HTTP_MSG_SERVER
	ListenIP为本地监听的地址，供php后台使用
	DBServerIP用于链接DB_PROXY,此处至少填两个数据库地址,也可以是同一个实例
	RouteServerIP用于链接RouteServer

	8.PUSH_SERVER
	ListenIP为本地监听的地址
	CertPath,KeyPath,KeyPassword,SandBox为苹果apns服务所需要参数	

###一键部署:
	"强烈建议"选择一台未安装过的NGINX,PHP,MySQL,REDIS,并且OS为CentOS 6.X或者7.X,在安装之前可以先执行"setup.sh check"命令进行上述环境的检查。检查通过后对各个模块进行一些配置文件的设置,其中主要设置的为IM_SERVER中的几个服务器地址设置,设置完成后运行"setup.sh install"

###模块部署:
	TeamTalk的各模块支持安装到不同的服务器上,所以部署可以根据自己的需要进行模块安装,主要修改的地方即为上述各个模块中的IP地址设置。根据自己的网络拓扑在conf文件夹下的各个配置文件中预先设置正确的IP地址,然后执行模块内的"setup install"即可


###IM_SERVER与IM_DB_PROXY架构图如下:

![](https://raw.githubusercontent.com/mogutt/TTServer/master/docs/pics/server.jpg)
