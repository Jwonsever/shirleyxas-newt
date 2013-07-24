<?

//Get the input values
     $filename="./TMP_INPUT.in";
     //Open the file

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
        $value=substr( $value, 4, strpos($value, "=")-4);

	if ($_POST[$value]) {
	    echo $value;
	    file_put_contents("../Shirley-data/tmp/tmpblocktest.in", $_POST[$value], FILE_APPEND);
	}
     }

     fclose($fp);

?>