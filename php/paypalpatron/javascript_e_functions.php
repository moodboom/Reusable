
// -----------------------------------------
// DOM ID FUNCTIONS FOR IMPORTANT ELEMENTS
// -----------------------------------------
// Type-based rows involve some javascript updates (unit-lookup, calorie calculation).
// We need to come up with DOM ID's that let us create row-specific targets.
function e_type_id               (row_id) { return "e_type"               +row_id; }
function e_type_cell_id          (row_id) { return "e_type_cell"          +row_id; }
function e_amount_id             (row_id) { return "e_amount"             +row_id; }
function e_amount_unit_id        (row_id) { return "e_amount_unit"        +row_id; }
function e_calories_per_amount_id(row_id) { return "e_calories_per_amount"+row_id; }  // sync with AJAX code
function e_calories_id           (row_id) { return "e_calories"           +row_id; }
function e_details_id            (row_id) { return "e_details"            +row_id; }
function e_add_id                (row_id) { return "e_add"                +row_id; }
function e_update_id             (row_id) { return "e_update"             +row_id; }
function e_delete_id             (row_id) { return "e_delete"             +row_id; }
function e_button_cell_id        (row_id) { return "e_button_cell"        +row_id; }
// -----------------------------------------


// -----------------------------------------
// DOM FUNCTIONS FOR DYNAMIC CHANGES TO PAGE
// -----------------------------------------

function e_insertRow(new_data, log_id, viaAJAX)
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

    var add_row = document.getElementById('exercise_log_table').insertRow(-1);
    add_row.id = row_id;

    // WARNING: this is the only way that is IE-compatible.  Stupid M$.
    var cell1 = add_row.insertCell(-1);
    var cell2 = add_row.insertCell(-1);
    var cell3 = add_row.insertCell(-1);
    var cell4 = add_row.insertCell(-1);
    var cell5 = add_row.insertCell(-1);

    cell2.innerHTML="<form><input ID='"+e_amount_id  (row_id)+"' "+extra_text+"type=text size=4 maxlength=20 onkeyup='e_calculateCalories(this)'> <span ID='"+e_amount_unit_id(row_id)+"'></span>";
    cell3.innerHTML="<input       ID='"+e_calories_id(row_id)+"' "+extra_text+"type=text size=4  maxlength=20 onkeyup='e_caloriesChanged(this)'>";
    cell4.innerHTML="<input       ID='"+e_details_id (row_id)+"' "+extra_text+"type=text size=40 maxlength=200 onkeyup='e_detailsChanged(this)'>";

    cell5.id = e_button_cell_id(row_id);
    e_insertButtons( row_id, new_data );

    if ( viaAJAX==1 )
    {
        // Use an AJAX call to build the types dropdown.
        // We need to pass the javascript row info to PHP via the AJAX post.
        cell1.innerHTML="<span id='"+e_type_cell_id(row_id)+"'>Loading...</span>";
        var param_string = "&table=0&user_id="+document.getElementById('user_id').value+"&type_dom_id="+e_type_id(row_id);
        ajaxRequest( 0, 3, param_string, e_type_cell_id(row_id), 0 );

    } else
    {
        // When using AJAX, we use a <span> around the dropdown to give us an update target.
        // We keep the <span> around the dropdown here to keep things consistent when we are navigating.
        var cell1_html = "<span id='"+e_type_cell_id(row_id)+"'>";

        // We have to call different php based on new_data.
        // But we can't actually pass the javascript var to php.
        // The only way is to branch in javascript and call different php.
        // This means we end up calling the query twice.  Oh well, it's the only way.
        if ( new_data )
        {
            cell1_html = cell1_html+"<? types_dropdown_as_innerHTML( $mysqli, 0, e_types_query($user_id), "e_temp_dropdown", 1 ); ?>";
        } else
        {
            cell1_html = cell1_html+"<? types_dropdown_as_innerHTML( $mysqli, 0, e_types_query($user_id), "e_temp_dropdown", 0 ); ?>";
        }
        cell1_html = cell1_html + "</span>";
        cell1.innerHTML=cell1_html;

        // Now reset the ID.
        document.getElementById('e_temp_dropdown').id = e_type_id(row_id);
    }

    return add_row;
}

function e_changeType(selector, new_data, update_data)
{
    var row_id  = selector.parentNode.parentNode.parentNode.id;

    if (selector.value==-1)
    {
        // "other"

        // if there is data entered, but no details, move the entered data to details...
        // nah, we've already lost the type at this time...
        // to do this right will require more work (save old type somehow).
        // if ( document.getElementById(e_details_id(row_id)).value=="" )

        // OK, this stuff is not needed, blank and disable it.
        document.getElementById(e_amount_id     (row_id)).value="";
        document.getElementById(e_amount_id     (row_id)).disabled = true;

        // However, note that we need to make sure all fields are there, so
        // that when we update the row, we can find the fields we expect.
        // THIS IS SOME FUCKED UP CODE.  But it works!
        var cal_per_amt_id = e_calories_per_amount_id(row_id);
        document.getElementById(e_amount_unit_id(row_id)).innerHTML="<input ID="+cal_per_amt_id+" type=hidden value=0>";

    } else
    {
        // alert(row_id);

        document.getElementById(e_amount_id     (row_id)).disabled = false;
        document.getElementById(e_amount_unit_id(row_id)).innerHTML="...";

        // We changed the type, we better clear the amount or the user may be confused by mismatched numbers.
        document.getElementById(e_amount_id     (row_id)).value = "";

        // Call the AJAX function to update the amount units.
        // We need to send it the row id so it can use a row-based DOM ID to hide the calories_per_unit value.
        var param_string = "&table=0&type_id="+selector.value+"&row_id="+row_id;
        ajaxRequest( 0, 1, param_string, e_amount_unit_id(row_id), 0 );
    }

    // Common cells
    document.getElementById(e_calories_id(row_id)).disabled=false;
    document.getElementById(e_details_id (row_id)).disabled=false;

    // alert( new_data );
    if ( new_data )
    {
        document.getElementById(e_add_id (row_id)).disabled=false;
        var status_text = "Adding row ";
        status_text += row_id;
        status_text += "...";
        document.getElementById('status').innerHTML=status_text;

    } else if ( update_data )
    {
        e_changedRow(row_id);
    }

    // alert(document.getElementById(e_add_id     (row_id)).value);

}

function e_fillRow( log_id, type_id, amount, calories, details )
{
    var add_row = e_insertRow( 0, log_id, 0 );

    // set type
    var selector = document.getElementById( e_type_id( log_id ) );
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
    e_changeType( selector, 0, false );

    // set amount, calories, details
    if ( type_id != -1 )
    {
        document.getElementById( e_amount_id( log_id ) ).value = amount;
    }
    document.getElementById( e_calories_id  ( log_id ) ).value = calories;
    document.getElementById( e_details_id   ( log_id ) ).value = details;

    // alert(amount+calories+details);
}

function e_calculateCalories(amount_text)
{
    var amount = amount_text.value;
    var row_id = amount_text.parentNode.parentNode.parentNode.id;

    var calories_per_amount;
    try
    {
        calories_per_amount = document.getElementById(e_calories_per_amount_id(row_id)).value;

    } catch(e)
    {
        // alert(e);
        calories_per_amount = 0;
    }
    document.getElementById(e_calories_id(row_id)).value = calories_per_amount * amount;

    e_changedRow(row_id);
}

function e_caloriesChanged(calories_text)
{
    var row_id = calories_text.parentNode.parentNode.id;
    e_changedRow(row_id);
}

function e_detailsChanged(details_text)
{
    var row_id = details_text.parentNode.parentNode.id;
    e_changedRow(row_id);
}

function e_changedRow(row_id)
{
    e_disableUpdate(row_id,false);
}

function e_disableUpdate( row_id, disable )
{
    document.getElementById(e_update_id(row_id)).disabled = disable;
}

function e_insertButtons( row_id, new_data )
{
    var buttons_html;
    if ( new_data )
    {
        buttons_html="<input      ID='"+e_add_id     (row_id)+"' DISABLED type=button onclick=\"e_addToLog(this)\" value=\"add\">";
    } else
    {
        buttons_html=             "<input ID='"+e_update_id(row_id)+"' DISABLED type=button onclick=\"e_updateLog(this)\" value=\"update\">";
        buttons_html=buttons_html+"<input ID='"+row_id+"' type=button onclick=\"e_deleteFromLog(this)\" value=\"delete\">";
    }
    buttons_html = buttons_html + "</form>";

    // alert(row_id);

    document.getElementById(e_button_cell_id(row_id)).innerHTML = buttons_html;
}

function e_addToLog( button )
{
    // alert(button.id);
    var row_id      = button.parentNode.parentNode.id;

    // gather up the data
    var daily_entries_id= document.getElementById('daily_entries_id').value;
    var type            = document.getElementById(e_type_id      (row_id)).value;
    var amount          = document.getElementById(e_amount_id    (row_id)).value;
    var calories        = document.getElementById(e_calories_id  (row_id)).value;
    var details         = document.getElementById(e_details_id   (row_id)).value;

    // validate
    if ( calories=='' ) calories = 0;
    if ( amount=='' ) amount = 0;
    details = escapeString( details );

    // rewrite the button space to say "saving..."
    document.getElementById(e_button_cell_id(row_id)).innerHTML = "saving...";
    
    // save the row id, we'll need to update it soon...
    document.getElementById('adding_row_id').value = row_id;

    // send add query via ajax
    // NOTE that if we don't have a daily_entries_id yet, we need to send the date, too!
    var param_string="&table=0&daily_entries_id="+daily_entries_id+"&type="+type+"&amount="+amount+"&calories="+calories+"&details="+details;
    if ( daily_entries_id == -1 )
    {
        var date = document.getElementById('date_id').value;
        param_string=param_string+"&date="+document.getElementById('date_id').value+"&user_id="+document.getElementById('user_id').value;;
    }
    // alert(param_string);
    ajaxRequest( 0, 4, param_string, 'added_row_id', 1 );

    // now we can insert a new row, ready for more input...
    e_insertRow( 1, 0, 1 );
}

function e_updateLog( button )
{
    var row_id = button.parentNode.parentNode.id;

    // gather up the data
    var daily_entries_id= document.getElementById('daily_entries_id').value;
    var type            = document.getElementById(e_type_id      (row_id)).value;
    var amount          = document.getElementById(e_amount_id    (row_id)).value;
    var calories        = document.getElementById(e_calories_id  (row_id)).value;
    var details         = document.getElementById(e_details_id   (row_id)).value;

    // validate
    if ( calories=='' ) calories = 0;
    if ( amount=='' ) amount = 0;
    details = escapeString( details );

    // rewrite the button space to say "saving..."
    document.getElementById(e_button_cell_id(row_id)).innerHTML = "saving...";
    
    // save the row id, we'll need to update the "saving" text...
    document.getElementById('updated_row_id').value = row_id;

    // send update query via ajax
    var param_string="&table=0&id="+row_id+"&type="+type+"&amount="+amount+"&calories="+calories+"&details="+details;
    // alert(param_string);

    ajaxRequest( 0, 5, param_string, 'status', 0 );

}

function e_deleteFromLog( button )
{
    var row_id = button.id;
    // alert(row_id);

    ajaxRequest( 0, 2, "&table=0&row_id_to_delete="+row_id, 'status', 0 );

    // Careful here, if you wrap the button in any tags, this needs to change.
    var i = button.parentNode.parentNode.rowIndex;
    // alert(i);
    document.getElementById('exercise_log_table').deleteRow(i);
}

function escapeString( st )
{
    st = st.replace(/%/g,"%25");      // 'other' %'s have to go, first...
    st = st.replace(/\+/g,"%2B");
    st = st.replace(/#/g,"%23");
    st = st.replace(/\?/g,"%3F");
    st = st.replace(/&/g,"%26");
    st = st.replace(/\//g,"%2F");
    st = st.replace(/\\/g,"%5C");
    st = st.replace(/=/g,"%3D");
    
    return st;
}

