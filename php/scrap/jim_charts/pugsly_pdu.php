<?php

function pc_ImageTTFCenter($image, $text, $font, $size) {
  // find the size of the image
  $xi = ImageSX($image);
  $yi = ImageSY($image);
  // find the size of the text
  $box = ImageTTFBBox($size, 0, $font, $text);
  $xr = abs(max($box[2], $box[4]));
  $yr = abs(max($box[5], $box[7]));
  // compute centering
  $x = intval(($xi - $xr) / 2);
  $y = intval(($yi + $yr) / 2);
  return array($x, $y);
}

$im = imagecreate(700, 45);

// white background and blue text
$bg = imagecolorallocate($im, 255, 255, 255);
$textcolor = imagecolorallocate($im, 255, 153, 0);
$font="/usr/local/fonts/DickVanDykeHeavy.ttf";
$text="pugsly disk usage /export/home";
$size="30";

list($x,$y)=pc_ImageTTFCenter($im,$text,$font,$size);
ImageTTFText($im,$size,0,$x,$y,$textcolor,$font,$text);

ImageColorTransparent($im,$bg);
header("Content-type: image/png");
imagepng($im);
ImageDestroy($im);

?>
