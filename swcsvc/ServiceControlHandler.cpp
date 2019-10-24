
#include "swcsvc.h"
#include "..\\include\\inirw.h"
#include "..\\include\\assist.h"
#include <time.h>

SOCKET server = NULL;
char userid[64] = {0};//��ͨ�û�
char password[64] = {0};
char userid2[64] = {0};//IC�û�
char password2[64] = {0};

time_t LastTime; //���������
int EffectiveTime=0; //֤����Чʱ��,һ�������ʱ5����

#define MIN_EFFECTIVE_TIME (5*60)//5����

#define SYNC_NO_ERROR     0
#define SYNC_SEND_ERROR   1
#define SYNC_READ_ERROR   2
#define SYNC_SOCKET_VALUE 3

//������
UINT WINAPI KeepActivityThread(LPVOID lpParam)
{
	long data = 0;
	MYSERVCONTEXT Context;
	memset(&Context,0, sizeof(MYSERVCONTEXT));
	while(running)
	{
		Sleep(28 * 1000);//��Ϣ28��
		if(userid[0]!='\0')
		{
			if(SOCKET_ERROR == mySend(server,(char*)&data, sizeof(data), &gSection))
			{
				//����������ʧ��
				closesocket(server);
				server = NULL;
				strcpy(Context.userid,userid);
				strcpy(Context.password, password);
				OnLogon(&Context);
			}
			else
			{
				LastTime = time(NULL);
			}
		}
	}
	return 0;
}

DWORD OnLogon(LPMYSERVCONTEXT lpContext)
{
	char * buffer;
	SOCKADDR_IN servAddr;
	Json::Value root; 
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	std::string sentbuf;
	DWORD error;

	char ipaddr[256]= "127.0.0.1";
	int port = 1808;
	
	if(server && userid[0]!='\0')
	{
		//�Ѿ����û���¼
		//mylog(LogPath,"�Ѿ����û���¼");
		return 1;
	}

	INIOBJECT iniobj =  CreateIniObject(szConfigFile);
	iniGetString(iniobj,"safewall server", "ipaddr", ipaddr, sizeof(ipaddr),"127.0.0.1");
	port = iniGetInt(iniobj,"safewall server", "port", port);
	ReleaseIniObject(iniobj);
	error = SOCKET_ERROR;
	do
	{
		server = myTcpSocket();

		if(FALSE == myConnect(server, ipaddr, port))
		{
			//mylog(szLogPath,"���ӷ�����ʧ��");
			break;
		}
	
		context["userid"] = lpContext->userid;
		context["password"] = lpContext->password;
		root["command"] = SAFEWALL_CLIENT_LOGON;
		root["context"] = context;
		sentbuf = root.toStyledString();

		if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
		{
			break;
		}

		if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
		{
			break;
		}

		if(true == reader.parse(buffer,reply))
		{
			free(buffer);
			if(0 == stricmp(reply["state"].asCString(),"success"))
			{
				//��¼�ɹ�������������ȡ����
				strcpy(userid, lpContext->userid);
				strcpy(password, lpContext->password);
				LastTime = time(NULL);
				return OnGetAccess();
			}
		}
		//������������ӳɹ��������˺����벻��ȷ֮���
		server = NULL;
		userid[0] = '\0';
		userid2[0] = '\0';
	}while(FALSE);
	
	return error;
}

DWORD OnGetUserID(LPMYSERVCONTEXT lpContext)
{
	BOOL timeout = FALSE;
	if(EffectiveTime > 0)
	{
		timeout = (time(NULL) - LastTime) > EffectiveTime ? TRUE : FALSE;
	}

	if(userid[0] && !timeout)
	{
		//�û��ѵ�¼������δ�������߳�ʱ
		strcpy(lpContext->userid, userid);
		return 0;
	}
	return 1;
}

DWORD OnCancelLogon(void)
{
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	DWORD error;
	char * buffer;

	error = 1;//���������һ����ע���ɹ���
	do
	{
		if(NULL == server)
		{
			//û���û���¼
			break;
		}
		root["command"] = SAFEWALL_CLIENT_CANCEL_LOGON;
		context["userid"] = userid;//����
		root["context"] = context;
		std::string sentbuf = root.toStyledString();

		if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
		{
			break;
		}
		shutdown(server,0);//�رն�
		error = 0;
	}while(FALSE);
	closesocket(server);
	//����ʲô������Ҫ�ˣ�ֱ��ע��
	userid[0] = '\0';
	userid2[0] = '\0';
	password[0] = '\0';
	
	server = NULL;
	//֪ͨ����
	return error;
}

DWORD OnChangePassword(LPMYSERVCONTEXT lpContext)
{
	Json::Value root; 
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	DWORD error;
	char * buffer;

	context["userid"] = lpContext->userid;
	context["password"] = lpContext->password;
	context["newpwd"] = lpContext->newpwd;
	root["command"] = SAFEWALL_CLIENT_CHANGE_PASSWORD;
	root["context"] = context;
	std::string sentbuf = root.toStyledString();

	if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
	{
		return SOCKET_ERROR;
	}

	if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
	{
		return SOCKET_ERROR;
	}

	if(true == reader.parse(buffer,reply))
	{
		free(buffer);
		if(!stricmp(reply["state"].asCString(),"success"))
		{
			//�޸ĳɹ�
			strcpy(password, lpContext->newpwd);
			return 0;
		}
	}
	
	return -2;
}

DWORD OnGetAccess(void)
{
	Json::Value root;
	Json::Value reply;
	Json::Value context;
	Json::Reader reader;
	Json::Value temp;
	DWORD error;
	char *buffer;

	context["userid"] = userid;//����
	root["command"] = SAFEWALL_CLIENT_LOAD_ACCESS;
	root["context"] = context;
	std::string sentbuf = root.toStyledString();

	if(SOCKET_ERROR == mySend(server,(char*)sentbuf.c_str(), sentbuf.length(), &gSection))
	{
		goto end;
	}

	if(SOCKET_ERROR == myRecv(server, &buffer, NULL))
	{
		goto end;
	}
	
	if(true == reader.parse(buffer,reply))
	{
		//mylog(szLogFile,buffer);
		free(buffer);

		temp = reply["access"]["EffectiveTime"];
		if(temp.isNull() || 0 == temp.asInt())
		{
			EffectiveTime = 0;//����ʱ
		}
		else if(MIN_EFFECTIVE_TIME > temp.asInt())
		{
			EffectiveTime = MIN_EFFECTIVE_TIME;
		}
		else
		{
			EffectiveTime = temp.asInt();
		}

		return 0;
	}
	
end:
	return SOCKET_ERROR;
}
