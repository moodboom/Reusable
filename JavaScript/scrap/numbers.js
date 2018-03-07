#!/usr/bin/env node

console.log(0.1 + 0.2);
console.log(0.1 + 0.2 == 0.3);

function areTheNumbersAlmostEqual(num1, num2) {
  return Math.abs( num1 - num2 ) < Number.EPSILON;
}
console.log("A better check: "+areTheNumbersAlmostEqual(0.1 + 0.2, 0.3));

console.log();

var a = 1
var b = 1.0
var c = 1.0 + Number.EPSILON * 2
// function isInteger(x) { return Math.round(x) === x; }
console.log("Is 1 an int: "+Number.isInteger(a));
console.log("Is 1.0 an int: "+Number.isInteger(b));
console.log("Is 1.0+ an int: "+Number.isInteger(c));

console.log();

console.log("Unary + turns strings into numbers: ");
console.log(1 + "2" +"3");
console.log(1 + +"2" + +"3");

