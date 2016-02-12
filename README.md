# usignal
mikro signal slot lib in one small header file for c++ 11+

mikro signal v 1.0
copyright Pedro Simoes
Licensed as mit or apache or lgpl v2+ or gpl v2+
choose your poison
what, no bsd, not at the moment, i'll think about it.

what, no emit function? why if you can just call the slot...

what makes more sense signal.on_click.emit() or just signal.on_click() ....

Example usage


```c++

#include <iostream>
#include <string>
#include "usignal.h"

using namespace usignal;

class Button {
public:
signal<> on_click;
signal<int> on_click2; //signal<void(int)> 
signal<int(int, int)> sum;
};

class Message {
std::string id;
public:
Message(std::string s) :id{ s } {}

void run_void() {
std::cout << "Hello World! from Slot " << id << std::endl;
}

void run_int(int i) {
std::cout << "Hello World! from Slot " << id << " runing  int param " << i << std::endl;
}

int median(int i, int j) { return (i + j)/2; };
};

int main() {
Button button;
Message m{ "M1" };
auto s1 = button.on_click.connect(&Message::run_void, m);
auto s2 = button.on_click2.connect(&Message::run_int, m);
button.on_click();
button.on_click2(42);
std::cout << std::endl << "test return" << std::endl;

auto l1=[](int i,int j)
{
static int a = 0;
return i +j + a++;
};

button.sum.connect(l1);
button.sum.connect(&Message::median, m);
button.sum.connect(l1);
auto test = button.sum(10, 1);;
for (auto i : test)
std::cout << i.get() << std::endl;

auto l2 = [](int i)
{
static int a = 0;
return i+a++;
};

signal<int(int)> signaltest;
signaltest.async(false); // in case you don't want threaded calls

auto s=signaltest.connect(l2);
signaltest.connect(l2);
auto test2 = signaltest(0);
// signaltest.toggle(s) // would toggle a specific slot in this case disable it as it is enabled by default
                        // without params would toggle the whole signal
signaltest.disconnect(s); // example slot disconnect
signaltest.disconnect(); // disconnect the signal from all the slots
for (auto i : test2)
std::cout << i.get() << std::endl;
std::cin.get();
return 0;
}
