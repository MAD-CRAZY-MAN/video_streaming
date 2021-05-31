CC = gcc
target = main
library = -I"~/ffmpeg_build/include"
LDFLAGS = $$(pkg-config --libs libavformat libavfilter libavcodec libavutil libswscale libavresample libavdevice)
$(target) : ./Src/main.o ./Src/device.o ./Src/save_ts.o
	$(CC) -g -o ./main ./Src/main.o ./Src/device.o ./Src/save_ts.o $(library) $(LDFLAGS)
./Src/main.o : ./Src/main.c
	$(CC) -g -c -o ./Src/main.o ./Src/main.c $(library) $(LDFLAGS)
./Src/device.o : ./Src/device.c
	$(CC) -g -c -o ./Src/device.o ./Src/device.c $(library) $(LDFLAGS)
./Src/output.o : ./Src/save_ts.o
	$(CC) -g -c -o ./Src/save_ts.o ./Src/save_ts.c $(library) $(LDFLAGS)
clean :
	rm $(target) ./Src/main.o ./Src/device.o ./Src/save_ts.o ./save_video/save.ts
