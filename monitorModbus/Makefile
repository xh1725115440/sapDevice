all: modbuswatch clean

modbuswatch: monModbus.o 
	arm-linux-gnueabihf-g++ -o modbuswatch monModbus.o  -pthread -std=c++11 -Wl,--no-as-needed

monModbus.o: monModbus.cpp
	arm-linux-gnueabihf-g++ -c monModbus.cpp  -pthread -std=c++11 -Wl,--no-as-needed
.PHONY: clean
clean:
	rm *.o
