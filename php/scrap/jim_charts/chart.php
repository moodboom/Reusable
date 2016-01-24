<?php
if (!isset($_REQUEST["file"])) {
  exit;
}

// read chart data file
if ($file=fopen($_REQUEST["file"],"r")) {
  unset($filearray);
  list($width,$title,$titlefont,$datalabelfont,$datafont,$tics)=explode(":",fgets($file));
  while(!feof($file)) {
    $array=fgets($file);
    if($array) {
      list($key,$datapoint)=explode(":",$array);
      $data[$key]=(int)$datapoint;
    }
  }
  fclose($file);
} else {
  exit;
}

//Find longest data label and count the total bars to display

$datapoints = 0;
$maxlablen = 0;

foreach ( $data as $key => $datapoint ) {
  $datapoints++;
  if (strlen($key) > $maxlablen) {
    $maxlablen=strlen($key);
  }
}

// figure out chart label info and graph height
$labelpixlen=imagefontwidth($datalabelfont)*$maxlablen;
$labelpixheight=imagefontheight($datalabelfont)+16;
$titleypos = imagefontheight($titlefont);

$height = $titleypos+3+($labelpixheight*$datapoints);

//create image
$image = imagecreate($width,$height);

/* Colors */
$black = imagecolorallocate($image,0,0,0);
$white = imagecolorallocate($image,255,255,255);
$red   = imagecolorallocate($image,255,0,0);
$grey  = imagecolorallocate($image, 204, 204, 153);

imagefill($image,0,0,$grey);

// Create border around image
imageline($image, 0, 0, 0, $height, $black);
imageline($image, 0, 0, $width, 0, $black);
imageline($image, $width-1, 0, $width-1, $height-1, $black);
imageline($image, 0, $height-1, $width-1, $height-1, $black);

// display title on graph
$titlesize = imagefontwidth($titlefont)*strlen($title); //pixel width
$titlexpos = ($width/2) - ($titlesize/2);
imagestring($image,$titlefont,$titlexpos,0,$title,$black);

// Display the labels
$starty=$titleypos+10;
foreach ($data as $key => $datapoint) {
  imagestring($image,$datalabelfont,1,$starty,$key,$black);
  $starty+=$labelpixheight;
}

//draw border around actual graph
$startx=$labelpixlen+10;
$endy=$titleypos+2;

imageline($image,$startx , $endy, $startx, $height, $black);
imageline($image, $startx, $endy, $width, $endy, $black);

//draw lines between graphs
for ($count=0; $count < $datapoints; $count++ ) {
  imageline($image, $startx, $endy+($count*$labelpixheight), $width, $endy+($count*$labelpixheight), $black);
}

//draw data lines
$edge=($width-$startx)/$tics;
for ($count=0; $count < $tics; $count++ ) {
  imageline($image, $startx+($count*$edge), $endy, $startx+($count*$edge), $height-1, $black);
}

//Find max value to display
$big = max($data);

//draw actual data graph
$pixelpoints = $width/$big;
reset($data);
$starty=$titleypos+10;
for ($count=0; $count < $datapoints; $count++ ) { 
  list($key,$point)=each($data);
  $ploty1 = $endy+($labelpixheight*$count)+4;
  $ploty2 = $endy+($labelpixheight*$count)+($labelpixheight-4);
  $plotx2 = $pixelpoints*$point;
  $numout = number_format($point);
  $length = imagefontwidth($datafont)*strlen($numout);
  $startstring = (($plotx2-3)-$length);
  if ($plotx2<$startx) $plotx2=$startx;
  imagefilledrectangle($image,$startx,$ploty1,$plotx2,$ploty2,$red);
  if($startstring>$startx)  {
    imagestring($image,$datafont,$startstring,$starty,$numout,$white);
  }
  $starty+=$labelpixheight;
}

header("Content-type: image/png");
imagepng($image);
?>
