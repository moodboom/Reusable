<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */
class OptionOrderRequest extends basicOrderRequest
{
	protected $stopLimitPrice; //BigDecimal
	protected $symbolInfo; //OptionSymbol
				/* object of OptionSymbol
				 *
				 protected String symbol;
				 protected CallOrPut callOrPut;
				 protected BigDecimal strikePrice;
				 protected BigInteger expirationYear;
				 protected BigInteger expirationMonth;
				 protected BigInteger expirationDay;
				 */
	protected $orderAction; // OptionOrderAction
	/*	enum :   	BUY_OPEN,  SELL_OPEN,	 BUY_CLOSE,	 SELL_CLOSE */
	
	protected $priceType; //OptionPriceType
	/* 	enum : 	MARKET,	 LIMIT,	 STOP,	 STOP_LIMIT	 */
	
	protected $routingDestination; //String

	protected $marketSession;
	/* enum :	 REGULAR,	 EXTENDED; */
	
	protected $orderTerm; //OptionOrderTerm
	/* enum :	 GOOD_UNTIL_CANCEL,	 GOOD_FOR_DAY,	 IMMEDIATE_OR_CANCEL,	 FILL_OR_KILL */
}
?>