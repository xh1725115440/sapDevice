#ifndef _BLUETOOTH_H_
#define _BLUETOOTH_H_
#include <string>
using namespace std;
int bluetooth_open(string &device_mac);
int blue_proc_message(string &meg);
#endif
