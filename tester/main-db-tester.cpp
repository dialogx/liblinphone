/*
 * Copyright (c) 2010-2022 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone
 * (see https://gitlab.linphone.org/BC/public/liblinphone).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "address/address.h"
#include "c-wrapper/internal/c-tools.h"
#include "core/core-p.h"
#include "db/main-db.h"
#include "event-log/events.h"
// TODO: Remove me.
#include "liblinphone_tester.h"
#include "private.h"
#include "tools/tester.h"

#ifndef _WIN32
#include <sys/resource.h>
#include <sys/time.h>
#endif

// =============================================================================

using namespace std;

using namespace LinphonePrivate;

// -----------------------------------------------------------------------------

class MainDbProvider {
public:
	MainDbProvider() : MainDbProvider("db/linphone.db") {
	}

	MainDbProvider(const char *db_file) {
		mCoreManager = linphone_core_manager_create("empty_rc");
		char *roDbPath = bc_tester_res(db_file);
		char *rwDbPath = bc_tester_file(core_db);
		BC_ASSERT_FALSE(liblinphone_tester_copy_file(roDbPath, rwDbPath));
		linphone_config_set_string(linphone_core_get_config(mCoreManager->lc), "storage", "uri", rwDbPath);
		bc_free(roDbPath);
		bc_free(rwDbPath);
		linphone_core_manager_start(mCoreManager, false);
		BC_ASSERT_TRUE(getMainDb().isInitialized());
	}

	void reStart(bool check_for_proxies = TRUE) {
		linphone_core_manager_reinit(mCoreManager);
		char *rwDbPath = bc_tester_file(core_db);
		linphone_config_set_string(linphone_core_get_config(mCoreManager->lc), "storage", "uri", rwDbPath);
		bc_free(rwDbPath);
		linphone_core_manager_start(mCoreManager, check_for_proxies);
	}

	~MainDbProvider() {
		linphone_core_manager_destroy(mCoreManager);
	}

	MainDb &getMainDb() {
		return *L_GET_PRIVATE(mCoreManager->lc->cppPtr)->mainDb;
	}

private:
	LinphoneCoreManager *mCoreManager;
	const char *core_db = "linphone.db";
};

// -----------------------------------------------------------------------------

static void get_events_count(void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		BC_ASSERT_EQUAL(mainDb.getEventCount(), 5175, int, "%d");
		BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceCallFilter), 0, int, "%d");
		BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceInfoFilter), 18, int, "%d");
		BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::ConferenceChatMessageFilter), 5157, int, "%d");
		BC_ASSERT_EQUAL(mainDb.getEventCount(MainDb::NoFilter), 5175, int, "%d");
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void get_messages_count(void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		BC_ASSERT_EQUAL(mainDb.getChatMessageCount(), 5157, int, "%d");
		BC_ASSERT_EQUAL(
		    mainDb.getChatMessageCount(ConferenceId(Address::create("sip:test-3@sip.linphone.org")->getSharedFromThis(),
		                                            Address::create("sip:test-1@sip.linphone.org"))),
		    861, int, "%d");
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void get_unread_messages_count(void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		BC_ASSERT_EQUAL(mainDb.getUnreadChatMessageCount(), 2, int, "%d");
		BC_ASSERT_EQUAL(mainDb.getUnreadChatMessageCount(
		                    ConferenceId(Address::create("sip:test-3@sip.linphone.org")->getSharedFromThis(),
		                                 Address::create("sip:test-1@sip.linphone.org"))),
		                0, int, "%d");
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void get_history(void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		BC_ASSERT_EQUAL(
		    (int)mainDb
		        .getHistoryRange(ConferenceId(Address::create("sip:test-4@sip.linphone.org")->getSharedFromThis(),
		                                      Address::create("sip:test-1@sip.linphone.org")),
		                         0, -1, MainDb::Filter::ConferenceChatMessageFilter)
		        .size(),
		    54, int, "%d");
		BC_ASSERT_EQUAL(
		    (int)mainDb
		        .getHistoryRange(ConferenceId(Address::create("sip:test-7@sip.linphone.org")->getSharedFromThis(),
		                                      Address::create("sip:test-7@sip.linphone.org")),
		                         0, -1, MainDb::Filter::ConferenceCallFilter)
		        .size(),
		    0, int, "%d");
		BC_ASSERT_EQUAL(
		    (int)mainDb
		        .getHistoryRange(ConferenceId(Address::create("sip:test-1@sip.linphone.org")->getSharedFromThis(),
		                                      Address::create("sip:test-1@sip.linphone.org")),
		                         0, -1, MainDb::Filter::ConferenceChatMessageFilter)
		        .size(),
		    804, int, "%d");
		BC_ASSERT_EQUAL(
		    (int)mainDb
		        .getHistory(ConferenceId(Address::create("sip:test-1@sip.linphone.org")->getSharedFromThis(),
		                                 Address::create("sip:test-1@sip.linphone.org")),
		                    100, MainDb::Filter::ConferenceChatMessageFilter)
		        .size(),
		    100, int, "%d");
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void get_conference_notified_events(void) {
	MainDbProvider provider;
	const MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		list<shared_ptr<EventLog>> events = mainDb.getConferenceNotifiedEvents(
		    ConferenceId(Address::create("sip:test-44@sip.linphone.org")->getSharedFromThis(),
		                 Address::create("sip:test-1@sip.linphone.org")),
		    1);
		BC_ASSERT_EQUAL((int)events.size(), 3, int, "%d");
		if (events.size() != 3) return;

		shared_ptr<EventLog> event;
		auto it = events.cbegin();

		event = *it;
		if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantRemoved)) return;
		{
			shared_ptr<ConferenceParticipantEvent> participantEvent =
			    static_pointer_cast<ConferenceParticipantEvent>(event);
			BC_ASSERT_TRUE(participantEvent->getConferenceId().getPeerAddress()->toString() ==
			               "sip:test-44@sip.linphone.org");
			BC_ASSERT_TRUE(participantEvent->getParticipantAddress()->toString() == "sip:test-11@sip.linphone.org");
			BC_ASSERT_TRUE(participantEvent->getNotifyId() == 2);
		}

		event = *++it;
		if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantDeviceAdded)) return;
		{
			shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent =
			    static_pointer_cast<ConferenceParticipantDeviceEvent>(event);
			BC_ASSERT_TRUE(deviceEvent->getConferenceId().getPeerAddress()->toString() ==
			               "sip:test-44@sip.linphone.org");
			BC_ASSERT_TRUE(deviceEvent->getParticipantAddress()->toString() == "sip:test-11@sip.linphone.org");
			BC_ASSERT_TRUE(deviceEvent->getNotifyId() == 3);
			BC_ASSERT_TRUE(deviceEvent->getDeviceAddress()->toString() == "sip:test-47@sip.linphone.org");
		}

		event = *++it;
		if (!BC_ASSERT_TRUE(event->getType() == EventLog::Type::ConferenceParticipantDeviceRemoved)) return;
		{
			shared_ptr<ConferenceParticipantDeviceEvent> deviceEvent =
			    static_pointer_cast<ConferenceParticipantDeviceEvent>(event);
			BC_ASSERT_TRUE(deviceEvent->getConferenceId().getPeerAddress()->toString() ==
			               "sip:test-44@sip.linphone.org");
			BC_ASSERT_TRUE(deviceEvent->getParticipantAddress()->toString() == "sip:test-11@sip.linphone.org");
			BC_ASSERT_TRUE(deviceEvent->getNotifyId() == 4);
			BC_ASSERT_TRUE(deviceEvent->getDeviceAddress()->toString() == "sip:test-47@sip.linphone.org");
		}
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void set_get_conference_info() {
	MainDbProvider provider;
	MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		auto confAddr = Address::create("sip:test-1@sip.linphone.org;conf-id=abaaa");
		std::shared_ptr<ConferenceInfo> info = ConferenceInfo::create();
		info->setOrganizer(Address::create("sip:test-47@sip.linphone.org"));
		info->addParticipant(Address::create("sip:test-11@sip.linphone.org"));
		info->addParticipant(Address::create("sip:test-44@sip.linphone.org"));
		info->setUri(confAddr);
		info->setDateTime(1682770620);
		info->setDuration(0);
		mainDb.insertConferenceInfo(info);

		auto confAddr2 = Address::create("sip:test-1@sip.linphone.org;conf-id=abbbb");
		std::shared_ptr<ConferenceInfo> info2 = ConferenceInfo::create();
		info2->setOrganizer(Address::create("sip:test-47@sip.linphone.org"));
		info2->addParticipant(Address::create("sip:test-11@sip.linphone.org"));
		info2->addParticipant(Address::create("sip:test-44@sip.linphone.org"));
		info2->setUri(confAddr2);
		info2->setDateTime(0);
		info2->setDuration(0);
		mainDb.insertConferenceInfo(info2);

		provider.reStart();
		MainDb &mainDb2 = provider.getMainDb();

		std::shared_ptr<ConferenceInfo> retrievedInfo = mainDb2.getConferenceInfoFromURI(confAddr);
		BC_ASSERT_PTR_NOT_NULL(retrievedInfo);
		if (retrievedInfo) {
			BC_ASSERT_EQUAL(1682770620, (long long)retrievedInfo->getDateTime(), long long, "%lld");
			BC_ASSERT_EQUAL((long long)info->getDateTime(), (long long)retrievedInfo->getDateTime(), long long, "%lld");
		}

		std::shared_ptr<ConferenceInfo> retrievedInfo2 = mainDb2.getConferenceInfoFromURI(confAddr2);
		BC_ASSERT_PTR_NOT_NULL(retrievedInfo2);
		if (retrievedInfo2) {
			BC_ASSERT_EQUAL((long long)info2->getDateTime(), (long long)retrievedInfo2->getDateTime(), long long,
			                "%lld");
			BC_ASSERT_EQUAL(0, (long long)retrievedInfo2->getDateTime(), long long, "%lld");
		}

		auto confAddr3 = Address::create("sip:test-1@sip.linphone.org;conf-id=abcd");
		std::shared_ptr<ConferenceInfo> retrievedInfo3 = mainDb2.getConferenceInfoFromURI(confAddr3);
		BC_ASSERT_PTR_NOT_NULL(retrievedInfo3);
		if (retrievedInfo3) {
			BC_ASSERT_EQUAL(1682770620, (long long)retrievedInfo3->getDateTime(), long long, "%lld");
		}

		auto confAddr4 = Address::create("sip:test-1@sip.linphone.org;conf-id=efgh");
		std::shared_ptr<ConferenceInfo> retrievedInfo4 = mainDb2.getConferenceInfoFromURI(confAddr4);
		BC_ASSERT_PTR_NOT_NULL(retrievedInfo4);
		if (retrievedInfo4) {
			BC_ASSERT_EQUAL(0, (long long)retrievedInfo4->getDateTime(), long long, "%lld");
		}
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void get_chat_rooms() {
	MainDbProvider provider;
	MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		std::string utf8Txt = "Héllo world ! Welcome to ベルドンヌ通信.";
		list<shared_ptr<AbstractChatRoom>> chatRooms = mainDb.getChatRooms();
		BC_ASSERT_EQUAL(chatRooms.size(), 86, size_t, "%zu");

		list<shared_ptr<AbstractChatRoom>> emptyChatRooms;
		shared_ptr<AbstractChatRoom> emptyMessageRoom = nullptr;
		shared_ptr<AbstractChatRoom> oneMessageRoom = nullptr;
		shared_ptr<AbstractChatRoom> multiMessageRoom = nullptr;
		for (const auto &chatRoom : chatRooms) {
			if (emptyMessageRoom == nullptr && chatRoom->getMessageHistorySize() == 0) {
				emptyMessageRoom = chatRoom;
			}
			if (oneMessageRoom == nullptr && chatRoom->getMessageHistorySize() == 1) {
				oneMessageRoom = chatRoom;
			}
			if (multiMessageRoom == nullptr && chatRoom->getMessageHistorySize() > 1) {
				multiMessageRoom = chatRoom;
			}

			shared_ptr<ChatMessage> lastMessage = chatRoom->getLastChatMessageInHistory();
			if (chatRoom->isEmpty()) {
				emptyChatRooms.push_back(chatRoom);
				BC_ASSERT_PTR_NULL(lastMessage);
			} else {
				BC_ASSERT_PTR_NOT_NULL(lastMessage);
			}
		}
		BC_ASSERT_EQUAL((int)emptyChatRooms.size(), 4, int, "%d");

		// Check an empty chat room last_message_id is updated after adding a message into it
		BC_ASSERT_PTR_NOT_NULL(emptyMessageRoom);
		if (emptyMessageRoom != nullptr) {
			shared_ptr<ChatMessage> lastMessage = emptyMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NULL(lastMessage);
			shared_ptr<ChatMessage> newMessage = emptyMessageRoom->createChatMessageFromUtf8(utf8Txt);
			newMessage->send();
			lastMessage = emptyMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NOT_NULL(lastMessage);
			BC_ASSERT_PTR_EQUAL(lastMessage, newMessage);
			mainDb.loadChatMessageContents(lastMessage); // Force read Database
			for (const auto &content : lastMessage->getContents()) {
				BC_ASSERT_EQUAL(content->getBodyAsUtf8String().compare(utf8Txt), 0, int, "%d");
			}
		}

		// Check last_message_id is updated to 0 if we remove the last message from a chat room with exactly 1 message
		BC_ASSERT_PTR_NOT_NULL(oneMessageRoom);
		if (oneMessageRoom != nullptr) {
			shared_ptr<ChatMessage> lastMessage = oneMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NOT_NULL(lastMessage);
			oneMessageRoom->deleteHistory();
			lastMessage = oneMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NULL(lastMessage);
		}

		// Check last_message_id is updated to previous message id if we remove the last message from a chat room with
		// more than 1 message
		BC_ASSERT_PTR_NOT_NULL(multiMessageRoom);
		if (multiMessageRoom != nullptr) {
			shared_ptr<ChatMessage> lastMessage = multiMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NOT_NULL(lastMessage);
			multiMessageRoom->deleteMessageFromHistory(lastMessage);
			shared_ptr<ChatMessage> lastMessage2 = multiMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NOT_NULL(lastMessage2);
			BC_ASSERT_PTR_NOT_EQUAL(lastMessage, lastMessage2);

			// Check a non empty chat room last_message_id is updated after adding a message into it
			shared_ptr<ChatMessage> newMessage = multiMessageRoom->createChatMessageFromUtf8(utf8Txt);
			newMessage->send();
			lastMessage = multiMessageRoom->getLastChatMessageInHistory();
			BC_ASSERT_PTR_NOT_NULL(lastMessage);
			BC_ASSERT_PTR_EQUAL(lastMessage, newMessage);
			BC_ASSERT_PTR_NOT_EQUAL(lastMessage, lastMessage2);
			mainDb.loadChatMessageContents(lastMessage); // Force read Database
			for (const auto &content : lastMessage->getContents()) {
				BC_ASSERT_EQUAL(content->getBodyAsUtf8String().compare(utf8Txt), 0, int, "%d");
			}
		}
	} else {
		BC_FAIL("Database not initialized");
	}
}

static void load_a_lot_of_chatrooms(void) {
#ifndef _WIN32
	int current_priority = getpriority(PRIO_PROCESS, 0);
	int err = setpriority(PRIO_PROCESS, 0, -20);
	if (err != 0) {
		ms_warning("load_a_lot_of_chatrooms(): setpriority failed [%s]- the time measurement may be unreliable",
		           strerror(errno));
	}
#endif

	chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();
	MainDbProvider provider("db/chatrooms.db");
	chrono::high_resolution_clock::time_point end = chrono::high_resolution_clock::now();
	long ms = (long)chrono::duration_cast<chrono::milliseconds>(end - start).count();
#ifdef ENABLE_SANITIZER
	BC_ASSERT_LOWER(ms, 3200, long, "%li");
#else
#if __APPLE__
	BC_ASSERT_LOWER(ms, 1000, long, "%li");
#else
	BC_ASSERT_LOWER(ms, 600, long, "%li");
#endif
#endif

#ifndef _WIN32
	err = setpriority(PRIO_PROCESS, 0, current_priority);
	if (err != 0) {
		ms_warning("load_a_lot_of_chatrooms(): cannot restore priority to [%i]: %s", current_priority, strerror(errno));
	}
#endif
}

static void load_chatroom_conference(void) {
	MainDbProvider provider("db/chatroom_conference.db");
	MainDb &mainDb = provider.getMainDb();
	if (mainDb.isInitialized()) {
		list<shared_ptr<AbstractChatRoom>> chatRooms = mainDb.getChatRooms();
		BC_ASSERT_EQUAL(chatRooms.size(), 1, size_t, "%zu");

		list<shared_ptr<ConferenceInfo>> conferenceInfos = mainDb.getConferenceInfos();
		BC_ASSERT_EQUAL(conferenceInfos.size(), 1, size_t, "%zu");

		for (const auto &conferenceInfo : conferenceInfos) {
			BC_ASSERT_PTR_NOT_NULL(mainDb.getConferenceInfoFromURI(conferenceInfo->getUri()));
		}
	} else {
		BC_FAIL("Database not initialized");
	}
}

test_t main_db_tests[] = {TEST_NO_TAG("Get events count", get_events_count),
                          TEST_NO_TAG("Get messages count", get_messages_count),
                          TEST_NO_TAG("Get unread messages count", get_unread_messages_count),
                          TEST_NO_TAG("Get history", get_history),
                          TEST_NO_TAG("Get conference events", get_conference_notified_events),
                          TEST_NO_TAG("Get chat rooms", get_chat_rooms),
                          TEST_NO_TAG("Set/get conference info", set_get_conference_info),
                          TEST_NO_TAG("Load a lot of chatrooms", load_a_lot_of_chatrooms),
                          TEST_NO_TAG("Load chatroom and conference", load_chatroom_conference)};

test_suite_t main_db_test_suite = {"MainDb",
                                   NULL,
                                   NULL,
                                   liblinphone_tester_before_each,
                                   liblinphone_tester_after_each,
                                   sizeof(main_db_tests) / sizeof(main_db_tests[0]),
                                   main_db_tests,
                                   0};
