#include <iostream>
#include <cmath>
#include <iterator>
#include <algorithm>
#include <iomanip>

#include "branchsim.hpp"
#include "gshare.hpp"
#include "tage.hpp"
#include "TAGE_GHR.hpp"
#include "Counter.hpp"

enum {SN, WN, WT, ST};
int i = 0;

// Don't modify this line -- Its to make the compiler happy
branch_predictor_base::~branch_predictor_base() {}


// ******* Student Code starts here *******

// Gshare Branch Predictor

void gshare::init_predictor(branchsim_conf *sim_conf)
{
    // TODO: implement me
    this->G = sim_conf->S - 1;
    this->hash = sim_conf->P;
    uint64_t size = (uint64_t) pow(2.0, (double) this->G);
    this->pattern_table = new Counter[size];
    for(uint64_t i = 0; i < size; i++){
        pattern_table[i].state = WT;
        pattern_table[i].width = i;
    }
    std::cout<<size;
    
}

uint64_t get_cap(uint64_t num_bits){
    if (num_bits == 12){
        return 0xfff;
    }
    else if (num_bits == 13 ){
        return 0x1fff;
    }
    else if (num_bits == 14 ){
        return 0x3fff;
    }
    else if (num_bits == 15 ){
        return 0x7fff;
    }
    else if (num_bits == 16 ){
        return 0xffff;
    }
    else if (num_bits == 17 ){
        return 0x1ffff;
    }
    else{
        return 0x3ffff;
    }
}
uint64_t get_index0(uint64_t pc, uint64_t G, uint64_t GHR){
    uint64_t pos = 2;
    uint64_t mask = 1;
    uint64_t num_bits = G;
    uint64_t pc_xtract = ((( mask << num_bits)- mask ) & (pc >> (pos )));
    return (GHR ^ pc_xtract) & get_cap(num_bits);
}

uint64_t get_index1(uint64_t pc, uint64_t G, uint64_t GHR){
    uint64_t pos = 0;
    uint64_t mask = 1;
    uint64_t num_bits = G;
    uint64_t pc_xtract = ((( mask << num_bits)-mask ) & (pc >> (pos )));
    return (GHR ^ pc_xtract) & get_cap(num_bits);
}

uint64_t get_index2(uint64_t pc, uint64_t G, uint64_t GHR){
    uint64_t pos = 2;
    uint64_t num_bits = G;
    uint64_t mask = 1;
    uint64_t pc_xtract = ((( mask << num_bits) - mask ) & (pc >> (pos )));
    uint64_t sum = pc_xtract + GHR;
    uint64_t sum_xtract = ((( mask << num_bits) - mask ) & (sum >> (pos-2 )));
    return sum_xtract & get_cap(num_bits);
}


bool gshare::predict(branch *branch, branchsim_stats *sim_stats)
{
    // TODO: implement me. NO I don-t want to,  what you gon do_?
    sim_stats->num_branch_instructions++;
    if (this->hash == 0) this->index = get_index0(branch->ip, this->G, this->ghr);
    if (this->hash == 1) this->index = get_index1(branch->ip, this->G, this->ghr);
    if (this->hash == 2) this->index = get_index2(branch->ip, this->G, this->ghr);
    //std::cout<<"index "<<this->index<< "\n";
    
    if (this->pattern_table[this->index].tag != branch->ip){
        sim_stats->num_tag_conflicts++;
        this->pattern_table[this->index].tag = branch->ip;
    }
    return this->pattern_table[this->index].state > 1;
    
}

void gshare::update_predictor(branch *branch)
{
    // TODO: implement me
    if(i < 10){
        std::cout << std::hex << this->ghr << std::endl;        
    }
    i++;
    this->pattern_table[this->index].update(branch->is_taken);
    if(branch->is_taken){
        update_ghr(1);
    } else {
        update_ghr(0);
    }

}

void gshare::update_ghr(uint64_t i){
    
    this->ghr = ghr << 1;
    this->ghr = this->ghr | i;
}

gshare::~gshare()
{
    // TODO: implement me
}

// TAGE PREDICTOR

void tage::init_predictor(branchsim_conf *sim_conf) {
    // TODO: implement me
}

bool tage::predict(branch *branch, branchsim_stats *sim_stats)
{
    // TODO: implement me
    return false;
}


void tage::update_predictor(branch *branch) {
    // TODO: implement me
}

tage::~tage() {
    // TODO: implement me
}


// Common Functions to update statistics and final computations, etc.

/**
 *  Function to update the branchsim stats per prediction. Here we now know the
 *  actual outcome of the branch, so you can update misprediction counters etc.
 *
 *  @param prediction The prediction returned from the predictor's predict function
 *  @param branch Pointer to the branch that is being predicted -- contains actual behavior
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_update_stats(bool prediction, branch *branch, branchsim_stats *sim_stats) {
    // TODO: implement me
    //std::cout<<i++<<" \n";
    if ((prediction  && branch->is_taken) || (!prediction  && !branch->is_taken) ){
        sim_stats->num_branches_correctly_predicted++;
    }
    else{
        sim_stats->num_branches_mispredicted++;
    }
}

/**
 *  Function to finish branchsim statistic computations such as prediction rate, etc.
 *
 *  @param stats Pointer to the simulation statistics -- update in this function
*/
void branchsim_finish_stats(branchsim_stats *sim_stats) {
    // TODO: implement me
    
}
