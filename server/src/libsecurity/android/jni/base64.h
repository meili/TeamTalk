/*================================================================
*   Copyright (C) 2015 All rights reserved.
*   
*   文件名称：base64.h
*   创 建 者：Zhang Yuanhao
*   邮    箱：bluefoxah@gmail.com
*   创建日期：2015年01月28日
*   描    述：
*
#pragma once
================================================================*/
#ifndef __BASE64_H__
#define __BASE64_H__
#include<iostream>
using namespace std;

string base64_decode(const string &ascdata);
string base64_encode(const string &bindata);

#endif

