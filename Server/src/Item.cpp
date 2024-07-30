
#include "Item.h"

#include<iostream>
#include <string.h>

// Costruttore : (itemName, itemPrice, itemSeller)
Item::Item(const char* itemName, const char* itemPrice, const char* itemSeller) {
    this-> price = itemPrice;
    this->name = itemName;
    this->seller = itemSeller;
}

void Item::getItem() {
    cout <<"Product: " << name << " - Price: " << price << " - Seller : " << seller <<endl ;
}

std::string Item::getName() {
    return name;
}