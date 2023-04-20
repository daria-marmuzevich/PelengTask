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
        int pointer;				// ��������� �� ����� ������� <1> ��������� ������ � ��������������� �����
        int message_num;			// ��� �� ������ N034, ��� ��������� �����; 1000 ��������� � 000 � ����� �������
        std::string    time;		// DDHHMM - ���� ������, ���, ������
        std::string    corr;		// ������ ������������� ���������, ���� �� �������� ��������
        string      message;	// SPECI UMPL 110852Z 12004MPS 080V160 4700 OVC005 M00/M01 Q1012 R08/710160 TEMPO 1100 BR OVC003=

        FileRecord();
 
        void Clear();

        static void print(FileRecord m);
};

