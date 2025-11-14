#pragma once
#include <cstdint>

namespace TDS {

class Server {
public:
    Server();
    ~Server();

    bool initialize(uint16_t port);
    void run();
    void shutdown();

private:
    void update(float dt);

    bool running;
    uint16_t port;
};

}
