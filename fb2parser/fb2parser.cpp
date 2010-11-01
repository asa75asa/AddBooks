/*
examples/standalone_example_docs1.cpp
*/

#define _CRT_SECURE_NO_WARNINGS


/* Standard C++ includes */
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#define TIXML_USE_STL
#include <tinyxml.h>
#include <mysql.h>

/*
Include directly the different
headers from cppconn/ and mysql_driver.h + mysql_util.h
(and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "MyFb2File.h"

using namespace std;

char globalQuery[globalLen];
char globalAnsiStr[globalLen];
WCHAR globalWcharStr[globalLen];
sql::Statement *stmt = NULL;

bool ParseFb2File(const string& fileName,
                  author_array &authors,
                  author_array &translators,
                  string_array &genres,
                  string &bookName,
                  string &annotation,
                  string &lang,
                  string &langOrig,
                  string &serieName,
                  int &serieNum)
{
  bool retVal = false;
  MyFb2File doc;
  if (doc.Open(fileName) != MyFb2File::SUCCESS)
  {
    return retVal;
  }
  if ((doc.GetEncoding() == "utf8") ||(doc.GetEncoding() == "UTF8") || (doc.GetEncoding() == "utf-8") ||(doc.GetEncoding() == "UTF-8"))
  {
    doc.m_useUtf8 = true;
  }
  else
  {
    doc.m_useUtf8 = false;
  }

  if (!doc.GetAuthors(authors))
  {
    string pS = "Автор неизвестен";
    author a;
    a.surname = pS;
    authors.clear();
    authors.push_back(a);
  }
  else
  {
    retVal = true;
  }

  if (!doc.GetTranslators(translators))
  {
    translators.clear();
  }
  else
  {
    retVal = true;
  }

  if (!doc.GetGenres(genres))
  {
    string g = "empty";
    genres.clear();
    genres.push_back(g);
  }
  else
  {
    retVal = true;
  }

  bookName = doc.GetTitle();
  annotation = doc.GetAnnotation();
  lang = doc.GetLanguage();
  langOrig = doc.GetLanguageOrig();
  if (!doc.GetSerie(serieName, serieNum))
  {
    serieName.clear();
    serieNum = 0;
  }
  else
  {
    retVal = true;
  }
  if (!retVal && !(bookName.empty() && annotation.empty() && lang.empty() && langOrig.empty()))
  {
    retVal = true;
  }

  return retVal;
}

#if 0
bool AddGenreToDatabase(const struct genre & genreName)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  bool retval = true;
  try
  {
    mysql_escape_string(globalAnsiStr, genreName.name.c_str(), genreName.name.length());
    sprintf(query, "INSERT INTO genre VALUES (%d, '%s')", genreName.id, globalAnsiStr);
    res = stmt->executeQuery(query);
    if (res)
    {
      delete res;
      res = NULL;
    }
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    else
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retval = false;
    }
  }
  return retval;
}
#endif
int GetGenreId(const string &genreName)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal;
  try
  {
    mysql_escape_string(globalAnsiStr, genreName.c_str(), genreName.length());
    sprintf(query, "SELECT id FROM genre WHERE name='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  // find largest id
  try
  {
    sprintf(query, "SELECT max(id) 'max' FROM genre");
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }

  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("max") + 1;
      }
      else
      {
        retVal = -1;
        delete res;
        res = NULL;
        return retVal;
      }
      delete res;
      res = NULL;
    }
  }

  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO genre (id, name) VALUES (%d, '%s')", retVal, globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;
}

#if 0
bool AddAuthorToDatabase(const struct author& authorName)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  bool retval = true;
  char eFirstName[1024];
  char eLastName[1024];
  char eMiddleName[1024];
  char eComment[1024];
  char eLongname[1024];
  try
  {
    mysql_escape_string(eFirstName, authorName.firstname.c_str(), authorName.firstname.length());
    mysql_escape_string(eLastName, authorName.lastname.c_str(), authorName.lastname.length());
    mysql_escape_string(eMiddleName, authorName.middlename.c_str(), authorName.middlename.length());
    mysql_escape_string(eComment, authorName.comment.c_str(), authorName.comment.length());
    mysql_escape_string(eLongname, authorName.longname.c_str(), authorName.longname.length());
    sprintf(query, "INSERT INTO author VALUES (%d, '%s', '%s', '%s', '%s', '%s')", 
      authorName.id, 
      eFirstName,
      eLastName,
      eMiddleName,
      eComment,
      eLongname);
    res = stmt->executeQuery(query);
    if (res)
    {
      delete res;
      res = NULL;
    }
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    else
    {
      cout << "Adding author: " << eLongname << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retval = false;
    }
  }
  return retval;
}

int GetAuthorId(const struct author &authorName)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal;
  try
  {
    mysql_escape_string(globalAnsiStr, authorName.longname.c_str(), authorName.longname.length());
    sprintf(query, "SELECT id FROM author WHERE longname='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  // find largest id
  try
  {
    sprintf(query, "SELECT max(id) 'max' FROM author");
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }

  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("max") + 1;
      }
      else
      {
        retVal = -1;
        delete res;
        res = NULL;
        return retVal;
      }
      delete res;
      res = NULL;
    }
  }

  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO author (id, firstname, surname, middlename, longname, comment) VALUES (%d, '%s', '%s', '%s', '%s', '%s')", 
      retVal, 
      authorName.firstname.c_str(),
      authorName.lastname.c_str(),
      authorName.middlename.c_str(),
      authorName.longname.c_str(),
      authorName.comment.c_str());
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;
}

bool AddGenresToDatabase()
{
  sql::ResultSet *res = NULL;
  char query[10240];
  bool retval = true;
  vector<struct genre>::iterator theIterator;
  for (theIterator = genres.begin(); theIterator != genres.end(); theIterator++)
  {
    char eGenre[1024];
    try
    {
      mysql_escape_string(eGenre, (*theIterator).name.c_str(), (*theIterator).name.length());
      sprintf(query, "INSERT INTO genre VALUES (%d, '%s')", (*theIterator).id, eGenre);
      res = stmt->executeQuery(query);
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() == 0)
      {
        if (res)
        {
          delete res;
          res = NULL;
        }
        continue;
      }
      cout << "Adding genre: " << eGenre << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retval = false;
      continue;
    }
  }
  return retval;

}

bool AddAuthorsToDatabase()
{
  sql::ResultSet *res = NULL;
  char query[10240];
  bool retval = true;
  vector<struct author>::iterator theIterator;
  for (theIterator = authors.begin(); theIterator != authors.end(); theIterator++)
  {
    char eFirstName[1024];
    char eLastName[1024];
    char eMiddleName[1024];
    char eComment[1024];
    char eLongname[1024];
    try
    {
      mysql_escape_string(eFirstName, (*theIterator).firstname.c_str(), (*theIterator).firstname.length());
      mysql_escape_string(eLastName, (*theIterator).lastname.c_str(), (*theIterator).lastname.length());
      mysql_escape_string(eMiddleName, (*theIterator).middlename.c_str(), (*theIterator).middlename.length());
      mysql_escape_string(eComment, (*theIterator).comment.c_str(), (*theIterator).comment.length());
      mysql_escape_string(eLongname, (*theIterator).longname.c_str(), (*theIterator).longname.length());
      sprintf(query, "INSERT INTO author VALUES (%d, '%s', '%s', '%s', '%s', '%s')", 
        (*theIterator).id, 
        eFirstName,
        eLastName,
        eMiddleName,
        eComment,
        eLongname);
      res = stmt->executeQuery(query);
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() == 0)
      {
        if (res)
        {
          delete res;
          res = NULL;
        }
        continue;
      }
      cout << "Adding author: " << eLongname << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retval = false;
      continue;
    }
  }
  return retval;
}

void CleanDatabase()
{
  sql::ResultSet *res = NULL;
  try
  {
    res = stmt->executeQuery("DELETE FROM book WHERE TRUE");
    if (res)
    {
      delete res;
      res = NULL;
    }
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    else
    {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
  }
  try
  {
    res = stmt->executeQuery("DELETE FROM author WHERE TRUE");
    if (res)
    {
      delete res;
      res = NULL;
    }
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    else
    {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
  }
  try
  {
    res = stmt->executeQuery("DELETE FROM genre WHERE TRUE");
    if (res)
    {
      delete res;
      res = NULL;
    }
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res)
      {
        delete res;
        res = NULL;
      }
    }
    else
    {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
  }

}
#endif

int AddBookToDatabase(const string &fName, 
                       const string &bookName, 
                       const string &annotation, 
                       DWORD size,
                       int langId,
                       int langOrigId)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal = -1;

  // check if such book exists
  try
  {
    mysql_escape_string(globalAnsiStr, fName.c_str(), fName.length());
    sprintf(query, "SELECT id FROM book WHERE filename='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      delete res;
      res = NULL;
      if (retVal > 0)
      {
        return 0;
      }
    }
  }


  static int maxId = 0;
  // find largest id
  if (maxId == 0)
  {
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM book");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = false;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = false;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }

  // add book
  try
  {
    char eBookName[10240];
    retVal = maxId;
    mysql_escape_string(eBookName, bookName.c_str(), bookName.length());
    mysql_escape_string(globalAnsiStr, annotation.c_str(), annotation.length());
    sprintf(globalQuery, "INSERT INTO book (id, lang, lang_orig, name, filename, size, description) VALUES (%d, %d, %d, '%s', '%s', %d, '%s')", 
      maxId++, langId, langOrigId, eBookName, fName.c_str(), size, globalAnsiStr);
    res = stmt->executeQuery(globalQuery);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() == 0)
    {
      if (res) delete res;
      return retVal;
    }
    cout << "Adding book: " << fName << endl;
    cout << "# ERR: SQLException in " << __FILE__;
    cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
    cout << "# ERR: " << e.what();
    cout << " (MySQL error code: " << e.getErrorCode();
    cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    return -1;
  }
  if (res) delete res;
  return retVal;
}


int AddLangToDatabase(const string &lang)
{
  if (lang.empty()) return 0;
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal;
  try
  {
    mysql_escape_string(globalAnsiStr, lang.c_str(), lang.length());
    sprintf(query, "SELECT id FROM lang WHERE name='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  static int maxId = 0;
  if (maxId == 0)
  {
    // find largest id
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM lang");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = -1;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = -1;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }
  retVal = maxId;

  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO lang (id, name) VALUES (%d, '%s')", 
      maxId++, 
      globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;
}

int AddAuthorToDatabase(author &a)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  char eFirstName[10240];
  char eMiddleName[10240];
  int retVal = -1;
  try
  {
    mysql_escape_string(globalAnsiStr, a.surname.c_str(), a.surname.length());
    mysql_escape_string(eFirstName, a.firstname.c_str(), a.firstname.length());
    mysql_escape_string(eMiddleName, a.middlename.c_str(), a.middlename.length());
    sprintf(query, "SELECT id FROM author WHERE firstname='%s' AND surname='%s' AND middlename='%s'", 
      eFirstName,
      globalAnsiStr,
      eMiddleName);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  static int maxId = 0;
  if (maxId == 0)
  {
    // find largest id
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM author");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = -1;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = -1;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }
  retVal = maxId;

  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO author (id, firstname, surname, middlename) VALUES (%d, '%s', '%s', '%s')", 
      maxId++, 
      eFirstName,
      globalAnsiStr,
      eMiddleName);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;
}

int AddTranslatorToDatabase(author &a)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  char eFirstName[10240];
  char eMiddleName[10240];
  int retVal = -1;
  try
  {
    mysql_escape_string(globalAnsiStr, a.surname.c_str(), a.surname.length());
    mysql_escape_string(eFirstName, a.firstname.c_str(), a.firstname.length());
    mysql_escape_string(eMiddleName, a.middlename.c_str(), a.middlename.length());
    sprintf(query, "SELECT id FROM translator WHERE firstname='%s' AND surname='%s' AND middlename='%s'", 
      eFirstName,
      globalAnsiStr,
      eMiddleName);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  static int maxId = 0;
  if (maxId == 0)
  {
    // find largest id
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM translator");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = -1;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = -1;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }
  retVal = maxId;

  // add translator to table
  try
  {
    sprintf(query, "INSERT INTO translator (id, firstname, surname, middlename) VALUES (%d, '%s', '%s', '%s')", 
      maxId++, 
      eFirstName,
      globalAnsiStr,
      eMiddleName);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;

}

int AddGenreToDatabase(const string& genre)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal;
  try
  {
    mysql_escape_string(globalAnsiStr, genre.c_str(), genre.length());
    sprintf(query, "SELECT id FROM genres_list WHERE name='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  static int maxId = 0;
  if (maxId == 0)
  {
    // find largest id
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM genre_unique");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = -1;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = -1;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }
  retVal = maxId++;

  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO genre_unique (id, name) VALUES (%d, '%s')", 
      retVal, 
      globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }
  // add genre to table
  try
  {
    sprintf(query, "INSERT INTO genres_list (id, name) VALUES (%d, '%s')", 
      retVal, 
      globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;

}

int AddSerieToDatabase(const string& serie)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  int retVal;
  try
  {
    mysql_escape_string(globalAnsiStr, serie.c_str(), serie.length());
    sprintf(query, "SELECT id FROM serie WHERE name='%s'", globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    if (res->rowsCount() > 0)
    {
      if (res->next())
      {
        retVal = res->getInt("id");
      }
      else
      {
        retVal = -1;
      }
      delete res;
      res = NULL;
      return retVal;
    }
  }

  static int maxId = 0;
  if (maxId == 0)
  {
    // find largest id
    try
    {
      sprintf(query, "SELECT max(id) 'max' FROM serie");
      res = stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
      if (e.getErrorCode() != 0)
      {
        cout << "Adding genre: " << globalAnsiStr << endl;
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
        retVal = -1;
        return retVal;
      }
    }

    if (res)
    {
      if (res->rowsCount() > 0)
      {
        if (res->next())
        {
          maxId = res->getInt("max") + 1;
        }
        else
        {
          retVal = -1;
          delete res;
          res = NULL;
          return retVal;
        }
        delete res;
        res = NULL;
      }
    }
  }
  retVal = maxId;

  // add serie to table
  try
  {
    sprintf(query, "INSERT INTO serie (id, name) VALUES (%d, '%s')", 
      maxId++, 
      globalAnsiStr);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      retVal = -1;
      return retVal;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }

  return retVal;

}


void AddAuthorToBook(int bookId, int authorId)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  // add author to table
  try
  {
    sprintf(query, "INSERT INTO book_authors (book, author) VALUES (%d, %d)", 
      bookId, 
      authorId);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }
}

void AddTranslatorToBook(int bookId, int translatorId)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  // add author to table
  try
  {
    sprintf(query, "INSERT INTO book_translators (book, translator) VALUES (%d, %d)", 
      bookId, 
      translatorId);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }
}

void AddGenreToBook(int bookId, int genreId)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  // add author to table
  try
  {
    sprintf(query, "INSERT INTO book_genres (book, genre) VALUES (%d, %d)", 
      bookId, 
      genreId);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }
}

void AddSerieToBook(int bookId, int serieId, int serieNum)
{
  sql::ResultSet *res = NULL;
  char query[10240];
  // add author to table
  try
  {
    sprintf(query, "INSERT INTO book_series (book, serie, serie_num) VALUES (%d, %d, %d)", 
      bookId, 
      serieId,
      serieNum);
    res = stmt->executeQuery(query);
  }
  catch (sql::SQLException &e)
  {
    if (e.getErrorCode() != 0)
    {
      cout << "Adding genre: " << globalAnsiStr << endl;
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return;
    }
  }
  if (res)
  {
    delete res;
    res = NULL;
  }
}


int main(void)
{

  sql::Driver *driver;
  sql::Connection *con;
  sql::ResultSet *res = NULL;

  try 
  {
    /* Create a database connection */
    driver = get_driver_instance();
    con = driver->connect("tcp://127.0.0.1:3306", "lib", "library");
    /* Connect to the MySQL test database */
    con->setSchema("library1");
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

//  CleanDatabase();

  // parsing fb2 files
  cerr.width(5);
  DWORD timeStart1 = GetTickCount();
  WIN32_FIND_DATA findData;
  HANDLE hFind = FindFirstFile("*.fb2", &findData);
  if (hFind == INVALID_HANDLE_VALUE)
  {
    cout << "Cannot find any .fb2 files" << endl;
  }
  else
  {
    int num = 1;
    DWORD maxTime = 0;
    do 
    {
      DWORD timeStart = GetTickCount();
      string fName(findData.cFileName);
      DWORD size = findData.nFileSizeLow;
      author_array authors;
      author_array translators;
      string_array genres;
      string bookName;
      string annotation;
      string lang;
      string langOrig;
      string serieName;
      int serieNum;
      if (!ParseFb2File(fName, 
        authors,
        translators,
        genres,
        bookName,
        annotation,
        lang,
        langOrig,
        serieName,
        serieNum))
      {
        cout << "parsing error, file " << fName << endl;
        cerr << endl << "parsing error, file " << fName << endl;
        char newFileName[MAX_PATH];
        sprintf(newFileName, "..\\%s", findData.cFileName);
        MoveFile(findData.cFileName, newFileName);
        continue;
      }

      int langId = AddLangToDatabase(lang);
      if (langId < 0)
      {
        cerr << "\nerror adding language '" << lang << "', file: " << fName << "\n";
        cout << "\nerror adding language '" << lang << "', file: " << fName << "\n";
        break;
      }
      int langOrigId = AddLangToDatabase(langOrig);
      if (langOrigId < 0)
      {
        cerr << "\nerror adding language '" << langOrig << "', file: " << fName << "\n";
        cout << "\nerror adding language '" << langOrig << "', file: " << fName << "\n";
        break;
      }
      if (langOrigId == 0)
      {
        langOrigId = langId;
      }

      int bookId;
      if ((bookId = AddBookToDatabase(fName, bookName, annotation, size, langId, langOrigId)) < 0)
      {
        cerr << "\nerror adding book " << fName << ", name: '" << bookName << "'\n";
        cout << "\nerror adding book " << fName << ", name: '" << bookName << "'\n";
        break;
      }
      if (bookId == 0)
      {
        cerr << "Skipped book " << fName << "\r";
        cout << "Skipped book " << fName << "\r";
        continue; // the book already exists in database
      }


      for (int i = 0; i < authors.size(); i++)
      {
        int authorId = AddAuthorToDatabase(authors[i]);
        if (authorId < 0)
        {
          cerr << "\nerror adding author '" << 
            authors[i].surname << " " << 
            authors[i].firstname << " " << 
            authors[i].middlename << "', file: " <<
            fName << "\n";
          cout << "\nerror adding author '" << 
            authors[i].surname << " " << 
            authors[i].firstname << " " << 
            authors[i].middlename << "', file: " <<
            fName << "\n";
        }
        else
        {
          AddAuthorToBook(bookId, authorId);
        }
      }

      for (int i = 0; i < translators.size(); i++)
      {
        int translatorId = AddTranslatorToDatabase(translators[i]);
        if (translatorId < 0)
        {
          cerr << "\nerror adding translator '" << 
            translators[i].surname << " " << 
            translators[i].firstname << " " << 
            translators[i].middlename << "', file: " <<
            fName << "\n";
          cout << "\nerror adding translator '" << 
            translators[i].surname << " " << 
            translators[i].firstname << " " << 
            translators[i].middlename << "', file: " <<
            fName << "\n";
        }
        else
        {
          AddTranslatorToBook(bookId, translatorId);
        }
      }

      for (int i = 0; i < genres.size(); i++)
      {
        int genreId = AddGenreToDatabase(genres[i]);
        if (genreId < 0)
        {
          cerr << "\nerror adding genre '" << genres[i] << "', file: " << fName << "\n";
          cout << "\nerror adding genre '" << genres[i] << "', file: " << fName << "\n";
        }
        else
        {
          AddGenreToBook(bookId, genreId);
        }
      }

      if (!serieName.empty())
      {
        int serieId = AddSerieToDatabase(serieName);
        if (serieId < 0)
        {
          cerr << "\nerror adding serie '" << serieName << "', file: " << fName << "\n";
          cout << "\nerror adding serie '" << serieName << "', file: " << fName << "\n";
        }
        else
        {
          AddSerieToBook(bookId, serieId, serieNum);
        }
      }

      DWORD timeStop = GetTickCount();
      DWORD time = timeStop - timeStart;
      if (time > maxTime)
      {
        maxTime = time;
      }
      DWORD totalTime = (timeStop - timeStart1) / 1000 / 60;
      cerr << "Processed book number " << num   << ", file " << fName << ", in " << time << " msec,  total time " << totalTime << " min\r";
      cout << "Processed book number " << num++ << ", file " << fName << ", in " << time << " msec, maximum time " << maxTime << " msec\r";
    } while (FindNextFile(hFind, &findData));
    FindClose(hFind);
  }
 
  if (res) delete res;
  if (stmt) delete stmt;
  if (con) delete con;

  return EXIT_SUCCESS;
}

