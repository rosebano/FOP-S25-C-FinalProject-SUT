#ifndef ROOM_H
#define ROOM_H

#include <stddef.h>

typedef struct {
    int room_number;
    char type[16];     // "Standard", "Royal", "Suite"
    int capacity;
    int is_reserved;   // 0 = available, 1 = reserved
} Room;

typedef struct Hotel {
    Room* rooms;
    int total_rooms;   // allocated size
    int room_count;    // used size
    int price_standard;
    int price_royal;
    int price_suite;
} Hotel;

// Hotel / Room management
Hotel* hotel_init(int total_rooms);
void   hotel_free(Hotel* hotel);

void   load_prices(Hotel* hotel, const char* filename);
void   load_rooms(Hotel* hotel, const char* filename);
void   save_rooms(Hotel* hotel, const char* filename);
void   generate_rooms_automatically(Hotel* hotel, const char* filename);

int    reserve_room(Hotel* hotel, const char* type,
                    const char* customer_name, const char* phone,
                    int nights, const char* booking_filename);

// New reservation with booking ID
int    reserve_room_with_id(Hotel* hotel, const char* type,
                           const char* customer_name, const char* phone,
                           int nights, const char* booking_filename);

// Cancellation and editing
// Cancel the first matching booking for (name, phone). Returns 1 if cancelled, 0 if not found, -1 on error.
int    cancel_reservation(Hotel* hotel, const char* name, const char* phone, const char* booking_filename);
// Cancel by booking ID
int    cancel_reservation_by_id(Hotel* hotel, int booking_id, const char* booking_filename);

// Edit nights for the first matching booking for (name, phone). Returns 1 if updated, 0 if not found, -1 on error.
int    edit_reservation_nights(Hotel* hotel, const char* name, const char* phone, int new_nights, const char* booking_filename);
// Edit by booking ID
int    edit_reservation_by_id(Hotel* hotel, int booking_id, const char* new_type, 
                             int new_nights, const char* booking_filename);

void   view_rooms_status(Hotel* hotel);

#endif