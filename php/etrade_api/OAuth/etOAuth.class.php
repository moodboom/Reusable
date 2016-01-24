<?php

/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */


class etOAuth 
{

	/**
	 * __construct
	 * @param object etOAuthConsumer
	 * @return object etOAuth
	 */
	function __construct($consumer)
	{
		$this->key 		= $consumer->key;
		$this->secret 	= $consumer->secret;
		$this->callback_url = $consumer->callback_url;
	}
	/**
	 * GetRequestToken
	 * @return array $req_token
	 * @throws OAuthException
	 * @throws Exception
	 */
	private function ValidateRequiredParams($required_params)
	{
		$empty_params = array();
		if(!is_array($required_params)) return;
		foreach($required_params as $key=>$value){
			if(empty($value)){
				$empty_params[] = $key;		
			}
		}
		if(!empty($empty_params))
		{
			$errorCode = 1051;
			$errorMessage = "Required parameter absent : " . implode(",",$empty_params);
			throw new ETWSException($errorMessage,$errorCode);
		}
	}
	function GetRequestToken($request_token_url = REQUEST_TOKEN_URL)
	{
		$required_params['Consumer Key'] = $this->key;
		$required_params['Consumer Secret'] = $this->secret;
		
		$this->ValidateRequiredParams($required_params);
			
		$etHttpObj = new etHttpUtils($this,$request_token_url,true);
		$etHttpObj->params = array('oauth_callback' => $this->callback_url);
		
		$etHttpObj->GetResponse();
		$req_token_str = $etHttpObj->response_body;

		parse_str ($req_token_str,$req_token);
		if( !isset($req_token['oauth_token']) or empty($req_token['oauth_token']) or
		!isset($req_token['oauth_token_secret']) or empty($req_token['oauth_token_secret']))
		{
			throw new OAuthException("Error : Could not get request token. See detailed message : \n" . $req_token_str);
		}

		$this->oauth_token 			= $req_token['oauth_token'];
		$this->oauth_token_secret 	= $req_token['oauth_token_secret'];

		return $req_token;
	}

	/**
	 * @method GetAuthorizeURL
	 * @return string
	 * @throws OAuthException
	 *
	 */	
	function GetAuthorizeURL()
	{
		$required_params['Consumer Key'] = $this->key;
		$required_params['Request Token']  = $this->oauth_token;
		
		$this->ValidateRequiredParams($required_params);
		
		return AUTHORIZE_URL . "?key=".$this->key."&token=". urlencode($this->oauth_token);
	
	}

	/**
	 *
	 * @method GetAccessToken
	 * @param  string $oauth_verifier
	 * @return array $token
	 * @throws OAuthException
	 */
	function GetAccessToken($oauth_verifier,$access_token_url = ACCESS_TOKEN_URL)
	{

		$required_params['Consumer Key'] 	= $this->key;
		$required_params['Consumer Secret'] = $this->secret;
		$required_params['Request Token']  	= $this->oauth_token;
		$required_params['Request Secret']  = $this->oauth_token_secret;
		$required_params['Verifier Code']  	= $oauth_verifier;
		
		$this->ValidateRequiredParams($required_params);
		
		$etHttpObj = new etHttpUtils($this,$access_token_url,true);
		$etHttpObj->params = array('oauth_verifier' => $oauth_verifier);
		$etHttpObj->GetResponse();
		$acc_token_str = $etHttpObj->response_body;

		parse_str ($acc_token_str,$token);
		
		if( !isset($token['oauth_token']) 		or empty($token['oauth_token']) or
		!isset($token['oauth_token_secret']) 	or empty($token['oauth_token_secret']))
		{
			throw new OAuthException("Error : Could not get access token. See detailed message : \n" . $acc_token_str);
		}

		$this->oauth_token 			= $token['oauth_token'];
		$this->oauth_token_secret	= $token['oauth_token_secret'];

		return $token;
	}

	/**
	 *
	 * @method RenewAccessToken
	 * @return sting
	 */
	function RenewAccessToken()
	{
		return $this->UpdateToken(RENEW_TOKEN_URL);
	}

	/**
	 *
	 * @method RevokeAccessToken
	 * @return string
	 */
	function RevokeAccessToken()
	{
		return $this->UpdateToken(REVOKE_TOKEN_URL);
	}

	/**
	 *
	 * @method UpdateToken
	 * @param sting $url
	 * @return string
	 */
	private function UpdateToken($url) 
	{
		$etHttpObj = new etHttpUtils($this,$url,true);
		$etHttpObj->GetResponse();
		return $etHttpObj->response_body;
	}
}

?>