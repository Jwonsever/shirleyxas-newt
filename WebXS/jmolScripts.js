/*
James Wonsever
Lawrence Berkeley Laboratory
Molecular Foundry
05/2012 -> Present

Custom Scripts and options for use with the Jmol applet.
Used with the moleculeEditor <div> in index.html.
*/

//---------------------------------
//Editor Functions-----------------
//---------------------------------
function initMainApp() {
    setTimeout("drawMol('main');", 200);
}

function help() {
    //Jmol.script("set ScriptCallback \"helpHelper\";", mainApplet)
    //var scr ="show unitcell;";
    //Jmol.script(scr,mainApplet);
    //explain jmol
}

//Button to minimize structure
function minimizeStructure() {
    Jmol.script(mainApplet,"minimize addHydrogens");
}

//Currently turns on modelkit mode, may be expanded.
function toggleModelkitMode() {
    var currentState = ($('#mkmode').val() == 'Off') ? 'On' : 'Off';
    Jmol.script(mainApplet, "set allowModelKit " + (currentState == 'On'));
    $('#mkmode').prop('value', currentState);
    if (currentState == 'On') Jmol.script(mainApplet, "set modelKitMode");
}

//upload files directly to the jmol main editor.
function uploadToEditor(form) {
    var file = form.uploadfile.files[0];
    var reader = new FileReader();  
    if (typeof FileReader !== "undefined") {
	try {
	    reader.onload = function(r) {
		var scr = "try{LOAD DATA \"mod\"\n";
		scr += String(reader.result);
		scr +="\nEND \"mod\" {1, 1, 1};unitcell ON;}catch(e){}";
		//alert(scr);
		Jmol.script(mainApplet, scr);
				    
		var AmIACrystal = Jmol.getPropertyAsJavaObject(mainApplet, "auxiliaryinfo.models[0].infoUnitcell", "all");
		console.log(AmIACrystal);
		if (AmIACrystal) {
		    //Send through Gdis && Keep Data
		}

		//drawMol();
	    }
	    reader.readAsText(file);	
	} catch(err) {
	    alert("File error: bad file");
	}
    } else {
	alert("Your browser does not support file uploading, please use google chrome or firefox.");
    }
}

function portCoordinates() {
    var submitForm = document.getElementById('inputs');
    var atoms = Jmol.getPropertyAsArray(mainApplet, "atomInfo", "visible"); // wont work on ubuntu chrome?
    //console.log(atoms);
    var coords = "";
    try {
	for (atom = 0; atom < atoms.length; atom++) {
	    coords += atoms[atom].sym + " ";
	    coords += atoms[atom].x + " ";
	    coords += atoms[atom].y + " ";
	    coords += atoms[atom].z + " ";
	    coords += "\n";
	    //console.log(atom);
	    //somehow port crystal data?
	}
	models.push(coords);

    } catch(err) {
	console.log("ERROR, Failed to export coords.");
	console.log(err);
    }

    makeCellSize();
    //fix so draws from .mol file?
    //drawMol();
}

//----------------------------------
//Preview Functions-----------------
//----------------------------------
function tryToGrabCrystalData() {
    var myform = document.getElementById('inputs');
    //reads only from models[0] as of right now
    var jsVar = Jmol.getPropertyAsJavaObject(previewApplet, "auxiliaryinfo.models[0].infoUnitcell", "all");
    //console.log(jsVar);
    if(jsVar) {
	
	CrystalSymmetry = "" + Jmol.getPropertyAsJavaObject(previewApplet, "auxiliaryInfo.models[0].spaceGroup", "all");
	myform.CrystalFlag.checked=true;
	//console.log(jsVar[5]);
	myform.CellA.value = "" + jsVar[0];
	myform.CellB.value = "" + jsVar[1];
	myform.CellC.value = "" + jsVar[2];
	myform.CellAlpha.value = "" + jsVar[3];
	myform.CellBeta.value = "" + jsVar[4];
	myform.CellGamma.value = "" + jsVar[5];
	return true;
    }
    else {
	CrystalSymmetry = null;
	myform.CrystalFlag.checked=false;
	return false;
	//make cell size array as noncrystal
    }
}
function makeCellSize() {

    var flag = document.getElementById('inputs').CrystalFlag.checked;
    if (flag) {
	CrystalSymmetry = "P 1";
	makeCrystalCellSize();
    } else {
	CrystalSymmetry = null;
	makeAbstractCellSize();
    }
    drawMolInPreview();
}
function makeCrystalCellSize() {
    //Does not modify alpha/beta/gamma (can it?)
    var myform = document.getElementById('inputs');
    //base on the first model...
    var coordinates = sterilize(models[0]).split("\n");
    var xyzmin = [10000, 10000, 10000];
    var xyzmax = [-10000, -10000, -10000];
    for (i =0; i < coordinates.length; i++) {
	line = coordinates[i].split(" ");
	xyzmin[0] = Math.min(line[1], xyzmin[0]);
	xyzmin[1] = Math.min(line[2], xyzmin[1]);
	xyzmin[2] = Math.min(line[3], xyzmin[2]);
	xyzmax[0] = Math.max(line[1], xyzmax[0]);
	xyzmax[1] = Math.max(line[2], xyzmax[1]);
	xyzmax[2] = Math.max(line[3], xyzmax[2]);
    }
    var abc = [xyzmax[0]-xyzmin[0],
	       xyzmax[1]-xyzmin[1],
	       xyzmax[2]-xyzmin[2]];

    myform.CellA.value = "" + abc[0];
    myform.CellB.value = "" + abc[1];
    myform.CellC.value = "" + abc[2];
    //console.log(abc);
    return abc;
}
function makeAbstractCellSize() {
    var myform = document.getElementById('inputs');

    var coordinates = [];
    for (i = 0; i < models.length; i++) {
	//console.log(sterilize(models[i]).split("\n"));
	coordinates = coordinates.concat(sterilize(models[i]).split("\n"));
    }

    var xyzmin = [10000, 10000, 10000];
    var xyzmax = [-10000, -10000, -10000];
    for (i =0; i < coordinates.length; i++) {
	line = coordinates[i].split(" ");
	if (line.length < 3) continue;
	xyzmin[0] = Math.min(line[1], xyzmin[0]);
	xyzmin[1] = Math.min(line[2], xyzmin[1]);
	xyzmin[2] = Math.min(line[3], xyzmin[2]);
	xyzmax[0] = Math.max(line[1], xyzmax[0]);
	xyzmax[1] = Math.max(line[2], xyzmax[1]);
	xyzmax[2] = Math.max(line[3], xyzmax[2]);
    }
    var abc = [Math.max(MIN_CELL_SIZE, xyzmax[0]-xyzmin[0]+CELL_EXPAND_FACTOR),
	       Math.max(MIN_CELL_SIZE, xyzmax[1]-xyzmin[1]+CELL_EXPAND_FACTOR),
	       Math.max(MIN_CELL_SIZE, xyzmax[2]-xyzmin[2]+CELL_EXPAND_FACTOR),
	       90, 90, 90];

    myform.CellA.value = "" + abc[0];
    myform.CellB.value = "" + abc[1];
    myform.CellC.value = "" + abc[2];
    myform.CellAlpha.value = "" + abc[3];
    myform.CellBeta.value = "" + abc[4];
    myform.CellGamma.value = "" + abc[5];
    //console.log(abc);
    return abc;
}
function readCoordsFromJmol() {
    //fix to check for multiple models being uploaded
    var modelInfo = Jmol.getPropertyAsArray(previewApplet, "modelInfo", "all");
    var modelNum = modelInfo.modelCount;
    var i = 1;
    while (i <= modelNum) {
	var coords = "";
	var atoms = Jmol.getPropertyAsArray(previewApplet, "atomInfo", "1." + i);
	//console.log(atoms);
	for (atom = 0; atom < atoms.length; atom++) {
	    coords += atoms[atom].sym + " ";
	    coords += atoms[atom].x + " ";
	    coords += atoms[atom].y + " ";
	    coords += atoms[atom].z + " ";
	    coords += "\n";
	}
	//Upload to the active model...
	models.push(coords);
	i++;
    }
    //Update the coords div.
    makeCoordsDiv();
}
//Upload coordinates directly to coordinates text box, and preview box.
function uploadCoordinates() {
    form = document.getElementById('inputs');
    var file = form.uploadfile.files[0];
    //console.log(file);

    var filename = form.uploadfile.value;
    filename = filename.replace("C:\\fakepath\\", "");//Chrome Bug
    var name = filename.split("//");
    name = name[name.length-1].replace(/\..*/, "");

    form.MOLNAME.value = name;

    if (typeof FileReader !== "undefined") {
	var reader = new FileReader();

	//level supercell
	$('#SupercellX').val(1);
	$('#SupercellY').val(1);
	$('#SupercellZ').val(1);

	try {
	    reader.onload = function(r) {
		var filedata = String(reader.result);
		var scr = "try{\nLOAD INLINE \"";
		scr += filedata;
		scr +="\";}catch(e){}";

		Jmol.script(previewApplet, scr);

		var gotCrystal = tryToGrabCrystalData();
		readCoordsFromJmol();
		if (!gotCrystal) {
		makeAbstractCellSize();
		} else {
		    runThroughGdisAndReload(filedata, filename);		    
		}
		
		//switchToModel(models.length-1);
	    }
	    reader.readAsText(file);
	} catch(err) {
	    form.coordinates.value="ERROR";
	    return;
	}
    } else {
	alert("Your browser does not support file uploading, please use google chrome or firefox.");
    }
}

/*Push file to server		      
  Execute Gdis
  return file to jmol
  remove files
  quit
*/
function runThroughGdisAndReload(filedata, filename) {
    //Clean dirty charcters from filename
    filename = filename.replace(/[|&;$%@"<>()+,]/g, "");
    //"//Emacs coloring bug (Serves no purpose other then making my screen look nicer)

    //post webdata
    $.newt_ajax({type: "PUT", 
		url: "/file/"+"hopper" + CODE_BASE_DIR + DATA_LOC + "/tmp/" + filename,
		data: filedata,
		success: function(res) {runGdis(filedata, filename);},});
}
function runGdis(filedata, filename) {
    command = SHELL_CMD_DIR + "convertUsingGdis.sh " + filename + " .xyz";
    $.newt_ajax({type: "POST",
		url:"/command/hopper",
		data: {"executable": command},
		success: function(res) {console.log(res);reloadGdisOutput(filedata, filename);},});
}
function reloadGdisOutput(filedata, filename) {
    var convertedFile = "../Shirley-data/tmp/"+filename+".xyz";
    var scr = "try{load "+convertedFile+";}catch(e){};";
    //Draw to Jmol
    Jmol.script(previewApplet, scr);
    //rewrite new coordinates
    models = models.slice(0, -1);
    readCoordsFromJmol();
    switchToModel(models.length-1);
    //Get rid of temporary file
    deleteGdisOutputFile(filename);
}
function deleteGdisOutputFile(filename) {
    command = "/bin/rm " + CODE_BASE_DIR + DATA_LOC + "/tmp/" + filename + ".xyz";
    $.newt_ajax({type: "POST",
		url:"/command/hopper",
		data: {"executable": command},
		success: function(res) {console.log("successful delete of gdis output");},});
}


function getUnitCell() {
    var myform = document.getElementById('inputs');
    var a = myform.CellA.value;
    var b = myform.CellB.value;
    var c = myform.CellC.value;
    var alp = myform.CellAlpha.value;
    var bet = myform.CellBeta.value;
    var gam = myform.CellGamma.value;
    //a = a * Math.sin(alp*Math.PI/180);
    //b = b * Math.sin(bet*Math.PI/180);
    //c = c * Math.sin(gam*Math.PI/180);Not correct transform
    var vector = "{"+a+" "+b+" "+c+" "+alp+" "+bet+" "+gam+"}";
    var offset = "{"+(a/2.0)+" "+(b/2.0)+" "+(c/2.0)+"}";
    //console.log(vector);
    var scr = "";

    if (!CrystalSymmetry) {
	scr += " {1 1 1} ";
	scr += "unitcell " + vector;
	scr += " offset " + offset;
    } else {
	scr += " {1 1 0} ";
	scr += "spacegroup \""+ CrystalSymmetry + "\"";
	scr += " unitcell " + vector;
    }
    //console.log(scr);
    return scr;
}
function initPreviewApp() {
    setTimeout("drawMol('preview');$('#previewLoadingText').hide();", 100);
    //This is a weird first load issue, that seems to be solved by a short timeout call.
    //allows selections to work (about) immediately, dom issue?
}
function unbindMobileClicks() {
    var scr = "";
    scr += "unbind 'RIGHT';";
    scr += "unbind 'LEFT' '_clickFrank'; ";
    return scr;
}

function drawMolInPreview() {
    //Set Animation Button
    $('#animatePreviewButton').show();
    $('#animatePreviewStop').hide();

    //Draw Molecule
    var scr = "";
    if (amIMobile) {scr += unbindMobileClicks();}
    
    scr += "set defaultLattice {1 1 1}; ";
    //change to load all of the models....
    var xyz = makeXYZfromCoords(activeModel);
    scr += "xyz = \"" + xyz + "\";";
    //Open Try on successful load
    scr += "try{\nLOAD \"@xyz\" ";
    scr += getUnitCell() + ";";
    scr += "selectionHalos on;";
    scr += "set PickCallback \"jmolscript:javascript selectionCallback();\";";
    scr += "set picking select atom;";
    scr += "unitcell ON; javascript addSelections(); refresh;";
    scr += "}catch(e){echo e;}";
    //console.log(scr);
    Jmol.script(previewApplet, scr);
    $(document).ready(addSelections());
}

//Redraw the molecule according to coordinates
function drawMol(suffix) {
    if (suffix == 'preview') {
	drawMolInPreview();
    } else {
	var xyz = makeXYZfromCoords();
	var scr = "";
	if (amIMobile) {scr += unbindMobileClicks();}
	scr += "try{\nLOAD DATA \"mod\"\n" + xyz + "\nEND \"mod\" {1, 1, 1}}catch(e){}";
	Jmol.script(mainApplet, scr);
    }
}

function addSelections() {
    //Show changed selections on preview
    var XAS = document.getElementById('inputs').XASELEMENTS.value;
    XAS = XAS.split(" ");
    var scr = "select ";
    for (var e = 0; e < XAS.length; e++) {
	element = XAS[e];
	if (element.match(/^[a-zA-Z]{1,2}\d+$/) != null) {
	    scr += element + " OR ";
	}
	if (element.match(/^[a-zA-Z]{1,2}$/) != null) {
	    scr += "_" + element + " OR ";
	}
    }
    scr += "none";
    //console.log(scr);
    Jmol.script(previewApplet, scr);
}
function selectionCallback() {
    var atoms = Jmol.getPropertyAsArray(previewApplet, "atomInfo", "selected");
    var XAS = "";
    for (a = 0; a < atoms.length; a++) {
	atom = atoms[a];
	XAS += atom.sym + atom.atomno + " ";
    }
    document.getElementById("inputs").XASELEMENTS.value = XAS;
    //edit so Knows to write "C" if all C's are selected
}
function animatePreview() {
    //Change Button
    $('#animatePreviewButton').hide();
    $('#animatePreviewStop').show();

    //Run Script
    var scr = "unbind 'RIGHT';";
    scr += "unbind 'LEFT' '_clickFrank'; "; 
    scr += "set defaultLattice {1 1 1}; ";
    scr += "xyz = \"";
    for (var i = 0; i < models.length; i++){
	    var xyz = makeXYZfromCoords(i);
	    scr += xyz + "\n";
    }
    scr += "\";";
    scr += "try{\nLOAD \"@xyz\" ";
    scr += getUnitCell() + ";";
    scr += "selectionHalos on; ";
    scr += "set PickCallback \"jmolscript:javascript selectionCallback();\";";
    scr += "set picking select atom;";
    scr += "unitcell ON;";
    scr += "javascript addSelections();";
    scr += "animation mode LOOP 1.0; frame PLAY;";
    scr += "}catch(e){}";
    //console.log(scr);
    Jmol.script(previewApplet, scr);
}
function supercellPreview() {
    var inx = Number($('#SupercellX').val());
    var iny = Number($('#SupercellY').val());
    var inz = Number($('#SupercellZ').val());

    var scr = "unbind 'RIGHT';";
    scr += "unbind 'LEFT' '_clickFrank'; "; 
    scr += "set defaultLattice {1 1 1}; ";
    //change to load all of the models....
    var xyz = makeXYZfromCoords(activeModel);
    scr += "xyz = \"" + xyz + "\";";
    //Open Try on successful load
    scr += "try{\nLOAD \"@xyz\" ";
    
    var myform = document.getElementById('inputs');
    var a = myform.CellA.value;
    var b = myform.CellB.value;
    var c = myform.CellC.value;
    var alp = myform.CellAlpha.value;
    var bet = myform.CellBeta.value;
    var gam = myform.CellGamma.value;
    var vector = "{"+a+" "+b+" "+c+" "+alp+" "+bet+" "+gam+"}";
    var offset = "{"+(a/2.0)+" "+(b/2.0)+" "+(c/2.0)+"}";
    var cellParams = "{" + "555" + " " + (4+inx) + (4+iny) + (4+inz) + " 1}";

    console.log(cellParams);
    if (!CrystalSymmetry) {
	scr += " " + cellParams + " ";
	scr += "unitcell " + vector;
	scr += " offset " + offset;
    } else {
	scr += " " + cellParams + " ";
	scr += "spacegroup \""+ CrystalSymmetry + "\"";
	scr += " unitcell " + vector;
    }

    scr += "selectionHalos on; ";
    scr += "set PickCallback \"jmolscript:javascript selectionCallback();\";";
    scr += "set picking select atom;";
    scr += "unitcell ON;";
    scr += "unitcell " + cellParams + "; ";
    scr += "javascript addSelections(); refresh;";
    scr += "}catch(e){}";
    //console.log(scr);
    Jmol.script(previewApplet, scr);
}

//Jmol Functions for results
function resultsReady() {
    resultsAppReady = true;
    console.log("ready");
}

var showUnitcellFlag = false;
function redrawModel(dir, jobName) {
     var model = $('#currModel').val() - 1;
     var file = dir +"/" + jobName + "_"+model+".xyz";
     $.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		 res = res + ""; 
		 res = res.replace(/\r\n|\r|\n/g, "\n");
		 
		 var scr = "";
		 if (amIMobile) {scr += unbindMobileClicks();}
		 scr = "try{mod = '" + res + "';";
		 scr += "\nLOAD '@mod'";
		 if (showUnitcellFlag) {
		     var unitcellinfo = $('#unitcellData').text().replace(/,/g," ").replace(/[\[\]]/g,"");
		     var oscalc = unitcellinfo.split(" ");
		     var offsetinfo = oscalc[0]/2 + " " + oscalc[1]/2 + " " + oscalc[2]/2;			 
		     scr += " {1 1 1} unitcell {" + unitcellinfo + "} offset {" + offsetinfo + "};";
		 }
		 console.log(scr);
		 scr += ";reset mod;selectionHalos on;select none;}catch(e){}";
		 if (resultsAppReady) {Jmol.script(resultsApplet, scr);}
	     },});
}

function animateResults(dir, numModels) {
     var jobName = $('#jobName').text();
     var modata = "";
     var count = 0;
     for (var i = 0; i < numModels; i++) {
	 var model = i;
	 var file = dir +"/" + jobName + "_"+model+".xyz";
	 $.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		 res = res + ""; 
		 res = res.replace(/\r\n|\r|\n/g, "\n");
		 modata += res;
		 count ++;
		 if (count == numModels) {
		     var scr = "try{mod = '" + modata + "';";
		     console.log(scr);
		     scr += "\nLOAD '@mod';reset mod;selectionHalos on;select none;animation mode LOOP 1.0; frame PLAY;}";
		     if (resultsAppReady) {Jmol.script(resultsApplet, scr);}
		 } else {
		     modata += "\n";
		 }
		 },});
     }
}

function unitcell(dir, jobName) {
    showUnitcellFlag = !showUnitcellFlag;
    redrawModel(dir, jobName);
}
function savePNG() {
    var scr = "write IMAGE 300 300 PNG 2 " + $('#jobName').text() + ".png";
    if (resultsAppReady) Jmol.script(resultsApplet, scr);
}
function savePOV() {
    var scr = "write POVRAY " + $('#jobName').text() + ".pov";
    console.log(scr);
    if (resultsAppReady) Jmol.script(resultsApplet, scr);
}