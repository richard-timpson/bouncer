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

   int encode_cool(AVFrame* frame, int frameNumber);


#endif
