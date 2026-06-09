#ifndef USER
#define USER

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <string>

using namespace std;

struct User {
    string username;
    string password;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(User, username, password);
};

unordered_map<string, string> userMap = {{"test", "123"}, {"admin", "123"}};

struct UserMessage {
    string username;
    string time;
    string message;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(UserMessage, time, username, message);
};

#endif