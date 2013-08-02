//Initialize all of the jsmol info settings.
//Defines the basic settings for jsmol to run.

var amIMobile = navigator.userAgent.match(/iPad|iPhone|iPod|android/i) != null || screen.width <= 600;


var mainApplet = "mainApplet";
var jmeApplet = "jmeApplet";
var previewApplet = "previewApplet";

var resultsApplet = "resultsApplet";

// logic is set by indicating order of USE -- default is HTML5 for this test page, though
var use = "HTML5"; // JAVA HTML5 WEBGL IMAGE  are all otions     
var s = document.location.search;
Jmol.debugCode = (s.indexOf("debugcode") >= 0);


var JmolInfo = {
      width: 400,
      height: 400,
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
      script: ""
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
