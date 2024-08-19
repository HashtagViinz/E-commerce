#include "Customer.h"
#include <string.h>


/* ------------------------------- Costruttore ------------------------------ */
Customer::Customer(int seed, int ID){
    this->customer_State = CUSTOMER_GENERATION;
    this->pid = getpid();
    this->myseed = seed;
    this->ID = ID;

    for(int i=0; i<3; i++){
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
        requestArticles();
        break;
    case WAITING_PHASE:
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

void Customer::connectToServer(){
    printf("### CONNECTION_PHASE Customer : %d\n",ID);

    // ? Connection
    printf("User %d: connecting to redis ...\n", ID);
    this->c2r = redisConnect("localhost", 6379);
    printf("User %d: connected to redis\n\n", ID);

    /* Create streams/groups */
    initStreams(c2r, WRITE_STREAM);
}

void Customer::requestArticles(){
    //$ info mex
    char product[100];      // Prodotto
    char price[4];          // prezzo
    char seller[100];       // valori azienda

    //! richiesta al server dei prodotti disponibili
    reply = RedisCommand(c2r, "XADD %s * type %s", WRITE_STREAM, "rp"); // rp = "RequestProducts"
    assertReply(c2r, reply);
    printf("CUSTOMER:(%d)---->SERVER : %s\n",ID,reply->str);
    freeReplyObject(reply);

    for(int i=0; i<ITEMS_SHAREABLE; i++){

        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                         block, WRITE_STREAM);
        assertReply(c2r, reply);    // Verifica errori nella comunicazione
        
        //! Server risponde
        printf("CUSTOMER:(%d)<----SERVER : %s\n",ID, reply->str);
        printf("CIAO");

        ReadStreamMsgVal(reply, 0, 0, 1, product);
        ReadStreamMsgVal(reply, 0, 0, 3, price);
        ReadStreamMsgVal(reply, 0, 0, 5, seller);
        
        //TODO Inserisci queste info in un ITEM e devi controllare che funziona la connessione
        addItem(Article(product, price, seller));
        printf("Numero elementi nel Catalogo: %ld",getItemCount());

        // Pulisco i valori dei buffer
        memset(product, 0, sizeof(product));
        memset(price, 0, sizeof(price));
        memset(seller, 0, sizeof(seller));
    }   

    // Scegliamo un articolo randomico
    // aspetta che l'ordine arrivi
    // Con probabilità 0.4 richiede un altro articolo
    // ...
    // ! Muore
}



void Customer::addItem(Article article) {
    catalog.push_back(article);
}

size_t Customer::getItemCount() const {
    return catalog.size();
}