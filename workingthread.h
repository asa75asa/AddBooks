#ifndef WORKINGTHREAD_H
#define WORKINGTHREAD_H

#include <QThread>
#include <QSqlDatabase>

class WorkingThread : public QThread
{
public:
  WorkingThread(QObject *parent);
  ~WorkingThread();
  void run();
  void stop();
  bool setData(const QString &newDir,
               const QString &oldDir,
               const QString &dbHost,
               const QString &dbName,
               const QString &dbUser,
               const QString &dbPass,
               bool compress,
               bool moveFiles,
               bool renameFiles);
private:
  volatile bool stopped;
  QString sNewDir;
  QString sOldDir;
  QString sDbHost;
  QString sDbName;
  QString sDbUser;
  QString sDbPass;
  bool bCompress;
  bool bMoveFiles;
  bool bRenameFiles;
  long lStartNumber; // start number for batch renaming
  QString sStatus;
  QSqlDatabase database;

  bool CheckData();
  bool RenameFiles();
  bool ParseFb2();
  bool MoveFiles();
};

#endif // WORKINGTHREAD_H
