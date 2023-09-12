/*
 * Copyright (c) 2010-2023 Belledonne Communications SARL.
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

#ifndef _L_PRESENCE_PERSON_H_
#define _L_PRESENCE_PERSON_H_

#include <memory>

#include "c-wrapper/c-wrapper.h"

// =============================================================================

LINPHONE_BEGIN_NAMESPACE

class PresenceActivity;
class PresenceModel;
class PresenceNote;
#ifdef HAVE_XML2
class XmlParsingContext;
#endif /* HAVE_XML2 */

class PresencePerson : public bellesip::HybridObject<LinphonePresencePerson, PresencePerson>, public UserDataAccessor {
public:
	PresencePerson(const std::string &id, time_t timestamp = time(nullptr));
	PresencePerson(const PresencePerson &other) = delete;
	virtual ~PresencePerson() = default;

	PresencePerson *clone() const override;

	// Friends
	friend PresenceModel;

	// Setters
	void setId(const std::string &id);

	// Getters
	const std::string &getId() const;
	unsigned int getNbActivities() const;
	unsigned int getNbActivitiesNotes() const;
	unsigned int getNbNotes() const;
	const std::shared_ptr<PresenceActivity> getNthActivity(unsigned int idx) const;
	const std::shared_ptr<PresenceNote> getNthActivitiesNote(unsigned int idx) const;
	const std::shared_ptr<PresenceNote> getNthNote(unsigned int idx) const;
	time_t getTimestamp() const;

	// Other
	LinphoneStatus addActivity(const std::shared_ptr<PresenceActivity> &activity);
	LinphoneStatus addActivitiesNote(const std::shared_ptr<PresenceNote> &note);
	LinphoneStatus addNote(const std::shared_ptr<PresenceNote> &note);
	void clearActivities();
	void clearActivitiesNotes();
	void clearNotes();
	bool hasActivities() const;
	bool hasActivitiesNotes() const;
	bool hasNotes() const;

private:
#ifdef HAVE_XML2
	int parsePidfXmlPresenceActivities(XmlParsingContext &xmlContext, unsigned int personIdx);
	int parsePidfXmlPresenceNotes(XmlParsingContext &xmlContext, unsigned int personIdx);
	int toXml(xmlTextWriterPtr writer) const;

	static constexpr std::string_view pidfXmlPrefix = "/pidf:presence/dm:person";
#endif /* HAVE_XML2 */

	time_t mTimestamp;
	std::string mId;
	std::vector<std::shared_ptr<PresenceActivity>> mActivities;
	std::vector<std::shared_ptr<PresenceNote>> mActivitiesNotes;
	std::vector<std::shared_ptr<PresenceNote>> mNotes;
};

LINPHONE_END_NAMESPACE

#endif // ifndef _L_PRESENCE_PERSON_H_
