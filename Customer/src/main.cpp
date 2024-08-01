
#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000


using namespace std;


int main() {
  
#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif
  // Inizializza il generatore di numeri casuali con il seme corrente
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  // Salva il seme in una variabile
  unsigned int seed = std::rand();

  // Generiamo un ID per i Customer
  int tmpID;
  vector<Customer> customers;
  for(int j=0; j<CUSTOMER_NUM; j++){
    customers.push_back(Customer(seed,tmpID)); 
    tmpID++;
  }
  // TODO CHECK se funziona tutto

}
