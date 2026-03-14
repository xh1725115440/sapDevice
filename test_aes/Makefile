
aes_test: aes_test.o aes_options.o
	arm-linux-gnueabihf-g++ -o  aes_test aes_test.o aes_options.o  -lssl -lcrypto -pthread -std=c++11 -Wl,--no-as-needed
aes_test.o: aes_test.c
	arm-linux-gnueabihf-g++ -c aes_test.c  -lopenssl -lssl -lcrypto -pthread -std=c++11 -Wl,--no-as-needed
aes_options.o: aes_options.c
	arm-linux-gnueabihf-g++ -c aes_options.c   -lssl -lcrypto -pthread -std=c++11 -Wl,--no-as-needed