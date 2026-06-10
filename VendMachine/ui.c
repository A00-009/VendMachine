#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "vending_logic.h"

void reset_screen(void)
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void print_main_screen(
    const Item* items,
    size_t item_count,
    const AppContext* context
)
{
    reset_screen();

    if (items == NULL || context == NULL) {
        print_error_message(ERROR_NULL_POINTER);
        return;
    }

    printf("************ 自動販売機 シミュレーション ソフトウェア ************\n\n");

    print_status_message(items, item_count, context);

    print_item_table(items, item_count, context);

    print_info_area(items, item_count, context);

    printf(">> ");
}

void print_status_message(
    const Item* items,
    size_t item_count,
    const AppContext* context
)
{
    const char* selected_item_name = "商品";

    if (items == NULL || context == NULL) {
        return;
    }

    if (context->selected_item_index >= 0
        && (size_t)context->selected_item_index < item_count) {
        selected_item_name = items[context->selected_item_index].name;
    }

    /*
        1. 基本ステータス行
        「選択してください」の右側には入金額だけを表示する。
    */
    if (context->inserted_amount > 0) {
        printf(
            "金銭投入後商品を選択してください。  "
            BLUE "入金%d円" RESET "\n",
            context->inserted_amount
        );
    }
    else {
        printf("金銭投入後商品を選択してください。\n");
    }

    /*
        2. 購入完了メッセージ
        商品テーブルとの間の空白行に表示する。
    */
    if (context->view == UI_VIEW_PURCHASED) {
        printf(
            BLUE "%sの購入、ありがとうございました。" RESET "\n",
            selected_item_name
        );
    }
}

void print_item_table(
    const Item* items,
    size_t item_count,
    const AppContext* context
)
{
    if (items == NULL || context == NULL) {
        return;
    }

    printf("+");
    for (size_t i = 0; i < item_count; i++) {
        printf("------------+");
    }
    printf("\n");

    printf("|");
    for (size_t i = 0; i < item_count; i++) {
        char label = 'A' + (char)i;
        const char* name = items[i].name;

        /* " A.商品名 " が12文字幅に収まるようパディング */
        /* " A." で3文字、名前、残りをスペースで埋める */
        int name_width = display_width(name);
        int padding = 8 - name_width; /* 8は名前フィールドの表示幅 */
        if (padding < 0) padding = 0;

        printf(" %c.%s%*s |", label, name, padding, "");
    }
    printf("\n");

    printf("+");
    for (size_t i = 0; i < item_count; i++) {
        printf("------------+");
    }
    printf("\n");

    printf("|");
    for (size_t i = 0; i < item_count; i++) {
        if (items[i].stock <= 0) {
            const char* sold_out = "売切";
            int padding = 10 - display_width(sold_out); /* 10は表示フィールド幅 */
            printf(RED " %s%*s " RESET "|", sold_out, padding, "");
        }
        else if (context->inserted_amount >= items[i].price) {
            printf(BLUE " %-10d " RESET "|", items[i].price);
        }
        else {
            printf(" %-10d |", items[i].price);
        }
    }
    printf("\n");

    printf("+");
    for (size_t i = 0; i < item_count; i++) {
        printf("------------+");
    }
    printf("\n");
}

void print_info_area(
    const Item* items,
    size_t item_count,
    const AppContext* context
)
{
    const char* selected_item_name = "商品";

    if (items == NULL || context == NULL) {
        print_error_message(ERROR_NULL_POINTER);
        return;
    }

    if (context->selected_item_index >= 0
        && (size_t)context->selected_item_index < item_count) {
        selected_item_name = items[context->selected_item_index].name;
    }

    /*
        1. 通知メッセージ
        購入完了・返金終了など、正常系の結果通知を表示する。
    */
    if (context->view == UI_VIEW_PURCHASED) {
        if (context->refund_amount > 0) {
            printf(
                BLUE "お釣りは%d円です。%sを受け取りください。" RESET "\n",
                context->refund_amount,
                selected_item_name
            );
        }
        else {
            printf(
                BLUE "%sを受け取りください。" RESET "\n",
                selected_item_name
            );
        }
    }
    else if (context->view == UI_VIEW_EXIT) {
        if (context->refund_amount > 0) {
            printf(
                BLUE "%d円を返金します。お受け取りください。" RESET "\nシステムを終了します。\n",
                context->refund_amount
            );
        }
        else {
            printf("システムを終了します。\n");
        }
    }

    /*
        2. 入金案内
        終了画面以外では常に表示する。
    */
    if (context->view != UI_VIEW_EXIT) {
        printf(
            "お金を入れてください。 "
            "1:10円 2:50円 3:100円 4:500円 5:1000円\n"
        );
    }

    /*
        3. 購入可能案内
        終了中は表示しない。
        エラー中でも、入金額が購入可能金額に達していれば表示する。
    */
    int min_price = get_min_buyable_item_price(items, item_count);

    if (min_price >= 0
        && context->inserted_amount >= min_price
        && context->view != UI_VIEW_EXIT) {
        printf("商品購入可能です。\n");
    }

    /*
        4. 警告表示
        エラー状態の場合だけ、最後の行に表示する。
    */
    if (context->view == UI_VIEW_ERROR) {
        switch (context->error) {
        case ERROR_INVALID_INPUT:
            printf(
                RED "無効な入力です。指定されたキーを入力してください。" RESET "\n"
            );
            break;

        case ERROR_INVALID_MONEY:
            printf(
                RED "使用できない金額です。" RESET "\n"
            );
            break;

        case ERROR_INVALID_ITEM:
            printf(
                RED "無効な商品キーです。" RESET "\n"
            );
            break;

        case ERROR_MONEY_LIMIT_OVER:
            if (context->money_limit_value == 1000) {
                printf(
                    RED "1000円紙幣の投入枚数が上限(%d枚)を超えています。" RESET "\n",
                    BILL_LIMIT
                );
            }
            else if (context->money_limit_value > 0) {
                printf(
                    RED "%d円硬貨の投入枚数が上限(%d枚)を超えています。" RESET "\n",
                    context->money_limit_value,
                    COIN_LIMIT
                );
            }
            else {
                printf(
                    RED "投入枚数が上限を超えています。" RESET "\n"
                );
            }
            break;

        case ERROR_SOLD_OUT:
            printf(
                RED "売切れ商品です。他の商品を選択してください。" RESET "\n"
            );
            break;

        case ERROR_NOT_ENOUGH_AMOUNT:
            printf(
                RED "金額が不足しています。お金を追加してください。" RESET "\n"
            );
            break;

        case ERROR_CHANGE_SHORTAGE:
            if (context->change_shortage_value > 0) {
                printf(
                    RED "%d円硬貨釣銭切れのため商品を購入できません。" RESET "\n",
                    context->change_shortage_value
                );
            }
            else {
                printf(
                    RED "釣銭切れのため商品を購入できません。" RESET "\n"
                );
            }
            break;

        case ERROR_FILE_OPEN:
        case ERROR_FILE_READ:
        case ERROR_FILE_WRITE:
        case ERROR_FILE_FORMAT:
        case ERROR_NULL_POINTER:
            print_error_message(context->error);
            break;

        case ERROR_NONE:
        default:
            print_error_message(context->error);
            break;
        }
    }
}

void print_error_message(ErrorCode error)
{
    printf(RED "[エラー] ");

    switch (error) {
    case ERROR_NONE:
        printf("エラーはありません。\n");
        break;

    case ERROR_INVALID_INPUT:
        printf("無効な入力です。\n");
        break;

    case ERROR_INVALID_MONEY:
        printf("使用できない金額です。\n");
        break;

    case ERROR_INVALID_ITEM:
        printf("無効な商品キーです。\n");
        break;

    case ERROR_MONEY_LIMIT_OVER:
        printf("投入枚数が上限に達しています。\n");
        break;

    case ERROR_SOLD_OUT:
        printf("売り切れです。\n");
        break;

    case ERROR_NOT_ENOUGH_AMOUNT:
        printf("金額が不足しています。\n");
        break;

    case ERROR_CHANGE_SHORTAGE:
        printf("釣銭が不足しています。\n");
        break;

    case ERROR_FILE_OPEN:
        printf("ファイルを開けません。\n");
        break;

    case ERROR_FILE_READ:
        printf("ファイル読み込みに失敗しました。\n");
        break;

    case ERROR_FILE_WRITE:
        printf("ファイル書き込みに失敗しました。\n");
        break;

    case ERROR_FILE_FORMAT:
        printf("ファイル形式が不正です。\n");
        break;

    case ERROR_NULL_POINTER:
        printf("内部エラーです。NULLポインタが渡されました。\n");
        break;

    default:
        printf("不明なエラーです。error=%d\n", error);
        break;
    }

    printf(RESET);
}

/* UTF-8文字列の「表示幅」を返す（ASCII=1, マルチバイト=2として計算） */
int display_width(const char* s)
{
    int width = 0;
    while (*s) {
        unsigned char c = (unsigned char)*s;
        if (c < 0x80) {
            /* ASCII */
            width += 1;
            s += 1;
        }
        else if (c < 0xE0) {
            /* 2バイト文字 */
            width += 2;
            s += 2;
        }
        else if (c < 0xF0) {
            /* 3バイト文字（ほとんどの日本語） */
            width += 2;
            s += 3;
        }
        else {
            /* 4バイト文字（絵文字など） */
            width += 2;
            s += 4;
        }
    }
    return width;
}