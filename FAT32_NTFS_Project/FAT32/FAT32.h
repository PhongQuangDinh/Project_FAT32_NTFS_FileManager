#pragma once
#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<vector>
#include<windows.h>
#include<iomanip>
#include<sstream>
#include<map>
#include "Element.h";
using namespace std;

class FAT32
{
private:
    LPCWSTR drive;
    unsigned int bytes_per_sector;
    unsigned int sectors_per_cluster;
    unsigned int sectors_of_bootsector;
    unsigned int numbers_of_fats;
    unsigned int sector_of_RDET;
    unsigned int sector_per_FAT;
    unsigned int first_sector_of_data;

    BYTE* FAT;
    BYTE* BootSector;
    vector<Element> list_file;

    Component Tree_File;
public:
    FAT32();
    FAT32(LPCWSTR drive);
    ~FAT32();

    //Chuyen number byte thanh int (little endian)
    static int64_t Get_Value_Little_Endian(BYTE* sector, int offset, int number);
    //Chuyen number byte thanh string
    static string Get_String(BYTE* DATA, int offset, int number);
    //Doc sector 512 byte luu vao sector
    int Read_Sector(int readPoint, BYTE*& sector, int size);
    //In thong tin sector theo kieu hexa
    static void Print_Sector(BYTE* sector);
    //In thong tin bootsector
    void Print_BootSector();
    //Doc thong tin RDET
    void Read_RDET(int sector_index, int level);
    //In thong tin RDET
    void Print_RDET();
    //In ra cay thu muc
    void Print_Directory_File_Tree();
    //Doc data tu sector thu index va chuyen thanh string
    string Read_Sector_Data(int index);
    //Ham doc file .TXT
    void Read_File_Content(int ID);
    //Ham chon file muon mow
    void MENU();
};