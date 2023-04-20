#include "MsgKind.h"
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

MsgKind::MsgKind(string fileName) {
	this->fileName = fileName;
}

deque<FileRecord>* MsgKind::GetDeque() {
    return &(this->msgs);
}

int MsgKind::GetDeqSize() {
    return this->deqSize;
}

ifstream MsgKind::OpenFile() {
    ifstream file(this->fileName, ios_base::binary);
    if (!file.is_open())
    {
        cout << "Cannot open file." << endl;
    }
    return file;
}

void MsgKind::SetLastPos() {
    ifstream file = OpenFile();
    file.seekg(0, ios::end);
    this->lastPos = SetToLastLine(file, file.tellg());
    file.close();
}

bool MsgKind::IsCorrecrt(const string& line, int pos2, int pos3) {

    char subBytes[4];
    line.copy(subBytes, 4, 6);
    union {
        char subBytesReverse[4];
        int len;
    };
    reverse_copy(subBytes, subBytes + 4, subBytesReverse);

    return pos3 - pos2 - 1 == len;
}

// функция ищет символ в файле с конца
int MsgKind::FindReverse(istream& file, char searchCh) {
    int fileSize = file.tellg();
    bool found = false;
    while (!found && fileSize > 0)
    {
        file.seekg(--fileSize);
        char ch = file.get();
        if (uint16_t(ch) == searchCh) {
            return fileSize;
        }
    }
    return -1;

}

// функция ищет символ в файле с установленной позиции и до конца
int MsgKind::Find(istream& file, char searchCh) {
    int startPos = file.tellg();
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(startPos);
    while (startPos < fileSize)
    {
        file.seekg(startPos);
        char ch = file.get();
        if (uint16_t(ch) == searchCh) {
            return startPos;
        }
        startPos++;
    }
    return -1;

}

//функция ставит указатель на последнюю строку файла
int MsgKind::SetToLastLine(ifstream& file, int startPos) {

    //переместить указатель чтения в конец файла
    file.seekg(startPos);

    // получить позицию указателя чтения (то есть размер файла)
    int fileSize = file.tellg();


    // если нашли начало строки, то переместить указатель чтения на эту позицию

    int pos3 = FindReverse(file, 3);
    int pos2 = FindReverse(file, 2);
    int posSlash = FindReverse(file, '/');
    int pos1 = FindReverse(file, 1);
    if (pos3 != -1 && pos2 != -1 && posSlash != -1 && pos1 != -1)
        file.seekg(pos1);
    else {
        file.seekg(0, ios::beg);
        return 0;
    }

    string lineTemp;
    string line;
    while (getline(file, lineTemp)) {
        line = line + lineTemp;
        line.push_back('\n');
    }
    file.clear();

    if (IsCorrecrt(line, pos2, pos3)) {
        file.seekg(pos1);
        return pos1;
    }
    else {
        file.seekg(pos1);
        return SetToLastLine(file, pos1);
    }

};

//функция для чтения сообщения с установленного указателя
string MsgKind::ReadMsg(ifstream& file, int fileSize) {
    int curPos = file.tellg();
    int pos1 = Find(file, 1);
    int posSlash = Find(file, '/');
    //pos_slash = find(file, '/');

    file.seekg(posSlash + 4);
    int pos2 = Find(file, 2);
    int pos3 = Find(file, 3);

    if (pos3 != -1 && pos2 != -1 && posSlash != -1 && pos1 != -1)
        file.seekg(pos1);
    else {
        file.clear();
        file.seekg(0, ios::end);
        return "";
    }

    string line(pos3 - pos1 + 1, '\0');
    file.read((char*)line.data(), pos3 - pos1 + 1);

    file.clear();
    file.seekg(pos3 + 1);

    if (IsCorrecrt(line, pos2, pos3)) {
        return line;
    }
    else {
        return "";
    }


}

//функция читает файл с заданной позиции пропуская первую строку
void MsgKind::ReadMsgsFromFile(int priority) {
    ifstream file = OpenFile();
    file.seekg(this->lastPos, ios::beg);
    int firstLinePos = file.tellg();
    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    file.seekg(firstLinePos);

    int pos_slash = Find(file, '/');
    //pos_slash = find(file, '/');
    file.seekg(pos_slash + 4);
    int pos2 = Find(file, 2);
    int pos3 = Find(file, 3);

    int endFirstLinePos = pos3 + 1;

    file.seekg(endFirstLinePos, ios::beg);
    while (endFirstLinePos < fileSize) {
        //считываем сообщение 
        file.seekg(endFirstLinePos);
        string msg = ReadMsg(file, fileSize);
        if (!msg.empty()) {
            this->msgs.push_back(ParseMsg(msg, priority));
        }
        endFirstLinePos = file.tellg();
        endFirstLinePos++;
    }

    UpdatePosSize(SetToLastLine(file, file.tellg()));
    file.close();
}


string MsgKind :: GetBodyMsg(const string& msg) {
    int pos2 = msg.find(2, 10);
    return msg.substr(pos2 + 1, msg.length() - pos2 - 2);
}

int MsgKind :: GetMsgNum(const string& msg) {
    int posN = msg.find('N');
    return stoi(msg.substr(posN + 1, 3));
}

string MsgKind :: GetTime() {
    auto const time = chrono::system_clock::now();
    return format("{:%d%H%M}", time);
}

MessageType MsgKind::FindMsgType(int priority) {
    switch (priority) {
    case 1:
        return MessageType::SPECI;
    case 2:
        return MessageType::METAR;
    case 3:
        return MessageType::KN01;
    case 4:
        return MessageType::AWOS;
    default:
        return MessageType::NOTHING;
    }
}

FileRecord MsgKind :: ParseMsg(const string& msg, int priority) {
    FileRecord fRec;
    fRec.type = FindMsgType(priority);
    fRec.pointer = this->lastPos;
    fRec.message_num = GetMsgNum(msg);
    fRec.time = GetTime();
    fRec.message = GetBodyMsg(msg);
    return fRec;
}

void MsgKind::UpdatePosSize(int last2) {
    if (this->lastPos != last2) {
        this->deqSize = this->msgs.size();
        this->lastPos = last2;
    }

}

FileRecord MsgKind::GetBackMsgFromDeq() {
    FileRecord msg = this->msgs.back();
    this->msgs.pop_back();
    return msg;
}


FileRecord MsgKind::GetFrontMsgFromDeq() {
    FileRecord msg = this->msgs.front();
    this->msgs.pop_front();
    return msg;
}

FileRecord MsgKind::GetBackMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos) {
    //достает последние записи
    FileRecord msg;
   
    deque<FileRecord>* spDeq = speci.GetDeque();
    deque<FileRecord>* mDeq = metar.GetDeque();
    deque<FileRecord>* kDeq = kn01.GetDeque();
    deque<FileRecord>* aDeq = awos.GetDeque();

    !spDeq->empty() && spDeq->size() == speci.GetDeqSize() ? msg = speci.GetBackMsgFromDeq() :
        (!mDeq->empty() && mDeq->size() == metar.GetDeqSize() ? msg = metar.GetBackMsgFromDeq() :
            (!kDeq->empty() && kDeq->size() == kn01.GetDeqSize() ? msg = kn01.GetBackMsgFromDeq():
                (!aDeq->empty() && aDeq->size() == awos.GetDeqSize() ? msg = awos.GetBackMsgFromDeq() : msg)));

    return msg;
}


FileRecord MsgKind::GetFrontMsg(MsgKind& speci, MsgKind& metar, MsgKind& kn01, MsgKind& awos) {
    //достает последние записи
    FileRecord msg;

    deque<FileRecord>* spDeq = speci.GetDeque();
    deque<FileRecord>* mDeq = metar.GetDeque();
    deque<FileRecord>* kDeq = kn01.GetDeque();
    deque<FileRecord>* aDeq = awos.GetDeque();

    !spDeq->empty() ? msg = speci.GetFrontMsgFromDeq() :
        (!mDeq->empty() ? msg = metar.GetFrontMsgFromDeq() :
            (!kDeq->empty() ? msg = kn01.GetFrontMsgFromDeq() : msg));

    return msg;
}

