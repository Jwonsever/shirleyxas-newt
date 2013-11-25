<!DOCTYPE html>
<?php if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; } ?>
<html>
  <head>
    <title>MFTheory Webtools</title>
    <!-- Style -->
    <?php include($dir.'php/style.php') ?>
    <!-- JavaScript -->
    <?php include($dir.'php/javascript.php') ?>
  </head>
  <body>

    <!-- Navigation Bar -->
    <?php include($dir.'php/navbar.php') ?>

    <!-- Content -->
    <div class="container">
      <div class="hero-unit">
        <h2>Welcome to Molecular Foundry Theory Web Tools</h2>
        <p>These tools are made for theorist and experimenalist to facilitate the computaion of
	  X-Ray absorption spectra. Log in with your NERSC account to continue.</p>
        <p><a class="btn btn-primary btn-large" href="about.html">Learn more. &raquo;</a></p>
      </div>

      <div class="row">
        <div class="span4">
        <center><img src='images/LBNL_Full_Logo_Final.png' width=250></center>
        </div>
        <div class="span4">
        <center>
        <img src='images/mfbuilding.jpg' width=350>
        </center>
        </div>
        <div class="span4">
        <center>
        <img src='images/NERSC_New_logo.jpg' width=250>
        </center>
        </div>
      </div>

      <hr>

      <footer>
        <p>&copy; LBNL Molecular Foundry 2013</p>
      </footer>

    </div> <!-- /container -->

    <!-- Authentication -->
    <?php include($dir.'php/authenticate.php') ?>

  </body>
</html>
