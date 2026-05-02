import os
from datetime import datetime

MAX_LOGIN_ATTEMPTS = 3
attempts = 0

def main():
    login()
print("""======================
|                    |
|      Welcome       |
|         to         |
|   Laptop Repair    |
|      Services      |
|                    |
======================""")

def login():
    global attempts
    while attempts < MAX_LOGIN_ATTEMPTS:
        username = input("Enter username: ")
        password = input("Enter password: ")
        if validate_credentials(username, password):
            print("Login Successful!")
            identify_role()
            return
        else:
            attempts += 1
            print("Invalid credentials. Attempts left:", MAX_LOGIN_ATTEMPTS - attempts)
    print("Maximum login attempts exceeded.")
    choice = input("Do you want to try again? (y/n): ").lower()
    if choice == 'y':
        attempts = 0
        login()
    else:
        print("Exiting the program.")
        exit()

def validate_credentials(username, password):
    if os.path.exists("users.txt"):
        with open("users.txt", "r") as file:
            for line in file:
                user, pwd, role = line.strip().split(", ")
                if user == username and pwd == password:
                    global current_role
                    current_role = role
                    return True
    return False

def identify_role():
    if current_role == "admin":
        admin_menu()
    elif current_role == "technician":
        technician_menu()
    elif current_role == "receptionist":
        receptionist_menu()
    elif current_role == "customer":
        customer_menu()
    else:
        print("Invalid role.")


def logout():
    print("Logging out...\n")
    main()

def admin_menu():
    while True:
        print("\nAdmin Menu:")
        print("1. Register Technician/Receptionist")
        print("2. Delete Technician/Receptionist Record")
        print("3. View Service Report and Total Income (Monthly)")
        print("4. Update Password")
        print("5. Logout")
        choice = input("Enter choice: ")
        menu_options = {
            "1": register_staff,
            "2": delete_staff_record,
            "3": view_service_report,
            "4": update_password,
            "5": logout
        }
        menu_options.get(choice, lambda: print("Invalid choice."))()

def technician_menu():
    while True:
        print("\nTechnician Menu:")
        print("1. View Service Requests")
        print("2. Complete Service and Update Details")
        print("3. Update Completion Description")
        print("4. Update Profile")
        print("5. Logout")
        choice = input("Enter choice: ")
        menu_options = {
            "1": view_service_requests,
            "2": complete_service,
            "3": update_completion_description,
            "4": lambda: update_profile("technician"),
            "5": logout
        }
        menu_options.get(choice, lambda: print("Invalid choice."))()

def receptionist_menu():
    while True:
        print("\nReceptionist Menu:")
        print("1. Register Customer")
        print("2. Assign Service")
        print("3. Accept Payment and Generate Receipt")
        print("4. View Feedback")
        print("5. Update Profile")
        print("6. Logout")
        choice = input("Enter choice: ")
        menu_options = {
            "1": register_customer,
            "2": assign_service,
            "3": accept_payment,
            "4": view_feedback,
            "5": lambda: update_profile("receptionist"),
            "6": logout
        }
        menu_options.get(choice, lambda: print("Invalid choice."))()

def customer_menu():
    while True:
        print("\nCustomer Menu:")
        print("1. Change Service")
        print("2. View Service Details and Payment Info")
        print("3. Send Feedback")
        print("4. Update Profile")
        print("5. Logout")
        choice = input("Enter choice: ")
        menu_options = {
            "1": change_service,
            "2": view_service_details,
            "3": send_feedback,
            "4": lambda: update_profile("customer"),
            "5": logout
        }
        menu_options.get(choice, lambda: print("Invalid choice."))()

def register_staff():
    name = input("Enter staff name: ")
    role = input("Enter role (Technician/Receptionist): ").lower()
    if role not in ["technician", "receptionist"]:
        print("Invalid role. Please enter Technician or Receptionist.")
        return
    username = input("Enter username for login: ")
    password = input("Enter password for login: ")

    with open("staff.txt", "a") as staff_file:
        staff_file.write(f"{name}, {role}\n")

    with open("users.txt", "a") as users_file:
        users_file.write(f"{username}, {password}, {role}\n")

    print("Staff registered successfully with login details.")

def delete_staff_record():
    name = input("Enter staff name to delete: ").strip()
    if not name:
        print("Name cannot be empty.")
        return

    staff_found = False
    with open("staff.txt", "r") as staff_file:
        staff_lines = staff_file.readlines()

    with open("staff.txt", "w") as staff_file:
        for line in staff_lines:
            if line.strip().startswith(name):
                staff_found = True
            else:
                staff_file.write(line)

    if not staff_found:
        print(f"No staff record found for {name}.")
        return

    confirmation = input(f"Are you sure you want to delete the record for {name}? (y/n): ").lower()
    if confirmation != "y":
        print("Operation cancelled.")
        return

    user_found = False
    with open("users.txt", "r") as users_file:
        user_lines = users_file.readlines()

    with open("users.txt", "w") as users_file:
        for line in user_lines:
            user_data = line.strip().split(", ")
            if user_data[0] == name:
                user_found = True
            else:
                users_file.write(line)

    if user_found:
        print(f"Staff record for {name} and their login credentials have been deleted.")
    else:
        print(f"User credentials for {name} not found in users.txt.")

def view_service_report():

    monthly_data = {}

    if os.path.exists("payments.txt"):
        with open("payments.txt", "r") as file:
            payments = file.readlines()

        for payment in payments:
            customer_name, amount, date = payment.strip().split(", ")
            amount = float(amount)

            month = date[:7]

            if month not in monthly_data:
                monthly_data[month] = {"total_income": 0, "transaction_count": 0}

            monthly_data[month]["total_income"] += amount
            monthly_data[month]["transaction_count"] += 1

        print("Monthly Income and Transaction Report:")
        for month, data in sorted(monthly_data.items()):
            print(f"{month}: Total Income = RM {data['total_income']}, Transactions = {data['transaction_count']}")
    else:
        print("No payment records found.")

def update_password():
    username = input("Enter your username: ")
    new_password = input("Enter your new password: ")

    if os.path.exists("users.txt"):
        updated = False
        with open("users.txt", "r") as file:
            users = file.readlines()

        with open("users.txt", "w") as file:
            for user in users:
                user_data = user.strip().split(", ")
                if user_data[0] == username and user_data[2] == current_role:

                    file.write(f"{username}, {new_password}, {current_role}\n")
                    updated = True
                else:
                    file.write(user)

        if updated:
            print("Password updated successfully.")
        else:
            print("User not found or role mismatch.")
    else:
        print("No user records found.")

def register_customer():
    name = input("Enter customer name: ")
    contact = input("Enter customer contact: ")

    create_account = input("Do you want to create a login account? (y/n): ").lower()

    with open("customers.txt", "a") as customer_file:
        customer_file.write(f"{name}, {contact}\n")

    if create_account == 'y':
        username = input("Enter username for login: ")
        password = input("Enter password for login: ")

        with open("users.txt", "a") as users_file:
            users_file.write(f"{username}, {password}, customer\n")

        print("Customer registered successfully with login details.")
    else:
        print("Customer registered successfully without login details.")

def assign_service():
    customer_name = input("Enter customer name: ")
    service = input("Enter service to assign: ")
    with open("services.txt", "a") as file:
        file.write(f"{customer_name}, {service}\n")
    print("Service assigned successfully.")


def accept_payment():
    customer_name = input("Enter customer name: ")
    amount = float(input("Enter payment amount: "))

    current_date = datetime.now().strftime("%Y-%m-%d")

    with open("payments.txt", "a") as file:
        file.write(f"{customer_name}, {amount}, {current_date}\n")

    print(f"Payment accepted. Receipt generated for RM {amount} on {current_date}.")

def view_feedback():
    if os.path.exists("feedback.txt"):
        with open("feedback.txt", "r") as file:
            feedbacks = file.readlines()
            if feedbacks:
                for feedback in feedbacks:
                    parts = feedback.strip().split(", ", 2)
                    if len(parts) == 3:
                        name, service, comment = parts
                        print(f"Feedback for {service} by {name}: {comment}")
                    else:
                        print(f"Invalid feedback format: {feedback.strip()}")
            else:
                print("No feedback available.")
    else:
        print("No feedback file found.")

def update_profile(role):
    current_username = input("Enter your current username: ")
    new_username = input("Enter your new username: ")
    new_password = input("Enter your new password: ")

    updated = False
    if os.path.exists("users.txt"):
        with open("users.txt", "r") as file:
            lines = file.readlines()

        with open("users.txt", "w") as file:
            for line in lines:
                user, pwd, user_role = line.strip().split(", ")
                if user == current_username and user_role == role:
                    file.write(f"{new_username}, {new_password}, {role}\n")
                    updated = True
                else:
                    file.write(line)

        if updated:
            print("Profile updated successfully.")
        else:
            print("Profile not found or role mismatch.")
    else:
        print("No user records found.")

def view_service_requests():
    if os.path.exists("services.txt"):
        with open("services.txt", "r") as file:
            for service in file:
                print(service.strip())
    else:
        print("No service requests available.")

def complete_service():
    customer_name = input("Enter customer name: ")
    service_details = input("Enter service completion details: ")
    with open("completed_services.txt", "a") as file:
        file.write(f"{customer_name}, {service_details}\n")
    print("Service completed and details updated.")

def update_completion_description():
    customer_name = input("Enter customer name: ")
    new_description = input("Enter new completion description: ")
    updated = False

    if os.path.exists("completed_services.txt"):
        with open("completed_services.txt", "r") as file:
            lines = file.readlines()
        with open("completed_services.txt", "w") as file:
            for line in lines:
                if customer_name in line:
                    file.write(f"{customer_name}, {new_description}\n")
                    updated = True
                else:
                    file.write(line)
        if updated:
            print("Completion description updated.")
        else:
            print("Customer not found.")
    else:
        print("No completed services found.")

def change_service():
    customer_name = input("Enter your name: ")
    new_service = input("Enter new service request: ")

    with open("services.txt", "r") as file:
        lines = file.readlines()

    customer_services = [line.strip() for line in lines if line.startswith(customer_name)]

    if not customer_services:
        print(f"No existing service requests found for {customer_name}.")
        return

    print(f"\nCurrent services for {customer_name}:")
    for index, service in enumerate(customer_services, 1):
        print(f"{index}. {service.split(', ')[1]}")  # Display the service name only

    try:
        service_choice = int(input("\nEnter the number of the service you want to change: "))
        if service_choice < 1 or service_choice > len(customer_services):
            print("Invalid choice. No service was changed.")
            return
    except ValueError:
        print("Invalid input. Please enter a valid number.")
        return

    old_service = customer_services[service_choice - 1]
    print(f"Service selected to change: {old_service.split(', ')[1]}")

    updated = False
    with open("services.txt", "w") as file:
        for line in lines:
            if line.strip() == old_service:
                file.write(f"{customer_name}, {new_service}\n")
                updated = True
            else:
                file.write(line)

    if updated:
        print(f"Service request for {customer_name} has been updated to: {new_service}.")
    else:
        print(f"Failed to update the service request for {customer_name}.")


def view_service_details():
    customer_name = input("Enter your name: ")
    service_found = False

    with open("services.txt", "r") as service_file:
        customer_services = [line.strip() for line in service_file if line.startswith(customer_name)]

    with open("payments.txt", "r") as payment_file:
        customer_payments = [line.strip() for line in payment_file if line.startswith(customer_name)]

    if not customer_services:
        print(f"No services found for {customer_name}.")
        return

    print(f"\nServices and payment details for {customer_name}:")

    payment_index = 0

    for service in customer_services:
        service_name = service.split(", ")[1]
        print(f"Service: {service_name}")

        if payment_index < len(customer_payments):
            payment_details = customer_payments[payment_index].split(", ")
            payment_name = payment_details[0]
            payment_amount = payment_details[1]

            if payment_name == customer_name:
                print(f"Payment: RM {payment_amount}")
                payment_index += 1
        else:
            print("Payment: Not yet paid")

    print()


def send_feedback():
    name = input("Enter your name: ")

    services = []
    with open("services.txt", "r") as file:
        for line in file:
            customer_name, service = line.strip().split(", ")
            if customer_name == name:
                services.append(service)

    if not services:
        print(f"No services found for {name}.")
        return

    print(f"\n{len(services)} service(s) found for {name}:")
    for idx, service in enumerate(services, 1):
        print(f"{idx}. {service}")

    service_choice = int(input("\nSelect the service you want to give feedback about (by number): "))

    if service_choice < 1 or service_choice > len(services):
        print("Invalid choice. Feedback not submitted.")
        return

    feedback = input("Enter your feedback: ")

    with open("feedback.txt", "a") as file:
        file.write(f"{name}, {services[service_choice - 1]}, {feedback}\n")

    print("Feedback sent successfully.")

def initialize_users():
    if not os.path.exists("users.txt"):
        users = [
            "Hassan, pass, admin",
            "Ahmed, pass, technician",
            "Rayyan, pass, receptionist",
            "Safwan, pass, customer"
        ]
        with open("users.txt", "w") as file:
            for user in users:
                file.write(user + "\n")
        print("Initial users created successfully.")

initialize_users()

main()
