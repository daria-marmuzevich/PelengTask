#include "FileRecord.h"
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <queue>
#include <thread>
#include <sstream>
#include <chrono>
#include <format>
using namespace std::chrono_literals;
using namespace std;

FileRecord::FileRecord()
{
    Clear();
}

void FileRecord::Clear(){
    type = NOTHING;
    pointer = 0;
    message_num = 0;
    time.clear();
    corr.clear();
    message.clear();
}

void FileRecord::print(FileRecord m) {
    cout << m.message << endl;
    cout << m.time << endl;
}
