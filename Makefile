all: main

CXXFLAGS=-g -O0 -Wall

OBJECTS=main.o datastruct.o sat-version.o

main: $(OBJECTS)
	g++ $(CXXFLAGS) -o main $(OBJECTS) -lz3

%.o: %.c $(@:.c=.h) $(SOURCES)
	g++ $(CXXFLAGS) -c -o $@ $< -lz3

clean:
	rm main *.o