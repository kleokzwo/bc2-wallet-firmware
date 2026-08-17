#include "receiveqrcode.h"

#include <cassert>
#include <stdexcept>

int main() {
    const ReceiveQrCode qr("BMwS8cCtf17gAqVnP8kq5uS4zMNyYxAV2B");
    assert(qr.size() == 29);
    assert(qr.module(0, 0));
    assert(!qr.module(-1, 0));
    assert(!qr.module(29, 0));

    bool rejected = false;
    try {
        ReceiveQrCode tooLong(std::string(54, 'A'));
    } catch (const std::invalid_argument &) {
        rejected = true;
    }
    assert(rejected);
    return 0;
}
