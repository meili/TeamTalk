/*******************************************************************************
 *  @file      OperationManager.h 2014\12\18 19:14:27 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef OPERATIONMANAGER_7EEF3272_2557_4A76_9C25_67D4639F40DB_H__
#define OPERATIONMANAGER_7EEF3272_2557_4A76_9C25_67D4639F40DB_H__

#include "network/ErrorCode.h"
#include <condition_variable>
#include <mutex>
#include <list>
#include <thread>
/******************************************************************************/
NAMESPACE_BEGIN(imcore)

class Operation;

/**
 * The class <code>OperationManager</code> 
 *
 */
class OperationManager
{
public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
	OperationManager() = default;
    /**
     * Destructor
     */
	~OperationManager();
    //@}
	OperationManager(OperationManager&) = delete;
	OperationManager(OperationManager&&) = delete;
	OperationManager& operator= (OperationManager&) = delete;
	OperationManager& operator= (OperationManager&&) = delete;

public:
	IMCoreErrorCode startup();
    void shutdown(IN int seconds = 2000);
    IMCoreErrorCode startOperation(IN Operation* pOperation, Int32 delay);
    IMCoreErrorCode startOperationWithLambda(std::function<void()> operationRun
        , Int32 delay
        , std::string oper_name);
    IMCoreErrorCode clearOperationByName(std::string oper_name);

private:
	std::list<Operation*>      m_vecDelayOperations;
	std::list<Operation*>      m_vecRealtimeOperations;

	std::mutex					m_cvMutex; // 互斥锁.
	std::condition_variable		m_CV; // 条件变量

	std::mutex					m_mutexOperation;

	bool                        m_bContinue = true;
	std::thread					m_operationThread;
};

OperationManager* getOperationManager();

NAMESPACE_END(imcore)
/******************************************************************************/
#endif// OPERATIONMANAGER_7EEF3272_2557_4A76_9C25_67D4639F40DB_H__