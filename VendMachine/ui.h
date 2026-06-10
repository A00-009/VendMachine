#ifndef UI_H
#define UI_H

#include "app.h"

/* UI */
void reset_screen(void);
void print_main_screen(const Item* items, size_t item_count, const AppContext* context);
void print_status_message(const Item* items, size_t item_count, const AppContext* context);
void print_item_table(const Item* items, size_t item_count, const AppContext* context);
void print_info_area(const Item* items, size_t item_count, const AppContext* context);
void print_error_message(ErrorCode error);
int display_width(const char* s);

#endif /* UI_H */
