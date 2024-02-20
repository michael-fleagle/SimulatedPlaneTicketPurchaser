// Michael Fleagle
// CS470
// Code based on the demo code created by Dr. Szilard Vajda

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h> 
#include <math.h>

#include <pthread.h>
#include <vector>
#include <iostream>
#include <signal.h>

#define BUFFER_SIZE 1024
#define PORT_NUMBER 5437


using namespace std;

int ROWS;
int COLS;

int ** planeSeats;

// mutex lock vairables
pthread_mutex_t planeLock;

// structure to handle the seat data
typedef struct seat_
{
	int connection;
	pid_t pid;
} seat;


// structure to handle the socket information
typedef struct connectStruct_
{
	// int socketTemp;
	int listenTemp;
} connectStruct;


// method to print the current plane
void planePrint()
{
	for(int i = 0; i < ROWS; i++)
	{
		for(int j = 0; j < COLS; j++)
		{
			cout << planeSeats[i][j];
		}
		cout << endl;
	}
}


// method to determine if the plane is full
// return true if full
// return false if not full
bool planeFull()
{	
	for(int i = 0; i < ROWS; i++)
	{
		for(int j = 0; j < COLS; j++)
		{
			// if any seat is 0, the plane is not full
			if(planeSeats[i][j] == 0)
			{
				return false;
			}
		}
	}
	
	planePrint();
	
	// if none of the values are 0, return true that the plane is full
	return true;
}


// create thread function to handle connected clients
void * buyTickets(void * seatParam)
{
	cout << "Client Thread Started" << endl;
	// get the data from the passed data
	seat * planeInfo = (seat *)seatParam;
	
	int con = planeInfo->connection;
	pid_t clientID = planeInfo->pid;
	
	// create bool flag to check if the client is still connected
	bool requestConnection = true;
	
	while(requestConnection == true)
	{
		// create array of size two to hold the requested seat row, column
		int reqSeat[] = {0, 0};
		
		// create integer return value for client
		int clientReturn;
		
		// read value
		int n = 0;
		
		// check the seat array to see if the requested seat is available
		if(n = read(con, &reqSeat, sizeof(reqSeat)) > 0)
		{
			// tell the user that the client has requested a seat
			cout << "[Server]: Client " << clientID << " has requested seat " << reqSeat[0] << ", " << reqSeat[1] << endl;
			
			// lock the mutex
			pthread_mutex_lock(&planeLock);
			
			// check if that position is available in the plane
			if(planeSeats[reqSeat[0]][reqSeat[1]] == 0)
			{
				// update the plane seat
				planeSeats[reqSeat[0]][reqSeat[1]] = 1;
				
				planePrint();
				
				// set client return to 0
				clientReturn = 0;
				
				// tell the client that they got the seat
				write(con, &clientReturn, sizeof(int));
			}
			else
			{
				// check if the plane is full
				if(planeFull() == true)
				{
					// set client return to 2
					clientReturn = 2;
				
					// tell the client that the seat is not available and the plane is full
					write(con, &clientReturn, sizeof(int));
					
					// stop the connection with the client
					requestConnection = false;
				}
				else
				{
					// set client return to 1
					clientReturn = 1;
					
					// tell the client that the seat is not available
					write(con, &clientReturn, sizeof(int));
				}
			}
			
			// unlock the mutex
			pthread_mutex_unlock(&planeLock);
		}
		else
		{
			cout << "[Server]: Error reading from client " << clientID << endl;
		}
	}
	
	// tell the user that the plane is full
	cout <<"[Server]: The plane is full. " << endl << "Client " << clientID << " is disconnecting." << endl;
	
	// close socket connection?
	close(con);
	
	return (void *)0;
}


// thread function to handle accepting clients
void * acceptClients(void * param)
{
	// create connect struct to get the data from param
	connectStruct * clientConnect = (connectStruct *) param;
	
	// get the data
	int listenfd = clientConnect->listenTemp;
	
	// create connection fd
	int connfd = 0;
	
	// create vector for pthreads
	vector<pthread_t> clientThreads;
	
	// create vector for client ids
	vector<pid_t> clients;
	
	// create plane size array
	int planeSize[] = {ROWS, COLS};
	
    while(1)
    {
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL); 
		
		int clientID;
		
		// recieve the process id from the client to use as a unique identifier
		read(connfd, &clientID, sizeof(clientID));
		
		// create boolean flag to determine if the id is in the vector
		bool newClient = true;
		
		// check that the clientID is new
		for(int i = 0; i < clients.size(); i++)
		{
			// if the client id is already in the vector, then don't create a new thread for it
			if(clients.at(i) == clientID)
			{
				newClient = false;
				break;
			}
		}
		
		// check if the flag is true or false
		if(newClient == true)
		{
			// tell the user that a client connected
			cout << "[Server]: Client " << clientID << " has connected" << endl;
			
			// create seat object
			seat * seatStruct = (seat *)malloc(sizeof(seat));
			
			// seat connection
			seatStruct->connection = connfd;
			seatStruct->pid = clientID;
			
			// temp pthread variable
			pthread_t tempThread;
			
			// create a thread
			pthread_create(&tempThread, NULL, buyTickets, (void *)seatStruct);
			
			// pthread_join(tempThread, NULL);
			
			// push the thread into the thread vector
			clientThreads.push_back(tempThread);
			
			// push the client id onto the id vector
			clients.push_back(clientID);
			
			// tell the client the size of the plane
			write(connfd, planeSize, sizeof(planeSize));
		}
		
		sleep(1);
    }
	
	return (void *)0;
}


// method to check if arguments are valid
bool validArgs(int argc, char *argv[])
{
	if(argc != 3)
	{
		return false;
	}
	
	for(int i = 0; i < argc; i++)
	{
		if(atoi(argv[i]) < 1)
		{
			return false;
		}
	}
	
	return true;
}

int main(int argc, char *argv[])
{
	// check if the number of arguments is correct
	if(!validArgs)
	{
		printf("[Server]: Invalid Arguments. Usage: %s <rows> <cols> \n", argv[0]);
		return 1;
	}
	
	// assign the row and column given the global variables ROWS and COLS
	ROWS = atoi(argv[1]);
	COLS = atoi(argv[2]);
	
	// create a plane using the row and columns given by the user
	planeSeats = (int **) malloc(ROWS * sizeof(int*));
	
	for(int i = 0; i < ROWS; i++)
	{
		planeSeats[i] = (int *) malloc (COLS * sizeof(int));
	}
	
	// tell the user that the server is starting
	cout << "[Server]: Server is starting with a plane of size " << ROWS << " x " << COLS << endl;

	// create boolean flag to check if the server should end
	bool endServer = false;
	
    int listenfd = 0;
    struct sockaddr_in serv_addr;

    char sendBuff[BUFFER_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 
	
	// setting up the server socket
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    listen(listenfd, 10); 
	
	//===thread version of accepting connections===//
	
	// create connect struct to give to accept thread
	connectStruct * connection = (connectStruct *)malloc(sizeof(connectStruct));
	
	// connection->socketTemp = 
	connection->listenTemp = listenfd;
	
	pthread_t clientAcceptor;
	
	// create thread to handle accepting clients
	pthread_create(&clientAcceptor, NULL, acceptClients, (void*)connection);
	
	// pthread_join(clientAcceptor, NULL);
	
	
	// check if the plane is full
	while(1)
	{
		if(planeFull() == true)
		{
			cout << "[Server]: Server is closing due to plane being full" << endl;
	
			sleep(20);
			 
			 // deallocate alloacted memory
			for(int i = 0; i < ROWS; i++)
			{
				free(planeSeats[i]);
			}
			 
			free(planeSeats);
			planeSeats = nullptr;
			 
			free(connection);
			 
			// return default value
			return 0;
		}
		
		sleep(2);
	}
	
	// return 1 if the program somehow reaches here
	return 1;
}
