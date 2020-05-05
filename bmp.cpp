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
    unsigned int width;
    unsigned int height;
public:
    
    bmp (const char* path);
    void get_bf (FILE* f_in);
    char* get_image ();
    BITMAPFILEHEADER* get_header ();
    void load_to_image (const char* path);
    unsigned int get_height () const;
    unsigned int get_width () const;
    
    ~bmp();
};




int main () {
    bmp kotik ("kotik.bmp");
    bmp background ("back.bmp");
    char* kotik_i = kotik.get_image ();
    char* back_i = background.get_image ();
    printf ("w = %u h = %u\n", kotik.get_width (), kotik.get_height ());
    

    background.load_to_image ("kot(1).bmp");

    return 0;
}


char* bmp::get_image () {
    return image;
}

BITMAPFILEHEADER* bmp::get_header () {
    return header;
}

bmp::bmp (const char* path)
{
    FILE* f_in = fopen (path, "rb");
    get_bf (f_in);
    header = new BITMAPFILEHEADER (bf);
    image = (bf + header->bfOffBits);
    width  = *((unsigned int*) (bf + 18));
    height = *((unsigned int*) (bf + 22));
    fclose (f_in);
}

bmp::~bmp ()
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

void bmp::load_to_image (const char* path) {
    FILE* f_out = fopen (path, "wb");
    fwrite (bf, sizeof (char), size_bf, f_out);
    fclose (f_out);
}

BITMAPFILEHEADER::BITMAPFILEHEADER (char* bf) {
        bfType = *(unsigned int*) bf;
        bfSize = *(unsigned int*) (bf + 2);
        bfReserved1 = *(unsigned short int*) (bf + 6);
        bfReserved2 = *(unsigned short int*) (bf + 8);
        bfOffBits   = *(unsigned int*) (bf + 10);
}

unsigned int bmp::get_height () const {
    return height;
}

unsigned int bmp::get_width () const {
    return width;
}