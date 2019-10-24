

#include "SafeWall.h"

//ȡ�ö�������
VOID  GetObjectName( IN PVOID Object, IN OUT PUNICODE_STRING Name )
{
    NTSTATUS status;
    CHAR nibuf[512];                                                                    //���շ��صĶ�����
    POBJECT_NAME_INFORMATION nameInfo = (POBJECT_NAME_INFORMATION)nibuf;                //nameInfo��ŵ���ָ��UNICODE_STRING�ṹ������ָ��
    ULONG retLength;                                                                    //����"ʵ�ʷ��صĶ���������"
        
    status = ObQueryNameString( Object, nameInfo, sizeof(nibuf), &retLength);        //���ض�����
    Name->Length = 0;
    if (NT_SUCCESS( status ))
    {
        RtlCopyUnicodeString( Name, &nameInfo->Name );
    }
}


//ȡ����ײ���豸����
VOID  GetBaseDeviceObjectName( IN PDEVICE_OBJECT DeviceObject, IN OUT PUNICODE_STRING Name )
{
	//�õ�ջ�׵��豸
    DeviceObject = IoGetDeviceAttachmentBaseRef( DeviceObject );
    GetObjectName( DeviceObject, Name );
    ObDereferenceObject( DeviceObject );//�ͷŵ��ü���
}


PUNICODE_STRING  GetFileName( IN PFILE_OBJECT FileObject, IN NTSTATUS CreateStatus, IN OUT PGET_NAME_CONTROL NameControl )
{
    POBJECT_NAME_INFORMATION nameInfo;
    NTSTATUS status;
    ULONG size;
    ULONG bufferSize;

    NameControl->allocatedBuffer = NULL;
    nameInfo = (POBJECT_NAME_INFORMATION)NameControl->smallBuffer;
    bufferSize = sizeof(NameControl->smallBuffer);
    status = ObQueryNameString( (NT_SUCCESS( CreateStatus ) ? (PVOID)FileObject : (PVOID)FileObject->DeviceObject), nameInfo, bufferSize, &size );
    if (status == STATUS_BUFFER_OVERFLOW)
    {
        bufferSize = size + sizeof(WCHAR);
        NameControl->allocatedBuffer = (PCHAR)ExAllocatePool( NonPagedPool, bufferSize);
        if (NULL == NameControl->allocatedBuffer)
        {
            RtlInitEmptyUnicodeString(
			(PUNICODE_STRING)&NameControl->smallBuffer,
			(PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
			(USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

			return (PUNICODE_STRING)&NameControl->smallBuffer;
		}
                
        nameInfo = (POBJECT_NAME_INFORMATION)NameControl->allocatedBuffer;
        status = ObQueryNameString(
                FileObject,
                nameInfo,
                bufferSize,
                &size );
	}
        
    if (NT_SUCCESS( status ) && !NT_SUCCESS( CreateStatus ))
    {
        ULONG newSize;
		PCHAR newBuffer;
		POBJECT_NAME_INFORMATION newNameInfo;
        newSize = size + FileObject->FileName.Length;
        if (NULL != FileObject->RelatedFileObject)
        {
            newSize += FileObject->RelatedFileObject->FileName.Length + sizeof(WCHAR);
        }
                
        if (newSize > bufferSize)
        {
            newBuffer = (PCHAR)ExAllocatePool( NonPagedPool, newSize );
            if (NULL == newBuffer)
            {
                RtlInitEmptyUnicodeString(
				(PUNICODE_STRING)&NameControl->smallBuffer,
				(PWCHAR)(NameControl->smallBuffer + sizeof(UNICODE_STRING)),
				(USHORT)(sizeof(NameControl->smallBuffer) - sizeof(UNICODE_STRING)) );

				return (PUNICODE_STRING)&NameControl->smallBuffer;
			}
                        
            newNameInfo = (POBJECT_NAME_INFORMATION)newBuffer;
                        
            RtlInitEmptyUnicodeString(
				&newNameInfo->Name,
				(PWCHAR)(newBuffer + sizeof(OBJECT_NAME_INFORMATION)),
				(USHORT)(newSize - sizeof(OBJECT_NAME_INFORMATION)) );

			RtlCopyUnicodeString( &newNameInfo->Name, &nameInfo->Name );
            if (NULL != NameControl->allocatedBuffer) 
            {
                    ExFreePool( NameControl->allocatedBuffer );
            }
                        
            NameControl->allocatedBuffer = newBuffer;
			bufferSize = newSize;
			nameInfo = newNameInfo;
		}
		else
		{
			nameInfo->Name.MaximumLength = (USHORT)(bufferSize - sizeof(OBJECT_NAME_INFORMATION));
		}
                
        if (NULL != FileObject->RelatedFileObject)
        {
            RtlAppendUnicodeStringToString( &nameInfo->Name, &FileObject->RelatedFileObject->FileName );
            RtlAppendUnicodeToString( &nameInfo->Name, L"\\" );
        }
                
        RtlAppendUnicodeStringToString( &nameInfo->Name, &FileObject->FileName );
        ASSERT(nameInfo->Name.Length <= nameInfo->Name.MaximumLength);
    }
        
    return &nameInfo->Name;
}

VOID GetFileNameCleanup( IN OUT PGET_NAME_CONTROL NameControl )
{

    if (NULL != NameControl->allocatedBuffer) {

        ExFreePool( NameControl->allocatedBuffer);
        NameControl->allocatedBuffer = NULL;
    }
}