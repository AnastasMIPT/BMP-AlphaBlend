#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cassert>
#include <emmintrin.h>
#include <pmmintrin.h>


struct BITMAPFILEHEADER
{
    BITMAPFILEHEADER (char* bf);
    unsigned int       bfType;
    unsigned int       bfSize;
    unsigned short int bfReserved1;
    unsigned short int bfReserved2;
    unsigned int       bfOffBits;
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
    
    bmp                          (const char* path);
    void get_bf                  (FILE* f_in);
    pixel* get_image             () const;
    BITMAPFILEHEADER* get_header () const;
    void load_to_image           (const char* path) const;
    uint get_height              () const;
    uint get_width               () const;
    void alpha_blend             (const bmp& front, unsigned int pos_x = 0, unsigned int pos_y = 0, 
                                  const char* path_result = "blend_result.bmp");
    void alpha_blend_fast        (const bmp& front, unsigned int pos_x = 0, unsigned int pos_y = 0,
                                  const char* path_result = "blend_result.bmp");
    ~bmp                         ();
};


int main () {
    bmp kotik      ("kotik.bmp");
    bmp background ("back.bmp");
    
    background.alpha_blend_fast (kotik, 600, 100);
    return 0;
}

void blend_pixels_x4 (__m128i* front, __m128i* back) {
    __m128i front_pxls = _mm_lddqu_si128 (front);
    _mm_storeu_si128 (back, front_pxls);
}


void bmp::alpha_blend_fast (const bmp& front, unsigned int pos_x, unsigned int pos_y,
                            const char* path_result) {
    pixel* front_i = front.get_image ();
    
    uint f_width  = front.get_width  ();
    uint f_height = front.get_height ();

    if (f_width > width || f_height > height) {
        printf ("ERROR in alpha_blend(): necessary front_width <= back_width && front_height <= back_height!\n");
        return;
    }

    if (pos_x > width || pos_y > height) {
        printf ("ERROR in alpha_blend(): necessary pos_x <= back_width && pos_y <= back_height!\n");
        return;
    }

    uint oft       = 0;
    uint oft2      = 0;
    uint oft_start = width * pos_y + pos_x;
    
    for (uint i = 0; i < f_height; ++i) {
        for (uint j = 0; j < f_width; j += 4) {
            oft  = i * f_width;
            oft2 = i * width;
            //printf ("i = %u\n", i);
            blend_pixels_x4 ((__m128i*) (front_i + oft + j),
                             (__m128i*) (image + oft2 + oft_start + j));
        
        }
    }
    
    load_to_image (path_result);


}



pixel blend_pixels_x1 (pixel& src, pixel& dst) {
    if (src.alpha == 0) return dst;
    
    float f_alpha = (float)((double)(src.alpha) * (1.0 / 255.0));	
    float not_a   = 1.0f - f_alpha;
    
    pixel res;
    
    res.blue  = lround (float (dst.blue)  * not_a) + src.blue;	
    res.green = lround (float (dst.green) * not_a) + src.green;
    res.red   = lround (float (dst.red)   * not_a) + src.red;
    res.alpha = lround (float (dst.alpha) * not_a) + src.alpha;
    
    return res;	
}

void bmp::alpha_blend (const bmp& front, unsigned int pos_x, unsigned int pos_y, const char* path_result) {
    pixel* front_i = front.get_image ();
    
    uint f_width  = front.get_width  ();
    uint f_height = front.get_height ();

    if (f_width > width || f_height > height) {
        printf ("ERROR in alpha_blend(): necessary front_width <= back_width && front_height <= back_height!\n");
        return;
    }

    if (pos_x > width || pos_y > height) {
        printf ("ERROR in alpha_blend(): necessary pos_x <= back_width && pos_y <= back_height!\n");
        return;
    }

    uint oft       = 0;
    uint oft2      = 0;
    uint oft_start = width * pos_y + pos_x;
    
    for (uint i = 0; i < f_height; ++i) {
        for (uint j = 0; j < f_width; ++j) {
            oft  = i * f_width;
            oft2 = i * width;
            image[oft2 + oft_start + j] = blend_pixels_x1 (front_i[oft + j], image[oft2 + oft_start + j]);
        
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
    image  = (pixel*) (bf + header->bfOffBits);
    
    width  = *((uint*) (bf + 18));
    height = *((uint*) (bf + 22));
    
    fclose (f_in);
}

bmp::~bmp ()
{
    delete[] bf;
    delete   header;
}

void bmp::get_bf (FILE* f_in) {
    
    fseek  (f_in, 0, SEEK_END);
    size_bf = ftell (f_in);
    rewind (f_in);
    
    bf = new char [size_bf + 1];

    fread (bf, sizeof (char), size_bf, f_in);    
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
        bfType      = *(unsigned int*)        bf;
        bfSize      = *(unsigned int*)       (bf + 2);
        bfReserved1 = *(unsigned short int*) (bf + 6);
        bfReserved2 = *(unsigned short int*) (bf + 8);
        bfOffBits   = *(unsigned int*)       (bf + 10);
}

uint bmp::get_height () const {
    return height;
}

uint bmp::get_width  () const {
    return width;
}