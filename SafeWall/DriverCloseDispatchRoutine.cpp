
#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS  DriverCloseDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	UNICODE_STRING ProcName;
	UNICODE_STRING FilePath;
	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;

    PAGED_CODE();

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject))
    {
		KdPrint(("�ҵ�����������һ���ر��ļ�����Ϣ\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;
	
	/*if(pFileObject == NULL)
	{
		KdPrint(("pFileObject�ǿյ�"));
	}
	else
	{
		FCB * pFcb = (FCB*)pFileObject->FsContext;
		PSAFEWALL_FILE_LIST pFileList = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
		if(pFileList != NULL && (pFileList->flags & SAFEWALL_FLAG_FILEGROUP))
		{
			KdPrint(("pFileList�ҵ�����������һ���ر��ļ�����Ϣ��UncleanCount:%d��OpenCount:%d��NonCachedUncleanCount:%d", \
				pFcb->UncleanCount,pFcb->OpenCount,pFcb->NonCachedUncleanCount));
		}
	}*/

	/*RtlAppendUnicodeToString(&tarFileName, L"config.txt");
	GetFileNameForPath(&pFileObject->FileName , &srcFileName);
	if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		GetCurrentProcessName(&ProcName);
		KdPrint(("���֡���%02xһ���ļ���%wZ ������ %wZ��  Options��0x%08x",stack->MajorFunction, \
			&pFileObject->FileName,&ProcName, stack->Parameters.Create.Options ));
	}*/

	/*RtlAppendUnicodeToString(&tarFileName, L"test.txt");
	GetFileNameForPath(&pFileObject->FileName , &srcFileName);
	if(RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	{
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return status;
	}*/

    //��ʼ���¼���������������̡�
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//����IoCallDriver�к�������ʱ����Ҫʹ��IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext(pIrp);

	//�����������
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//��״̬�� ����
    {
        NTSTATUS localStatus = KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }

	//if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
	//{
	//	GetCurrentProcessName(&ProcName);
	//	KdPrint(("���֡���%02xһ���ļ���%wZ ������ %wZ��  Options��0x%08x",stack->MajorFunction, \
	//		&pFileObject->FileName,&ProcName, stack->Parameters.Create.Options ));
	//}

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}
