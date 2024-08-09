

#include "src/client/client.hpp"
#include "src/server/server.hpp"
#include <string_view>
int main(int argc, char** argv) {
    for(int i = 0; i < argc; i++) {
        if ("-server" == std::string_view(argv[i])) {
            server server;
            server.start();
        }

        if ("-client" == std::string_view(argv[i])) {
            client server;
            server.start();
        }
    }
  return 0;
}