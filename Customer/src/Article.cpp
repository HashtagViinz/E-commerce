#include "Article.h"

#include<iostream>
#include <string.h>

// Costruttore : (itemName, itemPrice, itemSeller)
Article::Article(const char* itemName, const char* itemPrice, const char* itemSeller) {
    this-> price = itemPrice;
    this->name = itemName;
    this->seller = itemSeller;
}

void Article::getItem() {
    cout <<"Product: " << name << " - Price: " << price << " - Seller : " << seller <<endl ;
}


std::string Article::getName() {
    return name;
}

std::string Article::getPrice() {
    return price;
}

std::string Article::getSeller(){
    return seller;
}