<?php

// MDM I use the "newer" mysqli php interface here.  Simple as pie.
// You need to specify the dev-lang/php [mysqli] USE flag.
// Example: http://www.php.net/manual/en/function.mysqli-connect.php

$mysqli = new mysqli(
    "localhost" , // host
    "todaydiet" , // mysql user
    "dothediet" , // mysql pwd
    "todaydiet"   // mysql dbname
);

/* check connection */
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}

?>
