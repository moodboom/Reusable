<?php
/**
 * Etrade PHP-SDK
 *
 * LICENSE
 *
 *
 *
 * @package    	PHP-SDK
 * @version		1.0
 * @author		SatyaDev Sharma <satyadev.sharma@etrade.com>
 * @copyright  	Copyright (c) 2011 E*TRADE FINANCIAL Corp.
 *
 */
/**
 *
 * Class OrderClient
 *
 */
require_once 'Common/ETWSObj2Request.class.php';
require_once 'Common/RequestParamsMain.class.php';
require_once 'Orders/orderRequestMain.class.php';
require_once 'Orders/optionSymbol.class.php';
require_once 'Orders/ChangeOrderBase.class.php';
require_once 'Orders/basicOrderRequest.class.php';
require_once 'Orders/CancelOrder.class.php';

/* MDM fixed typo */
/* require_once 'Orders/cancelOrderRequest.class.php'; */
require_once 'Orders/CancelOrderRequest.class.php';

require_once 'Orders/changeEquityOrderRequest.class.php';
require_once 'Orders/changeOptionOrderRequest.class.php';
require_once 'Orders/equityOrderRequest.class.php';
require_once 'Orders/optionOrderRequest.class.php';
require_once 'Orders/orderListRequest.class.php';
require_once 'Orders/PlaceChangeEquityOrder.class.php';
require_once 'Orders/PlaceChangeOptionOrder.class.php';
require_once 'Orders/PlaceEquityOrder.class.php';
require_once 'Orders/PlaceOptionOrder.class.php';
require_once 'Orders/PreviewChangeEquityOrder.class.php';
require_once 'Orders/PreviewChangeOptionOrder.class.php';
require_once 'Orders/PreviewEquityOrder.class.php';
require_once 'Orders/PreviewOptionOrder.class.php';

class OrderClient
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
			throw new ETWSException("Error: Required @params: param_object(object).");
		}
	}

	private function getOrderResponse($url,$method = 'GET', $postfields = false)
	{
		//print_r($postfields);exit;
		
		$etHttpObj = new etHttpUtils($this->consumer,$url,true,$method);

		if($postfields){
			$etHttpObj->setPostfields($postfields);
		}

		$etHttpObj->GetResponse();

		return $etHttpObj->response_body;
	}

	/**
	 *
	 * @method getOrderList
	 * @param  $param_obj
	 *
	 */
	public function getOrderList($ac_id)
	{
		$resourceUrl = RequestParamsMain::buildFullURL(URL_ORDERLIST . '/'. $ac_id);
		return $this->getOrderResponse($resourceUrl);
	}
	/**
	 * 
	 * @param $request_object
	 */
	public function placeEquityOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'PlaceEquityOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PL_EQ_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function previewEquityOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'PreviewEquityOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PR_EQ_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function previewOptionOrder($request_object){
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'PreviewOptionOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PR_OP_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function placeOptionOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'PlaceOptionOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PL_OP_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function previewChangeEquityOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'previewChangeEquityOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PR_CH_EQ_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function placeChangeEquityOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'placeChangeEquityOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PL_CH_EQ_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function previewChangeOptionOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'previewChangeOptionOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PR_CH_OP_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}

	public function placeChangeOptionOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'placeChangeOptionOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_PL_CH_OP_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}
	public function cancelOrder($request_object)
	{
		$requestXml = ETWSObj2Request::buildRequestData($request_object,'cancelOrder');
		$resourceUrl = RequestParamsMain::buildFullURL(URL_CANCEL_ORDER);
		return $this->getOrderResponse($resourceUrl,'POST',$requestXml);
	}
	
}
?>
