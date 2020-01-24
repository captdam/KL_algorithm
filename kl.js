'use srict';
/* KL utils*/

//Get the D, I and E value for all nodes in the design
function getDIE(design,netlist) {
	['groupA','groupB'].map( group => {
		for (var i = 0; i < design[group].length; i++) { //Find D I E for all nodes
			design[group][i].I = 0;
			design[group][i].E = 0;
			netlist.map( edge => { //For each node in the design, check all edges
//				console.log('I'+design[group][i].I,'E'+design[group][i].E);
//				console.log('Check node '+design[group][i].name+' edge ',edge.nodes);
				//Case 1 - This edge is not connected to this node
				if (!edge.nodes.includes(design[group][i].name))
					return;
				//Then, consider all nodes connected to this edge, except this node
				var edgeWithoutThisNode = {
					"nodes" : edge.nodes.filter( x => {return x != design[group][i].name} )
				};
				//Case 2 - This edge connected nodes both inside and outside of the group
				if (isInterConnect(design,edgeWithoutThisNode)) {
					design[group][i].I += edge.weight;
					design[group][i].E += edge.weight;
					return;
				}
				//Case 3 - This edge connected nodes only in / out of the current group
				if ( getNodeGroup(design,design[group][i].name) == getNodeGroup(design,edgeWithoutThisNode.nodes[0]) ) //Same group
					design[group][i].I += edge.weight;
				else
					design[group][i].E += edge.weight;
				
			} );
			design[group][i].D = design[group][i].E - design[group][i].I;
		}
	} );
}

//Get the weighted cost of inter-connect edges in this design (cut-cost)
function countInterConnect(design,netlist) {
	var interConnect = 0, cost = 0;
	netlist.map( x => { //Check all edges
		if ( isInterConnect(design,x) ) {
			interConnect++;
			cost += x.weight;
		}
	} );
	return interConnect;
}

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
	for (var i = 0; i < design.groupA.length; i++) {
		if (design.groupA[i].name == nodeName)
			return 'A';
	}
	return 'B';
}

//Get node info ()
function getNodeInfo(design,nodeName) {
	var info = null;
	design['group'+getNodeGroup(design,nodeName)].map( x => {
		if (x.name == nodeName) info = x;
	} );
	return info;
}