#include "double_mean.h"
#include <ctime>
#include <random>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <future>
#include <vector>
#include <ThreadPool.h>

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
    printf("player[%02d]抢到了[%.2lf]元!!!, 耗时%ldms\n", num, grab_money, timestamp() - start);
  }
};

void send_and_grab_red_paper_in_single_thread(double money, int num)
{
  RedPaper rp(money, num);
  std::vector<std::future<void>> vec;
  auto func_grab = std::bind(&RedPaper::grab, &rp, std::placeholders::_1);
  // 线程池中预设8个工作线程
  ThreadPool pool(8);
  pool.init();

  auto start_time = timestamp();
  // 主线程模拟路由1000次请求
  for(int i = 1000; i > 0; i--)
  {
    auto f = pool.submit(func_grab, timestamp());
    vec.push_back(std::move(f));
  }
  printf("循环耗时: %ldms\n", timestamp() - start_time);
  // 等待全部请求结束
  for(auto &f: vec)
    f.get();
  pool.shutdown();

  printf("总金额: %.2lf\n", rp.sum);
  // 假装很激烈
  // std::this_thread::sleep_for(std::chrono::milliseconds(500));
  printf("耗时: %ldms\n", timestamp() - start_time);
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
