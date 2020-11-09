/*
Module : MEMMAP.H
Purpose: Interface for an MFC class to wrap memory mapped files
Created: PJN / 30-07-1997


Copyright (c) 1997 - 1998 by PJ Naughter.  
All rights reserved.

*/


////////////////////////////////// Macros ///////////////////////////

#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#define MAP_NOT_DATA sizeof(SHAREFILEHEADER)
//	定义文件类型常数
#define MAP_NULL		0
#define MAP_BMP_FILE	1
#define MAP_BMP_HANDLE	2
#define MAP_BMP_DATA	3


/////////////////////////// Classes /////////////////////////////////

typedef struct SHAREFILEHEADER
{
	UINT bfType;				//文件类型
	DWORD bfSize;				//文件大小
	DWORD bfOffBits;			//到数据的偏移量
	DWORD bDataSize;			//	数据大小		
	UINT uiMsg;					//消息	
	WPARAM wParam;				//参数
	LPARAM lParam;
	UINT bfReserved1;			//扩展1
	UINT bfReserved2;			//扩展2
}SHAREFILEHEADER;


class CMemMapFile : public CObject
{
public:
	//Constructors / Destructors
	CMemMapFile();
	~CMemMapFile();	



	//打开内存映象文件
	BOOL OpenMap(CString smapname,bool bReadonly=TRUE,bool bCreate=FALSE);
//**********************************************************************
//读内存映象文件应用函数
//**********************************************************************	
	BOOL ReadToFile(CString cFileName);
	DWORD ReadToData(LPVOID ptr,UINT iFrom,DWORD dwCount);
	HANDLE ReadToData(UINT iFrom,DWORD dwCount);
	HANDLE ReadToHandle();
	BOOL ReadMapHead();
//**********************************************************************
//写内存映象文件应用函数
//**********************************************************************	

	BOOL WriteDataToMap(LPVOID lpData,DWORD dwData);
	BOOL WriteFileToMap(CString sFile);
	BOOL WriteHandleToMap(HANDLE sourHandle);
	BOOL WriteFileToMap(CFile& file);

//**********************************************************************
//通用函数
//**********************************************************************
	CString GetMappingName() const;	
	DWORD   GetLength(); 
	//	设置内存文件头	
	BOOL SetMapMessage(UINT uiMsg ,WPARAM wParam,LPARAM lParam);//设置共享内存消息参数
	BOOL SetMapType(UINT uType);
	
	BOOL CopyMapfile(CString tagMapfile,CString srcMapfile);
	void DeleteMap(CString smapname);	

//**********************************************************************
//通用变量
//**********************************************************************

	SHAREFILEHEADER m_head;	//内存数据头文件结构
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif
	
protected:
	DECLARE_DYNAMIC(CMemMapFile)
	//变量
	BOOL m_bOpen;		//是否打开		
	HANDLE  m_hMapping;		//内存映象句柄
	bool   m_bReadonly;
	CString m_sMappingName;	//内存映射文件名

  	//函数
	BOOL CreateMap(CString mapname,DWORD mapsize);
	BOOL MapIsExist(CString smapname);
		//句柄操作
	HANDLE CopyHandle (HGLOBAL h);
	void FreeHandle(HANDLE handle);	
private:
	//配置共享内存文件名
	CString SetMappingName(const CString& sName);
};


#endif //__MEMMAP_H__
