#include "Common.h"
#include "Remote.h"

/*
#include <sys/socket.h> // socket, bind, listen...
#include <arpa/inet.h>	// sockaddr_in
int serverSock;
struct sockaddr_in serverAddr;

XprBool xprAppInitialize()
{
	// remember to add <uses-permission android:name="android.permission.INTERNET"></uses-permission>
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		xprDbgStr("Failed to open server socket");
	}
	
	memset(&serverAddr, 0, sizeof(serverAddr));		// Clear struct
	serverAddr.sin_family = AF_INET;					// Internet/IP
	serverAddr.sin_addr.s_addr = INADDR_ANY;			// Incoming addr
	serverAddr.sin_port = htons(8080);					// server port
	
	if (bind(serverSock, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		xprDbgStr("Failed to bind the server socket");
	}
    
	// Listen on the server socket
	if (listen(serverSock, SOMAXCONN) < 0) {
		xprDbgStr("Failed to listen on server socket");
	}
	
	xprRenderTargetSetViewport(0, 0, (float)xprAppContext.xres, (float)xprAppContext.yres, -1, 1);
	
	xprDbgStr("server started");

	return XprTrue;
}

void xprAppFinalize()
{
	close(serverSock);
}

#define BUFFSIZE 512

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
	int sock;
	struct sockaddr_in clientAddr;
	
	unsigned int clientlen = sizeof(clientAddr);
	// Wait for client connection
	if ((sock = accept(serverSock, (struct sockaddr *) &clientAddr, &clientlen)) < 0) {
		//Die("Failed to accept client connection");
		return;
	}
	
	{
		char buffer[BUFFSIZE];
		int received = -1;
		// Receive message
		if ((received = recv(sock, buffer, BUFFSIZE, 0)) < 0) {
			xprDbgStr("Failed to receive initial bytes from client");
		}
		xprDbgStr("received %d %s", received, buffer);
		// Send bytes and check for more incoming data in loop
		while (received > 0) {
		  // Send back received data
		  if (send(sock, buffer, received, 0) != received) {
			xprDbgStr("Failed to send bytes to client");
		  }
		  xprDbgStr("reply sent");
		}
		close(sock);
	}
}

void xprAppHandleMouse(int x, int y, int action)
{
}

void xprAppRender()
{
	xprRenderTargetClearColor(0, 1, 0, 1.0f);
}

void xprAppConfig()
{
	xprAppContext.appName = "Embedded Httpd";
	xprAppContext.xres = 480;
	xprAppContext.yres = 320;
	xprAppContext.multiSampling = XprFalse;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 2;
	xprAppContext.apiMinorVer = 1;
}
*/

RemoteConfig* _config = nullptr;
float bgR = 255;
float bgG = 127;
float bgB = 0;

void xprAppUpdate(unsigned int elapsedMilliseconds)
{
	//remoteConfigProcessRequest(_config);
}

void xprAppHandleMouse(int x, int y, int action)
{
}

void xprAppRender()
{
	float r, g, b;
	
	remoteConfigLock(_config);
	r = bgR / 255; g = bgG / 255; b = bgB / 255;
	remoteConfigUnlock(_config);

	xprRenderTargetClearColor(r, g, b, 1.0f);
}

void xprAppConfig()
{
	xprAppContext.appName = "Embedded Httpd";
	xprAppContext.xres = 480;
	xprAppContext.yres = 320;
	xprAppContext.multiSampling = XprFalse;
	xprAppContext.vsync = XprFalse;
	xprAppContext.apiMajorVer = 2;
	xprAppContext.apiMinorVer = 1;
}

void xprAppFinalize()
{
	remoteConfigFree(_config);
}

XprBool xprAppInitialize()
{
	RemoteVarDesc descs[] = {
		{"bgR", &bgR, 0, 255},
		{"bgG", &bgG, 0, 255},
		{"bgB", &bgB, 0, 255},
		{nullptr, nullptr, 0, 0}
	};

	xprRenderTargetSetViewport(0, 0, (float)xprAppContext.xres, (float)xprAppContext.yres, -1, 1);

	_config = remoteConfigAlloc();
	remoteConfigInit(_config, 8080, XprTrue);
	remoteConfigAddVars(_config, descs);

	return XprTrue;
}