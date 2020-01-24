#pragma once
#pragma pack(1)

#define BITMAP_SIGNATURE 'MB'

typedef struct {
  unsigned short int Signature;
  unsigned int Size;
  unsigned int Reserved;
  unsigned int BitsOffset;
} BITMAP_FILEHEADER;

typedef struct {
  unsigned int HeaderSize; // 0
  int Width; // 4
  int Height; // 8
  unsigned short int Planes; // 12
  unsigned short int BitCount; // 14
  unsigned int Compression; // 16
  unsigned int SizeImage; // 20
  int PelsPerMeterX; // 24
  int PelsPerMeterY; // 28
  unsigned int ClrUsed; // 32
  unsigned int ClrImportant; // 36
  // Appendix for Compression = 3
  unsigned int RedMask;
  unsigned int GreenMask;
  unsigned int BlueMask;
  unsigned int AlphaMask;
  unsigned int CsType;
  unsigned int Endpoints[9]; // see http://msdn2.microsoft.com/en-us/library/ms536569.aspx
  unsigned int GammaRed;
  unsigned int GammaGreen;
  unsigned int GammaBlue;
} BITMAP_HEADER;

