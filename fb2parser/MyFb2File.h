#pragma once

const int bufferLen = 1024 * 64;
#include <string>
#include <vector>
#include <windows.h>
const int globalLen = 1024 * 128;

using namespace std;

const string titleInfoTag = "title-info";
const string genreTag = "genre";
const string authorTag = "author";
const string translatorTag = "translator";
const string langTag = "lang";
const string langOrigTag = "src-lang";
const string serieTag = "<sequence";
const string serieNameTag = "name=";
const string serieNumTag = "number=";
const string firstNameTag = "first-name";
const string lastNameTag = "last-name";
const string middleNameTag = "middle-name";
const string bookTitleTag = "book-title";
const string annotationTag = "annotation";

typedef vector<string> string_array;

typedef struct _author
{
  string firstname;
  string surname;
  string middlename;
  string comment;
} author;

typedef vector<author> author_array;

typedef struct _book
{
  string name;
  string description;
  string filename;
  string lang;
  string lang_orig;
} book;



class MyFb2File
{
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
  FileError Open(const string& fileName);
  string GetEncoding(void);
  bool GetAuthor(string & firstName, string & lastName, string & midName, string & comment, string & longName);
  bool GetAuthors(author_array &authors);
  bool GetTranslators(author_array &translators);
  bool GetGenres(string_array &genres);
  string GetTitle(void);
  string GetAnnotation(void);
  string GetGenre(void);
  string GetLanguage(void);
  string GetLanguageOrig(void);
  bool GetSerie(string &serieName, int &serieNum);
  void Close(void);
  string GetXmlTag(const string& tag, const string &buffer, int &position);
  string GetXmlTag(const string& tag, int &position);
  string Utf8toAnsi( const string &utf8 );
  char* Utf8toAnsi( const char * utf8 );
protected:
  FILE* m_file;
  char m_buffer[bufferLen];
  char m_temp[bufferLen];
  string m_encoding;
  string m_titleInfo;
//  char m_globalQuery[globalLen];
  char m_globalAnsiStr[globalLen];
  WCHAR m_globalWcharStr[globalLen];

};
