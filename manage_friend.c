#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// --- Configuration (Standardizing Paths) ---
#define SESSIONS_FILE "C:\\xampp\\cgi-bin\\sessions.txt"
#define USERS_FILE "C:\\xampp\\cgi-bin\\users.txt"
#define FRIENDS_FILE "C:\\xampp\\cgi-bin\\friends.txt"
#define MAX_LINE 256
#define SESSION_KEY "session_id="

// Helper function: Simple URL decode
void decode(char *str) {
    char *p = str;
    int hex;
    while (*str) {
        if (*str == '%') {
            sscanf(str + 1, "%2x", &hex);
            *p++ = hex;
            str += 3;
        } else if (*str == '+') {
            *p++ = ' ';
            str++;
        } else {
            *p++ = *str++;
        }
    }
    *p = '\0';
}

// Helper function: Get parameter from query string
char* get_query_param(const char *query, const char *key) {
    if (!query) return NULL;
    char *temp_query = strdup(query);
    char *result = NULL;
    char *token = strtok(temp_query, "&");
    
    while (token != NULL) {
        char *eq = strchr(token, '=');
        if (eq) {
            *eq = '\0'; // Split token into key and value
            if (strcmp(token, key) == 0) {
                result = strdup(eq + 1);
                decode(result); // Decode the value
                break;
            }
        }
        token = strtok(NULL, "&");
    }
    free(temp_query);
    return result;
}

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

// Function to check if a user exists
int user_exists(const char *username) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;
    char u[50], ph[20], p[50];
    int found = 0;
    while (fscanf(fp, "%49s %19s %49s", u, ph, p) == 3) {
        if (strcmp(u, username) == 0) { 
            found = 1; 
            break; 
        }
    }
    fclose(fp);
    return found;
}

// Function to check if a friendship already exists
int friendship_exists(const char *user1, const char *user2) {
    FILE *fp = fopen(FRIENDS_FILE, "r");
    if (!fp) return 0;
    char line[MAX_LINE];
    int found = 0;

    char u1_sorted[50], u2_sorted[50];
    if (strcmp(user1, user2) < 0) {
        strcpy(u1_sorted, user1);
        strcpy(u2_sorted, user2);
    } else {
        strcpy(u1_sorted, user2);
        strcpy(u2_sorted, user1);
    }

    while (fgets(line, MAX_LINE, fp)) {
        char temp_line[MAX_LINE];
        strcpy(temp_line, line);
        char *f1 = strtok(temp_line, "|");
        char *f2 = strtok(NULL, "|");
        
        if (f1 && f2) {
            f1[strcspn(f1, "\n")] = 0;
            f2[strcspn(f2, "\n")] = 0;
            
            if (strcmp(f1, u1_sorted) == 0 && strcmp(f2, u2_sorted) == 0) {
                found = 1;
                break;
            }
        }
    }
    fclose(fp);
    return found;
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
    
    // Handle form submission (Add Friend)
    char *query = getenv("QUERY_STRING");
    char *friend_to_add = NULL;
    char *message = NULL;

    if (query) {
        friend_to_add = get_query_param(query, "friend_name");
        
        if (friend_to_add) {
            if (strcmp(friend_to_add, current_user) == 0) {
                message = strdup("Error: Cannot add yourself as a friend.");
            } else if (!user_exists(friend_to_add)) {
                message = strdup("Error: User does not exist.");
            } else if (friendship_exists(current_user, friend_to_add)) {
                message = strdup("Info: You are already friends with this user.");
            } else {
                // Add the friendship (store alphabetically)
                FILE *fp_write = fopen(FRIENDS_FILE, "a");
                if (fp_write) {
                    char u1_sorted[50], u2_sorted[50];
                    if (strcmp(current_user, friend_to_add) < 0) {
                        strcpy(u1_sorted, current_user);
                        strcpy(u2_sorted, friend_to_add);
                    } else {
                        strcpy(u1_sorted, friend_to_add);
                        strcpy(u2_sorted, current_user);
                    }
                    fprintf(fp_write, "%s|%s\n", u1_sorted, u2_sorted);
                    fclose(fp_write);
                    message = strdup("Success: Friend added!");
                } else {
                    message = strdup("Error: Server error saving friend list.");
                }
            }
        }
    }
    
    printf("Content-type: text/html\n\n"); // Now output the HTML page

    // --- HTML Output ---
    printf("<!DOCTYPE html>\n");
    printf("<html lang=\"en\">\n");
    printf("<head>\n");
    printf("    <meta charset=\"UTF-8\">\n");
    printf("    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    printf("    <title>Manage Friends | ChatMate</title>\n");
    printf("    <script src=\"https://cdn.tailwindcss.com\"></script>\n");
    printf("    <style>\n");
    printf("        body { font-family: 'Inter', sans-serif; }\n");
    printf("        .bg-bg-main { background-color: #0d1117; }\n");
    printf("        .bg-card { background-color: #161b22; }\n");
    printf("        .border-dark { border-color: #30363d; }\n");
    printf("        .text-light { color: #e6edf3; }\n");
    printf("        .bg-primary { background-color: #238636; }\n");
    printf("        .text-error { color: #f85149; }\n");
    printf("        .text-success { color: #2ea043; }\n");
    printf("    </style>\n");
    printf("</head>\n");
    printf("<body class=\"bg-bg-main text-text-light min-h-screen flex justify-center items-center p-4\">\n");
    printf("    <div class=\"w-full max-w-lg bg-card p-8 rounded-xl shadow-2xl border border-border-dark\">\n");
    printf("        <h1 class=\"text-3xl font-bold text-center text-primary mb-6\">Manage Friends</h1>\n");
    
    // Display message if available
    if (message) {
        int is_error = strstr(message, "Error") != NULL;
        printf("        <div class=\"p-3 mb-4 rounded-lg %s %s\">\n", 
            is_error ? "bg-red-900/50" : "bg-green-900/50",
            is_error ? "text-error" : "text-success");
        printf("            %s\n", message);
        printf("        </div>\n");
    }

    // Add Friend Form
    printf("        <h2 class=\"text-xl font-semibold mb-3 border-b border-border-dark pb-2\">Add New Friend</h2>\n");
    printf("        <form method=\"get\" action=\"/cgi-bin/manage_friend.cgi\" class=\"space-y-4 mb-8\">\n");
    printf("            <div>\n");
    printf("                <label for=\"friend_name\" class=\"block text-sm font-medium text-gray-400 mb-1\">Friend's Username</label>\n");
    printf("                <input type=\"text\" id=\"friend_name\" name=\"friend_name\" required \n");
    printf("                       class=\"w-full p-3 border border-border-dark rounded-lg bg-[#21262d] text-text-light focus:border-primary focus:ring-primary focus:outline-none transition-colors\" \n");
    printf("                       placeholder=\"Enter username\">\n");
    printf("            </div>\n");
    printf("            <button type=\"submit\" class=\"w-full py-3 bg-primary text-white font-semibold rounded-lg shadow-md hover:bg-[#2ea043] transition-colors duration-200\">\n");
    printf("                Add Friend\n");
    printf("            </button>\n");
    printf("        </form>\n");

    // Current Friend List
    printf("        <h2 class=\"text-xl font-semibold mb-3 border-b border-border-dark pb-2\">Your Current Friends</h2>\n");
    printf("        <div class=\"space-y-2 friend-list\">\n");
    
    // Read and display current friends
    FILE *fp_read = fopen(FRIENDS_FILE, "r");
    char line[MAX_LINE];
    int friend_found = 0;

    if (fp_read) {
        while (fgets(line, MAX_LINE, fp_read)) {
            char temp_line[MAX_LINE];
            strcpy(temp_line, line);
            
            char *u1 = strtok(temp_line, "|");
            char *u2 = strtok(NULL, "|");
            
            if (u1 && u2) {
                u1[strcspn(u1, "\n")] = 0;
                u2[strcspn(u2, "\n")] = 0;
                
                char *friend_name = NULL;
                if (strcmp(u1, current_user) == 0) {
                    friend_name = u2;
                } else if (strcmp(u2, current_user) == 0) {
                    friend_name = u1;
                }

                if (friend_name) {
                    printf("            <div class=\"friend-item p-3 bg-[#1f242b] rounded-lg border border-border-dark\">%s</div>\n", friend_name);
                    friend_found = 1;
                }
            }
        }
        fclose(fp_read);
    }

    if (!friend_found) {
        printf("            <p class=\"text-gray-500 text-center py-4\">You have no friends yet. Add one above!</p>\n");
    }

    printf("        </div>\n");

    // Back to Chat Link
    printf("        <div class=\"mt-8\">\n");
    printf("            <a href=\"/chat_app/chat.html\" class=\"w-full block text-center py-3 bg-gray-600/50 text-gray-300 font-semibold rounded-lg hover:bg-gray-600/70 transition-colors\">\n");
    printf("                ← Back to Chat\n");
    printf("            </a>\n");
    printf("        </div>\n");
    
    printf("    </div>\n");
    printf("</body>\n");
    printf("</html>\n");

    // Clean up allocated memory
    if (session_id) free(session_id);
    if (friend_to_add) free(friend_to_add);
    if (message) free(message);

    return 0;
}

