#include "Element.h"

Element::Element()
{
}

int Element::getSize()
{
	return this->size;
}

int Element::getID()
{
	return this->first_sector;
}

string Element::getType()
{
	return this->name.substr(this->name.size() - 4, 4);
}

string Element::getName()
{
	return this->name;
}

void Element::getElement()
{
	cout << Element::tabLevel(this->level) << this->name;
	if (this->size > 0)
		cout << " -ID: " << this->first_sector;
	cout << endl;
}

Element::Element(string name, int first_sector,int level, int size )
{
	this->name = "";
	int n = name.size() -1;
	if (int(name[n]) == -1)
		while (int(name[n]) == -1)
			n--;
	for (int i = 0; i < n+1; i++)
		this->name += name[i];
	this->first_sector = first_sector;
	this->level = level;
	this->size = size;
}

string Element::tabLevel(int level)
{
	string tab = "";
	for (int i = 0; i < level; i++)
		tab += "\t";
	return tab;
}