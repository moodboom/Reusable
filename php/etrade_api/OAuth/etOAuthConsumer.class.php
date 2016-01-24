<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class etOAuthConsumer {
	public $key;
	public $secret;
	public $oauth_token;
	public $oauth_token_secret;

	function __construct($key, $secret,$callback_url= 'oob')
	{
		$this->key 		= $key;
		$this->secret 	= $secret;
		$this->callback_url = $callback_url;
	}
}

?>