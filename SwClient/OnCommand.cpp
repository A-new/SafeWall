

#include "SwClient.h"
#include "..\\include\\inirw.h"

LRESULT OnCommand (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char userid[64] ={0};
	char password[64]={0};
	char title[64] = {0};
	MYSERVCONTEXT servContext = {0};
	servContext.fromHwnd = (long*)hwnd;
	UINT savepwd = BST_UNCHECKED;
	UINT autorun = BST_UNCHECKED;

	switch(LOWORD(wParam))
	{
	case IDOK:
		{
			int len = 0;

			if(NotifyService(MY_SERVICE_CONTROL_GET_USERID,&servContext))
			{
				if(servContext.userid[0] != '\0')
				{
					MessageBox(hwnd,L"�û��ѵ�¼�����µ�¼��ע����",L"��ʾ",NULL);
					return 0;
				}
			}
			len = GetDlgItemTextA(hwnd, IDC_USERID, userid, sizeof(userid));
			len = GetDlgItemTextA(hwnd, IDC_PASSWORD, password, sizeof(password));
			savepwd = IsDlgButtonChecked(hwnd, IDC_SAVEPWD);
			autorun = IsDlgButtonChecked(hwnd, IDC_AUTORUN);

			INIOBJECT iniobj = CreateIniObject(CONFILE);
			iniSetString(iniobj,"logon","userid",userid);
			if(BST_CHECKED  == savepwd )
			{
				iniSetInt(iniobj,"logon","savepwd",1,10);
				iniSetString(iniobj,"logon","password",password);
			}
			else 
			{
				iniSetInt(iniobj,"logon","savepwd",0,10);
				iniSetString(iniobj,"logon","password","");
			}

			if(BST_CHECKED  == autorun )
			{
				iniSetInt(iniobj,"logon","autorun",1,10);
			}
			else 
			{
				iniSetInt(iniobj,"logon","autorun",0,10);
			}
			ReleaseIniObject(iniobj);

			strcpy_s(servContext.userid, userid);
			strcpy_s(servContext.password, password);
			if(NotifyService(MY_SERVICE_CONTROL_LOGON,&servContext))
			{
				strcpy_s(title, servContext.userid);
				strcat_s(title, "-�ѵ�¼");
				SetWindowTextA(hwnd, title);
				MessageBox(hwnd,L"��¼�ɹ�",L"��ʾ",NULL);
			}
			else
			{
				MessageBox(hwnd,L"��¼ʧ��",L"��ʾ",NULL);
			}
		}break;
	case IDCANCEL:
		{
			ShowWindow(hwnd,FALSE);
		}break;
	case ID_MAIN: //������
		{
			ShowWindow(hwnd,TRUE);
		}break;
	case ID_CANCEL_LOGON:  //ע����¼
		{
			NotifyService(MY_SERVICE_CONTROL_CANCEL_LOGON,&servContext);
			SetWindowTextW(hwnd, szAppName);
			MessageBox(hwnd,L"��ע��",L"��ʾ",NULL);
		}break;
	case ID_LINK_CONFIG: //��������
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_LINK_CONFIG), hwnd,(DLGPROC)LinkConfigWndProc, NULL );
		}break;
	case ID_SYSTEM_CONFIG:  //ϵͳ����
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_SYSTEM_CONFIG), hwnd,(DLGPROC)SystemConfigWndProc, NULL );
			MessageBox(hwnd,L"ϵͳ����",L"��ʾ",NULL);
		}break;
	case ID_REFRESH_STRATEGY:  //ˢ�²���
		{
			if(NotifyService(MY_SERVICE_CONTROL_GET_ACCESS,&servContext))
			{
				MessageBox(hwnd,L"����ˢ�³ɹ�",L"��ʾ",NULL);
			}
			else
			{
				MessageBox(hwnd,L"����ˢ��ʧ��",L"��ʾ",NULL);
			}
		}break;
	case ID_CHANGE_PWD:
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_CHANGE_PWD), hwnd,(DLGPROC)ChangePwdWndProc,NULL );
		}break;
	case ID_SCANFILE:  //ɨ������ĵ�
		{
			DialogBoxParam((HINSTANCE)GetWindowLong(hwnd,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_SCANFILE), hwnd,(DLGPROC)ScanFileWndProc,NULL );
		}break;
	default: //δ֪
		break;
	}

	return 0;
}