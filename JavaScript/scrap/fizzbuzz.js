#!/usr/bin/env node

var count = 100;
for (var i=0; i<100; i++)
{
  if (i%5==0) console.log(i+" is a fizz multiple of 5");
  if (i%3==0) console.log(i+" is a buzz multiple of 3");
}
