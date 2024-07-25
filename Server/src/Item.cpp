
#include "Item.h"

#include<iostream>
#include <string.h>

// Costruttore : (itemName, itemPrice, itemSeller)
Item::Item(string itemName, int itemPrice, string itemSeller) {
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