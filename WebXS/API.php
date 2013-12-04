<!DOCTYPE html>
<?php 
  include('relpath.php');
  if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; }
?>
<meta charset="utf-8">
<html>
  <head>
      <TITLE>Exposed Shirley Inputs</TITLE>
    <!-- Style -->
    <?php include($dir.'php/style.php') ?>
    <!-- JavaScript -->
    <?php include($dir.'php/javascript.php') ?>

  <script> 
    function checkAuthCallback() {;}    
    myCheckAuth();
  </script>

<BODY>

    <!-- Navigation Bar -->
    <?php include($dir.'php/navbar.php') ?>


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
  
  filename = GLOBAL_SCRATCH_DIR + myUsername + "/CustomBlock.in";
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
