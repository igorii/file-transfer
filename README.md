File Share
==========

A simple file sharing client/server application over TCP for COMP 3203 Computer Networks, Carleton University.

# Build

    make

# Usage

Once a server has been started, clients may connect to it to send and receive files one at a time.

### Server

    $ ./bin/server *port*

### Client

    $ ./bin/client *hostname* *port*  # Eg. ./bin/client localhost 5000
    file-share(hostname): <enter command and hit enter>

## Currently supported commands

* ls
    * list all items in the current directory of the server
* get *filename*
    * create a local copy of the remote file
* put *filename*
    * create a remote copy of the local file
* rename *filename*   # This will prompt for a new name
