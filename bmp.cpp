#include <stdio.h>
#include <cstdlib>
#include <cmath>

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
    printf ("w = %u h = %u BW = %u\n", kotik.get_width (), kotik.get_height (), background.get_width ());
    unsigned int width = kotik.get_width ();
    unsigned int height = kotik.get_height ();
    unsigned int width2 = background.get_width ();
    unsigned int oft = 0;
    unsigned int oft2 = 0;
    for (unsigned int i = 0; i < abs (height); ++i) {
        for (unsigned int j = 0; j < abs (width * 4); j += 4) {
        oft = i * width * 4;
        oft2 = i * width2 * 4;
        for (unsigned int k = 0; k < 4; ++k) {
            if (kotik_i[oft + j + 3] == 0) break;
            double a = (double) (((double) kotik_i[oft + j + 3]) * (1.00/ 255.00));
            double not_a = 1.00 - a;
            //back_i[oft2 + j + k] = (back_i[oft2 + j + k] * (0xFF - kotik_i[oft + j + 3]) + back_i[oft2 + j + k] * kotik_i[oft + j + 3]) >> 8;
            back_i[oft2 + j + k] = double (back_i[oft2 + j + k] * not_a) + kotik_i[oft + j + k];
        }
        //printf ("i = %u j = %u\n", i, j);
        // back_i[oft2 + j + 0] = 0;
        // back_i[oft2 + j + 1] = 128;
        // back_i[oft2 + j + 2] = 255;
        // back_i[oft2 + j + 3] = 255;

        }
    }

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