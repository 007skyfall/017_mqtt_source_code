/*
Module : MEMMAP.CPP
Purpose: Implementation for an MFC class to wrap memory mapped files
Created: lfl / MEMMAP/1 / 11-01-2000
History: 
		lfl/2000-11-2  1)�����ڴ湲���ļ�ͷ�ṹ
					   2���޸��ڴ湲���ຯ����Ϊÿһ���ڴ湲���ļ�����ͷ
		lfl/2000-11-10
						�޸��ڴ�ӳ���ļ��������ļ���������


Copyright (c) 1999 - 2000 by lfl.  
All rights reserved.

*/

/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "memmap.h"

///////////////////////////////// Implementation //////////////////////////////

IMPLEMENT_DYNAMIC(CMemMapFile, CObject)

CMemMapFile::CMemMapFile()
{
	//Initialise variables to sane values
	m_hMapping = NULL;		//�ڴ���
	memset(&m_head,0,sizeof(SHAREFILEHEADER));
	m_bReadonly=true;		//ֻ��
	m_bOpen = FALSE;		//�Ƿ��
	m_sMappingName=_T("");	//��ǰ�ļ���
}

CMemMapFile::~CMemMapFile()
{
  //	UnMap();
}
//���ص�ǰ�ļ���
CString CMemMapFile::GetMappingName() const
{
  return m_sMappingName;
}

//DEL CString CMemMapFile::CreateMutexName() const
//DEL {
//DEL   return m_sMappingName + _T("MUTEX");
//DEL }
//�����ڴ��ļ������޸Ĳ������������ַ�����MAKEUPPER
CString CMemMapFile::SetMappingName(const CString& sName)
{
  CString rVal;
      rVal = sName;
    for (int i=0; i<sName.GetLength(); i++)
    {
      if (rVal.GetAt(i) == _T('\\'))
        rVal.SetAt(i, _T('_'));
	  if(rVal.GetAt(i)==_T(' '))
		  rVal.SetAt(i,_T('-'));
    }
    rVal.MakeUpper();
  return rVal;
 
  
}

#ifdef _DEBUG
void CMemMapFile::AssertValid() const
{
  CObject::AssertValid();
}
#endif

#ifdef _DEBUG

#endif
//�����ڴ��ļ���Ϣ
BOOL CMemMapFile::SetMapMessage(UINT uiMsg ,WPARAM wParam,LPARAM lParam)
{
	if(m_bOpen&&!m_bReadonly)
	{
		m_head.uiMsg=uiMsg&0xffff;
		m_head.wParam=wParam&0xffff;
		m_head.lParam=lParam&0xffff;
		return true;
	}
	return false;
}
//�����ڴ��ļ�����
BOOL CMemMapFile::SetMapType(UINT uType)
{
	if(m_bOpen&&!m_bReadonly)
	{
		m_head.bfType=uType;
		return true;
	}
	return false;
}
//���ƾ��
HANDLE CMemMapFile::CopyHandle (HGLOBAL h)
{
	if (h == NULL)
		return NULL;

	DWORD dwLen = ::GlobalSize((HGLOBAL) h);
	HGLOBAL hCopy = ::GlobalAlloc(GHND, dwLen);

	if (hCopy != NULL)
	{
		void* lpCopy = ::GlobalLock((HGLOBAL) hCopy);
		void* lp     = ::GlobalLock((HGLOBAL) h);
		memcpy(lpCopy, lp, dwLen);
		::GlobalUnlock(hCopy);
		::GlobalUnlock(h);
	}

	return hCopy;
}
//ɾ�����
void CMemMapFile::FreeHandle(HANDLE handle)
{
	if(handle)
	{
		::GlobalUnlock(handle);
		::GlobalFree(handle);
		handle=NULL;
	}
}
//���ص�ǰ�ļ�����
DWORD CMemMapFile::GetLength() 
{ 
	if(m_bOpen)
	{
		DWORD size= m_head.bDataSize;
		return size;
	}
	return 0;
}

//������ǰָ�����ƺʹ�С���ڴ��ļ�

BOOL CMemMapFile::CreateMap(CString mapname, DWORD mapsize)
{
	if(mapsize<0)
		mapsize=0;
	if(mapsize==0)
		mapsize=4;
	if(MapIsExist(mapname))
		return false;
	CString smap=SetMappingName(mapname);
	//	m_hMapping = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, mapsize+sizeof(SHAREFILEHEADER),smap);
	DWORD dwLength=mapsize+sizeof(SHAREFILEHEADER);
	m_hMapping = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dwLength, smap);
	if(m_hMapping!=NULL)
	{
		 
		m_head.bDataSize=mapsize;
		m_head.bfOffBits=sizeof(SHAREFILEHEADER);
		m_head.bfSize=mapsize+sizeof(SHAREFILEHEADER);
		return true;
	}
	return false;
}
//��һ�����д�뵱ǰ�ڴ�ӳ���ļ�
BOOL CMemMapFile::WriteHandleToMap(HANDLE sourHandle)
{
	if((sourHandle!=NULL)&&m_hMapping!=NULL&&!m_bReadonly)
	{
		DWORD hSize=::GlobalSize(sourHandle);
		DeleteMap(m_sMappingName);
		CreateMap(m_sMappingName,hSize);
		LPVOID lpvie=::MapViewOfFile(m_hMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
		if(lpvie!=NULL)
		{
			memcpy(lpvie,&m_head,sizeof(SHAREFILEHEADER));
			//	DWORD hSize=::GlobalSize(sourHandle);
			LPVOID lpsour=::GlobalLock(sourHandle);
			BYTE* lpdata=(BYTE*)lpvie+sizeof(SHAREFILEHEADER);
			memcpy(lpdata,lpsour,hSize);
			::GlobalUnlock(sourHandle);
			::UnmapViewOfFile(lpvie);
			return true;
		}

		return false;
	}
	return false;
}
//��һ���ļ����д�뵱ǰ�ڴ�ӳ���ļ�
BOOL CMemMapFile::WriteFileToMap(CFile& file)
{

	if((file.GetLength()!=0)&&m_hMapping!=NULL&&!m_bReadonly)
	{
		DWORD hSize=file.GetLength();
		DeleteMap(m_sMappingName);
		CreateMap(m_sMappingName,hSize);
		LPVOID lpvie=::MapViewOfFile(m_hMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
		if(lpvie!=NULL)
		{
			memcpy(lpvie,&m_head,sizeof(SHAREFILEHEADER));
	
			//	LPVOID lpsour=::GlobalLock(sourHandle);
			BYTE* lpdata=(BYTE*)lpvie+sizeof(SHAREFILEHEADER);
			//	memcpy(lpdata,lpsour,hSize);
			//file.ReadHuge(lpdata,hSize);
			file.Read(lpdata, hSize);

			::UnmapViewOfFile(lpvie);
			return true;
		}
		return false;
	}
	return false;
}
//��һ���ļ�д�뵱ǰ�ڴ�ӳ���ļ�
BOOL CMemMapFile::WriteFileToMap(CString sFile)
{
	CFile file(sFile,CFile::modeRead);
	
	if(file.GetLength()!=0)
		return WriteFileToMap(file);
	return false;
}
//��ĳһ���ڴ�����д�뵱ǰ�ڴ�ӳ���ļ�
BOOL CMemMapFile::WriteDataToMap(LPVOID lpData, DWORD dwData)
{

	if((lpData!=NULL)&&dwData>0&&m_hMapping!=NULL&&!m_bReadonly)
	{
		DWORD hSize=dwData;
		DeleteMap(m_sMappingName);
		CreateMap(m_sMappingName,hSize);
		LPVOID lpvie=::MapViewOfFile(m_hMapping,FILE_MAP_READ|FILE_MAP_WRITE,0,0,0);
		if(lpvie!=NULL)
		{
			memcpy(lpvie,&m_head,sizeof(SHAREFILEHEADER));
	
			//	LPVOID lpsour=::GlobalLock(sourHandle);
			BYTE* lpda=(BYTE*)lpvie+sizeof(SHAREFILEHEADER);
			memcpy(lpda,lpData,hSize);
			::UnmapViewOfFile(lpvie);
			return true;
		}
		::UnmapViewOfFile(lpvie);
	}
	return false;
}
//��ȡ��ǰ�ڴ��ļ���ָ���ľ��
HANDLE CMemMapFile::ReadToHandle()
{
	if(m_hMapping!=NULL&&m_bOpen)
	{
		ReadMapHead();
		DWORD dwDesiredAccess = (!m_bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
		LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_READ|dwDesiredAccess,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
			HANDLE handle=::GlobalAlloc(GHND|GMEM_SHARE,m_head.bDataSize);
			LPVOID lpdata=::GlobalLock(handle);
			BYTE* lpVie1=(BYTE*)lpVie+sizeof(SHAREFILEHEADER);
			memcpy(lpdata,lpVie1,m_head.bDataSize);
			GlobalUnlock(handle);
			UnmapViewOfFile((LPCVOID)lpVie);
			if(handle!=NULL)
				return handle;
		}
		UnmapViewOfFile((LPCVOID)lpVie);
	}
	return NULL;
}
//��ȡ��ǰ�ڴ��ļ�ָ��λ�úʹ�С�����ݵ�ָ���ڴ��ַ
//dwCount=0 ������ļ�β
//������COPY��һ���ڲ�����Ļ��������û�ʹ��������ͷ�
HANDLE CMemMapFile::ReadToData(UINT iFrom,DWORD dwCount)
{
	if(m_hMapping!=NULL&&m_bOpen)
	{
		ReadMapHead();
		if(iFrom>m_head.bDataSize)
			return NULL;
		if((iFrom+dwCount)>m_head.bDataSize||dwCount==0)
			dwCount=m_head.bDataSize-iFrom;
		DWORD dwDesiredAccess = (!m_bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
		LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_READ|dwDesiredAccess,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
			HANDLE handle=::GlobalAlloc(GHND|GMEM_SHARE,dwCount);
			LPVOID lpdata=::GlobalLock(handle);
			BYTE* lpVie1=(BYTE*)lpVie+sizeof(SHAREFILEHEADER)+iFrom;
			memcpy(lpdata,lpVie1,dwCount);
			GlobalUnlock(handle);
			UnmapViewOfFile((LPCVOID)lpVie);
			return handle;
		}
		UnmapViewOfFile((LPCVOID)lpVie);
	}
	return NULL;
}
//ptr:Ŀ�ĵ�ַ
//iFrom:λ��ƫ��
//dwCount:��ȡ��������
DWORD CMemMapFile::ReadToData(LPVOID ptr,UINT iFrom,DWORD dwCount)
{
	if((m_hMapping!=NULL && m_bOpen) && ptr!=NULL && dwCount>0)
	{
		ReadMapHead();
		if(iFrom>m_head.bDataSize)
			return 0;
		if((iFrom+dwCount)>m_head.bDataSize)
			dwCount=m_head.bDataSize-iFrom;
		DWORD dwDesiredAccess = (!m_bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
		LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_READ|dwDesiredAccess,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
			BYTE* lpVie1=(BYTE*)lpVie+sizeof(SHAREFILEHEADER)+iFrom;
			memcpy(ptr,lpVie1,dwCount);
			UnmapViewOfFile((LPCVOID)lpVie);
			return dwCount;
		}
		UnmapViewOfFile((LPCVOID)lpVie);
	}
	return 0;
}
//���浱ǰ�ڴ��ļ�Ϊ�����ļ�
BOOL CMemMapFile::ReadToFile(CString cFileName)
{
	if(m_hMapping!=NULL&&m_bOpen)
	{
		try
		{
			ReadMapHead();
			CFile file((LPCTSTR)cFileName,CFile::modeCreate|CFile::modeWrite);
			DWORD dwDesiredAccess = (!m_bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
			LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_READ|dwDesiredAccess,0,0,0);
			if((BYTE*)lpVie!=NULL)
			{
				BYTE* lpVie1=(BYTE*)lpVie+sizeof(SHAREFILEHEADER);
				//file.WriteHuge(lpVie1,m_head.bDataSize);
				file.Write(lpVie1, m_head.bDataSize);
				file.Close();
				UnmapViewOfFile((LPCVOID)lpVie);
				return true;
			}
			UnmapViewOfFile((LPCVOID)lpVie);
		}
		catch(CFileException* e)
		{
			e->ReportError(MB_OK,0);
			e->Delete();
			//	m_database.Rollback();
			return FALSE;
		}
	}
	return false;
}
//��ȡ��ǰ�ڴ��ļ���ͷ�ṹ
BOOL CMemMapFile::ReadMapHead()
{
	memset(&m_head,0,sizeof(SHAREFILEHEADER));
	if(m_hMapping!=NULL&&m_bOpen)
	{
		DWORD dwDesiredAccess = (!m_bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
		LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_READ|dwDesiredAccess,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
			
			memcpy(&m_head,lpVie,sizeof(SHAREFILEHEADER));
			UnmapViewOfFile((LPCVOID)lpVie);
		}
	}
	return (m_head.bDataSize!=0);
}

//�ж��ڴ�ӳ���ļ��Ƿ����
BOOL CMemMapFile::MapIsExist(CString smapname)
{
	CString mapname=SetMappingName(smapname);
	if(m_sMappingName==mapname)	 
	{
		if(m_bOpen&&m_hMapping!=NULL)
			return true;
	}
	HANDLE hMapping = ::OpenFileMapping(FILE_MAP_READ,false, mapname);
	if (hMapping == NULL)
	{
		TRACE("Failed in call to CreateFileMapping, GetLastError returned %d\n", GetLastError());			
		return FALSE;
	}
	else
	{
		LPVOID lpdata=::MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0);
		if((BYTE*)lpdata!=NULL)
		{
			SHAREFILEHEADER head;
			memcpy(&head,lpdata,sizeof(SHAREFILEHEADER));
			if(head.bfSize<sizeof(SHAREFILEHEADER))
				return false;
			else
			{
				//	if(GetLastError()==ERROR_ALREADY_EXISTS)			
				//	AfxMessageBox("�ڴ��ļ��Դ���",MB_OK,NULL);
				return TRUE;
			}
		}
		return FALSE;
	}
}
//���ڴ�ӳ���ļ�
//smapname �ڴ�ӳ���ļ���
//bCreate  ����ڴ�ӳ���ļ������ڣ��Ƿ񴴽��µ��ڴ��ļ�
//�����Ƿ�򿪳ɹ�
BOOL CMemMapFile::OpenMap(CString smapname,bool bReadonly, bool bCreate)
{
	if(m_bOpen||m_hMapping!=NULL)
	{
		m_bOpen=false;
		m_hMapping=NULL;
	}

	if(MapIsExist(smapname))
	{
		m_bReadonly=bReadonly;
		DWORD dwDesiredAccess = (!bReadonly) ? FILE_MAP_WRITE : FILE_MAP_READ;
		m_sMappingName=SetMappingName(smapname);
		m_hMapping=OpenFileMapping(FILE_MAP_READ|dwDesiredAccess,false,m_sMappingName);
		if( m_hMapping!=NULL)
			ReadMapHead();
		//	return true;

	}
	else
	{
		if(bCreate)
		{
			m_bReadonly=false;
			if(CreateMap(smapname,0))
				m_sMappingName=SetMappingName(smapname);

			//	return m_hMapping!=NULL;
		}
	}
	if(m_hMapping!=NULL)
	{
		m_bOpen=true;
		return true;
	}
	return false;
}

//ɾ���ڴ�ӳ���ļ�
void CMemMapFile::DeleteMap(CString smapname)
{
	CString mapname=SetMappingName(smapname);
	if(mapname==m_sMappingName)
	{
		if(!m_bOpen||m_hMapping==NULL)
			m_hMapping= ::OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE, FALSE, mapname);

		LPCVOID lpVie=MapViewOfFile(m_hMapping,FILE_MAP_WRITE,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
//		
			memset(&m_head,0,sizeof(SHAREFILEHEADER));
			memset((void *)lpVie,0,sizeof(SHAREFILEHEADER));
			UnmapViewOfFile((LPCVOID)lpVie);
		}
		//	m_hMapping =CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, 1, mapname);
		//	memset(&m_head,0,sizeof(SHAREFILEHEADER));
		m_bOpen=false;
		CloseHandle(m_hMapping);
		m_hMapping=NULL;
		
		return;
	}
	if(!MapIsExist(mapname))
		return;
	HANDLE hMapping= ::OpenFileMapping(FILE_MAP_READ|FILE_MAP_WRITE, FALSE, mapname);
	if(hMapping!=NULL)
	{
		LPCVOID lpVie=MapViewOfFile(hMapping,FILE_MAP_WRITE,0,0,0);
		if((BYTE*)lpVie!=NULL)
		{
//			SHAREFILEHEADER head;
//			memcpy(&head,lpVie,sizeof(SHAREFILEHEADER));
			memset((void *)lpVie,0,sizeof(SHAREFILEHEADER));
			UnmapViewOfFile((LPCVOID)lpVie);
		}
//		//	delete head;
		CloseHandle(hMapping);
		hMapping=NULL;
	}
}


//�����ڴ�ӳ���ļ�
BOOL CMemMapFile::CopyMapfile(CString tagMapfile, CString srcMapfile)
{
	if(MapIsExist(srcMapfile)&&!MapIsExist(tagMapfile))
	{
		HANDLE hMapping;
		CString mapname=SetMappingName(srcMapfile);
		if(mapname==m_sMappingName)
		{
			if(m_bOpen&&m_hMapping!=NULL)
				hMapping=m_hMapping;
			else
				hMapping=OpenFileMapping(FILE_MAP_READ,false,SetMappingName(srcMapfile));
		}
		else
		hMapping=OpenFileMapping(FILE_MAP_READ,false,SetMappingName(srcMapfile));
		if(hMapping!=NULL)
		{	
			LPCVOID lpVie=MapViewOfFile(hMapping,FILE_MAP_READ,0,0,0);//sizeof(SHAREFILEHEADER));
			if((BYTE*)lpVie!=NULL)
			{
				SHAREFILEHEADER head;
				memset(&head,0,sizeof(SHAREFILEHEADER));
				memcpy(&head,lpVie,sizeof(SHAREFILEHEADER));
				if(head.bfOffBits==sizeof(SHAREFILEHEADER)&&head.bDataSize>0)
				{
					HANDLE tagMap = ::CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, head.bfSize,SetMappingName(tagMapfile));
					if(tagMap!=NULL)
					{
						LPVOID lptag=MapViewOfFile(tagMap,FILE_MAP_WRITE,0,0,0);//sizeof(SHAREFILEHEADER));
						if((BYTE*)lptag!=NULL)
						{
							memcpy(lptag,lpVie,head.bfSize);		
							UnmapViewOfFile(lptag);
							UnmapViewOfFile(lpVie);
							return true;
						}
						else
						{
							UnmapViewOfFile(lptag);
							UnmapViewOfFile(lpVie);
							DeleteMap(tagMapfile);
							return false;
						}	
					}	
				}
			}
			UnmapViewOfFile(lpVie);	
		}
	}
	return false;
}


