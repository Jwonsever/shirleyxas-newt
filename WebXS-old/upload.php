<?php
if ($_FILES["file"]["error"] > 0)
  {}
else
  {
  echo file_get_contents($_FILES["file"]["tmp_name"]);
  }
?> 