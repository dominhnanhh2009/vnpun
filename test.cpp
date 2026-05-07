#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <iomanip>

#include "Syllable.h" // đổi lại đúng tên file header của bạn

using namespace vnpun;

struct TestCase {
    std::string input;

    std::string phuAmDau;
    std::string amDem;
    std::string amChinh;
    std::string amCuoi;
    Tone tone;

    std::string expectedOutput;
    bool expectValid = true;
};

std::string optStr(const std::string& s) {
    return s.empty() ? "∅" : s;
}

bool runOne(const TestCase& tc) {
    std::cout<<"running "<<tc.input<<'\n';
    auto sOpt = Syllable::fromUTF8(tc.input);

    if (!tc.expectValid) {
        if (sOpt.has_value()) {
            std::cout << "[FAIL] expected invalid but got valid: " << tc.input << "\n";
            return false;
        }
        std::cout << "[ OK ] invalid as expected: " << tc.input << "\n";
        return true;
    }

    if (!sOpt.has_value()) {
        std::cout << "[FAIL] parse failed: " << tc.input << "\n";
        return false;
    }

    const auto& s = *sOpt;

    bool ok = true;

    auto check = [&](const std::string& name,
                     const std::string& got,
                     const std::string& expect) {
        if (got != expect) {
            std::cout << "   mismatch " << name
                      << " | got=" << optStr(got)
                      << " expect=" << optStr(expect) << "\n";
            ok = false;
        }
    };

    check("phuAmDau", s.phuAmDau(), tc.phuAmDau);
    check("amDem",    s.amDem(),    tc.amDem);
    check("amChinh",  s.amChinh(),  tc.amChinh);
    check("amCuoi",   s.amCuoi(),   tc.amCuoi);

    if (s.tone() != tc.tone) {
        std::cout << "   mismatch tone | got=" << toneToString(s.tone())
                  << " expect=" << toneToString(tc.tone) << "\n";
        ok = false;
    }

    std::string out = s.toUTF8();
    if (out != tc.expectedOutput) {
        std::cout << "   mismatch toUTF8 | got=" << out
                  << " expect=" << tc.expectedOutput << "\n";
        ok = false;
    }

    if (ok) {
        std::cout << "[ OK ] " << tc.input << " -> " << out << "\n";
    } else {
        std::cout << "[FAIL] " << tc.input << "\n";
    }

    return ok;
}

int main() {
    freopen("o.txt","w",stdout);
    std::vector<TestCase> tests = {
        // ===== ví dụ đầu =====
        {"ruộng", "r", "", "ua", "ng", Tone::NANG, "ruộng"},

        // ===== bảng =====
        {"quảng", "c", "o", "a",  "ng", Tone::HOI,  "quảng"},
        {"kuết",  "c", "o", "ê",  "t",  Tone::SAC,  "quết"},
        {"tiết",  "t", "",  "ia", "t",  Tone::SAC,  "tiết"},
        {"thưởng","th","",  "ưa","ng",  Tone::HOI,  "thưởng"},
        {"rượu",  "r", "",  "ưa","u",   Tone::NANG, "rượu"},
        {"kên",   "c", "",  "ê", "n",   Tone::NGANG,"kên"},
        {"quên",  "c", "o", "ê", "n",   Tone::NGANG,"quên"},
        {"quỷ",   "c", "o", "i", "",    Tone::HOI,  "quỷ"},
        {"cũi",   "c", "",  "u", "i",   Tone::NGA,  "cũi"},
        {"cu",    "c", "",  "u", "",    Tone::NGANG,"cu"},

        // invalid
        {"qu",    "c", "o", "",  "",    Tone::NGANG,"", false},

        {"cuốc",  "c", "",  "ua","c",   Tone::SAC,  "cuốc"},
        {"quyên", "c", "o", "ia","n",   Tone::NGANG,"quyên"},
        {"khuya", "kh","o", "ia","",    Tone::NGANG,"khuya"},
        {"nghĩa", "ng","",  "ia","",    Tone::NGA,  "nghĩa"},
        {"ghế",   "g", "",  "ê", "",    Tone::SAC,  "ghế"},
        {"ghì",   "g", "",  "i", "",    Tone::HUYEN,"ghì"},
        {"kì",    "c", "",  "i", "",    Tone::HUYEN,"kì"},
        {"kỳ",    "c", "",  "i", "",    Tone::HUYEN,"kì"},
        {"của",   "c", "",  "ua","",    Tone::HOI,  "của"},
        {"thuở",  "th","",  "ưa","",    Tone::HOI,  "thuở"},
        {"quốc",  "c", "o", "ua","c",   Tone::SAC,  "quốc"},
        {"chuyện","ch","o", "ia","n",   Tone::NANG, "chuyện"},
        {"truyện","tr","o",  "ia","n",   Tone::NANG, "truyện"},
        {"duyệt", "d", "o", "ia","t",   Tone::NANG, "duyệt"},
        {"giếng", "gi","",  "ia","ng",  Tone::SAC,  "giếng"},
        {"nghiêng","ng","", "ia","ng",  Tone::NGANG,"nghiêng"},
        {"ngoáy", "ng","o", "a", "y",   Tone::SAC,  "ngoáy"},
        {"xoáy",  "x", "o", "a", "y",   Tone::SAC,  "xoáy"},
        {"huých", "h", "o", "i", "ch",  Tone::SAC,  "huých"},
        {"suýt",  "s", "o", "i", "t",   Tone::SAC,  "suýt"},
        {"quậy",  "c", "o", "â", "y",   Tone::NANG, "quậy"},
        {"quỳ",   "c", "o", "i", "",    Tone::HUYEN,"quỳ"},
        {"quy",   "c", "o", "i", "",    Tone::NGANG,"quy"},
        {"yến",   "",  "",  "ia","n",   Tone::SAC,  "yến"},
        {"yêu",   "",  "",  "ia","u",   Tone::NGANG,"yêu"},
        {"oai",   "",  "o", "a", "i",   Tone::NGANG,"oai"},
        {"uỷ",    "",  "o", "i", "",    Tone::HOI,  "uỷ"},
    };

    int pass = 0;

    for (const auto& tc : tests) {
        if (runOne(tc)) pass++;
    }

    std::cout << "\n==== RESULT ====\n";
    std::cout << "Passed: " << pass << " / " << tests.size() << "\n";

    return (pass == (int)tests.size()) ? 0 : 1;
}