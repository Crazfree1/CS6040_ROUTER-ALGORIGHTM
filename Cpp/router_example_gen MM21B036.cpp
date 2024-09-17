#include <random>
#include <iostream>
#include <fstream>

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(100, 3000);
  std::uniform_int_distribution<int> ARPANET_dist(0, 19);
  std::uniform_int_distribution<int> NSFNET_dist(0, 11);
  
  std::vector<int> ARPA_source_vec;
  std::vector<int> ARPA_dest_vec;
  std::vector<int> ARPA_b_1_vec;
  std::vector<int> ARPA_b_2_vec;
  std::vector<int> ARPA_b_3_vec;

  std::vector<int> NSF_source_vec;
  std::vector<int> NSF_dest_vec;
  std::vector<int> NSF_b_1_vec;
  std::vector<int> NSF_b_2_vec;
  std::vector<int> NSF_b_3_vec;
  
  int count = dist(gen) % 300;
  for (int i = 0; i < count; ++i) {
    
    ARPA_source_vec.push_back(ARPANET_dist(gen));
    ARPA_dest_vec.push_back(ARPANET_dist(gen));
    ARPA_b_1_vec.push_back(dist(gen));
    ARPA_b_2_vec.push_back(dist(gen));
    ARPA_b_3_vec.push_back(dist(gen));

        
    NSF_source_vec.push_back(NSFNET_dist(gen));
    NSF_dest_vec.push_back(NSFNET_dist(gen));
    NSF_b_1_vec.push_back(dist(gen));
    NSF_b_2_vec.push_back(dist(gen));
    NSF_b_3_vec.push_back(dist(gen));
  }

  std::ofstream ARPA("ARPANET-Example.txt");
  std::ofstream NSF("NSFNET-Example.txt");

  if (!ARPA) {
    std::cout << "Could NOT open ARPANET_example.txt file.\n";
    return 1;
  }
  if (!NSF) {
    std::cout << "Could NOT open NSFNET_example.txt file.\n";
    return 1;
  }

  for (int i =0; i < count-1; ++i) {
    ARPA << ARPA_source_vec.at(i) << ' ';
    ARPA << ARPA_dest_vec.at(i) << ' ';
    ARPA << ARPA_b_1_vec.at(i) << ' ';
    ARPA << ARPA_b_2_vec.at(i) << ' ';
    ARPA << ARPA_b_3_vec.at(i) << '\n';

    NSF << NSF_source_vec.at(i) << ' ';
    NSF << NSF_dest_vec.at(i) << ' ';
    NSF << NSF_b_1_vec.at(i) << ' ';
    NSF << NSF_b_2_vec.at(i) << ' ';
    NSF << NSF_b_3_vec.at(i) << '\n';
  }

  ARPA << ARPA_source_vec.at(count-1) << ' ';
  ARPA << ARPA_dest_vec.at(count-1) << ' ';
  ARPA << ARPA_b_1_vec.at(count-1) << ' ';
  ARPA << ARPA_b_2_vec.at(count-1) << ' ';
  ARPA << ARPA_b_3_vec.at(count-1);

  NSF << NSF_source_vec.at(count-1) << ' ';
  NSF << NSF_dest_vec.at(count-1) << ' ';
  NSF << NSF_b_1_vec.at(count-1) << ' ';
  NSF << NSF_b_2_vec.at(count-1) << ' ';
  NSF << NSF_b_3_vec.at(count-1);

  ARPA.close();
  NSF.close();
  
  return 0;
}
