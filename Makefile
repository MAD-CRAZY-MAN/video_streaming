CC = gcc
target = main
LDFLAGS = $$(pkg-config --libs libavformat libavcodec libavutil libswscale libavresample libavdevice)
$(target) : ./src/main.o ./src/device.o ./src/save_ts.o
	$(CC) -g -o ./main ./src/main.o ./src/device.o ./src/save_ts.o -I"~/ffmpeg_build/include" $(LDFLAGS)
./src/main.o : ./src/main.c
	$(CC) -g -c -o ./src/main.o ./src/main.c -I"~/ffmpeg_build/include" $(LDFLAGS)
./src/device.o : ./src/device.c
	$(CC) -g -c -o ./src/device.o ./src/device.c -I"~/ffmpeg_build/include" $(LDFLAGS)
./src/output.o : ./src/save_ts.o
	$(CC) -g -c -o ./src/save_ts.o ./src/save_ts.c -I"~/ffmpeg_build/include" $(LDFLAGS)
clean :
	rm $(target)
