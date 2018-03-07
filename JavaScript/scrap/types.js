#!/usr/bin/env node

"use strict";

var mynull = null
var myvar = 0
var myarray = [1,3,4,8]
var myfunky = function() { var a = 1; }
console.log("Is null an object? "+(typeof mynull == "object" ? "YES":"NO"));
console.log("Is an array an object? "+(typeof myarray == "object" ? "YES":"NO"));
console.log("Is a function an object? "+(typeof myfunky == "object" ? "YES":"NO"));
console.log("It is of type function, right? "+(typeof myfunky == "function" ? "YES":"NO"));
console.log("Checking defined var   with ('undefined' !== typeof a_var) "+('undefined' !== typeof myvar ? "YES":"NO"));
console.log("Checking undefined var with ('undefined' !== typeof a_var) "+('undefined' !== typeof novar ? "YES":"NO"));
