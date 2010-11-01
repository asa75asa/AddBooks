// batch_rename.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


// batch_rename.cpp : Defines the entry point for the console application.
//

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
  int num = 1;
  if (argc > 1)
  {
    // try to get number of the argument
    num = _wtoi(argv[1]);
    if (num <= 0) num = 1;
  }
  // parsing fb2 files
  WIN32_FIND_DATA findData;
  HANDLE hFind = FindFirstFile(L"*.fb2", &findData);
  if (hFind == INVALID_HANDLE_VALUE)
  {
    cout << L"Cannot find any .fb2 files" << endl;
  }
  else
  {
    TCHAR fName[MAX_PATH];
    do
    {
      wsprintf(fName, L"1\\%06d.fb2", num++);
      MoveFile(findData.cFileName, fName);
      cout << L"Moved file: " << findData.cFileName << L" to: " << fName << endl;
    } while (FindNextFile(hFind, &findData));
  }
  return 0;
}


