#!/bin/bash

export OCI_HOME=/instantclient_12_1
export OCI_LIB_DIR=$OCI_HOME
export OCI_INCLUDE_DIR=$OCI_HOME/sdk/include
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OCI_HOME
