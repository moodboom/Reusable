<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

require_once('Common/RequestParamsMain.class.php');
require_once('Market/getExpiryDateParams.class.php');
require_once('Market/getOptionChainsParams.class.php');
require_once('Market/getQuoteParams.class.php');
require_once('Market/productLookupParams.class.php');

class MarketClient
{
	/**
	 *
	 * Construct the MarketClient Object with consumer object.
	 * @param $consumer
	 *
	 */
	public function __construct($consumer)
	{
		$this->consumer = $consumer;
	}

	/**
	 *
	 * This method validates input.
	 * @method validateParamObj
	 * @param object $param_obj
	 * @param boolean $nullable
	 * @throws ETWSException
	 *
	 */
	private function validateParamObj($param_obj = null,$nullable = true)
	{
		if( ! is_object($param_obj) and (! is_null ($param_obj) or ! $nullable ))
		{
			$errorCode = 1020;
			$errorMessage = "Error: Required @params: param_object(object)";
			throw new OAuthException($errorMessage,$errorCode);
		}
	}
	
	/**
	 *
	 * @method getOptionChain
	 * @param object $param_obj
	 *
	 */
	public function getOptionChain($param_obj = null)
	{
		self::validateParamObj($param_obj,true);
		$resourceURL = RequestParamsMain::buildFullURL(URL_OPTIONCHAINS,null,$param_obj);
		return $this->getMarketResponse($resourceURL);
	}

	/**
	 *
	 * @method productLookup
	 * @param $param_obj
	 *
	 */
	public function productLookup($param_obj)
	{

		self::validateParamObj($param_obj,false);
		$resourceURL = RequestParamsMain::buildFullURL(URL_PRODUCTLOOKUP,null,$param_obj);
		return $this->getMarketResponse($resourceURL);

	}
	/**
	 *
	 * @method getExpiryDates
	 * @param object $param_obj
	 *
	 */
	public function getExpiryDates($param_obj)
	{
		self::validateParamObj($param_obj,false);
		$resourceURL = RequestParamsMain::buildFullURL(URL_EXPIRYDATES,null,$param_obj);
		return $this->getMarketResponse($resourceURL);

	}

	/**
	 *
	 * Enter description here ...
	 * @param  $param_obj
	 *
	 */
	public function getQuote($param_obj)
	{
		self::validateParamObj($param_obj,false);

		//SymboleList is REST part of URL, add it as URL part and remove it from params object.

		if(is_array($param_obj->symbolList))
		{
			//Remove empty values and join them using comma :
				
			$symbolList = implode(',',array_filter($param_obj->symbolList));
		}else{
			$symbolList = $param_obj->symbolList;
		}

		$param_obj->__set('symbolList',null);

		$resourceURL = RequestParamsMain::buildFullURL(URL_GETQUOTE,$symbolList,$param_obj);

		return $this->getMarketResponse($resourceURL);
	}



	private function getMarketResponse($url,$method = 'GET')
	{
		$etHttpObj = new etHttpUtils($this->consumer,$url,true,$method);
		$etHttpObj->GetResponse();

		return $etHttpObj->response_body;
	}
}
?>
