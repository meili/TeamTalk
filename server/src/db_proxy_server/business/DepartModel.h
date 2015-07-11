/*================================================================
*     Copyright (c) 2015�� lanhu. All rights reserved.
*   
*   �ļ����ƣ�DepartModel.h
*   �� �� �ߣ�Zhang Yuanhao
*   ��    �䣺bluefoxah@gmail.com
*   �������ڣ�2015��03��12��
*   ��    ����
*
#pragma once
================================================================*/
#ifndef __DEPARTMODEL_H__
#define __DEPARTMODEL_H__

#include "ImPduBase.h"
#include "IM.BaseDefine.pb.h"

class CDepartModel {
public:
    static CDepartModel* getInstance();
    ~CDepartModel() {}
    void getChgedDeptId(uint32_t& nLastTime, list<uint32_t>& lsChangedIds);
    void getDepts(list<uint32_t>& lsDeptIds, list<IM::BaseDefine::DepartInfo>& lsDepts);
    void getDept(uint32_t nDeptId, IM::BaseDefine::DepartInfo& cDept);
private:
    CDepartModel(){};
private:
    static CDepartModel* m_pInstance;
};

#endif /*defined(__DEPARTMODEL_H__) */
