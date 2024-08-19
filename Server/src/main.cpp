
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

  Server sw;
  //Item item1("Televisione",20,"Amazon");
  //Item item2("Scaffale", 50, "Ebay");

  //sw.addItem(item1);
  //sw.addItem(item2);

  //sw.getAvailable_Items();
}
