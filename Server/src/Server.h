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
#include <chrono>
#include <fstream>              // per gestire file input/output
#include <cctype>               // per std::isalnum
#include <cstring>              // per std::strlen
#include <sstream>
#include <iomanip>  // Necessario per std::setw e std::setfill





// Comunicazione redis
#include "con2redis.h"          //Includiamo l'interfaccia di con2redis

#include "main.h"
#include "Item.h"

#define SERVER_MAX_ITEMS 100
#define LIFE_STEPS 500          // indichiamo i passi di vita del Server
#define CHECK_DELAY 1           // valore di controllo del ritardo

#define DB "../../stats/tempDB.csv"
#define ERR_COMUNICATION "../../stats/Err_Comunication.csv"

#define CTRL "Control_Channel"
#define OBJ_CH "Object_Channel"
#define READ_STREAM "Seller_stream"
#define ORDER_STREAM "order_stream"      // ? Stream di ordini
#define CUST_W_STREAM "Customer_w_stream"   // Server scrive in questo canale
#define CUST_R_STREAM "Customer_r_stream"   // Server ascolta in questo canale

enum Server_State {
    ON_CONNECTION,  //$ Fase di connessione (1° Fase)
    ON_SELLER,      //$ prendiamo gli items dai venditori utilizzando redis per poi salvarli in DB
    ON_CUSTOMER,    //$ prendiamo gli ordini dei compratori (???)
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
        int timedBlock = 10000;             // ? Utilizzato per temporizzare l'attesa
        std::vector<std::string> intestazioneProduct = {"Product", "Price", "Seller"};
        std::vector<std::string> intestazioneErr_Com = {"User", "Timestamp"};

        std::ofstream file;                 // Stream per il file CSV



    // ! Metodi privati
        void changeState(Server_State swNewState);
        std::string getStrState();
        void running();         // Gestione delle funzioni in base allo stato
        void connection();      // Funzione utilizzata nella fase di connessione
        void listenSellers();          // Funzione che gestisce l'ascolto di tutti i canali di comunicazione provenienti dall'esterno
        void listenCustomer();  // Funzione chiamata da listen() serve per gestire le richieste dei customer
        size_t getItemCount() const;
        bool checkData(const char product[], const char price[], const char seller[]);
        bool isValidCharArray(const char *str);



    public:
        Server();
        int getPid();
        void addItem(Item item);
        void getAvailable_Items();
        double durationInSeconds(std::chrono::high_resolution_clock::time_point start,
                          std::chrono::high_resolution_clock::time_point end);
        const char* isLessThanOneSecond(std::chrono::high_resolution_clock::time_point start,
                          std::chrono::high_resolution_clock::time_point end);
        void aggiungiRigaAlCSV(const std::string& percorsoFile, const std::vector<std::string>& dati);
        void creaIntestazione(const std::vector<std::string>& intestazione);
        void creaIntestazioneErr_Com(const std::vector<std::string>& intestazione);
        //TODO Elimina questa Funzione
        static void printReply(redisReply *reply, int level = 0);
        std::string timestampToString();



};



#endif