#include "workingthread.h"
#include <QDirIterator>
#include <QSqlQuery>
#include <QVariant>
#include "MyFb2File.h"

WorkingThread::WorkingThread()
: QThread()
  , stopped(false)
  , sNewDir(tr(""))
  , sOldDir(tr(""))
  , sDbHost(tr(""))
  , sDbName(tr(""))
  , sDbUser(tr(""))
  , sDbPass(tr(""))
  , bCompress(true)
  , bMoveFiles(true)
  , bRenameFiles(true)
  , lStartNumber(1)
  , sStatus(tr(""))
{
  database = QSqlDatabase::addDatabase(tr("QMYSQL"));

}

WorkingThread::~WorkingThread()
{
  stop();
}

bool WorkingThread::setData(const QString &newDir,
                            const QString &oldDir,
                            const QString &dbHost,
                            const QString &dbName,
                            const QString &dbUser,
                            const QString &dbPass,
                            bool compress,
                            bool moveFiles,
                            bool renameFiles)
{
  if (isRunning())
  {
    // don't set data if the thread is already running
    sStatus = tr("Thread is already running");
    return false;
  }
  sNewDir = newDir;
  sOldDir = oldDir;
  sDbHost = dbHost;
  sDbName = dbName;
  sDbUser = dbUser;
  sDbPass = dbPass;
  bCompress = compress;
  bMoveFiles = moveFiles;
  bRenameFiles = renameFiles;
  return true;
}

void WorkingThread::stop()
{
  stopped = true;
}

void WorkingThread::run()
{
  if (stopped) return;

  // Check data (database connectivity, directories)
  if (!CheckData()) return;
  if (stopped) return;

  if (!RenameFiles()) return;
  if (stopped) return;

  if (!ParseFb2()) return;
  if (stopped) return;

  if (!MoveFiles()) return;
  if (stopped) return;

}

bool WorkingThread::CheckData()
{
  // check for existence of newDir
  QDir newDir(sNewDir);
  if (!newDir.exists())
  {
    sStatus = QString(tr("Directory with new files does not exist: %1")).arg(sNewDir);
    return false;
  }
  if (stopped) return false;

  // check for *.fb2 files in new dir
  QStringList filters;
  filters << tr("*.fb2") << tr("*.fb2.gz");
  newDir.setNameFilters(filters);
  QDirIterator newDirIterator(newDir);
  if (!newDirIterator.hasNext())
  {
    sStatus = QString(tr("New directory does not have fb2 files: %1")).arg(sNewDir);
    return false;
  }
  if (stopped) return false;
  // check for old directory, create if it does not exist
  QDir oldDir(sOldDir);
  if (!oldDir.exists())
  {
    if (!oldDir.mkpath(sOldDir))
    {
      sStatus = QString(tr("Cannot create directory for old files: %1")).arg(sNewDir);
      return false;
    }
  }
  if (stopped) return false;

  // check database connectivity
  if (database.isOpen())
  {
    database.close();
  }
  database.setDatabaseName(sDbName);
  database.setHostName(sDbHost);
  database.setUserName(sDbUser);
  database.setPassword(sDbPass);
  if (!database.open())
  {
    sStatus = QString(tr("Cannot connect to database %1 with user name %2")).arg(sDbName).arg(sDbUser);
    return false;
  }

  // find last file in database
  QString number(tr("0"));
  QSqlQuery query(tr("SELECT max(filename) max FROM book"), database);
  if (query.next()) 
  {
    QString maxFile = query.value(0).toString();
    if (maxFile.right(7) == QString(tr(".fb2.gz")))
    {
      number = maxFile.left(maxFile.length() - 7);
    }
    else if (maxFile.right(4) == QString(tr(".fb2")))
    {
      number = maxFile.left(maxFile.length() - 4);
    }
    else
    {
      number = tr("0");
    }
  }
  lStartNumber = number.toInt() + 1;
  if (stopped) return false;

  return true;
}

bool WorkingThread::RenameFiles()
{
  // make dir for renamed files
  QString renameDir;
  bool created = false;
  // try names from 1 to 100, otherwise function fails
  for (int i = 1; i <= 100; i++)
  {
    QString format;
    renameDir = sNewDir + QString(tr("/")) + QString(tr("%1")).arg(i);
    QDir rDir(renameDir);
    if (!rDir.exists())
    {
      if (rDir.mkdir(renameDir))
      {
        created = true;
        break;
      }
    }
  }
  if (!created)
  {
    sStatus = QString(tr("Cannot create sub-directory for renaming files in %1")).arg(sNewDir);
    return false;
  }

  // move or copy files to created dir
  QDir newDir(sNewDir);
  QStringList filters;
  filters << tr("*.fb2");
  newDir.setNameFilters(filters);
  QDirIterator newDirIterator(newDir);
  QString fileToRename;
  QString newFileName;
  long fileNumber = lStartNumber;
  while (newDirIterator.hasNext())
  {
    fileToRename = newDirIterator.next();
    newFileName = renameDir + QString(tr("/")) + QString(tr("%1.fb2")).arg(fileNumber++, 6, 10, QLatin1Char('0'));
    bool rv;
    if (bRenameFiles)
    {
      rv = QFile::rename(fileToRename, newFileName);
      if (!rv)
      {
        sStatus = QString(tr("Cannot rename file %1 to %d")).arg(fileToRename, newFileName);
      }
    }
    else
    {
      rv = QFile::copy(fileToRename, newFileName);
      if (!rv)
      {
        sStatus = QString(tr("Cannot copy file %1 to %d")).arg(fileToRename, newFileName);
      }
    }
    if (!rv) return false;
  }
  return true;
}

bool WorkingThread::ParseFb2()
{

  return false;
}

bool WorkingThread::MoveFiles()
{

  return false;
}

char globalQuery[globalLen];
char globalAnsiStr[globalLen];
WCHAR globalWcharStr[globalLen];

bool WorkingThread::ParseFb2File(const QString& fileName,
                                 author_array &authors,
                                 author_array &translators,
                                 string_array &genres,
                                 QString &bookName,
                                 QString &annotation,
                                 QString &lang,
                                 QString &langOrig,
                                 QString &serieName,
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


int WorkingThread::GetGenreId(const QString &genreName)
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


int WorkingThread::AddBookToDatabase(const string &fName, 
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


int WorkingThread::AddLangToDatabase(const string &lang)
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

int WorkingThread::AddAuthorToDatabase(author &a)
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

int WorkingThread::AddTranslatorToDatabase(author &a)
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

int WorkingThread::AddGenreToDatabase(const QString& genre)
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

int WorkingThread::AddSerieToDatabase(const QString& serie)
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


void WorkingThread::AddAuthorToBook(int bookId, int authorId)
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

void WorkingThread::AddTranslatorToBook(int bookId, int translatorId)
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

void WorkingThread::AddGenreToBook(int bookId, int genreId)
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

void WorkingThread::AddSerieToBook(int bookId, int serieId, int serieNum)
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
