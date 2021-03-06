'use strict';

//Read a file
function fileReader(file) {
	return new Promise( (resolve,reject) => {
		var reader = new FileReader();
		reader.onload = () => resolve(reader.result);
		reader.readAsText(file);
	} );
}

//Generate random number in the range
function getRandom(start,end) {
	return Math.floor((end-start)*Math.random()) + start;
}

//Log object, show the value at that monment instead of live value in inspector
// <-- See https://developer.mozilla.org/en-US/docs/Web/API/Console/log#Logging_objects
//Also, works as object deep copy
function lo(obj) {
	return (JSON.parse(JSON.stringify(obj)));
}

//Prompt user to save result
function saveResult(data,name) {
	var downloader = document.createElement('a');
	downloader.href = 'data:application/octet-stream;charset=utf-8;base64,' + btoa(data);
	downloader.download = name + '_result.json';
	downloader.click();
}
