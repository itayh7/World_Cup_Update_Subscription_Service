package bgu.spl.net.impl.stomp;

import bgu.spl.net.api.StompMessagingProtocol;
import bgu.spl.net.srv.Connections;

//import java.time.LocalDateTime;
import java.util.HashMap;
import java.util.Map;

public class StompProtocol implements StompMessagingProtocol<String> {

    private boolean shouldTerminate = false;
    Connections<String> connections;
    int connectionId;
    static final String STOMP_VERSION = "1.2";

    @Override
    public String process(String msg) {
        Frame frame = new Frame(msg);
        String command = frame.getCommand();
        Map<String,String> headers = frame.getHeaders();
        String body = frame.getBody();

        if(command.equals("SEND")){
            
            String channel = headers.get("destination");
            if(channel == null){
                returnError(frame, "malformed frame received", "Did not contain a destination header, which is REQUIRED for message propagation.");
            }
            else if(!connections.isSubscribed(channel , connectionId))
                returnError(frame, "Not subscribed", "You need to subscribe the topic before you send messages to it.");
            else{
                for(Integer id : connections.getChannelConnectionIDs(channel)){
                    Map<String,String> messageHeaders = new HashMap<>();
                    messageHeaders.put("subscription", connections.getSubscriberId(channel, connectionId).toString());
                    messageHeaders.put("message - id", connections.getMessageId().toString());
                    messageHeaders.put("destination", channel);
                    Frame message = new Frame("MESSAGE", messageHeaders, body);
                    connections.send(id, message.toString());
                }
                connections.incrementMessageId();
            }
        }

        else if(command.equals("SUBSCRIBE")){
            String channel = headers.get("destination");
            String id = headers.get("id");
            if(channel == null){
                returnError(frame, "malformed frame received", "Did not contain a destination header, which is REQUIRED for subscribe operation.");
            }
            else if(id == null || connections.subscribeIdExists(connectionId, Integer.parseInt(id))){
                returnError(frame, "malformed frame received", "Did not contain a id header, or id already exists");
            }
            else{
                connections.subscribeChannel(channel, connectionId, Integer.parseInt(id));
                String receipt = headers.get("receipt");
                if(receipt != null)
                    returnReceipt(receipt);
            }
            

        }

        else if(command.equals("UNSUBSCRIBE")){
            String id = headers.get("id");
            if(id == null)
                returnError(frame, "malformed frame received", "Did not contain a id header, which is REQUIRED for unsubscribe operation.");
            else{
                connections.unSubscribeChannel(connectionId, Integer.parseInt(id));
                String receipt = headers.get("receipt");
                if(receipt != null)
                    returnReceipt(receipt);
            }
        }

        else if(command.equals("DISCONNECT")){
            String receipt = headers.get("receipt");
            if(receipt == null)
                returnError(frame, "malformed frame received", "Did not contain a receipt header, which is REQUIRED for disconnect operation.");
            else{
                returnReceipt(receipt);
                disconnect();
            }
        }

        else if(command.equals("CONNECT")){
            String acceptVersion = headers.get("accept - version");
            String login = headers.get("login");
            String passcode = headers.get("passcode");
            if(acceptVersion == null || !acceptVersion.equals(STOMP_VERSION))
                returnError(frame, "version error", "Version has to be 1.2");
            else if(login == null || passcode == null)
                returnError(frame, "malformed frame received", "Did not contain a login or passcode header, which is REQUIRED for connect operation.");
            else if(connections.userConnected(login))
                returnError(frame, "user already logged in", "This username is already connected from another client");
            else if(connections.userExists(login) && !connections.checkPassword(login, passcode))
                returnError(frame, "wrong password", "Password is incorrect");
            else{
                if(!connections.userExists(login))
                    connections.addUser(login, passcode);
                connections.connectUser(connectionId, login);
                Map<String,String> connectedHeaders = new HashMap<>();
                connectedHeaders.put("version", STOMP_VERSION);
                Frame connectedFrame = new Frame("CONNECTED", connectedHeaders, "");
                connections.send(connectionId, connectedFrame.toString());
            }
        }



        return "";
    }

    @Override
    public boolean shouldTerminate() {
        return shouldTerminate;
    }

    @Override
    public void start(int connectionId, Connections<String> connections) {
        this.connectionId = connectionId;
        this.connections = connections;
    }

    private void returnReceipt(String receipt){
        Map<String,String> receiptHeaders = new HashMap<>();
        receiptHeaders.put("receipt - id", receipt);
        Frame receiptFrame = new Frame("RECEIPT", receiptHeaders, "");
        connections.send(connectionId, receiptFrame.toString());
    }

    private void returnError(Frame frame, String errorMessage, String errorBody){
        Map<String,String> errorHeaders = new HashMap<>();
        if(frame.getHeaders().containsKey("receipt"))
            errorHeaders.put("receipt - id", frame.getHeaders().get("receipt"));
        if(errorMessage.length() > 0)
            errorHeaders.put("message", errorMessage);

        Frame errorFrame = new Frame("ERROR", errorHeaders, "In frame:\n-----\n"+frame.toString()+"\n-----\n"+errorBody);
        connections.send(connectionId, errorFrame.toString());
        disconnect();
    }

    private void disconnect(){
        shouldTerminate = true;
        connections.disconnect(connectionId);
    }

}
