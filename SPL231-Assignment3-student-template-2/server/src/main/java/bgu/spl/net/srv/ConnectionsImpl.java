package bgu.spl.net.srv;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentSkipListSet;
import java.util.concurrent.Semaphore;


public class ConnectionsImpl<T> implements Connections<T>{


    ConnectionHandler<T>[] connectionHandlers;
    Map<Integer, String>[] subIDs;
    
    public static final int INITIAL_CONNECTIONS_SIZE = 10;
    Map<String,Set<Integer>> channels;
    Map<String,String> users;
    Integer messageId;
    Map<Integer,String> usersConnected;
    Semaphore semaphore;

    //Singelton
    private static class ConnectionHolder{
        private static ConnectionsImpl<?> instance = new ConnectionsImpl<>();
    }

    public static ConnectionsImpl<?> getInstance(){
        return ConnectionHolder.instance;
    }


    private ConnectionsImpl(){
        //Initialize Connections
        connectionHandlers = (ConnectionHandler<T>[]) new ConnectionHandler<?>[INITIAL_CONNECTIONS_SIZE];
        subIDs = new HashMap[INITIAL_CONNECTIONS_SIZE];
        channels = new ConcurrentHashMap<>();
        users = new ConcurrentHashMap<>();
        messageId = 0;
        usersConnected = new ConcurrentHashMap<Integer,String>();
        semaphore = new Semaphore(1, true);
    }


    private int getEmptySlot() {
        for(int i = 0; i<connectionHandlers.length; i++)
            if(connectionHandlers[i] == null)
                return i;
        return connectionHandlers.length;
    }

    public int pushCH(ConnectionHandler<T> ch) {
        try {
            semaphore.acquire();
        } catch (InterruptedException e) {}

        int emptySlot = getEmptySlot();
        if (emptySlot >= connectionHandlers.length) {
            connectionHandlers = Arrays.copyOf(connectionHandlers, connectionHandlers.length * 2);
            subIDs = Arrays.copyOf(subIDs, subIDs.length * 2);
        }
        connectionHandlers[emptySlot] = ch;
        subIDs[emptySlot] = new HashMap<Integer, String>();
        semaphore.release();
        return emptySlot;
    }

    //Channel methods:
    public void subscribeChannel(String channel, int connectionId, int subscribeId){
        if(channels.get(channel) == null)
            channels.put(channel, new ConcurrentSkipListSet<>());    
        channels.get(channel).add(connectionId);
        subIDs[connectionId].put(subscribeId, channel);
    }

    public boolean unSubscribeChannel(int connectionId, int subscribtionId){
        String channel = subIDs[connectionId].get(subscribtionId);
        if(channel == null)
            return false;
        Set<Integer> connectionsChannels = channels.get(channel);
        if(connectionsChannels == null)
            return false;
        subIDs[connectionId].remove(subscribtionId);
        return connectionsChannels.remove(connectionId);
    }

    public boolean isSubscribed(String channel, int connectionId){
        return subIDs[connectionId].values().contains(channel);
    }

    public boolean subscribeIdExists(int connectionId, int subscribeId){
        return subIDs[connectionId].keySet().contains(subscribeId);
    }

    public void disconnectClientFromChannels(int connectionId){
        for(Set<Integer> connectionsSet: channels.values())
            connectionsSet.remove(connectionId);
        subIDs[connectionId] = null;
    }

    public Set<Integer> getChannelConnectionIDs(String channel){
        return channels.get(channel);
    }

    public Integer getSubscriberId(String channel, int connectionId){
        for(Integer subscribeId: subIDs[connectionId].keySet()){
            if(subIDs[connectionId].get(subscribeId).equals(channel))
                return subscribeId;
        }
        return null;
    }

    //User methods:
    public boolean userExists(String user){
        return users.keySet().contains(user);
    }

    public boolean checkPassword(String user, String pass){
        return users.get(user).equals(pass);
    }

    public void addUser(String user, String pass){
        if(!userExists(user))
            users.put(user,pass);
    }

    public boolean userConnected(String user){
        return usersConnected.values().contains(user);
    }

    public void connectUser(int connectionId, String user){
        usersConnected.put(connectionId, user);
    }

    public void disconnectUser(int connectionId){
        usersConnected.remove(connectionId);
    }
    
    public Integer getMessageId(){
        return messageId;
    }
    
    public void incrementMessageId(){
        messageId++;
    }


    @Override
    public boolean send(int connectionId, T msg) {
        connectionHandlers[connectionId].send(msg);
        return false;
    }

    @Override
    public void send(String channel, T msg) {
        for(Integer connectionId : getChannelConnectionIDs(channel)){
            //add subscription id header
            send(connectionId, msg);
        }
    }

    @Override
    public void disconnect(int connectionId) {
        disconnectClientFromChannels(connectionId);
        disconnectUser(connectionId);
        connectionHandlers[connectionId] = null;
    }
    
}
    

