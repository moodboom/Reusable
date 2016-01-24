<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

require_once 'Common/RequestParamsMain.class.php';

class TransactionHistoryRequest extends RequestParamsMain
{
	protected $count;
	protected $startDate;
	protected $endDate;
	protected $marker;
}
?>