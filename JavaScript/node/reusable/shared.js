#!/usr/bin/env node

var fs = require('fs');

// =========== cdscripts ============
exports.cdscripts = function () {
    // NOTE that this is the way to get "home", cross-platform, if it is ever needed.
    // var homedir = (process.platform === 'win32') ? process.env.HOMEPATH : process.env.HOME;

    // we check the list in order, first match wins
    var candidates = 
    [
        'D:/Michael\'s Data/development/scripts',   // WARNING: ALWAYS escape the single quote (even if in double quotes) or node barfs!  FUCKSAKE
        '/home/m/development/scripts',
    ];

    for (var i = 0;i < candidates.length;i++) {

        // SYNCHRONOUS change wd
        // ignore failures until we find one that works
        try {
            process.chdir(candidates[i]);
            return process.cwd();
        }
        catch (err) {
        }
    }
};


// =========== cdfirst: change to first found folder ============
exports.cdfirst = function (candidates) {

    // TODO

};


// =========== run_cmd: run one command and get output ============
// Sync version, this is the most useful
// Usage:
// var run = require('./shared.js').run_command_sync;
// var lsout = run( "ls", ["-l"]);
exports.run_command_sync = function (cmd, args ) {
    var ss = require('child_process').spawnSync;
    var outp = ss(cmd, args, { encoding : 'utf8' });

    // DEBUG
    // console.log(outp.stdout);

    return outp.stdout;

    // From here:
    //  http://stackoverflow.com/questions/32393250/nodejs-child-process-spawnsync-or-child-process-spawn-wrapped-in-yieldable-gener
    // More example cde:
    // var ls = cp.spawnSync('ls', ['-l', '/usr'], { encoding : 'utf8' });
    // uncomment the following if you want to see everything returned by the spawnSync command
    // console.log('ls: ' , ls);
    // console.log('stdout here: \n' + ls.stdout);
}


// sync version; you have to provide a callback function
// Usage:
// run_cmd( "ls", ["-l"], function(text) { console.log (text) });
exports.run_command = function (cmd, args, callBack ) {

    var spawn = require('child_process').spawn;
    var child = spawn(cmd, args);
    var resp = "";

    child.stdout.on('data', function (buffer) { resp += buffer.toString() });
    child.stdout.on('end', function() { callBack (resp) });
}


// =========== justLogResponse ============
// This function is used to just log a command response,
// swallowing any errors, assuming this is part of a process that should carry on.
// It expects that the standard output of the command includes any necessary carriage returns; ie, none are added.
exports.justLogResponse = function (error, stdout, stderr) { 
    if (error) { 
        console.log(stdout);
        console.log(stderr);
        console.log("=== SEE ERROR ABOVE ===");
    } else { 
        // We don't use console.log() here because it adds \n.
        process.stdout.write(stdout);
    } 
}    


// =========== svn_last_changed_rev: gets the SVN "last changed rev" for the current folder, as a string ============
exports.svn_last_changed_rev = function () {

    var run = exports.run_command_sync;

    var svn_info = run("svn", ["info"]);

    // extract the "Last Changed Rev"
    var regx = /^Last Changed Rev: (.*)$/gm;
    var array_result = regx.exec(svn_info);

    // return the first group result ([0] contains the whole result)
    return array_result[1];
}


// =========== svn_rev: gets the SVN current revision for the current repo, as a string ============
exports.svn_rev = function () {

    var run = exports.run_command_sync;

    var svn_info = run("svn", ["info"]);

    // extract the "Last Changed Rev"
    var regx = /^Revision: (.*)$/gm;
    var array_result = regx.exec(svn_info);

    // return the first group result ([0] contains the whole result)
    return array_result[1];
}


// =========== runsteps: run a specific set of commands in specific directories ============
exports.runsteps = function (candidates,steps,verbosity) {

    // TODO, example follows...
    
    var exec = require('child_process').exec;
    var execSync = require('child_process').execSync;
    var fs = require('fs');
    var shared = require('./shared.js');

    shared.cdbase();
    var logfile = fs.createWriteStream('log.txt');

    var $msbuild = 'C:/Windows/Microsoft.NET/Framework/v4.0.30319/MSBuild.exe';
    var $msbuild2008 = 'C:/Windows/Microsoft.NET/Framework/v3.5/MSBuild.exe';
    var $mstest = '\"D:/Program Files (x86)/Microsoft Visual Studio 10.0/Common7/IDE/MSTest.exe\"';

    var steps = 
    [
        /* 
        // DB is coming...

        {   name: 'svn DB Power Flow'   , folder: 'DesignBase/Trunk/Power Flow'                            , cmd: 'svn up'                                                                                      },
        // may not be needeed...
        {   name: 'svn DB Short Circuit', folder: 'DesignBase/Trunk/Short Circuit'                         , cmd: 'svn up'                                                                                      },
        {   name: 'svn DB Pascal'       , folder: 'DesignBase/Trunk/Pascal'                                , cmd: 'svn up'                                                                                      },

        {   name: 'svn DB Editor'       , folder: 'DesignBase/Trunk/Editor'                                , cmd: 'svn up'                                                                                      },
        {   name: 'svn DB Build Devices', folder: 'DesignBase/Trunk/Build Devices'                         , cmd: 'svn up'                                                                                      },
        {   name: 'svn DB Shared'       , folder: 'DesignBase/Trunk/Shared'                                , cmd: 'svn up'                                                                                      },

        // Includes: PDC, edsaevent, edsaannot, ArcHeat, Transient
        {   name: 'svn DB Drawing'      , folder: 'DesignBase/Trunk/Drawing'                               , cmd: 'svn up'                                                                                      },
        */

        {   name: 'svn EPAG'            , folder: 'EPAG/Trunk'                , cmd: 'svn up'                                                                                                                   },
        {   name: 'pre EPAG'            , folder: '.'                         , cmd: 'EPAG\\Trunk\\PreBuildStepGW.bat'                                                                                          },
        {   name: 'build EPAG'          , folder: 'EPAG/Trunk'                , cmd: $msbuild + ' EPAG.sln /property:Configuration=TeamCity'                                                                    },
        {   name: 'build D4CRDriver61'  , folder: 'EPAG/Trunk/D4CRDriver61'   , cmd: $msbuild + ' D4CRDriver61.sln /property:Configuration=TeamCity'                                                            },
        
        // MDM I don't have VS2008 installed at the moment, skip the build of this component...
        // Make sure the three binaries are available here: C:\CODE\EPAG\Trunk\D4CRDriver52\TeamCity
        // {   name: 'build D4CRDriver52'  , folder: 'EPAG/Trunk/D4CRDriver52'   , cmd: $msbuild2008 + ' D4CRDriver52.sln /property:Configuration=TeamCity'    },

        {   name: 'test clean'          , folder: 'EPAG/Trunk'                , cmd: 'if exist TestResults\ rmdir /S /Q TestResults'                        },
        {   name: 'test EPAG framework' , folder: 'EPAG/Trunk'                , cmd: $mstest + ' ' + '"/testcontainer:C:\\CODE\\EPAG\\Trunk\\Testing\\FrameworkTests\\bin\\Release\\FrameworkTests.dll"'        },
        {   name: 'test EPAG persist'   , folder: 'EPAG/Trunk'                , cmd: $mstest + ' ' + '"/testcontainer:C:\\CODE\\EPAG\\Trunk\\Testing\\PersistenceTests\\bin\\Release\\PersistenceTests.dll"'    },
        {   name: 'test EPAG wsm'       , folder: 'EPAG/Trunk'                , cmd: $mstest + ' ' + '"/testcontainer:C:\\CODE\\EPAG\\Trunk\\Testing\\WSMTest\\bin\\Release\\WSMTest.dll"'                      },
        {   name: 'post EPAG'           , folder: '.'                         , cmd: 'EPAG\\Trunk\\PostBuildStepGW.bat'                                                                                         },

        {   name: 'svn DesignView'      , folder: 'DesignView/Trunk'          , cmd: 'svn up'                                                                                                                   },
        {   name: 'build DesignView'    , folder: 'DesignView/Trunk'          , cmd: $msbuild + ' PaladinWebClient.sln /property:Configuration=Release'                                                         },

        {   name: 'svn Paladin Reports' , folder: 'Paladin Reports/Trunk'     , cmd: 'svn up'                                                                                                                   },
        {   name: 'build PR'            , folder: 'Paladin Reports/Trunk'     , cmd: $msbuild + ' Paladin_Reports.sln /property:Configuration=Release'                                                          },

        {   name: 'svn PL'              , folder: 'PaladinLiveInstaller/Trunk'                           , cmd: 'svn up'                                                                                        },
        {   name: 'svn PL DB components', folder: 'PaladinLiveInstaller/Trunk/DesignBaseComponents'      , cmd: 'svn up'                                                                                        },
        {   name: 'svn PL DV help'      , folder: 'PaladinLiveInstaller/Trunk/DesignViewHtmlHelp'        , cmd: 'svn up'                                                                                        },
        {   name: 'svn PL docs'         , folder: 'PaladinLiveInstaller/Trunk/PaladinLiveDocumentation'  , cmd: 'svn up'                                                                                        },
        {   name: 'pre PL'              , folder: '.'                           , cmd: 'PaladinLiveInstaller\\Trunk\\PreBuildStepPI.bat'                                                                        },
        {   name: 'build PL'            , folder: 'PaladinLiveInstaller/Trunk'  , cmd: $msbuild + ' PaladinLiveInstaller.sln /property:Configuration=Release'                                                   },
        {   name: 'post PL'             , folder: '.'                           , cmd: 'PaladinLiveInstaller\\Trunk\\PostBuildStepPI.bat'                                                                       },

        {   name: 'test PL'             , folder: '.'                           , cmd: 'ContinuousIntegration\\Trunk\\scripts\\windows\\BuildMachine\\StartCI_local.bat'                                        },
    ];

    for (var i = 0;i < steps.length;i++) {

        // SYNCHRONOUS change wd
        // ignore failures until we find one that works
        try {
            console.log('step: ' + steps[i].name);

            shared.cdbase();
            process.chdir(steps[i].folder);
            // console.log(process.cwd());

            // VERBOSE: this shows output as commands execute...
            execSync(steps[i].cmd, {stdio:[0,1,2]}, function(error, stdout, stderr) {
            
            // QUIET: minimal output until an error is hit...
            // execSync(steps[i].cmd, function(error, stdout, stderr) {

                logfile.write('======' + steps[i].name + '======');

                if (error) {
                    logfile.write(stdout);
                    logfile.write(stderr);
                    logfile.write('==================== CI ERROR ====================\n');
                    
                    console.log('======= CI ERROR =======\n');
                    console.log('See log.txt for details.');

                    throw error;
                }

                // Log the details.  No need to bother the user.
                logfile.write(stdout);

            });

        }
        catch (err) {
            // console.log(err);
        }
    }
}