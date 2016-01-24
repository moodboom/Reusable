<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */
class optionSymbol extends RequestParamsMain
{
	protected $symbol;
	protected $callOrPut;
	protected $strikePrice;
	protected $expirationYear;
	protected $expirationMonth;
	protected $expirationDay;
}