#include "Counter.hpp"
#include <cstdint>

// ===================================================================
// Documentation for these methods are in the header file, Counter.hpp
// ===================================================================

Counter::Counter(uint64_t width) {
    // TODO: implement me
    this->width = width;
    this->state = 2;
}

void Counter::update(bool taken) {
    // TODO: implement me
    if (this->width == 3){
        if (taken){
            if (this->state < 7){
                this->state++;
            }
        } else{
            if(this->state > 0 ){
                this->state--;
            }
        }
    }
    else{
        if (taken){
            if (this->state < 3){
                this->state++;
            }
        } else{
            if(this->state > 0 ){
                this->state--;
            }
        } 
    }
    

}

uint64_t Counter::get() {
    // TODO: implement me
    return state;
}

bool Counter::isTaken() {
    // TODO: implement me
    if (this->width == 3){
        return (this->state > 3);
    }
    return (state > 1);
}

void Counter::setCount(uint64_t count) {
    // TODO: implement me
    this->state = count;
}

bool Counter::isWeak() {
    // TODO: implement me
    // Check this
    if (this->width == 3){
        return (state > 2 && state < 5);
    }
    return (state > 0 && state < 3);
}

void Counter::reset(bool taken) {
    // TODO: implement me
    if (taken){
        state = 4;
    }
    else{
        state = 3;
    }
}
