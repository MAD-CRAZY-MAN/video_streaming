#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>


typedef struct _FileContext
{
  AVInputFormat *fmt;
  AVFormatContext *fmt_ctx;
  AVCodec *codec;
  AVStream *stream;
  AVCodecContext *codec_ctx;
} FileContext;

FileContext deviceInput, fileOutput, upload;

bool end_stream;

void stream_video();
int open_device();


void set_codec_params();
int init_codec_stream();

void handle_signal(int signal);
