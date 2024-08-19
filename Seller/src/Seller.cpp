#include "Seller.h"

#include<iostream>
#include <string.h>

/* ------------------------------- Costruttore ------------------------------ */
Seller::Seller() {
    this->seller_State = SELLER_GENERATION;
    this->pid = getpid();
    this->myseed = (unsigned)time(NULL); // Uso un Seed per provare tante possibili strade per il mio Sellers. Ogni seed crea strade diverse.
    srand(myseed);

    for(int i = 0; i<2; i++){
        this->processing();
    }
}

Seller::Seller(string name) {
    this->seller_State = SELLER_GENERATION;
    this-> sellerName = name;
    this->pid = getpid();
    this->myseed = (unsigned)time(NULL); // Uso un Seed per provare tante possibili strade per il mio Sellers. Ogni seed crea strade diverse.
    srand(myseed);

    for(int i = 0; i<2; i++){
        this->processing();
    }
}

Seller::Seller(int seed) { // Costruttore con seed
    this->seller_State = SELLER_GENERATION;
    this->pid = getpid();
    this->myseed = seed;
}

Seller::Seller(int seed, string name){
    this->seller_State = SELLER_GENERATION;
    this-> sellerName = name;
    this->pid = getpid();
    this->myseed = seed;

    for(int i = 0; i<4; i++){
        this->processing();
    }
}

/* -------------------------------- State Manager -------------------------------- */
void Seller::processing(){
    switch (seller_State)
    {
    case SELLER_GENERATION:
        printf("\n\n*---------------------------------------------------------------------*\nSeller: %s\n",sellerName.c_str());
        nextState();                                                // Set next state
        break;
    case LOADING_PRODUCT:
        generateSellerProduct();
        printProductList();
        nextState();
        break;
    case CONNECTION_SERVER:
        connection();
        nextState();
        break;
    case UPLOADING_PRODUCT:
        send_products();
        break;
    default:
        break;
    }
}

std::string Seller::getStrState() {
    switch (seller_State) {
        case SELLER_GENERATION:
            return "SELLER_GENERATION";
        case LOADING_PRODUCT:
            return "LOADING_PRODUCT";
        case CONNECTION_SERVER:
            return "CONNECTION_SERVER";
        case UPLOADING_PRODUCT:
            return "UPLOADING_PRODUCT";
        default:
            return "Unknown state";
    }
}

void Seller::nextState() {
    seller_State = static_cast<Seller_State>((static_cast<int>(seller_State)+1)%SELLER_STATE_LENGHT);
}
/* ----------------------------- Setter & Getter ---------------------------- */
void Seller::setProductsList(int lenLst, int* pntList) {
    for(int i = 0; i<lenLst; i++) {
        sellerProducts[i] = *(pntList+i);
    }
}

/* -------------------------- Management ------------------------- */

string generateSellerName() {   // ! ESTERNA
    // pick randomly name from nameList
    size_t numElements = sizeof(allSellerNames) / sizeof(allSellerNames[0]);
    int randomIndex = rand() % numElements;

    return allSellerNames[randomIndex];             // Set the Seller Name
}

// Torna una lista lunga 'numProducts' di Prodotti vendibili dal Seller
/*
void generateSellerProduct(int numProducts) {
    string sellerProducts[numProducts];
    int selectedProducts[SELLER_MAX_PRODUCTS] {-1};         // Lista dei prodotto già inserit
    int i = 0;
flag3: 
    while (i < numProducts) {
        size_t numElements = sizeof(allSellerProducts) / sizeof(allSellerProducts[0]);  // totale numero di elmenti vendibili
        int randomIndex = rand() % numElements;             // Numero generato dalla lista di tutti i prodotti
        
        // Scorro la lista delle SelectedProducts per vedere se questo indice l'ho già inserito
        for (int j = 0; j < SELLER_MAX_PRODUCTS; j++) {
            if (randomIndex == selectedProducts[j]) {       // Se il numero generato l'ho già tirato fuori prima    
                goto flag3;                                   //!  Il prodotto che sto controllando è già presente. Ricicla.
            }
        }
        selectedProducts[i] = randomIndex;                  // Questo prodotto è disponibile -> lo inserisco in selected
        sellerProducts[i] = allSellerProducts[randomIndex]; // Questo prodotto è disponibile -> lo inserisco in sellerProducts
        i++;
    }
}
*/


void Seller::generateSellerProduct() {
    // Generate random number of Seller Products
    int selectedProducts[SELLER_MAX_PRODUCTS] {-1};
    int i = 0;
flag:
    while (i < SELLER_MAX_PRODUCTS) {
        size_t numElements = sizeof(allSellerProducts) / sizeof(allSellerProducts[0]);  // totale numero di elementi vendibili
        int randomIndex = rand() % numElements;             // Numero generato dalla lista di tutti i prodotti
        
        // Scorro la lista delle SelectedProducts per vedere se questo indice l'ho già inserito
        for (int j = 0; j < SELLER_MAX_PRODUCTS; j++) {
            if (randomIndex == selectedProducts[j]) {       // Se il numero generato l'ho già tirato fuori prima    
                goto flag;                                   //!  Il prodotto che sto controllando è già presente. Ricicla.
            }
        }
        int randomCost = rand() % 1001;                     // Max Cost is 1000€
        sellerProductsCost[i] = randomCost;                 // Aggiungo il costo del prodotto analizzato
        selectedProducts[i] = randomIndex;                  // Questo prodotto è disponibile -> lo inserisco in selected
        sellerProducts[i] = allSellerProducts[randomIndex]; // Questo prodotto è disponibile -> lo inserisco in sellerProducts          
        i++;
    }
}

void Seller::connection(){
    printf("### CONNECTION FASE Seller : %s \n",sellerName.c_str());

    // ? Connection
    printf("User %s: connecting to redis ...\n", sellerName.c_str());
    this->c2r = redisConnect("localhost", 6379);
    printf("User %s: connected to redis\n\n", sellerName.c_str());

    /* Create streams/groups */
    initStreams(c2r, WRITE_STREAM);

}

void Seller::send_products(){
    //! Protocollo di comunicazione Seller : {Product|cost|Seller}
    printf("#### \n%s Sending products...\n\n", sellerName.c_str());
    for(int i=0; i<SELLER_MAX_PRODUCTS; i++){         
        // reply = RedisCommand(c2r, "XADD %s * foo mem:%d", WRITE_STREAM, 30);
        reply = RedisCommand(c2r, "XADD %s * prod %s price %d seller %s", WRITE_STREAM , sellerProducts[i].c_str(), sellerProductsCost[i], sellerName.c_str());
        //assertReplyType(c2r, reply, REDIS_REPLY_STRING);
        assertReply(c2r, reply);
        printf("    -> Sended item %d : %s %d %s (id: %s)\n", i,sellerProducts[i].c_str(), sellerProductsCost[i], sellerName.c_str(), reply->str);
        freeReplyObject(reply);
    }

    printf("\n\n %s : Sending Done!\n", sellerName.c_str());
}


/* ------------------------------ Stamp Section ----------------------------- */

void Seller::printProductList() {
    for(int i = 0; i < SELLER_MAX_PRODUCTS; i++){
        if (sellerProducts[i] == "") {
            break;
        }
        printf("    %d - %d €: %s\n",i,sellerProductsCost[i],sellerProducts[i].c_str());
    }
    printf("(NUM_PROD : %d)\n",SELLER_MAX_PRODUCTS);
}

void Seller::toString() {
    printf("SEED: %u| %s SELL %d PRODUCTS:\n",myseed,sellerName.c_str(),SELLER_MAX_PRODUCTS);
    printProductList();
}
