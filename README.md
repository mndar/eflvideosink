# eflvideosink
GStreamer EFL Video Sink
A dynamic version of vtorri's emotion sink in the EFL sources.

Compiling:
The GStreamer sink element:
	cd src
	make -j4

Example:
	cd examples
	gcc -o button4 button4.c `pkg-config --cflags --libs elementary gstreamer-1.0`

Execution:
	GST_PLUGIN_PATH=../src/ ./button4


