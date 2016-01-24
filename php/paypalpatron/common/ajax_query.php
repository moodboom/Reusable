<?php


// -----------------------
//   AJAX QUERY HANDLER
// -----------------------

// Here, we simply respond to AJAX requests and return.
// AJAX requests are simply asynch html requests that can occur
// without reloading an entire web page.  Here, we just respond
// with the appropriate text.  Responses are typically
// handled with DOM to update the web page dynamically.

// First, get the parameters posted to us.
// We have to have at least the index, or bail out.
if ( !isset($_GET['query_index']) )
{
    echo "Bad AJAX request number...";
    exit;
}
    
$query_index = (int)$_GET['query_index'];

// DEBUG
// echo $query_index;
// exit;

switch ( $query_index )
{

// ===================================================================
// Return the type amount_unit for the given type id.
// ===================================================================
case 1:

    // DATABASE QUERY
    require_once './mysql_connect.php';

    if ( !isset($_GET['table']) || !isset($_GET['type_id']) || !isset($_GET['row_id']) )
    {
        echo "Invalid parameters for AJAX query 1...";
        exit;
    }

    $table   = (int)$_GET['table'];
    $type_id = (int)$_GET['type_id'];
    $row_id  = $_GET['row_id'];         // not (int), could be "new##"

    // DEBUG
    // echo "row id: ".$row_id." ";
    // exit;

    if ( $table == "1" )
    {
        $sqltable = "food_type";
        $prefix = "f";
    }  else 
    {
        $sqltable = "exercise_type";
        $prefix = "e";
    }
        
    $query_type_unit = "SELECT amount_unit, calories_per_amount FROM ".$sqltable." WHERE id=".$type_id;

    // DEBUG
    // echo $query_type_unit;
    // exit;

    // MDM Lost a chunk of hair to this, why does a valid $result fail here?
    // if ( $result = $mysqli->query( $query_type_unit ) )

    $result = $mysqli->query( $query_type_unit );
    $error = mysql_error();
    if ( $error )
    {
        echo $error;
        exit;
    }

    if ( $row = $result->fetch_row() )
    {
        // Display the unit, and drop in the cal/amt as a hidden field.
        echo $row[0]."<input id='".$prefix."_calories_per_amount".$row_id."' type=hidden value=\"".$row[1]."\">";

    } else
    {
        echo "badid";
    }

    $result->close();
    $mysqli->close();
    exit;


// ===================================================================
// Delete the exercise log row with the given type id.
// ===================================================================
case 2:

    // DATABASE QUERY
    require_once './mysql_connect.php';

    if ( !isset($_GET['table']) || !isset($_GET['row_id_to_delete']) )
    {
        echo "Invalid parameters for AJAX query 2...";
        $mysqli->close();
        exit;
    }

    $table  = (int)$_GET['table'];
    $row_id = (int)$_GET['row_id_to_delete'];

    if ( $table == "0" )
        $sqltable = "exercise_log";
    else
        $sqltable = "food_log";
        
    $query_type_unit = "DELETE FROM ".$sqltable." WHERE id=".$row_id;

    if ( $mysqli->query( $query_type_unit ) == TRUE )
    {
        echo "Successfully deleted row ".$row_id.".";
    } else
    {
        echo mysql_error();
    }

    $mysqli->close();
    exit;


// ===================================================================
// Call the shared function that echos the type dropdown.
// We do this when inserting a new row.
// ===================================================================
case 3:

    // DATABASE QUERY
    require_once './mysql_connect.php';

    if ( !isset($_GET['table'])  || !isset($_GET['user_id']) || !isset($_GET['type_dom_id']) )
    {
        echo "Invalid parameters for AJAX query 3...";
        $mysqli->close();
        exit;
    }

    $table       = (int)$_GET['table'];
    $user_id     = (int)$_GET['user_id'];
    $type_dom_id = $_GET['type_dom_id'];

    // Make sure we set the $user_id string before including this.
    require_once './queries.php';

    if ( $table == "0" )
        $query_types = e_types_query( $user_id );
    else
        $query_types = f_types_query( $user_id );
        
    // We call a shared function to accomplish this one.
    require_once './shared_functions.php';
    types_dropdown_as_innerHTML( $mysqli, $table, $query_types, $type_dom_id, 1 );

    $mysqli->close();
    exit;


// ===================================================================
// Add the log row for which we just received all the data
// to the database...
// ===================================================================
case 4:

    // DATABASE QUERY
    require_once './mysql_connect.php';

    if ( 
            !isset($_GET['table'])   
        ||  !isset($_GET['daily_entries_id'])   
        ||  !isset($_GET['type'])   
        ||  !isset($_GET['amount'])   
        ||  !isset($_GET['calories'])   
        ||  !isset($_GET['details'])   
    ) {
        echo "Invalid parameters for AJAX query 4...";
        $mysqli->close();
        exit;
    }

    $table              = (int)$_GET['table'];
    $daily_entries_id   = (int)$_GET['daily_entries_id'];
    $type               = (int)$_GET['type'];
    $amount             = $_GET['amount'];
    $calories           = $_GET['calories'];
    $details            = $_GET['details'];

    // The first thing we need to do is check daily_entries_id.
    // If it's -1, we don't have an item in the daily_entries table yet, and we'll need to add one!
    if ( $daily_entries_id == -1 )
    {
        // The caller checks if the above is true.
        // If so, the user and date are sent in addition to the other parameters.
        if ( 
                !isset($_GET['user_id'])   
            ||  !isset($_GET['date'])   
        ) {
            echo "Invalid additional parameters for AJAX query 4...";
            $mysqli->close();
            exit;
        }

        $user_id = (int)$_GET['user_id'];
        $date    = $_GET['date'];
    
        $query_add_daily_entry = "INSERT INTO daily_entries (user_id, date) VALUES(".$user_id.",'".$date."');";

        // DEBUG
        // echo $query_add_daily_entry;

        if ( $mysqli->query( $query_add_daily_entry ) == FALSE )
        {
            echo "SQL error: ".mysql_error();
        }

        // Now get the new daily_entries_id.
        $daily_entries_id = $mysqli->insert_id;

        // Echo it, we'll have to parse it out on the client.
        echo $daily_entries_id."|";
    }

    // Validation/correction.
    if ( $type == -1 ) $type = 'NULL';

    // DEBUG
    // echo $details;
    // exit;

    if ( $table == "0" )
    {
        $sqltable = "exercise_log";

    } else
    {
        $sqltable = "food_log";
    }
    $query_add = "INSERT INTO ".$sqltable." (daily_entries_id, type_id, amount, calories, details) VALUES(".$daily_entries_id.",".$type.",".$amount.",".$calories.",'".$details."');";

    // DEBUG
    // echo $query_add;

    if ( $mysqli->query( $query_add ) == FALSE )
    {
        echo "SQL error: ".mysql_error();
    }

    // Return the new log id, essential for subsequent updates.
    // See http://in2.php.net/manual/en/function.mysqli-insert-id.php
    echo $mysqli->insert_id;

    $mysqli->close();
    exit;

// ===================================================================
// Update the log with the updated information we just received...
// ===================================================================
case 5:

    // DATABASE QUERY
    require_once './mysql_connect.php';

    if ( 
            !isset($_GET['table'])   
        ||  !isset($_GET['id'])   
        ||  !isset($_GET['type'])   
        ||  !isset($_GET['amount'])   
        ||  !isset($_GET['calories'])   
        ||  !isset($_GET['details'])   
    ) {
        echo "Invalid parameters for AJAX query 5...";
        $mysqli->close();
        exit;
    }

    $table      = (int)$_GET['table'];
    $id         = (int)$_GET['id'];
    $type       = (int)$_GET['type'];
    $amount     = $_GET['amount'];
    $calories   = $_GET['calories'];
    $details    = $_GET['details'];

    // Validation/correction.
    if ( $type == -1 ) $type = 'NULL';

    if ( $table == "0" )
    {
        $sqltable = "exercise_log";

    } else
    {
        $sqltable = "food_log";
    }

    $query_update = "UPDATE ".$sqltable." set type_id=".$type.",amount=".$amount.",calories=".$calories.",details='".$details."' WHERE id=".$id;

    // DEBUG
    // echo $query_update;
    // exit;

    if ( $mysqli->query( $query_update ) == FALSE )
    {
        echo "SQL error: ".mysql_error();
    }

    echo "Updated log item ".$id."...";

    $mysqli->close();
    exit;

}

?>
