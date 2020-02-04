'use srict';
/* KL utils*/

//Get the D, I and E value for a nodes in the design
function getNodeDIE(nodeName,design,netlist) {
	var i = 0, d = 0, e = 0;
	// netlist.map( edge => { //For each node in the design, check all edges
		// //Case 1 - This edge is not connected to this node
		// if (!edge.nodes.includes(nodeName))
			// return;
		
		// //Then, consider all nodes connected to this edge, except this node
		// var edgeWithoutThisNode = {
			// "nodes" : edge.nodes.filter( x => {return x != nodeName} )
		// };
		// //Case 2 - This edge connects to nodes both inside and outside of the group
		// if (isInterConnect(design,edgeWithoutThisNode)) {
			// i += edge.weight;
			// e += edge.weight;
			// return;
		// }
		// //Case 3 - This edge connected nodes only in / out of the current group
		// /* edgeWithoutThisNode[0] represents the group all nodes*/
		// if ( getNodeGroup(design,nodeName) == getNodeGroup(design,edgeWithoutThisNode.nodes[0]) ) //Same group
			// i += edge.weight;
		// else
			// e += edge.weight;
		
	// } );
	
	['groupA','groupB'].map( group => {
		for (node in design[group]) {
			if (node != nodeName) { //To find the I and E value, check all nodes in the design except this node
				if ( getNodeGroup(design,nodeName) == getNodeGroup(design,node) ) //In same group
					i += getCost(netlist,nodeName,node);
				else
					e += getCost(netlist,nodeName,node);
			}
		}
		
	} );
	
	
	
	return [e-i,i,e];
	
	
}

//Get the weighted cost of inter-connect edges in this design (cut-cost)
/*function countInterConnect(design,netlist) {
	var interConnect = 0, cost = 0;
	netlist.map( x => { //Check all edges
		if ( isInterConnect(design,x) ) {
			interConnect++;
			cost += x.weight;
		}
	} );
	return interConnect;
}*/

//Test if a edge connects nodes from more than one group
function isInterConnect(design,edge) {
	var result = {"A":false,"B":false};
	edge.nodes.map( x => { //Check all nodes connected by this edge
		result[getNodeGroup(design,x)] = true;
	} );
	return result.A && result.B;
}

//Where is the node? return 'A' or 'B'
function getNodeGroup(design,nodeName) {
	if (design.groupA[nodeName])
		return 'A';
	return 'B';
}

//Get node info ()
/*function getNodeInfo(design,nodeName) {
	return design.groupA[nodeName] | design.groupB[nodeName];
}*/

//Get the sum of all edges that connects the given two nodes
function getCost(netlist,nodeAName,nodeBName) {
	cost = 0;
	netlist.map( x => {
		if (x.nodes.includes(nodeAName) && x.nodes.includes(nodeBName))
			cost += x.weight / (x.nodes.length-1);
	} );
	return cost;
}

//Swap element in a design (Notice: JS function pass the copy of reference of object)
function swap(design, nodeAName, nodeBName) {
	design.groupA[nodeBName] = design.groupB[nodeBName];
	design.groupB[nodeAName] = design.groupA[nodeAName];
	delete design.groupA[nodeAName];
	delete design.groupB[nodeBName];
}