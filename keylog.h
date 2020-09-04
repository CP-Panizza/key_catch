#ifndef KEYLOG_H
#define KEYLOG_H

#include "matrix.hpp"
#include <vector>
#include "item.h"



class Keylog
{
public:
    Keylog();

    Item *get_end();
    void add(Item *item);
    void update();
    bool empty();

    int screen_width;
    int screen_height;

    const int cap = 7;
    std::vector<Item *> key_logs;
    int end_index = -1;
};

#endif // KEYLOG_H
