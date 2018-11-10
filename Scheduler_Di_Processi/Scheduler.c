#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define P_SPACE 11

struct Task {
	int id;
	char nomeTask[9];
	int priorita;
	int esecuzioniRimanenti;

	struct Task *next;
};
typedef struct Task TASK;

enum policyType {
	PRIORITA, ESECUZIONIRIMANENTI
};
int policy = PRIORITA;
int numTask = 0;

void printMenu();
TASK choiceInsert(int *id);
void print_list(TASK *head);
void getSpaced(char str[]);
TASK *push(TASK *head, TASK task);
TASK *sortingByPolicy(TASK *head);
TASK *executeTaskHead(TASK *head);
TASK *executeTaskId(TASK *head, int *id);
TASK *deleteTaskId(TASK *head, int *id);
void modifyTaskIdPriority(TASK *head, int *id, int *priority);
TASK *changePolicy(TASK *head);
TASK *sortingByPolicy(TASK *head);

int main() {
	TASK *head;
	TASK t;
	head = malloc(sizeof(TASK)); // alloca memoria della dimensione di TASK per la testa della lista

	int menuChoice = 0, id = 1, selId = 0, priority = 0;

	while (menuChoice != 7) {
		printMenu();
		printf("Inserire un numero da 1 a 7 per selezionare un'opzione dal menu': ");
		scanf("%d", &menuChoice);
		while (menuChoice < 1 || menuChoice > 7) {
			/*
			 * ripulisce lo scanf se c'e' rimasto un '\n'
			 * dovuto ad un inserimento di un carattere al posto di un numero intero
			 */
			while (getchar() != '\n');
			printf("ERRORE! Inserire un numero da 1 a 7: ");
			scanf("%d", &menuChoice);
		}
		switch (menuChoice) {
		case 1: // inserimento nuovo task
			t = choiceInsert(&id);
			head = push(head, t);
			numTask++;
			break;
		case 2: // esecuzione task in testa
			if (numTask > 0) {
				head = executeTaskHead(head);
			} else {
				printf("ERRORE! Inserire prima un task.\n");
			}
			break;
		case 3: // esecuzione task a scelta
			if (numTask > 0) {
				printf("Inserire ID task: ");
				while (scanf("%d", &selId) == 0) { // se la scanf restituisce 0 allora l'input non è un intero
					while (getchar() != '\n');
					printf("ERRORE! Inserire nuovamente ID task: ");
				}
				head = executeTaskId(head, &selId);
			} else {
				printf("ERRORE! Inserire prima un task.\n");
			}
			break;
		case 4: // eliminazione task a scelta
			if (numTask > 0) {
				printf("Inserire ID task: ");
				while (scanf("%d", &selId) == 0) { // se la scanf restituisce 0 allora l'input non è un intero
					while (getchar() != '\n');
					printf("ERRORE! Inserire nuovamente ID task: ");
				}
				head = deleteTaskId(head, &selId);
			} else {
				printf("ERRORE! Inserire prima un task.\n");
			}
			break;
		case 5: // modifica priorita' task a scelta
			if (numTask > 0) {
				printf("Inserire ID task: ");
				while (scanf("%d", &selId) == 0) { // se la scanf restituisce 0 allora l'input non è un intero
					while (getchar() != '\n');
					printf("ERRORE! Inserire nuovamente ID task: ");
				}
				printf("Inserire nuova priorita': ");
				scanf("%d", &priority);
				while (priority < 0 || priority > 9) {
					while (getchar() != '\n');
					printf("ERRORE. Inserire nuovamente la nuova priorita': ");
					scanf("%d", &priority);
				}
				modifyTaskIdPriority(head, &selId, &priority);
				head = sortingByPolicy(head);
			} else {
				printf("ERRORE! Inserire prima un task.\n");
			}
			break;
		case 6: // modifica politica scheduling
			head = changePolicy(head);
			break;
		case 7: // esci
			printf("Programma Terminato...");
			return 0;
			break;
		}
		if (numTask > 0) {
			print_list(head);
		} else {
			printf("\nLista vuota! Nessun task e' presente.\n");
			printf("La Politica di Scheduling attuale e': %s \n", policy == 0 ? "Priorita'" : "Esecuzioni Rimanenti");
		}
	}

	return 0;
}

void printMenu() {
	printf("\nMenu\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("1. Inserire un nuovo task.\n");
	printf("2. Eseguire il task che si trova in testa alla coda.\n");
	printf("3. Eseguire il task il cui identificatore ID e' specificato dall'utente.\n");
	printf("4. Eliminare il task il cui identificatore ID e' specificato dall'utente.\n");
	printf("5. Modificare la PRIORITA' del task il cui identificatore ID è specificato dall'utente.\n");
	printf("6. Cambiare la politica di scheduling utilizzata, passando dalla a) alla b) e viceversa.\n");
	printf("7. Uscire dal programma.\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
}

TASK choiceInsert(int *id) {
	TASK task;
	task.id = (*id)++;

	printf("Inserisci stringa di max 8 caratteri per nome task: ");
	scanf("%s", task.nomeTask);
	while (strlen(task.nomeTask) > 8) {
		printf("ERRORE. Inserisci nuovamente nome task: ");
		scanf("%s", task.nomeTask);
	}

	printf("Inserisci numero compreso tra 0 e 9 per priorita' task: ");
	scanf("%d", &task.priorita);
	while (task.priorita < 0 || task.priorita > 9) {
		while (getchar() != '\n');
		printf("ERRORE. Inserisci nuovamente priorita' task: ");
		scanf("%d", &task.priorita);
	}

	printf("Inserisci esecuzioni rimanenti task: ");
	scanf("%d", &task.esecuzioniRimanenti);
	while (task.esecuzioniRimanenti < 1 || task.esecuzioniRimanenti > 99) {
		while (getchar() != '\n');
		printf("ERRORE. Inserisci nuovamente esecuzioni rimanenti task: ");
		scanf("%d", &task.esecuzioniRimanenti);
	}
	return task;
}

void print_list(TASK *head) {
	TASK *current = head;
	printf("\nLa Politica di Scheduling attuale e': %s \n",	policy == 0 ? "Priorita'" : "Esecuzioni Rimanenti");
	printf("+----+-----------+-----------+------------------+");
	printf("\n| ID | PRIORITA' | NOME TASK | ESECUZ. RIMANENTI|\n");
	printf("+----+-----------+-----------+------------------+\n");

	while (current != NULL && current->id != 0) {
		if (current->id % 10 == current->id) {
			printf("| %d  ", current->id);
		} else {
			printf("| %d ", current->id);
		}

		printf("|     %d     ", current->priorita);

		printf("|%s", current->nomeTask);
		getSpaced(current->nomeTask);

		if (current->esecuzioniRimanenti % 10 == current->esecuzioniRimanenti) {
			printf("|        %d         |", current->esecuzioniRimanenti);
		} else {
			printf("|        %d        |", current->esecuzioniRimanenti);
		}

		current = current->next;
		printf("\n+----+-----------+-----------+------------------+\n");
	}
}

void getSpaced(char str[]) {
	int length, space;
	char s[11];

	length = P_SPACE - strlen(str);
	for (space = 0; space < length; space++) {
		strcat(s, " ");
	}
	printf("%s", s);
}

TASK *push(TASK *head, TASK task) {
	TASK *current = head;
	TASK *previous = NULL;
	if (policy == PRIORITA) {
		while (current->priorita >= task.priorita) {
			if (current->priorita == task.priorita && current->id < task.id) {
				break;
			}
			previous = current;
			current = current->next;
		}
	} else {
		while (current->next != NULL && current->esecuzioniRimanenti <= task.esecuzioniRimanenti) {
			if (current->esecuzioniRimanenti == task.esecuzioniRimanenti && current->id < task.id) {
				break;
			}
			previous = current;
			current = current->next;
		}
	}
	TASK *temp = malloc(sizeof(TASK));
	temp->id = task.id;
	temp->priorita = task.priorita;
	strncpy(temp->nomeTask, task.nomeTask, 8);
	temp->esecuzioniRimanenti = task.esecuzioniRimanenti;
	temp->next = current;
	if (numTask == 0) { // con lista vuota restituisco il task creato
		return temp;
	}
	if (previous != NULL) {
		previous->next = temp;
	} else {
		head = temp;
	}
	return head;
}

TASK *executeTaskHead(TASK *head) {
	if (--head->esecuzioniRimanenti <= 0) {
		TASK* new_head = head->next;
		free(head);
		head = new_head;
		numTask--;
	}
	return head;
}

TASK *executeTaskId(TASK *head, int *id) {
	TASK *current = head;
	TASK *previous = current;
	while (current->id != *id && current->next != NULL) {
		previous = current;
		current = current->next;
	}
	if (current->id == *id) {
		if (--current->esecuzioniRimanenti <= 0 && current == head) {
			TASK *new_head = head->next;
			head = NULL;
			free(head);
			head = new_head;
			numTask--;
			return head;
		} else if (current->esecuzioniRimanenti <= 0) {
			TASK *new_current = current->next;
			current = NULL;
			free(current);
			current = new_current;
			previous->next = current;
			numTask--;
		}
	} else {
		printf("ERRORE! L'ID scelto non e' presente.\n");
	}
	return sortingByPolicy(head);
}

TASK *deleteTaskId(TASK *head, int *id) {
	TASK *current = head;
	TASK *previous = current;
	while (current->id != *id && current->next != NULL) {
		previous = current;
		current = current->next;
	}
	if (current->id == *id) {
		if (current == head) {
			TASK *new_head = head->next;
			head = NULL;
			free(head);
			head = new_head;

		} else {
			TASK *new_current = current->next;
			current = NULL;
			free(current);
			current = new_current;
			previous->next = current;
		}
		numTask--;
	} else {
		printf("ERRORE! L'ID scelto non e' presente.\n");
	}
	return head;
}

void modifyTaskIdPriority(TASK *head, int *id, int *priority) {
	TASK *current = head;
	while (current->id != *id && current->next != NULL) {
		current = current->next;
	}
	if (current->id == *id) {
		current->priorita = *priority;
	} else {
		printf("ERRORE! L'ID scelto non e' presente.\n");
	}
}

TASK *changePolicy(TASK *head) {
	if (policy == PRIORITA) {
		policy = ESECUZIONIRIMANENTI;
	} else {
		policy = PRIORITA;
	}
	return sortingByPolicy(head);
}

TASK *sortingByPolicy(TASK *head) {
	TASK *new_head = malloc(sizeof(TASK));
	TASK *current = head;
	while (current->next != NULL) {
		new_head = push(new_head, *current);
		current = current->next;
	}
	head = NULL;
	free(head);
	return new_head;
}
