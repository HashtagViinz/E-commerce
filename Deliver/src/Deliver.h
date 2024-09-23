#ifndef DELIVER_H           // Se non è definito allora lo definsco
#define DELIVER_H


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <random>
using namespace std;

#include "main.h"
#include "con2redis.h"

#define SELLER_STATE_LENGHT 3           // ? Dimensione dell'Enum degli Stati di Seller
#define READ_STREAM "order_stream"      // ? Stream di ordini


enum Delivery_state {
    DELIVER_GENERATION,
    ON_CONNECTION,
    ON_LISTEN,
};

class Deliver {
    private:
        // ! Attribute
        int pid;
        Delivery_state delivery_state;      // State of the Seller
        string delivery_name;               // Name of the
        unsigned int myseed;                // random Seed   
        
        redisContext *c2r;
        redisReply *reply;
        int block = 1000000000;

        // ! Metodi PRIVATE
        void processing();
        std::string getStrState();
        void nextState();
        void connection();
        void onListen();
        
    public:
        Deliver();
        

};
/* ---------------------------- FUNZIONI GENERALI --------------------------- */
#endif