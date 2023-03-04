#pragma once
#include <iostream>
#include <string>
using namespace std;

// Class cua TT/TM
class Element
{
protected:
	string name;
	int first_sector;
	int level;
	int size;
	int type;
public:
	Element();
	Element(string name, int first_sector, int level,int size);

	//Lay kich thuoc cua TT/TM
	int getSize();
	//Lay sector dau tien cua TT/TM dat no lam ID
	int getID();
	//Lay ra loai cua tap tin thu muc
	string getType();
	//in ra ten va ID cua TT/TM
	void getElement();
	//Lay ra ten TT/TM
	string getName();
	//Tab phan cap
	static string tabLevel(int level);
};