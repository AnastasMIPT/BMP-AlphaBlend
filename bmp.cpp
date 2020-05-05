#include <stdio.h>
#include <cstdlib>

struct BITMAPFILEHEADER
{
    BITMAPFILEHEADER (char* bf);
    unsigned int bfType;
    unsigned int bfSize;
    unsigned short int bfReserved1;
    unsigned short int bfReserved2;
    unsigned int bfOffBits;
};


class bmp
{
private:
    char* bf;
    unsigned int size_bf;
    BITMAPFILEHEADER* header;
    char* image;
public:
    
    bmp (const char* path);
    void get_bf (FILE* f_in);
    char* get_image ();
    BITMAPFILEHEADER* get_header ();
    
    ~bmp();
};




int main () {
    bmp kotik ("kotik.bmp");
    printf ("%u\n", kotik);
    return 0;
}


char* bmp::get_image () {
    return image;
}

BITMAPFILEHEADER* bmp::get_header () {
    return header;
}

bmp::bmp(const char* path)
{
    FILE* f_in = fopen (path, "rb");
    get_bf (f_in);
    header = new BITMAPFILEHEADER (bf);
    image = (bf + header->bfOffBits);
}

bmp::~bmp()
{
    delete[] (bf);
    delete header;
}

void bmp::get_bf (FILE* f_in) {
    fseek(f_in, 0, SEEK_END);
    size_bf = ftell(f_in);
    rewind(f_in);
    
    
    bf = new char [size_bf + 1];

    fread(bf, sizeof (char), size_bf, f_in);

    
}

BITMAPFILEHEADER::BITMAPFILEHEADER (char* bf) {
        bfType = *(unsigned int*) bf;
        bfSize = *(unsigned int*) (bf + 2);
        bfReserved1 = *(unsigned short int*) (bf + 6);
        bfReserved2 = *(unsigned short int*) (bf + 8);
        bfOffBits   = *(unsigned int*) (bf + 10);
}
