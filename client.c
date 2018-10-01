#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_SIZE 6000
#define PORT 50001

void reinitializeString(char * input) {
  int i = 0;
  for(i = 0;i < BUFFER_SIZE; i++) input[i] = '\0';
}

int main(int argc, char* argv[]) {
    int clientSocket, i, receivedMsgSize, sentMsgSize;

    struct sockaddr_in addr;

    char message[BUFFER_SIZE], receiveBuffer[BUFFER_SIZE] = "\0", userName[100] = "\0";

    // Get socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr("127.0.0.1"); /* Check for server on loopback */
    addr.sin_port=htons(PORT);

    // Get connection
    if(connect(clientSocket, (struct sockaddr *)&addr,
    sizeof(addr)) < 0) {
        printf("Error connecting to server. Please try again.\n");
        return -1;
    }

    // If you reached here, it means that you have been connected. Send init message. Display reply from server.
    sentMsgSize = send(clientSocket, "initialize", strlen("initialize"), 0);

    if (sentMsgSize != strlen("initialize")) { 
      printf("Error initializing connection, please try again.\n"); 
    }

    // Print message from server.
    receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
    printf("%s\n", receiveBuffer);

    reinitializeString(receiveBuffer);
    reinitializeString(message);

    // Now, we are initialized. We will send the user Command now. 
    while(receiveBuffer[0] != '+'){
      
      scanf ("%[^\n]%*c", message);
      
      //Send USER command
      sentMsgSize = send(clientSocket, message, strlen(message), 0);
      

      reinitializeString(receiveBuffer);
      receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
      printf("%s\n", receiveBuffer);
    }

    reinitializeString(receiveBuffer);

    while(receiveBuffer[0] != '!'){
      reinitializeString(message);
      
      scanf ("%[^\n]%*c", message);
      
      //Send USER command
      sentMsgSize = send(clientSocket, message, strlen(message), 0);
      

      reinitializeString(receiveBuffer);
      receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
      printf("%s\n", receiveBuffer);
    }

    // Authenticated and ready to send file commands.

    while(1) {
      reinitializeString(receiveBuffer);
      reinitializeString(message);

      scanf ("%[^\n]%*c", message);

      sentMsgSize = send(clientSocket, message, strlen(message), 0);

      receivedMsgSize = recv(clientSocket, receiveBuffer, BUFFER_SIZE, 0);
      printf("%s\n", receiveBuffer);

      if (strcmp(receiveBuffer, "+GoodBye") == 0) break;
    }


    // All operations over.
    close(clientSocket);

    return 0;
}
