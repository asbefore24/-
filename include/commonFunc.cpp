#include "commonFunc.h"

#include <cstring>
#include <algorithm>

string CommonFunc::readFile(const string& path) {
    ifstream file(path);
    if (file.is_open()) {
        stringstream ss;
        ss << file.rdbuf();
        return ss.str();
    }
    cout << "file open failed" << strerror(errno) << endl;
    return "";
}

void CommonFunc::tolower(string& str) {
    transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}