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

void readFile(char* buf, BYTE byte[], int size);
string DecToDateTime(ULONGLONG time);
string DectoTime(WORD dec);
string DectoDate(WORD dec);
string ByteToStr(BYTE[], int size);

UINT32 HexToDec(string hexSector, bool mode = true);
UINT StrToDec(BYTE[], int size, bool mode = true);
string DataSizeFomat(UINT64 size);

bool IsEquals(wstring str1, wstring str2);
bool isDelete(BYTE[32]);
bool isFolder(BYTE[32]);
bool isFile(BYTE[32]);
bool issubDirectory(BYTE[32]);
bool isDot(BYTE[32]);
bool isHiddenEntry(BYTE[32]);
bool isDisk_ENTRY(BYTE[32]);

