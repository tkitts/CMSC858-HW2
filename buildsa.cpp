#include "libsais-master/src/libsais.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "bitsery/bitsery.h"
#include "bitsery/adapter/stream.h"
#include <bitsery/traits/string.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/ext/std_map.h>
#include <bitsery/ext/std_tuple.h>
#include <math.h>
#include "saStruct.cpp"

using namespace std;
//run
int main(int argc, char * argv[]){
    string arg1 = argv[1];
    char *reference;
    char *output;
    string k = "0";
    if(arg1 == "--preftab"){
        k = argv[2];
        reference = argv[3];
        output = argv[4];
    }
    else{
        reference = argv[1];
        output = argv[2];
    }
    clock_t now = clock();
    buildsa oldBuild =  buildsa();
    buildsa newBuild =  buildsa();
    oldBuild.build(stoi(k), reference);
    //write to binary file
    std::ofstream file;
    file.open(output);
    bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{file};
    ser.object(oldBuild);
    //flush to writer
    ser.adapter().flush();
    file.close();
    cout << to_string((float)(clock()-now)/CLOCKS_PER_SEC);
}