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
 * PHP-SDK
 * config.php 
 * This file overrides default config file.
 * 
 */


// Configure system environments and URLs :

if (!function_exists('setConst')) {
	function setConst($name,$value)
	{
		if (! defined($name))	{ 
			define($name, $value);
		}
	}
}
//Server specific config:
$str = 'sandbox' ;// options : prod, sandbox
switch($str)
{
	case 'prod' :

                //===========<PROD - Fill out consumerKey(APP_KEY), secret(APP_SECRET), oauth access token & secret below>============
                setConst('ETWS_APP_KEY',                        '');
                setConst('ETWS_APP_SECRET',                     '');
                setConst('ETRADE_OAUTH_SERVER',         'https://etws.etrade.com');
                setConst('AUTHORIZE_URL',                       'https://us.etrade.com/e/t/etws/authorize');
                setConst('ETRADE_SERVER',                       'https://etws.etrade.com');
                setConst('OAUTH_ACCESS_TOKEN',          '');
                setConst('OAUTH_ACCESS_TOKEN_SECRET','');
                setConst('PUSH_URL', 'https://etwspush.etrade.com/apistream/cometd/oauth/');

                $url_str_part = '';

                break;

	case 'sandbox' : 	
		//===========<Sandbox - Fill out consumerKey(APP_KEY), secret(APP_SECRET), oauth access token & secret below>============
		setConst('ETWS_APP_KEY',			'');
		setConst('ETWS_APP_SECRET',			'');
		setConst('ETRADE_OAUTH_SERVER', 	'https://etws.etrade.com');
		setConst('AUTHORIZE_URL',			'https://us.etrade.com/e/t/etws/authorize');
		setConst('ETRADE_SERVER', 			'https://etwssandbox.etrade.com');
		setConst('OAUTH_ACCESS_TOKEN',		'');
		setConst('OAUTH_ACCESS_TOKEN_SECRET','');
		
		setConst('PUSH_URL', 'https://etwspushsb.etrade.com/apistream/cometd/oauth/');
		$url_str_part = 'sandbox/';
		break;
		
	default : 
		echo "No environment defined, please check config.php file....\n";
		exit;
}
setConst('RESPONSE_FORMAT', 'xml');
setConst('REQUEST_FORMAT', 'json'); // It can be 'json' or 'xml', default is xml.
setConst('DEBUG_MODE',1);

setConst('REQUEST_TOKEN_URL',	ETRADE_OAUTH_SERVER . '/oauth/request_token');
setConst('ACCESS_TOKEN_URL'	,	ETRADE_OAUTH_SERVER . '/oauth/access_token');
setConst('RENEW_TOKEN_URL'	,	ETRADE_OAUTH_SERVER . '/oauth/renew_access_token');
setConst('REVOKE_TOKEN_URL'	,	ETRADE_OAUTH_SERVER . '/oauth/revoke_access_token');


setConst('URL_ACCOUNTLIST', 	ETRADE_SERVER . '/accounts/'.$url_str_part.'rest/accountlist');
setConst('URL_ACCOUNTBALANCE',	ETRADE_SERVER . '/accounts/'.$url_str_part.'rest/accountbalance');
setConst('URL_ACCOUNTPOSITIONS',ETRADE_SERVER . '/accounts/'.$url_str_part.'rest/accountpositions');
setConst('URL_ACCOUNTALERTS',	ETRADE_SERVER . '/accounts/'.$url_str_part.'rest/alerts');
setConst('URL_TRANSACTION_HISTORY',	ETRADE_SERVER . '/accounts/'.$url_str_part.'rest');

setConst('URL_OPTIONCHAINS',	ETRADE_SERVER . '/market/'.$url_str_part.'rest/optionchains');
setConst('URL_MARKETINDICES',	ETRADE_SERVER . '/market/'.$url_str_part.'rest/marketindices');
setConst('URL_PRODUCTLOOKUP',	ETRADE_SERVER . '/market/'.$url_str_part.'rest/productlookup');
setConst('URL_GETQUOTE',		ETRADE_SERVER . '/market/'.$url_str_part.'rest/quote');
setConst('URL_EXPIRYDATES',		ETRADE_SERVER . '/market/'.$url_str_part.'rest/optionexpiredate');


setConst('URL_ORDERLIST',		ETRADE_SERVER . '/order/'.$url_str_part.'rest/orderlist');
setConst('URL_PL_EQ_ORDER',		ETRADE_SERVER . '/order/'.$url_str_part.'rest/placeequityorder');
setConst('URL_PL_OP_ORDER',		ETRADE_SERVER . '/order/'.$url_str_part.'rest/placeoptionorder');
setConst('URL_PR_EQ_ORDER',		ETRADE_SERVER . '/order/'.$url_str_part.'rest/previewequityorder');
setConst('URL_PR_OP_ORDER',		ETRADE_SERVER . '/order/'.$url_str_part.'rest/previewoptionorder');
setConst('URL_PR_CH_EQ_ORDER',	ETRADE_SERVER . '/order/'.$url_str_part.'rest/previewchangeequityorder');
setConst('URL_PL_CH_EQ_ORDER',	ETRADE_SERVER . '/order/'.$url_str_part.'rest/placechangeequityorder');
setConst('URL_PR_CH_OP_ORDER',	ETRADE_SERVER . '/order/'.$url_str_part.'rest/previewchangeoptionorder');
setConst('URL_PL_CH_OP_ORDER',	ETRADE_SERVER . '/order/'.$url_str_part.'rest/placechangeoptionorder');
setConst('URL_CANCEL_ORDER',	ETRADE_SERVER . '/order/'.$url_str_part.'rest/cancelorder');


?>
