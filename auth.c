#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char phone[15];
    char password[50];
} User;

// Check if phone already exists
int userExists(char *phone) {
    FILE *fp = fopen("users.txt", "r");
    if (!fp) return 0; // file doesn't exist yet
    User u;
    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.phone, phone) == 0) {
            fclose(fp);
            return 1; // found existing user
        }
    }
    fclose(fp);
    return 0;
}

// Signup function
void signup() {
    User u;
    printf("\nEnter phone number: ");
    scanf("%s", u.phone);

    if (userExists(u.phone)) {
        printf("Account already exists with this phone number.\n");
        return;
    }

    printf("Enter password: ");
    scanf("%s", u.password);

    FILE *fp = fopen("users.txt", "ab");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    fwrite(&u, sizeof(User), 1, fp);
    fclose(fp);

    printf("Signup successful! You can now sign in.\n");
}

// Signin function
int signin() {
    char phone[15], pass[50];
    User u;

    printf("\nEnter phone number: ");
    scanf("%s", phone);
    printf("Enter password: ");
    scanf("%s", pass);

    FILE *fp = fopen("users.txt", "r");
    if (!fp) {
        printf("No users found. Please sign up first.\n");
        return 0;
    }

    while (fread(&u, sizeof(User), 1, fp)) {
        if (strcmp(u.phone, phone) == 0 && strcmp(u.password, pass) == 0) {
            fclose(fp);
            printf("Login successful! Welcome user %s.\n", phone);
            return 1;
        }
    }

    fclose(fp);
    printf("Invalid phone number or password.\n");
    return 0;
}

// Main menu
int main() {
    int choice;
    while (1) {
        printf("\n===== USER AUTHENTICATION =====\n");
        printf("1. Sign Up\n");
        printf("2. Sign In\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                signup();
                break;
            case 2:
                signin();
                break;
            case 3:
                printf("Exiting program...\n");
                exit(0);
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    return 0;
}
