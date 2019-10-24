
//#define _CRT_SECURE_NO_DEPRECATE 1
//#define _CRT_NONSTDC_NO_DEPRECATE 1
#include "assist.h"
#include "devdef.h"

BOOL mylog(IN char *file,IN const char * text)
{
	FILE * flog = {0};
	char time[32] = {0};
	SYSTEMTIME systime;
	if(NULL == text) return FALSE;
	GetLocalTime(&systime);
	sprintf_s(time,32, "%04d-%02d-%02d %02d:%02d:%02d:%03d",systime.wYear,systime.wMonth, systime.wDay, 
		systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);

	//��ʱ��д�뵽�ļ�
	fopen_s(&flog, file , "a+");
	if(flog == NULL) return FALSE;
	
	if(0 > fprintf(flog, "%s\t%s\r\n",time, text))
	{
		fclose(flog);
		return FALSE;
	}

	fclose(flog);
	return TRUE;
}


BOOL FileNameToVolumePathName(IN LPCWSTR FileName,OUT LPWSTR VolumePathName,IN DWORD cchMax)
{
	DWORD offset = 0;
	DWORD copyset =0;
	WCHAR szDosName[16] = {0};
	WCHAR szVolumeName[32] = {0};
	cchMax = cchMax / sizeof(WCHAR);
	while(FileName[offset] != L'\0' && 64 > offset && (szDosName[offset] = FileName[offset]) &&
		L':' !=  FileName[offset++]);
	
	if(!QueryDosDevice(szDosName,szVolumeName, sizeof(szVolumeName)))
	{
		return FALSE;
	}

	while(cchMax > copyset && L'\0' != szVolumeName[copyset] && 
		(VolumePathName[copyset] = szVolumeName[copyset++]));

	while(cchMax > copyset && L'\0' != FileName[offset] &&
		(VolumePathName[copyset++] = FileName[offset++]));
	VolumePathName[copyset] = L'\0';
	return TRUE;
}


BOOL GetShortcutTarget(IN LPWSTR lpwLnkFile,OUT LPWSTR lpwTarFile)
{  
    BOOL bReturn = FALSE;  
    IShellLink *pShellLink;
	IPersistFile *ppf;

	bReturn = CoCreateInstance (CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,  
			IID_IShellLink, (void **)&pShellLink) >= 0;  
	if(bReturn)  
	{  
		bReturn = pShellLink->QueryInterface(IID_IPersistFile, (void **)&ppf) >= 0;  
		if(bReturn)  
		{  
			bReturn = ppf->Load(lpwLnkFile, TRUE) >= 0;  
			if(bReturn)  
			{  
				pShellLink->GetPath(lpwTarFile, MAX_PATH, NULL, 0);  
			}  
			ppf->Release();  
		}  
		pShellLink->Release();  
	}

    return bReturn;  
}  

int InsertOnceName(WCHAR * tagString,int tagstrLenght,int tagAllLenght, WCHAR * pName)
{
	int i=0,j,len;
	WCHAR  *p;
	BOOL fir = TRUE;
	
	do
	{
		p = tagString + i;

		if(fir == TRUE && *p == *pName && 0 == wcscmp(p, pName))
		{
			return tagstrLenght;
		}
		
		fir = (tagString[i] == L'\0') ? TRUE : FALSE;

	}while(tagstrLenght > (++i));

	i = tagstrLenght-1;
	if(tagAllLenght > tagstrLenght)
	{
		if(tagString[i-2]!=L'\0')
		{
			tagString[i-1] = L'\0';
		}
		else if(tagString[i-1]!=L'\0')
		{
			tagString[i] = L'\0';
			++i;
		}
	}

	len = wcslen(pName);
	if(i + len +1 > tagAllLenght) return tagstrLenght;

	for(j = 0 ; len >j ; j++,i++)
	{
		tagString[i] = pName[j];
	}

	tagString[i++] = L'\0';

	return i;
}

NTSTATUS _NtQueryInformationProcess(HANDLE ProcessHandle,PROCESSINFOCLASS ProcessInformationClass,
    PVOID ProcessInformation,ULONG ProcessInformationLength,PULONG ReturnLength) 
{
	HMODULE hNtDll = LoadLibrary(TEXT("ntdll.dll"));
	if (hNtDll == NULL) { return(-1);}
	NTSTATUS lStatus = -1;  

	//��ȡ������ַ
	PFN_NTQUERYINFORMATIONPROCESS _NtQueryInformationProcessPtr = 
		(PFN_NTQUERYINFORMATIONPROCESS)GetProcAddress(hNtDll, "NtQueryInformationProcess");
	if (_NtQueryInformationProcessPtr != NULL) {
		lStatus = _NtQueryInformationProcessPtr(ProcessHandle, ProcessInformationClass, 
			ProcessInformation, ProcessInformationLength, ReturnLength);
	}   
   
	FreeLibrary(hNtDll);
	return(lStatus);
}

DWORD WINAPI FollowParentProcess(LPVOID hMainWnd)
{
	LONG status;
	HANDLE hProcess;
	PROCESS_BASIC_INFORMATION pbi;
	HANDLE hParentProcess = NULL;

	//ֱ�ӵ���GetCurrentProcess��ʧ�ܣ�Ҫ��OpenProcessȡ�þ��
	hProcess =  OpenProcess(PROCESS_ALL_ACCESS, FALSE, GetCurrentProcessId());
	status = _NtQueryInformationProcess(hProcess, ProcessBasicInformation,
		(PVOID)&pbi,sizeof(PROCESS_BASIC_INFORMATION),NULL);
	CloseHandle(hProcess);
	if(!NT_SUCCESS(status)) goto end;//��ȡ�����̵�Basic Info��������Ϣ��ʧ��

	//pbi.Reserved3 �Ǹ�����ID
	hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, (DWORD)pbi.Reserved3 ); 
	if(hParentProcess == INVALID_HANDLE_VALUE || NULL == hParentProcess) goto end;//��ȡ�����̾��ʧ��
	
	WaitForSingleObject (hParentProcess,INFINITE);//���޵ȴ������̽�����������CPU
	CloseHandle(hParentProcess);

//����������
end:
	//���lParamΪ�գ���ǿ�ƽ������̣�
	//����������һ�����ڵľ������������ڽ���ע��
	NULL == hMainWnd ? ExitProcess(0) : PostMessage((HWND)hMainWnd, WM_DESTROY, 0, 0);
	return 0;
}

BOOL SendDeviceControl(__in int code,__inout char * buffer,__inout int buflen)
{
	BOOL ret = TRUE;
	DWORD Bytes = 0 ;
	DWORD IOCTL ;
	DWORD error;
	char text[MAX_PATH] = {0};
	HANDLE hDevice = CreateFile(SAFEWALL_DEVICE_DOSNAME,
		GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hDevice == INVALID_HANDLE_VALUE || NULL == hDevice)
	{
		error = GetLastError();
		
		sprintf_s(text,sizeof(text), "�豸��ʧ�ܣ�������룺%d ,hDevice = 0x%08x" , error, hDevice);
		MessageBoxA(NULL,text,"����Χǽ",MB_OK);
		return FALSE;
	}

	
	IOCTL = CTL_CODE(FILE_DEVICE_UNKNOWN, code,  METHOD_BUFFERED, FILE_ANY_ACCESS);
	DeviceIoControl(hDevice,IOCTL, buffer, buflen, NULL, NULL, &Bytes, NULL);

	if(ret == FALSE)
	{
		MessageBox(NULL,TEXT("������Ϣ����ʧ��"),TEXT("����Χǽ"),MB_OK);
	}
	CloseHandle(hDevice);
	return ret;
}



int bytes_to_format_chars(byte * src, int srclen, char * buf, int buflen)
{
	int len =0;
	int i = 0;
	if(NULL == buf)
	{
		len = srclen+1;
		for(i=0; i < srclen; i++,src++)
		{
			if(*src == '\0' || *src == '\t' || *src == '\n'
				|| *src == '\\' || *src == '\r' || *src == '\"')
			{
				len++;
			}
		}
		return len;
	}

	--buflen;//Ԥ��'\0'
	for(i=0; i<srclen && len<buflen; i++,src++)
	{
		switch (*src)
		{
		case '\0':
			{
				buf[len++] = '\\';
				buf[len++] = '0';
			}break;
		case '\t':
			{
				buf[len++] = '\\';
				buf[len++] = 't';
			}break;
		case '\r':
			{
				buf[len++] = '\\';
				buf[len++] = 'r';
			}break;
		case '\n':
			{
				buf[len++] = '\\';
				buf[len++] = 'n';
			}break;
		case '\\':
			{
				buf[len++] = '\\';
				buf[len++] = '\\';
			}break;
		case '\"':
			{
				buf[len++] = '\\';
				buf[len++] = '"';
			}break;
		default:
			{
				buf[len++] = *src;
			}break;
		}
	}
	buf[len] = '\0';
	return len;
}

int format_chars_to_bytes(char * src, byte * buf, int buflen)
{
	int len =0;
	if(NULL == buf)
	{
		for(;'\0' != *src; src++)
		{
			if(*src == '\\')
			{
				src++;//ֱ������һλ
			}
			len++;
		}
		return len;
	}

	for(; '\0' != *src && len<buflen; src++)
	{
		if(*src == '\\')
		{
			src++;
			switch (*src)
			{
			case '0':
				{
					buf[len++] = '\0';
				}break;
			case 't':
				{
					buf[len++] = '\t';
				}break;
			case 'r':
				{
					buf[len++] = '\r';
				}break;
			case 'n':
				{
					buf[len++] = '\n';
				}break;
			case '\\':
				{
					buf[len++] = '\\';
				}break;
			case '"':
				{
					buf[len++] = '\"';
				}break;
			default:
				{
					buf[len++] = *src;
				}break;
			}
		}
		else
		{
			buf[len++] = *src;
		}
	}
	return len;
}
