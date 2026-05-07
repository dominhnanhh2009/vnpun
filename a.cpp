#include "Syllable.h"

int main(){
    freopen("i.txt","r",stdin);
    freopen("o.txt","w",stdout);
    auto a=vnpun::Syllable::fromUTF8("nguyễn");
    a->setAmChinh("ê");
    std::cout<<a->toUTF8();
}