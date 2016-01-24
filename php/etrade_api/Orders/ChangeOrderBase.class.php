<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class ChangeOrderBase extends RequestParamsMain
{
	protected $accountId;
	protected $orderNum;
	protected $clientOrderId;
	protected $limitPrice;
	protected $previewId;
	protected $stopPrice;
	protected $allOrNone;
	protected $quantity;
	protected $reserveOrder;
	protected $reserveQuantity;
}