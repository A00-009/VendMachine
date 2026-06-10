#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include <windows.h>
#include <locale.h>

#include "app.h"
#include "input.h"
#include "ui.h"
#include "csv_io.h"
#include "vending_logic.h"
#include "test.h"

int main(void)
{
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    setlocale(LC_ALL, ".UTF8");

    Item items[ITEM_COUNT] = { 0 };     // 商品構造体
    Money money[MONEY_COUNT] = { 0 };   // 金銭構造体

    AppContext context = {
        APP_STATE_INIT,                 /* state */
        UI_VIEW_NORMAL,                 /* view */
        0,                              /* inserted_amount */
        ERROR_NONE,                     /* error */
        -1,                             /* selected_item_index */
        0,                              /* refund_amount */
        0,                              /* change_shortage_value */
        0,                              /* money_limit_value */
        { 0 }                           /* inserted_money_count */
    };

    char input[INPUT_SIZE] = { 0 };         // 入力バッファ
    InputEvent event = INPUT_EVENT_NONE;    // 入力イベント
    int value = 0;                          // 入力値
    ErrorCode error = ERROR_NONE;           // エラーコード

    // 商品管理データの読み込み
    error = load_items_csv(ITEMS_CSV, items, ITEM_COUNT);
    // エラーハンドラー
    if (error != ERROR_NONE) {
        print_error_message(error);
        return 1;
    }

    // 金銭管理データの読み込み
    error = load_money_csv(MONEY_CSV, money, MONEY_COUNT);
    // エラーハンドラー
    if (error != ERROR_NONE) {
        print_error_message(error);
        printf("load_money_csv error: %d\n", error);
        return 1;
    }

    context.state = APP_STATE_WAITING;      // 待機状態
    context.view = UI_VIEW_NORMAL;          // UI状態
    context.error = ERROR_NONE;             // エラーコード

    while (context.state != APP_STATE_EXITING) {
        print_main_screen(items, ITEM_COUNT, &context);

        printf("\n\n=== 現在の状態 ===\n");
        test_print_all(
            &context,
            items,
            ITEM_COUNT,
            money,
            MONEY_COUNT
        );
        printf(">>");

        error = input_line(input, INPUT_SIZE);
        if (error != ERROR_NONE) {
            context.error = error;
            context.view = UI_VIEW_ERROR;
            continue;
        }

        event = INPUT_EVENT_NONE;
        value = 0;

        error = parse_input_event(
            input,
            context.state,
            &event,
            &value
        );

        test_print_input_result(
            input,
            event,
            value,
            error
        );

        if (error != ERROR_NONE) {
            context.error = error;
            context.view = UI_VIEW_ERROR;
            continue;
        }

        error = handle_input_event(
            &context,
            event,
            value,
            items,
            ITEM_COUNT,
            money,
            MONEY_COUNT
        );

        printf("\n=== handle_input_event 後 ===\n");
        test_print_all(
            &context,
            items,
            ITEM_COUNT,
            money,
            MONEY_COUNT
        );

        if (error != ERROR_NONE) {
            context.error = error;
            context.view = UI_VIEW_ERROR;
            continue;
        }

        if (context.state == APP_STATE_EXITING) {
            print_main_screen(items, ITEM_COUNT, &context);
            printf("10秒後に終了します...\n");
            Sleep(10000);
        }
    }

    return 0;
}
