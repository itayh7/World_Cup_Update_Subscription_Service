#pragma once

#include "../include/ConnectionHandler.h"
#include "../include/Game.h"
#include <thread>
#include <mutex>
#include <condition_variable>


using namespace std;

class StompProtocol
{
private:
    const string STOMP_VERSION = "1.2";
    ConnectionHandler* connectionHandler;
    condition_variable cv;
    mutex mtx;
    bool thread_wait;
    int subscription_id;
    int receipt_id;
    int logout_receipt;
    bool connected;
    string username;
    map<string,string> channel_subscription_ids;
    map<string, map<string, Game*>> user_reports; //user >> channel >> game
    

public:

    
    StompProtocol();

    StompProtocol(const StompProtocol& other);
    ~StompProtocol();
    StompProtocol& operator=(const StompProtocol &other);
    StompProtocol(StompProtocol&& other);
    StompProtocol& operator=(StompProtocol&& other);

    void server_input();
    void terminal_input(vector<string> &words);
    void disconnect_client();
    void disconnect_client(string msg);

};
