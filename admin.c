#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "admin.h"
#include "room.h"

static const char* BOOKING_FILE = "booking.txt";
static const char* CUSTOMERS_FILE = "customers.txt";

char* format_price(int price) {
    char temp[64]; snprintf(temp, sizeof(temp), "%d", price);
    int len = (int)strlen(temp);
    int commas = (len-1)/3;
    int outlen = len + commas;
    char* out = (char*)malloc((size_t)outlen + 1);
    if(!out) return NULL;
    int i=len-1, j=outlen-1, c=0;
    out[outlen] = 0;
    while(i>=0) {
        out[j--] = temp[i--];
        c++;
        if(c==3 && i>=0) { out[j--] = ','; c=0; }
    }
    return out;
}

static int parse_ymd(const char* s, int* y,int* m,int* d) {
    return sscanf(s, "%d-%d-%d", y,m,d)==3;
}

static int days_between(int y1,int m1,int d1,int y2,int m2,int d2) {
    struct tm a = {0}, b = {0};
    a.tm_year = y1 - 1900; a.tm_mon = m1-1; a.tm_mday = d1;
    b.tm_year = y2 - 1900; b.tm_mon = m2-1; b.tm_mday = d2;
    time_t ta = mktime(&a); time_t tb = mktime(&b);
    double diff = difftime(tb, ta);
    return (int)(diff / 86400.0);
}

void view_all_bookings(const char* filename) {
    FILE* f = fopen(filename?filename:BOOKING_FILE, "r");
    if(!f) { printf("No bookings yet.\n"); return; }
    printf("ID DATE       ROOM  TYPE     NAME        PHONE       NIGHTS  PRICE  TOTAL\n");
    int id, room, nights, price, total;
    char date[16], type[32], name[64], phone[32];
    int count=0;
    while(fscanf(f, "%d %15s %d %31s %63s %31s %d %d %d", 
                 &id, date, &room, type, name, phone, &nights, &price, &total)==9) {
        printf("%-2d %-10s %-5d %-8s %-10s %-11s %-6d %-6d %-6d\n", 
               id, date, room, type, name, phone, nights, price, total);
        count++;
    }
    if(count==0) printf("No bookings yet.\n");
    fclose(f);
}

void search_bookings(const char* filename, const char* query) {
    FILE* f = fopen(filename?filename:BOOKING_FILE, "r");
    if(!f) { printf("No bookings yet.\n"); return; }
    int id, room, nights, price, total;
    char date[16], type[32], name[64], phone[32];
    int found=0;
    while(fscanf(f, "%d %15s %d %31s %63s %31s %d %d %d", 
                 &id, date, &room, type, name, phone, &nights, &price, &total)==9) {
        if(strstr(name, query) || strstr(phone, query)) {
            printf("%-2d %-10s %-5d %-8s %-10s %-11s %-6d %-6d %-6d\n", 
                   id, date, room, type, name, phone, nights, price, total);
            found=1;
        }
    }
    if(!found) printf("No matching bookings found.\n");
    fclose(f);
}

static void report_income_days(int days) {
    FILE* f = fopen(BOOKING_FILE, "r");
    if(!f) { printf("No bookings yet.\n"); return; }
    time_t t = time(NULL);
    struct tm* tmv = localtime(&t);
    int y2=tmv->tm_year+1900, m2=tmv->tm_mon+1, d2=tmv->tm_mday;
    
    int id, room, nights, price, total;
    char date[16], type[32], name[64], phone[32];
    long long sum = 0;
    
    while(fscanf(f, "%d %15s %d %31s %63s %31s %d %d %d", 
                 &id, date, &room, type, name, phone, &nights, &price, &total)==9) {
        int y1,m1,d1;
        if(parse_ymd(date,&y1,&m1,&d1)) {
            int diff = days_between(y1,m1,d1, y2,m2,d2);
            if(diff>=0 && diff<days) sum += total;
        }
    }
    fclose(f);
    char* s = format_price((int)sum);
    printf("Income last %d day(s): %s\n", days, s? s : "(alloc error)");
    free(s);
}

void view_all_users(const char* customers_filename) {
    const char* fname = customers_filename?customers_filename:CUSTOMERS_FILE;
    FILE* f = fopen(fname, "r");
    if(!f) { printf("No users registered.\n"); return; }
    printf("Registered users:\n");
    char u[128], p[64];
    int count = 0;
    while(fscanf(f, "%127s %63s", u, p) == 2) {
        printf(" - %s  (%s)\n", u, p);
        count++;
    }
    if(count==0) printf("No users registered.\n");
    fclose(f);
}

int delete_user_by_name_phone(const char* customers_filename, const char* username, const char* phone) {
    const char* fname = customers_filename?customers_filename:CUSTOMERS_FILE;
    FILE* f = fopen(fname, "r");
    if(!f) return 0;
    
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.tmp", fname);
    FILE* out = fopen(outpath, "w");
    if(!out) { fclose(f); return -1; }
    
    char buf[256];
    int found = 0;
    while(fgets(buf, sizeof(buf), f)) {
        char u[128], p[64];
        if(sscanf(buf, "%127s %63s", u, p) == 2) {
            if(!found && strcmp(u, username)==0 && strcmp(p, phone)==0) {
                found = 1;
                continue;
            }
        }
        fputs(buf, out);
    }
    fclose(f);
    fclose(out);
    
    if(found) {
        remove(fname);
        rename(outpath, fname);
        return 1;
    } else {
        remove(outpath);
        return 0;
    }
}

void show_admin_menu(Hotel* hotel) {
    int choice=0;
    while(1) {
        printf("\n.....ADMIN PANEL.....\n");
        printf("1. View all bookings\n");
        printf("2. Search bookings (by name/phone)\n");
        printf("3. Daily income report\n");
        printf("4. Weekly income report\n");
        printf("5. Cancel a booking by ID\n");
        printf("6. Edit a booking by ID\n");
        printf("7. View all users\n");
        printf("8. Delete a user\n");
        printf("9. Back\n");
        printf("Choose: ");
        
        if(scanf("%d",&choice)!=1) { 
            while(getchar()!='\n'); 
            continue; 
        }
        getchar(); // مصرف newline
        
        if(choice==1) view_all_bookings(BOOKING_FILE);
        else if(choice==2) {
            char q[64];
            printf("Enter name/phone to search: ");
            scanf("%63s", q);
            getchar();
            search_bookings(BOOKING_FILE, q);
        }
        else if(choice==3) report_income_days(1);
        else if(choice==4) report_income_days(7);
        else if(choice==5) {
            int id;
            printf("Enter booking ID to cancel: ");
            if(scanf("%d", &id)!=1) { 
                while(getchar()!='\n'); 
                printf("Invalid input.\n"); 
                continue; 
            }
            getchar();
            int r = cancel_reservation_by_id(hotel, id, BOOKING_FILE);
            if(r==1) printf("Booking cancelled.\n");
            else if(r==0) printf("Booking ID not found.\n");
            else printf("Error cancelling booking.\n");
        }
        else if(choice==6) {
            int id; char newtype[16]; int new_nights;
            printf("Enter booking ID to edit: ");
            if(scanf("%d", &id)!=1) { 
                while(getchar()!='\n'); 
                printf("Invalid input.\n"); 
                continue; 
            }
            printf("Enter new type (or - to keep): ");
            scanf("%15s", newtype);
            printf("Enter new number of nights: ");
            if(scanf("%d", &new_nights)!=1) { 
                while(getchar()!='\n'); 
                printf("Invalid input.\n"); 
                continue; 
            }
            getchar();
            const char* typ = (strcmp(newtype, "-")==0) ? NULL : newtype;
            int r = edit_reservation_by_id(hotel, id, typ, new_nights, BOOKING_FILE);
            if(r==1) printf("Booking updated.\n");
            else if(r==0) printf("Booking ID not found.\n");
            else printf("Error updating booking.\n");
        }
        else if(choice==7) view_all_users(CUSTOMERS_FILE);
        else if(choice==8) {
            char u[128], p[64];
            printf("Enter username to delete: ");
            scanf("%127s", u);
            printf("Enter phone: ");
            scanf("%63s", p);
            getchar();
            int r = delete_user_by_name_phone(CUSTOMERS_FILE, u, p);
            if(r==1) printf("User deleted.\n");
            else if(r==0) printf("User not found.\n");
            else printf("Error deleting user.\n");
        }
        else if(choice==9) break;
        else printf("Invalid choice.\n");
    }
}

void daily_income_report(const char* filename) {
    (void)filename;
    report_income_days(1);
}

void weekly_income_report(const char* filename) {
    (void)filename;
    report_income_days(7);
}