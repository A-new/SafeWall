
#include "swssvc.h"

void InitUserList(PUSERLIST_HEADER UserList)
{
	UserList->header.Blink = &UserList->header;
	UserList->header.Flink = &UserList->header;
	InitializeCriticalSection(&UserList->section);//�ٽ���
	UserList->icount =0;
}

void ReleaseUserList(PUSERLIST_HEADER UserList)
{
	UserList->header.Blink = &UserList->header;
	UserList->header.Flink = &UserList->header;
	DeleteCriticalSection(&UserList->section);
	UserList->icount =0;
}

long GetUserListCount(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle)
{
	EnterCriticalSection(&UserList->section);
	LeaveCriticalSection(&UserList->section);
	return UserList->icount;
}

void SelectUserListNode(PUSERLIST_HEADER UserList)
{
	EnterCriticalSection(&UserList->section);
	LeaveCriticalSection(&UserList->section);
}

BOOL DeleteUserListNode(PUSERLIST_HEADER UserList, PPER_HANDLE_DATA pPerHandle)
{
	PER_HANDLE_DATA * node, * ptr = NULL;
	LIST_ENTRY *p;
	BOOL bRet;

	EnterCriticalSection(&UserList->section);

	if(UserList->header.Flink ==  &UserList->header ||
		(PPER_HANDLE_DATA)UserList->header.Flink > pPerHandle || 
		pPerHandle > (PPER_HANDLE_DATA)UserList->header.Blink)
	{
		//��Ϊ�Ǵ�С��������ģ����pPerHandle���ڼ����У��򷵻�ʧ��
		LeaveCriticalSection(&UserList->section);
		return FALSE;
	}

	p = UserList->header.Flink;
	while(p != &UserList->header)
	{
		node = (PPER_HANDLE_DATA)p;
		if(node == pPerHandle)
		{
			p->Blink->Flink = p->Flink;
			p->Flink->Blink = p->Blink;
			bRet = TRUE;
			--UserList->icount;
			break;
		}
		//��Ϊ�ǰ�˳���ŵģ����pPerHandle���ڽڵ㣬�ͱ�ʾ�����ڸ�pPerHandle
		if(pPerHandle > node)
		{
			bRet = FALSE;
			break;
		}
		p = p->Flink;
	}
	LeaveCriticalSection(&UserList->section);
	return bRet;
}

BOOL InserUserListNode(PUSERLIST_HEADER UserList,PPER_HANDLE_DATA pPerHandle )
{
	PER_HANDLE_DATA * node = NULL;
	LIST_ENTRY * p;
	BOOL has = FALSE;
	EnterCriticalSection(&UserList->section);

	if(UserList->header.Flink ==  &UserList->header ||
		(PPER_HANDLE_DATA)UserList->header.Flink > pPerHandle)
	{
		//�����pPerHandle̫С���������ڵ�һλ
		p =  &UserList->header;
	}
	else if(pPerHandle > (PPER_HANDLE_DATA)UserList->header.Blink)
	{
		//�����pPerHandle̫�󣬽����������һλ
		p =  UserList->header.Blink;
	}
	else
	{
		p = UserList->header.Flink;
		while(p != &UserList->header)
		{
			node = (PPER_HANDLE_DATA)p;
			if(node == pPerHandle)
			{
				has = TRUE;
				//pPerHandle�Ѵ��ڣ�������Ϊ���ݴ������߼�������
				//���ᷢ���������ε�
				break;
			}
			//��Ϊ�ǰ�˳���ŵģ�����ڵ����pPerHandle��
			//�ͱ�ʾ����ڵ��ʺϼ���pPerHandle
			if(node > pPerHandle)
			{
				break;
			}
			p = p->Flink;
		}
	}

	if(FALSE == has)//û�����pPerHandle
	{
		pPerHandle->list.Flink = p->Flink;
		pPerHandle->list.Blink = p;

		p->Flink->Blink = (LIST_ENTRY*)pPerHandle;
		p->Flink = (LIST_ENTRY*)pPerHandle;
			
		++UserList->icount;
	}
	
	LeaveCriticalSection(&UserList->section);
	return !has;
}