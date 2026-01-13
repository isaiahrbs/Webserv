/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketClient.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irobinso <irobinso@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:33:58 by dinguyen          #+#    #+#             */
/*   Updated: 2026/01/13 07:40:42 by irobinso         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "ASocket.hpp"

class	SocketClient : public ASocket {
private:
	std::string _requestBuffer;// pour stocker petit à petit la requete
	std::string _responseBuffer;// pour renvoyer au client une reponse chuck par chunk

public:
	SocketClient(int fd, struct sockaddr_in addr);
	virtual	~SocketClient();

	void		create();
	void		setNonBlocking();
	ssize_t		sendData(const void* buf, size_t len);
	ssize_t		recvData(void* buf, size_t len);
	bool		isConnected() const;

	std::string& getRequestBuffer();
	std::string& getResponseBuffer();

};
