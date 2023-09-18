#ifndef FETCHHTML_H__
#define FETCHHTML_H__

#include <windows.h>
#include <wininet.h>
#include <string>
#include <iostream>
#include <psapi.h>

std::string FetchHtml(const std::string& url, std::string data = "", const std::string& method = "GET")
{
	// Using wininet
	//https://gist.github.com/gin1314/3434391

	HANDLE ih = InternetOpen("Mozilla/5.0 (compatible; MSIE 8.0; Windows NT 6.1; Trident/4.0; GTB7.4; InfoPath.2; SV1; .NET CLR 3.3.69573; WOW64; en-US)", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (ih == NULL)
	{
		printf("%s\n", "InternetOpen failed");
		return "";
	}

	// Tell wininet to decompress the data (gzip/deflate) if needed
	bool yes = true;
	InternetSetOption(ih, INTERNET_OPTION_HTTP_DECODING, &yes, sizeof(yes));

	char* error = "error visiting url.";
	char vhost[128] = { 0 };
	int vport;
	char vpath[256] = { 0 };

	// zero out urlComponents structure and set options
	URL_COMPONENTS urlComponents;
	memset(&urlComponents, 0, sizeof(urlComponents));
	urlComponents.dwStructSize = sizeof(urlComponents);
	urlComponents.dwHostNameLength = 1;
	urlComponents.dwUserNameLength = 1;
	urlComponents.dwPasswordLength = 1;
	urlComponents.dwUrlPathLength = 1;

	if (!InternetCrackUrl(url.c_str(), (DWORD)url.length(), 0, &urlComponents))
	{
		printf("%s\n", "invalid url");
		return "";
	}

	// copy url parts into variables
	vport = urlComponents.nPort;
	if (urlComponents.dwHostNameLength > 0)
		strncpy(vhost, urlComponents.lpszHostName, urlComponents.dwHostNameLength);
	if (urlComponents.dwUrlPathLength > 0)
		strncpy(vpath, urlComponents.lpszUrlPath, urlComponents.dwUrlPathLength);

	HINTERNET ch = InternetConnect(ih, vhost, vport, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
	if (ch == NULL) {
		printf("%s\n", "error InternetConnect");
		return "";
	}

	PCTSTR rgpszAcceptTypes[] = { "*/*", NULL };
	DWORD openFlags = 0;
	openFlags |= INTERNET_FLAG_NO_UI;
	openFlags |= INTERNET_FLAG_NO_AUTH;
	openFlags |= INTERNET_FLAG_NO_CACHE_WRITE;
	openFlags |= INTERNET_FLAG_PRAGMA_NOCACHE;
	openFlags |= INTERNET_FLAG_RELOAD;
	if (url.find("https:") == 0)
		openFlags |= INTERNET_FLAG_SECURE;

	HINTERNET req = HttpOpenRequest(ch, method.c_str(), vpath, NULL, NULL, rgpszAcceptTypes, openFlags, 0);
	if (req == NULL)
	{
		printf("%s\n", "error HttpOpenRequest");
		return "";
	}

	std::string header = "Content-Length: " + std::to_string(data.length());
	//header += "\nContent-Type: application/x-www-form-urlencoded";
	//std::cout << header << std::endl;
	//std::cout << data << std::endl;

	std::string result;
	if (HttpSendRequest(req, header.c_str(), header.length(), (void*)data.c_str(), data.length()))
	{
		//printf("%s\n", "success HttpSendRequest");

		char buffer[8192];
		DWORD len;
		while (InternetReadFile(req, &buffer, sizeof(buffer), &len) == TRUE)
		{
			if (len == 0)
				break;

			result += std::string(buffer, len);
		}
	}
	else
	{
		printf("%s\n", "error HttpSendRequest");
	}

	InternetCloseHandle(ch);
	InternetCloseHandle(req);

	return result;
}

#endif