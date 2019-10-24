

#include "SafeWall.h"


// ������
#pragma PAGEDCODE
void FileSystemCacheClear(PFILE_OBJECT pFileObject)
{
   PFSRTL_COMMON_FCB_HEADER pFcb;
   LARGE_INTEGER liInterval;
   BOOLEAN bNeedReleaseResource = FALSE;
   BOOLEAN bNeedReleasePagingIoResource = FALSE;
   KIRQL irql;
   PFCB p;

   pFcb = (PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext;
   if(pFcb == NULL)
       return;

   //���ص�ǰ�жϼ���
   irql = KeGetCurrentIrql();
   if (irql >= DISPATCH_LEVEL)
   {
       return;
   }

   liInterval.QuadPart = -1 * (LONGLONG)50;

   while (TRUE)
   {
       BOOLEAN bBreak = TRUE;
       BOOLEAN bLockedResource = FALSE;//��ǰ�߳��Ƿ���ж�ռ���ʸ�������Դ
       BOOLEAN bLockedPagingIoResource = FALSE;
       bNeedReleaseResource = FALSE;
       bNeedReleasePagingIoResource = FALSE;

	   // ��fcb��ȥ������
	   //��ҳIO��Դ
       if (pFcb->PagingIoResource)    //���Ӧ���ǻ�����,�����Ƿ�ΪNULL
		   //���ص�ǰ�߳��Ƿ���ж�ռ���ʸ�������Դ��
           bLockedPagingIoResource = ExIsResourceAcquiredExclusiveLite(pFcb->PagingIoResource);

	   //�鿴ԪPageingIoResource������ٲ鿴Resource���壬 ��֮һ��Ҫ�õ��������
       if (pFcb->Resource)//�洢������������ǻ���ָ��
       {
           bLockedResource = TRUE; 
		   //��pFcb->Resource �Ƿ��ж�ռ����Ȩ��
           if (ExIsResourceAcquiredExclusiveLite(pFcb->Resource) == FALSE)
           {
			   //�����ǰ�̶߳���Դ���������ж�ռ���ʵ�Ȩ��
               bNeedReleaseResource = TRUE;//�����ͷ���Դ
               if (bLockedPagingIoResource)//�����ҳ��Դ��������
               {
				   //ExAcquireResourceExclusiveLite�ǵ�ǰ�̻߳�ö�ռ����ָ����Դ��Ȩ�ޣ�
				   //ָ����Դ����������ȡʱ�ĳ�����Ϊ��
				   //���TRUE�����÷�������ȴ�״̬��ֱ����Դ���Ի�á�
				   //���FALSE���������������أ�������Դ�Ƿ���Ի�á�
                   if (ExAcquireResourceExclusiveLite(pFcb->Resource, FALSE) == FALSE)
                   {
                       bBreak = FALSE;//��Ϊ��������ѭ����ʧ�ܵĻ���������ѭ������һ�μ�����ѯȨ��
                       bNeedReleaseResource = FALSE;//û��ȡ�ö�ռȨ�ޣ������֮Ϊ����������Ҳ����Ҫ�ͷű�־
                       bLockedResource = FALSE; //��Դ�����ı�־ΪFALSE
                   }
               }
               else//�����ҳ��Դδ����
				   //�����ȡ��ռ����Ȩ�ޣ�ֱ���ɹ�Ϊֹ
                   ExAcquireResourceExclusiveLite(pFcb->Resource, TRUE);
           }
       }
   
	   //�����ҳ��Դδ����
       if (bLockedPagingIoResource == FALSE)
       {
           if (pFcb->PagingIoResource)
           {
               bLockedPagingIoResource = TRUE;
               bNeedReleasePagingIoResource = TRUE;

			   //�����Դ�������ģ���ô����ExAcquireResourceExclusiveLite���г����Ի�ȡȨ��
			   //�������δ�����ģ���һֱ�ȵ���ռ�ɹ�Ϊֹ
               if (bLockedResource)
               {
                   if (ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, FALSE) == FALSE)
                   {
                       bBreak = FALSE;
                       bLockedPagingIoResource = FALSE;
                       bNeedReleasePagingIoResource = FALSE;//û��ȡ�ɹ����Ͳ����ͷ���Դ��
                   }
               }
               else
               {
                   ExAcquireResourceExclusiveLite(pFcb->PagingIoResource, TRUE);
               }
           }
       }

       if (bBreak)//������������õ����ˣ�����ѭ��
       {
           break;
       }
       //��������Ϊֻ�õ��������һ����������Ҫ�ͷ��������»�ȡ
       if (bNeedReleasePagingIoResource)//�����Դ֮ǰ��ռ�ɹ��ˣ���ô�����Ҫ�ͷ�
       {
           ExReleaseResourceLite(pFcb->PagingIoResource);
       }
       if (bNeedReleaseResource)//�����Դ֮ǰ��ռ�ɹ��ˣ���ô�����Ҫ�ͷ�
       {
           ExReleaseResourceLite(pFcb->Resource);
       }

	   //�����PASSIVE_LEVEL��������KeDelayExecutionThread�ӳ�
	   //������event�ӳ�
       if (irql == PASSIVE_LEVEL)
       {
		   
           KeDelayExecutionThread(KernelMode, FALSE, &liInterval);
       }
       else
       {
           KEVENT waitEvent;
           KeInitializeEvent(&waitEvent, NotificationEvent, FALSE);
           KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, &liInterval);
       }
   }

   //PBCB Bcb = pFileObject->SectionObjectPointer->SharedCacheMap;
   //PMM_SECTION_SEGMENT Segment = (PMM_SECTION_SEGMENT)pFileObject->SectionObjectPointer->DataSectionObject;
   //PMM_IMAGE_SECTION_OBJECT ImageSectionObject = (PMM_IMAGE_SECTION_OBJECT)pFileObject->SectionObjectPointer->ImageSectionObject;
   //
   //���ϵͳ���Ѿ������˸��ļ����ļ����壬��ôӦ�ò�Ķ�д���������ᱻת��ΪFast IO����
   //ָ���ļ������ֻ���ڶ����ָ�롣�˳�Ա�����ļ�ϵͳ���ã����ڸ��ٻ��������������
   if (pFileObject->SectionObjectPointer)
   {
		IO_STATUS_BLOCK ioStatus;
		CcFlushCache(pFileObject->SectionObjectPointer, NULL, 0, &ioStatus);//�������
		//�������Ὣ�����ļ���ȫ���򲿷�ˢ�µ����̡�
		if (pFileObject->SectionObjectPointer->ImageSectionObject)
		{
			//������񲿷ֲ�Ϊ�գ���ˢ�¾����
			//MmFlushForWrite���ڴ򿪾���Σ����ṩ����
			//��������ڴ�
			MmFlushImageSection(pFileObject->SectionObjectPointer,MmFlushForWrite); // MmFlushForDelete
		}
		//��ϵ�y��������������ļ���ȫ���򲿷�
		CcPurgeCacheSection(pFileObject->SectionObjectPointer, NULL, 0, FALSE);
   }

   if (bNeedReleasePagingIoResource)//�����Դ֮ǰ��ռ�ɹ��ˣ���ô�����Ҫ�ͷ�
   {
       ExReleaseResourceLite(pFcb->PagingIoResource);
   }
   if (bNeedReleaseResource)//�����Դ֮ǰ��ռ�ɹ��ˣ���ô�����Ҫ�ͷ�
   {
       ExReleaseResourceLite(pFcb->Resource);
   }
}

