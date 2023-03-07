#include "FAT32.h"

FAT32::FAT32()
{
    LPCWSTR drive = L"////.//E:";
    this->BootSector = new BYTE[512];
    this->FAT = new BYTE[512];
    this->Read_Sector(0, this->BootSector, 512);

    this->bytes_per_sector = FAT32::Get_Value_Little_Endian(this->BootSector, 0x0B, 2);
    this->sectors_per_cluster = FAT32::Get_Value_Little_Endian(this->BootSector, 0x0D, 1);
    this->sectors_of_bootsector = FAT32::Get_Value_Little_Endian(this->BootSector, 0x0E, 2);
    this->numbers_of_fats = FAT32::Get_Value_Little_Endian(this->BootSector, 0x10, 1);
    this->sector_of_RDET = (FAT32::Get_Value_Little_Endian(this->BootSector, 0x11, 2) * 32) / bytes_per_sector;
    this->sector_per_FAT = FAT32::Get_Value_Little_Endian(this->BootSector, 0x24, 4);
    this->first_sector_of_data = this->sectors_of_bootsector + this->numbers_of_fats * this->sector_per_FAT + this->sector_of_RDET;
    this->Read_Sector(this->sectors_of_bootsector * 512, this->FAT, 512);
}
FAT32::FAT32(LPCWSTR drive)
{
    this->drive = drive;
    this->BootSector = new BYTE[512];
    this->FAT = new BYTE[512];
    this->Read_Sector(0, this->BootSector, 512);

    this->bytes_per_sector = Get_Value_Little_Endian(this->BootSector, 0x0B, 2);
    this->sectors_per_cluster = Get_Value_Little_Endian(this->BootSector, 0x0D, 1);
    this->sectors_of_bootsector = Get_Value_Little_Endian(this->BootSector, 0x0E, 2);
    this->numbers_of_fats = Get_Value_Little_Endian(this->BootSector, 0x10, 1);
    this->sector_of_RDET = (Get_Value_Little_Endian(this->BootSector, 0x11, 2) * 32) / bytes_per_sector;
    this->sector_per_FAT = Get_Value_Little_Endian(this->BootSector, 0x24, 4);
    this->first_sector_of_data = this->sectors_of_bootsector + this->numbers_of_fats * this->sector_per_FAT + this->sector_of_RDET;
    this->Read_Sector(this->sectors_of_bootsector * 512, this->FAT, 512);
}
FAT32::~FAT32()
{
    delete[] FAT;
    delete[] BootSector;
}
// we are bug here :33
int64_t FAT32::Get_Value_Little_Endian(BYTE* sector, int offset, int number)
{
    int64_t k = 0;
    memcpy(&k, sector + offset, number);
    return k;
}
// Chuyển "number" bytes DATA từ vị trí "offset" thành string
string FAT32::Get_String(BYTE* DATA, int offset, int number)
{
    char* tmp = new char[number + 1];
    memcpy(tmp, DATA + offset, number);
    string s = "";
    for (int i = 0; i < number; i++)
        if (tmp[i] != 0x00 && tmp[i] != 0xFF)
            s += tmp[i];
    return s;
}
int FAT32::Read_Sector(int readPoint, BYTE*& sector, int size)
{
    int retCode = 0;
    DWORD bytesRead;
    HANDLE device = NULL;

    device = CreateFileW(this->drive,    // Drive to open
        GENERIC_READ,           // Access mode
        FILE_SHARE_READ | FILE_SHARE_WRITE,        // Share Mode
        NULL,                   // Security Descriptor
        OPEN_EXISTING,          // How to create
        0,                      // File attributes
        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE) // Open Error
    {
        cout << "CreateFile : " << GetLastError() << endl;
        cout << endl;
        return 0;
    }

    SetFilePointer(device, readPoint, NULL, FILE_BEGIN);//Set a Point to Read

    if (!ReadFile(device, sector, size, &bytesRead, NULL))
    {
        cout << "Read_File_Content : " << GetLastError() << endl;
        return 0;
    }
    else
    {
        // cout << "Success !!!" << endl;
        return 1;
    }
}

//In thong tin sector
void FAT32::Print_Sector(BYTE* sector)
{
    int count = 0;
    int num = 0;

    cout << "         0  1  2  3  4  5  6  7    8  9  A  B  C  D  E  F" << endl;

    cout << "0x0" << num << "0  ";
    bool flag = 0;
    for (int i = 0; i < 512; i++)
    {
        count++;
        if (i % 8 == 0)
            cout << "  ";
        printf("%02X ", sector[i]);
        if (i == 255)
        {
            flag = 1;
            num = 0;
        }

        if (i == 511) break;
        if (count == 16)
        {
            int index = i;

            cout << endl;

            if (flag == 0)
            {
                num++;
                if (num < 10)
                    cout << "0x0" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x0" << hex << "0  ";
                }

            }
            else
            {
                if (num < 10)
                    cout << "0x1" << num << "0  ";
                else
                {
                    char hex = char(num - 10 + 'A');
                    cout << "0x1" << hex << "0  ";
                }
                num++;
            }

            count = 0;
        }
    }
    cout << endl;
}
void FAT32::Read_RDET(int sector_index, int level)
{
    BYTE* RDET = new BYTE[512];
    this->Read_Sector(sector_index * 512, RDET, 512);
    //this->Print_Sector(RDET);
    string file_name = "";
    int pointer = 2 * 32;
    do {
        if (pointer == 512)
        {
            pointer = 0;
            sector_index += 1;
            this->Read_Sector(sector_index * 512, RDET, 512);
        }
        if (FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x00)
            break;
        // Neu la entry phu
        else if (FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x0F)
            file_name = Get_String(RDET, pointer + 1, 10) + Get_String(RDET, pointer + 0x0E, 12) + Get_String(RDET, pointer + 0x1C, 4) + file_name;
        //Neu la entry chinh
        else if (FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x10 || FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x20)
        {
            //cout << "entry" << endl;
            unsigned int first_cluster = Get_Value_Little_Endian(RDET, pointer + 0x1A, 2); // cluster bat dau
            //unsigned int first_cluster = Get_Double_Value_Little_Endian(RDET, pointer + 0x1A,pointer + 0x14, 2);
            //cout << first_cluster << endl;
            unsigned int last_cluster = first_cluster; // cluster ket thuc
            while (true)
                //cout << Get_Value_Little_Endian(FAT, last_cluster * 4, 4) << endl;
                if (last_cluster >= 127) {
                    break;
                }
                else if (Get_Value_Little_Endian(FAT, last_cluster * 4, 4) == 0x0FFFFFFF || Get_Value_Little_Endian(FAT, last_cluster * 4, 4) == 0x0FFFFFF8 || last_cluster == 0)
                    break;
                else  if (Get_Value_Little_Endian(FAT, last_cluster * 4, 4) == 0x0FFFFFF7 || Get_Value_Little_Endian(FAT, last_cluster * 4, 4) == 0)
                {
                    cout << "Can read FAT table !!!" << endl;
                    break;
                }
                else
                {
                    last_cluster = Get_Value_Little_Endian(FAT, last_cluster * 4, 4);
                }

            if (FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x10)
            {
                if (file_name == "")
                    file_name = Get_String(RDET, pointer, 8);
                Element element(file_name, (first_cluster - 2) * sectors_per_cluster + first_sector_of_data, level, 0);
                this->list_file.push_back(element);
                cout << file_name << endl;
                cout << "\t+ Type : Folder" << endl;
                cout << "\t+ first cluster : " << first_cluster << endl;
                cout << "\tclusters :";
                for (int i = first_cluster; i <= last_cluster; i++)
                {
                    cout << i;
                    if (i != last_cluster) cout << ", ";
                }
                cout << endl;
                cout << "\t--> sectors : ";
                for (int i = (first_cluster - 2) * sectors_per_cluster + first_sector_of_data; i <= (last_cluster - 1) * sectors_per_cluster + first_sector_of_data; i++)
                {
                    cout << i;
                    if (i != (last_cluster - 1) * sectors_per_cluster + first_sector_of_data) cout << ", ";
                }
                file_name = "";
                cout << endl;

                if (Get_Value_Little_Endian(RDET, pointer + 28, 4) == 0)
                    this->Read_RDET((first_cluster - 2) * this->sectors_per_cluster + this->first_sector_of_data, level + 1);

            }
            else  if (FAT32::Get_Value_Little_Endian(RDET, pointer + 0x0B, 1) == 0x20)
            {
                if (file_name == "")
                    file_name = Get_String(RDET, pointer, 8) + "." + Get_String(RDET, pointer + 8, 3);
                Element element(file_name, (first_cluster - 2) * sectors_per_cluster + first_sector_of_data, level, FAT32::Get_Value_Little_Endian(RDET, pointer + 28, 4));
                this->list_file.push_back(element);
                cout << file_name << endl;
                cout << "\t+ Type : File" << endl;
                cout << "\t+ Size of file : " << FAT32::Get_Value_Little_Endian(RDET, pointer + 28, 4) << endl;
                cout << "\t+ first cluster : " << first_cluster << endl;
                cout << "\tclusters :";
                for (int i = first_cluster; i <= last_cluster; i++)
                {
                    cout << i;
                    if (i != last_cluster) cout << ", ";
                }
                cout << endl;
                cout << "\t--> sectors : ";
                for (int i = (first_cluster - 2) * sectors_per_cluster + first_sector_of_data; i <= (last_cluster - 1) * sectors_per_cluster + first_sector_of_data; i++)
                {
                    cout << i;
                    if (i != (last_cluster - 1) * sectors_per_cluster + first_sector_of_data) cout << ", ";
                }
                file_name = "";
                cout << endl;
            }
        }
        else
            file_name = "";
        pointer += 32;
        //cout << "end" << endl; 
        //cout << pointer << endl;
    } while (true);
}
void FAT32::Print_RDET()
{
    cout << "\n------------------------------------------------------------------------------------------------\n";
    cout << "\t \t\ \t \t \t \t RDET : " << endl;
    this->Read_RDET(this->first_sector_of_data, 0);
}

void FAT32::Print_Directory_File_Tree()
{
    cout << "\n------------------------------------------------------------------------------------------------\n";

    cout << "\t \t\ \t \t \t \t CAY THU MUC : \n\n";
    for (int i = 0; i < this->list_file.size(); i++)
        this->list_file[i].getElement();
}
// Doc data cua sector
string FAT32::Read_Sector_Data(int index)
{
    string res;
    BYTE* DATA = new BYTE[512];
    this->Read_Sector(index * 512, DATA, 512);
    res = Get_String(DATA, 0x00, 512);
    return res;
}
// Doc data cua file .txt
void FAT32::Read_File_Content(int ID)
{
    for (int i = 0; i < this->list_file.size(); i++)
        if (this->list_file[i].getID() == ID)
        {
            if (this->list_file[i].getType() == ".txt" || this->list_file[i].getType() == ".TXT")
            {
                int size = this->list_file[i].getSize(), index = ID;
                BYTE* data = new BYTE[size];
                cout << this->list_file[i].getName() << endl;
                cout << "- Content : ";
                while (size > 0)
                {
                    cout << this->Read_Sector_Data(index);
                    size -= 512;
                    index += 1;
                }
                cout << endl;
                return;
            }
            else
            {
                cout << this->list_file[i].getType() << endl;
                cout << "Can open this file!!!" << endl;
                return;
            }
        }
    cout << "ID not found!!" << endl;

}

void FAT32::MENU()
{
    int ID;
    char check = 'n';
    cout << "\n--------------------------------------------------------------------------------------------------\n";
    cout << "\t \t \t \t \t \t \tMENU\n";
    cout << "Ban muon mo file?(y/n)";
    cin >> check;
    while (check == 'y')
    {
        cout << "ID file : ";
        cin >> ID;
        this->Read_File_Content(ID);
        cout << "\n----------------------------------------------------------------------------------------------\n";
        cout << "Ban muon mo file khac ?(y/n)";
        cin >> check;
    }

}
void FAT32::Print_BootSector()
{
    cout << "\n------------------------------------------------------------------------------------------------\n";
    cout << "\t \t\ \t \t \t \t BOOT SECTOR : " << endl;
    cout << "Type of File System: FAT32" << endl;
    cout << "Bytes per sector: " << this->bytes_per_sector << endl;
    cout << "Sector per cluster: " << this->sectors_per_cluster << endl;
    cout << "Sector of BootSector: " << this->sectors_of_bootsector << endl;
    cout << "Number of FAT table: " << this->numbers_of_fats << endl;
    cout << "Sector of RDET table: " << this->sector_of_RDET << endl;
    cout << "Sector per FAT table: " << this->sector_per_FAT << endl;
    cout << "First sector of FAT table: " << this->sectors_of_bootsector << endl;
    cout << "First sector of RDET: " << this->sectors_of_bootsector + numbers_of_fats * sector_per_FAT << endl;
    cout << "First sector of Data: " << this->sectors_of_bootsector + numbers_of_fats * sector_per_FAT + sector_of_RDET << endl;
    cout << endl;
}
