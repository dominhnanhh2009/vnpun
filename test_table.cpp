#include "Syllable.h"
#include <iomanip>

int main() {
    freopen("i.txt", "r", stdin);
    freopen("o.txt", "w", stdout);

    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    std::string word;

    std::cout
        << std::left
        << std::setw(12) << "word"
        << std::setw(10) << "phuAm"
        << std::setw(8)  << "amDem"
        << std::setw(10) << "amChinh"
        << std::setw(8)  << "amCuoi"
        << std::setw(8)  << "tone"
        << std::setw(12) << "toUTF8"
        << "\n";

    std::cout << std::string(80, '-') << "\n";

    while (std::cin >> word) {
        auto opt = vnpun::Syllable::fromUTF8(word);

        if (!opt.has_value()) {
            std::cout
                << std::left
                << std::setw(12) << word
                << "[INVALID]\n";
            continue;
        }

        const auto& s = opt.value();

        std::cout
            << std::left
            << std::setw(12) << word
            << std::setw(10) << s.phuAmDau()
            << std::setw(8)  << s.amDem()
            << std::setw(10) << s.amChinh()
            << std::setw(8)  << s.amCuoi()
            << std::setw(8)  << vnpun::toneToString(s.tone())
            << std::setw(12) << s.toUTF8()
            << "\n";
    }

    return 0;
}