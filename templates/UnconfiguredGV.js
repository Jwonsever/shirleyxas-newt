/*
James Wonsever
Lawrence Berkeley Laboratory
Molecular Foundry
05/2012 -> Present

Global value settings.
This file acts as a header JS file, defining constants for future use.

USAGE:
  Update both this file AND globalVaules.in, these two files MUST MATCH up to //END

TODO:
  ./updateGV.sh
   -Re-reads changes to GlobalValues.in.
   -Rewrites them here as javascript.
  Manditory for changes in GlobalValues.in to take effect here as well.
 */

//GlobalValues.in
//Where this version of the code is 
var CODE_BASE_DIR="TMP_HOME/";
var SERVER_URL = "TMP_ADDR/WebXS";


//Shirley and Psuedos
var SHIRLEY_ROOT="TMP_HOME/WebXS/xas_input/shirley_QE4.3-www";
var PSEUDO_DIR="/project/projectdirs/mftheory/pseudos";//IDK IF THIS HAS TO CHANGE OR NOT

//Relative locations
var CODE_LOC="WebXS";
var DYN_LOC="WebDynamics";
var DATA_LOC="Shirley-data";
var XCH_SHIFTS="xas_input/XCHShifts.csv";

//Other relative locations
var JMOL_VERSION="jmol-13.3.3";
var JAVASCRIPT="javascript";
var JMOL_SCRIPTS="jmolScripts";
var IMAGES="images";
var SERVER_SCRIPTS="shell_commands";
var XAS_INPUTS="xas_input";
var DEFAULT_INPUT_BLOCK="xas_input/InputBlock.in";
//END

//Other Constants, may be built from above values.

/* Constants */
var FILES_PER_PAGE = 20; //for reading results/jobs (OLD)
var SHELL_CMD_DIR = CODE_BASE_DIR + "/WebXS/shell_commands/"; //Where related bash scritps are.
var DATABASE_DIR = CODE_BASE_DIR + DATA_LOC; //Where spectra-database is.  Also home to temp state files.
var GLOBAL_SCRATCH_DIR = "/global/scratch/sd/"; //postpend USER, global output Directory
var LOCAL_SCRATCH_DIR = "${SCRATCH}"; //Dont postpend USER, local output dir
var SHIFTS_DATABASE_CSV = CODE_BASE_DIR + "/WebXS/xas_input/XCHShifts.csv";//EV Shifts

/* App Flags */
var appletReady = false;
var previewReady = false;
var resultsAppReady = false;

/*Not Used*/
var JMOL_SCRIPT_DIR = "http://portal.nersc.gov/project/mftheory/www/james-xs/jmolScripts/"; //Where related jmol scritps are. (NOT USED)

/* Molecular System Parameters */
var MIN_CELL_SIZE = 10;//angstroms (Can be overridden)
var CELL_EXPAND_FACTOR = 10;//How far to expand the unit cell (Additive factor)
var CrystalSymmetry = null; // flag set when crystal is loaded, use for crystal data
