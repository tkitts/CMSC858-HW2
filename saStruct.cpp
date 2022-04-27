#include "libsais-master/src/libsais.h"
#include "libsais-master/src/libsais64.h"
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

using namespace std;
static constexpr const char alphabet[4] = {'A','C','G','T'};

 struct buildsa {
    string text;
    vector<int64_t> SA;
    int kLen;
    //unordered_map is implemented using a hashtable
    unordered_map<string, tuple<int,int>> prefix;
    buildsa(){
        text = "";
        SA = vector<int64_t>();
        prefix = unordered_map<string, tuple<int,int>>();
    }
    void build(int k, char *infile){
        //read in
        std::ifstream file;
        file.open(infile);
        //read all the other lines
        std::stringstream stream;
        int firstLine = ' ';
        //ignore the first line
        while (firstLine != '\n'){
            firstLine = file.get();
        }
        //read the rest of the file
        string a = "";
        while (file.good()){
            char input = file.get();
            input = toupper(input);
            if(input == 'n' || input == 'N'){
                input = alphabet[rand() % 4];
            }
            if(input != '\n' && input != EOF){
                a.push_back(input);
            }
        }
        file.close();
        a.append("$");
        int64_t n = a.length();
        int64_t fs = 0;
        kLen = k;

        int64_t *SAptr = (int64_t*) malloc((n+fs)*sizeof(int64_t));
        int64_t *freq = NULL;

        //build the suffix array
        libsais64((const uint8_t *)a.c_str(),SAptr,n,fs,freq);
        //copy into the variables
        //std::copy(a.begin(), a.end(), std::back_inserter(text));
        text = a;
        SA.assign(SAptr, SAptr+n+fs);

        //if prefix table is necessary
        tuple<int,int> prevInterval = make_tuple(0,1);
        if(k != 0){
            prefix = unordered_map<string, tuple<int,int>>();
            for(int i=0;i<pow(sizeof(alphabet),k);i++){
                string pref = prefixGen(i,k);
                //if no matches were found for that prefix
                prevInterval = prefixInterval(pref, get<1>(prevInterval));
                //if it got stuck on a string in the text that cannot be queried
                while(get<0>(prevInterval) == -1){
                    prevInterval = prefixInterval(pref, get<1>(prevInterval)+1);
                }
                /*cout << get<0>(prevInterval);
                cout << "-";
                cout << get<1>(prevInterval);
                cout << "-";
                cout << pref;
                cout << "\n";*/
                prefix[pref] = prevInterval;
            }
        }
    }
    string prefixGen(int i, int k){
        string out = "";
        if (k == 0){
            return out;
        }
        out.append(prefixGen(i/sizeof(alphabet),k-1));
        out.push_back(alphabet[i%sizeof(alphabet)]);
        return out;
    }
    tuple<int,int> prefixInterval(string &prefix, int prevEnd){
        int last = prevEnd;
        if(SA[last]+prefix.length() >= text.length() ){
            return make_tuple(-1,last);
        }
        //start the search at prevEnd
        //the first k char of text at SA[i]
        for(int i=0;sharesPrefix(prefix,last);i++){
            last++;
            //if the string is shorter than our prefix or ends in $ we won't be searching for it anyways
        }
        //if last==prevEnd either this prefix is not found at all or 
        return make_tuple(prevEnd,last);
    }
    bool sharesPrefix(string &prefix, int posInSA){
        for(int j=0;j<prefix.length();j++){
            if(SA[posInSA]+j >= text.length() ){
                return false;
            }
            if(text[SA[posInSA]+j] != prefix.at(j)){
                return false;
            }
        } 
        return true;
    }
};

template <typename S> 
void serialize(S& s, buildsa & b){
    //auto writeInt = [](S& s, int& v) { s.template value<sizeof(v)>(v); };
    s.text1b(b.text, b.text.max_size());
    s.container8b(b.SA, b.text.max_size());
    s.ext(b.prefix, bitsery::ext::StdMap{INT_MAX}, [](S& s1, string& key, tuple<int,int>& value) {
        s1.text1b(key, INT_MAX);//limited by size of n
        s1.value4b(get<0>(value));
        s1.value4b(get<1>(value));
    });
    s.value4b(b.kLen);
}