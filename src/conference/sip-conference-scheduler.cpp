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

#include <bctoolbox/defs.h>

#include "conference/params/call-session-params-p.h"
#include "conference/participant-info.h"
#include "conference/session/media-session.h"
#include "conference/sip-conference-scheduler.h"
#include "core/core-p.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

SIPConferenceScheduler::SIPConferenceScheduler(const shared_ptr<Core> &core) : ConferenceScheduler(core) {
}

SIPConferenceScheduler::~SIPConferenceScheduler() {
	if (mSession != nullptr) {
		mSession->setListener(nullptr);
	}
}

void SIPConferenceScheduler::createOrUpdateConferenceOnServer(const std::shared_ptr<ConferenceParams> &conferenceParams,
                                                              const std::shared_ptr<Address> &creator,
                                                              const std::list<std::shared_ptr<Address>> &invitees,
                                                              const std::shared_ptr<Address> &conferenceAddress) {

	mSession = getCore()->createOrUpdateConferenceOnServer(conferenceParams, creator, invitees, conferenceAddress);
	if (mSession == nullptr) {
		lError() << "[Conference Scheduler] [" << this << "] createConferenceOnServer returned a null session!";
		setState(State::Error);
		return;
	}
	mSession->setListener(this);
}

void SIPConferenceScheduler::onCallSessionSetTerminated(const std::shared_ptr<CallSession> &session) {
	const std::shared_ptr<Address> remoteAddress = session->getRemoteContactAddress();
	if (remoteAddress == nullptr) {
		auto conferenceAddress = mConferenceInfo->getUri();
		lError() << "[Conference Scheduler] [" << this
		         << "] The session to update the conference information of conference "
		         << (conferenceAddress && conferenceAddress->isValid() ? conferenceAddress->toString()
		                                                               : std::string("<unknown-address>"))
		         << " did not succesfully establish hence it is likely that the request wasn't taken into account by "
		            "the server";
		setState(State::Error);
	} else if (getState() != State::Error) {
		// Do not try to call inpromptu conference if a participant updates its informations
		if ((getState() == State::AllocationPending) && (session->getParams()->getPrivate()->getStartTime() < 0)) {
			lInfo() << "Automatically rejoining conference " << remoteAddress->toString();
			auto new_params = linphone_core_create_call_params(getCore()->getCCore(), nullptr);
			// Participant with the focus call is admin
			L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContactParameter("admin", Utils::toString(true));
			std::list<std::shared_ptr<Address>> addressesList;
			for (const auto &participantInfo : mConferenceInfo->getParticipants()) {
				addressesList.push_back(participantInfo->getAddress());
			}
			addressesList.sort([](const auto &addr1, const auto &addr2) { return *addr1 < *addr2; });
			addressesList.unique([](const auto &addr1, const auto &addr2) { return addr1->weakEqual(*addr2); });

			if (!addressesList.empty()) {
				auto content = Content::create();
				content->setBodyFromUtf8(Utils::getResourceLists(addressesList));
				content->setContentType(ContentType::ResourceLists);
				content->setContentDisposition(ContentDisposition::RecipientList);
				if (linphone_core_content_encoding_supported(getCore()->getCCore(), "deflate")) {
					content->setContentEncoding("deflate");
				}

				L_GET_CPP_PTR_FROM_C_OBJECT(new_params)->addCustomContent(content);
			}
			const LinphoneVideoActivationPolicy *pol = linphone_core_get_video_activation_policy(getCore()->getCCore());
			bool_t initiate_video = !!linphone_video_activation_policy_get_automatically_initiate(pol);
			linphone_call_params_enable_video(
			    new_params,
			    static_pointer_cast<MediaSession>(session)->getMediaParams()->videoEnabled() && initiate_video);

			linphone_core_invite_address_with_params_2(getCore()->getCCore(), remoteAddress->toC(), new_params,
			                                           L_STRING_TO_C(mConferenceInfo->getSubject()), NULL);
			linphone_call_params_unref(new_params);
		}

		auto conferenceAddress = remoteAddress;
		lInfo() << "[Conference Scheduler] [" << this
		        << "] Conference has been succesfully created: " << *conferenceAddress;
		setConferenceAddress(conferenceAddress);
	}
}

void SIPConferenceScheduler::onCallSessionStateChanged(const shared_ptr<CallSession> &session,
                                                       CallSession::State state,
                                                       BCTBX_UNUSED(const string &message)) {
	switch (state) {
		case CallSession::State::Error:
			setState(State::Error);
			break;
		case CallSession::State::StreamsRunning: {
			const LinphoneErrorInfo *errorCode = session->getErrorInfo();
			const std::shared_ptr<Address> address = session->getRemoteAddress();
			processResponse(errorCode, address);
			break;
		}
		default:
			break;
	}
}

void SIPConferenceScheduler::processResponse(BCTBX_UNUSED(const LinphoneErrorInfo *errorCode),
                                             BCTBX_UNUSED(const std::shared_ptr<Address> conferenceAddress)) {
	mSession->terminate();
}

LINPHONE_END_NAMESPACE
