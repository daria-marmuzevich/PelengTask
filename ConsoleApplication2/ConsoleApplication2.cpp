#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <queue>
#include <thread>
using namespace std::chrono_literals;
using namespace std;

class MeteoStation {

public:

    deque<string> speci_msgs;
    deque<string> metar_msgs;
    deque<string> kn01_msgs;
    deque<string> awos_msgs;

    string speci_file_name = "Send/speci.txt";
    string metar_file_name = "Send/metar.txt";
    string kn01_file_name = "Send/kn01.txt";
    string awos_file_name = "Send/awos.txt";


    int last_pos_speci;
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

    int find_reverse(istream& file, char b) {
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
 
    deque<string>* find_deq(int priority) {
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

        int pos3 = find_reverse(file, 3);
        int pos2 = find_reverse(file, 2);
        int pos_slash = find_reverse(file, '/');
        int pos1 = find_reverse(file, 1);
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
    string read_msg(ifstream& file, int file_size) {
        int cur_pos = file.tellg();
        int pos1 = find(file, 1);
        int pos_slash = find(file, '/');
        //pos_slash = find(file, '/');

        char len_char[4];
        file.read(len_char, 4);
        int msg_len = getLength(len_char);

        file.clear();
        file.seekg(pos_slash + 4);
        int pos2 = find(file, 2);
        file.seekg(pos2 + msg_len + 1);

        //проблема тройка в сообщении 
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
    void read_msgs_from_file(ifstream& file, int priority) {
        int first_line_pos = file.tellg();
        file.seekg(0, ios::end);
        int file_size = file.tellg();
        file.seekg(first_line_pos);

        int pos_slash = find(file, '/');
        //pos_slash = find(file, '/');
        char len_char[4];
        file.read(len_char, 4);
        int msg_len = getLength(len_char);
        file.clear();
        file.seekg(pos_slash + 4);
        int pos2 = find(file, 2);
        file.seekg(pos2 + msg_len + 1);
        int end_first_line_pos = find(file, 3) + 1;

        file.seekg(end_first_line_pos, ios::beg);
        while (end_first_line_pos < file_size) {
            //считываем сообщение 
            file.seekg(end_first_line_pos);
            string msg = read_msg(file, file_size);
            if (!msg.empty()) {
                
                deque<string>* deq = find_deq(priority);
                if (deq)
                    deq->push_back(msg);
                else {
                    throw exception("No such priority");
                }
            }
            end_first_line_pos = file.tellg();
            end_first_line_pos++;
        }
     
        
    }


    string get_back_msg_from_deq(deque<string>& deq) {
        if (deq.empty())
            return "";
        string msg = deq.back();
        deq.pop_back();
        return msg;
    }


    string get_front_msg_from_deq(deque<string>& deq) {
        if (deq.empty())
            return "";
        string msg = deq.front();
        deq.pop_front();
        return msg;
    }
    
    string GetNextRecord() {
        string msg;

        update_deque();



        //достает последние записи
        !speci_msgs.empty() && speci_msgs.size() == speci_deq_size ? msg = get_back_msg_from_deq(speci_msgs) :
            (!metar_msgs.empty() && metar_msgs.size() == metar_deq_size ? msg = get_back_msg_from_deq(metar_msgs) :
                (!kn01_msgs.empty() && kn01_msgs.size() == kn01_deq_size ? msg = get_back_msg_from_deq(kn01_msgs) :
                    (!awos_msgs.empty() && awos_msgs.size() == awos_deq_size ? msg = get_back_msg_from_deq(awos_msgs) : msg = "")));

        if (!msg.empty())
            return msg;


        //достает из всех очередей по очереди
        !speci_msgs.empty() ? msg = get_front_msg_from_deq(speci_msgs) :
            (!metar_msgs.empty() ? msg = get_front_msg_from_deq(metar_msgs) :
                (!kn01_msgs.empty() ? msg = get_front_msg_from_deq(kn01_msgs) : msg=""));

        return msg;
    }

    

    void start_module() {
      
        ifstream speci_file(speci_file_name, ios_base::binary);
        ifstream metar_file(metar_file_name, ios_base::binary);
        ifstream kn01_file(kn01_file_name, ios_base::binary);
        ifstream awos_file(awos_file_name, ios_base::binary);

        speci_file.seekg(0, ios::end);
        metar_file.seekg(0, ios::end);
        kn01_file.seekg(0, ios::end);
        awos_file.seekg(0, ios::end);

        last_pos_speci = setToLastLine(speci_file, 1, speci_file.tellg());
        last_pos_metar = setToLastLine(metar_file, 2, metar_file.tellg());
        last_pos_kn01 = setToLastLine(kn01_file, 3, kn01_file.tellg());
        last_pos_awos = setToLastLine(awos_file, 4, awos_file.tellg());

    }


    void update_deque() {

        ifstream speci_file(speci_file_name, ios_base::binary);
        ifstream metar_file(metar_file_name, ios_base::binary);
        ifstream kn01_file(kn01_file_name, ios_base::binary);
        ifstream awos_file(awos_file_name, ios_base::binary);

        speci_file.seekg(last_pos_speci, ios::beg);
        metar_file.seekg(last_pos_metar, ios::beg);
        kn01_file.seekg(last_pos_kn01, ios::beg);
        awos_file.seekg(last_pos_awos, ios::beg);

        read_msgs_from_file(speci_file, 1);
        read_msgs_from_file(metar_file, 2);
        read_msgs_from_file(kn01_file, 3);
        read_msgs_from_file(awos_file, 4);

        update_pos_size(last_pos_speci, setToLastLine(speci_file, 1, speci_file.tellg()), speci_deq_size, speci_msgs);
        update_pos_size(last_pos_metar, setToLastLine(metar_file, 2, metar_file.tellg()), metar_deq_size, metar_msgs);
        update_pos_size(last_pos_kn01, setToLastLine(kn01_file, 3, kn01_file.tellg()), kn01_deq_size, kn01_msgs);
        update_pos_size(last_pos_awos, setToLastLine(awos_file, 4, awos_file.tellg()), awos_deq_size, awos_msgs);

        speci_file.close();
        metar_file.close();
        kn01_file.close();
        awos_file.close();

    }

    void update_pos_size(int& last1, int last2, int& deq_size, deque<string>& deq) {
        if (last1 != last2) {
            deq_size = deq.size();
            last1 = last2;
        }
        
    }

    void send_msgs(const string &msg) {
        cout << msg << endl;
    }



    void send_queu(deque<string> &msgs) {
        while (!msgs.empty()) {
            send_msgs(msgs.front());
            msgs.pop_front();
        }
    }

 
};

bool connection_to_server() {
    return true;
}

void SendToServer(const string& message) {
    if (connection_to_server()) {
        // Send
    }
}


int main() {
    MeteoStation ms;
 
    ms.start_module();

    cout << "start\n";
    this_thread::sleep_for(60s);
    cout << "start2\n";

    while (connection_to_server()) {
        auto messageToSend = ms.GetNextRecord();
        if (!messageToSend.empty()) {
            cout << messageToSend << endl;
            SendToServer(messageToSend);
        }

        this_thread::sleep_for(100ms);
    }
        
    
    cout << "jjik";

    return 0;
}
