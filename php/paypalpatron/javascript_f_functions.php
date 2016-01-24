
// -----------------------------------------
// DOM ID FUNCTIONS FOR IMPORTANT ELEMENTS
// -----------------------------------------
// Type-based rows involve some javascript updates (unit-lookup, calorie calculation).
// We need to come up with DOM ID's that let us create row-specific targets.
function f_type_id               (row_id) { return "f_type"               +row_id; }
function f_type_cell_id          (row_id) { return "f_type_cell"          +row_id; }
function f_amount_id             (row_id) { return "f_amount"             +row_id; }
function f_amount_unit_id        (row_id) { return "f_amount_unit"        +row_id; }
function f_calories_per_amount_id(row_id) { return "f_calories_per_amount"+row_id; }  // sync with AJAX code
function f_calories_id           (row_id) { return "f_calories"           +row_id; }
function f_details_id            (row_id) { return "f_details"            +row_id; }
function f_add_id                (row_id) { return "f_add"                +row_id; }
function f_update_id             (row_id) { return "f_update"             +row_id; }
function f_delete_id             (row_id) { return "f_delete"             +row_id; }
function f_button_cell_id        (row_id) { return "f_button_cell"        +row_id; }
// -----------------------------------------


// -----------------------------------------
// DOM FUNCTIONS FOR DYNAMIC CHANGES TO PAGE
// -----------------------------------------

function f_insertRow(new_data, log_id, viaAJAX)
{
    // We need to do this on startup, AND on press of the "add" button..
    // Button-press requires AJAX, but we don't want to do the startup one via AJAX,
    // otherwise we'll be hitting obnoxious AJAX calls while loading, yuck.
    // Best way: put common code in a php file called directly here AND in the AJAX routine.
    // Cool.  BUT... remember to keep the php part "static" - there's no 
    // javascript-php interaction outside of post-load AJAX calls.

    // row_id is used as the basis for ID's for all elements in the row.
    // for new data, we have to use a reference count in a hidden field.
    // for existing data, we use the log id.
    var row_id;
    var extra_text = "";
    if ( new_data )
    {
        // alert(row_id);
        row_id="new"+parseInt(document.getElementById('new_row_id').value);
        document.getElementById('new_row_id').value=parseInt(document.getElementById('new_row_id').value) + 1;

        extra_text = "DISABLED ";
    } else
    {
        row_id=log_id;
    }

    var add_row = document.getElementById('food_log_table').insertRow(-1);
    add_row.id = row_id;

    // WARNING: this is the only way that is IE-compatible.  Stupid M$.
    var cell1 = add_row.insertCell(-1);
    var cell2 = add_row.insertCell(-1);
    var cell3 = add_row.insertCell(-1);
    var cell4 = add_row.insertCell(-1);
    var cell5 = add_row.insertCell(-1);

    cell2.innerHTML="<form><input ID='"+f_amount_id  (row_id)+"' "+extra_text+"type=text size=4 maxlength=20 onkeyup='f_calculateCalories(this)'> <span ID='"+f_amount_unit_id(row_id)+"'></span>";
    cell3.innerHTML="<input       ID='"+f_calories_id(row_id)+"' "+extra_text+"type=text size=4  maxlength=20 onkeyup='f_caloriesChanged(this)'>";
    cell4.innerHTML="<input       ID='"+f_details_id (row_id)+"' "+extra_text+"type=text size=40 maxlength=200 onkeyup='f_detailsChanged(this)'>";

    cell5.id = f_button_cell_id(row_id);
    f_insertButtons( row_id, new_data );

    if ( viaAJAX==1 )
    {
        // Use an AJAX call to build the types dropdown.
        // We need to pass the javascript row info to PHP via the AJAX post.
        cell1.innerHTML="<span id='"+f_type_cell_id(row_id)+"'>Loading...</span>";
        var param_string = "&table=1&user_id="+document.getElementById('user_id').value+"&type_dom_id="+f_type_id(row_id);
        ajaxRequest( 1, 3, param_string, f_type_cell_id(row_id), 0 );

    } else
    {
        // When using AJAX, we use a <span> around the dropdown to give us an update target.
        // We keep the <span> around the dropdown here to keep things consistent when we are navigating.
        var cell1_html = "<span id='"+f_type_cell_id(row_id)+"'>";

        // We have to call different php based on new_data.
        // But we can't actually pass the javascript var to php.
        // The only way is to branch in javascript and call different php.
        // This means we end up calling the query twice.  Oh well, it's the only way.
        if ( new_data )
        {
            cell1_html = cell1_html+"<? types_dropdown_as_innerHTML( $mysqli, 1, f_types_query( $user_id ), "f_temp_dropdown", 1 ); ?>";
        } else
        {
            cell1_html = cell1_html+"<? types_dropdown_as_innerHTML( $mysqli, 1, f_types_query( $user_id ), "f_temp_dropdown", 0 ); ?>";
        }
        cell1_html = cell1_html + "</span>";
        cell1.innerHTML=cell1_html;

        // Now reset the ID.
        document.getElementById('f_temp_dropdown').id = f_type_id(row_id);
    }

    return add_row;
}

function f_changeType(selector, new_data, update_data)
{
    var row_id  = selector.parentNode.parentNode.parentNode.id;

    if (selector.value==-1)
    {
        // "other"

        // if there is data entered, but no details, move the entered data to details...
        // nah, we've already lost the type at this time...
        // to do this right will require more work (save old type somehow).
        // if ( document.getElementById(f_details_id(row_id)).value=="" )

        // OK, this stuff is not needed, blank and disable it.
        document.getElementById(f_amount_id     (row_id)).value="";
        document.getElementById(f_amount_id     (row_id)).disabled = true;

        // However, note that we need to make sure all fields are there, so
        // that when we update the row, we can find the fields we expect.
        // THIS IS SOME FUCKED UP CODE.  But it works!
        var cal_per_amt_id = f_calories_per_amount_id(row_id);
        document.getElementById(f_amount_unit_id(row_id)).innerHTML="<input ID="+cal_per_amt_id+" type=hidden value=0>";

    } else
    {
        // alert(row_id);

        document.getElementById(f_amount_id     (row_id)).disabled = false;
        document.getElementById(f_amount_unit_id(row_id)).innerHTML="...";

        // We changed the type, we better clear the amount or the user may be confused by mismatched numbers.
        document.getElementById(f_amount_id     (row_id)).value = "";

        // Call the AJAX function to update the amount units.
        // We need to send it the row id so it can use a row-based DOM ID to hide the calories_per_unit value.
        var param_string = "&table=1&type_id="+selector.value+"&row_id="+row_id;
        ajaxRequest( 1, 1, param_string, f_amount_unit_id(row_id), 0 );
    }

    // Common cells
    document.getElementById(f_calories_id(row_id)).disabled=false;
    document.getElementById(f_details_id (row_id)).disabled=false;

    // alert( new_data );
    if ( new_data )
    {
        document.getElementById(f_add_id (row_id)).disabled=false;
        var status_text = "Adding row ";
        status_text += row_id;
        status_text += "...";
        document.getElementById('status').innerHTML=status_text;

    } else if ( update_data )
    {
        f_changedRow(row_id);
    }

    // alert(document.getElementById(f_add_id     (row_id)).value);

}

function f_fillRow( log_id, type_id, amount, calories, details )
{
    var add_row = f_insertRow( 0, log_id, 0 );

    // set type
    var selector = document.getElementById( f_type_id( log_id ) );
    for (n=0;n<selector.length;n=n+1) 
    {
        if (selector.options[n].value == type_id)
        {
            selector.selectedIndex = n;
            break;
        }
    }   

    // force type-based row update.
    // this also sets amount_units and calories_per_amount.
    f_changeType( selector, 0, false );

    // set amount, calories, details
    if ( type_id != -1 )
    {
        document.getElementById( f_amount_id( log_id ) ).value = amount;
    }
    document.getElementById( f_calories_id  ( log_id ) ).value = calories;
    document.getElementById( f_details_id   ( log_id ) ).value = details;

    // alert(amount+calories+details);
}

function f_calculateCalories(amount_text)
{
    var amount = amount_text.value;
    var row_id = amount_text.parentNode.parentNode.parentNode.id;

    var calories_per_amount;
    try
    {
        calories_per_amount = document.getElementById(f_calories_per_amount_id(row_id)).value;

    } catch(e)
    {
        // alert(e);
        calories_per_amount = 0;
    }
    document.getElementById(f_calories_id(row_id)).value = calories_per_amount * amount;

    f_changedRow(row_id);
}

function f_caloriesChanged(calories_text)
{
    var row_id = calories_text.parentNode.parentNode.id;
    f_changedRow(row_id);
}

function f_detailsChanged(details_text)
{
    var row_id = details_text.parentNode.parentNode.id;
    f_changedRow(row_id);
}

function f_changedRow(row_id)
{
    f_disableUpdate(row_id,false);
}

function f_disableUpdate( row_id, disable )
{
    document.getElementById(f_update_id(row_id)).disabled = disable;
}

function f_insertButtons( row_id, new_data )
{
    var buttons_html;
    if ( new_data )
    {
        buttons_html="<input      ID='"+f_add_id     (row_id)+"' DISABLED type=button onclick=\"f_addToLog(this)\" value=\"add\">";
    } else
    {
        buttons_html=             "<input ID='"+f_update_id(row_id)+"' DISABLED type=button onclick=\"f_updateLog(this)\" value=\"update\">";
        buttons_html=buttons_html+"<input ID='"+row_id+"' type=button onclick=\"f_deleteFromLog(this)\" value=\"delete\">";
    }
    buttons_html = buttons_html + "</form>";

    // alert(row_id);

    document.getElementById(f_button_cell_id(row_id)).innerHTML = buttons_html;
}

function f_addToLog( button )
{
    // alert(button.id);
    var row_id      = button.parentNode.parentNode.id;

    // gather up the data
    var daily_entries_id= document.getElementById('daily_entries_id').value;
    var type            = document.getElementById(f_type_id      (row_id)).value;
    var amount          = document.getElementById(f_amount_id    (row_id)).value;
    var calories        = document.getElementById(f_calories_id  (row_id)).value;
    var details         = document.getElementById(f_details_id   (row_id)).value;

    // validate
    if ( calories=='' ) calories = 0;
    if ( amount=='' ) amount = 0;
    details = escapeString( details );

    // rewrite the button space to say "saving..."
    document.getElementById(f_button_cell_id(row_id)).innerHTML = "saving...";
    
    // save the row id, we'll need to update it soon...
    document.getElementById('adding_row_id').value = row_id;

    // send add query via ajax
    // NOTE that if we don't have a daily_entries_id yet, we need to send the date, too!
    var param_string="&table=1&daily_entries_id="+daily_entries_id+"&type="+type+"&amount="+amount+"&calories="+calories+"&details="+details;
    if ( daily_entries_id == -1 )
    {
        var date = document.getElementById('date_id').value;
        param_string=param_string+"&date="+document.getElementById('date_id').value+"&user_id="+document.getElementById('user_id').value;;
    }
    // alert(param_string);
    ajaxRequest( 1, 4, param_string, 'added_row_id', 1 );

    // now we can insert a new row, ready for more input...
    f_insertRow( 1, 0, 1 );
}

function f_updateLog( button )
{
    var row_id = button.parentNode.parentNode.id;

    // gather up the data
    var daily_entries_id= document.getElementById('daily_entries_id').value;
    var type            = document.getElementById(f_type_id      (row_id)).value;
    var amount          = document.getElementById(f_amount_id    (row_id)).value;
    var calories        = document.getElementById(f_calories_id  (row_id)).value;
    var details         = document.getElementById(f_details_id   (row_id)).value;

    // validate
    if ( calories=='' ) calories = 0;
    if ( amount=='' ) amount = 0;
    details = escapeString( details );

    // rewrite the button space to say "saving..."
    document.getElementById(f_button_cell_id(row_id)).innerHTML = "saving...";
    
    // save the row id, we'll need to update the "saving" text...
    document.getElementById('updated_row_id').value = row_id;

    // send update query via ajax
    var param_string="&table=1&id="+row_id+"&type="+type+"&amount="+amount+"&calories="+calories+"&details="+details;
    // alert(param_string);

    ajaxRequest( 1, 5, param_string, 'status', 0 );


}

function f_deleteFromLog( button )
{
    var row_id = button.id;
    // alert(row_id);

    ajaxRequest( 1, 2, "&table=1&row_id_to_delete="+row_id, 'status', 0 );

    // Careful here, if you wrap the button in any tags, this needs to change.
    var i = button.parentNode.parentNode.rowIndex;
    // alert(i);
    document.getElementById('food_log_table').deleteRow(i);
}


