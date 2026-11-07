#ifndef MEMBER4_ORDERS_BILLING_H
#define MEMBER4_ORDERS_BILLING_H

/* MEMBER 4: MENU, ORDERS, KITCHEN QUEUE, BILLING AND ACTION HISTORY. */

#include "member3_waiting_queue.h"

#define MENU_COUNT 6
#define MAX_ORDER_ITEMS 20
#define HISTORY_SIZE 30
#define MAX_KITCHEN_TASKS 5000
#define TAX_RATE 0.05f

typedef struct
{
    int itemId;
    char itemName[40];
    float price;
} MenuItem;

typedef struct
{
    int itemId;
    int quantity;
} OrderItem;

typedef struct
{
    int itemCount;
    OrderItem items[MAX_ORDER_ITEMS];
} TableOrder;

typedef struct KitchenNode
{
    int tableId;
    int itemId;
    int quantity;
    struct KitchenNode *next;
} KitchenNode;

static MenuItem menuItems[MENU_COUNT] =
{
    {1, "Veg Burger", 120.0f},
    {2, "Paneer Pizza", 250.0f},
    {3, "French Fries", 90.0f},
    {4, "Pasta", 180.0f},
    {5, "Cold Coffee", 100.0f},
    {6, "Mineral Water", 30.0f}
};

static TableOrder tableOrders[MAX_TABLES] = {{0}};

static KitchenNode *kitchenFront = NULL;
static KitchenNode *kitchenRear = NULL;

static char actionHistory[HISTORY_SIZE][100];
static int historyTop = -1;

static double totalSales = 0.0;

/* Stack: store the latest system actions for display. */
static void pushAction(const char *action)
{
    int i;

    if (historyTop == HISTORY_SIZE - 1)
    {
        /* Remove the oldest entry when the fixed-size stack is full. */
        for (i = 1; i < HISTORY_SIZE; i++)
        {
            memmove(actionHistory[i - 1], actionHistory[i],
                    sizeof(actionHistory[i - 1]));
        }

        historyTop--;
    }

    historyTop++;

    snprintf(actionHistory[historyTop],
             sizeof(actionHistory[historyTop]), "%s", action);
}

static void displayActionHistory(void)
{
    int i;

    printf("\nRECENT ACTIONS (NEWEST FIRST)\n");

    if (historyTop < 0)
    {
        printf("No actions recorded.\n");
        return;
    }

    for (i = historyTop; i >= 0; i--)
    {
        printf("%2d. %s\n", historyTop - i + 1, actionHistory[i]);
    }
}

static void displayMenu(void)
{
    int i;

    printf("\nFOOD MENU\n");
    printf("%-8s %-25s %-12s\n", "Item ID", "Item", "Price");

    for (i = 0; i < MENU_COUNT; i++)
    {
        printf("%-8d %-25s Rs. %-8.2f\n",
               menuItems[i].itemId, menuItems[i].itemName,
               menuItems[i].price);
    }
}

static int findMenuItemIndex(int itemId)
{
    int i;

    for (i = 0; i < MENU_COUNT; i++)
    {
        if (menuItems[i].itemId == itemId)
        {
            return i;
        }
    }

    return -1;
}

/* Add a kitchen task at the rear of the FIFO queue. */
static int addKitchenTask(int tableId, int itemId, int quantity)
{
    KitchenNode *current = kitchenFront;
    int taskCount = 0;
    KitchenNode *node;

    while (current != NULL)
    {
        taskCount++;
        current = current->next;
    }

    if (taskCount >= MAX_KITCHEN_TASKS)
    {
        return 0;
    }

    node = (KitchenNode *)malloc(sizeof(KitchenNode));

    if (node == NULL)
    {
        return 0;
    }

    node->tableId = tableId;
    node->itemId = itemId;
    node->quantity = quantity;
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

    return 1;
}

static void displayKitchenQueue(void)
{
    KitchenNode *current = kitchenFront;
    int position = 1;
    int itemIndex;

    printf("\nKITCHEN QUEUE (FIFO)\n");

    if (current == NULL)
    {
        printf("No pending kitchen tasks.\n");
        return;
    }

    while (current != NULL)
    {
        itemIndex = findMenuItemIndex(current->itemId);

        printf("%d. Table %d | %s | Quantity: %d\n",
               position, current->tableId,
               itemIndex == -1 ? "Unknown item" : menuItems[itemIndex].itemName,
               current->quantity);

        current = current->next;
        position++;
    }
}

/* Complete and remove the oldest kitchen task. */
static void completeNextKitchenTask(void)
{
    KitchenNode *node;
    int itemIndex;
    char message[100];

    if (kitchenFront == NULL)
    {
        printf("Kitchen queue is empty.\n");
        return;
    }

    node = kitchenFront;
    kitchenFront = kitchenFront->next;

    if (kitchenFront == NULL)
    {
        kitchenRear = NULL;
    }

    itemIndex = findMenuItemIndex(node->itemId);

    printf("Completed: Table %d | %s | Quantity %d\n",
           node->tableId,
           itemIndex == -1 ? "Unknown item" : menuItems[itemIndex].itemName,
           node->quantity);

    snprintf(message, sizeof(message),
             "Kitchen task completed for table %d", node->tableId);

    pushAction(message);
    free(node);
}

static int hasPendingKitchenTasks(int tableId)
{
    KitchenNode *current = kitchenFront;

    while (current != NULL)
    {
        if (current->tableId == tableId)
        {
            return 1;
        }

        current = current->next;
    }

    return 0;
}

static void addFoodOrder(void)
{
    int tableId;
    int tableIndex;
    int itemId;
    int itemIndex;
    int quantity;
    int orderIndex;
    int isNewItem;
    int i;
    char message[100];

    tableId = readInteger("Occupied table ID: ", 1, MAX_TABLES);
    tableIndex = findTableIndex(tableId);

    if (tableIndex == -1 || tables[tableIndex].status != TABLE_OCCUPIED)
    {
        printf("Orders can only be added to an occupied table.\n");
        return;
    }

    displayMenu();

    while (1)
    {
        itemId = readInteger("Item ID (0 to finish): ", 0, MENU_COUNT);

        if (itemId == 0)
        {
            break;
        }

        itemIndex = findMenuItemIndex(itemId);

        if (itemIndex == -1)
        {
            printf("Invalid menu item.\n");
            continue;
        }

        quantity = readInteger("Quantity (1-100): ", 1, 100);
        orderIndex = -1;
        isNewItem = 1;

        /* Check whether this item already exists in the table's order. */
        for (i = 0; i < tableOrders[tableIndex].itemCount; i++)
        {
            if (tableOrders[tableIndex].items[i].itemId == itemId)
            {
                orderIndex = i;
                isNewItem = 0;
                break;
            }
        }

        if (isNewItem && tableOrders[tableIndex].itemCount >= MAX_ORDER_ITEMS)
        {
            printf("The order has reached its distinct-item limit.\n");
            continue;
        }

        if (isNewItem == 0 &&
            tableOrders[tableIndex].items[orderIndex].quantity > INT_MAX - quantity)
        {
            printf("Quantity is too large.\n");
            continue;
        }

        /* Add the kitchen task first. If allocation fails, do not change
           the customer's bill. */
        if (!addKitchenTask(tableId, itemId, quantity))
        {
            printf("Could not allocate memory. This item was not added.\n");
            continue;
        }

        if (isNewItem)
        {
            orderIndex = tableOrders[tableIndex].itemCount;

            tableOrders[tableIndex].items[orderIndex].itemId = itemId;
            tableOrders[tableIndex].items[orderIndex].quantity = 0;

            tableOrders[tableIndex].itemCount++;
        }

        tableOrders[tableIndex].items[orderIndex].quantity += quantity;

        printf("Added %d x %s.\n", quantity, menuItems[itemIndex].itemName);
    }

    snprintf(message, sizeof(message), "Food order updated for table %d", tableId);
    pushAction(message);
}

/* Calculate the bill and optionally display its details. */
static double calculateOrderTotal(int tableIndex, int printBill)
{
    int i;
    int itemIndex;
    double subtotal = 0.0;
    double tax;
    double total;

    if (tableIndex < 0 || tableIndex >= MAX_TABLES)
    {
        return 0.0;
    }

    if (printBill)
    {
        printf("\n================ ITEMIZED BILL ================\n");
        printf("Table ID: %d\n", tables[tableIndex].tableId);
        printf("%-25s %-10s %-12s\n", "Item", "Quantity", "Amount");
    }

    for (i = 0; i < tableOrders[tableIndex].itemCount; i++)
    {
        itemIndex = findMenuItemIndex(tableOrders[tableIndex].items[i].itemId);

        if (itemIndex != -1)
        {
            double amount = (double)menuItems[itemIndex].price *
                            tableOrders[tableIndex].items[i].quantity;

            subtotal += amount;

            if (printBill)
            {
                printf("%-25s %-10d Rs. %-8.2f\n",
                       menuItems[itemIndex].itemName,
                       tableOrders[tableIndex].items[i].quantity,
                       amount);
            }
        }
    }

    tax = subtotal * TAX_RATE;
    total = subtotal + tax;

    if (printBill)
    {
        printf("------------------------------------------------\n");
        printf("Subtotal : Rs. %.2f\n", subtotal);
        printf("Tax (5%%) : Rs. %.2f\n", tax);
        printf("TOTAL    : Rs. %.2f\n", total);
        printf("================================================\n");
    }

    return total;
}

static void viewBill(void)
{
    int tableId = readInteger("Table ID: ", 1, MAX_TABLES);
    int tableIndex = findTableIndex(tableId);

    if (tableIndex == -1 || tables[tableIndex].status != TABLE_OCCUPIED)
    {
        printf("A bill can only be viewed for an occupied table.\n");
        return;
    }

    if (tableOrders[tableIndex].itemCount == 0)
    {
        printf("No food items have been ordered.\n");
        return;
    }

    calculateOrderTotal(tableIndex, 1);
}

static void checkoutTable(void)
{
    int tableId = readInteger("Table ID for checkout: ", 1, MAX_TABLES);
    int tableIndex = findTableIndex(tableId);
    int confirm;
    double billAmount;
    char message[100];

    if (tableIndex == -1 || tables[tableIndex].status != TABLE_OCCUPIED)
    {
        printf("This table is not occupied.\n");
        return;
    }

    if (hasPendingKitchenTasks(tableId))
    {
        printf("Complete all pending kitchen tasks before checkout.\n");
        return;
    }

    billAmount = calculateOrderTotal(tableIndex, 1);

    confirm = readInteger(
        "Confirm payment and checkout? (1 = Yes, 0 = No): ", 0, 1);

    if (confirm == 0)
    {
        printf("Checkout cancelled. No records were changed.\n");
        return;
    }

    totalSales += billAmount;

    /* Remove the active reservation and release the physical table. */
    completeReservationForTable(tableId);

    tables[tableIndex].status = TABLE_AVAILABLE;
    tableOrders[tableIndex].itemCount = 0;

    snprintf(message, sizeof(message),
             "Checkout completed for table %d", tableId);

    pushAction(message);

    printf("Payment recorded: Rs. %.2f. Table %d is now available.\n",
           billAmount, tableId);

    /* A newly available table may serve a waiting customer. */
    assignWaitingCustomers();
}

static void displaySalesReport(void)
{
    KitchenNode *current = kitchenFront;
    int pendingTasks = 0;

    while (current != NULL)
    {
        pendingTasks++;
        current = current->next;
    }

    printf("\nRESTAURANT REPORT\n");
    printf("Recorded sales       : Rs. %.2f\n", totalSales);
    printf("Waiting customers    : %d\n", getWaitingCount());
    printf("Pending kitchen tasks: %d\n", pendingTasks);

    displayTableStatistics();
}

static void freeKitchenQueue(void)
{
    KitchenNode *current = kitchenFront;
    KitchenNode *next;

    while (current != NULL)
    {
        next = current->next;
        free(current);
        current = next;
    }

    kitchenFront = NULL;
    kitchenRear = NULL;
}

#endif