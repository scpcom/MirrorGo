
#include "bmpheader.h"
#include "bmp_priv.h"

void bitmap_fill_header(char* bfh,  char* bh, unsigned int w, unsigned int h, unsigned short int bits)
{
  BITMAP_FILEHEADER *FileHeader = (BITMAP_FILEHEADER *)bfh;
  BITMAP_HEADER *Header = (BITMAP_HEADER *)bh;

  FileHeader->Signature = BITMAP_SIGNATURE;
  FileHeader->BitsOffset = BITMAP_FILEHEADER_SIZE+sizeof(BITMAP_HEADER);

  Header->HeaderSize =  sizeof(BITMAP_HEADER);
  Header->Compression = 3; // BI_BITFIELDS
  Header->BitCount = bits;

  FileHeader->Size = (w * h * Header->BitCount) / 8 + FileHeader->BitsOffset;

  Header->Planes = 1;
  Header->Height = h;
  Header->Width = w;
  Header->SizeImage = (w * h * Header->BitCount) / 8;
  Header->PelsPerMeterX = 3780;
  Header->PelsPerMeterY = 3780;

  Header->RedMask   = 0xF800;
  Header->GreenMask = 0x07E0;
  Header->BlueMask  = 0x001F;
  Header->AlphaMask = 0x0000;
}

unsigned int bitmap_fileheader_size(char* bfh) {
  BITMAP_FILEHEADER *FileHeader = (BITMAP_FILEHEADER *)bfh;

  return FileHeader->Size;
}

