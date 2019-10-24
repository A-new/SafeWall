
#include "SwServer.h"

LRESULT CALLBACK FrameWndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;

    static SOCKET server;
	SOCKET client;
	int len =0;
	char  sendBuf[1024] = {0};
	char  recvBuf[1024] = {0};
	SOCKADDR_IN addrClient = {0};

	HMENU hMenu;

	switch (message)
	{
	case WM_CREATE:
		{
			return OnCreate(hwnd, message, wParam, lParam);
		}break;
	case WM_SOCKET_EVENT:
		{
			//server = (SOCKET)wParam; // ���������¼����׽���   
            //long event = WSAGETSELECTEVENT(lParam); // �¼�   
            int error = WSAGETSELECTERROR(lParam); // ������  

			switch(WSAGETSELECTEVENT(lParam))
			{
			case FD_ACCEPT:
				{
					//client = accept(server,(SOCKADDR*)&addrClient,&len);
					client = accept(server,NULL,NULL);

					//MessageBox(hwnd,"���ڼ��accept�Ƿ�ͨ������","��ʾ��",NULL);

					if(client != INVALID_SOCKET)
					{
						/*WSAAsyncSelect(client,hwnd,WM_NOTHING,NULL);
						ioctlsocket(client,FIONBIO,NULL);*/
						/*struct linger {
						  u_short l_onoff;
						  u_short l_linger;
						};*/
						linger m_sLinger;
						m_sLinger.l_onoff = 1;
						//�ڵ���closesocket����ʱ��������δ�����꣬����ȴ�
						//��m_sLinger.l_onoff=0�������closesocket������ǿ�ƹر�
						m_sLinger.l_linger = 5; //���õȴ�ʱ��Ϊ5��

						DWORD TimeOut = TIMEOUT;
						setsockopt(client, SOL_SOCKET, SO_RCVTIMEO,(const char*)&TimeOut,sizeof(TimeOut));
						setsockopt(client, SOL_SOCKET, SO_SNDTIMEO,(const char*)&TimeOut,sizeof(TimeOut));
					}
					else
					{
						//MessageBox(hwnd,_T("accept��������"),_T("��ʾ��"),NULL);
					}
				}
				break;
			}
			return 0;
		}break;
	case WM_COMMAND:
		{
			return OnCommand (hwnd, message, wParam, lParam);
		}break;
	case WM_PAINT:
		{
			hdc = BeginPaint (hwnd, &ps);

			EndPaint (hwnd, &ps);
			return 0;
		}
	case WM_DESTROY:
		{
			PostQuitMessage (0);
			return 0;
		}
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}


