#include "hanport_data.hpp"
#include <iostream>



int main(int argc, char** argv){
    //call constructor
    HanportData data_obj("./exampledata/meterdata.bin");

    std::cout << "data_obj instatiated";

    return 0;

}