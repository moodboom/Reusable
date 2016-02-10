#!/usr/bin/env node

// async is mo betta!
// var execSync = require('child_process').execSync;
var exec = require('child_process').exec;
var shared = require('./shared.js');
var logger = require('./shared.js').justLogResponse;

var cmd = 'git-sync';

// cd to our base, then to each subdir
shared.cdscripts();         exec(cmd, logger);
process.chdir("../config"); exec(cmd, logger);

