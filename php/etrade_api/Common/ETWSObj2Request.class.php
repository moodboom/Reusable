<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class ETWSObj2Request {

	/**
	 *    Encode an object as XML string
	 *    @param object $obj
	 *    @param string $root_node
	 *    @return string $xml
	 */
	
	public function buildRequestData($obj, $root_node, $request_format = REQUEST_FORMAT) 
	{
		
		if(! $request_format || ($request_format != 'json' && $request_format != 'xml'))
			$request_format = REQUEST_FORMAT;
		
		if($request_format == 'json')
			return self::requestJson($obj, $root_node) ;
		else
			return self::requestXml($obj, $root_node) ;
	}
	/**
	 * 
	 * @param object $obj
	 * @param string $root_node
	 * @return string
	 */
	private function requestJson($obj,$root_node)
	{
		$arr[$root_node] = self::object_to_array($obj);
		$json = json_encode($arr);
		return $json;
	}
	
	/**
	 * 
	 * @param request object $obj
	 * @param string $root_node
	 * @return string
	 */
	
	private function requestXml($obj, $root_node) {

		$xml = self::encodeXml($obj, $root_node, $depth = 0);
		return $xml;
	}


	/**
	 *    Encode an object as XML string
	 *    @param object|array $data
	 *    @param string $root_node
	 *    @param int $depth  used for indentation
	 *    @return string $xml
	 */

	private function encodeXml($data, $node, $depth) {
			
		$xml = str_repeat("\t", $depth);
		if($depth == 0 )
			$xml .= "<$node xmlns=\"http://order.etws.etrade.com\" >\n";
		else
			$xml .= "<$node>\n";

		foreach($data->getVarList() as $key)
		{
			$val = $data->__get($key);
			if(is_array($val) || is_object($val)) {
				$xml .= self::encodeXml($val, $key, ($depth + 1));
			} else {
					
				$xml .= str_repeat("\t", ($depth + 1));
				$xml .= "<$key>" . htmlspecialchars($val) . "</$key>\n";
			}
		}
		$xml .= str_repeat("\t", $depth);
		$xml .= "</$node>\n";
		return $xml;
	}
	/**
	 * 
	 * @param object $var
	 * @return array
	 */

	private function object_to_array($var)
	{
		
		$result = array();
	  
	    foreach ($var->getVarList() as $key) {
	    	$value = $var->__get($key);
	        if (is_object($value) || is_array($value)) {
	                $result[$key] = self::object_to_array($value);
	        } else {
	            $result[$key] = $value;
	        }
	    }
	   
	   return $result;
	}
}
?>