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

uint64_t Counter::get() {
    // TODO: implement me
    return state;
}

bool Counter::isTaken() {
    // TODO: implement me
    return (state > 1);
}

void Counter::setCount(uint64_t count) {
    // TODO: implement me
}

bool Counter::isWeak() {
    // TODO: implement me
    return (state > 0 && state < 3);
}

void Counter::reset(bool taken) {
    // TODO: implement me
}
