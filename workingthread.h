#ifndef WORKINGTHREAD_H
#define WORKINGTHREAD_H

#include <QThread>
#include <QSqlDatabase>

class WorkingThread : public QThread
{
  Q_OBJECT
public:
  WorkingThread();
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
  bool ParseFb2File(const QString& fileName,
                    author_array &authors,
                    author_array &translators,
                    string_array &genres,
                    QString &bookName,
                    QString &annotation,
                    QString &lang,
                    QString &langOrig,
                    QString &serieName,
                    int &serieNum);
  int GetGenreId(const QString &genreName);
  void AddSerieToBook(int bookId, int serieId, int serieNum);
  void AddGenreToBook(int bookId, int genreId);
  void AddTranslatorToBook(int bookId, int translatorId);
  void AddAuthorToBook(int bookId, int authorId);
  int AddSerieToDatabase(const QString& serie);
  int AddGenreToDatabase(const QString& genre);




};

#endif // WORKINGTHREAD_H
