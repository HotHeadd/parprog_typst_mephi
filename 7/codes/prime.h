#include <cstdint>

bool is_prime(int64_t num) {
    if (num < 2)
        return false;
    if (num == 2)
        return true;
    for (int64_t del = 2; del * del <= num; del++) {
        if (num % del == 0)
            return false;
    }
    return true;
}
