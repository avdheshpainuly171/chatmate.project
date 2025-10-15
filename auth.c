

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define USERS_FILE "users.txt"
#define SESSION_FILE "session.txt"

void trim_newline(char *s) { s[strcspn(s, "\n")] = 0; }

int username_exists(const char *username) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;
    char u[50], ph[20], p[50];
    while (fscanf(fp, "%49s %19s %49s", u, ph, p) == 3) {
        if (strcmp(u, username) == 0) { fclose(fp); return 1; }
    }
    fclose(fp);
    return 0;
}

void signup_user(const char *u, const char *ph, const char *pw) {
    if (username_exists(u)) {
        printf("<p>Username already taken!</p>");
        return;
    }
    FILE *fp = fopen(USERS_FILE, "a");
    if (fp) {
        fprintf(fp, "%s %s %s\n", u, ph, pw);
        fclose(fp);
        printf("<p>Signup successful! <a href='/chat_app/signin.html'>Sign in</a></p>");
    }
}

int signin_user(const char *login, const char *pw, char *logged_user) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;
    char u[50], ph[20], p[50];
    while (fscanf(fp, "%49s %19s %49s", u, ph, p) == 3) {
        if ((strcmp(login, u) == 0 || strcmp(login, ph) == 0) && strcmp(pw, p) == 0) {
            strcpy(logged_user, u);
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

int main(void) {
    printf("Content-Type: text/html\n\n");
    printf("<html><body>");

    char *query = getenv("QUERY_STRING");
    if (!query) { printf("<p>No query data!</p></body></html>"); return 0; }

    if (strstr(query, "action=signup")) {
        char username[50], phone[20], password[50];
        sscanf(query, "action=signup&username=%49[^&]&phone=%19[^&]&password=%49[^\n]", username, phone, password);
        for (int i = 0; username[i]; i++) if (username[i] == '+') username[i] = ' ';
        for (int i = 0; phone[i]; i++) if (phone[i] == '+') phone[i] = ' ';
        for (int i = 0; password[i]; i++) if (password[i] == '+') password[i] = ' ';
        signup_user(username, phone, password);
    } else if (strstr(query, "action=signin")) {
        char login[50], password[50], logged_in_user[50];
        sscanf(query, "action=signin&login=%49[^&]&password=%49[^\n]", login, password);
        for (int i = 0; login[i]; i++) if (login[i] == '+') login[i] = ' ';
        for (int i = 0; password[i]; i++) if (password[i] == '+') password[i] = ' ';
        if (signin_user(login, password, logged_in_user)) {
            FILE *sf = fopen(SESSION_FILE, "w");
            if (sf) { fprintf(sf, "%s\n", logged_in_user); fclose(sf); }
            printf("<p>Signin successful! Welcome, %s. <a href='/cgi-bin/friend.cgi'>Go to Chat</a></p>", logged_in_user);
        } else {
            printf("<p>Invalid credentials! <a href='/chat_app/signin.html'>Try again</a></p>");
        }
    } else {
        printf("<p>Invalid action!</p>");
    }

    printf("</body></html>");
    return 0;
}
