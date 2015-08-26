/******************************************************************************* 
 *  @file      OperationManager.cpp 2014\12\18 19:16:30 $
 *  @author    大佛<dafo@mogujie.com>
 *  @brief     
 ******************************************************************************/

#include "GlobalConfig.h"
#include "yaolog/yaolog.h"
#include "network/Operation.h"
#include "OperationManager.h"
#include <algorithm>
/******************************************************************************/
NAMESPACE_BEGIN(imcore)

namespace
{
	class LambdaOperation : public Operation
	{
	public:
		LambdaOperation(std::function<void()> operationRun)
			:m_operationRun(operationRun)
		{
		}

		virtual void processOpertion()
		{
			m_operationRun();
		}
		virtual void release()
		{
			delete this;
		}

	private:
		std::function<void()> m_operationRun;
	};
}

IMCoreErrorCode OperationManager::startup()
{
	m_operationThread = std::thread([&]
	{
		std::unique_lock <std::mutex> lck(m_cvMutex);
		Operation* pOperation = nullptr;
		while (m_bContinue)
		{
			if (!m_bContinue)
				break;
			if (m_vecRealtimeOperations.empty())
				m_CV.wait(lck);
			if (!m_bContinue)
				break;
			{
				std::lock_guard<std::mutex> lock(m_mutexOperation);
				if (m_vecRealtimeOperations.empty())
					continue;
				pOperation = m_vecRealtimeOperations.front();
				m_vecRealtimeOperations.pop_front();
			}

			if (!m_bContinue)
				break;

			if (pOperation)
			{
				pOperation->process();
				pOperation->release();
			}
		}
	});

	return IMCORE_OK;
}

void OperationManager::shutdown(IN int seconds /*= 2000*/)
{
	m_bContinue = false;
	m_CV.notify_one();

	//todo kuaidao ...join may be infinite waiting
	if (m_operationThread.joinable())
		m_operationThread.join();

	{
		std::lock_guard<std::mutex> locker(m_mutexOperation);
		for (IOperation* pOper : m_vecRealtimeOperations)
		{
			try
			{
				pOper->release();
			}
			catch (...)
			{

			}
		}
		m_vecRealtimeOperations.clear();
	}

	for (IOperation* pOper : m_vecDelayOperations)
	{
		delete pOper;
		pOper = 0;
	}
	m_vecDelayOperations.clear();
}

IMCoreErrorCode OperationManager::startOperation(IN Operation* pOperation, Int32 delay)
{
	assert(pOperation);
	if (0 == pOperation)
	{
		return IMCORE_ARGUMENT_ERROR;
	}

	//todo kuaidao...delay operntion process
	if (delay > 0)
	{
	}
	else
	{
		std::lock_guard<std::mutex> locker(m_mutexOperation);
		m_vecRealtimeOperations.push_back(pOperation);
		m_CV.notify_one();
	}

	return IMCORE_OK;
}

IMCoreErrorCode OperationManager::startOperationWithLambda(std::function<void()> operationRun
                                                            , Int32 delay
                                                            , std::string oper_name)
{
    LambdaOperation* pLambdaOper = new LambdaOperation(operationRun);
    pLambdaOper->set_name(oper_name);
    return startOperation(pLambdaOper, delay);
}

IMCoreErrorCode OperationManager::clearOperationByName(std::string oper_name)
{
    std::lock_guard<std::mutex> locker(m_mutexOperation);
    auto iter = std::remove_if(m_vecRealtimeOperations.begin(), m_vecRealtimeOperations.end(),
        [=](Operation* pOper)
    {
        if (pOper->name() == oper_name)
        {
            LOG__(APP, _T("clearOperationByName - %S"), oper_name.c_str());

            pOper->release();
            return true;
        }
        return false;
    });

    if (iter != m_vecRealtimeOperations.end())
    {
        m_vecRealtimeOperations.erase(iter, m_vecRealtimeOperations.end());
    }

    return IMCORE_OK;
}

OperationManager::~OperationManager()
{
	//捕捉可能抛出的未知异常
	try
	{
		shutdown();
	}
	catch (...)
	{
		LOG__(ERR, _T("OperationManager: shutdown throw unknown exception"));
		assert(FALSE);
	}
}

OperationManager* getOperationManager()
{
	static OperationManager manager;
	return &manager;
}

NAMESPACE_END(imcore)

/******************************************************************************/