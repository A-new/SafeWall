
#include "SafeWall.h"
#include "stdio.h"
#include "myfs.h"

void Dehead(LPSAFEWALL_OBJECT pobj);
void Enhead(LPSAFEWALL_OBJECT pobj);
int CreateSafeWallFilePath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING newFilePath);
NTSTATUS GetMyDeviceObjectByFilePath(IN PUNICODE_STRING FilePath, OUT PDEVICE_OBJECT *DeviceObject);
NTSTATUS SafeWallReplaceFile(HANDLE srcFile, PFILE_OBJECT srcFileObject, POBJECT_ATTRIBUTES srcFileAttributes,
							 HANDLE newFile, PFILE_OBJECT newFileObject ,POBJECT_ATTRIBUTES newFileAttributes, PDEVICE_OBJECT pDeviceObject);

//�����ļ�
#pragma PAGEDCODE
NTSTATUS SafeWallEncryptFile(PUNICODE_STRING srcFilePath ,PDEVICE_OBJECT DeviceObject)//�����ļ�
{
	//Ҫע��DeviceObjectһ����Դͷ�豸�����ӵ��豸������ʱҪ���¼��豸����Ϣ
	DWORD hFlags = NULL;
	HANDLE srcFile;
	PFILE_OBJECT newFileObject, srcFileObject;
	LPSAFEWALL_OBJECT safewall;
	IO_STATUS_BLOCK srcIoStatus = {0};
	LARGE_INTEGER srcOffset = {0};
	OBJECT_ATTRIBUTES srcFileAttributes;
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = DeviceObject;
	PAGED_CODE();
	//�ļ�������ȫ·��
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(srcFilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("��ȡ�ļ���Ӧ�Ĵ����豸ʧ��"));
			return status; 
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}
	//�޷���ռ��
	InitializeObjectAttributes(&srcFileAttributes, srcFilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
	
	status = devCreateFile(&srcFile, &srcFileObject, FILE_GENERIC_READ ,&srcFileAttributes, 
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT , 
		pDeviceObject);

	/*status = ZwCreateFile(&srcFile,FILE_GENERIC_READ, &srcFileAttributes,
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT,NULL,NULL);*/
	
	if(!NT_SUCCESS(status))
	{
		KdPrint(("���ļ� %wZ ʧ��",srcFilePath));
		return status;
	}
	KdPrint(("���ļ� %wZ �ɹ�",srcFilePath));
	
	//����safewall���󣬲���ȡ�ļ���Ϣ
	safewall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
	if(GetSafeWallByFileHandle(srcFile,srcFileObject, pDeviceObject, safewall))
	{
		hFlags = CheckSafeWallFlags(safewall);
	}
	
	
	KdPrint(("hFlags = 0x%08x",hFlags));
	if(hFlags & SAFEWALL_FLAG_FILEGROUP)//���ӵ���ļ����־����ʾ�Ѿ���������
	{
		CallNextCloseFile(srcFile,srcFileObject);
		//devCloseFile(srcFile,pDeviceObject);
		KdPrint(("%wZ �ļ��Ѿ�����",srcFilePath));
		goto end;
	}
	else
	{
		HANDLE newFile;
		UNICODE_STRING newFilePath = {0}; //�ļ�ȫ·��
		UNICODE_STRING FileName = {0};//�ļ���
		WCHAR dosFilePath[MY_MAX_PATH] = {0}; //�ļ�ȫ·������
		FILE_STANDARD_INFORMATION FileStandardInfo = {0};//�ļ���Ϣ
		IO_STATUS_BLOCK newStatus={0};
		OBJECT_ATTRIBUTES newFileAttributes;
		LARGE_INTEGER newOffset = {0};
		char * buffer = NULL;
		
		RtlInitEmptyUnicodeString(&newFilePath, dosFilePath, sizeof(dosFilePath));
		CreateSafeWallFilePath(srcFilePath, &newFilePath);

		//���뻺����
		buffer = (char *)ExAllocatePool(NonPagedPool, FILEPAGE_BUFFER_SIZE);
		
		status = devQueryInformationFile(srcFile,srcFileObject, &srcIoStatus,&FileStandardInfo,
			sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation,pDeviceObject);
		FileStandardInfo.EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;//���ļ�����

		KdPrint(("���ļ��� %wZ\t, ����:%d",&newFilePath,FileStandardInfo.EndOfFile.LowPart));
		InitializeObjectAttributes(&newFileAttributes, &newFilePath,
			OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);
		
		//�޷���ռ��
		status = devCreateFile(&newFile,&newFileObject, FILE_GENERIC_WRITE,&newFileAttributes, 
			&srcIoStatus, &FileStandardInfo.EndOfFile,FILE_ATTRIBUTE_NORMAL, NULL, FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT, 
			pDeviceObject);

		/*status = ZwCreateFile(&newFile, FILE_GENERIC_WRITE,&newFileAttributes,
			&newStatus,&FileStandardInfo.EndOfFile ,FILE_ATTRIBUTE_NORMAL,NULL,FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE| FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT,NULL,0);*/

		if(!NT_SUCCESS(status))
		{
			KdPrint(("�������ļ�ʧ�� status = 0x%08x",status));
			//CallNextCloseFile(srcFile, srcFileObject);
			CallNextCloseFile(srcFile,srcFileObject);
			ExFreePool(buffer);//�ͷŻ�����
			goto end;
		}
		KdPrint(("�������ļ��ɹ�"));

		//д���ļ�ͷ
		InitSafeWallObject(safewall);
		status = SetSafeWallToFileHead(safewall, newFile,newFileObject, pDeviceObject);

		if(!NT_SUCCESS(status))
		{
			KdPrint(("�ļ�ͷд��ʱ�������� status = 0x%08x",status));
			CallNextCloseFile(srcFile,srcFileObject);
			CallNextCloseFile(newFile,newFileObject);
			/*devCloseFile(srcFile,pDeviceObject);
			devCloseFile(newFile,pDeviceObject);*/
			ZwDeleteFile(&newFileAttributes);
			ExFreePool(buffer);//�ͷŻ�����
			goto end;
		}
		newOffset.QuadPart = SAFEWALL_OBJECT_SIZE;
		srcOffset.QuadPart = 0;
		
		//��ȡԭ�ļ������ܲ����Ƶ���ʱ�ļ�
		while(TRUE)
		{
			status = devReadFile(srcFile,srcFileObject, &srcIoStatus, buffer,
				PAGE_BUFFER_SIZE, &srcOffset, pDeviceObject);

			/*status = ZwReadFile(srcFile,NULL,NULL,NULL, &srcIoStatus, buffer,
				FILEPAGE_BUFFER_SIZE, &srcOffset, NULL);*/
			if(!NT_SUCCESS(status))
			{
				if(status == STATUS_END_OF_FILE)
				{
					KdPrint(("�ļ����꣺%d",srcOffset.LowPart ));
					status = STATUS_SUCCESS;
				}
				break;
			}
			//KdPrint(("��ȡ��%d, %s",srcOffset.LowPart,buffer ));
			//���ܻ�����
			Encode(safewall->AlgorithmVersion, buffer ,srcIoStatus.Information, &srcOffset, 
				&safewall->privateKey, &safewall->CompanyId);
			srcOffset.QuadPart += srcIoStatus.Information;
			//������д�����ļ�
			KdPrint(("srcIoStatus.Information = %u,srcOffset=%d,newOffset=%d",
				srcIoStatus.Information,srcOffset.LowPart,newOffset.LowPart));
			status = devWriteFile(newFile, newFileObject, &newStatus, buffer, srcIoStatus.Information,
				&newOffset, pDeviceObject);
			/*status = ZwWriteFile(newFile,NULL, NULL,NULL, &newStatus, buffer,  srcIoStatus.Information,
				&newOffset, NULL);*/
			if(!NT_SUCCESS(status))
			{
				//error:STATUS_WORKING_SET_QUOTA
				KdPrint(("�ļ�д��ʱ��������,ȡ������ status = 0x%08x",status));
				CallNextCloseFile(srcFile, srcFileObject);
				CallNextCloseFile(newFile, newFileObject);
				/*devCloseFile(srcFile,pDeviceObject);
				devCloseFile(newFile,pDeviceObject);*/
				//ZwDeleteFile(&newFileAttributes);
				ExFreePool(buffer);//�ͷŻ�����

				status = STATUS_DATA_ERROR;
				goto end;
			}
			//KdPrint(("д��λ�ã�%d,д�볤��:%d",newOffset.LowPart,newStatus.Information));
			newOffset.QuadPart += newStatus.Information;
		}
		ExFreePool(buffer);//�ͷŻ�����
		
		/*status = SafeWallReplaceFile(srcFile, &srcFileAttributes, newFile,
			&newFileAttributes, pDeviceObject);*/

		UNICODE_STRING newFileName = {0};//�ļ���
		IO_STATUS_BLOCK ioStatus={0};
		MY_FILE_RENAME_INFORMATION FileRenameInfo ={0}; //�����ṹ��

		RtlInitEmptyUnicodeString(&newFileName, FileRenameInfo.FileName, sizeof(FileRenameInfo.FileName));
		GetFileNameForPath(srcFileAttributes.ObjectName, &newFileName);
		FileRenameInfo.FileNameLength = newFileName.Length;
		FileRenameInfo.ReplaceIfExists = 0 ;
		FileRenameInfo.RootDirectory = NULL;

		CallNextCloseFile(srcFile,srcFileObject);
		status = ZwDeleteFile(&srcFileAttributes);//ɾ��Դ�ļ�
		if(NT_SUCCESS(status))
		{
			KdPrint(("�ļ�ɾ���ɹ����滻���ļ�"));

			status = devSetInformationFile(newFile, newFileObject,  &ioStatus,&FileRenameInfo,
				sizeof(MY_FILE_RENAME_INFORMATION), FileRenameInformation,pDeviceObject);
		
			CallNextCloseFile(newFile, newFileObject);
			if(!NT_SUCCESS(status))
			{
				KdPrint(("�������������ļ���Ϣʧ�ܣ�status = 0x%08x", status));	
			}
		}
		else
		{
			KdPrint(("Դ�ļ�ɾ��ʧ�ܣ�ȡ��������status = 0x%08x",status));
			CallNextCloseFile(newFile, newFileObject);
			ZwDeleteFile(&newFileAttributes);
		}
	}
	end:
	ExFreePool(safewall);
	return status;
}

//�����ļ�
#pragma PAGEDCODE
NTSTATUS SafeWallDecryptFile(PUNICODE_STRING srcFilePath, PDEVICE_OBJECT DeviceObject)//�����ļ�
{
	////Ҫע��DeviceObjectһ����Դͷ�豸�����ӵ��豸������ʱҪ���¼��豸����Ϣ
	DWORD hFlags=NULL;
	HANDLE srcFile;
	PFILE_OBJECT srcFileObject, newFileObject;
	LPSAFEWALL_OBJECT safewall;
	IO_STATUS_BLOCK srcIoStatus = {0};
	LARGE_INTEGER srcOffset = {0};
	OBJECT_ATTRIBUTES srcFileAttributes;
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT pDeviceObject = DeviceObject;
	PAGED_CODE();
	//�ļ�������ȫ·��
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(srcFilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("��ȡ�ļ���Ӧ�Ĵ����豸ʧ��"));
			return status;
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}
	InitializeObjectAttributes(&srcFileAttributes, srcFilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	status = devCreateFile(&srcFile, &srcFileObject, FILE_GENERIC_READ ,&srcFileAttributes, 
		&srcIoStatus, NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT , 
		pDeviceObject);

	if(!NT_SUCCESS(status))
	{
		KdPrint(("���ļ� %wZ ʧ��",srcFilePath));
		return status;
	}
	KdPrint(("���ļ� %wZ �ɹ�",srcFilePath));

	
	//����safewall���󣬲���ȡ�ļ���Ϣ
	safewall = (LPSAFEWALL_OBJECT)ExAllocatePool(NonPagedPool , SAFEWALL_OBJECT_SIZE);
	if(GetSafeWallByFileHandle(srcFile, srcFileObject,pDeviceObject, safewall))
	{
		hFlags = CheckSafeWallFlags(safewall);
	}
	KdPrint(("hFlags = 0x%08x",hFlags));
	if(!(hFlags & SAFEWALL_FLAG_FILEGROUP))//�ļ���û�б�����
	{
		CallNextCloseFile(srcFile,srcFileObject);
		KdPrint(("%wZ �ļ�δ����",srcFilePath));
		goto end;
	}
	else
	{
		//�ļ��Ѿ��������ˣ���Ҫ����
		HANDLE newFile;
		UNICODE_STRING newFilePath = {0}; //�ļ�ȫ·��
		UNICODE_STRING FileName = {0};//�ļ���
		WCHAR dosFilePath[MY_MAX_PATH] = {0}; //�ļ�ȫ·������
		FILE_STANDARD_INFORMATION FileStandardInfo = {0};//�ļ���Ϣ
		IO_STATUS_BLOCK newStatus={0};
		OBJECT_ATTRIBUTES newFileAttributes;
		LARGE_INTEGER newOffset = {0};
		char * buffer = NULL;
		
		RtlInitEmptyUnicodeString(&newFilePath, dosFilePath, sizeof(dosFilePath));
		CreateSafeWallFilePath(srcFilePath, &newFilePath);

		//���뻺����
		buffer = (char *)ExAllocatePool(NonPagedPool, FILEPAGE_BUFFER_SIZE);

		status = devQueryInformationFile(srcFile,srcFileObject ,&srcIoStatus,&FileStandardInfo,
			sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation,pDeviceObject);
		FileStandardInfo.EndOfFile.QuadPart += SAFEWALL_OBJECT_SIZE;//���ļ�����

		KdPrint(("���ļ��� %wZ\n",&newFilePath));
		InitializeObjectAttributes(&newFileAttributes, &newFilePath,
			OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

		status = devCreateFile(&newFile, &newFileObject, FILE_GENERIC_WRITE,&newFileAttributes, 
			&srcIoStatus, &FileStandardInfo.EndOfFile,FILE_ATTRIBUTE_NORMAL, NULL, FILE_CREATE, 
			FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING| FILE_SYNCHRONOUS_IO_NONALERT, 
			pDeviceObject);

		if(!NT_SUCCESS(status))
		{
			KdPrint(("�������ļ�ʧ��"));
			CallNextCloseFile(srcFile, srcFileObject);
			ExFreePool(buffer);//�ͷŻ�����
			goto end;
		}

		//���������ļ�ͷ����Ϊsafewall�ṹ���Ѿ���ȡ������
		srcOffset.QuadPart = SAFEWALL_OBJECT_SIZE;
		newOffset.QuadPart = 0;
		KdPrint(("�����ļ�ͷ�ɹ�,�㷨��%d",safewall->AlgorithmVersion));
		while(TRUE)
		{
			status = devReadFile(srcFile,srcFileObject, &srcIoStatus, buffer,
				PAGE_BUFFER_SIZE, &srcOffset, pDeviceObject);
			srcOffset.QuadPart += srcIoStatus.Information;
			if(!NT_SUCCESS(status))
			{
				if(status == STATUS_END_OF_FILE)
					status = STATUS_SUCCESS;
				break;
			}

			Decode(safewall->AlgorithmVersion, buffer,srcIoStatus.Information, &newOffset,
				&safewall->privateKey, &safewall->CompanyId);
			
			//�����ܻ���д�����ļ�
			status = devWriteFile(newFile, newFileObject, &newStatus, buffer, srcIoStatus.Information,
				&newOffset, pDeviceObject);

			if(!NT_SUCCESS(status))
			{
				KdPrint(("�ļ�д��ʱ�������� status = 0x%08x",status));
				CallNextCloseFile(srcFile, srcFileObject);
				CallNextCloseFile(newFile, newFileObject);
				ZwDeleteFile(&newFileAttributes);
				ExFreePool(buffer);//�ͷŻ�����
				status = STATUS_DATA_ERROR;
				goto end;
			}

			newOffset.QuadPart += newStatus.Information;
		}
		ExFreePool(buffer);//�ͷŻ�����
		
		status = SafeWallReplaceFile(srcFile,srcFileObject, &srcFileAttributes, 
			newFile, newFileObject,&newFileAttributes, pDeviceObject);
	}
	end:
	ExFreePool(safewall);
	return status;

}

//���ļ��滻���ļ����������ļ�ɾ��
#pragma PAGEDCODE
NTSTATUS SafeWallReplaceFile(HANDLE srcFile, PFILE_OBJECT srcFileObject, POBJECT_ATTRIBUTES srcFileAttributes,
							 HANDLE newFile, PFILE_OBJECT newFileObject ,POBJECT_ATTRIBUTES newFileAttributes, PDEVICE_OBJECT pDeviceObject)
{
	NTSTATUS status;
	UNICODE_STRING FileName = {0};//�ļ���
	IO_STATUS_BLOCK ioStatus={0};
	MY_FILE_RENAME_INFORMATION FileRenameInfo ={0}; //�����ṹ��
	FILE_BASIC_INFORMATION FileBasicInfo = {0};//�ļ���Ϣ

	/*status = devQueryInformationFile(srcFile,&ioStatus,&FileBasicInfo,
			sizeof(FILE_BASIC_INFORMATION), FileBasicInformation,pDeviceObject);
	if(!NT_SUCCESS(status))
	{
		ZwClose(srcFile); ZwClose(newFile); ZwDeleteFile(newFileAttributes); return status;
	}

	status = devSetInformationFile(newFile,  &ioStatus,&FileBasicInfo,
			sizeof(FILE_BASIC_INFORMATION), FileBasicInformation,pDeviceObject);
	if(!NT_SUCCESS(status))
	{
		ZwClose(srcFile); ZwClose(newFile); ZwDeleteFile(newFileAttributes); return status;
	}
	*/
	RtlInitEmptyUnicodeString(&FileName, FileRenameInfo.FileName, sizeof(FileRenameInfo.FileName));
	GetFileNameForPath(srcFileAttributes->ObjectName, &FileName);
	FileRenameInfo.FileNameLength = FileName.Length;
	FileRenameInfo.ReplaceIfExists = 0 ;
	FileRenameInfo.RootDirectory = NULL;

	CallNextCloseFile(srcFile,srcFileObject);
	status = ZwDeleteFile(srcFileAttributes);//ɾ��Դ�ļ�
	if(NT_SUCCESS(status))
	{
		KdPrint(("�ļ�ɾ���ɹ����滻���ļ�"));

		status = devSetInformationFile(newFile,newFileObject, &ioStatus,&FileRenameInfo,
			sizeof(MY_FILE_RENAME_INFORMATION), FileRenameInformation,pDeviceObject);
		
		CallNextCloseFile(newFile, newFileObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("�������������ļ���Ϣʧ�ܣ�status = 0x%08x", status));	
		}
	}
	else
	{
		KdPrint(("Դ�ļ�ɾ��ʧ�ܣ�ȡ��������status = 0x%08x",status));
		CallNextCloseFile(newFile,newFileObject);
		ZwDeleteFile(newFileAttributes);
	}

	return status;

}

//���ļ�ȫ·��ȡ���ļ���·��
#pragma PAGEDCODE
int GetFileDirectoryForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileDirectoryForPath)
{
	int offset;
	int count;
	int i, j;
	WCHAR wc;
	offset = srcFilePath->Length / sizeof(WCHAR) - 1;
	while(offset > 0 && L'\\' != srcFilePath->Buffer[--offset]);//offset��λ�� L'\\' ��һλ

	if(offset * sizeof(WCHAR) > FileDirectoryForPath->MaximumLength) return offset * sizeof(WCHAR);

	for(count = 0; offset > count; count++)
	{
		FileDirectoryForPath->Buffer[count] = srcFilePath->Buffer[count];
	}
	FileDirectoryForPath->Buffer[count] = L'\\';
	FileDirectoryForPath->Length = (count + 1) * sizeof(WCHAR);
	return FileDirectoryForPath->Length;
}

//��·������ȡ�ļ���
#pragma PAGEDCODE
int GetFileNameForPath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING FileName)
{
	int offset;
	int count;
	int i, j;
	WCHAR wc;
	offset = srcFilePath->Length / sizeof(WCHAR) - 1;

	//ȡ�õߵ��ļ���
	for(count = 0; offset >= count  && L'\\' != srcFilePath->Buffer[offset -count]; count ++)
	{
		FileName->Buffer[count] = srcFilePath->Buffer[offset -count];
	}

	if(count * sizeof(WCHAR) > FileName->MaximumLength) return count * sizeof(WCHAR) ;

	FileName->Length = count * sizeof(WCHAR);

	//���ļ����ַ���ԭ
	for(i=0, j=count-1; i < j; i++,j--)
	{
		wc = FileName->Buffer[i];
		FileName->Buffer[i] = FileName->Buffer[j];
		FileName->Buffer[j] = wc;
	}

	//KdPrint(("�ļ��� : %wZ", FileName));
	return FileName->Length ;
}

//������ʱ�ļ���
#pragma PAGEDCODE
int CreateSafeWallFilePath(IN PUNICODE_STRING srcFilePath,OUT PUNICODE_STRING newFilePath)
{
	int offset;
	BOOLEAN begin;
	UUID uuid;
	WCHAR name[32];
	UNICODE_STRING FileName;

	if(newFilePath->MaximumLength < MY_MAX_PATH || 
		newFilePath->MaximumLength < srcFilePath->MaximumLength)
	{
		return MY_MAX_PATH > srcFilePath->MaximumLength ? MY_MAX_PATH : srcFilePath->MaximumLength + sizeof(name);
	}
	ExUuidCreate(&uuid);

	swprintf(name, L"\\%08x.wall", uuid.Data1);

	offset = srcFilePath->Length / sizeof(WCHAR); //��ȫ��
	RtlCopyUnicodeString(newFilePath, srcFilePath);
	while(offset > 0 && L'\\' != srcFilePath->Buffer[--offset]);//offset��λ�� L'\\' ��һλ
	newFilePath->Length = offset * sizeof(WCHAR);
	newFilePath->Buffer[offset] = L'\0';

	RtlAppendUnicodeToString(newFilePath, name);
	return newFilePath->Length;
}

//��safewall������ܺ�д���ļ�ͷ
#pragma PAGEDCODE
NTSTATUS SetSafeWallToFileHead(IN LPSAFEWALL_OBJECT safewall,IN HANDLE FileHandle,
							   IN PFILE_OBJECT FileObject ,IN PDEVICE_OBJECT DeviceObject)
{
	NTSTATUS status;
	LPSAFEWALL_OBJECT enswo;
	IO_STATUS_BLOCK ioStatus={0};
	LARGE_INTEGER offset = {0};

	enswo = (LPSAFEWALL_OBJECT)ExAllocatePool( NonPagedPool, SAFEWALL_OBJECT_SIZE);
	*enswo = *safewall;
	if(enswo == NULL)
	{
		return -1;
	}
	Enhead(enswo);
	status = devWriteFile(FileHandle ,FileObject, &ioStatus, enswo, SAFEWALL_OBJECT_SIZE,  &offset, DeviceObject);
	ExFreePool(enswo);
	return status;
}

//���safewall�����еļ��ܱ�־
#pragma PAGEDCODE
DWORD CheckSafeWallFlags(LPSAFEWALL_OBJECT SafeWall)
{
	DWORD dwRet = 0;
	SafeWall->swv.myCompanyName[sizeof(SafeWall->swv.myCompanyName)/sizeof(WCHAR) -1] = L'\0';//��ȫ��
	if(!InlineIsEqualSWID(&SafeWall->swv.myId, (SWID*)&MYSAFEWALLGUID) ||
		wcscmp(SafeWall->swv.myCompanyName, MYSAFEWALLCOMPANYNAME)) 
	{
		//�ⲻ�����ǵļ����ļ�
		//KdPrint(("�ⲻ������Χǽ�ļ����ļ�\n"));
		return dwRet;
	}
	//����������������ǵļ��ܶ����ˣ���ӱ�־
	dwRet = SAFEWALL_FLAG_OBJECT;

	if(InlineIsEqualSWID(&SafeWall->FileGroupId, &gStandardSafeWallObj->CompanyId)) 
	{
		//KdPrint(("�������ڹ�˾��GUID\n"));
		dwRet |= (SAFEWALL_FLAG_MANAGEMENT | SAFEWALL_FLAG_FILEGROUP);
	}
	else if(InlineIsEqualSWID(&SafeWall->FileGroupId, &gStandardSafeWallObj->FileGroupId)) 
	{
		//KdPrint(("���������ļ����GUID\n"));
		dwRet |= SAFEWALL_FLAG_FILEGROUP;
	}
	return dwRet;
}

//���ļ�����ȡ��safewall����
#pragma PAGEDCODE
BOOLEAN GetSafeWallByFilePath(IN PUNICODE_STRING FilePath, IN PDEVICE_OBJECT DeviceObject,OUT LPSAFEWALL_OBJECT SafeWall)
{
	IO_STATUS_BLOCK iostatus ={0};
	HANDLE hFile;
	PFILE_OBJECT FileObject;
	LARGE_INTEGER offset = {0};
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_OBJECT  pDeviceObject = DeviceObject;
	OBJECT_ATTRIBUTES FileAttributes;
	PAGED_CODE();
	//�ļ�������ȫ·��
	if(NULL == pDeviceObject || pDeviceObject == gMyControlDeviceObject)
	{
		status = GetMyDeviceObjectByFilePath(FilePath, &pDeviceObject);
		if(!NT_SUCCESS(status))
		{
			KdPrint(("��ȡ�ļ���Ӧ�Ĵ����豸ʧ��"));
			return FALSE; 
		}
		pDeviceObject = ((PDEVICE_EXTENSION) pDeviceObject->DeviceExtension)->AttachedToDeviceObject;
	}

	InitializeObjectAttributes(&FileAttributes, FilePath,
		OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE, NULL, NULL);

	status = devCreateFile(&hFile,&FileObject,  FILE_GENERIC_READ,
		&FileAttributes,&iostatus,NULL, FILE_ATTRIBUTE_NORMAL, NULL, FILE_OPEN,
		FILE_NON_DIRECTORY_FILE|FILE_NO_INTERMEDIATE_BUFFERING|FILE_SYNCHRONOUS_IO_NONALERT,
		pDeviceObject);
	
	if(!NT_SUCCESS(status))
	{
		return FALSE;
	}
	
	status = devReadFile(hFile, FileObject, &iostatus, SafeWall, SAFEWALL_OBJECT_SIZE,
		&offset, pDeviceObject);
	CallNextCloseFile(hFile, FileObject);
	
	if(!NT_SUCCESS(status) || SAFEWALL_OBJECT_SIZE > iostatus.Information)
	{
		return FALSE;
	}
	Dehead(SafeWall);
	KdPrint(("privateKey=%08x-%08x-%08x-%08x",SafeWall->privateKey.data[0],
		SafeWall->privateKey.data[1],SafeWall->privateKey.data[2],SafeWall->privateKey.data[3]));
	//���safewall������

	
	return TRUE;
}

//�Ӿ����ȡ��safewall����
#pragma PAGEDCODE
BOOLEAN GetSafeWallByFileHandle(IN HANDLE FileHandle, IN PFILE_OBJECT FileObject, IN PDEVICE_OBJECT DeviceObject, OUT LPSAFEWALL_OBJECT SafeWall)
{
	IO_STATUS_BLOCK iostatus;
	LARGE_INTEGER offset = {0};
	LARGE_INTEGER CurrentByteOffset = {0};
	NTSTATUS status = STATUS_SUCCESS;


	status = devReadFile(FileHandle, FileObject,&iostatus, SafeWall, SAFEWALL_OBJECT_SIZE,
		&offset, DeviceObject);
	//ʧ�ܻ����ļ���LPSAFEWALL_OBJECT�ṹ��ҪС���϶���û����
	if(!NT_SUCCESS(status) || SAFEWALL_OBJECT_SIZE > iostatus.Information)
	{
		return FALSE;
	}

	//����
	Dehead(SafeWall);
	KdPrint(("privateKey=%08x-%08x-%08x-%08x",SafeWall->privateKey.data[0],
		SafeWall->privateKey.data[1],SafeWall->privateKey.data[2],SafeWall->privateKey.data[3]));
	//���safewall������
	
	return TRUE;
}


//��·����ȡ�豸ָ��
#pragma PAGEDCODE
NTSTATUS GetMyDeviceObjectByFilePath(IN PUNICODE_STRING FilePath, OUT PDEVICE_OBJECT *DeviceObject)
{
	NTSTATUS status;
	ULONG numDevices;
    PDEVICE_OBJECT *devList;
	UNICODE_STRING tmpFilePath;
	PUNICODE_STRING pDeviceName = NULL;
	ULONG i=0;

	//IoEnumerateDeviceObjectList�����Ӷ������ñ�֤�豸ָ�벻ʧЧ
	status = IoEnumerateDeviceObjectList( gMyDriverObject, NULL, 0, &numDevices);

	ASSERT(STATUS_BUFFER_TOO_SMALL == status);
	devList = (PDEVICE_OBJECT *)ExAllocatePool( NonPagedPool, (numDevices * sizeof(PDEVICE_OBJECT)) );
	if (NULL == devList)
	{
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	status = IoEnumerateDeviceObjectList(gMyDriverObject,
                        devList,(numDevices * sizeof(PDEVICE_OBJECT)), &numDevices);
    
    if (!NT_SUCCESS( status ))
    {
        ExFreePool( devList );
        return status;
    }

	status = -1;
	tmpFilePath.Buffer = FilePath->Buffer;
	tmpFilePath.Length = FilePath->Length;
	tmpFilePath.MaximumLength = FilePath->MaximumLength;
	for ( i=0; numDevices > i ; i++)
    {
		if (IS_MY_DEVICE_OBJECT(devList[i]))
        {
			pDeviceName = &((PDEVICE_EXTENSION)devList[i]->DeviceExtension)->DeviceName;
			if(pDeviceName->Length > FilePath->Length || 0 == pDeviceName->Length) continue;
			//KdPrint(("devNameString = %wZ", pDeviceName));
			tmpFilePath.Length = pDeviceName->Length;
			if(!RtlCompareUnicodeString(&tmpFilePath, pDeviceName, TRUE))
			{
				*DeviceObject = devList[i];
				status = 0;
				break;
			}
        }
    }

	for ( i=0; numDevices > i ; i++)
	{
		ObDereferenceObject( devList[i] );
	}
	ExFreePool(devList);
	//ObDereferenceObject( FileObject ); //������ٶ��ļ����������
	return status;
}
