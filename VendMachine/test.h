#ifndef TEST_H
#define TEST_H

#include <stddef.h>
#include "app.h"

const char* test_app_state_to_string(AppState state);
const char* test_ui_view_to_string(UiView view);
const char* test_input_event_to_string(InputEvent event);
const char* test_error_code_to_string(ErrorCode error);

void test_print_app_context(const AppContext* context);
void test_print_items(const Item* items, size_t item_count);
void test_print_money(const Money* money, size_t money_count);

void test_print_input_result(
    const char* input,
    InputEvent event,
    int value,
    ErrorCode error
);

void test_print_all(
    const AppContext* context,
    const Item* items,
    size_t item_count,
    const Money* money,
    size_t money_count
);

#endif
