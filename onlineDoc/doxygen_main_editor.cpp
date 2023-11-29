// doxygen_main_editor.cpp
//    by rrivera at fnal dot gov
//	  created May 2018
//
// This is a simple html code injector to improve the main.html generated by doxygen.
//
// Planned to be used in conjunction with OnlineDocPushUpdate.sh
//
// compile with:
// g++ doxygen_main_editor.cpp -o hw.o
//
// if developing, consider appending -D_GLIBCXX_DEBUG to get more
// descriptive error messages
//
// run with:
//./doxygen_main_editor.o <full main.html path> <full inject main html file path>
//

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iomanip>
#include <iostream>


// use this for normal printouts
#define __PRINTF__ printf

#define __SHORTFILE__ 		(__builtin_strstr(&__FILE__[0], "/srcs/") ? __builtin_strstr(&__FILE__[0], "/srcs/") + 6 : __FILE__)
#define __COUT_HDR_L__ 		"[" << std::dec        << __LINE__ << " |\t"
#define __COUT_HDR_FL__ 	__SHORTFILE__ << " "   << __COUT_HDR_L__

#define __COUT_TYPE__(X) 	std::cout << QUOTE(X) << ":" << __MF_SUBJECT__ << ":"
#define __COUT_ERR__ 		__COUT_TYPE__(LogError) << __COUT_HDR_FL__
#define __COUT_WARN__ 		__COUT_TYPE__(LogWarning) << __COUT_HDR_FL__
#define __COUT_INFO__ 		__COUT_TYPE__(LogInfo) << __COUT_HDR_FL__
#define __COUT__ 			__COUT_TYPE__(LogDebug) << __COUT_HDR_FL__

#define __SS__            	std::stringstream ss; ss << "|" << __MF_DECOR__ << ": " << __COUT_HDR_FL__
#define __SS_THROW__        { __COUT_ERR__ << "\n" << ss.str(); throw std::runtime_error(ss.str()); } //put in {}'s to prevent surprises, e.g. if ... else __SS_THROW__;
#define __E__ 				std::endl
#define Q(X) #X
#define QUOTE(X) Q(X)
#define __COUTV__(X) 		__COUT__ << QUOTE(X) << " = " << X << __E__

// and use this to suppress
// #define __PRINTF__ if(0) printf
// #define __COUT__  if(0) std::cout

int main(int argc, char** argv)
{
	__COUT__ << "Starting doxygen main.html editor..." << __E__;

	if(argc < 4)
	{
		__COUT__ << "Need 3 arguments: for the full path to main.html AND to "
		            "ARRAY:<html-to-inject>"
		         << __E__;
		return 0;
	}
	std::string mainfn    = argv[1];
	std::string injectfn  = argv[2];
	std::string inject2fn = argv[3];
	__COUT__ << "main.html destination full path: " << mainfn << __E__;
	__COUT__ << "main.html source full path: " << mainfn + ".bk" << __E__;
	__COUT__ << "inject.html source full path: " << injectfn << __E__;
	__COUT__ << "inject2.html source full path: " << inject2fn << __E__;

	FILE* mainSrc = fopen((mainfn + ".bk").c_str(), "r");
	if(!mainSrc)
	{
		__COUT__ << "Failed to open... " << mainfn + ".bk" << __E__;
		return 0;
	}
	FILE* injectSrc = fopen((injectfn).c_str(), "r");
	if(!injectSrc)
	{
		__COUT__ << "Failed to open... " << injectfn << __E__;
		return 0;
	}
	FILE* inject2Src = fopen((inject2fn).c_str(), "r");
	if(!inject2Src)
	{
		__COUT__ << "Failed to open... " << inject2fn << __E__;
		return 0;
	}
	FILE* mainDest = fopen((mainfn).c_str(), "w");
	if(!mainSrc)
	{
		__COUT__ << "Failed to open... " << mainfn << __E__;
		return 0;
	}

	char         line[1000];
	unsigned int countdown = -1;

	unsigned int injectIndex = 0;

	bool injected = true;
	while(fgets(line, 1000, mainSrc))
	{
		fputs(line, mainDest);  // output main line to file
		__COUT__ << line << (line[strlen(line) - 1] == '\n' ? "" : "\n");

		if(injected && !strcmp(line, "<div class=\"contents\">\n"))
		{
			injected    = false;
			countdown   = 0;
			injectIndex = 1;
			// continue;
		}
		else if(injected && !strcmp(line, "<head>\n"))
		{
			injected    = false;
			countdown   = 0;
			injectIndex = 2;
			// continue;
		}

		if(!injected && countdown == 0)  // inject file
		{
			injected = true;

			switch(injectIndex)
			{
			case 1: {
				// get one more line and modify it, ie, clip ugly start and delete close
				// tag for content div 				fgets(line,1000,mainSrc);
				//				__COUT__ << "MOD " << line << (line[strlen(line)-1] ==
				//'\n'?"":"\n"); 				line[strlen(line)-7] = '\0';
				//				for(countdown=strlen(line)-16;countdown<strlen(line);++countdown)
				//					if(line[countdown]=='v') break;
				//
				//				//keep version number
				//				fputs("<h3>",mainDest);
				//				__COUT__ << "<h3>" << __E__;
				//				fputs(&line[countdown],mainDest); //output main line to
				// file
				//				__COUT__ << &line[countdown] << __E__;

				while(fgets(line, 1000, injectSrc))
				{
					fputs(line, mainDest);  // output inject line to file
					__COUT__ << line << (line[strlen(line) - 1] == '\n' ? "" : "\n");
				}

				// add close content div
				//				fputs("</div>",mainDest);
				//				__COUT__ << "</div>" << __E__;

				break;
			}
			case 2: {
				while(fgets(line, 1000, inject2Src))
				{
					fputs(line, mainDest);  // output inject line to file
					__COUT__ << line << (line[strlen(line) - 1] == '\n' ? "" : "\n");
				}
				break;
			}
			default:
				__COUT__ << "Unknown injection!" << __E__;
			}
		}
		else if(!injected)
		{
			--countdown;
		}
	}

	fclose(mainDest);
	fclose(injectSrc);
	fclose(mainSrc);

	__COUT__ << "Doxygen main.html editor complete!" << __E__;

	return 0;
}
