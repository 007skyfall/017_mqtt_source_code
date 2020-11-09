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
//	�����ļ����ͳ���
#define MAP_NULL		0
#define MAP_BMP_FILE	1
#define MAP_BMP_HANDLE	2
#define MAP_BMP_DATA	3


/////////////////////////// Classes /////////////////////////////////

typedef struct SHAREFILEHEADER
{
	UINT bfType;				//�ļ�����
	DWORD bfSize;				//�ļ���С
	DWORD bfOffBits;			//�����ݵ�ƫ����
	DWORD bDataSize;			//	���ݴ�С		
	UINT uiMsg;					//��Ϣ	
	WPARAM wParam;				//����
	LPARAM lParam;
	UINT bfReserved1;			//��չ1
	UINT bfReserved2;			//��չ2
}SHAREFILEHEADER;


class CMemMapFile : public CObject
{
public:
	//Constructors / Destructors
	CMemMapFile();
	~CMemMapFile();	



	//���ڴ�ӳ���ļ�
	BOOL OpenMap(CString smapname,bool bReadonly=TRUE,bool bCreate=FALSE);
//**********************************************************************
//���ڴ�ӳ���ļ�Ӧ�ú���
//**********************************************************************	
	BOOL ReadToFile(CString cFileName);
	DWORD ReadToData(LPVOID ptr,UINT iFrom,DWORD dwCount);
	HANDLE ReadToData(UINT iFrom,DWORD dwCount);
	HANDLE ReadToHandle();
	BOOL ReadMapHead();
//**********************************************************************
//д�ڴ�ӳ���ļ�Ӧ�ú���
//**********************************************************************	

	BOOL WriteDataToMap(LPVOID lpData,DWORD dwData);
	BOOL WriteFileToMap(CString sFile);
	BOOL WriteHandleToMap(HANDLE sourHandle);
	BOOL WriteFileToMap(CFile& file);

//**********************************************************************
//ͨ�ú���
//**********************************************************************
	CString GetMappingName() const;	
	DWORD   GetLength(); 
	//	�����ڴ��ļ�ͷ	
	BOOL SetMapMessage(UINT uiMsg ,WPARAM wParam,LPARAM lParam);//���ù����ڴ���Ϣ����
	BOOL SetMapType(UINT uType);
	
	BOOL CopyMapfile(CString tagMapfile,CString srcMapfile);
	void DeleteMap(CString smapname);	

//**********************************************************************
//ͨ�ñ���
//**********************************************************************

	SHAREFILEHEADER m_head;	//�ڴ�����ͷ�ļ��ṹ
#ifdef _DEBUG
	virtual void AssertValid() const;
#endif
	
protected:
	DECLARE_DYNAMIC(CMemMapFile)
	//����
	BOOL m_bOpen;		//�Ƿ��		
	HANDLE  m_hMapping;		//�ڴ�ӳ����
	bool   m_bReadonly;
	CString m_sMappingName;	//�ڴ�ӳ���ļ���

  	//����
	BOOL CreateMap(CString mapname,DWORD mapsize);
	BOOL MapIsExist(CString smapname);
		//�������
	HANDLE CopyHandle (HGLOBAL h);
	void FreeHandle(HANDLE handle);	
private:
	//���ù����ڴ��ļ���
	CString SetMappingName(const CString& sName);
};


#endif //__MEMMAP_H__