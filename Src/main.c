#include "../Inc/define.h"
#include "../Inc/device.h"
#include "../Inc/save_ts.h"

int main(int argc, char *argv[])
{
  if (argc != 3) 
  {
    fprintf(stderr, "Usage: %s [device] [output_file]\n", argv[0]);
    return 1;
  }
  initialize();
  
  //open device
  if (open_device(argv[1]) != 0) //argv[1] ex: /dev/video0
    return 0;
  
  ////open output video file(.ts, mpeg2-ts)
  if (open_output(argv[2]) != 0) //argv[2] ex: ./save_video/save.ts
    return 0;

  //write video(./save.ts)
  write_video();
  release();
  return 0;
}

void release()
{
  av_write_trailer(fileOutput.fmt_ctx);
  //av_frame_free(&outframe);
  avio_close(fileOutput.fmt_ctx->pb);
  avformat_free_context(fileOutput.fmt_ctx);

  avio_close(deviceInput.fmt_ctx->pb);
  avformat_free_context(deviceInput.fmt_ctx);

  fprintf(stderr, "done.\n");
}

void handle_signal(int signal)
{
  fprintf(stderr, "Caught SIGINT, exiting now...\n");
  end_stream = true;
}

void initialize()
{
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 9, 100)
  av_register_all();
#endif
  avdevice_register_all();
  avformat_network_init();
  avfilter_register_all();

  end_stream = false;
  signal(SIGINT, handle_signal);
}