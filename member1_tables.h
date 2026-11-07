#ifndef MEMBER1_TABLES_H
#define MEMBER1_TABLES_H

/* MEMBER 1: TABLE MANAGEMENT
   A fixed-size array is suitable because the restaurant has a fixed number
   of physical tables. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#define MAX_TABLES 9
#define MAX_NAME_LENGTH 50
#define MAX_PHONE_LENGTH 20

#define TABLE_AVAILABLE 0
#define TABLE_RESERVED 1
#define TABLE_OCCUPIED 2

#define RESERVATION_RESERVED 1
#define RESERVATION_SEATED 2
#define RESERVATION_COMPLETED 3

/* Read a complete line safely. Names may contain spaces. */
static void readText(const char *prompt, char *buffer, size_t size)
{
    size_t length;
    int ch;

    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL)
    {
        printf("\nInput ended. Closing the program.\n");
        exit(EXIT_SUCCESS);
    }

    length = strlen(buffer);

    if (length > 0 && buffer[length - 1] == '\n')
    {
        buffer[length - 1] = '\0';
    }
    else
    {
        while ((ch = getchar()) != '\n' && ch != EOF)
        {
            /* Discard characters that do not fit in the input buffer. */
        }
    }
}

/* Read and validate an integer without leaving invalid input in stdin. */
static int readInteger(const char *prompt, int minimum, int maximum)
{
    char input[100];
    char *end;
    long value;

    while (1)
    {
        readText(prompt, input, sizeof(input));

        errno = 0;
        value = strtol(input, &end, 10);

        while (*end == ' ' || *end == '\t')
        {
            end++;
        }

        if (input[0] != '\0' && end != input && *end == '\0' &&
            errno != ERANGE && value >= minimum && value <= maximum)
        {
            return (int)value;
        }

        printf("Please enter a whole number between %d and %d.\n",
               minimum, maximum);
    }
}

typedef struct
{
    int tableId;
    int capacity;
    int status;
} Table;

static Table tables[MAX_TABLES];

static void initializeTables(void)
{
    int capacities[MAX_TABLES] = {2, 2, 2, 4, 4, 4, 4, 6, 6};
    int i;

    for (i = 0; i < MAX_TABLES; i++)
    {
        tables[i].tableId = i + 1;
        tables[i].capacity = capacities[i];
        tables[i].status = TABLE_AVAILABLE;
    }
}

static int findTableIndex(int tableId)
{
    int i;

    for (i = 0; i < MAX_TABLES; i++)
    {
        if (tables[i].tableId == tableId)
        {
            return i;
        }
    }

    return -1;
}

/* Best-fit allocation: choose the smallest available table that can seat
   the complete party. Returns an array index, or -1 if none is suitable. */
static int findBestFitTable(int partySize)
{
    int i;
    int bestIndex = -1;

    if (partySize < 1 || partySize > 6)
    {
        return -1;
    }

    for (i = 0; i < MAX_TABLES; i++)
    {
        if (tables[i].status == TABLE_AVAILABLE &&
            tables[i].capacity >= partySize)
        {
            if (bestIndex == -1 ||
                tables[i].capacity < tables[bestIndex].capacity)
            {
                bestIndex = i;
            }
        }
    }

    return bestIndex;
}

static void displayTables(void)
{
    int i;
    const char *status;

    printf("\n+--------------------------------------------------+\n");
    printf("|                  TABLE STATUS                    |\n");
    printf("+----------+----------+----------------------------+\n");
    printf("| Table ID | Capacity | Status                     |\n");
    printf("+----------+----------+----------------------------+\n");

    for (i = 0; i < MAX_TABLES; i++)
    {
        if (tables[i].status == TABLE_AVAILABLE)
        {
            status = "Available";
        }
        else if (tables[i].status == TABLE_RESERVED)
        {
            status = "Reserved";
        }
        else
        {
            status = "Occupied";
        }

        printf("| %-8d | %-8d | %-26s |\n",
               tables[i].tableId, tables[i].capacity, status);
    }

    printf("+----------+----------+----------------------------+\n");
}

static void displayTableStatistics(void)
{
    int i;
    int available = 0;
    int reserved = 0;
    int occupied = 0;
    int totalSeats = 0;
    int freeSeats = 0;

    for (i = 0; i < MAX_TABLES; i++)
    {
        totalSeats += tables[i].capacity;

        if (tables[i].status == TABLE_AVAILABLE)
        {
            available++;
            freeSeats += tables[i].capacity;
        }
        else if (tables[i].status == TABLE_RESERVED)
        {
            reserved++;
        }
        else
        {
            occupied++;
        }
    }

    printf("\nTABLE STATISTICS\n");
    printf("Total tables          : %d\n", MAX_TABLES);
    printf("Available tables      : %d\n", available);
    printf("Reserved tables       : %d\n", reserved);
    printf("Occupied tables       : %d\n", occupied);
    printf("Total seating capacity: %d\n", totalSeats);
    printf("Seats in free tables  : %d\n", freeSeats);
    printf("Occupancy             : %.2f%%\n",
           ((float)(reserved + occupied) / MAX_TABLES) * 100.0f);
}

#endif