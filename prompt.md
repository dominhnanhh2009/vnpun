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
 + âm chính: các nguyên âm và nguyên âm đôi (ua, ưa, ia). lưu ý "ua" là *always ATOMIC* và ko thể bị tách thành âm đệm "u" và âm chính "a", trong khi "oa" thì hoàn toàn có thể tách được thành âm đệm "o" + âm chính "a".
 các nguyên âm đôi cần lưu ý: "uô" thực ra là ua, "ươ" thật ra là "ưa", "iê" thật ra là "ia", ...
 + âm cuối: phụ âm (ng, c, nh, n,t ,m,p) hoặc bán nguyên âm (o,u,i,y)
 + thanh (ngang, sắc, huyền, hỏi, ngã, nặng)

như vậy, pipeline parse sẽ như sau (đây là thứ tự):
 + tách dấu thanh khỏi tiếng (lưu ý chỉ tách săc/huyền/hỏi/ngã/nặng. tuyệt đối giữ mọi dấu ê/ô/ơ/ư/ă/â)
 + tách phần phụ âm và vần (song song/có thể ooo)
 + chuẩn hóa phần vần
  * tách âm đệm (phần còn lại bên trái), âm chính (parse đầu tiên để đảm bảo nguyên atomic), âm cuối (phần còn lại bên phải)
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
 + fix "o" thành "u" nếu âm chính là "i"
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

```

yêu cầu:
 - implement vào file .cpp