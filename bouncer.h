/*
 * Header File for the bouncer application that uses ffmpeg to convert
 * JPG files into movies with an overlayed bouncing ball
 */

#ifndef BOUNCER_H
#define BOUNCER_H
 extern "C"
 {
   #include <libavcodec/avcodec.h>
   #include <libavformat/avformat.h>
   #include <libswscale/swscale.h>
   #include <libavutil/imgutils.h>
 }

int decode_and_scale_jpeg(char * filename, AVFrame *);
int animate_jpeg(AVFrame * frame);
int make_audio(int, int);
int encode_cool(AVFrame* frame, int frameNumber);
int av_frame_deep_copy(AVFrame * copyFrame, AVFrame* frame);

#endif
