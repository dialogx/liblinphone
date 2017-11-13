/*
 * chat-room-id.h
 * Copyright (C) 2010-2017 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _CHAT_ROOM_ID_H_
#define _CHAT_ROOM_ID_H_

#include "address/simple-address.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class ChatRoomIdPrivate;

class LINPHONE_PUBLIC ChatRoomId : public ClonableObject {
public:
	ChatRoomId (const SimpleAddress &peerAddress, const SimpleAddress &localAddress);
	ChatRoomId (const ChatRoomId &src);

	ChatRoomId &operator= (const ChatRoomId &src);

	bool operator== (const ChatRoomId &chatRoomId) const;
	bool operator!= (const ChatRoomId &chatRoomId) const;

	const SimpleAddress &getPeerAddress () const;
	const SimpleAddress &getLocalAddress () const;

private:
	L_DECLARE_PRIVATE(ChatRoomId);
};

LINPHONE_END_NAMESPACE

#endif // ifndef _CHAT_ROOM_ID_H_
