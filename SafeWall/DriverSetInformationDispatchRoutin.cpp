
#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS DriverSetInformationDispatchRoutin(IN PDEVICE_OBJECT pDeviceObject,IN PIRP pIrp)
{
	NTSTATUS status;
	DWORD FileFlags = 0;
	DWORD ProcFlags = 0;
	PSAFEWALL_FILE_LIST pFileNode;
	FILE_OBJECT FileObject;
	char * buffer;

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("�ҵ�����������һ�������ļ�����Ϣ\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }
    
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	PFILE_OBJECT pFileObject = stack->FileObject;

	pFileNode = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
	FileFlags = pFileNode ? pFileNode->flags : NULL;

	if(!(FileFlags & SAFEWALL_FLAG_OBJECT))//���������ܶ���,����
	{
		IoSkipCurrentIrpStackLocation( pIrp );
		status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
		return status;
	}
	
	//��������ǰ����˼��ܶ����ˣ�����
	//KdPrint(("FileObject, 0x%08x, SetFileObject��0x%08x\n",pFileObject,\
	//	stack->Parameters.SetFile.FileObject));

	//KdPrint(("FileInformationClass, %d, Length��%d\n",\
	//	stack->Parameters.SetFile.FileInformationClass,stack->Parameters.SetFile.Length));
	
	buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;
	switch(stack->Parameters.SetFile.FileInformationClass)
	{
	case FileAllocationInformation:
		{
			PFILE_ALLOCATION_INFORMATION pAllInformation = 
				(PFILE_ALLOCATION_INFORMATION)buffer;
			pAllInformation->AllocationSize.QuadPart += SAFEWALL_OBJECT_SIZE;
			break;
		}
	case FileEndOfFileInformation:
		{
			PFILE_END_OF_FILE_INFORMATION pEndInformation = 
				(PFILE_END_OF_FILE_INFORMATION)buffer;
			pEndInformation->EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;
			break;
		}
	case FileValidDataLengthInformation:
		{
			PFILE_VALID_DATA_LENGTH_INFORMATION pVdlInformation = 
				(PFILE_VALID_DATA_LENGTH_INFORMATION)buffer;
			pVdlInformation->ValidDataLength.QuadPart += SAFEWALL_OBJECT_SIZE;
			break;
		}
	case FilePositionInformation:
		{
			PFILE_POSITION_INFORMATION pPositionInformation = 
				(PFILE_POSITION_INFORMATION)buffer;
			pPositionInformation->CurrentByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
			break;
		}
	case FileStandardInformation:
		{
			((PFILE_STANDARD_INFORMATION)buffer)->EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;
			break;
		}
	case FileAllInformation:
		{
			((PFILE_ALL_INFORMATION)buffer)->PositionInformation.CurrentByteOffset.QuadPart += SAFEWALL_OBJECT_SIZE;
			((PFILE_ALL_INFORMATION)buffer)->StandardInformation.EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;
		}break;
	default:
		{
			KdPrint(("δ֪���������� %d",stack->Parameters.SetFile.FileInformationClass));
		}break;
	}

	IoSkipCurrentIrpStackLocation( pIrp );
	status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
	return status;
}


