#ifndef DELIVER_H           // Se non è definito allora lo definsco
#define DELIVER_H


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <random>
#include <fstream>              // per gestire file input/output
#include <chrono>
#include <iomanip>              // Necessario per std::setw e std::setfill

using namespace std;

#include "main.h"
#include "con2redis.h"

#define NOT_ACCEPT_ORDER 2                              // ? Probabilità che l'ordine non sia accettato
#define SELLER_STATE_LENGHT 3                           // ? Dimensione dell'Enum degli Stati di Seller
#define READ_STREAM "order_stream"                      // ? Stream di ordini
#define CUSTOMER_STREAM "Customer_Deliver_stream"       // ? Stream di ordini

#define ACC_RIF_DB "../../stats/Ordine.csv"

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
        std::vector<std::string> intestazioneDeliv = {"Stato", "Product", "Price", "Seller", "User", "Timestamp"};
  
        
        redisContext *c2r;
        redisReply *reply;
        int block = 1000000000;

        // ! Metodi PRIVATE
        void processing();
        std::string getStrState();
        void nextState();
        void connection();
        void onListen();
        bool acceptedOrder();
        void creaIntestazione(const std::vector<std::string>& intestazione);
        void aggiungiRigaAlCSV(const std::string &percorsoFile, const std::vector<std::string> &dati);
        std::string timestampToString();
        void saveData(string DB, string stato, string product, string price, string seller, string user,string time);

    public:
        Deliver();
        

};
/* ---------------------------- FUNZIONI GENERALI --------------------------- */
#endif