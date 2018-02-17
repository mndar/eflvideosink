#ifndef __EFL_GSTREAMER_H__
#define __EFL_GSTREAMER_H__
#include <glib.h>
#include <gst/gst.h>
#include <glib-object.h>
#include <gst/video/gstvideosink.h>
#include <gst/video/video.h>
#include <gst/tag/tag.h>
#include <gst/pbutils/pbutils.h>

#include <unistd.h>
#include <fcntl.h>

#include <Eina.h>
#include <Evas.h>
#include <Ecore.h>
#include <Efl.h>
#include "efl_convert.h"

#define EINA_DBL_EQ(a, b) (!!(fabs((double)a - (double)b) <= DBL_EPSILON))

typedef struct _EflVideoSinkPrivate EflVideoSinkPrivate;
typedef struct _EflVideoSink        EflVideoSink;
typedef struct _EflVideoSinkClass   EflVideoSinkClass;
typedef struct _Efl_Gstreamer_Buffer Efl_Gstreamer_Buffer;
typedef struct _Efl_Gstreamer_Message Efl_Gstreamer_Message;
typedef struct _Efl_Gstreamer Efl_Gstreamer;


Efl_Gstreamer_Buffer *efl_gstreamer_buffer_alloc(EflVideoSink *sink,
                                                         GstBuffer *buffer,
                                                         GstVideoInfo *info,
                                                         Evas_Colorspace eformat,
                                                         int eheight,
                                                         Evas_Video_Convert_Cb func);
void efl_gstreamer_buffer_free(Efl_Gstreamer_Buffer *send);

struct _Efl_Gstreamer_Message
{
   Efl_Gstreamer *ev;

   GstMessage *msg;
};

struct _Efl_Gstreamer
{

};

#define EFL_TYPE_VIDEO_SINK efl_video_sink_get_type()

#define EFL_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    EFL_TYPE_VIDEO_SINK, EflVideoSink))

#define EFL_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), \
    EFL_TYPE_VIDEO_SINK, EflVideoSinkClass))

#define EFL_IS_VIDEO_SINK(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
    EFL_TYPE_VIDEO_SINK))

#define EFL_IS_VIDEO_SINK_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), \
    EFL_TYPE_VIDEO_SINK))

#define EFL_VIDEO_SINK_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), \
    EFL_TYPE_VIDEO_SINK, EflVideoSinkClass))

struct _EflVideoSink {
    /*< private >*/
    GstVideoSink parent;
    EflVideoSinkPrivate *priv;
};

struct _EflVideoSinkClass {
    /*< private >*/
    GstVideoSinkClass parent_class;
};

struct _EflVideoSinkPrivate {
   Evas_Object *efl_object;
   Evas_Object *evas_object;
   
  GstVideoInfo info;
   unsigned int eheight;
   Evas_Colorspace eformat;
   Evas_Video_Convert_Cb func;

   Eina_Lock m;
   Eina_Condition c;

   Efl_Gstreamer_Buffer *send;

    /* We need to keep a copy of the last inserted buffer as evas doesn't copy YUV data around */
   GstBuffer        *last_buffer;
   GstMapInfo        map_info;

   GstVideoFrame last_vframe;

   int frames;
   int flapse;
   double rtime;
   double rlapse;

   // If this is TRUE all processing should finish ASAP
   // This is necessary because there could be a race between
   // unlock() and render(), where unlock() wins, signals the
   // GCond, then render() tries to render a frame although
   // everything else isn't running anymore. This will lead
   // to deadlocks because render() holds the stream lock.
   //
   // Protected by the buffer mutex
   Eina_Bool unlocked : 1;
   Eina_Bool mapped : 1;
   Eina_Bool vfmapped : 1;
};

struct _Efl_Gstreamer_Buffer
{
   GstVideoFrame vframe;
   EflVideoSink *sink;
   GstBuffer *frame;
   GstVideoInfo info;
   Evas_Video_Convert_Cb func;
   Evas_Colorspace eformat;
   int eheight;
   Eina_Bool vfmapped : 1;
};

GType efl_video_sink_get_type (void);
#endif /* __EFL_GSTREAMER_H__ */

