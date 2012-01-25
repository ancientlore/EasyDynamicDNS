EasyDynamicDNS
==============

EasyDynamicDNS is a dynamic DNS client for [easyDNS](https://web.easydns.com/). The client follows the protocol [specified by easyDNS](http://support.easydns.com/tutorials/dynamicUpdateSpecs.php).

EasyDynamicDNS is currently in beta.

Download
--------

The EasyDynamicDNS beta is packaged in a zip file. Unzip it and put it anywhere you like; it has no dependencies.

[Download EasyDynamicDNS](https://github.com/downloads/ancientlore/EasyDynamicDNS/EasyDynamicDNS.zip)

Documentation
-------------

EasyDynamicDNS is a Windows service that you can install from the command-line. After unzipping the executable, put it where you like and run the follow steps to set it up:

* Install service: `EasyDynamicDNS.exe /install`
* Set the interval that it will update easyDNS: `EasyDynamicDNS.exe /config timeout 60`
* Configure your domain: `EasyDynamicDNS.exe /config prompt`
* Check your settings: `EasyDynamicDNS.exe /config`
* Start the service: `EasyDynamicDNS.exe /start`

After the installation the service will run each time Windows starts. easyDNS will reject any timeout interval less than 10 minutes; in practice setting this to 6 hours is even ok in most cases if your address does not change that often.

In order to maintain the greatest flexibility, the program does not heavily validate the options you enter for your domain. However, the instructions to attempt to show what values you should enter. The most important ones are:

* Host Name – This is the name of the domain you want to update, like www.yourdomain.com
* My IP Address – Leave this blank to determine from your pc; or more commonly, set to 1.1.1.1 to let easyDNS detect it (works better if you have a firewall).
* Username – Your easyDNS username
* Password – Your easyDNS password (which is stored encrypted). Since EasyDynamicDNS does not support SSL, you should get a token from easyDNS to use (see their control panel).

The other options can just be blank for most users. [dyndns.com](http://www.dyndns.com/) users should read their [documentation](http://www.dyndns.com/developers/specs/syntax.html) for setup information. It’s slightly different from easyDNS in some important ways!

To remove EasyDynamicDNS, simply run `EasyDynamicDNS.exe /uninstall` and delete the executable.

EasyDynamicDNS does not currently support HTTPS.
