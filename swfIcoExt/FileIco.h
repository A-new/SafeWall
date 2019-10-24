// FileIco.h : CFileIco ������

#pragma once
#include "resource.h"       // ������
#include "swfIcoExt_i.h"
#include "shlobj.h"
#include "..\\include\\devdef.h"
#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;


// CFileIco

class ATL_NO_VTABLE CFileIco :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFileIco, &CLSID_FileIco>,
	public IDispatchImpl<IFileIco, &IID_IFileIco, &LIBID_swfIcoExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellIconOverlayIdentifier
{
public:
	HANDLE hDevice;
	CFileIco()
	{
		hDevice = CreateFile(SAFEWALL_DEVICE_DOSNAME,
			GENERIC_READ|GENERIC_WRITE , FILE_SHARE_READ|FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	~CFileIco()
	{
		CloseHandle(hDevice);
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FILEICO)


BEGIN_COM_MAP(CFileIco)
	COM_INTERFACE_ENTRY(IFileIco)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellIconOverlayIdentifier)
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

public:
	//��ʼ��
    HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ LPCITEMIDLIST pidlFolder,
        /* [in] */ IDataObject *pdtobj,
        /* [in] */ HKEY hkeyProgID);

	HRESULT STDMETHODCALLTYPE GetOverlayInfo(
		  OUT PWSTR pwszIconFile,
				int   cchMax,
		  OUT int   *pIndex,
		  OUT DWORD *pdwFlags);

	HRESULT STDMETHODCALLTYPE GetPriority(
		  OUT int *pPriority);

	HRESULT STDMETHODCALLTYPE IsMemberOf(
		  IN PCWSTR pwszPath,
			   DWORD  dwAttrib);

};

OBJECT_ENTRY_AUTO(__uuidof(FileIco), CFileIco)
