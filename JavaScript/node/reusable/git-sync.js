#!/usr/bin/env node

var exec = require('child_process').exec;
// var shared = require('./shared.js');

// Command line params.
// 0 = node, 1 = script path, so we ignore those.
var args = process.argv.slice(2);
var comment = "";
if (args.length > 0)
{
    args.forEach(function(val,index,array){ 
        if (index == 0) 
        { comment += " -m \""; } 
        else
        { comment += " "; } 
        comment += val; 
    });
    comment += "\"";
}

// MDM NOTE: NEVER prompt/pause/sleep within "sub"-scripts run by parent scripts.

// Run these in order.  That means nest the calls, w00t!
var cmd1 = 'git commit -a' + comment;
var cmd2 = 'git pull';
var cmd3 = 'git push';

exec(cmd1, function(error, stdout, stderr) {

    // Sometimes stupid emacs fails with "emacs: standard input is not a tty".
    // I can't BELIEVE I used that as my primary editor for years!!  Think globally, EDIT LOCALLY.
    if (stderr) {

        console.log(stdout);
        console.log(stderr);

    } else {

        exec(cmd2, function(error, stdout, stderr) {
            if (error) {
                console.log(stdout);
                console.log(stderr);
                console.log('==================== GIT PULL FAILED ====================\n');
            } else {
                exec(cmd3, function(error, stdout, stderr) {
                    if (error) {
                        console.log(stdout);
                        console.log(stderr);
                        console.log('==================== GIT PUSH FAILED ====================\n');
                    } else
                    {
                        // MDM Eventually remove this chatter.
                        // console.log(stdout);
                        // console.log(stderr);
                        // console.log('==================== SYNC COMPLETE ====================\n');
                        // console.log("<=> * [" + process.cwd() + "] " + cmd1 + " && " + cmd2 + " && " + cmd3);

                        console.log("<=> " + process.cwd());
                    }
                });
            }
        });
    }
});
