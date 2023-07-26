# World_Cup_Update_Subscription_Service
This project is an implementation of a World Cup update subscription service, where users can subscribe to game channels and receive updates about the games from other subscribed users. The project includes both a server (Java) and a client (C++) component, using the STOMP (Simple-Text-Oriented-Messaging-Protocol) for communication.
This is implemented as a part of an assignment given to me at Ben Gurion University, as part of my Computer Science degree.

## Installation Requierments;
Ubuntu Linux:

```bash
sudo apt install maven
sudo apt-get update
sudo apt-get install g++
sudo apt-get install libboost-all-dev
```

## Usage
Server:
1. Open in terminal from .../World_Cup_Update_Subscription_Service/SPL231-Assignment3-student-template-2/server directory.
2. Build using:
```bash
mvn compile
```
3. Run one of two server implementations:
> Thread per client server:
```bash
mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer"
-Dexec.args="<port> tpc"
```
> Reactor server:
```bash
mvn exec:java -Dexec.mainClass="bgu.spl.net.impl.stomp.StompServer"
-Dexec.args="<port> reactor"
```

Client:
1. Open in a new terminal from .../World_Cup_Update_Subscription_Service/SPL231-Assignment3-student-template-2/client directory.
2. Compile using:
```bash
make
```
3. Run using:
```bash
./bin/StompWCIClient
```

Client Commands:
```bash
login {host:port} {username} {password}     // connect to the server and log in as a new or existing user.
join {game_name}     // subscribe to channel to get and send reports. game_name format: <team1>_<team2>
report {file}     // report from a .json file that contains events (for example client/data/events1.json)
summary {game_name} {user} {file}     // print the game updates it got from {user} for {game_name} into the provided {file}
exit {game_name}     // unsubscribe from the channel
logout     // logout from user and unsubscribe from all channels, then close connection from server.
```
