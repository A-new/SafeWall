
#include "SafeWall.h"

MY_LIST_HERDER gProcessListHeader;       //���ܽ�������

#pragma PAGEDCODE
VOID InitializeMyProcessListHead()
{
	InitializeListHead((PLIST_ENTRY)&gProcessListHeader);//��ʼ���ļ�����
	KeInitializeSpinLock(&gProcessListHeader.locker);
	gProcessListHeader.icount = 0;
}
//
//#pragma PAGEDCODE
//SAFEWALL_FILE_LIST * SelectProcessListNode(PFSRTL_COMMON_FCB_HEADER pFcb)
//{
//	SAFEWALL_FILE_LIST * node, * ptr = NULL;
//	LIST_ENTRY *p;
//	KLOCK_QUEUE_HANDLE handle;
//
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb || 
//		pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb )
//	{
//		/*��Ϊ�Ǵ�С�����������ж�pfb�Ƿ��ڼ�����,���򷵻�*/
//		KeReleaseInStackQueuedSpinLock(&handle);
//		return ptr;
//	}
//	p = gFileListHeader.header.Flink ;
//	while( p != &gFileListHeader.header)
//	{
//		node = (PSAFEWALL_FILE_LIST)p;
//		if(node->pFcb == pFcb)
//		{
//			//KdPrint(("%wZ �ļ� fcb �������ҳɹ�", &node->FilePath));
//			ptr = node;
//			break;
//		}
//		//��Ϊ�ǰ�˳���ŵģ����fcb���ڽڵ㣬�ͱ�ʾ�����ڸ�fcb
//		if(pFcb > node->pFcb)
//		{
//			break;
//		}
//		p = p->Flink;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return ptr;
//}
//
//#pragma PAGEDCODE
//BOOLEAN DeleteFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb)
//{
//	BOOLEAN dwRet = FALSE;
//	SAFEWALL_FILE_LIST * node;
//	LIST_ENTRY *p;
//	KLOCK_QUEUE_HANDLE handle;
//
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb || 
//		pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb)
//	{
//		//��Ϊ�Ǵ�С��������ģ����fcb���ڼ����У��򷵻�ʧ��
//		KeReleaseInStackQueuedSpinLock(&handle);
//		return FALSE;
//	}
//
//	p = gFileListHeader.header.Flink;
//	while(p != &gFileListHeader.header)
//	{
//		node = (PSAFEWALL_FILE_LIST)p;
//		if(node->pFcb == pFcb)
//		{
//			p->Blink->Flink = p->Flink;
//			p->Flink->Blink = p->Blink;
//			ExFreePool(node);
//			dwRet = TRUE;
//			--gFileListHeader.icount;
//			break;
//		}
//		//��Ϊ�ǰ�˳���ŵģ����fcb���ڽڵ㣬�ͱ�ʾ�����ڸ�fcb
//		if(pFcb > node->pFcb)
//		{
//			dwRet = FALSE;
//			break;
//		}
//		p = p->Flink;
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return dwRet;
//}
//
//#pragma PAGEDCODE
//SAFEWALL_FILE_LIST * InsertSingleFileListNode(PFSRTL_COMMON_FCB_HEADER pFcb, BOOLEAN * IsHas)
//{
//	//���fcb�Ѿ����ڣ��򲻻���룬���ǻ᷵�ض�Ӧ������ָ�룬ishasҲ�᷵��TRUE
//	SAFEWALL_FILE_LIST * node = NULL;
//	SAFEWALL_FILE_LIST * newNode =NULL;
//	LIST_ENTRY *p = NULL;
//	KLOCK_QUEUE_HANDLE handle;
//	* IsHas = FALSE; //�Ƿ��Ѿ����ڸ�fcb
//	KeAcquireInStackQueuedSpinLock(&gFileListHeader.locker, &handle);
//	if(gFileListHeader.header.Flink ==  &gFileListHeader.header ||
//		((PSAFEWALL_FILE_LIST)gFileListHeader.header.Flink)->pFcb > pFcb)
//	{
//		//KdPrint(("fcb = 0x%08x �����fcb̫С���������ڵ�һλ",pFcb));
//		p =  &gFileListHeader.header;
//	}
//	else if(pFcb > ((PSAFEWALL_FILE_LIST)gFileListHeader.header.Blink)->pFcb)
//	{
//		//KdPrint(("fcb = 0x%08x �����fcb̫�󣬽����������һλ",pFcb));
//		p =  gFileListHeader.header.Blink;
//	}
//	else
//	{
//		p = gFileListHeader.header.Flink;
//		while(p != &gFileListHeader.header)
//		{
//			node = (PSAFEWALL_FILE_LIST)p;
//			if(node->pFcb == pFcb)
//			{
//				newNode = node;
//				* IsHas = TRUE;
//				//KdPrint(("fcb = 0x%08x fcb�Ѵ���",pFcb));
//				break;
//			}
//			//��Ϊ�ǰ�˳���ŵģ�����ڵ����fcb��
//			//�ͱ�ʾ����ڵ��ʺϼ���pcb
//			if(node->pFcb > pFcb)
//			{
//				//KdPrint(("node->pFcb = 0x%08x AND pFcb = 0x%08x ",node->pFcb , pFcb));
//				break;
//			}
//			p = p->Flink;
//		}
//	}
//	
//	if(FALSE == *IsHas)//û�����fcb
//	{
//		newNode = (SAFEWALL_FILE_LIST*)ExAllocatePool(NonPagedPool, sizeof(SAFEWALL_FILE_LIST));
//		if(NULL != newNode)
//		{
//			newNode->pFcb = pFcb;
//			RtlInitEmptyUnicodeString(&newNode->FilePath, newNode->wstr, sizeof(newNode->wstr));
//
//			newNode->list.Flink = p->Flink;
//			newNode->list.Blink = p;
//
//			p->Flink->Blink = (LIST_ENTRY*)newNode;
//			p->Flink = (LIST_ENTRY*)newNode;
//			
//			++gFileListHeader.icount;
//			KdPrint(("�µ�fcb���뵽�����У�0x%08x, �ۼƣ�%d", pFcb,gFileListHeader.icount));
//		}
//	}
//	KeReleaseInStackQueuedSpinLock(&handle);
//	return newNode;
//
//}
//
//#pragma PAGEDCODE
//VOID ReleaseAllFileListNode()
//{
//	LIST_ENTRY * node;
//	//for(node = gFileListHeader.Flink; node != &gFileListHeader; node = node->Flink)
//	//{
//	//	RemoveEntryList((PLIST_ENTRY)node);
//	//	ExFreePool(node);
//	//}
//}