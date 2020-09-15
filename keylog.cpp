#include "keylog.h"
#include <QRect>
#include <QDesktopWidget>
#include <QApplication>
#include <QDebug>
Keylog::Keylog()
{
    this->key_logs.resize(this->cap);
    QRect screenRect = QApplication::desktop()->screenGeometry();
    this->screen_width = screenRect.width();
    this->screen_height = screenRect.height();
}

Item *Keylog::get_end()
{
    return this->key_logs[this->end_index];
}


void Keylog::add(Item *item)
{
   if(this->end_index == this->cap - 1){
        Item *head = this->key_logs[0];
        for(int i = 0; i < this->cap - 1; i++){
            this->key_logs[i] = this->key_logs[i+1];
        }
        delete head;
        this->key_logs[this->end_index] = item;
    } else if(this->end_index < this->cap){
       this->end_index++;
       this->key_logs[this->end_index] = item;
   }

   this->update();
}

void  Keylog::update(){
    int index = 1;
    int distance = 60;
    for(int i = end_index; i >= 0; i--){
        this->key_logs[i]->y =  (screen_height - this->key_logs[i]->img->height) - (index * distance);
        this->key_logs[i]->x =  screen_width - this->key_logs[i]->img->width  - 40;
        index++;
    }
}

bool Keylog::empty()
{
    return end_index == -1 ? true : false;
}

void Keylog::clear()
{
    if(this->empty()){
        return;
    }
    for(int i = 0; i < this->end_index; i++){
        delete(this->key_logs[i]);
    }
    this->end_index = -1;
}
