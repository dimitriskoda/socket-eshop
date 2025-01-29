CC = gcc
CFLAGS = -Wall -Wextra -pedantic

TARGET = myeshop

all: $(TARGET)

$(TARGET): main.c myeshop.c myeshop.h
	$(CC) $(CFLAGS) -o $(TARGET) main.c myeshop.c

clean:
	rm -f $(TARGET) *.o
