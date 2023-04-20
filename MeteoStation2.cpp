#include "MeteoStation2.h"
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
#include "FileRecord.h"
#include "MsgKind.h"
using namespace std::chrono_literals;
using namespace std;

MeteoStation2::MeteoStation2() : speci("Send/speci.txt"), metar("Send/metar.txt"),
						awos("Send/awos.txt"), kn01("Send/kn01.txt") {
	//определение последних строк
	speci.SetLastPos();
	metar.SetLastPos();
	kn01.SetLastPos();
	awos.SetLastPos();
};





void MeteoStation2:: UpdateDeque() {
	speci.ReadMsgsFromFile(1);
	metar.ReadMsgsFromFile(2);
	kn01.ReadMsgsFromFile(3);
	awos.ReadMsgsFromFile(4);
}

FileRecord MeteoStation2::GetNextRecord() {
    FileRecord msg;
    UpdateDeque();
	msg = MsgKind::GetBackMsg(speci, metar, kn01, awos);

    if (msg.type != MessageType::NOTHING)
        return msg;

    //достает из всех очередей по очереди
	msg = MsgKind::GetFrontMsg(speci, metar, kn01, awos);

    return msg;
}

