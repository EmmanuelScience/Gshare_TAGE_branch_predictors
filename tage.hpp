#ifndef TAGE_H
#define TAGE_H

#include "branchsim.hpp"
#include "TAGE_GHR.hpp"
#include "Counter.hpp"

class tag_table_data{
public:    
    Counter useful_counter{2};
    Counter pred_counter{3};
    uint64_t tag;
    
};
// TAGE predictor definition
class tage : public branch_predictor_base
{

private:
    
    // TODO: add your fields here

public:
    u_int64_t H, E, T, P, num_entries;
    TAGE_GHR *GHR;
    bool used_tag_table = false, pred, has_found_lmb;
    Counter *T0;
    //tag_table_data *tag_table;
    //std::vector<tag_table_data> tag_table;
    uint64_t index_t0, tag_table_index, longest_matching_branch, longest_index;
    /*position ti of the longest tag table*/
    uint64_t tag_table_pos;

    // create optional helper functions here
    /*void allocate(uint64_t size){
        tag_table_data dummy;
        for (uint64_t i =  0; i < size; i++){
            tag_table.push_back(dummy);
        }
    }*/

    void init_predictor(branchsim_conf *sim_conf);
    
    // Return the prediction ({taken/not-taken}, target-address)
    bool predict(branch *branch, branchsim_stats *sim_stats);
    
    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any allocated memory here
    ~tage();
};

#endif
