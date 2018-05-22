#include <otsdaq-utilities/ECLWriter/ECLConnection.h>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream>
#include <cstring>
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutMacros.h"
#include <boost/algorithm/string.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>

ECLConnection::ECLConnection(std::string user,
							 std::string pwd,
							 std::string url)
{
	_user = user;
	_pwd = pwd;
	_url = url;

	srand(time(NULL));

}

size_t ECLConnection::WriteMemoryCallback(char *data, size_t size,
										  size_t nmemb, std::string* buffer)
{
	size_t realsize = 0;

	if (buffer != NULL) {
		buffer->append(data, size*nmemb);
		realsize = size * nmemb;
	}

	return realsize;
}

bool ECLConnection::Get(std::string s, std::string& response)
{
	response = "NULL";

	char errorBuffer[CURL_ERROR_SIZE];
	std::string buffer;
	CURL *curl_handle;

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */
	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);

	std::string fullURL = _url + s;

	/* specify URL to get */
	curl_easy_setopt(curl_handle, CURLOPT_URL, fullURL.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);

	/* send all data to this function  */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, ECLConnection::WriteMemoryCallback);

	/* we pass our 'chunk' struct to the callback function */
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &buffer);

	/* some servers don't like requests that are made without a user-agent
   field, so we provide one */
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	/* get it! */
	CURLcode result = curl_easy_perform(curl_handle);

	/* cleanup curl stuff */
	curl_easy_cleanup(curl_handle);

	if (result == CURLE_OK)
		response = buffer;
	else {
		std::cerr << "Error: [" << result << "] - " << errorBuffer << std::endl;
		return false;
	}

	curl_global_cleanup();

	return true;
}

bool ECLConnection::Search(std::string s)
{
	return false;
}

std::string ECLConnection::MakeSaltString()
{
	std::string rndString = "";

	std::string chars("abcdefghijklmnopqrstuvwxyz"
					  //			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
					  "1234567890");
	for (int i = 0; i < 10; ++i) {
		rndString += chars[rand() % chars.size()];
	}

	return rndString;
}


bool ECLConnection::Post(ECLEntry_t& e)
{
	std::string safe_url;
	if (!Get("/secureURL", safe_url)) return false;

	std::string rndString = MakeSaltString();

	std::string myURL = "/E/xml_post?";
	std::string mySalt = "salt=" + rndString;
	std::string fullURL = _url + myURL + mySalt;

	std::string myData = mySalt + ":" + _pwd + ":";

	// create text from xml form, but need to remove all \n's
    std::ostringstream oss;
    entry(oss, e);
	std::string eclString = oss.str();
	__COUT__ << "ECL XML is: " << eclString << std::endl;
	//std::string eclString = e.entry();
	eclString = eclString.substr(eclString.find_first_of(">") + 2);

	while(eclString.find('\n') != std::string::npos)
	  {
	    eclString = eclString.erase(eclString.find('\n'), 1);
	  }
	while(eclString.find('\r') != std::string::npos)
	  {
	    eclString = eclString.erase(eclString.find('\r'), 1);
	  }
	while(eclString.find(" <") != std::string::npos)
	  {
	    eclString = eclString.erase(eclString.find(" <"), 1); 
	  }
        boost::trim(eclString);
	myData += eclString;
 __COUT__ << "ECL Hash string is: " << myData << std::endl;
	unsigned char resultMD5[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)myData.c_str(), myData.size(), resultMD5);

	std::string xSig;
	char buf[3];
	for (auto i=0;i<MD5_DIGEST_LENGTH;i++){
	  sprintf(buf, "%02x", resultMD5[i]);
	  xSig.append( buf );
	}
	__COUT__ << "ECL MD5 Signature is: " << xSig << std::endl;


	typedef boost::archive::iterators::insert_linebreaks<
  boost::archive::iterators::base64_from_binary<
  boost::archive::iterators::transform_width<unsigned char *,6,8> >, 72 > it_base64_t;

  // Encode
  unsigned int writePaddChars = (3-MD5_DIGEST_LENGTH%3)%3;
  std::string base64Sig(it_base64_t(resultMD5),it_base64_t(resultMD5 + MD5_DIGEST_LENGTH));
  base64Sig.append(writePaddChars,'=');
	__COUT__ << "ECL base64 Signature is: " << base64Sig << std::endl;

	CURL *curl_handle;
	char errorBuffer[CURL_ERROR_SIZE];

	curl_global_init(CURL_GLOBAL_ALL);

	/* init the curl session */

	curl_handle = curl_easy_init();

	curl_easy_setopt(curl_handle, CURLOPT_ERRORBUFFER, errorBuffer);

	/* specify URL to get */

	struct curl_slist* headers = NULL;
	std::string buff = "X-User: " + _user;
	headers = curl_slist_append(headers, buff.c_str());
	//	buff = "X-Password: " + _pwd;
	//headers = curl_slist_append(headers,buff.c_str());
	headers = curl_slist_append(headers, "Content-type: text/xml");
	headers = curl_slist_append(headers, "X-Signature-Method: md5");
	//buff = "X-Signature: " + base64Sig;
	buff = "X-Signature: " + xSig;
	headers = curl_slist_append(headers, buff.c_str());

	const char* estr = eclString.c_str();

	__COUT__ << "ECL Setting message headers" << std::endl;
	curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, estr);
	curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl_handle, CURLOPT_URL, fullURL.c_str());
	//      curl_easy_setopt(curl_handle, CURLOPT_VERBOSE,1);

	// post it!

	__COUT__ << "ECL Posting message" << std::endl;
	CURLcode result = curl_easy_perform(curl_handle);

	if (result != CURLE_OK) {
		std::cerr << "Error: [" << result << "] - " << errorBuffer << std::endl;
		return false;
	}

	__COUT__ << "ECL Cleanup" << std::endl;
	// cleanup curl stuff
	curl_easy_cleanup(curl_handle);
	curl_slist_free_all(headers);

	curl_global_cleanup();

	return true;

}

