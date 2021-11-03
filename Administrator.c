#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include <sys/msg.h>

#define BUFFER_SIZE 512
#define MAX_SIZE 12
#define MSG_SIZE (2 * MAX_SIZE + (3 * sizeof(int)))

/**
 * SYSC 4001 - Assignment 3
 * Enterprise Record Keeping System
 * Administrator (client) program
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
};

struct response {
        long int msg_type;
	int returnData;
};

// Function prototypes
void insert(int, int);
void check_employee_number(int, int);
void check_salary(int, int);
void check(int, int);
void delete(int, int);
void quit(int);
int checkCommand(char[]);

/**
 * Entry-point for the application.
 */
int main() {
        int running = 1;
        char buffer[BUFFER_SIZE];

        // Create a message queue that will be used to send data to the server.
        int msgidC2S = msgget((key_t)1224, 0666 | IPC_CREAT | MSG_NOERROR);

        // Ensure the message queue was successfully created (will fail on Windows even if Linux is run ontop of it).
        if (msgidC2S == -1) {
                fprintf(stderr, "Client to server message queue failed to create. msgget failed with error: %d\n", errno);
        }

        // Create a message queue that will be used to receive data from the server.
        int msgidS2C = msgget((key_t)1234, 0666 | IPC_CREAT | MSG_NOERROR);

        // Ensure the message queue was successfully created (will fail on Windows even if Linux is run ontop of it).
        if (msgidS2C == -1) {
                fprintf(stderr, "Server to client message queue failed to create. msgget failed with error: %d\n", errno);
        }
	
	// Main routine.
        while (running) {
                printf("Please enter a command (insert, check_employee_number, check_salary, check, delete, quit): ");
                fgets(buffer, BUFFER_SIZE, stdin);
                int command = checkCommand(buffer);
		// Take appropriate action based on the command that was just entered.
                switch (command) {
                        case 0:
                                running = 0;
				quit(msgidC2S);
		      	        break;
                        case 1:
                                insert(msgidC2S, msgidS2C);
                                break;
                        case 2:
                                check_employee_number(msgidC2S, msgidS2C);
                                break;
                        case 3:
                                check_salary(msgidC2S, msgidS2C);
                                break;
                        case 4:
                                check(msgidC2S, msgidS2C);
                                break;
                        case 5:
                                delete(msgidC2S, msgidS2C);
                                break;
                        default:
                                printf("Invalid command, please try again. Commands are case-sensitive.\n");
                                break;
                }
        }
		
	printf("Thank you for using the record-keeping system! Exiting...\n");
        exit(EXIT_SUCCESS);

        return 0;
}

/**
 * Inserts a new employee into the server. Some input checking exists, but it
 * is by no means robust. The system will request an employee name, number, department,
 * and salary. This information is formed into a structure and subsequently passed to
 * the server. The server will return whether the operation was successful, and this
 * will be printed to the console on the client side.
 */
void insert(int msgidC2S, int msgidS2C) {
        struct msg newEmployee;
	struct response inserted;
        char name[MAX_SIZE];
        char departmentName[MAX_SIZE];
        char employeeNumber[MAX_SIZE];
	int empNum;
        char salary[MAX_SIZE];
	int sal;
	long int msgToReceive = 0;
	newEmployee.msg_type = 1;
	newEmployee.command = 1;

        printf("All information provided must be less than %d characters in length. Additional input will be truncated.\n", MAX_SIZE);
        // Get the employee's name.
	while (1) {
        	printf("Please enter the employee's name: ");
        	fgets(name, MAX_SIZE, stdin);
        	if (!isdigit(name[0])) {
			strcpy(newEmployee.employeeName, name);
			break;
		} else {
			printf("Error, an employee cannot have a number as a name. They are people too.\n");
		}
	}
        // Get the employee's department.
        while (1) {	
		printf("Please enter the employee's department name: ");
        	fgets(departmentName, MAX_SIZE, stdin);
        	if (!isdigit(departmentName[0])) {
			strcpy(newEmployee.employeeDepartment, departmentName);
			break;
		} else {
			printf("Error, a department cannot be a number.\n");
		}
	}
        // Get the employee's identification number.
        while (1) {
		printf("Please enter the employee's identification number: ");
        	fgets(employeeNumber, MAX_SIZE, stdin);
        	empNum = atoi(employeeNumber);
		if (empNum <= 0) {
			printf("Error, an employee identification number must a number greater than 0.\n");
		} else {
			newEmployee.employeeNumber = empNum;
			break;
		}
	}
        // Get the employee's salary.
        while (1) {
		printf("Please enter the employee's salary (Numbers only. Ex: 10000): ");
        	fgets(salary, MAX_SIZE, stdin);
        	sal = atoi(salary);
		if (sal <= 0) {
			printf("Error, an employee salary must be greater than 0. They need to eat.\n");
		} else {
			newEmployee.employeeSalary = sal;
			break;
		}
	}
        // Attempt to send the newly created employee info to the server.
        if (msgsnd(msgidC2S, (void *)&newEmployee, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "msgsnd failed\n");
                exit(EXIT_FAILURE);
        }

	// Wait for a response from the server.
	if (msgrcv(msgidS2C, (void *)&inserted, sizeof(int), msgToReceive, 0) == -1) {
		fprintf(stderr, "msgrcv failed\n");
		exit(EXIT_FAILURE);
	}
	
	// Check to see if the insertion was successful.
	if (inserted.returnData == -1) {
		printf("Error: Insertion could not be performed because the employee list is full.\n");
	} else {
		printf("The employee was successfully inserted.\n");
	}
}

/**
 * Checks for the employee number of a supplied employee name. There is
 * some input checking, but it is by no means robust. Once a name has been
 * supplied, the client awaits a response from the server. If a number was found,
 * it is printed to the console. If no number was found for the supplied name, this
 * is printed to the console.
 */
void check_employee_number(int msgidC2S, int msgidS2C) {
        struct msg checkEmployeeNumber;
        struct response number;
        char name[MAX_SIZE];
        long int msgToReceive = 0;
	checkEmployeeNumber.msg_type = 1;
        checkEmployeeNumber.command = 2;

        printf("All information provided must be less than %d characters in length. Additional input will be truncated.\n", MAX_SIZE);
        // Get the employee's name that we want to find the employee number.
        while (1) {
		printf("Please enter the employee's name: ");
        	fgets(name, MAX_SIZE, stdin);
		if (!isdigit(name[0])) {
        		strcpy(checkEmployeeNumber.employeeName, name);
			break;
		} else {
			printf("Error, an employee cannot have a number as a name. Please try again.\n");
		}
	}
        // Attempt to send the info to the server.
        if (msgsnd(msgidC2S, (void *)&checkEmployeeNumber, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "msgsnd failed\n");
                exit(EXIT_FAILURE);
        }

        // Wait for a response from the server.
        if (msgrcv(msgidS2C, (void *)&number, sizeof(int), msgToReceive, 0) == -1) {
                fprintf(stderr, "msgrcv failed\n");
                exit(EXIT_FAILURE);
        }
	// Remove the newline character from the returned data.
	strtok(name, "\n");
	// Check to see if an employee was found.
        if (number.returnData == -1) {
                printf("No employee was found with that name. Names are case-sensitive.\n");
        } else {
                printf("The employee number for %s is %d.\n", name, number.returnData);
        }
}

/**
 *
 */
void check_salary(int msgidC2S, int msgidS2C) {
        struct msg checkEmployeeSalary;
        struct response salary;
        char number[MAX_SIZE];
	int num;
        long int msgToReceive = 0;
	checkEmployeeSalary.msg_type = 1;
        checkEmployeeSalary.command = 3;

        printf("All information provided must be less than %d characters in length. Additional input will be truncated.\n", MAX_SIZE);
        // Get the employee number that corresponds to the salary we want to check.
  	while (1) {	
  		printf("Please enter an employee number: ");
        	fgets(number, MAX_SIZE, stdin);
        	num = atoi(number);
		if (num <= 0) {
			printf("Error, invalid input detected. Employee numbers must be numbers and must be greater than 0.\n");
		} else {
			checkEmployeeSalary.employeeNumber = num;
			break;	
		}
	}
        // Attempt to send the info to the server.
        if (msgsnd(msgidC2S, (void *)&checkEmployeeSalary, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "msgsnd failed\n");
                exit(EXIT_FAILURE);
        }

        // Wait for a response from the server.
        if (msgrcv(msgidS2C, (void *)&salary, sizeof(int), msgToReceive, 0) == -1) {
                fprintf(stderr, "msgrcv failed\n");
                exit(EXIT_FAILURE);
        }
	
	// Remove the newline character from the input.
	strtok(number, "\n");
	
	// Check to see that the query was successful.
        if (salary.returnData == -1) {
                printf("No employee was found with that id number. Please try again.\n");
        } else {
                printf("The salary of the employee with id number %s is %d.\n", number, salary.returnData);
        }

}

/**
 *
 */
void check(int msgidC2S, int msgidS2C) {
	struct msg checkDepartment;
	struct response empNums;
	struct response empty;	
	char department[MAX_SIZE];
	long int msgToReceive = 0;
	int count = 0;
	checkDepartment.msg_type = 1;
	checkDepartment.command = 4;	

	printf("All information provided must be less than %d characters in length. Additional input will be truncated.\n", MAX_SIZE);
	// Get the department name that corresponds to the check.
	while (1) {
		printf("Please enter a department name: ");
		fgets(department, MAX_SIZE, stdin);
		if (!isdigit(department[0])) {
			strcpy(checkDepartment.employeeDepartment, department);
			break;
		} else {
			("Error, invalid input detected. A department cannot be composed exclusively of numbers. Please try again.\n");
		}
	}

	// Attempt to send the info to the server.
	if (msgsnd(msgidC2S, (void *)&checkDepartment, MSG_SIZE, 0) == -1) {
		fprintf(stderr, "msgsnd failed\n");
	        exit(EXIT_FAILURE);
	}

	// Remove the newline character from the input.
	strtok(department, "\n"); 
	
	//Wait until the server indicates there are no more employee numbers for the department.
        while (1) {
		if (msgrcv(msgidS2C, (void *)&empNums, sizeof(int), msgToReceive, 0) == -1) {
         		fprintf(stderr, "msgrcv failed\n");
			exit(EXIT_FAILURE); 
		}
		if (empNums.returnData == -1) {
			break;
		} else {
			if (count == 0) {
				printf("The employee numbers for the %s department are:\n", department);
			}
			printf("%d\n", empNums.returnData);
			empNums = empty;
			count++;			
		}	
	} 

	if (count == 0) {
		printf("No employee numbers for found to belong to the %s department.\n", department);
	}
}

/**
 *
 */
void delete(int msgidC2S, int msgidS2C) {
	struct msg deletion;
	struct response success;
	char number[MAX_SIZE];
	long int msgToReceive = 0;
	int num;
	deletion.msg_type = 1;
	deletion.command = 5;
	
	printf("All information provided must be less than %d characters in length. Additional input will be truncated.\n", MAX_SIZE);
        // Get the employee number that corresponds to the employee we want to delete.
  	while (1) {	
  		printf("Please enter an employee number: ");
        	fgets(number, MAX_SIZE, stdin);
        	num = atoi(number);
		if (num <= 0) {
			printf("Error, invalid input detected. Employee numbers must be numbers and must be greater than 0.\n");
		} else {
			deletion.employeeNumber = num;
			break;	
		}
	}
        // Attempt to send the info to the server.
        if (msgsnd(msgidC2S, (void *)&deletion, MSG_SIZE, 0) == -1) {
                fprintf(stderr, "msgsnd failed\n");
                exit(EXIT_FAILURE);
        }

        // Wait for a response from the server.
        if (msgrcv(msgidS2C, (void *)&success, sizeof(int), msgToReceive, 0) == -1) {
                fprintf(stderr, "msgrcv failed\n");
                exit(EXIT_FAILURE);
        }
	
	// Remove the newline character from the input.
	strtok(number, "\n");
	
	// Check to see that the query was successful.
        if (success.returnData == -1) {
                printf("No employee was found with that id number. Please try again (server returned %d).\n", success.returnData);
        } else {
                printf("The employee with the identification number of %s is was successfully deleted (server returned %d).\n", number, success.returnData);
        }

}

void quit(int msgidC2S) {
	char d;
        char buffer[BUFFER_SIZE];
        while(1) {
           	printf("Would you like to shutdown the record-keeping server as well (Y/N)?\n");
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                	d = buffer[0];
                        buffer[strcspn(buffer, "\n")] = 0;
                        // Ensure that just a single char was entered, nothing more.
                        if (d == 'Y' && !buffer[1]) {
                        	struct msg shutdown;
				shutdown.msg_type = 1;	
				shutdown.command = 0;
				// Attempt to send the shutdown request to the server.
				if (msgsnd(msgidC2S, (void *)&shutdown, MSG_SIZE, 0) == -1) {
					fprintf(stderr, "msgsnd failed\n");
					exit(EXIT_FAILURE);
				}							
				break;
			} else if (d == 'N' && !buffer[1]) {
                               	break;
			} else {
                               	printf("Invalid input detected, please enter Y or N.\n");
			}
                } else {
                	printf("Invalid input detected, please enter Y or N.\n");
		}
	}
}

/**
 * Returns a corresponding integer to represent the command
 * that was just entered. If the input is not recognized, -1
 * is returned instead.
 */
int checkCommand(char buffer[]) {
        // Define commands to be checked against (include new line character because fgets() includes it).
        char insert[BUFFER_SIZE] = "insert\n";
        char check_employee_number[BUFFER_SIZE] = "check_employee_number\n";
        char check_salary[BUFFER_SIZE] = "check_salary\n";
        char check[BUFFER_SIZE] = "check\n";
        char delete[BUFFER_SIZE] = "delete\n";
        char quit[BUFFER_SIZE] = "quit\n";

        // Determine the command that was enterred
        if (strcmp(buffer, "quit\n") == 0) {
                return 0;
        } else if (strcmp(buffer, "insert\n") == 0) {
                return 1;
        } else if (strcmp(buffer, "check_employee_number\n") == 0) {
                return 2;
        } else if (strcmp(buffer, "check_salary\n") == 0) {
                return 3;
        } else if (strcmp(buffer, "check\n") == 0) {
                return 4;
        } else if (strcmp(buffer, "delete\n") == 0) {
                return 5;
        } else {
                return -1;
        }
}


