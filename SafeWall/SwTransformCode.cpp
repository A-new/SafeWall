
#include "SafeWall.h"

char * encode1(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
char * encode2(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
char * encode3(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
char * decode1(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
char * decode2(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
char * decode3(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey);
__inline void InlineLeftCircleSwid(SWID * swid, ULONG more);
__inline void InlineRightCircleSwid(SWID * swid, ULONG more);
EXTERN_C const GUID COMPANYGUID;

#pragma PAGEDCODE
BOOLEAN InitMySafeWallObject(LPSAFEWALL_OBJECT * lpSafeWall)
{
	LPSAFEWALL_OBJECT p = (LPSAFEWALL_OBJECT)ExAllocatePool( NonPagedPool, SAFEWALL_OBJECT_SIZE);
	if(NULL == p)
	{
		KdPrint(("�ڴ治�㣬���� LPSAFEWALL_OBJECT ʧ��"));
		return FALSE;
	}
	wcscpy(p->swv.myCompanyName, MYSAFEWALLCOMPANYNAME);
	memcpy(&p->swv.myId, &MYSAFEWALLGUID ,sizeof(p->swv.myId));
	p->swv.vObj = SAFEWALL_VERSION_SIZE;
	p->swv.objSize = SAFEWALL_OBJECT_SIZE;

	memcpy(&p->CompanyId, &COMPANYGUID, sizeof(p->CompanyId));
	memcpy(&p->FileGroupId, &COMPANYGUID, sizeof(p->FileGroupId));
	p->AlgorithmVersion = SAFEWALl_ALGORIT_VERSION_1;
	wcscpy(p->UserId, L"<non>");

	*lpSafeWall = p;
	return TRUE;
}

DWORD InitSafeWallObject(LPSAFEWALL_OBJECT lpSafeWall)
{
	DWORD dwRet;
	memcpy(lpSafeWall, gStandardSafeWallObj, SAFEWALL_OBJECT_SIZE);
	ExGuidCreate((GUID*)&lpSafeWall->privateKey);

	//�Լ����ܵĶ������϶��������ļ������
	dwRet = SAFEWALL_FLAG_OBJECT|SAFEWALL_FLAG_FILEGROUP;

	//���һ�¸��ļ����Ƿ��������Ȩ�޵��ļ���
	if(InlineIsEqualSWID(&lpSafeWall->FileGroupId, &gStandardSafeWallObj->CompanyId)) 
	{
		//KdPrint(("�������ڹ�˾��GUID\n"));
		dwRet |= SAFEWALL_FLAG_MANAGEMENT;
	}
	return dwRet;
}


#pragma PAGEDCODE
NTSTATUS ExGuidCreate(OUT GUID *Guid)
{
	//�ο��ں��㷨
	NTSTATUS status	= STATUS_SUCCESS;
	
	DWORD dwSeed;
    char * pBuffer;
    ULONG uPseudoRandom;
    LARGE_INTEGER time;
	ULONG dwLen = sizeof(GUID);
    static ULONG uCounter = 17;

    /* Get the first seed from the performance counter */
    KeQueryPerformanceCounter(&time);
    dwSeed = time.LowPart ^ time.HighPart ^ RtlUlongByteSwap(uCounter++);

    /* We will access the buffer bytewise */
    pBuffer = (char *)Guid;

    do
    {
        /* Use the pseudo random number generator RtlRandom, which outputs a 4-byte value and a new seed */
        uPseudoRandom = RtlRandom(&dwSeed);

        do
        {
            /* Get each byte from the pseudo random number and store it in the buffer */
            *pBuffer = (char)(uPseudoRandom >> 8 * (dwLen % 4) & 0xFF);
            ++pBuffer;
        } while(--dwLen % 4);
    } while(dwLen);

	KdPrint(("�����µ�GUID��%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x", Guid->Data1,
		Guid->Data2, Guid->Data3, Guid->Data4[0],Guid->Data4[1],
		Guid->Data4[2],Guid->Data4[3],Guid->Data4[4],
		Guid->Data4[5],Guid->Data4[6],Guid->Data4[7]));
	return status;
}

__inline int InlineIsEqualSWID(const SWID * swid1, const SWID * swid2)
{
   return (
	   swid1->data[0] == swid1->data[0] && swid1->data[1] == swid1->data[1] &&
	   swid1->data[2] == swid1->data[2] && swid1->data[3] == swid1->data[3]
      );
}

#pragma PAGEDCODE
void Dehead(LPSAFEWALL_OBJECT pobj)
{
	SWID * code, privateKey, key;
	int i,multiple;

	unsigned int bytes;

	multiple = SAFEWALL_OBJECT_SIZE / sizeof(SWID);
	code = (SWID*)pobj;

	privateKey.data[0] = pobj->privateKey.data[0] ^ ((unsigned long *) &SAFEWALLAUTOKEY)[0];
	privateKey.data[1] = pobj->privateKey.data[1] ^ ((unsigned long *) &SAFEWALLAUTOKEY)[1];
	privateKey.data[2] = pobj->privateKey.data[2] ^ ((unsigned long *) &SAFEWALLAUTOKEY)[2];
	privateKey.data[3] = pobj->privateKey.data[3] ^ ((unsigned long *) &SAFEWALLAUTOKEY)[3];

	key = privateKey;

	for(i=0; i < multiple; i++)
	{
		bytes =   key.data[0] & 0x00000001L;
		bytes |= (key.data[1] << 1) & 0x00000002L;
		bytes |= (key.data[2] << 2) & 0x00000004L;
		bytes |= (key.data[3] << 3) & 0x00000008L;

		key.data[0] = (key.data[0] >> 1) | (0x10000000L & (bytes << 28));
		key.data[1] = (key.data[1] >> 1) | (0x10000000L & (bytes << 31));
		key.data[2] = (key.data[2] >> 1) | (0x10000000L & (bytes << 30));
		key.data[3] = (key.data[3] >> 1) | (0x10000000L & (bytes << 29));

		code[i].data[0] ^=  key.data[0];
		code[i].data[1] ^=  key.data[1];
		code[i].data[2] ^=  key.data[2];
		code[i].data[3] ^=  key.data[3];

		code[i].data[0] = (~code[i].data[0]) ^ ((unsigned long *) &SAFEWALLAUTOKEY)[0];
		code[i].data[1] = (~code[i].data[1]) ^ ((unsigned long *) &SAFEWALLAUTOKEY)[1];
		code[i].data[2] = (~code[i].data[2]) ^ ((unsigned long *) &SAFEWALLAUTOKEY)[2];
		code[i].data[3] = (~code[i].data[3]) ^ ((unsigned long *) &SAFEWALLAUTOKEY)[3];
	}
	pobj->privateKey = privateKey;

	return;
}

#pragma PAGEDCODE
void Enhead(LPSAFEWALL_OBJECT pobj)
{
	SWID * code;
	int i,multiple;
	SWID swid, privateKey, key;
	unsigned int bytes;
	//�Զ�����м���
	multiple = SAFEWALL_OBJECT_SIZE / sizeof(GUID); 
	code = (SWID *)pobj;
	key = privateKey = pobj->privateKey;

	for(i=0; i < multiple; i++)
	{
		bytes =   key.data[0] & 0x00000001L;
		bytes |= (key.data[1] << 1) & 0x00000002L;
		bytes |= (key.data[2] << 2) & 0x00000004L;
		bytes |= (key.data[3] << 3) & 0x00000008L;

		key.data[0] = (key.data[0] >> 1) | (0x10000000L & (bytes << 28));
		key.data[1] = (key.data[1] >> 1) | (0x10000000L & (bytes << 31));
		key.data[2] = (key.data[2] >> 1) | (0x10000000L & (bytes << 30));
		key.data[3] = (key.data[3] >> 1) | (0x10000000L & (bytes << 29));

		code[i].data[0] ^= ((unsigned long *) &SAFEWALLAUTOKEY)[0];
		code[i].data[1] ^= ((unsigned long *) &SAFEWALLAUTOKEY)[1];
		code[i].data[2] ^= ((unsigned long *) &SAFEWALLAUTOKEY)[2];
		code[i].data[3] ^= ((unsigned long *) &SAFEWALLAUTOKEY)[3];

		code[i].data[0] = (~code[i].data[0]) ^ key.data[0];
		code[i].data[1] = (~code[i].data[1]) ^ key.data[1];
		code[i].data[2] = (~code[i].data[2]) ^ key.data[2];
		code[i].data[3] = (~code[i].data[3]) ^ key.data[3];
	}
	//����
	pobj->privateKey.data[0] = ((unsigned long *) &SAFEWALLAUTOKEY)[0] ^ privateKey.data[0];
	pobj->privateKey.data[1] = ((unsigned long *) &SAFEWALLAUTOKEY)[1] ^ privateKey.data[1];
	pobj->privateKey.data[2] = ((unsigned long *) &SAFEWALLAUTOKEY)[2] ^ privateKey.data[2];
	pobj->privateKey.data[3] = ((unsigned long *) &SAFEWALLAUTOKEY)[3] ^ privateKey.data[3];

	return;
}


#pragma PAGEDCODE
char * Encode(DWORD version, char *buffer,ULONG lenght, 
			  LARGE_INTEGER *keyOffset,  SWID * privateKey, SWID * publicKey)
{
	switch(version)
	{
	case SAFEWALl_ALGORIT_VERSION_1:
		{
			encode1(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	case SAFEWALl_ALGORIT_VERSION_2:
		{
			encode2(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	case SAFEWALl_ALGORIT_VERSION_3:
		{
			encode3(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	default:
		break;
	}
	return buffer;
}

#pragma PAGEDCODE 
char * Decode(DWORD version, char *buffer,ULONG lenght, 
			  LARGE_INTEGER *keyOffset,  SWID * privateKey, SWID * publicKey)
{
	switch(version)
	{
	case SAFEWALl_ALGORIT_VERSION_1:
		{
			decode1(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	case SAFEWALl_ALGORIT_VERSION_2:
		{
			decode2(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	case SAFEWALl_ALGORIT_VERSION_3:
		{
			decode3(buffer, lenght, *keyOffset, *privateKey, *publicKey);
		}break;
	default:
		break;
	}
	return buffer;
}

#pragma PAGEDCODE
__inline void InlineLeftCircleSwid(SWID * swid, ULONG more)
{
	//��SWID��תmore��,��Ϊ��������ʵ�÷ǳ�Ƶ��������ʹ������
	LARGE_INTEGER low, hight;

	more = more % (sizeof(SWID) * 8);//����תȦ��������һȦ����

	if(1== more)//1�Ĵ�����࣬���Է�ǰͷ
	{
		low.LowPart = (swid->data[0] >> 31) & 0x00000001L;
		low.HighPart = (swid->data[1] >> 31) & 0x00000001L;
		hight.LowPart = (swid->data[2] >> 31) & 0x00000001L;
		hight.HighPart = (swid->data[3] >> 31) & 0x00000001L;

		swid->data[0] = (swid->data[0] << 1) | low.HighPart;
		swid->data[1] = (swid->data[1] << 1) | hight.LowPart;
		swid->data[2] = (swid->data[2] << 1) | hight.HighPart;
		swid->data[3] = (swid->data[3] << 1) | low.LowPart;
	}
	else if(0 != more)
	{
		if(more > sizeof(__int64) * 8)
		{
			low.QuadPart = swid->QuadPart[0];
			swid->QuadPart[0] = swid->QuadPart[1];
			swid->QuadPart[1] = low.QuadPart;
			more -= sizeof(__int64) * 8;

			if(0 == more) return;
		}
		if(more > sizeof(unsigned int) * 8)
		{
			low.LowPart = swid->data[0];
			swid->data[0] = swid->data[1];
			swid->data[1] = swid->data[2];
			swid->data[2] = swid->data[3];
			swid->data[3] =low.LowPart;

			more -= sizeof(unsigned int) * 8;
			if(0 == more) return;
		}
		while(more--)
		{
			low.LowPart = (swid->data[0] >> 31) & 0x00000001L;
			low.HighPart = (swid->data[1] >> 31) & 0x00000001L;
			hight.LowPart = (swid->data[2] >> 31) & 0x00000001L;
			hight.HighPart = (swid->data[3] >> 31) & 0x00000001L;

			swid->data[0] = (swid->data[0] << 1) | low.HighPart;
			swid->data[1] = (swid->data[1] << 1) | hight.LowPart;
			swid->data[2] = (swid->data[2] << 1) | hight.HighPart;
			swid->data[3] = (swid->data[3] << 1) | low.LowPart;
		}
	}
	return;
}

#pragma PAGEDCODE
__inline void InlineRightCircleSwid(SWID * swid, ULONG more)
{
	//��SWID��תmore��,��Ϊ��������ʵ�÷ǳ�Ƶ��������ʹ������
	LARGE_INTEGER low, hight;

	more = more % (sizeof(SWID) * 8);//����תȦ��������һȦ����

	if(1== more)//1�Ĵ�����࣬���Է�ǰͷ
	{
		low.LowPart = (swid->data[0] << 31) & 0x80000000L;
		low.HighPart = (swid->data[1] << 31) & 0x80000000L;
		hight.LowPart = (swid->data[2] << 31) & 0x80000000L;
		hight.HighPart = (swid->data[3] << 31) & 0x80000000L;

		swid->data[0] = (swid->data[0] >> 1) | hight.HighPart;
		swid->data[1] = (swid->data[1] >> 1) | low.LowPart;
		swid->data[2] = (swid->data[2] >> 1) | low.HighPart;
		swid->data[3] = (swid->data[3] >> 1) | hight.LowPart;
	}
	else if(0 != more)
	{
		if(more > sizeof(__int64) * 8)
		{
			low.QuadPart = swid->QuadPart[0];
			swid->QuadPart[0] = swid->QuadPart[1];
			swid->QuadPart[1] = low.QuadPart;
			more -= sizeof(__int64) * 8;

			if(0 == more) return;
		}
		if(more > sizeof(unsigned int) * 8)
		{
			low.LowPart = swid->data[3];

			swid->data[3] = swid->data[2];
			swid->data[2] = swid->data[1];
			swid->data[1] = swid->data[0];
			swid->data[0] = low.LowPart;
			
			more -= sizeof(unsigned int) * 8;
			if(0 == more) return;
		}
		while(more--)
		{
			low.LowPart = (swid->data[0] << 31) & 0x80000000L;
			low.HighPart = (swid->data[1] << 31) & 0x80000000L;
			hight.LowPart = (swid->data[2] << 31) & 0x80000000L;
			hight.HighPart = (swid->data[3] << 31) & 0x80000000L;

			swid->data[0] = (swid->data[0] >> 1) | hight.HighPart;
			swid->data[1] = (swid->data[1] >> 1) | low.LowPart;
			swid->data[2] = (swid->data[2] >> 1) | low.HighPart;
			swid->data[3] = (swid->data[3] >> 1) | hight.LowPart;
		}
	}
	return;
}

#pragma PAGEDCODE
char * encode1(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	//���㷨(KEYBYTESLENGHT * KEYBYTESLENGHT * szieof(SWID))(256k)�ڲ��ظ�
	PSWID code;
	unsigned int i,j,k,publicLoop ,privateLoop;
	ULONG point;//��ǰ���崦���ĸ�λ��
	unsigned int fraction;

	//����publicKey��ƫ�ƴ���
	publicLoop = (keyOffset.QuadPart / (KEYBYTESOFFSET * sizeof(SWID))) % KEYBYTESOFFSET;//ÿ2KBתһ��,%����һȦ
	publicLoop += keyOffset.QuadPart % (KEYBYTESOFFSET * sizeof(SWID)) ? 1 : 0;
	InlineLeftCircleSwid(&publicKey, publicLoop);

	//����privateKey��ƫ�ƴ���
	privateLoop = (keyOffset.QuadPart / sizeof(SWID)) % KEYBYTESOFFSET;//ÿ16�ֽ�תһ�Σ�%����һȦ
	fraction = keyOffset.QuadPart % sizeof(SWID);
	privateLoop += fraction ? 1 : 0;
	InlineLeftCircleSwid(&privateKey,privateLoop);

	//��ʼ����
	point = 0;//��ʼ������λ��
	if(fraction)//�����ȴ���ǰͷ
	{
		for(i = fraction-1; sizeof(SWID) > i && lenght > point; i++,point++)
		{
			buffer[point] ^= publicKey.byte[i];
			buffer[point] = (buffer[point]&0x55<<1) | (buffer[point]&0xAA>>1);
			buffer[point] ^= privateKey.byte[i];
		}
	}
	
	//�м䴦��
	privateLoop = (lenght - point) / sizeof(SWID);
	k = KEYBYTESOFFSET * sizeof(SWID);
	while(privateLoop--)
	{
		if(0 == ((keyOffset.QuadPart + point) % k))
		{
			InlineLeftCircleSwid(&publicKey,1);
		}

		InlineLeftCircleSwid(&privateKey,1);
		code = (SWID*)(buffer + point);

		code->data[0] ^= publicKey.data[0];
		code->data[1] ^= publicKey.data[1];
		code->data[2] ^= publicKey.data[2];
		code->data[3] ^= publicKey.data[3];

		/*code->data[0] = ((code->data[0]&0x55555555)<<1) | ((code->data[0]&0xAAAAAAAA)>>1) ^ privateKey.data[0];
		code->data[1] = ((code->data[1]&0x55555555)<<1) | ((code->data[1]&0xAAAAAAAA)>>1) ^ privateKey.data[1];
		code->data[2] = ((code->data[2]&0x55555555)<<1) | ((code->data[2]&0xAAAAAAAA)>>1) ^ privateKey.data[2];
		code->data[3] = ((code->data[3]&0x55555555)<<1) | ((code->data[3]&0xAAAAAAAA)>>1) ^ privateKey.data[3];*/

		code->data[0] = ((code->data[0]&0x55555555)<<1) | ((code->data[0]&0xAAAAAAAA)>>1) ;
		code->data[1] = ((code->data[1]&0x55555555)<<1) | ((code->data[1]&0xAAAAAAAA)>>1) ;
		code->data[2] = ((code->data[2]&0x55555555)<<1) | ((code->data[2]&0xAAAAAAAA)>>1) ;
		code->data[3] = ((code->data[3]&0x55555555)<<1) | ((code->data[3]&0xAAAAAAAA)>>1) ;

		code->data[0] ^= privateKey.data[0];
		code->data[1] ^= privateKey.data[1];
		code->data[2] ^= privateKey.data[2];
		code->data[3] ^= privateKey.data[3];
		point += sizeof(SWID);
	}

	//��β����
	fraction = lenght - point;
	if(fraction)//��β������Ҫ����
	{
		if(0 == ((keyOffset.QuadPart + point) % k))
		{
			InlineLeftCircleSwid(&publicKey,1);
		}

		InlineLeftCircleSwid(&privateKey,1);
		for(i = 0; lenght > point; i++,point++)
		{
			buffer[point] ^= publicKey.byte[i];
			buffer[point] = (buffer[point]&0x55<<1) | (buffer[point]&0xAA>>1);
			buffer[point] ^= privateKey.byte[i];
		}
	}
	
	return buffer;
}

#pragma PAGEDCODE
char * encode2(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	return buffer;
}

#pragma PAGEDCODE
char * encode3(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	return buffer;
}

#pragma PAGEDCODE
char * decode1(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	//���㷨(KEYBYTESLENGHT * KEYBYTESLENGHT * szieof(SWID))(256k)�ڲ��ظ�
	PSWID code;
	unsigned int i,j,k,publicLoop ,privateLoop;
	ULONG point;//��ǰ���崦���ĸ�λ��
	unsigned int fraction;
	
	//����publicKey��ƫ�ƴ���
	publicLoop = (keyOffset.QuadPart / (KEYBYTESOFFSET * sizeof(SWID))) % KEYBYTESOFFSET;//ÿ2KBתһ��,%����һȦ
	publicLoop += keyOffset.QuadPart % (KEYBYTESOFFSET * sizeof(SWID)) ? 1 : 0;
	InlineLeftCircleSwid(&publicKey, publicLoop);

	//����privateKey��ƫ�ƴ���
	privateLoop = (keyOffset.QuadPart / sizeof(SWID)) % KEYBYTESOFFSET;//ÿ16�ֽ�תһ�Σ�%����һȦ
	fraction = keyOffset.QuadPart % sizeof(SWID);
	privateLoop += fraction ? 1 : 0;
	InlineLeftCircleSwid(&privateKey,privateLoop);

	//��ʼ����
	point = 0;//��ʼ������λ��
	if(fraction)//�����ȴ���ǰͷ
	{
		for(i = fraction-1; sizeof(SWID) > i && lenght > point; i++,point++)
		{
			buffer[point] ^= privateKey.byte[i];
			buffer[point] =(buffer[point]&0x55<<1) | (buffer[point]&0xAA>>1);
			buffer[point] ^= publicKey.byte[i];
		}
	}

	//�м䴦��
	privateLoop = (lenght - point) / sizeof(SWID);
	k = KEYBYTESOFFSET * sizeof(SWID);
	while(privateLoop--)
	{
		if(0 == ((keyOffset.QuadPart + point) % k))
		{
			InlineLeftCircleSwid(&publicKey,1);
		}

		InlineLeftCircleSwid(&privateKey,1);
		code = (SWID*)(buffer + point);
		code->data[0] ^= privateKey.data[0];
		code->data[1] ^= privateKey.data[1];
		code->data[2] ^= privateKey.data[2];
		code->data[3] ^= privateKey.data[3];

		/*code->data[0] = ((code->data[0]&0x55555555)<<1) | ((code->data[0]&0xAAAAAAAA)>>1) ^ publicKey.data[0];
		code->data[1] = ((code->data[1]&0x55555555)<<1) | ((code->data[1]&0xAAAAAAAA)>>1) ^ publicKey.data[1];
		code->data[2] = ((code->data[2]&0x55555555)<<1) | ((code->data[2]&0xAAAAAAAA)>>1) ^ publicKey.data[2];
		code->data[3] = ((code->data[3]&0x55555555)<<1) | ((code->data[3]&0xAAAAAAAA)>>1) ^ publicKey.data[3];*/
		code->data[0] = ((code->data[0]&0x55555555)<<1) | ((code->data[0]&0xAAAAAAAA)>>1) ;
		code->data[1] = ((code->data[1]&0x55555555)<<1) | ((code->data[1]&0xAAAAAAAA)>>1) ;
		code->data[2] = ((code->data[2]&0x55555555)<<1) | ((code->data[2]&0xAAAAAAAA)>>1) ;
		code->data[3] = ((code->data[3]&0x55555555)<<1) | ((code->data[3]&0xAAAAAAAA)>>1) ;

		code->data[0] ^= publicKey.data[0];
		code->data[1] ^= publicKey.data[1];
		code->data[2] ^= publicKey.data[2];
		code->data[3] ^= publicKey.data[3];

		point += sizeof(SWID);
	}

	//��β����
	fraction = lenght - point;
	if(fraction)//��β������Ҫ����
	{
		if(0 == ((keyOffset.QuadPart + point) % k))
		{
			InlineLeftCircleSwid(&publicKey,1);
		}

		InlineLeftCircleSwid(&privateKey,1);
		for(i = 0; lenght > point; i++,point++)
		{
			buffer[point] ^= privateKey.byte[i];
			buffer[point] = (buffer[point]&0x55<<1) | (buffer[point]&0xAA>>1);
			buffer[point] ^= publicKey.byte[i];
		}
	}
	return buffer;
}

#pragma PAGEDCODE
char * decode2(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	return buffer;
}

#pragma PAGEDCODE
char * decode3(char *buffer, ULONG lenght, LARGE_INTEGER keyOffset, SWID privateKey, SWID publicKey)
{
	return buffer;
}