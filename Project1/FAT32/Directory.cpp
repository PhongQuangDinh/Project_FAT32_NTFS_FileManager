#include "Directory.h"

void directory::readmainDirectory(BYTE sec[32])
{
	mEntry.read(sec);
}

void directory::pushsubDirectory(subDirectory s)
{
	sEntry.push_back(s);
}

wstring directory::getLongName()
{
	if (sEntry.size() == 0)
	{
		string a = mEntry.getName();
		wstring s(a.begin(), a.end());
		return s;
	}
	wstring s;
	size_t size = sEntry.size();
	for (int i = 0; i < size; i++)
		s += sEntry[i].getFullName();
	return s;
}

void directory::printDir(int x,FAT32 bst)
{
	cout << setw(x) << " " << string(40, '-') << endl;
	wcout << setw(x) << " " << L"Name: " << getLongName() << endl;
	cout << setw(x) << " " << "+ Type: " << mEntry.getAttribute() << endl;
	cout << setw(x) << " " << "+ Size(Byte): " << mEntry.getSizeFile() << endl;
	cout << setw(x) << " " << "+ Time Create: " << mEntry.getCreateTime() << "-" << mEntry.getCreateDate() << endl;
	cout << setw(x) << " " << "+ Date Access: " << mEntry.getLastAccessDate() << endl;
	cout << setw(x) << " " << "+ Time Modified: " << mEntry.getModifiedTime() << "-" << mEntry.getModifiedDate() << endl;
	cout << setw(x) << " " << "+ First Cluster: " << this->getFirstCluster() << endl;
	cout << setw(x) << " " << "-->Sectors: ";
	for (int i = bst.getFstRDET() + (this->getFirstCluster() - 2) * bst.getSecPerClus(); i < bst.getFstRDET() + (this->getFirstCluster() - 1) * bst.getSecPerClus();i++) {
		cout << i << ", ";
	}
	cout << endl;
	cout << setw(x) << " " << string(40, '-') << endl;
}

DWORD directory::getFirstCluster()
{
	return mEntry.getFirstCluster();
}

DWORD directory::getSize()
{
	return mEntry.getSizeFile();
}

void file::printTree(int x)
{
	wcout << setw(x) << char(195) << " " << getLongName();
	cout << DataSizeFomat(mEntry.getSizeFile()) << endl;
}

directory* file::SearchEntry(wstring name)
{
	if (IsEquals(name, this->getLongName()))
	{
		return this;
	}
	return nullptr;
}

void mainDirectory::read(BYTE sec[32])
{
	readFile((char*)this, sec, sizeof(mainDirectory));
}

string mainDirectory::getName()
{
	string str = "";
	for (int i = 0; i < 8; i++)
		str += Name[i];
	if ((Attr & 0x20) == 0x20)str += ".";
	for (int i = 8; i < 11; i++)
		str += Name[i];
	str.erase(std::remove(str.begin(), str.end(), ' '), str.end());

	return str;
}

string mainDirectory::getAttribute()
{
	string buf = "";
	if (((Attr >> 0) & 1))
		buf += "ReadOnly ";
	if (((Attr >> 1) & 1))
		buf += "Hidden ";
	if (((Attr >> 2) & 1))
		buf += "System ";
	if (((Attr >> 3) & 1))
		buf += "Volume ";
	if (((Attr >> 4) & 1))
		buf += "Folder ";
	if (((Attr >> 5) & 1))
		buf += "File ";
	if (((Attr >> 6) & 1))
		buf += "Device ";
	return buf;
}

string mainDirectory::getCreateTime()
{
	return DectoTime(CrtTime);
}

string mainDirectory::getCreateDate()
{
	return DectoDate(CrtDate);
}

string mainDirectory::getLastAccessDate()
{
	return DectoDate(LstAccDate);
}

string mainDirectory::getModifiedDate()
{
	return DectoDate(ModDate);
}

string mainDirectory::getModifiedTime()
{
	return DectoTime(ModTime);
}

DWORD mainDirectory::getFirstCluster()
{
	return FstClusHi * 65536 + FstClusLo;
}

void mainDirectory::setName(string Name)
{
	size_t n = Name.size() < 11 ? Name.size() : 11;
	for (int i = 0; i < n; i++)
	{
		this->Name[i] = Name[i];
	}
}

UINT32 mainDirectory::getSizeFile()
{
	return  Size;
}

void subDirectory::read(BYTE sec[32])
{
	readFile((char*)this, sec, sizeof(subDirectory));
}

wstring subDirectory::getFullName()
{
	wstring s;
	UINT d = 0;
	for (int i = 0; i < 5; i++)
	{
		d = LName1[i];
		if (d == 65535 || d == 0) break;
		s += d;
	}
	for (int i = 0; i < 6; i++)
	{
		d = LName2[i];
		if (d == 65535 || d == 0) break;
		s += d;
	}
	for (int i = 0; i < 2; i++)
	{
		d = LName3[i];
		if (d == 65535 || d == 0) break;
		s += d;
	}
	return s;
}



void folder::AddEntry(directory* en)
{
	listEntry.push_back(en);
}

void folder::printDir(int x, FAT32 bst)
{
	cout << setw(x) << " " << string(40, '-') << endl;
	wcout << setw(x) << " " <<"Name: " << getLongName() << endl;
	cout << setw(x) << " " << "+ Type: " << mEntry.getAttribute() << endl;
	cout << setw(x) << " " << "+ Cluster: " << this->getFirstCluster() << endl;
	cout << setw(x) << " " << "-->Sectors: ";
	for (int i = bst.getFstRDET() + (this->getFirstCluster() - 2) * bst.getSecPerClus(); i < bst.getFstRDET() + (this->getFirstCluster() - 1) * bst.getSecPerClus(); i++) {
		cout << i << ", ";
	}
	cout << endl;
	size_t n = listEntry.size();
	for (int i = 0; i < n; i++) {
		listEntry[i]->printDir(x + 5,bst);
	}
}
directory* folder::SearchEntry(wstring name)

{
	if (IsEquals(name, this->getLongName()))
	{
		return this;
	}
	int n = listEntry.size();
	for (int i = 0; i < n; i++)
	{
		directory* en = listEntry[i]->SearchEntry(name);
		if (en != nullptr)
			return en;
	}
	return nullptr;
}

folder::~folder() {
	size_t n = listEntry.size();
	for (int i = 0; i < n; i++)
	{
		delete listEntry[i];
	}
}

root::root(string name, WORD FstClus)
{
	mEntry.setName(name);
	mEntry.FstClusLo = FstClus;
}
