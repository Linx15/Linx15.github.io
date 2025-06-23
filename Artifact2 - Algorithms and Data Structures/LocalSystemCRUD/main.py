# Program that will create,read,update, and delete txt files
# Created by Alejandro Sarmiento

import sys
import os


# Function for creating a txt file with a specified name
def create():
    while True:
        filename = input("\nEnter the name of the file you want to create or say "
                         "back to return to the selection menu! ")

        if filename == "back":
            break

        elif os.path.exists(filename + ".txt"):
            print("That file already exists!")

        else:
            open(filename + ".txt", "x")

            if os.path.exists(filename + ".txt"):
                print("The file has been created.")
                continue

            else:
                print("It seems a file with that name can't be created. Please try are different name.")


# Function for reading a specified txt file
def read():
    while True:
        filename = input("\nEnter the name of the file you would like to read or say "
                         "back to return to the main selection menu: ")

        if filename == "back":
            break

        else:
            f = open(filename + ".txt")

            print("\nyou are reading" + filename + ".txt")

            print("\n" + f.read())
            continue


# Function to add or change text in a specified txt file
def update():
    while True:
        choice = input("\nWould you like to add or replace the information in the txt file? "
                       "(Say back to return to the main selection menu) ")

        if "back" in choice:
            break

        elif "add" in choice:
            filename = input("Enter the name of the file: ")
            text = input("type the text you would like to add! ")

            with open(filename + ".txt", "a") as f:
                f.write("\n" + text)

                print("\nYour text has been added!")

                continue

        elif "replace" in choice:
            filename = input("Enter the name of the file: ")
            text = input("type the text you would like to replace the original with!")

            with open(filename + ".txt", "w") as f:
                f.write(text)

                print("The text has been replaced!")

                continue

        else:
            print("Your input appears to be invalid, please try again!")


# Function to delete a specified txt file
def delete():
    while True:
        filename = input("\nEnter the name of the file you would like to delete or say back to "
                         "return to the main selection menu: ")

        if filename == "back":
            break

        else:
            if os.path.exists(filename + ".txt"):
                os.remove(filename + ".txt")

                print("\n" + filename + ".txt has been deleted!")
                continue

            else:
                print("\nThat file does not exist!")
                continue


# main function used to call the others functions
if __name__ == '__main__':

    while True:
        action = input(
            "\nWelcome to my txt CRUD program! \nYou can create, read, update, or delete txt files and their "
            "information or say exit to end the program."
            "\nPlease make your selection! ")

        # Calls the create function if the user selects it
        if "create" in action:
            create()

        # Calls the read function if the user selects it
        elif "read" in action:
            read()

        # Calls the update function if the user selects it
        elif "update" in action:
            update()

        # Calls the delete function if the user selects it
        elif "delete" in action:
            delete()

        # exits the program if the user chooses to and prints a thank-you message
        elif "exit" in action:
            print("\nThank you for using this program!")

            sys.exit(0)

        # Prints a message to let the user know that their input did not seem to work and to try again
        else:
            print("\nThat does not appear to be a valid selection, please try again!")
