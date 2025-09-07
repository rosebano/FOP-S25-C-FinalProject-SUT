#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "login.h"
#include "admin.h"
#include "customer.h"

static const char* CUSTOMERS_FILE = "customers.txt";
static const char* ADMIN_PASS_FILE = "admin_pass.txt";
static const char* COMMAND_FILE = "command.txt";

void print_login_help() {
    FILE* f = fopen(COMMAND_FILE, "r");
    if(!f) {
        printf("Commands:\n");
        printf("  user_register -u <username> -ph <phoneNo>\n");
        printf("  admin_register -u <username> -p <password>\n");
        printf("  user_login -u <username> -ph <phoneNo>\n");
        printf("  admin_login -u <username> -p <password>\n");
        return;
    }
    char line[256];
    while(fgets(line, sizeof(line), f)) {
        fputs(line, stdout);
    }
    fclose(f);
}

static int customer_exists(const char* username, const char* phone) {
    FILE* f = fopen(CUSTOMERS_FILE, "r");
    if(!f) return 0;
    char u[USERNAME_LEN], p[PHONE_LEN];
    int found = 0;
    while(fscanf(f, "%49s %19s", u, p)==2) {
        if(strcmp(u, username)==0 && strcmp(p, phone)==0) { found = 1; break; }
    }
    fclose(f);
    return found;
}

void customer_register(const char* username, const char* phone) {
    if(!username || !phone || !*username || !*phone) {
        printf("Invalid username/phone.\n"); return;
    }
    if(customer_exists(username, phone)) {
        printf("User already registered.\n"); return;
    }
    FILE* f = fopen(CUSTOMERS_FILE, "a");
    if(!f) { printf("Failed to open customers file.\n"); return; }
    fprintf(f, "%s %s\n", username, phone);
    fclose(f);
    printf("Registration successful.\n");
}

int verify_admin_password(const char* password) {
    FILE* f = fopen(ADMIN_PASS_FILE, "r");
    if(!f) return 0;
    char buf[PASSWORD_LEN];
    if(fgets(buf, sizeof(buf), f)==NULL) { fclose(f); return 0; }
    // remove newline
    buf[strcspn(buf, "\r\n")] = 0;
    fclose(f);
    return strcmp(buf, password)==0;
}

void admin_register(const char* username, const char* password) {
    (void)username; // Username not used for verification in this simple model
    if(!password || !*password) { printf("Password cannot be empty.\n"); return; }
    FILE* f = fopen(ADMIN_PASS_FILE, "w");
    if(!f) { printf("Cannot write admin password file.\n"); return; }
    fprintf(f, "%s\n", password);
    fclose(f);
    printf("Admin password updated successfully.\n");
}

void customer_login(Hotel* hotel, const char* username, const char* phone) {
    if(!customer_exists(username, phone)) {
        printf("No such user. Please register first.\n");
        return;
    }
    printf("Login successful. Welcome, %s!\n", username);
    show_customer_menu(hotel, username, phone);
}

void admin_login(Hotel* hotel, const char* username, const char* password) {
    (void)username;
    if(verify_admin_password(password)) {
        printf("Login successful.\n");
        show_admin_menu(hotel);
    } else {
        printf("Incorrect admin password.\n");
    }
}

void change_admin_password() {
    char newpass[PASSWORD_LEN];
    printf("Enter new admin password: ");
    if(scanf("%49s", newpass)!=1) { printf("Input error.\n"); return; }
    FILE* f = fopen(ADMIN_PASS_FILE, "w");
    if(!f) { printf("Cannot write admin password file.\n"); return; }
    fprintf(f, "%s\n", newpass);
    fclose(f);
    printf("Admin password changed.\n");
}
