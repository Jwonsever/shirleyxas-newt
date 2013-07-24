/*
James Wonsever
Lawrence Berkeley Laboratory
Molecular Foundry
05/2012 -> Present

James Wonsever & Justin Patel
Lawrence Berkeley Laboratory
Molecular Foundry

All Ajax and functionality scripts for the WebXS interface.
These scripts generally are used to send and retrieve data from NERSC computers.
 */

function checkAuthCallback() {
    //where to output files to for run jobs
    $('#outputDir').val(GLOBAL_SCRATCH_DIR + myUsername);
    $('#customInputBlock').val(GLOBAL_SCRATCH_DIR + myUsername + "/CustomBlock.in");
	
    //See if there is a transfer file
    openTransferFile();
    getUserInfo();
}

function getUserInfo() {
     $.newt_ajax({type: "GET", 
		 url: "/account/usage/user/"+myUsername,
		success: function(res) {
		console.log(res['items'][0]['rname']);
		$('#acctHours').val(res['items'][0]['rname']);
	},});
}

//Globals for multiple models and related functions, to control jmol and submission of multiple files...
var models = ["C -0.00025 -0.00025 -0.00025\nH 0.64018 0.64018 0.64018\nH -0.64075 -0.64075 0.64049\nH -0.64075 0.64049 -0.64075\nH 0.64049 -0.64075 -0.64075"];
activeModel = 0;

//Lists all of the jobs currently running on Hopper, by ajax qstat.
var machines=["hopper"]; //var machines=["hopper", "carver", "dirac"];
var autoInterval = 0;
function runningJobs() {

    var lastUpdate = new Date();
    var myText = "<h3>Running Calculations</h3><div align=left>";

    myText+="<div id='runningTable'>";
    myText+="<div id='ajaxLoader'><center><img src=\"ajax-loader-2.gif\" width=40></center></div>";
    myText+="<br></div><div id='errorLog' style='background-color:#8994b0;display:none;'>Error Log:<br></div>";
    myText+="<br>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    myText+="As of: <span id='lastDate'> ";
    myText+=" </span> &nbsp;&nbsp;<button onClick='cumulativeRunningJobs()'>Update</button>";

    cumulativeRunningJobs();
    window.clearInterval(autoInterval);
    autoInterval = window.setInterval(function(){cumulativeRunningJobs();}, 16000);

    $('#runningjobs').html(myText);
    $('#runningjobs').trigger('create');

}
//Run an Ajax command for the given machines, to get all running jobs.
function cumulativeRunningJobs() {
    var total = machines.length;
    var count = 0;
    var cumres = [];
    $('#ajaxLoader').html('<center><img src=\"ajax-loader-2.gif\" width=40><center>');
    for (m in machines) {
	var machineName = machines[m];
	if (typeof machineName != 'string') continue;
        //alert("/queue/"+machineName+"/?user="+myUsername)
	$.newt_ajax({type: "GET",
		    url: "/queue/"+machineName+"/?user="+myUsername,
		    success: function(res){
		        cumres = cumres.concat(res);
			count++;
			if (total == count) {
			    //console.log(cumres);
			    var resHTML=runningCalcs(cumres);
			    $('#runningTable').html(resHTML);
			    var dt = new Date();
			    $('#lastDate').text(dt.toString());
			    $('#ajaxLoader').html('');
			}
		},
		    error: function(request,testStatus,errorThrown) {
		        //console.log(request);
		        var dt = new Date();
			$('#lastDate').text(dt.toString());
			$('#ajaxLoader').html('');
			$('#errorLog').append("Unable to reach server. "+testStatus+": "+errorThrown+"<br>");
			$('#errorLog').show();
		},
	    });
    }
}
//Ajax success function. Tables the jobs, kill feature, etc.
//Writes the HTML
function runningCalcs(res) {
    var myText = "";
    if (res != null && res.length > 0) {
		
	myText += "<table width=100\%><tr><th width=53\% style='text-align:center;'>Job Name</th>";
	myText += "<th width=10\% style='text-align:center;'>Status</th>";
	myText += "<th width=25\% style='text-align:center;'>Machine</th><th></th></tr></table>";
	myText += "<br><table width=100\% cellpadding=5 class='table table-bordered'>";
	
	for (var i = 0 ; i < res.length ; i++) {
	    if (res[i].user == myUsername) {
		    
		myText += "<tr class='listitem'>";
		    
		if (res[i].status == "R"){
		    myText += "<td width=60\%>" + res[i].name + "</td><td width=10\% class=\"statusup\" align=center>"
			+ res[i].status + "</td><td width=30\%><center>"+res[i].hostname+"</center></td><td align=center><button onClick=\"killJob(\'"
			+ res[i].jobid +"\')\" type=\"button\" >Cancel</button></td>";
		} else {
		    myText += "<td width=60\%>" + res[i].name + "</td><td width=10\% class=\"statusnone\" align=center>"
			+ res[i].status + "</td><td width=30\%><center>"+res[i].hostname+"</center></td><td align=center><button onClick=\"killJob(\'"
			+ res[i].jobid +"\')\" type=\"button\" >Cancel</button></td>";
		}
		myText += "</tr>";
	    }
	}
	return myText;
    } else {
	myText += "<table width=100\%><tr><td align=left>No running jobs.</td></tr></table>";
	return myText;
    }
}
function killJob(job) {
    var jobid = job.replace(".hopper11",""); // was .sdb
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable":"/opt/torque/4.2.3.1/bin/qdel "+jobid},
		success: function(res){
		console.log("Job deleted. It may take a few minutes for status to update.");
	    },
		error: function(request,testStatus,errorThrown) {
		console.log("Failed to kill job.\n"+testStatus+":\n"+errorThrown);
	    },
		});
}


//Previous jobs javascript

//Returns true if the name is of the same format as the output of this tool.
//Used to validate that the job came from out tool and has a proper timestamp.
function properName(name) {
    return name.match(/^.*\d{2}\w{3}\d{10}$/);
}

//This command loads the "Finished Calculations" screen.
function previousJobs() {
    $('#previousjobslist').html("<div id='previousUnfinishedJobs'>"
				+ "<h3>Unfinished Calcualtions Available for Resubmission</h3><center>"
				+ "<img src=\"ajax-loader-2.gif\" width=40></center></div>"
				+ "<div id='previousFinishedJobs'><h3>Finished Calculations</h3>"
				+ "<center><img src=\"ajax-loader-2.gif\" width=40></center></div>");

    $.newt_ajax({type: "GET",
		url: "/file/hopper"+GLOBAL_SCRATCH_DIR+myUsername,
		success: function(res){
		if (res != null && res.length > 0) {
		    var myText = "<h3>Finished Calculations</h3>";
		    myText += "<button onclick='previousJobs()'>Update</button><br>"
		    myText += "<table width=100\%><tr><th width=62\% align=center>Job Name</th><th width=15\% align=center>Hours</th><th width=120></th></tr></table>";
		    myText += "<table width=100\% cellpadding=5 class='table table-bordered'>";
		    var files = res;

		    //Potentially rework this from a LS and text triggers on completed jobs??
		    //Would allow for directory searching of all jobs
		    //Would also be a strange result list OR many ajax calls, neither is kosher

		    var shortDir = GLOBAL_SCRATCH_DIR + myUsername;
		    var command = SHELL_CMD_DIR+"/exceededWalltimeWrapper.sh " + shortDir;
		    $.newt_ajax({type: "POST",
				url: "/command/hopper",
				data: {"executable": command},
				success: function(res){
			 	 unfText = "<h3>Unfinished Calcualtions Available for Resubmission</h3>"; 
				 unfText += "<button onclick='previousJobs()'>Update</button><br>"
				 unfText += "<table width=100\%><tr><th width=62\% align=center>Job Name</th><th width=15\% align=center>Hours</th><th width=120></th></tr></table>";
				 unfText += "<table width=100\% cellpadding=5 class='table table-bordered'>";
				 unfcount = 0;
				 var text = res.output.split("\n");
				 for (var i = 0; i < text.length; i++) {
				     var jname = text[i];

				     //Was it run using this tool?
				     if (!properName(jname)) continue;

				     unfcount = unfcount + 1;
				     if (unfcount%2 == 0) {	
					 unfText += "<tr class='warningodd'>";
				     } else {
					 unfText += "<tr class='warning'>";
				     }
				     unfText += "<td width=62\%>" + jname
					 + "</td><td class=\"statusnone\" width=15\% align=center>"
					 + "Unfinished</td><td><button onClick=\""
					 + "resubmit(\'" + jname + "\', \'hopper\');"
					 + "$(this).attr('disabled','disabled');\" type=\"button\">Resubmit</button></td><td>"
					 + "<button onClick=\"viewJobFiles(\'" 
					 + jname + "\', \'" + "hopper"
					 + "\')\" type=\"button\">View Files</button></td><tr>";
				 }
				 unfText += "</table><br>";
				 $('#previousUnfinishedJobs').html(unfText);

			    },
				error: function(request,testStatus,errorThrown) {
				console.log("Failed to grab incomplete jobs.");
			    },});


		    $.newt_ajax({type: "GET",
				url: "/queue/completedjobs/"+myUsername+"&limit=1500",
				success: function(res){
				if (res != null && res.length > 0) {
				    //console.log(res);

				    var occurs =[];
				    var fnames =[];
                                    var mycount = 0;
				    for (var i = 0 ; i < res.length ; i++) {					

					var fulljname = res[i].jobname;
					var jname = fulljname.replace("-ANALYSE","").replace("-REF","").replace("-XAS","");

					//Is this a repeat occurance?  If so, add hours and leave.
					if (fnames.indexOf(fulljname) == -1) {
					    fnames.push(fulljname);
					} else {
					    if (occurs[jname]) {
						occurs[jname][1]+= Number(res[i].rawhours.replace(",",""));
					    }
					    continue;
					}
					
					//Was it run using this tool?
					if (!properName(jname)) continue;
					
					//Does it still exist?
					var suc = false;
					for (var j = 0; j < files.length; j++) {
					    if (files[j].name == jname) {suc = true;break;}
					} if (!suc) continue;

					//does it finish all 3 tasks?  (note resubmissions can screw this up, not full wallhours/double posting)
					if(occurs[jname]) {
					    occurs[jname][0]++; //log hours, occurances
					    occurs[jname][1]+= Number(res[i].rawhours.replace(",",""));
					} else occurs[jname]=[1, Number(res[i].rawhours.replace(",",""))];
					if (!(occurs[jname][0] == 3)) continue;
			
					//Passes, display this job!
					
                                        mycount = mycount + 1;
                                        if (mycount%2 == 0) {	
 					  myText += "<tr class='warningodd'>";
                                        } else {
 					  myText += "<tr class='warning'>";
                                        }
					
					myText += "<td width=62\%>" + jname
					    + "</td><td class=\"statusnone\" width=15\% align=center>"
					    + (occurs[jname][1]+'').substr(0, 6)
					    + "</td><td><button onClick=\"viewJob(\'" 
					    + jname + "\', \'" + res[i].hostname
					    + "\')\" type=\"button\">View Results</button></td>"
					    + "<td><button onClick=\"viewJobFiles(\'" 
					    + jname + "\', \'" + res[i].hostname
					    + "\')\" type=\"button\">View Files</button></td>";
					myText += "</tr>";
				    }
				    myText += "</table><br>";
				    $('#previousFinishedJobs').html(myText);
				    $('#previousFinishedJobs').trigger('create');
				    
				} else {
				    $('#previousFinishedJobs').html("<table width=100\%><th align=left>"
							    + "Finished Calculations</th>"
							    + "</table><center><br>You don't"
							    + " have any finished calculations");
				}
			    },
				error: function(request,testStatus,errorThrown) {
				var myText = "Error: "+testStatus+" \n"+errorThrown;
				$('#previousFinishedJobs').html(myText);
				$('#previousFinishedJobs').trigger('create');},});
		} else {
		    $('#previousFinishedJobs').html("<table width=100\%><th align=left>Finished Calculations</th>"
					    + "</table><center><br>You don't have any finished calculations");
		}
	    },
		error: function(request,testStatus,errorThrown) {
		var myText = "Error: "+testStatus+" \n"+errorThrown;
		$('#previousjobslist').html(myText);
		$('#previousjobslist').trigger('create');
	    },
		});
}

//Rubmission function
function resubmit(jobName, machine) {
    var shortDir = "/global/scratch/sd/" + myUsername + "/" + jobName;
    var command = SHELL_CMD_DIR+"resubmit.sh " + shortDir;
    //Post job.
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command},
		success: function(res){
		alert("successfully restarted.");
	    },
		error: function(request,testStatus,errorThrown) {
		alert("restart failed.");
	    },});
}

//view results function
function individualJobOutput(jobName, machine) {
     if (!machine) machine = "hopper";
     $('#jobHeader').html("<h3>Results for " + jobName + "</h3><center><img src=\"ajax-loader-2.gif\" width=40 ></center>");
     //Show Ajax Loader as it writes the html

     activeElement = undefined;//reset activeElement
     machine = machine.toLowerCase();
     var shortDir = "/global/scratch/sd/" + myUsername;
     var directory = "/file/" + machine + "/global/scratch/sd/" + myUsername + "/"+jobName;
     var webdata = directory + "/webdata.in";
     var myHtml = ""

     $.newt_ajax({type: "GET",
		 url: webdata + "?view=read",
		 success: function(res){
		 webdata = res.split(/\r\n|\r|\n/);
		 myHtml += "<div id='jobName' style='display:none;'>"+jobName+"</div>";//Save the jobname on the webpage
		 myHtml += "<div id='xasAtoms' style='display:none;'>"+webdata[2]+"</div>";//Save the XAS atoms on the webpage
		 myHtml += "<div id='unitcellData' style='display:none;'>"+webdata[3]+"</div>";//Save the Unitcell on the webpage
		 myHtml += "<div id='totalAtoms' style='display:none;'>"+webdata[5]+"</div>";//Save the total atoms on the webpage
		 myHtml += "<h3>Results for " + webdata[0].slice(0, -15) + "</h3>";
		 var myTime = new Date(parseInt(webdata[1]));
		 myHtml += "<table><tr><td align=left>Run "+myTime.toLocaleString()+"</td>";
		 myHtml += "<td width=5></td><td align=right><button onClick=switchToPrevious()>Other Calculations</button>&nbsp;&nbsp;";
		 myHtml += "<button onClick=individualJobWrapper('"+jobName+"','"+machine+"')>View Files</button>&nbsp;&nbsp;	";
		 myHtml += "<button onClick=deleteJobFiles('"+shortDir+"','"+machine+"')>Delete Job</button></td></tr>";
		 myHtml += "</table>";

		 $('#jobHeader').html(myHtml);
		 loadJobOutputs(myHtml, directory, jobName, webdata);
	     },
		 error: function(request,testStatus,errorThrown) {
		 //This caters to old or possibly maunally run jobs (as best as possible)
		 myHtml += "<h3>Results for " + jobName + "</h3><table><tr><td align=right><button onClick=switchToPrevious()>Other Calculations</button><button onClick=individualJob('"+jobName+"','"+machine+"')>View Files</button></td></tr></table><br>";
		 $('#jobHeader').html(myHtml);
		 alert("no webdata, cannot view this job.");
		 ;}//findJobOutputs(myHtml, directory, jobName);}
	     ,});
}

var activeElement;//Which element is loaded by these jobOutputs (controlled by updateSpectraOptions(elem))
function loadJobOutputs(myHtml, directory, jobName, webdata)
{
    var XAS = webdata[2].split(" ");//All excited atoms & elements
    var numModels = Number(webdata[4]);
    var template = myHtml;

    if (activeElement == undefined) {
	activeElement = XAS[0].replace(/\d/g,''); //Default to the first excited element (type of)
    }

    var XASElements = []; //All excited elements
    for (var i = 0; i<XAS.length; i++) {	
	var element = XAS[i];
	if (element.match(/^[a-zA-Z]{1,2}\d+$/) || element=='') continue;
	XASElements.push(element);
    }
    
    myHtml = "View Spectra of: &nbsp;&nbsp;";
    for (var x = 0; x<XASElements.length; x++) {
	var xe = XASElements[x];
	myHtml+= '<input type="button" class="mybutton" ';
	myHtml+= 'onclick=updateSpectraOptions("'+xe+'","'+directory+'") value="'+xe+'"/>';
    }

    myHtml += "<br><br><div id=\"flotwrapper\"><center><b>Core Excitation Spectra of <span id='titleElem'>" + activeElement + "</span></b></center>";
    myHtml += "<center><div id=\"placeholder\"><center><img src=\"ajax-loader-2.gif\" width=40></center></div></center></div>";

    var XCHShift = getXCHShift(activeElement);
    var XCHtml = "<input id='XCH' size='5' class='field span1' type='textbox' onchange='addShift()' value= "+XCHShift+" />";
    var nb15 = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
    myHtml += "<center><b>(eV)</b></center><br>"
    myHtml += "<button onClick='savePlotFile(\""+directory+"\")'>Save Plot</button></span>";
    myHtml += nb15+"<span id='xchs'>XCH Shift:&nbsp;&nbsp;&nbsp;"+XCHtml+"</span><br><br>";

    myHtml += "<div id=\"flotOptions\">Show Excitation Spectrum of: <span id='selectableXAS'>";
    for (i in XAS) {
	var element = XAS[i];
	if (typeof element != 'string') continue;
	var regex = new RegExp("^" + activeElement + "\\d*$");
	if (!element.match(regex)) continue;
	myHtml += "&nbsp;&nbsp;&nbsp;<input id='spect"+i.toString()+"' name=\""+element.toString()+"\" type='checkbox' onchange='makePlotWrapper(\""+directory + "\",\""+jobName + "\")'/>" + element;
    }
    myHtml +="</span><br>";

    myHtml += "Show Variation by Dimension? <input id='dimensional' type='checkbox' onchange='makePlotWrapper(\""+directory + "\",\""+jobName + "\")'/>";
    myHtml += "<br>Show Models: ";
    for (i = 1; i <= numModels; i++) {
	myHtml += "<input id='model"+i+"' name=\""+i+"\" type='checkbox' onchange='makePlotWrapper(\""+directory + "\",\""+jobName + "\")'/>" + i;
    }

    console.log(numModels);
    myHtml += "</div>";
    
    var jmolOptsHtml = "<b>Jmol Options:</b>";
    jmolOptsHtml += "<br>View Model <select id = 'currModel' onchange='redrawModel(\""+directory + "\",\""+jobName + "\")'>";
    for (var i = 1; i <= numModels; i++) {
	jmolOptsHtml +="<option value="+i+">"+i+"</option>";
    }
    jmolOptsHtml += "</select><br>";
    jmolOptsHtml += "&nbsp;&nbsp;<button onclick='animateResults(\""+directory+"\",\""+numModels+"\")'>Animate</button>";
    jmolOptsHtml += "&nbsp;&nbsp;<button onclick='unitcell(\""+directory+"\",\""+jobName+"\")'>Unitcell</button>";
    jmolOptsHtml += "&nbsp;&nbsp;<button onclick='savePNG()'>Save IMG</button>&nbsp;&nbsp;<button onclick='savePOV()'>POV</button>";
    $('#jmolOptions').html(jmolOptsHtml);

    
    myHtml += "<div id='postprocessing' style='width=600px;overflow:auto;'></div>";
    $('#states').html("");
    //console.log(myHtml);

    $('#jobresults').html(myHtml);
    $('#jobresults').trigger('create');


    //Postprocessing
    var ppHtml = "<h4>Postprocessing Options</h4>";
    var stHtml = "<b>View States: </b>";
    
    var cmnd = SHELL_CMD_DIR + "topEvWrapper.sh " + directory.slice(12) + " " + jobName + " ";
    ppHtml += "<b>Energy </b><input id='x' size='6' type='textbox' value='0'>";
    ppHtml += "&nbsp;&nbsp;<button onclick='getStates(\""+cmnd+"\")'>Get States</button><br>";
    ppHtml += "<br><button onclick='saveOutput(\""+directory+"\",\""+jobName+"\")'>";
    ppHtml += "Save This Spectra To MFTheory Database</button><br>";
    
    stHtml += "<button style='float: right;' onclick=\"$('#customStateForm').show();\">Load Custom State</button><br><br>";
    stHtml += "<div id='topStates' style='min-height:100px;width:100\%;padding:0.1em;margin: 0.1em;border-style:solid;border-width:1px;'>No contributing states.</div>";
    stHtml += "Note, these will take a few minutes to load.";
    
    //This is hidden unless the user opens it
    stHtml += "<div id='customStateForm' style='text-align:center;display:none;'>";
    stHtml += "Atm#<input id='cuAt' type='text' size='4' value='"+XAS[0]+"'/> ";
    stHtml += "M#<input id='cuMo' type='text' size='4' value='1'/> ";
    stHtml += "St#<input id='cuSt' type='text' size='4' value='1'/><br>";
    stHtml += "<button onclick=\"$('#customStateForm\').hide();\">Cancel</button> ";
    stHtml += "&nbsp;&nbsp;<button onclick=\"postProcessing($('#cuAt').val(), $('#cuMo').val()-1, $('#cuSt').val());";		
    stHtml += "\">Run</button> ";
    stHtml += "&nbsp;&nbsp;<button onclick=\"drawState($('#cuAt').val(), $('#cuMo').val()-1, $('#cuSt').val());"
    stHtml += "\">View</button></div>";
    
    $('#postprocessing').html(ppHtml);
    $("#states").html(stHtml); 

    //Find Crashes 
    var shortDir = "/global/scratch/sd/" + myUsername + "/" + jobName;
    var findCrash = "/usr/bin/find " + shortDir + " -type f -name 'CRASH'";
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": findCrash},
		success: function(res){
		console.log(res);
		//Has a crash file in ground state.
		if (res.output.replace(/\n/g,'') != "") {
		    $('#postprocessing').append("<h3>This job crashed!</h3>Crash Files Here:<br>"+res.output);
		} else {
		//no crash file in ground state, give postprocessing options
		}
	    },
		error: function(request,testStatus,errorThrown) {
		console.log("failed to run find command, no knowledge of crash state");
	    },});
    
    //Find Missing Pseudos
    var findMissingPseudos = SHELL_CMD_DIR + "missingPseudos.sh " + shortDir;
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": findMissingPseudos},
		success: function(res){
		console.log(res);
		if (res.output.replace(/\n/g,'') != "") {
		    $('#postprocessing').append("<h3>Missing Pseudo!</h3>"+res.output);
		}
	    },
		error: function(request,testStatus,errorThrown) {
		console.log("failed to run missingPseudos, no knowledge of pseudo state");
	    },});

    //Find MPI Errors
    var findMPI = SHELL_CMD_DIR + "mpiErrors.sh " + shortDir;
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": findMPI},
		success: function(res){
		console.log(res);
		if (res.output.replace(/\n/g,'') != "") {
		    $('#postprocessing').append("<h3>MPI Errors!</h3>"+res.output);
		}
	    },
		error: function(request,testStatus,errorThrown) {
		console.log("failed to run MPIerrors, no knowledge of MPI results");
	    },});


    $("#placeholder").bind("plothover", function (event, pos, item) {
	    $("#x").val(pos.x.toFixed(2));       
	});
    $("#placeholder").bind("plotclick", function (event, pos, item) {
	    var cmnd = SHELL_CMD_DIR + "topEvWrapper.sh " + directory.slice(12) + " " + jobName;
	    getStates(cmnd, (pos.x.toFixed(3)-getXCHShift(activeElement)));
	});

    loadJmolResultsApp(directory, jobName);

    //Select first spectra and draw
    $("#spect0").prop("checked", true);
    $("#model1").prop("checked", true);
    $(document).ready(makePlotWrapper(directory, jobName));
}
  
function deleteJobFiles(dir, machine) {
    var molName = $('#jobName').text();
    if (!confirm('Are you sure?  All files will be permanently deleted.')) return;
    command = SHELL_CMD_DIR+"rmFileDir.sh " + dir + " " + molName;
    $.newt_ajax({type: "POST",
		url: "/command/" + machine,
		data: {"executable": command},
		success: function(res) {switchToPrevious();},});
}
//Updates the page on change of activeElement
function updateSpectraOptions(elem, directory) {
    //Throw Loading img.
    $('#placeholder').html("<center><img src=\"ajax-loader-2.gif\" width=40></center>");

    var myHtml = "";

    activeElement = elem;
    var jobName = $('#jobName').text();
    $('#titleElem').text(activeElement);

    var XAS = $('#xasAtoms').html().split(" ");//All excited atoms & elements

    var firstInit;
    for (i = 0; i < XAS.length; i++) {
	 var element = XAS[i];
	 var regex = new RegExp("^" + activeElement + "\\d*$");
	 if (!element.match(regex)) continue;
	 if (!firstInit) firstInit = "#spect"+i;
	 myHtml += "<input id='spect"+i+"' name=\""+element.toString()+"\" type='checkbox' onchange='makePlotWrapper(\""+directory + "\",\""+jobName + "\")'/>" + element;
    }

    $('#selectableXAS').html(myHtml);
    $('#XCH').val(getXCHShift(activeElement));
    $(firstInit).prop("checked", true);
    $(document).ready(makePlotWrapper(directory, jobName));
}

//Gets the states close to this ev.
function getStates(cmnd, ev) {
    $("#topStates").html('<center><img src=\"ajax-loader-2.gif\" width=40></center>');
    if (ev == undefined) {
	ev = $("#x").val() - getXCHShift(activeElement);
    }
    cmnd += " " + ev;
    $.newt_ajax({type: "POST",
			 url: "/command/hopper",
			 data: {"executable": cmnd},
			 success: function(res){
		         $("#topStates").text(res.output);
			 var sts = res.output.slice(0, -1).split("\n");
			 console.log(sts);
			 if (sts[0] == "") $("#topStates").text("No contributing states.");
			 else {
			     var myHtml = "<div>Atom|Model|State|Ev|Str</div>"
                             myHtml += "<table cellspacing=5>"
			     for (s = 0; s < sts.length; s++) {
				 var line = sts[s].split(',');
				 var type = line[0].replace(/\d/g,'');
				 if (type != activeElement.replace(/\d/g,'')) continue;
				 //myHtml +="<div><span onclick='postProcessing(\""+line[0]+"\",\""+(line[1]-1)+"\",\""+line[2]+"\")'>&nbsp;&nbsp;";
                                 myHtml += "<tr>"
				 var evshift = roundNumber(Number(line[3]) + getXCHShift(activeElement), 2);
				 myHtml += "<td>"+line[0] + "|" + line[1] + "|" + line[2] + "|" + evshift + "|" + line[4] + "</td>";
                                 myHtml += "<td width=20></td>";
				 //Dont Give Run Option anymore (todo after new states)
				 myHtml += "<td><button onclick='postProcessing(\""+line[0]+"\",\""+(line[1]-1)+"\",\""+line[2]+"\")'>Run</button></td>";
				 myHtml += "<td><button onclick='drawState(\""+line[0]+"\",\""+(line[1]-1)+"\",\""+line[2]+"\")'>View</button></td></tr>";
			     }
                             myHtml += "</table>"
			     $("#topStates").html(myHtml);
			 }
		     },
			 error: function(request,testStatus,errorThrown) {
		         $("#topStates").html("Server Error: Failed to load states");
			 console.log("Failed!");
		     },});
}

//Simple rounding helper
function roundNumber(num, dec) {
	var result = Math.round(num*Math.pow(10,dec))/Math.pow(10,dec);
	return result;
}
//Postprocessing functions
function longAtom(atomNo) {
    var type = atomNo.replace(/\d/g,'');
    if ($('#totalAtoms').text().length <= atomNo.replace(type, '').length) return atomNo;
    return longAtom(type + "0" + atomNo.replace(type, ''));
}
//post Postprocessing state job
function postProcessing(atomNo, activeMo, state) {
    activeMo = Number(activeMo);
    
    //Check all of the inputs
    var xasAtoms = $('#xasAtoms').text();
    if (!(atomNo.match(/[A-Z][a-z]?\d+/) && xasAtoms.match(atomNo))) {
	alert("Atom must be one of the calculated atoms.  For example 'C1'.");
	return;
    }
    var myOpts = document.getElementById('currModel').options;
    console.log(myOpts.length);
    var optsVals = [];
    for (i in myOpts) optsVals.push(myOpts[i].value);
    console.log(optsVals);
    if ((Number(activeMo)+1) < myOpts.length && Number(activeMo) > 0) {
	alert("This model does not exist. Please choose a valid Model#.");
	return;
    }
    if (isNaN(Number(state)) || Number(state) < 0) {
	alert("This state does not exist. Please choose a positive integer value.");
	return;
    }

    var jobName = $('#jobName').text();
    var activeDir = "/global/scratch/sd/"+myUsername+"/" + jobName + "/XAS/" + jobName + "_"+activeMo+"/"+atomNo+"/";
    var prefix = jobName + '.' + longAtom(atomNo) + "-XCH";
    var command = SHELL_CMD_DIR+"pp.sh " + activeDir + " " + prefix + " 24 24 " + state + " " + state; //todo (advanced procs)
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command},
		success: function(res){
		console.log("Success!");
	    },
		error: function(request,testStatus,errorThrown) {
		alert("Illegal atom/band number");
		},});
}
//draw the molecular orbital
function drawState(atomNo, activeMo, state) {
    var jobName = $('#jobName').text();
    var activeDir = "/global/scratch/sd/"+myUsername+"/" + jobName + "/XAS/" + jobName + "_"+activeMo+"/"+atomNo+"/";
    var command = SHELL_CMD_DIR + "fetchState.sh " + activeDir + " " + jobName + " " + state;
    var scr = "zap;set echo top left;font echo 16;echo \"loading...\";refresh;";
    Jmol.script(resultsApplet, scr);

    $.newt_ajax({type: "POST",
		    url: "/command/hopper",
		    data: {"executable": command},
		    success: function(res) {
		     console.log("fetched");
		     if (res.output=="File does not exist\n") {
			 scr = "set echo top left;font echo 16;echo \"Load Failed!  State does not exist.\";refresh;";
			 Jmol.script(resultsApplet, scr);
		     }
		     var stateFile = "../Shirley-data/tmp/"+jobName+"/state."+state+".cube";
		     scr = "set echo top left;font echo 16;echo \"Reading...\";refresh;";
		     scr += "load "+stateFile+";";
		     scr += "isosurface pos .001 '"+stateFile+"';color isosurface translucent;";
		     scr += "isosurface neg -.001 '"+stateFile+"';color isosurface translucent;";
		     scr += "selectionHalos on;select "+atomNo+";javascript deleteStateFileFromServer();";
		     //Draw to Jmol
		     Jmol.script(resultsApplet, scr);
		     //Match state - active model
		     $('#currModel').val(Number(activeMo) + 1);
		     //should remove the tmpfile here...
	            },
	 	    error: function(request,testStatus,errorThrown) {
		     scr = "set echo top left;font echo 16;echo \"Load Failed!  State does not exist.\";refresh;";
		     Jmol.script(resultsApplet, scr);
		     console.log("Failed to get state!");
	    },});
    
}
//This Function stops pollution on the server while enabling state file loading
function deleteStateFileFromServer() {
    var jobName = $('#jobName').text();
    var command = SHELL_CMD_DIR + "wipeState.sh " + jobName;
    $.newt_ajax({type: "POST",
		    url: "/command/hopper",
		    data: {"executable": command},
		    success: function(res) {
		console.log("successful delete");
	    },
		    error: function(request,testStatus,errorThrown) {
		console.log("Failed!");
	    },});
}   

//XCH Shift Functions
var cachedShifts = {pulled:false, elements:{}};
function getXCHShift(elem) {
    if (cachedShifts.pulled) {
	if (cachedShifts.elements[elem.toString()] != undefined)
	    return cachedShifts.elements[elem.toString()];
	else return 0;
    } else {
	 $.newt_ajax({type: "GET",
		url: "/file/hopper" + DATABASE_DIR + "/XCHShifts.csv?view=read",
		success: function(res){
		     res = res.split("\n");
		     cachedShifts.pulled=true;
		     for (l = 1; l < res.length; l++) {
			 var line = res[l].split(",");
			 cachedShifts.elements[line[0]] = Number(line[1]);
		     }
		 },});
	return 0;
    }
}
function addShift() {
    //NOTE, Flot will not update till next spectra is chosen
    var elem = activeElement;
    var newShift =  Number(document.getElementById("XCH").value);
    if (!isNaN(newShift)) {
	cachedShifts.elements[elem.toString()] = newShift;
    }
}

//adds the selected spectra to database (example C, C1, N4)
//needs alot of work //todo
function saveOutput(dir, jobName) {
    var elems = getSelectedElems();
    for (var e = 0; e < elems.length; e++) {
	var elem = elems[e];
	var type = elem.replace(/\d/g,'');
	var XCH = getXCHShift(type);
	var file = "";
	if (type == elem) {
	    file = dir.slice(12) + "/XAS/Spectrum-" + type + "/Spectrum-Ave-"+elem;//only works when the machine has a 6 letter name...
	} else {
	    var model = $('#currModel').val() - 1;
	    file = dir.slice(12) + "/XAS/Spectrum-" + type + "/" + jobName + "_"+model+"-" + jobName + "." + elem + "-XCH.xas.5.xas";
	}
	var name = jobName.slice(0,-15); var dt = jobName.slice(-15);
	var command = SHELL_CMD_DIR+"addToDB.sh " 
	    + file + " " + name + " " + dt + " " + elem + " " + XCH + " " + myUsername + " " + dir.slice(12);

	$.newt_ajax({type: "POST",
		    url: "/command/hopper",
		    data: {"executable": command},
		    success: function(res){
		    console.log("Success!");
		},
		    error: function(request,testStatus,errorThrown) {
		    console.log("Failed!");
		},});
    }
}

//Creates the jmol applet, couldn't use jmolapplet (wipes html)
function loadJmolResultsApp(dir, jobName) {

    var jMolOnLoadScript = "try{javascript resultsReady();javascript redrawModel('"+dir+"','"+jobName+"')}catch(e){}";
    Jmol.script(resultsApplet, jMolOnLoadScript);

}

function savePlotFile(dir) {
    var files = getSelectedElems();
    var jobName = $('#jobName').text();

    if (files.length > 1)
	alert("Please select one and only one spectra to save.");
    else {
	var elem = files[0];
	var model = $('#currModel').val() - 1;
	var type = elem.replace(/\d/g,'');
	var file = "";
	var XCHShift = getXCHShift(type);

	if (type == elem) {
	    file = dir + "/XAS/Spectrum-" + type + "/Spectrum-Ave-"+elem;
	} else {
	    file = dir + "/XAS/Spectrum-" + type + "/" + jobName + "_"+model+"-" + jobName + "." + elem + "-XCH.xas.5.xas";
	}

	window.open(newt_base_url+file + "?view=read");
    }
}
//merge with jmolscripts addSelections
function addResultsSelections(elems) {
    var scr = "select ";

    for (var e = 0; e < elems.length; e++) {
	element = elems[e];
	if (element.match(/^[a-zA-Z]{1,2}\d+$/) != null) {
	    scr += element + " OR ";
	}
	if (element.match(/^[a-zA-Z]{1,2}$/) != null) {
	    //I dont want it to match the all, because then you cant tell when comparing to average
	    //scr += "_" + element + " OR ";
	}
    }
    scr += "none";
    Jmol.script(resultsApplet, scr);
}

//Jquery Accesors
function getSelectedElems() {
    var elems = [];
    var i = 0;
    while (i < Number($('#totalAtoms').text())*2) {
	if (document.getElementById("spect"+i))
	    if (document.getElementById("spect"+i).checked)
		 elems.push(document.getElementById("spect"+i).name);
	i++;
    }
    return elems;
}
function getSelectedModels() {
    var selmodels = [];
    var i = 1;
    while (document.getElementById("model"+i)) {
	if (document.getElementById("model"+i).checked)
	    selmodels.push(Number(document.getElementById("model"+i).name)-1);
	i++;
    }
    return selmodels;
}
//makePlotWrapper
function makePlotWrapper(dir, jobName, elems, dataset) {

    $('#placeholder').html("<center><img src=\"ajax-loader-2.gif\" width=40></center>");
    
    if (!elems) {
	elems = getSelectedElems();
    }
    if (!dataset) dataset = [];
    if (resultsAppReady) addResultsSelections(elems);

    console.log(elems);

    var selModels = getSelectedModels();
    var elemObjs = [];
    for (var e = 0; e < elems.length; e++) {
	var elem = elems[e];
	var type = elem.replace(/\d/g,'');
	if (type == elem) {elemObjs.push([elem, '']);}
	else {
	    for (var m = 0; m < selModels.length; m++) {
		elemObjs.push([elem, selModels[m]]);
	    }
	}
    }

    console.log(elemObjs);

    var command = "/usr/bin/python " + SHELL_CMD_DIR + "ShirleyEndValue.py " + dir.substr(12);
    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command},
		success: function(res){
		//console.log(res.output);
		makePlot(dir, jobName, elemObjs, dataset, Number(res.output));},});
}

//Draw the plot
function makePlot(dir, jobName, elems, dataset, lim) {


    if(elems.length == 0 && dataset.length == 0) {
	$('#placeholder').html("<center>No Spectra Active.</center>");
	 return;}
    if(elems.length == 0) {//dude is clicking too much
	$.plot($("#placeholder"), dataset, options);
	return;
    }

    var elemObj = elems.pop();
    var elem = elemObj[0];
    var model = elemObj[1];
    var type = elem.replace(/\d/g,'');
    var file = "";
    var XCHShift = getXCHShift(type);
    var limit = lim + XCHShift;

    var options = {
	legend: {
	    show: true,
	    margin: 10,
	    position : "ne"
	},
	series: { lines: {show: true,}, shadowSize: 0 },
	xaxis: {zoomRange: [.05, 50], panRange: [-10, 30000]},
	yaxis: {zoomRange: [.001, 1], panRange: [-.01, 5]},
	grid: {hoverable:true, clickable:true,
	       markings: [{xaxis: {from: limit, to: limit}, yaxis:{from: -1, to: 5}, color: "#bb0000"}]},
        zoom: {interactive:true},
        pan: {interactive:true}
    };
    var XCHtml = "XCH Shift:<input id='XCH' size='5' type='textbox' onchange='addShift()' value= "+XCHShift+" />";
    $('#xchs').html(XCHtml);

    //while (XCHShift === undefined) {
    //	setTimeout("XCHShift = getXCHShift(type);", 1000);
    //}

    if (type == elem) {
	file = dir + "/XAS/Spectrum-" + type + "/Spectrum-Ave-"+elem;
    } else {
	file = dir + "/XAS/Spectrum-" + type + "/" + jobName + "_"+model+"-" + jobName + "." + elem + "-XCH.xas.5.xas";
    }

    umodel = Number(model) + 1;//plus 1 for user clarity
    
    if ($('#dimensional').prop('checked') && type != elem)
	{//Shows all 3 directional plots
	$.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		var fileText = res.split(/\r\n|\r|\n/);
		points = [[],[],[],[]];
		for (var i = 1; i < fileText.length; i++) {
		    var line = fileText[i].replace(/^\s*/g, '').replace(/\s*$/g, '').replace(/[  ]{2,}|\t/g, ' ').split(" ");
		    if (line.length < 5 || fileText[i].match('#')) continue;
		    var ev = XCHShift + parseFloat(line[0]);
		    points[0].push([ev, parseFloat(line[1])]);
		    points[1].push([ev, parseFloat(line[2])]);
		    points[2].push([ev, parseFloat(line[3])]);
		    points[3].push([ev, parseFloat(line[4])]);
		}
		dataset.push({label: elem + " M#"+umodel ,data: points[0]});
		dataset.push({label: elem + "x" + " M#"+umodel ,data: points[1]});
		dataset.push({label: elem + "y" + " M#"+umodel ,data: points[2]});
		dataset.push({label: elem + "z" + " M#"+umodel ,data: points[3]});
		if (elems.length == 0) $.plot($("#placeholder"), dataset , options);
		else makePlot(dir, jobName, elems, dataset, lim);
	    },
		error: function(request,testStatus,errorThrown) {
		if (elem.length <= 6)
		    elems.push([type + "0" + elem.replace(type, ''), model])
			makePlot(dir, jobName, elems, dataset, lim);
	    },});
	} else {//only shows the average plots
	$.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		var fileText = res.split(/\r\n|\r|\n/);
		points = [];
		for (var i = 1; i < fileText.length; i++) {
		    var line = fileText[i].replace(/^\s*/g, '').replace(/\s*$/g, '').replace(/[  ]{2,}|\t/g, ' ').split(" ");
		    if (line.length < 2 || fileText[i].match('#')) continue;
		    points.push([parseFloat(line[0])+XCHShift, parseFloat(line[1])]);
		}
		var labelText = elem;
		if (type != elem) labelText += " M#"+umodel;
		dataset.push({label: labelText ,data: points});
		if (elems.length == 0) $.plot($("#placeholder"), dataset , options);
		else makePlot(dir, jobName, elems, dataset, lim);
	    },
		error: function(request,testStatus,errorThrown) {
		if (elem.length <= 6)
		    elems.push([type + "0" + elem.replace(type, ''), model])
			makePlot(dir, jobName, elems, dataset, lim);
	    },});
    }
}



//View Files
//READ FILES PIECES
function individualJob(jobName, machine) {
    $('#previousjobsfiles').html("<table width=100\%><th align=left>Output files for "+jobName
			     + "</th><tr><button onclick='previousJobsWrapper()'>Return to list</button>"
			     + "</tr></table><center><img src=\"ajax-loader-2.gif\" width=40></center>");
    machine = machine.toLowerCase();
    var directory = "/file/" + machine + "/global/scratch/sd/" + myUsername + "/"+jobName;
    $.newt_ajax({type: "GET",
		url: directory,
		success: function(res){
		console.log(res);
		if (res != null && res.length > 0) {
		    var myText = "<table width=100\%><th align=left>Output files for " + jobName + "</th>"
			+ "<tr><button onclick='previousJobsWrapper()'>Return to list</button></tr></table><br>";

		    myText += "<table width=100\%><tr><th width=60\%>Name</th><th width=10\%>Size</th><th width=30\%></th></tr></table>";
		    myText += "<table width=100\% cellpadding=5>"
			for (var i = 0 ; i < res.length ; i++) {
			    var name = res[i].name.replace("-ANALYSE","").replace("-REF","").replace("-XAS","");
			    if (name == ".") continue;
			    myText += "<tr class='listitem'>";
			    myText += "<td width=60\%>" + res[i].name + "</td><td width=10\% class=\"statusnone\">"
				+ res[i].size + "</td>";
			    if (res[i].perms.indexOf("d") != -1) {
				myText += "<td align=center><button onClick=\"changeDir(\'" + directory + "/" + name
				    + "\', \'" + jobName + "\')\" type=\"button\">Change Dir.</button></td>";
			    } else {
				myText += "<td align=center><button onClick=\"getFile(\'" + directory + "/" + name
				    + "\', \'" + jobName + "\')\" type=\"button\">Get File</button></td>";
				myText += "<td><a href = '"+newt_base_url+directory+"/"+name+"?view=read'>Download File</a></td>";
			    }
			    myText += "</tr>";
			}
		    myText += "</table:><br>";
		    $('#previousjobsfiles').html(myText);
		    $('#previousjobsfiles').trigger('create');
		} else {
		    $('#previousjobsfiles').html("<table width=100\%><th align=left>Finished Calculations</th></table>"
					     + "<center><br>Directory apparently empty. Whoops.");
		}
	    },
		error: function(request,testStatus,errorThrown) {
		var myText = "<center>Error:<br><br>"+testStatus+"<br><br>"+errorThrown+"</center>";
		$('#previousjobsfiles').html(myText);
		$('#previousjobsfiles').trigger('create');
	    },
		});
}

function getFile(file, jobName) {
    $('#previousjobsfiles').html("<h3>Results for "+jobName
			     + "</h3><center><img src=\"ajax-loader-2.gif\" width=40></center>");
    $.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		var fileText = res.replace(/\n/g,"<br/>");
		directory = file + "/..";
		var myText = "<h3>Results for " + jobName + "</h3><br>";
		myText += "<button onclick='previousJobsWrapper()'>Return to list</button>"
		myText += "<table width=100\% cellpadding=5>";
		myText += "<tr class='listitem'><td width=10\%>..</td>";
		myText += "<td><button onClick=\"changeDir(\'" + directory + "\', \'" 
		    + jobName + "\')\" type=\"button\">Return to Directory</button></td></tr>";
		myText += "<td align=left>" + fileText + "</td></table><br>";
		$('#previousjobsfiles').html(myText);
		$('#previousjobsfiles').trigger('create');
	    },
		error: function(request,testStatus,errorThrown) {
		    
		var myText = "<center>Error:<br><br>"+testStatus+"<br><br>"+errorThrown+"</center>";
		$('#previousjobsfiles').html(myText);
		$('#previousjobsfiles').trigger('create');
	    },
		});
}

function changeDir(directory, jobName) {
    $('#previousjobsfiles').html("<h3>Results for "+jobName
			     + "</h3><center><img src=\"ajax-loader-2.gif\" width=40></center>");
    $.newt_ajax({type: "GET",
		url: directory,
		success: function(res){
		if (res != null && res.length > 0) {
		    var myText = "<h3>Results for " + jobName + "</h3><br>";
		    myText += "<table width=100\%><tr><th width=60\%>Name</th><th width=10%>Size</th><th width=30%></th></tr></table>";
		    myText += "<table width=100\% cellpadding=5>"
			for (var i = 0 ; i < res.length ; i++) {
			    var name = res[i].name;
			    myText += "<tr class='listitem'>";
			    myText += "<td width=60\%>" + name + "</td><td width=10\% class=\"statusnone\">"
				+ res[i].size + "</td>";
			    if (res[i].perms.indexOf("d") != -1) {
				myText += "<td align=center><button onClick=\"changeDir(\'" + directory + "/" + name
				    + "\', \'" + jobName + "\')\" type=\"button\">Change Dir.</button></td>";
			    } else {
				myText += "<td align=center><button onClick=\"getFile(\'" + directory + "/" + name
				    + "\', \'" + jobName + "\')\" type=\"button\">Get File</button></td>";
				myText += "<td><a href = '"+newt_base_url+directory+"/"+name+"?view=read'>Download File</a></td>";
			    }
			    myText += "</tr>";
			}
		    myText += "</table:><br>";
		    $('#previousjobsfiles').html(myText);
		    $('#previousjobsfiles').trigger('create');
		} else {
		    $('#previousjobsfiles').html("<table width=100\%><th align=left>Finished Calculations</th></table>"
					     + "<center><br>Directory apparently empty. Whoops.");
		}
	    },
		error: function(request,testStatus,errorThrown) {
		var myText = "<center>Error:<br><br>"+testStatus+"<br><br>"+errorThrown+"</center>";
		$('#previousjobsfiles').html(myText);
		$('#previousjobsfiles').trigger('create');
	    },
		});
}

//Check machine status
function updateStatus(machine) {
    $.newt_ajax({type: "GET",
		url: "/status/"+machine,
		success: function(res) {
		var myText = machine + " is " + res.status + "<br>";
		$('#clusterStatus').html(myText);
		$('#clusterStatus').trigger('create');
	    },
		error: function(request, testStatus, errorThrown) {
		console.log("Failed to get cluster status.\n"+testStatus+":\n"+errorThrown);
	    },
		});
}

//Toggle Show Advanced Options
function hideAdvancedOptions() {
    $('#advancedOptions').hide();
    $('#AdvancedButton').show();
}
function showAdvancedOptions() {
    $('#advancedOptions').show();
    $('#AdvancedButton').hide();
    updateStatus($('#machine').val());
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

//Returns how many atoms are excited in this scenario, and thus how many nodes to use.
function getExcitedElementsTotal(coordsArray, xasElements) {
    // does not handle full regex
    var numNodes = 0;
    for (var i = 0; i < xasElements.length; i++) {
	atoms = "" + xasElements[i];
	if (atoms.match(/^[a-zA-Z]{1,2}\d+$/) != null) {//C6 type
	    index = Number(atoms.match(/\d+/)[0])-1;
	    try {
		if (coordsArray[index].split(" ")[0] == atoms.match(/^[a-zA-Z]+/)[0])
		    numNodes++;
	    } catch(err) {}
	}
	else {//C Type
	    for (var j = 0; j < coordsArray.length; j++) {
		if (coordsArray[j].split(" ")[0] == atoms)
		    numNodes++;
	    }
	}
    }
    return numNodes;
}

//Turns the given coordinates into a valid xyz file.
//Used primarily by DrawMol
function makeXYZfromCoords(i) {
    //i could be 0
    if (i === undefined) i = activeModel;
    var myform=document.getElementById('inputs');
    var materialName = myform.material.value;
    var coords = sterilize(models[i]);
    var numberOfAtoms = coords.split("\n").length;
    var xyz = numberOfAtoms + "\n" + materialName + "\n" + coords;
    //console.log(xyz);
    return xyz;
}

//JOB SUBMISSION FUNCTIONS
//Confirm that all inputs are acceptable, then attempt to submit a new job.
function validateInputs(form) {
    form.Submit.disabled=true;
    var invalid = false;
    var message = "";
    if (form.material.value.length <= 0) {
	message += "Need material name to create output directory.\n";
	invalid = true; }
    //Set NEXCITED See if passes;
    var XAS = form.XASELEMENTS.value;
    //references number of excited from the first model, no checking others
    var coordinates = sterilize(models[0]).split("\n");
    form.NEXCITED.value = getExcitedElementsTotal(coordinates, XAS.split(" "));
    if (form.NEXCITED.value <= 0) {
	message += "Must excite at least one atom in coordinates. \"Xx\" matches all of element Xx, \"Xx2\" matches the Xx atom in the second row of your corrdinates. Different atoms must be separated by whitespace.\n";
	invalid = true; }
    //Check that all models have the same number of equivalent atoms
    var control = coordinates;
    for (var i = 1; i < models.length; i++) {
	var species = sterilize(models[i]).split("\n");
	for (var s = 0; s < species.length; s++) {
	    var specie = species[s].split(" ")[0];
	    if (specie != control[s].split(" ")[0]) {
		message += "Model "+(Number(i) + 1)+" must be the same species as Model 1\n";
		invalid = true;
		break;}}}
    //PPP Passes (AutoSet)
    //check nperatom
    if (form.NPERATOM.value.match(/^\d+$/) == null) {
	message += "Nodes per atom must be a positive integer. \n";
	invalid = true; }
    if (form.NBANDFAC.value.match(/^\d+$/) == null) {
	message += "NBANDFAC must be a positive integer. \n";
	invalid = true; }
    if (form.PPP.value.match(/^\d+$/) == null) {
	message += "PPP must be a positive integer. \n";
	invalid = true; }
    if (form.NTG.value.match(/^\d+$/) == null) {
	message += "NTG must be a positive integer. \n";
	invalid = true; }
    if (form.KPOINTS.value.match(/^\d+ \d+ \d+$/) == null) {
	message += "KPoints must be of the form 'x y z'. \n";
	invalid = true; }
	
    //Check walltime
    if (form.wallTime.value.match(/^\d{2,3}:\d{2}:\d{2}$/) == null) {
	message += "Walltime parameter must be of the form XX:XX:XX (Hours:Minutes:Seconds) \n";
	invalid = true; }
    //Check totalCharge
    var tchg = form.TOTCHG.value;
    if (tchg.match(/^\-?\d$/) == null) {
	message += "Total charge must be an integer value. \n";
	invalid = true; }  
  
    //check crystal stats
    var a = form.CellA.value;
    var b = form.CellB.value;
    var c = form.CellC.value;
    if (a.match(/^\d*.?\d*$/) == null ||
	b.match(/^\d*.?\d*$/) == null || 
	c.match(/^\d*.?\d*$/) == null) {
	message += "Cell Size Dimensions are not valid. \n";
	invalid = true; }    
    var alp = form.CellAlpha.value;
    var bet = form.CellBeta.value;
    var gam = form.CellGamma.value;
    if (alp.match(/^\d*.?\d*$/) == null || alp > 180 ||
	bet.match(/^\d*.?\d*$/) == null || bet > 180 ||
	gam.match(/^\d*.?\d*$/) == null || gam > 180) {
	message += "Cell Size Angles are not valid. \n";
	invalid = true; }
    var brv = form.IBRAV.value;
    if (brv.match(/^\d?/) == null || brv > 14 || brv < 0) {
	message += "Invalid Bravais Lattice. \n";
	invalid = true; }
    //Sterilize coordinates, if they have errors, drop them. (in newJobSubmission)
    if (invalid) {alert(message); form.Submit.disabled=false;}
    else 
	newJobSubmission(form);
 }

//Begin the process of submitting a new job.
function newJobSubmission(form) {
    form.Submit.disabled=true;
    $('#subStatus').show();
    $('#subStatus').html("<table width=100\%><th align=left>Working...</th></table><center><img src=\"ajax-loader-2.gif\" width=40></center>");
    var command = SHELL_CMD_DIR+"makeFileDir.sh "+$('#outputDir').val()+" ";
    var materialName = form.material.value.replace(/^\s+/g,"").replace(/\s+$/g,"").replace(/\s+/g," ").replace(" ","_");
   
    var machine = form.machine.value;

    var d = new Date();
    var dateStr = d.toUTCString();//Wed, 06 Jun 2012 17:30:05 GMT
    materialName += dateStr.replace(/[a-zA-Z]*,/g,"").replace(/\s|:|(GMT)/g,"");//06Jun2012173005
    command += materialName;

    //Make the directory for this job, then, upon completion, upload coordinates.
    $.newt_ajax({type: "POST",
		url: "/command/" + machine,
		data: {"executable": command},
                success: function(res) {pushFile(form, machine, materialName);},
    	        error: function(request,testStatus,errorThrown) {
		//Announce failure
		 form.Submit.disabled=false;
	 	 $('#subStatus').html("<table width=100\%><th align=left>Failed!\n"+testStatus+":\n" + errorThrown+"</th></table>");
	    },});
}

//upload the coordinates file to the correct machine
function pushFile(form, machine, molName) {
    var count = 1;
    for (var m = 0; m <models.length; m++) {
	var xyzcoords = makeXYZfromCoords(m);
	$.newt_ajax({type: "PUT", 
		    url: "/file/"+machine+$('#outputDir').val()+"/"+molName+"/"+molName+"_"+m+".xyz",
		    data: xyzcoords,
		    success: function(res) {
		    if (count == models.length)
			executeJob(form, molName);
		    else 
			count++;
		},
		    error: function(request,testStatus,errorThrown) {
		    var command = SHELL_CMD_DIR + "rmFileDir.sh "+ $('#outputDir').val() + " " + molName;
		    $.newt_ajax({type: "POST",
				url: "/command/" + machine,
				data: {"executable": command},});
		    form.Submit.disabled=false;
		    $('#subStatus').html("<table width=100\%><th align=left>Failed!\n"+testStatus+":\n" + errorThrown + "</th></table>");},});
    }
}

function toRad(Value) {
    /** Converts numeric degrees to radians */
    return Value * Math.PI / 180;
}

//Execute the submit script
function executeJob(form, materialName) {
    var dir =  $('#outputDir').val();
    var inputs = "";
    var XAS = form.XASELEMENTS.value;
    var PPP = form.PPP.value;
    var NTG = form.NTG.value;
    var PPN = form.PPN.value;
    var nodes = form.NEXCITED.value * form.NPERATOM.value;
    var machine = form.machine.value;
    var brv =  form.IBRAV.value;
    var totChg = form.TOTCHG.value;
    var useCustomBlock = $('#useCustom').prop('checked');

    //TODO Make this custom block absolute or something.  too many different things merging together here.

    //This is so hacked together, watch for escaped characters.
    //Pass inputs and pbs headers as files?
    var inputs="\"MOLNAME=\\\""+materialName+"\\\"\\n";
    inputs+="XASELEMENTS='"+XAS+"'\\n";
    inputs+="PPP="+PPP+"\\n";
    inputs+="IBRAV="+brv+"\\n";
    inputs+="A="+form.CellA.value+"\\n";
    inputs+="B="+form.CellB.value+"\\n";
    inputs+="C="+form.CellC.value+"\\n";
    inputs+="COSBC="+Math.cos(toRad(form.CellAlpha.value))+"\\n";
    inputs+="COSAC="+Math.cos(toRad(form.CellBeta.value))+"\\n";
    inputs+="COSAB="+Math.cos(toRad(form.CellGamma.value))+"\\n";
    inputs+="NJOB="+form.NEXCITED.value+"\\n";
    inputs+="NBND_FAC="+form.NBANDFAC.value+"\\n";
    inputs+="tot_charge="+totChg+"\\n";
    inputs+='PW_POSTFIX=\\"-ntg '+ NTG +'\\"\\n';
    inputs+='K_POINTS=\\\"K_POINTS automatic \\n'+form.KPOINTS.value+' 0 0 0\\\"\\n\"';

    console.log(inputs);
    

    var command = SHELL_CMD_DIR+"submit.sh ";
    command += dir + " ";
    command += materialName + " ";
    command += inputs + " ";
    command += nodes + " ";
    command += PPN + " ";
    command += machine + " ";
    command += form.Queue.value + " ";
    command += form.wallTime.value + " ";
    command += form.acctHours.value + " ";

    console.log(useCustomBlock);
    if (useCustomBlock) {
	command += $('#customInputBlock').val() + " ";
    }

    console.log(command);
 
    var webdata = materialName + "\n"; //[0] Name
    var ahora = new Date();
    webdata += ahora.getTime() + "\n"; //[1] Time
    webdata += expandXAS(XAS, form) + "\n"; //[2] XAS
    webdata+="["+form.CellA.value+",";
    webdata+=form.CellB.value+",";
    webdata+=form.CellC.value+",";
    webdata+=form.CellAlpha.value+",";
    webdata+=form.CellBeta.value+",";
    webdata+=form.CellGamma.value+"]\n"; //[3] Unitcell
    webdata+=models.length+"\n";//[4] Models
    webdata+=sterilize(models[0]).split("\n").length;//[5] Atoms
    
    //post webdata
    $.newt_ajax({type: "PUT", 
		url: "/file/"+machine+dir+"/"+materialName+"/"+ "webdata.in",
		data: webdata,
		success: function(res) {;},});

    console.log("Command "+command)

    //Post job.
    $.newt_ajax({type: "POST",
		url: "/command/" + machine,
		data: {"executable": command},
		success: function(res){
		form.Submit.disabled=false;
		$('#subStatus').hide();
		console.log("Success!");
	    },
		error: function(request,testStatus,errorThrown) {
		 command = SHELL_CMD_DIR + "rmFileDir.sh "+dir+ " " + molName;
		 $.newt_ajax({type: "POST",
			     url: "/command/" + machine,
			     data: {"executable": command},});
		 form.Submit.disabled=false;
	  	 $('#subStatus').html("<table width=100\%><th align=left>Failed!\n"+testStatus+":\n" + errorThrown + "</th></table>");},
		});
}
//helper to set the XAS elements for postprocessing
function expandXAS(XAS, form) {
    var coo = models[0];
    coo = coo.split("\n");
    var xas = XAS.split(" ");

    XAS = "";
    for (var x = 0; x < xas.length; x++) {

	if (xas[x].match(/^[A-Z][a-z]?$/)) { //get all of the averaged elements
	    XAS += xas[x] + " ";
	    for (var c = 0; c < coo.length; c++)
		if (coo[c].split(" ")[0] == xas[x]) 		    
		    XAS += xas[x]+(Number(c)+1)+" ";
	} else if (xas[x].match(/^[A-Z][a-z]?\d+$/) && coo[Number(xas[x].match(/\d+/)[0])-1].split(" ")[0] == xas[x].replace(/\d*/g, '')) {
	    if (XAS.indexOf(xas[x].replace(/\d*/g, '')+ " ") == -1) {
		XAS += xas[x].replace(/\d*/g, '') + " ";
	    }
	    XAS += xas[x] + " ";
	}
    }
    return XAS.slice(0, -1);//get rid of last whitespace
}

//Opening a transfered file from webDynamics
function openTransferFile() {
    $.newt_ajax({type: "GET",
		url: "/file/hopper"+DATABASE_DIR+"/tmp/"+myUsername+".xyz?view=read",
		success: function (res) {

		//TODO
		//Open the submit form and read this file in, on page load.
		console.log(res);
		Jmol.script(previewApplet, "LOAD INLINE '"+res+"';javascript readInTransferedFile();");
	    },  error: function(request,testStatus,errorThrown) { 
		console.log("noTransferFile");
	    },
	});
}
function readInTransferedFile() {
    readCoordsFromJmol();
    models.splice(0, 1);
    drawMolInPreview();
    makeCoordsDiv();
}


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
    if(CrystalSymmetry) drawMolInPreview();
    else makeCellSize();
}
function switchToModel(i) {
    activeModel = i;
    makeCoordsDiv();
    drawMolInPreview();
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
    drawMolInPreview();
}

//Searching the database for a saved job
function searchDB() {
    
    var term = $('#searchTerms').val();
    var command = "/usr/bin/python " + DATABASE_DIR + "spectraDB/search.py " + term;

    $.newt_ajax({type: "POST",
		url: "/command/hopper",
		data: {"executable": command},
		success: function(res){
		console.log(res);
		var results = res.output.split("\n");
		var myText = "<table width=100\% cellpadding=5><tr><th width=20\%>Name</th>"
		    + "<th>Date</th><th>User</th><th>Element</th><th>EVShift</th></tr>";
		for (r = 0; r < results.length; r++) {
		    res = results[r].split(" ");
		    if (res.length < 6) continue;
		    myText += "<tr class='listitem'>";
		    myText += "<td width=22\%>" + res[0] + "</td>";
		    myText += "<td width=22\%>" + res[1] + "</td>";
		    myText += "<td width=22\%>" + res[2] + "</td>";
		    myText += "<td width=12\%>" + res[3] + "</td>";
		    myText += "<td width=12\%>" + res[4] + "</td>";
		    myText += "<td><button onclick='dbJob(\""+res[3]+"\",\""+res[4]+"\",\""+res[5]+"\")'>Go!</button></td>";
		    myText += "</tr>";
		}
		myText += "</table><br><center><div id='flotHolder' style='display:none;width:400px;height:300px;'></div></center>";
		$('#searchResults').html(myText);
	    },
		error: function(request,testStatus,errorThrown) {
		$('#searchResults').html("Search failed!  " + errorThrown);
	    },
		});
}
//Show a saved job.
//needs filling out
function dbJob(elem, XCHShift, path) {
    var fullPath = DATABASE_DIR+"spectraDB/" + path;
    $('#flotHolder').html("<center><img src=\"ajax-loader-2.gif\" width=40></center>");
    $('#flotHolder').show();
    var options = {
	legend: {
	    show: true,
	    margin: 10,
	    position : "ne"
	},
	//step probably isnt a good idea
	series: { lines: {show: true, steps: true,}, shadowSize: 0 },
	xaxis: {zoomRange: [.05, 50], panRange: [-10, 1000]},
	yaxis: {zoomRange: [.001, 1], panRange: [-.01, 5]},
        zoom: {interactive:true},
        pan: {interactive:true}
    };

    var file = "/file/hopper" + fullPath;
    $.newt_ajax({type: "GET",
		url: file + "?view=read",
		success: function(res){
		var fileText = res.split(/\r\n|\r|\n/);
		points = [];
		for (var i = 1; i < fileText.length; i++) {
		    var line = fileText[i].replace(/^\s*/g, '').replace(/\s*$/g, '').replace(/[  ]{2,}|\t/g, ' ').split(" ");
		    if (line.length < 2 || fileText[i].match('#')) continue;
		    points.push([parseFloat(line[0])+Number(XCHShift), parseFloat(line[1])]);
		}
		var labelText = elem;
		var data = [{label: labelText ,data: points}];
		console.log(data);
		$.plot($("#flotHolder"), data, options);
	    },
		error: function(request,testStatus,errorThrown) {
		$('#flotHolder').html("Failed to load plot");
	    },});
}

// query the selected REST API.
var lastSearchResult = '';
function restQuery() {
  var dest = $('#searchResults');

  dest.html('working...');
  var root = $('#searchLocationChosen');
  var url = TreeEval.treeValue(root);

  var end = url.substring(url.lastIndexOf('/') + 1,
                          url.length);

  // only send SDFs to JSmol.
  var result = '';
  switch(end) {
    case 'SDF':
      $.get(url, function(data, status) {
        lastSearchResult = data;
        result = '<p id="searchResult" ';
        result += ' style="display:none">';
        result += 'Finished. Click "Submit Calculations" on the left to display results.</p>';
        dest.html(result);
        $('#searchResult').fadeIn();
      }).fail(function(xhr, textStatus, errorThrown) {
        result = 'An error occurred with the request:<br><pre>';
        result += errorThrown;
        result += '<br><br>';
        result += xhr.responseText;
        result += '</pre>';
        dest.html(result);
      });
      break;
    default:
      result = '<a id="searchResult" ';
      result += 'style="display:none" ';
      result += 'target="_blank" href="';
      result += url;
      if (end == 'PNG') {
        result += '">View Image</a><br>';
      } else {
        result += '">Open Result</a><br>';
      }
      dest.html(result);
      $('#searchResult').fadeIn();
      break;
  }
}

// display the result of the last search result in JSmol.
function displaySearchResult() {
  var script = "try{Load INLINE '" + lastSearchResult + "'}catch(e){;}";
  Jmol.script(previewApplet, script);
}

//Div Wrapper functions.  For Organization. Checks Login Status.
function previousJobsWrapper() {
    if (myUsername.indexOf("invalid") != -1) {
        //
    } else {
	$('#previousjobslist').show();
	$('#previousjobsfiles').hide();
	$('#previousjobresults').hide();
        previousJobs();
    }
}

function runningJobsWrapper() {
    if (myUsername.indexOf("invalid") != -1) {
        //
    } else {
        runningJobs();
    }
}

function individualJobWrapper(myJobId, machine) {
    if (myUsername.indexOf("invalid") != -1) {
        //
    } else {
	$('#previousjobslist').hide();
	$('#previousjobsfiles').show();	
	$('#previousjobresults').hide();
        individualJob(myJobId, machine);
    }
}
function viewJobOutputWrapper(myJobId, machine) {
    if (myUsername.indexOf("invalid") != -1) {
        //
    } else {
	$('#previousjobslist').hide();
	$('#previousjobsfiles').hide();	
	$('#previousjobresults').show();
        individualJobOutput(myJobId, machine);
    }
}
function editMoleculeWrapper() {
    initMainApp();
}
function searchWrapper() {
    if (myUsername.indexOf("invalid") != -1) {
	//
    } else {
        //
    }
}

//Div Functions.  Formats webpage.
function viewJobFiles(myJobId, machine) {
    individualJobWrapper(myJobId, machine);
}

function viewJob(myJobId, machine) {
    resultsAppReady = false; //app needs to load, put everywhere?? dunno how to structure this test
    viewJobOutputWrapper(myJobId, machine);
}

function switchToSubmitForm() {
    document.inputs.Submit.disabled=false;
    updateStatus(document.inputs.machine.value);
    makeCoordsDiv();
    initPreviewApp();
    window.clearInterval(autoInterval);
}
function switchToPrevious() {
    window.clearInterval(autoInterval);
    previousJobsWrapper();
}
function switchToRunning() {
    window.clearInterval(autoInterval);
    runningJobsWrapper();
}
function switchToDrawMolecule() {
    window.clearInterval(autoInterval);
    editMoleculeWrapper();
}
function switchToInfo() {
    window.clearInterval(autoInterval);
}
function switchToSearchDB() {
    window.clearInterval(autoInterval);
    searchWrapper();
}

// Search area switching functions.
function mutexShow(shownId, mutexClass) {
    // if nothing to show, no errors will be thrown. Thanks jquery!
    $('.' + mutexClass).hide();
    $('#' + shownId).show();
}

function switchToSearchLocation(loc) {
    mutexShow(loc, 'searchTermSet');
}

// organizes PubChem switching
// TODO: make this less hideous
PubChem = {
    Compound: {},
    Substance: {},
    Assay: {},
}
PubChem.chooseDomain = function(dom) {
  dom = 'PubChem_' + dom; 
  mutexShow(dom, 'PubChem_domain');
}
PubChem._chooseNamespace = function(namespace, domain) {
  // some namespaces don't have anything to show. This is okay.
  namespace = 'PubChem_' + domain + '_' + namespace;
  mutexShow(namespace, 'PubChem_' + domain + '_namespace');
}
PubChem._chooseOperation = function(op, domain) {
  op = 'PubChem_' + domain + '_op_' + op;
  mutexShow(op, 'PubChem_' + domain + '_op');
}
PubChem.Compound.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'compound');
}
PubChem.Compound.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'compound');
}
PubChem.Substance.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'substance');
}
PubChem.Substance.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'substance');
}
PubChem.Assay.chooseNamespace = function(namespace) {
  PubChem._chooseNamespace(namespace, 'assay');
}
PubChem.Assay.chooseOperation = function(op) {
  PubChem._chooseOperation(op, 'assay');
}
