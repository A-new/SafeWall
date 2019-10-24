
#include "SwClient.h"

LRESULT CALLBACK ChangePwdWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps= {0};
	MYSERVCONTEXT servContext = {0};

	char newpwd2[64] = {0};

	switch(message)
	{
	case WM_INITDIALOG:
		{
			//��ʼ����¼����
			NotifyService(MY_SERVICE_CONTROL_GET_USERID, &servContext);

			if(servContext.userid[0])
			{
				SetDlgItemTextA(hwnd, IDC_USERID, servContext.userid);
			}
			else
			{
				MessageBox(hwnd,L"�û�δ��¼�����¼��",L"��ʾ",NULL);
				EndDialog(hwnd, NULL);
			}

		}break;
	case WM_PAINT:
		{
			HDC hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return FALSE;
		}break;
	case WM_COMMAND:
		{
			switch(LOWORD(wParam))
			{
			case IDOK:
				{
					memset(&servContext,0, sizeof(MYSERVCONTEXT));
					GetDlgItemTextA(hwnd, IDC_USERID, servContext.userid, sizeof(servContext.userid));
					GetDlgItemTextA(hwnd, IDC_PASSWORD, servContext.password, sizeof(servContext.password));
					GetDlgItemTextA(hwnd, IDC_NEWPWD, servContext.newpwd, sizeof(servContext.newpwd));
					GetDlgItemTextA(hwnd, IDC_NEWPWD2, newpwd2, sizeof(newpwd2));

					if(strcmp(servContext.newpwd,newpwd2))
					{
						MessageBox(hwnd, L"�������������벻һ�£�",L"��ʾ",NULL);
						MessageBoxA(hwnd, servContext.newpwd ,newpwd2,NULL);
						return 0;
					}

					if(NotifyService(MY_SERVICE_CONTROL_CHANGE_PASSWORD,&servContext))
					{
						MessageBox(hwnd, L"�޸ĳɹ�",L"��ʾ",NULL);
					}
					else
					{
						MessageBox(hwnd, L"�޸�ʧ��",L"��ʾ",NULL);
					}
				}break;
			case IDCANCEL:
				{
					EndDialog(hwnd, NULL);
				}break;
			}
		}break;
	case WM_CLOSE:
		{
			EndDialog(hwnd, wParam);
			return 0;
		}break;
	}
	return FALSE;
}
