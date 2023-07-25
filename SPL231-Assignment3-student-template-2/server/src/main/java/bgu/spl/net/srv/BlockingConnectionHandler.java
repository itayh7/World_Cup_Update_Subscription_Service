package bgu.spl.net.srv;

import bgu.spl.net.api.MessageEncoderDecoder;
import bgu.spl.net.api.MessagingProtocol;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.concurrent.Semaphore;

public class BlockingConnectionHandler<T> implements Runnable, ConnectionHandler<T> {

    private final MessagingProtocol<T> protocol;
    private final MessageEncoderDecoder<T> encdec;
    private final Socket sock;
    private BufferedInputStream in;
    private BufferedOutputStream out;
    private volatile boolean connected = true;
    private Semaphore semaphore;

    public BlockingConnectionHandler(Socket sock, MessageEncoderDecoder<T> reader, MessagingProtocol<T> protocol) {
        this.sock = sock;
        this.encdec = reader;
        this.protocol = protocol;
        
        semaphore = new Semaphore(1, true);
        ConnectionsImpl<T> connections = (ConnectionsImpl<T>) ConnectionsImpl.getInstance();
        int connectedID = connections.pushCH(this);
        protocol.start(connectedID, connections);
    }

    @Override
    public void run() {
        try (Socket sock = this.sock) { //just for automatic closing
            int read;

            in = new BufferedInputStream(sock.getInputStream());

            while (!protocol.shouldTerminate() && connected && (read = in.read()) >= 0) {
                T nextMessage = encdec.decodeNextByte((byte) read);
                if (nextMessage != null){
                    System.out.println(nextMessage);
                    protocol.process(nextMessage); 

                }   
            }

        } catch (IOException ex) {
            ex.printStackTrace();
        }
        
        finally {try {close();
            } catch (IOException e) {}
        }
    }

    @Override
    public void close() throws IOException {
        connected = false;
        sock.close();
    }

    @Override
    public void send(T msg){
        try {
            semaphore.acquire();
            try {
                out = new BufferedOutputStream(sock.getOutputStream());
                out.write(encdec.encode(msg));
                out.flush();
            } catch (IOException e) {}
            semaphore.release();
        } catch (InterruptedException e) {}
        

    }
}
