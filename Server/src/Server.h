#ifndef SERVER_H           // Se non è definito allora lo definsco
#define SERVER_H

// Lib :
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <list>
#include <string.h>
#include<vector>

// Comunicazione redis
#include "con2redis.h"          //Includiamo l'interfaccia di con2redis
#include "main.h"
#include "Item.h"

#define READ_STREAM "Seller_stream"

enum Server_State {
    ON_CONNECTION,  // $ Fase di connessione (1° Fase)
    ON_LISTEN,  
    ON_SELLER,      //$ prendiamo gli items dai venditori utilizzando redis per poi salvarli in DB
    ON_CUSTOMER,    //$ prendiamo gli ordini dei compratori (???)
    ON_DELIVERY,    //$ inviamo gli ordini ai trasportatori. (???)
};

class Server {
    private:
        int pid; 
        Server_State swState;   // Server Status
        redisContext *c2r;
        redisReply *reply;  
        vector<Item> available_Items;       //TODO - Toglila in seguito non ha senso qui  
        bool read = true;                   //TODO Valore temporaneo
        int block = 1000000000;             //TODO Valore che indica l'attendere

    // ! Metodi privati
        void changeState(Server_State swNewState);
        std::string getStrState();
        void running();         // Gestione delle funzioni in base allo stato
        void connection();      // Funzione utilizzata nella fase di connessione
        void listen();          // Funzione che gestisce l'ascolto di tutti i canali di comunicazione provenienti dall'esterno
        size_t getItemCount() const;

    public:
        Server();
        int getPid();
        void addItem(Item item);
        void getAvailable_Items();
        
        
        
        //TODO Elimina questa funzione: static void printReply(redisReply *reply, int level = 0); // Argomento predefinito qui

};



#endif