#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "room.h"


Hotel* hotel_init(int total_rooms) {
    if(total_rooms <= 0) total_rooms = 1;
    Hotel* hotel = (Hotel*)calloc(1, sizeof(Hotel));
    if(!hotel) return NULL;
    hotel->rooms = (Room*)calloc((size_t)total_rooms, sizeof(Room));
    if(!hotel->rooms) { free(hotel); return NULL; }
    hotel->total_rooms = total_rooms;
    hotel->room_count = 0;
    hotel->price_standard = 0;
    hotel->price_royal = 0;
    hotel->price_suite = 0;
    return hotel;
}

void hotel_free(Hotel* hotel) {
    if(!hotel) return;
    free(hotel->rooms);
    free(hotel);
}

void load_prices(Hotel* hotel, const char* filename) {
    if(!hotel || !filename) return;
    FILE* f = fopen(filename, "r");
    if(!f) { return; }
    char type[32];
    int price;
    while(fscanf(f, "%31s %d", type, &price) == 2) {
        if(strcmp(type, "Standard")==0) hotel->price_standard = price;
        else if(strcmp(type, "Royal")==0) hotel->price_royal = price;
        else if(strcmp(type, "Suite")==0) hotel->price_suite = price;
    }
    fclose(f);
}

void load_rooms(Hotel* hotel, const char* filename) {
    if(!hotel || !filename) return;
    FILE* f = fopen(filename, "r");
    if(!f) return;
    Room r;
    while(fscanf(f, "%d %15s %d %d", &r.room_number, r.type, &r.capacity, &r.is_reserved) == 4) {
        if(hotel->room_count >= hotel->total_rooms) break;
        hotel->rooms[hotel->room_count++] = r;
    }
    fclose(f);
}

void save_rooms(Hotel* hotel, const char* filename) {
    if(!hotel || !filename) return;
    FILE* f = fopen(filename, "w");
    if(!f) { return; }
    for(int i=0;i<hotel->room_count;i++) {
        Room* r = &hotel->rooms[i];
        fprintf(f, "%d %s %d %d\n", r->room_number, r->type, r->capacity, r->is_reserved);
    }
    fclose(f);
}

void generate_rooms_automatically(Hotel* hotel, const char* filename) {
    if(!hotel) return;
    hotel->room_count = 0;
    int rn;
    for(rn=101; rn<=150 && hotel->room_count < hotel->total_rooms; rn++) {
        Room r = {rn, "Standard", 2, 0};
        hotel->rooms[hotel->room_count++] = r;
    }
    for(rn=151; rn<=180 && hotel->room_count < hotel->total_rooms; rn++) {
        Room r = {rn, "Royal", 3, 0};
        hotel->rooms[hotel->room_count++] = r;
    }
    for(rn=181; rn<=200 && hotel->room_count < hotel->total_rooms; rn++) {
        Room r = {rn, "Suite", 4, 0};
        hotel->rooms[hotel->room_count++] = r;
    }
    if(filename) save_rooms(hotel, filename);
}

static int price_for_type(Hotel* hotel, const char* type) {
    if(strcmp(type,"Standard")==0) return hotel->price_standard;
    if(strcmp(type,"Royal")==0)    return hotel->price_royal;
    if(strcmp(type,"Suite")==0)    return hotel->price_suite;
    return 0;
}

static void ymd_today(int* y,int* m,int* d) {
    time_t t = time(NULL);
    struct tm* tmv = localtime(&t);
    *y = tmv->tm_year + 1900;
    *m = tmv->tm_mon + 1;
    *d = tmv->tm_mday;
}

int reserve_room(Hotel* hotel, const char* type,
                 const char* customer_name, const char* phone,
                 int nights, const char* booking_filename) {
    if(!hotel || !type || !customer_name || !phone || nights<=0) return -1;
    for(int i=0;i<hotel->room_count;i++) {
        Room* r = &hotel->rooms[i];
        if(!r->is_reserved && strcmp(r->type, type)==0) {
            r->is_reserved = 1;
            int price = price_for_type(hotel, r->type);
            int total = price * nights;
            int y,m,d; ymd_today(&y,&m,&d);
            FILE* f = fopen(booking_filename?booking_filename:"booking.txt", "a");
            if(f) {
                fprintf(f, "%04d-%02d-%02d %d %s %s %d %d %d\n",
                    y,m,d, r->room_number, customer_name, phone, nights, price, total);
                fclose(f);
            }
            return r->room_number;
        }
    }
    return 0; // no available room for that type
}

// Cancel the first matching booking; set room to available and remove booking line.
int cancel_reservation(Hotel* hotel, const char* name, const char* phone, const char* booking_filename) {
    if(!hotel || !name || !phone) return -1;
    const char* fname = booking_filename?booking_filename:"booking.txt";
    FILE* f = fopen(fname, "r");
    if(!f) return 0;
    // Read all lines and find first matching entry by name and phone
    char buf[512];
    int found = 0;
    int roomno = 0;
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.tmp", fname);
    FILE* out = fopen(outpath, "w");
    if(!out) { fclose(f); return -1; }
    while(fgets(buf, sizeof(buf), f)) {
        char date[16], n[128], p[64];
        int r, nights, price, total;
        if(sscanf(buf, "%15s %d %127s %63s %d %d %d", date, &r, n, p, &nights, &price, &total)==7) {
            if(!found && strcmp(n, name)==0 && strcmp(p, phone)==0) {
                // skip this line (cancel)
                found = 1;
                roomno = r;
                continue;
            }
        }
        fputs(buf, out);
    }
    fclose(f);
    fclose(out);
    if(found) {
        // Rename temp -> original
        if(remove(fname)!=0) {
            // try to overwrite by copying
        }
        rename(outpath, fname);
        // mark room free
        for(int i=0;i<hotel->room_count;i++) {
            if(hotel->rooms[i].room_number == roomno) hotel->rooms[i].is_reserved = 0;
        }
        return 1;
    } else {
        // no match: remove temp
        remove(outpath);
        return 0;
    }
}

// Edit nights for first matching booking
int edit_reservation_nights(Hotel* hotel, const char* name, const char* phone, int new_nights, const char* booking_filename) {
    if(!hotel || !name || !phone || new_nights<=0) return -1;
    const char* fname = booking_filename?booking_filename:"booking.txt";
    FILE* f = fopen(fname, "r");
    if(!f) return 0;
    char buf[512];
    int found = 0;
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.tmp", fname);
    FILE* out = fopen(outpath, "w");
    if(!out) { fclose(f); return -1; }
    while(fgets(buf, sizeof(buf), f)) {
        char date[16], n[128], p[64];
        int r, nights, price, total;
        if(sscanf(buf, "%15s %d %127s %63s %d %d %d", date, &r, n, p, &nights, &price, &total)==7) {
            if(!found && strcmp(n, name)==0 && strcmp(p, phone)==0) {
                // update nights/total
                total = price * new_nights;
                fprintf(out, "%s %d %s %s %d %d %d\n", date, r, n, p, new_nights, price, total);
                found = 1;
                continue;
            }
        }
        fputs(buf, out);
    }
    fclose(f);
    fclose(out);
    if(found) {
        if(remove(fname)!=0) { }
        rename(outpath, fname);
        return 1;
    } else {
        remove(outpath);
        return 0;
    }
}

void view_rooms_status(Hotel* hotel) {
    if(!hotel) return;
    int free_std=0, free_roy=0, free_suite=0;
    for(int i=0;i<hotel->room_count;i++) {
        Room* r = &hotel->rooms[i];
        if(!r->is_reserved) {
            if(strcmp(r->type,"Standard")==0) free_std++;
            else if(strcmp(r->type,"Royal")==0) free_roy++;
            else if(strcmp(r->type,"Suite")==0) free_suite++;
        }
    }
    printf("Rooms available - Standard: %d, Royal: %d, Suite: %d\n",
           free_std, free_roy, free_suite);
}

static int get_next_booking_id(const char* filename) {
    int maxid = 0;
    FILE* f = fopen(filename?filename:"booking.txt", "r");
    if(!f) return 1;
    while(1) {
        int id;
        if(fscanf(f, "%d", &id) != 1) break;
        if(id > maxid) maxid = id;
        // skip rest of line
        int c;
        while((c=fgetc(f))!=EOF && c!='\n');
    }
    fclose(f);
    return maxid + 1;
}

// New reservation: writes booking with an ID and returns booking_id (>0) on success, 0 on no room, -1 on error
int reserve_room_with_id(Hotel* hotel, const char* type,
                         const char* customer_name, const char* phone,
                         int nights, const char* booking_filename) {
    if(!hotel || !type || !customer_name || !phone || nights<=0) return -1;
    for(int i=0;i<hotel->room_count;i++) {
        Room* r = &hotel->rooms[i];
        if(!r->is_reserved && strcmp(r->type, type)==0) {
            r->is_reserved = 1;
            int price = price_for_type(hotel, r->type);
            int total = price * nights;
            int y,m,d; ymd_today(&y,&m,&d);
            const char* fname = booking_filename?booking_filename:"booking.txt";
            int bid = get_next_booking_id(fname);
            FILE* f = fopen(fname, "a");
            if(f) {
                // Format: ID YYYY-MM-DD roomNo roomType name phone nights price total
                fprintf(f, "%d %04d-%02d-%02d %d %s %s %s %d %d %d\n",
                    bid, y,m,d, r->room_number, r->type, customer_name, phone, nights, price, total);
                fclose(f);
                return bid;
            } else {
                // rollback reservation flag
                r->is_reserved = 0;
                return -1;
            }
        }
    }
    return 0; // no available room for that type
}

// Cancel by booking id
int cancel_reservation_by_id(Hotel* hotel, int booking_id, const char* booking_filename) {
    if(!hotel || booking_id<=0) return -1;
    const char* fname = booking_filename?booking_filename:"booking.txt";
    FILE* f = fopen(fname, "r");
    if(!f) return 0;
    char buf[512];
    int found = 0;
    int roomno = 0;
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.tmp", fname);
    FILE* out = fopen(outpath, "w");
    if(!out) { fclose(f); return -1; }
    while(fgets(buf, sizeof(buf), f)) {
        int id;
        // parse id then room
        char date[16], roomType[32], name[128], phone[64];
        int rnum, nights, price, total;
        if(sscanf(buf, "%d %15s %d %31s %127s %63s %d %d %d", &id, date, &rnum, roomType, name, phone, &nights, &price, &total) >= 1) {
            if(id == booking_id && !found) {
                found = 1;
                roomno = rnum;
                continue; // skip this line
            }
        }
        fputs(buf, out);
    }
    fclose(f);
    fclose(out);
    if(found) {
        if(remove(fname)!=0) { }
        rename(outpath, fname);
        for(int i=0;i<hotel->room_count;i++) {
            if(hotel->rooms[i].room_number == roomno) hotel->rooms[i].is_reserved = 0;
        }
        return 1;
    } else {
        remove(outpath);
        return 0;
    }
}

// Edit by booking id: allow changing type and nights. If type change, try to find a free room of new type; free old room.
int edit_reservation_by_id(Hotel* hotel, int booking_id, const char* new_type, int new_nights, const char* booking_filename) {
    if(!hotel || booking_id<=0 || new_nights<=0) return -1;
    const char* fname = booking_filename?booking_filename:"booking.txt";
    FILE* f = fopen(fname, "r");
    if(!f) return 0;
    char buf[512];
    int found = 0;
    char outpath[512];
    snprintf(outpath, sizeof(outpath), "%s.tmp", fname);
    FILE* out = fopen(outpath, "w");
    if(!out) { fclose(f); return -1; }
    while(fgets(buf, sizeof(buf), f)) {
        int id;
        char date[16], roomType[32], name[128], phone[64];
        int rnum, nights, price, total;
        if(sscanf(buf, "%d %15s %d %31s %127s %63s %d %d %d", &id, date, &rnum, roomType, name, phone, &nights, &price, &total) >= 1) {
            if(id == booking_id && !found) {
                found = 1;
                // If type change requested and different
                char finalType[32];
                int finalRoom = rnum;
                int finalPrice = price;
                if(new_type && strlen(new_type)>0 && strcmp(new_type, roomType)!=0) {
                    // try to find a free room of new_type
                    int newRoom = 0;
                    for(int i=0;i<hotel->room_count;i++) {
                        if(!hotel->rooms[i].is_reserved && strcmp(hotel->rooms[i].type, new_type)==0) {
                            newRoom = hotel->rooms[i].room_number;
                            hotel->rooms[i].is_reserved = 1;
                            break;
                        }
                    }
                    if(newRoom==0) {
                        // cannot change type - keep original
                        strcpy(finalType, roomType);
                        finalRoom = rnum;
                    } else {
                        // free old room
                        for(int i=0;i<hotel->room_count;i++) {
                            if(hotel->rooms[i].room_number == rnum) hotel->rooms[i].is_reserved = 0;
                        }
                        strcpy(finalType, new_type);
                        finalRoom = newRoom;
                        finalPrice = price_for_type(hotel, finalType);
                    }
                } else {
                    strcpy(finalType, roomType);
                    finalRoom = rnum;
                    finalPrice = price_for_type(hotel, finalType);
                }
                int finalNights = new_nights>0 ? new_nights : nights;
                int finalTotal = finalPrice * finalNights;
                fprintf(out, "%d %s %d %s %s %s %d %d %d\n", id, date, finalRoom, finalType, name, phone, finalNights, finalPrice, finalTotal);
                continue;
            }
        }
        fputs(buf, out);
    }
    fclose(f);
    fclose(out);
    if(found) {
        if(remove(fname)!=0) { }
        rename(outpath, fname);
        return 1;
    } else {
        remove(outpath);
        return 0;
    }
}
