#include "../include/StompProtocol.h"
#include "../include/Frame.h"
#include "../include/Game.h"
#include "../include/event.h"
#include <string>
#include <fstream>
#include <iostream>


using namespace std;

StompProtocol::StompProtocol() : connectionHandler(nullptr), cv(), mtx(), thread_wait(false),subscription_id(0), receipt_id(0), logout_receipt(-1), connected(false), username(""), channel_subscription_ids({}), user_reports({}){

}

StompProtocol::StompProtocol(const StompProtocol& other) : connectionHandler(nullptr), cv(), mtx(), thread_wait(false), subscription_id(other.subscription_id), receipt_id(other.receipt_id), logout_receipt(other.logout_receipt), connected(other.connected), username(other.username), channel_subscription_ids(other.channel_subscription_ids), user_reports({}){

    for(auto &key_value_outer: other.user_reports)//user>>channel>>game
        for(auto &key_value_inner: key_value_outer.second)
            user_reports.at(key_value_outer.first).insert(make_pair(key_value_inner.first, key_value_inner.second->clone()));
}

StompProtocol::~StompProtocol(){
    cout<<"called ~StompProtocol distructor"<<endl;
    delete connectionHandler;
    for(auto &key_value_outer: user_reports)//user>>channel>>game
        for(auto &key_value_inner: key_value_outer.second)
            delete key_value_inner.second; //delete *Game
}

StompProtocol& StompProtocol::operator=(const StompProtocol &other){
    if(&other != this){

        //stay connection handler as is
        
        subscription_id = other.subscription_id;
        receipt_id = other.receipt_id;
        logout_receipt = other.logout_receipt;
        connected = other.connected;
        username = other.username;
        channel_subscription_ids = other.channel_subscription_ids;
        for(auto &key_value_outer: user_reports)//user>>channel>>game
            for(auto &key_value_inner: key_value_outer.second)
                delete key_value_inner.second; //delete *Game
            
        user_reports=other.user_reports;
        for(auto &key_value_outer: other.user_reports)//user>>channel>>game
            for(auto &key_value_inner: key_value_outer.second)
                *key_value_inner.second = (*key_value_inner.second->clone()); //delete *Game
            
    }
    return *this;
}

StompProtocol::StompProtocol(StompProtocol&& other) :  connectionHandler(nullptr), cv(), mtx(), thread_wait(false), subscription_id(other.subscription_id), receipt_id(other.receipt_id), logout_receipt(other.logout_receipt), connected(other.connected), username(other.username), channel_subscription_ids(other.channel_subscription_ids), user_reports(other.user_reports)
{
    other.connectionHandler = nullptr;
    for(auto &key_value_outer: other.user_reports)//user>>channel>>game
            for(auto &key_value_inner: key_value_outer.second)
                delete key_value_inner.second; //delete other.*Game
}

StompProtocol& StompProtocol::operator=(StompProtocol &&other){
    if(&other != this){
        if(connectionHandler) delete connectionHandler;
        for(auto &key_value_outer: user_reports)//user>>channel>>game
            for(auto &key_value_inner: key_value_outer.second)
                if(key_value_inner.second) 
                    delete key_value_inner.second; //delete *Game

        other.connectionHandler = nullptr;

        subscription_id = other.subscription_id;
        receipt_id = other.receipt_id;
        logout_receipt = other.logout_receipt;
        connected = other.connected;
        username = other.username;
        channel_subscription_ids = other.channel_subscription_ids;
        user_reports = other.user_reports;
        for(auto &key_value_outer: other.user_reports)//user>>channel>>game
            for(auto &key_value_inner: key_value_outer.second)
                key_value_inner.second = nullptr; 
    }
    return *this;
}

void StompProtocol::server_input(){
    
    while (1) {

        string answer;
        if (connectionHandler == nullptr) {
            if(logout_receipt == -1)//not waiting for disconnection approval from server
                disconnect_client();
            thread_wait = true;
            while(true){
                unique_lock<mutex> lock(mtx);
                cv.wait(lock, [this]{return !thread_wait;});
                if(!thread_wait)
                    break;
            }
            continue;

        }

        if(!connectionHandler->getFrameAscii(answer, '\0')){
            disconnect_client("Connection Handler disconnected...");
            continue;
        }

        Frame frame(answer);
        string command = frame.get_command();
        map<string,string> headers = frame.get_headers();
        string body = frame.get_body();

        if(command =="CONNECTED"){
            std::cout<<command<<": Login succesful"<<endl;
            logout_receipt = -1;
            connected = true;
        }

        else if(command == "RECEIPT"){
            if(headers.find("receipt - id") != headers.end()){
                string receipt_id = headers.at("receipt - id");

                if(receipt_id.compare(to_string(logout_receipt)) == 0)
                    disconnect_client("the server logged you out successfully");
            }
        }

        else if(command.compare("ERROR") == 0){
            string output = "Error: ";
            if(headers.find("message") != headers.end()){
                string message = headers.at("message");
                output += message; 
            }
            output += body;
            cout<<output<<endl;
            disconnect_client();
        }

        else if(command.compare("MESSAGE") == 0){
            if(headers.find("subscription") != headers.end() 
            && headers.find("message - id") != headers.end() 
            && headers.find("destination") != headers.end()){
                string channel = headers.at("destination");
                string user;
                
                size_t user_start = body.find("user: ");
                if(user_start!=string::npos){
                    string body_from_user = body.substr(user_start);
                    getline(stringstream(body_from_user),user);
                    size_t start = body_from_user.find(':')+2;
                    size_t end = body_from_user.find('\n');
                    user = body_from_user.substr(start, end-start);
                    if(user_reports.find(user) == user_reports.end()){ //user doesnt exists
                        Game* game = new Game(user, channel);
                        map<string, Game*> channel_to_game({{channel, game}});
                        user_reports.insert(make_pair(user, channel_to_game));
                    }
                    else if(user_reports.at(user).find(channel) == user_reports.at(user).end()){ //user exists
                        //channel doesnt exists
                        Game* game = new Game(user, channel);
                        user_reports.at(user).insert(make_pair(channel, game));
                    }
                    user_reports.at(user).at(channel)->report(body);
                }
            }
        
        }
    }
    
}


void StompProtocol::terminal_input(vector<string> &words){
	
        //commands from terminal
        bool success = true;
        string command = words[0];
        if(connectionHandler != nullptr &&connectionHandler->isConnected() && !connected)
            cout<<"The user is not connected yet. Please wait for the server.";

        else if(logout_receipt != -1)
            cout<<"Please wait for the server to disconnect you.";
        
        else if(command.compare("login") == 0){

			//if already connected send message!!!!!!!!!!!!!!
            if(connected)
                cout << "The client is already logged in, log out before trying again" << endl;
			else if(words.size() < 4)
                cout << "Usage: login {host:port} {username} {password}" << endl;
            else{
                string host_port = words[1];
                size_t colon_index = host_port.find(":");
                string host = host_port.substr(0, colon_index);
                short port = stoi(host_port.substr(colon_index+1));
                
                if(connectionHandler) delete(connectionHandler);
                connectionHandler = new ConnectionHandler(host, port);
                if (!connectionHandler->connect()) {
                    std::cerr << "Cannot connect to " << host << ":" << port << std::endl;
 
                }
                {
                    unique_lock<mutex> lock(mtx);
                    thread_wait = false; 
                }
                cv.notify_all();

                username = words[2];
                string passcode = words[3];


                map<string, string> frame_headers({
                    {"accept - version",STOMP_VERSION},
                    {"host",host},
                    {"login",username},
                    {"passcode",passcode}});

                Frame connect_frame = Frame("CONNECT", frame_headers, "");

                success = connectionHandler->sendFrameAscii(connect_frame.to_string(), '\0');
                cout<<"Trying to connect as "<<username<<endl;
            }
        }

        else if(connectionHandler == nullptr || !connectionHandler->isConnected())
            cout<<"You are not connected to the server.\nPlease login using the command login {host:port} {username} {password}"<<endl;

        else if(command.compare("join") == 0){
            if(words.size() < 2)
                cout << "Usage: " << "join {game_name}" << endl;
            else if(words[1].find('_') == string::npos)
                cout << "Channel name format: <team_a_name>_<Team_b_name>"<<endl;
            else{
                channel_subscription_ids.insert(make_pair(words[1], to_string(subscription_id)));

                map<string, string> headers({
                    {"destination",words[1]}, 
                    {"id", to_string(subscription_id++)}, 
                    {"receipt", to_string(receipt_id++)}
                    });

                Frame frame("SUBSCRIBE", headers, "");
                success = connectionHandler->sendFrameAscii(frame.to_string(), '\0');
                cout<<"Joined channel "<<words[1]<<endl;
            } 
        }

        else if(command.compare("exit") == 0){
            if(words.size() < 2)
                cout << "Usage: " << "exit {game_name}" << endl;
            else if(channel_subscription_ids.find(words[1]) == channel_subscription_ids.end())
                cout << "You're not subscribed to this channel" << endl;
            else{

                map<string, string> headers({
                    {"id", channel_subscription_ids.at(words[1])}, 
                    {"receipt", to_string(receipt_id++)}
                    });
                Frame frame("UNSUBSCRIBE", headers, "");
                channel_subscription_ids.erase(words[1]);

                success = connectionHandler->sendFrameAscii(frame.to_string(), '\0');
                cout<<"Exited channel "<<words[1]<<endl;
                
            }
            
        }

        else if(command.compare("logout") == 0){
            logout_receipt = receipt_id;
            map<string, string> headers({
                    {"receipt", to_string(receipt_id++)}
                    });
            Frame frame("DISCONNECT", headers, "");
            success = connectionHandler->sendFrameAscii(frame.to_string(), '\0');
        }

        else if(command.compare("summary") == 0){
            if(words.size() < 4)
                cout << "Usage: " << "summary {game_name} {user} {file}" << endl;
            else if(user_reports.find(words[2]) == user_reports.end())
                cout << "This user didn't reported" << endl;
            else if(user_reports.at(words[2]).find(words[1]) == user_reports.at(words[2]).end())
                cout << "This channel doesn't exist" << endl;
            else
                user_reports.at(words[2]).at(words[1])->summary(words[3]);
        }

        else if(command.compare("report") == 0){
            ifstream file(words[1]);
            if(words.size() < 2)
                cout << "Usage: " << "report {file}" << endl;
            else if(words[1].substr(words[1].length() - 5).compare(".json") != 0)
                cout << "File must be in .json type" << endl;
            else if(!file.good())
                cout << "File doesn't exists" << endl;
            else{
                names_and_events names_events(parseEventsFile(words[1]));
                string channel = names_events.team_a_name+"_"+names_events.team_b_name;
                if(channel_subscription_ids.find(channel) == channel_subscription_ids.end())
                    cout<<"You're not subscribed to channel "<< channel <<endl;
                else{    
                    for(Event event: names_events.events){
                        string body = "user: "+username;
                        if(event.get_team_a_name().length() > 0)
                            body += "\nteam a: "+event.get_team_a_name();
                        if(event.get_team_b_name().length() > 0)
                            body += "\nteam b: "+event.get_team_b_name();
                        if(event.get_name().length() > 0)
                            body += "\nevent name: "+event.get_name();
                        if(to_string(event.get_time()).length() > 0)
                            body += "\ntime: "+to_string(event.get_time());
                        if(event.get_game_updates().size() > 0){
                            body += "\ngeneral game updates: ";
                            for(auto &key_value: event.get_game_updates())
                                body += "\n    "+key_value.first+": "+key_value.second;
                        }
                        if(event.get_team_a_updates().size() > 0){
                            body += "\nteam a updates: ";
                            for(auto &key_value: event.get_team_a_updates())
                                body += "\n    "+key_value.first+": "+key_value.second;
                        }
                        if(event.get_team_b_updates().size() > 0){
                            body += "\nteam b updates: ";
                            for(auto &key_value: event.get_team_b_updates())
                                body += "\n    "+key_value.first+": "+key_value.second;
                        }
                        if(event.get_discription().length() > 0)
                            body += "\ndescription: "+event.get_discription();

                    
                        map<string, string> headers({
                            {"destination", names_events.team_a_name+"_"+names_events.team_b_name}
                            });
                        Frame frame("SEND", headers, body);
                        success = connectionHandler->sendFrameAscii(frame.to_string(), '\0');
                        
                    }
                    cout<<"You reported successfully to channel " << channel << endl;
                }
            }
            
        } else { cout<<"Unknown Command"<<endl;}
        if(!success)
            disconnect_client("Connection Handler disconnected (couldn't send message to server)...");

}

void StompProtocol::disconnect_client(){
    disconnect_client("");
}

void StompProtocol::disconnect_client(string msg){
    username = "";
    connected = false;
    logout_receipt=-1;
    if(connectionHandler){
        connectionHandler->close();
        delete(connectionHandler);
    }

    connectionHandler = nullptr;
    if(msg.length() > 0)
        cout<<"Client disconnected: "<< msg <<endl;
}
