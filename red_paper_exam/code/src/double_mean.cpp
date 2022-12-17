#include "double_mean.h"
#include <random>
#include <cmath>
using namespace std;

double std::calculate_one_paper_by_double_mean(double remain_money, uint16_t remain_num)
{
    if(remain_num == 1) return remain_money;
    // 随机数
    random_device rd{};
    mt19937 engine{rd()};
    // 均匀分布
    uniform_real_distribution<double> dist{0.0, 1.0};
    // 随机数范围
    double random_number_between_0_and_1 = dist(engine);
    // 保底0.01
    double cal = 0.01 + ceil(100 * (random_number_between_0_and_1 * (2 * (remain_money / remain_num) - 0.01))) / 100;
    return cal;
}

bool std::simulate_by_doble_mean(double money, uint16_t num, vector<pair<uint16_t, double>> &vec)
{
  double cal;
  while(num) 
  {
    cal = calculate_one_paper_by_double_mean(money, num);
    money -= cal;
    vec.push_back(pair<uint16_t, double>(num--, cal));
  }
  return false;
}
