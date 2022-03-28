#!/usr/bin/env node

console.log();
console.log('Test 1: split');
var a = "a string"
var b = a.split(' ');
var c = a.split(''); // on every char
var d = c.reverse();
var e = d.join('');  // slaps em back together, with optional separator
console.log(a+';'+b+';'+c+';'+d+';'+e)
console.log();

console.log('\nTest 2: replace whitespace');
const f = "a string with whitespace";
const g = f.replace(/\W/g, '====');
console.log(f, " => ", g);
console.log();

console.log('Test 3: replace single quotes');
const quotedString = `'this is quoted'`;
const unquotedString = quotedString.replace(/\'/g, '');
console.log(quotedString, " => ", unquotedString);
console.log();
