#include "FAT32.h"

void FAT32::Read(BYTE sec[512])
{
    readFile((char*)&BootSector, sec, 512);
}

UINT16 FAT32::getFstFAT()
{
    return BootSector.RsvdSecCnt;
}

UINT16 FAT32::getFstRDET()
{
    return BootSector.RsvdSecCnt + BootSector.NumFATs * BootSector.FATSz32;
}

UINT16 FAT32::getBytePerSec()
{
    return BootSector.BytesPerSec;
}

UINT16 FAT32::getSecPerClus()
{
    return BootSector.SecPerClus;
}

UINT16 FAT32::getClusRoot()
{
    return BootSector.RootClus;
}

void FAT32::printBootSector()
{
    cout << "\n------------------------------------------------------------------------------------------------\n";
    cout << "\t \t\ \t \t \t BOOT SECTOR : " << endl;
    cout << "OEM: " << BootSector.BS_OEMName << endl;
    cout << "FAT Type: " << ByteToStr(BootSector.BS_FilSysType, 8) << endl;
    cout << "Bytes per sector: " << BootSector.BytesPerSec << endl;
    cout << "Sector per cluster (Sc): " << (int)BootSector.SecPerClus << endl;
    cout << "Sector per track: " << (int)BootSector.SecPerTrk << endl;
    cout << "Head numbers: " << BootSector.NumHeads << endl;
    cout << "Sector of BootSector (Sb): " << BootSector.RsvdSecCnt << endl;
    cout << "Number of FAT table (Nf) : " << (int)BootSector.NumFATs << endl;
    cout << "Sector of RDET table (Nr) : " << BootSector.RootEntCnt << endl;
    cout << "Size of volume (Sv): " << BootSector.TotSec32 << endl;
    cout << "Sector per FAT table (Sf): " << BootSector.FATSz32 << endl;
    cout << "First Cluster  RDET: " << BootSector.RootClus << endl;
    cout << "First sector of FAT table: " << BootSector.RsvdSecCnt << endl;
    cout << "First sector of RDET: " << BootSector.RsvdSecCnt + (int)BootSector.NumFATs * BootSector.FATSz32 << endl;
    cout << "First sector of Data: " << BootSector.RsvdSecCnt + (int)BootSector.NumFATs * BootSector.FATSz32 + BootSector.RootEntCnt << endl;
    cout << "\n------------------------------------------------------------------------------------------------\n";
}
