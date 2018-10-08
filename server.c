#include <dirent.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 500
#define SMALLER_BUFFER_SIZE 50
#define PORT 50001

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
  }
  subString[i] = '\0';
  
  return subString;
}

/*Sets all the characters to '\0' in the string passed, assuming that string length = BUFFER_SIZE*/
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

/* Takes in a pre-allocated string and fills it with the contents of the current directory. Note: Do not forget to free the allocated string's memory after operation.*/
void readDir(char * response) {
  DIR *d;

  struct dirent *dir;

  d = opendir(".");

  int i = 0;

  if (d) {
    while ((dir = readdir(d)) != NULL) {
      printf("%d:%s\n", i, dir->d_name);
      strcat(response, dir->d_name);
      strcat(response, "\n");
      i++;
    }
    closedir(d);
  }
}

/*function to get size of the file.*/
long int findSize(const char *file_name)
{
    struct stat st; /*declare stat variable*/
     
    /*get the size using stat()*/
     
    if(stat(file_name,&st)==0)
        return (st.st_size);
    else
        return -1;
}



int main() {
  int serverSocket, clientSocket, i, receivedMsgSize, sentMsgSize,
  
  // Whether server is active or not.
  active = 1;

  /* Server replies */
  char success[] = "+",
    error[] = "-",
    number[] = "#",
    loggedIn[] = "!",

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

  // Flag to check whether user is authenticated.
  int authenticated = 0, userNameIndex = -1;

  // Now we are initialized. Check if User sent the USER command and the user exists.
  while (1){
      reinitializeString(receiveBuffer);

      receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

      char response[100];

      if (strcmp(substr(receiveBuffer, 0, 3), "USER") != 0) {
        append(response, error, "Invalid user-id, try again");

        sentMsgSize = send(clientSocket, response, strlen(response), 0);
        
        printf("Wrong Message: %s\n", receiveBuffer);
        
        continue;
      }

      char * userName = substr(receiveBuffer, 5, strlen(receiveBuffer));

      printf("Username: %s\n", userName);
      
      // The command USER was entered now, check if username is valid or not.
      for(i = 0;i < SMALLER_BUFFER_SIZE; i += 2) {
        printf("Username: %s, stored: %s\n", userName, credentials[i]);
        if ( strcmp( credentials[i], substr( receiveBuffer, 5, strlen(receiveBuffer) ) ) == 0 ) {
          printf("%d\n", i);
          userNameIndex = i;
          state = "2";
          authenticated = 1;

          break;
        }
      }

      if (authenticated) break; 
      append(response, error, "Invalid user-id, try again");

      sentMsgSize = send(clientSocket, response, strlen(response), 0);
    }

  printf("Received buffer:%s\n", receiveBuffer);
  // Username is valid. Send prompt for password.
  {
    char response[SMALLER_BUFFER_SIZE];
    append(response, success, "User-id valid, send password");
    sentMsgSize = send(clientSocket, response, strlen(response), 0);
  }

  reinitializeString(receiveBuffer);
  
  while(1) {
    reinitializeString(receiveBuffer);

    receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

    printf("%s\n", receiveBuffer);
    
    if (strcmp(receiveBuffer, credentials[userNameIndex + 1]) != 0) {
      char * response[SMALLER_BUFFER_SIZE];
      
      append(response, error, "Wrong password, try again");

      sentMsgSize = send(clientSocket, response, strlen(response), 0);

      printf("log: Wrong password: %s\n", receiveBuffer);
      continue;
    }

    printf("Right password: %s\n", receiveBuffer);
    char response[SMALLER_BUFFER_SIZE];

    append(response, loggedIn, "\n \n Password is ok and you can begin file transfers.");

    sentMsgSize = send(clientSocket, response, strlen(response), 0);
    break;
  }

  // If you are here, you have been authenticated. File operations can begin now.

  while(1) {
    reinitializeString(receiveBuffer);

    char * response = malloc(sizeof(char) *BUFFER_SIZE);

    receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

    printf("log: received command: %s\n", receiveBuffer);

    char * command;

    command = substr(receiveBuffer, 0, 3);
    if (strcmp(command, "LIST") == 0) {
      append(response, success, "");
      readDir(response);
    }
    else if (strcmp(command, "KILL") == 0) {
      int removed = remove(substr(receiveBuffer, 5, strlen(receiveBuffer)));

      if (!removed) {
        append(response, success, strcat(substr(receiveBuffer, 5, strlen(receiveBuffer)), " deleted") );
      }
      else {
        append(response, error, "Not deleted because unknown error occured. Please try again");
      }
    }
    else if (strcmp(command, "RETR") == 0) {
      int size = findSize(substr(receiveBuffer, 5, strlen(receiveBuffer)));

      printf("Size: %d\n", size);
      if (size == -1) {
        append(response, error, "File doesn't exist");
      }
      else {
        char sizeStr[SMALLER_BUFFER_SIZE];

        sprintf(sizeStr, "%d", size);
        append(response, number, sizeStr);

        //Send the size to user. Wait for SEND or STOP.
        sentMsgSize = send(clientSocket, response, strlen(response), 0);

        printf("log: Message sent to client: %s:%d\n", response, sentMsgSize);

        reinitializeString(receiveBuffer);
        reinitializeString(response);

        receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);

        if (strcmp(substr(receiveBuffer, 0, 3), "SEND") == 0) {
          //do nothing for now.
        }
        else if (strcmp(substr(receiveBuffer, 0, 3), "STOP"){
          append(response, success, "ok, RETR aborted");
        }
        else {
          append(response, error, "Invalid Command.");
        }
      }
    }
    else if (strcmp(command, "DONE") == 0) {
      append(response, success, "GoodBye");

      sentMsgSize = send(clientSocket, response, strlen(response), 0);

      printf("log: Message sent to client: %s:%d\n", response, sentMsgSize);

      break;
    }
    else {
      append(response, error, "Invalid Command, Please enter one of LIST, KILL, RETR or DONE.");
    }

    sentMsgSize = send(clientSocket, response, strlen(response), 0);

    printf("log: Message sent to client: %s:%d\n", response, sentMsgSize);
  }
  
  close(clientSocket); 

  // Free the memory taken up by credentials array.
  for(i = 0;i < 50; i++) {
    free(credentials[i]);
  }

  return 0;
}
