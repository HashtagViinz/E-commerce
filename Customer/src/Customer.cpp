#include "Customer.h"
#include <string.h>

//VERSION 14/09

/* ------------------------------- Costruttore ------------------------------ */
Customer::Customer(int seed, int ID){
    this->customer_State = CUSTOMER_GENERATION;
    this->pid = getpid();
    this->myseed = seed;
    this->ID = ID;
    srand((seed^ID));
    printf("----------------------: USER: %d | seed: %d\n", ID, seed);

    for(int i=0; i<20; i++){ 
        if(getStrState()=="SATISFACTION_PHASE"){
            break;
        }
        getStrState();
        elaboration();
    }
}

/* ------------------------------- Management ------------------------------- */
void Customer::elaboration(){
    switch(customer_State){
    case CUSTOMER_GENERATION:
        nextState();
        break;
    case SERVER_CONNECTION:
        connectToServer();
        nextState();
        break;
    case ORDER_PHASE:
        printf("\n[REQUESTED] : CATALOGO\n");
        requestArticles();
        printf("[RECEIVED] : CATALOGO\n");
        nextState();
        break;
    case MAKE_DECISION_PHASE:
        printf("Sto prendendo una decisione\n");
        makeDecision();         // gestione probabilità di morte
        break;
    case CHOICE_PHASE:
        printf("Sto scegliendo cosa ordinare\n");
        choose_item();
        break;
    case WAITING_PHASE:
        printf("# %s\n\n",getStrState().c_str());
        //redisFree(c2r);           // Libera la connessione a Redis
        usleep(500000);             //Accetta microSecondi 1s=1.000.000 micros | 0.5s=500.000micros
        changeState(SATISFACTION_PHASE);
        break;
    case SATISFACTION_PHASE:
        break;
    default:
        break;
    }
}

std::string Customer::getStrState() {
    switch (customer_State) {
        case CUSTOMER_GENERATION:
            return "CUSTOMER_GENERATION";
        case SERVER_CONNECTION:
            return "SERVER_CONNECTION";
        case ORDER_PHASE:
            return "ORDER_PHASE";
        case MAKE_DECISION_PHASE:
            return "MAKE_DECISION_PHASE";
        case CHOICE_PHASE:
            return "CHOICE_PHASE";
        case WAITING_PHASE:
            return "WAITING_PHASE";
        case SATISFACTION_PHASE:
            return "SATISFACTION_PHASE";
        default:
            return "Unknown state";
    }
}

// Funzione che ci permette di andare allo stato successivo
void Customer::nextState() {
    customer_State = static_cast<Customer_State>((static_cast<int>(customer_State)+1)%SELLER_STATE_LENGHT);
}

// Funzione che ci permette di andare allo stato richiesto
void Customer::changeState(Customer_State swNewState){
    this->customer_State = swNewState;
}

void Customer::connectToServer(){
    printf("### CONNECTION_PHASE Customer : %d\n",ID);

    // ? Connection
    printf("User %d: connecting to redis ...\n", ID);
    this->c2r = redisConnect("localhost", 6379);
    printf("User %d: connected to redis\n\n", ID);

    // ? Pulizia canale di comunicazione
    reply = RedisCommand(c2r, "DEL %s", WRITE_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    reply = RedisCommand(c2r, "DEL %s", CTRL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    reply = RedisCommand(c2r, "DEL %s", OBJ_CH);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    /* Create streams/groups */
    initStreams(c2r, WRITE_STREAM);
    initStreams(c2r, CTRL);
    initStreams(c2r, OBJ_CH);
}

void Customer::requestArticles(){
    //$ info mex
    char product[100];      // Prodotto
    char price[4];          // prezzo
    char seller[100];       // valori azienda

    sendReqObj();           // ! Richiediamo (tramite CTRL) al Server di Mandarci il Catalogo
    //! Customer in ascolto del Server
    for(int i=0; i<ITEMS_SHAREABLE; i++){

        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         timedBlock, WRITE_STREAM);
        assertReply(c2r, reply);    // Verifica errori nella comunicazione

        ReadStreamMsgVal(reply, 0, 0, 1, product);
        ReadStreamMsgVal(reply, 0, 0, 3, price);
        ReadStreamMsgVal(reply, 0, 0, 5, seller);

        //printf("Ricevuto (%d) - (%s, %s, %s)\n",i,product, price, seller);

        freeReplyObject(reply);
        addItem(Article(product, price, seller));   //Aggiungiamo gli articoli al catalogo dell'utente

        // Pulisco i valori dei buffer
        memset(product, 0, sizeof(product));
        memset(price, 0, sizeof(price));
        memset(seller, 0, sizeof(seller));
    }
    printf("# Numero prodotti Catalogo Acquisiti (%ld)\n", getItemCount());
}


// Funzione che sceglie un articolo dal catalogo e lo spedisce al server
void Customer::choose_item(){
    if (orderCounter == MAX_ORDER){             //Ne ho mandati già 4 allora dico al server che non devo mandarne più
        sendNoObj();
        changeState(WAITING_PHASE);

    }else if(orderCounter < MAX_ORDER){         // Se non ne ho mandati ancora 4 allora lo genero e lo spedisco
        Article art = catalog[rand() % 100 +1];   
        
        sendObj(); 
        reply = RedisCommand(c2r, "XADD %s * product %s price %s seller %s user %s", OBJ_CH, art.getName().c_str(), art.getPrice().c_str(), art.getSeller().c_str(), std::to_string(ID).c_str());  //? so - Sending Obj
        assertReply(c2r, reply);
        printf("#(%d) Ordine Spedito: (%s,%s,%s) da %d\n", orderCounter, art.getName().c_str(), art.getPrice().c_str(), art.getSeller().c_str(), ID);
        freeReplyObject(reply);         

        orderCounter++;
        changeState(MAKE_DECISION_PHASE);   // Vediamo se dobbiamo fare un'altro ordine
    }
}

/* ------------------------------- Utilitarie ------------------------------- */

//TODO - Elimina questa funzione
void Customer::printReply(redisReply *reply, int level) {
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

// Questa funzione ci indirizza randomicamente, con probabilità non nulla, in una delle due possibili scelte.
void Customer::makeDecision(){
    if(orderCounter < MAX_ORDER and (rand() %10 +1) > NOT_ORDER_PROB){   //controllo che non abbiamo mandato già 4 pacchetti e che la probabilità vada a favore della spedizione          
        changeState(CHOICE_PHASE);
    } else {
        sendNoObj();
        changeState(WAITING_PHASE);
    }
}

// Funzione che manda una comunicazione di controllo 'so'
void Customer::sendObj(){
    printf("    SEND on CTRL : OBJ\n");
    reply = RedisCommand(c2r, "XADD %s * requestCode %s", CTRL, "SendingObject");  //? so - Sending Obj
    assertReply(c2r, reply);
    freeReplyObject(reply);
}

// Funzione che manda una comunicazione di controllo 'no'
void Customer::sendNoObj(){
    printf("    SEND on CTRL : NO OBJ\n");
    reply = RedisCommand(c2r, "XADD %s * requestCode %s", CTRL, "StopSendingObject");  //? no - No Obj
    assertReply(c2r, reply);
    freeReplyObject(reply);
}

// Funzione che manda una comunicazione di controllo 'ro' Requesting Object - Richiesta Catalogo
void Customer::sendReqObj(){
    printf("    SEND on CTRL : RICHIESTA CATALOGO\n");
    reply = RedisCommand(c2r, "XADD %s * requestCode %s", CTRL, "RequestCatalog");  //? no - No Obj
    assertReply(c2r, reply);
    freeReplyObject(reply);
}

void Customer::addItem(Article article) {
    catalog.push_back(article);
}

size_t Customer::getItemCount() const {
    return catalog.size();
}