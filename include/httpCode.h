#include <string>
#include <map>

using namespace std;

struct HttpRequest {
    string head;   
    string body;
    string action;  // 请求行为
    string url;     // 请求路径
    string version; // 请求版本
    bool head_recv_end = false;
    bool body_recv_end = false;
    size_t contentLen = 0; // 头内规定的body长度
    map<string, string> keyMap;

    void push_chunk(string data) {
        if (head_recv_end) {
            body += data;
            return;
        }
        if (data.find("\r\n\r\n") == string::npos) {
            head += data;
        } else {
            head_recv_end = true;
            body_recv_end = true;

            head += data.substr(0, data.find("\r\n\r\n"));
            parseHeader();
            body = data.substr(data.find("\r\n\r\n") + 4);
        }
    }

    void parseHeader() {
        size_t pos = head.find("\r\n");
        string firstLine = head.substr(0, pos);
        auto space = firstLine.find(' ');
        if (space != string::npos) {
            action = firstLine.substr(0, space);
        } else {
            action = "GET";
        }
        auto space2 = firstLine.find(' ', space);
        if (space2 != string::npos) {
            url = firstLine.substr(space, space2);
            version = firstLine.substr(space2 + 1);
        } else {
            url = "/";
            version = "http/1.1";
        }
        pos += 2;
        while (pos != string::npos) {
            size_t next = head.find("\r\n", pos);
            string line = head.substr(pos, next - pos);
            CommonFunc::tolower(line);
            auto key = line.substr(0, line.find(": "));
            auto value = line.substr(line.find(": ") + 2);
            if (key == "content-length") {
                contentLen = stoi(value);
            }
            keyMap.insert({key, value});
            if (next != string::npos) {
                pos = next + 2;
            } else {
                break;
            }
        }
    }
};

struct HttpResponse {
    string buffer;

    void beginHead(string first, string second, string third) {
        buffer.append(first);
        buffer.append(" ");
        buffer.append(second);
        buffer.append(" ");
        buffer.append(third);
    }

    void writeHead(string key, string value) {
        buffer.append("\r\n");
        buffer.append(key);
        buffer.append(": ");
        buffer.append(value);
    }

    void endHead() {
        buffer.append("\r\n\r\n");
    }
};