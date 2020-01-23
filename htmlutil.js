'use strict';

//Read a file
function fileReader(file) {
	return new Promise( (resolve,reject) => {
		var reader = new FileReader();
		reader.onload = () => resolve(reader.result);
		reader.readAsText(file);
	} );
}

function getRandom(start,end) {
	return Math.floor((end-start)*Math.random()) + start;
}