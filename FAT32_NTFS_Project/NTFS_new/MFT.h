#pragma once
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <stdint.h>
#include <ctime>
#include <math.h>
#include <iomanip>
#include <Vector>
#include <stack>
#include <algorithm>
using namespace std;
// additional function for stuff
void readFile(char* buf, BYTE byte[], int size);
string Byte_to_Str(BYTE[], int size);
UINT32 Hex_to_dec(string hexSector, bool mode = true);
UINT Str_to_Dec(BYTE[], int size, bool mode = true);
string DataSizeFomat(UINT64 size);
bool strEqual(wstring str1, wstring str2);

bool isDelete(BYTE[32]);
bool isFolder(BYTE[32]);
bool isFile(BYTE[32]);
bool isSUB_ENTRY(BYTE[32]);
bool isDot(BYTE[32]);
bool isHiddenEntry(BYTE[32]);
bool isVOLUME_ENTRY(BYTE[32]);
#pragma pack(push,1)

class IndexBlock
{
public:
	// Index Block Header
	DWORD		Indx;
	WORD		OffsetOfUpdateSeq;		
	WORD		SizeOfUpdateSeq;
	ULONGLONG	LogFileSeqNum;	
	ULONGLONG	VCN;		
	// Index Header
	DWORD		EntryOffset;	
	DWORD		TotalEntrySize;	
	DWORD		AllocEntrySize;	
	BYTE		NotLeaf;		
	BYTE		Padding[3];		
};

class MftEntryHeader
{
public:
	DWORD       magic; //0x0 – 0x03
	WORD        updateOffset; //0x04 – 0x05
	WORD        updateNumber;//0x06 – 0x07
	LONGLONG    logFile;//0x08 – 0x0F
	WORD        sequenceNumber; //0x10 – 0x11
	WORD        hardLinkCount; //0x12 – 0x13
	WORD        AttributeOffset;//0x14 – 0x15 
	WORD        flag;// 0x16 – 0x17
	DWORD       usedSize; //0x18 – 0x1B
	DWORD       allocatedSize;// 0x1C – 0x1F
	LONGLONG    baseRecord; //0x20 – 0x27
	WORD        nextAttributeID;//0x28 – 0x29
	BYTE        unsed[2];//0x2A – 0x2B
	DWORD       MFTRecordIndex;//2c -2f
	WORD		updateSequenceNumber;
	WORD		updateSequenceArray[1];
};

//header of MTF Attribute
class MftHeaderAttribute
{
public:
	DWORD  TypeCode;
	DWORD  RecordLength;
	UCHAR  FormCode;   //is non-resident or not
	UCHAR  NameLength;
	USHORT NameOffset;
	USHORT Flags;
	USHORT AttributeID;
	union
	{
		// Resident
		struct
		{
			ULONG  Length;
			USHORT Offset;
			UCHAR  ResidentFlags;
			UCHAR  Reserved;
		} Res;
		// Non-Resident
		struct
		{
			LONGLONG LowestVcn;
			LONGLONG HighestVcn;
			USHORT   MappingPairsOffset;
			USHORT   CompressionUnit;
			UCHAR    Reserved[4];
			ULONGLONG AllocatedLength;
			ULONGLONG FileSize;
			ULONGLONG ValidDataLength;
		} N_Res;
	};
};

class MftDatarun
{
public:
	LONGLONG    offset;
	ULONGLONG   length;
};

class MftAttrStandardInfo
{
public:
	ULONGLONG	CreateTime;
	ULONGLONG	AlterTime;
	ULONGLONG	MFTTime;
	ULONGLONG	ReadTime;
	DWORD		Permission;
	DWORD		MaxVersionNo;
	DWORD		VersionNo;
	DWORD		ClassId;
	DWORD		OwnerId;
	DWORD		SecurityId;
	ULONGLONG	QuotaCharged;
	ULONGLONG	USN;
};

class MftAttrFilename
{
public:
	struct 
	{
		ULONGLONG RecordNum : 48;
		ULONGLONG SeqNum : 16;
	} ParentDir;					
	ULONGLONG	CreateTime;			
	ULONGLONG	ChangeTime;			
	ULONGLONG	MFTTime;			
	ULONGLONG	AccessTime;			
	ULONGLONG	AllocatedSize;		
	ULONGLONG	DataSize;			
	ULONG		FileAttributes;		
	ULONG		ReparseTag;
	UCHAR		NameLength;			
	UCHAR		NameType;			
	WCHAR		Name[1];		
};

class MftAttrIndexRoot
{
public:
	// Index Root Header
	DWORD		AttrType;
	DWORD		CollRule;
	DWORD		IBSize;
	BYTE		ClusPerIB;
	BYTE		Padding1[3];
	// Index Node Header
	DWORD		EntryOffset;
	DWORD		TotalEntrySize;
	DWORD		AllocEntrySize;
	BYTE		Flags;
	BYTE		Padding2[3];
};

class MftIndexEntry
{
public:
	union
	{
		ULONGLONG FileReference;
		struct
		{
			USHORT DataOffset;
			USHORT DataLength;
			ULONG32 ReservedForZero;
		};
	};
	USHORT Length;
	USHORT AttributeLength;
	USHORT Flags;
	USHORT Reserved;
	union {
		union {
			struct {
				ULONGLONG vcn;
			} asNode;
			struct {
				ULONG32 ReparseTag;
				ULONGLONG FileReference;
			} asKeys;
		} reparse;
		MftAttrFilename FileName;
	};
};

class FileName 
{
public :
	MftAttrFilename fName;
	wstring NameEntry;

	void read(BYTE sec[]);
	ULONGLONG getSize() { return fName.DataSize; };
	DWORD getFileAttr() { return fName.FileAttributes;}
	string getState();
	wstring getName() { return NameEntry; };
	FileName clone() { return *this; };
};

class Attribute
{
protected:
	MftHeaderAttribute headAttr;
public:
	virtual void read(BYTE[]) { }
};

class StandardInfoAttri :public Attribute 
{
	MftAttrStandardInfo standInfo;
public:
	void read(BYTE sec[]) {

		readFile((char*)&headAttr, sec, sizeof(MftHeaderAttribute));
		readFile((char*)&standInfo, sec+ 24, 48);
	}
};

class FilenameAttri :public Attribute, public FileName
{
public:
	void read(BYTE sec[]) 
	{
		readFile((char*)&headAttr, sec, (24));
		FileName::read(sec+24);
	}
};

class BlockFile 
{
public:
	FileName fname;
	DWORD curIndex;
};

class IndexRootAttri : public Attribute 
{
	MftAttrIndexRoot indexRoot;
	vector<BlockFile> list;
public:
	void read(BYTE sec[]);
	vector<BlockFile> getListIndexEntry() { return list; }
}; 

vector<MftDatarun> readDataRun(BYTE datarun[], int size);

class IndexAllocAttri : public Attribute 
{
	vector<MftDatarun> list;
public:
	void read(BYTE sec[]);
	vector<MftDatarun> getDatRun() { return list; };
};

class DATA_Attribute : public Attribute 
{
public:
	bool isDataRun;
	vector<MftDatarun> _list;
	string Data;

	void read(BYTE sec[]);
	vector<MftDatarun> getDataRun() { return _list; };
};

class MFT
{
	MftEntryHeader *Mft_head;
	vector <Attribute*> listAttribute;
public:

	void read(BYTE sec[]);
	vector<MftDatarun> getDataRun_INDEX();
	vector<MftDatarun> getDataRun_DATA();
	vector<BlockFile> getIndexEntrys();
	DATA_Attribute* getDataAttr();
	MFT() { Mft_head = new MftEntryHeader; };
	~MFT();
};

//Master file table entry index definition
#define ROOT_FILE_NAME_INDEX			(5)

#define $STANDARD_INFORMATION			(0x10)
#define $FILE_NAME						(0x30)
#define $DATA							(0x80)
#define $INDEX_ROOT						(0x90)
#define $INDEX_ALLOCATION				(0xA0)
#define $END							(0xFFFFFFFF)

#define RESIDENT						0x00
#define NON_RESIDENT					0x01

#define	ATTR_FILENAME_FLAG_READONLY		0x00000001
#define	ATTR_FILENAME_FLAG_HIDDEN		0x00000002
#define	ATTR_FILENAME_FLAG_SYSTEM		0x00000004
#define	ATTR_FILENAME_FLAG_ARCHIVE		0x00000020
#define	ATTR_FILENAME_FLAG_DEVICE		0x00000040
#define	ATTR_FILENAME_FLAG_NORMAL		0x00000080
#define	ATTR_FILENAME_FLAG_ENTRYECTORY	0x10000000

#define	ATTR_INDEXROOT_FLAG_SMALL	0x00	
#define	ATTR_INDEXROOT_FLAG_LARGE	0x01	
#define	INDEX_ENTRY_FLAG_SUBNODE	0x01	