/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   httpServer.h
 * Author: brehm
 *
 * Created on April 19, 2018, 11:03 AM
 */

#ifndef HTTPSERVER_H
#define HTTPSERVER_H
#include <stdint.h>

void startHttpServer(void);
int httpServerThread(void);
uint8_t stopHttpServer(void);
void sendReply(char *buf, int tcp_socket);
int Output_HTTP_Headers(int sock, char *responseMsg);
int handleRequest (int sock);


#endif /* HTTPSERVER_H */

