

#include "swssvc.h"
#include <time.h>

DWORD OnIcLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	Json::Reader reader;
	Json::Value user;
			
	std::string userid = context["userid"].asCString();
	
	if(NULL != pPerHandle->dwUser)//��֤OnLogon�ǵ�һ����Ϣ
	{
		goto failed;
	}

	if(userid.length() ==0 || userid.length() >= sizeof(pPerHandle->userid))
	{
		goto failed;
	}
	strcpy_s(pPerHandle->userid, userid.c_str());

	EnterCriticalSection(&IcUserFile.section);
	user = (*IcUserFile.root)[pPerHandle->userid];
	LeaveCriticalSection(&IcUserFile.section);

	if(user.isNull() == true)
	{
		goto failed;
	}

	pPerHandle->dwUser = USER_IC;
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

DWORD OnIcCancelLogon(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	closesocket(pPerHandle->socket);
	return 0;
}

DWORD OnIcGetAccess(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO,Json::Value context)
{
	//IC��û�в��Ե�
	return 0;
}