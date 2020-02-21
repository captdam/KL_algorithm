#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 255

#ifndef DEBUG_PARSER
#define DEBUG_PARSER 1
#endif


// Struct of net, includes the cost of the net, and name of all nodes connected to this edge
struct Net {
	double cost;
	unsigned long int nodeCount;
	char** connectedNode;
};


// Return name length ('\0' not included) if the given string (a line from node file) represents a node
// Otherwise, return 0
unsigned long int checkNode(char* nodename) {
	if (nodename[0] != 'a' && nodename[0] != 'p') //Node strats with 'a' or 'p'
		return 0;

	uint8_t ok = 1;
	unsigned long int i;
	for (i = 1; i < BUFFER_SIZE; i++) { //Check all chars in the line
		if (nodename[i] == '\n' || nodename[i] == '\r' || nodename[i] == ' ') //Until break found
			break;
		if (nodename[i] < '0' || nodename[i] > '9') { //After the 'a' or 'p' char, there should be numbers only
			ok = 0;
			break;
		}
	}

	return ok ? i : 0;
}


// Read the node file
// Return an array of String (0-terminated char array) of node names by reference, return number of nodes
unsigned long int readNodefile(char* filename, char** *nodelist) {

	//Open the node file
	char* fullFilename = (char*) malloc(strlen(filename)+6+1); //"param" + 6 ".nodes" + 1 "\0"
	strcpy(fullFilename,filename);
	strcat(fullFilename,".nodes");
	printf("--> Reading node file: %s\n",fullFilename);
	FILE* fp = fopen(fullFilename,"r");

	unsigned long int nodeCount = (unsigned long int) -1; //-1 means uninit
	unsigned long int nodeIndex = 0;

	//Read all lines in the node file
	char buffer[BUFFER_SIZE];
	while ( fgets(buffer,sizeof buffer, fp) != NULL) {
		unsigned long int nodenameLength;

#if(DEBUG_PARSER)
		printf("----> Read: %s",buffer);
#endif

		//Get number of nodes
		if (sscanf(buffer," NumNodes : %lu ",&nodeCount) > 0) {
#if(DEBUG_PARSER)
			printf("------> There are %lu nodes.\n",nodeCount);
#endif
			*nodelist = (char**) malloc( sizeof(char*) * nodeCount );
		}

		//Get a line indicates a node
		else if ( nodenameLength = checkNode(buffer) ) {

#if(DEBUG_PARSER)
			printf("------> New node found.");
#endif

			//We have to know how much nodes are here; otherwise, we are not able to add nodes into our array
			if ( nodeCount == (unsigned long int) -1 ) {
				printf("\n[ERROR] Parser error. Unknown nodelist size.\n");
				exit(0);
			}

			//Or, we find more nodes than what the file tells us at the beginning
			if ( nodeIndex == nodeCount ) {
				printf("\n[ERROR] Parser error. New node out of nodelist size.\n");
				exit(0);
			}

			//Push nodename into node array
			(*nodelist)[nodeIndex] = (char*) malloc(nodenameLength+1); //'\0' takes extra one byte
			for (unsigned long int i = 0; i < nodenameLength; i++)
				(*nodelist)[nodeIndex][i] = buffer[i];
			(*nodelist)[nodeIndex][nodenameLength] = '\0';
#if(DEBUG_PARSER)
			printf(" Node name = \"%s\" [%lu].\n",(*nodelist)[nodeIndex],nodeIndex);
#endif
			nodeIndex++;
		}

		//Get meanless line
		else {
#if(DEBUG_PARSER)
			printf("------> DNC.\n");
#endif
		}
	}
	
	//Done with the file
	fclose(fp);
	free(fullFilename);
	return nodeCount;
}


// A linkedlist used by netfile parser
struct ParserNetlist {
	struct Net* net;
	struct ParserNetlist* next;
};

// Read the net file
// Return an array of struct Net, return number of nets
unsigned long int readNetfile(char* filename, struct Net** *netlist) {

	/* NOTE:
	Linkedlist vs realloc array:
	Number of nets are unknown. To save all of them, linkedlist and array can be used.
	1 - Array: Read an element, push array. If more element, realloc the array size, push the new element.
	2 - Linked list: Read an element, save it in struct. If more element, alloc a new struct and link the new struct to old struct's next ptr.
	Realloc array is easy to write; however, the entire memory has to be copy to new location if overlap found (current segment is not big enough), which is an overhead.
	The parser will first read elements into linked list.
	After reading all elements, reformat the linked list into array for easy implementation for parent function.
	*/

	//Open the node file
	char* fullFilename = (char*) malloc(strlen(filename)+5+1); //"param" + 5 ".nets" + 1 "\0"
	strcpy(fullFilename,filename);
	strcat(fullFilename,".nets");
	printf("--> Reading net file: %s\n",fullFilename);
	FILE* fp = fopen(fullFilename,"r");

	//Read all lines in the node file
	char buffer[BUFFER_SIZE];
	struct ParserNetlist* firstNet = NULL;
	struct ParserNetlist** currentNetPtr = &firstNet;
	
	unsigned long int currentNetNodesCount = 0;
	while ( fgets(buffer,sizeof buffer, fp) != NULL ) {

#if(DEBUG_PARSER)
		printf("----> Read: %s",buffer);
#endif		

		//Case 1 - Want to find new edge
		if (!currentNetNodesCount) {
			//Check the current line: Says "NetDegree : nnn"?
			if (sscanf(buffer," NetDegree : %lu ",&currentNetNodesCount) > 0) { //Return number of found variable, 0 if no matched, -1 if no input
#if(DEBUG_PARSER)
				printf("------> New edge with %lu nodes connected.\n",currentNetNodesCount);
#endif
				*currentNetPtr = (struct ParserNetlist*) malloc(sizeof(struct ParserNetlist));
				(*currentNetPtr)->net = (struct Net*) malloc(sizeof(struct Net));

				((*currentNetPtr)->net)->cost = 1.0;
				((*currentNetPtr)->net)->nodeCount = currentNetNodesCount;
				((*currentNetPtr)->net)->connectedNode = malloc( sizeof(char*) * currentNetNodesCount );
				continue;
			}

			//Else, got to next line
			else
				continue;
		}

		//Case 2 - Finding nodes belongs to that edge
		else {
			//Check the current line: Is it a node? If not, go to next line
			unsigned long int nodenameLength = checkNode(buffer);
			if (!nodenameLength)
				continue;

			//Copy the nodename from file line buffer into nodename buffer, and append '\0' terminator
			char* nodename = (char*) malloc(nodenameLength+1); //'\0' takes extra one byte
			unsigned long int i;
			for (i = 0; i < nodenameLength; i++)
				nodename[i] = buffer[i];
			nodename[i] = '\0';

			//Save this nodename
#if(DEBUG_PARSER)
			printf("------> Find new node \"%s\" that is connected to this edge.\n",nodename);
#endif
			((*currentNetPtr)->net->connectedNode)[--currentNetNodesCount] = nodename;

			//Found all nodes, linkedlist pointer moves
			if (!currentNetNodesCount) {
#if(DEBUG_PARSER)
				printf("------> All nodes connected to this edge are found:\n");
				for (unsigned long int i = 0; i < ((*currentNetPtr)->net->nodeCount); i++)
					printf("--------> %s\n",((*currentNetPtr)->net->connectedNode)[i]);
#endif
				currentNetPtr = &((*currentNetPtr)->next);
				*currentNetPtr = NULL;
			}
		}
	}
	
	//Done with the file
	fclose(fp);
	free(fullFilename);

	//Calculate the amount of nets and alloc memory for netlist
	unsigned long int netCount = 0;
	currentNetPtr = &firstNet;
	while (*currentNetPtr != NULL) {
		netCount++;
		currentNetPtr = &((*currentNetPtr)->next);
	}
#if(DEBUG_PARSER)
	printf("--> %lu nets are found.\n",netCount);
#endif
	*netlist = (struct Net**) malloc( sizeof(struct Net**) * netCount );

	//Reformat the nodelist linkedlist into array
	currentNetPtr = &firstNet;
	for (unsigned long int i = 0; i < netCount; i++) {
		(*netlist)[i] = (*currentNetPtr)->net;
		struct ParserNetlist** justReadNetPtr = currentNetPtr;
		currentNetPtr = &((*currentNetPtr)->next);
		free(*justReadNetPtr); //Free the entire linkedlist recursively
	}

	return netCount;
}

