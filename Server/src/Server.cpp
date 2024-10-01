
#include "Server.h"
#include <iostream>

 /* ------------------------------- Costruttore ------------------------------ */
Server::Server() {
    this->pid = getpid();
    this->swState = ON_CONNECTION;
    
    // Creiamo l'intestazione solo se il file non esiste
    creaIntestazione(DB, intestazioneProduct);
    creaIntestazione(ERR_COMUNICATION, intestazioneErr_Com);
    creaIntestazione(LOG_ORD, intes_LOG_Ord);

    printf("Hello I'm Alive :D\n\n");

    for(int i = 0; i<LIFE_STEPS; i++){
        this->running();
    }
    file.close();
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
    freeReplyObject(reply);

    reply = RedisCommand(c2r, "DEL %s", CTRL);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    reply = RedisCommand(c2r, "DEL %s", ANOMALY_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);


    /* Create streams/groups */
    initStreams(this->c2r, READ_STREAM);
    initStreams(this->c2r, CTRL);
    initStreams(this->c2r, OBJ_CH);
    initStreams(this->c2r, ANOMALY_STREAM);
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

        std::vector<std::string> obj = {timestampToString(),product, price, seller};    // $ Ci salviamo le informazioni come vettore di stringhe
        aggiungiRigaAlCSV(DB, obj);

        addItem(Item(product, price, seller));
        //printf("(#!!#) Saved (%ld) || Item intercettato : (%s|%s|%s)\n", getItemCount(), product, price, seller);

        freeReplyObject(reply);
        i++;
    }
    printf("Product acquisition completed!\n");
}

// Funzione che gestisce la comunicazione fra Customer e Server
void Server::listenCustomer(){
    int delay = rand() % 3;
    bool wait = true;
    char msg[20];                       // Valore messaggio richiesta

    //? delete stream if it exists
    reply = RedisCommand(c2r, "DEL %s", CUST_R_STREAM);
    assertReply(c2r, reply);
    dumpReply(reply, 0);
    freeReplyObject(reply);

    /* Create streams/groups */
    initStreams(c2r, CUST_R_STREAM);

    printf("Attesa cliente...");

    //! Mandiamo il catalogo al Seller
    reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 STREAMS %s >", 
                          block, CTRL);
                          
    auto start = std::chrono::high_resolution_clock::now(); 

    std::vector<std::string> obj = {timestampToString(), "1"};    // $ Ci salviamo le informazioni come vettore di stringhe
    aggiungiRigaAlCSV(LOG_ORD, obj);
    
    assertReply(c2r, reply);                                // Verifica errori nella comunicazione
    memset(msg, 0, sizeof(msg));                            // Pulisco i valori dei buffer
    ReadStreamMsgVal(reply, 0, 0, 1, msg);                  // Leggo il valore 
    freeReplyObject(reply);

    if(strcmp(msg, "RequestCatalog") == 0){                 
        printf("# Sending Catalog...\n");
        sleep(delay);                                       // ? Genero un ritardo Randomico

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

    auto end = std::chrono::high_resolution_clock::now();   //! timestamp end
    printf("#RITARDO: %s %f | DELAY: %d\n",isLessThanOneSecond(start, end), durationInSeconds(start, end), delay);
    printf("# Catalog sended...\n");    

    // ! ASPETTIAMO ORDINI
                                        //$ info mex
    char product[100];                  // Prodotto
    char price[4];                      // prezzo
    char seller[100];                   // valori azienda
    char user[100];                     // user
    
    printf("#################### - In ascolto degli ordini del client:\n");
    while(wait){
        reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                            block, CTRL);        
        // TODO Togli questa sezione 
        if (reply == NULL) {
            printf("----------------Errore: reply è NULL\n");
            continue; // Salta al prossimo ciclo se reply è NULL
        }
        if (reply->type == REDIS_REPLY_ERROR) {
            printf("----------------Errore Redis: %s\n", reply->str);
            freeReplyObject(reply);
            continue;
        }

        assertReply(c2r, reply);                // Verifica errori nella comunicazione
        ReadStreamMsgVal(reply, 0, 0, 1, msg);  // Leggo il valore 
        freeReplyObject(reply);

        if(strcmp(msg, "SendingObject") == 0){             // E' Stato mandato un obj 
            //printf("# Connessione instaurata - Ordine in arrivo: \n");

            reply = RedisCommand(c2r, "XREADGROUP GROUP diameter Tom BLOCK %d COUNT 1 NOACK STREAMS %s >", 
                            timedBlock, OBJ_CH);
            assertReply(c2r, reply);  // Verifica errori nella comunicazione

            // ! Gestione ordine dal Client
            ReadStreamMsgVal(reply, 0, 0, 1, product);
            ReadStreamMsgVal(reply, 0, 0, 3, price);
            ReadStreamMsgVal(reply, 0, 0, 5, seller);
            ReadStreamMsgVal(reply, 0, 0, 7, user);
            freeReplyObject(reply);

            // ! Controllo Anomalia
            if(checkData(product, price, seller)){         
                printf("Sto mandando in CTRL : ANOMALY\n");
                reply = RedisCommand(c2r, "XADD %s * requestCode %s", ANOMALY_STREAM, "ANOMALY"); // $ Comunichiamo che abbiamo trovato un'anomalia
                assertReply(c2r, reply);
                freeReplyObject(reply); 

                std::vector<std::string> obj = {user, timestampToString()};    // $ Ci salviamo le informazioni come vettore di stringhe
                aggiungiRigaAlCSV(ERR_COMUNICATION, obj);
                printf("#ANOMALIA di %s\n", user);
                continue;
            }
            printf("Sto mandando in CTRL : NO_ANOMALY\n");
            reply = RedisCommand(c2r, "XADD %s * requestCode %s", ANOMALY_STREAM, "NO_ANOMALY");  // $ Comunichiamo che non c'è nessuna anomalia
            assertReply(c2r, reply);
            freeReplyObject(reply); 

            printf("# Ordine Ricevuto : (%s, %s, %s, %s)\n", product, price, seller, user);

            // ! Invio dell'ordine ai Deliver
            reply = RedisCommand(c2r, "XADD %s * product %s price %s seller %s user %s", ORDER_STREAM ,product, price, seller, user);  
            assertReply(c2r, reply);
            freeReplyObject(reply);

            // Pulisco i valori dei buffer
            memset(product, 0, sizeof(product));
            memset(price, 0, sizeof(price));
            memset(seller, 0, sizeof(seller));
            memset(seller, 0, sizeof(user));
        } else if(strcmp(msg, "StopSendingObject") == 0) {
            printf("# Nessun ordine effettuato.\n");
            wait=false;
        }
        memset(msg, 0, sizeof(msg));            // Pulisco i valori dei buffer
    }
    printf("#################### - Chiusura connessione Ordini\n");

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

int Server::getPid() {
    return pid;
}

void Server::addItem(Item item) {
    available_Items.push_back(item);
}

// Funzione per convertire timestamp in stringa
std::string Server::timestampToString() {
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

// Funzione che stampa gli items
void Server::getAvailable_Items() {
    for (Item i : available_Items){
        i.getItem();
    }
}

double Server::durationInSeconds(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end)
{
     // Calcola la differenza in secondi
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    
    return duration.count(); // Restituisce la durata in secondi come double
}

const char *Server::isLessThanOneSecond(std::chrono::high_resolution_clock::time_point start, std::chrono::high_resolution_clock::time_point end)
{
    if(durationInSeconds(start, end) < CHECK_DELAY){ // Restituisce true se la differenza è inferiore a 1 secondo
        return "FALSE";
    } else {
        return "TRUE";
    }
}

size_t Server::getItemCount() const {
    return available_Items.size();
}

// Creiamo il file csv per i log
void Server::creaIntestazione(const std::string& nomeFile, const std::vector<std::string>& intestazione) {
    std::ifstream fileTest(nomeFile);  // Apri il file passato come argomento
    bool esiste = fileTest.good();     // Controlla se il file esiste
    fileTest.close();

    // Se il file non esiste, scriviamo l'intestazione
    if (!esiste) {
        aggiungiRigaAlCSV(nomeFile, intestazione);  // Aggiungi l'intestazione al file specificato
    }
}

void Server::aggiungiRigaAlCSV(const std::string& percorsoFile, const std::vector<std::string>& dati) {
    std::ofstream file;

    // Apriamo il file in modalità append
    file.open(percorsoFile, std::ios::app);

    // Controlliamo se il file è stato aperto correttamente
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file!" << std::endl;
        return;
    }

    // Scriviamo i dati separati da virgole
    for (size_t i = 0; i < dati.size(); ++i) {
        file << dati[i];
        if (i != dati.size() - 1) {
            file << ",";  // Separatore tra i dati
        }
    }
    file << "\n";  // Fine riga

    // Chiudiamo il file
    file.close();
}

bool Server::checkData(const char product[], const char price[], const char seller[]) {
    return isValidCharArray(product) && isValidCharArray(price) &&
           isValidCharArray(seller);
}

bool Server::isValidCharArray(const char *str)
{   
    return strcmp(str, " ") == 0 || strcmp(str, "") == 0; 
    
}