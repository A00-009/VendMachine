#ifndef VENDING_LOGIC_H
#define VENDING_LOGIC_H

#include "app.h"

/* 釣銭・購入処理 */
ErrorCode calculate_change_greedy(
    int change_amount,
    const Money *money,
    size_t money_count,
    Money *payout,
    size_t payout_count,
    int *shortage_value
);

ErrorCode apply_payout(Money *money, size_t money_count, const Money *payout, size_t payout_count);
ErrorCode can_buy_item(const Item *item, int inserted_amount);
int find_money_index(const Money *money, size_t money_count, int value);
int get_min_buyable_item_price(const Item *items, size_t item_count);

#endif /* VENDING_LOGIC_H */
