TARGET=audio_play
CFLAGS=-lasound
OBJS=audio_play.o wav_lib.o alsa_ctrl.o

$(TARGET): $(OBJS)
	gcc -o $@ $(OBJS) $(CFLAGS)

%.o: %.c
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET)
