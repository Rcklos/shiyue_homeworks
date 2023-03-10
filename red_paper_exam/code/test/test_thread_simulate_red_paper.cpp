#include "double_mean.h"
#include <ctime>
#include <random>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

unsigned long timestamp()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

class RedPaper {
private:
  double money;
  int num;
  std::mutex m_mutex;
public:
  double sum = 0;
  RedPaper(double money, int num): money(money), num(num) {}
  void grab(unsigned long start)
  {
    std::unique_lock<std::mutex> lock(m_mutex);
    if(num == 0) return;
    num--;
    double grab_money = std::calculate_one_paper_by_double_mean(money, num + 1);
    money -= grab_money;
    sum += grab_money;
    lock.unlock();
    printf("player[%02d]抢到了[%.2lf]元!!!!!, 耗时%ldms\n", num, grab_money, timestamp() - start);
  }
};

void send_and_grab_red_paper_in_single_thread(double money, int num)
{
  RedPaper rp(money, num);
  auto start_time = timestamp();
  std::vector<std::thread> vec;
  for(int i = 1000; i > 0; i--)
  {
    std::thread t(&RedPaper::grab, &rp, timestamp());
    vec.push_back(std::move(t));
  }
  printf("循环耗时: %ldms\n", timestamp() - start_time);
  for(auto &t: vec)
    t.join();
  printf("sum: %.2lf\n", rp.sum);
  printf("总耗时: %ldms\n", timestamp() - start_time);
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
