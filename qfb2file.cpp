#include "qfb2file.h"

QFb2File::QFb2File(QObject *parent)
  : QObject(parent)
  , m_pFile(NULL)
{

}

QFb2File::~QFb2File()
{
  Close();
}

QFb2File::FileError QFb2File::Open(const QString &name)
{
  Close();
  m_pFile = new QFile(name);
  if (!m_pFile)
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

void QFb2File::Close()
{
  if (m_pFile)
  {
    delete m_pFile;
    m_pFile = NULL;
  }
}
