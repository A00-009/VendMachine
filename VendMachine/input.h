#ifndef INPUT_H
#define INPUT_H

#include "app.h"

/* 入力処理 */
ErrorCode input_line(char *buffer, size_t size);

/*
| 入力         | event                     | value            |
| ---------- - | ------------------------: | ------ - :       |
| `"1"`        | `INPUT_EVENT_DEPOSIT`     | `10`             |
| `"2"`        | `INPUT_EVENT_DEPOSIT`     | `50`             |
| `"3"`        | `INPUT_EVENT_DEPOSIT`     | `100`            |
| `"4"`        | `INPUT_EVENT_DEPOSIT`     | `500`            |
| `"5"`        | `INPUT_EVENT_DEPOSIT`     | `1000`           |
| `"A"`〜`"E"` | `INPUT_EVENT_SELECT_ITEM` | 商品インデックス |
| `"a"`〜`"e"` | `INPUT_EVENT_SELECT_ITEM` | 商品インデックス |
| `"9"`        | `INPUT_EVENT_EXIT`        | `9`              |
| その他       | `INPUT_EVENT_INVALID`     | `0`              |
*/
ErrorCode parse_input_event(
    const char *input,
    AppState state,
    InputEvent *out_event,
    int *out_value
);

ErrorCode handle_input_event(
    AppContext* context,
    InputEvent event,
    int value,
    Item* items,
    size_t item_count,
    Money* money,
    size_t money_count
);

int money_key_to_value(char key);
int item_key_to_index(char key);

#endif /* INPUT_H */
