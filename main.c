/*
  main.c

  Ενιαίο πρόγραμμα που, ανάλογα με το argv[1], λειτουργεί ως "server" ή "client".

  1) ./myeshop server
     - ξεκινά τη λογική server:
       * initializeCatalog()
       * socket, bind, listen στην πόρτα 9000
       * δέχεται 5 πελάτες (καθένας με 10 αιτήματα)
       * στο τέλος τυπώνει σύνοψη

  2) ./myeshop client
     - ξεκινά τη λογική client:
       * fork() σε 5 child
       * κάθε παιδί κάνει connect() στον server
       * στέλνει 10 αιτήματα
       * περιμένει απάντηση (success/cost)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>      // for fork, sleep, read, write, close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>  // for sockaddr_in
#include <arpa/inet.h>
#include <sys/wait.h>
#include <time.h>

#include "myeshop.h"

// Ρυθμίσεις πόρτας & αριθμού πελατών
#define SERVER_PORT 9000
#define CLIENTS_COUNT 5
#define ORDERS_PER_CLIENT 10

/***********************************
 * serverFunction():
 *  - initializeCatalog()
 *  - bind, listen σε SERVER_PORT
 *  - δέχεται CLIENTS_COUNT συνδέσεις
 *  - καθε σύνδεση στέλνει 10 παραγγελίες
 *  - επεξεργασία με orderHandler()
 *  - στο τέλος τυπώνει την αναφορά
 ***********************************/
void serverFunction(void)
{
    // 1) Φορτώνουμε το catalog
    initializeCatalog();

    // Δημιουργία socket TCP
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("socket() failed");
        exit(EXIT_FAILURE);
    }

    // bind() στη θύρα 9000
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family      = AF_INET;
    servAddr.sin_port        = htons(SERVER_PORT);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // 0.0.0.0

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind() failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }

    // listen()
    if (listen(serverSocket, 5) < 0) {
        perror("listen() failed");
        close(serverSocket);
        exit(EXIT_FAILURE);
    }
    printf("[Server] Listening on port %d\n", SERVER_PORT);

    // Δεχόμαστε CLIENTS_COUNT συνδέσεις
    for (int c = 0; c < CLIENTS_COUNT; c++) {
        struct sockaddr_in cliAddr;
        socklen_t cliLen = sizeof(cliAddr);

        int clientConn = accept(serverSocket, (struct sockaddr *)&cliAddr, &cliLen);
        if (clientConn < 0) {
            perror("accept() failed");
            close(serverSocket);
            exit(EXIT_FAILURE);
        }
        printf("[Server] Accepted client #%d.\n", c);

        // κάθε client θα στείλει 10 παραγγελίες
        for (int req = 0; req < ORDERS_PER_CLIENT; req++) {
            int productID;
            ssize_t rb = read(clientConn, &productID, sizeof(productID));
            if (rb <= 0) {
                printf("[Server] Client #%d disconnected early.\n", c);
                break;
            }

            // Προσομοίωση καθυστέρησης 0.5s
            usleep(500000);

            float costOut;
            int successFlag = orderHandler(productID, c, &costOut);

            // στέλνουμε πίσω successFlag + costOut
            if (write(clientConn, &successFlag, sizeof(successFlag)) < 0) {
                perror("[Server] write(successFlag) failed");
                break;
            }
            if (write(clientConn, &costOut, sizeof(costOut)) < 0) {
                perror("[Server] write(costOut) failed");
                break;
            }
        }
        close(clientConn);
        printf("[Server] Client #%d done.\n", c);
    }

    close(serverSocket);

    // Τελική αναφορά
    printf("\n****** Τελική Αναφορά (Server) ******\n");
    for (int i = 0; i < PRODUCTS_COUNT; i++) {
        printf("Product %2d (%s): ζητήθηκε=%d, πουλήθηκε=%d, υπόλοιπο=%d\n",
               i,
               catalogArray[i].desc,
               catalogArray[i].reqCount,
               catalogArray[i].soldCount,
               catalogArray[i].stock);

        if (catalogArray[i].failCnt > 0) {
            printf("  -> Πελάτες που απέτυχαν: ");
            for (int f = 0; f < catalogArray[i].failCnt; f++) {
                printf("%d ", catalogArray[i].failIDs[f]);
            }
            printf("\n");
        }
    }
    printf("------------------------------------\n");
    printf("Σύνολο παραγγελιών:   %d\n", requestsTotal);
    printf("Επιτυχημένες:         %d\n", successTotal);
    printf("Αποτυχημένες:         %d\n", failedTotal);
    printf("Συνολικά έσοδα:       %.2f\n", revenueTotal);
    printf("************************************\n\n");
}

/***********************************
 * clientFunction():
 *  - φτιάχνει 5 child διεργασίες με fork()
 *  - κάθε μία συνδέεται (connect()) στον server
 *  - στέλνει 10 productIDs
 *  - διαβάζει successFlag + costOut
 ***********************************/
void clientFunction(void)
{
    printf("[Client Program] Θα δημιουργηθούν %d child διεργασίες.\n", CLIENTS_COUNT);
    srand(time(NULL));

    for (int c = 0; c < CLIENTS_COUNT; c++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork() failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
            // Είμαστε σε child
            printf("[Child %d] ξεκίνησε, PID=%d\n", c, getpid());

            int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (clientSocket < 0) {
                perror("[Child] socket() failed");
                exit(EXIT_FAILURE);
            }

            struct sockaddr_in servAddr;
            memset(&servAddr, 0, sizeof(servAddr));
            servAddr.sin_family      = AF_INET;
            servAddr.sin_port        = htons(SERVER_PORT);
            servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  // localhost

            // Προσπαθούμε να συνδεθούμε
            while (connect(clientSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0) {
                sleep(1); // αν ο server δεν είναι έτοιμος ακόμα
            }

            // στέλνουμε 10 παραγγελίες
            for (int o = 0; o < ORDERS_PER_CLIENT; o++) {
                int productID = rand() % PRODUCTS_COUNT;
                if (write(clientSocket, &productID, sizeof(productID)) < 0) {
                    perror("[Child] write(productID) failed");
                    break;
                }

                int   successFlag;
                float costValue;
                ssize_t r1 = read(clientSocket, &successFlag, sizeof(successFlag));
                ssize_t r2 = read(clientSocket, &costValue, sizeof(costValue));
                if (r1 <= 0 || r2 <= 0) {
                    printf("[Child %d] Server έκλεισε τη σύνδεση.\n", c);
                    break;
                }

                if (successFlag == 1) {
                    printf("[Child %d] Παραγγελία #%d: OK! product=%d, cost=%.2f\n",
                           c, o, productID, costValue);
                } else {
                    printf("[Child %d] Παραγγελία #%d: FAIL product=%d (out of stock)\n",
                           c, o, productID);
                }

                sleep(1); // 1 δευτερόλεπτο ανάμεσα στις παραγγελίες
            }

            close(clientSocket);
            printf("[Child %d] Τέλος.\n", c);
            exit(0);
        }
        // αλλιώς ο γονέας απλώς συνεχίζει
    }

    // Περίμενε όλα τα παιδιά
    for (int c = 0; c < CLIENTS_COUNT; c++) {
        wait(NULL);
    }
    printf("[Client Program] Όλες οι child διεργασίες τερμάτισαν.\n");
}

/***********************************
 * main():
 *  - Διαβάζει argv[1].
 *  - Αν == "server", τρέχει serverFunction()
 *  - Αν == "client", τρέχει clientFunction()
 ***********************************/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Χρήση: %s [server|client]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "server") == 0) {
        serverFunction();
    }
    else if (strcmp(argv[1], "client") == 0) {
        clientFunction();
    }
    else {
        fprintf(stderr, "Λάθος όρισμα: %s (χρησιμοποιήστε 'server' ή 'client')\n", argv[1]);
        return 1;
    }

    return 0;
}
