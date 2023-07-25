# World_Cup_Update_Subscription_Service
This project is an implementation of a World Cup update subscription service, where users can subscribe to game channels and receive updates about the games from other subscribed users. The project includes both a server (Java) and a client (C++) component, using the STOMP (Simple-Text-Oriented-Messaging-Protocol) for communication.
This is implemented as a part of an assignment given to me at Ben Gurion University, as part of my Computer Science degree.

## Installation Requierments;
Ubuntu Linux:

```bash
sudo apt install maven
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
