#pragma once
#include<iostream>
#include<cstdlib>
#include<stdio.h>
#include<vector>
#include<windows.h>
#include<iomanip>
#include<sstream>
#include<map>
#include<string>
using namespace std;

// Đọc sector, trả về trạng thái : 1 - đọc thành công, 0 - đọc thất bại
int ReadSector(LPCWSTR drive, int readPoint, BYTE*& sector);

void ReadSect2(LPCWSTR disk, BYTE*& DATA, unsigned int _nsect);

// Lấy "number" bytes từ vị trí "offset"
int64_t Get_Bytes(BYTE* sector, int offset, int number);

// Đọc Bios Parameter Block
void Read_BPB(BYTE* sector, LPCWSTR disk);

// Chuyển "number" bytes DATA từ vị trí "offset" thành string
string toString(BYTE* DATA, int offset, int number);

//Chuyển hệ 10 sang hệ 2
string toBinary(int n);

//Đọc thông tin của entry $INFORMATION
int Read_Entry_INFORMATION(BYTE* Entry, int start);

// Đọc thông tin của Attribute $FILE_NAME
int Read_Entry_FILE_NAME(BYTE* Entry, int start, int ID);

// Đọc thông tin của Attribute $DATA
void Read_Entry_DATA(BYTE* Entry, int start);

// Hàm in "tab" lần /t
void print_Tab(int tab);

// lấy tên file trong mảng nameFile có ID file là "id"
string get_nameFile(int id);

// Hàm đệ quy in ra cây thư mục
void Print_Folder_Tree(int a, int tab, int vt);

// Đọc Bios Parameter Block
void Read_BPB(BYTE* sector, LPCWSTR disk);

// Đọc $MFT Entry
void read_MFT(unsigned int MFTStart, unsigned int sectors_per_cluster, LPCWSTR disk);

// xử lí cây thư mục
void folder_Tree(unsigned int len_MFT, unsigned int MFTStart, LPCWSTR disk);

// In bảng ra sector
void Print_Sector(BYTE* sector);