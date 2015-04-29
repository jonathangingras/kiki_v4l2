GTK_LIBS = `pkg-config --libs gtk+-3.0`
CV_LIBS = -lopencv_core -lopencv_imgproc
LIBS = $(GTK_LIBS) $(CV_LIBS)

CFLAGS = -g -O1 -O2 -O3

SRC = src
BIN = bin
INCLUDES = `pkg-config --cflags gtk+-3.0` -I./include
TARGET = $(BIN)/mirror $(BIN)/cam_show

all: $(BIN) $(TARGET)

$(BIN):
	mkdir -p $(BIN)

$(BIN)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $?

$(BIN)/mirror: example/mirror_gtk.cc $(BIN)/kiki_v4l2.o $(BIN)/yuv422_to_rgb888.o
	$(CXX) $(CFLAGS) $(INCLUDES) -o $@ $? $(LIBS)

$(BIN)/cam_show: example/camera_show_gtk.c $(BIN)/kiki_v4l2.o $(BIN)/yuv422_to_rgb888.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $? $(GTK_LIBS)

clean:
	rm -rf $(BIN)