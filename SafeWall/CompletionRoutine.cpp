
#include "SafeWall.h"

//����ɺ�����Ĭ�ϴ����������������¼��Ѿ����
#pragma PAGEDCODE
NTSTATUS  AutoCompletionRoutine( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PVOID Context )
{
    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Irp );

    ASSERT(IS_MY_DEVICE_OBJECT( DeviceObject ));
    ASSERT(Context != NULL);

    KeSetEvent((PKEVENT)Context, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
NTSTATUS  DeviceControlLoadFileSystemComplete ( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp )
{
    PDEVICE_EXTENSION devExt;
    NTSTATUS status;

    PAGED_CODE();

    devExt = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;

    if (!NT_SUCCESS( Irp->IoStatus.Status ) && (Irp->IoStatus.Status != STATUS_IMAGE_ALREADY_LOADED))
    {
		KdPrint(("��ʽ���豸�󶨵��ļ�ϵͳ\n"));
        IoAttachDeviceToDeviceStackSafe( DeviceObject, devExt->AttachedToDeviceObject, &devExt->AttachedToDeviceObject );
        ASSERT(devExt->AttachedToDeviceObject != NULL);
    }
    else
    {
        CleanupMountedDevice( DeviceObject );
        IoDeleteDevice( DeviceObject );
    }

    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

#pragma PAGEDCODE
NTSTATUS  FileSystemControlMountVolumeComplete( IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp, IN PDEVICE_OBJECT NewDeviceObject )
{
        /*
        ����˵��:

        DeviceObject:        ���ǰ󶨵��ļ�ϵͳ�����豸������豸ջ�ϣ�����һ�������豸����
        Irp:                 ���Ƿ��͸��ļ�ϵͳCDO�Ĺ�����������һ���¾�Ĺ�������
		NewDeviceObject:     �����´����Ĺ����豸�������ڰ󶨵��ļ�ϵͳ�ľ��豸������豸ջ�ϡ�

        */

    PVPB vpb;
    PDEVICE_EXTENSION newDevExt;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT attachedDeviceObject;
    NTSTATUS status;

    PAGED_CODE();

    newDevExt = (PDEVICE_EXTENSION)NewDeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

   /*
    * ��ȡ���Ǳ����VPB�����ʱ��Ϳ���ͨ�����豸����õ�VPB
    * VPB->DeviceObject��  �ļ�ϵͳ�����ľ��豸����
    * VPB->RealDevice��    �������������������豸����
    */
    vpb = newDevExt->StorageStackDeviceObject->Vpb;
    if (vpb != irpSp->Parameters.MountVolume.Vpb)
    {
		KdPrint(("irpSp->Parameters.MountVolume.Vpb �����ı�\n"));
    }

    if (NT_SUCCESS( Irp->IoStatus.Status ))
    {
		//������ٻ�����
        ExAcquireFastMutex( &gFastMutexAttachLock );
        if (!IsAttachedToDevice( vpb->DeviceObject, &attachedDeviceObject ))
        {          
			/*
			* SfAttachToMountedDevice�����壺�����Ǵ����Ĺ����豸����NewDeviceObject�󶨵��ļ�ϵͳ������VPB->DeviceObject���豸����ջ�ϡ�                                        
            */
            status = AttachToMountedDevice( vpb->DeviceObject, NewDeviceObject );
            if (!NT_SUCCESS( status ))
            {
                CleanupMountedDevice( NewDeviceObject );
                IoDeleteDevice( NewDeviceObject );
            }
            ASSERT( NULL == attachedDeviceObject );
        }
        else
        {
            CleanupMountedDevice( NewDeviceObject );
            IoDeleteDevice( NewDeviceObject );
            ObDereferenceObject( attachedDeviceObject );
        }
		//�ͷſ��ٻ�����
        ExReleaseFastMutex( &gFastMutexAttachLock );
    }
    else
    {
        CleanupMountedDevice( NewDeviceObject );
        IoDeleteDevice( NewDeviceObject );
    }
    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}
