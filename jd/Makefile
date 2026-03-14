all: jd clean

jd: testjd.o usbctl.o
	arm-linux-gnueabihf-g++ -o jd testjd.o usbctl.o  -pthread -std=c++11 -Wl,--no-as-needed

testjd.o: testjd.cpp
	arm-linux-gnueabihf-g++ -c testjd.cpp  -pthread -std=c++11 -Wl,--no-as-needed
usbctl.o:  ../driver/usbctl.c
	arm-linux-gnueabihf-g++  -c ../driver/usbctl.c -pthread -std=c++11 -Wl,--no-as-needed
#bluetooth.o: ../driver/bluetooth.c
#	arm-linux-gnueabihf-g++  -c ../driver/bluetooth.c -pthread -std=c++11 -Wl,--no-as-needed
# esp8266.o: driver/esp8266.c
# 	arm-linux-gnueabihf-g++  -c driver/esp8266.c -pthread -std=c++11 -Wl,--no-as-needed

.PHONY: clean
clean:
	rm *.o
