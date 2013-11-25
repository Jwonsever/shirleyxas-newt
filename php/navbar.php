<?php
if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; }
echo <<< HTML
    <!-- Bootstrap -->
    <link href="{$dir}bootstrap/css/bootstrap.min.css" rel="stylesheet" media="screen">
    <link href="{$dir}bootstrap/css/bootstrap-responsive.min.css" rel="stylesheet" media="screen">
    <!-- Navigation Bar -->
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
              <li><img src="{$dir}images/mfnewlogo.png" width=105 padding=0 border=0></li>
              <li class="active"><a href="{$dir}index.php">Home</a></li>
              <li><a href="https://portal.nersc.gov/project/als/beta/index.html">ALS Simulation Portal</a></li>
              <li class="dropdown">
                <a href="#" class="dropdown-toggle" data-toggle="dropdown">Simulation Tools <b class="caret"></b></a>
                <ul class="dropdown-menu">
                  <li class="nav-header">X-Ray Absorption</li>

                  <li><a href="{$dir}WebXS/index.php">WebXS</a></li>
		  <li><a href="{$dir}WebDynamics/index.php">WebDynamics</a></li>				    
                  <li><a href="http://leonardo.phys.washington.edu/feff/">FEFF</a></li>
                  <li class="divider"></li>
		  <li class="nav-header">Web Scraping</li>
		  <li><a href="http://matzo.lbl.gov">Matzo</a></li>
                  <li class="divider"></li>
		  <li class="nav-header">DFT</li>
		  <li><a href="{$dir}WebRelax/index.php">WebRelax</a></li>				    
		  <li><a href="{$dir}WebplusU/index.php">Web+U</a></li>				    
                  <li class="divider"></li>
                </ul>
              </li>
            </ul>
            <div class="pull-right" id="auth-spinner"> <img src="{$dir}images/white-ajax-loader.gif"></div>
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
    <!-- Navbar done -->
HTML
?>
