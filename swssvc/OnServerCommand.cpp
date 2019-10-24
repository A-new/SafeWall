
#include "swssvc.h"
#include "..\\include\\assist.h"


DWORD OnServerLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	Json::Reader reader;

	EnterCriticalSection(&PolicyFile.section);
	std::string pwd = (*PolicyFile.root)["password"].asString();
	LeaveCriticalSection(&PolicyFile.section);

	//mylog(szLogFile,context.toStyledString().c_str());

	if(NULL != pPerHandle->dwUser)//��֤OnLogon�ǵ�һ����Ϣ
	{
		goto failed;
	}

	if(strcmp(pwd.c_str(),context["password"].asCString()))//ƥ���û�������ʧ��
	{
		goto failed;
	}
	
	//��������ǵ�¼�ɹ��ˣ��ȸ��߿ͻ��˳ɹ��ˣ�Ȼ����û����Ժ�ȫ�ֲ��Է��͹�ȥ
	//֪ͨ
	pPerHandle->dwUser = USER_SERVER;
	pPerIO->size = sizeof(SUCCESSMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SUCCESSMSG);
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	closesocket(pPerHandle->socket);
	return 0;
}

DWORD OnServerStopService(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	return 1;
}

DWORD OnServerGetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	SentBuf = PolicyFile.root->toStyledString();
	LeaveCriticalSection(&PolicyFile.section);

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerGetControlKeyInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
    std::string SentBuf;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	SentBuf = PolicyFile.root->toStyledString();
	LeaveCriticalSection(&PolicyFile.section);

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerGetFileAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	Json::Value json;
	Json::Value context2;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	context2["suffix"] = (*PolicyFile.root)["extern"]["suffix"].asString();
	LeaveCriticalSection(&PolicyFile.section);

	json["command"] = SAFEWALL_SERVER_GET_FILE_ACCESS;
	json["comtext"] = context2;
	SentBuf = json.toStyledString();
	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	closesocket(pPerHandle->socket);
	return -1;
}

DWORD OnServerSetFileAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string FileAccess;
	std::string SentBuf;
	Json::Value json;
	Json::Value context2;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	FileAccess = context["suffix"].asString();

	EnterCriticalSection(&PolicyFile.section);
	(*PolicyFile.root)["extern"]["suffix"] = FileAccess;
	LeaveCriticalSection(&PolicyFile.section);
	SavePolicy(&PolicyFile);

	context2["state"] = "success";
	json["command"] = SAFEWALL_SERVER_SET_FILE_ACCESS;
	json["comtext"] = context2;
	SentBuf = json.toStyledString();
	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	closesocket(pPerHandle->socket);
	return -1;
}

DWORD OnServerGetProcessAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	Json::Value json;
	Json::Value context2;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}
	
	EnterCriticalSection(&PolicyFile.section);
	context2["isolation"] = (*PolicyFile.root)["extern"]["isolation"].asString();
	LeaveCriticalSection(&PolicyFile.section);

	json["command"] = SAFEWALL_SERVER_GET_PROCESS_ACCESS;
	json["context"] = context2;
	SentBuf = json.toStyledString();
	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	closesocket(pPerHandle->socket);
	return -1;
}

DWORD OnServerSetProcessAccessInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	Json::Value json;
	Json::Value context2;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	(*PolicyFile.root)["extern"]["isolation"] = context["isolation"];
	LeaveCriticalSection(&PolicyFile.section);
	SavePolicy(&PolicyFile);

	context2["state"] = "success";
	json["command"] = SAFEWALL_SERVER_SET_PROCESS_ACCESS;
	json["context"] = context2;
	SentBuf = json.toStyledString();
	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	closesocket(pPerHandle->socket);
	return -1;
}

DWORD OnServerGetUserList(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	//ȡ�������û�������Ҫö��User�ļ���IC�ļ�
	std::string SentBuf;
	Json::Value json;
	Json::Value context2;
	Json::Value key;
	Json::Value::Members mem,mem2;
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&UserFile.section);
	mem = UserFile.root->getMemberNames();
	LeaveCriticalSection(&UserFile.section);

	for(auto iter = mem.begin(); iter != mem.end(); iter++)
	{

	}

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	closesocket(pPerHandle->socket);
	return -1;
}

DWORD OnServerAddUser(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
}

DWORD OnServerGetUserInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	SentBuf = PolicyFile.root->toStyledString();
	LeaveCriticalSection(&PolicyFile.section);

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerSetUserInfo(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	SentBuf = PolicyFile.root->toStyledString();
	LeaveCriticalSection(&PolicyFile.section);

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerGetOnlineUser(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	std::string SentBuf;
	
	if(USER_SERVER != pPerHandle->dwUser)//û��¼�ͽ�����һ���������ر�
	{
		goto failed;
	}

	EnterCriticalSection(&PolicyFile.section);
	SentBuf = PolicyFile.root->toStyledString();
	LeaveCriticalSection(&PolicyFile.section);

	pPerIO->size = SentBuf.length();
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SentBuf.c_str());
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	shutdown(pPerHandle->socket, 0);//�رն�
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnServerSetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	return 1;
}