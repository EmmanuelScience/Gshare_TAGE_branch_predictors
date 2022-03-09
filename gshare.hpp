#ifndef GHSARE_H
#define GSHARE_H

#include "branchsim.hpp"
#include "Counter.hpp"

// Gshare predictor definition
class gshare : public branch_predictor_base
{

private:

    // TODO: add your fields here
    

public:
    uint64_t index, ghr, G, hash;
    Counter *pattern_table;
    // create optional helper functions here
    void update_ghr(uint64_t i);

    void init_predictor(branchsim_conf *sim_conf);

    // Return the prediction
    bool predict(branch *branch, branchsim_stats *sim_stats);

    // Update the branch predictor state
    void update_predictor(branch *branch);

    // Cleanup any allocated memory here
    ~gshare();
};

#endif
