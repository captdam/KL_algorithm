'use srict';
/* KL utils*/

//Get the number of inter-connect edges in this design (cut-cost)
/*function countInterConnect(design,netlist) {
	var interConnect = 0;
	netlist.map( x => { //Check all edges
		if ( isInterConnect(design,x) ) interConnect++;
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