#include "Disk.h"
int v = 0;
Diskfat::Diskfat()
{
    Device = NULL;
    RootENTRY = nullptr;
}
Diskfat::~Diskfat()
{
    CloseHandle(Device);
    if (RootENTRY)
        delete RootENTRY;
}
int Diskfat::init(LPCWSTR drive)
{
    BYTE sec[512];
    int status = this->OpenVolume(drive);
    if (status != 0) return -1;

    this->ReadSector(0, sec);
    BSector.Read(sec);
    if (BSector.getFileSystem().find("FAT32") == string::npos)
        return 0;

    SecPerClus = BSector.getSecPerClus();
    BytePerSec = BSector.getBytePerSec();
    FstFAT1 = BSector.getFstFAT();
    FstRDET = BSector.getFstRDET();
    WORD fst = BSector.getClusRoot();

    string name;
    name = int(drive[4]);
    RootENTRY = new root(name, fst);
    readDirectoryTree(RootENTRY);
    return 1;
}
int Diskfat::OpenVolume(LPCWSTR drive)
{
    Device = CreateFile(drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode

        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (Device == INVALID_HANDLE_VALUE) // Open Error
        return GetLastError();
    else
        return 0;
}

int Diskfat::ReadSector(UINT64 readPoint, BYTE sector[], UINT nByte)
{
    int retCode = 0;
    DWORD bytesRead;
    LONG HightOffset = readPoint >> 32;
    SetFilePointer(Device, (DWORD)readPoint, NULL, FILE_BEGIN);//Set a Point to Read
    if (!ReadFile(Device, sector, nByte, &bytesRead, NULL))
    {
        printf("ReadFile: %u\n", GetLastError());
        exit(1);
    }
    else
    {
        return bytesRead;
    }
}


void Diskfat::readDirectoryTree(directory* Folder)
{
    if (dynamic_cast<folder*>(Folder) == nullptr) return;
    BYTE* sec = new BYTE[BytePerSec];
    UINT32 fstClus = Folder->getFirstCluster();
    vector <UINT32> listSector = this->readListSectorFAT(fstClus);

    int n = listSector.size();

    stack <subDirectory> st;
    for (int i = 0; i < n; i++) {

        UINT32 sectorIndex = listSector[i];
        DWORD posByte = BytePerSec * (sectorIndex);
        this->ReadSector(posByte, sec, BytePerSec);

        for (int k = 0; k < BytePerSec; k += 32)
        {
            if ((int)sec[k + 0] == 0x00 || isDelete(sec + k) || isDot(sec + k)) { continue; }
            if (issubDirectory(sec + k))
            {
                subDirectory l;
                l.read(sec + k);
                st.push(l);
            }
            else
            {

                directory* Entry = nullptr;
                //read folder and file
                if (isFile(sec + k))
                {
                    Entry = new  file;
                }
                else  if (isFolder(sec + k))
                {
                    Entry = new folder;
                }
                else if (Entry == nullptr)
                {
                    while (!st.empty())   st.pop();
                    continue;
                }
                Entry->readmainDirectory(sec + k);

                while (!st.empty())
                {
                    Entry->pushsubDirectory(st.top());
                    st.pop();
                }
                readDirectoryTree(Entry); 
                Folder->AddEntry(Entry);

            }
        }
    }
    delete[] sec;
}



void Diskfat::printBootSector()
{
    if (!RootENTRY) return;
    BSector.printBootSector();
}

void Diskfat::printTreeFolder()
{
    if (!RootENTRY) return;
    cout << "\n------------------------------------------------------------------------------------------------\n";
    cout << "\t \t\ \t \t \t Directory: " << endl;
    RootENTRY->printDir(0,this->BSector);
}
void Diskfat::printDataFile(wstring name)
{
    if (!RootENTRY) return;
    directory* entry = RootENTRY->SearchEntry(name);
    if (entry == nullptr)
    {
        cout << "File Not Found!" << endl;
        return;
    }
    if (dynamic_cast <file*>(entry) != nullptr)
    {
        entry->printDir(0,this->BSector);
        cout << "Data: ";
        transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name.find(L".txt") == std::string::npos) {
            cout << "Use appropriate program to read file" << endl;
            return;
        }

        DWORD fstClus = entry->getFirstCluster();
        DWORD size = entry->getSize();
        if (size == 0) return;
        BYTE* sec = new BYTE[BytePerSec];
        for (int i = 0; i < 8; i++)
        {
            if (i * 512 > size) break;
            UINT64 posByte = (FstRDET + i + (fstClus - 2) * SecPerClus) * BytePerSec;
            this->ReadSector(posByte, sec, BytePerSec);
            cout << ByteToStr(sec, 512);
        }
        cout << endl;
        delete[] sec;
    }
    else
    {
        cout << "\n------------------------------------------------------------------------------------------------\n";
        cout << "\t \t\ \t \t \t Sub Directory Tree: " << endl;
        entry->printDir(0,this->BSector);
    }
}

bool isDelete(BYTE entry[32])
{
    if (entry[0x00] == DELETE_EN) return true;
    return false;
}
bool isFolder(BYTE entry[32])
{
    if ((entry[0x0B] & FOLDER_EN) == FOLDER_EN) return true;
    else return false;
}
bool isFile(BYTE entry[32])
{
    if ((entry[0x0B] & FILE_EN) == FILE_EN) return true;
    else return false;
}
bool issubDirectory(BYTE entry[32])
{
    if ((entry[0x0B] & SUB_EN) == SUB_EN) return true;
    else return false;
}
bool isDot(BYTE entry[32])
{
    if (entry[0x00] == DOT_EN) return true;
    return false;
}
bool isDisk_ENTRY(BYTE entry[32])
{

    if ((entry[0x0B] & 0x08) == 0x08) return true;
    else return false;

}
vector<UINT32> Diskfat::readListSectorFAT(UINT32 FstClus)
{

    vector<UINT32> listClus;
    if (FstClus < 2) return listClus;
    UINT32 d = FstClus;
    BYTE* sec = new BYTE[BytePerSec];
    while (d <= EOC - 7 && d >= 2)
    {

        listClus.push_back(d);
        UINT32 x = FstClus * 4 / BytePerSec;
        UINT32 y = (FstClus * 4) % BytePerSec;
        DWORD posByte = (FstFAT1 + x) * BytePerSec;
        this->ReadSector(posByte, sec, BytePerSec);
        readFile((char*)&d, sec + y, 4);
        FstClus = d;
    }
    delete[] sec;

    vector<UINT32> listSec;
    size_t  size = listClus.size();
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < SecPerClus; j++)
            listSec.push_back(FstRDET + (listClus[i] - 2) * SecPerClus + j);
    }

    return listSec;
}
