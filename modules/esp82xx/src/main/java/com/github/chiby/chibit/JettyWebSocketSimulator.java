package com.github.chiby.chibit;

import java.io.IOException;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import javax.websocket.*;
import javax.websocket.server.ServerEndpoint;

@ServerEndpoint(value="/ws")
public class JettyWebSocketSimulator {
    private Session session;
    
    private static Set<JettyWebSocketSimulator> activeEndpoints 
          = new CopyOnWriteArraySet<>();
    
	@OnOpen
	public void onOpen(Session session) {
		System.out.println("WebSocket opened: " + session.getId());
		this.session = session;
		activeEndpoints.add(this);
		
	}
	@OnMessage
	public void onMessage(Session session, String message)  throws IOException {
		broadcast(message);
	}
	
	@OnError
    public void onError(Session session, Throwable t) {
        System.out.println("Websocket Error !");
        t.printStackTrace();
    }

	@OnClose
	public void onClose(CloseReason reason, Session session) {
		System.out.println("Closing a WebSocket due to " + reason.getReasonPhrase());
        activeEndpoints.remove(this);
	}
	
	private static void broadcast(String message) 
      throws IOException{
  
        activeEndpoints.forEach(endpoint -> {
            synchronized (endpoint) {
                try {
                    endpoint.session.getBasicRemote().
                      sendText(message);
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        });
    }
}
