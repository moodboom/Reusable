<?php

/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

require_once 'Common/RequestParamsMain.class.php';
require_once 'Accounts/AccountPositionsRequest.class.php';
require_once 'Accounts/TransactionHistoryRequest.class.php';

/**
 * Class etAccounts
 */

class etAccounts
{
	/**
	 *
	 * @method __construct
	 * @param $consumer
	 */
	public function __construct($consumer)
	{
		$this->consumer = $consumer;
	}

	/**
	 * @method GetAccountList
	 */
	public function GetAccountList()
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTLIST);
		return $this->getAccountsResponse($resourceUrl);
	}

	/**
	 *
	 * @method GetAccountBalance
	 * @param int $ac_id
	 */
	public function GetAccountBalance($ac_id)
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTBALANCE,$ac_id);
		return $this->getAccountsResponse($resourceUrl);
	}

	/**
	 *
	 * @method GetAccountPositions
	 * @param int $ac_id
	 * @param object $param_obj
	 * @return string
	 * @throws OAuthException
	 */
	public function GetAccountPositions($ac_id,$param_obj)
	{
		if( empty($ac_id) or ! is_object($param_obj))
		{
			//throw new Exception("Error: Required @params: account_id(int),param_object(object).");
			$errorCode = 1011;
			$errorMessage = "Error: Required @params: account_id(int),param_object(object).";
			throw new ETWSException($errorMessage,$errorCode);
		}

		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTPOSITIONS, $ac_id ,$param_obj);
		return $this->getAccountsResponse($resourceUrl);

	}

	/**
	 * @method GetAlerts
	 * @return string
	 */

	public function GetAlerts()
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTALERTS);
		return $this->getAccountsResponse($resourceUrl);
		
	}

	/**
	 * @method GetAlertDetails
	 * @param int $alert_id
	 * @return string
	 */
	public function GetAlertDetails($alert_id)
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTALERTS,$alert_id);
		return $this->getAccountsResponse($resourceUrl);
	}

	/**
	 *
	 * @method DeleteAlert
	 * @param string $ac_id
	 * @return string
	 */
	public function DeleteAlert($alert_id)
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ACCOUNTALERTS,$alert_id);
		return $this->getAccountsResponse($resourceUrl,'DELETE');
		
	}

	/**
	 *
	 * @method GetTransactionHistory
	 */
	public function GetTransactionHistory($ac_id, $RESTParams = null, $param_obj = null )
	{

		$resourceUrl = RequestParamsMain::buildFullURL(URL_TRANSACTION_HISTORY .  '/'. $ac_id . '/transactions' ,$RESTParams,$param_obj);
		return $this->getAccountsResponse($resourceUrl);
	}

	/**
	 *
	 * @method GetTransactionHistory
	 * @param string $RESTParams - can be ActivityId or an URL with ActivityId.
	 */
	public function GetTransactionDetails($ac_id,$RESTParams)
	{

		if (is_numeric($RESTParams))
			$resourceUrl =  URL_TRANSACTION_HISTORY .  '/'. $ac_id . '/transactions/' .  $RESTParams;
		else
			$resourceUrl = $RESTParams;

		$resourceUrl = RequestParamsMain::buildFullURL($resourceUrl);
		return $this->getAccountsResponse($resourceUrl);
	}

	/**
	 *
	 * @method getAccountsResponse
	 * 	create a http call to server and get response.
	 * @param string $url
	 * @param string $method
	 */
	private function getAccountsResponse($url,$method = 'GET')
	{
		$etHttpObj = new etHttpUtils($this->consumer,$url,true,$method);
		$etHttpObj->GetResponse();

		return $etHttpObj->response_body;
	}
}

?>