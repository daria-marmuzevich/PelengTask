#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <queue>
#include <sstream>
#include <chrono>
#include <format>
#include "FileRecord.h"
#include "MsgKind.h"
using namespace std::chrono_literals;
using namespace std;

class MeteoStation2 {

    MsgKind speci;
    MsgKind metar;
    MsgKind kn01;
    MsgKind awos;

    void UpdateDeque();
  

public:
    MeteoStation2();

    FileRecord GetNextRecord();


};

