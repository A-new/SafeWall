// SafeWallExt.h : CSafeWallExt ������

#pragma once
#include "resource.h"       // ������



#include "SwShellExt_i.h"
#include "shlobj.h"
//���ļ������->��->�򵥵�ATL����������

//1.  HKEY_CLASSES_ROOT\Folder\Shellex\ContextMenuHandlers ���м����µ��
//	����Ĭ�ϵļ�ֵ�м�����չdll��GUID .
//������DLL�ļ���ʽ��Ϊshell���, ���������folder���ļ��У���Ч
//�������ӵ�*�У���������ļ���Ч
//
//2.  HKEY_LOCAL_MACHINE\Software\Microsoft\Windows\CurrentVersion\Shell Extensions\Approved 
//���м�����GUID���ֵļ���ʹ�ǹ���Ա�û����Ե��øýӿ�

#define IOMSG_ENCRYPT_FILE_MENU 0x11
#define IOMSG_DECRYPT_FILE_MENU 0X12

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Windows CE ƽ̨(�粻�ṩ��ȫ DCOM ֧�ֵ� Windows Mobile ƽ̨)���޷���ȷ֧�ֵ��߳� COM ���󡣶��� _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA ��ǿ�� ATL ֧�ִ������߳� COM ����ʵ�ֲ�����ʹ���䵥�߳� COM ����ʵ�֡�rgs �ļ��е��߳�ģ���ѱ�����Ϊ��Free����ԭ���Ǹ�ģ���Ƿ� DCOM Windows CE ƽ̨֧�ֵ�Ψһ�߳�ģ�͡�"
#endif

using namespace ATL;


// CSafeWallExt

class ATL_NO_VTABLE CSafeWallExt :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CSafeWallExt, &CLSID_SafeWallExt>,
	public IDispatchImpl<ISafeWallExt, &IID_ISafeWallExt, &LIBID_SwShellExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public IShellExtInit, //shell��չ
    public IContextMenu    //�����Ĳ˵�
{
public:
	CSafeWallExt()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_SAFEWALLEXT)


BEGIN_COM_MAP(CSafeWallExt)
	COM_INTERFACE_ENTRY(ISafeWallExt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)//Sell����
	COM_INTERFACE_ENTRY(IContextMenu)//�˵�����
END_COM_MAP()



	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
	}

//����Ϊ�Զ�����ӵ�����
public:
	//��ʼ��
    HRESULT STDMETHODCALLTYPE Initialize( 
        /* [in] */ LPCITEMIDLIST pidlFolder,
        /* [in] */ IDataObject *pdtobj,
        /* [in] */ HKEY hkeyProgID);
	
	//��˵���������ݣ����msdn��
	//�����������Ϊ�ؼ��ĺ���֮һ������ʵ�ؼ�������2���������Ǹ�������һ��֮һ��
	//Ҫ���Ӽ����˵�����ʲôλ�ã���ʲô���֣�������ͼ�꣬���������id�ȵȶ���������ɵġ�
    STDMETHOD(QueryContextMenu)(THIS_
        HMENU hmenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags);

	//�ؼ���������һ��֮һ������˵�����Ϣ��Ӧ��
	//��msdn�²���CMINVOKECOMMANDINFOEX��� lpVerb��������˼��
	//��ᷢ�����ĸ�λ��ʾ�����ַ�����������idƫ������
	//���Ƕ��ַ��������ģ����ǹ��ĵ�������id. ���������Ҫʵ�־������˵�����Ҫʵ�ֵĹ��ܡ�
	//�����ж��ǵ��� XXXX(102),����YYYY(103)�˵��ء�
    STDMETHOD(InvokeCommand)(THIS_
        LPCMINVOKECOMMANDINFO lpici);


	//����Ҽ��˵�������Ϣ������������Ϣ
    STDMETHOD(GetCommandString)(THIS_
        UINT_PTR    idCmd,
        UINT        uType,
        UINT      * pwReserved,
        LPSTR       pszName,
        UINT        cchMax);

private:

    WCHAR   m_pszFileName[MAX_PATH];
    HBITMAP m_hBitmap;
    CHAR    m_pszVerb[32];
    WCHAR   m_pwszVerb[32];

};

OBJECT_ENTRY_AUTO(__uuidof(SafeWallExt), CSafeWallExt)
