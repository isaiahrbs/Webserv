/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SocketClient.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dinguyen <dinguyen@student.42lausanne.c    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/03 11:33:58 by dinguyen          #+#    #+#             */
/*   Updated: 2025/12/03 12:35:08 by dinguyen         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma	once

#include "ASocket.hpp"

class	SocketClient : public ASocket {

public:
	SocketClient(int fd, struct sockaddr_in addr);
	virtual	~SocketClient();

	void		create();
	void		setNonBlocking();
	ssize_t		sendData(const void* buf, size_t len);
	ssize_t		recvData(void* buf, size_t len);
	bool		isConnected() const;

};
