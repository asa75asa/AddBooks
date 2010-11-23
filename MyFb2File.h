#ifndef MYFB2FILE_H
#define MYFB2FILE_H

#include <vector>
#include <QObject>
#include <QString>

class QFile;

const int bufferLen = 1024 * 64;
const int globalLen = 1024 * 128;

using namespace std;

const QString titleInfoTag = "title-info";
const QString genreTag = "genre";
const QString authorTag = "author";
const QString translatorTag = "translator";
const QString langTag = "lang";
const QString langOrigTag = "src-lang";
const QString serieTag = "<sequence";
const QString serieNameTag = "name=";
const QString serieNumTag = "number=";
const QString firstNameTag = "first-name";
const QString lastNameTag = "last-name";
const QString middleNameTag = "middle-name";
const QString bookTitleTag = "book-title";
const QString annotationTag = "annotation";

typedef vector<QString> string_array;

typedef struct _author
{
  QString firstname;
  QString surname;
  QString middlename;
  QString comment;
} author;

typedef vector<author> author_array;

typedef struct _book
{
  QString name;
  QString description;
  QString filename;
  QString lang;
  QString lang_orig;
} book;



class MyFb2File : public QObject
{
  Q_OBJECT
public:

  typedef enum _FileError 
  {
    SUCCESS, 
    CANNOT_OPEN_FILE,
    FILE_IS_NOT_FB2
  } FileError;
  bool m_useUtf8;



  MyFb2File(void);
  ~MyFb2File(void);
  FileError Open(const QString& fileName);
  QString GetEncoding(void);
  bool GetAuthor(QString & firstName, QString & lastName, QString & midName, QString & comment, QString & longName);
  bool GetAuthors(author_array &authors);
  bool GetTranslators(author_array &translators);
  bool GetGenres(string_array &genres);
  QString GetTitle(void);
  QString GetAnnotation(void);
  QString GetGenre(void);
  QString GetLanguage(void);
  QString GetLanguageOrig(void);
  bool GetSerie(QString &serieName, int &serieNum);
  void Close(void);
  QString GetXmlTag(const QString& tag, const QString &buffer, int &position);
  QString GetXmlTag(const QString& tag, int &position);
  QString Utf8toAnsi( const QString &utf8 );
  char* Utf8toAnsi( const char * utf8 );
protected:
  QFile* m_file;
  char m_buffer[bufferLen];
  char m_temp[bufferLen];
  QString m_encoding;
  QString m_titleInfo;
//  char m_globalQuery[globalLen];
  char m_globalAnsiStr[globalLen];
  QChar m_globalWcharStr[globalLen];

};
#endif // MYFB2FILE_H
