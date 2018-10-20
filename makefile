TARGET = sikradio-sender

CXX = g++
CXXFLAGS = -Wall -O2 -std=c++17 -pthread

sikradio-sender: main.o transmitter.o audio.o socket.o
	$(CXX) $(CXXFLAGS) $^ -o $@
	
audio.o: audio.cpp audio.h

main.o: main.cpp audio.h fixed_size_fifo_deque.h mutex_wrapper.h socket.h transmitter.h

socket.o: socket.cpp socket.h

transmitter.o: transmitter.cpp transmitter.h audio.h c_array_wrapper.h fixed_size_fifo_deque.h mutex_wrapper.h socket.h

.PHONY: clean
clean:
	rm -f $(TARGET) *.o *~ *.bak

