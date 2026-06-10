#include <stdio.h>
#include <string.h>

#include "input.h"

ErrorCode input_line(char *buffer, size_t size)
{
    size_t length;

    /* 1. NULLチェック */
    if (buffer == NULL) {
        return ERROR_NULL_POINTER;
    }

    /* 2. バッファサイズチェック */
    if (size == 0) {
        return ERROR_INVALID_INPUT;
    }

    /* 3. 標準入力から文字列を読み取る */
    if (fgets(buffer, size, stdin) == NULL) {
        return ERROR_INVALID_INPUT;
    }

    /* 4. 末尾の改行文字 '\n' を削除 */
    length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n') {
        buffer[length - 1] = '\0';
    }

    /* 5. 末尾のキャリッジリターン '\r' を削除 */
    length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\r') {
        buffer[length - 1] = '\0';
    }

    /* 6. 正常終了 */
    return ERROR_NONE;
}

ErrorCode parse_input_event(
    const char* input,
    AppState state,
    InputEvent* out_event,
    int* out_value
)
{
    char key;

    /* 1. NULLチェック */
    if (input == NULL || out_event == NULL || out_value == NULL) {
        return ERROR_NULL_POINTER;
    }

    /* 2. 初期値設定 */
    *out_event = INPUT_EVENT_NONE;
    *out_value = 0;

    /* 3. 状態チェック */
    if (state != APP_STATE_WAITING) {
        *out_event = INPUT_EVENT_INVALID;
        return ERROR_INVALID_INPUT;
    }

    /* 4. 空入力チェック */
    if (input[0] == '\0') {
        *out_event = INPUT_EVENT_INVALID;
        return ERROR_INVALID_INPUT;
    }

    /* 5. 1文字入力か確認 */
    if (input[1] != '\0') {
        *out_event = INPUT_EVENT_INVALID;
        return ERROR_INVALID_INPUT;
    }

    key = input[0];

    /* 6. 金銭投入キー */
    if (key >= '1' && key <= '5') {
        *out_event = INPUT_EVENT_DEPOSIT;
        *out_value = money_key_to_value(key);

        if (*out_value <= 0) {
            *out_event = INPUT_EVENT_INVALID;
            *out_value = 0;
            return ERROR_INVALID_MONEY;
        }

        return ERROR_NONE;
    }

    /* 7. 商品選択キー */
    if ((key >= 'A' && key <= 'E') || (key >= 'a' && key <= 'e')) {
        *out_event = INPUT_EVENT_SELECT_ITEM;
        *out_value = item_key_to_index(key);

        if (*out_value < 0) {
            *out_event = INPUT_EVENT_INVALID;
            *out_value = 0;
            return ERROR_INVALID_ITEM;
        }

        return ERROR_NONE;
    }

    /* 8. 終了キー */
    if (key == '9') {
        *out_event = INPUT_EVENT_EXIT;
        *out_value = 9;
        return ERROR_NONE;
    }

    /* 9. その他は不正入力 */
    *out_event = INPUT_EVENT_INVALID;
    *out_value = 0;

    return ERROR_INVALID_INPUT;
}

ErrorCode handle_input_event(
    AppContext* context,
    InputEvent event,
    int value,
    Item* items,
    size_t item_count,
    Money* money,
    size_t money_count
)
{
    ErrorCode error;
    int money_index;
    int item_index;
    int remaining_amount;
    int min_buyable_price;
    Money payout[MONEY_COUNT] = { 0 };
    int shortage_value = 0;

    /* 1. NULLチェック */
    if (context == NULL || items == NULL || money == NULL) {
        return ERROR_NULL_POINTER;
    }

    /* 2. 共通の初期化 */
    context->error = ERROR_NONE;
    context->selected_item_index = -1;
    context->refund_amount = 0;
    context->change_shortage_value = 0;
    context->money_limit_value = 0;

    switch (event) {
    case INPUT_EVENT_DEPOSIT:
        money_index = find_money_index(money, money_count, value);
        if (money_index < 0) {
            context->error = ERROR_INVALID_MONEY;
            context->view = UI_VIEW_ERROR;
            return ERROR_INVALID_MONEY;
        }

        if (value == 1000) {
            if (context->inserted_money_count[money_index] >= BILL_LIMIT) {
                context->error = ERROR_MONEY_LIMIT_OVER;
                context->view = UI_VIEW_ERROR;
                context->money_limit_value = value;
                return ERROR_MONEY_LIMIT_OVER;
            }
        }
        else {
            if (context->inserted_money_count[money_index] >= COIN_LIMIT) {
                context->error = ERROR_MONEY_LIMIT_OVER;
                context->view = UI_VIEW_ERROR;
                context->money_limit_value = value;
                return ERROR_MONEY_LIMIT_OVER;
            }
        }

        money[money_index].count++;
        context->inserted_money_count[money_index]++;
        context->inserted_amount += value;

        context->view = UI_VIEW_DEPOSITED;
        context->error = ERROR_NONE;

        return ERROR_NONE;

        case INPUT_EVENT_SELECT_ITEM:
        /*
            value には商品インデックスが入っている想定
            A -> 0, B -> 1, ...
        */
        item_index = value;

        if (item_index < 0 || (size_t)item_index >= item_count) {
            context->error = ERROR_INVALID_ITEM;
            context->view = UI_VIEW_ERROR;
            return ERROR_INVALID_ITEM;
        }

        context->selected_item_index = item_index;

        /*
            購入可能か確認する。
            ここで売切、金額不足などを判定する。
            まだ在庫は減らさない。
        */
        error = can_buy_item(&items[item_index], context->inserted_amount);
        if (error != ERROR_NONE) {
            context->error = error;
            context->view = UI_VIEW_ERROR;
            return error;
        }

        /*
            購入後の残額を計算する。
            ここでもまだ在庫は減らさない。
        */
        remaining_amount = context->inserted_amount - items[item_index].price;

        /*
            購入後の在庫状態を一時的に作って、
            「購入後も残額で別商品を買えるか」を判定する。
            実データの items はまだ変更しない。
        */
        Item temp_items[ITEM_COUNT];

        if (item_count > ITEM_COUNT) {
            context->error = ERROR_INVALID_INPUT;
            context->view = UI_VIEW_ERROR;
            return ERROR_INVALID_INPUT;
        }

        for (size_t i = 0; i < item_count; i++) {
            temp_items[i] = items[i];
        }

        temp_items[item_index].stock--;

        min_buyable_price = get_min_buyable_item_price(temp_items, item_count);

        /*
            残額でまだ商品を購入できる場合：
            - 釣銭払い出しは不要
            - ここで初めて在庫を減らす
            - 残額は inserted_amount に残す
        */
        if (min_buyable_price > 0 && remaining_amount >= min_buyable_price) {
            items[item_index].stock--;

            context->inserted_amount = remaining_amount;
            context->refund_amount = 0;
            context->view = UI_VIEW_PURCHASED;
            context->error = ERROR_NONE;

            error = save_items_csv(ITEMS_CSV, items, item_count);
            if (error != ERROR_NONE) {
                context->error = error;
                context->view = UI_VIEW_ERROR;
                return error;
            }

            return ERROR_NONE;
        }

        /*
            残額でこれ以上商品を購入できない場合：
            - 残額を釣銭として払い出す必要がある
            - まず釣銭を作れるか確認する
            - 失敗した場合、在庫は減らさずに ERROR_CHANGE_SHORTAGE を返す
        */
        error = calculate_change_greedy(
            remaining_amount,
            money,
            money_count,
            payout,
            money_count,
            &shortage_value
        );

        if (error != ERROR_NONE) {
            context->error = error;
            context->change_shortage_value = shortage_value;
            context->view = UI_VIEW_ERROR;
            return error;
        }

        /*
            釣銭を実際に払い出す。
            ここで失敗した場合も、まだ在庫は減らしていない。
        */
        error = apply_payout(
            money,
            money_count,
            payout,
            money_count
        );

        if (error != ERROR_NONE) {
            context->error = error;
            context->view = UI_VIEW_ERROR;
            return error;
        }

        /*
            釣銭払い出しまで成功したので、
            ここで初めて購入成立として在庫を減らす。
        */
        items[item_index].stock--;

        /*
            商品在庫と金銭枚数が変わったため、両方保存する。
        */
        error = save_items_csv(ITEMS_CSV, items, item_count);
        if (error != ERROR_NONE) {
            context->error = error;
            context->view = UI_VIEW_ERROR;
            return error;
        }

        error = save_money_csv(MONEY_CSV, money, money_count);
        if (error != ERROR_NONE) {
            context->error = error;
            context->view = UI_VIEW_ERROR;
            return error;
        }

        context->refund_amount = remaining_amount;
        context->inserted_amount = 0;

        for (size_t i = 0; i < MONEY_COUNT; i++) {
            context->inserted_money_count[i] = 0;
        }

        context->view = UI_VIEW_PURCHASED;
        context->error = ERROR_NONE;

        return ERROR_NONE;

    case INPUT_EVENT_EXIT:
        /*
            9入力時。
            入金済みなら返金額を表示して終了。
        */
        context->refund_amount = context->inserted_amount;
        context->inserted_amount = 0;

        for (size_t i = 0; i < MONEY_COUNT; i++) {
            context->inserted_money_count[i] = 0;
        }

        context->state = APP_STATE_EXITING;
        context->view = UI_VIEW_EXIT;
        context->error = ERROR_NONE;

        return ERROR_NONE;

    case INPUT_EVENT_INVALID:
        context->error = ERROR_INVALID_INPUT;
        context->view = UI_VIEW_ERROR;
        return ERROR_INVALID_INPUT;

    case INPUT_EVENT_NONE:
    default:
        context->error = ERROR_INVALID_INPUT;
        context->view = UI_VIEW_ERROR;
        return ERROR_INVALID_INPUT;
    }
}

int money_key_to_value(char key)
{
    // 金銭テーブル
    int table[] = { 10, 50, 100, 500, 1000 };

    // 金銭キーを金額に変換
    if (key >= '1' && key <= '5')
        return table[key - '1'];
    return -1;
}

int item_key_to_index(char key)
{
    // 小文字の商品キーを商品インデックスに変換
    if (key >= 'a' && key <= 'e')
        return (int)(key - 'a');
    // 大文字の商品キーを商品インデックスに変換
    if (key >= 'A' && key <= 'E')
        return (int)(key - 'A');
    return -1;
}
