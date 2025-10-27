#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <sys/stat.h>
#include <time.h>

// --- Configuration ---
#define SESSIONS_FILE "C:\\xampp\\cgi-bin\\sessions.txt"
#define CHATS_DIR "C:\\xampp\\cgi-bin\\chats\\" 
#define MAX_LINE 256
#define SESSION_KEY "session_id="

// --- Data Structures for Queue (Used for Reading Chat History) ---

// Structure to hold one chat message
typedef struct Message {
    char sender[MAX_LINE];
    char content[MAX_LINE * 2]; // Content field needs more space than sender
} Message;

// Node structure for the linked list queue
typedef struct Node {
    Message data;
    struct Node* next;
} Node;

// Queue structure
typedef struct Queue {
    Node *front;
    Node *rear;
} Queue;

// Function to create an empty queue
Queue* createQueue() {
    // Allocate memory for the Queue structure
    Queue *q = (Queue*)malloc(sizeof(Queue));
    if (!q) {
        fprintf(stderr, "Memory allocation failed for Queue.\n");
        exit(EXIT_FAILURE);
    }
    q->front = q->rear = NULL;
    return q;
}

// Function to check if the queue is empty
int isEmpty(Queue *q) {
    return (q->front == NULL);
}

// Function to add a message to the rear of the queue (Enqueue)
void enqueue(Queue *q, const char *sender, const char *content) {
    // Allocate memory for the new node
    Node *temp = (Node*)malloc(sizeof(Node));
    if (!temp) {
        fprintf(stderr, "Memory allocation failed for Queue Node.\n");
        return; 
    }

    // Copy data into the new node (and ensure null termination)
    strncpy(temp->data.sender, sender, MAX_LINE - 1);
    temp->data.sender[MAX_LINE - 1] = '\0';
    strncpy(temp->data.content, content, MAX_LINE * 2 - 1);
    temp->data.content[MAX_LINE * 2 - 1] = '\0';
    temp->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }

    // Link the new node
    q->rear->next = temp;
    q->rear = temp;
}

// Function to remove a message from the front of the queue (Dequeue)
// Returns the Node pointer, which the caller must free.
Node* dequeue(Queue *q) {
    if (isEmpty(q))
        return NULL;
    
    Node *temp = q->front;
    q->front = q->front->next;

    if (q->front == NULL)
        q->rear = NULL;
        
    return temp; 
}

// --- Helper Functions (as before) ---

// Helper function: Simple URL decode
void decode(char *str) {
    char *p = str;
    int hex;
    while (*str) {
        if (*str == '%') {
            // Safely decode hex value
            if (sscanf(str + 1, "%2x", &hex) == 1) {
                *p++ = hex;
                str += 3;
            } else {
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
    char *temp_query = strdup(query);
    char *result = NULL;
    char *token = strtok(temp_query, "&");
    
    while (token != NULL) {
        char *eq = strchr(token, '=');
        if (eq) {
            *eq = '\0'; 
            if (strcmp(token, key) == 0) {
                result = strdup(eq + 1);
                decode(result);
                free(temp_query);
                return result;
            }
        }
        token = strtok(NULL, "&");
    }
    free(temp_query);
    return NULL;
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
            char *uname = strtok(NULL, "|");
            
            if (sid && uname && strcmp(sid, session_id) == 0) {
                uname[strcspn(uname, "\n")] = 0; 
                strcpy(username, uname);
                fclose(fp);
                return 1; 
            }
        }
        fclose(fp);
    }
    return 0; 
}

// Function to determine the chat filename
void get_chat_filepath(const char *user1, const char *user2, char *filepath) {
    // Sort names alphabetically to ensure a consistent file path
    if (strcmp(user1, user2) < 0) {
        snprintf(filepath, MAX_LINE * 2, "%s%s_%s.txt", CHATS_DIR, user1, user2);
    } else {
        snprintf(filepath, MAX_LINE * 2, "%s%s_%s.txt", CHATS_DIR, user2, user1);
    }
}

// --- Main Program ---

int main() {
    char current_user[MAX_LINE] = {0};
    char *session_id = get_session_id();
    
    char *friend_name = NULL;
    char *content = NULL;

    // 1. Authentication Check
    if (!get_username_from_session(session_id, current_user)) {
        printf("Content-type: text/html\n");
        printf("Location: /chat_app/signin.html\n\n");
        if (session_id) free(session_id);
        return 0;
    }
    if (session_id) free(session_id); 

    // 2. Get Query Parameters
    char *query_string = getenv("QUERY_STRING");
    friend_name = get_query_param(query_string, "friend");
    content = get_query_param(query_string, "content");
    
    if (!friend_name) {
        printf("Content-type: text/plain\n\n");
        printf("ERROR: Missing friend parameter.\n");
        if (content) free(content); 
        return 0;
    }
    
    // --- CRITICAL FILE SYSTEM CHECK/SETUP ---
    // Ensure the chat directory exists (Windows compatible)
    #ifdef _WIN32
        _mkdir(CHATS_DIR);
    #else
        mkdir(CHATS_DIR, 0777); 
    #endif

    char chat_filepath[MAX_LINE * 2];
    get_chat_filepath(current_user, friend_name, chat_filepath); 

    if (content) {
        // ----------------------------------------------------
        // --- Write (SEND) message logic - Private Chat ---
        // ----------------------------------------------------
        // Queue is not required for simple file append.
        printf("Content-type: text/plain\n\n");
        
        if (strlen(content) > 0) {
            FILE *fp = fopen(chat_filepath, "a"); // Append to the specific chat file
            if (fp) {
                // Format: sender|content
                fprintf(fp, "%s|%s\n", current_user, content);
                fclose(fp);
                printf("OK"); // Success response
            } else {
                printf("ERROR: Could not open chat file for writing. Path: %s. Check permissions.\n", chat_filepath);
            }
        } else {
            printf("ERROR: Message content is empty.");
        }
    } else {
        
        // ----------------------------------------------------
        // --- Read (LOAD) message logic - Private Chat (USING QUEUE) ---
        // ----------------------------------------------------
        printf("Content-type: text/plain\n\n");
        
        Queue *message_queue = createQueue();
        FILE *fp = fopen(chat_filepath, "r"); 
        char line[MAX_LINE * 3];
        
        if (fp) {
            // 1. Read all messages from file and ENQUEUE them
            while (fgets(line, MAX_LINE * 3, fp)) {
                // Remove newline for strtok
                char *newline_pos = strchr(line, '\n');
                if (newline_pos) *newline_pos = '\0';

                char *pipe_pos = strchr(line, '|');
                if (pipe_pos) {
                    *pipe_pos = '\0'; // Split line into sender and content
                    char *sender = line;
                    char *msg_content = pipe_pos + 1;
                    // Store message in the queue
                    enqueue(message_queue, sender, msg_content);
                }
            }
            fclose(fp);
        }

        // 2. DEQUEUE and print messages in FIFO order
        Node *node_to_free = NULL;
        while (!isEmpty(message_queue)) {
            node_to_free = dequeue(message_queue);
            if (node_to_free) {
                // Print to standard output (for chat.html to consume)
                printf("%s|%s\n", node_to_free->data.sender, node_to_free->data.content);
                free(node_to_free); // Free the dynamically allocated node
            }
        }
        
        // 3. Free the queue structure itself
        if (message_queue) free(message_queue);
    }
    
    // 3. Cleanup dynamically allocated memory
    if (friend_name) free(friend_name);
    if (content) free(content);

    return 0;
}

