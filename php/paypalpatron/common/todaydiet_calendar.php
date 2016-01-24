<?php
/*
 * TODAY DIET CALENDAR
 * 
 * Features:
 * 
 *    display a month calendar
 *    days colored red/green/unknown depending on log data
 *    each day is a link to an AJAX request to change displayed date
 *    three calendars are displayed (prev|current|next)
 *
 * VERY remotely derived from the following:
 * =========================================
 * Quick Calendar Using PHP and AJAX
 * Copyright (C) 2005-2007
 * Version 1.1;
 * Last modified: 23 Jan 2007
 * Author: Bernard Peh
 * Email: bpeh@sitecritic.net
 * Website://web-developer.sitecritic.net/
 * File Name: quick_calendar.php
 *
 * LICENSE: 
 * This is my contribution back to the open source community. You may modify the codes according 
 * to your needs but please keep this section intact.
 * 
 * DESCRIPTION:
 * Generate a simple calendar that can integrate seamlessly into any system with minimal 
 * installation. You must be running be running PHP 4 at the minimal. 
 * 
 * SPECIAL THANKS TO FRIENDS FROM EVOLT.ORG
 * Adam Taylor, kirk837
 *
 * INSTALLATION:
 * 1. Save the code in a file call quick_calendar.php. Then Insert this file into anywhere where 
 * you want the calendar to appear. Use:
 *
 *      require_once('quick_calendar.php');
 *
 *		or if you save the file elsewhere, require_once('dir_path/quick_calendar.php')
 *
 * 2. Create a table in your database. If you are using your own table, you need to map the fields
 * appropriately.
 *
 * CREATE TABLE `calendar` ( 
 * `id` INT NOT NULL AUTO_INCREMENT ,
 * `day` VARCHAR( 2 ) NOT NULL ,
 * `month` VARCHAR( 2 ) NOT NULL ,
 * `year` VARCHAR( 4 ) NOT NULL ,
 * `link` VARCHAR( 255 ) NOT NULL ,
 * `desc` TEXT NOT NULL ,
 * PRIMARY KEY ( `id` ) 
 * );
 *
 * 3. Configure the db and path access below. Use any db of your choice. You can also configure 
 * the CSS to change the look and feel of the calendar.
 * =====================================
 */

// This actually connects to the database and gives us a live [mysqli] object to use.
// It was most likely already done before this point...
require_once './common/mysql_connect.php';
 
// This sets up all the date variables we need for "targetdate" and "today".
require_once './common/get_date.php';

// name of table
$tableName = 'calendar';

// name of css
$css = 'calendar';

// Location of the calendar script file from the root
$ajaxPath = '/quick_calendar.php';

// END OF CONFIGURATION. YOU CAN CHANGE THE CSS. THE OTHER CODES CAN BE KEPT AS DEFAULT IF YOU WANT.




// MDM our calendar uses a set of colors for each day - get them for the current month+year
// $color_sql = "SELECT color, date FROM daily_entries WHERE user_id = ".$user_id." && year(date) == ".$year." && month(date) == ".$month." && day(date) == ".$day." 



$sql = "SELECT * FROM $tableName WHERE (month='$m' AND year='$y') || (month='*' AND year='$y') || (month='$m' AND year='*') || (month='*' AND year='*')";

// MDM Load array of links associated with days.
// Eventually we'll replace this - each date will fire an AJAX call to reset the current date, and
// will use a mysql query to determine the weight change (which will determine the color).
$rs = $mysqli->query($sql);
$links = array(); 
while ($rw = $rs->fetch_row()) {
	extract($rw);
	$links[] = array('day'=>$day, 'month'=>$month, 'year'=>$year, 'link'=>$link, 'desc'=>$desc);
}
?>

<?php
class CreateQCalendarArray {

	var $daysInMonth;
	var $weeksInMonth;
	var $firstDay;
	var $week;
	var $month;
	var $year;

	function CreateQCalendarArray($month, $year) {
		$this->month = $month;
		$this->year = $year;
		$this->week = array();
		$this->daysInMonth = date("t",mktime(0,0,0,$month,1,$year));
		// get first day of the month
		$this->firstDay = date("w", mktime(0,0,0,$month,1,$year));
		$tempDays = $this->firstDay + $this->daysInMonth;
		$this->weeksInMonth = ceil($tempDays/7);
		$this->fillArray();
	}
	
	function fillArray() {
		// create a 2-d array
		for($j=0;$j<$this->weeksInMonth;$j++) {
			for($i=0;$i<7;$i++) {
				$counter++;
				$this->week[$j][$i] = $counter; 
				// offset the days
				$this->week[$j][$i] -= $this->firstDay;
				if (($this->week[$j][$i] < 1) || ($this->week[$j][$i] > $this->daysInMonth)) {	
					$this->week[$j][$i] = "";
				}
			}
		}
	}
}

class QCalendar {
	
	var $html;
	var $weeksInMonth;
	var $week;
	var $month;
	var $year;
	var $today;
    var $targetdate;
	var $links;
	var $css;
    var $align;         # MDM 0 = single, 1 = left 1of3, 2 = center 2of3, 3 = right 3of3

	function QCalendar($month, $year, $today, $targetdate, &$links, $css='', $align=0) 
    {
		$this->month = $month;
		$this->year = $year;
        if ( $align == 1 )
        {
            $this->month -= 1;
            if ( $this->month == 0 )
            {
                $this->month = 12;
                $this->year -= 1;
            }
        }
        if ( $align == 3 )
        {
            $this->month += 1;
            if ( $this->month == 13 )
            {
                $this->month = 1;
                $this->year += 1;
            }
        }

        $cArray = &new CreateQCalendarArray($this->month,$this->year);

		$this->weeksInMonth = $cArray->weeksInMonth;
		$this->week = $cArray->week;
		$this->today = $today;
		$this->targetdate = $targetdate;
		$this->links = $links;
		$this->css = $css;
        $this->align = $align;
		$this->createHeader();
		$this->createBody();
		$this->createFooter();
	}
	
	function createHeader() {
  		$header = date('M', mktime(0,0,0,$this->month,1,$this->year)).' '.$this->year;
  		$nextMonth = $this->month+1;
  		$prevMonth = $this->month-1;
  		// thanks adam taylor for modifying this part
		switch($this->month) {
			case 1:
	   			$lYear = $this->year;
   				$pYear = $this->year-1;
   				$nextMonth=2;
   				$prevMonth=12;
   				break;
  			case 12:
   				$lYear = $this->year+1;
   				$pYear = $this->year;
   				$nextMonth=1;
   				$prevMonth=11;
      			break;
  			default:
      			$lYear = $this->year;
	   			$pYear = $this->year;
    	  		break;
  		}
		// --
		$this->html = "<table cellspacing='0' cellpadding='0' class='$this->css'>
        <tr>
        <th class='header'>";
        if ( $this->align == 0 || $this->align == 1 )
        {
            $this->html .= "&nbsp;<a href=\"javascript:;\" onclick=\"displayQCalendar('$this->month','".($this->year-1)."')\" class='headerNav' title='Prev Year'><<</a>";
        }
        $this->html .= "</th><th class='header'>";
        if ( $this->align == 0 || $this->align == 1 )
        {
            $this->html .= "&nbsp;<a href=\"javascript:;\" onclick=\"displayQCalendar('$prevMonth','$pYear')\" class='headerNav' title='Prev Month'><</a>";
        }
        $this->html .= "</th><th colspan='3' class='header'>$header</th><th class='header'>";
        if ( $this->align == 0 || $this->align == 3 )
        {
            $this->html .= "<a href=\"javascript:;\" onclick=\"displayQCalendar('$nextMonth','$lYear')\" class='headerNav' title='Next Month'>></a>&nbsp;";
        }
        $this->html .= "</th><th class='header'>";
        if ( $this->align == 0 || $this->align == 3 )
        {
            $this->html .= "&nbsp;<a href=\"javascript:;\" onclick=\"displayQCalendar('$this->month','".($this->year+1)."')\"  class='headerNav' title='Next Year'>>></a>";
        }
        $this->html .= "</th></tr>";
	}
	
	function createBody()
    {
		// start rendering table
		$this->html.= "<tr><th>S</th><th>M</th><th>T</th><th>W</th><th>Th</th><th>F</th><th>S</th></tr>";
		for($j=0;$j<$this->weeksInMonth;$j++) 
        {
			$this->html.= "<tr>";
			for ($i=0;$i<7;$i++) 
            {
                $loopday = $this->week[$j][$i];

                // MDM Make each date a link to display that date.
                $cellValue = "<a href='./index.php?day=".$loopday."&month=".$this->month."&year=".$this->year."'>".$loopday."</a>";

                // Special Markup for today, targetdate                
				// Is this "today"?
				if (($this->today['day'] == $loopday) && ($this->today['month'] == $this->month) && ($this->today['year'] == $this->year)) 
                {
					$cell = "<div class='today'>$cellValue</div>";

				// Is this the "target" date?
                } else if (($this->targetdate['day'] == $loopday) && ($this->targetdate['month'] == $this->month) && ($this->targetdate['year'] == $this->year)) 
                {
					$cell = "<div class='targetdate'>$cellValue</div>";

				// "normal" day
				} else 
                {
					$cell = "$cellValue";
				}

                // MDM Old link thingee
				// $cellValue = $this->week[$j][$i];
				// if days with link
				// foreach ($this->links as $val) {
				// 	if (($val['day'] == $cellValue) && (($val['month'] == $this->month) || ($val['month'] == '*')) && (($val['year'] == $this->year) || ($val['year'] == '*'))) {
				// 		$cell = "<div class='link'><a href=\"{$val['link']}\" title='{$val['desc']}'>$cellValue</a></div>";
				// 		break;
				// 	}
				// }

				$this->html.= "<td>$cell</td>";
			}
			$this->html.= "</tr>";
		}	
	}
	
	function createFooter() {
		$this->html .= "<tr><td colspan='7' class='footer'>";
        if ( $this->align == 0 || $this->align == 2 )
        {
            $this->html .= "<a href=\"javascript:;\" onclick=\"displayQCalendar('{$this->today['month']}','{$this->today['year']}')\" class='footerNav'>Today is {$this->today['day']} ".date('M', mktime(0,0,0,$this->today['month'],1,$this->today['year']))." {$this->today['year']}</a>";
        }
        $this->html .= "</td></tr></table>";
	}
	
	function render() {
		echo $this->html;
	}
}
?>

