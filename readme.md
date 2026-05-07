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