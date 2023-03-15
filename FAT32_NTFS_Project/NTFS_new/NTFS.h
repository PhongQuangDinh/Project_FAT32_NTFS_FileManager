#pragma once
#include "MFT.h"
class NTFS_BoostSector
{
	class BootSector
	{
	public:
		BYTE        jmpBoot[3];
		BYTE        oemID[8];
		WORD        bytePerSec;
		BYTE        secPerClus;
		BYTE        reserved[2];
		BYTE        zero0[3];
		BYTE        unused1[2];
		BYTE        media;
		BYTE        zero1[2];
		WORD        secPerTrk;
		WORD        headNum;
		DWORD       HiddSec;
		BYTE        unused2[8];
		LONGLONG    totSec;
		LONGLONG    MFTClus;
		LONGLONG    MFTMirrClus;
		INT8		clusPerRecord;
		BYTE        unused4[3];
		INT8		clusPerBlock;
		BYTE        unused5[3];
		LONGLONG    serialNum;
		DWORD       checkSum;
		BYTE        bootCode[426];
		BYTE		endMarker[2];
	} boot_sec;
public:

	void ReadBS(BYTE sector[512]) { readFile((char*)&boot_sec, sector, sizeof(NTFS_BoostSector)); }
	DWORD getFstMTFSec() { return boot_sec.MFTClus; }
	UINT16 getBytePerSec() { return boot_sec.bytePerSec; }
	UINT16 getSecPerClus() { return boot_sec.secPerClus; }
	UINT16 getClusPerRecord() { return pow(2, abs(boot_sec.clusPerRecord)); }
	string getFileSystem() { return Byte_to_Str(boot_sec.oemID, 8); }
	void PrintBootSector();
};

class DriveComponent
{
public:
	DWORD indexEntry;
	DWORD Sector;
	FileName Name;
public:
	DriveComponent() { indexEntry = -1; };
	DriveComponent(DWORD index) { indexEntry = index; };
	virtual void addFileName(FileName name) { Name = name; };
	virtual void addComponent(DriveComponent* e) {}
	virtual void printFolderTree(int space = 0) {};
	virtual void PrintInfoEntry(int space = 0);
	virtual DriveComponent* SearchComponent(wstring name) { return nullptr;}
};

class File: public DriveComponent 
{
public:
	File(DWORD index) : DriveComponent(index){ }
	void printFolderTree(int space);
	DriveComponent* SearchComponent(wstring name);
};

class Folder: public DriveComponent 
{
	vector<DriveComponent*> InsideFolder;
public:
	void addComponent(DriveComponent* e) { InsideFolder.push_back(e); };
	Folder(DWORD index) : DriveComponent(index) {}
	void printFolderTree(int space);
	void PrintInfoEntry(int space = 0);
	DriveComponent* SearchComponent(wstring name);
	~Folder();
};

class NTFS
{
	HANDLE Device;
	NTFS_BoostSector BSector;
	WORD SecPerClus;
	WORD BytePerSec;
	DWORD FstMTF;
	WORD BytePerRecord;
	DriveComponent* RootEntry;
public:
	NTFS();
	~NTFS();
	bool Init(LPCWSTR  drive);
	int  OpenVolume(LPCWSTR  drive);
	int  ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte = 512);
	void PrintBootSector() { BSector.PrintBootSector(); }
	void readDirectoryTree(DriveComponent* folder = nullptr);
	void PrintFolderTree();
	void PrintFolderTreeDetail();
	void PrintDataEntry(wstring NameFile);
};