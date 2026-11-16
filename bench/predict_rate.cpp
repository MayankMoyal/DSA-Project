#include <cmath>
#include <iostream>
#include <vector>
int main() {
    for (int n : {1000, 2000, 4000}) {
        int limit = static_cast<int>(std::log(n) / std::log(1.0 / 0.7));
        std::vector<double> depth(limit + 1);
        depth[0] = 1;
        for (int k = 1; k < n; ++k) {
            double b = 2.0 / (k + 1);
            for (int d = limit; d >= 0; --d)
                depth[d] = (1-b)*depth[d] + (d ? b*depth[d-1] : 0);
        }
        double tail = 1;
        for (double p : depth) tail -= p;
        std::cout << n << " local rebuilds per 100 pairs: " << 100*tail << '\n';
    }
}
