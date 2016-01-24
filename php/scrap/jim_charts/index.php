<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN"
"http://www.w3.org/TR/html4/strict.dtd">
<html>
 <head>
  <title>pugsly /export/home usage</title>
  <meta http-equiv="Content-Type" content="text/html; charset=ISO-8859-1">
 </head>
 <body bgcolor="#ffffcc">
<center><img src="pugsly_pdu.php"></center>
<center><img src="pugsly_fordate.php"></center>
<br>
Click on Column name to sort on that column<br>
Click on directory name to see 30 day history for that directory<br>
<?php
$graph_page="graph.php";
$filename="/home/jbrown/quotausage/" . date("Ymd") . "-diskquota.out";
if(!$file=fopen($filename,"r")) {
  $dt=date("Y-m-d") . " 00:00:00";
  $tm=strtotime($dt);
  $filename="/home/jbrown/quotausage/" . date("Ymd",$tm - 24*60*60) . "-diskquota.out";
  $file=fopen($filename,"r") or die("Unable to find file $filename");
}

while (!feof($file)) {
  $filearray[]=explode(":",fgets($file));
}
if (isset($_REQUEST["sort"])) {
  foreach ($filearray as $key => $row) {
    $name[$key]=$row[0];
    $dir[$key]=$row[1];
    $usage[$key]=$row[2];
    $limit[$key]=$row[3];
    $hard[$key]=$row[4];
    $used[$key]=$row[5];
  }
  switch($_REQUEST["sort"]) {
    case "name":
      array_multisort($name,SORT_NUMERIC,SORT_DESC,$filearray);
      break;
    case "dir":
      array_multisort($dir,SORT_STRING,SORT_ASC,$filearray);
      break;
    case "usage":
      array_multisort($usage,SORT_NUMERIC,SORT_DESC,$filearray);
      break;
    case "limit":
      array_multisort($limit,SORT_NUMERIC,SORT_DESC,$filearray);
      break;
    case "hard":
      array_multisort($hard,SORT_NUMERIC,SORT_DESC,$filearray);
      break;
    case "used":
      array_multisort($used,SORT_NUMERIC,SORT_DESC,$filearray);
      break;
  }
}

print "<Table border=\"1\">";
print "<tr>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=name\">Username</a></th>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=dir\">Home Dir</a></th>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=usage\">Quota Usage</a></th>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=limit\">Quota Limit</a></th>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=hard\">Quota Hard</a></th>";
print "<th><a href=\"$_SERVER[PHP_SELF]?sort=used\">Space Used</a></th>";
print "</tr>";

foreach($filearray as $info) {
  if ($info[0]) {
    print "<tr><td>" . $info[0] . "</td>";
    print "<td>" . "<a href=\"$graph_page?dir=$info[1]\">" . $info[1] . "</a></td>";
    print "<td>" . number_format($info[2]) . "</td>";
    print "<td>" . number_format($info[3]) . "</td>";
    print "<td>" . number_format($info[4]) . "</td>";
    print "<td>" . number_format($info[5]) . "</td></tr>";
    $spaceused+=$info[5];
  }
}
print "<tr><td>Total Space Used</td><td></td><td></td><td></td><td></td>";
print "<td>" . number_format($spaceused) . "</td></tr></table>";
fclose($file);
?>

</BODY>
</HTML>
