#include "../include/Frame.h"



Frame::Frame(string command, map<string, string> &headers, string body) : 
command(command), 
headers(headers), 
body(body){}

Frame::Frame(const string &s) : command(""), headers({}), body("")
{
    vector<string> lines;
    size_t start_line = 0;
    size_t end_line;
    while(end_line != string::npos){
        end_line = s.find("\n", start_line);
        lines.push_back(s.substr(start_line,end_line-start_line));
        start_line = end_line+1;
    }
    
    command = lines[0];
    unsigned i = 1;
    for(; i<lines.size() && lines[i].length() > 0; i++){ //headers
        size_t colon_index = lines[i].find(":");
        string key = lines[i].substr(0, colon_index);
        string value = lines[i].substr(colon_index+1);
        headers.insert(make_pair(key,value));
    }
    for(; i<lines.size()-1 ;i++)
        body += lines[i] + "\n";
    body += lines[i];

}


const string &Frame::get_command() const{
    return command;
}

const map<string, string> &Frame::get_headers() const{
    return headers;
}

const string &Frame::get_body() const{
    return body;
}

const string Frame::to_string() const{
    string output = command+"\n";
    
    for(auto iterator = headers.begin(); iterator != headers.end(); ++iterator){
        output += iterator->first+":"+iterator->second+"\n";
    }
    output += "\n"+body;
    return output;
}
