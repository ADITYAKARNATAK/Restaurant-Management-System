/*
   RESTAURANT TABLE RESERVATION AND MANAGEMENT SYSTEM
   Main integration file.

   File handling is implemented here to keep the project limited to
   four member header files and one main integration file.

   Only actual record fields are written to disk.
   Linked-list pointers are NEVER written to a file.
*/

#include "member4_orders_billing.h"

#define DATA_FILE "restaurant_data.dat"
#define TEMP_FILE "restaurant_data.tmp"
#define BACKUP_FILE "restaurant_data.bak"
#define CORRUPT_FILE "restaurant_data.corrupt"

#define DATA_VERSION 1
#define MAX_SAVED_WAITING MAX_WAITING_CUSTOMERS
#define MAX_SAVED_KITCHEN MAX_KITCHEN_TASKS

typedef struct
{
    char signature[8];
    int version;
    int reservationCount;
    int waitingCount;
    int kitchenCount;
    int nextReservation;
    int nextWaiting;
    double sales;
} DataHeader;

/* Disk structures contain values only. They do not contain pointers. */
typedef struct
{
    int id;
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int partySize;
    int tableId;
    int status;
} SavedReservation;

typedef struct
{
    int id;
    char name[MAX_NAME_LENGTH];
    char phone[MAX_PHONE_LENGTH];
    int partySize;
} SavedWaitingCustomer;

typedef struct
{
    int itemCount;
    OrderItem items[MAX_ORDER_ITEMS];
} SavedTableOrder;

typedef struct
{
    int tableId;
    int itemId;
    int quantity;
} SavedKitchenTask;

static int writeExact(FILE *file, const void *data,
                      size_t size, size_t count)
{
    return fwrite(data, size, count, file) == count;
}

static int readExact(FILE *file, void *data,
                     size_t size, size_t count)
{
    return fread(data, size, count, file) == count;
}

/* Count current linked-list records before writing the file header. */
static void getRecordCounts(int *reservationCount,
                            int *waitingCount,
                            int *kitchenCount)
{
    Reservation *reservation = reservationHead;
    WaitingCustomer *waiting = waitingFront;
    KitchenNode *kitchen = kitchenFront;

    *reservationCount = 0;
    *waitingCount = 0;
    *kitchenCount = 0;

    while (reservation != NULL)
    {
        (*reservationCount)++;
        reservation = reservation->next;
    }

    while (waiting != NULL)
    {
        (*waitingCount)++;
        waiting = waiting->next;
    }

    while (kitchen != NULL)
    {
        (*kitchenCount)++;
        kitchen = kitchen->next;
    }
}

/* Save all active records to a temporary file first.
   The previous data file is retained as a backup until the new file
   has been successfully installed. */
static int saveRestaurantData(void)
{
    FILE *file;
    DataHeader header;

    Reservation *reservation;
    WaitingCustomer *waiting;
    KitchenNode *kitchen;

    SavedReservation savedReservation;
    SavedWaitingCustomer savedWaiting;
    SavedTableOrder savedOrder;
    SavedKitchenTask savedKitchen;

    int i;
    int writeSucceeded = 1;

    memset(&header, 0, sizeof(header));

    memcpy(header.signature, "RSTDATA1", 8);
    header.version = DATA_VERSION;

    getRecordCounts(&header.reservationCount,
                    &header.waitingCount,
                    &header.kitchenCount);

    header.nextReservation = nextReservationId;
    header.nextWaiting = nextWaitingId;
    header.sales = totalSales;

    if (header.reservationCount > MAX_TABLES ||
        header.waitingCount > MAX_SAVED_WAITING ||
        header.kitchenCount > MAX_SAVED_KITCHEN)
    {
        printf("Save failed: record count exceeds the supported limit.\n");
        return 0;
    }

    file = fopen(TEMP_FILE, "wb");

    if (file == NULL)
    {
        printf("Warning: could not open temporary data file.\n");
        return 0;
    }

    if (!writeExact(file, &header, sizeof(header), 1))
    {
        writeSucceeded = 0;
    }

    /* Save active reservations. */
    reservation = reservationHead;

    while (writeSucceeded && reservation != NULL)
    {
        memset(&savedReservation, 0, sizeof(savedReservation));

        savedReservation.id = reservation->reservationId;

        snprintf(savedReservation.name,
                 sizeof(savedReservation.name), "%s",
                 reservation->customerName);

        snprintf(savedReservation.phone,
                 sizeof(savedReservation.phone), "%s",
                 reservation->phone);

        savedReservation.partySize = reservation->partySize;
        savedReservation.tableId = reservation->tableId;
        savedReservation.status = reservation->status;

        if (!writeExact(file, &savedReservation,
                        sizeof(savedReservation), 1))
        {
            writeSucceeded = 0;
        }

        reservation = reservation->next;
    }

    /* Save waiting customers in their queue order. */
    waiting = waitingFront;

    while (writeSucceeded && waiting != NULL)
    {
        memset(&savedWaiting, 0, sizeof(savedWaiting));

        savedWaiting.id = waiting->waitingId;

        snprintf(savedWaiting.name,
                 sizeof(savedWaiting.name), "%s",
                 waiting->customerName);

        snprintf(savedWaiting.phone,
                 sizeof(savedWaiting.phone), "%s",
                 waiting->phone);

        savedWaiting.partySize = waiting->partySize;

        if (!writeExact(file, &savedWaiting,
                        sizeof(savedWaiting), 1))
        {
            writeSucceeded = 0;
        }

        waiting = waiting->next;
    }

    /* Save the active order of every table. */
    for (i = 0; writeSucceeded && i < MAX_TABLES; i++)
    {
        memset(&savedOrder, 0, sizeof(savedOrder));

        savedOrder.itemCount = tableOrders[i].itemCount;

        memcpy(savedOrder.items,
               tableOrders[i].items,
               sizeof(savedOrder.items));

        if (!writeExact(file, &savedOrder, sizeof(savedOrder), 1))
        {
            writeSucceeded = 0;
        }
    }

    /* Save pending kitchen tasks in FIFO order. */
    kitchen = kitchenFront;

    while (writeSucceeded && kitchen != NULL)
    {
        savedKitchen.tableId = kitchen->tableId;
        savedKitchen.itemId = kitchen->itemId;
        savedKitchen.quantity = kitchen->quantity;

        if (!writeExact(file, &savedKitchen,
                        sizeof(savedKitchen), 1))
        {
            writeSucceeded = 0;
        }

        kitchen = kitchen->next;
    }

    if (fflush(file) != 0)
    {
        writeSucceeded = 0;
    }

    if (fclose(file) != 0)
    {
        writeSucceeded = 0;
    }

    if (!writeSucceeded)
    {
        remove(TEMP_FILE);

        printf("Warning: data could not be saved.\n");
        printf("The previous data file was kept.\n");

        return 0;
    }

    /* Protect the old file before installing the new snapshot. */
    remove(BACKUP_FILE);

    if (rename(DATA_FILE, BACKUP_FILE) != 0 && errno != ENOENT)
    {
        remove(TEMP_FILE);

        printf("Warning: could not protect the previous data file.\n");
        return 0;
    }

    if (rename(TEMP_FILE, DATA_FILE) != 0)
    {
        /* Restore the previous file if installation fails. */
        rename(BACKUP_FILE, DATA_FILE);
        remove(TEMP_FILE);

        printf("Warning: could not install the updated data file.\n");
        return 0;
    }

    remove(BACKUP_FILE);

    return 1;
}

/* Reconstruct one reservation from its saved values.
   The hash table and table status are rebuilt here. */
static int appendLoadedReservation(const SavedReservation *saved)
{
    Reservation *node;
    Reservation *current;
    int tableIndex;

    if (saved->id < 1 || saved->id >= INT_MAX ||
        saved->partySize < 1 || saved->partySize > 6 ||
        saved->status < RESERVATION_RESERVED ||
        saved->status > RESERVATION_SEATED ||
        saved->name[0] == '\0' ||
        saved->phone[0] == '\0')
    {
        return 0;
    }

    tableIndex = findTableIndex(saved->tableId);

    if (tableIndex == -1 ||
        tables[tableIndex].status != TABLE_AVAILABLE ||
        tables[tableIndex].capacity < saved->partySize)
    {
        return 0;
    }

    if (findReservation(saved->id) != NULL)
    {
        return 0;
    }

    node = (Reservation *)malloc(sizeof(Reservation));

    if (node == NULL)
    {
        return 0;
    }

    node->reservationId = saved->id;

    snprintf(node->customerName,
             sizeof(node->customerName), "%s", saved->name);

    snprintf(node->phone,
             sizeof(node->phone), "%s", saved->phone);

    node->partySize = saved->partySize;
    node->tableId = saved->tableId;
    node->status = saved->status;
    node->next = NULL;

    if (!addReservationToHash(node))
    {
        free(node);
        return 0;
    }

    /* Append to preserve the saved linked-list order. */
    if (reservationHead == NULL)
    {
        reservationHead = node;
    }
    else
    {
        current = reservationHead;

        while (current->next != NULL)
        {
            current = current->next;
        }

        current->next = node;
    }

    if (saved->status == RESERVATION_RESERVED)
    {
        tables[tableIndex].status = TABLE_RESERVED;
    }
    else
    {
        tables[tableIndex].status = TABLE_OCCUPIED;
    }

    if (saved->id >= nextReservationId)
    {
        nextReservationId = saved->id + 1;
    }

    return 1;
}

/* Reconstruct a waiting queue node from its saved values. */
static int appendLoadedWaitingCustomer(const SavedWaitingCustomer *saved)
{
    WaitingCustomer *node;

    if (saved->id < 1 || saved->id >= INT_MAX ||
        saved->partySize < 1 || saved->partySize > 6 ||
        saved->name[0] == '\0' ||
        saved->phone[0] == '\0' ||
        waitingIdExists(saved->id))
    {
        return 0;
    }

    node = (WaitingCustomer *)malloc(sizeof(WaitingCustomer));

    if (node == NULL)
    {
        return 0;
    }

    node->waitingId = saved->id;

    snprintf(node->customerName,
             sizeof(node->customerName), "%s", saved->name);

    snprintf(node->phone,
             sizeof(node->phone), "%s", saved->phone);

    node->partySize = saved->partySize;
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

    if (saved->id >= nextWaitingId)
    {
        nextWaitingId = saved->id + 1;
    }

    return 1;
}

/* Clear partially loaded records if a saved file is damaged. */
static void resetLoadedData(void)
{
    freeWaitingQueue();
    freeKitchenQueue();
    freeReservationData();

    memset(tableOrders, 0, sizeof(tableOrders));

    initializeTables();

    nextReservationId = 1001;
    nextWaitingId = 1;
    totalSales = 0.0;
}

/* Load saved records and rebuild all in-memory data structures. */
static int loadRestaurantData(void)
{
    FILE *file;

    DataHeader header;

    SavedReservation savedReservation;
    SavedWaitingCustomer savedWaiting;
    SavedTableOrder savedOrder;
    SavedKitchenTask savedKitchen;

    KitchenNode *node;

    int i;
    int j;
    int loadedSuccessfully = 1;
    int tableIndex;

    file = fopen(DATA_FILE, "rb");

    if (file == NULL)
    {
        printf("No previous data found. Starting a new restaurant session.\n");
        return 1;
    }

    if (!readExact(file, &header, sizeof(header), 1) ||
        memcmp(header.signature, "RSTDATA1", 8) != 0 ||
        header.version != DATA_VERSION ||
        header.reservationCount < 0 ||
        header.reservationCount > MAX_TABLES ||
        header.waitingCount < 0 ||
        header.waitingCount > MAX_SAVED_WAITING ||
        header.kitchenCount < 0 ||
        header.kitchenCount > MAX_SAVED_KITCHEN ||
        header.nextReservation < 1001 ||
        header.nextWaiting < 1 ||
        header.sales < 0.0)
    {
        fclose(file);

        printf("Saved data file is invalid or incompatible.\n");
        printf("The file will be preserved as restaurant_data.corrupt.\n");

        remove(CORRUPT_FILE);
        rename(DATA_FILE, CORRUPT_FILE);

        return 0;
    }

    nextReservationId = header.nextReservation;
    nextWaitingId = header.nextWaiting;
    totalSales = header.sales;

    /* Load reservations and rebuild the linked list and hash table. */
    for (i = 0; i < header.reservationCount; i++)
    {
        if (!readExact(file, &savedReservation,
                       sizeof(savedReservation), 1) ||
            !appendLoadedReservation(&savedReservation))
        {
            loadedSuccessfully = 0;
            break;
        }
    }

    /* Load waiting customers in their original arrival order. */
    for (i = 0; loadedSuccessfully && i < header.waitingCount; i++)
    {
        if (!readExact(file, &savedWaiting,
                       sizeof(savedWaiting), 1) ||
            !appendLoadedWaitingCustomer(&savedWaiting))
        {
            loadedSuccessfully = 0;
            break;
        }
    }

    /* Load each table's active order. */
    for (i = 0; loadedSuccessfully && i < MAX_TABLES; i++)
    {
        if (!readExact(file, &savedOrder, sizeof(savedOrder), 1) ||
            savedOrder.itemCount < 0 ||
            savedOrder.itemCount > MAX_ORDER_ITEMS)
        {
            loadedSuccessfully = 0;
            break;
        }

        tableIndex = i;

        if (savedOrder.itemCount > 0 &&
            tables[tableIndex].status != TABLE_OCCUPIED)
        {
            loadedSuccessfully = 0;
            break;
        }

        tableOrders[tableIndex].itemCount = savedOrder.itemCount;

        for (j = 0; j < savedOrder.itemCount; j++)
        {
            if (findMenuItemIndex(savedOrder.items[j].itemId) == -1 ||
                savedOrder.items[j].quantity < 1)
            {
                loadedSuccessfully = 0;
                break;
            }

            tableOrders[tableIndex].items[j] = savedOrder.items[j];
        }

        if (!loadedSuccessfully)
        {
            break;
        }
    }

    /* Reconstruct the pending kitchen queue. */
    for (i = 0; loadedSuccessfully && i < header.kitchenCount; i++)
    {
        if (!readExact(file, &savedKitchen, sizeof(savedKitchen), 1))
        {
            loadedSuccessfully = 0;
            break;
        }

        tableIndex = findTableIndex(savedKitchen.tableId);

        if (tableIndex == -1 ||
            tables[tableIndex].status != TABLE_OCCUPIED ||
            findMenuItemIndex(savedKitchen.itemId) == -1 ||
            savedKitchen.quantity < 1)
        {
            loadedSuccessfully = 0;
            break;
        }

        node = (KitchenNode *)malloc(sizeof(KitchenNode));

        if (node == NULL)
        {
            loadedSuccessfully = 0;
            break;
        }

        node->tableId = savedKitchen.tableId;
        node->itemId = savedKitchen.itemId;
        node->quantity = savedKitchen.quantity;
        node->next = NULL;

        if (kitchenRear == NULL)
        {
            kitchenFront = node;
            kitchenRear = node;
        }
        else
        {
            kitchenRear->next = node;
            kitchenRear = node;
        }
    }

    /* Reject unexpected extra data at the end of the file. */
    if (loadedSuccessfully && fgetc(file) != EOF)
    {
        loadedSuccessfully = 0;
    }

    fclose(file);

    if (!loadedSuccessfully)
    {
        resetLoadedData();

        printf("Saved data was incomplete or inconsistent.\n");
        printf("Starting with empty records.\n");
        printf("The file will be preserved as restaurant_data.corrupt.\n");

        remove(CORRUPT_FILE);
        rename(DATA_FILE, CORRUPT_FILE);

        return 0;
    }

    printf("Saved restaurant records loaded successfully.\n");

    return 1;
}

/* Terminal interface. */
static void displayHeader(void)
{
    printf("\033[1;36m");

    printf("\n==============================================================\n");
    printf("              RESTAURANT MANAGEMENT SYSTEM                   \n");
    printf("==============================================================\n");

    printf("\033[0m");

    printf("  TABLES | RESERVATIONS | WAITING | ORDERS | BILLING | REPORTS\n");
    printf("--------------------------------------------------------------\n");
}

static void displayMainMenu(void)
{
    displayHeader();

    printf("\n  TABLE MANAGEMENT\n");
    printf("  [1]  View all tables\n");
    printf("  [2]  View table statistics\n");

    printf("\n  RESERVATION MANAGEMENT\n");
    printf("  [3]  Create reservation\n");
    printf("  [4]  Search reservation\n");
    printf("  [5]  View active reservations\n");
    printf("  [6]  Modify reservation\n");
    printf("  [7]  Cancel reservation\n");
    printf("  [8]  Check in customer\n");
    printf("  [9]  Sort reservations by party size\n");

    printf("\n  WAITING QUEUE\n");
    printf("  [10] Add customer to waiting queue\n");
    printf("  [11] View waiting queue\n");
    printf("  [12] Cancel waiting customer\n");

    printf("\n  ORDERS AND BILLING\n");
    printf("  [13] Display food menu\n");
    printf("  [14] Add food order\n");
    printf("  [15] View bill\n");
    printf("  [16] Checkout table\n");

    printf("\n  KITCHEN AND REPORTS\n");
    printf("  [17] View kitchen queue\n");
    printf("  [18] Complete next kitchen task\n");
    printf("  [19] View restaurant report\n");
    printf("  [20] View recent action history\n");

    printf("\n  [0]  Save and exit\n");
    printf("--------------------------------------------------------------\n");
}

int main(void)
{
    int choice;

    /* First initialize the physical tables, then restore saved records. */
    initializeTables();
    loadRestaurantData();

    do
    {
        displayMainMenu();

        choice = readInteger("  Select an option: ", 0, 20);

        printf("\n");

        switch (choice)
        {
            /* Table management */
            case 1:
                displayTables();
                break;

            case 2:
                displayTableStatistics();
                break;

            /* Reservation management */
            case 3:
                makeReservation();
                break;

            case 4:
                searchReservationMenu();
                break;

            case 5:
                displayAllReservations();
                break;

            case 6:
                modifyReservation();
                break;

            case 7:
                cancelReservation();
                assignWaitingCustomers();
                break;

            case 8:
                checkInReservation();
                break;

            case 9:
                sortReservationsByPartySize();
                break;

            /* Waiting queue */
            case 10:
                enqueueCustomer();
                assignWaitingCustomers();
                break;

            case 11:
                displayWaitingQueue();
                break;

            case 12:
                cancelWaitingCustomer();
                break;

            /* Orders and billing */
            case 13:
                displayMenu();
                break;

            case 14:
                addFoodOrder();
                break;

            case 15:
                viewBill();
                break;

            case 16:
                checkoutTable();
                break;

            /* Kitchen management */
            case 17:
                displayKitchenQueue();
                break;

            case 18:
                completeNextKitchenTask();
                break;

            /* Reports and action history */
            case 19:
                displaySalesReport();
                break;

            case 20:
                displayActionHistory();
                break;

            case 0:
                printf("Saving records before exit...\n");
                break;

            default:
                printf("Invalid menu option.\n");
                break;
        }

        /* Save after every menu operation. This also preserves changes
           made by automatic waiting-queue allocation. */
        if (!saveRestaurantData())
        {
            printf("WARNING: Current changes may not survive a restart.\n");
        }

        if (choice != 0)
        {
            char pauseBuffer[8];

            printf("\nPress ENTER to return to the main menu...");

            if (fgets(pauseBuffer, sizeof(pauseBuffer), stdin) == NULL)
            {
                break;
            }
        }

    } while (choice != 0);

    /* Final save and memory cleanup. */
    if (saveRestaurantData())
    {
        printf("All active records saved successfully.\n");
    }

    freeWaitingQueue();
    freeKitchenQueue();
    freeReservationData();

    printf("Thank you for using Restaurant Management System.\n");

    return 0;
}