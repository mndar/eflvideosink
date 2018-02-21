CC = gcc
CFLAGS = -c

DEPS = efl_convert.h eflvideosink.h
SOURCES = efl_alloc.c  efl_convert.c  eflvideosink.c
CFLAGS += `pkg-config --cflags gstreamer-1.0 gstreamer-video-1.0  glib-2.0 evas ecore eina`
LDFLAGS += `pkg-config --libs gstreamer-1.0 gstreamer-video-1.0 glib-2.0 evas ecore eina`
OBJ = $(SOURCES:.c=.o)

%.o : %.c $(DEPS)
	$(CC) -c -fPIC -o $@ $< $(CFLAGS)

libgsteflvideosink.so: $(OBJ)
	$(CC) -shared -o $@ $^ $(LDFLAGS)

clean:
	rm *.o libgsteflvideosink.so
	
