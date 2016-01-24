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
 * Default configuration file.
 * Configure system environments and URLs :
*/

if (!function_exists('setConst')) {
	function setConst($name,$value)
	{
		if (! defined($name))	{ 
			define($name, $value);
		}
	}
}

setConst('ET_SDK_PATH', dirname(dirname(__FILE__)));

setConst('REQUEST_FORMAT', 'json'); // It can be 'json' or 'xml', default is xml. 
setConst('RESPONSE_FORMAT', 'json'); // It can be 'json' or 'xml', default is xml.

setConst('CURL_SSL_VERIFYPEER',0);
setConst('CURL_DEBUG_MODE',1);

setConst('DEBUG_MODE',1);
setConst('DEBUG_LOG_FILE', ET_SDK_PATH . "/etws_log.txt");

setConst('ETRADE_OAUTH_SERVER', 	'https://etws.etrade.com');
setConst('AUTHORIZE_URL',			'https://us.etrade.com/e/t/etws/authorize');
setConst('ETRADE_SERVER', 			'https://etws.etrade.com');

setConst('REQUEST_TOKEN_URL'	,	ETRADE_SERVER . '/oauth/request_token');
setConst('ACCESS_TOKEN_URL'	,		ETRADE_SERVER . '/oauth/access_token');
setConst('RENEW_TOKEN_URL'	,		ETRADE_SERVER . '/oauth/renew_access_token');
setConst('REVOKE_TOKEN_URL'	,		ETRADE_SERVER . '/oauth/revoke_access_token');

setConst('URL_ACCOUNTLIST', 		ETRADE_SERVER . '/accounts/rest/accountlist');
setConst('URL_ACCOUNTBALANCE',		ETRADE_SERVER . '/accounts/rest/accountbalance');
setConst('URL_ACCOUNTPOSITIONS',	ETRADE_SERVER . '/accounts/rest/accountpositions');
setConst('URL_ACCOUNTALERTS',		ETRADE_SERVER . '/accounts/rest/alerts');
setConst('URL_TRANSACTION_HISTORY',	ETRADE_SERVER . '/accounts/rest');

setConst('URL_OPTIONCHAINS',		ETRADE_SERVER . '/market/rest/optionchains');
setConst('URL_MARKETINDICES',		ETRADE_SERVER . '/market/rest/marketindices');
setConst('URL_PRODUCTLOOKUP',		ETRADE_SERVER . '/market/rest/productlookup');
setConst('URL_GETQUOTE',			ETRADE_SERVER . '/market/rest/quote');
setConst('URL_EXPIRYDATES',			ETRADE_SERVER . '/market/rest/optionexpiredate');

setConst('URL_ORDERLIST',			ETRADE_SERVER . '/order/rest/orderlist');

setConst('URL_PL_EQ_ORDER',			ETRADE_SERVER . '/order/rest/placeequityorder');
setConst('URL_PL_OP_ORDER',			ETRADE_SERVER . '/order/rest/placeoptionorder');
setConst('URL_PR_EQ_ORDER',			ETRADE_SERVER . '/order/rest/previewequityorder');
setConst('URL_PR_OP_ORDER',			ETRADE_SERVER . '/order/rest/previewoptionorder');
setConst('URL_PR_CH_EQ_ORDER',		ETRADE_SERVER . '/order/rest/previewchangeequityorder');
setConst('URL_PL_CH_EQ_ORDER',		ETRADE_SERVER . '/order/rest/placechangeequityorder');
setConst('URL_PR_CH_OP_ORDER',		ETRADE_SERVER . '/order/rest/previewchangeoptionorder');
setConst('URL_PL_CH_OP_ORDER',		ETRADE_SERVER . '/order/rest/placechangeoptionorder');
setConst('URL_CANCEL_ORDER',		ETRADE_SERVER . '/order/rest/cancelorder');
setConst('PUSH_URL', 'https://etwspushsb.etrade.com/apistream/cometd/oauth/');

?>