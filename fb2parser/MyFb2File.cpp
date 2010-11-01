#include "MyFb2File.h"
#include <stdlib.h>
#include <errno.h>

#if 0
string trim(const string& str)
{
  string rv = str;
  if (str.empty()) return rv;
  size_type<string> begin = 0, len = orig_len = str.length();

  while (str.substr(begin, 1) == " ")
  {
    begin++;
    len--;
  }
  while (str.substr(orig_len - 2, 1) == " ")
  {
    len--;
    orig_len--;
  }
  rv = substr(begin, len);
  return rv;
}
#endif
MyFb2File::MyFb2File(void)
: m_file(NULL)
, m_encoding("")
, m_titleInfo("")
, m_useUtf8(false)
{
}

MyFb2File::~MyFb2File(void)
{
  Close();
}

MyFb2File::FileError MyFb2File::Open(const string& fileName)
{
  Close();
  if (fopen_s(&m_file, fileName.c_str(), "rb") != 0)
  {
    return MyFb2File::CANNOT_OPEN_FILE;
  }

  size_t readSize = fread(m_buffer, sizeof(char), bufferLen - 1, m_file);
  if (readSize == 0)
  {
    Close();
    return MyFb2File::FILE_IS_NOT_FB2;
  }
  m_buffer[bufferLen - 1] = '\0'; // for string functions to work properly

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
  char encString[] = "encoding=";
  char *enc = strstr(m_buffer, encString);
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
    quote = *(enc + strlen(encString));
    char *encStart = enc + strlen(encString) + 1;
    char *encEnd = strchr(encStart, quote);
    char temp[1024];
    strncpy_s(temp, encStart, ((encEnd - encStart) > 1023) ? 1023 : (encEnd - encStart));
    m_encoding = temp;
  }
  return MyFb2File::SUCCESS;
}

string MyFb2File::GetEncoding(void)
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
  string curFirstName = "";
  string curLastName = "";
  string curMiddleName = "";
  string curAuthor = "";
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
  string curFirstName = "";
  string curLastName = "";
  string curMiddleName = "";
  string curAuthor = "";
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

bool MyFb2File::GetAuthor(string & firstName, string & lastName, string & midName, string & comment, string & longName)
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
  string curFirstName = "";
  string curLastName = "";
  string curMiddleName = "";
  string curAuthor = "";
  bool firstAuthor = true;
  longName = "";
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
      comment = "";
      firstAuthor = false;
      if (firstName.empty() && lastName.empty() && midName.empty())
      {
        firstAuthor = true;
      }
    }

    if (!longName.empty())
    {
      if ((curLastName.length() + curFirstName.length() + curMiddleName.length()) > 0)
      {
        longName += ", ";
      }
    }
    string curAuthorName = "";
    if (!curLastName.empty())
    {
      curAuthorName += curLastName;
    }
    if (!curFirstName.empty())
    {
      if (!curAuthorName.empty())
      {
        curAuthorName += " ";
        curAuthorName += curFirstName;
      }
      else
      {
        curAuthorName = curFirstName;
      }
    }
    if (!curMiddleName.empty())
    {
      if (!curAuthorName.empty())
      {
        curAuthorName += " ";
        curAuthorName += curMiddleName;
      }
      else
      {
        curAuthorName = curMiddleName;
      }
    }

    if (!curAuthorName.empty())
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

string MyFb2File::GetTitle(void)
{
  string retVal = "";
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

string MyFb2File::GetLanguage(void)
{
  string retVal = "";
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

string MyFb2File::GetLanguageOrig(void)
{
  string retVal = "";
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

bool MyFb2File::GetSerie(string &serieName, int &serieNum)
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

  string::size_type start;

  // search for sequence tag
  if ((start = m_titleInfo.find(serieTag)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  // search for name tag
  string::size_type nameStart, numStart;
  if ((nameStart = m_titleInfo.find(serieNameTag, start)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  string::size_type nameQuote1, nameQuote2, numQuote1, numQuote2;
  if ((nameQuote1 = m_titleInfo.find('\"', nameStart)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((nameQuote2 = m_titleInfo.find('\"', nameQuote1 + 1)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }

  if ((numStart = m_titleInfo.find(serieNumTag, start)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((numQuote1 = m_titleInfo.find('\"', numStart)) == string::npos)
  {
    // no series
    serieName.clear();
    serieNum = 0;
    return retVal;
  }
  if ((numQuote2 = m_titleInfo.find('\"', numQuote1 + 1)) == string::npos)
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
    string name = m_titleInfo.substr(nameQuote1 + 1, nameQuote2 - nameQuote1 - 1);
    serieName = Utf8toAnsi(name);
  }

  if ((numQuote2 - numQuote1 - 1) <= 0)
  {
    serieNum = 0;
  }
  else
  {
    string num = m_titleInfo.substr(numQuote1 + 1, numQuote2 - numQuote1 - 1);
    serieNum = atoi(num.c_str());
    if (errno == ERANGE)
    {
      serieNum = 0;
    }
  }
  retVal = true;
  return retVal;
}


string MyFb2File::GetAnnotation(void)
{
  string retVal = "";
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
  string curGenre = "";
  while ((curGenre = GetXmlTag(genreTag, m_titleInfo, pos)).length() > 0)
  {
    string::size_type comma = curGenre.find(',');
    if (comma != string::npos)
    {
      string::size_type pos = 0;
      string g = "";
      while ((comma = curGenre.find(',', pos)) != string::npos)
      {
        string::size_type beg = pos, end = comma;
        while (curGenre.substr(beg, 1) == " ")
        {
          beg++;
        }
        while (curGenre.substr(end - 1, 1) == " ")
        {
          end--;
        }
        if ((end - beg) > 0)
        {
          g = curGenre.substr(beg, end - beg);
          genres.push_back(Utf8toAnsi(g));
        }
        pos = comma + 1;
      }
      string::size_type beg = pos, end = curGenre.length();
      while (curGenre.substr(beg, 1) == " ")
      {
        beg++;
      }
      while (curGenre.substr(end - 1, 1) == " ")
      {
        end--;
      }
      if ((end - beg) > 0)
      {
        g = curGenre.substr(beg, end - beg);
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

string MyFb2File::GetGenre(void)
{
  string retVal = "";
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
  string curGenre = "";
  while ((curGenre = GetXmlTag(genreTag, m_titleInfo, pos)).length() > 0)
  {
    if (!retVal.empty())
    {
      retVal += ", ";
    }
    retVal += curGenre;
  }

  return retVal;
}

void MyFb2File::Close(void)
{
  if (m_file)
  {
    fclose(m_file);
    m_file = NULL;
  }
}


string MyFb2File::GetXmlTag(const string& tag, int &position)
{
  string buffer = m_buffer;
  return GetXmlTag(tag, buffer, position);
}

string MyFb2File::GetXmlTag(const string& tag, const string &buffer, int &position)
{
  string retVal = "";
  if (!m_file)
  {
    return retVal;
  }

  string tagBegin = "<" + tag + ">";
  string tagEnd = "</" + tag + ">";

  string::size_type iBegin, iEnd;
  // search in buffer for begin tag
  iBegin = buffer.find(tagBegin, position);
  if ((iBegin == string::npos) || ((iBegin + tagBegin.length()) >= buffer.length()))
  {
    return retVal;
  }
  iBegin += tagBegin.length();

  iEnd = buffer.find(tagEnd, iBegin);
  if (iEnd == string::npos)
  {
    return retVal;
  }

#ifndef _DEBUG
  string::size_type len1 = iEnd - iBegin;
  string retValDebug = buffer.substr(iBegin, len1);
#endif

  while ((buffer.substr(iBegin, 1) == " ") || (buffer.substr(iBegin, 1) == "\n") || (buffer.substr(iBegin, 1) == "\r"))
  {
    iBegin++;
  }
  while ((buffer.substr(iEnd - 1, 1) == " ") || (buffer.substr(iEnd - 1, 1) == "\n") || (buffer.substr(iEnd - 1, 1) == "\r"))
  {
    iEnd--;
  }
  string::size_type len = iEnd - iBegin;
  if (((int)iEnd - (int)iBegin) > 0)
  {
    retVal = buffer.substr(iBegin, len);
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
  int len = strlen(utf8);
  int	 length	 = MultiByteToWideChar(CP_UTF8, 0, utf8, len, NULL, NULL );

  //this step intended only to use WideCharToMultiByte
  MultiByteToWideChar(CP_UTF8, 0, utf8, -1, m_globalWcharStr, length );

  //Conversion to ANSI (CP_ACP)
  WideCharToMultiByte(CP_ACP, 0, m_globalWcharStr, -1, m_globalAnsiStr, length, NULL, NULL);

  m_globalAnsiStr[length] = 0;

  return m_globalAnsiStr;
}


string MyFb2File::Utf8toAnsi( const string &utf8 )
{
  string retVal = "";
  char * rv = Utf8toAnsi(utf8.c_str());
  if (rv != NULL)
  {
    retVal = rv;
  }
  return retVal;
}
