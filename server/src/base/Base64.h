/*================================================================
*   Copyright (C) 2015 All rights reserved.
*   
*   �ļ����ƣ�base64.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��01��28��
*   ��    ����
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

