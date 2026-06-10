#include <stdio.h>
#include <string.h>

#include "csv_io.h"

ErrorCode load_items_csv(const char *filename, Item *items, size_t item_count)
{
    /* 1. NULLチェック */
    if (filename == NULL || items == NULL) {
        return ERROR_NULL_POINTER;
    }

    /* 2. ファイルオープン */
    FILE* fp = NULL;
    if (fopen_s(&fp, filename, "r") != 0 || fp == NULL) {
        return ERROR_FILE_OPEN;
    }

    char    line[256];
    size_t  read_count = 0;

    /* 3. 1行ずつ読み込む */
    while (fgets(line, sizeof(line), fp) != NULL) {
        /* 5. 件数超過チェック */
        if (read_count >= item_count) {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        /* 末尾の改行・CR を除去 */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
        if (len > 0 && line[len - 1] == '\r') line[--len] = '\0';

        /* 空行はスキップ */
        if (len == 0) continue;

        /* 4. CSVを分解: 商品名, 価格, 在庫数
        *    ※ items.csv に id 列はないため id は連番で付与する */
        char name[ITEM_NAME_LENGTH] = { 0 };
        int price = 0;
        int stock = 0;
        int pos = 0;

        int parsed = sscanf_s(
            line,
            " %31[^,],%d,%d %n",
            name,
            (unsigned int)ITEM_NAME_LENGTH,
            &price,
            &stock,
            &pos
        );

        /* 4. 3フィールドすべて取れなければフォーマットエラー */
        if (parsed != 3 || line[pos] != '\0') {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        /* 価格・在庫数が負値でないことを確認 */
        if (price <= 0 || stock < 0) {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        /* 6. Item 構造体へ格納 (id は 1 始まりの連番) */
        items[read_count].id = (int)read_count + 1;
        strncpy_s(
            items[read_count].name,
            ITEM_NAME_LENGTH,
            name,
            ITEM_NAME_LENGTH - 1
        );
        items[read_count].price = price;
        items[read_count].stock = stock;

        read_count++;
    }

    // fgetsが読み取りエラーを出していた場合
    if (ferror(fp)) {
        fclose(fp);
        return ERROR_FILE_READ;
    }

    // 商品の数が規定値を上回った場合
    if (read_count != item_count) {
        fclose(fp);
        return ERROR_FILE_READ;
    }

    /* 7. ファイルクローズ */
    fclose(fp);

    /* 8. 正常終了 */
    return ERROR_NONE;
}

ErrorCode load_money_csv(const char *filename, Money *money, size_t money_count)
{
    FILE* fp = NULL;        // ファイルポインタ
    char line[INPUT_SIZE];  // 入力バッファ
    size_t count = 0;       // 抽出済みの件数

    // ファイル・金銭構造体が存在しない場合
    if (filename == NULL || money == NULL) {
        return ERROR_NULL_POINTER;
    }

    // ファイルを読み取りモードで開く
    if (fopen_s(&fp, filename, "r") != 0 || fp == NULL) {
        return ERROR_FILE_OPEN;
    }

    // ファイルを開けなかった場合
    if (fp == NULL) {
        return ERROR_FILE_OPEN;
    }

    // 一件読み取る
    while (fgets(line, sizeof(line), fp) != NULL) {
        int value = 0;  // 金額
        int number = 0; // 枚数
        int pos = 0;

        /*
            金額円,枚数
            例：10円,20
            要素数が2と異なる場合
        */
        int parsed = sscanf_s(
            line,
            " %d円,%d %n",
            &value,
            &number,
            &pos
        );

        // 4.フィールドすべて取れなければフォーマットエラー
        if (parsed != 2 || line[pos] != '\0') {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        // 金額が0以下、もしくは枚数が0未満の場合
        if (value <= 0 || number < 0) {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        // 金銭構造体に値を格納する
        money[count].value = value;
        money[count].count = number;

        count++;
    }

    // fgetsが読み取りエラーを出していた場合
    if (ferror(fp)) {
        fclose(fp);
        return ERROR_FILE_READ;
    }

    // 金銭の数が規定値を上回った場合
    if (count != money_count) {
        fclose(fp);
        return ERROR_FILE_READ;
    }

    // ファイルを閉じる
    fclose(fp);

    // 正常終了
    return ERROR_NONE;
}

ErrorCode save_money_csv(const char* filename, const Money* money, size_t money_count)
{
    if (filename == NULL || money == NULL)
        return ERROR_NULL_POINTER;

    FILE* fp = fopen(filename, "w");
    if (fp == NULL)
        return ERROR_FILE_OPEN;

    const char* denom_name[MONEY_COUNT] = { "10円","50円","100円","500円","1000円" };

    for (size_t i = 0; i < money_count; i++) {
        if (fprintf(fp, "%s,%d\n", denom_name[i], money[i].count) < 0) {
            fclose(fp);
            return ERROR_FILE_WRITE;
        }
    }

    fclose(fp);
    return ERROR_NONE;
}

ErrorCode save_items_csv(const char* filename, const Item* items, size_t item_count)
{
    FILE* fp = NULL;

    /* 1. NULLチェック */
    if (filename == NULL || items == NULL) {
        return ERROR_NULL_POINTER;
    }

    /* 2. ファイルオープン */
    if (fopen_s(&fp, filename, "w") != 0 || fp == NULL) {
        return ERROR_FILE_OPEN;
    }

    /* 3. 商品データを書き込む */
    for (size_t i = 0; i < item_count; i++) {
        /*
            items.csv の形式:
            お茶,110,20
            コーラ,140,20
            コーヒー,200,20
            オレンジ,120,20
            紅茶,180,20
        */

        if (items[i].name[0] == '\0') {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        if (items[i].price <= 0 || items[i].stock < 0) {
            fclose(fp);
            return ERROR_FILE_FORMAT;
        }

        if (fprintf(
            fp,
            "%s,%d,%d\n",
            items[i].name,
            items[i].price,
            items[i].stock
        ) < 0) {
            fclose(fp);
            return ERROR_FILE_WRITE;
        }
    }

    /* 4. ファイルクローズ */
    if (fclose(fp) != 0) {
        return ERROR_FILE_WRITE;
    }

    /* 5. 正常終了 */
    return ERROR_NONE;
}
