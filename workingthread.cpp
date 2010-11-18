#include "workingthread.h"
#include <QDirIterator>

WorkingThread::WorkingThread(QObject *parent)
  : QThread(parent)
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
  if (running())
  {
    // don't set data if the thread is already running
    return;
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
}

void WorkingThread::stop()
{
  stopped = true;
}

WorkingThread::run()
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
  QString nameFilter(tr(*.fb2));
  // check for existence of newDir
  QDir newDir(sNewDir);
  if (!newDir.exists())
  {
    sStatus = QString(tr("Directory with new files does not exist: %1")).arg(sNewDir);
    return false;
  }
  // check for *.fb2 files in new dir
  newDir.setNameFilters(nameFilter);
  QDirIterator newDirIterator(newDir);
  if (!newDirIterator.hasNext())
  {
    sStatus = QString(tr("New directory does not have fb2 files: %1")).arg(sNewDir);
    return false;
  }
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

  // find last file in directory
  QStringList filesList;
  filesList = oldDir.entryList(nameFilter, QDir::NoFilter, QDir::Name | QDir::Reversed);
  if (filesList.isEmpty())
  {
    lStartNumber = 1;
  }
  else
  {
    // first file in list should have the last number
    QString fName = filesList.at(0);
    long len = fName.length();
    QString number = fName.left(len - nameFilter.length() + 1);
    lStartNumber = number.toInt() + 1;
  }

  // check database connectivity
  if (database.isOpen())
  {
    database.close();
  }
  database.set




}

bool WorkingThread::RenameFiles()
{

}

bool WorkingThread::ParseFb2()
{

}

bool WorkingThread::MoveFiles()
{

}
