
#include "SafeWall.h"
#include "myfs.h"

#pragma PAGEDCODE
NTSTATUS  DriverCreateDispatchRoutine( IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp )
{
    NTSTATUS status;
	KEVENT waitEvent;
	DWORD FileFlags;
	DWORD ProcFlags;

	UNICODE_STRING srcFileName;
	UNICODE_STRING tarFileName;
	WCHAR tarfilename[MY_MAX_PATH];
	WCHAR srcfilename[MY_MAX_PATH];
	DWORD dwDesiredAccess ;        //����ģʽ����дɾ��
	DWORD dwCreationDisposition ;  //��δ������򿪻򴴽��ļ�
	DWORD dwCreateOptions ;        //��ģʽ��Ŀ¼����Ŀ¼��ͬ�����첽
	DWORD dwShareMode ;            //����ģʽ��������������д��ɾ����
	DWORD dwFileAttributes ;       //�ļ����ԣ����ء�ֻ��
	LPSAFEWALL_OBJECT lpSafeWall;
	PFSRTL_COMMON_FCB_HEADER pFcb;
	PSAFEWALL_FILE_LIST pFileList;

	FILE_OBJECT FileObject;
	BOOLEAN IsHas;
    //PAGED_CODE();

    if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("�ҵ�����������һ�����ļ�����Ϣ\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

    //ASSERT(IS_MY_DEVICE_OBJECT( pDeviceObject ));

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;
	PDEVICE_OBJECT storageStackDeviceObject = stack->Parameters.MountVolume.Vpb->RealDevice;//�����豸����

	RtlInitEmptyUnicodeString(&srcFileName, srcfilename, sizeof(srcfilename));
	RtlInitEmptyUnicodeString(&tarFileName, tarfilename, sizeof(tarfilename));

    //��ʼ���¼���������������̡�
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	//����IoCallDriver�к�������ʱ����Ҫʹ��IoCopyCurrentIrpStackLocationToNext
    IoCopyCurrentIrpStackLocationToNext( pIrp );
	//�����������
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//��״̬�� ����
    {
        KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }

	//��Ceatea��Ϣ�У���Щ���������²���п��ܱ��޸ģ�����Ҫ�ȵ�
	//�²���÷���ʱ���õ�����Ϣ��������ʵ����Ϣ��
	//ΪʲôҪ���¼��ȴ�������������������е�������ķ�����
	//��Ϊ������̵����м���̫�ߣ����������������Ժܶ�ʱ��
	//���Ǵ���һ���¼�����ɺ���������ɺ����ɹ����ټ���ִ�в���
    //ASSERT(KeReadStateEvent(&waitEvent) || !NT_SUCCESS(pIrp->IoStatus.Status));

	if(!NT_SUCCESS(pIrp->IoStatus.Status) || (stack->Parameters.Create.Options & FILE_DIRECTORY_FILE))
	{
		//�������ʧ�ܻ���Ŀ�����·����־��ֱ�ӷ���
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		//KdPrint(("����ʧ�ܻ�filename���ļ���status=0x%08x��dwCreateOptions=0x%08x ",status,dwCreateOptions));
		return status;
	}
	
	dwDesiredAccess = stack->Parameters.Create.SecurityContext->DesiredAccess; //����ģʽ����дɾ��
	dwCreationDisposition = (stack->Parameters.Create.Options>>24);            //��δ������򿪻򴴽��ļ�
	dwCreateOptions = (stack->Parameters.Create.Options & 0x00ffffff);         //��ģʽ��Ŀ¼����Ŀ¼��ͬ�����첽
	dwShareMode = stack->Parameters.Create.ShareAccess;                        //����ģʽ��������������д��ɾ����
	dwFileAttributes = stack->Parameters.Create.FileAttributes;                //�ļ����ԣ����ء�ֻ��
	//��������ǳɹ��ˣ�ִ�����ǵĹ��˲���
	/*
		��ԭ����FILEOBJECT���͵��²��ȡ�ļ����ᷢ��һЩ��ֵ����飬�������û����治�ܴ�
		��ȡ�����ļ�����FILEOBJECT����һ���������͵��²�ͽ��������ļ�������ԭ������Ϊ
		���ļ���дʱ����д��FILEOBJECT��ĳЩ����
		����FileObject.CurrentByteOffset.QuadPart�Ǹı�֮һ
	*/
	
	if((FILE_OPEN & dwCreationDisposition) && (dwCreateOptions & FILE_NON_DIRECTORY_FILE))//����Ǵ�һ���ļ�
	{
		FileObject = *pFileObject;
		/*lpSafeWall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool, SAFEWALL_OBJECT_SIZE);
		FileFlags = GetFileSafeWallFlags(&FileObject,
			((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject,
			lpSafeWall);*/
		RtlAppendUnicodeToString(&tarFileName, L"safewall.txt");
		GetFileNameForPath(&FileObject.FileName , &srcFileName);
		FileFlags = NULL;
		if(0 == RtlCompareUnicodeString(&srcFileName,&tarFileName, TRUE))
		{
			FileFlags = SAFEWALL_FLAG_FILEGROUP | SAFEWALL_FLAG_OBJECT | SAFEWALL_FLAG_MANAGEMENT;
		}
		
		if(FileFlags & SAFEWALL_FLAG_FILEGROUP)
		{
			//KdPrint(("%wZ�Ǽ����ļ�", &FileObject.FileName));
			pFcb = (PFSRTL_COMMON_FCB_HEADER)FileObject.FsContext;
			pFileList = InsertSingleFileListNode((PFSRTL_COMMON_FCB_HEADER)FileObject.FsContext,&IsHas);
			if(FALSE == IsHas)
			{
				//�������û�еģ���������������ȥ
				pFileList->flags = FileFlags;
				/*pFcb->FileSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
				pFcb->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;*/
			}
			PSECTION_OBJECT_POINTERS SectionObjectPointer = pFileObject->SectionObjectPointer;

			KdPrint(("dwDesiredAccess=0x%08x",dwDesiredAccess));
			KdPrint(("dwCreationDisposition=0x%08x",dwCreationDisposition));
			KdPrint(("dwCreateOptions=0x%08x",dwCreateOptions));
			KdPrint(("dwShareMode=0x%08x",dwShareMode));
			KdPrint(("dwFileAttributes=0x%08x",dwFileAttributes));
		}
		//ExFreePool(lpSafeWall);
	}

    status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;

}


#pragma PAGEDCODE
ULONG GetFileObjectFullPath( PFILE_OBJECT FileObject, PUNICODE_STRING FilePath )
{
	NTSTATUS status;
	POBJECT_NAME_INFORMATION  obj_name_info = NULL;
	WCHAR buffer[64] = { 0 };
	void *objptr;
	ULONG length = 0;
	BOOLEAN need_split = FALSE;

	ASSERT( FileObject != NULL );
	if(FileObject == NULL)
		return 0;
	if(FileObject->FileName.Buffer == NULL)
		return 0;

	obj_name_info = (POBJECT_NAME_INFORMATION)buffer;
	do {

		// ��ȡFileNameǰ��Ĳ��֣��豸·�����߸�Ŀ¼·����
		if(FileObject->RelatedFileObject != NULL)
			objptr = (void *)FileObject->RelatedFileObject;
		else
			objptr= (void *)FileObject->DeviceObject;
		status = ObQueryNameString(objptr,obj_name_info,64*sizeof(WCHAR),&length);
		if(status == STATUS_INFO_LENGTH_MISMATCH)
		{
			obj_name_info = (POBJECT_NAME_INFORMATION)ExAllocatePool(NonPagedPool,length);
			if(obj_name_info == NULL)
				return STATUS_INSUFFICIENT_RESOURCES;
			RtlZeroMemory(obj_name_info,length);
			status = ObQueryNameString(objptr,obj_name_info,length,&length);            
		}
		// ʧ���˾�ֱ����������
		if(!NT_SUCCESS(status))
			break;

		// �ж϶���֮���Ƿ���Ҫ��һ��б�ܡ�����Ҫ��������:
		// FileName��һ���ַ�����б�ܡ�obj_name_info���һ��
		// �ַ�����б�ܡ�
		if( FileObject->FileName.Length > 2 &&
			FileObject->FileName.Buffer[ 0 ] != L'\\' &&
			obj_name_info->Name.Buffer[ obj_name_info->Name.Length / sizeof(WCHAR) - 1 ] != L'\\' )
			need_split = TRUE;

		// ���������ֵĳ��ȡ�������Ȳ��㣬Ҳֱ�ӷ��ء�
		length = obj_name_info->Name.Length + FileObject->FileName.Length;
		if(need_split)
			length += sizeof(WCHAR);
		if(FilePath->MaximumLength < length)
			break;

		// �Ȱ��豸��������ȥ��
		RtlCopyUnicodeString(FilePath,&obj_name_info->Name);
		if(need_split)
			// ׷��һ��б��
			RtlAppendUnicodeToString(FilePath,L"\\");

		// Ȼ��׷��FileName
		RtlAppendUnicodeStringToString(FilePath,&FileObject->FileName);
	} while(0);

	// ���������ռ���ͷŵ���
	if((void *)obj_name_info != (void *)buffer)
		ExFreePool(obj_name_info);
	return length;
}