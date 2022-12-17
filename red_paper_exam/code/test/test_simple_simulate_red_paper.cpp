#include "double_mean.h"
#include <ctime>
#include <random>
#include <iostream>
#include <string>
#include <thread>

unsigned long timestamp()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void send_and_grab_red_paper_in_single_thread(double money, int num)
{
  double remain_money = money;
  auto start_time = timestamp();
  for(int i = num; i > 0; i--)
  {
    double money = std::calculate_one_paper_by_double_mean(remain_money, i);
    remain_money -= money;
    printf("play[%02d]抢到红包[%.2lf]\n", i, money);
  }
  // 假装很激烈
  // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  printf("耗时: %ldmm\n", timestamp() - start_time);
}

int main (int argc, char *argv[])
{
  double money = 100.0;
  int num = 10;
  if(argc == 3) {
    money = std::stod(argv[1]);
    num = std::stoi(argv[2]);
  }
  auto t = std::thread(send_and_grab_red_paper_in_single_thread, money, num);
  t.join();
  return 0;
}
