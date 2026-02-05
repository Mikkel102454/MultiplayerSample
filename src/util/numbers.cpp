#include "util/numbers.h"

bool IsNumbersOnly(std::string_view str) {
    size_t i = 0;

    for (; i < str.size(); ++i) {
        if (str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}
