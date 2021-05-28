#include "main.h"

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, "Usage: %s [device] [output_file]\n", argv[0]);
    return 1;
  }
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
  av_register_all();
#endif
  avdevice_register_all();
  avformat_network_init();

  const char *device = argv[1];
  const char *output_path = argv[2];
  const char *output_format = "mpegts";

  end_stream = false;
  signal(SIGINT, handle_signal);

  stream_video();

  return 0;
}

void stream_video()
{

  const char *device_family = "v4l2";

  if (open_device() != 0)
  {
    return;
  }

  if (avformat_alloc_output_context2(&fileOutput.fmt_ctx, NULL, "mpegts", NULL) != 0)
  {
    fprintf(stderr, "cannot initialize output format context!\n");
    return;
  }

  if (avio_open2(&fileOutput.fmt_ctx->pb, "./save.ts", AVIO_FLAG_WRITE, NULL, NULL) != 0)
  {
    fprintf(stderr, "could not open IO context!\n");
    return;
  }

  fileOutput.codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  fileOutput.stream = avformat_new_stream(fileOutput.fmt_ctx, fileOutput.codec);
  fileOutput.codec_ctx = avcodec_alloc_context3(fileOutput.codec);

  set_codec_params();
  init_codec_stream();

  fileOutput.stream->codecpar->extradata = fileOutput.codec_ctx->extradata;
  fileOutput.stream->codecpar->extradata_size = fileOutput.codec_ctx->extradata_size;

  // av_dump_format(fileOutput.fmt_ctx, 0, output_path, 1);

  if (avformat_write_header(fileOutput.fmt_ctx, NULL) != 0)
  {
    fprintf(stderr, "could not write header to ouput context!\n");
    return;
  }

  AVFrame *frame = av_frame_alloc();
  AVFrame *outframe = av_frame_alloc();
  AVPacket *pkt = av_packet_alloc();

  int nbytes = av_image_get_buffer_size(fileOutput.codec_ctx->pix_fmt, fileOutput.codec_ctx->width, fileOutput.codec_ctx->height, 32);
  uint8_t *video_outbuf = (uint8_t *)av_malloc(nbytes);
  av_image_fill_arrays(outframe->data, outframe->linesize, video_outbuf, AV_PIX_FMT_YUV420P, fileOutput.codec_ctx->width, fileOutput.codec_ctx->height, 1);
  outframe->width = 800;
  outframe->height = 600;
  outframe->format = fileOutput.codec_ctx->pix_fmt;

  struct SwsContext *swsctx = sws_getContext(deviceInput.codec_ctx->width, deviceInput.codec_ctx->height, deviceInput.codec_ctx->pix_fmt, fileOutput.codec_ctx->width, fileOutput.codec_ctx->height, fileOutput.codec_ctx->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
  av_init_packet(pkt);

  long pts = 0;

  while (av_read_frame(deviceInput.fmt_ctx, pkt) >= 0 && !end_stream)
  {
    frame = av_frame_alloc();
    if (avcodec_send_packet(deviceInput.codec_ctx, pkt) != 0)
    {
      fprintf(stderr, "error sending packet to input codec context!\n");
      break;
    }

    if (avcodec_receive_frame(deviceInput.codec_ctx, frame) != 0)
    {
      fprintf(stderr, "error receiving frame from input codec context!\n");
      break;
    }

    av_packet_unref(pkt);
    av_init_packet(pkt);

    sws_scale(swsctx, (const uint8_t *const *)frame->data, frame->linesize, 0, deviceInput.codec_ctx->height, outframe->data, outframe->linesize);
    av_frame_free(&frame);
    outframe->pts = pts++;

    if (avcodec_send_frame(fileOutput.codec_ctx, outframe) != 0)
    {
      fprintf(stderr, "error sending frame to output codec context!\n");
      break;
    }

    if (avcodec_receive_packet(fileOutput.codec_ctx, pkt) != 0)
    {
      fprintf(stderr, "error receiving packet from output codec context!\n");
      break;
    }

    pkt->pts = av_rescale_q(pkt->pts, fileOutput.codec_ctx->time_base, fileOutput.stream->time_base);
    pkt->dts = av_rescale_q(pkt->dts, fileOutput.codec_ctx->time_base, fileOutput.stream->time_base);

    av_interleaved_write_frame(fileOutput.fmt_ctx, pkt);
    av_packet_unref(pkt);
    av_init_packet(pkt);
  }

  av_write_trailer(fileOutput.fmt_ctx);
  av_frame_free(&outframe);
  avio_close(fileOutput.fmt_ctx->pb);
  avformat_free_context(fileOutput.fmt_ctx);
  avio_close(deviceInput.fmt_ctx->pb);
  avformat_free_context(deviceInput.fmt_ctx);

  fprintf(stderr, "done.\n");
}

int open_device()
{
  deviceInput.fmt = av_find_input_format("v4l2");

  AVDictionary *options = NULL;
  av_dict_set(&options, "video_size", "800x600", 0);
  av_dict_set(&options, "framerate", "30", 0);
  av_dict_set(&options, "pixel_format", "uyvy422", 0);
  av_dict_set(&options, "probesize", "7000000", 0);

  if (avformat_open_input(&deviceInput.fmt_ctx, "/dev/video0", deviceInput.fmt, &options) != 0)
  {
    fprintf(stderr, "cannot initialize input device!\n");
    return 1;
  }

  avformat_find_stream_info(deviceInput.fmt_ctx, 0);
  // av_dump_format(deviceInput.fmt_ctx, 0, device_family, 0);

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

void set_codec_params()
{
  const AVRational dst_fps = {30, 1};

  fileOutput.codec_ctx->codec_tag = 0;
  fileOutput.codec_ctx->codec_id = AV_CODEC_ID_H264;
  fileOutput.codec_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
  fileOutput.codec_ctx->width = 800;
  fileOutput.codec_ctx->height = 600;
  fileOutput.codec_ctx->gop_size = 12;
  fileOutput.codec_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
  fileOutput.codec_ctx->framerate = dst_fps;
  fileOutput.codec_ctx->time_base = av_inv_q(dst_fps);
  if (fileOutput.fmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
  {
    fileOutput.codec_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
  }
}

int init_codec_stream()
{
  if (avcodec_parameters_from_context(fileOutput.stream->codecpar, fileOutput.codec_ctx) != 0)
  {
    fprintf(stderr, "could not initialize stream codec parameters!\n");
    return 1;
  }

  AVDictionary *codec_options = NULL;
  //av_dict_set(&codec_options, "profile", "high", 0);
  av_dict_set(&codec_options, "preset", "superfast", 0);
  av_dict_set(&codec_options, "tune", "zerolatency", 0);

  // open video encoder
  if (avcodec_open2(fileOutput.codec_ctx, fileOutput.codec, &codec_options) != 0)
  {
    fprintf(stderr, "could not open video encoder!\n");
    return 1;
  }

  return 0;
}

void handle_signal(int signal)
{
  fprintf(stderr, "Caught SIGINT, exiting now...\n");
  end_stream = true;
}
