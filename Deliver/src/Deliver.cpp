#include "Deliver.h"

#include<iostream>
#include <string.h>

/* ------------------------------- Costruttore ------------------------------ */
Deliver::Deliver() {
    this->delivery_state = DELIVER_GENERATION;
    this->pid = getpid();
    this->myseed = (unsigned)time(NULL); // Uso un Seed per provare tante possibili strade per il mio Sellers. Ogni seed crea strade diverse.
    srand(myseed);

    for(int i = 0; i<4; i++){
        this->processing();
    }
}

/* -------------------------------- State Manager -------------------------------- */
void Deliver::processing(){
    switch (delivery_state)
    {
    case DELIVER_GENERATION:
        printf("*---------------------------------------------------------------------*\n");
        nextState();
        break;
    case ON_CONNECTION:
        connection();
        nextState();
        break;
    case ON_LISTEN:
        printf("%s\n",getStrState().c_str());
        onListen();
        break;
    default:
        break;
    }
}

std::string Deliver::getStrState() {
    switch (delivery_state) {
        case DELIVER_GENERATION:
            return "DELIVER_GENERATION";
        case ON_CONNECTION:
            return "ON_CONNECTION";
        case ON_LISTEN:
            return "ON_LISTEN";
        default:
            return "Unknown state";
    }
}

void Deliver::nextState() {
    delivery_state = static_cast<Delivery_state>((static_cast<int>(delivery_state)+1)%SELLER_STATE_LENGHT);
}

/* -------------------------- Management ------------------------- */

void Deliver::connection(){
    printf("### CONNECTION FASE Deliver");

    // ? Connection
    printf("User Deliver: connecting to redis ...\n");
    this->c2r = redisConnect("localhost", 6379);
    printf("User Deliver: connected to redis\n\n");

    // ? Pulizia canale di comunicazione
    reply = RedisCommand(c2r, "DEL %s", READ_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    /* Create streams/groups */
    initStreams(c2r, READ_STREAM);
}

void Deliver::onListen(){
    //$ info mex
    char product[100];      // Prodotto
    char price[4];          // prezzo
    char seller[100];       // valori azienda
    char user[100];         // Utente che ha effettuato l'ordine
    bool onListen = true;

    while(onListen){
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, READ_STREAM);
        assertReply(c2r, reply);    // Verifica errori nella comunicazione

        ReadStreamMsgVal(reply, 0, 0, 1, product);
        ReadStreamMsgVal(reply, 0, 0, 3, price);
        ReadStreamMsgVal(reply, 0, 0, 5, seller);
        ReadStreamMsgVal(reply, 0, 0, 7, user);

        printf("(%s,%s,%s,%s)\n", product, price, seller, user);

        freeReplyObject(reply);

        // Pulisco i valori dei buffer
        memset(product, 0, sizeof(product));
        memset(price, 0, sizeof(price));
        memset(seller, 0, sizeof(seller));

    }


}   





