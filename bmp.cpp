#include <stdio.h>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cassert>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <ctime>
#include <iostream>

#pragma GCC diagnostic error "-Wold-style-cast"

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

    bmp                          (const bmp& image) = delete;
    bmp& operator=               (const bmp& image) = delete;

    void get_bf                  (FILE* f_in);
    pixel* get_image             () const;
    BITMAPFILEHEADER* get_header () const;
    uint get_height              () const;
    uint get_width               () const;

    void load_to_image           (const char* path) const;
    
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
    
   
}



void blend_pixels_x4 (__m128i* front, __m128i* back) {

    const static __m128i zero       = _mm_set_epi8 (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    const static __m128i alpha      = _mm_set_epi8 (255, 14, 255, 14, 255, 14, 255, 14,
                                                    255,  6, 255,  6, 255,  6, 255,  6);
    const static __m128i extend_255 = _mm_set_epi8 (0,  255, 0, 255, 0, 255, 0, 255,
                                                    0,  255, 0, 255, 0, 255, 0, 255);

    __m128i front_pxls = _mm_lddqu_si128 (front);       //4 pixels in 16 bytes
    __m128i back_pxls  = _mm_lddqu_si128 (back);
    
    __m128i low_pixels_b = reinterpret_cast<__m128i> (_mm_movelh_ps (reinterpret_cast<__m128> (back_pxls), 
                                                                     reinterpret_cast<__m128> (zero)));  // zero in upper 8 bytes and 2 pixels in lower 8 bytes
    
    __m128i low_pixels_f = reinterpret_cast<__m128i> (_mm_movelh_ps (reinterpret_cast<__m128> (front_pxls),
                                                                     reinterpret_cast<__m128> (zero)));


    // extend low pixels for correct multiplication
    low_pixels_b = _mm_cvtepu8_epi16 (low_pixels_b);
    low_pixels_f = _mm_cvtepu8_epi16 (low_pixels_f);
    

    __m128i low_alpha_f =  _mm_shuffle_epi8 (low_pixels_f, alpha);
    
   
    low_pixels_b = _mm_mullo_epi16 (low_pixels_b, _mm_sub_epi16   (extend_255,   low_alpha_f));
    low_pixels_b = _mm_add_epi16   (low_pixels_b, _mm_mullo_epi16 (low_pixels_f, low_alpha_f));
    low_pixels_b = _mm_srli_epi16  (low_pixels_b, 8);
    
    low_pixels_b = _mm_shuffle_epi8 (low_pixels_b, _mm_set_epi8 (255, 255, 255, 255, 255, 255, 255, 255,
                                                                 14, 12, 10, 8, 6, 4, 2, 0));





    __m128i up_pixels_b  = reinterpret_cast<__m128i> (_mm_movehl_ps (reinterpret_cast<__m128> (zero), 
                                                                     reinterpret_cast<__m128> (back_pxls)));  

    __m128i up_pixels_f  = reinterpret_cast<__m128i> (_mm_movehl_ps (reinterpret_cast<__m128> (zero),
                                                                     reinterpret_cast<__m128> (front_pxls)));

    // extend low pixels for correct multiplication   
    up_pixels_b  = _mm_cvtepu8_epi16 (low_pixels_b);
    up_pixels_f  = _mm_cvtepu8_epi16 (low_pixels_f);

    __m128i up_alpha_f  = _mm_shuffle_epi8 (up_pixels_f,  alpha);


    up_pixels_b  = _mm_mullo_epi16 (up_pixels_b, _mm_sub_epi16   (extend_255, up_alpha_f));
    up_pixels_b  = _mm_add_epi16   (up_pixels_b, _mm_mullo_epi16 (up_pixels_f, up_alpha_f));
    up_pixels_b  = _mm_srli_epi16  (up_pixels_b, 8);

    up_pixels_b  = _mm_shuffle_epi8 (up_pixels_b,  _mm_set_epi8 (14, 12, 10, 8, 6, 4, 2, 0,
                                                                 255, 255, 255, 255, 255, 255, 255, 255));  
    
    
    back_pxls   = _mm_or_si128 (up_pixels_b, low_pixels_b);      

    _mm_storeu_si128 (back, back_pxls);
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
            blend_pixels_x4 (reinterpret_cast<__m128i*> ((front_i + oft + j)),
                             reinterpret_cast<__m128i*> ((image + oft2 + oft_start + j)));
        
        }
    }
    
    load_to_image (path_result);


}



pixel blend_pixels_x1 (pixel& src, pixel& dst) {
    if (src.alpha == 0) return dst;
    
    float f_alpha = static_cast<float> ((static_cast<float> (src.alpha) * (1.0 / 255.0)));	
    float not_a   = 1.0f - f_alpha;
    
    pixel res;
    
    res.blue  = lround (static_cast<float> (dst.blue)  * not_a) + src.blue;	
    res.green = lround (static_cast<float> (dst.green) * not_a) + src.green;
    res.red   = lround (static_cast<float> (dst.red)   * not_a) + src.red;
    res.alpha = lround (static_cast<float> (dst.alpha) * not_a) + src.alpha;
    
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
    image  = reinterpret_cast<pixel*> (bf + header->bfOffBits);
    

    width  = *(reinterpret_cast<uint*> (bf + 18));
    height = *(reinterpret_cast<uint*> (bf + 22));
    
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
        bfType      = *reinterpret_cast<uint*>               (bf);
        bfSize      = *reinterpret_cast<uint*>               (bf + 2);
        bfReserved1 = *reinterpret_cast<unsigned short int*> (bf + 6);
        bfReserved2 = *reinterpret_cast<unsigned short int*> (bf + 8);
        bfOffBits   = *reinterpret_cast<uint*>               (bf + 10);
}

uint bmp::get_height () const {
    return height;
}

uint bmp::get_width  () const {
    return width;
}