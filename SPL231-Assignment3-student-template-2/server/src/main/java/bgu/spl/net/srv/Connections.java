package bgu.spl.net.srv;

import java.util.Set;

public interface Connections<T> {

    boolean send(int connectionId, T msg);

    void send(String channel, T msg);

    void disconnect(int connectionId);

    void subscribeChannel(String channel, int connectionId, int subscribeId);

    boolean unSubscribeChannel(int connectionId, int subscribtionId);

    boolean isSubscribed(String channel, int connectionId);

    void disconnectClientFromChannels(int connectionId);

    Set<Integer> getChannelConnectionIDs(String channel);

    Integer getSubscriberId(String channel, int connectionId);

    Integer getMessageId();

    void incrementMessageId();

    boolean subscribeIdExists(int connectionId, int subscribeId);

    boolean userExists(String user);

    boolean checkPassword(String user, String pass);

    void addUser(String user, String pass);

    boolean userConnected(String user);

    void connectUser(int connectionId, String user);

    void disconnectUser(int connectionId);

}
