# Campus Food Delivery Management System (C)

A role-based campus food delivery system implemented in C using file handling and modular programming.


## Overview

A console-based food delivery management system built in C.
The system manages interactions between students, restaurants, delivery personnel, and administrators within a campus environment.

## Features

* User roles: Student, Restaurant, Delivery, Admin
* Account registration and login
* Food ordering system
* Menu management for restaurants
* Order tracking and delivery updates
* Admin dashboard for system monitoring

## How to Run

Compile:
gcc main.c -o app

Run:
./app

## Concepts Used

* Structures (for data management)
* File handling (CSV storage)
* Pointers and arrays
* Conditional statements
* Modular programming (functions)

## Data Storage

* students.csv
* restaurants.csv
* delivery.csv
* menu.csv
* orders.csv

## Key Logic

* Order validation (balance + stock check)
* Order lifecycle: Pending → Prepared → Delivered
* Delivery earnings calculation (10%)

## Example Flow

1. Student logs in
2. Selects food item
3. System validates balance and stock
4. Order is created and stored
5. Delivery assigned and completed

## Limitations

* No encryption for passwords
* No real-time multi-user support
* Console-based (no GUI)

## Source

This project was originally developed as part of academic coursework and later refined for portfolio presentation.

## Example Output

1. Login as Student
2. View Menu
3. Place Order

Order placed successfully
Status: Pending

Restaurant prepares order
Status: Prepared

Delivery assigned
Status: Delivered

 
