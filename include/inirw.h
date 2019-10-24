/**
 * �ļ���inirw.h
 * �汾��1.0
 * ���ߣ�taoyuanmin@gmail.com
 *
 * ˵����ini�����ļ���д
 * 1��֧��;��#ע�ͷ��ţ�֧����βע�͡�
 * 2��֧�ִ�����'��"�ɶ�ƥ����ַ�������ȡʱ�Զ�ȥ���š������пɴ��������Ż�;#ע�ͷ���
 * 3��֧����section���section(����Ϊ��)��
 * 4��֧��10��16��8��������0x��ͷΪ16��������0��ͷΪ8���ơ�
 * 5��֧��section��key��=��ǰ����ո�
 * 6��֧��\n��\r��\r\n��\n\r���и�ʽ��
 * 7��������section��key��Сд����д��ʱ���´�Ϊ׼�����������Сд��
 * 8����������ʱ����section�������ڸý����һ����Ч���ݺ���ӣ��������ļ�β����ӡ�
 * 9��֧��ָ��key��������ɾ������ɾ���ü�ֵ������ע�͡�
 * 10�����Զ�������ʽ�����У��޸�ʱ��Ȼ������
 * 11���޸�ʱ����ԭע�ͣ���������ע�͡���βע��(����ǰ��ո�)��
 * 12���޸�ʱ����ԭ���С�����������Ҫ�Ǿ�������ԭ��ʽ��
 */
 

#ifndef _INI_RW_H_
#define _INI_RW_H_

#define SIZE_LINE		1024	//ÿ����󳤶�
#define SIZE_FILENAME	256		//�ļ�����󳤶�

#define _CRT_SECURE_NO_DEPRECATE 1
#define _CRT_NONSTDC_NO_DEPRECATE 1

//#define min(x, y)		(x <= y) ? x : y

#define __INIOBJECT  int*
#define INIOBJECT  __INIOBJECT

typedef enum _ELineType_ {
    LINE_IDLE,		//δ������
	LINE_ERROR,		//������
	LINE_EMPTY,		//�հ��л�ע����
	LINE_SECTION,	//�ڶ�����
	LINE_VALUE		//ֵ������
} ELineType ;


typedef struct _INI_OBJENT {
	char *buffer;
	int  buflen;
	char fileName[SIZE_FILENAME];
}* LPINIOBJECT;

#ifdef __cplusplus
extern "C" {
#endif

//����ini����
INIOBJECT CreateIniObject(const char * fileName);
//�ͷ�INI����
void ReleaseIniObject(INIOBJECT iniobj);
//����ini�ļ����ڴ�
int iniFileLoad(INIOBJECT iniobj);
//��ȡ�ַ�������������
int iniGetString(INIOBJECT iniobj, const char *section, const char *key, char *value, int maxlen, const char *defvalue);
//��ȡ����ֵ
int iniGetInt(INIOBJECT iniobj, const char *section, const char *key, int defvalue);
//��ȡ������
double iniGetDouble(INIOBJECT iniobj, const char *section, const char *key, double defvalue);

//�����ַ�������valueΪNULL����ɾ����key�����У�����ע��
int iniSetString(INIOBJECT iniobj, const char *section, const char *key, const char *value);
//��������ֵ��baseȡֵ10��16��8���ֱ��ʾ10��16��8���ƣ�ȱʡΪ10����
int iniSetInt(INIOBJECT iniobj, const char *section, const char *key, int value, int base);

#ifdef __cplusplus
}
#endif

#endif
