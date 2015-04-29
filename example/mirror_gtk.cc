/* Copyright (c) 2014-2015, Jonathan Gingras
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 */
#include <pthread.h>

#include <kiki_v4l2.h>
#include <gtk/gtk.h>

#include <opencv/cv.h>
#include <opencv2/core/core.hpp>

typedef struct {
  kiki_v4l2_device_handle_t *device_handle;
  raw_t *rgb_image_data;
  GtkImage* image;
  pthread_barrier_t* barrier;
  int *gtk_done;
} image_thread_args;

static void *refresh_image(void *args) {
  raw_t *rgb_image_data = ((image_thread_args*)args)->rgb_image_data;
  kiki_v4l2_device_handle_t *device_handle = ((image_thread_args*)args)->device_handle;
  GtkImage* image = ((image_thread_args*)args)->image;
  int *gtk_done = ((image_thread_args*)args)->gtk_done;
  pthread_barrier_t *barrier = ((image_thread_args*)args)->barrier;

  while(1) {
    kiki_v4l2_capture_frame(device_handle, rgb_image_data);

    cv::Mat flipped_matrix;
    cv::Mat matrix(
      device_handle->image_size.height, 
      device_handle->image_size.width, 
      CV_8UC3, rgb_image_data
    );

    cv::flip(matrix, flipped_matrix, 1);

    GdkPixbuf* pix = gdk_pixbuf_new_from_data (
      (guchar*)flipped_matrix.data,
      GDK_COLORSPACE_RGB,
      FALSE,
      8,
      flipped_matrix.size().width,
      flipped_matrix.size().height,
      flipped_matrix.step,
      NULL,
      NULL
    );

    if(*gtk_done) break;
    gtk_image_set_from_pixbuf(image, pix);
  }

  pthread_barrier_wait(barrier);
  return NULL;
}

int main(int argc, char **argv) {
  gtk_init (&argc, &argv);
  GtkWidget* mainwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW(mainwin), "/dev/video0");
  GtkWidget* image = gtk_image_new();
  gtk_container_add(GTK_CONTAINER(mainwin), image);
  gtk_widget_show_all (mainwin);

  kiki_v4l2_device_handle_t device_handle;
  if(kiki_v4l2_open_device(&device_handle, "/dev/video0", 1280, 720)) return -1;
  if(kiki_v4l2_reset_parameter(&device_handle, V4L2_CID_AUTO_WHITE_BALANCE)) {fprintf(stderr, "V4L2_CID_AUTO_WHITE_BALANCE won't reset!\n"); return -1;}
  if(kiki_v4l2_turn_device_on(&device_handle)) return -1;
  raw_t *rgb_image_data = kiki_v4l2_allocate_image_data(&device_handle);
  if(!rgb_image_data) return -1;
  device_handle.image_treatment = &kiki_v4l2_YUV422toRGB888;

  g_signal_connect (mainwin, "destroy", G_CALLBACK (gtk_main_quit), NULL);
  int gtk_done = 0;
  pthread_barrier_t barrier;
  pthread_barrier_init(&barrier, NULL, 2);

  pthread_t image_thread;
  image_thread_args image_args = (image_thread_args){&device_handle, rgb_image_data, GTK_IMAGE(image), &barrier, &gtk_done};
  pthread_create(&image_thread, NULL, &refresh_image, &image_args);

  gtk_main();
  gtk_done = 1;
  pthread_barrier_wait(&barrier);

  kiki_v4l2_turn_device_off(&device_handle);
  kiki_v4l2_close_device(&device_handle);

  return 0;
}