
#include "SafeWall.h"

#define IS_WINDOWS2000() ((gOsMajorVersion == 5) && (gOsMinorVersion == 0))
#define IS_WINDOWSXP() ((gOsMajorVersion == 5) && (gOsMinorVersion == 1))

NTSTATUS  IsShadowCopyVolume( IN PDEVICE_OBJECT StorageStackDeviceObject, OUT PBOOLEAN IsShadowCopy )
{

/*******************************************************************************************************************************************
SfIsShadowCopyVolume( )��������Ҫ�����ǣ��漰�� ��Ӱ��������ġ�

��Ӱ��������(Volume Shadow Copy Service��VSS)��һ�ֱ��ݺͻָ��ļ���������һ�ֻ���ʱ����������ļ������ļ�����
ͨ��ʹ�þ�Ӱ�����������ǿ������ض����Ͻ������ݿ���ʱ��㣬���ڽ�����ĳһʱ�̰����ݻָ����κ�һ������������ʱ����״̬��
���������һ���ǻָ���Ϊԭ����ɵ����ݶ�ʧ���û�������ش洢���д�����Ϣ���ļ��������ǲ�С��ɾ���ļ��������������������⡣
����VSS���ղ����ɾ������ݾ����Լ��ָ�ʱ��㿽�������������Ǽȿ��Իָ��������գ�Ҳ����ȡ���裬���߻�����ʹ��VSS���ݹ������ָ��������ļ����ļ��С�

VSS���Ƕ�Ӧ�ó�����б��ݵģ�VSS���Զ��߼���(Ӳ�̷���)���п��ա�
VSS��Windows�µĿ��ռ�������Requestor, Write,��Provider��ɣ���Ҫ����������Volsnap.sysʵ�֣����ӵ�������������ļ�ϵͳ����֮�䣬ͬʱ������COM������
��ˣ��������ǵ����ھ��ϵ�block���д������Ǻ͸���ϵͳӦ�������������SQL��EXCHANGE��AD�ȵȡ��Ӷ�ʹ���ڲ��ػ���Ҳ��ֹͣӦ�õ�����£������ա�
VSS���㷺��Ӧ�õ�Windows�ı��ݴ����С�

VSS �������ķ����ǣ�ͨ���ṩ����������Ҫʵ��֮���ͨѶ����֤���ݵĸ߶���ʵ�ͻָ����̵ļ�㡣
(1)�������������������ʱ������ݸ������Ӱ������Ӧ�ó��򣬱��籸�ݻ�洢����Ӧ�ó���
(2)д��������Ǹ�������֪ͨ�����ݱ�����д�������VSS������������Ӱ��������ս�������ĵط�����VSS�ľ�Ӱ���ƹ����л��漰һЩӦ�ó���
(3)�ṩ���������ڱ�¶����Ӳ��������ľ�Ӱ�����Ļ��ơ����洢Ӳ����Ӧ�̶���Ϊ���ǵĴ洢���б�д�ṩ����

VSS����Ψһ��ȱ���ǣ�
������ҪΪÿһ����Ӱ��������Ĵ��̿ռ䣬���Ǳ�����ĳ���洢��Щ��������ΪVSSʹ��ָ�����ݣ���Щ����ռ�õĿռ�Ҫ�������С�ö࣬���ǿ�����Ч�ش洢��Щ������

�й�VSS�ĸ���˵��������ȥ���Microsoft��������վ

http://technet.microsoft.com/en-us/library/ee923636.aspx

*********************************************************************************************************************************************/
    PAGED_CODE();

    *IsShadowCopy = FALSE;

#if WINVER >= 0x0501
    if (IS_WINDOWS2000())
        {
#endif
                UNREFERENCED_PARAMETER( StorageStackDeviceObject );
                return STATUS_SUCCESS;
#if WINVER >= 0x0501
        }
        
        if (IS_WINDOWSXP())
        {
                UNICODE_STRING volSnapDriverName;
                WCHAR buffer[MAX_DEVNAME_LENGTH];
                PUNICODE_STRING storageDriverName;
                ULONG returnedLength;
                NTSTATUS status;
                
                if (FILE_DEVICE_DISK != StorageStackDeviceObject->DeviceType)
                {
                        return STATUS_SUCCESS;
                }
                
                storageDriverName = (PUNICODE_STRING) buffer;
                RtlInitEmptyUnicodeString( storageDriverName, (PWCHAR)Add2Ptr( storageDriverName, sizeof( UNICODE_STRING ) ), sizeof( buffer ) - sizeof( UNICODE_STRING ) );
                status = ObQueryNameString( StorageStackDeviceObject, (POBJECT_NAME_INFORMATION)storageDriverName, storageDriverName->MaximumLength, &returnedLength );
                if (!NT_SUCCESS( status ))
                {
                        return status;
                }
                
                RtlInitUnicodeString( &volSnapDriverName, L"\\Driver\\VolSnap" );
				//�Ƚ��ַ����������ִ�Сд
                if (RtlEqualUnicodeString( storageDriverName, &volSnapDriverName, TRUE ))
                {
                        *IsShadowCopy = TRUE;
                }
                else
                {
                        NOTHING;
                }
                
                return STATUS_SUCCESS;
        }
        else
        {
                PIRP irp;
                KEVENT event;
                IO_STATUS_BLOCK iosb;
                NTSTATUS status;
                if (FILE_DEVICE_VIRTUAL_DISK != StorageStackDeviceObject->DeviceType)
                {
                        return STATUS_SUCCESS;
                }
                
                KeInitializeEvent( &event, NotificationEvent, FALSE );

           /*
                *Microsoft WDK�ٷ��ĵ��� IOCTL_DISK_IS_WRITABLE���������͵ģ�
                *Determines whether a disk is writable.
                *The Status field can be set to STATUS_SUCCESS, or possibly to STATUS_INSUFFICIENT_RESOURCES, STATUS_IO_DEVICE_ERROR, or STATUS_MEDIA_WRITE_PROTECTED.
                *
                *IOCTL_DISK_IS_WRITABLE��û������Ҳû������ġ�
                */
                irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_IS_WRITABLE,
                                             StorageStackDeviceObject,
                                             NULL,
                                             0,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &iosb );
                if (irp == NULL)
                {
                        return STATUS_INSUFFICIENT_RESOURCES;
                }
                
                status = IoCallDriver( StorageStackDeviceObject, irp );
                if (status == STATUS_PENDING)
                {
                        (VOID)KeWaitForSingleObject( &event,
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL );
                        status = iosb.Status;
                }

                if (STATUS_MEDIA_WRITE_PROTECTED == status)
                {
                        *IsShadowCopy = TRUE;
                        status = STATUS_SUCCESS;
                }

                return status;
        }
#endif
}