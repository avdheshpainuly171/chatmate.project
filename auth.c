#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- Configuration ---
// Note: USERS_FILE format is space-separated: username phone password
#define USERS_FILE "C:\\xampp\\cgi-bin\\users.txt"
#define SESSIONS_FILE "C:\\xampp\\cgi-bin\\sessions.txt"
#define MAX_LINE 256
#define SESSION_KEY "session_id="

// --- Helper Functions for URL Parsing (CRITICAL for CGI Robustness) ---

// Helper function: Trims newline character (as provided by user)
void trim_newline(char *s) { s[strcspn(s, "\n")] = 0; }

// Helper function: Simple URL decode (Handles '+' and '%XX')
void decode(char *str) {
    char *p = str;
    int hex;
    while (*str) {
        if (*str == '%') {
            // Safely read the next two characters as hex
            if (sscanf(str + 1, "%2x", &hex) == 1) {
                *p++ = hex;
                str += 3;
            } else {
                // If decoding fails, treat it as a literal '%'
                *p++ = *str++;
            }
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
    
    // We create a search string "key="
    char search_key[MAX_LINE];
    snprintf(search_key, sizeof(search_key), "%s=", key);

    char *token_start = strstr(query, search_key);
    if (!token_start) return NULL;
    token_start += strlen(search_key);

    // Find the end of the value (either '&' or end of string)
    char *token_end = strchr(token_start, '&');
    int length;
    if (token_end) {
        length = token_end - token_start;
    } else {
        length = strlen(token_start);
    }
    
    if (length == 0 || length >= MAX_LINE) return NULL;

    char *value = (char*)malloc(length + 1);
    if (!value) return NULL; // Check for malloc failure
    strncpy(value, token_start, length);
    value[length] = '\0';
    
    // Decode the value in place before returning
    decode(value);
    trim_newline(value); // Ensure no trailing newlines/carriage returns
    
    // If the resulting value is just an empty string after decoding/trimming, treat as NULL
    if (strlen(value) == 0) {
        free(value);
        return NULL;
    }
    
    return value;
}


// --- User Authentication Logic ---

// Checks if username exists in USERS_FILE (space delimiter assumed)
int username_exists(const char *username) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;
    char u[50], ph[20], p[50];
    while (fscanf(fp, "%49s %19s %49s", u, ph, p) == 3) {
        if (strcmp(u, username) == 0) { 
            fclose(fp); 
            return 1; 
        }
    }
    fclose(fp);
    return 0;
}

// Attempts to sign up the user. Handles redirection.
void signup_user(const char *u, const char *ph, const char *pw) {
    if (username_exists(u)) {
        // Redirect back to signup with error=1 (Username exists)
        printf("Location: /chat_app/signup.html?error=1\n\n");
        return;
    }
    
    FILE *fp = fopen(USERS_FILE, "a");
    if (fp) {
        // Writes space separated: username phone password. 
        fprintf(fp, "%s %s %s\n", u, ph, pw);
        fclose(fp);
        // Successful signup, redirect to signin page with success=1
        printf("Location: /chat_app/signin.html?success=1\n\n");
    } else {
        // File write error
        printf("Location: /chat_app/signup.html?error=2\n\n");
    }
}

// Function to generate a simple unique ID (using time for simplicity)
void generate_session_id(char *session_id) {
    // Generate a simple session ID (e.g., based on current time)
    srand(time(NULL));
    long long timestamp = (long long)time(NULL) * 1000 + (rand() % 1000);
    snprintf(session_id, 33, "%lld%d", timestamp, rand() % 1000);
}

// Attempts to sign in the user. Handles login via username or phone.
int signin_user(const char *login, const char *password, char *logged_in_user) {
    FILE *fp = fopen(USERS_FILE, "r");
    if (!fp) return 0;

    char u[50], ph[20], p[50];
    int signed_in = 0;
    
    // Check if login matches username or phone (space delimiter assumed)
    while (fscanf(fp, "%49s %19s %49s", u, ph, p) == 3) {
        if ((strcmp(u, login) == 0 || strcmp(ph, login) == 0) && strcmp(p, password) == 0) {
            strcpy(logged_in_user, u); // Copy the username
            signed_in = 1;
            break;
        }
    }
    fclose(fp);
    return signed_in;
}

// --- Main Logic ---

int main() {
    // CRITICAL: Output Content-type before any Location header
    printf("Content-type: text/html\n"); 

    char *query = getenv("QUERY_STRING");
    if (!query) {
        printf("Location: /chat_app/signin.html?error=3\n\n"); // Unknown error / No query
        return 0;
    }

    char *action = get_query_param(query, "action");
    int result = 0; // 0=OK, 1=Redirected, 2=Error

    if (action && strcmp(action, "signup") == 0) {
        char *username = get_query_param(query, "username");
        char *password = get_query_param(query, "password");
        char *phone = get_query_param(query, "phone"); 
        
        // --- Sign Up Action ---
        if (username && password) {
            // Use a non-empty placeholder for phone to maintain the 3-token format
            const char *phone_to_write = "N/A"; 
            if (phone && strlen(phone) > 0) {
                phone_to_write = phone;
            }
            
            signup_user(username, phone_to_write, password);
            result = 1; // Handled redirection inside signup_user
            
        } else {
            // Missing required fields
            printf("Location: /chat_app/signup.html?error=3\n\n"); 
            result = 1;
        }

        // Cleanup
        if (username) free(username);
        if (password) free(password);
        if (phone) free(phone);
        
    } else if (action && strcmp(action, "signin") == 0) {
        char *login = get_query_param(query, "login");
        char *password = get_query_param(query, "password");
        char logged_in_user[50] = {0};

        // --- Sign In Action ---
        if (login && password && signin_user(login, password, logged_in_user)) {
            // SUCCESSFUL SIGNIN
            char session_id[33];
            generate_session_id(session_id);

            FILE *sf = fopen(SESSIONS_FILE, "a");
            if (sf) {
                fprintf(sf, "%s|%s\n", session_id, logged_in_user);
                fclose(sf);
                
                // 1. Set the Session Cookie
                printf("Set-Cookie: %s%s; path=/; HttpOnly\n", SESSION_KEY, session_id);

                // 2. Redirect to the chat page
                printf("Location: /chat_app/chat.html\n\n"); 
                result = 1;
            } else {
                // Server error in session file write
                printf("Location: /chat_app/signin.html?error=3\n\n");
                result = 1;
            }
        } else {
            // FAILED SIGNIN (Invalid credentials or missing fields)
            printf("Location: /chat_app/signin.html?error=1\n\n");
            result = 1;
        }

        // Cleanup
        if (login) free(login);
        if (password) free(password);
        
    } else {
        // Unknown action or missing action
        printf("Location: /chat_app/signin.html?error=3\n\n");
        result = 1;
    }

    if (action) free(action);
    return 0;
}

