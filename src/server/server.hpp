#ifndef _snow_ball_server_
#define _snow_ball_server_

class server {
public:
  void start();

private:
  int remote_fd = -1;
};
#endif