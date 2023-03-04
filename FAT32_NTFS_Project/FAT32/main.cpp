#include "FAT32.h"
#include <iostream>
using namespace std;
int main()
{
    // xử lí nhập tên ổ đĩa
    wstring disk_name;
    cout << "Nhap ten o dia: ";
    wcin >> disk_name;
    disk_name = L"\\\\.\\" + disk_name + L":";
    LPCWSTR drive = disk_name.c_str();
    FAT32 fat32(drive);
    fat32.Print_BootSector();
    fat32.Print_RDET();
    fat32.Print_Directory_File_Tree();
    fat32.MENU();
}