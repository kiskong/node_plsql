# Installation instructions

## Prerequisites (before installing)

You first need to download and install the [Oracle instant client](http://www.oracle.com/technetwork/database/features/instant-client/index-097480.html) with the following components:

* Instant Client Package - Basic or Basic Lite: All files required to run OCI, OCCI, and JDBC-OCI applications
* Instant Client Package - SDK: Additional header files and an example makefile for developing Oracle applications with Instant Client

### Notes

* It is reccomended to use the latest version (12.1) of the Oracle instant client.
* Please make sure you download the correct packages for your system architecture, such as 64 bit vs 32 bit.
* Unzip the files 1 and 2 into the same directory, such as /opt/instantclient_12_1 or c:\instantclient_12_1_x64.

### Workarounds

* The Oracle Bug 17481818 (GETTING A CONNECTION FROM OCI CONNECTION POOL IS SLOW ON WINDOWS PLATFORMS) affects Oracle Clients 11.2.0.3, 11.2.0.4 and 12.1.0.1 and
drammaticaly slows down the connection to the Database when more than one naming method is used in NAMES.DIRECTORY_PATH in the SQLNET.ORA configuration file.
The reccomended workaround is to only use the TNSNAMES adapter in NAMES.DIRECTORY_PATH in sqlnet.ora.
NAMES.DIRECTORY_PATH=(TNSNAMES)

### Set up the following environment variables

Windows:
```bat
set OCI_LIB_DIR=<directory of Oracle instant client>\sdk\lib\msvc
set OCI_INCLUDE_DIR=<directory of Oracle instant client>\sdk\include
export NLS_LANG=AMERICAN_AMERICA.UTF8
PATH=...;<directory of Oracle instant client>
```

MacOS/Linux:
```bash
export OCI_LIB_DIR=<directory of Oracle instant client>
export OCI_INCLUDE_DIR=<directory of Oracle instant client>/sdk/include
export NLS_LANG=AMERICAN_AMERICA.UTF8
```

### Create the following symbolic links

MacOS:
```
cd <directory of Oracle instant client>
ln -s libclntsh.dylib.12.1 libclntsh.dylib
```

Linux:
```
cd <directory of Oracle instant client>
ln -s libclntsh.so.12.1 libclntsh.so
```

`libaio` library is required on Linux systems:

On Unbuntu/Debian
```
sudo apt-get install libaio1
```

On Fedora/CentOS/RHEL
```
sudo yum install libaio
```

### Configure the dynamic library path

MacOS:
```
export DYLD_LIBRARY_PATH=$OCI_LIB_DIR
```

Linux:
```
export LD_LIBRARY_PATH=$OCI_LIB_DIR
```

## Install the node_plsql server locally

Create and move to a new directory and then install the node_plsql module.

```
npm install node_plsql
```

### Install the PL/SQL sample application

Install the PL/SQL examples using SQL*Plus and running `install.sql` in the `./node_modules/node_plsql/sql` directory while connected as a user with administrative privileges.

### Configure the node_plsql server

Create a sample configuration file using the --init command line parameter

MacOS/Linux:
```bash
./node_modules/.bin/node_plsql --init=sample.json
```

Windows:
```bat
node_modules\.bin\node_plsql --init=sample.json
```

and then change the configuration parameters as needed.

### Start the node_plsql server

MacOS/Linux:
```bash
./node_modules/.bin/node_plsql --config=sample.json
```

Windows:
```bat
./node_modules/.bin/node_plsql --config=sample.json
```

## Install the node_plsql server globally

```
npm install node_plsql --global
```

### Identify the directory where the global packages are installed

Use `npm list --global` to see if and where the node_plsql module has been installed.

The typical directories are:

MacOS/Linux: `/usr/local` or wherever node is installed

Windows: `%APPDATA%\npm`

### Install the PL/SQL sample application

Install the PL/SQL examples using SQL*Plus and running `install.sql` in the `./node_modules/node_plsql/sql` directory while connected as a user with administrative privileges.

### Configure the node_plsql server

Create a sample configuration file using the --init command line parameter

MacOS/Linux:
```bash
node_plsql --init=sample.json
```

Windows:
```bat
node_plsql --init=sample.json
```

and then change the configuration parameters as needed.

### Start the node_plsql server

MacOS/Linux:
```bash
node_plsql --config=sample.json
```

Windows:
```bat
node_plsql --config=sample.json
```

## Connect to the node_plsql server

Open a browser and use the following [URL](http://localhost:8999/sample/sample.pageIndex).
