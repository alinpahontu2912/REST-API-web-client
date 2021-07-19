#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include<string.h>
#include "helpers.h"
#include "requests.h"
#include "parson.h"

char cookie[500];
char JWTtoken[500];
char buffer[100];
char * message;
char * response;
char * serialized_string = NULL;
void register_acc(int sockfd) {
    JSON_Value * root_value = json_value_init_object();
    JSON_Object * root_object = json_value_get_object(root_value);
    char username[50], password[50];
    // creates a new JSON object containing the useername and password of
    // the new user
    printf("username=");
    fgets(username, 50, stdin);
    username[strlen(username) - 1] = '\0';
    json_object_set_string(root_object, "username", username);
    printf("password=");
    fgets(password, 50, stdin);
    password[strlen(password) - 1] = '\0';
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);
    message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/register",
     "application/json", serialized_string, NULL, NULL);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    char * token = strtok(response, " ");
    token = strtok(NULL, " ");
    int code = atoi(token);
    // if the code received is different than 201,
    // the user is already taken    
    if (code == 201) {
        printf("Registered succesfully\n");
    } else {
        printf("Register error\n");
    }
    // freeing up memory
    free(message);
    free(response);
}

void login(int sockfd) {
    // clearing the old cookie if needed
    memset(cookie, 0, 500);
    JSON_Value * root_value = json_value_init_object();
    JSON_Object * root_object = json_value_get_object(root_value);
    // creates a json object and turns it into a string that makes the 
    // body of the http message
    char username[50], password[50];
    printf("username=");
    fgets(username, 50, stdin);
    username[strlen(username) - 1] = '\0';
    json_object_set_string(root_object, "username", username);
    printf("password=");
    fgets(password, 50, stdin);
    password[strlen(password) - 1] = '\0';
    json_object_set_string(root_object, "password", password);
    serialized_string = json_serialize_to_string_pretty(root_value);
    message = compute_post_request("34.118.48.238", "/api/v1/tema/auth/login",
     "application/json", serialized_string, NULL, NULL);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
    send_to_server(sockfd, message);
    response = receive_from_server(sockfd);
    char * copy = strdup(response);
    char * token = strtok(response, " ");
    token = strtok(NULL, " ");
    int code = atoi(token);
    if (code == 200) {
        printf("Logged in succesfully\n");
        char * token2 = strtok(copy, "\r\n");
        while (strncmp(token2, "Set-Cookie: ", 12) != 0) {
            token2 = strtok(NULL, "\r\n");
        }
        strcpy(cookie, token2 + 12);
        printf("%s\n", cookie);
    } else {
        printf("Log in error\n");
    }
    // freeing up memory
    free(copy);
    free(message);
    free(response);
}

void enter_library(int sockfd) {
    // tries to make a get request to enter the library
    // if no login cookie exists, it will print error message
    if (strlen(cookie) != 0) {
        memset(JWTtoken, 0, 500);
        message = compute_get_request("34.118.48.238", "/api/v1/tema/library/access", NULL, cookie, NULL);
        send_to_server(sockfd, message);
        free(message);
        response = receive_from_server(sockfd);
        char * copy = strdup(response);
        char * token = strtok(response, " ");
        token = strtok(NULL, " ");
        int code = atoi(token);
        if (code == 200) {
            printf("Entered library\n");
        } else {
            printf("Did not enter library\n");
        }
        // copy the JWT token from the received message
        char * start = strstr(copy, "{");
        JSON_Value * schema = json_parse_string(start);
        JSON_Object * root_object = json_value_get_object(schema);
        serialized_string = json_object_get_string(root_object, "token");
        strcpy(JWTtoken, serialized_string);
        printf("%s\n", JWTtoken);
        // freeing up memory
        json_value_free(schema);
        free(copy);
        free(response);
    } else {
        printf("Log in first\n");
    }

}

void get_books(int sockfd) {
    // must be logged in and 
    // must have entered library
    if (strlen(cookie) == 0) {
        printf("Log in first\n");
    } else if (strlen(JWTtoken) == 0) {
        printf("Enter library first\n");
    } else {
        message = compute_get_request("34.118.48.238", "/api/v1/tema/library/books", NULL, cookie, JWTtoken);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        char * copy = strdup(response);
        char * token = strtok(response, " ");
        token = strtok(NULL, " ");
        char * start = strstr(copy, "[");
        // if the message code is 200, then success
        if (atoi(token) == 200) {
            printf("Got all books:\n");
        } else {
            printf("Could not get books\n");
        }
        JSON_Value * schema = json_parse_string(start);
        JSON_Array * array = json_array(schema);
        int len = json_array_get_count(array);
        for (int i = 0; i < len; i++) {
            JSON_Value * res = json_array_get_value(array, i);
            serialized_string = json_serialize_to_string_pretty(res);
            printf("%s\n", serialized_string);
            json_free_serialized_string(serialized_string);
        }
        // freeing up memory
        free(copy);
        free(message);
        free(response);
        json_array_clear(array);
        json_value_free(schema);
    }

}

void add_book(int sockfd) {
    // must be logged in and 
    // must have entered library
    if (strlen(cookie) == 0) {
        printf("Log in first\n");
    } else if (strlen(JWTtoken) == 0) {
        printf("Enter library first\n");
    } else {
        // get needed data
        char title[50], author[50], genre[50], publisher[50], page_count[50];
        printf("title=");
        fgets(title, 50, stdin);
        title[strlen(title) - 1] = '\0';
        printf("author=");
        fgets(author, 50, stdin);
        author[strlen(author) - 1] = '\0';
        printf("genre=");
        fgets(genre, 50, stdin);
        genre[strlen(genre) - 1] = '\0';
        printf("publisher=");
        fgets(publisher, 50, stdin);
        publisher[strlen(publisher) - 1] = '\0';
        printf("page_count=");
        fgets(page_count, 50, stdin);
        page_count[strlen(page_count) - 1] = '\0';
        // create new json object
        JSON_Value * root_value = json_value_init_object();
        JSON_Object * root_object = json_value_get_object(root_value);
        // check if the data entered is correct
        if (strlen(title) > 0 && strlen(author) > 0 && strlen(genre) > 0 
            && strlen(page_count) > 0 && strlen(publisher) > 0 &&
            (page_count[0] <= '9' && page_count[0] >= '0')) {
            json_object_set_string(root_object, "title", title);
            json_object_set_string(root_object, "author", author);
            json_object_set_string(root_object, "genre", genre);
            json_object_set_number(root_object, "page_count", atoi(page_count));
            json_object_set_string(root_object, "publisher", publisher);
            serialized_string = json_serialize_to_string_pretty(root_value);
            message = compute_post_request("34.118.48.238", "/api/v1/tema/library/books",
             "application/json", serialized_string, cookie, JWTtoken);
            json_free_serialized_string(serialized_string);
            json_value_free(root_value);
            send_to_server(sockfd, message);
            response = receive_from_server(sockfd);
            char * token = strtok(response, " ");
            token = strtok(NULL, " ");
            int code = atoi(token);
            // if the message code is 200, then succes
            if (code == 200) {
                printf("Added succesfully\n");
            } else {
                printf("Could not add books\n");
            }
            // freeing up memory
            free(message);
            free(response);
        } else {
            printf("Data introduced is not correct\n");
        }

    }
}
void delete_book(int sockfd) {
    // must be logged in and 
    // must have entered library
    if (strlen(cookie) == 0) {
        printf("Log in first\n");
    } else if (strlen(JWTtoken) == 0) {
        printf("Enter library first\n");
    } else {
        // get wanted information
        char id[50];
        printf("id=");
        fgets(id, 50, stdin);
        id[strlen(id) - 1] = '\0';
        char delete[100];
        memset(delete, 0, 100);
        strcpy(delete, "/api/v1/tema/library/books/");
        strcat(delete, id);
        message = compute_delete_request("34.118.48.238", delete, cookie, JWTtoken);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        char * token = strtok(response, " ");
        token = strtok(NULL, " ");
        int code = atoi(token);
        if (code == 200) {
            printf("Deleted succesfully\n");
        } else {
            printf("Could not delete books\n");
        }
        // freeing up memory
        free(message);
        free(response);

    }
}

void logout(int sockfd) {
    // must be logged in
    if (strlen(cookie) == 0) {
        printf("You are not even logged in\n");
    } else {
        message = compute_get_request("34.118.48.238", "/api/v1/tema/auth/logout", NULL, cookie, NULL);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        memset(cookie, 0, 500);
        memset(JWTtoken, 0, 500);
        char * copy = strdup(response);
        char * token = strtok(response, " ");
        token = strtok(NULL, " ");
        int code = atoi(token);
        // if code is 200, then success
        if (code == 200) {
            printf("Logout succesfully\n");
        } else {
            printf("Could not log out\n");
        }
        // freeing up memory
        free(copy);
        free(message);
        free(response);
    }
}

void get_book(int sockfd) {
    // must be logged in and 
    // must have entered library
    if (strlen(cookie) == 0) {
        printf("Log in first\n");
    } else if (strlen(JWTtoken) == 0) {
        printf("Enter library first\n");
    } else {
        char id[50];
        printf("id=");
        // scanf("%s", id);
        fgets(id, 50, stdin);
        id[strlen(id) - 1] = '\0';
        char info[100];
        memset(info, 0, 100);
        strcpy(info, "/api/v1/tema/library/books/");
        strcat(info, id);
        message = compute_get_request("34.118.48.238", info, NULL, cookie, JWTtoken);
        send_to_server(sockfd, message);
        response = receive_from_server(sockfd);
        char * copy = strdup(response);
        char * token = strtok(response, " ");
        token = strtok(NULL, " ");
        int code = atoi(token);
        // if code is 200, then success
        if (code == 200) {
            printf("Book accessed\n");
            char * start = strstr(copy, "{");
            JSON_Value * schema = json_parse_string(start);
            serialized_string = json_serialize_to_string_pretty(schema);
            printf("%s\n", serialized_string);
            json_free_serialized_string(serialized_string);
            json_value_free(schema);
            serialized_string = NULL;

        } else {
            printf("Did not acces book\n");
        }
        // freeing memory
        free(copy);
        free(message);
        free(response);
    }
}

int main(int argc, char * argv[]) {

    int sockfd;
    // based on stdin input, one of the following operations will be made
    while (1) {
        sockfd = open_connection("34.118.48.238", 8080, AF_INET, SOCK_STREAM, 0);
        // connection to the server is open and closed after every operation
        fgets(buffer, 100, stdin);
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        if (strncmp(buffer, "register", 8) == 0) {
            register_acc(sockfd);
        }
        if (strncmp(buffer, "login", 5) == 0) {
            login(sockfd);
        }
        if (strncmp(buffer, "enter_library", 11) == 0) {
            enter_library(sockfd);
        }
        if (strncmp(buffer, "get_books", 9) == 0) {
            get_books(sockfd);
        }
        if (strncmp(buffer, "add_book", 8) == 0) {
            add_book(sockfd);
        }
        if (strncmp(buffer, "delete_book", 11) == 0) {
            delete_book(sockfd);
        }
        if (strncmp(buffer, "get_book", 8) == 0 && strncmp(buffer, "get_books", 9) != 0) {
            get_book(sockfd);
        }
        if (strncmp(buffer, "logout", 6) == 0) {
            logout(sockfd);
        }
        close_connection(sockfd);
    }

    return 0;

}