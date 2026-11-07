# Restaurant Table Reservation and Management System

A menu-driven, command-line application developed in **C** that manages restaurant tables, customer reservations, waiting lists, food orders, kitchen operations and billing using fundamental data structures.

## Project Overview

The Restaurant Table Reservation and Management System aims to simplify restaurant operations by organizing table allocation, reservations, waiting customers, food ordering and billing.

The system uses different data structures and algorithms to manage information efficiently while demonstrating their practical applications in a real-world scenario.

## Key Features

- **Table Management:** Manage restaurant tables, seating capacities and availability.
- **Best-Fit Table Allocation:** Assign the smallest suitable available table based on party size.
- **Reservation Management:** Create, search, modify, cancel and check in reservations.
- **Waiting List:** Maintain waiting customers and assign tables using the first-feasible-customer policy.
- **Food Ordering:** Display the menu and manage customer orders.
- **Kitchen Management:** Process kitchen tasks using FIFO order.
- **Billing and Checkout:** Calculate bills, apply demonstration tax and complete customer checkout.
- **Action History:** Maintain a record of recent system operations.
- **File Handling:** Save and restore active system data across program restarts.
- **Restaurant Analytics:** Display occupancy and sales information.

## Data Structures and Algorithms Used

| Data Structure / Algorithm | Application |
|---|---|
| Array | Restaurant table management |
| Singly Linked List | Reservation management |
| Hash Table | Reservation searching |
| Queue | Waiting customer management |
| Queue | Kitchen order processing |
| Stack | Action history |
| Bubble Sort | Sorting reservations by party size |
| Best-Fit Algorithm | Allocating suitable restaurant tables |

## Project Structure

```text
Restaurant-Management-System/
│
├── main.c
├── member1_tables.h
├── member2_reservations.h
├── member3_waiting_queue.h
├── member4_orders_billing.h
├── .gitignore
└── README.md
```

### Module Distribution

| Member | Module | File |
|---|---|---|
| Member 1 | Table Management | `member1_tables.h` |
| Member 2 | Reservation Management | `member2_reservations.h` |
| Member 3 | Waiting Queue Management | `member3_waiting_queue.h` |
| Member 4 | Orders and Billing | `member4_orders_billing.h` |
| Integration | Main Menu and File Handling | `main.c` |

## Technologies Used

- **Programming Language:** C
- **Compiler:** GCC
- **Development Environment:** Visual Studio Code
- **Version Control:** Git and GitHub

## Installation and Execution

### Prerequisites

- GCC compiler installed.
- Visual Studio Code or any C-compatible development environment.

### 1. Clone the Repository

```bash
git clone https://github.com/ADITYAKARNATAK/Restaurant-Management-System.git
```

### 2. Navigate to the Project Directory

```bash
cd Restaurant-Management-System
```

### 3. Compile the Project

```bash
gcc -std=c11 -Wall -Wextra -Werror -pedantic main.c -o restaurant
```

### 4. Run the Application

**Windows:**

```powershell
.\restaurant.exe
```

**Linux / macOS:**

```bash
./restaurant
```

## Data Persistence

The application uses a binary data file named `restaurant_data.dat` to preserve active reservations, waiting customers, table orders, pending kitchen tasks and cumulative sales.

The data file is generated automatically when the application saves its data. It is excluded from Git through `.gitignore`.

## Learning Objectives

- Understand the practical implementation of data structures in C.
- Apply searching and sorting algorithms.
- Understand memory allocation and linked-list operations.
- Implement queue and stack operations.
- Apply data structures to solve a real-world management problem.
- Develop modular programming and file-handling skills.
- Understand integration and collaboration in a team-based project.

## Future Scope

- Graphical user interface.
- Online reservation functionality.
- Database integration.
- Customer accounts and authentication.
- Advanced restaurant analytics and reporting.

## Academic Project

**Project:** Restaurant Table Reservation and Management System Using Data Structures in C

**Course:** Data Structures with C

**University:** Graphic Era Hill University

**Repository:** [GitHub Repository](https://github.com/ADITYAKARNATAK/Restaurant-Management-System)

---

*Developed as an academic project to demonstrate the practical application of data structures and algorithms in C.*
