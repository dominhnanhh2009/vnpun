#include "Syllable.h"

int main(){
    freopen("i.txt","r",stdin);
    freopen("o.txt","w",stdout);
    vnpun::Syllable a;
    vnpun::vin vcin(std::cin);
    vnpun::vout vcout(std::cout);

    auto a_opt=vnpun::Syllable::fromUTF8("qoả");
    if(!a_opt)std::cout<<"parrse failed";
    else{
        a=*a_opt;
        std::cout<< a.phuAmDau() <<' '<< a.amDem()<<' '<<a.amChinh()<<' '<<a.amCuoi()<<' '<<vnpun::toneToString(a.tone());
    }

    //vcout<<a;
}