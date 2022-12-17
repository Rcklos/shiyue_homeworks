#ifndef __REMAIN_RANDOM_H__
#define __REMAIN_RANDOM_H__

#include <stdint.h>
#include <vector>

#ifndef MIN_MONEY
#define MIN_MONEY 0.01
#endif

namespace std {

double calculate_one_paper(double remain_money, uint16_t remain_num);
bool simulate(double money, uint16_t num, vector<pair<uint16_t, double>> &vec);

};
#endif /* __REMAIN_RANDOM_H__ */
