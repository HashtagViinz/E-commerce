#include "Deliver.h"

#include<iostream>
#include <string.h>

/* ------------------------------- Costruttore ------------------------------ */
Deliver::Deliver() {
    this->delivery_state = DELIVER_GENERATION;
    this->pid = getpid();
    this->myseed = (unsigned)time(NULL); // Uso un Seed per provare tante possibili strade per il mio Sellers. Ogni seed crea strade diverse.
    srand(myseed);

    // Creiamo l'intestazione solo se il file non esiste
    creaIntestazione(intestazioneDeliv);


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

    reply = RedisCommand(c2r, "DEL %s", CUSTOMER_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    /* Create streams/groups */
    initStreams(c2r, READ_STREAM);
    initStreams(c2r, CUSTOMER_STREAM);
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

        freeReplyObject(reply);

        // ? Genero la possibilità che un ordine venga accettato.
        if(acceptedOrder()){
            printf("# ORDINE RIFIUTATO :(%s,%s,%s,%s)\n", product, price, seller, user);
            reply = RedisCommand(c2r, "XADD %s * stato %s product %s price %s seller %s", CUSTOMER_STREAM, "RIFIUTATO", product, price, seller);  
            assertReply(c2r, reply);
            freeReplyObject(reply);

            saveData(ACC_RIF_DB,"RIFIUTATO",product, price, seller, user,timestampToString());

        }
        else{
            printf("# ORDINE ACCETTATO :(%s,%s,%s,%s)\n", product, price, seller, user);
            reply = RedisCommand(c2r, "XADD %s * stato %s product %s price %s seller %s", CUSTOMER_STREAM, "ACCETTATO", product, price, seller); 
            assertReply(c2r, reply);
            freeReplyObject(reply);

            saveData(ACC_RIF_DB, "ACCETTATO",product, price, seller, user,timestampToString());
        }
        // Pulisco i valori dei buffer
        memset(product, 0, sizeof(product));
        memset(price, 0, sizeof(price));
        memset(seller, 0, sizeof(seller));
        memset(seller, 0, sizeof(user));
    }
}   

void Deliver::saveData(string DB,string stato, string product, string price, 
                                            string seller, string user,string time){
    std::vector<std::string> obj = {stato,product, price, seller, user, time};    // $ Ci salviamo le informazioni come vettore di stringhe
    aggiungiRigaAlCSV(DB, obj);
}

bool Deliver::acceptedOrder(){
    return (rand() %10 +1) <= NOT_ACCEPT_ORDER;
}

void Deliver::creaIntestazione(const std::vector<std::string> &intestazione){
    std::ifstream fileTest(ACC_RIF_DB);
    bool esiste = fileTest.good();
    fileTest.close();
    // Se il file non esiste, scriviamo l'intestazione
    if (!esiste) {
        aggiungiRigaAlCSV(ACC_RIF_DB, intestazione);
    }
}

void Deliver::aggiungiRigaAlCSV(const std::string& percorsoFile, const std::vector<std::string>& dati) {
    std::ofstream file;
    file.open(percorsoFile, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file!" << std::endl;
        return;
    }
    for (size_t i = 0; i < dati.size(); ++i) {
        file << dati[i];
        if (i != dati.size() - 1) {
            file << ","; 
        }
    }
    file << "\n"; 
    file.close();
}

std::string Deliver::timestampToString() {
    std::string ss;

    //auto timestamp = std::chrono::high_resolution_clock::now();
    auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

    // Convertilo in struttura di tm
    std::tm* local_time = std::localtime(&time);

    // Estrai ore, minuti e secondi
    int hours = local_time->tm_hour;
    int minutes = local_time->tm_min;
    int seconds = local_time->tm_sec;

    // Crea una stringa con ore, minuti e secondi
    std::ostringstream time_stream;
    time_stream << std::setw(2) << std::setfill('0') << hours << ":"
                << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << seconds;    
    return time_stream.str();
}