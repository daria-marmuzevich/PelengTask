#include <iostream>
#include <thread>
#include "MeteoStation2.h"
#include "FileRecord.h"

using namespace std;

int main() {
    MeteoStation2 ms;

    cout << "start\n";
    this_thread::sleep_for(30s);
    cout << "start2\n";

    while (true) {
        auto messageToSend = ms.GetNextRecord();
        if (messageToSend.type != MessageType::NOTHING) {
            FileRecord::print(messageToSend);
        }

        this_thread::sleep_for(100ms);
    }

}