// Michael Fleagle
// CS470
// Code based on the demo code created by Dr. Szilard Vajda

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

#include <random>
#include <iostream>
#include <tuple>
#include <fstream>
#include <sstream>
#include <algorithm>

#define BUFFER_SIZE 1024
// #define PORT_NUMBER 5437

// namespace declaration
using namespace std;

// create global variables to handle IP, Port, and Timeout and set their default values
string iniIP = "127.0.0.1";
int iniPort = 5437;
int iniTimeout = 4;


// generate random number within provided max
int randomNum(int max)
{
	int num;
	
	// generate random number between 0-max
	num = rand() % max;
	
	return num;
}


// Method to handle automatic client connection to the server
int automatic(int clientID, int socketfd, int rows, int cols)
{
	int n = 0;
	
	// create variable to check if all the seats have been purchased
	bool planeFull = false;
	
	// create vector to hold all seats recieved
	vector<tuple<int, int>> recievedSeats;
	
	while(planeFull == false)
	{
		// get a random position to request
		int reqSeat[] = {0, 0};
		
		reqSeat[0] = randomNum(rows);
		reqSeat[1] = randomNum(cols);
		
		if(write(socketfd, reqSeat, sizeof(reqSeat)) < 0)
		{
			printf("Error: Unable to send request to server");
			
			// return non-zero value due to failure
			return 1;
		}
		
		// get a response from the server about the seat request
		int seatResp = -1;
		
		// read the response for a requested seat from the server
		// responses:
			// 1: seat not available
			// 2: seat not available and plane is full
		n = read(socketfd, &seatResp, sizeof(seatResp));
		
		// handle if the server does not respond
		if(n < 0)
		{
			cout << "[Client " << clientID << "]: Error: no response from server. Please check the server's status" << endl;
		}
		
		if(seatResp == 0)
		{
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is reserved! Thank you for choosing to fly with us!" << endl;
			tuple<int, int> recievedSeat;
			recievedSeat = make_tuple(reqSeat[0], reqSeat[1]);
			
			recievedSeats.push_back(recievedSeat);
		}
		else if(seatResp == 1)
		{
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is unavailable" << endl;
			
		}
		else if(seatResp == 2)
		{
			// tell the user that the seat requested is unavailable
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is unavailable and the plane is now full" << endl;
			
			// set the planeFull tag to be true
			planeFull = true;
		}
		
		// sleep to prevent an immediate re-request
		int sleepVal = randomNum(3);
		
		if(sleepVal == 1)
		{
			sleep(3);
		}
		else if(sleepVal == 2)
		{
			sleep(5);
		}
		else 
		{
			sleep(7);
		}
	}
	
	return 0;
}


// method to check whether the user input is valid 
bool validUserInput(int uRow, int row, int uCol, int col)
{
	if(uRow < row && uRow > -1 && uCol < col && uCol > -1)
	{
		return true;
	}
	
	return false;
}


// Method to handle manual client connection to the server
int manual(int clientID, int socketfd, int rows, int cols)
{
	// create integer value to handle read requests
	int n = 0;
	
	// create variable to check if all the seats have been purchased
	bool planeFull = false;
	
	// tell the user their row and size limitations
	printf("[Client %d]: The plane has %d rows and %d columns. \nThe rows have a range of [0 - %d] and the columns have a range of [0 - %d]\n", clientID, rows, cols, (rows - 1), (cols - 1));
	
	while(planeFull == false)
	{
		// get a random position to request
		int reqSeat[] = {0, 0};
		
		// flag to determine if the user input is valid
		bool validInput = false;
		
		while(validInput == false)
		{
			// ask the user for an input
			cout << "[Client " << clientID << "]: Please provide the row of the seat you wish to request: " << endl;
			cin >> reqSeat[0];
			
			cout << "[Client " << clientID << "]: Please provide the column of the seat you wish to request: " << endl;
			cin >> reqSeat[1];
			
			// check that the user input is valid
			if(!validUserInput(reqSeat[0], rows, reqSeat[1], cols))
			{
				cout << "[Client " << clientID << "]: Error: the row or column is outside the bounds of the plane." << endl;
			}
			else
			{
				validInput = true;
			}
		}
		
		// send the request to the server
		if(write(socketfd, reqSeat, sizeof(reqSeat)) < 0)
		{
			printf("Error: Unable to send request to server");
			
			// return non-zero value due to failure
			return 1;
		}
		
		// get a response from the server about the seat request
		int seatResp = -1;
		
		// read the response for a requested seat from the server
		// responses:
			// 1: seat not available
			// 2: seat not available and plane is full
		n = read(socketfd, &seatResp, sizeof(seatResp));
		
		// handle if the server does not respond
		if(n < 0)
		{
			cout << "[Client " << clientID << "]: Error: no response from server. Please check the server's status" << endl;
		}
		
		if(seatResp == 0)
		{
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is reserved! Thank you for choosing to fly with us!" << endl;
		}
		else if(seatResp == 1)
		{
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is unavailable" << endl;
			
		}
		else if(seatResp == 2)
		{
			// tell the user that the seat requested is unavailable
			cout << "[Client " << clientID << "]: Seat " << reqSeat[0] << ", " << reqSeat[1] << " is unavailable and the plane is now full" << endl;
			
			// set the planeFull tag to be true
			planeFull = true;
		}
		
		// sleep to prevent an immediate re-request
		int sleepVal = randomNum(3);
		
		if(sleepVal == 1)
		{
			sleep(3);
		}
		else if(sleepVal == 2)
		{
			sleep(5);
		}
		else 
		{
			sleep(7);
		}
	}
	
	return 0;
}

// method to handle the ini file if it is passed to the client
// INI format is:
	// [HEADER]
	// IP = 0.0.0.0
	// Port = 0000
	// Timeout = 0
int readIni(char * file)
{
	// Create fstream object
	fstream readFile;
	
	// open the given file
	readFile.open(file, ios::in);
	
	// check if the file exists
	if(readFile.is_open() == false)
	{
		return 3;
	}
	
	// the file exists and is open
	// read the information from the INI file
	vector<string> iniFile;
	string tempLine;
	
	// read from the file the information
	while(getline(readFile, tempLine))
	{	
		iniFile.push_back(tempLine);
	}
	
	readFile.close();
	
	// get the IP address from the data
	stringstream tokenIP(iniFile.at(1));
	string ipTemp;
	vector<string> ipTempVec;
	
	while(getline(tokenIP, ipTemp, ' '))
	{
		ipTempVec.push_back(ipTemp);
	}
	
	// separate the numerical values of the ip
	stringstream ipPieces(ipTempVec.at(2));
	string ipPieceTemp;
	vector<int> ipPiecesVec;
	
	while(getline(ipPieces, ipPieceTemp, '.'))
	{
		ipPiecesVec.push_back(stoi(ipPieceTemp.c_str()));
	}
	
	// recombine the vector pieces into a full ip address
	string ipAddr = to_string(ipPiecesVec.at(0));
	
	for(int i = 1; i < ipPiecesVec.size(); i++)
	{
		ipAddr.append(".");
		ipAddr.append(to_string(ipPiecesVec.at(i)));
	}
	
	iniIP = ipAddr;
	
	// get the port number from the data
	stringstream tokenPort(iniFile.at(2));
	string portTemp;
	vector<string> portTempVec;
	
	while(getline(tokenPort, portTemp, ' '))
	{
		portTempVec.push_back(portTemp);
	}
	
	iniPort = atoi(portTempVec.at(2).c_str());
	
	// get the timeout from the data
	stringstream tokenTime(iniFile.at(3));
	string timeTemp;
	vector<string> timeTempVec;
	
	while(getline(tokenTime, timeTemp, ' '))
	{
		timeTempVec.push_back(timeTemp);
	}
	
	iniTimeout = atoi(timeTempVec.at(2).c_str());
	
	return 0;
}

// method to check if the inputs from the user are valid
// Input: int argc, cahr *argv[]
// Output: int
	// 1 = manual mode called
	// 2 = automatic mode called
	// -1 = invalid arguments
int isValid(int argc, char *argv[])
{
	if(argc < 2 || argc > 3)
	{
		printf(" Incorrect amount of arguments \n Usage: %s <mode> [<ip of server>] \n",argv[0]);
        return -1;
	}
	else
	{
		// check if an ini was passed
		if(argc == 3)
		{
			cout << "[Client]: Checking the INI file passed" << endl;
			if(readIni(argv[2]) == 3)
			{
				return 3;
			}
		}
		
		// check which mode to run the client in
		if(strcmp(argv[1], "Manual") == 0)
		{
			return 1;
		}
		else if(strcmp(argv[1], "Automatic") == 0)
		{
			return 2;
		}
		else
		{
			printf(" Mode %s not recognized\n Usage: %s <mode> [<ip of server>] \n Proper modes are:\n\tAutomatic\n\tManual \n", argv[1], argv[0]);
			return -1;
		}
	}
}


int main(int argc, char *argv[])
{
    int sockfd = 0, n = 0;
    char recvBuff[BUFFER_SIZE];
    struct sockaddr_in serv_addr; 
	
	int runningMode = isValid(argc, argv);
	
	// if arguments are invalid
	if(runningMode == -1)
	{
		return 1;
	}
	else if(runningMode == 3)
	{
		cout << "[Client]: Error: Given INI file not found." << endl;
		return 1;
	}

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(iniPort); 
	
    if(inet_pton(AF_INET, iniIP.c_str(), &serv_addr.sin_addr)<=0)
    {
        printf(" inet_pton error occured\n");
        return 1;
    } 
	
	// get the process id
	pid_t clientID = getpid();
	
	// Attempt to connect to the server at the discresion of the INI file or the default values
	for(int i = 0; i < iniTimeout; i++)
	{
		if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
		   printf("\n Error : Connect Failed \n");
		   
		   // tell the user when the max time out has been reached 
		   if(i == iniTimeout - 1)
		   {
				cout << iniTimeout << " Attempts were made to connect and all were unsuccessful. Please check the server status." << endl;
				
				return 1;
		   }
		   
		   cout << "[Client]: Retrying connection in 2 seconds..." << endl;
		   
		   sleep(2);
		}
		else
		{
			// tell the user that we are sending our identifier
			cout << "[Client " << clientID << "]: Connection established with server." << endl;
			
			break;
		}
	}

	// seed random number generator
	srand(time(0));
	
	// send the process id to the server to be used as a unique identifier
	write(sockfd, &clientID, sizeof(clientID));
	
	// create an array to hold the size of the plane (rows, columns)
	int planeSize[] = {0, 0};
	
	// read plane size from server 
	n = read(sockfd, &planeSize, sizeof(planeSize)-1);
	
	if (n < 0)
	{
		cout << "\nError: Unable to read from server \n";
	}
	
	cout << "[Client " << clientID << "]: The size of the plane is (" << planeSize[0] << " X " << planeSize[1] << ") \n";

	// if running in manual mode
	if(runningMode == 1)
	{
		// tell the user that auto mode is starting
		cout << "[Client " << clientID << "]: Manual mode is starting." << endl;
		
		int manRet = manual(clientID, sockfd, planeSize[0], planeSize[1]);
		
		// if the manual function fails (returns something other than 0)
		if(manRet != 0)
		{
			printf("Error: Manual mode failed");
		}
		
		// return the returned value from manual function
		return manRet;
	}

	// if running in automatic mode
	if(runningMode == 2)
	{
		// tell the user that auto mode is starting
		cout << "[Client " << clientID << "]: Automatic mode is starting." << endl;
		
		int autoRet = automatic(clientID, sockfd, planeSize[0], planeSize[1]);
		
		// if the automatic function fails (returns something other than 0)
		if(autoRet != 0)
		{
			printf("Error: Manual mode failed");
		}
		
		// return the returned value from automatic function
		return autoRet;
	}

    close(sockfd);
    return 0;
}
