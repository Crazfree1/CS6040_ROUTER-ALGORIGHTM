#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <iomanip>  // For std::fixed and std::setprecision
#include <bits/stdc++.h>
using namespace std;

#define MIN_R 100            // Minimum number of connection requests
#define MAX_R 300            // Maximum number of connection requests
#define MIN_BANDWIDTH 1   // Minimum bandwidth value
#define MAX_BANDWIDTH 8   // Maximum bandwidth value

int generateRandomNumber(int minValue, int maxValue, int exclude1 = -1, int exclude2 = -1) {
    int num;
    do {
        num = minValue + rand() % (maxValue - minValue + 1);
    } while (num == exclude1 || num == exclude2);
    return num;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: " << argv[0] << " <MAX_NODE>" <<" <CONNECTION REQS>" << "filename"<< endl;
        return 1;
    }
    int MAX_NODE = atoi(argv[1]);
    int R = atoi(argv[2]);

    ofstream outfile(argv[3]);
    srand(static_cast<unsigned int>(time(0)));
    
    outfile << R << endl;
    
    for (int i = 0; i < R; ++i) {
        int source = rand() % (MAX_NODE);
        int destination = rand() % (MAX_NODE);
        while (source == destination) {
            destination =  rand() % (MAX_NODE);
        }
        int a = generateRandomNumber(MIN_BANDWIDTH, MAX_BANDWIDTH);
        int b = generateRandomNumber(MIN_BANDWIDTH, MAX_BANDWIDTH, a);
        int c = generateRandomNumber(MIN_BANDWIDTH, MAX_BANDWIDTH, a, b);

        int nums[3] = {a, b, c};
        sort(nums, nums + 3);
        
        outfile << source << " " << destination << " "
                << nums[0] << " " << nums[1] << " " << nums[2] << endl;
    }
    
    outfile.close();
    return 0;
}
