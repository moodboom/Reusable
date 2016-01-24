#!/usr/bin/env node

// TODO even better, use this thingamathing:
// https://www.npmjs.com/package/minimist

var args = process.argv.slice(2);
console.log('arguments: [' + args.join(',') + ']');
