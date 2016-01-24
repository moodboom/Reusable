<?php
/**
 * E*TRADE PHP SDK
 *
 * @package    	PHP-SDK
 * @version		1.1
 * @copyright  	Copyright (c) 2012 E*TRADE FINANCIAL Corp.
 *
 */

class ETWSException extends Exception
{
	protected $errorCode;
	protected $errorMessage;
	protected $httpCode;
	/**
	 * Constructor ETWSException
	 *
	 */
	public function __construct($errorMessage, $errorCode = null, $httpCode = null, Exception $previous = null) {
		$this->errorMessage 	= $errorMessage;
		$this->errorCode 		= $errorCode;
		$this->httpCode 		= $httpCode;
	}

	/**
	 * Gets the value of the errorCode property.
	 *
	 * @return
	 *     possible object is
	 *     {@link Integer }
	 *
	 */
	public function getErrorCode() {
		return $this->errorCode;
	}

	/**
	 * Gets the value of the errorMessage property.
	 *
	 * @return
	 *     possible object is
	 *     {@link String }
	 *
	 */
	public function getErrorMessage() {
		return $this->errorMessage;
	}


	/**
	 * Gets the value of the httpStatusCode property.
	 *
	 * @return
	 *     possible object is
	 *     {@link String }
	 *
	 */
	public function getHttpCode() {
		return $this->httpCode;
	}

}

?>