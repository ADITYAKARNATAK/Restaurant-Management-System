#ifndef MEMBER3_WAITING_QUEUE_H
#define MEMBER3_WAITING_QUEUE_H

/* MEMBER 3: WAITING QUEUE
   Front and rear pointers make normal enqueue and dequeue O(1). */

#include "member2_reservations.h"

#define MAX_WAITING_CUSTOMERS 1000

typedef struct WaitingCustomer
{
    int waitingId;
    char customerName[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int partySize;
    struct WaitingCustomer *next;
} WaitingCustomer;

static WaitingCustomer *waitingFront = NULL;
static WaitingCustomer *waitingRear = NULL;
static int nextWaitingId = 1;

static int getWaitingCount(void);

static int waitingIdExists(int id)
{
    WaitingCustomer *current = waitingFront;

    while (current != NULL)
    {
        if (current->waitingId == id)
        {
            return 1;
        }

        current = current->next;
    }

    return 0;
}

static void enqueueCustomer(void)
{
    WaitingCustomer *node;

    if (getWaitingCount() >= MAX_WAITING_CUSTOMERS ||
        nextWaitingId >= INT_MAX)
    {
        printf("Waiting queue is full or its ID limit has been reached.\n");
        return;
    }

    node = (WaitingCustomer *)malloc(sizeof(WaitingCustomer));

    if (node == NULL)
    {
        printf("Memory allocation failed. Customer was not added.\n");
        return;
    }

    readText("Customer name: ", node->customerName,
             sizeof(node->customerName));

    readText("Contact number: ", node->phone, sizeof(node->phone));

    if (node->customerName[0] == '\0' || node->phone[0] == '\0')
    {
        printf("Name and contact number cannot be empty.\n");
        free(node);
        return;
    }

    node->partySize = readInteger("Party size (1-6): ", 1, 6);
    node->waitingId = nextWaitingId++;
    node->next = NULL;

    if (waitingRear == NULL)
    {
        waitingFront = node;
        waitingRear = node;
    }
    else
    {
        waitingRear->next = node;
        waitingRear = node;
    }

    printf("Customer added to waiting queue. Waiting ID: %d\n",
           node->waitingId);
}

static void displayWaitingQueue(void)
{
    WaitingCustomer *current = waitingFront;
    int position = 1;

    printf("\nWAITING QUEUE\n");

    if (current == NULL)
    {
        printf("No customers are waiting.\n");
        return;
    }

    printf("%-7s %-10s %-22s %-8s\n",
           "Pos.", "Wait ID", "Customer", "Party");

    while (current != NULL)
    {
        printf("%-7d %-10d %-22.22s %-8d\n",
               position, current->waitingId,
               current->customerName, current->partySize);

        current = current->next;
        position++;
    }
}

static int getWaitingCount(void)
{
    WaitingCustomer *current = waitingFront;
    int count = 0;

    while (current != NULL)
    {
        count++;
        current = current->next;
    }

    return count;
}

static void cancelWaitingCustomer(void)
{
    int id = readInteger("Waiting ID to cancel: ", 1, INT_MAX);
    WaitingCustomer *current = waitingFront;
    WaitingCustomer *previous = NULL;

    while (current != NULL && current->waitingId != id)
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL)
    {
        printf("Waiting customer not found.\n");
        return;
    }

    if (previous == NULL)
    {
        waitingFront = current->next;
    }
    else
    {
        previous->next = current->next;
    }

    if (current == waitingRear)
    {
        waitingRear = previous;
    }

    free(current);
    printf("Customer removed from waiting queue.\n");
}

/* First-feasible policy:
   Scan customers in arrival order and choose the first person who fits an
   available table. Remove the queue node only after reservation creation
   succeeds, so an allocation failure cannot lose a waiting customer. */
static void assignWaitingCustomers(void)
{
    WaitingCustomer *current;
    WaitingCustomer *previous;
    WaitingCustomer *selected;
    WaitingCustomer *selectedPrevious;
    Reservation *reservation;
    int tableIndex;

    while (waitingFront != NULL)
    {
        current = waitingFront;
        previous = NULL;
        selected = NULL;
        selectedPrevious = NULL;
        tableIndex = -1;

        while (current != NULL)
        {
            tableIndex = findBestFitTable(current->partySize);

            if (tableIndex != -1)
            {
                selected = current;
                selectedPrevious = previous;
                break;
            }

            previous = current;
            current = current->next;
        }

        if (selected == NULL)
        {
            break;
        }

        reservation = createReservationForTable(
            selected->customerName,
            selected->phone,
            selected->partySize,
            tableIndex);

        if (reservation == NULL)
        {
            printf("Could not create reservation. Waiting customer remains in queue.\n");
            break;
        }

        if (selectedPrevious == NULL)
        {
            waitingFront = selected->next;
        }
        else
        {
            selectedPrevious->next = selected->next;
        }

        if (selected == waitingRear)
        {
            waitingRear = selectedPrevious;
        }

        printf("Waiting customer %s received reservation ID %d.\n",
               selected->customerName, reservation->reservationId);

        free(selected);
    }
}

static void freeWaitingQueue(void)
{
    WaitingCustomer *current = waitingFront;
    WaitingCustomer *next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    waitingFront = NULL;
    waitingRear = NULL;
}

#endif