//---------------------------------------------------------------------------
#define MAXCOLORTC

#include "wingraph.h"
#include "limits.h"
//---------------------------------------------------------------------------
typedef struct TBMPHeaderStruct {
  short bftype;
  long  bfsize;
  short rez1, rez2;
  long  bfoffbits;
  long  bisize;
  long  biwidth;
  long  biheight;
  short biplanes;
  short bibitcount;
  long  bicompression;
  long  bisizeimage;
  long  bix;
  long  biy;
  long  biclrused;
  long  biclrimp;
} TBMPHeader;

typedef struct TPaletteStruct
{
  unsigned char Red;
  unsigned char Green;
  unsigned char Blue;
} TPalette;

// ��������� ���������� ������ ������ � ����� ������, ����������� �� 32-������ �������
int Offset(int width)
{
  int offset = 0;
  while (width % 4 != 0) {
    width++;
    offset++;
  }
  return offset;
}

// ������ 256-������� BMP ����
void Read256BMPFile(char *buf, int bufSize, char **pImg, int& imgSize)
{
  TBMPHeader *header = (TBMPHeader *) buf;
  buf += sizeof(TBMPHeader);
//  bufSize -= sizeof(TBMPHeader);
  int width = header->biwidth; // ������
  int height = header->biheight; // ������
  int offset = Offset(width);
  imgSize = width * height * 3;
  *pImg = new char[imgSize];
  unsigned char palitra[256][3]; // �������
  for (int i = 0; i < 256; i++) {
    palitra[i][2] = *buf;
    buf++;
    palitra[i][1] = *buf;
    buf++;
    palitra[i][0] = *buf;
    buf += 2;
  }
  int m = 0; // ����� ������� ������� �� ������� ������
  // ������� ���� ���������� � ������ �������
  for (int i = height - 1; i >= 0; i--) {
    int pos = i * width * 3;
    for (int j = 0; j < width; j++) {
      unsigned char c = buf[m];
      for (int k = 0; k < 3; k++)
        (*pImg)[pos + j * 3 + k] = palitra[c][k];
      m++;
    }
    m += offset; // ���������� ������ �����
  }
}

// ������� �������� �� �����
void ShowPicture(char *buf, int w, int h, int x, int y)
{
  int k = 0;
  for (int i = 0; i < h; i++)
    for (int j = 0; j < w; j++) {
      putpixel(x + j, y + i, RGB(buf[k], buf[k + 1], buf[k + 2]));
      k += 3;
    }
}

// ��������� ������� ����� RGB �������������
int Difference(TPalette c1, TPalette c2)
{
  return pow(c1.Red - c2.Red, 2) + pow(c1.Green - c2.Green, 2) + pow(c1.Blue - c2.Blue, 2);
}

// ���������� ��� ����� �� ���� ������� �����
bool AlikeColors(TPalette c1, TPalette c2)
{
  c1.Red = c1.Red & 0xC0;
  c2.Red = c2.Red & 0xC0;
  c1.Blue = c1.Blue & 0xC0;
  c2.Blue = c2.Blue & 0xC0;
  c1.Green = c1.Green & 0xC0;
  c2.Green = c2.Green & 0xC0;
  return c1.Red == c2.Red && c1.Green == c2.Green && c1.Blue == c2.Blue;
}

// ������� 16-������� BMP-����
void Create16BMP(char *img, int imgSize, int width, int height, const char *fileName)
{
  const int colorsNum = 16; // ���������� ������
  TPalette palette[colorsNum]; // �������
  // �������� ���������� ������� ������ � ������� ��� ������� ����� �������
  int mr[colorsNum];
  for (int i = 0; i < colorsNum; i++) {
    palette[i].Red = 0;
    palette[i].Green = 0;
    palette[i].Blue = 0;
    mr[i] = 0;
  }
  int addedColors = 0; // ���������� ����������� ������ � �������
  // ���� �� �������� ������� (��� ������� 30-�� �������, ����� �������� ������� ���������)
  for (int i = 0; i < imgSize; i += 3 * 30) {
    TPalette c = *((TPalette *) &img[i]);
    bool foundAlike = false; // = true, ���� ������ ������� ���� � �������
    for (int j = 0; j < colorsNum; j++)
      if (AlikeColors(c, palette[j])) {
        foundAlike = true;
        break;
      }
    if (foundAlike)
      continue;
    int r = 0; // ���������� ������� ������ � �������
    // ���������� ���� �������� ������� �� ����� ���������� � �������
    for (int k = i + 3; k < imgSize; k += 3)
      if (AlikeColors(c, *((TPalette *) &img[k])))
        r++;
    if (addedColors < colorsNum) { // ���� ������� ��� �� ��������� ���������
      palette[addedColors] = c;
      mr[addedColors] = r;
      addedColors++;
    } else {
      /* ���� � ������� ����, ������� ����������� ���� �������� � �������
         �������� ���������� �� ��������, � �������� ��� �� ������� */
      int minDifference = INT_MAX; // ����������� ��������
      int k = -1;
      for (int j = 0; j < colorsNum; j++)
        if (r > mr[j]) {
          int difference = Difference(c, palette[j]);
          if (difference < minDifference) {
            minDifference = difference;
            k = j;
          }
        }
      if (k != -1) {
        palette[k] = c;
        mr[k] = r;
      }
    }
  }

  TBMPHeader BMPHeader;
  BMPHeader.bftype = 19778;
//  BMPHeader.bfsize = ������ �����
  BMPHeader.bfoffbits = 118;
  BMPHeader.bisize = 40;
  BMPHeader.biwidth = width;
  int _w = BMPHeader.biwidth; // �������� ������ ������ �����������
  if (BMPHeader.biwidth % 2 == 1)
    BMPHeader.biwidth++; // �.�. ������ �������� � ������ ����� 551
  BMPHeader.biheight = height;
  BMPHeader.biplanes = 1;
  BMPHeader.bibitcount = 4;
  BMPHeader.bicompression = 0;
  BMPHeader.bisizeimage = BMPHeader.biheight * BMPHeader.biwidth / 2;
  BMPHeader.bix = 0;
  BMPHeader.biy = 0;
  BMPHeader.biclrused = 16;
  FILE *f = fopen(fileName, "wb");
  fwrite(&BMPHeader, sizeof(BMPHeader), 1, f); // ���������� ��������� � �������� ����
  char r = 0;
  // ���������� ������� � �������� ����
  for (int i = 0; i < colorsNum; i++) {
    fwrite(&palette[i].Blue, 1, 1, f);
    fwrite(&palette[i].Green, 1, 1, f);
    fwrite(&palette[i].Red, 1, 1, f);
    fwrite(&r, 1, 1, f); // ���������� ������ ���� (����� ������ ������� � BMP-������)
  }

  /* ����������� ����������� ������ � 16-������� ������, �.�. ������ ���� ������
     RGB ����� �������� ����� (4 ����) � ������ �� 0 �� 15 */
  int h = BMPHeader.biheight;
  int w = BMPHeader.biwidth;
  char q;
  // ���������� � BMP-���� ������� ������ ������ �����������
  for (int i = h - 1; i >= 0; i--)
    for (int j = 0; j < w; j++) {
      int pos = (i * _w + j) * 3;
      TPalette c = *((TPalette *) &img[pos]);
      int minDifference = INT_MAX;
      int k;
      // �������, ����� ���� ������� �� 0 �� 15 �������� ������ ����� ������� RGB
      for (int b = 0; b < colorsNum; b++) {
        int difference = Difference(c, palette[b]);
        if (difference < minDifference) {
          minDifference = difference;
          k = b;
        }
      }
      if (pos % 2 == 0) { // ���������� � ���� �������� ����������� � ���� ����
         q = k;
         q <<= 4;
      }
      else {
        q |= k;
        fwrite(&q, 1, 1, f);
      }
    }
  fclose(f);
}

// ������� 16-������� BMP-���� �� �����
void Show16BMP(const char *fileName, int x, int y)
{
  FILE *f = fopen(fileName, "rb");
  TBMPHeader header;
  fread(&header, sizeof(header), 1, f); // ������ ���������
  int width = header.biwidth; // ������
  int height = header.biheight; // ������
  unsigned char palitra[16][4];
  fread(palitra, sizeof(palitra), 1, f); // ��������� �������
  int offset = Offset(width / 2);
  byte c, c1, c2;
  for (int i = height - 1; i >= 0; i--) {
    for (int j = 0; j < width / 2; j++) {
      fread(&c, 1, 1, f); // ��������� ���� ���� �������� � ����� �����
      c1 = c >> 4; // �������� ���� ������� �������
      // ���������� ������ ������� � ����
      putpixel(x + j * 2, y + i, RGB(palitra[c1][2], palitra[c1][1], palitra[c1][0]));
      c2 = c & 0x0F; // �������� ���� ������� �������
      // ���������� ������ ������� � ����
      putpixel(x + j * 2 + 1, y + i, RGB(palitra[c2][2], palitra[c2][1], palitra[c2][0]));
    }
    fseek(f, offset, SEEK_CUR); // ���������� ������ ����� � ����� ������
  }
  fclose(f);
}

void main()
{
  const char inFileName[] = "cat256.bmp"; // ������� ����
  const char outFileName[] = "16.bmp"; // �������� ����
  FILE *inFile, *outFile;
  char *buf;
  int bufSize;
  inFile = fopen(inFileName, "rb");
  fseek(inFile, 0, SEEK_END);
  bufSize = ftell(inFile);
  buf = new char[bufSize];
  fseek(inFile, 0, SEEK_SET);
  fread(buf, 1, bufSize, inFile); // ���������� ��� ���������� �� �������� ����� � �����
  char *img;
  int imgSize;
  Read256BMPFile(buf, bufSize, &img, imgSize);
  fclose(inFile);
  TBMPHeader *header = (TBMPHeader *) buf;
  int width = header->biwidth; // ������
  int height = header->biheight; // ������
  resize(width * 2 + 20, height + 20);
  ShowPicture(img, width, height, 5, 5);
  Create16BMP(img, imgSize, width, height, outFileName);
  Show16BMP(outFileName, width + 10, 5);
}

