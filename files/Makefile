CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Wno-format-truncation
LDFLAGS = -lpigpio -lpthread -lrt

TARGET  = menu
SPLASH  = splash

SOURCES = main.c button.c screen.c scripts.c menu.c hid.c cJSON.c creds.c
OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET) $(SPLASH)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(SPLASH): splash.o screen.o
	$(CC) splash.o screen.o -o $(SPLASH)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) splash.o $(TARGET) $(SPLASH)

.PHONY: all clean
