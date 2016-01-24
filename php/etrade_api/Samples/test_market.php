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
require_once 'Market/MarketClient.class.php';

$consumer 	= new etOAuthConsumer(ETWS_APP_KEY,ETWS_APP_SECRET);
 
$consumer->oauth_token 			= OAUTH_ACCESS_TOKEN;
$consumer->oauth_token_secret 	= OAUTH_ACCESS_TOKEN_SECRET;

$ac_obj	= new MarketClient($consumer);

echo "-------------------------------------------------\n";
echo "Welcome to Market SDK test utility.\n";
echo "-------------------------------------------------\n";
$valid_choice = array(0,1,2,3,4,5,6);
$choice = '1';
while (in_array($choice,$valid_choice))
{
	$choice = show_menu();
	try{
		switch($choice)
		{
			case 1:
				$request_params	= new getOptionChainsParams();
				$request_params->__set('expirationMonth', '1');
				$request_params->__set('expirationYear', '2015');
				$request_params->__set('chainType', 'PUT');
				$request_params->__set('skipAdjusted', 'true');
				$request_params->__set('underlier', 'GOOG');
				$out 	= $ac_obj->getOptionChain($request_params);
				break;
					
			case 2:

				$request_params	= new productLookupParams();
				$request_params->__set('company', 'cisco');
				$request_params->__set('type', 'eq');
				$out 	= $ac_obj->productLookup($request_params);
				break;
					
			case 3 :

				$request_params	= new getExpiryDateParams();
				$request_params->__set('underlier', 'GOOG');
				//$request_params->__set('expiryType', 'eq');
				$out 	= $ac_obj->getExpiryDates($request_params);
				break;
					
			case 4:
					
				$request_params	= new getQuoteParams();
				$request_params->__set('symbolList', array('GOOG','CSCO'));
				$request_params->__set('detailFlag', 'All');
				$out 	= $ac_obj->getQuote($request_params);
				break;
					
			default :
				exit;
		}



	}catch(ETWSException $e){
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

	$mkt_responce_obj = etHttpUtils::GetResponseObject($out);
	print_r($mkt_responce_obj);


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
	echo "1. Get Option Chain\n";
	echo "2. Product Lookup\n";
	echo "3. Get Expiry Dates\n";
	echo "4. Get Quote \n";
	echo "0. Exit\n\n";
	echo "Enter your choice:";
	$choice = trim(fgets(STDIN));
	return $choice;
}



?>
