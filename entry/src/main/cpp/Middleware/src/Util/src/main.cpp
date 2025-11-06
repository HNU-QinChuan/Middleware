#include "napi_util.h"
#include <iostream>

int main(int argc, char* argv[]){
    std::vector<std::string> tokens;
    for(int i=1; i < argc; i++){
        tokens.push_back(argv[i]);
    }
    runCommand(tokens);
    return 0;
}
