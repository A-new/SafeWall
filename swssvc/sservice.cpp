
#include "swssvc.h"

void WINAPI ServiceProc(DWORD argc, LPTSTR *argv) 
{
	WSADATA wsaData;
	SYSTEM_INFO sysinfo;
	int port = 1808;
	running = TRUE;
	if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) return ;
	HANDLE hCompletion = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);

	GetSystemInfo(&sysinfo);
	for(int i=0; i < sysinfo.dwNumberOfProcessors; i++)
	_beginthreadex(NULL, 0 , ServerThread, (LPVOID)hCompletion, 0,0);

	//SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0,NULL ,0 ,WSA_FLAG_OVERLAPPED);
	SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
	int reuse = 1;
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR ,(char*)&reuse, sizeof(reuse));
	SOCKADDR_IN si;
	si.sin_family = AF_INET;
	si.sin_port = htons(port);
	si.sin_addr.S_un.S_addr = INADDR_ANY;
	
	bind(server,(sockaddr *)&si, sizeof(si));
	listen(server, 100);//windows���listen 200
	
    while(running)
    {   
		SOCKADDR_IN clientAddr;
		int addrlen = sizeof(clientAddr);

		SOCKET client = accept(server,(SOCKADDR*)&clientAddr, &addrlen);
		if(NULL == client)
		{
			continue;
		}
		PPER_HANDLE_DATA pPerHandle = (PPER_HANDLE_DATA)_HeapAlloc(sizeof(PER_HANDLE_DATA));
		pPerHandle->socket = client;
		memcpy(&pPerHandle->addr, &clientAddr, addrlen);
		if(!CreateIoCompletionPort((HANDLE)pPerHandle->socket, hCompletion,(DWORD)pPerHandle,0))
		{
			continue;
		}
		//HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, BUFFER_SIZE);
		PPER_IO_DATA pPerIO = (PPER_IO_DATA) _HeapAlloc(sizeof(PER_IO_DATA));

		RecvProc(pPerHandle,pPerIO,NULL);
    }

	closesocket(server);

	WSACleanup();
	return;
}


UINT WINAPI ServerThread(LPVOID lpParam)
{
	HANDLE hCompletion = (HANDLE)lpParam;
	DWORD dwTrans;
	PPER_HANDLE_DATA pPerHandle;
	PPER_IO_DATA pPerIO;
	DWORD nFlags = 0;
	//mylog("GetQueuedCompletionStatus ������ɶ˿�״̬����");
	DWORD dwRet = -1 ;
	while(running)
	{
		BOOL bOK = GetQueuedCompletionStatus(hCompletion,
			&dwTrans,(LPDWORD)&pPerHandle,(LPOVERLAPPED*)&pPerIO,WSA_INFINITE);

		if(!bOK || (dwTrans ==0 && (pPerIO->dwOperationType == OP_READ || pPerIO->dwOperationType == OP_WRITE)))
		{
			//mylog("�µ�IO���������Ƿ�������");
			closesocket(pPerHandle->socket);
			if(pPerHandle->dwUser == USER_CLIENT)
			{
				DeleteUserListNode(&gUserList,pPerHandle);
			}
			
			HeapFree(GetProcessHeap(),NULL,pPerHandle);
			HeapFree(GetProcessHeap(),NULL,pPerIO->buffer);
			HeapFree(GetProcessHeap(),NULL,pPerIO);
			continue;
		}
		//MYLOG("�µ�IO����");
		switch(pPerIO->dwOperationType)
		{
		case OP_READ:
			{
				//MYLOG("���� RecvProc");
				RecvProc(pPerHandle, pPerIO, dwTrans);
			}break;
		case OP_WRITE:
			{
				//MYLOG("���� SendProc");
				SendProc(pPerHandle, pPerIO, dwTrans);
			}break;
		case OP_ACCEPT:
			{
			}break;
		default:
			{
			}break;
		}
	}
	//MYLOG("GetQueuedCompletionStatus �˳�");
	return 0 ;
}

void RecvProc(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans)
{
	WSABUF buffer;
	DWORD nFlags = 0;

	if(0 == dwTrans)//ֻ�е�һ�ε��õ�ʱ��Ż��յ� 0
	{
		pPerIO->buffer = NULL;
		pPerIO->size = NULL;
		pPerIO->offset = 0 - sizeof(pPerIO->size);
		pPerIO->dwOperationType = OP_READ;

		buffer.buf = (char*)&pPerIO->size;
		buffer.len = sizeof(pPerIO->size);
		WSARecv(pPerHandle->socket, &buffer,1,NULL, &nFlags,&pPerIO->ol, NULL);
		return;
	}

	if(NULL == pPerIO->buffer)
	{
		if(0 == pPerIO->size)//���һ��������
		{
			RecvProc(pPerHandle,pPerIO, NULL);
			return;
		}
		//���յ��°��ĳ��ȣ������ڴ�
		pPerIO->size = ntohl(pPerIO->size);
		pPerIO->buffer = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ALIGNLENGHT(pPerIO->size));
		pPerIO->offset = 0;
		if(pPerIO->buffer == NULL) return;

		buffer.buf = pPerIO->buffer;
		buffer.len = pPerIO->size > BUFFER_SIZE ? BUFFER_SIZE : pPerIO->size;
		WSARecv(pPerHandle->socket, &buffer, 1, NULL, &nFlags, &pPerIO->ol, NULL);
		return;
	}

	pPerIO->offset += dwTrans;
	if(pPerIO->size > pPerIO->offset)
	{
		//���ݰ�δ�����꣬��������
		buffer.buf = (char *) pPerIO->buffer + pPerIO->offset;
		buffer.len = pPerIO->size - pPerIO->offset > BUFFER_SIZE ? BUFFER_SIZE : pPerIO->size - pPerIO->offset;
		WSARecv(pPerHandle->socket, &buffer, 1, NULL, &nFlags, &pPerIO->ol, NULL);
		return;
	}

	//�������
	pPerIO->buffer[pPerIO->offset] = '\0';
	
	//MYLOG("������ɣ�����worker����");

	//�Խ��յ������ݽ��з���
	worker(pPerHandle,pPerIO);
	return ;
}

void SendProc(PPER_HANDLE_DATA pPerHandle, PPER_IO_DATA pPerIO, DWORD dwTrans)
{
	WSABUF buffer;
	DWORD nFlags = 0;
	long size;

	if(dwTrans == 0)//ֻ�е�һ�ε��õ�ʱ��Ż��յ� 0
	{
		size = htonl(pPerIO->size);
		pPerIO->offset = 0 - sizeof(size);
		buffer.buf = (char*)&size;
		buffer.len = sizeof(size);
		pPerIO->dwOperationType = OP_WRITE;
		WSASend(pPerHandle->socket, &buffer, 1, NULL, 0, &pPerIO->ol, NULL);
		return;
	}

	pPerIO->offset += dwTrans;
	if(pPerIO->size > pPerIO->offset)
	{
		//MYLOG("δ�ӷ�����ɣ���������");
		buffer.buf = (char*) pPerIO->buffer + pPerIO->offset;
		buffer.len = pPerIO->size - pPerIO->offset > BUFFER_SIZE ? BUFFER_SIZE : pPerIO->size - pPerIO->offset;
		WSASend(pPerHandle->socket, &buffer, 1, NULL, 0, &pPerIO->ol, NULL);
		return;
	}

	HeapFree(GetProcessHeap(),NULL, pPerIO->buffer);
	//MYLOG("������ɣ��ȴ�������һ����Ϣ ");
	RecvProc(pPerHandle,pPerIO, NULL);
	
	return ;
}
