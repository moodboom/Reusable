<?php
    //for use with my FIRST list box
    $list1[1] = "Item1";
    $list1[2] = "Item2";
    
    if( isset($_GET['Param']) )
    {
        $NewData = "";
        $P = (int)$_GET['Param'];

        //Emulate the database
        $DB[1][] = "NewItem1 - 1";
        $DB[1][] = "NewItem1 - 2";

        $DB[2][] = "NewItem2 - 1";
        $DB[2][] = "NewItem2 - 2";
        
        //Emulate the select
        $row = $DB[$P];
        
        //use contents from DB to build the Options
        foreach($row as $K1 => $V1)
        {
            $NewData .= "<option value='$K1'>$V1</option>\n";
        }
        echo $NewData; //Send Data back
        exit; //we're finished so exit..
    }
?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />
<title>Simple Dymanic Drop Down</title>
<script language="javascript">


function ajaxFunction(ID, Param)
{
    //link to the PHP file your getting the data from
    //var loaderphp = "register.php";
    //i have link to this file
    var loaderphp = "<?php echo $_SERVER['PHP_SELF'] ?>";
    
    //we don't need to change anymore of this script
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
            document.getElementById(ID).innerHTML = xmlHttp.responseText;
        }
    }
    xmlHttp.open("GET", loaderphp+"?Param="+Param,true);
    xmlHttp.send(null);
}
</script>

</head>
<body>

<!-- OK a basic form-->
<form method="post" enctype="multipart/form-data" name="myForm" target="_self">
<table border="0">
  <tr>
    <td>
        <!-- 
        OK here we call the ajaxFuntion LBox2 refers to where the returned date will go
        and the this.value will be the value of the select option
        -->
        <select name="list1" onchange="ajaxFunction('LBox2', this.value);">
            <option value=''></option>
        <?php 
            foreach($list1 as $K1 => $V1)
            {
                echo "<option value='$K1'>$V1</option>";
            }
        ?>
        </select>
    </td>
    <td>
        <select name="list2" id="LBox2">
                <!-- OK the ID of this list box is LBox2 as refered to above -->
        </select>
    </td>
  </tr>
</table>
  <input type="submit" name="Submit" value="Submit" />
</form>
</body>
</html>