#This is the Makefile

CXXFLAGS = -std=gnu++11 -Wall -Wextra -Wpedantic -Wshadow -pthread
CXX = g++

all:
	$(CXX) $(CXXFLAGS) -o httpserver httpserver.cpp
exec:
	./httpserver
clean:
	rm -rf httpserver
	rm -rf *.exe
	rm -rf *.o
