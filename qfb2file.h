#ifndef QFB2FILE_H
#define QFB2FILE_H

#include <QObject>

class QFb2File : public QObject
{
  Q_OBJECT

public:
  typedef enum _FileError 
  {
    SUCCESS, 
    CANNOT_OPEN_FILE,
    FILE_IS_NOT_FB2,
    ERROR_PARSING_XML
  } FileError;

public:
    QFb2File(QObject *parent);
    ~QFb2File();
    QFb2File::FileError Open(const QString &name);
    void Close();

private:
  QFile *m_pFile;
    
};

#endif // QFB2FILE_H
