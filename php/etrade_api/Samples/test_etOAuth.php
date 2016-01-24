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
require_once(dirname(__FILE__) . '/../OAuth/etOAuth.class.php');

//-----------------------Configuration---------------------------

$key 	= ETWS_APP_KEY;
$secret = ETWS_APP_SECRET;


$browser_cmd = "C:\Program Files (x86)\Internet Explorer\iexplore";
//---------------------------------------------------------------
if(empty($key) or empty($secret))
{
	echo "Could not find application key and secret, please define them first\n";
}

echo "-------------------------------------------------\n";
echo "Welcome to etoAuth test utility.\n";
echo "-------------------------------------------------\n";

function show_menu()
{
	echo "\n\nChoose from following options..\n\n";
	echo "1. Get Token\n"; 
	echo "2. Renew Token\n";
	echo "3. Revoke Token\n";
	echo "0. Exit\n\n";
	echo "Enter your choice:";
	$choice = trim(fgets(STDIN));
	return $choice;
}


$consumer 	= new etOAuthConsumer($key,$secret);
$request 	= new etOAuth($consumer);

$choice = show_menu();
	try{
		switch($choice){
			case "1":
				//Create a fresh request.
				$request 	= new etOAuth($consumer);
				$req_token = $request->GetRequestToken();

				if(isset($req_token['oauth_token']) and isset($req_token['oauth_token_secret']))
				{
					echo "Your unauthorized token is : ";
					echo "\n---------------------------------------------------------------\n";
					echo "\nToken		: ". $req_token['oauth_token'];
					echo "\nSecret		: ". $req_token['oauth_token_secret'];
					echo "\n---------------------------------------------------------------\n";
				}else{
					echo "Could not get request token...\n";
					exit;
				}

				$auth_url = $request->GetAuthorizeURL();


				echo "Your token authorize URL is : \n";
				echo "\n---------------------------------------------------------------\n";
				echo $auth_url . "\n";
				echo "\n---------------------------------------------------------------\n";
				echo "Please follow the above URL and get verifier code, required to get final token\n\n";
				echo "Do you want to launch browser with above URL ?[y/n]\n";
				$browser = trim(fgets(STDIN));
				if(strtolower($browser) == 'y' or  strtolower($browser) == 'yes')
				{
					echo "Please close browser once you get verifier code\n";
					$command = "\"$browser_cmd\" \"$auth_url\"";
					exec ($command);
				}
				//--------------------- GET varifier --------------------------
				echo "Please enter verifier code :";
				$h = fopen ("php://stdin","r");
				$v = fgets($h);
				$v = trim($v);
					
				fclose($h);
				//--------------------- GET varifier END --------------------------

				$access_token = $request->GetAccessToken($v);
					
					
				if(isset($access_token['oauth_token']) and isset($access_token['oauth_token_secret']))
				{
					echo "Here is your final authorized token\n";
					echo "\n---------------------------------------------------------------\n";
					echo "\nToken		: ". $access_token['oauth_token'];
					echo "\nSecret		: ". $access_token['oauth_token_secret'];
					echo "\n---------------------------------------------------------------\n";
				}else{
					echo "Could not get request token...\n";
					exit;
				}
				break;
			case "2":
				if(!isset($request->oauth_token) or empty($request->oauth_token))
				{
					echo "Token is not set, please enter it manually :\n";
					echo "Token  :";
					$request->oauth_token = trim(fgets(STDIN));
					echo "Secret :";
					$request->oauth_token_secret = trim(fgets(STDIN));
				}

				$renew =  $request->RenewAccessToken();
				echo $renew;
				echo "\n";

				break;
			case "3":
				if(!isset($request->oauth_token) or empty($request->oauth_token))
				{
					echo "Token is not set, please enter it manually :\n";
					echo "Token  :";
					$request->oauth_token = trim(fgets(STDIN));
					echo "Secret :";
					$request->oauth_token_secret = trim(fgets(STDIN));
				}
				echo $request->RevokeAccessToken();
				break;
			case "0":
			default:
				echo "\n\nThanks for using test_etoAuth application!\nExiting.....\n\n";
				exit();
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


?>