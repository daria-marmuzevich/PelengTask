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

public:

    deque<FileRecord> speci_msgs;
    deque<FileRecord> metar_msgs;
    deque<FileRecord> kn01_msgs;
    deque<FileRecord> awos_msgs;

    string speci_file_name = "Send/speci.txt";
    string metar_file_name = "Send/metar.txt";
    string kn01_file_name = "Send/kn01.txt";
    string awos_file_name = "Send/awos.txt";


    int lastPosSpeci;
    int last_pos_metar;
    int last_pos_kn01;
    int last_pos_awos;

    int speci_deq_size;
    int metar_deq_size;
    int kn01_deq_size;
    int awos_deq_size;


    int getLength(char len_char[4]) {

        union {
            char len_char_reverse[4];
            int len;
        };
        reverse_copy(len_char, len_char + 4, len_char_reverse);

        return len;
    }



    bool isCorrecrt(string line, int pos2, int pos3) {

        char g[4];
        line.copy(g, 4, 6);
        union {
            char g_reverse[4];
            int len;
        };
        reverse_copy(g, g + 4, g_reverse);
      
        return pos3 - pos2 - 1 == len;    }

    int findReverse(istream& file, char b) {
        int fileSize = file.tellg();

        bool found = false;
        while (!found && fileSize > 0)
        {
            file.seekg(--fileSize);
            char c = file.get();
            if (uint16_t(c) == b) {
                return fileSize;
            }
        }
        return -1;
           
    }

    // функция ищет символ в файле с установленной позиции и до конца
    int find(istream& file, char b) {
        int start_pos = file.tellg();
        file.seekg(0, ios::end);
        int file_size = file.tellg();
        file.seekg(start_pos);
        while (start_pos < file_size)
        {
            file.seekg(start_pos);
            char c = file.get();
            if (uint16_t(c) == b) {
                return start_pos;
            }
            start_pos++;
        }
        return -1;

    }
 
    deque<FileRecord>* find_deq(int priority) {
        switch (priority) {
        case 1:
            return &speci_msgs;
        case 2:
            return &metar_msgs;
        case 3:
            return &kn01_msgs;
        case 4:
            return &awos_msgs;
        default:
            return nullptr;

        }
    }

    //функция ставит указатель на последнюю строку файла
    int setToLastLine(ifstream& file, int priority, int start_pos) {
        if (!file.is_open())
        {
            cout << "Cannot open file." << endl;
        }

        //переместить указатель чтения в конец файла
        file.seekg(start_pos);

        // получить позицию указателя чтения (то есть размер файла)
        int fileSize = file.tellg();


        // если нашли начало строки, то переместить указатель чтения на эту позицию

        int pos3 = findReverse(file, 3);
        int pos2 = findReverse(file, 2);
        int pos_slash = findReverse(file, '/');
        int pos1 = findReverse(file, 1);
        if (pos3 != -1 && pos2 != -1 && pos_slash != -1 && pos1 != -1)
            file.seekg(pos1);
        else {
            // может поместить указатель в начало файла??
            file.seekg(0, ios::beg);
            cout << "No messages in file"<<endl;
            return 0;
        }
       
        string line_temp;
        string line;
        while (getline(file, line_temp)) {
            line = line + line_temp;
            line.push_back('\n');
        }
        file.clear();
       
        if (isCorrecrt(line, pos2, pos3)) {

            file.seekg(pos1);
            return pos1;
        }
        else {
            file.seekg(pos1);
            return setToLastLine(file, priority, pos1);

        }
        
    };



    //функция для чтения сообщения с установленного указателя
    string readMsg(ifstream& file, int file_size) {
        int cur_pos = file.tellg();
        int pos1 = find(file, 1);
        int pos_slash = find(file, '/');
        //pos_slash = find(file, '/');

        //char len_char[4];
        //file.read(len_char, 4);
        //int msg_len = getLength(len_char);

        //file.clear();
        file.seekg(pos_slash + 4);
        int pos2 = find(file, 2);
        //file.seekg(pos2 + msg_len + 1);

         
        int pos3 = find(file, 3);

        if (pos3 != -1 && pos2 != -1 && pos_slash != -1 && pos1 != -1)
            file.seekg(pos1);
        else {
            file.clear();
            file.seekg(0, ios::end);
            int p = file.tellg();
            //cout << "No new messages"<<endl;
            return "";
        }

        string line(pos3-pos1+1, '\0');
        file.read((char*)line.data(), pos3 - pos1 + 1);
       
        file.clear();
        file.seekg(pos3 + 1);

        if (isCorrecrt(line, pos2, pos3)) {
            return line;
        }
        else {
            return "";
        }


    }

    //функция читает файл с заданной позиции пропуская первую строку
    void readMsgsFromFile(ifstream& file, int priority) {
        int first_line_pos = file.tellg();
        file.seekg(0, ios::end);
        int file_size = file.tellg();
        file.seekg(first_line_pos);

        int pos_slash = find(file, '/');
        //pos_slash = find(file, '/');

       /* char len_char[4];
        file.read(len_char, 4);
        int msg_len = getLength(len_char);
        file.clear();*/

        file.seekg(pos_slash + 4);
        int pos2 = find(file, 2);
        int pos3 = find(file, 3);
        //file.seekg(pos2 + msg_len + 1);
        //int end_first_line_pos = find(file, 3) + 1;
        int end_first_line_pos = pos3 + 1;

        file.seekg(end_first_line_pos, ios::beg);
        while (end_first_line_pos < file_size) {
            //считываем сообщение 
            file.seekg(end_first_line_pos);
            string msg = readMsg(file, file_size);
            if (!msg.empty()) {
                
                deque<FileRecord>* deq = find_deq(priority);

                if (deq)
                    deq->push_back(parseMsg(msg, priority));
                else {
                    throw exception("No such priority");
                }
            }
            end_first_line_pos = file.tellg();
            end_first_line_pos++;
        }
     
        
    }


    FileRecord getBackMsgFromDeq(deque<FileRecord>& deq) {
        FileRecord msg = deq.back();
        deq.pop_back();
        return msg;
    }


    FileRecord getFrontMsgFromDeq(deque<FileRecord>& deq) {
        FileRecord msg = deq.front();
        deq.pop_front();
        return msg;
    }
    
    FileRecord GetNextRecord() {
        FileRecord msg;

        updateDeque();



        //достает последние записи
        !speci_msgs.empty() && speci_msgs.size() == speci_deq_size ? msg = getBackMsgFromDeq(speci_msgs) :
            (!metar_msgs.empty() && metar_msgs.size() == metar_deq_size ? msg = getBackMsgFromDeq(metar_msgs) :
                (!kn01_msgs.empty() && kn01_msgs.size() == kn01_deq_size ? msg = getBackMsgFromDeq(kn01_msgs) :
                    (!awos_msgs.empty() && awos_msgs.size() == awos_deq_size ? msg = getBackMsgFromDeq(awos_msgs) : msg )));

        if (msg.type != MessageType::NOTHING) 
            return msg;
        


        //достает из всех очередей по очереди
        !speci_msgs.empty() ? msg = getFrontMsgFromDeq(speci_msgs) :
            (!metar_msgs.empty() ? msg = getFrontMsgFromDeq(metar_msgs) :
                (!kn01_msgs.empty() ? msg = getFrontMsgFromDeq(kn01_msgs) : msg));

      
        return msg;
    }

    MessageType findMsgType(int priority) {
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

    int findLastPos(MessageType mt) {
        switch (mt) {
        case SPECI:
            return lastPosSpeci;
        case METAR:
            return last_pos_metar;
        case KN01:
            return last_pos_kn01;
        case AWOS:
            return last_pos_awos;
        default:
            return -1;
        }
    }

    string getBodyMsg(const string& msg) {
        int pos2 = msg.find(2, 10);
        return msg.substr(pos2 + 1, msg.length() - pos2 - 2);
    }

    int getMsgNum(const string& msg) {
        int posN = msg.find('N');
        return stoi(msg.substr(posN + 1, 3));

    }

    string getTime() {

        auto const time = chrono::system_clock::now();
        return format("{:%d%H%M}", time);
    }

    FileRecord parseMsg(const string& msg, int priority) {
        FileRecord fRec;
        fRec.type = findMsgType(priority);
        fRec.pointer = findLastPos(fRec.type);
        fRec.message_num = getMsgNum(msg);
        fRec.time = getTime();
        fRec.message = getBodyMsg(msg);
        return fRec;
    }

    void startModule() {
      
        ifstream speci_file(speci_file_name, ios_base::binary);
        ifstream metar_file(metar_file_name, ios_base::binary);
        ifstream kn01_file(kn01_file_name, ios_base::binary);
        ifstream awos_file(awos_file_name, ios_base::binary);

        speci_file.seekg(0, ios::end);
        metar_file.seekg(0, ios::end);
        kn01_file.seekg(0, ios::end);
        awos_file.seekg(0, ios::end);

        lastPosSpeci = setToLastLine(speci_file, 1, speci_file.tellg());
        last_pos_metar = setToLastLine(metar_file, 2, metar_file.tellg());
        last_pos_kn01 = setToLastLine(kn01_file, 3, kn01_file.tellg());
        last_pos_awos = setToLastLine(awos_file, 4, awos_file.tellg());

    }


    void updateDeque() {

        ifstream speci_file(speci_file_name, ios_base::binary);
        ifstream metar_file(metar_file_name, ios_base::binary);
        ifstream kn01_file(kn01_file_name, ios_base::binary);
        ifstream awos_file(awos_file_name, ios_base::binary);

        speci_file.seekg(lastPosSpeci, ios::beg);
        metar_file.seekg(last_pos_metar, ios::beg);
        kn01_file.seekg(last_pos_kn01, ios::beg);
        awos_file.seekg(last_pos_awos, ios::beg);

        readMsgsFromFile(speci_file, 1);
        readMsgsFromFile(metar_file, 2);
        readMsgsFromFile(kn01_file, 3);
        readMsgsFromFile(awos_file, 4);

        //найти время

        UpdatePosSize(lastPosSpeci, setToLastLine(speci_file, 1, speci_file.tellg()), speci_deq_size, speci_msgs);
        UpdatePosSize(last_pos_metar, setToLastLine(metar_file, 2, metar_file.tellg()), metar_deq_size, metar_msgs);
        UpdatePosSize(last_pos_kn01, setToLastLine(kn01_file, 3, kn01_file.tellg()), kn01_deq_size, kn01_msgs);
        UpdatePosSize(last_pos_awos, setToLastLine(awos_file, 4, awos_file.tellg()), awos_deq_size, awos_msgs);

        speci_file.close();
        metar_file.close();
        kn01_file.close();
        awos_file.close();

    }

    void UpdatePosSize(int& last1, int last2, int& deq_size, deque<FileRecord>& deq) {
        if (last1 != last2) {
            deq_size = deq.size();
            last1 = last2;
        }
        
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

    ms.startModule();

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
