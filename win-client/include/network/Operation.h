/*******************************************************************************
 *  @file      Operation.h 2014\12\23 13:35:31 $
 *  @author    ¿ìµ¶<kuaidao@mogujie.com>
 *  @brief     
 ******************************************************************************/

#ifndef OPERATION_D8F2BC78_108D_4219_9D6E_F1728D1B8C95_H__
#define OPERATION_D8F2BC78_108D_4219_9D6E_F1728D1B8C95_H__

#include "GlobalDefine.h"
#include "IOperation.h"
#include "network/networkdll.h"
/******************************************************************************/
NAMESPACE_BEGIN(imcore)

/**
 * The class <code>Operation</code> 
 *
 */
class NETWORK_DLL Operation : public IOperation
{
	enum OperationState
	{
		OPERATION_IDLE = 0,
		OPERATION_STARTING,
		OPERATION_RUNNING,
		OPERATION_CANCELLING,
		OPERATION_FINISHED
	};

public:
    /** @name Constructors and Destructor*/

    //@{
    /**
     * Constructor 
     */
    Operation();
	Operation(const std::string& name);
    /**
     * Destructor
     */
    virtual ~Operation();
    //@}

public:
	virtual void processOpertion() = 0;

public:
	virtual void process();
	virtual void release();

    inline std::string name() const { return m_name; }
    inline void set_name(__in std::string name){ m_name = name; }

private:
	OperationState			m_state;
    std::string				m_name;
};

NAMESPACE_END(imcore)
/******************************************************************************/
#endif// OPERATION_D8F2BC78_108D_4219_9D6E_F1728D1B8C95_H__
