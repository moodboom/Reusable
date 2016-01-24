<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

require_once("config.php");
require_once(dirname(__FILE__) . '/../Common/Common.php');
require_once 'Orders/OrderClient.class.php';

$consumer 	= new etOAuthConsumer(ETWS_APP_KEY,ETWS_APP_SECRET);
 
$consumer->oauth_token 			= OAUTH_ACCESS_TOKEN;
$consumer->oauth_token_secret 	= OAUTH_ACCESS_TOKEN_SECRET;

// MDM update as desired!
$accountno = 35383656;


$ac_obj	= new OrderClient($consumer);

echo "-------------------------------------------------\n";
echo "Welcome to Accounts SDK test utility.\n";
echo "-------------------------------------------------\n";
$valid_choice = array(0,1,2,3,4,5,6,7,8,9,10);
$choice = '1';
while (in_array($choice,$valid_choice))
{
	$choice = show_menu();
	try{
		switch($choice)
		{
			//1. Get Order List
			case 1: 
				// $out 	= $ac_obj->getOrderList(83310032);
				$out 	= $ac_obj->getOrderList($accountno);
			
				break;
			//2. Preview Equity Order
			case 2:
				$request_params = new EquityOrderRequest();
				
				/* From orderRequestMain. */
				$request_params->__set('accountId',$accountno);
				$request_params->__set('clientOrderId','123123');
				$request_params->__set('limitPrice',300);
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice',300);
			
				/* From basicOrderRequest. */
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity',4);
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity',0);
			
				/* From EquityOrderRequest */
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('symbol','AAPL');
				$request_params->__set('orderAction','BUY'); //{BUY,   SELL,    BUY_TO_COVER,    SELL_SHORT'}
				$request_params->__set('priceType','LIMIT');// { MARKET,	LIMIT,	STOP,	STOP_LIMIT,	MARKET_ON_CLOSE'}
				$request_params->__set('routingDestination','');
				$request_params->__set('marketSession','REGULAR');// { REGULAR, EXTENDED }
				$request_params->__set('orderTerm','GOOD_FOR_DAY'); //{ GOOD_UNTIL_CANCEL,GOOD_FOR_DAY,IMMEDIATE_OR_CANCEL,FILL_OR_KILL}
				
				$request_xml_object = new PreviewEquityOrder($request_params);
				
				$out 	= 	$ac_obj->previewEquityOrder($request_xml_object);
					
				break;
			//3. Place Equity Order
			case 3:
				
				$request_params = new EquityOrderRequest();
			
				/* From orderRequestMain. */
				$request_params->__set('accountId',83600842);
				$request_params->__set('clientOrderId','asdfdsa12312');
				$request_params->__set('limitPrice',300);
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice',300);
			
				/* From basicOrderRequest. */
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity',4);
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity',0);
			
				/* From EquityOrderRequest */
				
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('symbol','GE');
				$request_params->__set('orderAction','SELL'); //{BUY,   SELL,    BUY_TO_COVER,    SELL_SHORT'}
				$request_params->__set('priceType','MARKET');
				
				/* 
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('symbol','AAPL');
				$request_params->__set('orderAction','BUY'); //{BUY,   SELL,    BUY_TO_COVER,    SELL_SHORT'}
				$request_params->__set('priceType','LIMIT');// { MARKET,	LIMIT,	STOP,	STOP_LIMIT,	MARKET_ON_CLOSE'}
				 */
				$request_params->__set('routingDestination','');
				$request_params->__set('marketSession','REGULAR');// { REGULAR, EXTENDED }
				$request_params->__set('orderTerm','GOOD_FOR_DAY'); //{ GOOD_UNTIL_CANCEL,GOOD_FOR_DAY,IMMEDIATE_OR_CANCEL,FILL_OR_KILL}
	
				
				$request_xml_object = new PlaceEquityOrder($request_params);
				$out 	= $ac_obj->placeEquityOrder($request_xml_object);
				
				break;
			
			
			
			//4. Preview Option Order
			case 4:
			
					//Build option_symbol_obj
					$option_symbol_obj = new optionSymbol();
					$option_symbol_obj->__set('symbol','AAPL');
					$option_symbol_obj->__set('callOrPut','CALL');/// enum :  CALL,PUT;
					$option_symbol_obj->__set('strikePrice',115);
					$option_symbol_obj->__set('expirationYear','2011');
					$option_symbol_obj->__set('expirationMonth','11');
					$option_symbol_obj->__set('expirationDay','17');
					
				//Build request_params
				$request_params = new OptionOrderRequest();
				
				/* From orderRequestMain. */
				$request_params->__set('accountId',83405188);
				$request_params->__set('clientOrderId',12345);
				$request_params->__set('limitPrice',3);
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
			
				/* From basicOrderRequest. */
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity',4);
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				/* From OptionOrderRequest */ 
				$request_params->__set('symbolInfo',$option_symbol_obj);
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('orderAction','BUY_OPEN'); //{BUY_OPEN, SELL_OPEN, BUY_CLOSE, SELL_CLOSE'}
				$request_params->__set('priceType','LIMIT');// { MARKET,	LIMIT,	STOP,	STOP_LIMIT,	MARKET_ON_CLOSE'}
				$request_params->__set('marketSession','REGULAR');// { REGULAR, EXTENDED }
				$request_params->__set('orderTerm','GOOD_FOR_DAY'); //{ GOOD_UNTIL_CANCEL,GOOD_FOR_DAY,IMMEDIATE_OR_CANCEL,FILL_OR_KILL}
				$request_params->__set('routingDestination','');
				
				
				$request_xml_object = new PreviewOptionOrder($request_params);
				$out = 	$ac_obj->previewOptionOrder($request_xml_object);
					
				break;
				
			//5. Place Option Order
			case 5:
				
				//Build option_symbol_obj
					$option_symbol_obj = new optionSymbol();
					$option_symbol_obj->__set('symbol','AAPL');
					$option_symbol_obj->__set('callOrPut','CALL');/// enum :  CALL,PUT;
					$option_symbol_obj->__set('strikePrice',115);
					$option_symbol_obj->__set('expirationYear','2011');
					$option_symbol_obj->__set('expirationMonth','11');
					$option_symbol_obj->__set('expirationDay','17');
					
				//Build request_params
				$request_params = new OptionOrderRequest();
				
				/* From orderRequestMain. */
				$request_params->__set('accountId',83405188);
				$request_params->__set('clientOrderId',12345);
				$request_params->__set('limitPrice',3);
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
			
				/* From basicOrderRequest. */
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity',4);
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				/* From OptionOrderRequest */ 
				$request_params->__set('symbolInfo',$option_symbol_obj);
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('orderAction','BUY_OPEN'); //{BUY_OPEN, SELL_OPEN, BUY_CLOSE, SELL_CLOSE'}
				$request_params->__set('priceType','LIMIT');// { MARKET,	LIMIT,	STOP,	STOP_LIMIT,	MARKET_ON_CLOSE'}
				$request_params->__set('marketSession','REGULAR');// { REGULAR, EXTENDED }
				$request_params->__set('orderTerm','GOOD_FOR_DAY'); //{ GOOD_UNTIL_CANCEL,GOOD_FOR_DAY,IMMEDIATE_OR_CANCEL,FILL_OR_KILL}
				$request_params->__set('routingDestination','');
				
				
				$request_xml_object = new PlaceOptionOrder($request_params);
				$out 	= 	$ac_obj->placeOptionOrder($request_xml_object);
					
				break;
				
			//6. Preview Change Equity Order	
			case 6:
				$request_params = new changeEquityOrderRequest();

				//From changeEquityOrderRequest 
				$request_params->__set('priceType','');
				$request_params->__set('orderTerm','GOOD_UNTIL_CANCEL');
				
				//From ChangeOrderBase
				$request_params->__set('accountId',83405188);
				$request_params->__set('orderNum',162);
				$request_params->__set('clientOrderId','asdfse');
				$request_params->__set('limitPrice','');
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity','');
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				$request_xml_object = new PreviewChangeEquityOrder($request_params);
				$out = 	$ac_obj->previewChangeEquityOrder($request_xml_object);
				break;
				
			//7. Place Change equity Order
			case 7:
				$request_params = new changeEquityOrderRequest();

				//From changeEquityOrderRequest 
				$request_params->__set('priceType','');
				$request_params->__set('orderTerm','GOOD_UNTIL_CANCEL');
				
				//From ChangeOrderBase
				$request_params->__set('accountId',83405188);
				$request_params->__set('orderNum',162);
				$request_params->__set('clientOrderId','asdfse');
				$request_params->__set('limitPrice','');
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity','');
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				$request_xml_object = new PlaceChangeEquityOrder($request_params);
				$out = 	$ac_obj->placeChangeEquityOrder($request_xml_object);
				
				break;

			//8. Preview Change Option Order
			case 8:
				$request_params = new changeOptionOrderRequest();

				//From changeEquityOrderRequest 
				$request_params->__set('stopLimitPrice','');
				$request_params->__set('priceType','');
				$request_params->__set('orderTerm','GOOD_UNTIL_CANCEL');
				
				//From ChangeOrderBase
				$request_params->__set('accountId',83405188);
				$request_params->__set('orderNum',162);
				$request_params->__set('clientOrderId','asdfse');
				$request_params->__set('limitPrice','');
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity','');
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				$request_xml_object = new PreviewChangeOptionOrder($request_params);
				$out = 	$ac_obj->previewChangeOptionOrder($request_xml_object);
				
				break;
			//9. Place Change Option Order
			case 9:
				$request_params = new changeOptionOrderRequest();

				//From changeEquityOrderRequest 
				$request_params->__set('priceType','');
				$request_params->__set('orderTerm','GOOD_UNTIL_CANCEL');
				
				//From ChangeOrderBase
				$request_params->__set('accountId',83405188);
				$request_params->__set('orderNum',162);
				$request_params->__set('clientOrderId','asdfse');
				$request_params->__set('limitPrice','');
				$request_params->__set('previewId','');
				$request_params->__set('stopPrice','');
				$request_params->__set('allOrNone','');
				$request_params->__set('quantity','');
				$request_params->__set('reserveOrder','');
				$request_params->__set('reserveQuantity','');
				
				$request_xml_object = new PlaceChangeOptionOrder($request_params);
				$out = 	$ac_obj->placeChangeOptionOrder($request_xml_object);
				
				break;
				
				
				
			//10. Cancel Order
			case 10:
				
				$request_params = new CancelOrderRequest();
				$request_params->__set('accountId',83491757);
				$request_params->__set('orderNum',262);
				print_r($request_params);
				
				$request_xml_object = new CancelOrder($request_params);
				print_r($request_xml_object);
				$out 	= $ac_obj->cancelOrder($request_xml_object);
				
				break;
					
			default :
				exit;
		}



	}catch(ETWSException $e){
		/* $h2t  = new html2text($e->getErrorMessage());
		$msgtxt = $h2t->get_text(); */
		echo 	"***Caught exception***  \n".
				"Error Code 	: " . $e->getErrorCode()."\n" .
				"Error Message 	: " . $e->getErrorMessage() . "\n" ;
		if(DEBUG_MODE) echo $e->getTraceAsString() . "\n" ;
		exit;
	}catch(Exception $e){
		echo 	"***Caught exception***  \n".
				"Error Code 	: " . $e->getCode()."\n" .
				"Error Message 	: " . $e->getMessage() . "\n" ;
		if(DEBUG_MODE) echo $e->getTraceAsString() . "\n" ;
		echo "Exiting...\n";
		exit;

	}
	echo "==============Response==================";
	print_r($out);
	echo "============== Response End==================";

}


//---------------------------------------------------------

/**
 * Get command line input
 * @param
 */

function get_input($str)
{
	echo "\nPlease enter * $str * : ";
	return trim(fgets(STDIN));
}
function show_menu()
{
	echo "\n\nChoose from following options..\n\n";
	echo "1. Get Order List\n";
	echo "2. Preview Equity Order\n";
	echo "3. Place Equity Order\n";
	echo "4. Preview Option Order\n";
	echo "5. Place Option Order\n";
	echo "6. Preview Change Equity Order\n";
	echo "7. Place Change equity Order\n";
	echo "8. Preview Change Option Order\n";
	echo "9. Place Change Option Order\n";
	echo "10. Cancel Order\n";
	echo "0. Exit\n\n";
	
	echo "Enter your choice:";
	$choice = trim(fgets(STDIN));
	return $choice;
}



?>