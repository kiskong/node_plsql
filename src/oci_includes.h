#ifndef OCI_INCLUDES__H
#define OCI_INCLUDES__H

#if (USE_ORACLE_DUMMY == 1)

// Load the fake Oracle Call Interface includes

# include "oracle_dummy/oci_dummy.h"

#else // (USE_ORACLE_DUMMY == 1)

// Load the real Oracle Call Interface includes

# include <oci.h>
# include <ociap.h>

#endif // (USE_ORACLE_DUMMY == 1)

#endif // OCI_INCLUDES__H
