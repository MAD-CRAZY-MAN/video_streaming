#ifndef DEFINE_H
#define DEFINE_H

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
#include <libavfilter/buffersink.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/avfiltergraph.h>
#include <libavfilter/buffersrc.h>


typedef struct _FileContext
{
  AVFormatContext *fmt_ctx;
  AVInputFormat *fmt;
  
  AVStream *stream;

  AVCodecContext *codec_ctx;
  AVCodec *codec; 
} FileContext;

typedef struct _FilterContext
{
  AVFilterGraph* filter_graph;
  AVFilterContext* src_ctx;
  AVFilterContext* sink_ctx;
} FilterContext;

FileContext deviceInput, fileOutput, upload;
FilterContext vfilter_ctx;

bool end_stream;
void initialize();
void release();
void handle_signal(int signal);

#endif