#include "double_mean.h"
#include <ctime>
#include <queue>
#include <random>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <future>
#include <vector>
#include <ThreadPool.h>
#include <cofish/coroutine.h>

unsigned long timestamp()
{
  return std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

class RedPaper {
private:
  double money;
  // 写成不需要竞争资源
  // std::mutex m_mutex;
public:
  int num;
  double sum = 0;
  RedPaper(double money, int num): money(money), num(num) {}
  void grab()
  {
    if(num == 0) return;
    num--;
    double grab_money = std::calculate_one_paper_by_double_mean(money, num + 1);
    money -= grab_money;
    sum += grab_money;
    printf("player[%02d]抢到了[%.2lf]元!!!!!\n", num, grab_money);
  }
};

/**
* 路由协程不做任何判断，只分配推入请求到队列中交由工作协程处理
*/
void coroutine_route(std::queue<char> &mq) {
  for(int i = 0; i < 1000; i++) {
    // 推进去一千次请求
    mq.push(0);
  }
  while(true) fish::Coroutine::Yield();
}

void coroutine_work(std::queue<char> &mq, RedPaper &rp, int &count) {
  while(true) {
    while(mq.empty()) {
      fish::Coroutine::Yield();
    }
    // 模拟取出请求
    mq.pop();
    // 抢红包
    rp.grab();
    count++;
  }
}

void send_and_grab_red_paper_in_single_thread(double money, int num)
{
  RedPaper rp(money, num);

  auto start_time = timestamp();
  std::vector<fish::Coroutine::Ptr> vec;
  auto func = std::bind(&RedPaper::grab, &rp);
  fish::Coroutine::StackMem share_mem;
  std::queue<char> mq;
  int count = 0;
  // 新建8个协程作为接待
  for(int i = 8; i > 0; i--)
  {
    auto co = fish::Coroutine::Create(std::bind(coroutine_work, std::ref(mq), std::ref(rp), std::ref(count)), &share_mem);
    vec.push_back(co);
  }
  vec.push_back(fish::Coroutine::Create(std::bind(coroutine_route, std::ref(mq)), &share_mem));
  while(count < 1000) {
    for(auto it = vec.begin(); it != vec.end(); it++)
      (*it)->Resume();
  }
  printf("sum: %.2lf\n", rp.sum);
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
