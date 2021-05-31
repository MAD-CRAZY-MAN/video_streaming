#include "../Inc/device.h"

int open_device(const char* device_name)
{
  deviceInput.fmt_ctx = avformat_alloc_context();
  deviceInput.fmt = av_find_input_format("v4l2");

  AVDictionary *options = NULL;
  av_dict_set(&options, "video_size", "800x600", 0);
  av_dict_set(&options, "framerate", "30", 0);
  av_dict_set(&options, "pixel_format", "uyvy422", 0);
  av_dict_set(&options, "probesize", "7000000", 0);

  if (avformat_open_input(&deviceInput.fmt_ctx, device_name, deviceInput.fmt, &options) != 0) //입력 스트림을 열고 헤더를 읽는다. 코덱을 열리지 않음
  {
    fprintf(stderr, "cannot initialize input device!\n");
    return 1;
  }

  avformat_find_stream_info(deviceInput.fmt_ctx, 0);
  av_dump_format(deviceInput.fmt_ctx, 0, "v4l2", 0); //print info

  deviceInput.codec = avcodec_find_decoder(deviceInput.fmt_ctx->streams[0]->codecpar->codec_id);
  deviceInput.stream = avformat_new_stream(deviceInput.fmt_ctx, deviceInput.codec);
  deviceInput.codec_ctx = avcodec_alloc_context3(deviceInput.codec);

  AVDictionary *codec_options = NULL;
  //av_dict_set(&codec_options, "profile", "high", 0);
  av_dict_set(&codec_options, "framerate", "30", 0);
  av_dict_set(&codec_options, "preset", "superfast", 0);

  avcodec_parameters_to_context(deviceInput.codec_ctx, deviceInput.fmt_ctx->streams[0]->codecpar);
  if (avcodec_open2(deviceInput.codec_ctx, deviceInput.codec, &codec_options) != 0)
  {
    fprintf(stderr, "cannot initialize video decoder!\n");
    return 1;
  }

  return 0;
}