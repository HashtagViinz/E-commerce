
#include "Server.h"
#include <iostream>

 /* ------------------------------- Costruttore ------------------------------ */
Server::Server() {
    this->pid = getpid();
    this->swState = ON_CONNECTION;
    
    printf("Hello I'm Alive :D\n\n");

    for(int i = 0; i<4; i++){
        this->running();
    }


    redisFree(c2r);
}

/* ------------------------------ State Manager ----------------------------- */

// Funzione che gestisce tutta la run del server
void Server::running(){
    switch (swState) {
        case ON_CONNECTION:
            connection();
            changeState(ON_SELLER);     // $ Siamo pronti per ascoltare
            break;
        case ON_SELLER:
            listenSellers();
            changeState(ON_CUSTOMER);    // $ Ascoltiamo i customer
            break;
        case ON_CUSTOMER:
            listenCustomer();
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
        case ON_SELLER:
            return "ON_SELLER";
        case ON_CUSTOMER:
            return "ON_CUSTOMER";
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

    // Pulizia Canale di comunicazione
    reply = RedisCommand(c2r, "DEL %s", READ_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);

    reply = RedisCommand(c2r, "DEL %s", CTRL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);

    /* Create streams/groups */
    initStreams(this->c2r, READ_STREAM);
    initStreams(this->c2r, CTRL);
    initStreams(this->c2r, OBJ_CH);
}

// Funzione che gestisce la fase di listen. La comunicazione può avvenire da tutti i canali di comunicazione
void Server::listenSellers(){
  
    //$ info mex
    char product[100];      // Prodotto
    char price[4];          // prezzo
    char seller[100];       // valori azienda
    int i = 0;

    printf("SERVER ON LISTEN FOR SELLER() pid : %d stream: %s \n", pid, READ_STREAM);
    while(i<SERVER_MAX_ITEMS){
        
        // Pulisco i valori dei buffer
        memset(product, 0, sizeof(product));
        memset(price, 0, sizeof(price));
        memset(seller, 0, sizeof(seller));

        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, READ_STREAM);
        assertReply(c2r, reply);    // Verifica errori nella comunicazione
        
        ReadStreamMsgVal(reply, 0, 0, 1, product);
        ReadStreamMsgVal(reply, 0, 0, 3, price);
        ReadStreamMsgVal(reply, 0, 0, 5, seller);

        addItem(Item(product, price, seller));
        //printf("(#!!#) Saved (%ld) || Item intercettato : (%s|%s|%s)\n", getItemCount(), product, price, seller);

        //dumpReply(reply, 0);
        freeReplyObject(reply);
        i++;
    }
    printf("Product acquisition completed!\n");
}

// Funzione che gestisce la comunicazione fra Customer e Server
void Server::listenCustomer(){
    bool wait = true;
    char msg[10];                       // Valore messaggio richiesta ("so","no")

    //? delete stream if it exists
    reply = RedisCommand(c2r, "DEL %s", CUST_R_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    
    /* Create streams/groups */
    initStreams(c2r, CUST_R_STREAM);

    //! Mandiamo il catalogo al Seller
    printf("Sending Catalog...\n");

    reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                            timedBlock, CTRL);
    assertReply(c2r, reply);                // Verifica errori nella comunicazione
    memset(msg, 0, sizeof(msg));            // Pulisco i valori dei buffer
    ReadStreamMsgVal(reply, 0, 0, 1, msg);  // Leggo il valore 
    freeReplyObject(reply);

    if(strcmp(msg, "ro") == 0){             // 'ro' - requesting object
        printf("Richiesta Accettata... Inizio a mandare il Catalogo...\n");
        for (Item i : available_Items){

            // Recupera i valori dai getter
            string name = i.getName();
            string price = i.getPrice();
            string seller = i.getSeller();

            reply = RedisCommand(c2r, "XADD %s * prod %s price %s seller %s", CUST_R_STREAM ,  name.c_str(), price.c_str(), seller.c_str());
            assertReply(c2r, reply);
            //printf("Server sta mandando : %s %s %s\n", name.c_str(), price.c_str(), seller.c_str());
            freeReplyObject(reply);
        }
    }
    printf("Catalog sended...\n");

    // ! ASPETTIAMO ORDINI

    //$ info mex
    char product[100];                  // Prodotto
    char price[4];                      // prezzo
    char seller[100];                   // valori azienda
    
    while(wait){
        
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                            timedBlock, CTRL);
        assertReply(c2r, reply);                // Verifica errori nella comunicazione
        memset(msg, 0, sizeof(msg));            // Pulisco i valori dei buffer
        ReadStreamMsgVal(reply, 0, 0, 1, msg);  // Leggo il valore 
        freeReplyObject(reply);

        printf("\nCentro di controllo: %s\n",msg);

        if(strcmp(msg, "so") == 0){             // E' Stato mandato un obj 
            printf("CTRL : OBJ arriving...\n");
            // Pulisco i valori dei buffer
            memset(product, 0, sizeof(product));
            memset(price, 0, sizeof(price));
            memset(seller, 0, sizeof(seller));

            reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                            timedBlock, OBJ_CH);
            assertReply(c2r, reply);  // Verifica errori nella comunicazione

            //printReply(reply);

            ReadStreamMsgVal(reply, 0, 0, 1, product);
            ReadStreamMsgVal(reply, 0, 0, 3, price);
            ReadStreamMsgVal(reply, 0, 0, 5, seller);
            freeReplyObject(reply);

            printf("#Ordine Ricevuto : (%s, %s, %s)\n", product, price, seller);

        } else if(strcmp(msg, "no") == 0) {
            wait=false;
            printf(" CTRL : NO OBJ Sended : (%s)\n",msg);
        }
    }
}

/* ------------------------------- Utilitarie ------------------------------- */

//TODO - Elimina questa funzione
void Server::printReply(redisReply *reply, int level) {
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

/* ------------------------------- Utilitarie ------------------------------- */


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


