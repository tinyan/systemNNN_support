//
// namelist.cpp
//

#include <windows.h>
#include <mbstring.h>
#include <stdio.h>

#include "namelist.h"

#define WORK_LENGTH 64

//#define VAR_FILE_NAME "nnndir\\var.txt"


CNameList::CNameList()
{
	m_nameKosuu = 0;
	m_nameKosuuMax = 0;
	m_nameData = NULL;
	m_editFlag = FALSE;

	ExpandWork(1000);
}

CNameList::CNameList(int n)
{
	m_nameKosuu = 0;
	m_nameKosuuMax = 0;
	m_nameData = NULL;
	m_editFlag = FALSE;

	ExpandWork(n);
}


CNameList::~CNameList()
{
	End();
}

void CNameList::End(void)
{
	if (m_nameData != NULL)
	{
		delete [] m_nameData;
		m_nameData = NULL;
		m_nameKosuuMax = 0;
		m_nameKosuu = 0;
	}
}


LPSTR CNameList::CheckSameNameExist(void)
{
	for (int i=0;i<m_nameKosuu-1;i++)
	{
		LPSTR name = m_nameData + WORK_LENGTH * i;
		for (int j=i+1;j<m_nameKosuu;j++)
		{
			LPSTR name2 = m_nameData + WORK_LENGTH * j;
			if (strcmp(name,name2)==0) return name;
		}
	}

	return NULL;
}


void CNameList::AddName(LPSTR name, BOOL bNoCheckFlag)
{
	int ln = strlen(name);
	if ((ln>WORK_LENGTH) || (ln==0))
	{
		if (bNoCheckFlag == FALSE)
		{
			MessageBox(NULL,"�ϐ��̒����ɖ�肪����܂��ɂ�","ERROR",MB_OK);
		}
		return;
	}

	CopyMemory(m_nameData + m_nameKosuu * WORK_LENGTH,name,ln+1);
	m_nameKosuu++;

	if (m_nameKosuu>=m_nameKosuuMax)
	{
		ExpandWork();
	}
}



BOOL CNameList::ExpandWork(int n)
{
	int k = m_nameKosuuMax + n;
	char* tmp = new char[k*sizeof(char)*WORK_LENGTH];
	if (tmp == NULL) return FALSE;

	ZeroMemory(tmp,k*sizeof(char)*WORK_LENGTH);


	if (m_nameData != NULL)
	{
		CopyMemory(tmp,m_nameData,m_nameKosuuMax * sizeof(char) * WORK_LENGTH);
		delete [] m_nameData;
	}

	if (n<0)
	{
		MessageBox(NULL,"expand error","error",MB_OK);
	}

	ZeroMemory(tmp+m_nameKosuuMax*sizeof(char)*WORK_LENGTH,n*sizeof(char)*WORK_LENGTH);

	m_nameData = tmp;
	m_nameKosuuMax += n;
	return TRUE;
}


BOOL CNameList::SaveFile(LPSTR filename,BOOL angouFlag)
{
	char tmp[(WORK_LENGTH+2)*10+256];

	int n = GetNameKosuu();

	FILE* file = fopen(filename,"wb");
	for (int j=0;j<n / 5;j++)
	{
		int k = j * 5;

		sprintf(tmp,"%s,%s,%s,%s,%s,\x00d\x00a",
			m_nameData+k*WORK_LENGTH,
			m_nameData+(k+1)*WORK_LENGTH,
			m_nameData+(k+2)*WORK_LENGTH,
			m_nameData+(k+3)*WORK_LENGTH,
			m_nameData+(k+4)*WORK_LENGTH);
		fwrite(tmp,sizeof(char),strlen(tmp),file);
	}
	fclose(file);

	if (angouFlag) SaveAngouRoutine(filename);

	m_editFlag = FALSE;
	return TRUE;
}

BOOL CNameList::SaveFile2(LPSTR filename,BOOL angouFlag)
{
	char tmp[(WORK_LENGTH+2)*10+256];

	int n = GetNameKosuu();

	FILE* file = fopen(filename,"wb");
	for (int j=0;j<n / 2;j++)
	{
		int k = j * 2;

		sprintf(tmp,"%s,%s\x00d\x00a",
			m_nameData+k*WORK_LENGTH,
			m_nameData+(k+1)*WORK_LENGTH);
		fwrite(tmp,sizeof(char),strlen(tmp),file);
	}
	fclose(file);

	if (angouFlag) SaveAngouRoutine(filename);

	m_editFlag = FALSE;
	return TRUE;
}


void CNameList::SaveAngouRoutine(LPSTR filename)
{
	int tmp[8192/4];

	FILE* file = fopen(filename,"r+b");

	BOOL flg = TRUE;
	int block = 0;

	while (flg)
	{
		fseek(file,block*8192,SEEK_SET);
		int ln = fread(tmp,sizeof(char),8192,file);

		//
		for (int i=0;i<8192/4;i++)
		{
			tmp[i] ^= 0xffffffff;
		}

		if (ln>0)
		{
			fseek(file,block*8192,SEEK_SET);
			fwrite(tmp,sizeof(char),ln,file);
		}

		block++;
		if (ln<8192) flg = FALSE;
	}

	fclose(file);


}




BOOL CNameList::LoadFile(LPSTR filename,BOOL angouFlag)
{
	int kosuu = m_nameKosuuMax;


	int lMax = 0;
	FILE* file = fopen(filename,"rb");
	if (file == NULL)
	{
		MessageBox(NULL,filename,"file not found",MB_OK);
		return FALSE;
	}

	fseek(file,0,SEEK_END);
	int fsize = ftell(file);
	fseek(file,0,SEEK_SET);


//	char* tmp = new char[(WORK_LENGTH+6)*kosuu];
	char* tmp = new char[fsize+2];

	if (tmp == NULL)
	{
		fclose(file);
		MessageBox(NULL,"�������[�s���ɂ�","ERROR",MB_OK);
		return FALSE;
	}

//	ZeroMemory(tmp,fsize+2);

	lMax = fread(tmp,sizeof(char),fsize,file);
	fclose(file);

	if (angouFlag)
	{
		for (int i=0;i<lMax;i++)
		{
			*(tmp+i) ^= 0xff;
		}
	}


	*(tmp+lMax) = 0;
	*(tmp+lMax+1) = 0;

//	m_nameKosuu = 0;
	int n = 0;
	while (n<lMax)
	{
		int sk = SkipSpace(tmp+n);
		n += sk;
		if (n<lMax)
		{
			int ln = GetToken(tmp+n);
//			if ((ln>WORK_LENGTH) || (ln==0))
			if (ln>WORK_LENGTH)
			{
				MessageBox(NULL,"�ϐ��̒����ɖ�肪����܂��ɂ�","ERROR",MB_OK);
				break;
			}

			if (ln>0)
			{
				int hhh = 0;
				hhh++;
			}
			CopyMemory(m_nameData + m_nameKosuu * WORK_LENGTH,tmp+n,ln);
			*(m_nameData + m_nameKosuu * WORK_LENGTH+ln) = 0;
			n += ln;
			m_nameKosuu++;

			if (m_nameKosuu>=m_nameKosuuMax)
			{
				ExpandWork();
			}
		}

	}

	for (int i=m_nameKosuu;i<m_nameKosuuMax;i++)
	{
		*(m_nameData + i * WORK_LENGTH) = 0;
	}

	delete [] tmp;
	m_editFlag = FALSE;
	return TRUE;
}

int CNameList::SkipSpace(LPSTR ptr)
{
	char c = *ptr;
	int n = 0;

	if (c == ',')
	{
		ptr++;
		c = *ptr;
		n++;
	}

//	while ((c  == ' ') || (c == '\t') || (c == ',') || (c == 0x0a) || (c == 0x0d))
	while ((c  == ' ') || (c == '\t') ||               (c == 0x0a) || (c == 0x0d))
	{
		ptr++;
		c = *ptr;
		n++;
	}

	return n;
}


int CNameList::GetToken(LPSTR ptr)
{
	char c = *ptr;
	int n = 0;

	while ((c != 0) && (c  != ' ') && (c != '\t') && (c != ',') && (c != 0x0a) && (c != 0x0d))
	{
		ptr++;
		c = *ptr;
		n++;
	}

	return n;
}


int CNameList::GetNameKosuu(void)
{
	return m_nameKosuu;
}

LPSTR CNameList::GetName(int n)
{
	if (n<0) return NULL;
	if (n>=m_nameKosuu) return NULL;
	return m_nameData + n * sizeof(char) * WORK_LENGTH;
}


void CNameList::SetName(int n,LPSTR name)
{
	if (name == NULL) return;
	if ((n<0) || (n>=m_nameKosuuMax)) return;

	int ln = strlen(name);
	if (ln>WORK_LENGTH-2) ln = WORK_LENGTH-2;

	memcpy(m_nameData + n * WORK_LENGTH,name,ln);
	*(m_nameData + n * WORK_LENGTH+ln) = 0;
	*(m_nameData + n * WORK_LENGTH+ln+1) = 0;

	if (n >= m_nameKosuu) m_nameKosuu = n+1;

	m_editFlag = TRUE;
}

int CNameList::GetNameNumber(LPSTR nam,int skip,int start)
{
	if (m_nameKosuu == 0) return -1;

	int l1 = strlen(nam);
	for (int i=start;i<m_nameKosuu;i+=skip)
	{
		int l2 = strlen(m_nameData + i * sizeof(char) * WORK_LENGTH);
		if (l1 == l2)
		{
//			if (memcmp(nam,m_nameData + i * sizeof(char) * WORK_LENGTH,l1) == 0) return i;
			if (strcmp(nam,m_nameData + i * sizeof(char) * WORK_LENGTH) == 0) return i;
		}
	}

	return -1;
}


BOOL CNameList::CheckModify(void)
{
	return m_editFlag;
}


int CNameList::SearchName(LPSTR name, int searchStart, BOOL backSearchFlag)
{

	int dlt = 1;
	if (backSearchFlag) dlt = -1;
	int kosuu = m_nameKosuu;

	if (kosuu <= 0) return -1;

	int n = searchStart;
	if (searchStart == -1)
	{
		if (backSearchFlag == FALSE)
		{
			n = 0;
		}
		else
		{
			n = kosuu - 1;
		}
	}

	n %= kosuu;

	for (int i=0;i<kosuu;i++)
	{
		LPSTR mes = m_nameData + n * WORK_LENGTH;
		if ((*mes) != 0)
		{
			if (strcmp(name,mes)==0)
			{
				return n;
			}
		}

		n += dlt;
		n += kosuu;
		n %= kosuu;
	}

	return -1;
}





int CNameList::SearchName2(LPSTR name, BOOL stricmpFlag, int searchStart , BOOL backSearchFlag)
{

	int dlt = 2;
	if (backSearchFlag) dlt *= -1;
	int kosuu = m_nameKosuu;
	int loopKosuu = kosuu / 2;

	if (kosuu <= 0) return -1;

	int n = searchStart;
	if (searchStart == -1)
	{
		if (backSearchFlag == FALSE)
		{
			n = 0;
		}
		else
		{
			n = kosuu/2 - 2;
			if (n<0) n = 0;
			n *= 2;
		}
	}

	n %= kosuu;


	for (int i=0;i<loopKosuu;i++)
	{
		LPSTR mes = m_nameData + n * WORK_LENGTH;
		if ((*mes) != 0)
		{
			if (stricmpFlag)
			{
				if (_stricmp(name,mes)==0)
				{
					return n;
				}
			}
			else
			{
				if (strcmp(name,mes)==0)
				{
					return n;
				}
			}
		}

		n += dlt;
		n += kosuu;
		n %= kosuu;
	}

	return -1;
}



void CNameList::SetModify(BOOL b)
{
	m_editFlag = b;
}


BOOL CNameList::MoveBlock(int from, int to, int size)
{
	if (from<0) return FALSE;
	if (to<0) return FALSE;
	if (size<0) return FALSE;
	if (size==0) return TRUE;
	if (from+size>m_nameKosuuMax) return FALSE;
	if (to+size>m_nameKosuuMax) return FALSE;

	if (from == to) return TRUE;

	memmove(m_nameData+to*WORK_LENGTH,m_nameData+from*WORK_LENGTH,size*WORK_LENGTH);
	return TRUE;
}

void CNameList::SetNameKosuuIsMax(void)
{
	m_nameKosuu = m_nameKosuuMax;
}


void CNameList::AdjustNameKosuu(int kosuu)
{
	if (kosuu > m_nameKosuuMax) return;

	m_nameKosuu = kosuu;
}



/*_*/
