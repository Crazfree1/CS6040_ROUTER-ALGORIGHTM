#include <bits/stdc++.h>
#include <random>
using namespace std;

int main(int argc, char* argv[]) {
    // Step 1: Create a random device to seed the random number generator
    std::random_device rd;

    // Step 2: Initialize a random number engine (Mersenne Twister)
    std::mt19937 gen(rd());

    // Step 3: Define a distribution (uniform distribution between 1 and 100)
    

    string connectionsFile = argv[1];

    ofstream outputFile(connectionsFile); 

    if (!outputFile.is_open()) { 
        cerr << "Error opening the file!" << endl; 
        return 1; 
    }

    int N = stoi(argv[2]);
    std::uniform_int_distribution<> distrib(100, 300);
    int random_number = stoi(argv[3]);
    outputFile<<random_number<<endl;

    for (int i = 0; i < random_number; ++i) {
        std::uniform_int_distribution<> distrib1(0, N - 1);
        int src = distrib1(gen);
        int dst = distrib1(gen);
        while(dst == src){
            dst = distrib1(gen);
        }

        std::uniform_real_distribution<> distrib2(1, 4);
        double b_min = distrib2(gen);

        std::uniform_real_distribution<> distrib3(1.2 * b_min, 1.5 * b_min);
        double b_avg = distrib3(gen);

        std::uniform_real_distribution<> distrib4(1.2 * b_avg, 1.5 * b_avg);
        double b_max = distrib4(gen);

        outputFile<<src<<" "<<dst<<" "<<(int)b_min<<" "<<ceil(b_avg)<<" "<<ceil(b_max)<<endl;
    }

    outputFile.close();
    return 0;
}
