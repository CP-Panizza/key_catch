#include "item.h"
#include "matrix.hpp"


Item::Item(Matrix<int> *_img, time_t _show_time):img(_img), show_time(_show_time){

}
Item::~Item(){
    delete this->img;
}
