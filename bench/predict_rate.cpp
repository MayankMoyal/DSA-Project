// Random-insertion reference model, not a guarantee for actual churn.
#include <cmath>
#include <iostream>
#include <vector>

// The probability model treats depth as a sum of Bernoulli variables.
// probability[d] is the chance that model depth equals d.
// States above the allowed depth are not stored; their total mass is the tail.
double predictedRebuilds(int n) {
    const double alpha = 0.7;
    int maxDepth = static_cast<int>(std::log(n) / std::log(1.0 / alpha));
    std::vector<double> probability(maxDepth + 1, 0.0);
    probability[0] = 1.0;

    for (int k = 1; k < n; k++) {
        double chanceOfIncrease = 2.0 / (k + 1);
        // Go backwards so probability[d - 1] still belongs to the previous step.
        for (int d = maxDepth; d >= 0; d--) {
            double sameDepth = (1.0 - chanceOfIncrease) * probability[d];
            double increasedDepth = 0.0;
            if (d > 0) {
                increasedDepth = chanceOfIncrease * probability[d - 1];
            }
            probability[d] = sameDepth + increasedDepth;
        }
    }
    double tailProbability = 1.0;
    for (double p : probability) {
        tailProbability -= p;
    }
    return 100.0 * tailProbability;
}

int main() {
    int sizes[] = {1000, 2000, 4000};
    for (int n : sizes) {
        double predicted = predictedRebuilds(n);
        std::cout << n << " local rebuilds per 100 pairs: " << predicted << '\n';
    }
    return 0;
}
