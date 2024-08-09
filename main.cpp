#include <stdio.h>
#include <string_view>
#include "./src/server/server.hpp"
#include "src/client/client.hpp"

int main(int argc, char** argv){
    bool serverMode=false, clientMode=false;
    for (int i = 0; argv[i] != NULL; i++) {
        printf("Argument %d: %s\n", i, argv[i]);
        if ("-server" == std::string_view(argv[i])) {
            serverMode = true;
        }
        if ("-client" == std::string_view(argv[i])) {
            clientMode = true;
        }
    }



    if (serverMode) {
        server server;
        server.start();
    }

    if (clientMode) {
        client client;
        client.start();
    }

}
