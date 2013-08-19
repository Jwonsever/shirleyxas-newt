<!DOCTYPE html>
<html>
  <head>
    <title>Web XS</title>

    <!-- Bootstrap & CSS -->
    <link href="../bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">
    <style>
      body {
        padding-top: 60px; /* 60px to make the container go all the way to the bottom of the topbar */
      }
    </style>
    <link href="../bootstrap/css/bootstrap-responsive.min.css" rel="stylesheet" media="screen">
    <link href="../als_portal.css" rel="stylesheet" type="text/css" />
    <link rel="stylesheet" href="WebXS.css">


    <!-- Als Bootstrap, JQuery, etc-->
    <script src="../js/jquery-latest.js"></script>    
    <script src="../bootstrap/js/bootstrap.min.js"></script>
    <script src="../js/mynewt.js"></script>
    <script src="../js/alsapi.js"></script>
    <script src="../js/als_portal.js"></script>


    <!-- Major Function Values-->
    <script src="../GlobalValues.js"></script>

    <!--Scripts required for JME injection-->
    <script src="javascript/jsmol-13.1.15/jsmol/jsme/jsme/jsme.nocache.js" language="javascript" type="text/javascript">
    </script>

    <!--jsMol-->
    <script type="text/javascript" src="javascript/jsmol-13.1.15/jsmol/JSmol.min.nojq.js"></script>
    <script type="text/javascript" src="javascript/jsmol-13.1.15/jsmol/js/JSmolJME.js"></script>

    <!-- This is supposed to be for writes, but it breaks getstates && other things... idk -->
    <!--script type="text/php" src="javascript/jsmol-13.1.15/jsmol/jsmol.php"></script-->

    <!-- Flot plotting -->
    <script src="javascript/Flot/jquery.flot.js"></script>
    <script src="javascript/Flot/jquery.flot.navigate.js"></script>

    <!-- Potential for improved browser support, uploads on older browsers -->
    <script src="ajaxfileupload.js"></script>

    <!-- Local JS-->
    <script src="WebXS.js"></script>
    <script src="jmolScripts.js"></script>
    <Script src="jmolInit.js"></script>

    <!-- Notice Hopper Downtime, and alert the user. -->
    <script>
      $.newt_ajax({type: "GET",
      url:newt_base_url + "/status/",
      success:function(res) {
      if (res[0].status != "up") {
      $("#alarm-bar").html("<b>The NERSC Hopper machine is down!  Carver will be used, job submission will not work.</b>");
      mymachine="carver";
      }

      },});
    </script>

  </head>
  <body>

    <div class="navbar navbar-inverse navbar-fixed-top">
      <div class="navbar-inner">
        <div class="container">
          <a class="btn btn-navbar" data-toggle="collapse" data-target=".nav-collapse">
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
            <span class="icon-bar"></span>
          </a>
          <div class="nav-collapse collapse">
	    <ul class="nav">
              <li><img src="../images/mfnewlogo.png" width=105 padding=0 border=0></li>
              <li><a href="../index.html">Home</a></li>
              <li><a href="https://portal.nersc.gov/project/als/beta/index.html">ALS Simulation Portal</a></li>
              <li class="dropdown active">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">Simulation Tools <b class="caret"></b></a>
                <ul class="dropdown-menu">
                  <li class="nav-header">X-Ray Absorption</li>
                  <li class="active"><a href="#">WebXS</a></li>
		  <li><a href="../WebDynamics/index.php">WebDynamics</a></li>
                  <li><a href="http://leonardo.phys.washington.edu/feff/">FEFF</a></li>
                  <li class="divider"></li>
                  <li class="nav-header">Web Scraping</li>
		  <li><a href="http://matzo.lbl.gov">Matzo</a></li>
                  <li class="divider"></li>
                </ul>
              </li>
            </ul>
            <div class="pull-right" id="auth-spinner"> <img src="../images/white-ajax-loader.gif"></div>
            <div id=login-area style='display:none;' class="pull-right">
              <form class="navbar-form pull-right" method=POST action='javascript: submission();'>
                <input class="span2" type="text" placeholder="Username" id="id_username">
                <input class="span2" type="password" placeholder="Password" id="id_password"> </button>
                <button type="submit" class="btn">Sign in</button>
              </form>
            </div>
            <div id=logout-area class="pull-right" style='display:none;'>
              <form class="navbar-form pull-right" method=POST action='javascript: logout();'>
                <button type="submit" class="btn">Logout</button>
              </form>
              <div id=username-area class="pull-right" style="padding:10px"></div>
            </div>
          </div><!--/.nav-collapse -->
        </div>
      </div>
    </div>

    <div class="container">

      <div id="alarm-bar"></div>

      <div class="row">

        <div class="span2">
          <div class="tabbable"> <!-- Only required for left/right tabs -->
            <ul class="nav nav-pills nav-stacked">
              <li class="active"><a href="#shirleyinfo" data-toggle="tab">What is this?</a></li>
              <li><a href="#searchDB" data-toggle="tab" onclick="switchToSearchDB()">Structure Database</a></li>
              <li><a href="#moleculeEditor" data-toggle="tab" onclick="switchToDrawMolecule()">Draw Molecule</a></li>
	      <li><a href="#submitjobs" data-toggle="tab" onclick="switchToSubmitForm()">Submit Calculations</a></li>
	      <li><a href="#runningjobs" data-toggle="tab" onclick="switchToRunning()">Running Calculations</a></li>
              <li><a href="#previousjobs" data-toggle="tab" onclick="switchToPrevious()">Finished Calculations</a></li>
	      <li><a href="#spectrumDB" data-toggle="tab" onclick="switchToSpectrumDB()">Spectrum Database</a></li>
            </ul>
          </div>
        </div>

        <div class="span10" style='min-height:500px;background-color: #eeeeee; border-radius:5px;'>
          <div style='padding:20px'>
            <div class="tab-content">

	      <div id='runningjobs' class="tab-pane"></div>
	      
	      <div id='previousjobs' class="tab-pane">
		<div id='previousjobslist' style="display: none;"></div>
		<div id='previousjobsfiles' style="display: none;"></div>
		<div id='previousjobresults' style="display: none;">
		  <div id='jobHeader'></div>
		  <table style="width:95%; table-layout:fixed;">
		    <tr style="vertical-align: top;">
		      <td width=55%>
			<br>
			<div id='jobresults'></div>
		      </td><td>
			<br>
			<div id='jmolactivediv'> 
			  <script>
			    JmolInfo.width = 375;
			    JmolInfo.height = 375;
			    resultsApplet=Jmol.getApplet(resultsApplet, JmolInfo);
			  </script>
			</div>
			<br>
			<div id='jmolOptions'></div>
			<br><br>
			<div id='states'></div>
		      </td>
		    </tr>
		  </table>
		</div>
	      </div>
	      
	      <div id ='spectrumDB' class="tab-pane">
		<h2 align="left">MFTheory Core Excitation Spectrum Database</h2>
		
		<p align="left">
		  If you cannot find what you are looking for, try these:<br>
		  <a href="http://unicorn.mcmaster.ca/corex/Search-List.html">Mcmasters Gas phase Core Excitation experimental database.</a><br>
		  <a href="http://foundry.lbl.gov/">Molecular Foundry collected experimental database.</a>
		</p><br>
		
		--WebXS predicted Spectra<br>
		<input type="Text" id="spectraSearchTerms" value=""/>
		<button onclick="searchSpectrumDB()">Search</button>
		<div id="searchSpectrumResults"><br></div>
	
		<!--
		<a href="Methane.xyz" download>Click here to download</a>
		
		<script>  
		uriContent = "data:application/octet-stream," + encodeURIComponent("testthisout");
		newWindow=window.open(uriContent, 'neuesDokument');
		</script>
		-->
	      </div>


	      <!--
		 Search previously run spectra for a result, note this template accesses a bunch of js preloaded in < head >
		 This is done so the same code doesn't need to be repeated here and webdynamics -->
	      <div id='searchDB' class="tab-pane">
		<div id='searchWrapper'>   
	          <h2 align="left">MFTheory Structure Database</h2>
		  <?php
		     include("../templates/search.html.template")
		     ?>
		</div>
	      </div>
	      <!--End-->
	      
 	      <!--What is ShirleyXAS and what does it do?-->
	      <div id='shirleyinfo' class="tab-pane active">
	  	    <img src="images/WebXS.png" style="padding:20px" align="left" height="200" width="200"/> <br>
		    <p align="left">To facilitate the simulation of near-edge x-ray absorption fine structure (NEXAFS) spectra using established 
methods based on constrained density functional theory (DFT), we have developed a web-based interface which enables the user to launch such 
simulations on high-performance computing resources at the National Energy Research Scientific Computing Center (NERSC). Using the NERSC web toolkit 
(NEWT), we have written a web-interface to run highly parallelized DFT calculations based on this Web2.0 API. The interface enables the user to 
upload/build their molecule/crystal of choice, visualize it, generate its NEXAFS spectrum and interpret spectral features based on visualization of 
electronic excited states.<br><br>This tool uses modern HTML 5 techniques, and is not buit with backwards compatibility for  IE.  Please use a
different browser, such as the latest Firefox or Google Chrome for full functionionality.  Other browsers are not tested.<br><br> Please send 
questions and comments regarding the website to jwonsever@lbl.gov.
  	  	    <!--Needs work-->
		    <button valign="middle" onclick="window.open( './faq.html');">FAQ and Tutorial</button></p>
		    <p><center>
		    <img src="images/MFLogo.png" width="300px" height="100px"/>
		    <img src="images/NERSCLogo.png" width="200px" height="100px"/>
		    <img src="images/JSmol_logo13.png" width="250px" height="100px"/><br>
		    </center><p>
	      </div>
	      <!-- End -->

	      <!--All Functions for editing and manipulating molecules-->
	      <div id='moleculeEditor' class="tab-pane">
		    <h3>Jmol Molecular Viewer</h3><br>

		    <a href="javascript:Jmol.showInfo(mainApplet, true);">2D</a>
		    <a href="javascript:Jmol.showInfo(mainApplet, false);">3D</a>
		    <script>
		      JmolInfo.width = 600;
		      mainApplet = Jmol.getApplet(mainApplet, JmolInfo);
		      jmeApplet = Jmol.getJMEApplet(jmeApplet, JMEInfo, mainApplet);
		      initMainApp();
		    </script>
		    <br> 
		    <a href="#" onclick="window.open('http://chemapps.stolaf.edu/jmol/docs?ver=13.0');")>Command Line Instructions</a><br>
		    <script>
		      Jmol.jmolCommandInput(mainApplet);
		    </script>

		    <form id='loadfile'>
		      <input type=file id="uploadfile" name="uploadfile" onchange="uploadToEditor(this.form)"/>
		      <br></form>	 
     
		    <button onClick="minimizeStructure()">Optimize Structure</button>
		    <button onClick="toggleModelkitMode()" id="mkmode" value="Off">Toggle Model Kit Mode</button>
		    <button onClick="portCoordinates()">Port Coordinates</button><br>
		    <a href="../WebDynamics/index.html">WebDynamics</a>
	      </div> 
	      <!-- End -->

	      <!-- Submitting a job form. -->
	      <div id='submitjobs' align=left class="tab-pane">

		<!-- These wrappers allow us to block on failed authentication -->
		<div id='wrapper2'>
		    <h3>Submit Calculation</h3>
                    <table valign=top><tr><td valign=top>
		    <form id='inputs' name='inputs' METHOD="GET">
		      Material Name<br>
		      <input type='text' name='MOLNAME' id='material' value="MolName"/><br>
		      Excited Atoms<br>
		      <input type="text" name="XASELEMENTS" id="XASELEMENTS" value="C" onkeyup="addSelections();"/><br>
		      <!-- </td></tr></tr><td> -->
		      Coordinates<br><div id = "Coordinates"></div>
		      <script>makeCoordsDiv()</script><br>
		      Upload Coordinates<br>
		      <input type=file id='uploadfile' name='uploadfile' onchange="uploadCoordinates()">
		      <!-- Adds a new coordinates, upload loads to the current coordinates. -->
		      <br>
		      <div id="advancedOptions" style="display:none;"><br>
		        <b>Advanced Options</b><br><br>
		      <!--Advanced Options-->
                      <label class="checkbox">
		      <input type="Checkbox" onchange="makeCellSize()" id="CrystalFlag" value="True"/> Crystal?
                      </label>
		      <input type="Text" id="IBRAV" size="4" value="14"/> Bravais Lattice<br>
		      <br>Cell Size Attributes: (In &#8491;ngstr&#246;ms)<br>
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellA" value="10"/> A
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellB" value="10"/> B
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellC" value="10"/> C
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellAlpha" value="90"/> &alpha;
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellBeta" value="90"/> &beta;
		      <input type="text" size="10" onchange="drawMolInPreview()" id="CellGamma" value="90"/> &gamma;
		      <br><br>
                      <table cellpadding=8>
			<tr><td>
			    Output Directory:</td><td><input type="Text" size="25" id="outputDir" value="/global/scratch/sd/invalid"/>
			</td></tr>
			<tr><td>
			    Input Block:</td><td><input type="Text" size="35" id="customInputBlock" value="DEFAULT"/>
			    <br>Note, values here may overrule values in your custom input block.<br>
			</td></tr>

		      <tr><td>Cluster:</td><td>
		      <select name="Select Cluster" id="machine" onchange="form.PPN.value=24; updateStatus(this.value);" value="hopper">
		        <option value="hopper">Hopper</option>
                      </select> <br><span id='clusterStatus' style='display:none;'></span></td></tr>
		  
		      <tr><td>Nodes Per Atom:</td><td>
		      <input type="Text" id="NPERATOM" value="1"/></td></tr>
		      <tr><td>BND_FAC:</td><td>
		      <input type="Text" id="NBANDFAC" value="5"/></td></tr>
		      <tr><td>Total Charge:</td><td>
		      <input type="Text" id="TOTCHG" value="0"/></td></tr>
		      <tr><td>K-Points</td><td>
		      <input type="Text" id="KPOINTS" value="1 1 1"/></td></tr>

		      <tr><td>Queue:</td><td>
		      <select name="Select Queue" id="Queue" value="regular"
			  onChange="if(this.value=='debug') $('#wallTime').val('00:30:00'); else $('#wallTime').val('01:30:00');">
		        <option value="regular">Regular</option>
		        <option value="debug">Debug</option>
		        <option value="low">Low priority</option>
		        <option value="premium">Premium*</option>
		        <option value="scavenger">Scavenger**</option>
                      </select>
		      &nbsp;&nbsp;<a href="#" onclick="window.open('http://www.nersc.gov/users/computational-systems/hopper/running-jobs/queues-and-policies/');")>About</a>
                      </td></tr>		  

		      <tr><td>Walltime:</td><td><input type="Text" id="wallTime" value="01:30:00"/></td></tr>
		      <tr><td>Account:</td><td><input type="Text" id="acctHours" value="m1707"/></td></tr>
		      <tr><td>PPP:</td><td><input type="Text" id="PPP" value="4"/></td></tr>
		      <tr><td>NTG:</td><td><input type="Text" id="NTG" value="24"/></td></tr>
                      </table>

		      <!-- Hidden From User todo-->
		      <input type="hidden" id="NEXCITED" value="1"/>
		      <input type="hidden" id="PPN" value="24"/>

		      <input id="Open Full API" type='button' value='Full API' onClick="window.open('./API.php')"/>
		      <!-- End Advanced Options -->
		      </div>
		      <input id="lessAdvanced" style="display:none" data-inline='true' type='button' value='Hide Advanced Options' onClick="hideAdvancedOptions()"/>
		      <input id="AdvancedButton" data-inline='true' type='button' value='Advanced Options' onClick="showAdvancedOptions()"/>

		      <input id="Submit" data-inline='true' type='button' value='Submit' onClick="validateInputs(this.form)"/>
		      <div id="subStatus"></div>
         
		    </form>
		    </td><td width=400></td><td valign=top>
		      <!--Molecule Preview -->
		      Preview<br>
		      <div style="background-color:black;color:#AA2222;">
		        <span id="previewLoadingText" style="float:left;">Please Wait for Jmol to load....</span>
		        <script>
		          JmolInfo.width = 400;
		          JmolInfo.height = 400;
		          previewApplet = Jmol.getApplet(previewApplet, JmolInfo);
			  initPreviewApp();
		        </script>
		      </div><br>
		      <br>

		      <span id="animatePreviewButton">
			<input type='button' value='Animate' onClick="animatePreview();">
		      </span><span id="animatePreviewStop" style="display:none">
			<input type='button' value='Stop Animation' onClick="drawMolInPreview();">
		      </span>
		      <span>&nbsp&nbsp<b> Supercell:</b> X
		      <select id='SupercellX' onChange="supercellPreview()" style="width: 50px">
		        <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
                      </select> Y
		      <select id='SupercellY' onChange="supercellPreview()" style="width: 50px">
                         <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
	              </select> Z
		      <select id='SupercellZ' onChange="supercellPreview()" style="width: 50px">
                         <option value="1">1</option><option value="2">2</option><option value="3">3</option><option value="4">4</option>
	              </select>
                      </span>
                      <span>
			  <input type='button' value='Make Supercell the Unitcell' onClick="adjustToSupercell();">
			  Note: Can get BIG.
                      </span>
		      <br>
		      <span><input id="DisplaySearchQuery" type="button" value="Display from Search Query" onclick="displaySearchResult(previewApplet)"/></span><br>
		      <span><input type='button' value='OpenWebDynamicsFile' onClick="openTransferFile();"></span><br>		      
		      <span><input type='button' value='Center Atoms' onClick="centerCoords();"></span><br>
		      
		      </td></tr></table>
		</div>
	       </div>
	    </div>
          </div>
        </div>
      </div>

      <hr>

      <footer>
	<p>JSmol: an open-source HTML5 viewer for chemical structures in 3D. http://www.jmol.org/<br>
	  &copy; LBL LDRD 2013</p>
      </footer>

    </div> <!-- /container -->

    <script>
      myCheckAuth();
      makeCoordsDiv();
      getXCHShift();
    </script>


  </body>
</html>

