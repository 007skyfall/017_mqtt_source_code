/*
Module : MEMMAP.CPP
Purpose: Implementation for an MFC class to wrap memory mapped files
Created: lfl / MEMMAP/1 / 11-01-2000
History: 
		lfl/2000-11-2  1)添加内存共享文件头结构
					   2）修改内存共享类函数，为每一个内存共享文件添加头
		lfl/2000-11-10
						修改内存映射文件类似于文件操作类型


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
	m_hMapping = NULL;		//内存句柄
	memset(&m_head,0,sizeof(SHAREFILEHEADER));
	m_bReadonly=true;		//只读
	m_bOpen = FALSE;		//是否打开
	m_sMappingName=_T("");	//当前文件名
}

CMemMapFile::~CMemMapFile()
{
  //	UnMap();
}
//返回当前文件名
CString CMemMapFile::GetMappingName() const
{
  return m_sMappingName;
}

//DEL CString CMemMapFile::CreateMutexName() const
//DEL {
//DEL   return m_sMappingName + _T("MUTEX");
//DEL }
//设置内存文件名，修改不符合条件的字符，并MAKEUPPER
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
//设置内存文件消息
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
//设置内存文件类型
BOOL CMemMapFile::SetMapType(UINT uType)
{
	if(m_bOpen&&!m_bReadonly)
	{
		m_head.bfType=uType;
		return true;
	}
	return false;
}
//复制句柄
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
//删除句柄
void CMemMapFile::FreeHandle(HANDLE handle)
{
	if(handle)
	{
		::GlobalUnlock(handle);
		::GlobalFree(handle);
		handle=NULL;
	}
}
//返回当前文件长度
DWORD CMemMapFile::GetLength() 
{ 
	if(m_bOpen)
	{
		DWORD size= m_head.bDataSize;
		return size;
	}
	return 0;
}

//创建当前指定名称和大小的内存文件

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
//将一个句柄写入当前内存映射文件
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
//将一个文件句柄写入当前内存映射文件
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
//将一个文件写入当前内存映射文件
BOOL CMemMapFile::WriteFileToMap(CString sFile)
{
	CFile file(sFile,CFile::modeRead);
	
	if(file.GetLength()!=0)
		return WriteFileToMap(file);
	return false;
}
//将某一段内存内容写入当前内存映射文件
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
//读取当前内存文件到指定的句柄
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
//读取当前内存文件指定位置和大小的数据到指定内存地址
//dwCount=0 则读到文件尾
//把数据COPY到一个内部分配的缓冲区，用户使用完后来释放
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
//ptr:目的地址
//iFrom:位置偏移
//dwCount:读取的数据量
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
//保存当前内存文件为物理文件
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
//读取当前内存文件的头结构
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

//判断内存映象文件是否存在
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
				//	AfxMessageBox("内存文件以存在",MB_OK,NULL);
				return TRUE;
			}
		}
		return FALSE;
	}
}
//打开内存映象文件
//smapname 内存映象文件名
//bCreate  如果内存映象文件不存在，是否创建新的内存文件
//返回是否打开成功
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

//删除内存映射文件
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


//复制内存映射文件
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



