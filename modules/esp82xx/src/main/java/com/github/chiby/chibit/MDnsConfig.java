package com.github.chiby.chibit;

import java.io.IOException;

import javax.annotation.PreDestroy;
import javax.jmdns.JmmDNS;
import javax.jmdns.ServiceInfo;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.boot.autoconfigure.condition.ConditionalOnProperty;
import org.springframework.stereotype.Component;

@Component
@ConditionalOnProperty(name="chibit.mdns.active", havingValue="true")
public class MDnsConfig {
    Log logger = LogFactory.getLog(MDnsConfig.class);
    JmmDNS mdns;
    
    public MDnsConfig() throws IOException {
        // Create a multi-homed JmDNS instance
        mdns = JmmDNS.Factory.getInstance();

        int port = 8080;
        // Register a service
        ServiceInfo serviceInfo = ServiceInfo.create("_chibitws._tcp.local.", "chibitws", port, "info=Websocket Chibit broker");
        mdns.registerService(serviceInfo);
        logger.info(String.format("Registered Chibit web service on port %d", port));

    }
    
    @PreDestroy
    public void preDestroy(){
        // Unregister all services
        mdns.unregisterAllServices();
    }
}
