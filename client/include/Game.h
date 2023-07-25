#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
// TODO: implement the STOMP protocol
using namespace std;

class Game
{
private:
    string user;
    string game_name;
    map<string, string> general_stats;
    map<string, string> team_a_stats;
    map<string, string> team_b_stats;
    map<string, string> game_event_reports;
public:
    Game(string user, string game_name);
    void summary(string file_path);
    void report(string body);
    Game* clone();

};