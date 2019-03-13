#include <cstdio>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <string>
#include <iostream>
#include <sstream>
#include <array>
#include <iomanip>

size_t write_data(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::ostringstream *stream = (std::ostringstream*)userdata;
    size_t count = size * nmemb;
    stream->write(ptr, count);
    return count;
}

int main(void)
{
	std::cout << "Enter password:" << std::endl;
	std::string password;
	std::cin >> password;
	std::array<unsigned char, 20> hash_base;

	SHA1(
		reinterpret_cast<const unsigned char *>(password.c_str()),
		password.length(),
		hash_base.data()
	);

	std::stringstream hash_stream;
	hash_stream << std::hex << std::uppercase << std::setfill('0');
	for(auto i = 0; i < 20; ++i) {
		hash_stream << std::setw(2) << static_cast<unsigned>(hash_base[i]);
	}
	const auto hash = hash_stream.str();

	std::cout << password << ": " << hash << std::endl;
	CURL *curl;
	CURLcode res;

	curl = curl_easy_init();
	if(curl) {
		/* First set the URL that is about to receive our POST. This URL can
		just as well be a https:// URL if that is what should receive the
		data. */
		auto url = std::string("https://api.pwnedpasswords.com/range/") +  hash.substr(0, 5);
// 		std::cout << url << std::endl;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		std::stringstream stream;

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &stream);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);

		if(res != 0) {
			std::cerr << "Could not connect to api.pwnedpasswords.com" << std::endl;
			curl_easy_cleanup(curl);
			return 1;
		}

		std::string line;
		bool pwned = false;
		while(std::getline(stream, line)) {
// 			std::cout << line.substr(0,35) <<" \n" << hash.substr(5) << std::endl;
			if(line.substr(0,35) == hash.substr(5)) {
// 				std::cout << line << std::endl;
				const auto times = std::stoi(line.substr(36));
				std::cout << "You have been pwned " << times << " times." << std::endl;
				pwned = true;
			}
		}
		if(!pwned) {
			std::cout << "You have not been pwned." << std::endl;
		}
		/* always cleanup */
		curl_easy_cleanup(curl);
	}
	return 0;
}
