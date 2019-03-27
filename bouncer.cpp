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
#include <cmath>


int ERROR_CODE = -1;

int main (int argc, char **argv)
{
  AVFrame * frame = av_frame_alloc();
  if (decode_and_scale_jpeg(argv[1], frame) < 0)
  {
    std::cout << "Couldn't decode and scale jpeg" << std::endl;
    return ERROR_CODE;
  }
  if (animate_jpeg(frame) < 0 )
  {
    std::cout << "Couldn't animate image" << std::endl;
    return ERROR_CODE;
  }
  encode_cool(frame, 0);
}

int decode_and_scale_jpeg(char * filename, AVFrame * destFrame)
{

  //  std::cout << "Hello world!" << std::endl;
  //  std::cout << filename << std::endl;
  

  /*******************************************************************************************
   following code is modified from this blog post

   http://random-stuff-mine.blogspot.com/2014/01/decoding-jpeg-image-file-using-libavcodec.html 

  *******************************************************************************************/

  // opening format context with jpeg file
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

  if ( avcodec_send_packet(pCodecCtx, &encodedPacket) < 0)
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  // receiving frame from decoder
  if ( avcodec_receive_frame(pCodecCtx,decodedFrame) < 0)
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  // allocating a frame that we can put the scaled encoded frame into (modify the pixel format)
  AVPixelFormat destFormat = AV_PIX_FMT_RGB8;    // we want to modify this in the future to get only the supported pixel formats from the cool codec

  // allocating the actual memory for the image data
  if( av_image_alloc(destFrame->data, destFrame->linesize, decodedFrame->width, decodedFrame->height, destFormat, 32) < 0)
  {
    std::cout << "Error allocating image for destination package" << std::endl;
    return 1;
  }

  // metadata values for the frame 
  destFrame -> width = decodedFrame -> width;
  destFrame -> height = decodedFrame -> height;
  destFrame->format = destFormat;

  // Debugging code
  //  std::cout << decodedFrame->width << " " << decodedFrame -> height << std::endl;
  //  std::cout << destFrame->width << " " << destFrame -> height << std::endl;
  //  std::cout << pCodecCtx->width << " " << pCodecCtx -> height << std::endl;
 

  // geting an sws context for scaling the encoded frame to the destintation frame (change pixel format)
  SwsContext *ctxt = sws_getContext(decodedFrame->width,
				    decodedFrame->height,
				    (AVPixelFormat)decodedFrame->format,
				    decodedFrame->width, decodedFrame->height,
				    destFormat,
				    SWS_BILINEAR, 
				    NULL, 
				    NULL, 
				    NULL);
  if (ctxt == NULL)
    {
      printf ("Error while calling sws_getContext"); 
      return ERROR_CODE;
    }

  // puts the information in the destFrame data with the newly specified pixel format
  sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destFrame->data, destFrame->linesize);
  sws_freeContext(ctxt);

  //  encode_cool(destFrame, 0);  
}

int animate_jpeg(AVFrame * frame)
{
  int centerXInitial = frame->width/2;
  int centerYInitial = frame->height/2;
  int radius = (frame->width + frame->height)/ 10;
  int diameter = 2 * radius;
  int centerX = centerXInitial;
  int centerY = centerYInitial;
  for (int k = 0 ; k < 300; k++)
  {
    AVFrame * current_frame = av_frame_alloc();
    if (av_frame_copy(current_frame,frame) < 0)
    {
      std::cout << "Couldn't copy frame data" << std::endl;
      return ERROR_CODE;
    }

    int topLeftX = centerX - radius;
    int topLeftY = centerY - radius;
    for (int i = topLeftX; i < topLeftX + diameter; i+=3)
    {
      for (int j = topLeftY; j < topLeftY + diameter; j++)
      {

	int distance = std::sqrt(std::pow(i - centerX,2) + std::pow(j - centerY,2));
	//std::cout << "distance is " << distance << std::endl;
	if (distance < radius)
	{
	  int index = j* frame->linesize[0] + i;
	  *(current_frame->data[0] + index) = 255;
	  *(current_frame->data[0] + index + 1) = 255;    
	  *(current_frameframe->data[0] + index + 2) = 255;
	}
      //std::cout << "radius is " << radius << std::endl;
      }

    }
    encode_cool(frame, k);
  }
return 1;





  


}

int encode_cool(AVFrame* frame, int frameNumber) 
{

  //  std::cout << "Entering Encode function " << std::endl;

  // get the cool codec
  //  std::cout << "Finding cool codec " << std::endl;
  const AVCodec* cool_codec = avcodec_find_encoder_by_name("cool");
  if (!cool_codec)
  {
    std::cout << "Couldn't find cool codec" << std::endl;
    return ERROR_CODE;
  }


  // get the cool codec context
  //  std::cout << "Finding codec context " << std::endl;
  AVCodecContext* codec_context = avcodec_alloc_context3(cool_codec);
  if (!codec_context)
  {
    std::cout << "Couldn't find cool codec context" << std::endl;
    return ERROR_CODE;
  }

  // frame-> format = codec_context -> pix_fmt;
  //  std::cout << "frame format is " << frame->format <<  std::endl;

  // set metadata for the context so we can put the frame into the av packet. 
  codec_context->time_base = (AVRational){1, 25};
  codec_context->framerate = (AVRational){25, 1};
  codec_context->pix_fmt = AV_PIX_FMT_RGB8;
  codec_context->width = frame->width;
  codec_context->height = frame->height;
  codec_context->bit_rate = 4000000;
  codec_context->gop_size = 1;


  // allocate packet for cool file
  //  std::cout << "allocating packet  " << std::endl;
  AVPacket* cool_pkt = av_packet_alloc();
  if (!cool_pkt)
  {
    std::cout << "Couldn't allocate packet" << std::endl;
    return ERROR_CODE;
  }

  // open the codec
  //  std::cout << "opening codec " << std::endl;
  int codecOpenRet = avcodec_open2(codec_context, cool_codec, NULL);
  if (codecOpenRet < 0)
  {
    std::cout << "Couldn't open codec" << std::endl;
    return ERROR_CODE;
  }


  // encode the frame using cool codec
  //  std::cout << "sending frame to packe " << std::endl;
  int sendFrameRet = avcodec_send_frame(codec_context, frame);
  if (sendFrameRet < 0)
  {
    std::cout << "Couldn't open codec" << std::endl;
    return ERROR_CODE;
  }
  
  // receive the encoded data into a packet
  int receivePktRet = avcodec_receive_packet(codec_context, cool_pkt);
  if (receivePktRet == AVERROR(EAGAIN) || receivePktRet == AVERROR_EOF)
    return ERROR_CODE;
  else if (receivePktRet < 0) 
  {
    std::cout << "Error during encoding" << std::endl;
    return ERROR_CODE;
  }
  
  char* filename = "test";
  strcat(filename, itoa(frameNumber));
  strcat(filename, ".cool");
  // write the image data to a cool file. 
  FILE*f = fopen(filename, "wb");
  fwrite(cool_pkt->data, 1, cool_pkt->size, f);

}

