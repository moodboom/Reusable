#!/usr/bin/env node

var shared = require('./shared.js');
console.log(shared.last_changed_rev());


// This was moved to shared.
/*
var run = require('./shared.js').run_command_sync;

var svn_info = run("svn", ["info"]);

// extract the "Last Changed Rev"
var regx = /^Last Changed Rev: (.*)$/gm;
var array_result = regx.exec(svn_info);

// Get the first group result ([0] contains the whole result)
console.log(array_result[1]);
*/