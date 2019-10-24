
#include "manager.h"
#include "resource.h"

CMyTcpSocket * server;
Json::Value root;

CDuiFrameWnd::CDuiFrameWnd() {};

LPCTSTR CDuiFrameWnd::GetWindowClassName() const {   return _T("ManagerFrame");  };
UINT CDuiFrameWnd::GetClassStyle() const { return CS_DBLCLKS; };
void CDuiFrameWnd::OnFinalMessage(HWND /*hWnd*/) { delete this; };


void CDuiFrameWnd::OnPrepare()
{
	CLoginFrameWnd* pLoginFrame = new CLoginFrameWnd();
    if( pLoginFrame == NULL ) { Close(); return; }
    pLoginFrame->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
	ShowWindow(false);
    pLoginFrame->CenterWindow();
		
	server = new CMyTcpSocket();
    if(pLoginFrame->ShowModal())
	{
		ShowWindow(true);
	}
	else
	{
		delete server;
	}
}

void CDuiFrameWnd::OnClose()
{
	return ;
}

void CDuiFrameWnd::Notify(TNotifyUI& msg)
{
	CDuiString    strName     = msg.pSender->GetName();
	if( msg.sType == _T("windowinit") ) 
	{
		OnPrepare();
	}
    else if(msg.sType == _T("selectchanged"))
    {
        
        CTabLayoutUI* pControl = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("TabFrame")));
 
        if(strName == _T("OptControlKey"))
            pControl->SelectItem(0);
        else if(strName == _T("OptFileAccess"))
            pControl->SelectItem(1);
        else if(strName == _T("OptProcessAccess"))
            pControl->SelectItem(2);
		else if(strName == _T("OptUserSetting"))
            pControl->SelectItem(3);
		else if(strName == _T("OptOnlineUser"))
            pControl->SelectItem(4);
		else if(strName == _T("OptLookLog"))
            pControl->SelectItem(5);
    }
	else if(msg.sType == _T("click"))
	{
		if(strName == _T("closebtn"))
		{
			//ϵͳ��ť-�ر�
			OnClose();
			PostQuitMessage(0);
			return;
		}
		else if(strName == _T("btnChangePwd"))
		{
			//�޸ļ�������
			CChangePwdFrameWnd* pDialog= new CChangePwdFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnfaSave"))
		{
			//�����ļ���׺����
		}
		else if(strName == _T("btnfaCancel"))
		{
			//ȡ���޸��ļ���׺����
		}
		else if(strName == _T("btnPaAdd"))
		{
			//�������̲���
			CAddProcessItemFrameWnd* pDialog= new CAddProcessItemFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnPaSave"))
		{
			//������̲���
		}
		else if(strName == _T("btnPaCancel"))
		{
			//ȡ���޸Ľ��̲���
		}
		else if(strName == _T("btnAddUser"))
		{
			//����û�
			CAddUserFrameWnd* pDialog= new CAddUserFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnChangeUser"))
		{
			//�޸��û�
			CUserAccessFrameWnd* pDialog= new CUserAccessFrameWnd();
			if( pDialog == NULL ) { Close(); return; }
			pDialog->Create(m_hWnd, _T(""), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
			pDialog->CenterWindow();
		
			pDialog->ShowModal();
		}
		else if(strName == _T("btnDelUser"))
		{
			//ɾ���û�
		}
	}
	else if(msg.sType == _T("itemselect"))
	{
	}
};

LRESULT CDuiFrameWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;

	switch(uMsg)
	{
	case WM_CREATE:
		{
			LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
			styleValue &= ~(WS_CAPTION);
			::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
			m_PaintManager.Init(m_hWnd);

			CDialogBuilder builder;
			CControlUI* pRoot = builder.Create(_T("sysres\\manager.xml"), (UINT)0, NULL, &m_PaintManager);   // duilib.xml��Ҫ�ŵ�exeĿ¼��
			ASSERT(pRoot && "Failed to parse XML");

			m_PaintManager.AttachDialog(pRoot);
			m_PaintManager.AddNotifier(this);   // ��ӿؼ�����Ϣ��Ӧ��������Ϣ�ͻᴫ�ﵽduilib����Ϣѭ�������ǿ�����Notify����������Ϣ����

			m_pCloseBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));//���ϵͳ��ť
			return lRes;
		}break;
		// ����3����ϢWM_NCACTIVATE��WM_NCCALCSIZE��WM_NCPAINT��������ϵͳ������
	case WM_NCACTIVATE:
		{
			if( !::IsIconic(m_hWnd) ) 
			{
				return (wParam == 0) ? TRUE : FALSE;
			}
		}break;
	case WM_NCCALCSIZE:
		{
			return 0;
		}break;
	case WM_NCPAINT:
		{
			return 0;
		}break;
	case WM_NCHITTEST:
		{
			//ͨ���������϶�����
			POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
			::ScreenToClient(*this, &pt);

			RECT rcClient;
			::GetClientRect(*this, &rcClient);

			RECT rcCaption = m_PaintManager.GetCaptionRect();
			if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
				&& pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
					CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(pt));
					//����ǰ�ť��һ���ǹر�֮��İ�ť���϶�������
					if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
						return HTCAPTION;
			}

			return HTCLIENT;
		}break;
	default:
		break;
	}


    if( m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes) ) 
    {
        return lRes;
    }

    return __super::HandleMessage(uMsg, wParam, lParam);
}


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    CPaintManagerUI::SetInstance(hInstance);
	HRESULT Hr = ::CoInitialize(NULL);
	CMyTcpSocket::WSAStartup();
    CDuiFrameWnd * pFrame = new CDuiFrameWnd();

	pFrame->Create(NULL, _T("����Χǽ������"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
    pFrame->CenterWindow();
	pFrame->ShowModal();
	pFrame->SetIcon(IDI_ICON1);
	//ShowWindow(*pFrame, SW_SHOWNORMAL);

 //   CPaintManagerUI::MessageLoop();
	CMyTcpSocket::WSACleanup();
	::CoUninitialize();
    return 0;
}