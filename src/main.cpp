#include "hanport_data.hpp"
#include <iostream>
#include <cstdlib>

#ifndef EX_DATA_PATH
#define EX_DATA_PATH "ex_data"
#endif
int main(/*int argc, char** argv*/){
    //call constructor
    //const char* path_to_testfile = getenv("example_data");
    try{
        HanportData data_obj(EX_DATA_PATH);
        //compare transmitted crc with calculated crc
    }
    catch (const std::exception& e){
        std::cerr << "\nx Error " << e.what() << "\n";
        return 1;
    }
    return 0;
}
