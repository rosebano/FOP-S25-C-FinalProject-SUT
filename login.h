
#ifndef LOGIN_H
#define LOGIN_H

#include "room.h"

#define USERNAME_LEN 50
#define PASSWORD_LEN 50
#define PHONE_LEN    20

void print_login_help();
void customer_register(const char* username, const char* phone);
void admin_register(const char* username, const char* password);
void customer_login(Hotel* hotel, const char* username, const char* phone);
void admin_login(Hotel* hotel, const char* username, const char* password);
int  verify_admin_password(const char* password);
void change_admin_password();

#endif