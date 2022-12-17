#include "remain_random.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

double std::calculate_one_paper(double remain_money, uint16_t remain_num)
{
  if(remain_num == 1) return remain_money;
  double cal = MIN_MONEY + 
    ceil((100 * ((double)rand()) / ((double)(RAND_MAX / remain_money - MIN_MONEY)))) / 100;
  // 判断剩余金额是否足够给剩余用户分配
  double remain = remain_money - cal;
  if(remain < MIN_MONEY * (remain_num - 1))
    // 不足则给其他用户分配"低保"
    cal -= MIN_MONEY * (remain_num - 1) - remain;
  return cal;
}

bool std::simulate(double money, uint16_t num, vector<pair<uint16_t, double>> &vec)
{
  double cal;
  while(num) 
  {
    cal = calculate_one_paper(money, num);
    money -= cal;
    vec.push_back(pair<uint16_t, double>(num--, cal));
  }
  return false;
}
