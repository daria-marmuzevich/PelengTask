#pragma once
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

enum MessageType
{
    NOTHING = 0,
    METAR = 1,
    SPECI = 2,
    AWOS = 3,
    KN01 = 4,
    TREND = 5,
    AEROWARNING = 6,
    WINDSHIFT = 7,
    STATE = 8
};
struct FileRecord
    {
        MessageType type;
        int pointer;				// указатель на номер символа <1> последней записи в соответствующем файле
        int message_num;			// это из группы N034, три последние цифры; 1000 переходит в 000 в самќй станции
        std::string    time;		// DDHHMM - день мес€ца, час, минута
        std::string    corr;		// группа исправленного сообщени€, пока не обращать внимани€
        string      message;	// SPECI UMPL 110852Z 12004MPS 080V160 4700 OVC005 M00/M01 Q1012 R08/710160 TEMPO 1100 BR OVC003=

        FileRecord();
 
        void Clear();

        static void print(FileRecord m);
};

