#include <iostream>
#include <fstream>
#include <sstream>
#include <math.h>
#include <map>

using namespace std;
static constexpr const char alphabet[4] = {'A','C','G','T'};

int main(int argc, char * argv[]){
    
    std::ifstream infile;
    std::ofstream outfile;
    infile.open("chromosome20.fa");
    std::stringstream stream;
    string text = "";
    //skip firstline
    int firstLine = ' ';
    while (firstLine != '\n'){
        firstLine = infile.get();
    }
    while (infile.good()){
        char input = infile.get();
        input = toupper(input);
        if(input == 'n'){
            input = alphabet[rand() % 4];
        }
        if(input != '\n'){
            text.push_back(input);
        }
    }
    infile.close();
    outfile.open("Chr20.fa");
    for(int i=0;i<10000;i++){
        int pos = rand() % text.length();
        int length = rand() % 50 + 20;
        string line = "";
        string query = text.substr(pos,length);
        for(int j=0;j<length;j++){
            if(query.at(j) == 'N' || query.at(j) == 'n'){
                string replacement = "";
                replacement += alphabet[rand() % 4];
                query.replace(j, 1, replacement);
            }
        }
        line.append(">");
        line.append(to_string(i));
        line.append(":");
        line.append(to_string(i));
        if(rand() % 50 == 0){
            line.append(":M\n");
            int mutaLen = rand() % 6 + 1;
            string replacement = "";
            replacement += alphabet[rand() % 4];
            query.replace(rand() % (query.length()-mutaLen),mutaLen,replacement);
            line.append(query);
        }
        else{
            line.append(":R\n");
            line.append(query);
        }
        line.append("\n");
        outfile << line;    
    }
    outfile.close();

}