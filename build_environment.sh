export OCI_HOME=/instantclient_12_1
export OCI_LIB_DIR=$OCI_HOME
export OCI_INCLUDE_DIR=$OCI_HOME/sdk/include

echo "  OCI_HOME: $OCI_HOME"
echo "  OCI_LIB_DIR: $OCI_LIB_DIR"
echo "  OCI_INCLUDE_DIR: $OCI_INCLUDE_DIR"

sudo -E node-gyp
