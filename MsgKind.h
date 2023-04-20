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
#include "FileRecord.h"
using namespace std::chrono_literals;
using namespace std;
class MsgKind
{
private:

    deque<FileRecord> msgs;
    string fileName;
    int lastPos;
    int deqSize;

    ifstream OpenFile();

    bool IsCorrecrt(const string& line, int pos2, int pos3);

    // ������� ���� ������ � ����� � �����
    int FindReverse(istream& file, char searchCh);

    // ������� ���� ������ � ����� � ������������� ������� � �� �����
    int Find(istream& file, char searchCh);


    //������� ������ ��������� �� ��������� ������ �����
    int SetToLastLine(ifstream& file, int startPos);

    //������� ��� ������ ��������� � �������������� ���������
    string ReadMsg(ifstream& file, int fileSize);

    string GetBodyMsg(const string& msg);

    int GetMsgNum(const string& msg);

    string GetTime();

    FileRecord ParseMsg(const string& msg, int priority);

    MessageType FindMsgType(int priority);

    void UpdatePosSize(int last2);

    FileRecord GetBackMsgFromDeq();

    FileRecord GetFrontMsgFromDeq();

public:

    MsgKind(string fileName);

    deque<FileRecord>* GetDeque();

    int GetDeqSize();

    void SetLastPos();
    //������� ������ ���� � �������� ������� ��������� ������ ������
    void ReadMsgsFromFile(int priority);

    static FileRecord GetBackMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos);

    static FileRecord GetFrontMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos);


};

