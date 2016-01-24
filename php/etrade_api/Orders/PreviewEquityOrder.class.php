<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class PreviewEquityOrder extends RequestParamsMain
{
	protected $EquityOrderRequest;
	function __construct($EquityOrderRequest)
	{
		$this->EquityOrderRequest = $EquityOrderRequest;
	}
}

?>