<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class PreviewChangeOptionOrder extends RequestParamsMain
{
	protected $changeOptionOrderRequest;
	function __construct($changeOptionOrderRequest)
	{
		$this->changeOptionOrderRequest = $changeOptionOrderRequest;
	}
}

?>