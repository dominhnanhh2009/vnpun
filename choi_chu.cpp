#include <bits/stdc++.h>
#include "Syllable.h"

using namespace std;
using namespace vnpun;

/**
 * Hàm reorders: Tạo ra tất cả các biến thể hoán vị từ n Syllable đầu vào.
 * Logic: 
 * 1. Tách các thành phần (Dau, Dem, Chinh, Cuoi, Tone) của mỗi Syllable vào các vector riêng.
 * 2. Thực hiện hoán vị các vector thành phần này.
 * 3. Tổ hợp lại thành các Syllable mới và kiểm tra tính hợp lệ (isValid).
 */
set<vector<Syllable>> reorders(vector<Syllable> v) {
    size_t n = v.size();
    if (n == 0) return {};

    // Thu thập các thành phần của từng âm tiết
    vector<string> daus, dems, chinhs, cuois;
    vector<Tone> tones;

    for (const auto& s : v) {
        daus.push_back(s.phuAmDau());
        dems.push_back(s.amDem());
        chinhs.push_back(s.amChinh());
        cuois.push_back(s.amCuoi());
        tones.push_back(s.tone());
    }

    // Sắp xếp để next_permutation có thể duyệt hết các trường hợp
    sort(daus.begin(), daus.end());
    sort(dems.begin(), dems.end());
    sort(chinhs.begin(), chinhs.end());
    sort(cuois.begin(), cuois.end());
    sort(tones.begin(), tones.end());

    set<vector<Syllable>> results;

    // Lưu ý: Việc hoán vị tất cả các thành phần cùng lúc tạo ra tổ hợp rất lớn (n!^5)
    // Ở đây ta mô phỏng việc tạo ra các tiếng mới từ "nguyên liệu" của các tiếng cũ
    // Để tránh treo máy với n lớn, thuật toán này phù hợp nhất với n = 2 hoặc 3
    
    // Do cấu trúc hoán vị lồng nhau rất phức tạp, ta sẽ tập trung vào việc 
    // hoán vị các bộ thành phần (Dau, Van, Thanh) hoặc (Dau, Dem, Chinh, Cuoi, Thanh)
    
    // Demo hoán vị phổ biến nhất: Giữ nguyên thứ tự vị trí, hoán vị nội dung thành phần
    do {
        do {
            do {
                do {
                    do {
                        vector<Syllable> current_v;
                        bool all_valid = true;
                        
                        for (size_t i = 0; i < n; ++i) {
                            Syllable new_s(daus[i], dems[i], chinhs[i], cuois[i], tones[i]);
                            if (new_s.isValid()) {
                                current_v.push_back(new_s);
                            } else {
                                all_valid = false;
                                break;
                            }
                        }
                        
                        if (all_valid && current_v.size() == n) {
                            results.insert(current_v);
                        }
                        
                    } while (next_permutation(tones.begin(), tones.end()));
                } while (next_permutation(cuois.begin(), cuois.end()));
            } while (next_permutation(chinhs.begin(), chinhs.end()));
        } while (next_permutation(dems.begin(), dems.end()));
    } while (next_permutation(daus.begin(), daus.end()));

    return results;
}

int main() {
    // Mở file nhập xuất
    ifstream fin("i.txt");
    ofstream fout("o.txt");

    if (!fin.is_open()) {
        cerr << "Khong the mo file i.txt" << endl;
        return 1;
    }

    vin vcin(fin);
    vout vcout(fout);

    vector<Syllable> v;
    Syllable s;

    // Đọc tối đa 3 âm tiết từ file đầu vào
    while (v.size() < 3 && (vcin >> s)) {
        v.push_back(s);
    }

    if (v.empty()) {
        fout << "Khong co du lieu dau vao hop le." << endl;
        return 0;
    }

    // Tính toán các hoán vị (reorders)
    set<vector<Syllable>> all_reorders = reorders(v);

    // Xuất kết quả
    fout << "Ket qua hoan vi (reorders) cho: ";
    for (size_t i = 0; i < v.size(); ++i) {
        vcout << v[i];
        if (i < v.size() - 1) fout << " ";
    }
    fout << "\n---------------------------\n";

    for (const auto& phrase : all_reorders) {
        // Loại bỏ chính cụm từ gốc nếu không muốn in lại
        if (phrase == v) continue; 

        for (size_t i = 0; i < phrase.size(); ++i) {
            vcout << phrase[i];
            if (i < phrase.size() - 1) fout << " ";
        }
        fout << "\n";
    }

    fin.close();
    fout.close();

    return 0;
}