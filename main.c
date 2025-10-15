#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void trim_newline(char *s) {
    s[strcspn(s, "\n")] = 0;
}

int main(void) {
    printf("Content-Type: text/html\n\n");
    printf("<html><head><title>C Chat App - Home</title></head><body>");
    printf("<h2>Welcome to the C Chat Application</h2>");

    FILE *sf = fopen("session.txt", "r");
    char user[50];
    int logged_in = 0;

    if (sf && fgets(user, sizeof(user), sf)) {
        trim_newline(user);
        logged_in = 1;
        fclose(sf);
    }

    if (logged_in) {
        printf("<p>Hello, <b>%s</b>! You are logged in.</p>", user);
        printf("<p><a href='/cgi-bin/friend.cgi'>Go to Friends & Chat</a></p>");
        printf("<p><a href='/chat_app/logout.cgi'>Logout</a></p>");
    } else {
        printf("<p><a href='/chat_app/signup.html'>Signup</a></p>");
        printf("<p><a href='/chat_app/signin.html'>Signin</a></p>");
    }

    printf("<hr><small>Developed using C + CGI</small>");
    printf("</body></html>");
    return 0;
}


