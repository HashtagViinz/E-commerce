#ifndef ARTICLE_H           // Se non è definito allora lo definsco
#define ARTICLE_H

#include <string>
using namespace std;

class Article {
    private:
        string name;
        string price;
        string seller;
    public:
        Article(const char* itemName, const char* itemPrice, const char* itemSeller);
        void getItem();
        std::string getName();
        std::string getPrice();
        std::string getSeller();
};

#endif