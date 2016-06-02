// OS dependant type definition
#ifndef __OS_TYPE_H__
#define __OS_TYPE_H__

#ifdef _WIN32
    #include <WinSock2.h>
    #include <WinBase.h>
    #include <Windows.h>
    #include <direct.h>
#else
    #ifdef __APPLE__
        #include <sys/event.h>
        #include <sys/time.h>
        #include <sys/syscall.h>	// syscall(SYS_gettid)
    #else
        #include <sys/epoll.h>
    #endif
    #include <pthread.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/ioctl.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <errno.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <stdint.h>		// define int8_t ...
    #include <signal.h>
    #include <unistd.h>
    #define closesocket close
    #define ioctlsocket ioctl
#endif

#include <stdexcept>

#ifdef __GNUC__
    #include <ext/hash_map>
    using namespace __gnu_cxx;
    namespace __gnu_cxx {
        template<> struct hash<std::string> {
            size_t operator()(const std::string& x) const {
                return hash<const char*>()(x.c_str());
            }
        };
    }
#else
    #include <hash_map>
    using namespace stdext;
#endif

#ifdef _WIN32
    typedef char			int8_t;
    typedef short			int16_t;
    typedef int				int32_t;
    typedef	long long		int64_t;
    typedef unsigned char	uint8_t;
    typedef unsigned short	uint16_t;
    typedef unsigned int	uint32_t;
    typedef	unsigned long long	uint64_t;
    typedef int				socklen_t;
#else
    typedef int	SOCKET;
    typedef int BOOL;
#ifndef  __APPLE__
    const int TRUE = 1;
    const int FALSE = 0;
#endif
    const int SOCKET_ERROR	= -1;
    const int INVALID_SOCKET = -1;
#endif

typedef unsigned char	uchar_t;
typedef int				net_handle_t;
typedef int				conn_handle_t;

enum {
	NETLIB_OK		= 0,
	NETLIB_ERROR	= -1
};

#define NETLIB_INVALID_HANDLE	-1

enum
{
	NETLIB_MSG_CONNECT = 1,
	NETLIB_MSG_CONFIRM,	//2
	NETLIB_MSG_READ,	// 3
	NETLIB_MSG_WRITE,	// 4
	NETLIB_MSG_CLOSE,	// 5
	NETLIB_MSG_TIMER,
	NETLIB_MSG_LOOP,
	NETLIB_MSG_CLOSE_UDP,
	NETLIB_MSG_READ_UDP,
	NETLIB_MSG_WRITE_UDP
};

enum
{
	SOCKET_STATE_IDLE,	// =0
	SOCKET_STATE_LISTENING,	// 1
	SOCKET_STATE_CONNECTING,// 2
	SOCKET_STATE_CONNECTED,	// 3
	SOCKET_STATE_CLOSING,	// 4 
	SOCKET_STATE_UDP_BIND	// 5
};

const uint32_t INVALID_UINT32  = (uint32_t) -1;
const uint32_t INVALID_VALUE = 0;

typedef void (*callback_t)(void* callback_data, uint8_t msg, uint32_t handle, void* pParam);

#endif
