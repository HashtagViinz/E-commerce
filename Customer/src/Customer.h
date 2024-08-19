#ifndef SELLER_H           // Se non è definito allora lo definsco
#define SELLER_H

#include "main.h"
#include "../../global.h"
#include "Article.h"
#include <string>


#define WRITE_STREAM "Customer_r_stream"    // Server ascolta in questo canale 
#define READ_STREAM "Customer_w_stream"     // Server scrive in questo canale

#define SELLER_STATE_LENGHT 5               // ? Dimensione dell'Enum degli Stati di Customer



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
        vector<Article> catalog;       //TODO - Toglila in seguito non ha senso qui  


        redisContext *c2r;
        redisReply *reply;
        int block = 1000000000;             //TODO Valore che indica l'attendere


    //! Funzioni private
        void generateID();
        void elaboration();
        void nextState();
        std::string getStrState();
        void connectToServer();
        void requestArticles();
        void addItem(Article article);
        size_t getItemCount() const;

    public:
        Customer(int seed, int ID);


};



#endif