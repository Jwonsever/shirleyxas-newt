<!DOCTYPE html>
<html>
  <head>
    <title>Web Dynamics</title>
    <!-- Bootstrap -->
    <link href="../bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">
    <style>
      body {
        padding-top: 60px; /* 60px to make the container go all the way to the bottom of the topbar */
      }
    </style>

    <link href="../bootstrap/css/bootstrap-responsive.min.css" rel="stylesheet" media="screen">
    <link href="../als_portal.css" rel="stylesheet" type="text/css" />
    <link href="wd.css" rel="stylesheet" type="text/css" />

    <!-- <script src="http://code.jquery.com/jquery-latest.js"></script> -->
    <script src="../js/jquery-latest.js"></script>
    <script src="../bootstrap/js/bootstrap.min.js"></script>
    <script src="../js/mynewt.js"></script>
    <script src="../js/alsapi.js"></script>
    <script src="../js/als_portal.js"></script>

    
    <!--Scripts required for JME injection-->
    <script src="javascript/jsmol-13.1.15/jsmol/jsme/jsme/jsme.nocache.js" language="javascript" type="text/javascript">
    </script>

    <!--jsMol-->
    <script type="text/javascript" src="javascript/jsmol-13.1.15/jsmol/JSmol.min.nojq.js"></script>
    <script type="text/javascript" src="javascript/jsmol-13.1.15/jsmol/js/JSmolJME.js"></script>

    <!--Downloadify (Req. Flash 10)-->
    <script type="text/javascript" src="javascript/Downloadify/js/swfobject.js"></script>
    <script type="text/javascript" src="javascript/Downloadify/js/downloadify.min.js"></script>

    <!--Local-->
    <script type="text/javascript" src="../GlobalValues.js"></script>
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
      j2sPath: "javascript/jsmol-13.1.15/jsmol/j2s",

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
                  <li><a href="../WebXS/index.php">WebXS</a></li>
		  <li class="active"><a href="#">WebDynamics</a></li>
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
      <div class="hero-unit">
        <h2>Web Dynamics</h2>
        <p>This page is meant for the searching of chemical structures, DFT relaxations,
 crystal structure interpretations, manipulation of chemical structures, and computation
of molecular dynamics and trajectories.  These advanced, realistic structures can then be
sent to and accessed by other tools located in the portal.  Work in progress. </p>

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
	    <br>
	    <input type="text" id="mainApplet_query" onkeypress="13==event.which&amp;&amp;Jmol._applets['mainApplet']._search()" size="32" value="">
	    <select id="mainApplet_select">
	      <option value="$" selected>NCI(small molecules)</option>
	      <option value=":">PubChem(small molecules)</option>
	      <option value="=">RCSB(macromolecules)</option></select>
	    <button id="mainApplet_submit" onclick="Jmol._applets['mainApplet']._search()">Search</button>
	    <br>
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
	    <button onClick="sendToWebXS()">NEXAFS</button>
	    <button onClick="SaveFile()">Save on Hopper:</button>
	    <span><object id="downloadify"></object></span>
	    <button onClick="">Undo</button>
	    <button onClick="">Redo</button>
	  </span>
	</div>
	
      </div>

      <center>
	  <img src="./images/MFLogo.png" width="300px" height="100px"/>
	  <img src="./images/NERSCLogo.png" width="200px" height="100px"/>
	  <img src="./images/JSmol_logo13.png" width="250px" height="100px"/><br>
      </center>

      <r>
      <footer>
	<p>JSmol: an open-source HTML5 viewer for chemical structures in 3D. http://www.jmol.org/<br>
	  &copy; LBL LDRD 2013</p>
      </footer>

    </div> <!-- /container -->

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
      swf: 'javascript/Downloadify/media/downloadify.swf',
      downloadImage: 'javascript/Downloadify/images/download.png',
      width: 100,
      height: 30,
      transparent: false,
      append: false
      });
		
    </script>

  </body>
</html>

