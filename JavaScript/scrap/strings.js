#!/usr/bin/env node

var a = "a string"
var b = a.split(' ');
var c = a.split(''); // on every char
var d = c.reverse();
var e = d.join('');  // slaps em back together, with optional separator

console.log(a+';'+b+';'+c+';'+d+';'+e)

var f = e.toUpperCase();
console.log(f);

var g = f.replace(/\W/g, '====');
console.log(g);
