<?php

// In this module, we estimate the metabolic rate.
// We also estimate weight, if it is not already available.
// We use historical data from daily_entries, exercise_log and food_log.
// We set the following variables:
//
//      $rate           estimated metabolic rate
//      $weight_est     estimated weight
//      $weight_adjust  which adjustment to use (default: 0 average)
//
// A note about PHP scope: these vars are assigned directly within this module; include the module and you get the vars!

// TO DO
$rate = 2000.0;
$weight_est = 180.0;
$weight_adjust = 0;

// DEBUG
// echo $weight_est." ".$rate;

?>

