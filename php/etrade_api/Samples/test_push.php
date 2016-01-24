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
 * test_push.php
 * @descriptions This application test and verify handshake with push notification server and deliver notifications.
 * 				Application should be deployed under webserver and accessed from browser.
 */

require_once("config.php");
require_once(dirname(__FILE__) . '/../Common/Common.php');


$consumer 	= new etOAuthConsumer(ETWS_APP_KEY,ETWS_APP_SECRET);
 
$consumer->oauth_token 			= OAUTH_ACCESS_TOKEN;
$consumer->oauth_token_secret 	= OAUTH_ACCESS_TOKEN_SECRET;
$method = 'GET';

		$etHttpObj = new etHttpUtils($consumer,GetURL(URL_ACCOUNTLIST),true,$method);
		$token_obj 	= new OAuthToken(	$consumer->oauth_token,	$consumer->oauth_token_secret);
		$request_obj = OAuthRequest::from_consumer_and_token($consumer,
															$token_obj,
															$method,
															PUSH_URL ,
															array());
		$sig_method = new OAuthSignatureMethod_HMAC_SHA1();	
		$request_obj->sign_request($sig_method, $consumer, $token_obj);
		$new_url = $request_obj->to_url(); 
		
?>


<html><head>
<meta http-equiv="content-type" content="text/html; charset=ISO-8859-1"><script type="text/javascript" src="js/cometd.js"></script>
    <script type="text/javascript" src="js/jquery-1.js"></script>
    <script type="text/javascript" src="js/json2.js"></script>
    <script type="text/javascript" src="js/jquery_002.js"></script>
    <script type="text/javascript" src="js/jquery.js"></script>
<script type="text/javascript" src="js/etwscomet.js"></script>
<script type="text/javascript" src="<?php echo $new_url; ?>"></script>
</head><body><h1>ETWS Streaming Demo</h1>
<div id="status"></div><br>
<button onclick="etws.init('<? echo $new_url; ?>')">Connect</button><br>
<input id="accountId"> 	<button onclick="etws.addAccount($('#accountId')[0].value)">Add Account</button><br>
<input id="raccountId"> <button onclick="etws.removeAccount($('#raccountId')[0].value)">Remove Account</button><br><br>
<button onclick="$('#logwindow').text('');$('#status').text('')">Clear</button><br>
<div id="logwindow" style="width:300px;height:300px"></div>

</body></html>