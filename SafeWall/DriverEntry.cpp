

#include "SafeWall.h"

EXTERN_C ULONG g_process_name_offset = 0;
EXTERN_C BOOLEAN g_safewall_start = FALSE;

EXTERN_C ULONG gOsMajorVersion = 0;
EXTERN_C ULONG gOsMinorVersion = 0;

EXTERN_C PDRIVER_OBJECT gMyDriverObject = NULL;        //������I/O���������ɲ��������������
EXTERN_C PDEVICE_OBJECT gMyControlDeviceObject = NULL; //�����ɱ������������ɵĿ����豸����
EXTERN_C FAST_MUTEX gFastMutexAttachLock = {0};        //����һ�����ٻ���ṹ����(����),���ؾ�ʱ�õ�
EXTERN_C LPSAFEWALL_OBJECT gStandardSafeWallObj  = NULL;      //�Ѿ��Ѿ���ʽ����salewall����


PFILE_OBJECT pTestID = NULL;

#pragma INITCODE
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	//���裺
	//1.ȡ��ϵͳ�汾
	//2.���÷ַ�����
	//���²�����AddDriver������
	//1.�����豸���������豸�Ŀ���IO�ַ�����
	//2.�����ļ�ϵͳ�䶯֪ͨ���������Ұ��ѹ��ص��ļ�ϵͳ���ӵ��豸
	//3.��δ��ʽ����CD�ʹ���Ҳ���ӵ��豸(��Ϊδ��ʽ�����豸��δ���ص��κ��ļ�ϵͳ��
	//  ���Բ����յ�֪ͨ��Ҫ���⴦��)
	NTSTATUS status = STATUS_SUCCESS;
	KdPrint(("Enter DriverEntry\n"));

	gMyDriverObject = pDriverObject;

	RTL_OSVERSIONINFOW versionInfo = {0};
    versionInfo.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOW );
	status = RtlGetVersion(&versionInfo);//ȡ�õ�ǰϵͳ�汾
    ASSERT( NT_SUCCESS( status ) );
	gOsMajorVersion = versionInfo.dwMajorVersion;
	gOsMinorVersion = versionInfo.dwMinorVersion;

	InitMySafeWallObject(&gStandardSafeWallObj);
	InitializeMyFileListHead();//��ʼ���ļ�����
	InitializeMyProcessListHead();//��ʼ����������

	ExInitializeFastMutex( &gFastMutexAttachLock );//��ʼ��"FastMutex(���ٻ���)"����,�Ժ���߳�ֻ�ܻ��������

	//ȡ�ý�������ƫ��λ��
	ULONG offset;
	PEPROCESS curproc;
	curproc = PsGetCurrentProcess();
	ULONG offset_end = 3 * 4 * 1024;

	for(offset = 0; offset < offset_end; offset++)
	{
		if(!strncmp("System",(PCHAR)curproc + offset, strlen("System")))
		{
			g_process_name_offset = offset;
			KdPrint(("g_process_name_offset = %d",g_process_name_offset));
			break;
		}
	}

	pDriverObject->DriverExtension->AddDevice = AddDevice;
	pDriverObject->DriverUnload = DriverUnload;

	for (int i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)                                                                
    {
        pDriverObject->MajorFunction[i] = DispatchRoutine;                                                                
    }
	//ע�������ǲ����
    //pDriverObject->MajorFunction[IRP_MJ_CREATE] = DriverCreateDispatchRoutine;
    pDriverObject->MajorFunction[IRP_MJ_CREATE_NAMED_PIPE] = DispatchRoutine;//�����ܵ�
    pDriverObject->MajorFunction[IRP_MJ_CREATE_MAILSLOT] = DispatchRoutine;//�Ͳ�
	//pDriverObject->MajorFunction[IRP_MJ_READ] = DriverReadDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_WRITE] = DriverWriteDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = DriverCleanUpDispatchRoutine;
    //pDriverObject->MajorFunction[IRP_MJ_CLOSE] = DriverCloseDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_FILE_SYSTEM_CONTROL] = FileSystemDeviceControl;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DriverControlDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_QUERY_INFORMATION] = DriverQueryInformationDispatchRoutine;
	//pDriverObject->MajorFunction[IRP_MJ_SET_INFORMATION] = DriverSetInformationDispatchRoutin;
	//pDriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL] = DriverDirectoryControlDispatchRoutine;
	KdPrint(("DriverEntry end\n"));

	return status;
}


#pragma PAGEDCODE
NTSTATUS AddDevice(IN PDRIVER_OBJECT pDriverObject,IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	
	NTSTATUS status;
	UNICODE_STRING devNameString;                        //�������ִ��ṹ����
	KdPrint(("Enter AddDevice\n"));

	//���������豸����
    RtlInitUnicodeString( &devNameString, L"\\FileSystem\\Filters\\SafeWallSysFilter" );//���������ļ�ϵͳ�����豸����

	//���������豸����
    status = IoCreateDevice( pDriverObject,
                                0,                                      //û�� �豸��չ 
                                &devNameString,                            //�豸��:   FileSystem\\Filters\\SFilter
                                FILE_DEVICE_DISK_FILE_SYSTEM,           //�豸����: �����ļ�ϵͳ
                                FILE_DEVICE_SECURE_OPEN,                //�豸����: �Է��͵�CDO�Ĵ�������а�ȫ���
                                FALSE,                                  //����һ�����û�ģʽ��ʹ�õ��豸
                                &gMyControlDeviceObject );         //�������ɵ�"�����豸����"

#if WINVER < 0x0501
	if (status == STATUS_OBJECT_PATH_NOT_FOUND)                         //�ж��Ƿ� δ�ҵ�·��
    {
        RtlInitUnicodeString( &devNameString, L"\\FileSystem\\SafeWallSysFilter" );  //���´��� �����豸���� 
        status = IoCreateDevice( pDriverObject, 0,
                                    &devNameString,                           
                                    FILE_DEVICE_DISK_FILE_SYSTEM,
                                    FILE_DEVICE_SECURE_OPEN,
                                    FALSE,
                                    &gMyControlDeviceObject );        //�������ɵ� �����豸���� 
    }
#endif

	if (!NT_SUCCESS( status )) //�ж�IoCreateDevice�����Ƿ�ɹ�
    {
		KdPrint(( "�ļ�ϵͳ \"%wZ\" �����豸����ʧ�� \n", &devNameString));
		return status;                //���󷵻�(����ʧ��)
    }
	KdPrint(( "�ļ�ϵͳ \"%wZ\" �����豸�����ɹ� \n", &devNameString));



	//�������ӷ��ţ��û�̬����CreateFile�򿪸�·��
	UNICODE_STRING symLinkName;
#ifdef _WIN64
	status = IoRegisterDeviceInterface( PhysicalDeviceObject, &MYDEVICE, NULL ,&symLinkName);
	if(!NT_SUCCESS(status))
	{
		KdPrint(( "�豸���ӷ��� \"%wZ\" ע��ʧ�� \n", &symLinkName));
		IoDeleteDevice(gMyControlDeviceObject);
		return status;
	}
	status=IoSetDeviceInterfaceState(&symLinkName, TRUE);
	if(!NT_SUCCESS(status))
	{
		KdPrint(( "�豸���ӷ��� \"%wZ\" ����ʧ�� \n", &symLinkName));
		IoDeleteDevice(gMyControlDeviceObject);
		return status;
	}
	KdPrint(( "�豸���ӷ��� \"%wZ\" ���óɹ� \n", &symLinkName));
	RtlFreeUnicodeString(&symLinkName);
#else  /* _WIN32 */
	
	RtlInitUnicodeString(&symLinkName, SAFEWALL_DEVICE_SYMNAME);
	status = IoCreateSymbolicLink(&symLinkName, &devNameString);
	if(!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&symLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devNameString);
		if( !NT_SUCCESS(status))
		{
			KdPrint(( "�豸���ӷ��� \"%wZ\" ����ʧ�� \n", &symLinkName));
			IoDeleteDevice(gMyControlDeviceObject );   //ɾ�����洴����CDO
			return status;
		}
	}
	KdPrint(( "�豸���ӷ��� \"%wZ\" �����ɹ� \n", &symLinkName));
#endif


	//���潫�������IO�ַ�����
	KdPrint(( "��ʼ�������IO�ַ����� \n"));
	PFAST_IO_DISPATCH fastIoDispatch = (PFAST_IO_DISPATCH)ExAllocatePool( NonPagedPool,                          //�ӷǷ�ҳ���з���
                                                sizeof( FAST_IO_DISPATCH ));        //Ҫ������ֽ���
    
    if (!fastIoDispatch)                //�ڴ����ʧ��
    {
		IoDeleteDevice(gMyControlDeviceObject );   //ɾ�����洴����CDO
        return STATUS_INSUFFICIENT_RESOURCES;            //����һ������status��(��Դ����)
    }
    RtlZeroMemory( fastIoDispatch, sizeof( FAST_IO_DISPATCH ) );                        
    pDriverObject->FastIoDispatch = fastIoDispatch;                                  //��FastIo���ɱ��浽���������FastIoDispatch��
    fastIoDispatch->SizeOfFastIoDispatch = sizeof( FAST_IO_DISPATCH );              //����FastIo���ɱ�ĳ�����

    fastIoDispatch->FastIoCheckIfPossible = FastIoCheckIfPossible;                //����FastIo���ɺ���,��21��
    fastIoDispatch->FastIoRead = FastIoRead;
    fastIoDispatch->FastIoWrite = FastIoWrite;
    fastIoDispatch->FastIoQueryBasicInfo = FastIoQueryBasicInfo;
    fastIoDispatch->FastIoQueryStandardInfo = FastIoQueryStandardInfo;
    fastIoDispatch->FastIoLock = FastIoLock;
    fastIoDispatch->FastIoUnlockSingle = FastIoUnlockSingle;
    fastIoDispatch->FastIoUnlockAll = FastIoUnlockAll;
    fastIoDispatch->FastIoUnlockAllByKey = FastIoUnlockAllByKey;
    fastIoDispatch->FastIoDeviceControl = FastIoDeviceControl;
    fastIoDispatch->FastIoDetachDevice = FastIoDetachDevice;
    fastIoDispatch->FastIoQueryNetworkOpenInfo = FastIoQueryNetworkOpenInfo;
    fastIoDispatch->MdlRead = FastIoMdlRead;
    fastIoDispatch->MdlReadComplete = FastIoMdlReadComplete;
    fastIoDispatch->PrepareMdlWrite = FastIoPrepareMdlWrite;
    fastIoDispatch->MdlWriteComplete = FastIoMdlWriteComplete;
    fastIoDispatch->FastIoReadCompressed = FastIoReadCompressed;
    fastIoDispatch->FastIoWriteCompressed = FastIoWriteCompressed;
    fastIoDispatch->MdlReadCompleteCompressed = FastIoMdlReadCompleteCompressed;
    fastIoDispatch->MdlWriteCompleteCompressed = FastIoMdlWriteCompleteCompressed;
    fastIoDispatch->FastIoQueryOpen = FastIoQueryOpen;
	KdPrint(( "����IO�ַ������������ \n"));

	KdPrint(( "------------------------------ע��fsFilter�ص�����------------------------------- \n"));
	//--------------------------------ע��fsFilter�ص�����-------------------------------
    FS_FILTER_CALLBACKS fsFilterCallbacks;
    fsFilterCallbacks.SizeOfFsFilterCallbacks = sizeof( FS_FILTER_CALLBACKS );
	fsFilterCallbacks.PreAcquireForSectionSynchronization = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForSectionSynchronization = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForSectionSynchronization = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForSectionSynchronization = PostFsFilterPassThrough;
	fsFilterCallbacks.PreAcquireForCcFlush = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForCcFlush = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForCcFlush = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForCcFlush = PostFsFilterPassThrough;
	fsFilterCallbacks.PreAcquireForModifiedPageWriter = PreFsFilterPassThrough;
	fsFilterCallbacks.PostAcquireForModifiedPageWriter = PostFsFilterPassThrough;
	fsFilterCallbacks.PreReleaseForModifiedPageWriter = PreFsFilterPassThrough;
	fsFilterCallbacks.PostReleaseForModifiedPageWriter = PostFsFilterPassThrough;
	
	
	/*��������wdk�����ĵ�
	�ڽ��������󴫵ݸ��ͼ�ɸѡ����������͵ײ��ļ�ϵͳ֮ǰ����ɸѡ��֪ͨ�ص����̡�
	�ڻص����̣�������������Ӧִ���κα�Ҫ�Ĵ�����������status_success�����������
	��������Ļص���������һ��״ֵ̬������status_success����ʹ�ò�������ʧ�ܡ��ظ�ʧ
	�ܵ�ĳЩҪ�����������󣬿�����ֹϵͳ�Ľ�������ˣ���������������Ӧ��ʧ��ʱ����
	��������ֻ���ھ��Ա�Ҫ�ġ�����Щ����ʧ��ʱ��ɸѡ����������Ӧ������ȫ��׼ȷ�ط���
	����״ֵ̬��
	ע��ɸѡ�����������֪ͨ�ص����̲����ͷ��ļ�ϵͳ��Դ���������һ��������������
	����һ��״ֵ̬�������κ�����֪ͨ�ص�����status_success��״ֵ̬�����ԡ�*/

	//�ڹ��˲�����ɺ󣬵����ϲ���������ǰ������filtercallback����
	status = FsRtlRegisterFileSystemFilterCallbacks( pDriverObject, &fsFilterCallbacks );
	if (!NT_SUCCESS( status ))
    {
        pDriverObject->FastIoDispatch = NULL;
		ExFreePool( fastIoDispatch );
		IoDeleteDevice(gMyControlDeviceObject );   //ɾ�����洴����CDO
		return status;
    }
	KdPrint(( "------------------------------fsFilter�ص�����ע�����------------------------------- \n"));
	//ע���ļ�ϵͳ�䶯�����������µ��ļ�ϵͳ����ʱ������ö�Ӧ�ĺ���
	//XP���ϵĲ���ϵͳ,�������µ�����ʱ����ʱ�ļ�ϵͳ�Ѽ��أ�Ҳ�����һ�α䶯����
	//��XP���ϵ��򲻻ᣬ��server2000

	status = IoRegisterFsRegistrationChange( pDriverObject, FileSystemChangeNotification );
	if (!NT_SUCCESS( status ))
	{
		KdPrint(( "�ļ�ϵͳ�䶯����ע��ʧ�ܣ�" ));

		pDriverObject->FastIoDispatch = NULL;                    //ע��ָ��fastIo�������ָ��ΪNULL
		ExFreePool( fastIoDispatch);     //�ͷŷ����fastIo��������ڴ�
		IoDeleteDevice(gMyControlDeviceObject );   //ɾ�����洴����CDO
		return status;                                                                                        //���󷵻�
	}

	KdPrint(( "�ļ�ϵͳ�䶯����ע��ɹ���" ));

	do{
                
		PDEVICE_OBJECT rawDeviceObject;
		PFILE_OBJECT fileObject;
		RtlInitUnicodeString( &devNameString, L"\\Device\\RawDisk" ); //RawDisk: δ��ʽ���Ĵ���

		/*
		IoGetDeviceObjectPointer�����Ĺ�����:
		�����²���豸��������������²��豸ָ�롣�ú�������˶��²��豸�����Լ��²��豸��������Ӧ���ļ���������á�
		�������������ж��֮ǰ���²���豸��������û�û�����������²�������ж�ػᱻֹͣ����˱���Ҫ�������²��豸��������á�
		���ǳ���һ�㲻��ֱ�Ӷ��²��豸��������ü��١����ֻҪ���ٶ��ļ���������þͿ��Լ����ļ�������豸����������������á�
		��ʵ�ϣ�IoGetDeviceObjectPointer���صĲ������²��豸�����ָ�룬���Ǹ��豸��ջ�ж�����豸�����ָ�롣

		IoGetDeviceObjectPointer�����ĵ��ñ����� IRQL=PASSIVE_LEVEL�ļ��������С�
		*/

		status = IoGetDeviceObjectPointer( &devNameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );
		if (NT_SUCCESS( status ))
		{
			FileSystemChangeNotification( rawDeviceObject, TRUE );
			ObDereferenceObject( fileObject ); //������ٶ��ļ����������
		}

		RtlInitUnicodeString( &devNameString, L"\\Device\\RawCdRom" );
		status = IoGetDeviceObjectPointer( &devNameString, FILE_READ_ATTRIBUTES, &fileObject, &rawDeviceObject );
		if (NT_SUCCESS( status ))
		{
			FileSystemChangeNotification( rawDeviceObject, TRUE );
			ObDereferenceObject( fileObject );//������ٶ��ļ����������
		}
	}while(FALSE);

	//����DO_BUFFERED_IO �������տ�����
	gMyControlDeviceObject->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	//�����ʼ����־����־�������Կ�ʼ������Ϣ��
	ClearFlag( gMyControlDeviceObject->Flags, DO_DEVICE_INITIALIZING );

	KdPrint(("Leave AddDevice\n"));
	return STATUS_SUCCESS;
}


#pragma PAGEDCODE
VOID  DriverUnload( IN PDRIVER_OBJECT DriverObject )
{
    PDEVICE_EXTENSION devExt;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    ULONG numDevices;
    LARGE_INTEGER interval;
	KdPrint(("Enter DriverUnload\n"));
    PDEVICE_OBJECT devList[DEVOBJ_LIST_SIZE];
    ASSERT(DriverObject == gMyDriverObject);

    while(TRUE)
    {
		/*ʹ��IoEnumerateDeviceObjectList�� DriverObject->DeviceObject->nextDeviceObject ��
		  �������ڣ�IoEnumerateDeviceObjectList�����Ӷ�������ã������豸��ж��ʱ��
		  �豸ָ������Ч
		*/
		status = IoEnumerateDeviceObjectList( DriverObject, devList, sizeof(devList), &numDevices);
        if (numDevices <= 0)
        {
                break;
        }

		//���豸��������DEVOBJ_LIST_SIZEʱ��һ�δ����꣬��������ѭ������������
        numDevices = min( numDevices, DEVOBJ_LIST_SIZE );
        for (ULONG i=0; i < numDevices; i++)
        {
			devExt = (PDEVICE_EXTENSION)devList[i]->DeviceExtension;
            if (NULL != devExt)
            {
                    IoDetachDevice( devExt->AttachedToDeviceObject );
            }
        }

		//���󣬿�����Щ������Ϣ��û������ɣ�������Ϣ5�룬�ٶ���������ɾ��
        interval.QuadPart = (5 * DELAY_ONE_SECOND);		//delay 5 seconds
        KeDelayExecutionThread( KernelMode, FALSE, &interval );
        for (ULONG i=0; i < numDevices; i++)
        {
            if (NULL != devList[i]->DeviceExtension)
            {
                CleanupMountedDevice( devList[i] );
            }
            else
			{
                ASSERT(devList[i] == gMyControlDeviceObject);
                gMyControlDeviceObject = NULL;
            }

            IoDeleteDevice( devList[i] );
			ObDereferenceObject( devList[i] );
		}
	}

    fastIoDispatch = DriverObject->FastIoDispatch;
    DriverObject->FastIoDispatch = NULL;
    ExFreePool( fastIoDispatch );
	ExFreePool( gStandardSafeWallObj);
	KdPrint(("Leave DriverUnload\n"));
}

#pragma PAGEDCODE
NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	//���TRP
	pIrp->IoStatus.Status = status;
	pIrp->IoStatus.Information = 0;

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("Ĭ�ϴ����ҵ�����������һ����Ϣ\n"));
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
        return STATUS_SUCCESS;
    }

    IoSkipCurrentIrpStackLocation( pIrp );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
	return status;

}