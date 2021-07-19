#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

char *compute_get_request(char *host, char *url, char *query_params,
                            char *cookies, char* JWTtoken)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    strcat(message, "Host: ");
    compute_message(message, host);

    if (JWTtoken != NULL){
        strcat(message, "Authorization: Bearer ");
        compute_message(message, JWTtoken);
    }

    if (cookies != NULL){
        strcat(message, "Cookie: ");
        compute_message(message, cookies);
    }
    free(line);
    compute_message(message, "");
    return message;
}

char *compute_post_request(char *host, char *url, char* content_type, char *body_data,
    char *cookies, char* JWTtoken)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *body_data_buffer = calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    

    strcat(message, "Host: ");
    compute_message(message, host);

    strcat(message, "Content-Type: ");
    compute_message(message, content_type);

    strcat(body_data_buffer, body_data);

    char* length = (char *) malloc (10 * sizeof(char));
    sprintf(length, "%ld", strlen(body_data_buffer));

    strcat(message, "Content-Length: ");
    compute_message(message, length);

    free(length);

    if (JWTtoken != NULL){
        strcat(message, "Authorization: Bearer ");
        compute_message(message, JWTtoken);
    }

    if (cookies != NULL){
        strcat(message, "Cookie: ");
        compute_message(message, cookies);
    }

    strcat(message, "\r\n");

    memset(line, 0, LINELEN);
    compute_message(message, body_data_buffer);

    free(line);
    free(body_data_buffer);
    return message;
}
char *compute_delete_request(char *host, char *url,
                            char *cookies, char* JWTtoken)
{
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    sprintf(line, "DELETE %s HTTP/1.1", url);
   
    compute_message(message, line);

    strcat(message, "Host: ");
    compute_message(message, host);

    if (JWTtoken != NULL){
        strcat(message, "Authorization: Bearer ");
        compute_message(message, JWTtoken);
    }
    if (cookies != NULL){
        strcat(message, "Cookie: ");
        compute_message(message, cookies);
    }
    
    compute_message(message, "");
    free(line);
    return message;
}