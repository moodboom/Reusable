#!/usr/bin/php

<?php

require 'jsmin-1.1.1.php';

if ($argc != 2 || in_array($argv[1], array('--help', '-help', '-h', '-?')))
{
    echo "Usage: php ./minify.php filename >newfilename \n";
    exit;
}
    
$file = $argv[1];

// Output a minified version of example.js.
echo JSMin::minify(file_get_contents($file));

?>