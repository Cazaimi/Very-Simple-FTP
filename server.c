#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 500
#define SMALLER_BUFFER_SIZE 50
#define PORT 50002

/* Appends preFix+sufFIx into string target*/
void append(char *target, char *preFix, char *sufFix) {
    strcpy(target, preFix);
    strcat(target, sufFix);
}

/*Gets the substring from start to end out of input and returns.*/
char * substr(char * input, int start, int end) {
  char * subString = malloc(end - start + 2);
  int i = 0;

  for(i = start;i <= end; i++) {
    subString[i - start] = input[i];
    printf("%d", i);
  }
  printf("\n%d\n", i);
  subString[i] = '\0';
  
  return subString;
}

/*Sets all the characters to '\0' in the string passed.*/
void reinitializeString(char * input) {
  int i = 0;
  for(i = 0;i < BUFFER_SIZE; i++) input[i] = '\0';
}

/*Does the same thing as reinitializeString but also expects length. This is better for strings that do not have the same length as BUFFER_SIZE*/
void reinitializeWithLength(char * target, int size) {
  int i;
  for(i = 0;i < size; i++) {
    target[i] = '\0';
  }
}

/*Returns file pointer to a given file in the directory.*/
FILE * getFile(char * fileName) {
  FILE * fp;

  fp = fopen(fileName, "r");
 
  if (fp == NULL) {
    perror("Error while opening the file.\n");
    exit(EXIT_FAILURE);
  }

  return fp;
}

void getCredentials(char * target[], int size, FILE * file) {
  char ch, word[50];

  int i = 0, j = 0;

  //strcpy(target, "");

  while( (ch = fgetc(file)) != EOF) {
    if (ch == '\n' || ch == ' ') {
      i = 0;

      printf("j: %d, word: %s\n", j, word);

      strcpy(target[j], word);
      j++;

      printf("here\n");
      reinitializeWithLength(word, 50);
    }
    else {
      word[i] = ch;
      i++;
    }
  }

  strcpy(target[j], word);
  j++;
}

int main() {
  int serverSocket, clientSocket, i, receivedMsgSize, sentMsgSize,
  
  // Whether server is active or not.
  active = 1;

  /* Server replies */
  char success[] = "+",
    error[] = "-",
    number[] = "#",
    loggedIn[] = "1",

    /* For access control. 
    0 => Initialization to be done
    1 => Initialization done, but unauthenticated
    2 =>  Correct Username
    3 => Authenticated. */
    state = "0",
    * credentials[SMALLER_BUFFER_SIZE];


  struct sockaddr_in saddr, caddr;

  // Get socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  saddr.sin_family=AF_INET;
  saddr.sin_addr.s_addr=htonl(INADDR_ANY); // Accept connections from all hosts.
  saddr.sin_port=htons(PORT);  //Server port

  // Bind socket to port.
  if (bind(serverSocket, (struct sockaddr *)&saddr, sizeof(saddr))) {
    printf("Error binding port\n");
    
    return 0;
  }
  
  // Start listening on port:50001
  if (listen(serverSocket, 1)) {
    printf("Error initiating listen\n");

    return 0;
  }

  // Accept connection
  clientSocket = accept(serverSocket, (struct sockaddr *)NULL, NULL);

  char receiveBuffer[BUFFER_SIZE] = "\0",
    helloMesssagePlus[] = "Now connected to the VSFTP server.\n",
    helloMesssageMinus[] = "VSFTP does not feel like talking right now. Check back later.\n";

  receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

  /*If init command is passed, set state to initialized and reply with hello message. 
  Else if the command passed is not init, reject politely. :|*/
  if (strcmp(receiveBuffer, "initialize") == 0) {
    state = "1";
    if (active) {
      char response[100] = "\0";

      //prefix message with success.
      append(response, success, helloMesssagePlus);
      sentMsgSize = send(clientSocket, response, strlen(response), 0);
    }
    else {
      char response[100] = "\0";

      append(response, error, helloMesssageMinus);

      sentMsgSize = send(clientSocket, response, strlen(response), 0);

      close(clientSocket);

      return 0;
    }
  }
  else {
      char response[100] = "\0";

      append(response, error, helloMesssageMinus);

      sentMsgSize = send(clientSocket, response, strlen(response), 0);

      close(clientSocket);

      return 0;
  }

  // Now we are initialized. Check if User sent the USER command and the user exists.
  while (strcmp(substr(receiveBuffer, 0, 4), "USER") != 0){
    reinitializeString(receiveBuffer);
    printf("Buffer before receive: %s\n", receiveBuffer);
    receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

    char response[100];

    append(response, error, "Invalid user-id, try again");

    sentMsgSize = send(clientSocket, response, strlen(response), 0);
    printf("Wrong Message: %s\n", receiveBuffer);
  }

  printf("Right message: %s\n", receiveBuffer);
    close(clientSocket); 

  // USER command was received. Read users.txt
  char fileName[] = "users.txt";

  FILE * usersFile;
  usersFile = getFile(fileName);

  // Parse usernames and passwords from Users.txt
  // Allocate memory to users array. 
  for (i = 0;i < SMALLER_BUFFER_SIZE; i++) {
    /* Assuming username/password will be a string no longer than 30 characters. */
    credentials[i] = malloc(30*sizeof(char*));
    strcpy(credentials[i], "");
  }

  // Store the username and passwords in the credentials array. 
  /* Assuming that no more than 25 usernames and 25 passwords will be present.*/
  getCredentials(credentials, 50, usersFile);

  for(i = 0;i < 50; i++) {
    printf("Credential: %s\n", credentials[i]);
    free(credentials[i]);
  }

  return 0;
}
