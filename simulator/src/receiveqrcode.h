#ifndef BC2_SIMULATOR_RECEIVEQRCODE_H
#define BC2_SIMULATOR_RECEIVEQRCODE_H

#include <cstdint>
#include <string>
#include <vector>

class ReceiveQrCode final {
public:
    static constexpr int Size = 29; // QR version 3
    explicit ReceiveQrCode(const std::string &payload);
    bool module(int x, int y) const;
    int size() const { return Size; }

private:
    using Matrix = std::vector<std::vector<std::uint8_t>>;
    Matrix modules_;
};

#endif
