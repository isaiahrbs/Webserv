/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:34:17 by dinguyen          #+#    #+#             */
/*   Updated: 2026/04/06 10:50:09 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ASocket.hpp"
#include "SocketClient.hpp"

class	SocketServer : public ASocket {

private:
	int	_port;
	int	_maxUsers;

public:
	SocketServer(int port, const std::string& host, int maxUsers);
	virtual	~SocketServer();

	void		create();
	void		setNonBlocking();
	void		bindSocket();
	void		listenSocket();
	SocketClient*			acceptClient();

};
