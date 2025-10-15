#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FRIENDS_FILE "friends.txt"

typedef struct Friend {
    char name[50];
    struct Friend *next;
} Friend;

void trim_newline(char *s) { s[strcspn(s, "\n")] = 0; }

int friendship_exists(const char *u1, const char *u2) {
    FILE *fp = fopen(FRIENDS_FILE, "r");
    if (!fp) return 0;
    char a[50], b[50];
    while (fscanf(fp, "%49s %49s", a, b) == 2) {
        if ((strcmp(a,u1)==0 && strcmp(b,u2)==0) || (strcmp(a,u2)==0 && strcmp(b,u1)==0)) {
            fclose(fp); return 1;
        }
    }
    fclose(fp);
    return 0;
}

void add_friendship(const char *u1, const char *u2) {
    FILE *fp = fopen(FRIENDS_FILE, "a");
    if (fp) { fprintf(fp, "%s %s\n", u1, u2); fclose(fp); }
}

Friend* load_friends(const char *user) {
    FILE *fp = fopen(FRIENDS_FILE, "r");
    if (!fp) return NULL;
    Friend *head=NULL,*tail=NULL;
    char a[50],b[50];
    while (fscanf(fp, "%49s %49s", a, b)==2) {
        char *f = NULL;
        if (strcmp(a,user)==0) f=b;
        else if (strcmp(b,user)==0) f=a;
        if (f) {
            Friend *n=malloc(sizeof(Friend));
            strcpy(n->name,f);
            n->next=NULL;
            if(!head) head=tail=n;
            else {tail->next=n;tail=n;}
        }
    }
    fclose(fp);
    return head;
}

void free_friends(Friend *h){while(h){Friend*t=h;h=h->next;free(t);} }

int main(void) {
    printf("Content-Type: text/html\n\n");
    printf("<html><body>");

    FILE *sf = fopen("session.txt", "r");
    if (!sf) { printf("<h3>Please <a href='/chat_app/signin.html'>Sign in</a>.</h3></body></html>"); return 0; }
    char user[50];
    fgets(user,sizeof(user),sf);
    fclose(sf);
    trim_newline(user);

    printf("<h2>Welcome, %s!</h2>", user);

    char *query = getenv("QUERY_STRING");
    if (query && strstr(query, "friend=")) {
        char fname[50];
        sscanf(query,"friend=%49[^&]",fname);
        for(int i=0;fname[i];i++) if(fname[i]=='+') fname[i]=' ';
        if(strlen(fname)>0 && strcmp(fname,user)!=0){
            if(!friendship_exists(user,fname)){
                add_friendship(user,fname);
                printf("<p>Friend <b>%s</b> added!</p>",fname);
            } else printf("<p><b>%s</b> is already your friend.</p>",fname);
        }
    }

    printf("<h3>Your Friends:</h3>");
    Friend *f=load_friends(user);
    if(!f) printf("<p>No friends yet.</p>");
    else {
        printf("<ul>");
        for(Friend*t=f;t;t=t->next)
            printf("<li><a href='/cgi-bin/chat.cgi?friend=%s'>%s</a></li>",t->name,t->name);
        printf("</ul>");
    }

    printf("<h3>Add Friend:</h3>");
    printf("<form action='/cgi-bin/friend.cgi' method='get'>");
    printf("<input type='text' name='friend' required placeholder='Enter username'>");
    printf("<input type='submit' value='Add Friend'></form>");
    printf("<a href='/chat_app/index.html'>Back to Home</a>");

    free_friends(f);
    printf("</body></html>");
    return 0;
}

