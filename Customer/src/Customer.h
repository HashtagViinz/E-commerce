#ifndef SELLER_H           // Se non è definito allora lo definsco
#define SELLER_H

#include "main.h"
#include <string>

#define WRITE_STREAM "stream1"     //$ Canale di comunicazione fra Customer e Sw in Read 4 Customer  

#define SELLER_STATE_LENGHT 5           // ? Dimensione dell'Enum degli Stati di Customer



enum Customer_State {
    CUSTOMER_GENERATION,    
    SERVER_CONNECTION,
    ORDER_PHASE,        //Richiede la lista degli Items al Server e ne sceglie 1
    WAITING_PHASE,      // Aspetta che arrivi il pacco
    SATISFACTION_PHASE, // Il pacco è arrivato
    DIED,               // Utente morto | Non effettua più nessun ordine
};

class Customer{
    private:
        int pid;
        int ID;
        Customer_State customer_State;
        unsigned int myseed;            // random Seed 

        redisContext *c2r;
        redisReply *reply;

    //! Funzioni private
        void generateID();
        void elaboration();
        void nextState();
        std::string getStrState();
        void connectToServer();
        void requestArticles();

    public:
        Customer(int seed, int ID);


};



#endif