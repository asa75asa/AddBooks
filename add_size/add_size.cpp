// add_size.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#define _CRT_SECURE_NO_WARNINGS


/* Standard C++ includes */
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <mysql_connection.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
  sql::Driver *driver;
  sql::Connection *con;
  sql::ResultSet *res = NULL;
  sql::Statement *stmt = NULL;

  try 
  {
    /* Create a database connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "lib", "library");
    /* Connect to the MySQL test database */
    con->setSchema("library");
    stmt = con->createStatement();
    res = stmt->executeQuery("SET NAMES 'cp1251'");
  } 
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
  }

  // parsing fb2 files
  WIN32_FIND_DATA findData;
  HANDLE hFind = FindFirstFile("*.fb2", &findData);
  if (hFind == INVALID_HANDLE_VALUE)
  {
    cout << "Cannot find any .fb2 files" << endl;
  }
  else
  {
    int i = 1;
    do 
    {
      string fName(findData.cFileName);
      DWORD size = findData.nFileSizeLow;
      char query[10240];
      sprintf_s(query, "UPDATE library.book set size=%d where filename='%s'", size, fName.c_str());
      try
      {
        res = stmt->executeQuery(query);
      }
      catch (sql::SQLException &e)
      {
        if (e.getErrorCode() != 0)
        {
          cout << "# ERR: SQLException in " << __FILE__;
          cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
          cout << "# ERR: " << e.what();
          cout << " (MySQL error code: " << e.getErrorCode();
          cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        }
      }

      cerr << "processing " << i << " file, file name: " << fName << "\r";
      cout << "processing " << i++ << " file, file name: " << fName << "\r";
    } while (FindNextFile(hFind, &findData));

  }
  FindClose(hFind);

  if (res) delete res;
  if (stmt) delete stmt;
  if (con) delete con;

  return EXIT_SUCCESS;
}

