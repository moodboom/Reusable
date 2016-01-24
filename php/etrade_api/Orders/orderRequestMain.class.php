<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */
require_once(dirname(__FILE__) . '/../Common/RequestParamsMain.class.php');
class orderRequestMain extends RequestParamsMain{

	protected $accountId;
	protected $clientOrderId;
	protected $limitPrice;
	protected $previewId;
	protected $stopPrice;
}



