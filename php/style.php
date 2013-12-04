<?php
if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; }
echo <<< HTML
    <style>
      body {
        padding-top: 60px; /* 60px to make the container go all the way to the bottom of the topbar */
      }
    </style>
    <link href="{$dir}css/als_portal.css" rel="stylesheet" type="text/css" />
HTML
?>
