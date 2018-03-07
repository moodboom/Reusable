#!/usr/bin/env node

global_var = 2;
(function ()
{
  var local_var = 1;
  var tricky_var = new_global_var = 3;
})();

console.log("local var : "+('undefined' !== typeof local_var  ? local_var  : "undefined"));
console.log("global var: "+('undefined' !== typeof global_var ? global_var : "undefined"));
console.log("tricky var: "+('undefined' !== typeof tricky_var ? tricky_var : "undefined"));