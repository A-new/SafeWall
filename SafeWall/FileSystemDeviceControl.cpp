
#include "SafeWall.h"

//�ļ�ϵͳ�ַ�����
NTSTATUS  FileSystemControlMountVolume( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
NTSTATUS  DeviceControlLoadFileSystem( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp );
#pragma PAGEDCODE
NTSTATUS  FileSystemDeviceControl( IN PDEVICE_OBJECT DeviceObject, IN PIRP pIrp )
{
    /*
    ����˵��:
    DeviceObject:    ���Ǵ������豸�������Ǳ��󶨵��ļ�ϵͳ�����豸����ջ�ϡ�
    */

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( pIrp );
    //PAGED_CODE();
    //ASSERT(!IS_MY_CONTROL_DEVICE_OBJECT( DeviceObject ));
    //ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    switch (irpSp->MinorFunction) 
	{
	case IRP_MN_MOUNT_VOLUME://һ��������
		{
			//��һ����
			return FileSystemControlMountVolume( DeviceObject, pIrp );
		}break;
	case IRP_MN_LOAD_FILE_SYSTEM: //�����ļ�ϵͳ
		{
			return DeviceControlLoadFileSystem( DeviceObject, pIrp );
		}break;
	case IRP_MN_USER_FS_REQUEST:
        {
            switch (irpSp->Parameters.FileSystemControl.FsControlCode) 
			{
				//�����һ����ʲôҲ������ֻ�Ǵ�ӡ������Ȼ�󴫵ݸ���һ��
				//��˵�������ǿ��Եģ�ֻ�ǻ������������ڴ�й©��
				//���ǽӹ����¼������������������Կ�����ʱ����
                case FSCTL_DISMOUNT_VOLUME:
                {
                    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
                    break;
                }
            }
            break;
        }
    }        

	//���ݸ��²�
    IoSkipCurrentIrpStackLocation( pIrp );
    return IoCallDriver( ((PDEVICE_EXTENSION)DeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
}


//�󶨹��ؾ�
#pragma PAGEDCODE
NTSTATUS  FileSystemControlMountVolume( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    /*
    ����˵��:

    DeviceObject:    �������Ǵ������豸���������󶨵��ļ�ϵͳCDO���豸ջ�ϡ�
    Irp:             ���Ƿ��͸��ļ�ϵͳCDO�Ĺ�����������һ���¾�Ĺ�������
        
    */
        
    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation( Irp );

    PDEVICE_OBJECT storageStackDeviceObject;

    // newDeviceObject�ǽ�Ҫ�󶨵��ļ�ϵͳ�ľ��豸�����ϡ�����˵�����newDeviceObjectҪ���󶨵��¹��ؾ���豸���ϡ�
    PDEVICE_OBJECT newDeviceObject;

    PDEVICE_EXTENSION newDevExt;
    NTSTATUS status;
    BOOLEAN isShadowCopyVolume;
    

    /*PAGED_CODE();
    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(IS_DESIRED_DEVICE_TYPE(DeviceObject->DeviceType));*/

    /*
    �ڰ�IRP���͵��ļ�ϵͳ֮ǰ��������������ʱ��Vpb->RealDevice������ǣ���Ҫ�����صĴ����豸����
    storageStackDeviceObject���ȱ�����VPB��ֵ��������Ϊ����IRP�·����ײ������󣬿��ܻ�ı䡣
    */
    storageStackDeviceObject = irpSp->Parameters.MountVolume.Vpb->RealDevice;

    status = IsShadowCopyVolume ( storageStackDeviceObject, &isShadowCopyVolume );

    if (NT_SUCCESS(status) && isShadowCopyVolume) 
    {
		UNICODE_STRING shadowDeviceName;
		WCHAR shadowNameBuffer[MAX_DEVNAME_LENGTH];

		RtlInitEmptyUnicodeString( &shadowDeviceName, shadowNameBuffer, sizeof(shadowNameBuffer) );
		GetObjectName( storageStackDeviceObject, &shadowDeviceName );

		//���������󶨾�Ӱ��������һ������
		IoSkipCurrentIrpStackLocation( Irp );
		return IoCallDriver( devExt->AttachedToDeviceObject, Irp );
	}

    status = IoCreateDevice( gMyDriverObject, sizeof( DEVICE_EXTENSION ), NULL, 
		DeviceObject->DeviceType,  0, FALSE, &newDeviceObject );

    /*�������IRP���͵��ļ�ϵͳ�У���ô�ļ�ϵͳ�Ͳ����յ������Ĺ�������*/
    if (!NT_SUCCESS( status ))
    {
        KdPrint(( "�����󶨹��ؾ��豸ʧ��, status=%08x\n", status ));

		Irp->IoStatus.Information = 0;
		Irp->IoStatus.Status = status;
		IoCompleteRequest( Irp, IO_NO_INCREMENT );

		return status;
	}

    //��д�豸��չ������Ŀ���ǣ���������ɺ��������׵�storageStackDeviceObject
    newDevExt = (PDEVICE_EXTENSION)newDeviceObject->DeviceExtension;
	newDevExt->StorageStackDeviceObject = storageStackDeviceObject;
	RtlInitEmptyUnicodeString( &newDevExt->DeviceName, newDevExt->DeviceNameBuffer, sizeof(newDevExt->DeviceNameBuffer) );
	GetObjectName( storageStackDeviceObject, &newDevExt->DeviceName );

    //�������������¼����󣬰���������������С���������Ŀ���ǣ�֪ͨ��ǰ���̣��ļ�ϵͳ�Ѿ�����˵�ǰ��Ĺ��ء�
    KEVENT waitEvent;
    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
	IoCopyCurrentIrpStackLocationToNext ( Irp );
	IoSetCompletionRoutine( Irp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
	status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );
	if (STATUS_PENDING == status) 
	{
		//�ȴ����ײ��������ɣ�Ȼ��ͻ����������̡�
		status = KeWaitForSingleObject( &waitEvent, Executive, KernelMode, FALSE, NULL );
		//ASSERT( STATUS_SUCCESS == status );
	}
	//ASSERT(KeReadStateEvent(&waitEvent) ||!NT_SUCCESS(Irp->IoStatus.Status));

	KdPrint(("���ؾ���Ϣ������ɣ����������Ծ���а�\n"));
    //ִ�е������˵����Ĺ����Ѿ���ɣ�Ҫ��ʼ�󶨾��ˡ��ȵ���ɺ����������¼�֮�������󶨾�
	status = FileSystemControlMountVolumeComplete( DeviceObject, Irp, newDeviceObject );

    return status;
}

#pragma PAGEDCODE
NTSTATUS  DeviceControlLoadFileSystem( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
	//�����ļ�ϵͳ
    PDEVICE_EXTENSION devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    NTSTATUS status;

    PAGED_CODE();

    KEVENT waitEvent;
        
    KeInitializeEvent( &waitEvent,NotificationEvent, FALSE );

    IoCopyCurrentIrpStackLocationToNext( Irp );
        
    IoSetCompletionRoutine( Irp,AutoCompletionRoutine, &waitEvent, TRUE,TRUE, TRUE );

    status = IoCallDriver( devExt->AttachedToDeviceObject, Irp );

    if (STATUS_PENDING == status) 
    {
		status = KeWaitForSingleObject( &waitEvent, Executive, KernelMode, FALSE, NULL );
        //ASSERT( STATUS_SUCCESS == status );
    }

	//ASSERT(KeReadStateEvent(&waitEvent) || !NT_SUCCESS(Irp->IoStatus.Status));

	KdPrint(("�ļ�ϵͳ���سɹ������濪ʼ���豸�󶨵��ļ�ϵͳ\n"));
    status = DeviceControlLoadFileSystemComplete( DeviceObject, Irp );
    
    return status;
}