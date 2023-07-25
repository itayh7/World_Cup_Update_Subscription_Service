package bgu.spl.net.impl.stomp;


import bgu.spl.net.srv.Server;

public class StompServer {

    public static void main(String[] args) {
        if(args[1].equals("tpc")){
            Server.threadPerClient(
                    7777, //port
                    () -> new StompProtocol(), //protocol factory
                FrameMessageEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }
        else if(args[1].equals("reactor")){
            Server.reactor(
                    Runtime.getRuntime().availableProcessors(),
                    7777, //port
                    () -> new StompProtocol(), //protocol factory
                    FrameMessageEncoderDecoder::new //message encoder decoder factory
            ).serve();
        }
        else throw new IllegalArgumentException("Expected argument: \"tpc\" or \"reactor\". Given argument: \"" + args[1] + "\"");
    
    }
}
