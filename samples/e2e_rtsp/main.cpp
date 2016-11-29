#include <iostream>

namespace e2e
{
namespace rtp
{
    struct EncodedFrame;

    struct Camera
    {
        Camera(const std::string& host, short port);

        EncodedFrame GetFrame() const;
    };
}
}

int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}