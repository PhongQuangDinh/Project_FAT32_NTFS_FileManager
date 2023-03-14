#include "NTFS.h"
void NTFS_BoostSector::PrintBootSector()
{
	cout << "<----------- Boot Sector NTFS ----------->" << endl;
	cout << "Ten OEM: " << boot_sec.oemID << endl;
	cout << "Bytes per Sector: " << boot_sec.bytePerSec << endl;
	cout << "Sectors per Cluster: " << (int)boot_sec.secPerClus << endl;
	cout << "Sectors per Track: " << (int)boot_sec.secPerTrk << endl;
	cout << "Head amount: " << (int)boot_sec.headNum << endl;
	cout << "Total Sector: " << boot_sec.totSec << endl;
	cout << "First Cluster MTF: " << boot_sec.MFTClus << endl;
	cout << "First Cluster MFTMirror : " << boot_sec.MFTMirrClus << endl;
	cout << "Size MTF Entry: " << getClusPerRecord() << " byte" << endl;
	cout << "Bytes of Index Block:" << boot_sec.clusPerBlock * boot_sec.secPerClus * boot_sec.bytePerSec << endl;
}
NTFS::NTFS()
{
	Device = NULL;
	RootEntry = nullptr;
}
NTFS::~NTFS()
{
	CloseHandle(Device);
	delete RootEntry;
}
bool NTFS::Init(LPCWSTR drive)
{
	BYTE sec[512];
	int status = OpenVolume(drive);
	if (status != 0) return false;

	ReadSector(0, sec);
	BSector.ReadBS(sec);
	BSector.getFileSystem();
	if (BSector.getFileSystem().find("NTFS") == string::npos) return false;

	SecPerClus = BSector.getSecPerClus();
	BytePerSec = BSector.getBytePerSec();
	FstMTF = BSector.getFstMFTSec() * SecPerClus;
	BytePerRecord = BSector.getClusPerRecord();
	RootEntry = new Folder(ROOT_FILE_NAME_INDEX);
	RootEntry->Sector = FstMTF + ROOT_FILE_NAME_INDEX * 2;
	readDirectoryTree(RootEntry);
	return true;
}

int NTFS::OpenVolume(LPCWSTR drive)
{
	Device = CreateFile(drive,    // Drive to open
		GENERIC_READ,           // Access mode
		FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
		NULL,                   // Security Descriptor
		OPEN_EXISTING,          // How to create
		0,                      // File Attributes
		NULL);                  // Handle to template

	if (Device == INVALID_HANDLE_VALUE) return GetLastError();
	else return 0;
}

int NTFS::ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte)
{
	int retCode = 0;
	DWORD bytesRead;
	LONG Hightoffset = readPoint >> 32;
	SetFilePointer(Device, (DWORD)readPoint, &Hightoffset, FILE_BEGIN);
	if (!ReadFile(Device, sector, nByte, &bytesRead, NULL))
	{
		printf("ReadFile: %u\n", GetLastError());
		exit(1);
	}
	else return bytesRead;
}

void NTFS::readDirectoryTree(DriveComponent* folder)
{
	BYTE* byte = new BYTE[2048];
	UINT64 pos = (FstMTF * 512 + 1024 * folder->indexEntry);
	DriveComponent* entry = nullptr;
	FileName fName;
	ReadSector(pos, byte, 1024);

	MFT MftRecord;
	MftRecord.read(byte);
	vector<BlockFile> InsideFolder = MftRecord.getIndexEntrys();
	for (int i = 0; i < InsideFolder.size(); i++)
	{
		if (InsideFolder[i].fname.getFileAttr() & ATTR_FILENAME_FLAG_ENTRYECTORY)
		{
			entry = new Folder(InsideFolder[i].curIndex);
			readDirectoryTree(entry);
		}
		else if (InsideFolder[i].fname.getFileAttr() & ATTR_FILENAME_FLAG_ARCHIVE) entry = new File(InsideFolder[i].curIndex);
		if (entry != nullptr)
		{
			entry->addFileName(InsideFolder[i].fname);
			entry->Sector = FstMTF + InsideFolder[i].curIndex * 2;
			folder->addComponent(entry);
		}
	}
	 
	vector<MftDatarun> run = MftRecord.getDataRun_INDEX();
	INT64 offset = 0;
	BYTE InsideFolder_US[20];

	for (int i = 0; i < run.size(); i++)
	{
		int US = 0;
		offset += run[i].offset;
		pos = offset * SecPerClus * BytePerSec;
		ReadSector(pos, byte, 1024);
		IndexBlock idb;
		readFile((char*)&idb, byte, sizeof IndexBlock);
		readFile((char*)InsideFolder_US, byte + idb.OffsetOfUpdateSeq + 2, (idb.SizeOfUpdateSeq - 1) * 2);
		byte[BytePerSec - 2] = InsideFolder_US[US++];
		byte[BytePerSec - 1] = InsideFolder_US[US++];

		UINT64 pos2 = idb.EntryOffset + 0x18;
		if (idb.Indx != 'XDNI')
		{
			cout << "\nMagic number error!\n";
			exit(1);
		}
		UINT64 total = 0;
		while (total + 66 <= idb.TotalEntrySize)
		{
			if (pos2 > 512)
			{
				pos += 512;
				pos2 -= 512;
				ReadSector(pos, byte, 1024);

				byte[BytePerSec - 2] = InsideFolder_US[US++];
				byte[BytePerSec - 1] = InsideFolder_US[US++];
			}
			MftIndexEntry ie;
			readFile((char*)&ie, byte + pos2, 84);
			if (ie.Flags == 0x02) break;

			fName.read(byte + pos2 + 16);
			DWORD idex = ie.DataOffset;
			entry = nullptr;
			if (fName.getFileAttr() == ATTR_FILENAME_FLAG_ENTRYECTORY)
			{
				entry = new Folder(idex);
				readDirectoryTree(entry);
			}
			else if ((fName.getFileAttr() & ATTR_FILENAME_FLAG_ARCHIVE) != 0) entry = new File(idex);
			if (entry != nullptr)
			{
				entry->addFileName(fName);
				entry->Sector = FstMTF + idex * 2;
				folder->addComponent(entry);
			}
			pos2 += ie.Length;
			total += ie.Length;
		}
	}
	delete[] byte;
}

void NTFS::PrintFolderTree()
{
	cout << "<------------- Cay Thu Muc ------------->\n";
	if (RootEntry) RootEntry->printFolderTree();
}

void NTFS::PrintFolderTreeDetail()
{
	if (!RootEntry) return;
	cout << "<------------- Cay Thu Muc ------------->\n";
	RootEntry->PrintInfoEntry();
}

void NTFS::PrintDataEntry(wstring NameFile)
{
	if (!RootEntry) return;
	DriveComponent* entry = RootEntry->SearchComponent(NameFile);
	if (entry == nullptr)
	{
		cout << "File hoac thu muc khong ton tai!" << endl;
		return;
	}
	if (dynamic_cast <File*>(entry) != nullptr) //For File
	{
		cout << " ---Thong Tin File---" << endl;
		entry->PrintInfoEntry(0);
		BYTE* sec = new BYTE[1024];
		ULONGLONG sizeFile = entry->Name.getSize();
		UINT64 pos = (FstMTF * 512 + 1024 * entry->indexEntry);
		ReadSector(pos, sec, 1024);
		MFT MFT;
		MFT.read(sec);
		DATA_Attribute* dataAtr = MFT.getDataAttr();

		if (NameFile.find(L".txt") == std::string::npos) {
			cout << "Dung phan mem tuong thich de doc noi dung!" << endl;
			return;
		}
		if (dataAtr == nullptr) cout << "File hoac thu muc khong ton tai!";
		else if (dataAtr->isDataRun == false)  cout << dataAtr->Data;
		else
		{
			cout << "---------------------DATA---------------------" << endl;
			vector<MftDatarun> run = MFT.getDataRun_DATA();
			int size = run.size();
			BYTE* byte = new BYTE[BytePerSec];
			ULONGLONG total = 0;
			for (int i = 0; i < size; i++)
			{
				for (int j = 0; j < SecPerClus * run[i].length; j++)
				{
					ReadSector(((run[i].offset) * SecPerClus + j) * BytePerSec, byte, BytePerSec);
					cout << Byte_to_Str(byte, BytePerSec);
					if (total > sizeFile)
					{
						delete[] sec;
						delete[] byte;
						return;
					}
					total += BytePerSec;
				}
			}
			delete[] byte;
		}
		delete[] sec;
	}
	else
	{
		cout << "<---------- Cay thu muc con ---------->\n";
		entry->printFolderTree();
		cout << "<---------- Cay thu muc chi tiet ---------->\n";
		entry->PrintInfoEntry();
	}
}

void DriveComponent::PrintInfoEntry(int x)
{
	wcout << setw(x) << "|" << L"Name: " << Name.getName() << endl;
	cout << setw(x) << "|" << "State: " << Name.getState() << endl;
	cout << setw(x) << "|" << "File size: " << Name.getSize() << " Byte" << endl;
	cout << setw(x) << "|" << "Index entry: " << indexEntry << endl;
	cout << setw(x) << "|" << "Sector: " << Sector << endl;
	cout << setw(x) << "|" << string(40, '-') << endl;
}

void File::printFolderTree(int x)
{
	wstring a = Name.getName();
	wcout << setw(x) << L"|- " << a;
	cout << " " << DataSizeFomat(Name.fName.DataSize) << endl;
}
DriveComponent* File::SearchComponent(wstring name)
{
	if (strEqual(name, Name.getName())) return this;
	return nullptr;
}

void Folder::printFolderTree(int x)
{
	wcout << setw(x) << L">- " << Name.getName() << L":" << endl;
	size_t n = InsideFolder.size();
	for (int i = 0; i < n; i++)
		InsideFolder[i]->printFolderTree(x + 3);
}
void Folder::PrintInfoEntry(int x)
{
	cout << setw(x) << "_" << string(35, '_') << endl;
	wcout << setw(x) << "|" << "Name: " << Name.getName() << endl;
	cout << setw(x) << "|" << "State: " << Name.getState() << endl;
	cout << setw(x) << "|" << "First Index: " << indexEntry << endl;
	cout << setw(x) << "|" << "Sector MFT entry: " << Sector << endl;
	cout << setw(x) << "|" << string(35, '_') << endl;

	size_t n = InsideFolder.size();
	for (int i = 0; i < n; i++)
		InsideFolder[i]->PrintInfoEntry(x + 5);
}

DriveComponent* Folder::SearchComponent(wstring name)
{
	if (strEqual(name, Name.getName())) return this;
	int n = InsideFolder.size();
	for (int i = 0; i < n; i++)
	{
		DriveComponent* en = InsideFolder[i]->SearchComponent(name);
		if (en != nullptr)
			return en;
	}
	return nullptr;
}
Folder::~Folder()
{
	for (auto i : InsideFolder)
		if (i != NULL) delete i;
}