#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>   // memcpy
#include <stdexcept>
#include <type_traits>

#if defined(_WIN32)
  #include <winsock2.h>  // htonl/ntohl
#else
  #include <arpa/inet.h>
#endif

// --- 辅助：大小端转换（网络序：大端） ---
inline uint16_t to_be16(uint16_t v) { return htons(v); }
inline uint16_t from_be16(uint16_t v) { return ntohs(v); }
inline uint32_t to_be32(uint32_t v) { return htonl(v); }
inline uint32_t from_be32(uint32_t v) { return ntohl(v); }
inline uint64_t to_be64(uint64_t v) {
    uint32_t hi = uint32_t(v >> 32), lo = uint32_t(v & 0xFFFFFFFFu);
    hi = htonl(hi); lo = htonl(lo);
    return (uint64_t(lo) << 32) | hi; // 注意顺序
}
inline uint64_t from_be64(uint64_t v) {
    uint32_t hi = uint32_t(v >> 32), lo = uint32_t(v & 0xFFFFFFFFu);
    hi = ntohl(hi); lo = ntohl(lo);
    return (uint64_t(lo) << 32) | hi;
}

// --- 将 float/double 以 IEEE754 的位模式按大端传输 ---
inline uint32_t float_to_be32(float f) {
    static_assert(sizeof(float) == 4, "float must be 4 bytes");
    uint32_t u; std::memcpy(&u, &f, 4); return to_be32(u);
}
inline float float_from_be32(uint32_t be) {
    uint32_t u = from_be32(be); float f; std::memcpy(&f, &u, 4); return f;
}
inline uint64_t double_to_be64(double d) {
    static_assert(sizeof(double) == 8, "double must be 8 bytes");
    uint64_t u; std::memcpy(&u, &d, 8); return to_be64(u);
}
inline double double_from_be64(uint64_t be) {
    uint64_t u = from_be64(be); double d; std::memcpy(&d, &u, 8); return d;
}

// =================== Writer ===================
struct PacketWriter {
    std::vector<uint8_t> buf;

    const uint8_t* data() const { return buf.data(); }
    size_t size() const { return buf.size(); }
    void writeU8(uint8_t v) { buf.push_back(v); }
    void writeU16(uint16_t v) {
        uint16_t be = to_be16(v);
        auto p = (uint8_t*)&be; buf.insert(buf.end(), p, p+2);
    }
    void writeU32(uint32_t v) {
        uint32_t be = to_be32(v);
        auto p = (uint8_t*)&be; buf.insert(buf.end(), p, p+4);
    }
    void writeU64(uint64_t v) {
        uint64_t be = to_be64(v);
        auto p = (uint8_t*)&be; buf.insert(buf.end(), p, p+8);
    }
    void writeF32(float f) {
        uint32_t be = float_to_be32(f);
        auto p = (uint8_t*)&be; buf.insert(buf.end(), p, p+4);
    }
    void writeF64(double d) {
        uint64_t be = double_to_be64(d);
        auto p = (uint8_t*)&be; buf.insert(buf.end(), p, p+8);
    }
    // 字符串：先写uint16长度，再写UTF-8字节
    void writeString(const std::string& s) {
        if (s.size() > 0xFFFF) throw std::length_error("string too long");
        writeU16(static_cast<uint16_t>(s.size()));
        buf.insert(buf.end(), s.begin(), s.end());
    }
    // 写原始字节
    void writeBytes(const void* data, size_t n) {
        auto p = static_cast<const uint8_t*>(data);
        buf.insert(buf.end(), p, p+n);
    }
};

// =================== Reader ===================
struct PacketReader {
    const uint8_t* p = nullptr;
    size_t len = 0;
    size_t off = 0;

    PacketReader(const void* data, size_t size) : p((const uint8_t*)data), len(size) {}

    bool remaining(size_t n) const { return off + n <= len; }
    void need(size_t n) {
        if (!remaining(n)) throw std::runtime_error("packet underflow");
    }

    uint8_t readU8() {
        need(1); return p[off++];
    }
    uint16_t readU16() {
        need(2); uint16_t be; std::memcpy(&be, p+off, 2); off+=2; return from_be16(be);
    }
    uint32_t readU32() {
        need(4); uint32_t be; std::memcpy(&be, p+off, 4); off+=4; return from_be32(be);
    }
    uint64_t readU64() {
        need(8); uint64_t be; std::memcpy(&be, p+off, 8); off+=8; return from_be64(be);
    }
    float readF32() {
        need(4); uint32_t be; std::memcpy(&be, p+off, 4); off+=4; return float_from_be32(be);
    }
    double readF64() {
        need(8); uint64_t be; std::memcpy(&be, p+off, 8); off+=8; return double_from_be64(be);
    }
    std::string readString() {
        uint16_t n = readU16();
        need(n);
        std::string s((const char*)p+off, (const char*)p+off+n);
        off += n;
        return s;
    }
    // 读原始字节（拷贝出去）
    std::vector<uint8_t> readBytes(size_t n) {
        need(n); std::vector<uint8_t> out(p+off, p+off+n); off += n; return out;
    }
};
