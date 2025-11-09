#include <vector>
#include <memory>
#include <random>
#include <algorithm>

class TMyGenerator {
public:
    TMyGenerator(int arrSize) : ArrSize_(arrSize), Rng_(Rd_()) {}

    std::unique_ptr<int32_t[]> GenPartiallySorted(double sortedPart, bool reversed) {
        auto arr = std::make_unique<int32_t[]>(ArrSize_);

        int sorted_count = static_cast<int>(ArrSize_ * sortedPart);
        if (sorted_count < 0) sorted_count = 0;
        if (sorted_count > ArrSize_) sorted_count = ArrSize_;

        std::iota(arr.get(), arr.get() + ArrSize_, 0);

        int shuffle_count = static_cast<int>((1.0 - sortedPart) * ArrSize_ / 2);
        std::uniform_int_distribution<int> dist(0, ArrSize_ - 1);

        for (int k = 0; k < shuffle_count; ++k) {
            int i = dist(Rng_);
            int j = dist(Rng_);
            std::swap(arr[i], arr[j]);
        }

        if (reversed) {
            std::reverse(arr.get(), arr.get() + ArrSize_);
        }

        return arr;
    }

    std::unique_ptr<int32_t[]> GenLocallySorted(int32_t blockSize) {
        std::unique_ptr<int32_t[]> arr = std::make_unique<int[]>(ArrSize_);

        std::iota(arr.get(), arr.get() + ArrSize_, 1);

        for (int32_t i = 0; i < ArrSize_; i += blockSize) {
            size_t end = std::min(i + blockSize, ArrSize_);
            std::sort(arr.get() + i, arr.get() + end);
        }
        return arr;

    }

    std::unique_ptr<int32_t[]> GenRandom(double uniquePart) {
        std::uniform_int_distribution<int32_t> dist(0, ArrSize_ * uniquePart);

        std::unique_ptr<int32_t[]> arr = std::make_unique<int[]>(ArrSize_);

        for (int i = 0; i < ArrSize_; ++i){
            arr[i] = dist(Rng_);
        }
        return arr;
    }

private:
    int ArrSize_;
    std::random_device Rd_;
    std::mt19937_64 Rng_;
};
