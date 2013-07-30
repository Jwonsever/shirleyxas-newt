<!DOCTYPE html>
<meta charset="utf-8">

<HTML>
  <link href="../bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">
  <link href="../bootstrap/css/bootstrap-responsive.min.css" rel="stylesheet" media="screen">
  <link href="../als_portal.css" rel="stylesheet" type="text/css" />

  <script src="../jquery-latest.js"></script>    
  <script src="../bootstrap/js/bootstrap.min.js"></script>
  <script src="../mynewt.js"></script>
  <script src="../alsapi.js"></script>
  <script src="../als_portal.js"></script>
  <script> 
    function checkAuthCallback() {;}    
    myCheckAuth();
  </script>

   <HEAD>
      <TITLE>Exposed Shirley Inputs</TITLE>
   </HEAD>

<BODY>

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
              <li><img src="../alslogo.png" width=56 padding=0 border=0></li>
              <li><a href="../index.html">Home</a></li>
              <li><a href="../about.html">About</a></li>
              <li><a href="../status.html">Status</a></li>
              <li><a href="../data.html">Data Browser</a></li>
              <li class="dropdown">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">Simulation Tools <b class="caret"></b></a>
                <ul class="dropdown-menu">
                  <li class="nav-header">X-Ray Scattering</li>
                  <li><a href="#">GISAXS Code 1</a></li>
                  <li><a href="#">GISAXS Code 2</a></li>
                  <li class="divider"></li>
                  <li class="nav-header">X-Ray Absorption</li>
                  <li class="active"><a href="#">WebXS</a></li>
		  <li><a href="../WebDynamics/index.html">WebDynamics</a></li>
                  <li><a href="#">FEFF</a></li>
                  <li class="divider"></li>
                  <li class="nav-header">Photoemission</li>
                  <li><a href="#">BerkeleyGW</a></li>
                </ul>
              </li>
            </ul>
            <div class="pull-right" id="auth-spinner"> <img src="../white-ajax-loader.gif"></div>
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
  <br>
  <hr>
  <H3>Here is the ResetVariables full list of inputs.</H3>       
  <p><input TYPE="button" onClick="parent.location='index.html'" value="Back">
  Anything you check here will go into a temporary input file appended to InputBlock.in.</p>
  <p>If you are here, I assume you know how to properly format your inputs, because I don't, and I don't check them for you.</p>
  
  <!--form action="APIprocess.php" method="post"-->
  <form>
  <?php
     //Get all of the valid inputs to Shirley from ResetVariables

     $filename="./TMP_INPUT.in";
     // Open the file

     $fp = fopen($filename, 'r'); 

     $fLines=array();

     // Add each line to an array
     if ($fp) {
        $cont = file_get_contents($filename);
        $fLines = explode("\n", $cont);
     }
     
     $pattern="/^TMP.*$/";
     $inputLines=preg_grep($pattern, $fLines);
     
 
     foreach ($inputLines as &$value) {
        $value=substr( $value, 4, strpos($value, "=")-3);
        echo $value;
        $value=substr($value, 0, strpos($value, "="));
        echo "<input class='inputBlockValue' name='".$value."' id='".$value."' type='text' size='30'/><br>\n";
     }

     fclose($fp);

     //Read in Input Block
     $fp = fopen("./xas_input/Input_Block.in", 'r'); 
     $fLines=array();

     // Add each line to an array
     if ($fp) {
        $cont = file_get_contents("./xas_input/Input_Block.in");
        $fLines = explode("\n", $cont);
     }
     $pattern="/^.*=.*$/";
     $fLines=preg_grep($pattern, $fLines);
     foreach ($inputLines as &$value) {
       foreach ($fLines as &$inpbl) {
           $curVal=substr( $inpbl, strpos($inpbl, "=") + 1);
           $curVal=str_replace(chr(39),chr(34),$curVal);

           $pref=substr( $inpbl, 0, strpos($inpbl, "="));
           //echo $value . "=" . $pref."\n";

           if($pref == $value) {
              echo "<script type='text/javascript'>$('#".$value."').val('".$curVal."');</script>\n";
           }
       }
     }
     ?>

  <input type="button" value="Save Custom Input_Block.in" onClick="writeFile()">
  </form>
</div>

<script>
  function writeFile() {
  
  filename = "/global/scratch/sd/" + myUsername + "/CustomBlock.in";
  var customblock = "";
  $( ".inputBlockValue" ).each(function( key, value ) {
    myVal = $(this).val();
    if(myVal == "") return;
    if($(this).attr('id') == "K_POINTS") { //Give it the newline in the middle
      var typeKP = myVal.split(" ");
      var firstLine = typeKP.shift() + " " + typeKP.shift(); //Grab first two elements
      var secLine = typeKP.join(" ");
      myVal = firstLine + "\n" + secLine;
    }
    customblock = customblock + $(this).attr('id') + "=" + myVal + "\n";
    console.log(customblock);
  });


  $.newt_ajax({type: "PUT", 
  url: "/file/hopper" + filename,
  data: customblock,
  success: function(res) {
    console.log("Written to Hopper");
    //chmod it 777
    $.newt_ajax({type: "POST", 
    url: "/command/hopper",
    data: {"executable": "/bin/chmod 777 "+filename},
    success: function(res) {
    console.log("Chmoded the file.  No residuals.");
    },});
    alert("File written to filesystem, here: " + filename + "\nInput into WebXS in advanced options.");
  },});  

  }
</script>
</BODY>
</HTML>
