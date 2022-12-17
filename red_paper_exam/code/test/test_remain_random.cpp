#include <cstdint>
#include <ctime>
#include <iostream>
#include "remain_random.h"
using namespace std;

int main (int argc, char *argv[])
{
  srand(time(0));
  vector<pair<uint16_t, double>> vec;
  simulate(10.0, 10, vec);
  for(auto item: vec)
  {
    // cout << "num: " << item.first << " ----> money: " << item.second << endl;
    printf("player[%02d] ------> %.2lf\n", item.first, item.second);
  }
  return 0;
}
