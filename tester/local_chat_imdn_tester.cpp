/*
 * copyright (c) 2010-2023 belledonne communications sarl.
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

#include "conference/participant.h"
#include "liblinphone_tester.h"
#include "local_conference_tester_functions.h"

namespace LinphoneTest {

static void group_chat_room_with_imdn(void) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		bool_t encrypted = FALSE;
		ClientConference marie("marie_rc", focus.getIdentity(), encrypted);
		ClientConference marie2("marie_rc", focus.getIdentity(), encrypted);
		ClientConference michelle("michelle_rc", focus.getIdentity(), encrypted);
		ClientConference michelle2("michelle_rc", focus.getIdentity(), encrypted);
		ClientConference pauline("pauline_rc", focus.getIdentity(), encrypted);
		ClientConference pauline2("pauline_rc", focus.getIdentity(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(marie2);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(pauline2);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline2.getLc()));

		stats marie_stat = marie.getStats();
		stats marie2_stat = marie.getStats();
		stats michelle_stat = michelle.getStats();
		stats michelle2_stat = michelle2.getStats();
		stats pauline_stat = pauline.getStats();
		stats pauline2_stat = pauline2.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, marie2.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, pauline2.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_X3dhUserCreationSuccess,
			                             marie_stat.number_of_X3dhUserCreationSuccess + 1, x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_X3dhUserCreationSuccess,
			                             marie2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_X3dhUserCreationSuccess,
			                             michelle_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_X3dhUserCreationSuccess,
			                             michelle2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_X3dhUserCreationSuccess,
			                             pauline_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_X3dhUserCreationSuccess,
			                             pauline2_stat.number_of_X3dhUserCreationSuccess + 1,
			                             x3dhServer_creationTimeout));

			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie2.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline2.getLc()));
		}

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));
		Address pauline2Addr = pauline2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(pauline2Addr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, 2, encrypted,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		LinphoneChatRoom *marie2Cr = check_creation_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat,
		                                                                  confAddr, initialSubject, 2, TRUE);
		BC_ASSERT_PTR_NOT_NULL(marie2Cr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &michelle_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &michelle2_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *pauline2Cr = check_creation_chat_room_client_side(
		    coresList, pauline2.getCMgr(), &pauline2_stat, confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(pauline2Cr);

		// Marie sends the message
		const char *marieMessage = "Hey ! What's up ?";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(marieCr, marieMessage);
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageReceived,
		                             marie2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *marie2LastMsg = marie2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marie2LastMsg);
		if (marie2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(marie2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageReceived,
		                             michelle_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(michelleLastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageReceived,
		                             michelle2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(michelle2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
		                             pauline_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(paulineLastMsg);
		if (paulineLastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(paulineLastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageReceived,
		                             pauline2_stat.number_of_LinphoneMessageReceived + 1,
		                             liblinphone_tester_sip_timeout));
		LinphoneChatMessage *pauline2LastMsg = pauline2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(pauline2LastMsg);
		if (pauline2LastMsg) {
			LinphoneChatMessageCbs *cbs = linphone_chat_message_get_callbacks(pauline2LastMsg);
			linphone_chat_message_cbs_set_msg_state_changed(cbs, liblinphone_tester_chat_message_msg_state_changed);
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(paulineCr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDisplayed,
		                             marie_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &marie2.getStats().number_of_LinphoneMessageDisplayed,
		                             marie2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneMessageDisplayed,
		                             michelle_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneMessageDisplayed,
		                             michelle2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));
		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline2.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline2_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		linphone_chat_message_unref(msg);
		msg = nullptr;

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(L_GET_C_BACK_PTR(chatRoom), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, marie2, michelle, michelle2, pauline, pauline2})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void
group_chat_room_with_client_idmn_after_restart_base(bool_t encrypted, bool_t add_participant, bool_t stop_core) {
	Focus focus("chloe_rc");
	{ // to make sure focus is destroyed after clients.
		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference michelle2("michelle_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), encrypted);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), encrypted);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(michelle);
		focus.registerAsParticipantDevice(michelle2);
		focus.registerAsParticipantDevice(laure);
		focus.registerAsParticipantDevice(pauline);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(michelle2.getLc()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		stats michelle_stat = michelle.getStats();
		stats michelle2_stat = michelle2.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());
		coresList = bctbx_list_append(coresList, michelle.getLc());
		coresList = bctbx_list_append(coresList, michelle2.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle.getLc()));
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(michelle2.getLc()));
		}

		bctbx_list_t *participantsAddresses = NULL;
		Address michelleAddr = michelle.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelleAddr.toC()));
		Address michelle2Addr = michelle2.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(michelle2Addr.toC()));
		Address laureAddr = laure.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));
		Address paulineAddr = pauline.getIdentity();
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(paulineAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues (characters: $ £ çà)";
		LinphoneChatRoom *marieCr = create_chat_room_client_side_with_expected_number_of_participants(
		    coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject, 3, encrypted,
		    LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);
		char *conference_address = linphone_address_as_string(confAddr);

		// Check that the chat room is correctly created on Michelle's side and that the participants are added
		LinphoneChatRoom *michelleCr = check_creation_chat_room_client_side(
		    coresList, michelle.getCMgr(), &michelle_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelleCr);
		LinphoneChatRoom *michelle2Cr = check_creation_chat_room_client_side(
		    coresList, michelle2.getCMgr(), &michelle2_stat, confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(michelle2Cr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);

		// Check that the chat room is correctly created on Laure's side and that the participants are added
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat,
		                                                                 confAddr, initialSubject, 3, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure}).wait([&focus] {
			for (auto chatRoom : focus.getCore().getChatRooms()) {
				for (auto participant : chatRoom->getParticipants()) {
					for (auto device : participant->getDevices())
						if (device->getState() != ParticipantDevice::State::Present) {
							return false;
						}
				}
			}
			return true;
		}));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_LinphoneConferenceStateCreated,
		                             michelle_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_LinphoneConferenceStateCreated,
		                             michelle2_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneConferenceStateCreated,
		                             laure_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneConferenceStateCreated,
		                             pauline_stat.number_of_LinphoneConferenceStateCreated + 1,
		                             liblinphone_tester_sip_timeout));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, laure}).waitUntil(chrono::seconds(5), [] {
			return false;
		});

		ms_message("%s goes offline", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), FALSE);

		ClientConference berthe("berthe_rc", focus.getConferenceFactoryAddress(), encrypted);
		focus.registerAsParticipantDevice(berthe);
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(berthe.getLc()));
		stats berthe_stat = berthe.getStats();
		coresList = bctbx_list_append(coresList, berthe.getLc());

		if (encrypted) {
			BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(berthe.getLc()));
		}

		marie_stat = marie.getStats();
		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();
		michelle_stat = michelle.getStats();
		michelle2_stat = michelle2.getStats();
		LinphoneChatRoom *bertheCr = NULL;
		if (add_participant) {
			Address bertheAddr = berthe.getIdentity();
			// Add and remove Berthe in order to create a gap between Laure's last notify and the actual notify of the
			// chatroom on the server side so that a full state is triggered
			for (int cnt = 0; cnt <= 6; cnt++) {
				marie_stat = marie.getStats();
				berthe_stat = berthe.getStats();
				pauline_stat = pauline.getStats();
				michelle_stat = michelle.getStats();
				michelle2_stat = michelle2.getStats();
				ms_message("Try #%0d - %s adds %s to chatroom %s", cnt, linphone_core_get_identity(marie.getLc()),
				           linphone_core_get_identity(berthe.getLc()), conference_address);
				linphone_chat_room_add_participant(marieCr, bertheAddr.toC());
				bertheCr = check_creation_chat_room_client_side(coresList, berthe.getCMgr(), &berthe_stat, confAddr,
				                                                initialSubject, 4, FALSE);
				// wait until chatroom is deleted from Berthe's standpoint
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					return berthe.getCore().getChatRooms().size() == 1;
				}));
				BC_ASSERT_PTR_NOT_NULL(bertheCr);
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
				                             pauline_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
				                             pauline_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
				                             michelle_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
				                             michelle_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_added,
				                             michelle2_stat.number_of_participants_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participant_devices_added,
				                             michelle2_stat.number_of_participant_devices_added + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&focus] {
					for (auto chatRoom : focus.getCore().getChatRooms()) {
						for (auto participant : chatRoom->getParticipants()) {
							for (auto device : participant->getDevices())
								if (device->getState() != ParticipantDevice::State::Present) {
									return false;
								}
						}
					}
					return true;
				}));
				ms_message("Try #%0d - %s removes %s to chatroom %s", cnt, linphone_core_get_identity(marie.getLc()),
				           linphone_core_get_identity(berthe.getLc()), conference_address);
				LinphoneParticipant *bertheParticipant = linphone_chat_room_find_participant(marieCr, bertheAddr.toC());
				BC_ASSERT_PTR_NOT_NULL(bertheParticipant);
				if (bertheParticipant) {
					linphone_chat_room_remove_participant(marieCr, bertheParticipant);
				}
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					for (auto chatRoom : berthe.getCore().getChatRooms()) {
						if (chatRoom->getState() != ChatRoom::State::Terminated) {
							return false;
						}
					}
					return true;
				}));
				if (bertheCr) {
					linphone_core_delete_chat_room(berthe.getLc(), bertheCr);
				}
				// wait until chatroom is deleted from Berthe's standpoint
				BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, berthe, laure}).wait([&berthe] {
					return berthe.getCore().getChatRooms().size() == 0;
				}));
				BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_removed,
				                             pauline_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_removed,
				                             michelle_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
				BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_removed,
				                             michelle2_stat.number_of_participants_removed + 1,
				                             liblinphone_tester_sip_timeout));
			}
			marie_stat = marie.getStats();
			berthe_stat = berthe.getStats();
			pauline_stat = pauline.getStats();
			michelle_stat = michelle.getStats();
			michelle2_stat = michelle2.getStats();
			ms_message("%s adds %s to chatroom %s one more time", linphone_core_get_identity(marie.getLc()),
			           linphone_core_get_identity(berthe.getLc()), conference_address);
			linphone_chat_room_add_participant(marieCr, bertheAddr.toC());
			bertheCr = check_creation_chat_room_client_side(coresList, berthe.getCMgr(), &berthe_stat, confAddr,
			                                                initialSubject, 4, FALSE);
			BC_ASSERT_PTR_NOT_NULL(bertheCr);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participants_added,
			                             pauline_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_participant_devices_added,
			                             pauline_stat.number_of_participant_devices_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participants_added,
			                             michelle_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle.getStats().number_of_participant_devices_added,
			                             michelle_stat.number_of_participant_devices_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participants_added,
			                             michelle2_stat.number_of_participants_added + 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &michelle2.getStats().number_of_participant_devices_added,
			                             michelle2_stat.number_of_participant_devices_added + 1, 5000));
		}

		std::string msg_text = "message pauline blabla";
		LinphoneChatMessage *msg = ClientConference::sendTextMsg(paulineCr, msg_text);

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([msg] {
			return (linphone_chat_message_get_state(msg) == LinphoneChatMessageStateDelivered);
		}));

		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([marieCr] {
			return linphone_chat_room_get_unread_messages_count(marieCr) == 1;
		}));
		LinphoneChatMessage *marieLastMsg = marie.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(marieLastMsg);
		if (marieLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(marieLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([michelleCr] {
			    return linphone_chat_room_get_unread_messages_count(michelleCr) == 1;
		    }));
		LinphoneChatMessage *michelleLastMsg = michelle.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelleLastMsg);
		if (michelleLastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelleLastMsg), msg_text.c_str());
		}

		BC_ASSERT_TRUE(
		    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([michelle2Cr] {
			    return linphone_chat_room_get_unread_messages_count(michelle2Cr) == 1;
		    }));
		LinphoneChatMessage *michelle2LastMsg = michelle2.getStats().last_received_chat_message;
		BC_ASSERT_PTR_NOT_NULL(michelle2LastMsg);
		if (michelle2LastMsg) {
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(michelle2LastMsg), msg_text.c_str());
		}

		if (bertheCr) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([bertheCr] {
				    return linphone_chat_room_get_unread_messages_count(bertheCr) == 1;
			    }));
			LinphoneChatMessage *bertheLastMsg = berthe.getStats().last_received_chat_message;
			BC_ASSERT_PTR_NOT_NULL(bertheLastMsg);
			if (bertheLastMsg) {
				BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(bertheLastMsg), msg_text.c_str());
			}
		}

		linphone_chat_room_mark_as_read(michelleCr);
		linphone_chat_room_mark_as_read(michelle2Cr);
		if (bertheCr) {
			linphone_chat_room_mark_as_read(bertheCr);
		}
		linphone_chat_room_mark_as_read(marieCr);
		linphone_chat_room_mark_as_read(paulineCr);

		for (const auto client : {marie.getCMgr(), michelle.getCMgr(), michelle2.getCMgr(), berthe.getCMgr(),
		                          laure.getCMgr(), pauline.getCMgr()}) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline})
			        .wait([client, &berthe, &laure, &pauline, &add_participant, &msg] {
				        bool ret = false;
				        LinphoneChatMessage *lastMsg =
				            (client->lc == pauline.getLc()) ? msg : client->stat.last_received_chat_message;
				        if ((client->lc == laure.getLc()) || (!add_participant && (client->lc == berthe.getLc()))) {
					        ret = (lastMsg == nullptr);
				        } else {
					        ret = (lastMsg != nullptr);
					        if (lastMsg) {
						        bctbx_list_t *displayed_list = linphone_chat_message_get_participants_by_imdn_state(
						            lastMsg, LinphoneChatMessageStateDisplayed);
						        size_t expected_displayed_number = 2 + (add_participant ? 1 : 0);
						        ret &= (bctbx_list_size(displayed_list) == expected_displayed_number);
						        bctbx_list_free_with_data(displayed_list,
						                                  (bctbx_list_free_func)linphone_participant_imdn_state_unref);
					        }
				        }
				        return ret;
			        }));
		}

		pauline_stat = pauline.getStats();
		laure_stat = laure.getStats();
		ms_message("%s comes back online", linphone_core_get_identity(laure.getLc()));
		linphone_core_set_network_reachable(laure.getLc(), TRUE);
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
		                             laure_stat.number_of_LinphoneMessageReceived + 1, liblinphone_tester_sip_timeout));
		LinphoneAddress *laureDeviceAddr = linphone_address_clone(
		    linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(laure.getLc())));
		laureCr = linphone_core_search_chat_room(laure.getLc(), NULL, laureDeviceAddr, confAddr, NULL);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		char *uuid = NULL;
		if (linphone_config_get_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", NULL)) {
			uuid =
			    bctbx_strdup(linphone_config_get_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", NULL));
		}
		BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_NotifyReceived,
		                             laure_stat.number_of_NotifyReceived + 1, liblinphone_tester_sip_timeout));
		if (laureCr) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline}).wait([laureCr] {
				    return linphone_chat_room_get_unread_messages_count(laureCr) == 1;
			    }));
			linphone_chat_room_mark_as_read(laureCr);
			if (stop_core) {
				ms_message("%s stops its core", linphone_core_get_identity(laure.getLc()));
				coresList = bctbx_list_remove(coresList, laure.getLc());
				linphone_core_manager_stop(laure.getCMgr());
			}
		}
		linphone_address_unref(laureDeviceAddr);

		BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageDisplayed,
		                             pauline_stat.number_of_LinphoneMessageDisplayed + 1,
		                             liblinphone_tester_sip_timeout));

		if (stop_core) {
			linphone_core_manager_configure(laure.getCMgr());
			// Make sure gruu is preserved
			linphone_config_set_string(linphone_core_get_config(laure.getLc()), "misc", "uuid", uuid);
			ms_message("%s starts its core", linphone_core_get_identity(laure.getLc()));
			linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));
			linphone_core_manager_start(laure.getCMgr(), TRUE);
			coresList = bctbx_list_append(coresList, laure.getLc());
		}

		if (uuid) {
			bctbx_free(uuid);
		}

		for (const auto client : {marie.getCMgr(), michelle.getCMgr(), michelle2.getCMgr(), berthe.getCMgr(),
		                          laure.getCMgr(), pauline.getCMgr()}) {
			BC_ASSERT_TRUE(
			    CoreManagerAssert({focus, marie, michelle, michelle2, berthe, laure, pauline})
			        .wait([client, &berthe, &add_participant, &confAddr] {
				        const LinphoneAddress *deviceAddr =
				            linphone_proxy_config_get_contact(linphone_core_get_default_proxy_config(client->lc));
				        LinphoneChatRoom *cr =
				            linphone_core_search_chat_room(client->lc, NULL, deviceAddr, confAddr, NULL);
				        LinphoneChatMessage *lastMsg = cr ? linphone_chat_room_get_last_message_in_history(cr) : NULL;
				        bool ret = false;
				        if (!add_participant && (client->lc == berthe.getLc())) {
					        ret = (lastMsg == nullptr);
				        } else {
					        ret = (lastMsg != nullptr);
					        if (lastMsg) {
						        bctbx_list_t *displayed_list = linphone_chat_message_get_participants_by_imdn_state(
						            lastMsg, LinphoneChatMessageStateDisplayed);
						        size_t expected_displayed_number = 3 + (add_participant ? 1 : 0);
						        ret &= (bctbx_list_size(displayed_list) == expected_displayed_number);
						        bctbx_list_free_with_data(displayed_list,
						                                  (bctbx_list_free_func)linphone_participant_imdn_state_unref);
					        }
				        }
				        return ret;
			        }));
		}

		linphone_chat_message_unref(msg);
		msg = nullptr;

		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(L_GET_C_BACK_PTR(chatRoom), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, michelle, michelle2, berthe, laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, michelle, michelle2, berthe, laure})
		    .waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		ms_free(conference_address);
		bctbx_list_free(coresList);
	}
}

static void group_chat_room_with_client_idmn_after_restart(void) {
	group_chat_room_with_client_idmn_after_restart_base(FALSE, TRUE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, FALSE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, TRUE, FALSE);
}

static void secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added_and_core_stopped(void) {
	group_chat_room_with_client_idmn_after_restart_base(TRUE, TRUE, TRUE);
}

static void group_chat_room_lime_session_corrupted(void) {
	Focus focus("chloe_rc");
	LinphoneChatMessage *msg;
	{ // to make sure focus is destroyed after clients.
		linphone_core_enable_lime_x3dh(focus.getLc(), true);

		ClientConference marie("marie_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference pauline("pauline_rc", focus.getConferenceFactoryAddress(), true);
		ClientConference laure("laure_tcp_rc", focus.getConferenceFactoryAddress(), true);

		focus.registerAsParticipantDevice(marie);
		focus.registerAsParticipantDevice(pauline);
		focus.registerAsParticipantDevice(laure);

		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(marie.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(pauline.getLc()));
		linphone_im_notif_policy_enable_all(linphone_core_get_im_notif_policy(laure.getLc()));

		stats marie_stat = marie.getStats();
		stats pauline_stat = pauline.getStats();
		stats laure_stat = laure.getStats();
		bctbx_list_t *coresList = bctbx_list_append(NULL, focus.getLc());
		coresList = bctbx_list_append(coresList, marie.getLc());
		coresList = bctbx_list_append(coresList, pauline.getLc());
		coresList = bctbx_list_append(coresList, laure.getLc());

		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(marie.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(pauline.getLc()));
		BC_ASSERT_TRUE(linphone_core_lime_x3dh_enabled(laure.getLc()));

		Address paulineAddr = pauline.getIdentity();
		Address laureAddr = laure.getIdentity();
		bctbx_list_t *participantsAddresses = bctbx_list_append(NULL, linphone_address_ref(paulineAddr.toC()));
		participantsAddresses = bctbx_list_append(participantsAddresses, linphone_address_ref(laureAddr.toC()));

		// Marie creates a new group chat room
		const char *initialSubject = "Colleagues";
		LinphoneChatRoom *marieCr =
		    create_chat_room_client_side(coresList, marie.getCMgr(), &marie_stat, participantsAddresses, initialSubject,
		                                 TRUE, LinphoneChatRoomEphemeralModeDeviceManaged);
		BC_ASSERT_PTR_NOT_NULL(marieCr);
		const LinphoneAddress *confAddr = linphone_chat_room_get_conference_address(marieCr);

		// Check that the chat room is correctly created on Pauline's side and that the participants are added
		LinphoneChatRoom *paulineCr = check_creation_chat_room_client_side(coresList, pauline.getCMgr(), &pauline_stat,
		                                                                   confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(paulineCr);
		LinphoneChatRoom *laureCr = check_creation_chat_room_client_side(coresList, laure.getCMgr(), &laure_stat,
		                                                                 confAddr, initialSubject, 2, FALSE);
		BC_ASSERT_PTR_NOT_NULL(laureCr);
		if (paulineCr && laureCr) {
			// Marie sends the message
			const char *marieMessage = "Hey ! What's up ?";
			msg = _send_message(marieCr, marieMessage);
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *paulineLastMsg = pauline.getStats().last_received_chat_message;
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 1,
			                             liblinphone_tester_sip_timeout));
			LinphoneChatMessage *laureLastMsg = laure.getStats().last_received_chat_message;
			linphone_chat_message_unref(msg);
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg)) goto end;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;

			// Check that the message was correctly decrypted if encrypted
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieMessage);
			LinphoneAddress *marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieMessage);
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);

			// Corrupt Pauline sessions in lime database: WARNING: if SOCI is not found, this call does nothing and the
			// test fails
			lime_delete_DRSessions(pauline.getCMgr()->lime_database_path, NULL);
			// Trick to force the reloading of the lime engine so the session in cache is cleared
			linphone_core_enable_lime_x3dh(pauline.getLc(), FALSE);
			linphone_core_enable_lime_x3dh(pauline.getLc(), TRUE);

			// Marie send a new message, it shall fail and get a 488 response
			const char *marieTextMessage2 = "Do you copy?";
			msg = _send_message(marieCr, marieTextMessage2);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
			                             marie_stat.number_of_LinphoneMessageDelivered + 2,
			                             liblinphone_tester_sip_timeout)); // Delivered to the server
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 2,
			                             liblinphone_tester_sip_timeout)); // the message is correctly received by Laure
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageNotDelivered,
			                             marie_stat.number_of_LinphoneMessageNotDelivered + 1,
			                             liblinphone_tester_sip_timeout)); // Not delivered to pauline
			BC_ASSERT_EQUAL(pauline.getStats().number_of_LinphoneMessageReceived, 1, int, "%d");
			linphone_chat_message_unref(msg);
			laureLastMsg = laure.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage2);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);

			// Try again, it shall work this time
			const char *marieTextMessage3 = "Hello again";
			marie_stat = marie.getStats();
			msg = _send_message(marieCr, marieTextMessage3);
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageSent, 1, 5000));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDelivered,
			                             marie_stat.number_of_LinphoneMessageDelivered + 1,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &pauline.getStats().number_of_LinphoneMessageReceived,
			                             pauline_stat.number_of_LinphoneMessageReceived + 2,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &laure.getStats().number_of_LinphoneMessageReceived,
			                             laure_stat.number_of_LinphoneMessageReceived + 3,
			                             liblinphone_tester_sip_timeout));
			BC_ASSERT_TRUE(wait_for_list(coresList, &marie.getStats().number_of_LinphoneMessageDeliveredToUser,
			                             marie_stat.number_of_LinphoneMessageDeliveredToUser + 1,
			                             liblinphone_tester_sip_timeout));
			paulineLastMsg = pauline.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(paulineLastMsg)) goto end;
			laureLastMsg = laure.getStats().last_received_chat_message;
			if (!BC_ASSERT_PTR_NOT_NULL(laureLastMsg)) goto end;
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(paulineLastMsg), marieTextMessage3);
			BC_ASSERT_STRING_EQUAL(linphone_chat_message_get_utf8_text(laureLastMsg), marieTextMessage3);
			marieAddr = linphone_address_new(linphone_core_get_identity(marie.getLc()));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(paulineLastMsg)));
			BC_ASSERT_TRUE(
			    linphone_address_weak_equal(marieAddr, linphone_chat_message_get_from_address(laureLastMsg)));
			linphone_address_unref(marieAddr);
			linphone_chat_message_unref(msg);
		}

	end:
		for (auto chatRoom : focus.getCore().getChatRooms()) {
			for (auto participant : chatRoom->getParticipants()) {
				//  force deletion by removing devices
				std::shared_ptr<Address> participantAddress = participant->getAddress();
				linphone_chat_room_set_participant_devices(L_GET_C_BACK_PTR(chatRoom), participantAddress->toC(), NULL);
			}
		}

		// wait until chatroom is deleted server side
		BC_ASSERT_TRUE(CoreManagerAssert({focus, marie, pauline, laure}).wait([&focus] {
			return focus.getCore().getChatRooms().size() == 0;
		}));

		// wait bit more to detect side effect if any
		CoreManagerAssert({focus, marie, pauline, laure}).waitUntil(chrono::seconds(2), [] { return false; });

		// to avoid creation attempt of a new chatroom
		auto config = focus.getDefaultProxyConfig();
		linphone_proxy_config_edit(config);
		linphone_proxy_config_set_conference_factory_uri(config, NULL);
		linphone_proxy_config_done(config);

		bctbx_list_free(coresList);
	}
}

static void group_chat_room_lime_server_clear_message(void) {
	group_chat_room_lime_server_message(FALSE);
}

} // namespace LinphoneTest

static test_t local_conference_chat_imdn_tests[] = {
    TEST_ONE_TAG("Group chat with client IMDN after restart",
                 LinphoneTest::group_chat_room_with_client_idmn_after_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Group chat Lime Server chat room send imdn error",
                LinphoneTest::group_chat_room_lime_session_corrupted),
    TEST_ONE_TAG("Secure group chat with client IMDN sent after restart",
                 LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart,
                 "LeaksMemory"), /* because of network up and down */
    TEST_ONE_TAG("Secure group chat with client IMDN sent after restart and participant added",
                 LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added,
                 "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Secure Group chat with IMDN", LinphoneTest::group_chat_room_with_imdn),
    TEST_ONE_TAG(
        "Secure group chat with client IMDN sent after restart and participant added and core stopped before sending "
        "IMDN",
        LinphoneTest::secure_group_chat_room_with_client_idmn_sent_after_restart_and_participant_added_and_core_stopped,
        "LeaksMemory"), /* because of network up and down */
    TEST_NO_TAG("Group chat Lime Server chat room clear message",
                LinphoneTest::group_chat_room_lime_server_clear_message)};

test_suite_t local_conference_test_suite_chat_imdn = {"Local conference tester (Chat IMDN)",
                                                      NULL,
                                                      NULL,
                                                      liblinphone_tester_before_each,
                                                      liblinphone_tester_after_each,
                                                      sizeof(local_conference_chat_imdn_tests) /
                                                          sizeof(local_conference_chat_imdn_tests[0]),
                                                      local_conference_chat_imdn_tests,
                                                      0};
