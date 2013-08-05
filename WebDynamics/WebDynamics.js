/*
Web Dynamics Javascript Functions
James Wonsever
Mftheory
6/26/2013 -> present
*/

//Changing active viewing page
var currentDiv;
function CngClass(obj){
    if (currentDiv) currentDiv.addClass('optionsDiv');
    obj.removeClass('optionsDiv');
    currentDiv=obj;
}

//Button to minimize structure
function minimizeStructure() {
    Jmol.script(mainApplet,"minimize");
}

//Button to minimize structure
function minimizeAddHydrogens() {
    Jmol.script(mainApplet,"minimize addHydrogens");
}

//Currently turns on modelkit mode, may be expanded.
function toggleModelkitMode() {
    var currentState = ($('#mkmode').val() == 'Off') ? 'On' : 'Off';
    Jmol.script(mainApplet, "set allowModelKit " + (currentState == 'On'));
    $('#mkmode').prop('value', currentState);
    if (currentState == 'On') Jmol.script(mainApplet, "set modelKitMode");
}

//Coordinates Div
function makeCoordsDiv() {
    var myHtml = '';
    //console.log(models.length);
    for (var m = 0; m < models.length; m++ ) {
	//console.log(models[m]);
	var n = Number(m)+1;
	myHtml+='<input type="button" class="mybutton" onclick="switchToModel('+m+')" value="'+n+'"/>';
    }
    myHtml += '<input type="button" class="mybutton" onclick="addNewModel()" style="color:green" value="+"/>';
    myHtml += '<input type="button" class="mybutton" onclick="removeCurrentModel()" style="color:red" value="X"/>';
    myHtml += '<input type="button" class="mybutton" onclick="removeAllModels()" style="color:red" value="Clear"/>';
    myHtml+='<br><TEXTAREA id="activeCoords" class="field span5" rows="12" cols="30" onkeyup="updateModels()">'+models[activeModel]+'</TEXTAREA><br>';
    $('#Coordinates').html(myHtml);
}

function updateModels() {
    models[activeModel] = $('#activeCoords').val();
    drawMol();
}
function switchToModel(i) {
    activeModel = i;
    makeCoordsDiv();
    drawMol();
}
function removeCurrentModel() {
    var mIndex = Number(activeModel);
    if (models.length == 1) return;
    models = models.slice(0,mIndex).concat(models.slice(mIndex+1));
    switchToModel(Math.max(mIndex-1, 0));
}
function removeAllModels() {
    models = [""];
    switchToModel(0);
}
function addNewModel() {
    models.push("Please Enter or Upload Model Cooridnates");
    activeModel = models.length - 1;
    makeCoordsDiv();
    drawMol();
}
function readCoordsFromJmol() {
    //fix to check for multiple models being uploaded
    var modelInfo = Jmol.getPropertyAsArray(mainApplet, "modelInfo", "all");
    var modelNum = modelInfo.modelCount;
    var i = 1;
    while (i <= modelNum) {
	var coords = "";
	var atoms = Jmol.getPropertyAsArray(mainApplet, "atomInfo", "1." + i);
	//console.log(atoms);
	for (atom = 0; atom < atoms.length; atom++) {
	    coords += atoms[atom].sym + " ";
	    coords += atoms[atom].x + " ";
	    coords += atoms[atom].y + " ";
	    coords += atoms[atom].z + " ";
	    coords += "\n";
	}
	//Upload to the active model...
	if (models[(i-1)] == "") models[(i-1)] = coords;
	else models.push(coords);
	i++;
    }
    //Update the coords div.
    makeCoordsDiv();
}

//upload files directly to the jmol main editor.
function uploadCoordinates(uploadobj) {
    var file = uploadobj.files[0];
    var reader = new FileReader();  
    if (typeof FileReader !== "undefined") {
	try {
	    reader.onload = function(r) {
		var scr = "try{LOAD INLINE \"";
		scr += String(reader.result);
		scr +="\" {1, 1, 1};}catch(e){}";
		//alert(scr);
		Jmol.script(mainApplet, scr);
	    }
	    reader.readAsText(file);	
	} catch(err) {
	    alert("File error: bad file");
	}
    } else {
	alert("Your browser does not support file uploading, please use google chrome or firefox.");
    }
}

//Ensures that coordinates are properly written.
function sterilize(xyzcoords) {
    var lines = xyzcoords.split("\n");
    var out = "";
    for (l in lines) {
	var line = lines[l];
	if (typeof line != "string") continue;
	//removes whitespace
	//console.log(line);
	line = line.replace(/^\s*/g, '').replace(/\s*$/g, '');
	line = line.replace(/[  ]{2,}|\t/g, ' ');
	//checks if the regex matches proper form
	//ex C 0 -.1 1.2e-4 valid number formats
	line = line.replace(/^((?!([a-zA-Z]{1,2}([ \t]-?(\d+\.?\d*|\.\d+)(e-?\d+)?){3})).)*$/, '');
	//console.log(line);
	if (line != "") {
	    if (l == 0) {
		out += line;
	    } else {
		out +="\n" + line;
	    }
	}
    }
    return out;
}

//Turns the given coordinates into a valid xyz file.
function makeXYZfromCoords(i) {
    //i could be 0
    if (i === undefined) i = activeModel;

    var materialName = ${"#structureName"}.val();
    var cell = $("#structureCell").val();

    var coords = sterilize(models[i]);
    var numberOfAtoms = coords.split("\n").length;
    var xyz = numberOfAtoms + "\n" + materialName + ", Cell " + cell + "\n" + coords;
    console.log(xyz);
    return xyz;
}
//Redraw the molecule according to coordinates
function drawMol() {
    var xyz = makeXYZfromCoords();
    var scr = "";
    if (amIMobile) {scr += unbindMobileClicks();}
    scr += "try{LOAD INLINE \"" + xyz + "\" ";
    scr += getUnitCell() + ";}catch(e){}";
    console.log(scr);
    Jmol.script(mainApplet, scr);
}
function getUnitCell(){
    var cell = $('#structureCell').val();
    cell = cell.split("x");
    var vector = "{"+cell[0]+" "+cell[1]+" "+cell[2]+" "+cell[3]+" "+cell[4]+" "+cell[5]+"}";
    var offset = "{"+(cell[0]/2.0)+" "+(cell[1]/2.0)+" "+(cell[2]/2.0)+"}";
    return " {1 1 1} unitcell " + vector + " offset " + offset;
    //TODO
    //return crystal cell on crstals.
}
function unbindMobileClicks() {
    var scr = "";
    scr += "unbind 'RIGHT';";
    scr += "unbind 'LEFT' '_clickFrank'; ";
    return scr;
}

function sendToWebXS() {
    if (models[0] == "") readCoordsFromJmol();
    writeFileToFilesystem(DATABASE_DIR + "/tmp/"+myUsername+".xyz");
    window.open("../WebXS/index.html");
}

function saveFile() {	
    var fname = prompt("Save file as?", GLOBAL_SCRATCH_DIR + myUsername + "/WebDynamics.xyz");

    //put on Hopper
    writeFileToFilesystem(fname);
    //give options...
}
function writeFileToFilesystem(filename) {
    var xyz = "";
    for (var i = 0; i < models.length; i++) {
	xyz += makeXYZfromCoords(i) + "\n";
    }
    //post file
    //console.log(xyz);
    $.newt_ajax({type: "PUT", 
		url: "/file/hopper/" + filename,
		data: xyz,
		success: function(res) {
		
		console.log("Written to Hopper");
		//chmod it 777
		$.newt_ajax({type: "POST", 
			    url: "/command/hopper",
			    data: {"executable": "/bin/chmod 777 "+filename},
			    success: function(res) {
			    console.log("Chmoded the file.  No residuals.");
			},});
	},});    
}

function runCp2k() {
    //grab all inputs
    var press = $("#pressure").val();
    var temp = $("#temperature").val();
    var walltime = $("#walltime").val();
    var nnodes = $("#nnodes").val();
    var nsnaps = $("#snapshots").val();

    //put xyz file.
    var now= new Date();
    now = now.toDateString.replace(" ","") + ".xyz";
    alert(now);
    var filePath=GLOBAL_SCRATCH_DIR + myUsername + "/WebDynamics/cp2k/" + now;
    writeFileToFilesystem(filePath);


    var command = CODE_BASE_DIR + DYN_LOC + "/cp2k/runCp2k.sh " ;
    var args = filePath + " " + now + " ";
    //(pass unitcell)

    //runcp2k script for n snapshots    
    /*
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command + args},
	});
    */

    //Track
    //draw progressbar based on walltime req

    //load in at bottom (from gscratch location)
    //Same interface as ABOVE

    //pull into jsmol
    var scr = "try {"

    scr += ";}catch(e){}"
    Jmol.script("mainApplet", scr/*Do This*/)

}
