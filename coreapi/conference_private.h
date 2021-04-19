/*
 * Copyright (c) 2010-2019 Belledonne Communications SARL.
 *
 * This file is part of Liblinphone.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CONFERENCE_PRIVATE_H
#define CONFERENCE_PRIVATE_H

#include "linphone/core.h"
#include "linphone/conference.h"
#include "conference/conference.h"
#include "call/audio-device/audio-device.h"

#include "c-wrapper/c-wrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Type of the funtion to pass as callback to linphone_conference_params_set_state_changed_callback()
 * @param conference The conference instance which the state has changed
 * @param new_state The new state of the conferenece
 * @param user_data Pointer pass to user_data while linphone_conference_params_set_state_changed_callback() was being called
 */
typedef void (*LinphoneConferenceStateChangedCb)(LinphoneConference *conference, LinphoneConferenceState new_state, void *user_data);

/**
 * A function to convert a #LinphoneConferenceState into a string
 */
char *linphone_conference_state_to_string(LinphoneConferenceState state);

/**
 * Get the state of a conference
 */
LinphoneConferenceState linphone_conference_get_state(const LinphoneConference *obj);

/**
 * Set a callback which will be called when the state of the conferenec is switching
 * @param obj A #LinphoneConference object
 * @param cb The callback to call
 * @param user_data Pointer to pass to the user_data parameter of #LinphoneConferenceStateChangedCb
 */
void linphone_conference_set_state_changed_callback(LinphoneConference *obj, LinphoneConferenceStateChangedCb cb, void *user_data);

#ifdef __cplusplus
}
#endif

LINPHONE_BEGIN_NAMESPACE

class Call;
class CallSessionListener;
class Participant;
class AudioControlInterface;
class VideoControlInterface;
class MixerSession;

class ConferenceFactoryInterface;
class ConferenceParamsInterface;
class ConferenceParams;
class Conference;

class RemoteConferenceEventHandler;
#ifdef HAVE_ADVANCED_IM
class LocalAudioVideoConferenceEventHandler;
#endif // HAVE_ADVANCED_IM


class ConferenceCbs : public bellesip::HybridObject<LinphoneConferenceCbs, ConferenceCbs>, public Callbacks{
public:
	LinphoneConferenceCbsParticipantAddedCb participantAddedCb;
	LinphoneConferenceCbsParticipantRemovedCb participantRemovedCb;
	LinphoneConferenceCbsParticipantDeviceAddedCb participantDeviceAddedCb;
	LinphoneConferenceCbsParticipantDeviceRemovedCb participantDeviceRemovedCb;
	LinphoneConferenceCbsParticipantAdminStatusChangedCb participantAdminStatusChangedCb;
	LinphoneConferenceCbsParticipantDeviceMediaChangedCb participantDeviceMediaChangedCb;
	LinphoneConferenceCbsStateChangedCb stateChangedCb;
	LinphoneConferenceCbsSubjectChangedCb subjectChangedCb;
	LinphoneConferenceCbsAudioDeviceChangedCb audioDeviceChangedCb;
};

namespace MediaConference{ // They are in a special namespace because of conflict of generic Conference classes in src/conference/*

class Conference;
class LocalConference;
class RemoteConference;




/*
 * Base class for audio/video conference.
 */

class LINPHONE_PUBLIC Conference : public bellesip::HybridObject<LinphoneConference, Conference>, 
					public LinphonePrivate::Conference,
					public LinphonePrivate::CallbacksHolder<LinphonePrivate::ConferenceCbs>,
					public UserDataAccessor{
#ifdef HAVE_ADVANCED_IM
	friend class LocalAudioVideoConferenceEventHandler;
#endif // HAVE_ADVANCED_IM
public:
	Conference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~Conference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) = 0;
	virtual bool addParticipant(const IdentityAddress &participantAddress) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;

	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call);

	virtual int removeParticipantDevice(const std::shared_ptr<LinphonePrivate::CallSession> & session);
	int removeParticipant(std::shared_ptr<LinphonePrivate::Call> call);
	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession);
	virtual int removeParticipant(const IdentityAddress &addr) = 0;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;

	virtual bool removeParticipants (const std::list<std::shared_ptr<LinphonePrivate::Participant>> &participants) override;

	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) = 0;
	virtual int participantDeviceMediaChanged(const IdentityAddress &addr) = 0;
	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) = 0;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session, uint32_t ssrc) = 0;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> & device) = 0;

	virtual int terminate() = 0;
	virtual void finalizeCreation() = 0;

	virtual int enter() = 0;
	virtual void leave() override = 0;

	void setInputAudioDevice(AudioDevice *audioDevice);
	void setOutputAudioDevice(AudioDevice *audioDevice);
	AudioDevice *getInputAudioDevice() const;
	AudioDevice *getOutputAudioDevice() const;

	virtual AudioControlInterface * getAudioControlInterface() const = 0;
	virtual VideoControlInterface * getVideoControlInterface() const = 0;
	virtual AudioStream *getAudioStream() = 0; /* Used by the tone manager, revisit.*/

	virtual int getSize() const {return getParticipantCount() + (isIn()?1:0);}

	virtual int startRecording(const char *path) = 0;
	virtual int stopRecording() = 0;
	virtual bool isRecording() const = 0;

	void setState (LinphonePrivate::ConferenceInterface::State state) override;
	void setStateChangedCallback(LinphoneConferenceStateChangedCb cb, void *userData) {
		mStateChangedCb = cb;
		mCbUserData = userData;
	}

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;

	virtual void join () override;
	virtual void join (const IdentityAddress &participantAddress) override;

	virtual void onConferenceTerminated (const IdentityAddress &addr) override;

	void checkIfTerminated();

	void setID(const std::string &conferenceID) {
		mConferenceID = conferenceID;
	}
	const std::string& getID() const {
		return mConferenceID;
	}

	void setConferenceAddress (const ConferenceAddress &conferenceAddress);
	virtual void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) override;

protected:
	void setConferenceId (const ConferenceId &conferenceId);

protected:

	// Legacy member
	std::string mConferenceID;

	LinphoneConferenceStateChangedCb mStateChangedCb = nullptr;
	void *mCbUserData = nullptr;
};


/*
 * Class for an audio/video conference running locally.
 */
class LINPHONE_PUBLIC LocalConference: public Conference {
public:
	LocalConference(const std::shared_ptr<Core> &core, const IdentityAddress &myAddress, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);

	virtual ~LocalConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	virtual bool addParticipants(const std::list<std::shared_ptr<LinphonePrivate::Call>> &call) override;
	virtual bool addParticipants (const std::list<IdentityAddress> &addresses) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool addParticipant(const IdentityAddress &participantAddress) override;
	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) override;
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual bool update(const ConferenceParamsInterface &params) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;
	virtual void onConferenceTerminated (const IdentityAddress &addr) override;
	virtual void setSubject (const std::string &subject) override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording(const char *path) override;
	virtual int stopRecording() override;
	virtual bool isRecording() const override;
	
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void subscribeReceived (LinphoneEvent *event);
	void subscriptionStateChanged (LinphoneEvent *event, LinphoneSubscriptionState state);

	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) override;
	virtual int participantDeviceMediaChanged(const IdentityAddress &addr) override;
	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session, uint32_t ssrc) override;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> & device) override;

	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) override;
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant) override;
	virtual std::shared_ptr<ConferenceParticipantEvent> notifyParticipantSetAdmin (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, bool isAdmin) override;
	virtual std::shared_ptr<ConferenceSubjectEvent> notifySubjectChanged (time_t creationTime, const bool isFullState, const std::string subject) override;
	virtual std::shared_ptr<ConferenceAvailableMediaEvent> notifyAvailableMediaChanged (time_t creationTime, const bool isFullState, const std::map<ConferenceMediaCapabilities, bool> mediaCapabilities) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceAdded (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceRemoved (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) override;
	virtual std::shared_ptr<ConferenceParticipantDeviceEvent> notifyParticipantDeviceMediaChanged (time_t creationTime,  const bool isFullState, const std::shared_ptr<Participant> &participant, const std::shared_ptr<ParticipantDevice> &participantDevice) override;

	virtual void notifyFullState () override;

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;

	virtual void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) override;

private:

	bool updateAllParticipantSessionsExcept(const std::shared_ptr<CallSession> & session);
	void chooseAnotherAdminIfNoneInConference();
	void addLocalEndpoint();
	void removeLocalEndpoint();
	std::unique_ptr<MixerSession> mMixerSession;
	bool mIsIn = false;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<LocalAudioVideoConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM
};

/*
 * Class for an audio/video conference that is running on a remote server.
 */
class LINPHONE_PUBLIC RemoteConference:
	public Conference,
	public ConferenceListenerInterface {
public:
	RemoteConference(const std::shared_ptr<Core> &core, const IdentityAddress &focus, const ConferenceId &conferenceId, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	RemoteConference(const std::shared_ptr<Core> &core, const std::shared_ptr<LinphonePrivate::Call>& focusCall, const ConferenceId &conferenceId, CallSessionListener *listener, const std::shared_ptr<ConferenceParams> params);
	virtual ~RemoteConference();

	virtual int inviteAddresses(const std::list<const LinphoneAddress*> &addresses, const LinphoneCallParams *params) override;
	virtual bool addParticipant(std::shared_ptr<LinphonePrivate::Call> call) override;
	virtual bool addParticipant(const IdentityAddress &participantAddress) override;
	virtual bool addParticipantDevice(std::shared_ptr<LinphonePrivate::Call> call) override;

	virtual int removeParticipant(const std::shared_ptr<LinphonePrivate::CallSession> & session, const bool preserveSession) override;
	virtual int removeParticipant(const IdentityAddress &addr) override;
	virtual bool removeParticipant(const std::shared_ptr<LinphonePrivate::Participant> &participant) override;
	virtual int terminate() override;
	virtual void finalizeCreation() override;

	virtual int enter() override;
	virtual void leave() override;
	virtual bool isIn() const override;

	virtual int startRecording (const char *path) override {
		return 0;
	}
	virtual int stopRecording() override {
		return 0;
	}
	virtual bool isRecording() const override {
		return false;
	}
	virtual const std::shared_ptr<CallSession> getMainSession() const override;
	virtual AudioControlInterface * getAudioControlInterface() const override;
	virtual VideoControlInterface * getVideoControlInterface() const override;
	virtual AudioStream *getAudioStream() override;

	void multipartNotifyReceived (const Content &content);
	void notifyReceived (const Content &content);

	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session) override;
	virtual int participantDeviceMediaChanged(const IdentityAddress &addr) override;
	virtual int participantDeviceMediaChanged(const std::shared_ptr<LinphonePrivate::Participant> & participant, const std::shared_ptr<LinphonePrivate::ParticipantDevice> &device) override;
	virtual int participantDeviceSsrcChanged(const std::shared_ptr<LinphonePrivate::CallSession> & session, uint32_t ssrc) override;

	virtual int getParticipantDeviceVolume(const std::shared_ptr<LinphonePrivate::ParticipantDevice> & device) override;

	virtual void onStateChanged(LinphonePrivate::ConferenceInterface::State state) override;
	virtual void onParticipantAdded (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;
	virtual void onParticipantRemoved (const std::shared_ptr<ConferenceParticipantEvent> &event, const std::shared_ptr<Participant> &participant) override;

	virtual void setParticipantAdminStatus (const std::shared_ptr<LinphonePrivate::Participant> &participant, bool isAdmin) override;
	virtual void setSubject (const std::string &subject) override;
	virtual bool update(const ConferenceParamsInterface &params) override;

	virtual void notifyStateChanged (LinphonePrivate::ConferenceInterface::State state) override;

#ifdef HAVE_ADVANCED_IM
	std::shared_ptr<RemoteConferenceEventHandler> eventHandler;
#endif // HAVE_ADVANCED_IM
private:
	std::shared_ptr<CallSession> focusSession;

	bool focusIsReady() const;
	bool transferToFocus(std::shared_ptr<LinphonePrivate::Call> call);
	void reset();

	void onFocusCallStateChanged(LinphoneCallState state);
	void onPendingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> call, LinphoneCallState callState);
	void onTransferingCallStateChanged(std::shared_ptr<LinphonePrivate::Call> transfered, LinphoneCallState newCallState);

	static void callStateChangedCb(LinphoneCore *lc, LinphoneCall *call, LinphoneCallState cstate, const char *message);
	static void transferStateChanged(LinphoneCore *lc, LinphoneCall *transfered, LinphoneCallState new_call_state);

	LinphoneCoreCbs *m_coreCbs;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_pendingCalls;
	std::list<std::shared_ptr<LinphonePrivate::Call>> m_transferingCalls;

};

}// end of namespace MediaConference

LINPHONE_END_NAMESPACE

#endif //CONFERENCE_PRIVATE_H
