#include "NTFS.h"

int main(int argc, char** argv)
{
    // xử lí nhập tên ổ đĩa
    wstring disk_name;
    cout << "Nhap ten o dia: ";
    wcin >> disk_name;
    disk_name = L"\\\\.\\" + disk_name + L":";
    LPCWSTR drive = disk_name.c_str();

    // xử lí đọc
    BYTE* sector = new BYTE[512];
    int flag = ReadSector(drive, 0, sector);

    // nếu đọc thành công
    if (flag == 1)
    {
        // in ra bảng sector vừa đọc
        Print_Sector(sector);

        // Đọc Bios Parameter Block
        Read_BPB(sector, drive);

        // giải phóng bộ nhớ
        delete[] sector;
    }
    else
    {
        cout << "Doc khong thanh cong !!!" << endl;
        return 0;
    }
}