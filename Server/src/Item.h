#ifndef ITEM_H           // Se non è definito allora lo definsco
#define ITEM_H

#include <string>
using namespace std;

class Item {
    private:
        string name;
        int price;
        string seller;
    public:
        Item(string itemName, int itemPrice, string itemSeller);
        void getItem();
        std::string getName();
};

#endif