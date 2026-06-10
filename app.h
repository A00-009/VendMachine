#ifndef APP_H
#define APP_H

#include <stddef.h>

/* ITEMSファイルの名前 */
#define ITEMS_CSV "items.csv"

/* MONEYファイルの名前 */
#define MONEY_CSV "money.csv"

/* 入力バッファのサイズ */
#define INPUT_SIZE 128

/* 商品名の最大文字数 */
#define ITEM_NAME_LENGTH 32

/* 商品の種類数 */
#define ITEM_COUNT 5

/* 金銭の種類数 */
#define MONEY_COUNT 5

/* 硬貨・紙幣の投入枚数上限 */
#define COIN_LIMIT 20
#define BILL_LIMIT 2

/* カラーテーブル */
#define RESET   "\x1b[0m"
#define RED     "\x1b[31m"
#define BLUE    "\x1b[34m"
#define GREEN   "\x1b[32m"

/* エラー定義 */
typedef enum {
    ERROR_NONE = 0,                 // エラーなし

    ERROR_INVALID_INPUT,            // 不正な入力値
    ERROR_INVALID_MONEY,
    ERROR_INVALID_ITEM,

    ERROR_MONEY_LIMIT_OVER,
    ERROR_SOLD_OUT,
    ERROR_NOT_ENOUGH_AMOUNT,
    ERROR_CHANGE_SHORTAGE,

    ERROR_FILE_OPEN,                // ファイルオープン不可
    ERROR_FILE_READ,                // ファイル読み取り不可
    ERROR_FILE_WRITE,               // ファイル書き込み不可
    ERROR_FILE_FORMAT,              // ファイルフォーマット不正

    ERROR_NULL_POINTER              // 不正アドレス
} ErrorCode;

/* 商品管理データ構造体 */
typedef struct {
    int id;                         /* 商品ID */
    char name[ITEM_NAME_LENGTH];    /* 商品名 */
    int price;                      /* 価格 */
    int stock;                      /* 在庫数 */
} Item;

/* 金銭管理データ構造体 */
typedef struct {
    int value;                      /* 金額: 10, 50, 100, 500, 1000 */
    int count;                      /* 枚数 */
} Money;

/* アプリケーション状態 */
typedef enum {
    APP_STATE_INIT = 0,             /* 初期化中 */
    APP_STATE_WAITING,              /* 初期画面・投入待ち */
    APP_STATE_EXITING               /* 終了処理中 */
} AppState;

/* 入力イベント */
typedef enum {
    INPUT_EVENT_NONE = 0,
    INPUT_EVENT_DEPOSIT,
    INPUT_EVENT_SELECT_ITEM,
    INPUT_EVENT_EXIT,
    INPUT_EVENT_INVALID
} InputEvent;

/* UI表示状態 */
typedef enum {
    UI_VIEW_NORMAL = 0,             /* 通常表示 */
    UI_VIEW_DEPOSITED,              /* 入金後表示 */
    UI_VIEW_PURCHASED,              /* 購入完了表示 */
    UI_VIEW_ERROR,                  /* エラー表示 */
    UI_VIEW_EXIT                    /* 終了表示 */
} UiView;

/* 状態変数構造体 */
typedef struct {
    AppState state;                         /* 現在のアプリケーション状態 */
    UiView view;                            /* 現在のUI表示状態 */
    int inserted_amount;                    /* 入金額 */
    ErrorCode error;                        /* 直近のエラー */
    int selected_item_index;                /* 選択された商品インデックス */
    int refund_amount;                      /* 返金額 */
    int change_shortage_value;              /* 釣銭切れとなった金種 */
    int money_limit_value;                  /* 投入上限を超えた金種 */
    int inserted_money_count[MONEY_COUNT];  /* 金銭ごとの入金枚数 */
} AppContext;

#endif /* APP_H */