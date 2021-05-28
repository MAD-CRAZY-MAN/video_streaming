#include "define.h"
#include "device.h"
#include "save_ts.h"

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

  //open device
  if (open_device() != 0)
    return 0;
  
  ////open output video file(.ts, mpeg2-ts)
  if (open_output() != 0)
    return 0;

  //write video(./save.ts)
  write_video();

  return 0;
}





void handle_signal(int signal)
{
  fprintf(stderr, "Caught SIGINT, exiting now...\n");
  end_stream = true;
}
