#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/msg.h>

#define MAX_SIZE 12
#define MAX_EMPLOYEES 10
#define MSG_SIZE ((2 * MAX_SIZE) + (3 * sizeof(int)))

/**
 * SYSC 4001 - Assignment 3
 * Entreprise Record Keeping System
 * Record-Keeper (server) program
 * John Breton
 */

// Structure definitions for message passing
struct msg {
        long int msg_type;
	int command;
        char employeeName[MAX_SIZE];
        char employeeDepartment[MAX_SIZE];
        int employeeNumber;
        int employeeSalary;
	struct msg *next;
};

struct response {
        long int msg_type;
	int returnData;
};

/**
 * Entry-point for the application.
 */
int main() {
	// Constants used to clear the structures after each message.
	static const struct msg emptyMsg;
	static const struct response emptyRes;
	// The message number to listen for.
	long int msgToReceive = 0;
	// Used to store all of the employees on the server. A linked list is used since deletion will occur.
	struct msg *head = NULL;
	struct msg *tail = NULL;
	// Used to keep track of the number of employees currently stored on the server.
	int count = 0;
	// Used to determine when the program should stop executing.
	int running = 1;

        // Create a message queue that will be used to receive data to the client.
        int msgidC2S = msgget((key_t)1224, 0666 | IPC_CREAT | MSG_NOERROR);

        // Ensure the message queue was successfully created (will fail on Windows even if Linux is run ontop of it).
        if (msgidC2S == -1) {
                fprintf(stderr, "Client to server message queue failed to create. msgget failed with error: %d\n", errno);
        }

        // Create a message queue that will be used to send data to the client.
        int msgidS2C = msgget((key_t)1234, 0666 | IPC_CREAT | MSG_NOERROR);

	// Ensure the message queue was successfully created (will fail on Windows even if Linux is run ontop of it).
	if (msgidS2C == -1) {
		fprintf(stderr, "Server to client message queue failed to create. msgget failed with error %d\n", errno);
	}

	// Main routine.
	while (running) {
		// Clear the strucutres
		struct msg query = emptyMsg;
		struct response res = emptyRes;

		// Initialize the message type to a postive value.
		res.msg_type = 1;

		// Await a message.
		if (msgrcv(msgidC2S, (void *)&query, MSG_SIZE, msgToReceive, 0) == -1) {
			fprintf(stderr, "msgrcv failed with error: %d\n", errno);
			exit(EXIT_FAILURE);
		} 
		// Take the appropriate action based on the type of message received.
		switch (query.command) {
			case 0:
				// Shutdown request.
				running = 0;
				break;
			case 1:
				// Insert the employee into the array of employees, if there is space. 
				if (count < MAX_EMPLOYEES) {
					// Initialize a new node in the employee list
					struct msg *newEmployee = (struct msg*) malloc(sizeof(struct msg));
					
					// Copy the data from the received query into the new node.					
					memcpy(newEmployee->employeeName, query.employeeName, MAX_SIZE);
					memcpy(newEmployee->employeeDepartment, query.employeeDepartment, MAX_SIZE);
					newEmployee->employeeNumber = query.employeeNumber;
					newEmployee->employeeSalary = query.employeeSalary;
					newEmployee->next = NULL;
					count++;
					// Assign the newest node as the head and tail if the employee list was empty.
					if (head == NULL) {
						head = newEmployee;
						tail = newEmployee;
					} else {
						// Insert the new employee at the end of the list.
						tail->next = newEmployee;
						tail = newEmployee;
					}
					
					res.returnData = 0;
				} else {
					res.returnData = -1;
				}
				
				// Let the client know if the insertion was successful.
				if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}

				break;
			case 2:
				// check_employee_number
				// Can't check for employees if there are none stored.
				res.returnData = -1;
				if (count != 0) {
					// Used to traverse the employee list.
					struct msg *curr = head;
					// Search the employee list for the name of the employee
					while (curr != NULL) {
						if (strcmp(curr->employeeName, query.employeeName) == 0) {
							res.returnData = curr->employeeNumber;
							break;	
						}	
						curr = curr->next;
					}
				}
				
				// Send the information to the admin.
				if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}
				
				break;
			case 3:
				// check_employee_salary
				// Can't check for employees if there are none stored.
				res.returnData = -1;
				if (count != 0) {
					// Used to traverse the employee list.
					struct msg *curr = head;
					// Search the employee list for the employee number
					while (curr != NULL) {
						if (curr->employeeNumber == query.employeeNumber) {
							res.returnData = curr->employeeSalary;
							break;
						}
						curr = curr->next;
					}
				}

				// Send the information to the admin.
				if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}

				break;
			case 4:
				// check
				// Can't check for employees if there are none stored.
				if (count != 0) {
					// Used to traverse the employee list.
					struct msg *curr = head;
					// Search the employee list for the department name.
					while (curr != NULL) {
						res.returnData = -1;
						if (strcmp(curr->employeeDepartment, query.employeeDepartment) == 0) {
							res.returnData = curr->employeeNumber;
							// Send the information to the client.
							if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
								fprintf(stderr, "msgsnd failed\n");
								exit(EXIT_FAILURE);
							}
						}
						curr = curr->next;
					}
				} 

				res.returnData = -1;
				// Notify the client that we have finished searching the employee list.
				if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}
				
				break;
			case 5:
				// delete
				res.returnData = -1;
				// Can't remove employees if there are none.
				if (count != 0) { 
					// Used to traverse the employee list.
					struct msg *curr = head;
					// Check if the employee to be deleted is at the front of the list. 
					if (head->employeeNumber == query.employeeNumber) {
						if (head->next != NULL) {
							head = head->next;
						} else {
							head = NULL;
						}
						// Free the deleted node from memory.
						free(curr);
						count--;
						res.returnData = 0;
					} else {
						// Search the employee list for the employee number. 
						while (curr->next != NULL) {
							if (curr->next->employeeNumber == query.employeeNumber) {
								// Used to keep a reference to the node we are about to delete.
								struct msg *temp = curr->next;
								curr->next = curr->next->next;
								count--;
								res.returnData = 0;
								// Free the deleted node from memory.
								free(temp);
								break;
							}
							curr = curr->next;
						}
					}
				}
				// Notify the client that we have completed the deletion routine.
				if (msgsnd(msgidS2C, (void *)&res, sizeof(int), 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}

				break;

			default:
				printf("This is bad. This isn't supposed to happen.");
				exit(EXIT_FAILURE);
				break;	
		}
	}

	// Clean up message queues and finish execution.
	if ((msgctl(msgidC2S, IPC_RMID, 0) == -1) || (msgctl(msgidS2C, IPC_RMID, 0) == -1)) {
		fprintf(stderr, "Could not clean up message queues. Please run \"ipcrm --all=msg\" via the terminal.\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
	return 0;
}

