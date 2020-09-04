#ifndef ITEM_H
#define ITEM_H
#include "matrix.hpp"

class Item{
public:
    Item(Matrix<int> *_img, time_t _show_time);
    ~Item();

    Matrix<int> *img;  //key_img
    time_t show_time;  //show time in screen
    int x;             //position x  on screen
    int y;             //position y  on screen
};

#endif // ITEM_H
