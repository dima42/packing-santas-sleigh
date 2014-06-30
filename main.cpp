#define MAINFILE

#include <ctime>
#include <iostream>
#include <string>
#include "MetaPresent.h"
#include "Sleigh.h"

int main(){
    std::time_t start = time(0);
    MetaPresentV presents("presents.csv", 1000000);
    Sleigh s(1000, 1000);
    s.packPresents(presents);
    std::cout << s.maxHeight()*2 << " " << (long)(time(0)-start) << std::endl;
    s.generateSubmission("result16smalllookaheadpointoh2.csv");
    return 0;
}
