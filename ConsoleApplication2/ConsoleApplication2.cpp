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
    int message_num;			// это из группы N034, три последние цифры; 1000 переходит в 000 в самОй станции
    std::string    time;		// DDHHMM - день месяца, час, минута
    std::string    corr;		// группа исправленного сообщения, пока не обращать внимания
    string      message;	// SPECI UMPL 110852Z 12004MPS 080V160 4700 OVC005 M00/M01 Q1012 R08/710160 TEMPO 1100 BR OVC003=

    FileRecord()
    {
        Clear();
    }
public:
    void Clear()
    {
        type = NOTHING;
        pointer = 0;
        message_num = 0;
        time.clear();
        corr.clear();
        message.clear();
    }

    static void print(FileRecord m) {
        cout << m.message << endl;
        cout << m.time << endl;
    }
};


class MeteoStation {

private:

    deque<FileRecord> speciMsgs;
    deque<FileRecord> metarMsgs;
    deque<FileRecord> kn01Msgs;
    deque<FileRecord> awosMsgs;

    string speciFileName = "Send/speci.txt";
    string metarFileName = "Send/metar.txt";
    string kn01FileName = "Send/kn01.txt";
    string awosFileName = "Send/awos.txt";


    int lastPosSpeci;
    int lastPosMetar;
    int lastPosKn01;
    int lastPosAwos;

    int speciDeqSize;
    int metarDeqSize;
    int kn01DeqSize;
    int awosDeqSize;



    bool IsCorrecrt(const string& line, int pos2, int pos3) {

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
    int FindReverse(istream& file, char searchCh) {
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
    int Find(istream& file, char searchCh) {
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

    deque<FileRecord>* Find_deq(int priority) {
        switch (priority) {
        case 1:
            return &speciMsgs;
        case 2:
            return &metarMsgs;
        case 3:
            return &kn01Msgs;
        case 4:
            return &awosMsgs;
        default:
            return nullptr;

        }
    }

    //функция ставит указатель на последнюю строку файла
    int SetToLastLine(ifstream& file, int priority, int startPos) {
        if (!file.is_open())
        {
            cout << "Cannot open file." << endl;
        }

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
            return SetToLastLine(file, priority, pos1);
        }

    };

    //функция для чтения сообщения с установленного указателя
    string ReadMsg(ifstream& file, int fileSize) {
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
    void ReadMsgsFromFile(ifstream& file, int priority) {
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

                deque<FileRecord>* deq = Find_deq(priority);

                if (deq)
                    deq->push_back(ParseMsg(msg, priority));
                else {
                    throw exception("No such priority");
                }
            }
            endFirstLinePos = file.tellg();
            endFirstLinePos++;
        }


    }


    FileRecord GetBackMsgFromDeq(deque<FileRecord>& deq) {
        FileRecord msg = deq.back();
        deq.pop_back();
        return msg;
    }


    FileRecord GetFrontMsgFromDeq(deque<FileRecord>& deq) {
        FileRecord msg = deq.front();
        deq.pop_front();
        return msg;
    }

    MessageType FindMsgType(int priority) {
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

    int FindLastPos(MessageType mt) {
        switch (mt) {
        case SPECI:
            return lastPosSpeci;
        case METAR:
            return lastPosMetar;
        case KN01:
            return lastPosKn01;
        case AWOS:
            return lastPosAwos;
        default:
            return -1;
        }
    }

    string GetBodyMsg(const string& msg) {
        int pos2 = msg.find(2, 10);
        return msg.substr(pos2 + 1, msg.length() - pos2 - 2);
    }

    int GetMsgNum(const string& msg) {
        int posN = msg.find('N');
        return stoi(msg.substr(posN + 1, 3));

    }

    string GetTime() {
        auto const time = chrono::system_clock::now();
        return format("{:%d%H%M}", time);
    }

    FileRecord ParseMsg(const string& msg, int priority) {
        FileRecord fRec;
        fRec.type = FindMsgType(priority);
        fRec.pointer = FindLastPos(fRec.type);
        fRec.message_num = GetMsgNum(msg);
        fRec.time = GetTime();
        fRec.message = GetBodyMsg(msg);
        return fRec;
    }


    void UpdateDeque() {

        ifstream speciFile(speciFileName, ios_base::binary);
        ifstream metarFile(metarFileName, ios_base::binary);
        ifstream kn01File(kn01FileName, ios_base::binary);
        ifstream awosFile(awosFileName, ios_base::binary);

        speciFile.seekg(lastPosSpeci, ios::beg);
        metarFile.seekg(lastPosMetar, ios::beg);
        kn01File.seekg(lastPosKn01, ios::beg);
        awosFile.seekg(lastPosAwos, ios::beg);

        ReadMsgsFromFile(speciFile, 1);
        ReadMsgsFromFile(metarFile, 2);
        ReadMsgsFromFile(kn01File, 3);
        ReadMsgsFromFile(awosFile, 4);

        UpdatePosSize(lastPosSpeci, SetToLastLine(speciFile, 1, speciFile.tellg()), speciDeqSize, speciMsgs);
        UpdatePosSize(lastPosMetar, SetToLastLine(metarFile, 2, metarFile.tellg()), metarDeqSize, metarMsgs);
        UpdatePosSize(lastPosKn01, SetToLastLine(kn01File, 3, kn01File.tellg()), kn01DeqSize, kn01Msgs);
        UpdatePosSize(lastPosAwos, SetToLastLine(awosFile, 4, awosFile.tellg()), awosDeqSize, awosMsgs);

        speciFile.close();
        metarFile.close();
        kn01File.close();
        awosFile.close();

    }

    void UpdatePosSize(int& last1, int last2, int& deqSize, deque<FileRecord>& deq) {
        if (last1 != last2) {
            deqSize = deq.size();
            last1 = last2;
        }

    }


public:
    
    FileRecord GetNextRecord() {
        FileRecord msg;

        UpdateDeque();

        //достает последние записи
        !speciMsgs.empty() && speciMsgs.size() == speciDeqSize ? msg = GetBackMsgFromDeq(speciMsgs) :
            (!metarMsgs.empty() && metarMsgs.size() == metarDeqSize ? msg = GetBackMsgFromDeq(metarMsgs) :
                (!kn01Msgs.empty() && kn01Msgs.size() == kn01DeqSize ? msg = GetBackMsgFromDeq(kn01Msgs) :
                    (!awosMsgs.empty() && awosMsgs.size() == awosDeqSize ? msg = GetBackMsgFromDeq(awosMsgs) : msg )));

        if (msg.type != MessageType::NOTHING) 
            return msg;
        
        //достает из всех очередей по очереди
        !speciMsgs.empty() ? msg = GetFrontMsgFromDeq(speciMsgs) :
            (!metarMsgs.empty() ? msg = GetFrontMsgFromDeq(metarMsgs) :
                (!kn01Msgs.empty() ? msg = GetFrontMsgFromDeq(kn01Msgs) : msg));

      
        return msg;
    }

   
    void StartModule() {
      
        ifstream speciFile(speciFileName, ios_base::binary);
        ifstream metarFile(metarFileName, ios_base::binary);
        ifstream kn01File(kn01FileName, ios_base::binary);
        ifstream awosFile(awosFileName, ios_base::binary);

        speciFile.seekg(0, ios::end);
        metarFile.seekg(0, ios::end);
        kn01File.seekg(0, ios::end);
        awosFile.seekg(0, ios::end);

        lastPosSpeci = SetToLastLine(speciFile, 1, speciFile.tellg());
        lastPosMetar = SetToLastLine(metarFile, 2, metarFile.tellg());
        lastPosKn01 = SetToLastLine(kn01File, 3, kn01File.tellg());
        lastPosAwos = SetToLastLine(awosFile, 4, awosFile.tellg());

    }
 
};

bool connection_to_server() {
    return true;
}

void SendToServer(const FileRecord& message) {
    if (connection_to_server()) {
        // Send
    }
}


int main() {
    MeteoStation ms;

    //ifstream f("speci.txt", ios_base::binary);
    //f.seekg(0, ios::beg);
    //ms.readMsgsFromFile(f,1);


    //cout << "8888888888";

    ms.StartModule();

    cout << "start\n";
    this_thread::sleep_for(60s);
    cout << "start2\n";

    while (connection_to_server()) {
        auto messageToSend = ms.GetNextRecord();
        if (messageToSend.type != MessageType :: NOTHING) {
            FileRecord::print(messageToSend);

            SendToServer(messageToSend);
        }

        this_thread::sleep_for(100ms);
    }
        
    

    return 0;
}
