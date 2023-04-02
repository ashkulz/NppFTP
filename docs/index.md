---
layout: default
---

## Download

You can install it via the "Plugin Manager". In case the latest version is not available via the Plugin Manager, you can download it from [GitHub](https://github.com/ashkulz/NppFTP/releases/latest) and [install it manually](http://docs.notepad-plus-plus.org/index.php?title=Plugin_Development#How_to_install_a_plugin).

## Support

In case you need any help, please search the Github [issue tracker](https://github.com/ashkulz/NppFTP/issues) before creating a [new issue](https://github.com/ashkulz/NppFTP/issues/new).

## Usage

To start using the plugin, use the **Show NppFTP Window** option from the plugins menu, or use the Notepad++ toolbar button. To find some information about the plugin, use the **About NppFTP** option from the menu. There is a button there for a link to the NppFTP site.

## Configuring
There are two configuration dialogs for NppFTP. These can be accessed by clicking on the settings button in the NppFTP toolbar (cog icon).

### General configuration
In the general configuration dialog, the default cache location can be entered. See [Cache paths] for more details. It will map to the root directory on the server (`/`) and if no other cache locations are provided by a profile, this will always be the target.

### Profile configuration
In the profiles configuration dialog, profiles can be created, modified and deleted. Initially, no profiles exists and no connection can be made. To create a new profile, click the Add profile button and enter the name of the new profile. Please provide an unique name for your own ease of use. Renaming and deleting a profile is done with the corresponding buttons.

In the connections tab, settings for each connection can be entered. At minimum provide a hostname (address) and port. In the authentication tab a private keyfile could be provided. The expected format is an OpenSSH Key. In the transfers tab, settings for FTP transfers can be edited. In the cache tab, specific cache mappings can be added for the selected profile. See [Cache paths] for more details. The inputfield "Groupname:" under the "FTP Misc." Tab can be used to group Profiles under Submenus inside the Connection Menu. The entry is the submenutext, all Profiles with the same Groupname are below this submenu.

## Cache paths
When downloading files from a server, they are by default stored in the cache. When a file in the cache is saved, it will automatically be uploaded. To allow more fine grained control over what files go to where, a cache mapping can be created. A cache map consists of a local directory and an external path. The local directory provides the location on the local computer to look for files to upload and to download to. For example, if `C:\ftpfiles\myserver\home` were entered, files in that directory and subdirectory would be transferred to the corresponding path on the external server. The external path provides the location to download files from and upload to. For example, `/home/myuser/public_html/` would map files on that path and its subpaths to the corresponding directory. Determining a cache map for a file-transfer is done on a first match basis (rather than 'best fit'). For example, consider the following scenario:

**Profile cache maps**:
```
Local                        External
C:\webfiles                  /home/user/public_html
C:\webfiles                  /home/user2/public_html
C:\rootfiles                 /root
D:\serverfilesystem          /
```

**General cache map**:
```
C:\myuser@server.com\        / (fixed)
```

**Downloads**:

* The external file `/home/user/public_html/index.html` would be transferred to `C:\webfiles\index.html`
* The external file `/home/user/.bash_rc` would be transferred to `D:\serverfilesystem\home\user\.bash_rc`
* The external file `/root/apache.conf` would be transferred to `C:\rootfiles\apache.conf`
* The external file `/vmlinuz.img` would be transferred to `D:\serverfilesystem\vmlinuz.img`

No download would be directed to `C:\myuser@server.com\`

**Uploads**:

* The local file `C:\webfiles\home\user\.bash_rc` would be transferred to `/home/user/public_html/home/user/.bash_rc` (user2 will NOT be considered)
* The local file `D:\serverfilesystem\boot\grub\menu.lst` would be transferred to `/boot/grub/menu.lst`
* The local file `C:\myuser@server.com\home\user\public_html\index.html` would be transferred to `/home/user/public_html/index.html`

Ordering is important. The general cache map will always be considered last, the profile maps will be considered from top to bottom. So if
```
D:\serverfilesystem          /
```
were to be at the top, all files would be downloaded to `D:\serverfilesystem`

## Toolbar

The toolbar provides the following buttons:

* _(Dis)Connect_: Either connect to a server from a profile form a dropdown menu, or disconnect from the current server.
* _Open Directory_: Navigation aid to quickly show the contents of an external directory. The full external path must be input, e.g. `/home/user/public_html`
* _Download file_: If a file is selected in the treeview, download it to the cache.
* _Upload file_: If a directory is selected in the treeview, upload the current file to that directory.
* _Refresh_: If a directory is selected in the treeview, refresh its contents.
* _Abort_: If a transfer is active, abort it.
* _Settings_: Access settings dialogs.
* _Show Message Window_: Hide or Show the messages window.

## Treeview
If an FTP session is active, the treeview will show the files on the server. Some actions of the toolbar depend on the selected object in the treeview (see toolbar). Double-clicking on a directory will show its contents. Double-clicking on a file will download it to the cache and open it.

## Queue

The queue window shows the currently active and queued file transfers, along with their progress and file path. Right-clicking on an item allows to abort or cancel it, depending whether the transfer is active or queued.

## Message window
The message window shows the output of various operations. If something goes wrong, look for errors here. Notifications are blue, server messages are green, errors are red.
