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
int ERROR_CODE = -1;

int main (int argc, char **argv)
{
  
  char* filename = argv[1];

 

 
  FILE *file;

  uint8_t read_buf[READ_BUF_SIZE + AV_INPUT_BUFFER_PADDING_SIZE];

  std::cout << "Hello world!" << std::endl;
  std::cout << filename << std::endl;

  
  /* http://random-stuff-mine.blogspot.com/2014/01/decoding-jpeg-image-file-using-libavcodec.html */
  // following code is modified from this blog post

  // opening format context with jpef file
  AVFormatContext *jpgFC;
  if (avformat_open_input(&jpgFC, filename, NULL, NULL) != 0 ) 
  {
    printf("Error in opening input file %s",filename);
    return ERROR_CODE;
  }

  // opening the input stream associated with the file
  if (avformat_find_stream_info(jpgFC,NULL)<0)
  {
    printf("Error in finding stream info");
    avformat_close_input(&jpgFC); //release AVFormatContext memory
    return ERROR_CODE;
  }


  // finding the video stream from the format context
  int videoStreamIndex=-1;
  for (int a=0;a<jpgFC->nb_streams;a++)
  {
    if (jpgFC->streams[a]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
    {
      videoStreamIndex=a;
      break;
    }
  }
  if (videoStreamIndex==-1)
  {
     printf("Couldn't find video stream");
     avformat_close_input(&jpgFC);
     return ERROR_CODE;
  }

  // Getting the coded associated with the video stream. 
  AVCodecContext *pCodecCtx=jpgFC->streams[videoStreamIndex]->codec;
  AVCodec *pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec==NULL)
  {
     printf("Cannot find decoder");
     avformat_close_input(&jpgFC);
     return ERROR_CODE;
  }
  if (avcodec_open2(pCodecCtx,pCodec, NULL)<0)
  {
     printf("Cannot open decoder");
     avformat_close_input(&jpgFC);
     return ERROR_CODE;
  }
  

  // putting a the frame from the image into an avpacket.
  AVPacket* encodedPacket;
  av_init_packet(encodedPacket);
  encodedPacket->data=NULL;
  encodedPacket->size=0;
  //now read a frame into this AVPacket
  if (av_read_frame(jpgFC,encodedPacket)<0)
  {
     printf("Cannot read frame");
     av_packet_unref(encodedPacket);
     avcodec_close(pCodecCtx);
     avformat_close_input(&jpgFC);
     return ERROR_CODE;
  }
  

  // sending frame to a decoder. 
  int frameFinished=0;
  AVFrame *decodedFrame=av_frame_alloc();
  int sendRet = avcodec_send_packet(pCodecCtx, encodedPacket);
  if (sendRet < 0) 
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  // receiving frame from decoder
  int receiveRet = avcodec_receive_frame(pCodecCtx,decodedFrame);
  if (receiveRet < 0) 
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  AVFrame* destFrame = av_frame_alloc();
  //we will convert to RGB32
  AVPixelFormat destFormat = AV_PIX_FMT_RGB24;
  av_image_alloc(destFrame->data, destFrame->linesize, destFrame->width, destFrame->height, destFormat, 32);
  //  avpicture_alloc(&destPic,destFormat, decodedFrame->width,decodedFrame->height);
  SwsContext *ctxt = sws_getContext(decodedFrame->width, decodedFrame->height,
				    (AVPixelFormat)decodedFrame->format,decodedFrame->width, decodedFrame->height,
				    destFormat,SWS_BILINEAR, NULL, NULL, NULL);
  if (ctxt == NULL)
    {
      printf ("Error while calling sws_getContext");
      return ERROR_CODE;
    }
  sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destFrame->data, destFrame->linesize);
  sws_freeContext(ctxt);















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
  // std::ifstream ifs(filename, std::ifstream::binary);
  // std::filebuf* pointer_to_file_buffer = ifs.rdbuf();

  // std::size_t size = pointer_to_file_buffer->pubseekoff (0,ifs.end,ifs.in);
  // pointer_to_file_buffer->pubseekpos (0,ifs.in);
   

  // uint8_t buffer[size]; 
  // pointer_to_file_buffer->sgetn (buffer,size);
  
  // ifs.close();


  // file = fopen(filename, "rb");
  // if (!file) 
  // {
  //    fprintf(stderr, "Could not open %s\n", filename);
  //    exit(1);
  // }


  

  // while (!feof(f)) 
  // {
  //    /* read raw data from the input file */
  //    data_size = fread(read_buf, 1, INBUF_SIZE, file);
  //    if (!data_size)
  //        break;

  //    /* use the parser to split the data into frames */
  //    data = read_buf;
  //    while (data_size > 0) 
  //    {
  //      ret = av_parser_parse2(parser, c, &pkt->data, &pkt->size,
  //                                data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
  //      if (ret < 0) 
  //      {
  // 	 fprintf(stderr, "Error while parsing\n");
  //        exit(1);
  //       }
  //       data      += ret;
  //       data_size -= ret;

  //       if (pkt->size)
  //           decode(c, frame, pkt, outfilename);
  //    }
  // }


  
  
}
