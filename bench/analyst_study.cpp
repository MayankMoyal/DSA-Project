// Runs five independent trials, summarises their CSV rows, and draws SVG graphs.
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;
namespace fs = std::filesystem;

const int TRIALS = 5;
const int CHECKPOINTS = 100;
// Alphabetical order matches the original std::map output order.
const vector<string> STRATEGIES = {
    "random_policy", "scapegoat_array", "scapegoat_dsw", "successor"
};
const vector<string> METRICS = {
    "depth", "local_events_per_pair", "nodes_per_pair",
    "ns_per_operation", "rotations_per_pair", "whole_events"
};

vector<string> splitCSV(const string& line) {
    vector<string> columns;
    istringstream input(line);
    string value;
    while (getline(input, value, ',')) {
        columns.push_back(value);
    }
    return columns;
}

double mean(const vector<double>& values) {
    double sum = 0.0;
    for (double value : values) {
        sum += value;
    }
    return sum / values.size();
}

double sampleSD(const vector<double>& values) {
    double average = mean(values);
    double sumSquared = 0.0;
    for (double value : values) {
        double difference = value - average;
        sumSquared += difference * difference;
    }
    return sqrt(sumSquared / (values.size() - 1));
}

int strategyIndex(const string& name) {
    for (int i = 0; i < static_cast<int>(STRATEGIES.size()); i++) {
        if (STRATEGIES[i] == name) {
            return i;
        }
    }
    throw runtime_error("Unknown strategy: " + name);
}

// Each vector holds one value per independent trial, not per checkpoint.
struct StrategyData {
    vector<double> depthAt[CHECKPOINTS];
    vector<double> finalMetric[6];
};

// A named row avoids confusing r[7] with r[9] elsewhere in the program.
struct BenchmarkRow {
    string strategy;
    int checkpointPairs;
    int seed;
    string traceHash;
    double depth;
    double nsPerOperation;
    double localRebuilds;
    double wholeRebuilds;
    double nodesRebuilt;
    double rotations;
};

BenchmarkRow parseRow(const string& line) {
    vector<string> columns = splitCSV(line);
    if (columns.size() != 14) {
        throw runtime_error("Unexpected CSV schema");
    }
    BenchmarkRow row;
    row.strategy = columns[4];
    row.checkpointPairs = stoi(columns[5]);
    row.seed = stoi(columns[3]);
    row.traceHash = columns[6];
    row.depth = stod(columns[7]);
    row.nsPerOperation = stod(columns[9]);
    row.localRebuilds = stod(columns[10]);
    row.wholeRebuilds = stod(columns[11]);
    row.nodesRebuilt = stod(columns[12]);
    row.rotations = stod(columns[13]);
    return row;
}

void saveFinalMetrics(StrategyData& data, const BenchmarkRow& row) {
    double pairs = row.checkpointPairs;
    data.finalMetric[0].push_back(row.depth);
    data.finalMetric[1].push_back(row.localRebuilds / pairs);
    data.finalMetric[2].push_back(row.nodesRebuilt / pairs);
    data.finalMetric[3].push_back(row.nsPerOperation);
    data.finalMetric[4].push_back(row.rotations / pairs);
    data.finalMetric[5].push_back(row.wholeRebuilds);
}

// Run the benchmark in its own output folder. Its temporary file is trial.csv.
void runBenchmark(const fs::path& outputDir, int n, int pairs,
                  int seed, int trial, int step) {
    string arguments = to_string(n) + " " + to_string(pairs) + " " +
        to_string(seed) + " " + to_string(trial) + " random " + to_string(step);
    fs::path originalDir = fs::current_path();
#ifdef _WIN32
    string binaryName = "churn_benchmark.exe";
    string command = binaryName + " " + arguments + " > trial.csv";
#else
    string binaryName = "churn_benchmark";
    string command = "./" + binaryName + " " + arguments + " > trial.csv";
#endif
    fs::copy_file(originalDir / binaryName, outputDir / binaryName,
                  fs::copy_options::skip_existing);
    fs::current_path(outputDir);
    int status = system(command.c_str());
    fs::current_path(originalDir);
    if (status != 0) {
        throw runtime_error("Benchmark failed");
    }
}

void readTrial(const fs::path& path, ofstream& raw, bool& firstHeader,
               StrategyData data[], int pairs, int seed, int step) {
    ifstream input(path);
    if (!input) {
        throw runtime_error("Cannot read trial CSV");
    }
    string line;
    if (!getline(input, line)) {
        throw runtime_error("Empty trial CSV");
    }
    if (firstHeader) {
        raw << line << '\n';
        firstHeader = false;
    }
    string expectedHash;
    int counts[4] = {0, 0, 0, 0};
    while (getline(input, line)) {
        BenchmarkRow row = parseRow(line);
        int index = strategyIndex(row.strategy);
        counts[index]++;
        if (expectedHash.empty()) {
            expectedHash = row.traceHash;
        }
        // All four strategies must replay the same trace, in the same checkpoints.
        if (row.traceHash != expectedHash || row.seed != seed ||
            row.checkpointPairs != counts[index] * step || counts[index] > CHECKPOINTS) {
            throw runtime_error("Unpaired or missing checkpoint");
        }
        raw << line << '\n';
        int checkpointIndex = counts[index] - 1;
        data[index].depthAt[checkpointIndex].push_back(row.depth);
        if (row.checkpointPairs == pairs) {
            saveFinalMetrics(data[index], row);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (counts[i] != CHECKPOINTS) {
            throw runtime_error("Incomplete trial");
        }
    }
}

void writeSummary(ofstream& summary, const StrategyData data[], int n) {
    for (int i = 0; i < 4; i++) {
        for (int metric = 0; metric < 6; metric++) {
            const vector<double>& values = data[i].finalMetric[metric];
            if (values.size() != TRIALS) {
                throw runtime_error("Missing final trial values");
            }
            summary << n << ',' << STRATEGIES[i] << ',' << METRICS[metric]
                    << ',' << mean(values) << ',' << sampleSD(values) << '\n';
        }
    }
}

// SVG coordinates: y goes down the screen, so larger depth means smaller y.
double graphY(double depth, double maxDepth) {
    return 320.0 - 265.0 * depth / maxDepth;
}

double graphTop(const StrategyData data[], int n) {
    double top = ceil(sqrt(n)) + 2;
    for (int i = 0; i < 4; i++) {
        for (int c = 0; c < CHECKPOINTS; c++) {
            double depth = ceil(mean(data[i].depthAt[c])) + 1;
            top = max(top, depth);
        }
    }
    return top;
}

void writeGraph(const fs::path& dir, const StrategyData data[], int n, int pairs) {
    ofstream svg(dir / ("depth-" + to_string(n) + ".svg"));
    if (!svg) {
        throw runtime_error("Cannot write SVG");
    }
    double top = graphTop(data, n);
    svg << "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 800 410'>"
        << "<rect width='800' height='410' fill='white'/><g font-family='sans-serif'>"
        << "<text x='45' y='25'>Mean node depth (edges), five trials, n = " << n << "</text>";

    double references[] = {2 * log(n), sqrt(n)};
    string referenceNames[] = {"2 ln(n)", "sqrt(n)"};
    for (int i = 0; i < 2; i++) {
        double y = graphY(references[i], top);
        svg << "<path d='M55 " << y << " h560' stroke='gray' stroke-dasharray='4 4'/>"
            << "<text x='620' y='" << y << "'>" << referenceNames[i]
            << " = " << references[i] << "</text>";
    }
    string colors[] = {"#1164ac", "#ca5800", "#008675", "#842599"};
    for (int i = 0; i < 4; i++) {
        svg << "<polyline fill='none' stroke='" << colors[i]
            << "' stroke-width='2' points='";
        for (int c = 0; c < CHECKPOINTS; c++) {
            int completed = (c + 1) * (pairs / CHECKPOINTS);
            double x = 55 + 560.0 * completed / pairs;
            double y = graphY(mean(data[i].depthAt[c]), top);
            svg << x << ',' << y << ' ';
        }
        svg << "'/><text x='" << 45 + 185 * i << "' y='390' fill='"
            << colors[i] << "'>" << STRATEGIES[i] << "</text>";
    }
    svg << "<path d='M55 55 V320 H615' fill='none' stroke='black'/>"
        << "<text x='30' y='60'>" << top << "</text><text x='40' y='324'>0</text>"
        << "<text x='55' y='346'>0</text><text x='240' y='346'>Completed churn pairs</text>"
        << "<text x='575' y='346'>" << pairs << "</text></g></svg>";
}

void studySize(const fs::path& dir, ofstream& raw, ofstream& summary,
               bool& firstHeader, int n) {
    int pairs = n * n;
    int step = pairs / CHECKPOINTS;
    StrategyData data[4];
    for (int trial = 1; trial <= TRIALS; trial++) {
        int seed = 202611160 + 31 * n + trial;
        runBenchmark(dir, n, pairs, seed, trial, step);
        readTrial(dir / "trial.csv", raw, firstHeader, data, pairs, seed, step);
    }
    writeSummary(summary, data, n);
    writeGraph(dir, data, n, pairs);
}

int main(int argc, char** argv) {
    try {
        // Optional --quick: five tiny trials at n=100, not submitted evidence.
        if (argc != 2 && argc != 3) {
            throw runtime_error("Pass a new results directory [--quick]. Run from the folder containing churn_benchmark.");
        }
        bool quick = false;
        if (argc == 3) {
            if (string(argv[2]) != "--quick") {
                throw runtime_error("Unknown option");
            }
            quick = true;
        }
        fs::path dir = fs::absolute(argv[1]);
        if (!fs::create_directories(dir)) {
            throw runtime_error("Choose a fresh directory.");
        }
        ofstream raw(dir / "raw.csv");
        ofstream summary(dir / "summary.csv");
        if (!raw || !summary) {
            throw runtime_error("Cannot open output CSVs");
        }
        summary << "n,strategy,metric,mean,sample_sd\n";
        bool firstHeader = true;
        if (quick) {
            studySize(dir, raw, summary, firstHeader, 100);
        } else {
            int sizes[] = {1000, 2000, 4000};
            for (int n : sizes) {
                studySize(dir, raw, summary, firstHeader, n);
            }
        }
        return 0;
    } catch (const exception& error) {
        cerr << error.what() << '\n';
        return 1;
    }
}
