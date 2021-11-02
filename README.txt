## Enterprise Record Keeping System

This was an assignment I implemented for the SYSC 4001 Operating Systems course at Carleton University. 
The purpose of the assingnment was on inter-process communication using message queues.
The implementation follows a client-server paradigm, with the Adminstrator acting as the client and the Record Keeper acting as the server.

### Setup
In order to compile the program:
- Clone the repo to a directory on your local Linux machine.
- Access that directory using the terminal and then type "make" (without the quotation marks). 
This should compile the programs. To execute the programs:
- type "./client" into one terminal and "./server" into a different terminal (without the quotation
  marks).
- Interaction occurs via the ./client program, so all input should be entered there. Note that
  the server will not display output unless an error occurs. 
  However, to test that it is actually doing its job, I would suggest
  stopping the execution of the ./client program after a few insertions and then rerunning it.
  This will show that the data is still available from the server for retrieval, and it isn't
  just being stored in the Administrator.c program. 