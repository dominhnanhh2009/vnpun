#pragma once

#include <string>
#include <optional>
#include <stdexcept>
#include<bits/stdc++.h>

namespace vnpun {

// =========================
// Tone (Dấu thanh)
// =========================
enum class Tone : uint8_t {
    NGANG = 0,
    SAC,
    HUYEN,
    HOI,
    NGA,
    NANG
};
inline std::string toneToString(Tone t) {
    switch (t) {
        case Tone::NGANG: return "ngang";
        case Tone::SAC:   return "sac";
        case Tone::HUYEN: return "huyen";
        case Tone::HOI:   return "hoi";
        case Tone::NGA:   return "nga";
        case Tone::NANG:  return "nang";
        default:          return "unknown";
    }
}

// =========================
// Exception
// =========================
class InvalidSyllable : public std::runtime_error {
public:
    explicit InvalidSyllable(const std::string& msg)
        : std::runtime_error(msg) {}
};

// =========================
// Syllable
// =========================
class Syllable {
public:
    // ---- ctor ----
    Syllable() = default;

    Syllable(std::string phuAmDau,
             std::string amDem,
             std::string amChinh,
             std::string amCuoi,
             Tone tone);

    // ---- factory ----
    // parse UTF-8 string -> Syllable (normalized)
    static std::optional<Syllable> fromUTF8(const std::string& input);

    // ---- serialize ----
    // normalized -> correct Vietnamese orthography
    std::string toUTF8() const;

    // ---- getter ----
    const std::string& phuAmDau() const noexcept { return phuAmDau_; }
    const std::string& amDem() const noexcept { return amDem_; }
    const std::string& amChinh() const noexcept { return amChinh_; }
    const std::string& amCuoi() const noexcept { return amCuoi_; }
    Tone tone() const noexcept { return tone_; }

    // ---- setter (auto normalize) ----
    void setPhuAmDau(const std::string& v);
    void setAmDem(const std::string& v);
    void setAmChinh(const std::string& v);
    void setAmCuoi(const std::string& v);
    void setTone(Tone t) noexcept { tone_ = t; }

    // ---- validation ----
    bool isValid() const noexcept;

    // ---- helpers ----
    bool hasAmDem() const noexcept { return !amDem_.empty(); }
    bool hasAmCuoi() const noexcept { return !amCuoi_.empty(); }
    bool hasPhuAmDau() const noexcept { return !phuAmDau_.empty(); }

    // xác định vị trí đặt dấu thanh (index trong amChinh sau khi build vần)
    // implementation sẽ phụ thuộc rule bạn mô tả
    int tonePosition() const noexcept;
    // ---- comparison operators ----
    bool operator==(const Syllable& other) const;
    bool operator!=(const Syllable& other) const;
    bool operator<(const Syllable& other) const;

private:
    // =========================
    // normalized components
    // =========================
    std::string phuAmDau_; // phụ âm đầu (normalized)
    std::string amDem_;    // âm đệm ("o")
    std::string amChinh_;  // nguyên âm / nguyên âm đôi (ua, ưa, ia, ...)
    std::string amCuoi_;   // phụ âm cuối / bán nguyên âm
    Tone tone_ = Tone::NGANG;

private:
    // =========================
    // normalization pipeline
    // =========================
    void normalize(); // normalize toàn bộ field

    static std::string normalizePhuAmDau(const std::string&);
    static std::string normalizeAmDem(const std::string&);
    static std::string normalizeAmChinh(const std::string&);
    static std::string normalizeAmCuoi(const std::string&);

    // =========================
    // validation internals
    // =========================
    static bool isValidPhuAmDau(const std::string&);
    static bool isValidAmDem(const std::string&);
    static bool isValidAmChinh(const std::string&);
    static bool isValidAmCuoi(const std::string&);

    // =========================
    // orthography pipeline (toUTF8)
    // =========================
    std::string buildVan() const;        // amDem + amChinh + amCuoi
    std::string applyOrthography() const; // full pipeline
    std::string applyToneMark(const std::string& syllable) const;

    // =========================
    // ICU helpers (no impl here)
    // =========================
    static std::string toLowerUTF8(const std::string&);
    static std::string normalizeNFC(const std::string&);
    static std::string stripTone(const std::string&, Tone& outTone);
    static std::string applyTone(const std::string&, Tone tone, int pos);
};


// =========================
// IO wrapper (vin / vout)
// =========================
class vin {
public:
    explicit vin(std::istream& is) : is_(is) {}
    inline explicit operator bool() const {return !is_.fail();}

    vin& operator>>(Syllable& s);

private:
    std::istream& is_;
};

class vout {
public:
    explicit vout(std::ostream& os) : os_(os) {}

    vout& operator<<(const Syllable& s);

private:
    std::ostream& os_;
};

} // namespace vnpun