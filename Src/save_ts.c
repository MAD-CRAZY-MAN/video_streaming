#include "../Inc/save_ts.h"

int open_output(const char* output_path)
{
  if (avformat_alloc_output_context2(&fileOutput.fmt_ctx, NULL, "mpegts", NULL) != 0)
  {
    fprintf(stderr, "cannot initialize output format context!\n");
    return 1;
  }

  if (avio_open2(&fileOutput.fmt_ctx->pb, output_path, AVIO_FLAG_WRITE, NULL, NULL) != 0)
  {
    fprintf(stderr, "could not open IO context!\n");
    return 1;
  }

  fileOutput.codec = avcodec_find_encoder(AV_CODEC_ID_H264);
  fileOutput.stream = avformat_new_stream(fileOutput.fmt_ctx, fileOutput.codec);
  fileOutput.codec_ctx = avcodec_alloc_context3(fileOutput.codec);

  //set codec params
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

  //init code stream
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

void write_video()
{
  static int count = 0;
  count++;
  fileOutput.stream->codecpar->extradata = fileOutput.codec_ctx->extradata;
  fileOutput.stream->codecpar->extradata_size = fileOutput.codec_ctx->extradata_size;
  snprintf(text_filter_str, sizeof(text_filter_str), "drawtext=text=%d:x=100:y=100:fontsize=40:fontcolor=black", count);
  // av_dump_format(fileOutput.fmt_ctx, 0, output_path, 1);

  if (avformat_write_header(fileOutput.fmt_ctx, NULL) != 0)
  {
    fprintf(stderr, "could not write header to ouput context!\n");
    return;
  }

  AVFrame *frame = av_frame_alloc();
  AVFrame *outframe = av_frame_alloc();
  AVPacket *pkt = av_packet_alloc();
  AVFrame *filterFrame = av_frame_alloc();

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

    if(av_buffersrc_add_frame(vfilter_ctx.src_ctx, outframe) < 0)
    {
      printf("Error filter context");
    }
    if(av_buffersink_get_frame(vfilter_ctx.sink_ctx, filterFrame)<0)
    {
      printf("errror");
    }
    

    if (avcodec_send_frame(fileOutput.codec_ctx, filterFrame) != 0)
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
  av_frame_free(&outframe);
  av_frame_free(&filterFrame);
}