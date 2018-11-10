#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

int main() {
	int clientID = 0;
	int cs; // file descriptor per csPath
	char *csPath = "/tmp/csFifo"; // utilizzato per connessione client
	char clientConnected[BUFSIZ][4]; // max 3 cifre per ID client
	int id;
	int pidProc[BUFSIZ];
	int numConnected = 0;

	int client_to_server; // fd per myFifo
	char *myfifo = "/tmp/client_to_server_fifo";

	int server_to_client[BUFSIZ]; // array di fd per myFifo2
	char *myfifo2 = "/tmp/server_to_client_fifo";

	char buf[BUFSIZ], buf2[BUFSIZ]; // buffer temporanei di appoggio
	char csFifo[BUFSIZ], scFifo[BUFSIZ];

	printf("Server ON.\n");

	/* crea la FIFO (pipe con nome) */
	mkfifo(myfifo, 0666);
	mkfifo(myfifo2, 0666);

	/* apre le due FIFO */
	client_to_server = open(myfifo, O_RDONLY);
	server_to_client[0] = open(myfifo2, O_WRONLY);

	while (1) {
		server_to_client[0] = open(myfifo2, O_WRONLY);
		read(client_to_server, buf, sizeof(buf));

		if (strcmp("1", buf) == 0) { // 1. Connect to Server
			char client[10];
			printf("Received: %s\n", buf); // stampa l'opzione ricevuta
			clientID++;
			sprintf(clientConnected[clientID - 1], "%d", clientID);
			numConnected++;
			write(server_to_client[0], &clientID, sizeof(clientID));
			sprintf(buf, "your id is %d", clientID);
			sprintf(client, "Client:%d", clientID);

			unlink(myfifo2); // elimina il file per ripulirlo
			mkfifo(myfifo2, 0666); // ricrea la fifo
			write(server_to_client[0], "OK", sizeof("OK")); // aspetta il Client
			server_to_client[clientID] = open(myfifo2, O_WRONLY);

			write(server_to_client[clientID], client, BUFSIZ);

			/* nuova connessione per dare un id al client */
			char *tmp = client;
			strcat(strcat(csFifo, csPath), tmp);
			write(server_to_client[clientID], csFifo, BUFSIZ);
			read(client_to_server, buf, sizeof(buf)); // aspetta il Client

			csPath = csFifo;
			mkfifo(csPath, 0666);
			cs = open(csPath, O_RDONLY);

			/* prendo il pid del processo client */
			int pid;
			read(cs, &pid, sizeof(pid));
			pidProc[clientID] = pid;

			read(cs, client, sizeof(client));
			printf("Connected: %s\n", client);

		} else if (strcmp("2", buf) == 0) { // 2. Get Client list
			int i = 0, j = 0;
			printf("Received: %s\n", buf);
			write(server_to_client[0], &numConnected, sizeof(numConnected));
			while (i < numConnected) { // invia i client connessi uno per uno
				if (strcmp(clientConnected[j], "") != 0) {
					write(server_to_client[0], clientConnected[j], BUFSIZ);
					i++;
				}
				j++;
			}

		} else if (strcmp("3", buf) == 0) { // 3. Send message to another Client or a group of Client
			char clientIDs[BUFSIZ], message[BUFSIZ], clients[BUFSIZ][3];
			int presentID; // booleano per presenza ID
			int countPresentIDs = 0;
			int i, j;
			int seqTerminated;
			int idRecipient;
			printf("Received: %s\n", buf);
			/* riceve il nome del cliente che invia il messaggio */
			read(client_to_server, &id, sizeof(id));
			do {
				i = j = 0;
				presentID = 1;
				/* riceve la lista dei client a cui inviare il messaggio */
				read(client_to_server, clientIDs, sizeof(clientIDs));
				seqTerminated = strcmp(clientIDs, "0") == 0; // si termina con il carattere 0

				while (i < numConnected) {
					if (strcmp(clientConnected[j], "0") != 0) {
						/* se le due stringhe sono uguali la funzione ritorna 0 */
						if (strcmp(clientConnected[j], clientIDs) != 0) {
							presentID = 0; // ID non presente
						} else {
							presentID = 1; // ID presente
							strcpy(clients[countPresentIDs++], clientIDs);
							break;
						}
						i++;
					}
					j++;
				}
				if(!seqTerminated) {
					write(server_to_client[0], &presentID, sizeof(presentID));
				} else {
					presentID = 0;
					write(server_to_client[0], &presentID, sizeof(presentID));
					break; // esce se nessun ID e' presente e la sequenza e' terminata
				}
			} while (!seqTerminated);

			if (countPresentIDs > 0) {
				/* riceve il messaggio da recapitare ai client */
				read(client_to_server, message, sizeof(message));
				/* invia messaggio ai client presenti */
				for (i = 0; i < countPresentIDs; i++) {
					idRecipient = atol(clients[i]);
					write(server_to_client[idRecipient], &id, sizeof(id));
					write(server_to_client[idRecipient], message, sizeof(message));
					kill(pidProc[idRecipient], SIGUSR1);
				}
			}

		} else if (strcmp("4", buf) == 0 || strcmp("5", buf) == 0) { // 4. Disconnect e 5. Exit
			printf("Received: %s\n", buf);
			read(client_to_server, &id, sizeof(id));
			close(server_to_client[id]);
			memset(clientConnected[id - 1], 0, sizeof(clientConnected[id - 1]));
			numConnected--;
			printf("Client:%d has disconnected from the Server.\n", id);
		}


		/* clean buf from any data */
		memset(buf, 0, sizeof(buf));
		memset(buf2, 0, sizeof(buf2));
		memset(csFifo, 0, sizeof(csFifo));
		memset(scFifo, 0, sizeof(scFifo));
		csPath = "/tmp/csFifo";
		close(server_to_client[0]);
		server_to_client[0] = 0;
	}

	close(client_to_server);

	unlink(myfifo);
	unlink(myfifo2);
	return 0;
}
