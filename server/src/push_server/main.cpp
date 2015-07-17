     //
//  main.cpp
//  my_push_server
//
//  Created by luoning on 14-11-4.
//  Copyright (c) 2014Äê luoning. All rights reserved.
//

#include <iostream>
#include "push_app.h"
#include "timer/Timer.hpp"
#include <sys/signal.h>

void writePid()
{
    uint32_t curPid;
#ifdef _WIN32
    curPid = (uint32_t) GetCurrentProcess();
#else
    curPid = (uint32_t) getpid();
#endif
    FILE* f = fopen("server.pid", "w");
    assert(f);
    char szPid[32];
    snprintf(szPid, sizeof(szPid), "%d", curPid);
    fwrite(szPid, strlen(szPid), 1, f);
    fclose(f);
}


int main(int argc, const char * argv[]) {
    // insert code here...
    printf("start push server...\n");
    signal(SIGPIPE, SIG_IGN);
    CPushApp::GetInstance()->Init();
    CPushApp::GetInstance()->Start();
    writePid();
    while (true) {
        S_Sleep(1000);
    }
    return 0;
}
