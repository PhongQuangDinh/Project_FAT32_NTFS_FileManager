#pragma once
#include <iostream>
#include <string>
#include <vector>
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
	Element(string name, int first_sector, int level, int size);

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

class Component
{
public:
	string name;
	int first_sector;
	int size;
	vector<Component> subTree;

	Component() { }
	Component(string name, int first_sector, int size)
	{

	}
	void out(string space)
	{
		for (int i = 0; i < subTree.size(); i++)
		{
			cout << "+--" << subTree[i].name;
			if (subTree[i].size > 0) cout << " -ID: " << subTree[i].first_sector;
			else subTree[i].out(space + "  ");
			if (i == subTree.size() - 1) cout << "|  \n";
		}
	}
};