#include "libsais-master/src/libsais.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include "bitsery/bitsery.h"
#include "bitsery/adapter/buffer.h"
#include <bitsery/traits/string.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/traits/vector.h>
#include <math.h>
#include <map>
#include "saStruct.cpp"

using namespace std;
using Buffer = std::vector<uint8_t>;
using OutputAdapter = bitsery::OutputBufferAdapter<Buffer>;
using InputAdapter = bitsery::InputBufferAdapter<Buffer>;
using bitsery::ext::PointerOwner;
using bitsery::ext::PointerType;

//returns number of characters shared and
// - if prefix less than SA target, 0 if equal, + if greater than
tuple<int,int> isPrefixOf(string &prefix, int index, string &inputText, int skip){
    int shared=skip;
    int comparison;
    //return make_tuple(0,prefix.compare(inputText.substr(index+shared,prefix.length())));
    while(prefix.at(shared) == inputText.at(index+shared)){
        shared++;
        //if we've run out of characters for either string
        if (shared >= prefix.size()){
            return make_tuple(shared, 0);
        }
        if(index+shared >= inputText.size()){
            return make_tuple(shared, 1);
        }
    }
    //after that loop shared will be the index of the differing character or we've run out of characters
    if(prefix.at(shared) < inputText.at(index+shared)){
        return make_tuple(shared,-1);
    }
    else{
        return make_tuple(shared,1);
    }
}
tuple<int,int> findRange(int l, int m, int r, string &queryString, buildsa &SA){
    int newL = l;
    int newR = m;
    int finalL;
    int finalR;
    int newM;
    //find left
    while (newL <= newR){
        newM = (newL + (newR))/2;
        int comparison, shared;
        tie(shared,comparison) = isPrefixOf(queryString, SA.SA.at(newM), SA.text, 0);
        // Check if prefix matches
        if (comparison > 0){
            newL = newM + 1;
        }
        else if(comparison < 0){
            newR = newM - 1;
        }
        else{
            newR = newM - 1;
        }
    }
    finalL = newL;
    newL = m;
    newR = r;
    //find right
    while (newL <= newR){
        newM = (newL + (newR+1))/2;
        int comparison, shared;
        tie(shared,comparison) = isPrefixOf(queryString, SA.SA.at(newM), SA.text, 0);
        // Check if prefix matches
        if (comparison > 0){
            newL = newM + 1;
        }
        else if(comparison < 0){
            newR = newM - 1;
        }
        else{
            newL = newM + 1;
        }
    }
    finalR = newR;
    return make_tuple(finalL,finalR);
}

tuple<int,int> searchSANaive(buildsa &SA, string &queryString){
    //hit_1 through hit_k are positions in original text or the query string
    string kPrefix = "";
    int l, r;
    l = 0;
    r = SA.SA.size();
    if (!SA.prefix.empty()){
        kPrefix = queryString.substr(0,SA.kLen);
        tie(l, r) = SA.prefix.at(kPrefix);
    }

    while (l <= r){
        int m = (l + r)/2;
        int comparison, shared;
        tie(shared,comparison) = isPrefixOf(queryString, SA.SA.at(m), SA.text, 0);
        // Check if prefix matches
        if (comparison == 0){
            //if you find a match then split into two binary searches to find the range
            tie(l,r) = findRange(l,m, r, queryString, SA);
            return make_tuple(l,r);
        }
        if (comparison > 0){
            l = m + 1;
        }
        else{
            r = m - 1;
        }
    }
    return make_tuple(-1,-1);
}
//do the simple accelerant
//Binary search with some skipping using LCPs
//between P and T’s suffixes. Still O(n log m), but it
//can be argued it’s near O(n + log m) in practice.
tuple<int,int> searchSAAccel(buildsa &SA, string &queryString){
    //hit_1 through hit_k are positions in original text or the query string
    string kPrefix;
    int LCPpl=0, LCPpr = 0;
    int l, r;
    l = 0;
    r = SA.SA.size();

    if (!SA.prefix.empty()){
        kPrefix = queryString.substr(0,SA.kLen);
        tie(l, r) = SA.prefix.at(kPrefix);
    }
    while(l <= r){
        //skip the first min(LCPpl, LCPpr) characters and then compare characters
        int LCP = min(LCPpl, LCPpr);
        int m = (l + r)/2;
        int comparison, shared;
        tie(shared,comparison) =  isPrefixOf(queryString, SA.SA.at(m), SA.text, LCP);
        // Check if prefix matches
        if (comparison == 0){
            //if you find a match then split into two binary searches to find the range
            tie(l,r) = findRange(l,m, r, queryString,SA);
            return make_tuple(l,r);
        }
        if (comparison > 0){
            LCPpl = shared;
            l = m + 1;
        }
        else{
            LCPpr = shared;
            r = m - 1;
        }

    }
    return make_tuple(-1,-1);
}
void query(buildsa &SA, string &queries, string &queryMode, string &output){
    string name;
    string queryString;
    map<string, vector<int>> hits;
    float time = 0;
    //read from query input
    std::ifstream file;
    file.open(queries);
    getline(file,name,'>');
    while(getline(file,name)){
        //first line is >#:name:R
        getline(file, queryString, '>');
        queryString = queryString.substr(0,queryString.size()-1);
        hits[name] = vector<int>();
        int l,r;
        clock_t now = clock();
        if(queryMode == "naive"){
            tie(l,r) = searchSANaive(SA, queryString);
        }
        else if(queryMode == "simpaccel"){
            tie(l,r) = searchSAAccel(SA, queryString);
        }
        time += clock()-now;
        //set hits to the elements between l and r
        for(int i=l;i<=r && i >= 0;i++){
            hits[name].push_back(SA.SA.at(i));
        }
    }
    file.close();
    cout << time/1000000;

    //write to output file
    ofstream fileOut;
    fileOut.open(output);
    for(std::pair<string, vector<int>> queryhits : hits){
        string line = queryhits.first+"\t"+to_string(queryhits.second.size());
        for(int element : queryhits.second){
            line.append("\t");
            line.append(to_string(element));
        }
        line.append("\n");
        fileOut << line;
    }
    fileOut.close();
}

int main(int argc, char * argv[]){
    string index = argv[1];
    string queries = argv[2];
    string queryMode = argv[3];
    string output = argv[4];
    buildsa oldBuild =  buildsa();
    bitsery::ext::PointerLinkingContext ctx{};
    //read from binary file
    std::ifstream file;
    file.open(index);
    auto state = bitsery::quickDeserialization<bitsery::InputStreamAdapter>(file, oldBuild);
    file.close();
    query(oldBuild, queries, queryMode, output);
}