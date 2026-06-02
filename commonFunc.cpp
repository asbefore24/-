#include "commonFunc.h"

#include <cstring>

string CommonFunc::readFile(const std::string& path) {
    ifstream file(path);
    if (file.is_open()) {
        cout << "file opened" << endl;
        stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
    cout << "file open failed" << strerror(errno) << endl;
    return "";
}