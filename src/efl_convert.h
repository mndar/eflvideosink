#include <gst/video/video.h>
#include <Eina.h>
#include <Evas.h>
#define ERR GST_ERROR
#define DBG GST_DEBUG
#define MSG GST_DEBUG
#define INF	GST_INFO

typedef struct _Efl_Convert_Info Efl_Convert_Info;

struct _Efl_Convert_Info
{
   unsigned int bpp[4];
   unsigned int stride[4];
   unsigned char *plane_ptr[4];
};

typedef void (*Evas_Video_Convert_Cb)(unsigned char *evas_data,
                                      const unsigned char *gst_data,
                                      unsigned int w,
                                      unsigned int h,
                                      unsigned int output_height,
                                      Efl_Convert_Info *info);
                                      
typedef struct _ColorSpace_Format_Convertion ColorSpace_Format_Convertion;

struct _ColorSpace_Format_Convertion
{
   const char *name;
   GstVideoFormat format;
   GstVideoColorMatrix colormatrix;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;
   Eina_Bool force_height;
};

extern const ColorSpace_Format_Convertion colorspace_format_convertion[];

void
_evas_video_bgrx_step(unsigned char *evas_data, const unsigned char *gst_data,
                      unsigned int w, unsigned int h EINA_UNUSED,
                      unsigned int output_height, unsigned int step);
void
_evas_video_bgr(unsigned char *evas_data, const unsigned char *gst_data,
                unsigned int w, unsigned int h, unsigned int output_height,
                Efl_Convert_Info *info EINA_UNUSED);
void
_evas_video_bgrx(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h, unsigned int output_height,
                 Efl_Convert_Info *info EINA_UNUSED);
void
_evas_video_bgra(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Efl_Convert_Info *info EINA_UNUSED);
void
_evas_video_i420(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Efl_Convert_Info *info);
void
_evas_video_yv12(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Efl_Convert_Info *info);
void
_evas_video_yuy2(unsigned char *evas_data, const unsigned char *gst_data,
                 unsigned int w, unsigned int h EINA_UNUSED,
                 unsigned int output_height,
                 Efl_Convert_Info *info EINA_UNUSED);
void
_evas_video_nv12(unsigned char *evas_data,
                 const unsigned char *gst_data EINA_UNUSED,
                 unsigned int w EINA_UNUSED, unsigned int h EINA_UNUSED,
                 unsigned int output_height, Efl_Convert_Info *info);
                 
