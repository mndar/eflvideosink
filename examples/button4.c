#include <Elementary.h>
#include <gst/gst.h>

EAPI_MAIN int
elm_main(int argc, char **argv)
{
	Evas_Object *win;
	if (ecore_evas_init() <= 0)
		return 1;
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	win = elm_win_util_standard_add("Main", "Hello, World!");
	elm_win_autodel_set(win, EINA_TRUE);
	//win 1280x720
	evas_object_resize(win, 1280, 720);

	Evas_Object *box;


	box = elm_box_add(win);
	elm_box_padding_set(box, 16, 6);
	elm_box_horizontal_set(box, EINA_FALSE);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_resize (box, 800, 700);
	elm_win_resize_object_add(win, box);

	//basic text button
	Evas_Object *button;
	button = elm_button_add(win);
	elm_object_text_set(button,"Click me");
	//how a container object should resize a given child within its area
	evas_object_size_hint_weight_set(button, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//how to align an object
	evas_object_size_hint_align_set(button, EVAS_HINT_FILL, 0.5);
	evas_object_resize(button, 100, 30);
	evas_object_show(button);

	Evas_Object *button2;
	button2 = elm_button_add(win);
	elm_object_text_set(button2,"Click me 2");
	//how a container object should resize a given child within its area
	evas_object_size_hint_weight_set(button2, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//how to align an object
	evas_object_size_hint_align_set(button2, EVAS_HINT_FILL, 0.5);
	evas_object_resize(button2, 100, 30);
	evas_object_show(button2);

	Evas_Object *image;
	image = elm_image_add (win);
	evas_object_size_hint_min_set(image, 640, 360); // <--important
	evas_object_resize (image, 640, 360);
	//printf ("Image File Set: %d\n", elm_image_file_set (image, "enlightenment.png", NULL));
	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(image, EVAS_HINT_FILL, 0.5);
	evas_object_show(image);

	Evas_Object *video = elm_image_object_get (image);
	evas_object_size_hint_min_set(video, 640, 360);
	evas_object_image_filled_set (video, EINA_TRUE);

	elm_box_pack_start (box, button);
	elm_box_pack_start (box, button2);
	elm_box_pack_start (box, video); 

	evas_object_show(box);
	evas_object_show(win);
    
    
    
	gst_init (&argc, &argv);
	
	GError *error = NULL;
	GstElement *pipeline = gst_parse_launch ("videotestsrc ! video/x-raw,width=320,height=240,format=I420 ! eflvideosink name=eflvideosink", &error);
	//GstElement *pipeline = gst_parse_launch ("rtspsrc location=rtsp://10.168.1.103:10554/udp/av0_0 user-id=admin user-pw=admin ! rtph264depay ! h264parse ! vaapidecode ! eflvideosink name=eflvideosink", &error);
	//GstElement *pipeline = gst_parse_launch ("rtspsrc location=rtsp://10.168.1.64:554/Streaming/Channels/101?transportmode=unicast&profile=Profile_1 user-id=admin user-pw=admin ! rtph264depay ! h264parse ! vaapidecode ! eflvideosink name=eflvideosink", &error);
	//GstElement *pipeline = gst_parse_launch ("rtspsrc location=rtsp://10.168.1.10:554/user=admin_password=tlJwpbo6_channel=1_stream=0.sdp?real_stream user-id=admin user-pw=admin ! rtph264depay ! h264parse ! vaapidecode ! eflvideosink name=eflvideosink", &error);
	
	if (error) {
		g_error ("Cannot Create Pipeline. Error Code:%d Message:%s", error->code, error->message);
		return -1;
	}
	
	GstElement *eflvideosink = gst_bin_get_by_name (GST_BIN (pipeline), "eflvideosink");
	if (!eflvideosink) {
		g_error ("EflVideoSink Could Not Be Created");
		return -1;
	}
	g_object_set (eflvideosink, "efl-object", video, NULL);
	gst_element_set_state (pipeline, GST_STATE_PLAYING);
	elm_run();
	return 0;
}
ELM_MAIN()
