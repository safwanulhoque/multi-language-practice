# Role-Based Service Management System (Python)

A simple role-based service management system built using Python and file handling.


## Overview

A console-based system that manages user access and services based on roles such as admin, technician, receptionist, and customer.

## Features

* User authentication system
* Role-based access control
* Service management system
* Customer handling and feedback system

## How to Run

python main.py

## Concepts Used

* Functions (modular programming)
* File handling (users.txt, services.txt, etc.)
* Conditional logic (if-else)
* Role-based access control

## System Roles

* Admin: Full control of system
* Technician: Handles services
* Receptionist: Manages customers and payments
* Customer: Books services and gives feedback

## Core Functions

* login()
* validate_credentials()
* identify_role()
* admin_menu()
* technician_menu()
* customer_menu()

## Data Storage

* users.txt
* services.txt
* payments.txt
* feedback.txt

## Example Flow

1. User logs in
2. System verifies credentials
3. Role is identified
4. User accesses role-specific menu

## Limitations

* Command-line interface only
* No database (file-based storage)
* Basic authentication system

## Source

This project was originally developed as part of academic coursework and later improved for portfolio purposes.


## Example Output

Enter username: admin
Enter password: ****

Login successful
Role: Admin

1. Register User
2. View Services
3. Logout

## Future Improvements

* Convert to web app using Flask
* Add database (SQLite/MySQL)
* Improve authentication security

