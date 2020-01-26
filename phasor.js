'use strict';

function getNodes(nodefile) {
	var nodes = [];
	var format = /^[ap][0-9]+/;
	nodefile.split(/\n/).map( x => { //Read each line of the file
		if ( format.test(x.trim()) ) nodes.push({
			"name"		: x.replace('terminal','').trim(), //Remove the "terminal" note
			"pos"		: [Math.random(),Math.random()], //A random position for canvas
			"I"		: null,
			"E"		: null,
			"D"		: null,
			"locked"	: false
		});
	} );
	return nodes;
}

/*
Format of nodes:
	e.g.1 - p123
	e.g.2 - a678
Which shows:
	At a new line, no space; a letter either 'a' or 'p', followed by number(s)
RegExp "/^[ap][0-9]+/" will match any line described above:
	^: Start at beginning of the string
	[ap]: One letter, either 'a' or 'p'
	[0-9]+: One or more number (any character in range from '0' to '9')
*/

function getEdges(netfile) {
	var edges = [];
	netfile = netfile.split(/\n/); //Split into lines
	var line1Format = /^NetDegree \: ([0-9]+)$/ //Return the number if RegExp.exec()
	for (var i = 0; i < netfile.length; i++) {
		var match = line1Format.exec(netfile[i].trim());
		/*
		If matched: match[0] = matchedString; match[1...] = matchedSubString...
		If mismatched: match = null
		*/
		if (match) {
			var nodes = []; //Get all nodes connected by this edge
			for (var j = 1; j <= match[1]; j++)
				nodes.push(netfile[i+j].replace(' B','').trim()); //Remove the meanless "B"
			edges.push({
				"nodes"		: nodes, //A collection (array) of all connected nodes
				"weight"	: 1, //Assume weight of edge to be 1
				"pos"		: [Math.random(),Math.random()], //A random position for canvas
			});
			console.log('Find new net: ',nodes);
		}
	}
	return edges;
}

/*
Format of nets:
	NetDegree : 2
	p1 B
	a0 B
Which shows:
	Line 1: String "NetDegeree : " followed by a number, call it $length
	Line 2 to (1+$length): A node name (either "a123" or "p456"), followed by " B" which is meanless
To process this:
	1 - Find Line 1, get $length
		1 - Using RegExp "/^NetDegree \: [0-9]+$/"
			^...$: This line only contains ..., no any leading or tailing string
			NetDegree \: : A string, escape the ':'
			[0-9]+: An int (one or more character in range from '0' to '9')
	2 - Read $length lines followed by Line 1
*/