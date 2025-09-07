
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "customer.h"
#include "room.h"

static const char* BOOKING_FILE = "booking.txt";

void show_customer_menu(Hotel* hotel, const char* name, const char* phone) {
    if(!hotel) return;
    int choice = 0;
    while(1) {
        printf("\n.....CUSTOMER PANEL.....\n");
        printf("Welcome dear %s!\n", name);
        printf("1. View rooms status\n");
        printf("2. Reserve a room (Standard/Royal/Suite)\n");
        printf("3. Cancel my reservation\n");
        printf("4. Edit my reservation (change nights)\n");
        printf("5. Back\n");
        printf("Choose: ");
        if(scanf("%d", &choice)!=1) { while(getchar()!='\n'); continue; }
        getchar(); // مصرف newline
        
        if(choice==1) {
            view_rooms_status(hotel);
        } else if(choice==2) {
            char type[16]; int nights;
            printf("Enter type (Standard/Royal/Suite): ");
            scanf("%15s", type);
            printf("Nights: ");
            scanf("%d", &nights);
            getchar();
            int bid = reserve_room_with_id(hotel, type, name, phone, nights, BOOKING_FILE);
            if(bid > 0) {
                printf("Reservation successful. Booking ID: %d\n", bid);
            } else if(bid == 0) {
                printf("No available room of type %s.\n", type);
            } else {
                printf("Reservation failed.\n");
            }
        } else if(choice==3) {
            int id;
            printf("Enter booking ID to cancel: ");
            if(scanf("%d", &id)!=1) { while(getchar()!='\n'); printf("Invalid input.\n"); continue; }
            getchar();
            int res = cancel_reservation_by_id(hotel, id, BOOKING_FILE);
            if(res==1) printf("Your reservation was cancelled successfully.\n");
            else if(res==0) printf("No reservation found under your name/phone.\n");
            else printf("Cancellation failed due to an error.\n");
        } else if(choice==4) {
            int id; char newtype[16]; int new_nights;
            printf("Enter booking ID to edit: ");
            if(scanf("%d", &id)!=1) { while(getchar()!='\n'); printf("Invalid input.\n"); continue; }
            printf("Enter new type (or - to keep): ");
            scanf("%15s", newtype);
            printf("Enter new number of nights: ");
            if(scanf("%d", &new_nights)!=1) { while(getchar()!='\n'); printf("Invalid input.\n"); continue; }
            getchar();
            const char* typ = (strcmp(newtype, "-")==0) ? NULL : newtype;
            int res = edit_reservation_by_id(hotel, id, typ, new_nights, BOOKING_FILE);
            if(res==1) printf("Reservation updated successfully.\n");
            else if(res==0) printf("No reservation found under your name/phone.\n");
            else printf("Update failed.\n");
        } else if(choice==5) {
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
}