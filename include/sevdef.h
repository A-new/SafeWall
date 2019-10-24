
#ifndef __SEVDEF_H
#define __SEVDEF_H

#define szClientMapName  "{1153BFB1-4EFF-4496-B663-6050CF71F4FE}" //�ͻ��˽��������ͨ�ŵĹ����ڴ�����
#define szClientMutexName "{F770230D-8FB1-49E4-B049-032A38421693}"//�ͻ�����Ϣ���л�

#define szClientServiceName  L"swcsvc" //��������
#define szClientDescriptionName L"����Χǽ�ͻ��˺�̨����"  //����˵��

#define szServerServiceName   L"swssvc"  //��������
#define szServerDescriptionName   L"����Χǽ����˺�̨����"; //����˵��

#define MY_SERVICE_CONTROL_LOGON               (SERVICE_USER_DEFINED_CONTROL - 1) //��¼
#define MY_SERVICE_CONTROL_GET_USERID          (SERVICE_USER_DEFINED_CONTROL - 2) //ȡ�õ�ǰ��¼�û���Ϣ
#define MY_SERVICE_CONTROL_CANCEL_LOGON        (SERVICE_USER_DEFINED_CONTROL - 3) //ע��
#define MY_SERVICE_CONTROL_CHANGE_PASSWORD     (SERVICE_USER_DEFINED_CONTROL - 4) //�޸�����
#define MY_SERVICE_CONTROL_GET_ACCESS          (SERVICE_USER_DEFINED_CONTROL - 5) //ȡ�ü��ܲ���
//#define MY_SERVICE_CONTROL_CLOSE             (SERVICE_USER_DEFINED_CONTROL + 5) //�رշ���
//

typedef struct _MYSERVCONTEXT{
	long* fromHwnd;
	int  userType;
	char userid[64];
	char password[64];
	char newpwd[64];
	long error;
}MYSERVCONTEXT, *LPMYSERVCONTEXT;


#ifndef BUFFER_SIZE
#define BUFFER_SIZE  4096
#endif

#ifndef ALIGNLENGHT
#define ALIGNLENGHT(len) ((((len+sizeof(char)) / BUFFER_SIZE) + \
	((len+sizeof(char)) % BUFFER_SIZE ? 1 : 0)) * BUFFER_SIZE)
#endif

#define _HeapAlloc(size)  HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, ALIGNLENGHT((size)))
#define _HeapFree(buffer)  HeapFree(GetProcessHeap(),NULL, (buffer))

typedef struct _NET_BUFFER{
	long buflen;
	char buffer[1];
}NETBUFFER, * PNETBUFFER;

#define SOCKET_ERROR_IOCP_NOERR    997
#define SOCKET_ERROR_CLOSE_READ    10058
#define SOCKET_ERROR_CLOSE_WRITE   -1

//_PER_IO_DATA::dwOperationType
#define OP_READ      1
#define OP_WRITE     2
#define OP_ACCEPT    3

//_PER_IO_DATA::dwUser
#define USER_CLIENT     1   //�ͻ���
#define USER_SERVER     2   //���ع���
#define USER_IC         3   //����KEY
#define USER_CONTROL    4   //δ֪

//ͨ������ json["command"]
#define  SAFEWALL_CLIENT_LOGON                 1
#define  SAFEWALL_CLIENT_CANCEL_LOGON          2
#define  SAFEWALL_CLIENT_LOAD_ACCESS           3
#define  SAFEWALL_CLIENT_CHANGE_PASSWORD       4

#define  SAFEWALL_IC_LOGON                     401
#define  SAFEWALL_IC_CANCEL_LOGON              402
#define  SAFEWALL_IC_GET_ACCESS                403

#define  SAFEWALL_SERVER_LOGON                 801
#define  SAFEWALL_SERVER_GET_ACCESS            802
#define  SAFEWALL_SERVER_CANCEL_LOGON          803
#define  SAFEWALL_SERVER_STOP_SERVICE          804
#define  SAFEWALL_SERVER_GET_FILE_ACCESS       805
#define  SAFEWALL_SERVER_SET_FILE_ACCESS       806
#define  SAFEWALL_SERVER_GET_PROCESS_ACCESS    807
#define  SAFEWALL_SERVER_SET_PROCESS_ACCESS    808
#define  SAFEWALL_SERVER_GET_USERLIST          809
#define  SAFEWALL_SERVER_ADD_USER              810
#define  SAFEWALL_SERVER_GET_USERINFO          811
#define  SAFEWALL_SERVER_SET_USERINFO          812

#define  FAILEDMSG  "{\"command\": \"reply\",\"state\": \"failed\"}"
#define  SUCCESSMSG  "{\"command\": \"reply\",\"state\": \"success\"}"


#endif