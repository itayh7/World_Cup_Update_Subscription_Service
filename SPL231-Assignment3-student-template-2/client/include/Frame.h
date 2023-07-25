#pragma once

#include <string>
#include <iostream>
#include <map>
#include <vector>

// TODO: implement the STOMP protocol

using std::string;
using std::map;
using std::vector;

class Frame
{
private:
    string command;
    map<string, string> headers;
    string body;
public:
    Frame(string command, map<string, string> &headers, string body);
    Frame(const string &s);
    const string &get_command() const;
    const map<string, string> &get_headers() const;
    const string &get_body() const;
    const string to_string() const;
};