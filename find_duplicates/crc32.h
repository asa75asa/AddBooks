#ifndef __CRC_H__
#define __CRC_H__

// class for calculating CRC32 of a file or data block
// usage: create instance of the class, 
// then call ProcessCRC method with the data which CRC is to be calculated
// If file is too big for one data block, need to call ProcessCRC several times to cover all the data
// After that m_crc32 stores the CRC32 code
class CCrc32{
protected:
  unsigned table[256];
public:
  unsigned m_crc32;
  CCrc32();
  void ProcessCRC(void* pData, int nLen);
};

#endif