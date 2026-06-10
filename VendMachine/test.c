#include <stdio.h>
#include "test.h"

const char* test_app_state_to_string(AppState state)
{
    switch (state) {
    case APP_STATE_INIT:
        return "APP_STATE_INIT";
    case APP_STATE_WAITING:
        return "APP_STATE_WAITING";
    case APP_STATE_EXITING:
        return "APP_STATE_EXITING";
    default:
        return "UNKNOWN_APP_STATE";
    }
}

const char* test_ui_view_to_string(UiView view)
{
    switch (view) {
    case UI_VIEW_NORMAL:
        return "UI_VIEW_NORMAL";
    case UI_VIEW_DEPOSITED:
        return "UI_VIEW_DEPOSITED";
    case UI_VIEW_PURCHASED:
        return "UI_VIEW_PURCHASED";
    case UI_VIEW_ERROR:
        return "UI_VIEW_ERROR";
    case UI_VIEW_EXIT:
        return "UI_VIEW_EXIT";
    default:
        return "UNKNOWN_UI_VIEW";
    }
}

const char* test_input_event_to_string(InputEvent event)
{
    switch (event) {
    case INPUT_EVENT_NONE:
        return "INPUT_EVENT_NONE";
    case INPUT_EVENT_DEPOSIT:
        return "INPUT_EVENT_DEPOSIT";
    case INPUT_EVENT_SELECT_ITEM:
        return "INPUT_EVENT_SELECT_ITEM";
    case INPUT_EVENT_EXIT:
        return "INPUT_EVENT_EXIT";
    case INPUT_EVENT_INVALID:
        return "INPUT_EVENT_INVALID";
    default:
        return "UNKNOWN_INPUT_EVENT";
    }
}

const char* test_error_code_to_string(ErrorCode error)
{
    switch (error) {
    case ERROR_NONE:
        return "ERROR_NONE";
    case ERROR_INVALID_INPUT:
        return "ERROR_INVALID_INPUT";
    case ERROR_INVALID_MONEY:
        return "ERROR_INVALID_MONEY";
    case ERROR_INVALID_ITEM:
        return "ERROR_INVALID_ITEM";
    case ERROR_MONEY_LIMIT_OVER:
        return "ERROR_MONEY_LIMIT_OVER";
    case ERROR_SOLD_OUT:
        return "ERROR_SOLD_OUT";
    case ERROR_NOT_ENOUGH_AMOUNT:
        return "ERROR_NOT_ENOUGH_AMOUNT";
    case ERROR_CHANGE_SHORTAGE:
        return "ERROR_CHANGE_SHORTAGE";
    case ERROR_FILE_OPEN:
        return "ERROR_FILE_OPEN";
    case ERROR_FILE_READ:
        return "ERROR_FILE_READ";
    case ERROR_FILE_WRITE:
        return "ERROR_FILE_WRITE";
    case ERROR_FILE_FORMAT:
        return "ERROR_FILE_FORMAT";
    case ERROR_NULL_POINTER:
        return "ERROR_NULL_POINTER";
    default:
        return "UNKNOWN_ERROR_CODE";
    }
}

void test_print_app_context(const AppContext* context)
{
    if (context == NULL) {
        printf("[AppContext] NULL\n");
        return;
    }

    printf("\n[AppContext]\n");

    printf(
        "state                 = %s (%d)\n",
        test_app_state_to_string(context->state),
        context->state
    );

    printf(
        "view                  = %s (%d)\n",
        test_ui_view_to_string(context->view),
        context->view
    );

    printf("inserted_amount       = %d\n", context->inserted_amount);

    printf(
        "error                 = %s (%d)\n",
        test_error_code_to_string(context->error),
        context->error
    );

    printf("selected_item_index   = %d\n", context->selected_item_index);
    printf("refund_amount         = %d\n", context->refund_amount);
    printf("change_shortage_value = %d\n", context->change_shortage_value);
    printf("money_limit_value    = %d\n", context->money_limit_value);

    printf("inserted_money_count  = ");
    for (size_t i = 0; i < MONEY_COUNT; i++) {
        printf("%d ", context->inserted_money_count[i]);
    }
    printf("\n");
}

void test_print_items(const Item* items, size_t item_count)
{
    if (items == NULL) {
        printf("[Items] NULL\n");
        return;
    }

    printf("\n[Items]\n");
    printf("index | id | name | price | stock\n");
    printf("----------------------------------\n");

    for (size_t i = 0; i < item_count; i++) {
        printf(
            "%5zu | %2d | %s | %5d | %5d\n",
            i,
            items[i].id,
            items[i].name,
            items[i].price,
            items[i].stock
        );
    }
}

void test_print_money(const Money* money, size_t money_count)
{
    if (money == NULL) {
        printf("[Money] NULL\n");
        return;
    }

    printf("\n[Money]\n");
    printf("index | value | count\n");
    printf("---------------------\n");

    for (size_t i = 0; i < money_count; i++) {
        printf(
            "%5zu | %5d | %5d\n",
            i,
            money[i].value,
            money[i].count
        );
    }
}

void test_print_input_result(
    const char* input,
    InputEvent event,
    int value,
    ErrorCode error
)
{
    printf("\n[Input Result]\n");

    if (input == NULL) {
        printf("input = NULL\n");
    }
    else {
        printf("input = \"%s\"\n", input);
    }

    printf(
        "event = %s (%d)\n",
        test_input_event_to_string(event),
        event
    );

    printf("value = %d\n", value);

    printf(
        "error = %s (%d)\n",
        test_error_code_to_string(error),
        error
    );
}

void test_print_all(
    const AppContext* context,
    const Item* items,
    size_t item_count,
    const Money* money,
    size_t money_count
)
{
    test_print_app_context(context);
    test_print_items(items, item_count);
    test_print_money(money, money_count);
}
