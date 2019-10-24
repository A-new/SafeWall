// FileIco.cpp : CFileIco ��ʵ��


#include "stdafx.h"
#include "FileIco.h"
#include "..\\include\\assist.h"

//��IsMemberOf���ж��Ƿ�������Ҫ����ͼ��Ķ���
//��GetPriority��ָ�������ͼ������ȼ�
//��GetOverlayInfo��ָ��ͼ��·����ͼ��������

//IsMemberOf����OKִ��GetOverlayInfo����ѯ���ĸ�ͼ���ͼ��·����
STDMETHODIMP CFileIco::IsMemberOf(THIS_ LPCWSTR pwszPath, DWORD dwAttrib)
{
	WCHAR buffer =  NULL;
	WCHAR VolumePathName[MY_MAX_PATH] = {0};
	HRESULT hr = S_FALSE;
	DWORD Bytes = 0 ;
	DWORD ShowIco = 0;

	if(hDevice == INVALID_HANDLE_VALUE || NULL == hDevice)
	{
		/*wsprintf(VolumePathName,L"hDevice=0x%08x, Error=%d",hDevice,GetLastError());
		MessageBox(NULL,VolumePathName,VolumePathName,NULL);*/
		return hr;
	}

	if(PathIsDirectory(pwszPath) == FILE_ATTRIBUTE_DIRECTORY) return hr; //�ļ��в���ʾͼ��
	/*FileNameToVolumePathName(pwszPath, VolumePathName, sizeof(VolumePathName));
	MessageBox(NULL,VolumePathName, pwszPath, NULL);*/

	//���̷�ת���ɴ���·���� Ȼ������������������ѯ���ļ��Ƿ�����ļ�
	if(FileNameToVolumePathName(pwszPath, VolumePathName, sizeof(VolumePathName)) &&
		DeviceIoControl(hDevice,IOCTL_QUERY_FILEATTRIBUTES, VolumePathName,
		sizeof(VolumePathName), &ShowIco, sizeof(ShowIco), &Bytes, NULL) && ShowIco)
	{
		hr = S_OK;
	}

	return hr;
}

//GetOverlayInfo����OKִ��GetPriority

STDMETHODIMP CFileIco::GetOverlayInfo(THIS_ LPWSTR pwszIconFile, int cchMax, int * pIndex, DWORD * pdwFlags)
{
	HKEY hSubKey; 
	//��Ico·������pwszIconFile������
	RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"SOFTWARE\\SafeWall\\Client",
		0,KEY_READ| KEY_WRITE, &hSubKey);
	RegQueryValueEx(hSubKey,L"FileIco",NULL, NULL ,(LPBYTE)pwszIconFile ,(LPDWORD)&cchMax);
	RegCloseKey(hSubKey);
	//MessageBox(NULL,pwszIconFile,pwszIconFile,NULL);
	*pIndex=0; 
	*pdwFlags = ISIOI_ICONFILE | ISIOI_ICONINDEX;
	return S_OK;
}

STDMETHODIMP CFileIco::GetPriority(THIS_ int * pIPriority)
{
	*pIPriority = 0;//���ȼ���־��һ����0��
	return S_OK;
}
