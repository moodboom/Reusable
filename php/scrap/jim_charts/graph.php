<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>Graphs /export/home pugsly</title>
<meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
</head>
<body bgcolor="#ffffcc">
<center><img src="pugsly_gf.php" ></center>
<center><img src="pugsly_fordate.php" ></center>

<?php
if (!isset($_REQUEST["dir"])) {
  print "Error in page request....";
  exit(1);
}
$dir=explode("/",$_REQUEST["dir"]);
if(!$dir[3]) {
  print "Error in page request....";
  exit(1);
}
$dir=$_REQUEST["dir"];
print "<center><img src=\"pugsly_dir.php?dir=$dir\" ></center>";

$fileout=fopen("chartdata","w");
fwrite($fileout,"700:Space Used Over 30 Day Period:3:3:4:15\n");
for($count=29;$count>=0;--$count) {
  $day[$count]=date("Ymd",mktime(0,0,0,date("m"),date("d")-$count,date("Y")));
  $daylabel=date("m/d",mktime(0,0,0,date("m"),date("d")-$count,date("Y")));
  $filename="/home/jbrown/quotausage/".$day[$count]."-diskquota.out";
  if ($file=fopen($filename,"r")) {
    unset($filearray);
    while(!feof($file)) {
      $filearray=explode(":",fgets($file));
      if($filearray[1] == $dir) {
        fwrite($fileout,"$daylabel:$filearray[5]");
      }
    }
  }
}
fclose($fileout);

print "<img src=\"chart.php?file=chartdata>";

?>

</body>
</html>
