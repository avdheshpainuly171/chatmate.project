#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Configuration must match auth.c
#define SESSIONS_FILE "C:\\xampp\\cgi-bin\\sessions.txt"
#define MAX_LINE 256
#define SESSION_KEY "session_id="

// Helper function: Extracts session ID from the cookie
char* get_session_id() {
    char *cookie_string = getenv("HTTP_COOKIE");
    if (!cookie_string) return NULL;

    char *token_start = strstr(cookie_string, SESSION_KEY);
    if (!token_start) return NULL;

    token_start += strlen(SESSION_KEY);
    
    char *token_end = strchr(token_start, ';');
    int length;
    if (token_end) {
        length = token_end - token_start;
    } else {
        length = strlen(token_start);
    }
    
    if (length == 0 || length > 32) return NULL;

    char *session_id = (char*)malloc(length + 1);
    strncpy(session_id, token_start, length);
    session_id[length] = '\0';
    
    return session_id;
}

// Function to check if a session ID is valid
int is_session_valid(const char *session_id) {
    if (!session_id || strlen(session_id) == 0) return 0;

    FILE *fp = fopen(SESSIONS_FILE, "r");
    char line[MAX_LINE];
    int valid = 0;

    if (fp) {
        while (fgets(line, MAX_LINE, fp)) {
            char temp_line[MAX_LINE];
            strcpy(temp_line, line);
            
            char *sid = strtok(temp_line, "|");
            if (sid && strcmp(sid, session_id) == 0) {
                valid = 1;
                break;
            }
        }
        fclose(fp);
    }
    return valid;
}

int main() {
    char *session_id = get_session_id();
    int logged_in = is_session_valid(session_id);

    // CRITICAL: If logged in, redirect to the main chat page
    if (logged_in) {
        printf("Content-type: text/html\n");
        printf("Location: /chat_app/chat.html\n\n"); // Redirect to the main chat page
    } else {
        // If not logged in, display the index page (which leads to sign in)
        // Note: The original index.c outputs HTML for a landing page.
        // If the user lands here logged out, the existing HTML page will show.
        // We will output the HTML content of the landing page here.
        printf("Content-type: text/html\n\n");
        
        // This is the HTML content of the original index.html file, 
        // embedded here to ensure it's displayed when the user is not logged in.
        printf("<!DOCTYPE html>\n");
        printf("<html lang=\"en\">\n");
        printf("<head>\n");
        printf("  <meta charset=\"UTF-8\">\n");
        printf("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
        printf("  <title>ChatMate | C Chat App</title>\n");
        printf("  <style>\n");
        printf("    body, html { margin: 0; padding: 0; height: 100%%; font-family: 'Inter', sans-serif; }\n");
        printf("    .container { display: flex; height: 100vh; }\n");
        printf("    .left { flex: 1; background-color: #0d1117; color: #e6edf3; display: flex; flex-direction: column; justify-content: center; align-items: center; text-align: center; padding: 40px; }\n");
        printf("    .right { flex: 1; background-color: #238636; color: #ffffff; display: flex; flex-direction: column; justify-content: center; align-items: center; text-align: center; padding: 40px; }\n");
        printf("    h1 { font-size: 3rem; font-weight: 800; margin-bottom: 10px; letter-spacing: 1px; text-shadow: 2px 2px 5px rgba(0,0,0,0.2); }\n");
        printf("    p { font-size: 1.125rem; margin-bottom: 40px; max-width: 500px; }\n");
        printf("    a.button { display: inline-block; background: #161b22; color: #e6edf3; text-decoration: none; padding: 12px 30px; border-radius: 8px; font-weight: bold; font-size: 1rem; margin: 10px; transition: 0.3s; box-shadow: 0 4px 6px rgba(0,0,0,0.2); }\n");
        printf("    a.button:hover { background: #30363d; }\n");
        printf("    @media (max-width: 768px) {\n");
        printf("        .container { flex-direction: column; }\n");
        printf("        .left, .right { min-height: 50vh; }\n");
        printf("        h1 { font-size: 2.5rem; }\n");
        printf("    }\n");
        printf("  </style>\n");
        printf("</head>\n");
        printf("<body>\n");
        printf("  <div class=\"container\">\n");
        printf("    <div class=\"left\">\n");
        printf("        <h1>ChatMate</h1>\n");
        printf("        <p>Your secure, simple, and fast C-based chat application. Connect with friends instantly!</p>\n");
        printf("    </div>\n");
        printf("    <div class=\"right\">\n");
        printf("      <h1>Ready to Chat?</h1>\n");
        printf("      <p>Sign in or create an account to start messaging.</p>\n");
        printf("      <a href=\"/chat_app/signin.html\" class=\"button\">Sign In</a>\n");
        printf("      <a href=\"/chat_app/signup.html\" class=\"button\">Sign Up</a>\n");
        printf("    </div>\n");
        printf("  </div>\n");
        printf("</body>\n");
        printf("</html>\n");
    }
    
    // Free the session_id memory if it was allocated
    if (session_id) {
        free(session_id);
    }

    return 0;
}

