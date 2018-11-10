#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define flushscanf while ((getchar()) != '\n')
#define microsec 10000

void printMenu();
void handle_ctrlC_signal(int signal);
void handle_message_signal(int signal);

int id = 0, connected = 0;
int *ptrScBuffer; // utilizzato per segnali
int *ptrCsBuffer; // utilizzato per segnali

int main() {
	char clientID[BUFSIZ];
	char message[BUFSIZ];
	char clientServerPath[BUFSIZ];
	int cs; // file descriptor per csPath
	char *csPath = "/tmp/csFifo";
	int numConnected = 0;

	int client_to_server; // fd per myFifo
	ptrCsBuffer = &client_to_server;
	char *myfifo = "/tmp/client_to_server_fifo";

	int server_to_client[BUFSIZ]; // array di fd per myFifo2
	ptrScBuffer = server_to_client;
	char *myfifo2 = "/tmp/server_to_client_fifo";
	/* richiama la funzione di libreria sigaction su entrambi i tipi di struct */
	struct sigaction sa1, sa2;

	sa1.sa_handler = &handle_ctrlC_signal;
	sa2.sa_handler = &handle_message_signal;
	if (sigaction(SIGINT, &sa1, NULL) < 0) {
		perror("Error: cannot handle SIGINT"); // Should not happen
	}
	if (sigaction(SIGUSR1, &sa2, NULL) < 0) {
		perror("Error: cannot handle SIGUSR1"); // Should not happen
	}

	int choice = 0;
	while (choice != 5) {
		printMenu();
		choice = 0; // rimetto a 0 perche' dopo una signal salta la scanf e prende choice com'era prima
		scanf("%d", &choice);

		switch (choice) {

		/* Connessione, con la quale il client si registra presso il server. */
		case 1:
			if (!connected) {
				client_to_server = open(myfifo, O_WRONLY);
				server_to_client[0] = open(myfifo2, O_RDONLY);

				write(client_to_server, "1", sizeof("1"));
				read(server_to_client[0], &id, sizeof(id));

				read(server_to_client[0], message, sizeof(message)); // aspetta il Server

				server_to_client[id] = open(myfifo2, O_RDONLY);

				read(server_to_client[id], message, sizeof(message));
				printf("Connected, your id is %s\n\n", message);
				connected = 1;

				read(server_to_client[id], message, sizeof(message));
				sprintf(clientServerPath, "%s", message);
				csPath = clientServerPath;

				write(client_to_server, "OK", sizeof("OK"));

				cs = open(csPath, O_WRONLY);
				int pid = getpid();
				write(cs, &pid, sizeof(pid));
				write(cs, "OK", 3);

			} else {
				printf("Error: client already connected!\n\n");
			}
			break;

			/*
			 * Richiesta elenco ID dei client registrati, con la quale si richiede
			 * al server l'elencodei client attualmente registrati.
			 */
		case 2:
			if (connected) {
				client_to_server = open(myfifo, O_WRONLY);
				server_to_client[0] = open(myfifo2, O_RDONLY);
				write(client_to_server, "2", sizeof("2"));
				/* riceve il numero di client connessi */
				read(server_to_client[0], &numConnected, sizeof(numConnected));
				printf("Connected IDs:");
				int i = 0;
				while (i < numConnected) {
					read(server_to_client[0], message, sizeof(message)); // riceve i client connessi uno per uno
					i++;
					printf(" %s", message);
				}
				printf("\n\n");
			} else {
				printf("Error: connect before running this command\n\n");
			}
			break;

			/*
			 * Invio di un messaggio testuale a un altro client o
			 * a un insieme di client (specificandone l'ID).
			 */
		case 3:
			if (connected) {
				client_to_server = open(myfifo, O_WRONLY);
				server_to_client[0] = open(myfifo2, O_RDONLY);
				int presentID = 0; // booleano per presenza ID
				int countPresentIDs = 0;

				write(client_to_server, "3", sizeof("3"));
				usleep(microsec); // aspetta che il server elabori
				write(client_to_server, &id, sizeof(id)); // invia al server il proprio ID

				printf("Insert Client IDs, 0 to quit.\n");
				do {
					printf("Insert Client ID: ");
					scanf("%s", clientID);

					countPresentIDs++;
					/* invia al server l'ID del client a cui inviare il messaggio */
					write(client_to_server, clientID, sizeof(clientID));
					read(server_to_client[0], &presentID, sizeof(presentID));
					if (presentID == 0 && strcmp(clientID, "0") != 0) { // il client inserito non e' connesso
						printf("Server feedback: this ID is not connected, try another.\n");
						countPresentIDs--;
					}
					if (presentID == 0 && strcmp(clientID, "0") == 0) {
						printf("Insert done.\n");
						countPresentIDs--;
						break; // i client inseriti sono tutti connessi ed esce dal ciclo
					}
				} while (countPresentIDs == 0 || strcmp(clientID, "0") != 0);

				if (countPresentIDs > 0) {
					/* c'è rimansto nello stdin uno '\n' dovuto all'ultimo inserimento */
					flushscanf // macro, ripuliamo lo stdin
					memset(message, 0, sizeof(message));

					printf("Input message to server: ");
					fgets(message, 100, stdin); // 100 massima lungezza messaggio, scanf non riconsce gli spazi

					write(client_to_server, message, sizeof(message));
				} else {
					printf("No client selected.\n\n");
				}
			} else {
				printf("Error: connect before running this command\n\n");
			}

			break;

			/*
			 * Disconnessione, con la quale il client richiede la cancellazione
			 * della registrazione presso il server.
			 */
		case 4:
			if (connected) {
				client_to_server = open(myfifo, O_WRONLY);
				usleep(microsec); // aspetta che il server elabori
				write(client_to_server, "4", sizeof("4"));
				usleep(microsec); // aspetta che il server elabori
				write(client_to_server, &id, sizeof(id)); // invia al server il proprio ID

				close(client_to_server);
				close(server_to_client[id]);
				connected = 0;
			} else {
				printf("Error: connect before running this command\n\n");
			}
			break;

			/* Uscita dal programma. */
		case 5:
			if (connected) {
				client_to_server = open(myfifo, O_WRONLY);
				usleep(microsec); // aspetta che il server elabori
				write(client_to_server, "5", sizeof("5"));
				usleep(microsec); // aspetta che il server elabori
				write(client_to_server, &id, sizeof(id)); // invia al server il proprio ID

				close(client_to_server);
				close(server_to_client[id]);
			}
			return 0;

		default:
			if (choice != 0) {
				printf("\nError: insert a number between 1 and 5\n\n");
			}
			break;
		}
	}

	close(client_to_server);
	close(server_to_client[0]);
	return 0;
}

void printMenu() {
	printf("============== Menu ===============\n");
	printf("1. Connect to Server\n");
	printf("2. Get Client list\n");
	printf("3. Send message to another Client or a group of Client\n");
	printf("4. Disconnect\n");
	printf("5. Exit\n\n");

	if (connected == 1) {
		printf("Status: connected\n");
	} else {
		printf("Status: not connected\n");
	}
	printf("===================================\n");
}

/* segnale che entra in esecuzione quando viene premuto Ctrl-C */
void handle_ctrlC_signal(int signal) {
	printf("\nSignal catch. Exit...\n");
	write(*ptrCsBuffer, "5", sizeof("5"));
	usleep(microsec);
	write(*ptrCsBuffer, &id, sizeof(id));

	close(*ptrCsBuffer);
	close(*(ptrScBuffer + id));
	exit(0);
}

/* segnale che entra in esecuzione quando viene ricevuto un messaggio */
void handle_message_signal(int signal) {
	char message[BUFSIZ];
	int idSender;
	read(*(ptrScBuffer + id), &idSender, sizeof(idSender));
	read(*(ptrScBuffer + id), message, sizeof(message));
	printf("\nYou have a new message from Client %d: %s\n", idSender, message);
}
