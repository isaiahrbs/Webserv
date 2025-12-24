/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: irobinso <irobinso@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:34:17 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/24 12:31:43 by irobinso         ###   ########.fr       */
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
	SocketServer(int port, int maxUsers);
	virtual	~SocketServer();

	void		create();
	void		setNonBlocking();
	void		bindSocket();
	void		listenSocket();
	SocketClient*			acceptClient(); // a implementer

	bool		isListening() const;
	void		setReuseAddr();
	int			getPort() const;
	int			getMaxUsers() const;


};
