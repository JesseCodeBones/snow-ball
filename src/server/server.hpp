#ifndef _snow_ball_server_
#define _snow_ball_server_
#include <string>

class server {
public:
  void start();

private:
  int remote_fd = -1;
  std::string remote_host;
  int remote_port = -1;
};
#endif