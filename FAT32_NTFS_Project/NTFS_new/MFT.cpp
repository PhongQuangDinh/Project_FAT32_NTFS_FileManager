#include "MFT.h"
void readFile(char* buf, BYTE byte[], int size)
{
	stringstream temp;
	for (int i = 0; i < size; i++)
		temp << byte[i];
	temp.read(buf, size);
}

string Byte_to_Str(BYTE byte[], int size)
{
	string temp = "";
	for (int i = 0; i < size; i++)
		if ((int)byte[i] != 0)
			temp += byte[i];
	return temp;
}

UINT32 Hex_to_dec(string hexSector, bool mode)
{
	long temp = 0;
	if (mode)
	{
		string LitteIndiaHex;
		int n = hexSector.length();
		for (int i = n - 1; i >= 0; i -= 2)
		{
			LitteIndiaHex.push_back(hexSector[int(i - 1)]);
			LitteIndiaHex.push_back(hexSector[i]);
		}
		temp = stoll(LitteIndiaHex, 0, 16);
	}
	else temp = stoll(hexSector, 0, 16);
	return temp;
}

UINT Str_to_Dec(BYTE str[], int size, bool mode)
{
	UINT Dec = 0;
	readFile((char*)&Dec, str, size);
	return Dec;
}

string DataSizeFomat(UINT64 size)
{
	double a = 0;
	if (size < 1024) return "(" + to_string(size) + " B)";
	else if (size < pow(1024, 2))
	{
		a = round(double(size * 100 / 1024)) / 100;
		return "(" + to_string(a).substr(0, 5) + " KB)";
	}
	else if (size < pow(1024, 3))
	{
		a = round(double(size * 100 / pow(1024, 2))) / 100;
		return "(" + to_string(a).substr(0, 5) + " MB)";
	}
	a = round(double(size * 100 / pow(1024, 3))) / 100;
	return "(" + to_string(a).substr(0, 5) + " GB)";
}

bool strEqual(wstring str1, wstring str2)
{
	transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
	transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
	return str1 == str2;
}
void IndexRootAttri::read(BYTE sec[])
{
	readFile((char*)&headAttr, sec, 24);
	readFile((char*)&indexRoot, sec + headAttr.Res.Offset, sizeof(MftAttrIndexRoot));
	DWORD pos = headAttr.Res.Offset + sizeof(MftAttrIndexRoot);
	while (pos + 84 < headAttr.RecordLength)
	{
		MftIndexEntry temp;
		readFile((char*)&temp, sec + pos, 16);
		FileName f;
		f.read(sec + pos + 16);
		BlockFile block{ f,temp.DataOffset };
		list.push_back(block);
		pos += temp.Length;
	}
}

void MFT::read(BYTE sec[])
{
	MftHeaderAttribute headAttr, headAttr2;
	readFile((char*)Mft_head, sec, sizeof(MftEntryHeader));
	if (this->Mft_head->magic != 'ELIF') return;
	int k = Mft_head->updateOffset;
	sec[510] = sec[k + 3];
	sec[511] = sec[k + 4];
	sec[1022] = sec[k + 5];
	sec[1023] = sec[k + 6];

	DWORD curOffset = Mft_head->AttributeOffset;
	Attribute* attr = nullptr;
	while (true) 
	{
		readFile((char*)&headAttr, sec + curOffset, 16);
		attr = nullptr;
		if (headAttr.TypeCode == $FILE_NAME) attr = new FilenameAttri;
		else
			if (headAttr.TypeCode == $INDEX_ROOT) attr = new IndexRootAttri;
			else if (headAttr.TypeCode == $INDEX_ALLOCATION) attr = new IndexAllocAttri;
			else if (headAttr.TypeCode == $DATA) attr = new DATA_Attribute;
		if (attr) 
		{
			attr->read(sec + curOffset);
			listAttribute.push_back(attr);
		}
		curOffset += headAttr.RecordLength;
		if (headAttr.TypeCode == $END || curOffset > 1024) break;
	}
}

vector<MftDatarun> MFT::getDataRun_INDEX()
{
	IndexAllocAttri* ia = nullptr;
	for (int i = 0; i < listAttribute.size(); i++)
	{
		ia = dynamic_cast<IndexAllocAttri*> (listAttribute[i]);
		if (ia != nullptr)
			return ia->getDatRun();
	}
	return vector<MftDatarun>();
}

vector<MftDatarun> MFT::getDataRun_DATA()
{
	vector<MftDatarun> datarun;
	DATA_Attribute* data = nullptr;
	for (int i = 0; i < listAttribute.size(); i++)
	{
		data = dynamic_cast<DATA_Attribute*> (listAttribute[i]);
		if (data != nullptr)
		{
			vector<MftDatarun>  run = data->getDataRun();
			datarun.insert(datarun.end(), run.begin(), run.end());
		}
	}
	return datarun;
}

vector<BlockFile> MFT::getIndexEntrys()
{
	IndexRootAttri* id = nullptr;
	for (int i = 0; i < listAttribute.size(); i++)
	{
		id = dynamic_cast<IndexRootAttri*> (listAttribute[i]);
		if (id != nullptr) return id->getListIndexEntry();
	}
	return vector<BlockFile>();
}

DATA_Attribute* MFT::getDataAttr()
{
	DATA_Attribute* data = nullptr;
	for (int i = 0; i < listAttribute.size(); i++)
	{
		data = dynamic_cast<DATA_Attribute*> (listAttribute[i]);
		if (data != nullptr)
			return data;
	}
	return nullptr;
}


MFT::~MFT()
{
	for (auto attr : listAttribute)
		if (attr != nullptr) delete attr;
	delete Mft_head;
}


vector <MftDatarun> readDataRun(BYTE datarun[], int size)
{
	int i = 0;
	vector <MftDatarun> list;
	while (datarun[i] != 0x00)
	{
		int offset_len = datarun[i] >> 4;
		int length_len = datarun[i] & 0xf;
		ULONGLONG length = 0;
		LONGLONG offset = 0;
		MftDatarun drun;
		readFile((char*)&length, datarun + i + 1, length_len);
		readFile((char*)&offset, datarun + i + 1 + length_len, offset_len);
		drun.length = length;
		drun.offset = offset;
		i += offset_len + length_len + 1;
		list.push_back(drun);
	}
	return list;
}

void IndexAllocAttri::read(BYTE sec[])
{
	readFile((char*)&headAttr, sec, 64);
	USHORT fstData = headAttr.N_Res.MappingPairsOffset;
	DWORD lengRecord = headAttr.RecordLength;
	list = readDataRun(sec + fstData, lengRecord - fstData);
}

void FileName::read(BYTE sec[])
{
	wchar_t* buf = new wchar_t[256];
	readFile((char*)&fName, sec, sizeof(MftAttrFilename));

	WORD length = fName.NameLength;
	readFile((char*)buf, sec + 66, length * 2);
	buf[length] = NULL;
	NameEntry = L"";
	for (int i = 0; i < length; i++)
	{
		if (buf[i] < 32 || buf[i]>128) continue;
		NameEntry += buf[i];
	}
}

string FileName::getState() 
{
	string buf;
	DWORD Attr = fName.FileAttributes;
	if (Attr & ATTR_FILENAME_FLAG_READONLY)
		buf += "ReadOnly ";
	if (Attr & ATTR_FILENAME_FLAG_HIDDEN)
		buf += "Hidden ";
	if (Attr & ATTR_FILENAME_FLAG_SYSTEM)
		buf += "System ";
	if (Attr & ATTR_FILENAME_FLAG_ENTRYECTORY)
		buf += "Folder ";
	if (Attr & ATTR_FILENAME_FLAG_ARCHIVE)
		buf += "File ";
	if (Attr & ATTR_FILENAME_FLAG_DEVICE)
		buf += "Device ";
	return buf;
}

void DATA_Attribute::read(BYTE sec[])
{
	readFile((char*)&headAttr, sec, 24);
	if (headAttr.FormCode == RESIDENT)
	{
		isDataRun = false;
		Data = Byte_to_Str(sec + headAttr.Res.Offset, headAttr.Res.Length);
	}
	else
	{
		isDataRun = true;
		readFile((char*)&headAttr, sec, 64);
		USHORT fstData = headAttr.N_Res.MappingPairsOffset;
		DWORD lengRecord = headAttr.RecordLength;
		_list = readDataRun(sec + fstData, lengRecord - fstData);
	}
}