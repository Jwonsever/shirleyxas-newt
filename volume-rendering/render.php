<?php

  $dataset = "";
  if (isset($_GET['dataset'])){
    $dataset=$_GET['dataset'];
  }

  $montaddress = "http://portal.nersc.gov/project/als/renders/".$dataset."/montage.jpg";

  include "/global/project/projectdirs/als/www/renders/".$dataset."/stats.phpi";

  //echo "hello World";
  //echo $npixelsx;

  //$npixelsx = 0;
  //if (isset($_GET['npixelsx'])){
  //  $npixelsx=$_GET['npixelsx'];
  //}

  //$npixelsy = 0;
  //if (isset($_GET['npixelsy'])){
  //  $npixelsy=$_GET['npixelsy'];
  //}

  //$npixelsz = 0;
  //if (isset($_GET['npixelsz'])){
  //  $npixelsz=$_GET['npixelsz'];
  //}

  //$nx = 0;
  //if (isset($_GET['nx'])){
  //  $nx=$_GET['nx'];
  //}

  //$ny = 0;
  //if (isset($_GET['ny'])){
  //  $ny=$_GET['ny'];
  //}

?>


<head></head>
<body>
<p><script id="vshader" type="x-shader/x-vertex">// <![CDATA[
	attribute vec3 vPosition;
	varying vec3 v_Color;
	void main()
	{    
		v_Color = vPosition;
		gl_Position = vec4(vPosition, 1.0);
	}
// ]]&gt;</script><script id="fshader" type="x-shader/x-fragment">
#ifdef GL_ES
	precision mediump float;
#endif
	varying vec3 v_Color;
	void main()
	{
		gl_FragColor = vec4(v_Color, 1.0);
	}
</script><script type="text/javascript" src="webgl-utils.js"></script><script type="text/javascript" src="J3DI.js"></script><script type="text/javascript" src="J3DIMath.js"></script><script type="text/javascript" src="atvtdemo.js"></script><script type="text/javascript" src="atvoltex.js"></script></p>
<div style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;">
	<canvas id="webgl_canvas" style="border: 1px solid #202020; margin-top: 5px; margin-bottom: 5px; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" width="630" height="360"><br />
		Your browser does not support the HTML 5 &lt;canvas&gt; element.<br />
	</canvas><br />
	<button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="decreaseBrightness();">-</button><button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="increaseBrightness();">+</button>|<button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="decreaseSize();">s</button><button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="increaseSize();">S</button>|<button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="toggleLinearFiltering();">HiQ</button> <span id="hiq" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;"></span> |<button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="decreaseNumberOfSteps();">.</button><button type="button" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" onclick="increaseNumberOfSteps();">&#8230;</button> <span id="steps" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;"></span> | <span id="framerate" style="-moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;">Select a dataset to start</span><br />
	<span style="display: inline-block; width: 33px; vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; 
-webkit-user-select: none; -webkit-user-select: none;">Red &nbsp;</span> <canvas style="margin-top: 10px; border: 1px solid #202020; 
vertical-align: 
middle; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" id="canvas_red" width="522" height="42"></canvas> <button style="vertical-align: bottom; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" type="button" onclick="clear2D(0);">Reset</button><br />
	<span style="display: inline-block; width: 33px; vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; 
-webkit-user-select: none; -webkit-user-select: none;">Green &nbsp;</span> <canvas style="margin-top: 10px; border: 1px solid #202020; 
vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" id="canvas_green" width="522" height="42"></canvas> <button style="vertical-align: bottom; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" type="button" onclick="clear2D(1);">Reset</button><br />
	<span style="display: inline-block; width: 33px; vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; 
-webkit-user-select: none; -webkit-user-select: none;">Blue &nbsp;</span> <canvas style="margin-top: 10px; border: 1px solid #202020; 
vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" id="canvas_blue" width="522" height="42"></canvas> <button style="vertical-align: bottom; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" type="button" onclick="clear2D(2);">Reset</button><br />
	<span style="display: inline-block; width: 33px; vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; 
-webkit-user-select: none; -webkit-user-select: none;">Alpha &nbsp;</span> <canvas style="margin-top: 10px; border: 1px solid #202020; 
vertical-align: middle; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" id="canvas_alpha" width="522" height="42"></canvas> <button style="vertical-align: bottom; -moz-user-select: -moz-none; -khtml-user-select: none; -webkit-user-select: none; -webkit-user-select: none;" type="button" onclick="clear2D(3);">Reset</button>
</div>
<p><script type="text/javascript">// <![CDATA[
	initCanvases();
<?php
        echo "loadCustomMontage(\"".$dataset."\",".$npixelsx.",".$npixelsy.",".$npixelsz.",".$nx.",".$ny.");";
        echo "\n";
?>
// ]]&gt;</script></p>
</body>