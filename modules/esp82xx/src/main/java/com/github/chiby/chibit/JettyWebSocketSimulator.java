package com.github.chiby.chibit;

import java.io.IOException;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import javax.websocket.*;
import javax.websocket.server.ServerEndpoint;

@ServerEndpoint(value="/ws", subprotocols = {"arduino",""})
public class JettyWebSocketSimulator {
    private Session session;
    
    private static Set<JettyWebSocketSimulator> activeEndpoints 
          = new CopyOnWriteArraySet<>();
    
	@OnOpen
	public void onOpen(Session session) {
		System.out.println("WebSocket opened: " + session.getId());//+ " subprotocol '" +session.getNegotiatedSubprotocol()+"'");
		this.session = session;
		// We do not repropagate data to arduino clients
		if( ! session.getNegotiatedSubprotocol().equals("arduino")){
		  activeEndpoints.add(this);
	    }
		
	}
	@OnMessage
	public void onMessage(Session session, String message)  throws IOException {
		broadcast(message);
	}
	
	@OnError
    public void onError(Session session, Throwable t) {
        System.out.println("Websocket Error  on "+session.getId()+" !");
        t.printStackTrace();
    }

	@OnClose
	public void onClose(CloseReason reason, Session session) {
		System.out.println("Closing a WebSocket due to " + reason.getReasonPhrase());
        if(activeEndpoints.contains(this)) {
                activeEndpoints.remove(this);
        }
	}
	
	private static void broadcast(String message) 
      throws IOException{
  
        activeEndpoints.forEach(endpoint -> {
            synchronized (endpoint) {
                try {
                    endpoint.session.getBasicRemote().sendText(message);
                } catch (IOException e) {
                    System.out.println("Exception upon propagation to "+endpoint.session.getId());
                    e.printStackTrace();
                }
            }
        });
    }
}
