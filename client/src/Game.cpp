#include "../include/Game.h"

Game::Game(string user, string game_name) : user(user), game_name(game_name), general_stats({}), team_a_stats({}), team_b_stats({}), game_event_reports({}){  }


void Game::summary(string file_path){
    ifstream file(file_path);
    ofstream outfile(file_path);
    string output = "";
    size_t sep = game_name.find("_");
    string team_a = game_name.substr(0, sep);
    string team_b = game_name.substr(sep+1);
    output += team_a + " vs " + team_b + "\n";
    output += "Game stats:\n";
    output += "General stats:\n";
    for(auto &key_value: general_stats)
        output += key_value.first+":" +key_value.second+"\n";
    output += team_a+" stats:\n";
    for(auto &key_value: team_a_stats)
        output += key_value.first+":" +key_value.second+"\n";
    output += team_b+" stats:\n";
    for(auto &key_value: team_b_stats)
        output += key_value.first+":" +key_value.second+"\n";
    output += "Game event reports:\n";
    for(auto &key_value: game_event_reports)
        output += key_value.first+":\n\n" +key_value.second+"\n\n\n";

    outfile << output;
    cout<<user<<"'s reports to channel "<<game_name<<" summarized to "<<file_path<<endl;
}

void Game::report(string body){
    vector<string> lines;
    size_t start_line = 0;
    size_t end_line;
    while(end_line != string::npos){
        end_line = body.find("\n", start_line);
        lines.push_back(body.substr(start_line,end_line-start_line));
        start_line = end_line+1;
    }
    string event_name, time, description;
    for(unsigned i =0; i<lines.size(); i++){
        size_t colon_index = lines[i].find(":");
        string key = lines[i].substr(0, colon_index);
        string value = lines[i].substr(colon_index+1);
        if(key.compare("event name")==0)
            event_name=value.substr(1);
        else if(key.compare("time")==0)
            time=value.substr(1);
        else if(key.compare("description")==0)
            description=value.substr(1);
        else if(key.compare("general game updates")==0){
            for(i++;i<lines.size() && lines[i].substr(0,4).compare("    ")==0;i++){
                size_t colon_index = lines[i].find(':');
                key = lines[i].substr(4, colon_index-4);
                value = lines[i].substr(colon_index+1);
                general_stats.erase(key);
                general_stats.insert(make_pair(key,value));
            }
            i--;
        }
        else if(key.compare("team a updates")==0){
            for(i++;i<lines.size() && lines[i].substr(0,4).compare("    ")==0;i++){
                size_t colon_index = lines[i].find(':');
                key = lines[i].substr(4, colon_index-4);
                value = lines[i].substr(colon_index+1);
                team_a_stats.erase(key);
                team_a_stats.insert(make_pair(key,value));
            }
            i--;
        }
        else if(key.compare("team b updates")==0){
            for(i++;i<lines.size() && lines[i].substr(0,4).compare("    ")==0;i++){
                size_t colon_index = lines[i].find(':');
                key = lines[i].substr(4, colon_index-4);
                value = lines[i].substr(colon_index+1);
                team_b_stats.erase(key);
                team_b_stats.insert(make_pair(key,value));
            }
            i--;
        }
    }

    game_event_reports.insert(make_pair(time+" - "+event_name, description));

}

Game* Game::clone(){
    return new Game(*this);
}