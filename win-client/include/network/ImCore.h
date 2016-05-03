/*******************************************************************************
 *  @file      ImCore.h 2014\11\28 13:28:49 $
 *  @author    蓝狐<lanhu@mogujie.com>
 *  @brief     im cross platform library,currently include network、opertion manager
 ******************************************************************************/

#ifndef IMCORE_A19CD639_91DF_4383_9D8F_4698959A8FD1_H__
#define IMCORE_A19CD639_91DF_4383_9D8F_4698959A8FD1_H__

#include "GlobalDefine.h"
#include "util.h"
#include <iostream>
#include <string.h>
#include <functional>
using namespace std;

#ifdef ANDROID
	#include <jni.h>
	#include <android/LOG__.h>
#endif
/******************************************************************************/
class ClientConn;
struct ITcpSocketCallback;
NAMESPACE_BEGIN(imcore)

const std::string OPERATION_NAME_MSG_READ_ACK = "operation_name_msg_read_ack";

class Operation;

#ifdef  __cplusplus
	extern "C" {
#endif
	
	/**@name start/stop*/
	//@{
	NETWORK_DLL bool IMLibCoreRunEvent();
	NETWORK_DLL bool IMLibCoreStopEvent();
	NETWORK_DLL bool IMLibCoreIsRunning();
	//@}

	/**@name network*/
	//@{
	NETWORK_DLL int IMLibCoreConnect(string ip, int port);
	NETWORK_DLL int IMLibCoreWrite(int key, uchar_t* data, uint32_t size);
	NETWORK_DLL void IMLibCoreShutdown(int key);
	NETWORK_DLL void IMLibCoreClose(int key);
	NETWORK_DLL void IMLibCoreRegisterCallback(int handle,ITcpSocketCallback* pCB);
	NETWORK_DLL void IMLibCoreUnRegisterCallback(int handle);
	//@}

	/**@name operation*/
	//@{
	/**
	* 发起一个Operation到任务队列尾部(放入容器的Operation对象实例不需要自己释放)
	*
	* @param   IOperation * pOperation
	* @param   int delay = 0
	* @return  void
	*/
	NETWORK_DLL void IMLibCoreStartOperation(IN Operation* pOperation, Int32 delay = 0);
	/**
	* 将Operation 以lambda表达式方式放入到任务队列中
	*
	* @param   std::function<void()> operationRun
	* @return  void
	*/
	NETWORK_DLL void IMLibCoreStartOperationWithLambda(std::function<void()> operationRun
        , Int32 delay = 0
        , std::string oper_name = "_common_operation_name");
    /**
    * 从任务队列中删除
    *
    * @param   std::function<void()> operationRun
    * @return  void
    */
    NETWORK_DLL void IMLibCoreClearOperationByName(std::string oper_name);
	//@}

    

#ifdef ANDROID
	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_setJNIEnv(JNIEnv *, jobject);
	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_runEvent(JNIEnv*, jclass);
	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_stopEvent(JNIEnv*, jclass);
	JNIEXPORT jboolean JNICALL Java_com_mogujie_im_libcore_LibCore_isRunning(JNIEnv*, jclass);
	JNIEXPORT jint JNICALL Java_com_mogujie_im_libcore_LibCore_connect(JNIEnv*, jclass, jstring, jint);
	JNIEXPORT jint JNICALL Java_com_mogujie_im_libcore_LibCore_write(JNIEnv* env, jclass, jint, jstring);
	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_close(JNIEnv*, jclass, jint);
	JNIEXPORT jstring JNICALL Java_com_mogujie_im_libcore_LibCore_getToken(JNIEnv*, jclass, jstring);
	char* jstringTostr(JNIEnv* env, jstring jstr);
	jstring strToJstring(JNIEnv* env, const char* pStr);

	void onRead(int nHandle, string strJson);
	void onClose(int nHandle);
	void onConnect(int nHandle);
#endif

#ifdef _MSC_VER
	unsigned int __stdcall event_run(void* threadArgu);
#else
	void *event_run(void* arg);
#endif

#ifdef __cplusplus
	}
#endif
NAMESPACE_END(imcore)
/******************************************************************************/
#endif// IMCORE_A19CD639_91DF_4383_9D8F_4698959A8FD1_H__

