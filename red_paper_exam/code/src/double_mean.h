#ifndef __DOUBLE_MEAN_H__
#define __DOUBLE_MEAN_H__

#include <stdint.h>
#include <vector>

#ifndef MIN_MONEY
#define MIN_MONEY 0.01
#endif

namespace std {

double calculate_one_paper_by_double_mean(double remain_money, uint16_t remain_num);
bool simulate_by_doble_mean(double money, uint16_t num, vector<pair<uint16_t, double>> &vec);

};
#endif /* __DOUBLE_MEAN_H__ */
