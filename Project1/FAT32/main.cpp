#include "Disk.h"
int main()
{
    Disk* disk = nullptr;
    disk = new Diskfat;
    wstring disk_name;
    cout << "Name of disk: ";
    wcin >> disk_name;
    disk_name = L"\\\\.\\" + disk_name + L":";
    LPCWSTR drive = disk_name.c_str();
    int status = disk->init(drive);

    disk->printBootSector();

    disk->printTreeFolder();

    int c = 1;
    while (c!= 0)
    {
        cout << "#0 Break #Press number -> Continue: ";
        cin >> c;
        wcin.ignore();
        wstring filename;
        cout << "Enter File/Folder Name: ";
        getline(wcin, filename);
        disk->printDataFile(filename);
    }
    delete disk;
    system("pause");
}