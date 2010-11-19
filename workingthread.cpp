#include "workingthread.h"
#include <QDirIterator>

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

  // find last file in directory
  oldDir.setNameFilters(filters);
  QDirIterator oldDirIterator(oldDir);
  QString lastNumber = tr("0");
  while (oldDirIterator.hasNext())
  {
    if (stopped) return false;
    oldDirIterator.next();
    QString fName = oldDirIterator.fileName();
    QString number;
    if (fName.right(7) == QString(tr(".fb2.gz")))
    {
      number = fName.left(fName.length() - 7);
    }
    else if (fName.right(4) == QString(tr(".fb2")))
    {
      number = fName.left(fName.length() - 4);
    }
    else
    {
      number = tr("0");
    }
    if (number > lastNumber)
    {
      lastNumber = number;
    }
  }
  lStartNumber = lastNumber.toInt() + 1;
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
  return true;
}

bool WorkingThread::RenameFiles()
{

  return false;
}

bool WorkingThread::ParseFb2()
{

  return false;
}

bool WorkingThread::MoveFiles()
{

  return false;
}
