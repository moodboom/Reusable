#!/usr/bin/env node

// Closures

var globalVar = "xyz";

(function outerFunc(outerArg) {
  var outerVar = 'a';

  (function innerFunc(innerArg) {
    var innerVar = 'b';

    console.log(
      "outerArg = " + outerArg + "\n" +
      "innerArg = " + innerArg + "\n" +
      "outerVar = " + outerVar + "\n" +
      "innerVar = " + innerVar + "\n" +
      "globalVar = " + globalVar);

  })(456);
})(123);


var cascade = function() {

  // Sorry charlie, the loop will complete before any timeout fires, you'll get all 5's
  // BUT, the timing of the output will be good :-)
  for (var i = 0; i < 5; i++) {
    setTimeout(function() { console.log(i); }, i * 1000 );
  }

  // USE let to cause i to have "block scope" (ie it will NOT be shared with internal scope!)
  for (let i = 0; i < 5; i++) {
    setTimeout(function() { console.log(i); }, i * 1000 );
  }

  // Or explicitly use a closure
  for (var i = 0; i < 5; i++) {
    (function(x) {
      setTimeout(function() { console.log(x); }, x * 1000 );
    })(i);
  }
}
cascade()