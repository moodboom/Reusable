#!/usr/bin/env node

//    USAGE:
//
//      cd [setup.js folder]
//      node setup.js
//
//    From then on, all commands defined in package.json [bin] will be available from any path.

// Install with [npm link] while in development.
// That all we need to use the "binaries" (defined below) anywhere on the local machine.
// Later, if it is useful, we can actually publish with [npm publish ...].
//
// NOTE that under Windows, this takes a dump in [C:\Users\Michael\AppData\Roaming\npm\].

var exec = require('child_process').exec;

var cmd = 'npm link';
exec(cmd, function(error, stdout, stderr) {
    if (error) {
        console.log('==================== Local install (npm link) failed ====================\n');
        console.log(stdout);
        console.log(stderr);
        throw error;
    }

    console.log('-------------------- Local install (npm link) succeeded --------------------\n');
    console.log(stdout);
});
