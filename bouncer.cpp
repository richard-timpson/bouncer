/*
 * A simple application that can be run given a jpg file name. The application
 * takes the input file and creates a set of 300 frames of the background image
 * with a bouncing ball overlayed on top of it. Using the movie function the 
 * frames can be used to view the ball bouncing
 *
 * March 20, 2019
 * Richard Timpson, Tyler Brewster
 */ 


#include "bouncer.h"
#include <iostream>
#include <string>

int main (int argc, char **argv)
{
  
  char* filename = argv[1];
  const AVCodec *codec;
  AVPacket *av_pkt;
  AVCodecParserContext *parser;
  AVCodecContext *codec_context = NULL;
  FILE *file;
  AVFrame *frame;

  std::cout << "Hello world!" << std::endl;
  std::cout << filename << std::endl;


  /* Finding jpeg codec and exiting if error */
  codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
  if (!codec)
  {
    std:: cout << "Couldn't read codec" << std::endl;
    exit (-1);
  }

  /* allocating an AvPacket to put data in and exiting if error */
  av_pkt = av_packet_alloc();
  if (!av_pkt) 
  {
    std::cout << "Couldn't allocate packet" << std::endl;
    exit(-1);
  }

   parser = av_parser_init(codec->id);
   if (!parser) 
   {
     fprintf(stderr, "parser not found\n");
     exit(1);
   }

   codec_context = avcodec_alloc_context3(codec);
   if (!codec_context) 
   {
     std::cout << "Could not allocate video codec context" << std::endl;
     exit(1);
   }

   if (avcodec_open2(codec_context, codec, NULL) < 0) 
   {
     fprintf(stderr, "Could not open codec\n");
     exit(1);
   }

    file = fopen(filename, "rb");
    if (!file) 
    {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) 
    {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }


  
  
}
