<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class EquityOrderRequest extends basicOrderRequest
{

	protected $stopLimitPrice; //BigDecimal
	protected $symbol;
	protected $orderAction; 
	/* enum of EquityOrderAction  BUY,   SELL,    BUY_TO_COVER,    SELL_SHORT; */

	protected $priceType; 
	/*  enum - EquityPriceType MARKET,	LIMIT,	STOP,	STOP_LIMIT,	MARKET_ON_CLOSE;	*/

	protected $routingDestination;
	protected $marketSession;
	/* enum - MarketSession 	REGULAR,	EXTENDED; 	*/

	protected $orderTerm; 
	/* enum - EquityOrderTerm 	GOOD_UNTIL_CANCEL,	GOOD_FOR_DAY,	IMMEDIATE_OR_CANCEL,	FILL_OR_KILL;	*/

}
?>