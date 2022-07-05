STATIC_LIB = -lm
CFLAGS =  -std=c99 

SRC = main.c

TARGET = svg2png

all: $(TARGET)

$(TARGET): $(SRC)
	gcc $(CFLAGS) $^ $(STATIC_LIB) -o $@

clean:
	rm -f $(TARGET) 
