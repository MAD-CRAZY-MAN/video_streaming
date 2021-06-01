CC = gcc
target = main
library = -I"~/ffmpeg_build/include"
LDFLAGS = $$(pkg-config --libs libavformat libavfilter libavcodec libavutil libswscale libavresample libavdevice)
$(target) : ./Src/main.o ./Src/device.o ./Src/save_ts.o ./Src/filter.o
	$(CC) -g -o ./main ./Src/main.o ./Src/device.o ./Src/save_ts.o ./Src/filter.o $(library) $(LDFLAGS)
./Src/main.o : ./Src/main.c
	$(CC) -g -c -o ./Src/main.o ./Src/main.c $(library) $(LDFLAGS)
./Src/device.o : ./Src/device.c
	$(CC) -g -c -o ./Src/device.o ./Src/device.c $(library) $(LDFLAGS)
./Src/save_ts.o : ./Src/save_ts.c
	$(CC) -g -c -o ./Src/save_ts.o ./Src/save_ts.c $(library) $(LDFLAGS)
./Src/filter.o : ./Src/filter.c
	$(CC) -g -c -o ./Src/filter.o ./Src/filter.c $(library) $(LDFLAGS)
clean :
	rm $(target) ./Src/main.o ./Src/device.o ./Src/save_ts.o ./Src/filter.o ./save_video/save.ts
