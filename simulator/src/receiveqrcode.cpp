#include "receiveqrcode.h"

#include <algorithm>
#include <array>
#include <stdexcept>

namespace {
constexpr int kSize = ReceiveQrCode::Size;
constexpr int kDataCodewords = 55;
constexpr int kEccCodewords = 15;
using Matrix = std::vector<std::vector<std::uint8_t>>;
using Reserved = std::vector<std::vector<bool>>;

void appendBits(std::vector<bool> &bits, unsigned value, int count) {
    for (int i = count - 1; i >= 0; --i) bits.push_back(((value >> i) & 1U) != 0U);
}

std::uint8_t multiply(std::uint8_t x, std::uint8_t y) {
    unsigned result = 0;
    unsigned value = x;
    for (unsigned factor = y; factor != 0; factor >>= 1) {
        if ((factor & 1U) != 0U) result ^= value;
        value = (value << 1U) ^ ((value & 0x80U) != 0U ? 0x11DU : 0U);
    }
    return static_cast<std::uint8_t>(result);
}

std::vector<std::uint8_t> generator(int degree) {
    std::vector<std::uint8_t> result(static_cast<std::size_t>(degree), 0);
    result.back() = 1;
    std::uint8_t root = 1;
    for (int i = 0; i < degree; ++i) {
        for (int j = 0; j < degree; ++j) {
            result[static_cast<std::size_t>(j)] = multiply(result[static_cast<std::size_t>(j)], root);
            if (j + 1 < degree) result[static_cast<std::size_t>(j)] ^= result[static_cast<std::size_t>(j + 1)];
        }
        root = multiply(root, 0x02);
    }
    return result;
}

std::vector<std::uint8_t> ecc(const std::vector<std::uint8_t> &data) {
    const auto divisor = generator(kEccCodewords);
    std::vector<std::uint8_t> result(kEccCodewords, 0);
    for (std::uint8_t value : data) {
        const std::uint8_t factor = value ^ result.front();
        std::rotate(result.begin(), result.begin() + 1, result.end());
        result.back() = 0;
        for (int i = 0; i < kEccCodewords; ++i)
            result[static_cast<std::size_t>(i)] ^= multiply(divisor[static_cast<std::size_t>(i)], factor);
    }
    return result;
}

void set(Matrix &m, Reserved &r, int x, int y, bool dark, bool reserve = true) {
    if (x < 0 || y < 0 || x >= kSize || y >= kSize) return;
    m[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = dark ? 1U : 0U;
    if (reserve) r[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] = true;
}

void finder(Matrix &m, Reserved &r, int cx, int cy) {
    for (int dy = -4; dy <= 4; ++dy) for (int dx = -4; dx <= 4; ++dx) {
        const int distance = std::max(std::abs(dx), std::abs(dy));
        set(m, r, cx + dx, cy + dy, distance != 2 && distance != 4);
    }
}

void alignment(Matrix &m, Reserved &r, int cx, int cy) {
    for (int dy = -2; dy <= 2; ++dy) for (int dx = -2; dx <= 2; ++dx)
        set(m, r, cx + dx, cy + dy, std::max(std::abs(dx), std::abs(dy)) != 1);
}

bool maskBit(int mask, int x, int y) {
    switch (mask) {
        case 0: return (x + y) % 2 == 0;
        case 1: return y % 2 == 0;
        case 2: return x % 3 == 0;
        case 3: return (x + y) % 3 == 0;
        case 4: return (x / 3 + y / 2) % 2 == 0;
        case 5: return (x * y) % 2 + (x * y) % 3 == 0;
        case 6: return ((x * y) % 2 + (x * y) % 3) % 2 == 0;
        default: return ((x + y) % 2 + (x * y) % 3) % 2 == 0;
    }
}

int formatBits(int mask) {
    int data = (1 << 3) | mask; // ECC L
    int rem = data;
    for (int i = 0; i < 10; ++i) rem = (rem << 1) ^ (((rem >> 9) & 1) != 0 ? 0x537 : 0);
    return ((data << 10) | rem) ^ 0x5412;
}

void drawFormat(Matrix &m, Reserved &r, int mask) {
    const int bits = formatBits(mask);
    auto bit = [bits](int i) { return ((bits >> i) & 1) != 0; };
    for (int i = 0; i <= 5; ++i) set(m, r, 8, i, bit(i));
    set(m, r, 8, 7, bit(6)); set(m, r, 8, 8, bit(7)); set(m, r, 7, 8, bit(8));
    for (int i = 9; i < 15; ++i) set(m, r, 14 - i, 8, bit(i));
    for (int i = 0; i < 8; ++i) set(m, r, kSize - 1 - i, 8, bit(i));
    for (int i = 8; i < 15; ++i) set(m, r, 8, kSize - 15 + i, bit(i));
    set(m, r, 8, kSize - 8, true);
}

int penalty(const Matrix &m) {
    int score = 0;
    for (int y = 0; y < kSize; ++y) {
        int run = 1;
        for (int x = 1; x < kSize; ++x) {
            if (m[y][x] == m[y][x - 1]) { if (++run == 5) score += 3; else if (run > 5) ++score; } else run = 1;
        }
    }
    for (int x = 0; x < kSize; ++x) {
        int run = 1;
        for (int y = 1; y < kSize; ++y) {
            if (m[y][x] == m[y - 1][x]) { if (++run == 5) score += 3; else if (run > 5) ++score; } else run = 1;
        }
    }
    for (int y = 0; y < kSize - 1; ++y) for (int x = 0; x < kSize - 1; ++x) {
        const auto c = m[y][x]; if (c == m[y][x+1] && c == m[y+1][x] && c == m[y+1][x+1]) score += 3;
    }
    int dark = 0; for (const auto &row : m) for (auto c : row) dark += c != 0;
    score += std::abs(dark * 20 - kSize * kSize * 10) / (kSize * kSize) * 10;
    return score;
}
}

ReceiveQrCode::ReceiveQrCode(const std::string &payload) {
    if (payload.size() > 53U) throw std::invalid_argument("QR payload too long for version 3-L");
    std::vector<bool> bits;
    appendBits(bits, 0x4, 4); appendBits(bits, static_cast<unsigned>(payload.size()), 8);
    for (unsigned char c : payload) appendBits(bits, c, 8);
    const int capacity = kDataCodewords * 8;
    appendBits(bits, 0, std::min(4, capacity - static_cast<int>(bits.size())));
    while (bits.size() % 8 != 0) bits.push_back(false);
    std::vector<std::uint8_t> data;
    for (std::size_t i = 0; i < bits.size(); i += 8) {
        unsigned value = 0; for (int j = 0; j < 8; ++j) value = (value << 1) | (bits[i + j] ? 1U : 0U);
        data.push_back(static_cast<std::uint8_t>(value));
    }
    for (bool toggle = true; data.size() < kDataCodewords; toggle = !toggle) data.push_back(toggle ? 0xEC : 0x11);
    auto codewords = data; const auto correction = ecc(data); codewords.insert(codewords.end(), correction.begin(), correction.end());

    Matrix base(kSize, std::vector<std::uint8_t>(kSize, 0));
    Reserved reserved(kSize, std::vector<bool>(kSize, false));
    finder(base, reserved, 3, 3); finder(base, reserved, kSize - 4, 3); finder(base, reserved, 3, kSize - 4);
    for (int i = 8; i < kSize - 8; ++i) { set(base, reserved, i, 6, i % 2 == 0); set(base, reserved, 6, i, i % 2 == 0); }
    alignment(base, reserved, 22, 22);
    drawFormat(base, reserved, 0);

    std::vector<bool> stream; for (auto value : codewords) appendBits(stream, value, 8);
    int bitIndex = 0; int direction = -1;
    for (int right = kSize - 1; right >= 1; right -= 2) {
        if (right == 6) --right;
        for (int vertical = 0; vertical < kSize; ++vertical) {
            const int y = direction < 0 ? kSize - 1 - vertical : vertical;
            for (int offset = 0; offset < 2; ++offset) {
                const int x = right - offset;
                if (!reserved[y][x]) base[y][x] = bitIndex < static_cast<int>(stream.size()) && stream[static_cast<std::size_t>(bitIndex++)] ? 1U : 0U;
            }
        }
        direction = -direction;
    }

    int bestScore = 1 << 30;
    for (int mask = 0; mask < 8; ++mask) {
        Matrix candidate = base; Reserved candidateReserved = reserved;
        for (int y = 0; y < kSize; ++y) for (int x = 0; x < kSize; ++x)
            if (!reserved[y][x] && maskBit(mask, x, y)) candidate[y][x] ^= 1U;
        drawFormat(candidate, candidateReserved, mask);
        const int current = penalty(candidate);
        if (current < bestScore) { bestScore = current; modules_ = std::move(candidate); }
    }
}

bool ReceiveQrCode::module(int x, int y) const {
    return x >= 0 && y >= 0 && x < Size && y < Size && modules_[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] != 0;
}
