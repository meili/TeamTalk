/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�Task.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��01��12��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __TASK_H__
#define __TASK_H__

class CTask {
public:
    CTask(){}
    virtual ~CTask(){}
    
    virtual void run() = 0;
private:
};

#endif /*defined(__TASK_H__) */
