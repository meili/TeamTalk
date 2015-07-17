/*================================================================
*     Copyright (c) 2015年 lanhu. All rights reserved.
*   
*   文件名称：Task.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月12日
*   描    述：
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
