/*
 Шамраев Андрей Евгеньевич
 08.08.2023
 Затраченное время: 6ч
 */

#include <iostream>
#include<fstream>
#include <sstream>
#include <unordered_map>


using namespace std;

//first task
void printBinary(long decimalNumber) {

    bool isNegative = decimalNumber < 0;
    vector<int> res;

    decimalNumber = abs(decimalNumber);
    while (decimalNumber > 0) {
        res.push_back(decimalNumber % 2);
        decimalNumber = decimalNumber / 2;
    }
    
    if (isNegative) {
        bool isPlusOne = true;
        res.push_back(0);
        for (auto& it : res) {
            if (not isPlusOne) {
                it = not it;
                continue;
            }
            
            if (it == 1) {
                isPlusOne = false;
            }
        }
    }

    reverse(begin(res), end(res));
    
    for (auto& binaryRank : res) {
        cout << binaryRank << endl;
    }
}
//first task

//second task
void removeDups(string & str) {
    char currentSymbol = '\0';
    string resStr;

    for (int i = 0; i < str.size(); i++) {
        if (currentSymbol != str[i]) {
            resStr.push_back(str[i]);
            currentSymbol = str[i];
        }
    }
    
    str = resStr;
}

void removeDups(char* str) {
    int writeIndex = 0;
    
    if (str[writeIndex] == '\0') {
        return;
    }
    
    int readIndex = 1;
    for (; str[readIndex] != '\0'; readIndex++) {
        if (str[readIndex] != str[writeIndex]) {
            str[++writeIndex] = str[readIndex];
        }
    }
    
    str[writeIndex+1] = '\0';
}
//second task

//third task
struct ListNode {
    ListNode* prev = nullptr; // указатель на предыдущий элемент списка, либо `nullptr` в случае начала списка
    ListNode* next = nullptr;
    ListNode* rand = nullptr; // указатель на произвольный элемент данного списка, либо `nullptr`
    string data; // произвольные пользовательские данные
};
    
class List {
public:
    void serialize(FILE* file) {
        assert(file);
        
        fprintf(file, "%d\n", count);
        if (isEmpty()) {
            return;
        }
        
        unordered_map<ListNode*, int> nodeIndexes;
        int index = 0;
        
        //write data to file
        for (auto node = head; node != nullptr; node = node->next) {
            fprintf(file, "%s\n",  node->data.c_str());
            nodeIndexes[node] = index++;
        }
        
        //write rand pointers indexes to file
        for (auto node = head; node != nullptr; node = node->next) {
            if (node->rand and nodeIndexes.find(node->rand) != nodeIndexes.end()) {
                fprintf(file, "%d\n", nodeIndexes[node->rand]);
            } else {
                fprintf(file, "%d\n", -1);
            }
        }
    }
    
    void deserialize(FILE* file) {
        assert(file);
        
        clear();
        
        int length = -1;
        fscanf(file, "%d", &length);
        getc(file);
        
        assert(length >= 0);
        
        if (length <= 0) {
            return;
        }

        std::vector<ListNode*> nodes;
        nodes.reserve(length);
        
        //add elements to list
        for (int index = 0; index < length; ++index) {
            string str;
            
//          Если не использовать стандарт С++17
            char ch = getc(file);
            while (ch != '\n') {
                str += ch;
                ch = getc(file);
            }
//          Если не использовать стандарт С++17
            
            //Если использовать стандарт C++17 можно сделать чтение названия нод так fscanf(file, "%s", str.data());
            
            addElement(str);
            nodes.push_back(tail);
        }
        
        int index;
        //fill rand pointers
        for (auto node = head; node != nullptr; node = node->next) {
            fscanf(file, "%d", &index);
            node->rand = index >= 0 && index < count ? nodes[index] : nullptr;
        }
    }
    
    void serialize(ostream& stream) {
        assert(stream.good());
        
        stream << count << endl;
        if (isEmpty()) {
            return;
        }
        
        unordered_map<ListNode*, int> nodeIndexes;
        int index = 0;
        
        //write data to file
        for (auto node = head; node != nullptr; node = node->next) {
            stream << node->data << endl;
            nodeIndexes[node] = index++;
        }
        
        //write rand pointers indexes to file
        for (auto node = head; node != nullptr; node = node->next) {
            if (node->rand and nodeIndexes.find(node->rand) != nodeIndexes.end()) {
                stream << nodeIndexes[node->rand] << endl;
            } else {
                stream << -1 << endl;
            }
        }

        assert(stream);
    }

    void deserialize(istream& stream) {
        assert(stream.good());
        
        clear();
        
        int length = -1;
        stream >> length;
        
        assert(length >= 0);
        
        if (length <= 0) {
            return;
        }

        std::vector<ListNode*> nodes;
        nodes.reserve(length);
        
        //add elements to list
        for (int index = 0; index < length; ++index) {
            string str;
            stream >> str;
            assert(stream.good());
            addElement(str);
            nodes.push_back(tail);
        }
        
        int index;
        //fill rand pointers
        for (auto node = head; node != nullptr; node = node->next) {
            stream >> index;
            node->rand = index >= 0 && index < count ? nodes[index] : nullptr;
        }
    }
    
    bool isEmpty() const {
        return not head and not tail;
    }
    
    void clear() {
        while (head) {
            ListNode* nextNode = head->next;
            delete head;
            head = nextNode;
        }
        
        count = 0;
        head = nullptr;
        tail = nullptr;
    }
    
    void addElement(const string& str, bool toHead = false) {
        count++;
        
        ListNode* node = new ListNode;
        node->data = str;
    
        if (isEmpty()) {
            head = tail = node;
            return;
        }
        
        if (not toHead) {
            tail->next = node;
            node->prev = tail;
            tail = node;
            return;
        }
        
        head->prev = node;
        node->next = head;
        head = node;
    }
    
    void fillRandNodes() {
        srand(time(nullptr));
        
        for (auto node = head; node; node = node->next) {
            int randIndex = -1 + rand() % count;
            if (randIndex == -1) {
                continue;
            }
            
            int index = 0;
            ListNode* randNode = node;
            while (index < randIndex) {
                randNode = node->next;
                index++;
            }
            node->rand = randNode;
        }
    }
    
private:
    ListNode* head = nullptr;
    ListNode* tail = nullptr;
    int count = 0;
};
//third task

int main(int argc, const char * argv[]) {
    int taskNumber = 0;
    
    cout << "What task do you want to run? Available options are 1, 2, 3. Enter 0 to exit." << endl;
    cin >> taskNumber;
    
    while (taskNumber) {
        switch (taskNumber) {
            case 1: {
                long testNumber = 8;
                printBinary(testNumber);
            }
                break;
            case 2: {
                string data = "AAA BBB     AAAA";
                char data1[] = "AAA BBB     AAAA";
                removeDups(data);
                removeDups(data1);
                cout << data << endl;
                cout << data1 << endl;
            }
                break;
            case 3: {
                List testList;
                //Fill test list
                testList.addElement("node1");
                testList.addElement("node2");
                testList.addElement("node3", true);
                testList.addElement("node4");
                testList.fillRandNodes();
                
                FILE* wf = fopen("test.dat", "wb");
                testList.serialize(wf);
                fclose(wf);
                
                FILE* rf = fopen("test.dat", "rb");
                testList.deserialize(rf);
                fclose(rf);
            }
            default:
                break;
        }
        cout << "What task do you want to run? Available options are 1, 2, 3. Enter 0 to exit." << endl;
        cin >> taskNumber;
    }
    
    return 0;
}
