<!DOCTYPE html>
<?php 
  include('relpath.php');
  if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; }
?>
<html>
  <head>
    <title>Web+U</title>
    <!-- Style -->
    <?php include($dir.'php/style.php') ?>
    <link href="webplusu.css" rel="stylesheet" type="text/css" />

    <!-- JavaScript -->
    <?php include($dir.'php/javascript.php') ?>
    
<?php
echo <<< HTML
    <!--Scripts required for JME injection-->
    <script src="{$dir}js/jsmol-13.3.3/jsmol/jsme/jsme/jsme.nocache.js" language="javascript" type="text/javascript">
    </script>

    <!--jsMol-->
    <script type="text/javascript" src="{$dir}js/jsmol-13.3.3/jsmol/JSmol.min.nojq.js"></script>
    <script type="text/javascript" src="{$dir}js/jsmol-13.3.3/jsmol/js/JSmolJME.js"></script>

    <!--Downloadify (Req. Flash 10)-->
    <script type="text/javascript" src="{$dir}js/Downloadify/js/swfobject.js"></script>
    <script type="text/javascript" src="{$dir}js/Downloadify/js/downloadify.min.js"></script>

    <!--Local-->
    <script type="text/javascript" src="{$dir}GlobalValues.js"></script>
    <script type="text/javascript" src="WebDynamics.js"></script>

    <script>
      var amIMobile = navigator.userAgent.match(/iPad|iPhone|iPod|android/i) != null || screen.width <= 600;
	

      var mainApplet = "mainApplet";
      var jmeApplet = "jmeApplet";

      // logic is set by indicating order of USE -- default is HTML5 for this test page, though
      var use = "HTML5"; // JAVA HTML5 WEBGL IMAGE  are all otions     
      var s = document.location.search;
      Jmol.debugCode = (s.indexOf("debugcode") >= 0);


      var JmolLoadScript = "load :methane;javascript readCoordsFromJmol();javascript drawMol();"
      var JmolInfo = {
      width: 500,
      height: 500,
      debug: false,
      color: "black",
      addSelectionOptions: false,
      serverURL: "./javascript",

      //for jsmol
      use: "HTML5",
      j2sPath: "{$dir}js/jsmol-13.3.3/jsmol/j2s",

      //These are for java version
      //jarPath: "javascript/jmol-13-0",
      //jarFile: "JmolAppletSigned.jar",
      //isSigned: true,

      //disableJ2SLoadMonitor: true,
      //disableInitialConsole: true,
      allowJavaScript: true,
      console:"none",
      script: JmolLoadScript
      };
      
      var JMEInfo = {
      use: "HTML5"

      //for the java version
      //jarPath: "javascript/",
      //jarFile: "JME.jar"
      };
      

      Jmol.setGrabberOptions(
      [["$", "NCI(small molecules)"],
      [":", "PubChem(small molecules)"],
      ["=", "RCSB(macromolecules)"]]);
    </script>
HTML
?>


  </head>
  <body>

    <!-- Navigation Bar -->
    <?php include($dir.'php/navbar.php') ?>

    <div class="container">
      <div class="hero-unit">
        <h2>Web+U</h2>
        <p>This utility is provided to enable self-consistent DFT+U calculationsa la Cococcioni et al.
        </p>

	<ul class="optionsNavBar">
	    <li class="optionsDivider"></li>
	    <li><a href="#Basic" onclick="CngClass($('#basicOptions'))">Basic</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#Search" onclick="CngClass($('#searchOptions'))">Search</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#ModelKit" onclick="CngClass($('#ModelKitOptions'))">Model Kit</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#Coordinates" onclick="CngClass($('#CoordinatesOptions'));">Coordinates</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#Upload" onclick="CngClass($('#UploadOptions'))">Upload Files</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#Relaxations" onclick="CngClass($('#RelaxationOptions'))">Molecular Relaxations</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#MolecularDynamics" onclick="CngClass($('#DynamicsOptions'))">Molecular Dynamics</a></li>
	    <li class="optionsDivider"></li>
	    <li><a href="#JmolConsole" onclick="CngClass($('#JmolConsole'))">Jmol Console</a></li>
	    <li class="optionsDivider"></li>
	</ul>

	<hr>

	<div id="jmolApp">
	  
	  <script>
	    mainApplet = Jmol.getApplet(mainApplet, JmolInfo);
	    jmeApplet = Jmol.getJMEApplet(jmeApplet, JMEInfo, mainApplet);
	  </script>
	  <a href="javascript:Jmol.showInfo(mainApplet, true);">2D</a>
	  <a href="javascript:Jmol.showInfo(mainApplet, false);">3D</a>
	</div>

	<div id="optionsWrapper">
	  <div id="basicOptions" class="optionsDiv">
	    <br>
	    System Name<input type="text" id="structureName" value="WebDynamics"><br>
	    Unit Cell<input type="text" onkeyup="drawMol()" id="structureCell" value="10x10x10x90x90x90"><br>
	    Replication<input type="text" onkeyup="drawMol()" id="cellReplication" value="1x1x1"><br>
	    <input type="button" onclick="foldWrapper()" value="Fold Structure into Unitcell"><br>
	    <input type="button" onclick="createUnitcell()" value="Make Unitcell based on System Size"><br>
	  </div>

	  <div id="searchOptions" class="optionsDiv">
	    <?php
	       include($dir."templates/search.html.template");
	       ?>
	    <span><input id="DisplaySearchQuery" type="button" value="Display from Search Query" onclick="displaySearchResult(mainApplet)"/></span><br>
	  </div>

	  <div id="ModelKitOptions" class="optionsDiv">
	    Right click on the JSMol application in model-kit mode, and options for editing the molecule will appear.
	    <br>
	    <button onClick="toggleModelkitMode()" id="mkmode" value="Off">Toggle Model Kit Mode</button><br>
	    <button onClick="minimizeStructure()">Optimize Structure</button><br>
	    <button onClick="minimizeAddHydrogens()">Optimize Structure and Add Hydrogens</button>	    
	  </div>

	  <div id="CoordinatesOptions" class="optionsDiv">
	    Active Molecular Coordinates.  Editable.
	    <div id = "Coordinates"></div>
	  </div>

	  <div id="UploadOptions" class="optionsDiv">
	    Upload file, as crystal, specify cell?
	    <br>
	    <input type=file id='uploadfile' name='uploadfile' onchange="uploadCoordinates(this)">
	    <br>
	    Add feature to save current unitcells/coordinates
	  </div>

	  <div id="RelaxationOptions" class="optionsDiv">
	    Empirical Jmol Relaxations
	    <button onClick="minimizeStructure()">Optimize Structure</button><br>
	    <button onClick="minimizeAddHydrogens()">Optimize Structure and Add Hydrogens</button>	    
	    <hr>
	    Empirical Avagadro Relaxations
	    <hr>
	    Quantum Espresso DFT Relaxations
	    (<a href="#" onclick="alert(Submits a torque job, will appear here after completion.)">Warning</a>)
	    <br>Name your relaxation:<input id="name" type="text" />
	    <div id="completedRelaxations">
	      Here will be listed, in order, all successful relaxations completed, and names/dates
	    </div>
	    
	  </div>

	  <div id="DynamicsOptions" class="optionsDiv">
	    Run a cp2k simulation!
	    <br>
	    XYZ File?  (Will be the file shown in JMOL right now, AS IS!)
	    <br>
	    Change this in the other tabs.
	    <hr>
	    Temperature<input type="text" id="temperature" value="298">K<br>
	    Pressure<input type="text" id="pressure" value="1.0">Bar<br>
	    WallTime<input type="text" id="walltime" value="00:30:00"> (do n^2 on atoms?)<br>
	    Nodes<input type="text" id="nnodes" value="1"> (do [n_atoms/24]?)<br>
	    Snapshots<input type="text" id="snapshots" value="10"> To return, funct time/step<br>
	    <button onClick="runCp2k()">Run</button>

	    <div id="completedCP2K">
	      Here will be shown completed calculations
	    </div>

	  </div>
	  <div id="JmolConsole" class="optionsDiv">
	   Full jmol console, with help links
	   <script>
	    Jmol.jmolCommandInput("mainApplet");
	   </script>
	  </div>

	  <span id="optionsBottomBar">
	    <button onClick="sendToWebXS()">Send To WebXS</button>
	    <button onClick="SaveFile()">Save on Hopper:</button>
	    <span><object id="downloadify"></object></span>

	    <!-- This would make sense with a structure class, if that ever gets done.
	    <button onClick="">Undo</button-->

	  </span>
	</div>
	
      </div>

<?php
echo <<< HTML
      <center>
	  <img src="{$dir}images/MFLogo.png" width="300px" height="100px"/>
	  <img src="{$dir}images/NERSCLogo.png" width="200px" height="100px"/>
	  <img src="{$dir}images/JSmol_logo13.png" width="250px" height="100px"/><br>
      </center>
HTML
?>

      <r>
      <footer>
	<p>JSmol: an open-source HTML5 viewer for chemical structures in 3D. http://www.jmol.org/<br>
	  &copy; LBNL Molecular Foundry 2013</p>
      </footer>

    </div> <!-- /container -->

<?php
echo <<< HTML
    <script>
      myCheckAuth();
      var models = [""]; 
      var activeModel = 0; 
      makeCoordsDiv();

      //todo
      Downloadify.create('downloadify',{
      filename: function(){
      return "WebDynamics.xyz";
      },
      data: function(){ 
      return "tempDATA";
      },
      onComplete: function(){ alert('Your File Has Been Saved!'); },
      onCancel: function(){ alert('You have cancelled the saving of this file.'); },
      onError: function(){ alert('You must put something in the File Contents or there will be nothing to save!'); },
      transparent: false,
      swf: '{$dir}/js/Downloadify/media/downloadify.swf',
      downloadImage: '{$dir}/js/Downloadify/images/download.png',
      width: 100,
      height: 30,
      transparent: false,
      append: false
      });
		
    </script>
HTML
?>

  </body>
</html>
