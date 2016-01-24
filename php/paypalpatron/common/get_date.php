<?php

// In this module, we get the date from the URL, or use today if not provided.
// We set the following variables, used to get the daily entries and build the calendar:
//
//      $today          today, according to the system clock (array)
//      $targetdate     the date for which we're loading and displaying data (array)
//      $date           targetdate, in "YYYY-MM-DD" format (for mysql)
//      $month          month of targetdate
//      $year           year  of targetdate
//      $day            day   of targetdate
//
// A note about PHP scope: these vars are assigned directly within this module; include the module and you get the vars!

// MDM Get the day, month and year from the url.
// Validate them.  If they are missing or invalid, use today.
$have_date = false;
if ( isset($_GET['day']) && isset($_GET['month']) && isset($_GET['year']) )
{
    $day   = (int)$_GET['day'  ];
    $month = (int)$_GET['month'];
    $year  = (int)$_GET['year' ];

    // Validate.
    if ( 0 < $month && $month < 12 && 1000 < $year && $year < 9999 )
    {
        $daysInMonth = date("t",mktime(0,0,0,$month,1,$year));    
        if ( $day > 0 && $day <= $daysInMonth )
        {
            $have_date = true;
        }
    }
}
if ( !$have_date )
{
    $year  = date('Y');
    $month = date('n');
    $day   = date('j');
}

$date = sprintf( "%d-%'02d-%'02d", $year, $month, $day );
$today      = array('day'=>date('j'), 'month'=>date('n'), 'year'=>date('Y'));
$targetdate = array('day'=>$day     , 'month'=>$month   , 'year'=>$year    );

// DEBUG
// echo $date." ".$year." ".$month." ".$day;

?>

