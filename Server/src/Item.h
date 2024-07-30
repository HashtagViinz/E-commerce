#ifndef ITEM_H           // Se non è definito allora lo definsco
#define ITEM_H

#include <string>
using namespace std;

class Item {
    private:
        string name;
        string price;
        string seller;
    public:
        Item(const char* itemName, const char* itemPrice, const char* itemSeller);
        void getItem();
        std::string getName();
};

#endif