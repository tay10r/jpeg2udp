.PHONY: all
all: jpeg2udp

jpeg2udp: main.o camera.o Makefile
	$(CXX) main.o camera.o -o $@ $(shell pkg-config --libs opencv4)

main.o: main.c camera.h Makefile
	$(CC) -c $< -o $@ $(shell pkg-config --cflags opencv4)

camera.o: camera.cpp camera.h
	$(CXX) -c $< -o $@ $(shell pkg-config --cflags opencv4)

.PHONY: clean
clean:
	$(RM) main.o jpeg2udp
