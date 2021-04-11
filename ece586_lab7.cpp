/*  Max Hawkins
    CWID: 11789594
    ECE-487: Lab 7
    April 13th, 2021
*/
// Include various libraries for use in program
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
    Integer mem_address: The numeric main memory address to either read or write
    Integer mem_block: The numeric main memory block
    String tag: The string representation of the memory address tag
    Integer cache_set: The cache set the main memory block is associated with
    Integer cache_block_start: The starting block cache of the cache set
    String result: The result of the memory operation "hit" or "miss"
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

/*
    Cache block structure for use in keeping track of cache status

Members:
    Boolean valid: Whether the cache block contains valid data
    Boolean dirty: Whether the cache block data is dirty
    String tag: The tag of the block's data
    Int data: The main memory block number of the cache data
    Int cache_set: The cache set associated with the cache block
*/
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

Members:
    Integer size_main_mem: The main memory size in bytes
    Integer size_cache: The cache memory size in bytes
    Integer size_line: The size of a line/block of memory
    Integer assoc_deg: The degree of association of the cache
    Character replace_policy: The system's replace policy
        "L" for least-recently-used or
        "F" for first-in-first-out
    String input_filename: The filename of the memory instructions file
    Integer num_tag_bits: The number of tag bits in the memory addresses
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

    MemorySim(){} // Default constructor

    /**************************************************************************************************************
    Function name:         calc_mem_addr_layout()
    Input parameters:      None
    Return value:          Void - Returns nothing
    Purpose:               Calculates and displays various memory simulator parameters necessary for operation
    **************************************************************************************************************/
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
        cout << "Total cache size required = " << size_total_cache << " bytes";
    }

    /**************************************************************************************************************
    Function name:         exec_ops(memOp ops[], int num_ops, cacheBlock cache_blocks[], int num_cache_blocks)
    Input parameters:      memOp ops[]: The array of memory operations to be executed
                           integer num_ops: The number of memory operations
                           cacheBlock cache_blocks[]: The array of cache blocks emulating cache memory
                           integer num_cache_blocks: The number of cache blocks in the system cache
    Return value:          Void - Returns nothing
    Purpose:               Runs the memory operations with the given cache memory and simulator parameters
    **************************************************************************************************************/
    void exec_ops(memOp ops[], int num_ops, cacheBlock cache_blocks[], int num_cache_blocks){
        bool tag_found; // Boolean flag of whether or not a tag match was found
        int cache_block_idx; // Variable to store the current cache block index to be searched
        //cout << "Executing operations" << endl;
        // Iterate through all memory operations
        for(int i=0; i < num_ops; i++){
            //printf("\nMem op: %d, address: %d,  tag: %s\n", i, ops[i].mem_address, ops[i].tag.c_str());

            // Initialize the found status to false so that default is not found
            tag_found = false;
            // Search through memory block's associated cache blocks in its cache set
            for(int cache_block_offset=0; cache_block_offset < assoc_deg; cache_block_offset++){
                // Add cache set offset to global cache offset
                cache_block_idx = ops[i].cache_block_start + cache_block_offset;
                //printf("Cache Block: %d,  Cache Tag: %s\n", cache_block_idx, cache_blocks[cache_block_idx].tag.c_str());

                // If the main memory address tag and cache tag match
                if(ops[i].tag == cache_blocks[cache_block_idx].tag){
                    //printf("Tag match found!\n");
                    tag_found = true; // Set found flag to true
                    ops[i].result = "hit"; // Set operation result as hit

                    // If the operation was a write
                    if(ops[i].op_type == 'W' || ops[i].op_type == 'w'){
                        //printf("Writing dirty bit!\n");
                        // Set cache block dirty flag to true
                        cache_blocks[cache_block_idx].dirty = true;
                    }
                    break; // Break out of cache block search loop
                }
            }

            // If the desired main memory tag was not found in cache
            // Find LRU or FIFO block to overwrite. => Miss
            if(!tag_found){
                int cache_block_to_edit = -1; // Initialize cache block to edit to impossible value
                ops[i].result = "miss"; // Set operation result to miss

                // Check to see if the cache set has a block that has not
                // been written to this simulator execution run.
                // Did not want to include this in above loop just incase there
                // could be some edgecase where blocks would not be filled sequentially
                for(int cache_block_offset=0; cache_block_offset < assoc_deg; cache_block_offset++){
                    // Add cache set offset to global cache offset
                    cache_block_idx = ops[i].cache_block_start + cache_block_offset;
                    // If the cache block is unyet written to, make that block the one to operate on
                    if(cache_blocks[cache_block_idx].tag.at(0) == 'x'){
                        cache_block_to_edit = cache_block_idx;
                        //printf("Empty cache block found: %d", cache_block_idx);
                        break; // Break out of cache block search loop
                    }
                }

                // If no 'empty' cache blocks were found, use replacement policy as given by user
                if(cache_block_to_edit < 0){
                    bool cache_repl_block_found = false; // Initialize found status to false
                    int cur_search_main_block; // Store current main memory block searched for

                    // If using least recently used replacement policy
                    if(replace_policy == 'L' || replace_policy == 'l'){
                        bool cache_block_age_found; // Variable to store whether or not the cache block age found
                        int cache_blocks_age[assoc_deg]; // Array of cache set blocks ages
                        // Iterate through current cache set blocks
                        for(int cache_block_offset=0; cache_block_offset < assoc_deg; cache_block_offset++){
                            // Initialize that the current cache block of interest's age has not been found
                            cache_block_age_found = false;
                            // Add cache set offset to global cache offset
                            cache_block_idx = ops[i].cache_block_start + cache_block_offset;

                            // Iterate backwards through memory operations starting at current operation
                            for(int search_op=i; search_op > 0; search_op--){
                                //printf("LRU Search op idx: %d", search_op);
                                // If the historical operated data block and the current cache block
                                // searched for match
                                if(ops[search_op].mem_block == cache_blocks[cache_block_idx].data){
                                    // Set the associated cache block age to the difference in operation index
                                    // of the current executed operation and the operation the main memory block
                                    // was last operated on
                                    cache_blocks_age[cache_block_offset] = i - search_op;
                                    // Break out of search operation loop
                                    break;
                                }
                                
                            }
                        } // End cache block search loop
                        
                        int max_age = cache_blocks_age[0]; // Initialize max age to first cache block age
                        int max_age_cache_offset = 0; // Initialize max age index to 0
                        // Iterate through cache block ages to find the index (cache set offset)
                        // of the maximum element of the cache block age
                        for(int idx=1; idx < assoc_deg; idx++){
                            // If the current block's age is greater than max age
                            if(cache_blocks_age[idx] > max_age){
                                // Set max age to current block's age
                                max_age = cache_blocks_age[idx];
                                // Set max age index to current index
                                max_age_cache_offset = idx;
                            }
                        }
                        //printf("LRU min age cache offset: %d,  age: %d", max_age_cache_offset, max_age);

                        // Set cache block to overwrite to the cache block with index of mad age
                        cache_block_to_edit = ops[i].cache_block_start + max_age_cache_offset;
                    }

                    // If using first-in-first-out replacement policy
                    else if(replace_policy == 'F' || replace_policy == 'f'){
                        // Iterate forwards through memory operations 
                        for(int search_op=0; search_op < num_ops; search_op++){
                            //printf("FIFO Search op idx: %d", search_op);
                            // For every search operation in history, see if the main memory
                            // block is in the cache set of interest
                            cur_search_main_block = ops[search_op].mem_block;
                            for(int cache_block_offset=0; cache_block_offset < assoc_deg; cache_block_offset++){
                                // Add cache set offset to global cache offset
                                cache_block_idx = ops[i].cache_block_start + cache_block_offset;
                                // If the cache set contains the historical main memory block operated on of interest
                                if(cache_blocks[cache_block_idx].data == cur_search_main_block){
                                    cache_repl_block_found = true; // Set replace block found
                                    cache_block_to_edit = cache_block_idx; // Set replace index to current index
                                    //printf("FIFO cache block found: %d", cache_block_idx);
                                    break; // Break out cache block search
                                }
                            }
                            // If a cache block to replace has been found, exit search for loop
                            if(cache_repl_block_found)
                                break;
                        }
                    }
                    // If invalid replacement policy - should NOT happen since in 487
                    else{
                        printf("---Invalid replacement policy!---\n");
                    }
                }

                // Execute memory operation on given block (empty or otherwise)

                // Set cache block to valid
                cache_blocks[cache_block_to_edit].valid = true; 
                // Set cache block tag to main memory address tag
                cache_blocks[cache_block_to_edit].tag   = ops[i].tag; 
                // Set cache block data to main memory block number
                cache_blocks[cache_block_to_edit].data  = ops[i].mem_block; 
                // If operation is a write
                if(ops[i].op_type == 'W' || ops[i].op_type == 'w'){
                    //printf("Writing dirty bit!\n");
                    // Set cache block dirty flag to true
                    cache_blocks[cache_block_idx].dirty = true;
                }// Else dirty bit is false (read operation)
                else{
                    cache_blocks[cache_block_to_edit].dirty = false;
                }
            }            
        }
    }
};

/**************************************************************************************************************
    Function name:         display_ops(memOp ops[], int num_ops, int assoc_deg)
    Input parameters:      memOp ops[]: The array of memory operations to be executed
                           integer num_ops: The number of memory operations
                           integer assoc_deg: The cache set association degree
    Return value:          Void - Returns nothing
    Purpose:               Displays the memory operations' information and result to the user
**************************************************************************************************************/
void display_ops(memOp ops[], int num_ops, int assoc_deg){
    // String variable to store each memory addresses potential cache memory blocks
    string cache_blocks;
    // Display memory operation table header
    cout << "\n\nmain memory address\tmm blk #\tcm set #\tcm blk #\thit/miss" << endl;
    cout << "-----------------------------------------------------------------------------------" << endl;
    // Iterate through memory operations
    for(int i=0; i < num_ops; i++){
        // Initialize cache blocks display string to the starting cache set block number
        cache_blocks = to_string(ops[i].cache_block_start);
        // If the degree of association is 2, use 'or' when displaying cache blocks
        if(assoc_deg == 2){
            cache_blocks.append(" or " + to_string(ops[i].cache_block_start + assoc_deg - 1));
        // Else if association degree is greater than 2, use '-' (scales better)
        }else if(assoc_deg > 2){
            cache_blocks.append(" - " + to_string(ops[i].cache_block_start + assoc_deg - 1));
        }
        // Print formatted display of memory operation information
        printf("%8d %20d %15d %17s %14s\n", ops[i].mem_address, ops[i].mem_block, ops[i].cache_set, cache_blocks.c_str(), ops[i].result.c_str());
    }
}
/**************************************************************************************************************
Function name:         parse_tag(int mem_address, int num_address_lines, int num_tag_bits)
Input parameters:      Integer mem_address: The main memory address as an integer
                       Integer num_address_lines: The number of system address lines/bits
                       Integer num_tag_bits: The number of tag bits in a main memory address
Return value:          String tag_string: The tag string representation
Purpose:               Creates and returns the string representation of the main memory address tag
**************************************************************************************************************/
string parse_tag(int mem_address, int num_address_lines, int num_tag_bits){
    string tag_string = ""; // Variable to store string representation of tag
    // Shift the main memory address bits right by the number of
    // offset and index bits
    mem_address = mem_address >> (num_address_lines - num_tag_bits);
    // Iterate through the tag bits of the main memory address
    for(int bit=1; bit <= num_tag_bits; bit++){
        // If the current tag bit of interest (masked with 1)
        // is 1
        if((mem_address & 0x1) == 1){
            tag_string.insert(0, "1"); // Add to "1" tag string
        }else{ // Else if current tag bit is 0, add "0" to tag string
            tag_string.insert(0, "0");
        }
        // Shift the main memory address bits right by 1 digit
        mem_address = mem_address >> 1;
    }
    // Return the tag string representation
    return tag_string;
}
/**************************************************************************************************************
Function name:         display_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits)
Input parameters:      cacheBlock cache_blocks[]: The array of cache blocks emulating cache memory
                       Integer num_cache_blocks: The number of cache blocks in the system cache
                       Integer num_tag_bits: The number of tag bits in a main memory address
Return value:          void - Returns nothing
Purpose:               Displays the status of the system cache
**************************************************************************************************************/
void display_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits){
    string data_string; // Variable to store the string representation of the cache data
    cout << "\n\nFinal \"status\" of the cache:" << endl;
    // Display cache table header
    cout << "Cache blk #\tdirty bit\tvalid bit\ttag\t\tData" << endl;
    cout << "------------------------------------------------------------------------------" << endl;
    // Iterate through cache blocks in cache
    for(int i=0; i < num_cache_blocks; i++){
        // Create string representation of cache data
        data_string = (cache_blocks[i].data < 0) ? "xxx" : "mm blk # " + to_string(cache_blocks[i].data);
        // Display single cache block information
        printf("%7d %14d %15d %12s %16s\n", i, cache_blocks[i].dirty, cache_blocks[i].valid, cache_blocks[i].tag.c_str(), data_string.c_str());
    }
}
/**************************************************************************************************************
Function name:         init_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits)
Input parameters:      cacheBlock cache_blocks[]: The array of cache blocks emulating cache memory
                       Integer num_cache_blocks: The number of cache blocks in the system cache
                       Integer num_tag_bits: The number of tag bits in a main memory address
Return value:          void - Returns nothing
Purpose:               Initializes the system cache
**************************************************************************************************************/
void init_cache(cacheBlock cache_blocks[], int num_cache_blocks, int num_tag_bits){
    // Iterate through the cache blocks
    for(int i=0; i < num_cache_blocks; i++){
        // Initialize cache dirty and valid statuses to be false
        cache_blocks[i].dirty = false;
        cache_blocks[i].valid = false;
        // Set cache block tag to be the correct length but filled with 'x's
        cache_blocks[i].tag   = string(num_tag_bits, 'x');
        // Initialize the cache data block to be an impossible value
        cache_blocks[i].data  = -1;
    }
}
/**************************************************************************************************************
    Function name:         calc_hit_rates(memOp ops[], int num_ops)
    Input parameters:      memOp ops[]: The array of memory operations to be executed
                           integer num_ops: The number of memory operations
    Return value:          Void - Returns nothing
    Purpose:               Calculates and displays the memory operation optimum and actual hit rates
**************************************************************************************************************/
void calc_hit_rates(memOp ops[], int num_ops){
    int num_hits = 0; // Initialize the number of hits to 0
    // Create a set containing the main memory blocks yet operated on
    set<int> blocks_seen;

    // Iterate through memory operations
    for(int i=0; i < num_ops; i++){
        // If the current main memory block to be operated on has been seen before
        if(blocks_seen.find(ops[i].mem_block) != blocks_seen.end()){
            num_hits++; // Increment the number of hits
        } 
        // Else, add the curent main memory block to the seen list
        else{
            blocks_seen.insert(ops[i].mem_block);
        }
    }
    // Calculate and print the optimum hit rate
    printf("\nHighest possible hit rate = %d/%d = %2.0f%%\n", num_hits, num_ops, (float)num_hits/num_ops*100.0);

    num_hits = 0; // Reset the number of hits for actual calculation
    // Iterate through the memory operations
    for(int i=0; i < num_ops; i++){
        // If the operation resulted in a cache hit
        if(ops[i].result == "hit")
            num_hits++; // Increment the number of hits
    }
    // Calculate and print the actual cache hit rate
    printf("Actual hit rate = %d/%d = %2.0f%%\n", num_hits, num_ops, (float)num_hits/num_ops*100.0);
}

int main(int argc, char *argv[]) {
    // Character input to parse for continue operation status
    char cont_input;
    // Dummy string for dumping file empty line to
    string dummy_string;
    // Infinite operation loop
    while(1){
        // Create memory simulator to later populate with values
        MemorySim mem_sim = MemorySim();

        // Parse all user input regarding the simulator operation
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

        // Calculate any other necessary parameters about the simulator operation
        mem_sim.calc_mem_addr_layout();

        // Calculate the number of cache blocks
        int num_cache_blocks = mem_sim.size_cache / mem_sim.size_line;
        // Declare an array of cache blocks representing the system cache
        cacheBlock cache_blocks[num_cache_blocks];
        // Initialize the system cache to starting values
        init_cache(cache_blocks, num_cache_blocks, mem_sim.num_tag_bits);

        // Parse memory operation file
        std::ifstream input_stream; // Create input file stream
        int num_mem_ops; // Variable to store number of memory operations
        input_stream.open(mem_sim.input_filename); // Open memory operations file
        input_stream >> num_mem_ops; // Get number of memory operations from file
        memOp operations[num_mem_ops]; // Create array of memory operations
        
        getline(input_stream, dummy_string); // Ignore empty line in input file

        char op_char; // Variable to store temporary memory operation type
        int mem_loc; // Variable to store temporary memory address
        // Calculate and store number of address lines
        int num_addr_lines = (int)log2(mem_sim.size_main_mem);

        // Parse input memory operations into operations array
        for(int i=0; i<num_mem_ops; i++){
            input_stream >> operations[i].op_type; // Get op type from file
            input_stream >> operations[i].mem_address; // Get memory address from file
            // Calculate and set memory operation main memory block
            operations[i].mem_block = floor(operations[i].mem_address / mem_sim.size_line);
            // Calculate and set memory operation cache set
            operations[i].cache_set = operations[i].mem_block % (mem_sim.size_cache / mem_sim.size_line / mem_sim.assoc_deg);
            // Calculate and set memory operation starting cache block
            operations[i].cache_block_start = operations[i].cache_set * mem_sim.assoc_deg;
            // Initialize memory operation result to unknown string
            operations[i].result = UNKOWN_STR;
            // Set memory operation address tag
            operations[i].tag = parse_tag(operations[i].mem_address, num_addr_lines, mem_sim.num_tag_bits);
        }

        

        // Execute memory operations given the simulator setup and system cache
        mem_sim.exec_ops(operations, num_mem_ops, cache_blocks, num_cache_blocks);

        // Print table of memory operations and associated information
        display_ops(operations, num_mem_ops, mem_sim.assoc_deg);

        // Calculate and print the optimum and actual hit rates
        calc_hit_rates(operations, num_mem_ops);

        // Display final cache status
        display_cache(cache_blocks, num_cache_blocks, mem_sim.num_tag_bits);

        // Check if user wants to run another memory simulator
        cout << "Continue? (y = yes, n = no): ";
        cin  >> cont_input; // Parse user input
        // Only continue if input is 'y'since don't need to error check
        // If the input isn't y, default to ending program
        if(cont_input != 'y')
            return 0; // End program
    }
    return 0; // End program operation
}