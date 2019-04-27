/*
 * A simple application that can be run given a jpg file name. The application
 * takes the input file and creates a set of 300 frames of the background image
 * with a bouncing ball overlayed on top of it. Using the movie function the 
 * frames can be used to view the ball bouncing
 *
 * We heavily relied on the example gives in the ffmpeg documentation, which can be found at
 * ffmpeg/doc/examples. We specifically used decode_video.c and encode_video.c. 
 *
 * April 5, 2019
 * Richard Timpson, Tyler Brewster
 */

#include "bouncer.h"
#include <iostream>
#include <string>
#include <cmath>
#include <sstream>

int ERROR_CODE = -1;

int main(int argc, char **argv)
{
  // make sure we only have one image in the agruments
  if (argc != 2)
  {
    std::cout << "Must supply one image" << std::endl;
    return ERROR_CODE;
  }

  

  // check the last 4 characters of the input, and make sure it's a .jpg extension
  char last_4_letter[4];
  char *file_name = argv[1];


  int len = strlen(file_name);
  const char *last_four = &file_name[len-4];
  std:: cout << last_four << std:: endl;

  // int string_size = sizeof(file_name) / sizeof(char);
  // int index = 0;
  // for (int i = string_size; i > string_size - 4; i--)
  // {
  //   last_4_letter[index] = file_name[i];
  //   index++;
  // }
  if (strcmp(last_four, ".jpg") != 0)
  {
    std::cout << "Must be a .jpg extension" << " Found: " << last_4_letter << std::endl;
    return ERROR_CODE;
  }

  // allocate a frame for scaling the jpg image
  AVFrame *frame = av_frame_alloc();
  if (decode_and_scale_jpeg(argv[1], frame) < 0)
  {
    std::cout << "Couldn't decode and scale jpeg" << std::endl;
    return ERROR_CODE;
  }

  // animate the jpg, which will output the .cool files.
  if (animate_jpeg(frame) < 0)
  {
    std::cout << "Couldn't animate image" << std::endl;
    return ERROR_CODE;
  }

  // make the audio which will output the Bounce_Audio.mp2
  if (make_audio(frame->height, frame->width) < 0)
  {
    std::cout << "Couldn't make audio" << std::endl;
    return ERROR_CODE;
  }
}

/* Decode a jpeg file, scale it to a rgb24 pixel format, and set it inside of an AVFrame */
int decode_and_scale_jpeg(char *filename, AVFrame *destFrame)
{

  /*******************************************************************************************
   following code is modified from this blog post

   http://random-stuff-mine.blogspot.com/2014/01/decoding-jpeg-image-file-using-libavcodec.html

  *******************************************************************************************/

  // opening format context with jpeg file
  AVFormatContext *jpgFC = NULL;
  if (avformat_open_input(&jpgFC, filename, NULL, NULL) != 0)
  {
    printf("Error in opening input file %s", filename);
    return ERROR_CODE;
  }

  // opening the input stream associated with the file
  if (avformat_find_stream_info(jpgFC, NULL) < 0)
  {
    printf("Error in finding stream info");
    avformat_close_input(&jpgFC); //release AVFormatContext memory
    return ERROR_CODE;
  }

  // finding the video stream from the format context
  int videoStreamIndex = -1;
  for (int a = 0; a < jpgFC->nb_streams; a++)
  {
    if (jpgFC->streams[a]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
    {
      videoStreamIndex = a;
      break;
    }
  }
  if (videoStreamIndex == -1)
  {
    printf("Couldn't find video stream");
    avformat_close_input(&jpgFC);
    return ERROR_CODE;
  }

  // Getting the codec associated with the video stream.
  AVCodecContext *pCodecCtx = jpgFC->streams[videoStreamIndex]->codec;
  AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
  if (pCodec == NULL)
  {
    printf("Cannot find decoder");
    avformat_close_input(&jpgFC);
    return ERROR_CODE;
  }
  if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
  {
    printf("Cannot open decoder");
    avformat_close_input(&jpgFC);
    return ERROR_CODE;
  }

  // putting a the frame from the image into an avpacket.
  AVPacket encodedPacket;
  av_init_packet(&encodedPacket);
  encodedPacket.data = NULL;
  encodedPacket.size = 0;
  //now read a frame into this AVPacket
  if (av_read_frame(jpgFC, &encodedPacket) < 0)
  {
    printf("Cannot read frame");
    av_packet_unref(&encodedPacket);
    avcodec_close(pCodecCtx);
    avformat_close_input(&jpgFC);
    return ERROR_CODE;
  }

  // sending frame to a decoder.
  int frameFinished = 0;
  AVFrame *decodedFrame = av_frame_alloc();

  if (avcodec_send_packet(pCodecCtx, &encodedPacket) < 0)
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  // receiving frame from decoder
  if (avcodec_receive_frame(pCodecCtx, decodedFrame) < 0)
  {
    std::cout << "Error sending a packet for decoding" << std::endl;
    return ERROR_CODE;
  }

  // setting the pixel format for the scaled frame to rgb24.
  // required for the animation.
  AVPixelFormat destFormat = AV_PIX_FMT_RGB24;

  // allocating the actual memory for the image data
  if (av_image_alloc(destFrame->data, destFrame->linesize, decodedFrame->width, decodedFrame->height, destFormat, 32) < 0)
  {
    std::cout << "Error allocating image for destination package" << std::endl;
    return 1;
  }

  // metadata values for the frame
  destFrame->width = decodedFrame->width;
  destFrame->height = decodedFrame->height;
  destFrame->format = destFormat;

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
    printf("Error while calling sws_getContext");
    return ERROR_CODE;
  }

  // puts the information in the destFrame data with the newly specified pixel format
  sws_scale(ctxt, decodedFrame->data, decodedFrame->linesize, 0, decodedFrame->height, destFrame->data, destFrame->linesize);
  sws_freeContext(ctxt);
}

/* Animate an AVframe (assuming a rgb24 pixel format) */
int animate_jpeg(AVFrame *frame)
{
  // setting up initial values for the animation
  int centerXInitial = frame->width / 2;
  int centerYInitial = frame->height / 2;
  int radius = (frame->width + frame->height) / 10;
  int diameter = 2 * radius;
  int centerX = centerXInitial;
  int centerY = centerYInitial;
  int highlightX = centerXInitial - radius / 4;
  int highlightY = centerYInitial - radius / 4;
  bool going_down = true;

  // using an increment variable to ensure that the ball changes 
  // directions every 50 frames. 
  int increment = (frame->height - (centerYInitial + radius)) /50;

  // make 300 frames in a loop
  for (int k = 0; k < 300; k++)
  {
    // copy the frame
    AVFrame *current_frame = av_frame_alloc();
    if (av_frame_deep_copy(current_frame, frame) < 0)
    {
      std::cout << "Couldn't copy frame data" << std::endl;
      return ERROR_CODE;
    }

    // recalculate the box to loop through as the frame changes
    int topLeftX = centerX - radius;
    int topLeftY = centerY - radius;

    // used for the highlight calcuations,
    highlightX = centerX - radius / 4;
    highlightY = centerY - radius / 4;
    for (int i = topLeftX; i < topLeftX + diameter; i += 1)
    {
      for (int j = topLeftY; j < topLeftY + diameter; j++)
      {
        // distance for ball
        int distance = std::sqrt(std::pow(i - centerX, 2) + std::pow(j - centerY, 2));

        // number used for color gradient
        int distanceH = std::sqrt(std::pow(i - highlightX, 2) + std::pow(j - highlightY, 2));

        // makeing sure we never get an invalid byte value for the rgb channels
        if (distanceH > 255)
        {
          distanceH = 255;
        }

        // make sure the distance is within the radius (draw a circle)
        if (distance < radius)
        {
          // modify each byte of the pixel
          int index = j * frame->linesize[0] + i * 3;
          *(current_frame->data[0] + index) = 255;
          *(current_frame->data[0] + index + 1) = 255 - distanceH;
          *(current_frame->data[0] + index + 2) = 255;
        }
      }
    }
    // move the ball up and down
    if (going_down)
    {
      centerY += increment;
      if (centerY + radius > frame->height)
      {
        going_down = false;
      }
    }
    else if (!going_down)
    {
      centerY -= increment;
      if (centerY < centerYInitial)
        going_down = true;
    }

    encode_cool(current_frame, k);
  }
  return 1;
}

/* Make a deep copy of the AVFrames used in creating the animation frames */
int av_frame_deep_copy(AVFrame *copyFrame, AVFrame *frame)
{
  // allocated the image size for the new AVFrame
  if (av_image_alloc(copyFrame->data, copyFrame->linesize, frame->width, frame->height, (AVPixelFormat)frame->format, 32) < 0)
  {
    std::cout << "Error allocating image for new frame" << std::endl;
    return 1;
  }

  // set metadata on frame
  copyFrame->format = frame->format;
  copyFrame->width = frame->width;
  copyFrame->height = frame->height;

  // copy one from to the other.
  if (av_frame_copy(copyFrame, frame) < 0)
  {
    std::cout << "Could not get copy" << std::endl;
    return ERROR_CODE;
  }
}

/* encode an AVframe into cool AVPacket and write to file based on frame number */
int encode_cool(AVFrame *frame, int frameNumber)
{
  // get the cool codec
  const AVCodec *cool_codec = avcodec_find_encoder_by_name("cool");
  if (!cool_codec)
  {
    std::cout << "Couldn't find cool codec" << std::endl;
    return ERROR_CODE;
  }

  // get the cool codec context
  AVCodecContext *codec_context = avcodec_alloc_context3(cool_codec);
  if (!codec_context)
  {
    std::cout << "Couldn't find cool codec context" << std::endl;
    return ERROR_CODE;
  }

  // set metadata for the context so we can put the frame into the av packet.
  // we used the encode_video.c example to know what values to set.
  codec_context->time_base = (AVRational){1, 25};
  codec_context->framerate = (AVRational){25, 1};
  codec_context->pix_fmt = AV_PIX_FMT_RGB24;
  codec_context->width = frame->width;
  codec_context->height = frame->height;
  codec_context->bit_rate = 64000;
  codec_context->gop_size = 1;

  // allocate packet for cool file
  AVPacket *cool_pkt = av_packet_alloc();
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
  av_frame_free(&frame);

  // receive the encoded data into a packet
  int receivePktRet = avcodec_receive_packet(codec_context, cool_pkt);
  if (receivePktRet == AVERROR(EAGAIN) || receivePktRet == AVERROR_EOF)
    return ERROR_CODE;
  else if (receivePktRet < 0)
  {
    std::cout << "Error during encoding" << std::endl;
    return ERROR_CODE;
  }

  // make a string for file name
  std::stringstream ss;
  int x = 23;
  ss << "frame";
  ss << frameNumber;
  ss << ".cool";
  std::string str = ss.str();
  const char *fileName = str.c_str();

  // write the image data to a cool file.
  FILE *f = fopen(fileName, "wb");
  fwrite(cool_pkt->data, 1, cool_pkt->size, f);
}

/***************************************************************************************
*                                                                
* All of the code below was taken and modified from ffmpeg/doc/examples/encode_audio.c * 
* Used for the audio portion of the assignment. 
*
****************************************************************************************/
/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(const AVCodec *codec, enum AVSampleFormat sample_fmt)
{
  const enum AVSampleFormat *p = codec->sample_fmts;

  while (*p != AV_SAMPLE_FMT_NONE)
  {
    if (*p == sample_fmt)
      return 1;
    p++;
  }
  return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(const AVCodec *codec)
{
  const int *p;
  int best_samplerate = 0;

  if (!codec->supported_samplerates)
    return 44100;

  p = codec->supported_samplerates;
  while (*p)
  {
    if (!best_samplerate || abs(44100 - *p) < abs(44100 - best_samplerate))
      best_samplerate = *p;
    p++;
  }
  return best_samplerate;
}

/* select layout with the highest channel count */
static int select_channel_layout(const AVCodec *codec)
{
  const uint64_t *p;
  uint64_t best_ch_layout = 0;
  int best_nb_channels = 0;

  if (!codec->channel_layouts)
    return AV_CH_LAYOUT_STEREO;

  p = codec->channel_layouts;
  while (*p)
  {
    int nb_channels = av_get_channel_layout_nb_channels(*p);

    if (nb_channels > best_nb_channels)
    {
      best_ch_layout = *p;
      best_nb_channels = nb_channels;
    }
    p++;
  }
  return best_ch_layout;
}

static void encode(AVCodecContext *ctx, AVFrame *frame, AVPacket *pkt,FILE *output)
{
  int ret;

  /* send the frame for encoding */
  ret = avcodec_send_frame(ctx, frame);
  if (ret < 0)
  {
    fprintf(stderr, "Error sending the frame to the encoder\n");
    exit(1);
  }

  /* read all the available output packets (in general there may be any
     * number of them */
  while (ret >= 0)
  {
    ret = avcodec_receive_packet(ctx, pkt);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return;
    else if (ret < 0)
    {
      fprintf(stderr, "Error encoding audio frame\n");
      exit(1);
    }

    fwrite(pkt->data, 1, pkt->size, output);
    av_packet_unref(pkt);
  }
}


int make_audio(int height, int width)
{
  // set up variables for later usage. 
  const char *filename;
  const AVCodec *codec;
  AVCodecContext *context = NULL;
  AVFrame *frame;
  AVPacket *pkt;
  int i, j, k, ret;
  FILE *f;
  uint16_t *samples;
  float t, tincr;

  // hardcoding the audio file name. Not optimal, but it works. 
  filename = "Bounce_Audio.mp2";

  // get the MP2 encoder 
  codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
  if (!codec)
  {
    fprintf(stderr, "Codec not found\n");
    exit(1);
  }

  // get the codec context for mp2
  context = avcodec_alloc_context3(codec);
  if (!context)
  {
    fprintf(stderr, "Could not allocate audio codec context\n");
    exit(1);
  }

  /* put sample parameters */
  context->bit_rate = 64000;

  /* check that the encoder supports s16 pcm input */
  context->sample_fmt = AV_SAMPLE_FMT_S16;
  if (!check_sample_fmt(codec, context->sample_fmt))
  {
    fprintf(stderr, "Encoder does not support sample format %s",
            av_get_sample_fmt_name(context->sample_fmt));
    exit(1);
  }

  /* select other audio parameters supported by the encoder */
  context->sample_rate = select_sample_rate(codec);
  context->channel_layout = select_channel_layout(codec);
  context->channels = av_get_channel_layout_nb_channels(context->channel_layout);

  /* open it */
  if (avcodec_open2(context, codec, NULL) < 0)
  {
    fprintf(stderr, "Could not open codec\n");
    exit(1);
  }

  // if we can't open, exit and print error
  f = fopen(filename, "wb");
  if (!f)
  {
    fprintf(stderr, "Could not open %s\n", filename);
    exit(1);
  }

  /* packet for holding encoded output */
  pkt = av_packet_alloc();
  if (!pkt)
  {
    fprintf(stderr, "could not allocate the packet\n");
    exit(1);
  }

  /* frame containing input raw audio */
  frame = av_frame_alloc();
  if (!frame)
  {
    fprintf(stderr, "Could not allocate audio frame\n");
    exit(1);
  }

  // setting up metadata for audio
  frame->nb_samples = context->frame_size;
  frame->format = context->sample_fmt;
  frame->channel_layout = context->channel_layout;

  /* allocate the data buffers */
  ret = av_frame_get_buffer(frame, 0);
  if (ret < 0)
  {
    fprintf(stderr, "Could not allocate audio data buffers\n");
    exit(1);
  }

  /* encoding a wave like sound to correspond with the bounce of a ball*/
  t = 3000;

  // getting information about the frame to use for bounce calcuation 
  float start_position = height / 2;
  float position = start_position;
  float radius = (width + height) / 10;
  bool going_down = true;

  // we found this value to be a good ratio for the synchronization of the audio with the video. 
  // meaning, making 300 frames of audio produced sound that was too short for the length of the video. 
  // We found that by increasing the number of frames of the audio to match the length of the video
  // and finding a ratio between number of audio frames and number of video frames allowed us to have 
  // an approximately good sync between the two at the bounce of the ball. that number was roughly 1.32
  float audio_visual_sync_ratio = 1.32;

  // setting the increment of the "position" of the ball, using the right ratio. 
  // Because we set the ball to change directions every 50 frames, we want the audio
  // to switch it's pitch at 50 frames times the ratio. 
  float increment = ((float)height - ((float)start_position + (float)radius))/ (50 * audio_visual_sync_ratio);

  // the starting pitch of the audio
  int pitch = 440;
  int number_of_frames = 300 * audio_visual_sync_ratio;
  for (i = 0; i < number_of_frames; i++)
  {
    /* make sure the frame is writable -- makes a copy if the encoder
         * kept a reference internally */
    ret = av_frame_make_writable(frame);
    if (ret < 0)
      exit(1);
    samples = (uint16_t *)frame->data[0];

    // writing to the data samples for the audio frame
    for (j = 0; j < context->frame_size; j++)
    {
      samples[2 * j] = (int)(sin(t) * 10000);
      // writing the same data value to all channels
      for (k = 1; k < context->channels; k++)
        samples[2 * j + k] = samples[2 * j];

      // this effectively creates a bounce sound, whenever we hit the bottom of the image
      if( !going_down  && (position +radius + increment * 5 >= height) )
      {
        t +=  M_PI * pitch / context->sample_rate;
      }
      // use the modified pitch to dynamically change the sound
      // the pitch will go down when moving down, and up when moving up
      else
      {
        t += 2 * M_PI * (pitch) / context->sample_rate;
      }
    }
    
    /* move the pitch up and down */
    if (going_down)
    {
      // if we are moving down, increase the position, and decrease the pitch
      position += increment;

      // decreasing pitch by arbitrary number, seemed to work well
      pitch -= 5;
      
      // if we hit the bottom, change direction flag
      if (position + radius >= height)
      {
        going_down = false;
      }
    }
    else if (!going_down)
    {
      // if we are moving up, decrement the position, but increase the pitch. 
      position -= increment;
      pitch += 5;
      if (position < start_position)
      {
        going_down = true;
      }
    }

    // once we have performed the right calcuations for the frame, write that frame 
    // to the audio file, using function from example files. 
    encode(context, frame, pkt, f);
  }

  /* flush the encoder */
  encode(context, NULL, pkt, f);

  fclose(f);

  av_frame_free(&frame);
  av_packet_free(&pkt);
  avcodec_free_context(&context);

  return 0;
}
