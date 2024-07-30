
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
  
    //$ info mex
    char product[100];      // Prodotto
    char price[4];          // prezzo
    char seller[100];       // valori azienda
   
    printf("SERVER_ON_LISTEN() pid : %d stream: %s \n", pid, READ_STREAM);
    while(read){
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, READ_STREAM);
        assertReply(c2r, reply);    // Verifica errori nella comunicazione
        
        ReadStreamMsgVal(reply, 0, 0, 1, product);
        ReadStreamMsgVal(reply, 0, 0, 3, price);
        ReadStreamMsgVal(reply, 0, 0, 5, seller);

        addItem(Item(product, price, seller));
        printf("(#!!#) Saved (%ld) || Item intercettato : (%s|%s|%s)\n", getItemCount(), product, price, seller);

        //dumpReply(reply, 0);
        freeReplyObject(reply);
    }
}

//TODO - Elimina questa funzione
/* 
void Server::printReply(redisReply *reply, int level = 0) {
    if (reply == nullptr) {
        return;
    }
    
    // Indentation for readability
    std::string indent(level * 2, ' ');

    switch (reply->type) {
        case REDIS_REPLY_STRING:
            std::cout << indent << "STRING: " << reply->str << std::endl;
            break;
        case REDIS_REPLY_ARRAY:
            std::cout << indent << "ARRAY of " << reply->elements << " elements:" << std::endl;
            for (size_t i = 0; i < reply->elements; ++i) {
                printReply(reply->element[i], level + 1);
            }
            break;
        case REDIS_REPLY_INTEGER:
            std::cout << indent << "INTEGER: " << reply->integer << std::endl;
            break;
        case REDIS_REPLY_NIL:
            std::cout << indent << "NIL" << std::endl;
            break;
        case REDIS_REPLY_STATUS:
            std::cout << indent << "STATUS: " << reply->str << std::endl;
            break;
        case REDIS_REPLY_ERROR:
            std::cout << indent << "ERROR: " << reply->str << std::endl;
            break;
        default:
            std::cout << indent << "UNKNOWN TYPE: " << reply->type << std::endl;
            break;
    }
}
*/



int Server::getPid() {
    return pid;
}

void Server::addItem(Item item) {
    available_Items.push_back(item);
}

// Funzione che stampa gli items
void Server::getAvailable_Items() {
    for (Item i : available_Items){
        i.getItem();
    }
}

size_t Server::getItemCount() const {
    return available_Items.size();
}


