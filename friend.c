#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Configuration (Standardizing Paths) ---
#define SESSIONS_FILE "C:\\xampp\\cgi-bin\\sessions.txt"
#define FRIENDS_FILE "C:\\xampp\\cgi-bin\\friends.txt"
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
    int length = token_end ? token_end - token_start : strlen(token_start);
    if (length == 0 || length > 32) return NULL;
    char *session_id = (char*)malloc(length + 1);
    strncpy(session_id, token_start, length);
    session_id[length] = '\0';
    return session_id;
}

// Function to get username from a valid session ID
int get_username_from_session(const char *session_id, char *username) {
    if (!session_id) return 0;
    FILE *fp = fopen(SESSIONS_FILE, "r");
    char line[MAX_LINE];

    if (fp) {
        while (fgets(line, MAX_LINE, fp)) {
            char temp_line[MAX_LINE];
            strcpy(temp_line, line);
            
            char *sid = strtok(temp_line, "|");
            char *user = strtok(NULL, "|");

            if (sid && user && strcmp(sid, session_id) == 0) {
                // Found a valid session, copy the username and clean it up
                user[strcspn(user, "\n")] = 0; 
                strcpy(username, user);
                fclose(fp);
                return 1;
            }
        }
        fclose(fp);
    }
    return 0; // Session not found or file error
}

// --- Main Logic ---
int main() {
    char current_user[50] = {0};
    char *session_id = get_session_id();
    
    // Check if the user is logged in
    if (!session_id || !get_username_from_session(session_id, current_user)) {
        // Not logged in, redirect to signin
        printf("Content-type: text/html\n");
        printf("Location: /chat_app/signin.html\n\n");
        if (session_id) free(session_id);
        return 0;
    }
    
    if (session_id) free(session_id); // Free allocated session_id

    // CRITICAL: Output Content-type: text/html for the HTML fragments
    printf("Content-type: text/html\n\n");
    
    // 3. Read the friends file
    FILE *fp = fopen(FRIENDS_FILE, "r");
    char line[MAX_LINE];
    int friend_count = 0;

    if (fp) {
        while (fgets(line, MAX_LINE, fp)) {
            char temp_line[MAX_LINE];
            strcpy(temp_line, line);
            
            // Format: user1|user2 (must be sorted alphabetically in the file)
            char *u1 = strtok(temp_line, "|");
            char *u2 = strtok(NULL, "|");
            
            if (u1 && u2) {
                u1[strcspn(u1, "\n")] = 0; // Clean up
                u2[strcspn(u2, "\n")] = 0;
                
                char *friend_name = NULL;
                // Determine which one is the friend
                if (strcmp(u1, current_user) == 0) {
                    friend_name = u2;
                } else if (strcmp(u2, current_user) == 0) {
                    friend_name = u1;
                }
                
                if (friend_name) {
                    // CRITICAL: Output the HTML fragment with data-name attribute
                    printf("<div class=\"friend\" data-name=\"%s\">%s</div>\n", friend_name, friend_name);
                    friend_count++;
                }
            }
        }
        fclose(fp);
    }
    
    if (friend_count == 0) {
        printf("<div class=\"p-4 text-center text-gray-500\">No friends yet. Add one!</div>\n");
    }

    return 0;
}

