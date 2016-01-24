<?php

function get_daily_entries_id( $mysqli, $date, $user_id )
{
    $de_id = -1;

    // DEBUG
    // echo de_id_query( $user_id, $date );

    // Run query.  We should only get 0..1 results.  If no results, return -1.
    if ( $result = $mysqli->query( de_id_query( $user_id, $date ) ) )
    {
        // fetch object array
        while ($row = $result->fetch_row()) 
        {
            if ( $de_id != -1 )
            {
                // We received more than one row, this is an ERROR.
                // There is a unique key that should prevent this.
                echo "ERROR, more than one daily_entries_id with same user/date...";
            }
            $de_id = $row[0];
        }

        // free result set
        $result->close();
    } else
    {
        echo "de_id query failed: ".mysql_error()."<br />";
    }

    // DEBUG
    // echo "daily_entries_id = ".$de_id;

    return $de_id;
}

function types_dropdown_as_innerHTML( $mysqli, $table, $query, $type_dom_id, $new_data )
{
    // Here, we set up a string for exercise types that we can stuff as innerHTML
    // to create a dropdown select.

    // The only weird part is that all double quotes must be escaped with an actual backslash.
    // No, really, this is weird.  Example: echo "onchange=\\\"doThis()\\\""
    // Err... that's only for direct php calls; we should AVOID the double-escape when called via AJAX.
    // For now, we'll avoid the issue by using a single tick.

    // Also, we use this for existing as well as "to be added" dropdowns.
    // BUT... don't think we can do any selection here when called directly.  
    // This is php code, called before the page is served and before any javascript is active.
    // We are using javascript to insert rows, so it can be done both on start
    // and dynamically.  The javascript will not be able to "tell" the php
    // code what to select.  The php code will always build the dropdown in the same
    // way, then the javascript will do a selection.
    // Note that when using AJAX, we CAN pass a param from javascript to php via post vars...

    $prefix = "e";
    if ( $table == "1" )
        $prefix = "f";

    echo "<select NAME=short_name ID='".$type_dom_id."' onchange='".$prefix."_changeType(this,".$new_data.",true)'>";

    // If this is for the new "to be added" row, 
    // set up a dummy option so the user has to select something else.
    // if ( $selected_id == -1 )
    // {
        echo "<OPTION SELECTED DISABLED>Select...</OPTION>";
    // }

    // Perform the query to get exercise types.
    // Create an OPTIONS tag for each.
    if ( $result = $mysqli->query($query) )
    {
        // fetch object array
        while ($row = $result->fetch_row()) 
        {
            echo "<OPTION VALUE=".$row[0].">".$row[1]."</OPTION>";

            // MDM The old, foolhardy "I can do the selection" method.
            // echo "<OPTION ";
            // if ( $selected_id == $row[0] )
            // {
            //     echo "SELECTED ";
            // }
            // echo "VALUE=".$row[0].">".$row[1]."</OPTION>";
        }

        // free result set
        $result->close();
    } else
    {
        echo "<OPTION>query failed: ".mysql_error()."</OPTION>";
    }

    echo "<OPTION VALUE=-1>other...</OPTION>";
    echo "</SELECT>";

}

?>