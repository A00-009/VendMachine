#include <limits.h>

#include "vending_logic.h"
#include "csv_io.h"

ErrorCode calculate_change_greedy(
    int change_amount,
    const Money* money,
    size_t money_count,
    Money* payout,
    size_t payout_count,
    int* shortage_value
)
{
    int remaining_change = change_amount;
    int shortage_money_value = 0;

    if (money == NULL || payout == NULL || shortage_value == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (money_count != payout_count) {
        return ERROR_INVALID_INPUT;
    }

    if (change_amount < 0) {
        return ERROR_INVALID_INPUT;
    }

    *shortage_value = 0;

    for (size_t i = 0; i < payout_count; i++) {
        payout[i].value = money[i].value;
        payout[i].count = 0;
    }

    /*
        money は 10, 50, 100, 500, 1000 の昇順で
        格納されている前提。
        高額金種から順に払い出し枚数を決める。
    */
    for (int i = (int)money_count - 1; i >= 0; i--) {
        int money_value = money[i].value;
        int available_count = money[i].count;
        int needed_count;
        int payout_count_value;

        if (money_value <= 0 || available_count < 0) {
            return ERROR_INVALID_MONEY;
        }

        needed_count = remaining_change / money_value;

        /*
            この金種が必要だが、枚数が足りない場合、
            釣銭切れの金種として記録する。

            ただし、この時点では下位金種で代替できる可能性があるため、
            すぐには ERROR_CHANGE_SHORTAGE を返さない。
        */
        if (needed_count > available_count) {
            shortage_money_value = money_value;
        }

        payout_count_value =
            (needed_count > available_count) ? available_count : needed_count;

        payout[i].count = payout_count_value;

        remaining_change -= payout_count_value * money_value;
    }

    /*
        全金種を使っても釣銭を作れない場合、
        不足した金種を shortage_value に入れる。
    */
    if (remaining_change > 0) {
        *shortage_value = shortage_money_value;
        return ERROR_CHANGE_SHORTAGE;
    }

    return ERROR_NONE;
}

ErrorCode apply_payout(
    Money* money,
    size_t money_count,
    const Money* payout,
    size_t payout_count
)
{
    if (money == NULL || payout == NULL) {
        return ERROR_NULL_POINTER;
    }

    if (money_count != payout_count) {
        return ERROR_INVALID_INPUT;
    }

    for (size_t i = 0; i < money_count; i++) {
        if (money[i].value <= 0 || payout[i].value <= 0) {
            return ERROR_INVALID_MONEY;
        }

        if (money[i].value != payout[i].value) {
            return ERROR_INVALID_MONEY;
        }

        if (money[i].count < 0 || payout[i].count < 0) {
            return ERROR_INVALID_MONEY;
        }

        if (money[i].count < payout[i].count) {
            return ERROR_CHANGE_SHORTAGE;
        }
    }

    for (size_t i = 0; i < money_count; i++) {
        money[i].count -= payout[i].count;
    }

    return ERROR_NONE;
}

ErrorCode can_buy_item(const Item* item, int inserted_amount)
{
    // 商品構造体がNULLでないことを確認する
    if (item == NULL)
        return ERROR_NULL_POINTER;
    // 商品が売り切れでないことを確認する
    if (item->stock <= 0)
        return ERROR_SOLD_OUT;
    // 入金額が商品単価を上回っているか確認する
    if (inserted_amount < item->price)
        return ERROR_NOT_ENOUGH_AMOUNT;
    // 正常終了
    return ERROR_NONE;
}

int find_money_index(const Money* money, size_t money_count, int value)
{
    // 金銭構造体がNULLでないことを確認する
    if (money == NULL) return -1;
    // 金額を金銭構造体のインデックスに変換
    for (size_t i = 0; i < money_count; i++)
        if (money[i].value == value) return (int)i;
    return -1;
}

int get_min_buyable_item_price(const Item* items, size_t item_count)
{
    // 商品構造体がNULLでないことを確認する
    if (items == NULL) return -1;

    // 最小額を初期化
    int min = INT_MAX;

    // 価格が最小の商品を探す
    for (size_t i = 0; i < item_count; i++)
        if (items[i].stock > 0 && items[i].price < min)
            // 最小額を更新
            min = items[i].price;

    // 最小額に何らかの値が挿入されていたら正常終了
    return (min == INT_MAX) ? -1 : min;
}
