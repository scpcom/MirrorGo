#define BITMAP_FILEHEADER_SIZE 14

#define BITMAP_HEADER_SIZE 40+68

void bitmap_fill_header(char* bfh,  char* bh, unsigned int w, unsigned int h, unsigned short int bits);

unsigned int bitmap_fileheader_size(char* bfh);

