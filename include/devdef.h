
#ifndef __DEVDEF_H_
#define __DEVDEF_H_

#define MYSAFEWALLCOMPANYNAME L"����Χǽ-�й�COPY������޹�˾\0"

//============================== ���ú궨�� ============================
#define MY_MAX_PATH   360

#ifndef MAX_PATH
#define MAX_PATH  260
#endif

#ifndef WORD
typedef unsigned short       WORD;
#endif

#ifndef DWORD
typedef unsigned long       DWORD;
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b) ((short)(((char)(((DWORD_PTR)(a)) & 0xff)) | ((short)((char)(((DWORD_PTR)(b)) & 0xff))) << 8))
#endif
#ifndef MAKELONG
#define MAKELONG(a, b) ((LONG)(((short)(a)) | ((DWORD)((short)(b))) << 16))
#endif

#ifndef Add2Ptr
//�﷨: PVOID  Add2Ptr(PUCHAR p,PUCHAR i);  ����: ָ��ƫ��
#define Add2Ptr(P,I) ((PVOID)((PUCHAR)(P) + (I)))  
#endif

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))
//============================== ���ú궨�� ============================

//============================== ���������� ============================
#define SAFEWALL_DEVICE_DOSNAME  L"\\\\.\\safewall"
#define SAFEWALL_DEVICE_SYMNAME  L"\\DosDevices\\SafeWall"

#define SAFEWALL_START    0x801
#define SAFEWALL_STOP     0x802
#define SAFEWALL_ENCRYPT_FILE  0x803
#define SAFEWALL_DECRYPT_FILE  0x804
#define SAFEWALL_QUERY_FILEATTRIBUTES     0x805
#define SAFEWALL_SHOWICO 0x806
#define SAFEWALL_FILEHIDE 0x807
#define SAFEWALL_SET_USER_ACCESS  0x821
#define SAFEWALL_SET_PROCESS_ACCESS 0x822
#define SAFEWALL_SET_FILE_ACCESS  0x823

#define SAFEWALL_TEST     0x899

#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)
#endif

#define IOCTL_START CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_START, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_STOP CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_STOP, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ENCRYPT CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_ENCRYPT_FILE,METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_DECRYPT CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_DECRYPT_FILE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_QUERY_FILEATTRIBUTES CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_QUERY_FILEATTRIBUTES, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SHOWICO CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_SHOWICO, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FILEHIDE CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_FILEHIDE, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_USER_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_USER_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_PROCESS_ACCESS  CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_PROCESS_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SET_FILE_ACCESS CTL_CODE(FILE_DEVICE_UNKNOWN,SAFEWALL_SET_FILE_ACCESS, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TEST CTL_CODE(FILE_DEVICE_UNKNOWN, SAFEWALL_TEST, METHOD_BUFFERED, FILE_ANY_ACCESS)


//============================== ���������� ============================



//============================== ����������ǩ ============================
//safewall����汾
#define SAFEWALl_DWORD_VERSION_1     1
#define SAFEWALl_DWORD_VERSION_2     2
#define SAFEWALl_DWORD_VERSION_3     3

//�㷨�汾
#define SAFEWALl_ALGORIT_VERSION_1     0x00000001
#define SAFEWALl_ALGORIT_VERSION_2     0x00000002
#define SAFEWALl_ALGORIT_VERSION_3     0x00000003

//����SALEWALL_OBJECT ��������
//�����ļ�״̬
#define SAFEWALL_FLAG_OBJECT      0x00000001   //�������ǵļ��ܶ���
#define SAFEWALL_FLAG_FILEGROUP   0x00000002   //�������ǵ��ļ���
#define SAFEWALL_FLAG_MANAGEMENT  0x00000004   //����ID��־�����Ȩ�޵��ļ���

//�����ļ�״̬
#define SAFEWALL_FLAG_INFECTED    0x80000000   //�Ѽ���
#define SAFEWALL_FLAG_WAITENCRYPT     0x40000000   //�ȴ�������

#define MAX_DEVNAME_LENGTH 64                                //���峣��ֵ
#define DEVOBJ_LIST_SIZE 64

#define DELAY_ONE_MICROSECOND   (-10)  
#define DELAY_ONE_MILLISECOND   (DELAY_ONE_MICROSECOND*1000)
#define DELAY_ONE_SECOND        (DELAY_ONE_MILLISECOND*1000)


#define FILE_HIDE      0x00000001  //���ؼ����ļ�
#define FILE_SHOWICO   0x00000002  //��ʾ����ͼ��

//============================== ����������ǩ ============================


//============================== ���ܲ��� ============================
//ȫ�ֽ���Ȩ�޽ṹ��
typedef struct _PROCESS_ACCESS_STRUCT{
	char AccessName[32]; //Ȩ������
	long AccessFlags; //Ȩ�ޱ�־
	struct _PROCESS_ACCESS_STRUCT * next;//��һ���ṹ���λ��
	int lenght;  //ProcessName�ĳ���
	char ProcessName[1]; //�������ƣ���'\0'�ָ�
}PROCESS_ACCESS, * PPROCESS_ACCESS;

//ȫ���ļ�Ȩ��
typedef struct _FILE_ACCESS_STRUCT{
	int size;//suffix���ַ���
	wchar_t suffix[1];
}FILE_ACCESS, *PFILE_ACCESS;

//�û�˽��Ȩ��
typedef struct _USER_ACCESS_STRUCT{
	char userid[64];
	long AccessFlags;
}USER_ACCESS, *PUSER_ACCESS;

//ͨ�ñ�־
#define SAFEWALL_ENABLE   0x00000001 //����
#define SAFEWALL_DISABLE  0x00000002 //����
#define SAFEWALL_NOLYREAD 0x00000004 //ֻ��

//��������
#define PROCESS_NO_ACCESS    0x00000001 //��ֹ���ʼ����ļ�
#define PROCESS_ACCESS_NORMAL    0x00000002 //Ĭ�ϸ��롣����δ���ܾͲ��ܷ��ʼ����ļ�
#define PROCESS_ACCESS_INHERIT   0x00000004 //�����ӽ��̼̳иý��̵�����
#define PROCESS_ACCESS_CLIPBOARD 0x00000008 //����ճ���岻����

#define USER_ACCESS_DISABLE_PRINT 0x00000001 //��ֹ��ӡ
#define USER_ACCESS_DISABLE_SCREENSHOT 0x00000002 //��ֹ����
#define USER_ACCESS_RUN_NETWORK_PROCESS  0x00000004 //����������Ȩ���������

//============================== ���ܲ��� ============================

#endif 