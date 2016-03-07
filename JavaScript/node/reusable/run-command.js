#!/usr/bin/env node

// TODO debug this, it doesn't work yet

// Usage: run-command ls -la --color
var run_cmd = require('./shared.js').run_command_sync;
var rawargs = process.argv.slice(2);
var cmd = rawargs[0];
var args = rawargs.slice(1);
console.log(run_cmd(cmd,args));
