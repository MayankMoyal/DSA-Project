#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <numeric>
using namespace std;
namespace fs = std::filesystem;

vector<string> fields(const string& line) {
    vector<string> result;
    istringstream input(line);
    string value;
    while (getline(input,value,',')) result.push_back(value);
    return result;
}
double mean(const vector<double>& v) {
    return accumulate(v.begin(),v.end(),0.0)/v.size();
}
double sd(const vector<double>& v) {
    double sum=0, center=mean(v);
    for(double x:v) sum+=(x-center)*(x-center);
    return sqrt(sum/(v.size()-1));
}
int main(int argc,char** argv) {
    try {
        if(argc!=2) throw runtime_error("Pass a new results directory. Run from the folder containing churn_benchmark.");
        fs::path dir=argv[1];
        if(!fs::create_directories(dir)) throw runtime_error("Choose a fresh directory.");
        ofstream raw(dir/"raw.csv"), summary(dir/"summary.csv");
        summary<<"n,strategy,metric,mean,sample_sd\n";
        bool header=true;
        for(int n:{1000,2000,4000}) {
            int pairs=n*n, step=pairs/100;
            map<string,map<int,vector<double>>> depths;
            map<string,map<string,vector<double>>> final;
            for(int trial=1;trial<=5;++trial) {
                int seed=202611160+31*n+trial;
                string args=to_string(n)+" "+to_string(pairs)+" "+to_string(seed)+" "+to_string(trial)+" random "+to_string(step);
                // Run from the private output folder so the temporary CSV is unique to this study.
                fs::path root=fs::current_path();
#ifdef _WIN32
                auto binary=root/"churn_benchmark.exe";
#else
                auto binary=root/"churn_benchmark";
#endif
                fs::copy_file(binary,dir/binary.filename(),fs::copy_options::skip_existing);
                fs::current_path(dir);
#ifdef _WIN32
                int status=system(("churn_benchmark.exe "+args+" > trial.csv").c_str());
#else
                int status=system(("./churn_benchmark "+args+" > trial.csv").c_str());
#endif
                fs::current_path(root);
                if(status) throw runtime_error("Benchmark failed");
                ifstream in(dir/"trial.csv");
                string line,hash;
                getline(in,line);
                if(header){raw<<line<<'\n'; header=false;}
                map<string,int> counts;
                while(getline(in,line)){
                    auto r=fields(line);
                    if(r.size()!=14) throw runtime_error("Unexpected CSV schema");
                    string strategy=r[4];
                    int p=stoi(r[5]);
                    if(hash.empty()) hash=r[6];
                    if(hash!=r[6] || p!=++counts[strategy]*step || stoi(r[3])!=seed)
                        throw runtime_error("Unpaired or missing checkpoint");
                    raw<<line<<'\n';
                    depths[strategy][p].push_back(stod(r[7]));
                    if(p==pairs){
                        auto& m=final[strategy];
                        m["depth"].push_back(stod(r[7]));
                        m["ns_per_operation"].push_back(stod(r[9]));
                        m["local_events_per_pair"].push_back(stod(r[10])/p);
                        m["whole_events"].push_back(stod(r[11]));
                        m["nodes_per_pair"].push_back(stod(r[12])/p);
                        m["rotations_per_pair"].push_back(stod(r[13])/p);
                    }
                }
                for(string name:{"successor","random_policy","scapegoat_array","scapegoat_dsw"})
                    if(counts[name]!=100) throw runtime_error("Incomplete trial");
            }
            for(auto& [name,metrics]:final) for(auto& [metric,values]:metrics)
                summary<<n<<','<<name<<','<<metric<<','<<mean(values)<<','<<sd(values)<<'\n';
            ofstream svg(dir/("depth-"+to_string(n)+".svg"));
            double top=ceil(sqrt(n))+2;
            for(auto& [name,points]:depths) for(auto& [p,v]:points) top=max(top,ceil(mean(v))+1);
            auto y=[top](double v){return 320-265*v/top;};
            svg<<"<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 800 410'><rect width='800' height='410' fill='white'/><g font-family='sans-serif'><text x='45' y='25'>Mean node depth (edges), five trials, n = "<<n<<"</text>";
            for(double ref:{2*log(n),sqrt(n)}) svg<<"<path d='M55 "<<y(ref)<<" h560' stroke='gray' stroke-dasharray='4 4'/><text x='620' y='"<<y(ref)<<"'>"<<(ref==sqrt(n)?"sqrt(n)":"2 ln(n)")<<" = "<<ref<<"</text>";
            vector<string> colors={"#1164ac","#ca5800","#008675","#842599"};
            int index=0;
            for(auto& [name,points]:depths){
                svg<<"<polyline fill='none' stroke='"<<colors[index]<<"' stroke-width='2' points='";
                for(auto& [p,v]:points) svg<<55+560.0*p/pairs<<','<<y(mean(v))<<' ';
                svg<<"'/><text x='"<<45+185*index<<"' y='390' fill='"<<colors[index]<<"'>"<<name<<"</text>";
                ++index;
            }
            svg<<"<path d='M55 55 V320 H615' fill='none' stroke='black'/><text x='30' y='60'>"<<top<<"</text><text x='40' y='324'>0</text><text x='55' y='346'>0</text><text x='240' y='346'>Completed churn pairs</text><text x='575' y='346'>"<<pairs<<"</text></g></svg>";
        }
        return 0;
    } catch(const exception& e){cerr<<e.what()<<'\n';return 1;}
}
