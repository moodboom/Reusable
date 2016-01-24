<?php

// MDM Our common queries, provided as functions
// so they can be created when we have the appropriate data.

// Get user weight data.
function weight_query( $user_id )
{
    return "SELECT de.date, de.weight, de.empty 
            FROM users u, daily_entries de 
            where u.id='".$user_id."' and u.id = de.user_id";
}

// Get the daily entries id, if any, for the given user+date...
function de_id_query( $user_id, $date )
{
    return "SELECT id FROM daily_entries WHERE user_id = ".$user_id." AND date = '".$date."'";
}

// Get the user daily exercise log for a given daily_entries_id.
function el_query( $daily_entries_id )
{
    return "SELECT el.id, IFNULL( el.type_id, -1 ), el.amount, el.calories, el.details
            FROM exercise_log el 
            WHERE el.daily_entries_id='".$daily_entries_id."'";
}

// Get the user daily food log for a given daily_entries_id.
function fl_query( $daily_entries_id )
{
    return "SELECT fl.id, IFNULL( fl.type_id, -1 ), fl.amount, fl.calories, fl.details
            FROM food_log fl 
            WHERE fl.daily_entries_id='".$daily_entries_id."'";
}

// Get exercise types.
function e_types_query( $user_id )
{
    return "SELECT et.id, et.short_name FROM users u, exercise_type et WHERE u.id='".$user_id."'";
}

// Get food types.
function f_types_query( $user_id )
{
    return "SELECT ft.id, ft.short_name FROM users u, food_type ft WHERE u.id='".$user_id."'";
}

?>
