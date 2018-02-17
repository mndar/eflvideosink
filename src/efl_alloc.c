#include "eflvideosink.h"

Efl_Gstreamer_Buffer *
efl_gstreamer_buffer_alloc(EflVideoSink *sink,
                               GstBuffer *buffer,
                               GstVideoInfo *info,
                               Evas_Colorspace eformat,
                               int eheight,
                               Evas_Video_Convert_Cb func)
{
   Efl_Gstreamer_Buffer *send;
	DBG ("Evas Object Buffer Alloc Function: %p", sink->priv->evas_object);
   if (!sink->priv->evas_object) {
   	return NULL;
 	}

   send = calloc(1, sizeof(Efl_Gstreamer_Buffer));
   if (!send) {
   	return NULL;
 	}

   send->sink = gst_object_ref(sink);
   send->frame = gst_buffer_ref(buffer);
   send->info = *info;
   DBG ("Trying To Render Frame Of Size: %dx%d", info->width, info->height);
   if (gst_video_frame_map(&(send->vframe), info, buffer, GST_MAP_READ)) {
     send->vfmapped = EINA_TRUE;
   }
   else {
   	g_critical ("Could Not Map Video Frame");
     send->vfmapped = EINA_FALSE;
   }
   send->eformat = eformat;
   send->eheight = eheight;
   send->func = func;
   return send;
}

void
efl_gstreamer_buffer_free(Efl_Gstreamer_Buffer *send)
{
   gst_object_unref(send->sink);
   gst_buffer_replace(&send->frame, NULL);
   free(send);
}
/*
Efl_Gstreamer_Message *
efl_gstreamer_message_alloc(Efl_Gstreamer *ev,
                                GstMessage *msg)
{
   Efl_Gstreamer_Message *send;

   if (!ev) return NULL;

   send = malloc(sizeof (Efl_Gstreamer_Message));
   if (!send) return NULL;

   send->ev = efl_gstreamer_ref(ev);
   send->msg = gst_message_ref(msg);

   return send;
}*/

/*
void
efl_gstreamer_message_free(Efl_Gstreamer_Message *send)
{
   efl_gstreamer_unref(send->ev);
   gst_message_unref(send->msg);
   free(send);
}*/
