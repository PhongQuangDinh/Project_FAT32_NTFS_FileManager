#include "NTFS.h"
#include <fcntl.h>
void Menu(int& option)
{
	cout << "\nNhap lua chon:" << endl;
	cout << "1. Thong tin Partition Boot Sector " << endl;
	cout << "2. Cay thu muc" << endl;
	cout << "3. Cay thu muc chi tiet" << endl;
	cout << "4. Tim File hoac Folder." << endl;
	cout << "5. Thoat." << endl;
	cout << "Lua chon cua ban:";
	cin >> option;
}
int main()
{
	NTFS ntfs;
	WCHAR  DriveName[] = L"\\\\.\\C:";
	wchar_t temp;
	cout << "Nhap ten o dia: ";
	wcin >> temp;
	DriveName[4] = temp;
	if (!ntfs.Init(DriveName))
	{
		cout << "Khong doc duoc drive";
		return 0;
	}
	int option = 0;
	while (true)
	{
		Menu(option);
		system("cls");
		if (option == 1) ntfs.PrintBootSector();
		else if (option == 2) ntfs.PrintFolderTree();
		else if (option == 3) ntfs.PrintFolderTreeDetail();
		else if (option == 4)
		{
			wstring FileName;
			wcin.ignore();
			cout << "Nhap ten file hoac thu muc (neu la file thi can them duoi vd .txt, .doc): ";
			getline(wcin, FileName);
			ntfs.PrintDataEntry(FileName);
		}
		else break;
	}
	return 0;
}