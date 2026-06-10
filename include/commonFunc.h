#ifndef COMMONFUNC
#define COMMONFUNC

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

namespace CommonFunc {
    string readFile(const std::string& path);

    void tolower(string& str);
};

#endif COMMONFUNC