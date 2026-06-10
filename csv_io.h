#ifndef CSV_IO_H
#define CSV_IO_H

#include "app.h"

/* ファイル入出力 */
ErrorCode load_items_csv(const char *filename, Item *items, size_t item_count);
ErrorCode load_money_csv(const char *filename, Money *money, size_t money_count);
ErrorCode save_items_csv(const char *filename, const Item *items, size_t item_count);
ErrorCode save_money_csv(const char *filename, const Money *money, size_t money_count);

#endif /* CSV_IO_H */
