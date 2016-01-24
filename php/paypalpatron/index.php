<?php

// ================
//  DAILY LOG VIEW
// ================
//
// This page presents the exercise and food logs for a specific user on a specific date.
// 
// Given:
//
//      user
//      date
//      daily_entries id
//
// Components:
//
//      weight entry
//      exercise log
//      food log
//      daily report


// ======= to do ====================
// MDM Hardcode the user for the moment.
// Eventually this will be required in URL parameters or cookies,
// once we have registration set up.
$user_id = "1";
// ======= to do ====================


// This actually connects to the database and gives us a live [mysqli] object to use.
require_once './common/mysql_connect.php';

// Functions to build every one of the queries we'll need on this page.
require_once './common/queries.php';

// Common functions.  Called here, and also in separate AJAX requests, etc.
require_once './common/shared_functions.php';

// This sets up all the date variables we need for "targetdate" and "today".
require_once './common/get_date.php';

// Check the database - do we have an entry on this date?
$daily_entries_id = get_daily_entries_id( $mysqli, $date, $user_id );

// This sets up the weight variables used for weight change estimating.
require_once './common/get_weight.php';

// TO DO
// MDM Set up all HTML elements that get repeated more than once as php strings.

?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<link rel="stylesheet" href="./common/style.css" type="text/css" media="all" />

<title>My Today Diet</title>

<? 
/* MDM If you put this above the head stuff you'll get screwy CSS behavior... */
require_once './common/todaydiet_calendar.php'; 
?>

<script language="javascript">

// -----------------------------------------
// AJAX REQUESTS
// -----------------------------------------
// All requests go through one function, to avoid duplicate xmlHttp code.
// Post a request to [common/ajax_query.php], and use the result to update the page.
// Params:
//  table               the html table we're working with (exercise = 0, food = 1,...)
//  ajax_query_index    the index of the AJAX query used to process the request server-side
//  param_string        all the parameters passed to the server, in URL format: "&param1=blah&param2=blah"
//                          (may be blank)
//  results_dom_id      the ID of the DOM target; its innerHTML or value will be set to the query results
//  results_type        0 = innerHTML, 1 = value
// -----------------------------------------
function ajaxRequest(table, ajax_query_index, param_string, results_dom_id, results_type )
{
    // Relative is always better when we can get away with it.
    // var ajax_php = "http://thedigitalmachine.com/todaydiet/common/ajax_query.php";
    var ajax_php = "./common/ajax_query.php";

    // Absolutely fubar and absolutely correct,
    // this is how we do AJAX.  Don't change this.
    var xmlHttp;
    try
    {
        // Firefox, Opera 8.0+, Safari
        xmlHttp=new XMLHttpRequest();
    }catch(e){
        // Internet Explorer
        try
        {
            xmlHttp=new ActiveXObject("Msxml2.XMLHTTP");
        }catch(e){
            try
            {
                xmlHttp=new ActiveXObject("Microsoft.XMLHTTP");
            }catch(e){
                alert("Your browser does not support AJAX!");
                return false;
            }
        }
    }
    xmlHttp.onreadystatechange=function()
    {
        if(xmlHttp.readyState==4)
        {
            // DEBUG
            // Be careful, this is great but we'll end up with multiple elements inserted.
            // status = document.getElementById('status').innerHTML;
            // document.getElementById('status').innerHTML = status + "<br />" + xmlHttp.responseText;

            if ( results_type == 0 )
                document.getElementById(results_dom_id).innerHTML = xmlHttp.responseText;
            else
                document.getElementById(results_dom_id).value = xmlHttp.responseText;

            ajaxPostProcessing( table, ajax_query_index );
        }
    }

    // alert( ajax_php+"?query_index="+ajax_query_index+param_string );
    xmlHttp.open("GET", ajax_php+"?query_index="+ajax_query_index+param_string,true);
    xmlHttp.send(null);
}

function ajaxPostProcessing( table, ajax_query_index )
{
    if ( ajax_query_index == 4 )
    {
        var adding_row_id = document.getElementById('adding_row_id').value;
        var added_row_id  = document.getElementById('added_row_id' ).value;

        // if daily_entries_id was -1, we need to parse a new value out of the result.
        if ( document.getElementById('daily_entries_id').value == -1 )
        {
            // added_row_id has a prefix of the new daily_entries_id value.
            // parse it out.
            var new_daily_entries_id;

            var splitstring = added_row_id.split("|",1);
            document.getElementById('daily_entries_id').value = splitstring[0];
            added_row_id = splitstring[1];
        }

        // update the row to behave like an existing entry
        //  1) update row id's
        //  2) convert add button to update/delete

        // now swap old id for new
        if ( table == 0 )
        {
            document.getElementById(e_type_id               (adding_row_id)).id = e_type_id               (added_row_id);
            document.getElementById(e_type_cell_id          (adding_row_id)).id = e_type_cell_id          (added_row_id);
            document.getElementById(e_amount_id             (adding_row_id)).id = e_amount_id             (added_row_id);
            document.getElementById(e_amount_unit_id        (adding_row_id)).id = e_amount_unit_id        (added_row_id);
            document.getElementById(e_calories_per_amount_id(adding_row_id)).id = e_calories_per_amount_id(added_row_id);
            document.getElementById(e_calories_id           (adding_row_id)).id = e_calories_id           (added_row_id);
            document.getElementById(e_details_id            (adding_row_id)).id = e_details_id            (added_row_id);
            document.getElementById(e_button_cell_id        (adding_row_id)).id = e_button_cell_id        (added_row_id);
            e_insertButtons( added_row_id, 0 );

        } else
        {
            document.getElementById(f_type_id               (adding_row_id)).id = f_type_id               (added_row_id);
            document.getElementById(f_type_cell_id          (adding_row_id)).id = f_type_cell_id          (added_row_id);
            document.getElementById(f_amount_id             (adding_row_id)).id = f_amount_id             (added_row_id);
            document.getElementById(f_amount_unit_id        (adding_row_id)).id = f_amount_unit_id        (added_row_id);
            document.getElementById(f_calories_per_amount_id(adding_row_id)).id = f_calories_per_amount_id(added_row_id);
            document.getElementById(f_calories_id           (adding_row_id)).id = f_calories_id           (added_row_id);
            document.getElementById(f_details_id            (adding_row_id)).id = f_details_id            (added_row_id);
            document.getElementById(f_button_cell_id        (adding_row_id)).id = f_button_cell_id        (added_row_id);
            f_insertButtons( added_row_id, 0 );
        }

    } else if ( ajax_query_index == 5 )
    {
        var updated_row_id = document.getElementById('updated_row_id').value;
        // alert(updated_row_id);

        if ( table == 0 )
        {
            e_insertButtons( updated_row_id, 0 );
            e_disableUpdate(updated_row_id,true);
        } else
        {
            f_insertButtons( updated_row_id, 0 );
            f_disableUpdate(updated_row_id,true);
        }   
    }
}


<? 
require_once './javascript_e_functions.php';
require_once './javascript_f_functions.php';

require_once './javascript_summary_functions.php';
?>

</script>

<STYLE TYPE="text/css">
/* ---------------------------------------- */
/* REQUIRED CSS STYLES, i.e. non-negotiable */
/* ---------------------------------------- */
/* Put style-oriented CSS in [common/style.css]. */

/* ID selectors (per-element) */
#amount_header  { width:16ex }
#button_header  { width:10ex }

</STYLE>

</head>

<body>

<center>
<h2><a href="http://thedigitalmachine.com/mytodaydiet">My Today Diet</a></h2>


<? /* ---------- */ ?>
<? /*  CALENDAR  */ ?>
<? /* ---------- */ ?>

<?php
// Render the row of 3 calendars now...
echo "<div id='quickCalender'><table><tr><td valign=top>";
$cal1 = &new QCalendar($month, $year, $today, $targetdate, $links, $css, 1);
$cal1->render();
echo "</td><td valign=top>";
$cal2 = &new QCalendar($month, $year, $today, $targetdate, $links, $css, 2);
$cal2->render();
echo "</td><td valign=top>";
$cal3 = &new QCalendar($month, $year, $today, $targetdate, $links, $css, 3);
$cal3->render();
echo "</td></tr></table></div>";
?>


<? /* -------- */ ?>
<? /*  WEIGHT  */ ?>
<? /* -------- */ ?>

<h2>Weight</h2>
<form><input type=text ID='weight' size=6 maxlength=20 onkeyup='onWeight()' value='<? echo $weight_est; ?>'> 
<select ID='weight_adjust' onchange='onWeight()'>
<option <? if ( $weight_adjust == -2 ) echo "SELECTED"; ?> value=-2>empty</option>
<option <? if ( $weight_adjust == -1 ) echo "SELECTED"; ?> value=-1>almost empty</option>
<option <? if ( $weight_adjust ==  0 ) echo "SELECTED"; ?> value=0>average</option>
<option <? if ( $weight_adjust ==  1 ) echo "SELECTED"; ?> value=1>almost full</option>
<option <? if ( $weight_adjust ==  2 ) echo "SELECTED"; ?> value=2>full</option>
</select>
</form>


<? /* ---------- */ ?>
<? /*  EXERCISE  */ ?>
<? /* ---------- */ ?>

<h2>Exercise</h2>

<table cellpadding=8 border=1 id="exercise_log_table">
  <THEAD>
    <TR>
      <TH>Type</TH>
      <TH ID="amount_header">Amount</TH>
      <TH>Calories</TH>
      <TH>Details</TH>
      <TH><form><input type=submit name=exercise_types value="types"></form></TH>
    </TR>
  </THEAD>
  <TBODY>

<? 

// MDM Exercise log
if ( $result = $mysqli->query( el_query($daily_entries_id) ))
{
    echo "<script type='text/javascript'>";

    /* fetch object array */
    while ($row = $result->fetch_row()) 
    {
        $clean_details = str_replace("'","\'",$row[4]);

        // log_id, type_id, amount, calories, 'details'
        echo "e_fillRow( ".$row[0].",".$row[1].",".$row[2].",".$row[3].",'".$clean_details."');";
    }

    echo "</script>";

    /* free result set */
    $result->close();
} else
{
    die ("query failed: ".mysql_error());
}

?>
 

  </TBODY>
</TABLE>


<? /* -------- */ ?>
<? /*   FOOD   */ ?>
<? /* -------- */ ?>

<h2>Food</h2>

<table cellpadding=8 border=1 id="food_log_table">
  <THEAD>
    <TR>
      <TH>Type</TH>
      <TH ID="amount_header">Amount</TH>
      <TH>Calories</TH>
      <TH>Details</TH>
      <TH><form><input type=submit name=food_types value="types"></form></TH>
    </TR>
  </THEAD>
  <TBODY>

<? 

// DEBUG
// echo $query[1];

// MDM Food log
if ( $result = $mysqli->query( fl_query($daily_entries_id) ))
{
    echo "<script type='text/javascript'>";

    /* fetch object array */
    while ($row = $result->fetch_row()) 
    {
        $clean_details = str_replace("'","\'",$row[4]);

        // log_id, type_id, amount, calories, 'details'
        echo "f_fillRow( ".$row[0].",".$row[1].",".$row[2].",".$row[3].",'".$clean_details."');";
        // echo "f_fillRow(1,-1,0,12,'test');";
        // echo "f_insertRow(0,12,0);";
    }

    echo "</script>";

    /* free result set */
    $result->close();
} else
{
    die ("query failed: ".mysql_error());
}

?>
 

  </TBODY>
</TABLE>


<? /* --------- */ ?>
<? /*  SUMMARY  */ ?>
<? /* --------- */ ?>

<h2>Summary</h2>

<span id="summary"></span>

<? /* -------- */ ?>
<? /*  STATUS  */ ?>
<? /* -------- */ ?>

<h2>Status</h2>

<span id="status"></span>

<? /* Hidden DOM input fields for javascript state storage */ ?>
<? /* These may hold values provided from the server, or   */ ?>
<? /* values that change depending on javascript needs.    */ ?>
<input type=hidden id='user_id' value='<? echo $user_id; ?>'>
<input type=hidden id='date_id'  value='<? echo $date; ?>'>
<input type=hidden id='daily_entries_id' value='<? echo $daily_entries_id; ?>'>
<input type=hidden id='rate'  value='<? echo $rate; ?>'>
<input type=hidden id='weight_est'  value='<? echo $weight_est; ?>'>
<input type=hidden id='new_row_id' value='1'>
<input type=hidden id='adding_row_id' value='-1'>
<input type=hidden id='added_row_id'  value='-1'>
<input type=hidden id='updated_row_id'  value='-1'>

<? /* Initial javascript page actions */ ?>
<script type="text/javascript">e_insertRow(1,0,0);</script>
<script type="text/javascript">f_insertRow(1,0,0);</script>
<script type="text/javascript">recalcSummary();</script>

<?
/* close connection */
$mysqli->close();
?>

</center>
</BODY>
</HTML>


