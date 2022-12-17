#include <cstdint>
#include <ctime>
#include <iostream>
#include "double_mean.h"
using namespace std;

int main (int argc, char *argv[])
{
  srand(time(0));
  vector<pair<uint16_t, double>> vec;
  simulate_by_doble_mean(10.0, 10, vec);
  double sum = 0;
  for(auto item: vec)
  {
    // cout << "num: " << item.first << " ----> money: " << item.second << endl;
    printf("player[%02d] ------> %.2lf\n", item.first, item.second);
    sum += item.second;
  }
  printf("sum: %.2lf\n", sum);
  // for(size_t i = 0; i < vec.size(); i++)
  // {
  //   printf("%.2lf ", vec[i].second);
  //   if(i != vec.size() - 1)
  //     cout << "+ ";
  //   sum += vec[i].second;
  // }
  // printf("= %.2lf\n", sum);
  return 0;
}
