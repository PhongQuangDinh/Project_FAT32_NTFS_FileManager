#pragma once
#include <Windows.h>
#include "Func.h"
#include "FAT32.h"
using namespace std;

#pragma pack(push,1)
class mainDirectory
{
public:
	BYTE	Name[11];
	BYTE	Attr;
	BYTE	Resever;
	BYTE	CrtTimeTenth;
	WORD	CrtTime;
	WORD	CrtDate;
	WORD	LstAccDate;
	WORD	FstClusHi;
	WORD	ModTime;
	WORD	ModDate;
	WORD	FstClusLo;
	UINT32	Size;
public:
	mainDirectory() {
		FstClusLo = 0;
		FstClusHi = 0;
	}
	void read(BYTE sec[32]);
	string getName();
	string getAttribute();
	string getCreateTime();
	string getCreateDate();
	string getLastAccessDate();
	string getModifiedDate();
	string getModifiedTime();
	DWORD  getFirstCluster();
	void   setName(string Name);
	UINT32 getSizeFile();
};
class subDirectory
{
public:
	BYTE LOrd;
	WORD LName1[5];
	BYTE LATT;
	BYTE LType;
	BYTE LChksum;
	WORD LName2[6];
	WORD LFstClusLO;
	WORD LName3[2];
public:
	void read(BYTE sec[32]);
	wstring getFullName();
};
#pragma pack(pop)

class directory {
protected:
	mainDirectory mEntry; 
	vector <subDirectory> sEntry; 

public:
	virtual void readmainDirectory(BYTE[32]);
	virtual void pushsubDirectory(subDirectory s);
	virtual wstring getLongName();
	virtual DWORD getFirstCluster();
	virtual DWORD getSize();

	virtual void AddEntry(directory* en) {}
	virtual void printDir(int x, FAT32 bst);
	virtual directory* SearchEntry(wstring name) {
		return nullptr;
	}
	virtual ~directory() {}
};
class file :public directory
{
public:
	void printTree(int x);
	directory* SearchEntry(wstring name);
};
class folder :public directory
{
protected:
	vector <directory*> listEntry;
public:
	void AddEntry(directory* en);
	void printDir(int x, FAT32 bst);
	directory* SearchEntry(wstring name);
	~folder();
};
class root :public folder
{
public:
	root(string name, WORD FstClus);
};
#define EOC 0x0FFFFFFF
#define DELETE_EN 0xE5
#define FILE_EN 0x20
#define FOLDER_EN 0x10
#define SUB_EN 0x0F 
#define DOT_EN 0x2E
#define HIDDEN_EN 0x02


