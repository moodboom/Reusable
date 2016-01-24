<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

/**
 * - Copy this file under Samples/ 
 * - It will use Samples/config.php
 *
 * - Configure following values : 
 * YOUR PUSH_URL
 * YOUR OAUTH_ACCESS_TOKEN
 * YOUR OAUTH_ACCESS_TOKEN_SECRET
 */

		define('PUSH_URL', 'https://etwspush.etrade.com/apistream/cometd/oauth/');
		define('OAUTH_ACCESS_TOKEN', 'your token');
		define('OAUTH_ACCESS_TOKEN_SECRET', 'your secret');

		
		require_once("config.php");
		require_once(dirname(__FILE__) . '/../Common/Common.php');
		
		
		$consumer 	= new etOAuthConsumer(ETWS_APP_KEY,ETWS_APP_SECRET);
 
		$consumer->oauth_token 			= OAUTH_ACCESS_TOKEN; // YOUR ACCESS TOKEN 
		$consumer->oauth_token_secret 	= OAUTH_ACCESS_TOKEN_SECRET; // YOUR ACCESS TOKEN.
		$method = 'POST';

		$etHttpObj = new etHttpUtils($consumer,GetURL(URL_ACCOUNTLIST),true,$method);
		$token_obj 	= new OAuthToken(	$consumer->oauth_token,	$consumer->oauth_token_secret);
		$request_obj = OAuthRequest::from_consumer_and_token($consumer,
															$token_obj,
															$method,
															PUSH_URL ,
															array());
		$sig_method = new OAuthSignatureMethod_HMAC_SHA1();	
		$request_obj->sign_request($sig_method, $consumer, $token_obj);
		$header_str = $request_obj->to_header();
		echo "\n----headers---------\n";
		print_r($header_str);
		echo "\n--------------------\n";
		
		$etHttpObj = new etHttpUtils($consumer,PUSH_URL,true,$method);
		$etHttpObj->GetResponse();
		print_r($etHttpObj);
		

?>
