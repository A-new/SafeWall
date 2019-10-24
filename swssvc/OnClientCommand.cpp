
#include "swssvc.h"
#include <time.h>
#include "..\\include\\assist.h"

DWORD OnClientLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	Json::Reader reader;
	Json::Value user;

	std::string userid = context["userid"].asString();
	std::string password = context["password"].asString();

	if(context.isNull() == true)
	{
		//mylog(szLogFile, "contextΪ��");
		goto failed;
	}
	
	if(NULL != pPerHandle->dwUser)//��֤OnLogon�ǵ�һ����Ϣ
	{
		//mylog(szLogFile, "�û��ѵ�¼");
		goto failed;
	}

	if(userid.length() ==0 || 
		userid.length() >= sizeof(pPerHandle->userid))
	{
		//mylog(szLogFile, "�û����ƹ���");
		goto failed;
	}

	strcpy_s(pPerHandle->userid, userid.c_str());
	
	//MYLOG(root.toStyledString().c_str());
	EnterCriticalSection(&UserFile.section);
	user = (*UserFile.root)[pPerHandle->userid];
	LeaveCriticalSection(&UserFile.section);

	if(user.isNull() == true ||
		strcmp(user["password"].asCString(),password.c_str()))
	{
		//mylog(szLogFile, "����ƥ��ʧ��");
		goto failed;
	}

	//��������ǵ�¼�ɹ���
	pPerHandle->dwUser = USER_CLIENT;
	pPerIO->size = sizeof(SUCCESSMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SUCCESSMSG);

	pPerHandle->loginTime = time(NULL);
	InserUserListNode(&gUserList,pPerHandle);
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

DWORD OnClientCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	closesocket(pPerHandle->socket);
	return 0;
}

DWORD OnClientChangePwd(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	Json::Value reply;
	Json::Value access;	
	Json::Reader reader;
	std::string pwd;

	BOOL IsSucceed = FALSE;

	std::string password = context["password"].asString();
	std::string newpwd = context["newpwd"].asString();
	if(USER_CLIENT != pPerHandle->dwUser || '\0' == pPerHandle->userid[0])
	{
		//û��¼��������룬�ر���
		shutdown(pPerHandle->socket, 0);//�رն�
		goto failed;
		return 0;
	}

	EnterCriticalSection(&UserFile.section);
	pwd = (*UserFile.root)[pPerHandle->userid]["password"].asString();
	if(!strcmp(pwd.c_str(),password.c_str()))
	{
		(*UserFile.root)[pPerHandle->userid]["password"] = newpwd;
		IsSucceed = TRUE;
	}
	LeaveCriticalSection(&UserFile.section);

	if(FALSE == IsSucceed || FALSE == SavePolicy(&UserFile))
	{
		goto failed;
	}
	
	//�ɹ�
	pPerIO->size = sizeof(SUCCESSMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, SUCCESSMSG);
	SendProc(pPerHandle, pPerIO, 0);
	return 0;

failed:
	pPerIO->size = sizeof(FAILEDMSG);
	pPerIO->buffer = (char*)_HeapAlloc(pPerIO->size);
	strcpy(pPerIO->buffer, FAILEDMSG);
	SendProc(pPerHandle, pPerIO, 0);
	return -1;
}

DWORD OnClientLoadAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	Json::Reader reader;
	Json::Value reply;
	Json::Value pliRoot;
	Json::Value user;
	Json::Value access;
	std::string SentBuf;

	if(USER_CLIENT != pPerHandle->dwUser || '\0' == pPerHandle->userid[0])
	{
		goto failed;
	}

	//ȫ�ֲ���
	EnterCriticalSection(&PolicyFile.section);
	access["extern"] = (*PolicyFile.root)["extern"];
	LeaveCriticalSection(&PolicyFile.section);

	//�û�˽�в���
	EnterCriticalSection(&UserFile.section);
	access["access"] = (*UserFile.root)[pPerHandle->userid]["access"];
	LeaveCriticalSection(&UserFile.section);

	if(access["extern"].isNull() == true ||
		access["access"].isNull() == true)
	{
		goto failed;
	}

	reply["command"] = "reply";
	reply["context"] = access;

	SentBuf = reply.toStyledString();
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