#ifndef SELLER_H           // Se non è definito allora lo definsco
#define SELLER_H


#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <random>
using namespace std;




#include "main.h"
#include "con2redis.h"

#define READ_STREAM "stream1"
#define WRITE_STREAM "stream2"
#define SELLER_STATE_LENGHT 4           // ? Dimensione dell'Enum degli Stati di Seller
#define SELLER_MAX_PRODUCTS 10          // ? Numero massimo di oggetti vendibili dal Seller

enum Seller_State {
    SELLER_GENERATION,
    LOADING_PRODUCT,
    CONNECTION_SERVER,
    UPLOADING_PRODUCT,
};

class Seller {
    private:
        // ! Attribute
        int pid;
        Seller_State seller_State;      // State of the Seller
        string sellerName;              // Name of the
        int numRealSellingProducts = 0; // Number of the real Product that i sell
        string sellerProducts[10];      // Seller can Sell max 10 items 
        int sellerProductsCost[10];     // Cost of the i item.
        unsigned int myseed;            // random Seed   
        redisContext *c2r;
        int block = 1000000000;

        // ! Metodi PRIVATE
        void getNameFromDB();
        void generateSellerProduct();
        void printProductList();
        void connection();
        void nextState();

    public:
        Seller();
        Seller(string name);
        Seller(int seed);
        Seller(int seed, string name);
        void processing();
        void setProductsList(int lenLst, int* pntList);
        string getStrState();
        void toString();

};
/* ---------------------------- FUNZIONI GENERALI --------------------------- */
string generateSellerName();
#endif