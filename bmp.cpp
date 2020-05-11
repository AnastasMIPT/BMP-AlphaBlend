#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <cstring>

struct BITMAPFILEHEADER
{
    BITMAPFILEHEADER (char* bf);
    unsigned int bfType;
    unsigned int bfSize;
    unsigned short int bfReserved1;
    unsigned short int bfReserved2;
    unsigned int bfOffBits;
};


struct pixel {
    u_char blue;
    u_char green;
    u_char red;
    u_char alpha;    
};



class bmp
{
private:
    char* bf;
    uint size_bf;
    BITMAPFILEHEADER* header;
    pixel* image;
    uint width;
    uint height;
public:
    
    bmp (const char* path);
    void get_bf (FILE* f_in);
    pixel* get_image () const;
    BITMAPFILEHEADER* get_header () const;
    void load_to_image (const char* path) const;
    uint get_height () const;
    uint get_width () const;
    void alpha_blend (const bmp& front, const char* path_result = "blend_result.bmp");

    ~bmp();
};

pixel blend_pixels_x1 (pixel& src, pixel& dst) {
    if (src.alpha == 0) return dst;
    float f_alpha = (float)((double)(src.alpha)* (1.0 / 255.0));	
    float not_a = 1.0f - f_alpha;
    //printf ("%f\n", not_a);
    pixel res;
    res.blue =  lround (float(dst.blue) * not_a) + src.blue;	
    res.green = lround (float(dst.green) * not_a) + src.green;
    res.red =   lround (float(dst.red) * not_a) + src.red;
    res.alpha=  lround (float(dst.alpha) * not_a) + src.alpha;
    //printf ("res.alpha = %u src.alpha = %u lround = %u\n", res.alpha, src.alpha, lround(float(dst.alpha) * not_a));
    if (res.alpha != 255) printf ("res_alpha = %u\n", res.alpha); 
    return res;	
}
int main () {
    bmp kotik ("AskhatCat.bmp");
    bmp background ("back.bmp");
    background.alpha_blend (kotik);
    
    return 0;
}

void bmp::alpha_blend (const bmp& front, const char* path_result) {
    pixel* kotik_i = front.get_image ();
    
    unsigned int f_width = front.get_width ();
    unsigned int f_height = front.get_height ();
    unsigned int oft = 0;
    unsigned int oft2 = 0;
    uint oft_start = 1920 * 100 + 600;
    for (unsigned int i = 0; i < abs (f_height); ++i) {
        for (unsigned int j = 0; j < abs (f_width); ++j) {
            oft = i * f_width;
            oft2 = i * width;
            image[oft2 + oft_start + j] = blend_pixels_x1 (kotik_i[oft + j], image[oft2 + oft_start + j]);
        
        }
    }
    load_to_image (path_result);

}

pixel* bmp::get_image () const {
    return image;
}

BITMAPFILEHEADER* bmp::get_header () const{
    return header;
}

bmp::bmp (const char* path)
{
    FILE* f_in = fopen (path, "rb");
    get_bf (f_in);
    header = new BITMAPFILEHEADER (bf);
    image = (pixel*) (bf + header->bfOffBits);
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

void bmp::load_to_image (const char* path) const {
    FILE* f_out = fopen (path, "wb");
    if (!f_out) {
        const char * command = strcat ("touch ", path);
        system (command);
        f_out = fopen (path, "wb");
    }
    
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