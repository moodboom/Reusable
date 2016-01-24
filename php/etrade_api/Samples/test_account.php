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
require_once(dirname(__FILE__) . '/../Accounts/etAccounts.class.php');


$consumer 	= new etOAuthConsumer(ETWS_APP_KEY,ETWS_APP_SECRET);
 
$consumer->oauth_token 			= OAUTH_ACCESS_TOKEN;
$consumer->oauth_token_secret 	= OAUTH_ACCESS_TOKEN_SECRET;


$ac_obj = new etAccounts($consumer);

echo "-------------------------------------------------\n";
echo "Welcome to Accounts SDK test utility.\n";
echo "-------------------------------------------------\n";
$valid_choice = array(0,1,2,3,4,5,6,7,8);
$choice = '1';
while (in_array($choice,$valid_choice))
{
	$choice = show_menu();
	$starttime = ETWSCommon::get_time();
	try {
		switch($choice)
		{

			/**
			 * Get Account list
			 */
			case 1:
					
				$ac	= $ac_obj->GetAccountList();
				break;
					
					
				/**
				 * Get Account Balance
				 */
			case 2:
				//Get account id as input
				$ac_id 	= get_cmdline_input('Account ID');
				$ac 	= $ac_obj->GetAccountBalance($ac_id);
				break;
					
				/**
				 * Get Account Positions
				 */
			case 3:

				//Get account id as input.
				$ac_id 	= get_cmdline_input('Account ID');
				//Request Params.
				$request_params			= new AccountPositionsRequest();
				$request_params->__set(count,3);
				$request_params->__set(expDay,16);
					
				//TODO : User Marker for pagination.
					
				$ac 	= $ac_obj->GetAccountPositions($ac_id,$request_params); // Add counts,marker,
			
				break;
					
					
				/**
				 * Get Account Alerts
				 */
			case 4:
				$ac = $ac_obj->GetAlerts();
				break;
					
					
				/**
				 * Get Account Alerts Details
				 */
			case 5:
				//Get Alert ID as input
				$alert_id = get_cmdline_input('Alert ID');
				$ac = $ac_obj->GetAlertDetails($alert_id);
				break;
					
					
				/**
				 * Get Account Alerts Details
				 */
			case 6:
				//Get Alert ID as input
				$alert_id = get_cmdline_input('Alert ID');
				$ac = $ac_obj->DeleteAlert($alert_id);
				break;
			
			case 7:

				//Get account id as input.
				$ac_id 	= get_cmdline_input('Account ID');
				//Request Params.
				$request_params	 = new TransactionHistoryRequest();
				$request_params->__set(count,3);
				$RESTParams = '';	
				$ac 	= $ac_obj->GetTransactionHistory($ac_id,$request_params, $RESTParams); // Add counts,marker,
			
				break;
					
			
			case 8:

				//Get account id as input.
				$ac_id 	= get_cmdline_input('Account ID');
				$activity_id 	= get_cmdline_input('Activity ID');
				
				$ac 	= $ac_obj->GetTransactionDetails($ac_id,$activity_id);
			
				break;
						
			default:
				echo "\n\nThanks for using test_etoAuth application!\nExiting.....\n\n";
				exit();
		}
	}catch(ETWSException $e){
		echo 	"***Caught ETWSException***  \n".
				"Error Code 	: " . $e->getErrorCode()."\n" .
				"Error Message 	: " . $e->getErrorMessage() . "\n" ;
		if(DEBUG_MODE) echo $e->getTraceAsString() . "\n" ;
		exit;
	}catch(Exception $e){
		echo 	"***Caught Exception***  \n".
				"Error Code 	: " . $e->getCode()."\n" .
				"Error Message 	: " . $e->getMessage() . "\n" ;
		if(DEBUG_MODE) echo $e->getTraceAsString() . "\n" ;
		echo "Exiting...\n";
		exit;
	}
	$endtime = ETWSCommon::get_time();
		
	$ac_responce_obj = etHttpUtils::GetResponseObject($ac);
	print_r($ac_responce_obj);

	echo "\n totel time for optoin $choice : " . ETWSCommon::get_time_diff($starttime, $endtime);

}
/**
 * Get command line input
 * @param string $str
 */
function get_cmdline_input($str)
{
	echo "\nPlease enter * $str * : ";
	return trim(fgets(STDIN));
}

/**
 *
 * @method show_menu
 * Provides menu options.
 *
 */
function show_menu()
{
	echo "\n\nChoose from following options..\n\n";
	echo "1. Get Account list\n";
	echo "2. Get Account Balance\n";
	echo "3. Get Account Positions\n";
	echo "4. Get Account Alerts\n";
	echo "5. Get Account Alert Details\n";
	echo "6. Delete Alert\n";
	echo "7. Transaction History\n";
	echo "8. Transaction History Details\n";
	echo "0. Exit\n\n";
	echo "Enter your choice:";
	$choice = trim(fgets(STDIN));
	return $choice;
}

?>