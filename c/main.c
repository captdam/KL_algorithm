#define MAX_TRY_TIME 3

#define DEBUG 1
#define DEBUG_PARSER 0
#define DEBUG_KLUTIL 0
#define DEBUG_FINDLOCALMAX 0

#define SAVE_INTERRESULE 1

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "parser.c"
#include "klutil.c"

int main(int argc, char* argv[]) {

/*******************************************************************************
1 - Get user input: What file I am gonna to process
*******************************************************************************/

	if (argc < 2) {
		printf("Missing argument.\n");
		return -1;
	}

/*******************************************************************************
2 - Read the nodes and nets file, saving in nodelist and netlist
*******************************************************************************/

	printf("Reading nodesfile and netsfile: %s\n",argv[1]);

	char** nodelist;
	unsigned long int nodeCount = readNodefile(argv[1],&nodelist);
#if(DEBUG)
	printf("> Found following nodes:\n");
	for (unsigned long int i = 0; i < nodeCount; i++)
		printf("> Found node %s.\n",nodelist[i]);
#endif

	struct Net** netlist;
	unsigned long int netCount = readNetfile(argv[1],&netlist);
#if(DEBUG)
	printf("> Found following nets:\n");
	for (unsigned long int i = 0; i < netCount; i++) {
		printf("> Found net. With nodes: ");
		struct Net* currentNet = netlist[i];
		for (unsigned long int j = 0; j < currentNet->nodeCount; j++)
			printf("%s ",currentNet->connectedNode[j]);
		printf("\n");
	}
#endif

	/* NOTE:
	There are (unsigned long int)nodeCount of nodenames saved in (char**)nodelist
		- char** nodelist:
			- char* nodename;
		Using nodelist to create array_pair with other array to create Object
	There are (unsigned long int)netCount of nets saved in (Net** netlist)
		- Net** netlist:
			- Net* net:
				- double cost;
				- unsigned long int nodeCount;
				- char** connectedNode:
					- char* nodename;
	*/


/*******************************************************************************
3 - Find the cost between all node pair, saving in costAB
*******************************************************************************/

	//For each node pair, get the sum of (cost of that node pair in an edge)
	printf("Calculating cost between nodes.\n");
	double costAB[nodeCount][nodeCount];

	for (unsigned long int i = 0; i < nodeCount; i++) { //By default, there is no cost (not inter-connected)
		for (unsigned long int j = 0; j < nodeCount; j++) {
			costAB[i][j] = 0;
		}
	}

	for (unsigned long int i = 0; i < netCount; i++) {
		struct Net* currentNet = netlist[i];
		double costForEachNode = currentNet->cost / (currentNet->nodeCount-1);
#if(DEBUG)
		printf("> Net with cost of %.3f for each connected nodes: ",costForEachNode);
		for (unsigned long int j = 0; j < currentNet->nodeCount; j++) {
			printf("%s ",currentNet->connectedNode[j]);
		}
		printf("\n");
#endif
		for (unsigned long int j = 0; j < currentNet->nodeCount - 1; j++) { //Add up current edge's cost to the nodes
			for (unsigned long int k = j + 1; k < currentNet->nodeCount; k++) {
				char* nodenameA = currentNet->connectedNode[j];
				unsigned long int nodeAIndex = getNodeIndex(nodenameA,nodelist,nodeCount);
				char* nodenameB = currentNet->connectedNode[k];
				unsigned long int nodeBIndex = getNodeIndex(nodenameB,nodelist,nodeCount);
				costAB[nodeAIndex][nodeBIndex] += costForEachNode;
				costAB[nodeBIndex][nodeAIndex] += costForEachNode;
			}
		}
	}

#if(SAVE_INTERRESULE)
	printf("Saving cost table in CSV file for reference.\n");
	printf("--> Note: Accuracy will be lost due to number representation.\n");

	char* fullFilename = (char*) malloc(strlen(argv[1])+9+1); //"param" + 9 ".cost.csv" + 1 "\0"
	strcpy(fullFilename,argv[1]);
	strcat(fullFilename,".cost.csv");
	FILE* fp = fopen("cost.csv","w");

	for (unsigned long int i = 0; i < nodeCount; i++) { //Header
		char sep = (i == nodeCount - 1) ? '\n' : ',';
		fputs(nodelist[i],fp);
		fputc(sep,fp);
	}

	for (unsigned long int j = 0; j < nodeCount; j++) {
		for (unsigned long int k = 0; k < nodeCount; k++) {
			char sep = (k == nodeCount - 1) ? '\n' : ',';
			fprintf(fp,"%f",costAB[j][k]);
			fputc(sep,fp);
		}
	}

	fclose(fp);
	free(fullFilename);
#endif

/*******************************************************************************
4 - Create the init partitionning: Design 0
*******************************************************************************/
	printf("\nNew design 0 (Init):\n");
	char nodeGroup[nodeCount]; //'A' or 'B'
	struct NodeInfo nodeInfo[nodeCount];

	//Partition
	for (unsigned long int i = 0; i < nodeCount; i++)
		nodeGroup[i] = 'A' + (i&1); //A+0=A, A+1=B

	printf("> Group A:\n");
	for (unsigned long int i = 0; i < nodeCount; i++) {
		if (nodeGroup[i] == 'A')
			printf("--> %s\n",nodelist[i]);
	}
	printf("> Group B:\n");
	for (unsigned long int i = 0; i < nodeCount; i++) {
		if (nodeGroup[i] == 'B')
			printf("--> %s\n",nodelist[i]);
	}

	//Find DIE value
	double totalE = 0, totalI = 0;
	for (unsigned long int i = 0; i < nodeCount; i++) {
		nodeInfo[i] = findNodeInfo(i,nodeCount,nodeGroup,costAB);
		totalE += nodeInfo[i].e;
		totalI += nodeInfo[i].i;
	}
	
	printf("> Total E cost = %.3f, total I cost = %.3f.\n",totalE, totalI);
#if(DEBUG)
	for (unsigned long int i = 0; i < nodeCount; i++) {
		printf("--> Node %5s (%5lu) G-%c \t D = %.3f, I = %.3f, E = %.3f.\n",nodelist[i],i,nodeGroup[i],nodeInfo[i].d,nodeInfo[i].i,nodeInfo[i].e);
	}
#endif
	
#if(SAVE_INTERRESULE)
	
#endif

/*******************************************************************************
5 - KL greedy algorithm loop
*******************************************************************************/

	double klGain;
	unsigned long int workloop = 1;
	unsigned long int pairCount = nodeCount >> 1; //floor(nodeCount/2) --> deals with uneven group
	unsigned long int sizeGroupA = pairCount + nodeCount&1, sizeGroupB = pairCount;

	do {
		printf("\nNew design %lu:\n",workloop);

		//Create a new design, find the DIE value
		char klNodeGroup[nodeCount];
		char klBestNodeGroup[nodeCount];
		uint8_t klLocked[nodeCount];
		struct NodeInfo klNodeInfo[nodeCount];

		for (unsigned long int i = 0; i < nodeCount; i++) {
			klNodeGroup[i] = nodeGroup[i]; //Deep copy the current partitioning
			klBestNodeGroup[i] = nodeGroup[i];
			klLocked[i] = 0; //Unlock all nodes
		}

		for (unsigned long int i = 0; i < nodeCount; i++) { //We have to first know the partition before calculate DIE values
			klNodeInfo[i] = findNodeInfo(i,nodeCount,klNodeGroup,costAB);
		}

		//To record the gain
		unsigned long int klGainStep[pairCount];
		double klGainAcc = 0;
		double klGainAccMax = 0;
		unsigned long int klGainAccMaxAt;

		//Find the global max gain
		for (unsigned long int i = 0; i < pairCount; i++) {

#if(DEBUG)
			printf("--> Finding best pair, current=%lu / total=%lu:\n",i,pairCount);
#endif

			//Find best pair (local max)
			double localMaxGain = 0;
			uint8_t localMaxGainValid = 0;
			for (unsigned long int ia = 0; ia < nodeCount-1; ia++) { //For all nodes
				for (unsigned long int ib = ia+1; ib < nodeCount; ib++) {
#if(DEBUG_FINDLOCALMAX)
					printf("----> Check node %5s (%5lu) G-%c with Node %5s (%5lu) G-%c: ",nodelist[ia],ia,klNodeGroup[ia],nodelist[ib],ib,klNodeGroup[ib]);
#endif

					if ( klNodeGroup[ia] == klNodeGroup[ib] || klLocked[ia] || klLocked[ib] ) { //Unlocked, from different group
#if(DEBUG_FINDLOCALMAX)
						printf("Same group or locked.\n");
#endif
						continue;
					}

					double gain = klNodeInfo[ia].d + klNodeInfo[ib].d - costAB[ia][ib] - costAB[ib][ia];
#if(DEBUG_FINDLOCALMAX)
					printf("Gain = %f.\n",gain);
#endif
					if ( (!localMaxGainValid) || localMaxGain < gain ) { //Not set yet or find new max
						localMaxGain = gain;
						localMaxGainValid = 1;
					}
				}
			}

			//Lock the best pair, record gain and swap
			for (unsigned long int ia = 0; ia < nodeCount-1; ia++) {
				for (unsigned long int ib = ia+1; ib < nodeCount; ib++) {
					if ( klNodeGroup[ia] == klNodeGroup[ib] || klLocked[ia] || klLocked[ib] )
						continue;

					double gain = klNodeInfo[ia].d + klNodeInfo[ib].d - costAB[ia][ib] - costAB[ib][ia];
					if (localMaxGain == gain) { //Find the best pair

						//Record gain (local max)
						klGainStep[i] = gain;
						klGainAcc += gain;
#if(DEBUG)
						printf("--> Best pair node %5s (%5lu) G-%c with Node %5s (%5lu) G-%c. Gain = %f (Acc=%f).\n",nodelist[ia],ia,klNodeGroup[ia],nodelist[ib],ib,klNodeGroup[ib],gain,klGainAcc);
#endif

						//Swap group
						char tmp = klNodeGroup[ia];
						klNodeGroup[ia] = klNodeGroup[ib];
						klNodeGroup[ib] = tmp;

						//Lock current node pair
						klLocked[ia] = 1;
						klLocked[ib] = 1;

						//New global max found
						if (klGainAccMax < klGainAcc) {
							klGainAccMax = klGainAcc;
							klGainAccMaxAt = i;
							for (unsigned long int j = 0; j < nodeCount; j++) {//Save the current partition
								klBestNodeGroup[j] = klNodeGroup[j];
							}
#if(DEBUG)
							printf("--> New gloabl max (%f) found at pair %lu.\n",klGainAccMax,klGainAccMaxAt);
#endif
						}

						ia = nodeCount; //Stop when find the first max value
						ib = nodeCount;
					}
				}
			}

			//Update DIE of unlocked
			for (unsigned long int j = 0; j < nodeCount; j++) {
//				if (!klLocked[j])
					klNodeInfo[j] = findNodeInfo(j,nodeCount,klNodeGroup,costAB);
			}
		}

#if(DEBUG)
		printf("Result at now:\n");
		printf("> Group A:\n");
		for (unsigned long int i = 0; i < nodeCount; i++) {
			if (klBestNodeGroup[i] == 'A')
				printf("--> %s\n",nodelist[i]);
		}
		printf("> Group B:\n");
		for (unsigned long int i = 0; i < nodeCount; i++) {
			if (klBestNodeGroup[i] == 'B')
				printf("--> %s\n",nodelist[i]);
		}
#endif

		//KL Greedy Algorithm - Is the current loop positive?
		klGain = klGainAccMax;
		if (klGain > 0) {
			printf("Current KL loop is positive. Gain = %f, pair = %lu.\n",klGain,klGainAccMaxAt);
			for (unsigned long int j = 0; j < nodeCount; j++) //Apply the partition at global max
				nodeGroup[j] = klBestNodeGroup[j];
		}
		else
			printf("Current KL loop is negative.\nFollowing partition (from last loop) is the best result:\n");

	} while (klGain > 0 && workloop++ < MAX_TRY_TIME);

/*******************************************************************************
6 - Show final result
*******************************************************************************/
	printf("\nFinal result:\n");
	printf("> Group A:\n");
	for (unsigned long int i = 0; i < nodeCount; i++) {
		if (nodeGroup[i] == 'A')
			printf("--> %s\n",nodelist[i]);
	}
	printf("> Group B:\n");
	for (unsigned long int i = 0; i < nodeCount; i++) {
		if (nodeGroup[i] == 'B')
			printf("--> %s\n",nodelist[i]);
	}

	//Find DIE value
	totalE = 0, totalI = 0;
	for (unsigned long int i = 0; i < nodeCount; i++) {
		nodeInfo[i] = findNodeInfo(i,nodeCount,nodeGroup,costAB);
		totalE += nodeInfo[i].e;
		totalI += nodeInfo[i].i;
	}
	
	printf("> Total E cost = %.3f, total I cost = %.3f.\n",totalE, totalI);
#if(DEBUG)
	for (unsigned long int i = 0; i < nodeCount; i++) {
		printf("--> Node %5s (%5lu) G-%c \t D = %.3f, I = %.3f, E = %.3f.\n",nodelist[i],i,nodeGroup[i],nodeInfo[i].d,nodeInfo[i].i,nodeInfo[i].e);
	}
#endif

	printf("DONE! (loop=%lu)\n\n",workloop);
	return 0;
}


