
#include "SafeWall.h"
#include "myfs.h"

EXTERN_C DWORD FileFlags = FILE_SHOWICO|FILE_HIDE;

#pragma PAGEDCODE
NTSTATUS DriverControlDispatchRoutine(IN PDEVICE_OBJECT pDeviceObject,  IN PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	//KdPrint(("Enter DriverControlDispatchRoutine\n"));

	if (!IS_MY_CONTROL_DEVICE_OBJECT(pDeviceObject)) 
    {
		//KdPrint(("һ���������������Ŀ�����Ϣ\n"));
		return DispatchRoutine(pDeviceObject, pIrp);
    }
	//��������������Լ�����������ô���Լ�����

	//�õ���ǰ��ջ
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(pIrp);
	//�õ����뻺������С
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	//�õ������������С
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	//�õ�IOCTL��
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	//KdPrint(("Default��%X\n",code));
	ULONG info = 0;
	//KdPrint(("pDeviceObject = 0x%08x", pDeviceObject));
	//�ļ���
	UNICODE_STRING srcFilePath = {0};
	WCHAR dosFilePath[MY_MAX_PATH] = {0};
	RtlInitEmptyUnicodeString(&srcFilePath, dosFilePath, sizeof(dosFilePath));

	WCHAR * InputBuffer = (WCHAR *)pIrp->AssociatedIrp.SystemBuffer;

	
	switch(code)
	{
	case IOCTL_ENCRYPT:
		{
			
			InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
			KdPrint(("���ܣ�%wZ\n",&srcFilePath));
			
			SafeWallEncryptFile(&srcFilePath, NULL);
		}break;
	case IOCTL_DECRYPT:
		{
			InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
			KdPrint(("���ܣ�%wZ\n",&srcFilePath));

			SafeWallDecryptFile(&srcFilePath, NULL);
		}break;
	case IOCTL_START:
		{
			g_safewall_start = TRUE;
			KdPrint(("��������Χǽ"));
		}break;
	case IOCTL_STOP:
		{
			g_safewall_start = FALSE;
			KdPrint(("��ͣ����Χǽ"));
		}break;
	case IOCTL_SET_USER_ACCESS:
		{
		}break;
	case IOCTL_SET_PROCESS_ACCESS:
		{
		}break;
	case IOCTL_SET_FILE_ACCESS:
		{
		}break;
	case IOCTL_SHOWICO:
		{
			if((int) *InputBuffer)
			{
				FileFlags |= FILE_SHOWICO;
			}
			else
			{
				FileFlags &= (~FILE_SHOWICO);
			}
			
		}break;
	case SAFEWALL_FILEHIDE:
		{
			if((int) *InputBuffer)
			{
				FileFlags |= FILE_HIDE;
			}
			else
			{
				FileFlags &= (~FILE_HIDE);
			}
		}break;
	case IOCTL_QUERY_FILEATTRIBUTES://��ѯ�ļ��Ƿ����ڼ����ļ�
		{
			DWORD * OutputBuffer = (DWORD*)pIrp->AssociatedIrp.SystemBuffer;
			DWORD Flags = NULL;
			//if(g_safewall_start && (FileFlags & FILE_SHOWICO))
			if(TRUE)
			{
				InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
				RtlAppendUnicodeToString(&srcFilePath, InputBuffer);
				LPSAFEWALL_OBJECT SafeWall = 
					(LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
				if(SafeWall != NULL)
				{
					if(GetSafeWallByFilePath(&srcFilePath,NULL,SafeWall))
					{
						Flags = CheckSafeWallFlags(SafeWall);
					}
					ExFreePool(SafeWall);
				}
				
				KdPrint(("����%d, ����%wZ\n",cbin,&srcFilePath));
				if(Flags & SAFEWALL_FLAG_OBJECT)//����������ܶ�������ʾ
				{
					* OutputBuffer = 1;
				}
				else
				{
					* OutputBuffer = 0;
				}
			}
			else
			{
				* OutputBuffer = 0;
			}
			info = sizeof(DWORD);
		}break;
	case IOCTL_TEST:
		{
			
			//InputBuffer[cbin / sizeof(WCHAR) -1] = L'\0';
			KdPrint(("�յ�һ�����Ե���Ϣ\n"));
		}break;
	default:
		{
			KdPrint(("Default��%08x\n",code));
			status = STATUS_INVALID_VARIANT;
		}break;
	}

	end:
	//����IRP�����״̬
	pIrp->IoStatus.Status = status;
	//����IRP����������ֽ���
	pIrp->IoStatus.Information = info;
	//����IRP����
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
	//KdPrint(("Leave DriverControlDispatchRoutine\n"));
	return status;
}