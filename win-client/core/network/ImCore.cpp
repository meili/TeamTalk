#include "network/ImCore.h"
#include "TcpSocketsManager.h"
#include "OperationManager.h"
#include <process.h>

NAMESPACE_BEGIN(imcore)

#ifdef ANDROID
	//全局变量
	JavaVM *g_jvm = NULL;
	jobject g_obj = NULL;
#endif
	static CLock g_lock;

#ifdef _MSC_VER
	HANDLE g_hThreadHandle = 0;
	unsigned int __stdcall event_run(void* threadArgu)
	{
		LOG__(NET,  _T("event_run"));
		netlib_init();
		netlib_set_running();
		netlib_eventloop();
		return NULL;
	}
#else
	void *event_run(void* arg) {
		LOG__(NET,  _T("event_run"));
		netlib_init();
		netlib_eventloop();
		return NULL;
	}
#endif

#ifdef ANDROID
	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_setJNIEnv(
		JNIEnv* env, jobject thiz) {
		env->GetJavaVM(&g_jvm);
		g_obj = env->NewGlobalRef(thiz);
	}

	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_runEvent(JNIEnv* env,
		jclass thiz) {
		IMLibCoreRunEvent();
	}

	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_stopEvent(JNIEnv* env,
		jclass thiz) {
		IMLibCoreRunEvent();
	}

	JNIEXPORT jboolean JNICALL Java_com_mogujie_im_libcore_LibCore_isRunning(JNIEnv*, jclass)
	{
		jboolean running = IMLibCoreIsRunning();
		return running ? JNI_TRUE : JNI_FALSE;
	}

	JNIEXPORT jint JNICALL Java_com_mogujie_im_libcore_LibCore_connect(JNIEnv* env,
		jclass thiz, jstring ip, jint port) {
		jint nRet = -1;
		char* pIp = jstringTostr(env, ip);
		int nPort = port;
		return IMLibCoreConnect(pIp, port);
	}

	JNIEXPORT jint JNICALL Java_com_mogujie_im_libcore_LibCore_write(JNIEnv* env,
		jclass thiz, jint key, jstring data) {
		jint nRet = -1;
		int nHandle = key;
		char* pData = jstringTostr(env, data);
		nRet = IMLibCoreWrite(nHandle, pData);
		return nRet;
	}

	JNIEXPORT void JNICALL Java_com_mogujie_im_libcore_LibCore_close(JNIEnv* env,
		jclass thiz, jint key) {
		int nHandle = key;
		IMLibCoreClose(nHandle);
	}

	void onRead(int nHandle, string strJson) {
		JNIEnv *env;
		jclass cls;
		jmethodID mid;

		if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
			LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
			return;
		}
		cls = env->GetObjectClass(g_obj);
		//cls = env->FindClass("com/mogujie/im/LibCore");
		if (NULL == cls) {
			LOGE("FindClass() Error.....");

			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		mid = env->GetStaticMethodID(cls, "onRead", "(ILjava/lang/String;)V");
		if (NULL == mid) {
			LOGE("GetMethodID() Error.....");
			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		jstring data = strToJstring(env, strJson.c_str());
		env->CallStaticVoidMethod(cls, mid, nHandle, data);
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
	}

	void onClose(int nHandle) {
		JNIEnv *env;
		jclass cls;
		jmethodID mid;
		if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
			LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
			return;
		}
		cls = env->GetObjectClass(g_obj);
		//cls = env->FindClass("com/mogujie/im/LibCore");
		if (NULL == cls) {
			LOGE("FindClass() Error.....");

			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		mid = env->GetStaticMethodID(cls, "onClose", "(I)V");
		if (NULL == mid) {
			LOGE("GetMethodID() Error.....");
			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		env->CallStaticVoidMethod(cls, mid, nHandle);
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}

	}

	void onConnect(int nHandle) {
		JNIEnv *env;
		jclass cls;
		jmethodID mid;

		if (g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK) {
			LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
			return;
		}
		cls = env->GetObjectClass(g_obj);
		//cls = env->FindClass("com/mogujie/im/LibCore");
		if (NULL == cls) {
			LOGE("FindClass() Error.....");

			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		mid = env->GetStaticMethodID(cls, "onConnect", "(I)V");
		if (NULL == mid) {
			LOGE("GetMethodID() Error.....");
			//Detach主线程
			if (g_jvm->DetachCurrentThread() != JNI_OK) {
				LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
			}
			return;
		}
		env->CallStaticVoidMethod(cls, mid, nHandle);
		if (g_jvm->DetachCurrentThread() != JNI_OK) {
			LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
		}
	}

	char* jstringTostr(JNIEnv* env, jstring jstr) {
		char* pStr = NULL;

		jclass jstrObj = env->FindClass("java/lang/String");
		jstring encode = env->NewStringUTF("utf-8");
		jmethodID methodId = env->GetMethodID(jstrObj, "getBytes",
			"(Ljava/lang/String;)[B");
		jbyteArray byteArray = (jbyteArray)env->CallObjectMethod(jstr, methodId,
			encode);
		jsize strLen = env->GetArrayLength(byteArray);
		jbyte *jBuf = env->GetByteArrayElements(byteArray, JNI_FALSE);

		if (jBuf > 0) {
			pStr = (char*)malloc(strLen + 1);

			if (!pStr) {
				return NULL;
			}

			memcpy(pStr, jBuf, strLen);

			pStr[strLen] = 0;
		}

		env->ReleaseByteArrayElements(byteArray, jBuf, 0);

		return pStr;
	}

	//C字符串转java字符串
	jstring strToJstring(JNIEnv* env, const char* pStr) {
		int strLen = strlen(pStr);
		jclass jstrObj = env->FindClass("java/lang/String");
		jmethodID methodId = env->GetMethodID(jstrObj, "<init>",
			"([BLjava/lang/String;)V");
		jbyteArray byteArray = env->NewByteArray(strLen);
		jstring encode = env->NewStringUTF("utf-8");

		env->SetByteArrayRegion(byteArray, 0, strLen, (jbyte*)pStr);

		return (jstring)env->NewObject(jstrObj, methodId, byteArray, encode);
	}

	JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
		JNIEnv* env = NULL;
		jint result = -1;
		//获取JNI版本
		if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK) {
			LOGE("GetEnv failed!");
			return result;
		}
		return JNI_VERSION_1_4;
	}

	JNIEXPORT jstring JNICALL Java_com_mogujie_im_libcore_LibCore_getToken(JNIEnv* env, jclass thiz, jstring uuid)
	{
		char* pData = jstringTostr(env, uuid);
		char token[64];
		int uid = urltoid(pData);
		genToken(uid, 0, token);
		return strToJstring(env, token);
	}
#endif

	bool IMLibCoreRunEvent()
	{	
		LOG__(NET, _T("==============================================================================="));

		getOperationManager()->startup();

		CAutoLock lock(&g_lock);
		if (!netlib_is_running())
		{
#ifdef _MSC_VER
			unsigned int m_dwThreadID;
			g_hThreadHandle = (HANDLE)_beginthreadex(0, 0, event_run, 0, 0, (unsigned*)&m_dwThreadID);
			if (g_hThreadHandle < (HANDLE)2)
			{
				m_dwThreadID = 0;
				g_hThreadHandle = 0;
			}
			return g_hThreadHandle >(HANDLE)1;
#else
			pthread_t pt;
			pthread_create(&pt, NULL, event_run, NULL);
#endif
		}

		return true;
	}

	bool IMLibCoreStopEvent()
	{
		bool bRet = false;
		netlib_stop_event();
#ifdef _MSC_VER
		if (g_hThreadHandle)
		{
			if (::WaitForSingleObject(g_hThreadHandle, 3000) == WAIT_TIMEOUT)
			{
				::TerminateThread(g_hThreadHandle, 0);
				WaitForSingleObject(g_hThreadHandle, 500);
			}
			::CloseHandle(g_hThreadHandle);
			g_hThreadHandle = 0;
		}
#endif
		netlib_destroy();

		getOperationManager()->shutdown();

		return bRet;
	}

	bool IMLibCoreIsRunning()
	{
		return netlib_is_running();
	}

	int IMLibCoreConnect(string ip, int port)
	{
		return TcpSocketsManager::getInstance()->connect(ip.c_str(), port);
	}

	int IMLibCoreWrite(int key, uchar_t* data, uint32_t size)
	{
		int nRet = -1;
		int nHandle = key;
		CImConn* pConn = TcpSocketsManager::getInstance()->get_client_conn(nHandle);
		if (pConn) {
			pConn->Send((void*)data, size);
		}
		else {
			LOG__(NET,  _T("connection is invalied:%d"), key);
		}

		return nRet;
	}

	void IMLibCoreClose(int key)
	{
		LOG__(NET,  _T("close key:%d"), key);
		int nHandle = key;
		CImConn* pConn = TcpSocketsManager::getInstance()->get_client_conn(nHandle);
		if (pConn) {
			pConn->Close();
		}
	}

	NETWORK_DLL void IMLibCoreShutdown(int key)
	{
		LOG__(NET, _T("shutdown key:%d"), key);
		int nHandle = key;
		CImConn* pConn = TcpSocketsManager::getInstance()->get_client_conn(nHandle);
		if (pConn) {
			pConn->Shutdown();
		}
	}

	void IMLibCoreRegisterCallback(int handle,ITcpSocketCallback* pCB)
	{
		TcpSocketsManager::getInstance()->registerCallback(handle,pCB);
	}

	void IMLibCoreUnRegisterCallback(int handle)
	{
		TcpSocketsManager::getInstance()->unRegisterCallback(handle);
	}

	void IMLibCoreStartOperation(IN Operation* pOperation, Int32 delay /*= 0*/)
	{
		if (getOperationManager()->startOperation(pOperation, delay) != IMCORE_OK)
		{
			LOG__(ERR, _T("push operation failed"));
		}
	}

    void IMLibCoreStartOperationWithLambda(std::function<void()> operationRun, Int32 delay /*= 0*/, std::string oper_name)
    {
        if (getOperationManager()->startOperationWithLambda(operationRun, delay, oper_name) != IMCORE_OK)
        {
            LOG__(ERR, _T("push operation with lambda failed"));
        }
    }

    void IMLibCoreClearOperationByName(std::string oper_name)
    {
        if (getOperationManager()->clearOperationByName(oper_name) != IMCORE_OK)
        {
            LOG__(ERR, _T("clear operation by name failed"));
        }
    }


	NAMESPACE_END(imcore)