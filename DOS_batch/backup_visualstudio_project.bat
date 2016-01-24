@echo off

rem ===================
rem CONFIGURABLE PARAMS
rem ===================
rem set	projdrive=e:
set	projdrive=c:
set	projbase=\michae~1\softwa~1
set 	projname=cccont~1
set   	backupdir=e:\backup
rem ===================

rem DEBUG - "rem" next line to use...
goto NODEBUG
set	projdrive=c:
set	projbase=\michae~1\softwa~1\cccont~1
set 	projname=latest
set   	backupdir=f:\
:NODEBUG

%projdrive%
if exist %projbase%\%projname%\*.* goto ProjFound
echo.
echo ***  No files found ***  
echo for Project %projbase%\%projname%...
echo.
echo press Ctrl-C to abort, or if you want to continue anyway,
pause
goto go
:ProjFound
echo.
echo Using %projdrive%\%projbase%\%projname%\*.*...
echo.
echo.

rem Project open?
if not exist "%projbase%\%projname%\%projname%.aps" goto go
del "%projbase%\%projname%\%projname%.aps" >nul
if exist "%projbase%\%projname%\%projname%.aps" goto project_open

:go

cd %projbase%\%projname%

rem DEBUG
echo From %projdrive%%projbase%\%projname% To %backupdir%\%projname%.zip
rem pause

rem MDM Moving from winzip to 7zip...
rem
rem For winzip (old):
rem -u Update, -whs Include hidden/system, -ybc Answer "yes", -r -p Include path
rem Note that you will still get an error msg if no files need updating.  Silly, eh?
rem wzzip -u -r -p -ybc -whs -bC:\ %backupdir%\%projname%.zip -i@%backupdir%\Files.txt
rem
rem For 7z:
rem Usage: 7z <command> [<switches>...] <archive_name> [<file_names>...]
rem        [<@listfiles...>]
rem 
rem <Commands>
rem   a: Add files to archive
rem   d: Delete files from archive
rem   e: Extract files from archive (without using directory names)
rem   l: List contents of archive
rem   t: Test integrity of archive
rem   u: Update files to archive
rem   x: eXtract files with full paths
rem <Switches>
rem   -ai[r[-|0]]{@listfile|!wildcard}: Include archives
rem   -ax[r[-|0]]{@listfile|!wildcard}: eXclude archives
rem   -bd: Disable percentage indicator
rem   -i[r[-|0]]{@listfile|!wildcard}: Include filenames
rem   -m{Parameters}: set compression Method
rem   -o{Directory}: set Output directory
rem   -p{Password}: set Password
rem   -r[-|0]: Recurse subdirectories
rem   -sfx[{name}]: Create SFX archive
rem   -si[{name}]: read data from stdin
rem   -so: write data to stdout
rem   -t{Type}: Set type of archive
rem   -v{Size}[b|k|m|g]: Create volumes
rem   -u[-][p#][q#][r#][x#][y#][z#][!newArchiveName]: Update options
rem   -w[{path}]: assign Work directory. Empty path means a temporary directory
rem   -x[r[-|0]]]{@listfile|!wildcard}: eXclude filenames
rem   -y: assume Yes on all queries
7z a -r -y -tzip %backupdir%\%projname%.zip @%backupdir%\backup_files.txt
goto end

:project_open
echo Project appears open...
pause

:end

rem If needed, copy the backup or whatever...
rem 7z a -r -y -tzip %backupdir%\%projname%.zip L:\shared\development\backup\*

rem exit
