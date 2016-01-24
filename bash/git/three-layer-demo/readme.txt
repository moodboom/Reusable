#there are only three possible states, handle them all automatically fucker
#    1) up-to-date, we last pushed from this location
#    2) out-of-date, in need of new changes from other locations
#    3) out-of-date, changes happened here AND upstream and you fucked up again
#    ---
#    use a master for rebased cleaned-up commits, and daily_grind for daily git-r-dun work
#        the base shared repo can keep master checked out to avoid git complaining about pushes
#        also, we can reset the daily_grind branch when we pull, in case it's been totally rebased
#   ---
#   script the h3ll out of this and put in checks to minimize fuckups, fuckup ;-)



