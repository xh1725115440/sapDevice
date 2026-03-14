all: modbus_9001 clean

modbus_9001: modbus_9001.o usbctl.o lora.o  bluetooth.o get.o protocol_process.o softwarewdt.o communicaManage.o registManage.o usart.o dataStore.o
	arm-linux-gnueabihf-g++ -o modbus_9001 modbus_9001.o get.o usbctl.o lora.o  bluetooth.o protocol_process.o softwarewdt.o communicaManage.o registManage.o usart.o dataStore.o -L. -lmodbus -pthread -std=c++11 -Wl,--no-as-needed

modbus_9001.o: modbus_9001.cpp
	arm-linux-gnueabihf-g++ -c modbus_9001.cpp  -pthread -std=c++11 -Wl,--no-as-needed
usbctl.o:  driver/usbctl.c
	arm-linux-gnueabihf-g++  -c driver/usbctl.c -pthread -std=c++11 -Wl,--no-as-needed
bluetooth.o: driver/bluetooth.c
	arm-linux-gnueabihf-g++  -c driver/bluetooth.c -pthread -std=c++11 -Wl,--no-as-needed
# esp8266.o: driver/esp8266.c
# 	arm-linux-gnueabihf-g++  -c driver/esp8266.c -pthread -std=c++11 -Wl,--no-as-needed
lora.o: driver/lora.c
	arm-linux-gnueabihf-g++  -c driver/lora.c -pthread -std=c++11 -Wl,--no-as-needed
get.o: driver/get.cpp
	arm-linux-gnueabihf-g++  -c driver/get.cpp -pthread -std=c++11 -Wl,--no-as-needed
protocol_process.o: driver/protocol_process.cpp
	arm-linux-gnueabihf-g++  -c driver/protocol_process.cpp -pthread -std=c++11 -Wl,--no-as-needed	
softwarewdt.o:	driver/softwarewdt.cpp
	arm-linux-gnueabihf-g++  -c driver/softwarewdt.cpp -pthread -std=c++11 -Wl,--no-as-needed
communicaManage.o:	driver/communicaManage.cpp
	arm-linux-gnueabihf-g++  -c driver/communicaManage.cpp -pthread -std=c++11 -Wl,--no-as-needed
registManage.o:	driver/registManage.cpp
	arm-linux-gnueabihf-g++  -c driver/registManage.cpp -pthread -std=c++11 -Wl,--no-as-needed	
usart.o: driver/usart.cpp
	arm-linux-gnueabihf-g++  -c driver/usart.cpp -pthread -std=c++11 -Wl,--no-as-needed
dataStore.o: driver/dataStore.cpp
	arm-linux-gnueabihf-g++  -c driver/dataStore.cpp -pthread -std=c++11 -Wl,--no-as-needed
.PHONY: clean
clean:
	rm *.o
