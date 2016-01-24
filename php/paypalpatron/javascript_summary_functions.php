
// -----------------------------------------
// DOM ID FUNCTIONS FOR IMPORTANT ELEMENTS
// -----------------------------------------
// -----------------------------------------


// -----------------------------------------
// DOM FUNCTIONS FOR DYNAMIC CHANGES TO PAGE
// -----------------------------------------

function onWeight()
{
    recalcSummary();
    queueSave();
}

function recalcSummary()
{
    // Completely update the summary section, we have new weight data.

    // Get current weight.
    // Get current adjustment.
    // Adjust weight.
    // Get metabolic rate.

    // Get exercise calories.
    // If blank, use default.
    // If out of range, warn and use default.

    // Get food calories.

    // Calculate weight change.
    // calculate color, BMI, % body fat

    // Display...
    //   weight change - h2 - green/red shades
    //   metabolic rate
    //   BMI
    //   % body fat

    var sum_html = "<table class='summary_table' border=1>";

    sum_html = sum_html + "<tr><td><h2>Estimated weight change</h2></td><td><h2>";
    sum_html = sum_html + document.getElementById('weight').value;
    sum_html = sum_html + "</h2></td></tr>";

    sum_html = sum_html + "<tr><td>Metabolic rate</td><td>";
    sum_html = sum_html + document.getElementById('rate').value;
    sum_html = sum_html + "</td></tr>";

    sum_html = sum_html + "<tr><td>BMI</td><td>";
    sum_html = sum_html + document.getElementById('weight').value;
    sum_html = sum_html + "</td></tr>";

    sum_html = sum_html + "<tr><td>% body fat</td><td>";
    sum_html = sum_html + document.getElementById('weight').value;
    sum_html = sum_html + "</td></tr>";

    sum_html = sum_html + "</table>";

    document.getElementById('summary').innerHTML = sum_html;
}

var timer_id = -1
function queueSave()
{
    if ( timer_id != -1 )
    {
        clearTimeout( timer_id );
    }
    timer_id = setTimeout( "saveWeight()", 7000 );
}

function saveWeight()
{
    alert( "test" );
}