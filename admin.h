#ifndef ADMIN_H
#define ADMIN_H

#include "room.h"

void show_admin_menu(Hotel* hotel);
void view_all_bookings(const char* booking_filename);
void search_bookings(const char* booking_filename, const char* query);
void daily_income_report(const char* booking_filename);
void weekly_income_report(const char* booking_filename);
char* format_price(int price); // caller must free

// User management by admin
void view_all_users(const char* customers_filename);
int  delete_user_by_name_phone(const char* customers_filename, const char* username, const char* phone);

#endif