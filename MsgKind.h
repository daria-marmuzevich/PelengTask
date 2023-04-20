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

    // функция ищет символ в файле с конца
    int FindReverse(istream& file, char searchCh);

    // функция ищет символ в файле с установленной позиции и до конца
    int Find(istream& file, char searchCh);


    //функция ставит указатель на последнюю строку файла
    int SetToLastLine(ifstream& file, int startPos);

    //функция для чтения сообщения с установленного указателя
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
    //функция читает файл с заданной позиции пропуская первую строку
    void ReadMsgsFromFile(int priority);

    static FileRecord GetBackMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos);

    static FileRecord GetFrontMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos);


};

