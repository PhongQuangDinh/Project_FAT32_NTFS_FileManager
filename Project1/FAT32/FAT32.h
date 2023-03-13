#pragma once
#include"Func.h"
#define BS_OEMName_LENGTH 8
#define BS_VolLab_LENGTH 11
#define BS_FilSysType_LENGTH 8 
#pragma pack(push,1)
struct Fat32BootSector {
	BYTE	BS_jmpBoot[3];
	BYTE	BS_OEMName[BS_OEMName_LENGTH];
	WORD	BytesPerSec;
	BYTE	SecPerClus;
	WORD	RsvdSecCnt;
	BYTE	NumFATs;
	WORD	RootEntCnt;
	WORD	TotSec16;
	BYTE	Media;
	WORD	FATSz16;
	WORD	SecPerTrk;
	WORD	NumHeads;
	DWORD	HiddSec;
	DWORD	TotSec32;
	DWORD	FATSz32;
	WORD	ExtFlags;
	WORD	FSVer;
	DWORD	RootClus;
	WORD	FSInfo;
	WORD	BkBootSec;
	BYTE	reserved[12];
	BYTE	DrvNum;
	BYTE	Reserved1;
	BYTE	BootSig;
	DWORD	VolID;
	BYTE	BS_VolLab[BS_VolLab_LENGTH];
	BYTE    BS_FilSysType[BS_FilSysType_LENGTH];
	BYTE	BS_CodeReserved[420];
	BYTE	Sig[2];
};
#pragma pack(pop)
class FAT32 {
public:
	Fat32BootSector BootSector;
public:
	void Read(BYTE[512]);
	UINT16 getFstFAT();
	UINT16 getFstRDET();
	UINT16 getBytePerSec();
	UINT16 getSecPerClus();
	UINT16 getClusRoot();
	string getFileSystem()
	{
		return ByteToStr(BootSector.BS_FilSysType, 8);
	}
	void printBootSector();
};
