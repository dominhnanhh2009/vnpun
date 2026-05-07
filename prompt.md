tham khảo các file sau:
```md
file: readme.md
vnpun:
 + cung cấp 1 class Syllable biểu diễn 1 âm trong tiếng việt
 + vin/vout đơn giản wrap logic từ Syllable để IO string/Syllable (`vin>>syll1>>syll2;`, `vout<<syll2;`)
 + implement: lạm dụng triệt để ICU, ko tự parse/thao tác bất cứ logic nghiệp vụ unicode nào (phải đảm bảo độ chính xác tuyệt đối)

layout của 1 tiếng trong tiếng việt: phụ âm đầu + vần (âm đệm + âm chính + âm cuối) + dấu thanh (đây cũng là order quy ước, thứ tự trước sau tuân theo)

quy tắc chuẩn hóa cho các field (các field là utf8 string viết thường):
 + phụ âm đầu phải là phụ âm gốc, ko phải phụ âm theo chính tả (ngh -> ng, k/q -> c, gh -> g). có riêng pattern cho "q"
 + âm đệm "o" ("u" -> "o")
 + âm chính: các nguyên âm và nguyên âm đôi atomic (ua, ưa, ia). lưu ý "ua" là *always ATOMIC* và ko thể bị tách thành âm đệm "u" và âm chính "a", trong khi "oa" thì hoàn toàn có thể tách được thành âm đệm "o" + âm chính "a".
 các nguyên âm đôi cần lưu ý: "uô" thực ra là ua, "ươ" thật ra là "ưa", "iê" thật ra là "ia", ...
 + âm cuối: phụ âm (ng, c, nh, n,t ,m,p) hoặc bán nguyên âm (o,u,i,y)
 + thanh (ngang, sắc, huyền, hỏi, ngã, nặng)

như vậy, pipeline parse sẽ như sau (đây là thứ tự):
 + tách dấu thanh khỏi tiếng (lưu ý chỉ tách săc/huyền/hỏi/ngã/nặng. tuyệt đối giữ mọi dấu ê/ô/ơ/ư/ă/â)
 + tách phần phụ âm và vần (song song/có thể ooo)
 + chuẩn hóa phần vần
  * tách âm đệm (phần còn lại bên trái), âm chính (parse đầu tiên để đảm bảo nguyên atomic), âm cuối (phần còn lại bên phải). lưu ý trường hợp: "ui" thì "u" là âm chính và âm cuối "i", trong khi "uy" thì "y" là âm chính và "u" là âm đệm (sau đó tiếp tục được chuẩn hóa ở các bước sau)
  * âm đệm chuẩn hóa "u" thành "o"
  * ở âm chính:
   - nếu bắt đầu bằng "y" (có thể do trước đó `ko có phụ âm đầu` hoặc `có âm đệm` hoặc `do user cố ý`), chuyển "y" thành "i"
   - chuẩn hóa "iê"->"ia", "uô"->"ua", "ươ"->"ưa"
 + chuẩn hóa phần phụ âm đầu:
  * chuẩn hóa "ngh" -> "ng", "k" -> "c", "gh" -> "g"
 + fix pattern "q": nếu phụ âm đầu là "q" và có âm đệm thì phụ âm đầu "q" -> "c"

mô tả class Syllable:
 + chứa các private data member như PhuAmDau, AmDem, AmChinh, AmCuoi, DauThanh (tất cả đều phải được chuẩn hóa nếu đã khởi tạo)
 + chứa các getter/setter, hàm liên quan khác, check valid, hàm để xác định dấu thanh.....
 + các hàm support convert string utf8 (fromUTF8, toUTF8)

input/output contract:
 + fromUTF8 parse và trả về Syllable nếu tiếng đó có thể tách thành theo layout của tiếng việt (bắt buộc có âm chính, những thứ còn lại optional). input ko cần đúng chính tả. ví dụ abc ko parse được, trong khi đó "cích" thì parse được dù ko đúng chính tả tiếng việt
 + toUTF8 sẽ xuất ra 1 string utf8 đảm bảo các quy tắc chính tả cơ bản

các nguyên tắc chính tả cơ bản mà toUTF8 sẽ đảm bảo (pipeline dưới đây chính là order cứng):
 + convert c -> k, ng -> ngh, g -> gh khi theo sau (tức âm đệm nếu có, ko thì là âm chính) là nguyên âm bắt đầu bằng "i","e","ê" (i, e, ê, ia)
 + fix bỏ 1 chữ "i" khi phụ âm đầu là "gi" và phần liền ngay sau có chứa "i" ở đầu
 + fix "o" thành "u" nếu âm chính bắt đầu là "i","ê"
 + fix "ia" thành "iê", "ua" thành "uô", "ưa" thành "ươ" nếu đằng sau âm chính có âm cuối
 + convert "i" thành "y" ở nguyên âm chính nếu phụ âm đầu là rỗng hoặc khi có âm đệm
 + đảm bảo vị trí thanh đúng vị trí trong tiếng: thanh đánh vào âm chính của phần vần. nếu trường hợp là nguyên âm đôi thì đánh vào ký tự trước nếu ko có âm cuối, ngược lại đánh vào ký tự thứ 2


ví dụ:
 fromUTF8("ruộng") -> PhuAmDau "r", AmDem "", "ua", AmCuoi "ng", thanh nặng
 Syllable {"ch","u","ia","n", THANH_HOI}.toUTF8() -> "chuyển"

thêm nhiều ví dụ (lưu ý: toUTF8() có thể có nhiều cách viết hợp lệ do dấu thanh/quy tắc "q" nên bảng chỉ là ví dụ cho 1 cách hợp lý):
## Ví dụ parse `fromUTF8`

| Input     | Phụ âm đầu | Âm đệm | Âm chính | Âm cuối | Thanh |
| --------- | ---------- | ------ | -------- | ------- | ----- |
| `ruộng`   | `r`        | ``     | `ua`     | `ng`    | nặng  |
| `nghiêng` | `ng`       | ``     | `ia`     | `ng`    | ngang |
| `quốc`    | `c`        | ``     | `ua`     | `c`     | sắc   |
| `quả`     | `c`        | `o`    | `a`      | ``      | hỏi   |   
| `thuỷ`    | `th`       | `o`    | `i`      | ``      | hỏi   |
| `chuyện`  | `ch`       | `o`    | `ia`     | `n`     | nặng  |
| `giếng`   | `gi`       | ``     | `ia`     | `ng`    | sắc   |
| `yến`     | ``         | ``     | `ia`     | `n`     | sắc   |
| `toán`    | `t`        | `o`    | `a`      | `n`     | sắc   |
| `cích`    | `c`        | ``     | `i`      | `ch`    | sắc   |
| `ngoại`   | `ng`       | `o`    | `a`      | `i`     | nặng  |
| `phượng`  | `ph`       | ``     | `ưa`     | `ng`    | nặng  |
| `kéo`     | `c`        | ``     | `e`      | `o`     | sắc   |
| `ghế`     | `g`        | ``     | `ê`      | ``      | sắc   |
| `quyển`   | `c`        | `o`    | `ia`     | `n`     | hỏi   |

---

## Ví dụ `toUTF8`

| Syllable normalized               | Output UTF8 |
| --------------------------------- | ----------- |
| `{ "r", "", "ua", "ng", NẶNG }`   | `ruộng`     |
| `{ "ch", "o", "ia", "n", HỎI }`   | `chuyển`    |
| `{ "c", "", "i", "ch", SẮC }`     | `kích`      |
| `{ "g", "", "ê", "", SẮC }`       | `ghế`       |
| `{ "ng", "", "ia", "ng", NGANG }` | `nghiêng`   |
| `{ "c", "o", "a", "c", SẮC }`     | `quốc`      |
| `{ "", "", "ia", "n", SẮC }`      | `yến`       |
| `{ "th", "o", "i", "", HỎI }`     | `thuỷ`      |
| `{ "gi", "", "ia", "ng", SẮC }`   | `giếng`     |
| `{ "ph", "", "ưa", "ng", NẶNG }`  | `phượng`    |
| `{ "th", "o", "i", "", HỎI }`     | `thủy`    |

## 1 vài ví dụ full pipeline:
* fromUTF8("nghiện"): "nghiên" + thanh nặng; "ngh" + "iên" + thanh nặng; "ng" + "ia" + "n" + thanh nặng
  fromUTF("truyền"): "truyên" + thanh hỏi; "tr" + "u" + "yê" + "n"; "yê" -> "iê" -> "ia";
* {"ng","","ia", "n", thanh_nang}.toUTF8(): "ng" -> "ngh"; "ia" -> "iê"; "nghiên" + thanh nặng -> "nghiện"
* {"ng","u","ia", "n", thanh_ngã}.toUTF8(): "ng"->"ng"(ko làm gì cả); "ia" -> "iê" -> "yê" (do phía trước có âm đệm); "nguyên" + thanh ngã; "nguyễn"
```

```h
file: Syllable.h
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
```


các file mà bạn sẽ làm việc:
```cpp
file: Syllable.cpp
// file: Syllable.cpp
#include "Syllable.h"

#include <algorithm>
#include <vector>
#include <optional>
#include <utility>
#include <iostream>

#include <unicode/unistr.h>
#include <unicode/stringpiece.h>
#include <unicode/normalizer2.h>
#include <unicode/locid.h>
#include <unicode/utypes.h>

namespace vnpun {

namespace {

// ============================================================
// Small string helpers
// ============================================================

bool startsWith(const std::string& s, const std::string& prefix) {
    return s.rfind(prefix, 0) == 0;
}

bool containsString(const std::vector<std::string>& arr, const std::string& x) {
    return std::find(arr.begin(), arr.end(), x) != arr.end();
}

std::string unicodeStringToUTF8(const icu::UnicodeString& us) {
    std::string out;
    us.toUTF8String(out);
    return out;
}

int utf8CodePointLength(const std::string& s) {
    icu::UnicodeString us = icu::UnicodeString::fromUTF8(icu::StringPiece(s));
    int count = 0;
    for (int32_t i = 0; i < us.length();) {
        UChar32 c = us.char32At(i);
        i += U16_LENGTH(c);
        ++count;
    }
    return count;
}

// ============================================================
// Tone mark helpers
// ============================================================

bool isVietnameseToneMark(UChar32 c) {
    return c == 0x0301 || // sắc
           c == 0x0300 || // huyền
           c == 0x0309 || // hỏi
           c == 0x0303 || // ngã
           c == 0x0323;   // nặng
}

Tone toneFromCombiningMark(UChar32 c) {
    switch (c) {
        case 0x0301: return Tone::SAC;
        case 0x0300: return Tone::HUYEN;
        case 0x0309: return Tone::HOI;
        case 0x0303: return Tone::NGA;
        case 0x0323: return Tone::NANG;
        default:     return Tone::NGANG;
    }
}

UChar32 combiningMarkFromTone(Tone tone) {
    switch (tone) {
        case Tone::SAC:   return 0x0301;
        case Tone::HUYEN: return 0x0300;
        case Tone::HOI:   return 0x0309;
        case Tone::NGA:   return 0x0303;
        case Tone::NANG:  return 0x0323;
        case Tone::NGANG:
        default:          return 0;
    }
}

// ============================================================
// Vietnamese component tables
// ============================================================

const std::vector<std::string> VALID_PHU_AM_DAU = {
    "", "b", "c", "ch", "d", "đ", "g", "gi", "h", "kh", "l", "m", 
    "n", "ng", "nh", "p", "ph", "r", "s", "t", "th", "tr", "v", "x"
};

const std::vector<std::string> VALID_AM_DEM = {
    "", "o"
};

const std::vector<std::string> VALID_AM_CHINH = {
    "a", "ă", "â", "e", "ê", "i", "o", "ô", "ơ", "u", "ư", 
    "ia", "ua", "ưa"
};

const std::vector<std::string> VALID_AM_CUOI = {
    "", "c", "ch", "m", "n", "ng", "nh", "p", "t", "i", "y", "o", "u"
};

// Phụ âm đầu dạng chính tả. (Dài xếp trước ngắn)
const std::vector<std::string> INITIAL_CANDIDATES = {
    "ngh", "gh", "ng", "nh", "ch", "tr", "th", "ph", "kh", "gi", 
    "b", "c", "d", "đ", "g", "h", "k", "l", "m", "n", "p", "q", 
    "r", "s", "t", "v", "x"
};

// Âm chính dạng chính tả. (Dài xếp trước ngắn để bảo toàn atomic)
const std::vector<std::string> NUCLEUS_CANDIDATES = {
    "iê", "yê", "uô", "ươ", "ia", "ua", "ưa", 
    "ă", "â", "ê", "ô", "ơ", "ư", "a", "e", "i", "y", "o", "u"
};

struct ParsedVan {
    std::string amDem;
    std::string amChinh;
    std::string amCuoi;
};

struct OrthographicSegments {
    std::string initial;
    std::string medial;
    std::string nucleus;
    std::string final;
};

bool isValidMedialRaw(const std::string& s) {
    return s.empty() || s == "o" || s == "u";
}

bool isValidFinalRaw(const std::string& s) {
    return containsString(VALID_AM_CUOI, s);
}

// ============================================================
// Parse vần
// ============================================================

std::optional<ParsedVan> parseVanNormal(const std::string& van) {
    if (van.empty()) {
        return std::nullopt;
    }

    // "ua" luôn atomic.
    if (startsWith(van, "ua")) {
        std::string right = van.substr(std::string("ua").size());
        if (isValidFinalRaw(right)) return ParsedVan{"", "ua", right};
    }

    // "uô" -> âm chính ua.
    if (startsWith(van, "uô")) {
        std::string right = van.substr(std::string("uô").size());
        if (isValidFinalRaw(right)) return ParsedVan{"", "uô", right};
    }

    // "ươ" -> âm chính ưa.
    if (startsWith(van, "ươ")) {
        std::string right = van.substr(std::string("ươ").size());
        if (isValidFinalRaw(right)) return ParsedVan{"", "ươ", right};
    }

    // "uơ" dạng chính tả của âm chính ưa (thuở).
    if (startsWith(van, "uơ")) {
        std::string right = van.substr(std::string("uơ").size());
        if (isValidFinalRaw(right)) return ParsedVan{"", "ưa", right};
    }

    // "ui": u là âm chính, i là âm cuối.
    if (startsWith(van, "ui")) {
        std::string right = van.substr(std::string("u").size());
        if (isValidFinalRaw(right)) return ParsedVan{"", "u", right};
    }

    // "uya": u là âm đệm, ia là âm chính.
    if (startsWith(van, "uya")) {
        std::string right = van.substr(std::string("uya").size());
        if (isValidFinalRaw(right)) return ParsedVan{"u", "ia", right};
    }

    // "uy": u là âm đệm, y/yê là âm chính.
    if (startsWith(van, "uy")) {
        std::string rest = van.substr(std::string("u").size());
        for (const std::string& nucleus : NUCLEUS_CANDIDATES) {
            if (!startsWith(rest, nucleus)) continue;
            std::string right = rest.substr(nucleus.size());
            if (isValidFinalRaw(right)) return ParsedVan{"u", nucleus, right};
        }
    }

    // Không có âm đệm.
    for (const std::string& nucleus : NUCLEUS_CANDIDATES) {
        if (!startsWith(van, nucleus)) continue;
        std::string right = van.substr(nucleus.size());
        if (isValidFinalRaw(right)) return ParsedVan{"", nucleus, right};
    }

    // Có âm đệm o/u.
    for (const std::string& medial : std::vector<std::string>{"o", "u"}) {
        if (!startsWith(van, medial)) continue;
        std::string rest = van.substr(medial.size());
        for (const std::string& nucleus : NUCLEUS_CANDIDATES) {
            if (!startsWith(rest, nucleus)) continue;
            std::string right = rest.substr(nucleus.size());
            if (isValidFinalRaw(right)) return ParsedVan{medial, nucleus, right};
        }
    }

    return std::nullopt;
}

// ============================================================
// Pattern q
// ============================================================

std::optional<ParsedVan> parseVanWithQPattern(const std::string& van) {
    if (van.empty() || van == "u") {
        return std::nullopt; // "qu" không có âm chính
    }

    // quốc: q + uôc
    if (startsWith(van, "uô")) {
        std::string right = van.substr(std::string("uô").size());
        if (isValidFinalRaw(right)) return ParsedVan{"u", "uô", right};
    }

    // quyển, quỷ, quyền...
    if (startsWith(van, "uy")) {
        std::string rest = van.substr(std::string("u").size());
        for (const std::string& nucleus : NUCLEUS_CANDIDATES) {
            if (!startsWith(rest, nucleus)) continue;
            std::string right = rest.substr(nucleus.size());
            if (isValidFinalRaw(right)) return ParsedVan{"u", nucleus, right};
        }
    }

    // quả, quảng...
    if (startsWith(van, "u")) {
        std::string afterU = van.substr(std::string("u").size());
        auto parsed = parseVanNormal(afterU);
        if (parsed && parsed->amDem.empty()) {
            parsed->amDem = "u";
            return parsed;
        }
    }

    return std::nullopt;
}

std::pair<std::string, std::string> splitInitialAndVan(const std::string& noTone) {
    for (const std::string& initial : INITIAL_CANDIDATES) {
        if (startsWith(noTone, initial)) {
            return {initial, noTone.substr(initial.size())};
        }
    }
    return {"", noTone};
}

// ============================================================
// Orthography helpers
// ============================================================

bool shouldUseSoftInitial(const std::string& next) {
    return startsWith(next, "i") || startsWith(next, "e") ||
           startsWith(next, "ê") || startsWith(next, "ia");
}

OrthographicSegments buildOrthographicSegmentsFromNormalized(
    const std::string& phuAmDau,
    const std::string& amDem,
    const std::string& amChinh,
    const std::string& amCuoi
) {
    OrthographicSegments seg;
    seg.initial = phuAmDau;
    seg.medial  = amDem;
    seg.nucleus = amChinh;
    seg.final   = amCuoi;

    // Special case: thuở (th + "" + ưa + "")
    if (seg.initial == "th" && seg.medial.empty() &&
        seg.nucleus == "ưa" && seg.final.empty()) {
        seg.nucleus = "uơ";
        return seg;
    }

    // Q-pattern khi xuất
    if (seg.initial == "c" && seg.medial == "o" && seg.nucleus == "ua" && seg.final == "c") {
        seg.initial = "q"; seg.medial = ""; seg.nucleus = "uô"; return seg;
    }
    if (seg.initial == "c" && seg.medial == "o" && seg.nucleus == "a" && seg.final == "c") {
        seg.initial = "q"; seg.medial = ""; seg.nucleus = "uô"; return seg;
    }
    if (seg.initial == "c" && seg.medial == "o") {
        seg.initial = "q"; seg.medial = "u";
    }

    // 1. Convert c -> k, ng -> ngh, g -> gh
    {
        std::string next = !seg.medial.empty() ? seg.medial : seg.nucleus;
        if (seg.initial == "c" && shouldUseSoftInitial(next)) seg.initial = "k";
        else if (seg.initial == "ng" && shouldUseSoftInitial(next)) seg.initial = "ngh";
        else if (seg.initial == "g" && shouldUseSoftInitial(next)) seg.initial = "gh";
    }

    // 2. ia -> iê, ua -> uô, ưa -> ươ khi có âm cuối
    if (!seg.final.empty()) {
        if (seg.nucleus == "ia") seg.nucleus = "iê";
        else if (seg.nucleus == "ua") seg.nucleus = "uô";
        else if (seg.nucleus == "ưa") seg.nucleus = "ươ";
    }

    // 3. Giữ 1 chữ i sau `gi`
    {
        std::string rest = seg.medial + seg.nucleus + seg.final;
        if (seg.initial == "gi" && startsWith(rest, "i")) {
            if (seg.medial.empty() && startsWith(seg.nucleus, "i")) {
                seg.nucleus = seg.nucleus.substr(std::string("i").size());
            }
        }
    }

    // 4. i -> y ở âm chính
    if (seg.initial.empty() || !seg.medial.empty()) {
        if (seg.nucleus == "i") seg.nucleus = "y";
        else if (startsWith(seg.nucleus, "i")) {
            seg.nucleus = "y" + seg.nucleus.substr(std::string("i").size());
        }
    }

    // 5. Fix âm đệm o -> u nếu âm chính bắt đầu i/y
    if (seg.medial == "o" && (startsWith(seg.nucleus, "i") || startsWith(seg.nucleus, "y"))) {
        seg.medial = "u";
    }

    return seg;
}

int toneTargetIndexInsideNucleus(const std::string& orthographicNucleus, const std::string& final) {
    bool isDiphthong = 
        orthographicNucleus == "ia" || orthographicNucleus == "iê" ||
        orthographicNucleus == "yê" || orthographicNucleus == "ua" ||
        orthographicNucleus == "uô" || orthographicNucleus == "ưa" ||
        orthographicNucleus == "ươ" || orthographicNucleus == "uơ";

    if (!isDiphthong) return 0;
    if (orthographicNucleus == "uơ") return 1; // thuở
    return final.empty() ? 0 : 1;
}

} // namespace

// ============================================================
// Constructor
// ============================================================

Syllable::Syllable(std::string phuAmDau, std::string amDem, std::string amChinh, std::string amCuoi, Tone tone)
    : phuAmDau_(std::move(phuAmDau)),
      amDem_(std::move(amDem)),
      amChinh_(std::move(amChinh)),
      amCuoi_(std::move(amCuoi)),
      tone_(tone) {
    normalize();
    if (!isValid()) {
        throw InvalidSyllable("Invalid Vietnamese syllable components");
    }
}

// ============================================================
// Factory
// ============================================================

std::optional<Syllable> Syllable::fromUTF8(const std::string& input) {
    if (input.empty()) return std::nullopt;

    try {
        std::string s = normalizeNFC(toLowerUTF8(input));
        Tone tone = Tone::NGANG;
        std::string noTone = stripTone(s, tone);

        if (noTone.empty()) return std::nullopt;

        auto [rawInitial, rawVan] = splitInitialAndVan(noTone);
        if (rawVan.empty()) return std::nullopt;

        std::optional<ParsedVan> parsedVan;

        if (rawInitial == "q") {
            parsedVan = parseVanWithQPattern(rawVan);
        } else if (rawInitial == "gi") {
            parsedVan = parseVanNormal("i" + rawVan);
            if (!parsedVan) parsedVan = parseVanNormal(rawVan);
        } else {
            parsedVan = parseVanNormal(rawVan);
        }

        if (!parsedVan) return std::nullopt;

        Syllable result(rawInitial, parsedVan->amDem, parsedVan->amChinh, parsedVan->amCuoi, tone);
        if (!result.isValid()) return std::nullopt;

        return result;
    } catch (...) {
        return std::nullopt;
    }
}

// ============================================================
// Serialize
// ============================================================

std::string Syllable::toUTF8() const {
    if (!isValid()) throw InvalidSyllable("Cannot serialize invalid Vietnamese syllable");
    std::string noTone = applyOrthography();
    return applyToneMark(noTone);
}

// ============================================================
// Setters
// ============================================================

void Syllable::setPhuAmDau(const std::string& v) {
    phuAmDau_ = normalizePhuAmDau(v);
    if (!isValidPhuAmDau(phuAmDau_)) throw InvalidSyllable("Invalid phuAmDau");
}

void Syllable::setAmDem(const std::string& v) {
    amDem_ = normalizeAmDem(v);
    if (!isValidAmDem(amDem_)) throw InvalidSyllable("Invalid amDem");
}

void Syllable::setAmChinh(const std::string& v) {
    amChinh_ = normalizeAmChinh(v);
    if (!isValidAmChinh(amChinh_)) throw InvalidSyllable("Invalid amChinh");
}

void Syllable::setAmCuoi(const std::string& v) {
    amCuoi_ = normalizeAmCuoi(v);
    if (!isValidAmCuoi(amCuoi_)) throw InvalidSyllable("Invalid amCuoi");
}

// ============================================================
// Validation
// ============================================================

bool Syllable::isValid() const noexcept {
    return isValidPhuAmDau(phuAmDau_) &&
           isValidAmDem(amDem_) &&
           isValidAmChinh(amChinh_) &&
           isValidAmCuoi(amCuoi_);
}

bool Syllable::isValidPhuAmDau(const std::string& s) { return containsString(VALID_PHU_AM_DAU, s); }
bool Syllable::isValidAmDem(const std::string& s) { return containsString(VALID_AM_DEM, s); }
bool Syllable::isValidAmChinh(const std::string& s) { return containsString(VALID_AM_CHINH, s); }
bool Syllable::isValidAmCuoi(const std::string& s) { return containsString(VALID_AM_CUOI, s); }

// ============================================================
// Normalize fields
// ============================================================

void Syllable::normalize() {
    phuAmDau_ = normalizePhuAmDau(phuAmDau_);
    amDem_    = normalizeAmDem(amDem_);
    amChinh_  = normalizeAmChinh(amChinh_);
    amCuoi_   = normalizeAmCuoi(amCuoi_);

    if (phuAmDau_ == "q") phuAmDau_ = "c";
}

std::string Syllable::normalizePhuAmDau(const std::string& input) {
    std::string s = normalizeNFC(toLowerUTF8(input));
    if (s == "ngh") return "ng";
    if (s == "gh")  return "g";
    if (s == "k")   return "c";
    if (s == "q")   return "c";
    return s;
}

std::string Syllable::normalizeAmDem(const std::string& input) {
    std::string s = normalizeNFC(toLowerUTF8(input));
    if (s == "u") return "o";
    return s;
}

std::string Syllable::normalizeAmChinh(const std::string& input) {
    std::string s = normalizeNFC(toLowerUTF8(input));
    if (startsWith(s, "y")) s = "i" + s.substr(std::string("y").size());

    if (s == "iê") return "ia";
    if (s == "yê") return "ia";
    if (s == "uô") return "ua";
    if (s == "ươ") return "ưa";
    return s;
}

std::string Syllable::normalizeAmCuoi(const std::string& input) {
    return normalizeNFC(toLowerUTF8(input));
}

// ============================================================
// Tone position
// ============================================================

int Syllable::tonePosition() const noexcept {
    OrthographicSegments seg = buildOrthographicSegmentsFromNormalized(
        phuAmDau_, amDem_, amChinh_, amCuoi_
    );
    int posInNucleus = toneTargetIndexInsideNucleus(seg.nucleus, seg.final);
    return utf8CodePointLength(seg.medial) + posInNucleus;
}

// ============================================================
// Orthography pipeline
// ============================================================

std::string Syllable::buildVan() const {
    return amDem_ + amChinh_ + amCuoi_;
}

std::string Syllable::applyOrthography() const {
    OrthographicSegments seg = buildOrthographicSegmentsFromNormalized(
        phuAmDau_, amDem_, amChinh_, amCuoi_
    );
    return seg.initial + seg.medial + seg.nucleus + seg.final;
}

std::string Syllable::applyToneMark(const std::string& syllable) const {
    if (tone_ == Tone::NGANG) return normalizeNFC(syllable);

    OrthographicSegments seg = buildOrthographicSegmentsFromNormalized(
        phuAmDau_, amDem_, amChinh_, amCuoi_
    );

    int posInNucleus = toneTargetIndexInsideNucleus(seg.nucleus, seg.final);
    int globalPos = utf8CodePointLength(seg.initial) + utf8CodePointLength(seg.medial) + posInNucleus;

    return applyTone(syllable, tone_, globalPos);
}

// ============================================================
// ICU helpers
// ============================================================

std::string Syllable::toLowerUTF8(const std::string& input) {
    icu::UnicodeString us = icu::UnicodeString::fromUTF8(icu::StringPiece(input));
    us.toLower(icu::Locale("vi"));
    return unicodeStringToUTF8(us);
}

std::string Syllable::normalizeNFC(const std::string& input) {
    UErrorCode status = U_ZERO_ERROR;
    const icu::Normalizer2* nfc = icu::Normalizer2::getNFCInstance(status);
    if (U_FAILURE(status)) return input;

    icu::UnicodeString src = icu::UnicodeString::fromUTF8(icu::StringPiece(input));
    icu::UnicodeString dst;
    nfc->normalize(src, dst, status);

    if (U_FAILURE(status)) return input;
    return unicodeStringToUTF8(dst);
}

std::string Syllable::stripTone(const std::string& input, Tone& outTone) {
    outTone = Tone::NGANG;
    UErrorCode status = U_ZERO_ERROR;
    const icu::Normalizer2* nfd = icu::Normalizer2::getNFDInstance(status);
    if (U_FAILURE(status)) return input;

    icu::UnicodeString src = icu::UnicodeString::fromUTF8(icu::StringPiece(input));
    icu::UnicodeString decomposed;
    nfd->normalize(src, decomposed, status);

    if (U_FAILURE(status)) return input;

    icu::UnicodeString removed;
    for (int32_t i = 0; i < decomposed.length();) {
        UChar32 c = decomposed.char32At(i);
        i += U16_LENGTH(c);

        if (isVietnameseToneMark(c)) {
            outTone = toneFromCombiningMark(c);
            continue;
        }
        removed.append(c);
    }
    return normalizeNFC(unicodeStringToUTF8(removed));
}

std::string Syllable::applyTone(const std::string& input, Tone tone, int pos) {
    if (tone == Tone::NGANG) return normalizeNFC(input);
    UChar32 toneMark = combiningMarkFromTone(tone);
    if (toneMark == 0) return normalizeNFC(input);

    UErrorCode status = U_ZERO_ERROR;
    const icu::Normalizer2* nfd = icu::Normalizer2::getNFDInstance(status);
    if (U_FAILURE(status)) return input;

    icu::UnicodeString src = icu::UnicodeString::fromUTF8(icu::StringPiece(input));
    icu::UnicodeString result;
    int cpIndex = 0;

    for (int32_t i = 0; i < src.length();) {
        UChar32 c = src.char32At(i);
        i += U16_LENGTH(c);

        icu::UnicodeString one;
        one.append(c);

        icu::UnicodeString decomposedOne;
        UErrorCode localStatus = U_ZERO_ERROR;
        nfd->normalize(one, decomposedOne, localStatus);

        if (U_FAILURE(localStatus)) result.append(c);
        else result.append(decomposedOne);

        if (cpIndex == pos) result.append(toneMark);
        ++cpIndex;
    }
    return normalizeNFC(unicodeStringToUTF8(result));
}

// ============================================================
// IO wrapper
// ============================================================

vin& vin::operator>>(Syllable& s) {
    std::string token;
    if (!(is_ >> token)) return *this;
    
    auto parsed = Syllable::fromUTF8(token);
    if (!parsed) {
        is_.setstate(std::ios::failbit);
        return *this;
    }
    s = *parsed;
    return *this;
}

vout& vout::operator<<(const Syllable& s) {
    os_ << s.toUTF8();
    return *this;
}

} // namespace vnpun
```


yêu cầu:
 - hãy sửa implement để khớp với readme.md