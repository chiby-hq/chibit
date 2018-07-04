package com.github.chiby.chibit;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.web.socket.config.annotation.EnableWebSocket;

@SpringBootApplication
@EnableWebSocket
public class ChibitWebApplication {

	public static void main(String[] args) {
		SpringApplication.run(ChibitWebApplication.class, args);
	}
}
