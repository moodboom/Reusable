downloaded_from_etrade_2015_03_19
I had to fix a typo in one include, just a capitalization problem that probably didn't show up in their Windoze testing
all works great as of 2015_07_17 (man it took a while to get up and running)
i added it to my Reusable repo

============
Instructions
============
cd ~/development/Reusable/php/etrade_api/Samples/

# save off orig config, and set up a private one with a symlink so it doesn't end up in the public repo
cp config.php config_example.php
mv config.php config_private_dont_share.php
ln -s config_private_dont_share.php config.php
emacs config.php # Put dev keys in config.php (DO NOT SHARE THEM)

# get a user-specific access token
php test_etOAuth.php
# Once you authenticate with a PIN, paste the keys into config.php
# Then you can run the rest of the php samples!

# to (eg) preview an order
emacs test_Orders.php    # paste your account into the variable in this file
php test_Orders.php

# etc
