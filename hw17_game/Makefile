CFLAGS = -Wall -Wextra -Wpedantic -std=c11
SRCS = main.c game_logic.c records.c ui.c
OBJS = $(SRCS:.c=.o)
TARGET = game2048
LIBS = -lallegro -lallegro_font -lallegro_ttf -lallegro_primitives -lallegro_audio -lallegro_acodec -lallegro_image

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
