#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_NODENAME_SIZE 255

#ifndef DEBUG_KLUTIL
#define DEBUG_KLUTIL 1
#endif

// Struct to store D I and E value of a node
struct NodeInfo {
	double d;
	double i;
	double e;
};

// To find the index of a node in nodelist by nodename
// This is helpful to find an element in the nodelist-X array pair
unsigned long int getNodeIndex(char* nodename, char** nodelist, unsigned long int nodeCount) {
	for (unsigned long int i = 0; i < nodeCount; i++) {
		if (strncmp(nodename,nodelist[i],MAX_NODENAME_SIZE) == 0)
			return i;
	}
	return (unsigned long int)-1; //Not found
}

// To calculate the D, I and E value of a node by nodename
struct NodeInfo findNodeInfo(unsigned long int nodeIndex, unsigned long int nodeCount, char nodeGroup[], double costTable[][nodeCount]) {
	double i = 0, e = 0;
	char thisNodeGroup = nodeGroup[nodeIndex];
	for (unsigned long int index = 0; index < nodeCount; index++) { //Find the sum cost
		if (thisNodeGroup == nodeGroup[index]) //In same group
			i += costTable[nodeIndex][index];
		else
			e += costTable[nodeIndex][index];
		/* NOTE:
		Since cost[thisNode][thisNode] is always 0, including this cost in the calculation will not nagatively affect the result,
		and can avoid branch overhead for the hardware.
		*/
	}
	struct NodeInfo die = {
		.i = i,
		.e = e,
		.d = e - i
	};
	return die;
}

// To calculate the cut size (number of nets which connects nodes from both group)
unsigned long int findCutsize(char nodeGroup[], char* nodelist[], unsigned long int nodeCount, struct Net* netlist[], unsigned long netCount) {
	unsigned long cutsize = 0;

	//Check all nets
	for (unsigned long int i = 0; i < netCount; i++) {
		uint8_t hasAB[2] = {0,0}; //hasAB[0]=1 means this edge are connected to groupA, hasAB[1]=1 means this edge are connected to groupB

		//Check all nodes in this net
		for (unsigned long int j = 0; j < netlist[i]->nodeCount; j++) {
			unsigned long int nodeIndex = getNodeIndex(netlist[i]->connectedNode[j], nodelist, nodeCount);
			char group = nodeGroup[nodeIndex];
#if(DEBUG_KLUTIL)
			printf("--> [Net-%5lu] Check node %s (G-%c)\n",i,netlist[i]->connectedNode[j],group);
#endif
			hasAB[group-'A'] = 1;
			if (hasAB[0] && hasAB[1]) { //This net connects nodes from both group
				cutsize++;
#if(DEBUG_KLUTIL)
				puts("--> Cutsize++");
#endif
				j = netlist[i]->nodeCount; //Check next net
			}
		}
	}

	return cutsize;
}

