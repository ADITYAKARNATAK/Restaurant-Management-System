#ifndef MEMBER2_RESERVATIONS_H
#define MEMBER2_RESERVATIONS_H

/* MEMBER 2: RESERVATION MANAGEMENT
   Linked list stores reservation records. A hash table provides faster
   lookup by reservation ID. Separate nodes are used for hash collisions. */

#include "member1_tables.h"

#define HASH_SIZE 101

typedef struct Reservation
{
    int reservationId;
    char customerName[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int partySize;
    int tableId;
    int status;
    struct Reservation *next;
} Reservation;

typedef struct HashNode
{
    int reservationId;
    Reservation *reservation;
    struct HashNode *next;
} HashNode;

static Reservation *reservationHead = NULL;
static HashNode *reservationHash[HASH_SIZE] = {NULL};
static int nextReservationId = 1001;

static int reservationHashIndex(int id)
{
    return id % HASH_SIZE;
}

static int addReservationToHash(Reservation *reservation)
{
    int index = reservationHashIndex(reservation->reservationId);
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));

    if (node == NULL)
    {
        return 0;
    }

    node->reservationId = reservation->reservationId;
    node->reservation = reservation;
    node->next = reservationHash[index];
    reservationHash[index] = node;

    return 1;
}

static void removeReservationFromHash(int id)
{
    int index = reservationHashIndex(id);
    HashNode *current = reservationHash[index];
    HashNode *previous = NULL;

    while (current != NULL)
    {
        if (current->reservationId == id)
        {
            if (previous == NULL)
            {
                reservationHash[index] = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            free(current);
            return;
        }

        previous = current;
        current = current->next;
    }
}

static Reservation *findReservation(int id)
{
    int index;
    HashNode *current;

    if (id < 1)
    {
        return NULL;
    }

    index = reservationHashIndex(id);
    current = reservationHash[index];

    while (current != NULL)
    {
        if (current->reservationId == id)
        {
            return current->reservation;
        }

        current = current->next;
    }

    return NULL;
}

/* Used by normal reservation creation and by the waiting queue. */
static Reservation *createReservationForTable(
    const char *name,
    const char *phone,
    int partySize,
    int tableIndex)
{
    Reservation *node;

    if (name == NULL || phone == NULL || name[0] == '\0' ||
        phone[0] == '\0' || nextReservationId >= INT_MAX ||
        partySize < 1 || partySize > 6 ||
        tableIndex < 0 || tableIndex >= MAX_TABLES ||
        tables[tableIndex].status != TABLE_AVAILABLE ||
        tables[tableIndex].capacity < partySize)
    {
        return NULL;
    }

    node = (Reservation *)malloc(sizeof(Reservation));

    if (node == NULL)
    {
        return NULL;
    }

    node->reservationId = nextReservationId;
    snprintf(node->customerName, sizeof(node->customerName), "%s", name);
    snprintf(node->phone, sizeof(node->phone), "%s", phone);
    node->partySize = partySize;
    node->tableId = tables[tableIndex].tableId;
    node->status = RESERVATION_RESERVED;
    node->next = NULL;

    /* Allocate the hash node before changing the list or table. */
    if (!addReservationToHash(node))
    {
        free(node);
        return NULL;
    }

    node->next = reservationHead;
    reservationHead = node;
    tables[tableIndex].status = TABLE_RESERVED;
    nextReservationId++;

    printf("Reservation created. ID: %d | Table: %d\n",
           node->reservationId, node->tableId);

    return node;
}

static void makeReservation(void)
{
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int partySize;
    int tableIndex;

    readText("Customer name: ", name, sizeof(name));
    readText("Contact number: ", phone, sizeof(phone));

    if (name[0] == '\0' || phone[0] == '\0')
    {
        printf("Name and contact number cannot be empty.\n");
        return;
    }

    partySize = readInteger("Party size (1-6): ", 1, 6);
    tableIndex = findBestFitTable(partySize);

    if (tableIndex == -1)
    {
        printf("No suitable table is available. Add the customer to the waiting queue.\n");
        return;
    }

    if (createReservationForTable(name, phone, partySize, tableIndex) == NULL)
    {
        printf("Reservation could not be created. No table was changed.\n");
    }
}

static const char *reservationStatusText(int status)
{
    if (status == RESERVATION_RESERVED)
    {
        return "Reserved";
    }

    if (status == RESERVATION_SEATED)
    {
        return "Seated";
    }

    return "Completed";
}

static void displayOneReservation(const Reservation *reservation)
{
    if (reservation == NULL)
    {
        return;
    }

    printf("\nReservation ID : %d\n", reservation->reservationId);
    printf("Customer       : %s\n", reservation->customerName);
    printf("Contact        : %s\n", reservation->phone);
    printf("Party size     : %d\n", reservation->partySize);
    printf("Table ID       : %d\n", reservation->tableId);
    printf("Status         : %s\n",
           reservationStatusText(reservation->status));
}

static void searchReservationMenu(void)
{
    int id = readInteger("Reservation ID: ", 1, INT_MAX);
    Reservation *reservation = findReservation(id);

    if (reservation == NULL)
    {
        printf("Reservation not found.\n");
    }
    else
    {
        displayOneReservation(reservation);
    }
}

static void displayAllReservations(void)
{
    Reservation *current = reservationHead;

    printf("\nACTIVE RESERVATIONS\n");

    if (current == NULL)
    {
        printf("No active reservations.\n");
        return;
    }

    printf("%-8s %-20s %-8s %-8s %-12s\n",
           "ID", "Customer", "Party", "Table", "Status");

    while (current != NULL)
    {
        printf("%-8d %-20.20s %-8d %-8d %-12s\n",
               current->reservationId, current->customerName,
               current->partySize, current->tableId,
               reservationStatusText(current->status));
        current = current->next;
    }
}

static void modifyReservation(void)
{
    int id = readInteger("Reservation ID to modify: ", 1, INT_MAX);
    Reservation *reservation = findReservation(id);
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int newPartySize;
    int oldTableIndex;
    int newTableIndex;

    if (reservation == NULL || reservation->status != RESERVATION_RESERVED)
    {
        printf("Only a reservation that has not checked in can be modified.\n");
        return;
    }

    readText("New customer name: ", name, sizeof(name));
    readText("New contact number: ", phone, sizeof(phone));

    if (name[0] == '\0' || phone[0] == '\0')
    {
        printf("Name and contact number cannot be empty. No changes made.\n");
        return;
    }

    newPartySize = readInteger("New party size (1-6): ", 1, 6);
    oldTableIndex = findTableIndex(reservation->tableId);

    if (oldTableIndex == -1 || tables[oldTableIndex].status != TABLE_RESERVED)
    {
        printf("Reservation and table data do not match. No changes made.\n");
        return;
    }

    /* Temporarily release the old table. Restore it if allocation fails. */
    tables[oldTableIndex].status = TABLE_AVAILABLE;
    newTableIndex = findBestFitTable(newPartySize);

    if (newTableIndex == -1)
    {
        tables[oldTableIndex].status = TABLE_RESERVED;
        printf("No suitable table for the new party size. Reservation unchanged.\n");
        return;
    }

    snprintf(reservation->customerName, sizeof(reservation->customerName), "%s", name);
    snprintf(reservation->phone, sizeof(reservation->phone), "%s", phone);
    reservation->partySize = newPartySize;
    reservation->tableId = tables[newTableIndex].tableId;
    tables[newTableIndex].status = TABLE_RESERVED;

    printf("Reservation updated successfully. New table: %d\n",
           reservation->tableId);
}

static int cancelReservationById(int id)
{
    Reservation *current = reservationHead;
    Reservation *previous = NULL;
    int tableIndex;

    while (current != NULL && current->reservationId != id)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL || current->status != RESERVATION_RESERVED)
    {
        return 0;
    }

    tableIndex = findTableIndex(current->tableId);

    if (tableIndex == -1 || tables[tableIndex].status != TABLE_RESERVED)
    {
        return 0;
    }

    if (previous == NULL)
    {
        reservationHead = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    tables[tableIndex].status = TABLE_AVAILABLE;
    removeReservationFromHash(id);
    free(current);

    return 1;
}

static void cancelReservation(void)
{
    int id = readInteger("Reservation ID to cancel: ", 1, INT_MAX);

    if (cancelReservationById(id))
    {
        printf("Reservation cancelled and table released.\n");
    }
    else
    {
        printf("Cancellation failed. Check the ID and reservation status.\n");
    }
}

static void checkInReservation(void)
{
    int id = readInteger("Reservation ID for check-in: ", 1, INT_MAX);
    Reservation *reservation = findReservation(id);
    int tableIndex;

    if (reservation == NULL || reservation->status != RESERVATION_RESERVED)
    {
        printf("Reservation not found or cannot be checked in.\n");
        return;
    }

    tableIndex = findTableIndex(reservation->tableId);

    if (tableIndex == -1 || tables[tableIndex].status != TABLE_RESERVED)
    {
        printf("Table status mismatch. Check-in was not performed.\n");
        return;
    }

    reservation->status = RESERVATION_SEATED;
    tables[tableIndex].status = TABLE_OCCUPIED;
    printf("Check-in successful. Customer seated at table %d.\n",
           reservation->tableId);
}

/* At checkout, remove the active reservation from both structures.
   The completed sale is retained separately as a cumulative sales total. */
static void completeReservationForTable(int tableId)
{
    Reservation *current = reservationHead;
    Reservation *previous = NULL;

    while (current != NULL)
    {
        if (current->tableId == tableId && current->status == RESERVATION_SEATED)
        {
            int reservationId = current->reservationId;

            if (previous == NULL)
            {
                reservationHead = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            removeReservationFromHash(reservationId);
            free(current);
            return;
        }

        previous = current;
        current = current->next;
    }
}

/* Sort pointers to records. The linked list itself remains unchanged. */
static void sortReservationsByPartySize(void)
{
    Reservation *current = reservationHead;
    Reservation **items;
    Reservation *temporary;
    int count = 0;
    int i;
    int j;

    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    if (count < 2)
    {
        printf("At least two active reservations are required.\n");
        return;
    }

    items = (Reservation **)malloc((size_t)count * sizeof(Reservation *));

    if (items == NULL)
    {
        printf("Not enough memory to sort reservations.\n");
        return;
    }

    current = reservationHead;

    for (i = 0; i < count; i++)
    {
        items[i] = current;
        current = current->next;
    }

    for (i = 0; i < count - 1; i++)
    {
        for (j = 0; j < count - i - 1; j++)
        {
            if (items[j]->partySize > items[j + 1]->partySize)
            {
                temporary = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temporary;
            }
        }
    }

    printf("\nRESERVATIONS SORTED BY PARTY SIZE\n");

    for (i = 0; i < count; i++)
    {
        printf("ID: %d | %-20s | Party: %d | Table: %d\n",
               items[i]->reservationId, items[i]->customerName,
               items[i]->partySize, items[i]->tableId);
    }

    free(items);
}

static void freeReservationData(void)
{
    Reservation *current = reservationHead;
    Reservation *nextReservation;
    HashNode *hashCurrent;
    HashNode *nextHash;
    int i;

    while (current != NULL)
    {
        nextReservation = current->next;
        free(current);
        current = nextReservation;
    }

    reservationHead = NULL;

    for (i = 0; i < HASH_SIZE; i++)
    {
        hashCurrent = reservationHash[i];

        while (hashCurrent != NULL)
        {
            nextHash = hashCurrent->next;
            free(hashCurrent);
            hashCurrent = nextHash;
        }

        reservationHash[i] = NULL;
    }
}

#endif