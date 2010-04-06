// $Id: Socket.h 6272 2010-04-05 13:30:49Z FloSoft $
//
// Copyright (c) 2005 - 2010 Settlers Freaks (sf-team at siedler25.org)
//
// This file is part of Siedler II.5 RTTR.
//
// Siedler II.5 RTTR is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Siedler II.5 RTTR is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Siedler II.5 RTTR. If not, see <http://www.gnu.org/licenses/>.
#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#pragma once

#ifdef _WIN32
	#include <ws2tcpip.h>
	typedef int socklen_t;
#else
	#include <arpa/inet.h>
	#include <sys/types.h>
       	#include <sys/socket.h>
       	#include <netdb.h>

	#define SOCKET int
	#define INVALID_SOCKET -1
	#define SOCKET_ERROR -1
	#define HINSTANCE void*

	#define closesocket close
#endif // !_WIN32

///Socket-Wrapper-Klasse f�r portable TCP/IP-Verbindungen
class Socket
{
private:
	enum STATUS
	{
		INVALID = INVALID_SOCKET,
		VALID = 0,
		LISTEN,
		CONNECT
	} status;

public:
	/// Standardkonstruktor von @p Socket.
	Socket(void);

	/// Konstruktor von @p Socket.
	Socket(const SOCKET so, STATUS st);

	/// Setzt ein Socket auf �bergebene Werte.
	void Set(const SOCKET so, STATUS st);

	/// Initialisiert die Socket-Bibliothek.
	static bool Initialize(void);

	/// r�umt die Socket-Bibliothek auf.
	static void Shutdown(void);

	/// erstellt und initialisiert das Socket.
	bool Create(int family = AF_INET);

	/// schliesst das Socket.
	void Close(void);

	/// setzt das Socket auf Listen.
	bool Listen(unsigned short port, bool use_ipv6);

	/// akzeptiert eingehende Verbindungsversuche.
	bool Accept(Socket& client);

	enum PROXY_TYPE {
		PROXY_NONE = 0,
		PROXY_SOCKS4 = 4,
		PROXY_SOCKS5 = 5
	};

	/// versucht eine Verbindung mit einem externen Host aufzubauen.
	bool Connect(const std::string &hostname, const unsigned short port, bool use_ipv6, const PROXY_TYPE typ = PROXY_NONE, const std::string proxy_hostname = "", const unsigned int proxy_port = 0);

	/// liest Daten vom Socket in einen Puffer.
	int Recv(void *buffer, int length, bool block = true);

	/// schreibt Daten von einem Puffer auf das Socket.
	int Send(const void *buffer, const int length);

	/// setzt eine Socketoption.
	bool SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel = IPPROTO_TCP);

	/// Zuweisungsoperator.
	Socket &operator =(const Socket &sock);

	/// Zuweisungsoperator.
	Socket &operator =(const SOCKET &sock);

	/// Gr��er-Vergleichsoperator.
	bool operator >(const Socket &sock);

	/// pr�ft auf wartende Bytes.
	int BytesWaiting(void);

	/// pr�ft auf wartende Bytes.
	int BytesWaiting(unsigned int *dwReceived);

	/// liefert die IP des Remote-Hosts.
	std::string GetPeerIP(void);

	/// liefert die IP des Lokalen-Hosts.
	std::string GetSockIP(void);
	
	/// liefert einen Zeiger auf das Socket.
	SOCKET* GetSocket(void);

	void Sleep(unsigned int ms);

	/// liefert Ip-Adresse(n) f�r einen Hostnamen.
	struct HostAddr
	{
		HostAddr() : host(""), port("0"), addr(NULL), ipv6(false), lookup(true) { }

		// copy
		HostAddr(const HostAddr &ha) : host(ha.host), port(ha.port), addr(NULL), ipv6(ha.ipv6), lookup(ha.lookup)
		{
			UpdateAddr();
		}

		~HostAddr()
		{
			if(lookup)
				freeaddrinfo(addr);
			else
			{
				free(addr->ai_addr);
				delete addr;
				addr = NULL;
			}
		}

		// set
		void UpdateAddr()
		{
			freeaddrinfo(addr);

			// do not use addr resolution for localhost
			lookup = (host != "localhost");

			if(lookup)
			{
				addrinfo hints;
				memset(&hints, 0, sizeof(addrinfo));
				hints.ai_flags = AI_NUMERICHOST;
				hints.ai_socktype = SOCK_STREAM;

				if(ipv6)
					hints.ai_family = AF_INET6;
				else
					hints.ai_family = AF_INET;

				getaddrinfo(host.c_str(), port.c_str(), &hints, &addr);
			}
			else // fill with loopback
			{
				addr = new addrinfo;
				addr->ai_family = (ipv6 ? AF_INET6 : AF_INET);
				addr->ai_addrlen = (ipv6 ? sizeof(sockaddr_in6) : sizeof(sockaddr_in));
				addr->ai_addr = (sockaddr *)calloc(1, addr->ai_addrlen);

				if(ipv6)
				{
					sockaddr_in6 *addr6 = (sockaddr_in6*)addr->ai_addr;
					addr6->sin6_family = AF_INET6;
					addr6->sin6_port = htons(atoi(port.c_str()));
					addr6->sin6_addr = in6addr_loopback;
				}
				else
				{
					sockaddr_in *addr4 = (sockaddr_in*)addr->ai_addr;
					addr4->sin_family = AF_INET;
					addr4->sin_port = htons(atoi(port.c_str()));
					addr4->sin_addr.s_addr = inet_addr("127.0.0.1");
				}
			}
		}

		std::string host;
		std::string port;
		addrinfo *addr;
		bool ipv6;
		bool lookup;
	};
	std::vector<HostAddr> HostToIp(const std::string &hostname, const unsigned int port, bool get_ipv6);

	/// liefert einen string der �bergebenen Ip.
	std::string &IpToString(const sockaddr *addr, std::string &buffer);

	/// liefert den Status des Sockets.
	bool isValid(void) { return (status != INVALID); }

private:
	SOCKET	sock; ///< Unser Socket
};

#endif // SOCKET_H_INCLUDED
