<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class CancelOrder extends RequestParamsMain
{
	protected $cancelOrderRequest;
	function __construct($cancelOrderRequest)
	{
		$this->cancelOrderRequest = $cancelOrderRequest;
	}
}