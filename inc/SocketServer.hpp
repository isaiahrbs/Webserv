/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketServer.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:34:17 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/03 12:32:55 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ASocket.hpp"

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
	int			acceptClient();

	bool		isListening() const;
	void		setReuseAddr();
	int			getPort() const;
	int			getMaxUsers() const;


};
