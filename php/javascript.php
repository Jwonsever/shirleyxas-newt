<?php 
if (defined('RELPATH')) { $dir=RELPATH; } else { $dir=''; }
echo <<< HTML
    <script src='{$dir}js/jquery-latest.js'></script>
    <script src='{$dir}bootstrap/js/bootstrap.min.js'></script>
    <script src='{$dir}js/mynewt.js'></script>
    <script src='{$dir}js/alsapi.js'></script>
    <script src='{$dir}js/als_portal.js'></script>
HTML;
?>
