<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class PreviewOptionOrder extends RequestParamsMain
{
	protected $OptionOrderRequest;
	function __construct($OptionOrderRequest)
	{
		$this->OptionOrderRequest = $OptionOrderRequest;
	}
}

?>