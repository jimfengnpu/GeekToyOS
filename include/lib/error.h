#pragma once
enum {
	// Kernel error codes -- keep in sync with list in lib/printfmt.c.
	E_UNSPECIFIED,      // Unspecified or unknown problem
	E_BAD_ENV,	        // Environment doesn't exist or otherwise
				        // cannot be used in requested action
	E_INVAL,	        // Invalid parameter
	E_NO_MEM,	        // Request failed due to memory shortage
	E_NO_FREE_ENV,	    // Attempt to create a new environment beyond
				        // the maximum allowed
	E_FAULT,	        // Memory fault
	E_IPC_NOT_RECV,	    // Attempt to send to env that is not recving
	E_EOF,	            // Unexpected end of file

	MAXERROR
};

extern const char * const error_string[MAXERROR];