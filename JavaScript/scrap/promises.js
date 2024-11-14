#!/usr/bin/env node


// MAJOR ASYNC GOALS
// 1) CHAIN perform a sequence of long-running async tasks in synchronous order (main thread won't know when it finishes - aka "fire and forget")
// 2) PARALLEL perform parallel tasks, each with their own interwoven output.
// 3) BATCH perform a group of long-running async tasks simultaneously, then run another task only after they all complete.  (you want to do something when it's "done")

// Just use these patterns and save yourself time.

// REMEMBER node always rips right past all the promise functions immediately!
// You can't run anything in the main node process (there is only 1!) AFTER promises resolve.
// Might as well ponder that 'til you get it.


const runChain = true;
const runBatch = true;
const runParallel = true;


// -----------------------
// Worker functions
const localeTime = () => new Date().toLocaleTimeString();
function getPrimes(max) {
    var sieve = [], i, j, primes = [];
    for (i = 2; i <= max; ++i) {
        if (!sieve[i]) {
            // i has not been marked -- it is prime
            primes.push(i);
            for (j = i << 1; j <= max; j += i) {
                sieve[j] = true;
            }
        }
    }
    return primes;
}
// -----------------------


// -----------------------
// Simple Promise functions
// These examples take no params and return results directly in resolve(...).
// -----------------------
async function fast() {
    return new Promise(resolve => {
        resolve( `${localeTime()} fast` );
    });
}
async function waitASec() {
    return new Promise(resolve => {
        const timeout = Math.random() * 1000;
        setTimeout(() => {
            resolve( `${localeTime()} waited ${(timeout/1000.0).toFixed(2)} secs`);
        }, timeout );
    });
}
async function waitFiveSecs() {
    return new Promise(resolve => {
        const timeout = 2000 + Math.random() * 3000;
        setTimeout(() => {
            resolve( `${localeTime()} waited ${(timeout/1000.0).toFixed(2)} secs`);
        }, timeout );
    });
}
// Even though these are called "slow", they are way faster than waits.  :-)
async function getSlowData() {
    return new Promise(resolve => {
        const oneHundredPrimeNumbers = getPrimes( 100 );
        resolve( `${localeTime()} ${oneHundredPrimeNumbers}` );
    });
}
async function getSlowerData() {
    return new Promise(resolve => {
        const twoHundredPrimeNumbers = getPrimes( 200 );
        resolve( `${localeTime()} ${twoHundredPrimeNumbers}` );
    });
}
// -----------------------


console.log(`${localeTime()} Script routine started...`);


// 1) CHAIN
const chainParent = async () => {
    try {
        console.log(`${localeTime()} Chain running...`);

        const c1 = await getSlowData();
        console.log( `${localeTime()} Chain first batch: ${c1}` );

        const w1 = await waitASec();
        console.log( `${localeTime()} Chain first wait: ${w1}` );

        const c2 = await getSlowerData();
        console.log( `${localeTime()} Chain second batch: ${c2}` );

        const w2 = await waitASec();
        console.log( `${localeTime()} Chain second wait: ${w2}` );

        const c3 = await getSlowData();
        console.log( `${localeTime()} Chain third batch: ${c3}` );

    } catch(e) {
        console.log( `${localeTime()} Chain error!` );
    }
}
if ( runChain )
    chainParent().then( () => {
        console.log(`${localeTime()} Chain done.`);

        // IMPORTANT NOTE regarding exiting this script without killing promises:
        // If you do not forcably exit, node scripts will wait for all promises to resolve, then exit.
        // That is best practice.
        //
        // If you really want to exit the script after a promise resolves:
        //      myp.then( process.exit );
        //      DO NOT USE myp.then(process.exit(0)); as that will be called immediately!
        //      then() needs a function!
        //
        // You can sleep for a while but not very useful unless you wrap some code inside...
        // const sleepSec = 5; setTimeout(() => {}, sleepSec * 1000);
        // const sleepMin = 1; setTimeout(() => {}, sleepMin * 60 * 1000);

        // WARNING: This will KILL any other unresolved promises that happen to be unfinished.
        // So Don't do it, it's pointless.  Instead, comment out the promises you don't want to run.
        // process.exit(0);
    });


// 2) PARALLEL
// You can fire a bunch of stuff off independently to get results as they resolve, eg:
// WaitFiveSecs is the slowest so it will return results last, even though it started first.
// I chained two of them together as well, just for fun.  Use the others as simpler guides.
if ( runParallel ) {
    waitFiveSecs().then( response1 => waitFiveSecs().then(response2 => console.log(`${localeTime()} Parallel task 1 complete ${response1} ${response2}`)));
    getSlowData().then(response => console.log(`${localeTime()} Parallel task 2 complete ${response}`));
    getSlowerData().then(response => console.log(`${localeTime()} Parallel task 3 complete ${response}`));
    getSlowData().then(response => console.log(`${localeTime()} Parallel task 4 complete ${response}`));
    waitASec().then(response => console.log(`${localeTime()} Parallel task 5 complete ${response}`));
    fast().then(response => console.log(`${localeTime()} Parallel task 6 complete ${response}`));
}


// 3) BATCH
// But when you batch with Promise.all(), you won't get any CONSOLE results until they all resolve.
// But they are actually running and resolving in parallel.  The details of this stuff are SO annoying.
const myBatch = async () => {

    // We are firing off all our jobs at once here!
    // Promise.all() takes a set of promises and makes a container Promise for them
    // that resolves when the sets all resolve, or rejects as soon as any in the set rejects.
    const batch = await Promise.all([ waitFiveSecs(), getSlowData(), getSlowerData(), getSlowData(), waitASec(), fast() ]);
    console.log('-----------------------------');
    console.log('vvvvvvvvvvvvvvvvvvvvvvvvvvvvv');
    console.log('Batch all jobs done.');
    console.log('NOTE: Results are returned in array order, not completion order.');
    console.log('See timestamps for completion times.');
    console.dir( batch );
    // or loop them if you want... same result...
    // for(const response of batch) {
    //     console.log(`  ${response}`);
    // }
    // for (let i = 0; i<batch.length; i++){
    //     console.log(batch[i]);
    // }
    console.log('^^^^^^^^^^^^^^^^^^^^^^^^^^^^^');
    console.log('-----------------------------');
};
if ( runBatch )
    myBatch();

// NOTE: YOU WILL NOT SEE THIS EARLY IN THE OUTPUT.
console.log(`${localeTime()} Script routine finished (Promises are now running).`);































// MORE NOTES if you want

// async/await keywords are the cleanest way as of 2022.
// ALL THE REST IS SEMANTIC BULLSHIT AND HISTORICAL CRUFT.

// -----------------------
// ASYNC/AWAIT (ES2017 aka ES8)
// async/await keywords are the cleanest 
// Define a function as async, then put one await keyword in it.
// When you call the async function, it returns a Promise.
// The async function runs until it hits the await function.
// The await function takes a Promise, and will only continue after the Promise resolves.

// Try to always do promises this way,
// until they fix await to be available "top-level".
// https://medium.com/@_bengarrison/javascript-es8-introducing-async-await-functions-7a471ec7de8a
// https://www.pluralsight.com/guides/handling-nested-promises-using-asyncawait-in-react
// https://stackoverflow.com/questions/14220321/how-to-return-the-response-from-an-asynchronous-call
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Using_promises
//
// THE BASICS
// async functions return a promise
// async functions return a promise as soon as the first await is hit inside their function body
//  (so to the caller an async function is still non-blocking 
//   and the caller must still deal with a returned promise
//   and get the result from that promise). 
// But, inside the async function, you can write more sequential-like code using await on promises.
// 
// Keep in mind that await only does something useful if you await a promise,
// so in order to use async/await, your asynchronous operations must all be promise-based.
//
// Promise.resolve() (and Promise.reject()) are shortcuts to manually create an already resolved promise.
// Useful to chain, eg: Promise.resolve().then(func1).then(func2);
//
// To avoid surprises, functions passed to then() will never be called synchronously, even with an already-resolved promise.
// So you can write sync code AFTER the Promise and it will ALWAYS run first.
// Promise functions are put on a "microtask queue" which always runs after the current JavaScript event loop finishes.
// But if they complete quickly, they'll execute before entering the next event loop.  Weird, but whatever.
//
// ANNOYINGLY!, you might have to use an "IIFE", meaning the defined function is immediately invoked.
// We have to do this just to provide an "async" wrapper, just so we can use "await".
// But might as well just create an actual funtion, then call it, in two steps.
// (async function() {
//     let chunk = await requestData();
//     console.dir( chunk );
// })();
//
//
// THE TAKEAWAY
// 
// Write all your helper functions to return a promise:
// 
//      async function getSlowerData() {
//          return new Promise( (resolve, reject) => {
//              if ( Math.random() > 0.9 ) {
//                  reject( 'You drew the short straw, honey!' );
//                  return;
//              }
//              // calculate prime numbers or whatever...
//              resolve( 'def' );
//          })
//      }
//
// Write your top-level async function to use awaits.  See Composition functions below.
// You still have to recognize that your top-level function will execute "quickly".
// 
// -----------------------

// Here we have several examples of using Promises to chain a sequence of events.
// The console output will likely be a mixture of logs from each of the big tasks,
// depending on the internal times and delays.
// But note that the subtasks of each of the major tasks are completed sequentially.
// Cool.



// -----------------------
// Manual composition
// Gives us complete control to manage the results from each step.
// -----------------------
const requestData_ManualCompose = async () => {
// async function requestData() {   // < equivalent
    try {
        await waitASec();

        console.log('Getting first batch...');
        const slow = await getSlowData();
        console.dir( slow );

        await waitASec();

        console.log('Getting bigger batch...');
        const slower = await getSlowerData();
        console.dir( slower );

    } catch(e) {
        console.log( 'Error' );
    }
}
// requestData_ManualCompose();
// -----------------------


// -----------------------
// Chained Promise functions
// Each of these receive the results from the previous function in the chain, cool.
// -----------------------
async function add10Primes( prevResult ) {
    return new Promise(resolve => {
        prevResult.ten = getPrimes( 10 );
        resolve( prevResult );
    });
}
async function add20Primes( prevResult ) {
    await waitASec(); // take this out and our big composition will likely finish first!  cast cpu be FAST...
    return new Promise(resolve => {
        prevResult.twenty = getPrimes( 20 );
        resolve( prevResult );
    });
}
async function add200Primes( prevResult ) {
    await waitASec();
    return new Promise(resolve => {
        prevResult.many = getPrimes( 200 );
        resolve( prevResult );
    });
}
async function bumpPrimes( prevResult ) {
    return new Promise(resolve => {
        // heavier math - but it takes NO time compared to the waits!
        prevResult.many = [...prevResult.many, ...getPrimes( 1000000 )];
        resolve( prevResult );
    });
}
// -----------------------


// -----------------------
// Sequential composition
// This chains results sequentially through each of the functions in a list.
// Nice!
// -----------------------
const requestData_SequentialComposition = async () => {
    let result = {};
    for (const f of [add10Primes, add20Primes, add200Primes, bumpPrimes, bumpPrimes, bumpPrimes]) {
        result = await f(result);
    }
    console.log( 'Sequential build of a buncha primes...');
    console.dir( result );
}
// Comment in to run.
// requestData_SequentialComposition();
// -----------------------


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
        setTimeout(() => resolve("We finished successfully, with input param1 = "+param1), 2000);

    })
}
 
// Comment in to run.
/*
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
    console.log("Our defensive coding caught an error: "+result)
})
*/

// -----------------------


// -----------------------
// ALL
// -----------------------
// We can start operations in parallel and wait for them all to finish like this:
//     Promise.all([func1(), func2(), func3()])
//     .then(([result1, result2, result3]) => { /* use result1, result2 and result3 */ });

// This example uses fetch() - to use that i'd have to make scrap a node module and I don't feel like it...
/*
let urls = [
    'https://api.github.com/users/iliakan',
    'https://api.github.com/users/remy',
    'https://api.github.com/users/jeresig'
];

Promise.all(urls.map(url => { fetch(url) }))
// for each response show its status
.then(responses => { // (*)
    for(let response of responses) {
        // console.log(`${response.url}: ${response.status}`);
        console.log(`${response}`);
    }
});
*/
// -----------------------


// -----------------------
// CHAINING
// -----------------------
// TODO
// -----------------------


