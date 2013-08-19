/*
Web Dynamics Javascript Functions
James Wonsever
Mftheory
6/26/2013 -> present
*/

//Am i working with a crystal or a molecule?
var crystalFlag = false;

//On authentication:
function checkAuthCallback() {;}

//On DSR
function DSRcallback() {
    readCoordsFromJmol();
    switchToModel(models.length-1);
}

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
    var filename = file.name;

    var reader = new FileReader();  
    if (typeof FileReader !== "undefined") {
	try {
	    reader.onload = function(r) {
		var filedata = String(reader.result);
		var scr = "try{\nLOAD INLINE \"";
		scr += filedata;
		scr +="\";}catch(e){}";

		Jmol.script(mainApplet, scr);

		var gotCrystal = tryToGrabCrystalData();
		readCoordsFromJmol();
		if (!gotCrystal) {
		    switchToModel(models.length-1);
		} else {
		    runThroughGdisAndReload(filedata, filename);		    
		}
	    }
	    reader.readAsText(file);	
	} catch(err) {
	    alert("File error: bad file");
	}
    } else {
	alert("Your browser does not support file uploading, please use google chrome or firefox.");
    }
}

function tryToGrabCrystalData() {
    //reads only from models[0] as of right now
    var jsVar = Jmol.getPropertyAsJavaObject(mainApplet, "auxiliaryinfo.models[0].infoUnitcell", "all");
    if(jsVar) {
	var newUnitCell = "" + jsVar[0];
	newUnitCell += "x" + jsVar[1];
	newUnitCell += "x" + jsVar[2];
	newUnitCell += "x" + jsVar[3];
	newUnitCell += "x" + jsVar[4];
	newUnitCell += "x" + jsVar[5];
	$('#structureCell').val(newUnitCell);
	crystalFlag = true;
	return true;
    }
    else {
	crystalFlag = false;
	return false;
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

    var materialName = $("#structureName").val();
    var cell = $("#structureCell").val();

    var coords = sterilize(models[i]);
    var numberOfAtoms = coords.split("\n").length;
    var xyz = numberOfAtoms + "\n" + materialName + ", Cell " + cell + "\n" + coords;
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

//Return the cell
function getUnitCell(){
    var cell = $('#structureCell').val();
    cell = cell.split("x");
    var vector = "{"+cell[0]+" "+cell[1]+" "+cell[2]+" "+cell[3]+" "+cell[4]+" "+cell[5]+"}";
    var offset = "{"+(cell[0]/2.0)+" "+(cell[1]/2.0)+" "+(cell[2]/2.0)+"}";
    var repstr = getRepstr();
    if (crystalFlag) {
	return " {555 "+ repstr + " 1} unitcell " + vector;
    }
    else { // Adds offset to place molecules in the middle of their cell, (More intuitive to user)
	return " {555 "+ repstr + " 1} unitcell " + vector + " offset " + offset;
    }
}
//Return the number of times to replicate the cell
function getRepstr() {
    var rep = $('#cellReplication').val();
    rep=rep.split("x");
    var repnum = "" + (4 + parseInt(rep[0])) + (4 + parseInt(rep[1])) + (4 + parseInt(rep[2]));
    return repnum;
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

function SaveFile() {	
    var sysname = $("#structureName").val();
    var spcext = "                                                                            ";//Force Textbox Width
    var fname = prompt("Save file as? " + spcext, GLOBAL_SCRATCH_DIR + myUsername + "/"+ sysname +".xyz");

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






//RUN CP2K Simulation, roughly equivalent to the submit commands in WebXS
function runCp2k() {
    //grab all inputs
    var sysname = $("#structureName").val();


    //put xyz file.
    var now= new Date();
    now = now.toDateString().replace(/ /g,"");
    var fullname = sysname + now;
    //alert(fullname);
    var basePath=GLOBAL_SCRATCH_DIR + myUsername + "/WebDynamics/cp2k/" + fullname + "/"
    var filePath=GLOBAL_SCRATCH_DIR + myUsername + "/WebDynamics/cp2k/" + fullname + "/" + sysname + ".xyz";
    
    //Make Output Directory
    var mkd = SHELL_CMD_DIR+"makeFileDir.sh "+basePath;
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": mkd},
		success: function(res) {writeFileToFilesystem(filePath);
		executeCp2kScript(filePath);},
    });
}
function executeCp2kScript(filePath) {   
    var command = CODE_BASE_DIR + DYN_LOC + "/cp2k/runCp2k.sh " ;
    
    var args = filePath + " ";

    var sysname = $("#structureName").val();
    args += sysname + " ";

    var unitcell = $("#structureCell").val();
    args += unitcell + " ";

    var temp = $("#temperature").val();
    args += temp + " ";

    var press = $("#pressure").val();
    args += press + " ";


    var nnodes = $("#nnodes").val();
    args += nnodes + " ";

    //todo
    var walltime = $("#walltime").val();
    var nsnaps = $("#snapshots").val();

    //runcp2k script for n snapshots    
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command + args},
		success: function (res) {console.log("Started CP2K");updateCp2kTracker()}
	});
}
function updateCp2kTracker() {
    //Track
    //draw progressbar based on walltime req

    //load in at bottom (from gscratch location)
    //Same interface as ABOVE

   

}
function openFinishedCp2k(structure) {
    //Jmol Show Loading
    var scr = "zap;set echo top left;font echo 16;echo \"loading...\";refresh;";
    Jmol.script(mainApplet, scr);
    

    file = GLOBAL_SCRATCH_DIR + myUsername + "/WebDynamics/cp2k/" + structure + "/results/snapshots.xyz";

     $.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		 res = res + ""; 
		 res = res.replace(/\r\n|\r|\n/g, "\n");
	 
		 if (amIMobile) {scr = unbindMobileClicks();}
		 scr += "try{load INLINE '" + res + "';}catch(e){}";   
		 Jmol.script("mainApplet", scr/*Do This*/);
    
	     },});
    //TODO: update cell
}

/*Write XYZ Files
  Convert to CIF
  Append cifCellParams
  rewrite to XYZ (Folding in the cell)
*/
function foldWrapper() {
    //Currently only acts on active model, TODO act on all models.
    var fdata = makeXYZfromCoords();
    foldXYZUsingGdis(fdata);
}
function foldXYZUsingGdis(filedata, filename) {
    if (!filename) filename = $("#structureName").val() + ".xyz";
    filename=cleanDirtyChars(filename);

    var unitcell = $("#structureCell").val().split("x");
    var cifCellParams = "_cell_length_a                     "+unitcell[0]+"\n"
	              + "_cell_length_b                     "+unitcell[1]+"\n"
	              + "_cell_length_c                     "+unitcell[2]+"\n"
               	      + "_cell_angle_alpha                  "+unitcell[3]+"\n"
	              + "_cell_angle_beta                   "+unitcell[4]+"\n"
	              + "_cell_angle_gamma                  "+unitcell[5]+"\n";
    
    //post XYZ file
    $.newt_ajax({type: "PUT", 
		url: "/file/"+"hopper" + CODE_BASE_DIR + DATA_LOC + "/tmp/" + filename,
		data: filedata,
		success: function(res) {foldWithGdis(filedata, filename, cifCellParams);},});
}
function foldWithGdis(filedata, filename, cifParams) {
    //Note that this accesses the script in WebXS at the moment.  (TODO/TOFIX?)
    command = SHELL_CMD_DIR + "foldUsingGdis.sh " + filename + " '"+ cifParams + "'";
    $.newt_ajax({type: "POST",
		url:"/command/hopper",
		data: {"executable": command},
		success: function(res) {console.log(res);reloadGdisOutput(filedata, filename, "");},});
}

/*Push file to server		      
  Execute Gdis
  return file to jmol
  remove files
  quit
*/
function runThroughGdisAndReload(filedata, filename) {
    
    filename=cleanDirtyChars(filename);

    //post file
    $.newt_ajax({type: "PUT", 
		url: "/file/"+"hopper" + CODE_BASE_DIR + DATA_LOC + "/tmp/" + filename,
		data: filedata,
		success: function(res) {convertWithGdis(filedata, filename);},});
}
function convertWithGdis(filedata, filename) {
    //Note that this accesses the script in WebXS at the moment.  (TODO/TOFIX?)
    command = SHELL_CMD_DIR + "convertUsingGdis.sh " + filename + " .xyz";
    $.newt_ajax({type: "POST",
		url:"/command/hopper",
		data: {"executable": command},
		success: function(res) {console.log(res);reloadGdisOutput(filedata, filename, ".xyz");},});
}

//Take whatever came out of gdis, load it, erase it, and return it to the user.
function reloadGdisOutput(filedata, filename, ext) {
    var convertedFile = "../Shirley-data/tmp/" + filename + ext;
    var scr = "try{load "+convertedFile+";}catch(e){};";
    //Draw to Jmol
    Jmol.script(mainApplet, scr);
    //rewrite new coordinates
    models = models.slice(0, -1);
    readCoordsFromJmol();
    switchToModel(models.length-1);
    //Get rid of temporary file
    deleteGdisOutputFile(filename, ext);
}
function deleteGdisOutputFile(filename, ext) {
    command = "/bin/rm " + CODE_BASE_DIR + DATA_LOC + "/tmp/" + filename + ext;
    $.newt_ajax({type: "POST",
		url:"/command/hopper",
		data: {"executable": command},
		success: function(res) {console.log("successful delete of gdis output");},});
}
function cleanDirtyChars(filename){
    //Clean dirty charcters from filename
    filename = filename.replace(/[|&;$%@"<>()+,]/g, "");
    //"//Emacs coloring bug
    return filename;
}