
#include "SafeWall.h"

#pragma PAGEDCODE
NTSTATUS DriverQueryInformationDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp)
{
	NTSTATUS status;
	KEVENT waitEvent;
	DWORD FileFlags = 0;
	PSAFEWALL_FILE_LIST pFileNode;
	PIO_STACK_LOCATION stack;
	PFILE_OBJECT pFileObject;
	char * buffer;

	if (IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		KdPrint(("�ҵ�����������һ����ѯ�ļ�����Ϣ\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }

	stack = IoGetCurrentIrpStackLocation(pIrp);
	pFileObject = stack->FileObject;
	pFileNode = SelectFileListNode((PFSRTL_COMMON_FCB_HEADER)pFileObject->FsContext);
	FileFlags = pFileNode ? pFileNode->flags : NULL;

	if(!(FileFlags & SAFEWALL_FLAG_OBJECT))
	{
		//���������ܶ���ֱ�ӷ��²�
		IoSkipCurrentIrpStackLocation( pIrp );
		status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
		return status;
	}


    KeInitializeEvent( &waitEvent, NotificationEvent, FALSE );
    IoCopyCurrentIrpStackLocationToNext( pIrp );
	IoSetCompletionRoutine( pIrp,AutoCompletionRoutine, &waitEvent, TRUE, TRUE, TRUE );
    status = IoCallDriver( ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject, pIrp );
    if (STATUS_PENDING == status)//��״̬�� ����
    {
        KeWaitForSingleObject(&waitEvent, Executive, KernelMode, FALSE, NULL);
    }
	
	//��ʱ�ɹ������ļ���Ϣ��Ҳ�п��ܷ���ʧ��
	/*if(!NT_SUCCESS(pIrp->IoStatus.Status) && !(FileFlags & SAFEWALL_FLAG_OBJECT))
	{
		status = pIrp->IoStatus.Status;
		IoCompleteRequest( pIrp, IO_NO_INCREMENT );
		return status;
	}*/ 

#ifdef DBG
	UNICODE_STRING ProcName;
	UNICODE_STRING FileName;
	WCHAR procname[MY_MAX_PATH];
	WCHAR filename[MY_MAX_PATH];
	RtlInitEmptyUnicodeString(&ProcName, procname, sizeof(procname));
	RtlInitEmptyUnicodeString(&FileName, filename, sizeof(filename));
	
	GetCurrentProcessName(&ProcName);
	KdPrint(("%wZ���̲�ѯ�ļ���%wZ. FileInformationClass=%d",&ProcName,&pFileObject->FileName,\
		stack->Parameters.QueryFile.FileInformationClass));

#endif
		
	buffer = (char*)pIrp->AssociatedIrp.SystemBuffer;
    switch(stack->Parameters.QueryFile.FileInformationClass)
    {
    case FileAllInformation:
        {
            // ע��FileAllInformation���������½ṹ��ɡ���ʹ���Ȳ�����
            // ��Ȼ���Է���ǰ����ֽڡ�
            //typedef struct _FILE_ALL_INFORMATION {
            //    FILE_BASIC_INFORMATION BasicInformation;
            //    FILE_STANDARD_INFORMATION StandardInformation;
            //    FILE_INTERNAL_INFORMATION InternalInformation;
            //    FILE_EA_INFORMATION EaInformation;
            //    FILE_ACCESS_INFORMATION AccessInformation;
            //    FILE_POSITION_INFORMATION PositionInformation;
            //    FILE_MODE_INFORMATION ModeInformation;
            //    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
            //    FILE_NAME_INFORMATION NameInformation;
            //} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;
            // ������Ҫע����Ƿ��ص��ֽ����Ƿ������StandardInformation
            // �������Ӱ���ļ��Ĵ�С����Ϣ��
            PFILE_ALL_INFORMATION pAllInformation = (PFILE_ALL_INFORMATION)buffer;
            if(pIrp->IoStatus.Information >= 
                sizeof(FILE_BASIC_INFORMATION) + sizeof(FILE_STANDARD_INFORMATION))
            {
				//ASSERT(pAllInformation->StandardInformation.EndOfFile.QuadPart >= SAFEWALL_OBJECT_SIZE);
				pAllInformation->StandardInformation.EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
                pAllInformation->StandardInformation.AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
                if(pIrp->IoStatus.Information >= 
                    sizeof(FILE_BASIC_INFORMATION) + 
                    sizeof(FILE_STANDARD_INFORMATION) +
                    sizeof(FILE_INTERNAL_INFORMATION) +
                    sizeof(FILE_EA_INFORMATION) +
                    sizeof(FILE_ACCESS_INFORMATION) +
                    sizeof(FILE_POSITION_INFORMATION))
                {
                    if(pAllInformation->PositionInformation.CurrentByteOffset.QuadPart >= SAFEWALL_OBJECT_SIZE)
                        pAllInformation->PositionInformation.CurrentByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
                }
            }
        }break;
    case FileAllocationInformation:
        {
		    PFILE_ALLOCATION_INFORMATION pAllocInformation = 
				(PFILE_ALLOCATION_INFORMATION)buffer;
            //ASSERT(pAllocInformation->AllocationSize.QuadPart >= SAFEWALL_OBJECT_SIZE);
		    pAllocInformation->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;        
        }break;
    case FileValidDataLengthInformation:
        {
		    PFILE_VALID_DATA_LENGTH_INFORMATION pVdlInformation = 
                (PFILE_VALID_DATA_LENGTH_INFORMATION)buffer;
            //ASSERT(pVdlInformation->ValidDataLength.QuadPart >= SAFEWALL_OBJECT_SIZE);
		    pVdlInformation->ValidDataLength.QuadPart -= SAFEWALL_OBJECT_SIZE;
        }break;
    case FileStandardInformation:
        {
            PFILE_STANDARD_INFORMATION pStandardInformation = (PFILE_STANDARD_INFORMATION)buffer;
            //ASSERT(pStandardInformation->AllocationSize.QuadPart >= SAFEWALL_OBJECT_SIZE);
			pStandardInformation->AllocationSize.QuadPart -= SAFEWALL_OBJECT_SIZE;
            pStandardInformation->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
        }break;
    case FileEndOfFileInformation:
        {
		    PFILE_END_OF_FILE_INFORMATION pEndInformation = 
                (PFILE_END_OF_FILE_INFORMATION)buffer;
            //ASSERT(pEndInformation->EndOfFile.QuadPart >= SAFEWALL_OBJECT_SIZE);
		    pEndInformation->EndOfFile.QuadPart -= SAFEWALL_OBJECT_SIZE;
            
        }break;
	case FilePositionInformation:
		{
			PFILE_POSITION_INFORMATION pPositionInformation =
				(PFILE_POSITION_INFORMATION)buffer; 
            if(pPositionInformation->CurrentByteOffset.QuadPart > SAFEWALL_OBJECT_SIZE)
			    pPositionInformation->CurrentByteOffset.QuadPart -= SAFEWALL_OBJECT_SIZE;
		}break;
	case FileStreamInformation:
		{
			PFILE_STREAM_INFORMATION pStreamInformation =
				(PFILE_STREAM_INFORMATION)buffer; 
			/*KdPrint(("StreamAllocationSize=%d, StreamSize=%d",pStreamInformation->StreamAllocationSize.LowPart,\
				pStreamInformation->StreamSize.LowPart));*/
			//�һ��ļ����Ի��յ�������Ϣ������֪����ʲô�õ�
		}break;
    default:
		{
			KdPrint(("δ֪�Ĳ�ѯ���� %d",stack->Parameters.QueryFile.FileInformationClass));
		}break;
    }

	status = pIrp->IoStatus.Status;
    IoCompleteRequest( pIrp, IO_NO_INCREMENT );
    return status;
}

