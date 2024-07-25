
#include "Server.h"
#include <iostream>

 /* ------------------------------- Costruttore ------------------------------ */
Server::Server() {
    this->pid = getpid();
    redisContext *c2r;
    redisReply *reply;
    
    char streamname[100];
    char msgid[100];
    char fval[100];

    // ? SETUP Connection 
    printf("Server(): pid %d: user %s: connecting to redis ...\n", getPid(), "Server ");
    c2r = redisConnect("localhost", 6379);
    printf("main(): pid %d: user %s: connected to redis\n", getPid(), "Server");
    
    /* Create streams/groups */
    initStreams(c2r, READ_STREAM);

    while(read){
        printf("SERVER_ON_LISTEN() pid : %d stream: %s \n", pid, READ_STREAM);
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, READ_STREAM);
        
        assertReply(c2r, reply);                                                // Verifica errori nella comunicazione

        for (int k=0; k < ReadNumStreams(reply); k++){
            ReadStreamName(reply, streamname, k);                               // Leggiamo quale stram stiamo vedendo
            printf("-->ReadNumStreams: %d\n",ReadStreamName(reply, streamname, k));
            for (int i=0; i < ReadStreamNumMsg(reply, k); i++){
                ReadStreamNumMsgID(reply, k, i, msgid);                         
                printf("Message(?): %d\n",ReadStreamMsgNumVal(reply, k, i));    // Num of Mex in the block of message
                
                for (int h = 0; h < ReadStreamMsgNumVal(reply, k, i); h++){
                ReadStreamMsgVal(reply, k, i, h, fval);
                printf("(#?#) : Value %s\n", fval);
                }	 
            }
        }

        //dumpReply(reply, 0);
        freeReplyObject(reply);
    }

    redisFree(c2r);
}

/* ------------------------------ State Manager ----------------------------- */

void Server::running(){
    switch (swState) {
        case ON_LISTEN:
            connection();
            break;
        case ON_SELLER:
            break;
        case ON_CUSTOMER:
            break;
        case ON_DELIVERY:
            break;
        default:
            break;
    }
}

// Server State Setter
void Server::changeState(Server_State swNewState){
    this->swState = swNewState;
}

// State State String Getter
std::string Server::getStrState() {
    switch (swState) {
        case ON_LISTEN:
            return "running : ON_LISTEN";
        case ON_SELLER:
            return "ON_SELLER";
        case ON_CUSTOMER:
            return "ON_CUSTOMER";
        case ON_DELIVERY:
            return "ON_DELIVERY";
        default:
            return "Unknown state";
    }
}

/* ------------------------------- Management ------------------------------- */

void Server::connection(){
    
}





int Server::getPid() {
    return pid;
}

void Server::addItem(Item item) {
    available_Items.push_back(item);
}

void Server::getAvailable_Items() {
    for (Item i : available_Items){
        i.getItem();
    }
}


