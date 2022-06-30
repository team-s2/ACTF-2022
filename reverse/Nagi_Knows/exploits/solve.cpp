#include <iostream>
#include "nagi.hpp"

int main() {
    std::cout << Nagi<float, const float&, const float&, float, float&, const int, const
        float, const float, const float&, float&, const int, const float&, const
        int&, int, float, float&, float&, int&, float, const float, const float&,
        const int&, float, const float, const int&, int, float, const int, const
        float, const float, float&, int>::GetFlag() << std::endl;
    return 0;
}
