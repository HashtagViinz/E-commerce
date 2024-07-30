

#include "main.h"

// cc -Wall -g -ggdb -o streams streams.c -lhiredis
// Usage: ./streams <add count> <read count> [block time, default: 1]

#define DEBUG 1000

#define READ_STREAM "stream1"
#define WRITE_STREAM "Seller_stream"

using namespace std;


int main() {

#if (DEBUG > 0)
  setvbuf(stdout, (char*) NULL, _IONBF, 0);
  setvbuf(stderr, (char*) NULL, _IONBF, 0);
#endif

  int i=0;

  // Inizializza il generatore di numeri casuali con il seme corrente
  std::srand(static_cast<unsigned int>(std::time(nullptr)));

  // Salva il seme in una variabile
  unsigned int seed = std::rand();


  // TODO - !  TOGLI IL FOR E GENERA SOLO UN SELLER


  // ! GENERATE 10 Seller-names all different
  std::string namesList[10] {""};
flag2:
  while(i<10){
    string tmpName = generateSellerName();
    for(int j = 0; j<10; j++){
      if (namesList[j] == tmpName){
        goto flag2;
      }
    }
    namesList[i] = tmpName;
    i++;
  }
  printf("\n\n");
  
  /*
  // ! Mostrami i nomi casuali:
  for(int i = 0; i<10; i++){
    printf("%d - %s\n",i,namesList[i].c_str());
  }
  printf("\n\n");
  */

  // ! Creami Seller con i Nomi trovati
  vector<Seller> sellerList;
  for(int i = 0; i<10; i++) {
    sellerList.push_back(Seller(seed,namesList[i])); 
  }
  
}

