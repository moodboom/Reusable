@ARGV = ('.') unless @ARGV;
my $linecount = 'wc -l `find $ARGV[0] |grep cpp$` `find $ARGV[0] |grep h$ |grep -v stl``;

# This will do it from the CygWin prompt:
# wc -l `find . |grep cpp$` `find . |grep h$ |grep -v stl`
