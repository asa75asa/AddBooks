#include "MyFb2File.h"
#include <QFile>
#include <string.h>

MyFb2File::MyFb2File(void)
: m_file(NULL)
, m_encoding(tr(""))
, m_titleInfo(tr(""))
, m_useUtf8(false)
{
}

MyFb2File::~MyFb2File(void)
{
  Close();
}

MyFb2File::FileError MyFb2File::Open(const QString& fileName)
{
  Close();
  m_file = new QFile(fileName);
  if (!m_file)
  {
    return MyFb2File::CANNOT_OPEN_FILE;
  }
  if (!m_file->open(QIODevice::ReadOnly))
  {
    return MyFb2File::CANNOT_OPEN_FILE;
  }

  qint64 readSize = m_file->read(m_buffer, bufferLen - 1);
  if (readSize == 0)
  {
    Close();
    return MyFb2File::FILE_IS_NOT_FB2;
  }
  m_buffer[bufferLen - 1] = '\0'; // for QString functions to work properly

  // check for xml tag
  char* p = strstr(m_buffer, "<?xml");
  if ((p == NULL) || ((p - m_buffer) > 100))
  {
    Close();
    return MyFb2File::FILE_IS_NOT_FB2;
  }

  // find end of xml header
  p = strstr(m_buffer, "?>");
  if ((p == NULL) || ((p - m_buffer) > 1000))
  {
    Close();
    return MyFb2File::FILE_IS_NOT_FB2;
  }

  // read encoding
  char encQString[] = "encoding=";
  char *enc = strstr(m_buffer, encQString);
  if ((enc == NULL) || (enc > p))
  {
    m_encoding = "";
    // try to recognize utf8
    int count = 0;
    for (int i = 0; i < readSize; i++)
    {
      if ((((unsigned char)m_buffer[i]) == 0xd0) || (((unsigned char)m_buffer[i]) == 0xd1))
      {
        count++;
      }
    }
    if (count > (readSize / 10))
    {
      m_encoding = "utf-8";
    }
  }
  else
  {
    // find quote symbol around encoding
    char quote;
    quote = *(enc + strlen(encQString));
    char *encStart = enc + strlen(encQString) + 1;
    char *encEnd = strchr(encStart, quote);
    char temp[1024];
    strncpy_s(temp, encStart, ((encEnd - encStart) > 1023) ? 1023 : (encEnd - encStart));
    m_encoding = temp;
  }
  return MyFb2File::SUCCESS;
}

QString MyFb2File::GetEncoding(void)
{
  return m_encoding;
}

bool MyFb2File::GetTranslators(author_array &translators)
{
  bool retVal = false;
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  translators.clear();

  pos = 0;
  QString curFirstName = tr("");
  QString curLastName = tr("");
  QString curMiddleName = tr("");
  QString curAuthor = tr("");
  bool firstAuthor = true;

  while ((curAuthor = GetXmlTag(translatorTag, m_titleInfo, pos)).length() > 0)
  {
    int authorPos = 0;
    curFirstName = GetXmlTag(firstNameTag, curAuthor, authorPos);
    authorPos = 0;
    curLastName = GetXmlTag(lastNameTag, curAuthor, authorPos);
    authorPos = 0;
    curMiddleName = GetXmlTag(middleNameTag, curAuthor, authorPos);

    if ((curFirstName.length() + curLastName.length() + curMiddleName.length()) > 0)
    {
      author a;
      a.firstname = Utf8toAnsi(curFirstName);
      a.surname = Utf8toAnsi(curLastName);
      a.middlename = Utf8toAnsi(curMiddleName);
      translators.push_back(a);
      retVal = true;
    }
  }

  return retVal;

}

bool MyFb2File::GetAuthors(author_array &authors)
{
  bool retVal = false;
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  authors.clear();

  pos = 0;
  QString curFirstName = tr("");
  QString curLastName = tr("");
  QString curMiddleName = tr("");
  QString curAuthor = tr("");
  bool firstAuthor = true;

  while ((curAuthor = GetXmlTag(authorTag, m_titleInfo, pos)).length() > 0)
  {
    int authorPos = 0;
    curFirstName = GetXmlTag(firstNameTag, curAuthor, authorPos);
    authorPos = 0;
    curLastName = GetXmlTag(lastNameTag, curAuthor, authorPos);
    authorPos = 0;
    curMiddleName = GetXmlTag(middleNameTag, curAuthor, authorPos);

    if ((curFirstName.length() + curLastName.length() + curMiddleName.length()) > 0)
    {
      author a;
      a.firstname = Utf8toAnsi(curFirstName);
      a.surname = Utf8toAnsi(curLastName);
      a.middlename = Utf8toAnsi(curMiddleName);
      authors.push_back(a);
      retVal = true;
    }
  }

  return retVal;

}

bool MyFb2File::GetAuthor(QString & firstName, QString & lastName, QString & midName, QString & comment, QString & longName)
{
  bool retVal = false;
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  QString curFirstName = tr("");
  QString curLastName = tr("");
  QString curMiddleName = tr("");
  QString curAuthor = tr("");
  bool firstAuthor = true;
  longName = tr("");
  while ((curAuthor = GetXmlTag(authorTag, m_titleInfo, pos)).length() > 0)
  {
    int authorPos = 0;
    curFirstName = GetXmlTag(firstNameTag, curAuthor, authorPos);
    authorPos = 0;
    curLastName = GetXmlTag(lastNameTag, curAuthor, authorPos);
    authorPos = 0;
    curMiddleName = GetXmlTag(middleNameTag, curAuthor, authorPos);

    if (firstAuthor)
    {
      firstName = curFirstName;
      lastName = curLastName;
      midName = curMiddleName;
      comment = tr("");
      firstAuthor = false;
      if (firstName.isEmpty() && lastName.isEmpty() && midName.isEmpty())
      {
        firstAuthor = true;
      }
    }

    if (!longName.isEmpty())
    {
      if ((curLastName.length() + curFirstName.length() + curMiddleName.length()) > 0)
      {
        longName += tr(", ");
      }
    }
    QString curAuthorName = tr("");
    if (!curLastName.isEmpty())
    {
      curAuthorName += curLastName;
    }
    if (!curFirstName.isEmpty())
    {
      if (!curAuthorName.isEmpty())
      {
        curAuthorName += tr(" ");
        curAuthorName += curFirstName;
      }
      else
      {
        curAuthorName = curFirstName;
      }
    }
    if (!curMiddleName.isEmpty())
    {
      if (!curAuthorName.isEmpty())
      {
        curAuthorName += tr(" ");
        curAuthorName += curMiddleName;
      }
      else
      {
        curAuthorName = curMiddleName;
      }
    }

    if (!curAuthorName.isEmpty())
    {
      longName += curAuthorName;
    }
  }

  if ((firstName.length() + lastName.length() + midName.length() + comment.length() + longName.length()) > 0)
  {
    retVal = true;
  }


  return retVal;
}

QString MyFb2File::GetTitle(void)
{
  QString retVal = tr("");
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  retVal = Utf8toAnsi(GetXmlTag(bookTitleTag, m_titleInfo, pos));

  return retVal;
}

QString MyFb2File::GetLanguage(void)
{
  QString retVal = tr("");
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  retVal = Utf8toAnsi(GetXmlTag(langTag, m_titleInfo, pos));

  return retVal;
}

QString MyFb2File::GetLanguageOrig(void)
{
  QString retVal = tr("");
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  retVal = Utf8toAnsi(GetXmlTag(langOrigTag, m_titleInfo, pos));

  return retVal;
}

bool MyFb2File::GetSerie(QString &serieName, int &serieNum)
{
  bool retVal = false;
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  int start;

  // search for sequence tag
  if ((start = m_titleInfo.indexOf(serieTag)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  // search for name tag
  int nameStart, numStart;
  if ((nameStart = m_titleInfo.indexOf(serieNameTag, start)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  int nameQuote1, nameQuote2, numQuote1, numQuote2;
  if ((nameQuote1 = m_titleInfo.indexOf('\"', nameStart)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((nameQuote2 = m_titleInfo.indexOf('\"', nameQuote1 + 1)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  if ((numStart = m_titleInfo.indexOf(serieNumTag, start)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((numQuote1 = m_titleInfo.indexOf('\"', numStart)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((numQuote2 = m_titleInfo.indexOf('\"', numQuote1 + 1)) == -1)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  if ((nameQuote2 - nameQuote1 - 1) <= 0)
  {
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  else
  {
    QString name = m_titleInfo.mid(nameQuote1 + 1, nameQuote2 - nameQuote1 - 1);
    serieName = Utf8toAnsi(name);
  }

  if ((numQuote2 - numQuote1 - 1) <= 0)
  {
    serieNum = 0;
  }
  else
  {
    QString num = m_titleInfo.mid(numQuote1 + 1, numQuote2 - numQuote1 - 1);
    bool ok;
    serieNum = num.toInt(&ok);
    if (!ok)
    {
      serieNum = 0;
    }
  }
  retVal = true;
  return retVal;
}


QString MyFb2File::GetAnnotation(void)
{
  QString retVal = tr("");
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  retVal = Utf8toAnsi(GetXmlTag(annotationTag, m_titleInfo, pos));

  return retVal;
}

bool MyFb2File::GetGenres(string_array &genres)
{
  bool retVal = false;
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  genres.clear();

  pos = 0;
  QString curGenre = tr("");
  while ((curGenre = GetXmlTag(genreTag, m_titleInfo, pos)).length() > 0)
  {
    int comma = curGenre.indexOf(',');
    if (comma != -1)
    {
      int pos = 0;
      QString g = tr("");
      while ((comma = curGenre.indexOf(',', pos)) != -1)
      {
        int beg = pos, end = comma;
        while (curGenre.mid(beg, 1) == tr(" "))
        {
          beg++;
        }
        while (curGenre.mid(end - 1, 1) == tr(" "))
        {
          end--;
        }
        if ((end - beg) > 0)
        {
          g = curGenre.mid(beg, end - beg);
          genres.push_back(Utf8toAnsi(g));
        }
        pos = comma + 1;
      }
      int beg = pos, end = curGenre.length();
      while (curGenre.mid(beg, 1) == tr(" "))
      {
        beg++;
      }
      while (curGenre.mid(end - 1, 1) == tr(" "))
      {
        end--;
      }
      if ((end - beg) > 0)
      {
        g = curGenre.mid(beg, end - beg);
        genres.push_back(Utf8toAnsi(g));
      }
    }
    else
    {
      genres.push_back(Utf8toAnsi(curGenre));
    }
    retVal = true;
  }

  return retVal;

}

QString MyFb2File::GetGenre(void)
{
  QString retVal = tr("");
  int pos = 0;
  if (m_titleInfo.length() == 0)
  {
    m_titleInfo = GetXmlTag(titleInfoTag, pos);
  }
  if (m_titleInfo.length() == 0)
  {
    return retVal;
  }

  pos = 0;
  QString curGenre = tr("");
  while ((curGenre = GetXmlTag(genreTag, m_titleInfo, pos)).length() > 0)
  {
    if (!retVal.isEmpty())
    {
      retVal += tr(", ");
    }
    retVal += curGenre;
  }

  return retVal;
}

void MyFb2File::Close(void)
{
  if (m_file)
  {
    delete m_file;
    m_file = NULL;
  }
}


QString MyFb2File::GetXmlTag(const QString& tag, int &position)
{
  QString buffer = m_buffer;
  return GetXmlTag(tag, buffer, position);
}

QString MyFb2File::GetXmlTag(const QString& tag, const QString &buffer, int &position)
{
  QString retVal = tr("");
  if (!m_file)
  {
    return retVal;
  }

  QString tagBegin = tr("<") + tag + tr(">");
  QString tagEnd = tr("</") + tag + tr(">");

  int iBegin, iEnd;
  // search in buffer for begin tag
  iBegin = buffer.indexOf(tagBegin, position);
  if ((iBegin == -1) || ((iBegin + tagBegin.length()) >= buffer.length()))
  {
    return retVal;
  }
  iBegin += tagBegin.length();

  iEnd = buffer.indexOf(tagEnd, iBegin);
  if (iEnd == -1)
  {
    return retVal;
  }

#ifndef _DEBUG
  int len1 = iEnd - iBegin;
  QString retValDebug = buffer.mid(iBegin, len1);
#endif

  while ((buffer.mid(iBegin, 1) == tr(" ")) || (buffer.mid(iBegin, 1) == tr("\n")) || (buffer.mid(iBegin, 1) == tr("\r")))
  {
    iBegin++;
  }
  while ((buffer.mid(iEnd - 1, 1) == tr(" ")) || (buffer.mid(iEnd - 1, 1) == tr("\n")) || (buffer.mid(iEnd - 1, 1) == tr("\r")))
  {
    iEnd--;
  }
  int len = iEnd - iBegin;
  if (((int)iEnd - (int)iBegin) > 0)
  {
    retVal = buffer.mid(iBegin, len);
  }
  position = iBegin;
  return retVal;
}

char* MyFb2File::Utf8toAnsi( const char * utf8 )
{
  if (!utf8) return NULL;

  char *ansistr	= NULL;
  if (!m_useUtf8)
  {
    ansistr = const_cast<char*>(utf8);
    return ansistr;
  }

  QString ansi = QString::fromUtf8(utf8);
  string str = ansi.toStdString();
  strncpy(m_globalAnsiStr, str.c_str(), str.length());
  m_globalAnsiStr[str.length()] = 0;

  return m_globalAnsiStr;
}


QString MyFb2File::Utf8toAnsi( const QString &utf8 )
{
  QString retVal = "";
  char * rv = Utf8toAnsi(utf8.toStdString().c_str());
  if (rv != NULL)
  {
    retVal = rv;
  }
  return retVal;
}
