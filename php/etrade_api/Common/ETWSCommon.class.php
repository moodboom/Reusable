<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class ETWSCommon {

	/**
	 * Convert html br tags to new line
	 *
	 * @param string @str
	 * @return string
	 */
	public function br2nl($str)
	{
		return preg_replace('/\<br(\s*)?\/?\>/i', "\n", $str);
	}

	// Determine Start Time
	public function get_time() {
		$mtime = microtime();
		$mtime = explode(" ",$mtime);
		$mtime = $mtime[1] + $mtime[0];
		$starttime = $mtime;

		return $starttime;
	}

	// Determine end time
	public function get_time_diff($starttime,$endtime)
	{
		$totaltime = ($endtime - $starttime);
		return round($totaltime,4);
	}

	public function write_log($msg)
	{

		$date = date("Ymd H:i:s");
		$today = date("y.m.d");
		$ip = "";
			
		if ( isset($_SERVER["REMOTE_ADDR"]) )
			$ip = $_SERVER["REMOTE_ADDR"];

		$str = "[$date][$ip] " ;

		if(is_array($msg))
		{
			$str .= implode($msg);
		}else{
			$str .= $msg;
		}

		$filename = DEBUG_LOG_FILE;

		$fd = fopen($filename, "a");
		fwrite($fd, $str . PHP_EOL);
		fclose($fd);

	}

	public function objectIntoArray($arrObjData, $arrSkipIndices = array())
	{
		$arrData = array();

		// if input is object, convert into array
		if (is_object($arrObjData)) {
			$arrObjData = get_object_vars($arrObjData);
		}

		if (is_array($arrObjData)) {
			foreach ($arrObjData as $index => $value) {
				if (is_object($value) || is_array($value)) {
					$value = self::objectIntoArray($value, $arrSkipIndices); // recursive call
				}
				if (in_array($index, $arrSkipIndices)) {
					continue;
				}
				$arrData[$index] = $value;
			}
		}
		return $arrData;
	}
}
?>