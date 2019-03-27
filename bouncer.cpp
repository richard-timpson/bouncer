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
  AVFormatContext *jpgFC = NULL;
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

  // Getting the codec associated with the video stream. 
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
  AVPacket encodedPacket;
  av_init_packet(&encodedPacket);
  encodedPacket.data=NULL;
  encodedPacket.size=0;
  //now read a frame into this AVPacket
  if (av_read_frame(jpgFC,&encodedPacket)<0)
  {
     printf("Cannot read frame");
     av_packet_unref(&encodedPacket);
     avcodec_close(pCodecCtx);
     avformat_close_input(&jpgFC);
     return ERROR_CODE;
  }

  // sending frame to a decoder. 
  int frameFinished=0;
  AVFrame *decodedFrame=av_frame_alloc();
  int sendRet = avcodec_send_packet(pCodecCtx, &encodedPacket);
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
  // we want to modify this in the future to get only the supported pixel formats from the cool codec
  AVPixelFormat destFormat = AV_PIX_FMT_RGB8;
  // modifies the buffer but does not set up things like height, width, and format // I believe
  int retImage = av_image_alloc(destFrame->data, destFrame->linesize, decodedFrame->width, decodedFrame->height, destFormat, 32);
  if(retImage < 0)
  {
    std::cout << "Error allocating image for destination package" << std::endl;
    return 1;
  }

  // Maybe remove later
  destFrame -> width = decodedFrame -> width;
  destFrame -> height = decodedFrame -> height;

  // Debugging code
  std::cout << decodedFrame->width << " " << decodedFrame -> height << std::endl;
  std::cout << destFrame->width << " " << destFrame -> height << std::endl;
  std::cout << pCodecCtx->width << " " << pCodecCtx -> height << std::endl;
 
  //  avpicture_alloc(&destPic,destFormat, decodedFrame->width,decodedFrame->height);
  SwsContext *ctxt = sws_getContext(decodedFrame->width, decodedFrame->height,
				    (AVPixelFormat)decodedFrame->format,decodedFrame->width, decodedFrame->height,
				    destFormat,SWS_BILINEAR, NULL, NULL, NULL);
  if (ctxt == NULL)
    {
      printf ("Error while calling sws_getContext");
      return ERROR_CODE;
    }

  // puts the information in the destFrame data but does not set everything that needs to be set for the frame
  sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destFrame->data, destFrame->linesize);
  sws_freeContext(ctxt);

  


  encode_cool(destFrame, 0);

  
  
}


int encode_cool(AVFrame* frame, int frameNumber) 
{

  std::cout << "Entering Encode function " << std::endl;

  // get the cool codec
  const AVCodec* cool_codec = avcodec_find_encoder_by_name("cool");
  if (!cool_codec)
  {
    std::cout << "Couldn't find cool codec" << std::endl;
    return ERROR_CODE;
  }


  // get the cool codec context
  AVCodecContext* codec_context = avcodec_alloc_context3(cool_codec);
  if (!codec_context)
  {
    std::cout << "Couldn't find cool codec context" << std::endl;
    return ERROR_CODE;
  }

  frame-> format = codec_context -> pix_fmt;

  codec_context->time_base = (AVRational){1, 25};
  codec_context->framerate = (AVRational){25, 1};
  codec_context->pix_fmt = AV_PIX_FMT_RGB8;
  codec_context->width = frame->width;
  codec_context->height = frame->height;
  codec_context->bit_rate = 4000000;
  codec_context->gop_size = 1;


  // allocate packet for cool file
  AVPacket* cool_pkt = av_packet_alloc();
  if (!cool_pkt)
  {
    std::cout << "Couldn't allocate packet" << std::endl;
    return ERROR_CODE;
  }

  // open the codec
  int codecOpenRet = avcodec_open2(codec_context, cool_codec, NULL);
  if (codecOpenRet < 0)
  {
    std::cout << "Couldn't open codec" << std::endl;
    return ERROR_CODE;
  }


  // encode the frame using cool codec
  int sendFrameRet = avcodec_send_frame(codec_context, frame);
  if (sendFrameRet < 0)
  {
    std::cout << "Couldn't open codec" << std::endl;
    return ERROR_CODE;
  }

  int receivePktRet = avcodec_receive_packet(codec_context, cool_pkt);
  if (receivePktRet == AVERROR(EAGAIN) || receivePktRet == AVERROR_EOF)
    return ERROR_CODE;
  else if (receivePktRet < 0) 
  {
    std::cout << "Error during encoding" << std::endl;
    return ERROR_CODE;
  }

  FILE*f = fopen("test.cool", "wb");

  fwrite(cool_pkt->data, 1, cool_pkt->size, f);
  
  // AVFormatContext* fmt_context = avformat_alloc_context();
  // if (!fmt_context)
  // {
  //   std::cout << "Couldn't allocate format contextc" << endl;
  //   return ERROR_CODE;
  // }

  // AVOutputFormat* fmt_output = av_guess_format(NULL, "test.cool", NULL);
  // fmt_context->oformat = ftm_output;

  

  

  

  

}

