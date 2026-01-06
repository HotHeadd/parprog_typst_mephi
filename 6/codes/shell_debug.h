#include <mpi.h>
#include <cstdint>
#include <vector>

struct GapInfo {
    int gap;
    uint64_t comps;
    uint64_t swaps;
    double time_ms;
};

struct SortStats {
    double total_ms = 0.0;
    uint64_t comps = 0;
    uint64_t swaps = 0;
    std::vector<GapInfo> gaps;
};

SortStats shell_sort_instrumented(int *array, int size) {
    SortStats st;
    double t0 = MPI_Wtime();
    for (int s = size / 512; s > 0; s /= 2) {
        GapInfo gi;
        gi.gap = s;
        gi.comps = 0;
        gi.swaps = 0;
        double g0 = MPI_Wtime();

        for (int offset = 0; offset < s; ++offset) {
            for (int i = offset + s; i < size; i += s) {
                for (int j = i - s; j >= 0; j -= s) {
                    ++gi.comps;
                    if (array[j] > array[j + s]) {
                        ++gi.swaps;
                        int tmp = array[j];
                        array[j] = array[j + s];
                        array[j + s] = tmp;
                    } else {
                        break;
                    }
                }
            }
        }

        double g1 = MPI_Wtime();
        gi.time_ms = (g1 - g0) * 1000.0;
        st.gaps.push_back(gi);
        st.comps += gi.comps;
        st.swaps += gi.swaps;
    }
    double t1 = MPI_Wtime();
    st.total_ms = (t1 - t0) * 1000.0;
    return st;
}
