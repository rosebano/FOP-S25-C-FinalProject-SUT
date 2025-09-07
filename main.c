#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "login.h"
#include "admin.h"
#include "customer.h"
#include "room.h"

static void trim_newline(char* s) { if(!s) return; s[strcspn(s, "\r\n")] = 0; }

int main(void) {
    printf("***Hotel Reservation System***\n");
    printf("Type 'help' for available commands\n\n");

    // Initialize hotel with 200 rooms
    Hotel* hotel = hotel_init(200);
    if(!hotel) { fprintf(stderr, "Memory error.\n"); return 1; }

    // Load prices and rooms
    load_prices(hotel, "prices.txt");
    // If rooms.txt empty, auto-generate
    FILE* rf = fopen("rooms.txt","r");
    int empty = 1;
    if(rf) { int c=fgetc(rf); if(c!=EOF) empty=0; fclose(rf); }
    if(empty) {
        generate_rooms_automatically(hotel, "rooms.txt");
    } else {
        load_rooms(hotel, "rooms.txt");
    }

    char line[256];
    while(1) {
        printf("\ncmd> ");
        if(!fgets(line, sizeof(line), stdin)) break;
        trim_newline(line);
        if(strlen(line)==0) continue;

        char cmd[64]={0};
        sscanf(line, "%63s", cmd);

        if(strcmp(cmd, "help")==0) {
            print_login_help();
        } else if(strcmp(cmd, "user_register")==0) {
            char username[USERNAME_LEN]={0}, phone[PHONE_LEN]={0};
            sscanf(line, "%*s -u %49s -ph %19s", username, phone);
            if(username[0] && phone[0]) customer_register(username, phone);
            else printf("Usage: user_register -u <username> -ph <phoneNo>\n");
        } else if(strcmp(cmd, "admin_register")==0) {
            char username[USERNAME_LEN]={0}, password[PASSWORD_LEN]={0};
            sscanf(line, "%*s -u %49s -p %49s", username, password);
            if(password[0]) admin_register(username, password);
            else printf("Usage: admin_register -u <username> -p <password>\n");
        } else if(strcmp(cmd, "user_login")==0) {
            char username[USERNAME_LEN]={0}, phone[PHONE_LEN]={0};
            sscanf(line, "%*s -u %49s -ph %19s", username, phone);
            if(username[0] && phone[0]) customer_login(hotel, username, phone);
            else printf("Usage: user_login -u <username> -ph <phoneNo>\n");
        } else if(strcmp(cmd, "admin_login")==0) {
            char username[USERNAME_LEN]={0}, password[PASSWORD_LEN]={0};
            sscanf(line, "%*s -u %49s -p %49s", username, password);
            if(password[0]) admin_login(hotel, username, password);
            else printf("Usage: admin_login -u <username> -p <password>\n");
        } else if(strcmp(cmd, "change_admin_password")==0) {
            change_admin_password();
        } else if(strcmp(cmd, "exit")==0 || strcmp(cmd, "quit")==0) {
            break;
        } else {
            printf("Invalid command. Type 'help' for available commands.\n");
        }
    }

    // Save rooms state on exit
    save_rooms(hotel, "rooms.txt");
    hotel_free(hotel);
    printf("Bye.\n");
    return 0;
}
