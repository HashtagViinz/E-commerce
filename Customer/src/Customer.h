#ifndef SELLER_H           // Se non è definito allora lo definsco
#define SELLER_H

#include "main.h"
#include "../../global.h"
#include "Article.h"
#include <string>
#include <iostream>
#include <unistd.h>  // Per sleep


#define WRITE_STREAM "Customer_r_stream"    // Server ascolta in questo canale 
#define READ_STREAM "Customer_w_stream"     // Server scrive in questo canale
#define CTRL "Control_Channel"
#define OBJ_CH "Object_Channel"
#define DELIVER_STREAM "Customer_Deliver_stream"      // ? Stream di ordini
#define ANOMALY_STREAM "Anomaly_Stream"



#define SELLER_STATE_LENGHT 5                   // ? Dimensione dell'Enum degli Stati di Customer
#define MAX_ORDER 4                             // ? Numero massimo di ordini
#define NOT_ORDER_PROB  2                       // ! Probabilità di NON ordinare nuovamente (4/10)
#define NULL_ORDER  1                           // ! Probabilità Ordine vuoto


enum Customer_State {
    CUSTOMER_GENERATION,    
    SERVER_CONNECTION,
    ORDER_PHASE,        //Richiede la lista degli Items al Server e ne sceglie 1
    MAKE_DECISION_PHASE,// Fase che permette di fare una scelta
    CHOICE_PHASE,       //Scegli gli articoli nel suo catalogo personale 
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
        int orderCounter = 0;

        redisContext *c2r;
        redisReply *reply;
        int block = 1000000000;             //TODO Valore che indica l'attendere
        int timedBlock = 10000;             // ? Utilizzato per temporizzare l'attesa 


    //! Funzioni private
        void generateID();
        void elaboration();
        void nextState();
        void changeState(Customer_State swNewState);
        std::string getStrState();
        void connectToServer();
        void requestArticles();
        void addItem(Article article);
        void makeDecision();
        void choose_item();
        size_t getItemCount() const;
        void listenDeliver();
        void listenAnomalies();

        void sendObj();
        void sendNoObj();
        void sendReqObj();

    public:
        Customer(int seed, int ID);
        //TODO Elimina questa Funzione
        static void printReply(redisReply *reply, int level = 0); // Argomento predefinito qui


};



#endif