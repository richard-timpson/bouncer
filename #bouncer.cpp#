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

int READ_BUF_SIZE = 4096;

int main (int argc, char **argv)
{
  
  char* filename = argv[1];

 

 
  FILE *file;

  uint8_t read_buf[READ_BUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

  std::cout << "Hello world!" << std::endl;
  std::cout << filename << std::endl;


  // Finding jpeg codec and exiting if error 
  const AVCodec *codec;
  codec = avcodec_find_decoder(AV_CODEC_ID_JPEG2000);
  if (!codec)
  {
    std:: cout << "Couldn't read codec" << std::endl;
    exit (-1);
  }

  // allocating an AvPacket to put data in and exiting if error 
  AVPacket *av_pkt;
  av_pkt = av_packet_alloc();
  if (!av_pkt) 
  {
    std::cout << "Couldn't allocate packet" << std::endl;
    exit(-1);
  }

  // getting a parser and exiting if error 
  AVCodecParserContext *parser;
  parser = av_parser_init(codec->id);
  if (!parser) 
  {
    fprintf(stderr, "parser not found\n");
    exit(1);
  }

  // getting the codec context based off of current coded, exiting if error 
  AVCodecContext *codec_context = NULL;
  codec_context = avcodec_alloc_context3(codec);
  if (!codec_context) 
  {
     std::cout << "Could not allocate video codec context" << std::endl;
     exit(1);
  }


  //  opening the codec 
  if (avcodec_open2(codec_context, codec, NULL) < 0) 
  {
     fprintf(stderr, "Could not open codec\n");
     exit(1);
  }
   
  // allocating a frame, and exiting if error
  AVFrame* frame;
  frame = av_frame_alloc();
  if (!frame) 
  {
     fprintf(stderr, "Could not allocate video frame\n");
     exit(1);
  }


  // opening an input file stream and writing the contents
  // of the jpeg into a buffer
  std::ifstream ifs(filename, std::ifstream::binary);
  std::filebuf* pointer_to_file_buffer = ifs.rdbuf();

  std::size_t size = pointer_to_file_buffer->pubseekoff (0,ifs.end,ifs.in);
  pointer_to_file_buffer->pubseekpos (0,ifs.in);
   

  uint8_t buffer[size]; 
  pointer_to_file_buffer->sgetn (buffer,size);
  
  ifs.close();


  // file = fopen(filename, "rb");
  // if (!file) 
  // {
  //    fprintf(stderr, "Could not open %s\n", filename);
  //    exit(1);
  // }


  

  while (!feof(f)) 
  {
     /* read raw data from the input file */
     data_size = fread(read_buf, 1, INBUF_SIZE, file);
     if (!data_size)
         break;

     /* use the parser to split the data into frames */
     data = read_buf;
     while (data_size > 0) 
     {
       ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
                                 data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
       if (ret < 0) 
       {
	 fprintf(stderr, "Error while parsing\n");
         exit(1);
        }
        data      += ret;
        data_size -= ret;

        if (pkt->size)
            decode(c, frame, pkt, outfilename);
     }
  }


  
  
}
