// config.h - adapted for raidtcl project 2018 by Rainer Müller 

// directories are different for Windows and Linux
#ifdef WIN32
	/* directory for application resources */
	#define RESOURCE_DIR "../../data"
	
	/* directory for the translations of the program */
	#define TRANSLATIONSDIR ""	

	/* directory html package documentation in html */
	#define HTML_DOC_DIR "../../doc"
#else
	/* directory for application resources */
	#define RESOURCE_DIR "/usr/local/share/dtcltiny"

	/* directory for the translations of the program */
	#define TRANSLATIONSDIR "/usr/local/share/dtcltiny/translations"

	/* directory html package documentation in html */
	#define HTML_DOC_DIR "/usr/local/share/doc/dtcltiny"
#endif

/* Name of package */
#define PACKAGE "dtcltiny"

/* Version number of package */
#define VERSION "0.4.4"
