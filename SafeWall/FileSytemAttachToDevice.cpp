
#include "SafeWall.h"

#pragma PAGEDCODE
VOID  FileSystemChangeNotification( IN PDEVICE_OBJECT DeviceObject, IN BOOLEAN FsActive )
{

/*
   SfFsNotification������
   ������һ���豸���󣬲��������ӵ�ָ�����ļ�ϵͳ�����豸����(File System CDO)�Ķ���ջ�ϡ������������豸����������з��͸��ļ�ϵͳ������
   ���������Ǿ��ܹ����һ�����ؾ�����󣬾Ϳ��Ը��ӵ�����µľ��豸������豸����ջ�ϡ�
  
   ��SfFsNotification������������Ժ����ǵĹ��������豸������ܹ����յ����͵��ļ�ϵͳCDO�����󣬼����յ�IRP_MJ_FILE_SYSTEM_CONTROL������˵��
   �ļ�ϵͳ�����豸�Ѿ����󶨣����Զ�̬��ؾ�Ĺ����ˡ���ô�Ժ�Ĺ�������Ҫ��ɶԾ�ļ�ذ��ˡ�
  
   
   ����˵��:
  
   DeviceObject:   ��ָ���ļ�ϵͳ�Ŀ����豸����(CDO)���� �������������File System CDO
   FsActive:       ֵΪTRUE����ʾ�ļ�ϵͳ�ļ��ֵΪFALSE����ʾ�ļ�ϵͳ��ж�ء�
  
  */
	KdPrint(("Enter FileSystemChangeNotification\n"));
    UNICODE_STRING name;                                        //����ṹ����
    WCHAR nameBuffer[MAX_DEVNAME_LENGTH];        //������ַ�������,����64

    PAGED_CODE();

    RtlInitEmptyUnicodeString( &name, nameBuffer, sizeof(nameBuffer) );                //��ʼ��name(��ԱBuffer->nameBuffer,Length=0,MaximumLength=64)
    GetObjectName( DeviceObject, &name );       //ȡ���豸���ƣ����߿������Ϊȡ���̷�                                                                                         

    if (FsActive)
    {
		KdPrint(("׼�����ļ�ϵͳ %wZ ���ӵ������豸\n",&name));
        AttachToFileSystemDevice( DeviceObject, &name );        //������ɶ��ļ�ϵͳ�����豸�İ�
    }
    else
    {
		KdPrint(("����ļ�ϵͳ�����豸: %wZ\n",&name));
        DetachFromFileSystemDevice( DeviceObject );
    }

	KdPrint(("Leave FileSystemChangeNotification\n"));
}

#pragma PAGEDCODE
NTSTATUS  AttachToFileSystemDevice( IN PDEVICE_OBJECT DeviceObject, IN PUNICODE_STRING DeviceName )
{
    /*
        SfAttachToFileSystemDevice����������ɶ��ļ�ϵͳ�����豸�İ󶨡�

        ����˵��:
        DeviceObject:   ��ָ���ļ�ϵͳ�Ŀ����豸����(CDO)���� �������������File System CDO

    */

    PDEVICE_OBJECT newDeviceObject;                  //���豸����
    PDEVICE_EXTENSION devExt;                //�ļ�ϵͳ��������������豸��չ
    NTSTATUS status;                                 //״̬��
    UNICODE_STRING fsrecName;                                                                                                                
    UNICODE_STRING fsName;                           //�ļ�ϵͳ��
    WCHAR tempNameBuffer[MAX_DEVNAME_LENGTH];		 //��ʱ������(������ִ�)

    PAGED_CODE();

    if (!IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType))        //���Ը����豸�ǲ�������Ҫ���ĵ��豸
    {
		KdPrint(("���Ƕ�  %wZ  �ļ�ϵͳ�������ģ�����\n",DeviceName ));
        return STATUS_SUCCESS;
    }
    
#if DBG

	switch(DeviceObject->DeviceType)
	{
	case FILE_DEVICE_DISK_FILE_SYSTEM:
		{
			KdPrint(("%wZ �Ǵ����ļ�ϵͳ\n",DeviceName ));
		}break;
	case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
		{
			KdPrint(("%wZ �ǹ����ļ�ϵͳ\n",DeviceName ));
		}break;
	case FILE_DEVICE_NETWORK_FILE_SYSTEM:
		{
			KdPrint(("%wZ �������ļ�ϵͳ\n",DeviceName ));
		}break;
	default:
		{
			KdPrint(("%wZ ��δ֪�豸\n",DeviceName ));
		}
	}

#endif

    /*
    * Windows�ı�׼�ļ�ϵͳʶ���������϶��������� \FileSystem\Fs_Rec ���ɵġ�����ֱ���ж����������ֿ��Խ��һ�������⡣
    * Ҳ��һ���ǵ�Ҫ���ļ�ϵͳʶ��������������\FileSystem\Fs_Rec���档ֻ��˵��һ�����������\FileSystem\Fs_Rec���档
    */
    RtlInitEmptyUnicodeString( &fsName, tempNameBuffer, sizeof(tempNameBuffer) );

	//�����ļ�ϵͳʶ����
    RtlInitUnicodeString( &fsrecName, L"\\FileSystem\\Fs_Rec" );
    GetObjectName( DeviceObject->DriverObject, &fsName );
	
    if (RtlCompareUnicodeString( &fsName, &fsrecName, TRUE ) == 0)
    {        
        //ͨ���������������ֱ������Windows�ı�׼�ļ�ϵͳʶ����������ǣ���ô���سɹ���Ҳ���Ƿ������ˡ�
        //������д�����ļ�ϵͳʶ����û�б��жϵ����ļ�ϵͳ��������Ĺ����������ж�Ӧ�Ĵ���
		KdPrint(("�����ļ�ʶ����\n"));
        return STATUS_SUCCESS;
    }

    //�����ǹ��ĵ��ļ�ϵͳ���Ҳ���΢����ļ�ϵͳʶ�������豸������һ���豸������豸����
	status = IoCreateDevice( gMyDriverObject,sizeof(DEVICE_EXTENSION ),
                    NULL,DeviceObject->DeviceType, 0, FALSE, &newDeviceObject );

	if (!NT_SUCCESS( status ))
    {
		KdPrint(("�ļ�ϵͳ�����豸����ʧ�� status=%08x\n",status));
        return status;
    }

	if ( FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
    {
        SetFlag( newDeviceObject->Flags, DO_BUFFERED_IO );                                                                
    }

	if ( FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
    {
        SetFlag( newDeviceObject->Flags, DO_DIRECT_IO );                                                                        
    }

	if ( FlagOn( DeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN ) )
    {
        SetFlag( newDeviceObject->Characteristics, FILE_DEVICE_SECURE_OPEN );                
	}

    devExt = (PDEVICE_EXTENSION)newDeviceObject->DeviceExtension;
 
    /*
    ����SfAttachDeviceToDeviceStack�����������豸����󶨵�File System CDO���豸ջ���档���������ǵ�newDeviceObject�Ϳ��Խ��յ����͵�
    File System CDO��IRP_MJ_FILE_SYSTEM_CONTROL�����ˡ� �Ժ󣬳���Ϳ���ȥ�󶨾��ˡ�
    ʹ��SfAttachDeviceToDeviceStack���������а󶨡�����1�󶨵�����2���󶨺������ص��豸�洢�ڲ���3�С�
    */

    status = IoAttachDeviceToDeviceStackSafe( newDeviceObject, DeviceObject,  &devExt->AttachedToDeviceObject );
    if (!NT_SUCCESS( status ))
    {
		KdPrint(("�ļ�ϵͳ��ʧ�ܣ�status= %08x", status));
        goto ErrorCleanupDevice;
    }
	KdPrint(("�ļ�ϵͳ�󶨳ɹ���newDeviceObject= 0x%08x���²�AttachedToDeviceObject = 0x%08x", newDeviceObject,devExt->AttachedToDeviceObject));

    RtlInitEmptyUnicodeString( &devExt->DeviceName, devExt->DeviceNameBuffer, sizeof(devExt->DeviceNameBuffer) );
	RtlCopyUnicodeString( &devExt->DeviceName, DeviceName );        //Save Name
	//��DO_DEVICE_INITIALIZING����ʼ���У���־��������ܽ��յ�������������Ϣ
	ClearFlag( newDeviceObject->Flags, DO_DEVICE_INITIALIZING ); 
	KdPrint(("�ļ�ϵͳ %wZ �Ѱ󶨳ɹ�����ʼ������Ϣ",&devExt->DeviceName ));
    /* 
    ����SpyEnumerateFileSystemVolumesö�ٸ������ļ�ϵͳ�µĵ�ǰ���ڵ����й����˵��豸�����Ұ����ǡ�
	��������Ŀ�ģ�����Ϊ��������������ʱ�����أ����Ǽ��ع���������ʱ���ļ�ϵͳ�Ѿ������˾��豸��
    ���ǣ��ù��������Ӻ��أ���ʱ���ܰ��Ѿ����ڻ�ոչ����������ļ�ϵͳ���豸��
    */
    status = EnumerateFileSystemVolumes( DeviceObject, &fsName );
    if (!NT_SUCCESS( status ))
    {
        IoDetachDevice( devExt->AttachedToDeviceObject );
        goto ErrorCleanupDevice;
    }

    return STATUS_SUCCESS;

    ErrorCleanupDevice:
        CleanupMountedDevice( newDeviceObject );
        IoDeleteDevice( newDeviceObject );

    return status;
}

#pragma PAGEDCODE
VOID  DetachFromFileSystemDevice( IN PDEVICE_OBJECT DeviceObject )
{
    PDEVICE_OBJECT ourAttachedDevice;
    PDEVICE_EXTENSION devExt;

    PAGED_CODE();

    ourAttachedDevice = DeviceObject->AttachedDevice;
    while (NULL != ourAttachedDevice)
    {
        if (IS_MY_DEVICE_OBJECT( ourAttachedDevice ))
        {
            devExt = (PDEVICE_EXTENSION) ourAttachedDevice->DeviceExtension;

            CleanupMountedDevice( ourAttachedDevice );
			IoDetachDevice( DeviceObject );
			IoDeleteDevice( ourAttachedDevice );
			return;
		}
        
		DeviceObject = ourAttachedDevice;
		//ourAttachedDevice = ourAttachedDevice->AttachedDevice;
		ourAttachedDevice = DeviceObject->AttachedDevice;
	}
}

//�ж�һ���豸�Ƿ��Ѿ������˰�
#pragma PAGEDCODE
BOOLEAN  IsAttachedToDevice( PDEVICE_OBJECT DeviceObject, PDEVICE_OBJECT *AttachedDeviceObject OPTIONAL )
{
    PDEVICE_OBJECT currentDevObj;
    PDEVICE_OBJECT nextDevObj;

    PAGED_CODE();

    currentDevObj = IoGetAttachedDeviceReference( DeviceObject );
        
    do {   
        if (IS_MY_DEVICE_OBJECT( currentDevObj ))
        {
            if (ARGUMENT_PRESENT(AttachedDeviceObject))
            {
                 *AttachedDeviceObject = currentDevObj;
            }
            else
            {
                 ObDereferenceObject( currentDevObj );
            }
            return TRUE;
        }

        nextDevObj = IoGetLowerDeviceObject( currentDevObj );
		ObDereferenceObject( currentDevObj );
		currentDevObj = nextDevObj;
	} while (NULL != currentDevObj);
        
	if (ARGUMENT_PRESENT(AttachedDeviceObject))
	{
		*AttachedDeviceObject = NULL;
	}
        
    return FALSE;
}

//�����
#pragma PAGEDCODE
VOID  CleanupMountedDevice( IN PDEVICE_OBJECT DeviceObject )
{        
	//ж���豸����ʵʲôҲ����Ҳ���Եģ�ϵͳ���Զ���������ɴ󲿷ֵ�ж�ز�����
	//�����������������ڴ�й©�����������豸�Ĳ�������Ƶ������������ж�غ����ǻ�Ҫ������
    UNREFERENCED_PARAMETER( DeviceObject );
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
}

//���豸���а�
#pragma PAGEDCODE
NTSTATUS  AttachToMountedDevice( IN PDEVICE_OBJECT DeviceObject, IN PDEVICE_OBJECT SFilterDeviceObject )
{
    /*
    SfAttachToMountedDevice�����Ĺ���: ��ɰ�һ���ļ�ϵͳ���豸�Ĳ�����
        
    ����˵��:
    SFilterDeviceObject:  ��������ʹ��IoCreateDevice�������������豸����

    */

    PDEVICE_EXTENSION newDevExt = (PDEVICE_EXTENSION)SFilterDeviceObject->DeviceExtension;
    NTSTATUS status;
	ULONG i;

    PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( SFilterDeviceObject ));
    ASSERT(!IsAttachedToDevice ( DeviceObject, NULL ));

    if (FlagOn( DeviceObject->Flags, DO_BUFFERED_IO ))
    {
        SetFlag( SFilterDeviceObject->Flags, DO_BUFFERED_IO );
    }

    if (FlagOn( DeviceObject->Flags, DO_DIRECT_IO ))
    {
        SetFlag( SFilterDeviceObject->Flags, DO_DIRECT_IO );
    }

    for (i=0; i < 8; i++)
    {
        LARGE_INTEGER interval;

        //����SfAttachDeviceToDeviceStack��������  ��İ�
        status = IoAttachDeviceToDeviceStackSafe( SFilterDeviceObject, DeviceObject, &newDevExt->AttachedToDeviceObject );
        if (NT_SUCCESS(status))
        {
			
            ClearFlag( SFilterDeviceObject->Flags, DO_DEVICE_INITIALIZING );
			KdPrint(("�� %wZ�豸�󶨳ɹ���DeviceObject= 0x%08x ��ʼ������Ϣ\n",&newDevExt->DeviceName,SFilterDeviceObject));
            return STATUS_SUCCESS;
        }
		//���ʧ�ܣ��ȴ�һ�£�Ȼ��������
        interval.QuadPart = (500 * DELAY_ONE_MILLISECOND);      //delay 1/2 second  0.5��
		KeDelayExecutionThread( KernelMode, FALSE, &interval );
	}

    return status;
}

//
#pragma PAGEDCODE
NTSTATUS  EnumerateFileSystemVolumes( IN PDEVICE_OBJECT FSDeviceObject, IN PUNICODE_STRING Name ) 
{
    /*
    ����˵��:
	FSDeviceObject:    ��ָ���ļ�ϵͳ�Ŀ����豸����(CDO)���� �������������File System CDO
    Name:              �����ļ�ϵͳ�����֣�����NTFS����һ���ļ�ϵͳ
    */

    PDEVICE_OBJECT newDeviceObject;
    PDEVICE_EXTENSION newDevExt;
    PDEVICE_OBJECT *devList;
    PDEVICE_OBJECT storageStackDeviceObject;
    NTSTATUS status;
    ULONG numDevices;
    ULONG i;
    BOOLEAN isShadowCopyVolume;

    PAGED_CODE();
        
    /*
    * IoEnumerateDeviceObjectList����ö����������µ��豸�����б����������������2�Ρ�
    * ��1�ε��ã� ��ȡ�豸�б��е��豸�����������
    * ��2�ε���:  ���ݵ�1�εĽ��numDevicesֵ�������豸����Ĵ�ſռ䣬�Ӷ��õ��豸��devList��
    */
    status = IoEnumerateDeviceObjectList(FSDeviceObject->DriverObject,
                NULL,0, &numDevices);
        
    if (!NT_SUCCESS( status ))
    {
        ASSERT(STATUS_BUFFER_TOO_SMALL == status);
        numDevices += 8;  //Ϊ��֪���豸�����ڴ�ռ���д洢����������8�ֽڡ�

        devList = (PDEVICE_OBJECT *)ExAllocatePool( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)) );
        if (NULL == devList)
        {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
                
        status = IoEnumerateDeviceObjectList(FSDeviceObject->DriverObject,
                        devList,(numDevices * sizeof(PDEVICE_OBJECT)), &numDevices);
                
        if (!NT_SUCCESS( status ))
        {
            ExFreePool( devList );
            return status;
        }
                
        //���α��������豸����
        for (i=0; i < numDevices; i++)
        {
            storageStackDeviceObject = NULL;
            __try 
			{        
                //����豸�������ļ�ϵͳCDO�������ǲ��������͵ģ��������Ѿ��󶨵�
                if ((devList[i] == FSDeviceObject) || (devList[i]->DeviceType != FSDeviceObject->DeviceType) || IsAttachedToDevice( devList[i], NULL ))
                {
                        __leave;//�뿪try��
                }
                                
				//��ȡ�豸������ջ���ļ�ϵͳ��ײ���豸���������
                GetBaseDeviceObjectName( devList[i], Name );
                if (Name->Length > 0) //��������֣��뿪
                {
                        __leave;
                }

                /*
                ����IoGetDiskDeviceObject��������ȡһ�����ļ�ϵͳ�豸�����йصĴ����豸����ֻ���Ѿ�ӵ��һ�������豸������ļ�ϵͳ�豸����
                */
				status = IoGetDiskDeviceObject( devList[i], &storageStackDeviceObject );

				if (!NT_SUCCESS( status ))
                {
                        __leave;
                }
                                
                status = IsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

				//���󶨾�Ӱ
				if (NT_SUCCESS(status) && isShadowCopyVolume) 
				{
					//UNICODE_STRING shadowDeviceName;
					//WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

					//RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
					//GetObjectName( storageStackDeviceObject, &shadowDeviceName );

                    __leave;
                }
                                

                // ��һ�������豸���󣬴����µ��豸����׼���󶨡�
                status = IoCreateDevice( gMyDriverObject,
                            sizeof(DEVICE_EXTENSION ),
                            NULL,
                            devList[i]->DeviceType,
                            0,
                            FALSE,
                            &newDeviceObject );
                if (!NT_SUCCESS( status ))
                {
                        __leave;
                }
                                
                newDevExt = (PDEVICE_EXTENSION) newDeviceObject->DeviceExtension;
				newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
                RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer,sizeof(newDevExt->DeviceNameBuffer) );
                GetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );

                /*
                    �ڰ�����ʱ���ٲ����£����豸�Ƿ񱻰󶨹����������һ���������û���󶨣���ִ������İ󶨹��̣�����ֱ�ӷ��ء�
                */
                ExAcquireFastMutex( &gFastMutexAttachLock );
                if (!IsAttachedToDevice( devList[i], NULL ))
                {
                    status = AttachToMountedDevice( devList[i], newDeviceObject );
                    if (!NT_SUCCESS( status ))
                    {
                        CleanupMountedDevice( newDeviceObject );
                        IoDeleteDevice( newDeviceObject );
                    }
                }
                else
                {
                    CleanupMountedDevice( newDeviceObject );
                    IoDeleteDevice( newDeviceObject );
                }
                
                ExReleaseFastMutex( &gFastMutexAttachLock );
            }/*try end*/
		__finally 
			{       
				/*
					�����豸����ļ���������������ɺ���IoGetDiskDeviceObject���ӵġ��ɹ��󶨺󣬾ͼ��ٸ��豸����ļ�����
					һ���ɹ��󶨵�devList[i]��I/O��������ȷ���豸ջ���²��豸����һֱ���ڣ�һֱ������ļ�ϵͳջ��ж����
				*/
                if (storageStackDeviceObject != NULL)
                {
                    ObDereferenceObject( storageStackDeviceObject );
                }

                //�����豸����ļ���������������ɺ���IoEnumerateDeviceObjectList���ӵġ�
                ObDereferenceObject( devList[i] );
			}
		}/* for end*/

        status = STATUS_SUCCESS;
        ExFreePool( devList );
    }
    return status;
}
