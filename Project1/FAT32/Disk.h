#pragma once
#include "Func.h"
#include "Directory.h"
#include "FAT32.h"
class Disk
{
public:
	virtual int init(LPCWSTR  drive) = 0;
	virtual int OpenVolume(LPCWSTR  drive) = 0;
	virtual int ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte = 512) = 0;
	virtual void printBootSector() = 0;
	virtual	void printTreeFolder() = 0;
	virtual void printDataFile(wstring name) = 0;
};
class Diskfat : public Disk
{
private:
	HANDLE			Device;
	FAT32			BSector;
	directory*		RootENTRY;
	UINT16			SecPerClus;
	UINT16			BytePerSec;
	UINT16			FstFAT1;
	UINT16			FstRDET;
public:
	Diskfat();
	~Diskfat();
	int init(LPCWSTR  drive);
	int OpenVolume(LPCWSTR  drive);
	int ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte = 512);
	void readDirectoryTree(directory* Folder = nullptr);
	void printBootSector();
	void printTreeFolder();
	void printDataFile(wstring name);
	vector<UINT32>readListSectorFAT(UINT32 FstClus);
};
