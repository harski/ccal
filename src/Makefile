

CC = gcc
CFLAGS = -g -Werror -Wall -pedantic -std=c99
LIBS = -L/usr/local/lib -lvector -lncurses

TARGET = hcal
OBJS = cal.o entry.o getline.o settings.o strutils.o

all: $(TARGET)

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): main.c $(OBJS)
	$(CC) $(CFLAGS) main.c $(LIBS) $(OBJS) -o $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET)
