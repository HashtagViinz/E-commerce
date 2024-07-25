#ifndef SERVER_H           // Se non è definito allora lo definsco
#define SERVER_H

// Lib :
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <list>
#include <string.h>

// Comunicazione redis
#include "con2redis.h"          //Includiamo l'interfaccia di con2redis
#include "main.h"
#include "Item.h"

#define READ_STREAM "stream2"

enum Server_State {
    ON_LISTEN,
    ON_SELLER,
    ON_CUSTOMER,
    ON_DELIVERY,
};

class Server {
    private:
        int pid; 
        Server_State swState = ON_LISTEN;
        redisContext *c2r;
        redisReply *reply;
        std::list<Item> available_Items;
        bool read = true;       //TODO Valore temporaneo
        int block = 1000000000; //TODO Valore che indica l'attendere

    // ! Metodi privati
        void changeState(Server_State swNewState);
        std::string getStrState();
        void running();         // Gestione delle funzioni in base allo stato
        void connection();

    public:
        Server();
        int getPid();
        void addItem(Item item);
        void getAvailable_Items();
};



#endif