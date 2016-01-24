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
 * Class etHttpUtils
 * 
 */
class etHttpUtils
{
	public $consumer;
	public $method = 'GET';
	public $postfields = false;
	public $params = array();


	/**
	 *
	 * Constructor for etHttpUtils class.
	 * @param string $url
	 * @param array or string $postfields
	 * @param string $headers
	 * @param string $method GET, POST, DELETE, PUT, false
	 */
	function __construct(	$consumer,	$url, $headers = false,	$method = 'GET')
	{
		$this->consumer 	= $consumer;
		$this->request_url	= $url;
		$this->headers 		= $headers;
		$this->method 		= $method;
		$this->use_ssl		= CURL_SSL_VERIFYPEER;
	}
	/**
	 * @method getSignedURLandHeaders
	 */
	public function getSignedURLandHeaders()
	{
		$signedObj	=	$this->getRequestObject();

		if($this->headers){
			
			if(REQUEST_FORMAT == 'json')
				$this->headers =  array("Content-Type: application/json",$signedObj->to_header());
			else
				$this->headers =  array("Content-Type: application/xml", $signedObj->to_header());
		}else{
			//Use fallback as Query String 
			$this->request_url = $signedObj->to_url();
		}
		
	}
	/**
	 * @method setPostfields
	 * @param boolean,string,array $postfields
	 */
	public function setPostfields($postfields)
	{
		if(is_array($postfields))
		{
			$this->postfields = http_build_query($postfields, '', '&');
		}else{
			$this->postfields = $postfields;
		}
	}

	/**
	 * @method getRequestObject
	 * @return $request_obj
	 */
	private function getRequestObject()
	{
		if(isset($this->consumer->oauth_token) 			and	!empty($this->consumer->oauth_token)
		and isset($this->consumer->oauth_token_secret) 	and !empty($this->consumer->oauth_token_secret)
		)
		{
			$token_obj 	= new OAuthToken(	$this->consumer->oauth_token,
			$this->consumer->oauth_token_secret);
		}else{
			$token_obj 	= null;
		}
		$request_obj = OAuthRequest::from_consumer_and_token($this->consumer,
															$token_obj,
															$this->method,
															$this->request_url,
															$this->params);
	
		$sig_method = new OAuthSignatureMethod_HMAC_SHA1();
		$request_obj->sign_request($sig_method, $this->consumer, $token_obj);

		return $request_obj;
	}

	/**
	 *
	 * Make http request and get reesponse.
	 * @throws OAuthException
	 */
	public function DoHttpRequest()
	{
		$ch = curl_init();
		// set URL and other appropriate options
		curl_setopt($ch, CURLOPT_URL, 			$this->request_url );
		curl_setopt($ch, CURLOPT_VERBOSE, 		CURL_DEBUG_MODE);
		curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, $this->use_ssl);
		if($this->postfields or $this->method == 'POST')
		{
			curl_setopt($ch, CURLOPT_POST, 		true);
		}
		if($this->postfields )
		{
			curl_setopt($ch, CURLOPT_POSTFIELDS, $this->postfields);
		}
		if($this->headers)
		{
			curl_setopt($ch, CURLOPT_HTTPHEADER, $this->headers);
		}
		if($this->method  and $this->method != 'GET' and $this->method != 'POST' )
		{
			curl_setopt($ch, CURLOPT_CUSTOMREQUEST,$this->method);
		}
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		curl_setopt($ch, CURLOPT_HEADER, true);
		curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 60);
		/*curl_setopt($ch, CURLOPT_SSLVERSION, 3);*/
		curl_setopt($ch, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);

		//--------------------------------
		$this->result = curl_exec($ch);
		//--------------------------------

		if(curl_errno($ch))
		{
			$errorCode = 1001;
			$errorMessage = "Error no : " . curl_errno($ch) . "\nError : " . curl_error($ch);
			throw new OAuthException($errorMessage,$errorCode);
		}
		else
		{
			$curl_info 				= curl_getinfo($ch);

			$this->response_header	= substr($this->result, 0,$curl_info['header_size']);
			$this->response_body	= substr($this->result, $curl_info['header_size']);
			$this->http_code 		= $curl_info['http_code'];

		}
		// close cURL resource, and free up system resources
		curl_close($ch);
		
		if(preg_match("/<Error><ErrorCode>/",$this->response_body))
		{
			throw new ETWSException($this->response_body,$this->http_code);
			
		}elseif($this->http_code < 200 or $this->http_code > 299 ){
			$msg_str  = 	$this->response_header ;
			if(DEBUG_MODE){
				ETWSCommon::write_log($this->response_body);
			}
			throw new ETWSException($msg_str,$this->http_code);
		}
	}

	/**
	 * @method GetResponse
	 */
	public function GetResponse()
	{
		$this->getSignedURLandHeaders();
		$this->DoHttpRequest();
	}
	
	/**
	 * Get response as an object based on response format.
	 * @method GetResponseObject
	 * @param string $str
	 */
	public function GetResponseObject($str)
	{
		if( RESPONSE_FORMAT == 'json' )
		{
			$response_as_object = json_decode($str);
		}else{
			$response_as_object = new SimpleXMLElement($str);
		}
		return $response_as_object;
	}

}
?>
