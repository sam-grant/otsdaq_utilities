#include <otsdaq-utilities/ECLWriter/ECLConnection.h>
#include <iomanip>
#include <openssl/md5.h>
#include <sstream>
#include <cstring>
#include "otsdaq-core/MessageFacility/MessageFacility.h"
#include "otsdaq-core/Macros/CoutMacros.h"


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
	oss << e;
	std::string eclString = oss.str();
	__COUT__ << "ECL XML is: " << eclString << std::endl;
	//std::string eclString = e.entry();
	eclString = eclString.substr(eclString.find_first_of(">") + 2);

	unsigned char resultMD5[MD5_DIGEST_LENGTH];
	MD5((unsigned char*)eclString.c_str(), eclString.size(), resultMD5);

	std::ostringstream sout;
	sout << std::hex << std::setfill('0');
	for (long long c : resultMD5)
	{
		sout << std::setw(2) << (long long)c;
	}
	auto xSig = sout.str();

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
	headers = curl_slist_append(headers, "Content-type: text/xml");
	headers = curl_slist_append(headers, "X-Signature-Method: md5");
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

