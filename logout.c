#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // 1. Invalidate the session cookie by setting its expiration date in the past
    // This tells the browser to delete the cookie immediately.
    printf("Set-Cookie: session_id=deleted; path=/; expires=Thu, 01 Jan 1970 00:00:00 GMT; HttpOnly\n");

    // 2. Output headers for redirection
    printf("Content-type: text/html\n");
    // Ensure correct relative path to signin.html
    printf("Location: /chat_app/signin.html\n\n"); 

    return 0;
}
