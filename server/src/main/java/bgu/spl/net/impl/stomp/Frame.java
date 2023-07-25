package bgu.spl.net.impl.stomp;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.StringReader;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class Frame{
    
    private String command;
    private Map<String, String> headers;
    private String body;

    Frame(String command, Map<String, String> headers, String body){
        this.command = command;
        this.headers = headers;
        this.body = body;
    }

    Frame(String s){
        this("",new HashMap<>(), "");
        BufferedReader reader = new BufferedReader(new StringReader(s));
        
        try {
            command = reader.readLine();
            String line = null;

            while(!(line = reader.readLine()).equals("")){ //parse headers
                int colonIndex = line.indexOf(':');
                String key = line.substring(0, colonIndex);
                String value = line.substring(colonIndex + 1);
                headers.put(key, value);
            }
            
            while((line = reader.readLine()) != null)
                body += line+"\n";
        
        } catch (IOException e) {}
    }

    public String getCommand(){
        return command;
    }

    public Map<String,String> getHeaders(){
        return headers;
    }
    
    public String getBody(){
        return body;
    }
    
    public String toString() { 
        String output = command+"\n";
        if(headers != null){
            Iterator<Map.Entry<String, String>> iterator = headers.entrySet().iterator();
            while (iterator.hasNext()) {
                Map.Entry<String, String> entry = iterator.next();
                String key = entry.getKey();
                String value = entry.getValue();
                output += key+":"+value+"\n";
            }
        }
        output += "\n"+body;
        return output;
     }
}
