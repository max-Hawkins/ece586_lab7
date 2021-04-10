#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <math.h>

using namespace std;

// Display string for unknown data values. Constant string instead of #define
// so that string comparisons are viable
const string UNKOWN_STR = "xxx";

/* 
    Memory Operation structure that the simulator will execute.
    Parsed in from the input file.
Members:
    Character op_type: Memory operation type. 'R' = Read and 'W' = Write
    Integer mem_address: The numeric memory address to either read or write
*/
struct memOp{
    char op_type;
    int  mem_address;
    int  mem_block;
    string  tag;
    int  cache_set;
    int  cache_block_start;
    string result;
};

struct cacheBlock{
    bool valid;
    bool dirty;
    string tag;
    int data;
    int cache_set;
};

/*
    Memory Simulator Class

Used to store all necessary information for the simulation
of a memory hierarchy and memory operations.



*/
class MemorySim{
public:
    int size_main_mem;
    int size_cache;
    int size_line;
    int assoc_deg;
    char replace_policy;
    string input_filename;
    int num_tag_bits;

    MemorySim(){}

    void display(){
        cout << "\n--- Memory Sim ---" ;
        cout << "\nMain Mem size: " << size_main_mem;
        cout << "\nCache size   : " << size_cache;
        cout << "\nLine size    : " << size_line;
        cout << "\nDegree of ass: " << assoc_deg;
        cout << "\nReplace Pol  : " << replace_policy;
        cout << "\nInput File   : " << input_filename;

        cout << endl;
    }

    void calc_mem_addr_layout(){
        cout << "\nSimulator Output:" << endl;
        // Calculate number of address, convert to integer, and print to screen
        int num_addr_lines = (int)log2(size_main_mem);
        cout << "Total address lines required = " << num_addr_lines << endl;
        // Calculate number of offset bits, convert to integer, and print to screen
        int num_offset_bits = (int)log2(size_line);
        cout << "Number of bits for offset = " << num_offset_bits << endl;
        // Calculate number of index bits, convert to integer, and print to screen
        int num_index_bits = (int)log2(size_cache / size_line / assoc_deg);
        cout << "Number of bits for index = " << num_index_bits << endl;
        // Calculate number of tag bits and print to screen
        num_tag_bits = num_addr_lines - num_offset_bits - num_index_bits;
        cout << "Number of bits for tag = " << num_tag_bits << endl;
        // Calculate cache size required and print to screen
        // num_cache_blks * (1+1(dirty and valid bits) + num_tag_bits + 8 * block_size) / 8
        int size_total_cache = (int)(size_cache / size_line * (1 + 1 + num_tag_bits) / 8 + size_cache);
        cout << "Total cache size required = " << size_total_cache << " bytes" << endl;
    }

    void exec_ops(memOp ops[], int num_ops){
        cout << "Executing operations" << endl;

        for(int i=0; i < num_ops; i++){
            
        }
    }
};

void display_ops(memOp ops[], int num_ops, int assoc_deg){
    // String variable to store each memory addresses potential cache memory blocks
    string cache_blocks = "";
    cout << "\n\nmain memory address\tmm blk #\tcm set #\tcm blk #\thit/miss" << endl;
    cout << "-----------------------------------------------------------------------------------" << endl;
    for(int i=0; i < num_ops; i++){
        cache_blocks = to_string(ops[i].cache_block_start);
        for(int block=1; block < assoc_deg; block++){
            cache_blocks.append(" or " + to_string(ops[i].cache_set + block));
        }
        printf("%8d %20d %15d %17s %14s\n", ops[i].mem_address, ops[i].mem_block, ops[i].cache_set, cache_blocks.c_str(), ops[i].result.c_str());
    }

    
}

string parse_tag(int mem_block, int num_address_lines, int num_tag_bits){
    string tag_string = "";

    return "420";
}

void display_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits){
    string data_string;
    cout << "\n\nFinal \"status\" of the cache:" << endl;
    cout << "Cache blk #\tdirty bit\tvalid bit\ttag\t\tData" << endl;
    cout << "--------------------------------------------------------------------------------------" << endl;
    for(int i=0; i < num_cache_blocks; i++){
        data_string = (cache_blocks[i].data < 0) ? "xxx" : to_string(cache_blocks[i].data);
        printf("%7d %14d %15d %12s %18s\n", i, cache_blocks[i].dirty, cache_blocks[i].valid, cache_blocks[i].tag.c_str(), data_string.c_str());
    }
}

void init_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits){
    for(int i=0; i < num_cache_blocks; i++){
        cache_blocks[i].dirty = false;
        cache_blocks[i].valid = false;
        cache_blocks[i].tag   = "xxxt";
        cache_blocks[i].data  = -1;
    }
}

void calc_hit_rates(memOp ops[], int num_ops){
    int num_hits = 0;
    set<int> blocks_seen;

    for(int i=0; i < num_ops; i++){
        if(blocks_seen.find(ops[i].mem_block) != blocks_seen.end()){
            printf("Block %d found.\n", ops[i].mem_block);
            num_hits++;
        } 
        else{
            printf("Block %d not found!\n", ops[i].mem_block);
            blocks_seen.insert(ops[i].mem_block);
        }
    }

    // TODO Calculate highest and actual hit rate!!!
    printf("\nHighest possible hit rate = \n");
    printf("Actual hit rate = \n");
}

int main(int argc, char *argv[]) {

    char cont_input;

    while(1){

        MemorySim mem_sim = MemorySim();

        cout << "Enter the size of main memory in bytes: ";
        cin >> mem_sim.size_main_mem;
        cout << "Enter the size of the cache in bytes: ";
        cin >> mem_sim.size_cache;
        cout << "Enter the block/line size: ";
        cin >> mem_sim.size_line;
        cout << "Enter the degree of set-associativity (input n for an n-way set-associative mapping): ";
        cin >> mem_sim.assoc_deg;
        cout << "Enter the replacement policy (L = LRU, F = FIFO): ";
        cin >> mem_sim.replace_policy;
        cout << "Enter the name of the input file containing the list of memory references generated by the CPU:";
        cin >> mem_sim.input_filename;

        int num_cache_blocks = mem_sim.size_cache / mem_sim.size_line;
        cacheBlock cache_blocks[num_cache_blocks];
        init_cache(cache_blocks, num_cache_blocks, mem_sim.num_tag_bits);

        mem_sim.display();

        //----------------------------
        // Parse memory operation file
        //----------------------------
        std::ifstream input_stream;
        int num_mem_ops;
        input_stream.open(mem_sim.input_filename);
        input_stream >> num_mem_ops;
        memOp operations[num_mem_ops];
        

        // Ignore empty line in input file
        string s;
        getline(input_stream, s);
        char op_char;
        int mem_loc;
        int num_addr_lines = (int)log2(mem_sim.size_main_mem);
        // Parse input memory operations into operations array
        for(int i=0; i<num_mem_ops; i++){
            input_stream >> operations[i].op_type;
            input_stream >> operations[i].mem_address;
            operations[i].mem_block = floor(operations[i].mem_address / mem_sim.size_line);
            operations[i].cache_set = operations[i].mem_block % (mem_sim.size_cache / mem_sim.size_line / mem_sim.assoc_deg);
            operations[i].cache_block_start = operations[i].cache_set * mem_sim.assoc_deg;
            operations[i].result = "TBD";
            operations[i].tag = parse_tag(operations[i].mem_block, num_addr_lines, mem_sim.num_tag_bits);
        }

        mem_sim.calc_mem_addr_layout();

        // TODO: Execute operations
        mem_sim.exec_ops(operations, num_mem_ops);

        calc_hit_rates(operations, num_mem_ops);

        // Print table of memory operations and associated information
        display_ops(operations, num_mem_ops, mem_sim.assoc_deg);

        // Display final cache status
        display_cache(cache_blocks, num_cache_blocks, mem_sim.num_tag_bits);

        // Check if user wants to run another memory simulator
        cout << "Continue? (y = yes, n = no): ";
        cin  >> cont_input;
        // Only continue if input is 'y'since don't need to error check
        // If the input isn't y, default to ending program
        if(cont_input != 'y')
            return 0; // End program
    }
    //return 0;
}