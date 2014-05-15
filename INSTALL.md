# Installation instructions

You first need to download and install the [Oracle instant client](http://www.oracle.com/technetwork/database/features/instant-client/index-097480.html) with the following components:

1. Instant Client Package - Basic or Basic Lite: All files required to run OCI, OCCI, and JDBC-OCI applications
2. Instant Client Package - SDK: Additional header files and an example makefile for developing Oracle applications with Instant Client

**It is reccomended to use the latest version (12.1) of the Oracle instant client**
**Please make sure you download the correct packages for your system architecture, such as 64 bit vs 32 bit**
**Unzip the files 1 and 2 into the same directory, such as /opt/instantclient_12_1 or c:\instantclient_12_1_x64**

1. Set up the following environment variables

MacOS/Linux:

```bash
export OCI_LIB_DIR=<directory of Oracle instant client>
export OCI_INCLUDE_DIR=<directory of Oracle instant client>/sdk/include
export NLS_LANG=AMERICAN_AMERICA.UTF8
```

Windows:

```bat
set OCI_LIB_DIR=<directory of Oracle instant client>\sdk\lib\msvc
set OCI_INCLUDE_DIR=<directory of Oracle instant client>\sdk\include
export NLS_LANG=AMERICAN_AMERICA.UTF8
PATH=...;<directory of Oracle instant client>
```

2. Create the following symbolic links

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

* On Unbuntu/Debian

```
sudo apt-get install libaio1
```

* On Fedora/CentOS/RHEL

```
sudo yum install libaio
```

3. Configure the dynamic library path

MacOS:

```
export DYLD_LIBRARY_PATH=$OCI_LIB_DIR
```

Linux:

Add the shared object files to the ld cache:

```
# Replace /opt/instantclient_12_1/ with wherever you extracted the Basic Lite files to
echo '/opt/instantclient_12_1/' | sudo tee -a /etc/ld.so.conf.d/oracle_instant_client.conf
sudo ldconfig
```
