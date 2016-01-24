<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */
class RequestParamsMain
{

	public function __set($property, $value) {
		if(property_exists($this,$property))
		{
			$this->{$property} = $value;
			return true;
		}else{
			return false;
		}
	}

	public function __get($property)
	{
		if(property_exists($this,$property))
			return $this->$property;
		else
			return false;
	}

	public function buildQueryString($queryParamsObj)
	{
		$url = '';
		foreach($queryParamsObj as $k=>$v)
		{
			if(!empty($v))
			{
				$url .= $k.'='. urlencode($v) .'&';
			}
		}
		$url = rtrim($url,"&");
		return $url;
	}
	public function buildFullURL($baseURL, $RESTParams = null, $queryParamsObj = null)
	{
		$url = rtrim($baseURL, '/');
		
		$RESTParams = trim($RESTParams);
		$RESTParams = rtrim($RESTParams, '/');
		
		if(! is_null($RESTParams) and ! empty($RESTParams)) 
		{
			$url  .= '/'. $RESTParams;
		}
		
		if( RESPONSE_FORMAT == 'json' )	{
			$url .= '.json';
		}
		
		if( is_object($queryParamsObj) and !is_null($queryParamsObj) )
		{
			$url .= '?' . self::buildQueryString($queryParamsObj);
		}
		return $url;
	}
	public function getVarList()
	{
		$a = array();
		foreach($this as $var=>$val)
		{
			$a[] = $var;
		}
		return $a;
	}

}

?>