
#include "Server.h"
#include <iostream>

 /* ------------------------------- Costruttore ------------------------------ */
Server::Server() {
    this->pid = getpid();
    this->swState = ON_CONNECTION;
    
    printf("Hello I'm Alive :D\n\n");

    for(int i = 0; i<2; i++){
        this->running();
    }

    /*

    */
    redisFree(c2r);
}

/* ------------------------------ State Manager ----------------------------- */

// Funzione che gestisce tutta la run del server
void Server::running(){
    switch (swState) {
        case ON_CONNECTION:
            connection();
            changeState(ON_LISTEN);     // $ Siamo pronti per ascoltare
            break;
        case ON_LISTEN:
            listen();
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
        case ON_CONNECTION:
            return "ON CONNECTION";
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

// Funzione che gestisce la connessione a redis usando l'attributo c2r.
void Server::connection(){
     // ? SETUP Connection 
    printf("Server ON : Connection(): connecting to redis ...| pid: %d - User: %s\n", getPid(), "Server ");
    this->c2r = redisConnect("localhost", 6379);
    printf("Server ON : Connection(): connected to redis | pid: %d - User: %s\n", getPid(), "Server");
    
    /* Create streams/groups */
    initStreams(this->c2r, READ_STREAM);
}

// Funzione che gestisce la fase di listen. La comunicazione può avvenire da tutti i canali di comunicazione
void Server::listen(){
    char streamname[100];   // Nome del canale di comunicazione
    char msgid[100];        // id del messaggio
    char fval[100];         // valori dentro il messaggio

   
    
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


