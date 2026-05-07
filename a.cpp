#include "Syllable.h"
#include <iostream>
#include <iomanip>
#include <cassert>

using namespace vnpun;

/**
 * Phân tích từ "nguyễn":
 * - Onset: NGH (ng)
 * - Glide: U (u)
 * - Nucleus: IE (yê)
 * - Coda: N (n)
 * - Tone: Nga (~)
 */

int main() {
    freopen("o.txt","w",stdout);
    freopen("i.txt","r",stdin);
    // 1. Khởi tạo bằng Factory Method (Parse)
    std::string word; std::cin>>word;
    auto optSyllable = Syllable::from_string(word);

    if (!optSyllable.has_value()) {
        std::cerr << "Lỗi: Không thể parse từ '" << word << "'" << std::endl;
        return 1;
    }

    Syllable s = *optSyllable;

    std::cout << "=== PHÂN TÍCH TỪ: " << word << " ===" << std::endl;

    // 2. Sử dụng Getters để kiểm tra thành phần ngữ âm
    std::cout << "Onset   : " << to_string(s.onset()) << std::endl;
    std::cout << "Glide   : " << (s.has_glide() ? to_string(s.glide()) : "None") << std::endl;
    std::cout << "Nucleus : " << to_string(s.nucleus()) << std::endl;
    std::cout << "Coda    : " << (s.has_coda() ? to_string(s.coda()) : "None") << std::endl;
    std::cout << "Tone    : " << to_string(s.tone()) << std::endl;

    // 3. Kiểm tra các phương thức logic (Utilities)
    std::cout << "\n=== KIỂM TRA LOGIC ===" << std::endl;
    std::cout << "Has Onset?       : " << (s.has_onset() ? "Yes" : "No") << std::endl;
    std::cout << "Is Open Syllable?: " << (s.is_open_syllable() ? "Yes" : "No") << std::endl;
    std::cout << "Is Valid?        : " << (s.is_valid() ? "Yes" : "No") << std::endl;

    // 4. Sử dụng Setters để thay đổi từ 
    std::cout << "\n=== THAY ĐỔI DỮ LIỆU (SETTER) ===" << std::endl;
    std::cout << "Chuyển thanh Nga -> thanh Nặng..." << std::endl;
    s.set_tone(Tone::Nang);
    s.set_coda(Coda::null);
    
    // Xuất thông qua Vout (Toán tử << đã được overload)
    vout << "Kết quả sau khi đổi tone: ";
    vout << s; 
    std::cout << " (Dạng thô: " << s.to_string() << ")" << std::endl;

    // 5. Kiểm tra tính hợp lệ sau khi sửa đổi
    if (s.is_valid_combination()) {
        std::cout << "Kết hợp âm mới vẫn hợp lệ chính tả." << std::endl;
    }

    // 6. Test Edge Case: Reset về mặc định
    std::cout << "\n=== RESET SYLLABLE ===" << std::endl;
    s = Syllable(); // Default constructor
    std::cout << "Syllable rỗng, Nucleus mặc định: " 
              << (s.nucleus() == Nucleus::null ? "NONE" : "OTHER") << std::endl;

    std::cout << "\nHoàn tất kiểm tra file a.cpp" << std::endl;

    return 0;
}