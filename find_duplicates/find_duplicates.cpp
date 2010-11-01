// find_duplicates.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <map>
#include <stdio.h>
#include <windows.h>
#include "crc32.h"

using namespace std;

typedef struct _fileInfo
{
  string name;
  unsigned crc32;
} fileInfo;

typedef pair <int, fileInfo> myPair;
multimap<unsigned, fileInfo> myMap;

const long bufferLen = 1024 * 1024;
BYTE buffer[bufferLen];

unsigned getCrc32(const string& fileName)
{
  // open file
  FILE *file;
  errno_t error = fopen_s(&file, fileName.c_str(), "rb");
  if (error != 0)
  {
    return 0;
  }
  DWORD nRead = 0;
  CCrc32 crc32;

  while ((nRead = fread(buffer, 1, bufferLen, file)) > 0)
  {
    crc32.ProcessCRC((void*)buffer, nRead);
  }
  fclose(file);
  return crc32.m_crc32;
}

void DuplicateFile(const string &fileName)
{
  string fileName1 = fileName;
  string extension = fileName.substr(fileName.length() - 3, 3);
  if (extension == string(".gz"))
  {
    fileName1 = fileName.substr(0, fileName.length() - 3);
  }
  printf("DELETE FROM book_authors WHERE book=(SELECT id FROM book WHERE filename='%s' LIMIT 1);\n", fileName1.c_str());
  printf("DELETE FROM book_genres WHERE book=(SELECT id FROM book WHERE filename='%s' LIMIT 1);\n", fileName1.c_str());
  printf("DELETE FROM book_series WHERE book=(SELECT id FROM book WHERE filename='%s' LIMIT 1);\n", fileName1.c_str());
  printf("DELETE FROM book_translators WHERE book=(SELECT id FROM book WHERE filename='%s' LIMIT 1);\n", fileName1.c_str());
  printf("DELETE FROM book WHERE filename='%s';\n", fileName1.c_str());
  string newFileName = string("..\\duplicate\\") + fileName;
  MoveFile(fileName.c_str(), newFileName.c_str());
}

int _tmain(int argc, _TCHAR* argv[])
{
  fprintf(stderr, "searching for duplicate files\n");

  WIN32_FIND_DATA findData;
  HANDLE hFind = FindFirstFile("*.fb2.*", &findData);
  if (hFind == INVALID_HANDLE_VALUE)
  {
    fprintf(stderr, "Cannot find any .fb2 files\n");
    return 0;
  }

  fileInfo fInfo;
  long num = 1;
  do 
  {
    // search for such element in map
    multimap<unsigned, fileInfo>::iterator myIter;
    myIter = myMap.find(findData.nFileSizeLow);

    if (myIter == myMap.end())
    {
      fileInfo info;
      info.crc32 = 0;
      info.name = string(findData.cFileName);
      myMap.insert(myPair(findData.nFileSizeLow, info));
    }
    else
    {
      // compare crc32 of files
      unsigned crc1;
      if (myIter->second.crc32 == 0)
      {
        myIter->second.crc32 = crc1 = getCrc32(myIter->second.name);
      }
      else
      {
        crc1 = myIter->second.crc32;
      }
      unsigned crc2 = getCrc32(findData.cFileName);

      if (crc1 == crc2)
      {
        // duplicate
        DuplicateFile(findData.cFileName);
      }
      else
      {
        // search for other possible files in the map
        myIter++;
        while ((myIter != myMap.end()) && (myIter->first == findData.nFileSizeLow))
        {
          // compare crc32 of files
          if (myIter->second.crc32 == 0)
          {
            myIter->second.crc32 = crc1 = getCrc32(myIter->second.name);
          }
          else
          {
            crc1 = myIter->second.crc32;
          }

          if (crc1 == crc2)
          {
            // duplicate
            DuplicateFile(findData.cFileName);
            break;
          }
          myIter++;
        }
        if ((myIter == myMap.end()) || (myIter->first != findData.nFileSizeLow))
        {
          // didn't find duplicate, let's add file to map
          fileInfo info;
          info.crc32 = crc2;
          info.name = string(findData.cFileName);
          myMap.insert(myPair(findData.nFileSizeLow, info));
        }
      }
    }
    fprintf(stderr, "Scanned book number %d\r", num++);
  } while (FindNextFile(hFind, &findData));

  FindClose(hFind);

	return 0;
}

