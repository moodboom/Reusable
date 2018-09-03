#!/usr/bin/env node


// https://javascript.info/promise-chaining
// "As a rule, an asynchronous action should always return a promise."


// -----------------------
// BASIC THEN AND CATCH
// -----------------------

function MyPromiseFunction(param1,param2) {
    return new Promise((resolve,reject) => {

        var thingsAreOK = (param1 == 1);

        // If we hit an error, call reject to bail out, pronto.
        if (!thingsAreOK) { reject(new Error("Bailing out!  Not cool to use input param1 = "+param1+".")) }

        // Do slow async shit.
        // Decide when to call resolve() to trigger that we are finished.
        setTimeout(() => resolve("We finished successfully, with input param1 = "+param1), 300);

    })
}
 
// then and catch are async functions, but will only fire once promise is complete.
var myGoodCall = MyPromiseFunction(1,2)
.then(function(result) {
    console.log(result)
})
.catch(function(result) {
    console.log(result)
})

// Same as above, except we pass a "bad" parameter, and we use => shorthand
var myBadCall = MyPromiseFunction(2,2)
.then(result => {
    console.log(result)
})
.catch(result => {
    console.log("We caught an error: "+result)
})

// -----------------------


// -----------------------
// ALL
// -----------------------
// TODO
/*
let urls = [
    'https://api.github.com/users/iliakan',
    'https://api.github.com/users/remy',
    'https://api.github.com/users/jeresig'
];
  
Promise.all(urls.map(url => { console.log(url) }))
// for each response show its status
.then(responses => { // (*)
    for(let response of responses) {
    alert(`${response.url}: ${response.status}`);
    }
});
*/
// -----------------------


// -----------------------
// CHAINING
// -----------------------
// TODO
// -----------------------
