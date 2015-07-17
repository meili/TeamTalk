//
//  push_define.h
//  my_push_server
//
//  Created by luoning on 14-11-5.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#ifndef my_push_server_push_define_h
#define my_push_server_push_define_h

#include "slog_api.h"
#define PDU_VERSION     1

#define PUSH_TYPE_NORMAL        1
#define PUSH_TYPE_SILENT        2

#define LOG_MODULE_PUSH         "PUSH"
extern CSLog g_pushlog;

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1):__FILE__)

#define PUSH_SERVER_FATAL(fmt, ...) \
{\
    g_pushlog.Fatal("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}

#define PUSH_SERVER_ERROR(fmt, ...) \
{\
    g_pushlog.Error("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}

#define PUSH_SERVER_WARN(fmt, ...) \
{\
    g_pushlog.Warn("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}

#define PUSH_SERVER_INFO(fmt, ...) \
{\
    g_pushlog.Info("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}

#define PUSH_SERVER_DEBUG(fmt, ...) \
{\
    g_pushlog.Debug("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}

#define PUSH_SERVER_TRACE(fmt, ...) \
{\
    g_pushlog.Trace("<%s>|<%d>|<%s>," fmt, __FILENAME__, __LINE__, __FUNCTION__, ##__VA_ARGS__);\
}



#endif
