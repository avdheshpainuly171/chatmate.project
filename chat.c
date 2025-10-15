#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHAT_FILE "chat.txt"

typedef struct Message {
    char user[50];
    char content[500];
    struct Message *next;
} Message;

Message *head = NULL;

void trim_newline(char *s){ s[strcspn(s,"\n")]=0; }

void add_message(const char *user, const char *content) {
    Message *m = malloc(sizeof(Message));
    strcpy(m->user, user);
    strcpy(m->content, content);
    m->next = NULL;
    if(!head) head = m;
    else { Message *t=head; while(t->next) t=t->next; t->next=m; }
}

void save_message(const char *user,const char *msg){
    FILE *fp=fopen(CHAT_FILE,"a");
    if(fp){fprintf(fp,"%s:%s\n",user,msg);fclose(fp);}
}

void load_messages(){
    FILE *fp=fopen(CHAT_FILE,"r");
    if(!fp)return;
    char u[50], c[500];
    while(fscanf(fp,"%49[^:]:%499[^\n]\n",u,c)==2)
        add_message(u,c);
    fclose(fp);
}

void display_messages(){
    printf("<div id='chatbox' style='border:1px solid #000;padding:10px;width:50%%;"
           "height:300px;overflow-y:scroll;background:#fafafa;'>");
    if(!head) printf("<p>No messages yet!</p>");
    else for(Message*t=head;t;t=t->next)
        printf("<p><b>%s:</b> %s</p>",t->user,t->content);
    printf("</div>");
}

int main(void){
    printf("Content-Type: text/html\n\n");
    printf("<html><head>");
    printf("<meta http-equiv='refresh' content='3'>"); // Auto refresh every 3 seconds
    printf("<title>C Chat</title>");
    printf("</head><body style='font-family:sans-serif;'>");

    FILE *sf=fopen("session.txt","r");
    if(!sf){printf("<h3>Please <a href='/chat_app/signin.html'>Sign in</a>.</h3></body></html>");return 0;}
    char user[50];fgets(user,sizeof(user),sf);fclose(sf);
    trim_newline(user);

    printf("<h2>Chat - %s</h2>",user);

    char *query=getenv("QUERY_STRING");
    if(query && strstr(query,"content=")){
        char msg[500];
        sscanf(query,"content=%499[^&]",msg);
        for(int i=0;msg[i];i++) if(msg[i]=='+') msg[i]=' ';
        if(strlen(msg)>0){ save_message(user,msg); }
    }

    load_messages();

    printf("<form action='/cgi-bin/chat.cgi' method='get'>");
    printf("<input type='text' name='content' required style='width:300px;'>");
    printf("<input type='submit' value='Send'>");
    printf("</form>");

    printf("<h3>Messages:</h3>");
    display_messages();

    printf("<br><a href='/chat_app/index.html'>Back to Home</a>");
    printf("</body></html>");

    Message *t;
    while(head){t=head;head=head->next;free(t);}
    return 0;
}



