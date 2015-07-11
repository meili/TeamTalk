/*================================================================
 *   Copyright (C) 2014 All rights reserved.
 *
 *   �ļ����ƣ�RelationModel.h
 *   �� �� �ߣ�Zhang Yuanhao
 *   ��    �䣺bluefoxah@gmail.com
 *   �������ڣ�2014��12��15��
 *   ��    ����
 *
 ================================================================*/

#ifndef RELATION_SHIP_H_
#define RELATION_SHIP_H_

#include <list>

#include "util.h"
#include "ImPduBase.h"
#include "IM.BaseDefine.pb.h"

using namespace std;


class CRelationModel {
public:
	virtual ~CRelationModel();

	static CRelationModel* getInstance();
    uint32_t getRelationId(uint32_t nUserAId, uint32_t nUserBId, bool bAdd);
    bool updateRelation(uint32_t nRelationId, uint32_t nUpdateTime);
    bool removeRelation(uint32_t nRelationId);
    
private:
	CRelationModel();
    uint32_t addRelation(uint32_t nSmallId, uint32_t nBigId);
private:
	static CRelationModel*	m_pInstance;
};
#endif
