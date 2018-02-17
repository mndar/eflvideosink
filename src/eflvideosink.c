#include "eflvideosink.h"

Eina_Bool debug_fps = EINA_FALSE;
static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK, GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS(GST_VIDEO_CAPS_MAKE("{ I420, YV12, YUY2, NV12, BGRx, BGR, BGRA }"))); 

GST_DEBUG_CATEGORY_STATIC(efl_video_sink_debug);
#define GST_CAT_DEFAULT efl_video_sink_debug

enum {
  LAST_SIGNAL
};

enum {
  PROP_0,
  PROP_EFL_OBJECT,
  PROP_LAST
};

#define _do_init GST_DEBUG_CATEGORY_INIT(efl_video_sink_debug, "efl_video_sink", 0, "efl video sink")
#define parent_class efl_video_sink_parent_class
G_DEFINE_TYPE_WITH_CODE (EflVideoSink, efl_video_sink, GST_TYPE_VIDEO_SINK, _do_init);

static void efl_video_sink_main_render(void *data);

static void
efl_video_sink_init(EflVideoSink* sink)
{
	EflVideoSinkPrivate* priv;

	GST_DEBUG("sink init");
	sink->priv = priv = G_TYPE_INSTANCE_GET_PRIVATE(sink, EFL_TYPE_VIDEO_SINK, EflVideoSinkPrivate);
	gst_video_info_init (&priv->info);
	priv->eheight = 0;
	priv->func = NULL;
	
	priv->eformat = EVAS_COLORSPACE_ARGB8888;
	eina_lock_new(&priv->m);
	eina_condition_new(&priv->c, &priv->m);
	priv->unlocked = EINA_FALSE;
}

static void
_cleanup_priv(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	EflVideoSinkPrivate * priv;

	priv = data;

	eina_lock_take(&priv->m);
	if (priv->evas_object == obj)
		priv->evas_object = NULL;
	eina_lock_release(&priv->m);
}

static void
efl_video_sink_set_property(GObject * object, guint prop_id,
                             const GValue * value, GParamSpec * pspec)
{
	EflVideoSink* sink;
	EflVideoSinkPrivate* priv;

	sink = EFL_VIDEO_SINK (object);
	priv = sink->priv;

	switch (prop_id) {
	case PROP_EFL_OBJECT:
		eina_lock_take(&priv->m);
		priv->evas_object = g_value_get_pointer (value);
		eina_lock_release(&priv->m);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		GST_DEBUG("invalid property");
		break;
	}
}

static void
efl_video_sink_get_property(GObject * object, guint prop_id,
                             GValue * value, GParamSpec * pspec)
{
	EflVideoSink* sink;
	EflVideoSinkPrivate* priv;

	sink = EFL_VIDEO_SINK (object);
	priv = sink->priv;

	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		GST_DEBUG("invalide property");
		break;
	}
}

static void
efl_video_sink_dispose(GObject* object)
{
}

gboolean efl_video_sink_set_caps(GstBaseSink *bsink, GstCaps *caps)
{
	EflVideoSink* sink;
	EflVideoSinkPrivate* priv;
	GstVideoInfo info;
	unsigned int i;

	sink = EFL_VIDEO_SINK(bsink);
	priv = sink->priv;

	if (!gst_video_info_from_caps(&info, caps))
	{
		DBG("Unable to parse caps.");
		return FALSE;
	}

	priv->info = info;
	priv->eheight = info.height;

	for (i = 0; colorspace_format_convertion[i].name; i++)
	{
		if ((info.finfo->format == colorspace_format_convertion[i].format) &&
		((colorspace_format_convertion[i].colormatrix == GST_VIDEO_COLOR_MATRIX_UNKNOWN) ||
		(colorspace_format_convertion[i].colormatrix == info.colorimetry.matrix)))
		{
			DBG("Found '%s'", colorspace_format_convertion[i].name);
			priv->eformat = colorspace_format_convertion[i].eformat;
			priv->func = colorspace_format_convertion[i].func;
			if (colorspace_format_convertion[i].force_height)
			{
				priv->eheight = (priv->eheight >> 1) << 1;
			}
			return TRUE;
		}
	}

	DBG("unsupported : %s\n", gst_video_format_to_string(info.finfo->format));
	return FALSE;
}

static GstFlowReturn
efl_video_sink_show_frame(GstVideoSink* vsink, GstBuffer* buffer)
{
	Efl_Gstreamer_Buffer *send;
	EflVideoSinkPrivate *priv;
	EflVideoSink *sink;

	GST_DEBUG("sink render %p", buffer);

	sink = EFL_VIDEO_SINK(vsink);
	priv = sink->priv;

	eina_lock_take(&priv->m);
	if (priv->unlocked) {
		GST_DEBUG("LOCKED");
		eina_lock_release(&priv->m);
		return GST_FLOW_FLUSHING;
	}
	send = efl_gstreamer_buffer_alloc(sink, buffer, &priv->info, priv->eformat, priv->eheight, priv->func);
	/* If there still is a pending frame, neutralize it */
	if (priv->send)
	{
		gst_buffer_replace(&priv->send->frame, NULL);
	}
	priv->send = send;
	if (!send) {
		eina_lock_release(&priv->m);
		return GST_FLOW_ERROR;
	}
//	_efl_pending_ecore_begin();
	ecore_main_loop_thread_safe_call_async(efl_video_sink_main_render, send);
	eina_condition_wait(&priv->c);
	eina_lock_release(&priv->m);
	efl_gstreamer_buffer_free (send);
	return GST_FLOW_OK;
}

static void
efl_video_sink_class_init(EflVideoSinkClass* klass)
{
	GObjectClass* gobject_class;
	GstElementClass* gstelement_class;
	GstBaseSinkClass* gstbase_sink_class;
	GstVideoSinkClass* gstvideo_sink_class;

	gobject_class = G_OBJECT_CLASS(klass);
	gstelement_class = GST_ELEMENT_CLASS(klass);
	gstbase_sink_class = GST_BASE_SINK_CLASS(klass);
	gstvideo_sink_class = GST_VIDEO_SINK_CLASS(klass);

	g_type_class_add_private(klass, sizeof(EflVideoSinkPrivate));

	gobject_class->set_property = efl_video_sink_set_property;
	gobject_class->get_property = efl_video_sink_get_property;

	g_object_class_install_property (gobject_class, PROP_EFL_OBJECT,
		                              g_param_spec_pointer ("efl-object", "Efl Object",
		                                                    "The Efl object where the display of the video will be done", G_PARAM_WRITABLE));

	gobject_class->dispose = efl_video_sink_dispose;

	gst_element_class_add_pad_template(gstelement_class, gst_static_pad_template_get(&sinktemplate));
	gst_element_class_set_static_metadata(gstelement_class, "Efl video sink", "Sink/Video",
																												 "Sends video data from a GStreamer pipeline to an Efl object", 
																												 "Vincent Torri <vtorri@univ-evry.fr>, Mandar Joshi <emailmandar@gmail.com>");

	gstbase_sink_class->set_caps = efl_video_sink_set_caps;
	/*gstbase_sink_class->stop = efl_video_sink_stop;
	gstbase_sink_class->start = efl_video_sink_start;
	gstbase_sink_class->unlock = efl_video_sink_unlock;
	gstbase_sink_class->unlock_stop = efl_video_sink_unlock_stop;*/
	gstvideo_sink_class->show_frame = efl_video_sink_show_frame;
}

static void
efl_video_sink_main_render(void *data)
{
	Efl_Gstreamer_Buffer *send;
	EflVideoSinkPrivate *priv;
	GstBuffer *buffer = NULL;
	GstMapInfo map;
	unsigned char *evas_data;
	double ratio;
	Efl_Convert_Info info;
	send = data;
	priv = send->sink->priv;
	eina_lock_take(&priv->m);
	/* Sink was shut down already or this is a stale
	* frame */
	if (priv->send != send) {
		goto exit_point;
	}
	if (!send->frame) {
		goto exit_point;
	}

	priv->send = NULL;
	if (!priv->evas_object || priv->unlocked) {
		goto exit_point;
	}
	/* Can happen if cleanup_priv was called */
	if (priv->evas_object)
	{
		evas_object_event_callback_add(priv->evas_object, EVAS_CALLBACK_DEL, _cleanup_priv, priv);
		evas_object_image_pixels_get_callback_set(priv->evas_object, NULL, NULL);	
	}
	if (!priv->evas_object)
		goto exit_point;
	buffer = gst_buffer_ref(send->frame);
	if (!send->vfmapped)
	{
		if (!gst_buffer_map(buffer, &map, GST_MAP_READ))
		{
			gst_buffer_unref(buffer);
			GST_DEBUG("Cannot map video buffer for read.\n");
			goto exit_point;
		}
	}

	INF ("sink main render [%i, %i] (source height: %i)", send->info.width, send->eheight, send->info.height);

	evas_object_image_alpha_set(priv->evas_object, 0);
	evas_object_image_colorspace_set(priv->evas_object, send->eformat);
	evas_object_image_size_set(priv->evas_object, send->info.width, send->eheight);
	evas_data = evas_object_image_data_get(priv->evas_object, 1);
	if (!evas_data)
	{
		if (!send->vfmapped)
		{
			gst_buffer_unmap(buffer, &map);
			priv->mapped = EINA_FALSE;
		}
		else
		{
			gst_video_frame_unmap(&(send->vframe));
			priv->vfmapped = EINA_FALSE;
		}
		DBG ("Bad Evas Data");
		gst_buffer_unref(buffer);
		goto exit_point;
	}
	// XXX: need to handle GstVideoCropMeta to get video cropping right
	// XXX: can't get crop meta from buffer (always null)
	//   GstVideoCropMeta *meta;
	//   meta = gst_buffer_get_video_crop_meta(buffer);
	//   printf("META: %p\n", meta);

	/* this just is a demo of broken vaapi back-end values for stride and
	* plane offset - the below is what i needed to fix them up for a few videos
	*/
	/*
	info.stride[0] = 64 * ((send->info.stride[0] + 63) / 64);
	info.stride[1] = 64 * ((send->info.stride[1] + 63) / 64);
	info.stride[2] = 64 * ((send->info.stride[2] + 63) / 64);
	info.stride[3] = 64 * ((send->info.stride[3] + 63) / 64);
	info.plane_offset[0] = send->info.offset[0];
	info.plane_offset[1] = (((send->info.height + 15) / 16) * 16) * info.stride[1];
	info.plane_offset[2] = send->info.offset[2];
	info.plane_offset[3] = send->info.offset[3];
	*/
	if (send->vfmapped)
	{
		GstVideoFrame *vframe = &(send->vframe);

		map.data = GST_VIDEO_FRAME_PLANE_DATA(vframe, 0);
		info.bpp[0] = GST_VIDEO_FRAME_COMP_PSTRIDE(vframe, 0);
		info.bpp[1] = GST_VIDEO_FRAME_COMP_PSTRIDE(vframe, 1);
		info.bpp[2] = GST_VIDEO_FRAME_COMP_PSTRIDE(vframe, 2);
		info.bpp[3] = GST_VIDEO_FRAME_COMP_PSTRIDE(vframe, 3);
		info.stride[0] = GST_VIDEO_FRAME_COMP_STRIDE(vframe, 0);
		info.stride[1] = GST_VIDEO_FRAME_COMP_STRIDE(vframe, 1);
		info.stride[2] = GST_VIDEO_FRAME_COMP_STRIDE(vframe, 2);
		info.stride[3] = GST_VIDEO_FRAME_COMP_STRIDE(vframe, 3);
		info.plane_ptr[0] = GST_VIDEO_FRAME_PLANE_DATA(vframe, 0);
		info.plane_ptr[1] = GST_VIDEO_FRAME_PLANE_DATA(vframe, 1);
		info.plane_ptr[2] = GST_VIDEO_FRAME_PLANE_DATA(vframe, 2);
		info.plane_ptr[3] = GST_VIDEO_FRAME_PLANE_DATA(vframe, 3);
	}
	else
	{
		info.bpp[0] = 1;
		info.bpp[1] = 1;
		info.bpp[2] = 1;
		info.bpp[3] = 1;
		info.stride[0] = send->info.stride[0];
		info.stride[1] = send->info.stride[1];
		info.stride[2] = send->info.stride[2];
		info.stride[3] = send->info.stride[3];
		info.plane_ptr[0] = ((unsigned char *)map.data) + send->info.offset[0];
		info.plane_ptr[1] = ((unsigned char *)map.data) + send->info.offset[1];
		info.plane_ptr[2] = ((unsigned char *)map.data) + send->info.offset[2];
		info.plane_ptr[3] = ((unsigned char *)map.data) + send->info.offset[3];
	}


	if (send->func) {
		send->func(evas_data, map.data, send->info.width, send->info.height, send->eheight, &info);
	}
	else {
		g_error("No way to decode %x colorspace !", send->eformat);
	}
	evas_object_image_data_set(priv->evas_object, evas_data);
	evas_object_image_data_update_add(priv->evas_object, 0, 0, send->info.width, send->eheight);
	evas_object_image_pixels_dirty_set(priv->evas_object, 0);
	ratio = (double) send->info.width / (double) send->eheight;
	ratio *= (double) send->info.par_n / (double) send->info.par_d;
	//evas_object_resize(priv->evas_object, send->info.width, send->eheight);
	if (priv->vfmapped)
	{
		gst_video_frame_unmap(&(priv->last_vframe));
	}
	else
	{
	if ((priv->mapped) && (priv->last_buffer))
		gst_buffer_unmap(priv->last_buffer, &(priv->map_info));
	}
	if (send->vfmapped)
	{
		priv->last_vframe = send->vframe;
		priv->vfmapped = EINA_TRUE;
	}
	else
	{
		priv->vfmapped = EINA_FALSE;
		priv->map_info = map;
		priv->mapped = EINA_TRUE;
	}
	if (priv->last_buffer) gst_buffer_unref(priv->last_buffer);
		priv->last_buffer = buffer;

	exit_point:
	if (!priv->unlocked)
		eina_condition_signal(&priv->c);

	eina_lock_release(&priv->m);

}

gboolean
plugin_init (GstPlugin * plugin)
{
	return gst_element_register (plugin,
                               "eflvideosink",
                               GST_RANK_NONE,
                               EFL_TYPE_VIDEO_SINK);
}

#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "EflVideoSink"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "EflVideoSink"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "GStreamer Efl Plugin"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    efl_video_sink,
    "Efl Video Sink",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)
    
