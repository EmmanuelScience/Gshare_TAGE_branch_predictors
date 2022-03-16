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
uint64_t N;
enum {SN, WN, WT, ST};
uint64_t *hist_lengt;
//int i = 0;
int k = 1;
int min = 0;
int max = 1;
FILE *file;
// Don't modify this line -- Its to make the compiler happy
branch_predictor_base::~branch_predictor_base() {}


// ******* Student Code starts here *******

// Gshare Branch Predictor

void gshare::init_predictor(branchsim_conf *sim_conf)
{
    // TODO: implement me
    this->G = sim_conf->S - 1;
    this->hash = sim_conf->P;
    N = sim_conf->N;
    uint64_t size = (uint64_t) pow(2.0, (double) this->G);
    this->pattern_table = new Counter[size];
    for(uint64_t i = 0; i < size; i++){
        pattern_table[i].state = WT;
        pattern_table[i].width = 2;
    }
    
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
    sim_stats->total_instructions = branch->inst_num;
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
    /*if(i < 10){
        std::cout << std::hex << this->ghr << std::endl;        
    }
    i++;*/
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
/**
 * @brief Get the partial tag 
 * 
 * @param GHR 
 * @param E 
 * @param T 
 * @param PC 
 * @return uint64_t 
 */
uint64_t get_partial_tag(uint64_t GHR, uint64_t E, uint64_t T, uint64_t PC){
    uint64_t pos = 2 + E, mask = 1, num_bits = E + T;
    uint64_t pc_part = ((( mask << num_bits)- mask ) & (PC >> (pos )));
    pos = 0;
    uint64_t ghr_part = ((( mask << T)- mask ) & (GHR >> (pos )));
    uint64_t xor_part = pc_part ^ ghr_part;
    //TODO: check this
    return ((( mask << T)- mask ) & (xor_part >> (pos )));
}
/**
 * @brief Get the tag index 
 * 
 * @param E 
 * @param PC 
 * @param compressed_hist 
 * @return uint64_t 
 */
uint64_t get_tag_index(uint64_t E, uint64_t PC, uint64_t compressed_hist){
    uint64_t mask = 1;
    uint64_t pos = 2;
    uint64_t pc_part = ((( mask << E)- mask ) & (PC >> (pos )));
    uint64_t xor_part = pc_part ^ compressed_hist;
    pos = 0;
    uint64_t m = (mask << E) -mask;
    return m & xor_part;
}

uint64_t get_t0_index(uint64_t H, uint64_t PC){
    uint64_t mask = 1;
    uint64_t pos = 2;
    uint64_t index = ((( mask << H)- mask ) & (PC >> (pos )));
    pos = 0;
    return ((( mask << H)- mask ) & (index >> (pos )));
}

void initialize_hist_lengths(uint64_t l, uint64_t E){
    hist_lengt = (uint64_t*)malloc(l*sizeof(uint64_t));
    for ( uint64_t i = 0; i < l; i++ ){
        if (i == 0){
            hist_lengt[i] = floor(E/2.0 +0.5);
        }
        else{
            hist_lengt[i] = floor(1.75 * hist_lengt[i-1] + 0.5);
        }
    }
}
tag_table_data *tag_table;


void tage::init_predictor(branchsim_conf *sim_conf) {
    // TODO: implement me
    N = sim_conf->N;
    //initializing t0
    file = fopen("myout.txt", "w+");
    this->H = sim_conf->S - 4;
    uint64_t size = (uint64_t) pow(2.0, (double) this->H);
    this->T0 = new Counter[size];
    this->P = sim_conf->P;
    for(uint64_t i = 0; i < size; i++){
        T0[i].state = WT;
        T0[i].width = 2;
    }

    //tag and tag table stuff
    //just create a matrix like array where the tables are contiguos
    // and to access the ith table do num_entries * i
    
    this->T = sim_conf->P + 4;
    uint64_t size_tag_tables = (pow(2, sim_conf->S) - pow(2, this->H));
    uint64_t size_entry = 2 + 3 + this->T;
    uint64_t size_tag_table = size_tag_tables / sim_conf->P;
    this->E = floor(log2(size_tag_table) - log2(size_entry));
    this->num_entries = pow(2, this->E);
    uint64_t s = num_entries * this->P;
    //initialize tag table entries
    //this->allocate(s);
    tag_table = (tag_table_data*)malloc(s*sizeof(tag_table_data));
    //this->tag_table.reserve(s*sizeof(tag_table_data));
    //this->tag_table = new tag_table_data[num_entries * sim_conf->P];
   // std::cout<<" size of tag tables: "<<sizeof(tag_table)/sizeof(tag_table_data)<<"\n";
    //std::cout<<" expected size of tag tables: "<<s<<"\n";
   // std::cout<<"num_entries: "<<num_entries<<"\n";

    
    
    
    //std::cout<<"stuff";
    for (uint64_t i = 0; i < (num_entries * sim_conf->P); i++ ){
        tag_table[i].tag = 0;
        tag_table[i].pred_counter.width = 3;
        tag_table[i].pred_counter.state = 0;
        tag_table[i].useful_counter.state = 0;
    }
    
    //initialize hist length
    initialize_hist_lengths(sim_conf->P, this->E);
    this->GHR = new TAGE_GHR{hist_lengt[this->P-1]};
}

bool tage::predict(branch *branch, branchsim_stats *sim_stats)
{
    // TODO: implement me
    //Getting prediction from t0
    bool pred;
    sim_stats->num_branch_instructions++;
    sim_stats->total_instructions = branch->inst_num;
    this->index_t0 = get_t0_index(this->H, branch->ip);
    if (this->T0[this->index_t0].tag != branch->ip){
        this->T0[this->index_t0].tag = branch->ip;
    }
    pred = this->T0[this->index_t0].state > 1;

    /*getting prediction from tags*/
    bool used_tagged_tables = false; // if a tagged table has been used or not
    has_found_lmb = false; // used to check if we have the longest matching branch already
    //gets the partial tag from the ghr
    uint64_t inx;
    uint64_t temp1;
    uint64_t partial_tag = get_partial_tag(this->GHR[0].getHistory(), this->E, this->T, branch->ip);
    for(int i = this->P-1; i >= 0; i--){
        uint64_t length = hist_lengt[i]; // gets the length 
        uint64_t compresed_hist = this->GHR[0].getCompressedHistory(length, this->E);
        // index used to access the tag tables
        uint64_t tag_index = get_tag_index(this->E, branch->ip, compresed_hist); 
       /* if (((i * num_entries) + tag_index) > 384){
            std::cout<<" tagged table error index: "<<tag_index<<" num_entries: "<<num_entries<<"\n";
            printf(" i : %lu, tag_index: %lu, this.p: %lu\n", i, tag_index, this->P-1);
            exit(0);
            return false;
        }*/
        
        if (tag_table[(i * num_entries) + tag_index].tag == partial_tag ){
            if (!has_found_lmb){
                this->longest_matching_branch = (i * num_entries) + tag_index;
                this->tag_table_pos = i;
                this->longest_index = tag_index;
                has_found_lmb = true;
                if(k < max){
                    printf("found it----------------------*******************************************\n");
                }
            }
            if (!(tag_table[(i * num_entries) + tag_index].pred_counter.isWeak() 
                    && tag_table[(i * num_entries) + tag_index].useful_counter.state == 0)){
                        pred = tag_table[(i * num_entries) + tag_index].pred_counter.isTaken();
                        used_tagged_tables = true;
                        inx = i;
                        this->tag_table_index = (i * num_entries) + tag_index;
                        temp1 = tag_index;
                        break; //yeeeeeeeet
            }
        }
    }
    //increment the tag conflicts if the tag table is not used
    if(!used_tagged_tables){
       sim_stats->num_tag_conflicts++; 
       if(k < max){
        std::cout << std::hex <<"ip: " <<branch->ip<<" hit in table0 , index: "<<index_t0<<" prediction: "
                    <<this->T0[this->index_t0].state << " ghr: "<<this->GHR[0].getHistory();
                    printf(" branch num : %d\n", k);
                    //std::cout <<" i: "<<k<< std::endl;
                   // fprintf(file, "ip: %lu, index: %lu, pred: %d , branch %d\n", branch->ip,index_t0,
                    //this->T0[this->index_t0].state, k    );
       }
            
    } 
    
    else{
         if(k < max){
             std::cout << std::hex <<"ip: " <<branch->ip<<" hit in table"<<inx+1<< ", index: "<<temp1<<" prediction: "
            <<tag_table[tag_table_index].pred_counter.state<<" partial tag: "<<tag_table[tag_table_index].tag <<
             " ghr: "<<this->GHR[0].getHistory();
             printf(" branch num : %d\n", k);
            // std::cout <<" i: "<<k<< std::endl;
         }
         if (inx==0)
         fprintf(file, "ip: %lu, index: %lu, pred: %d , branch %d\n", branch->ip,temp1,
                    tag_table[tag_table_index].pred_counter.state, k    );
            
        //printf("\tIP: 0x%" PRIx64 ", Hit in Table T0, index: 0x%" PRIx64 ", prediction: 0x%x (Taken)\n", branch->ip, index_t0, pred);
       //k++;
    }
    this->used_tag_table = used_tagged_tables;
    
    this->pred = pred;
    return pred;
}


void tage::update_predictor(branch *branch) {
    // TODO: implement me
    bool found_0 = false;
    if (has_found_lmb){
        
        if ((tag_table[longest_matching_branch].pred_counter.isTaken()  && branch->is_taken) 
        || (!tag_table[longest_matching_branch].pred_counter.isTaken()  && !branch->is_taken) ){
            tag_table[longest_matching_branch].useful_counter.update(true);
        }
        else{
            tag_table[longest_matching_branch].useful_counter.update(false);
        }
        tag_table[longest_matching_branch].pred_counter.update(branch->is_taken);
        if (k < max){
           std::cout<<std::hex<<" updating t"<<tag_table_pos+1
            <<" index: "<<longest_index<<" useful counter "
            <<tag_table[longest_matching_branch].useful_counter.get()
            <<" new pred counter: "<<tag_table[longest_matching_branch].pred_counter.get();
            printf(" branch num : %d\n", k); 
        }
        
        if (tag_table[longest_matching_branch].pred_counter.isTaken() == branch->is_taken){
            this->GHR[0].shiftLeft(branch->is_taken);
            k++;
            return;
        }
        
    }
    else {
        /*updates the t0 if its the only on with a hit*/
        this->T0[this->index_t0].update(branch->is_taken);
        //check if new prediction match the actual prediction
        if(k < max){
            std::cout<<std::hex<<" updating t0"
                        <<" index: "<<index_t0
                        <<" new pred counter: "<<this->T0[this->index_t0].get();
            printf(" branch num : %d\n", k);
        }
        
        if((this->T0[this->index_t0].isTaken() && branch->is_taken) || (!this->T0[this->index_t0].isTaken() && !branch->is_taken)){
            this->GHR[0].shiftLeft(branch->is_taken);
            k++;
            return; //yeeeeeeeeeeeeeeeeeeeeeet
        }
        else{
            if (k < max){
                printf("-------------------------------------------------------------------------------------------\n");
            }
        }
        
    }
    
    /* allocating New entries */
    
    uint64_t ii;
    uint64_t index_to_allocate;
    uint64_t partial_tag = get_partial_tag(this->GHR[0].getHistory(), this->E, this->T, branch->ip);
    uint64_t iter;
    uint64_t temp;
    if (used_tag_table){
        iter = tag_table_pos+1;
    }
    else{
        iter =0;
    }
    for(uint64_t i = iter; i < this->P; i++){
        uint64_t length = hist_lengt[i]; // gets the length 
        uint64_t compresed_hist = this->GHR[0].getCompressedHistory(length, this->E);
        // index used to access the tag tables
        uint64_t tag_index = get_tag_index(this->E, branch->ip, compresed_hist); 
        if (tag_index > num_entries){
            std::cout<<"tagged table error\n";
            k++;
            return;
        }      
        if (tag_table[(i * num_entries) + tag_index].useful_counter.get() == 0 ){
            index_to_allocate = (i * num_entries) + tag_index;
            temp = tag_index;
            ii = i;
            found_0 = true;
            break;
        }
    }

    // if we find a useful counter with zero
    if (found_0){
        tag_table[index_to_allocate].tag = partial_tag;
        tag_table[index_to_allocate].useful_counter.setCount(0);
        tag_table[index_to_allocate].pred_counter.reset(branch->is_taken);
        //Allocating entry in table T2, index: 0x7b, new partial tag: 0xe, useful counter: 0x0, prediction counter: 0x4
        if (k < max){
            std::cout<<std::hex<<" allocating in table --------------------------------------------------"
            <<ii+1<<" index: "<<temp<<" partial tag "<<partial_tag<<" pred: "<<tag_table[index_to_allocate].pred_counter.state;
        printf(" branch num : %d\n", k);
        }
        

    }
    else{
        for(uint64_t i = iter; i < this->P; i++){
            uint64_t length = hist_lengt[i]; // gets the length 
            uint64_t compresed_hist = this->GHR[0].getCompressedHistory(length, this->E);
            // index used to access the tag tables
            uint64_t tag_index = get_tag_index(this->E, branch->ip, compresed_hist); 
            if (tag_index > num_entries){
                std::cout<<"tagged table error\n";
                k++;
                return;
            }      
            tag_table[(i * num_entries) + tag_index].useful_counter.update(false);
        }
    }
    this->GHR[0].shiftLeft(branch->is_taken);
    k++;
    return;
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
    sim_stats->prediction_accuracy = 1.0 * sim_stats->num_branches_correctly_predicted/sim_stats->num_branch_instructions;
    sim_stats->fraction_branch_instructions = 1.0 * sim_stats->num_branch_instructions/sim_stats->total_instructions;
    sim_stats->misses_per_kilo_instructions = sim_stats->num_branches_mispredicted/(sim_stats->total_instructions/1000) ;
    
    if(N <= 7){
        sim_stats->stalls_per_mispredicted_branch = 2;
    }
    else{
        sim_stats->stalls_per_mispredicted_branch = (N/2)-1;
    }
    sim_stats->average_CPI = 1 + 1.0 * sim_stats->stalls_per_mispredicted_branch * ((1.0 * sim_stats->num_branches_mispredicted)/(1.0 * sim_stats->total_instructions));

    free(tag_table);
    
}
